// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// msg_assemble.cc
//
// assemble methods for DsMdvxMsg object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1999
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <Mdv/DsMdvxMsg.hh>
#include <didss/DsMsgPart.hh>
#include <toolsa/mem.h>
#include <toolsa/TaStr.hh>
using namespace std;

/////////////////////////////////////////////////////
// assemble readAllHeaders message
//
// Returns assembled message on success, NULL on error.
// getErrorStr() returns the error str for this call.

void *DsMdvxMsg::assembleReadAllHdrs(const DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> assembleReadAllHdrs <<-----" << endl;
  }

  clearAll();
  _clearErrStr();

  // set header attributes
  
  setType(MDVP_REQUEST_MESSAGE);
  setSubType(MDVP_READ_ALL_HDRS);

  // indicate that this is the start of a series of get messages
  // in fact there is only one in the series, the return will
  // always be of category endSeries

  setCategory(StartGet);

  // add calling client host and user
  
  addClientHost();
  addClientIpaddr();
  addClientUser();

  // add formats part

  _addReadFormat(mdvx._readFormat);

  // add read search specs

  if (_addReadSearch(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::assembleReadAllHdrs.\n";
    return NULL;
  }
  
  // time list as well?
  
  if (mdvx._readTimeListAlso) {
    _addReadTimeListAlso();
    _addTimeListOptions(mdvx._timeList.getMode(),
			mdvx._timeListUrl,
			mdvx._timeList.getStartTime(),
			mdvx._timeList.getEndTime(),
			mdvx._timeList.getGenTime(),
			mdvx._timeList.getSearchTime(),
			mdvx._timeList.getTimeMargin());
  }

  // set latest mod time?

  if (mdvx._timeList.checkLatestValidModTime()) {
    _addReadLatestValidModTime(mdvx._timeList.getLatestValidModTime());
  }

  // assemble
  
  ui08 *msg = DsMessage::assemble();

  if (_debug) {
    cerr << "--->> assembleReadAllHdrs <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

/////////////////////////////////////////////////////
// assemble readAllHeadersReturn message
// Returns assembled message.

void *DsMdvxMsg::assembleReadAllHdrsReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleReadAllHdrsReturn <<-----" << endl;
    _printReturnHeaderType(cerr);
  }

  clearAll();
  _clearErrStr();

  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(MDVP_READ_ALL_HDRS);

  // indicate that this is the end of the series of messages

  setCategory(EndSeries);

  // add formats part
  // echo back the read format in case it has been changed

  _addInternalFormat(mdvx._internalFormat);
  _addReadFormat(mdvx._readFormat);

  // add header parts

  if (_use32BitHeaders) {
    // 32-bit headers
    _addMasterHeader32(mdvx._mhdrFile, MDVP_MASTER_HEADER_FILE_PART_32);
    size_t nFields = MIN(mdvx._fhdrsFile.size(), mdvx._vhdrsFile.size());
    for (size_t i = 0; i < nFields; i++) {
      Mdvx::field_header_t fhdr = mdvx._fhdrsFile[i];
      Mdvx::vlevel_header_t vhdr = mdvx._vhdrsFile[i];
      _addFieldHeader32(fhdr, MDVP_FIELD_HEADER_FILE_PART_32);
      _addVlevelHeader32(vhdr, MDVP_VLEVEL_HEADER_FILE_PART_32,
                         fhdr.nz, fhdr.field_name);
    }
    for (size_t i = 0; i < mdvx._chdrsFile.size(); i++) {
      _addChunkHeader32(mdvx._chdrsFile[i],
                        MDVP_CHUNK_HEADER_FILE_PART_32);
    }
  } else {
    // 64-bit headers
    _addMasterHeader64(mdvx._mhdrFile, MDVP_MASTER_HEADER_FILE_PART_64);
    size_t nFields = MIN(mdvx._fhdrsFile.size(), mdvx._vhdrsFile.size());
    for (size_t i = 0; i < nFields; i++) {
      Mdvx::field_header_t fhdr = mdvx._fhdrsFile[i];
      Mdvx::vlevel_header_t vhdr = mdvx._vhdrsFile[i];
      _addFieldHeader64(fhdr, MDVP_FIELD_HEADER_FILE_PART_64);
      _addVlevelHeader64(vhdr, MDVP_VLEVEL_HEADER_FILE_PART_64,
                         fhdr.nz, fhdr.field_name);
    }
    for (size_t i = 0; i < mdvx._chdrsFile.size(); i++) {
      _addChunkHeader64(mdvx._chdrsFile[i],
                        MDVP_CHUNK_HEADER_FILE_PART_64);
    }
  }

  // add path in use

  _addPathInUse(mdvx.getPathInUse().c_str());

  // add the time list data if available

  _addTimeLists(mdvx);

  // assemble
  
  ui08 *msg = DsMessage::assemble();

  if (_debug) {
    cerr << "--->> assembleReadAllHdrsReturn <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

//////////////////////////////////////////////////////////////
// assemble readVolume message
//
// If _readAsSinglePart is true, the mdvx data will be returned
// as a buffer in a single part rather than in separate parts.
//
// Returns assembled message on success, NULL on error.
// getErrorStr() returns the error str for this call.

void *DsMdvxMsg::assembleReadVolume(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleReadVolume <<-----" << endl;
  }

  clearAll();
  _clearErrStr();

  // set header attributes
  
  setType(MDVP_REQUEST_MESSAGE);
  setSubType(MDVP_READ_VOLUME);

  // indicate that this is the start of a series of get messages
  // in fact there is only one in the series, the return will
  // always be of category endSeries

  setCategory(StartGet);

  // add calling client host and user
  
  addClientHost();
  addClientIpaddr();
  addClientUser();

  // add formats part

  _addReadFormat(mdvx._readFormat);

  // add read search specs

  if (_addReadSearch(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::assembleReadVolume.\n";
    return NULL;
  }

  // set the data volume specs

  _addReadQualifiers(mdvx);

  // time list as well?
  
  if (mdvx._readTimeListAlso) {
    _addReadTimeListAlso();
    _addTimeListOptions(mdvx._timeList.getMode(),
			mdvx._timeListUrl,
			mdvx._timeList.getStartTime(),
			mdvx._timeList.getEndTime(),
			mdvx._timeList.getGenTime(),
			mdvx._timeList.getSearchTime(),
			mdvx._timeList.getTimeMargin());
  }

  // set latest mod time?

  if (mdvx._timeList.checkLatestValidModTime()) {
    _addReadLatestValidModTime(mdvx._timeList.getLatestValidModTime());
  }

  // assemble
  
  ui08 *msg = DsMessage::assemble();

  if (_debug) {
    cerr << "--->> assembleReadVolume <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

/////////////////////////////////////////////////////
// assemble readVolumeReturn message
// Returns assembled message.

void *DsMdvxMsg::assembleReadVolumeReturn(const DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> assembleReadVolumeReturn <<-----" << endl;
    _printReturnHeaderType(cerr);
  }

  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(MDVP_READ_VOLUME);

  // indicate that this is the end of the series of messages
  
  setCategory(EndSeries);

  // add formats part
  // echo back the read format in case it has been changed

  _addInternalFormat(mdvx._internalFormat);
  _addReadFormat(mdvx._readFormat);

  // add the data and fields etc

  if (mdvx._internalFormat == Mdvx::FORMAT_NCF) {
    
    _addNcfHdrAndData(mdvx);

  } else {

    _addHdrsAndDataExtended(mdvx);

  } // if (mdvx._internalFormat == Mdvx::FORMAT_NCF)

  // add path in use

  _addPathInUse(mdvx.getPathInUse().c_str());

  // add the time list data if available

  _addTimeLists(mdvx);

  // assemble
  
  ui08 *msg = DsMessage::assemble();

  // print
  
  if (_debug) {
    cerr << "--->> assembleReadVolumeReturn <<-----" << endl;
    print(cerr, "  ");
    cerr << "--->> message length assembled: " << _lengthAssembled << endl;
  }

  return msg;

}

//////////////////////////////////////////////////////////////
// assemble readVsection message
//
// If _readAsSinglePart is true, the mdvx data will be returned
// as a buffer in a single part rather than in separate parts.
//
// Returns assembled message on success, NULL on error.
// getErrorStr() returns the error str for this call.

void *DsMdvxMsg::assembleReadVsection(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleReadVsection <<-----" << endl;
  }

  clearAll();
  _clearErrStr();

  // set header attributes
  
  setType(MDVP_REQUEST_MESSAGE);
  setSubType(MDVP_READ_VSECTION);

  // indicate that this is the start of a series of get messages
  // in fact there is only one in the series, the return will
  // always be of category endSeries

  setCategory(StartGet);

  // add calling client host and user
  
  addClientHost();
  addClientIpaddr();
  addClientUser();

  // add formats part

  _addReadFormat(mdvx._readFormat);

  // add read search specs

  if (_addReadSearch(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::assembleReadVsection.\n";
    return NULL;
  }

  // set the data volume specs
  
  _addReadQualifiers(mdvx);

  // add the waypoint information

  _addReadVsectQualifiers(mdvx);

  // time list as well?
  
  if (mdvx._readTimeListAlso) {
    _addReadTimeListAlso();
    _addTimeListOptions(mdvx._timeList.getMode(),
			mdvx._timeListUrl,
			mdvx._timeList.getStartTime(),
			mdvx._timeList.getEndTime(),
			mdvx._timeList.getGenTime(),
			mdvx._timeList.getSearchTime(),
			mdvx._timeList.getTimeMargin());
  }

  // set latest mod time?

  if (mdvx._timeList.checkLatestValidModTime()) {
    _addReadLatestValidModTime(mdvx._timeList.getLatestValidModTime());
  }

  // assemble
  
  ui08 *msg = DsMessage::assemble();

  if (_debug) {
    cerr << "--->> assembleReadVsection <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

/////////////////////////////////////////////////////
// assemble readVsectionReturn message
// Returns assembled message.

void *DsMdvxMsg::assembleReadVsectionReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleReadVsectionReturn <<-----" << endl;
    _printReturnHeaderType(cerr);
  }

  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(MDVP_READ_VSECTION);

  // add formats part
  // echo back the read format in case it has been changed

  _addInternalFormat(mdvx._internalFormat);
  _addReadFormat(mdvx._readFormat);

  // indicate that this is the end of the series of messages
  
  setCategory(EndSeries);
  
  // add the data and fields etc

  if (mdvx._internalFormat == Mdvx::FORMAT_NCF) {
    
    _addNcfHdrAndData(mdvx);

  } else {
    
    _addHdrsAndDataExtended(mdvx);

  } // if (mdvx._internalFormat == Mdvx::FORMAT_NCF)

  // add path in use

  _addPathInUse(mdvx.getPathInUse().c_str());

  // add waypts, segments and sample pts
  
  _addReturnVsectInfo(mdvx);

  // add the time list data if available

  _addTimeLists(mdvx);

  // assemble
  
  ui08 *msg = DsMessage::assemble();

  if (_debug) {
    cerr << "--->> assembleReadVsectionReturn <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

////////////////////////////////
// assemble write message
// Returns assembled message.

void *DsMdvxMsg::assembleWrite(msg_subtype_t subtype,
			       const DsMdvx &mdvx,
			       const string &output_url)

{

  if (_debug) {
    cerr << "--->> assembleWrite <<-----" << endl;
  }

  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REQUEST_MESSAGE);
  setSubType(subtype);

  // indicate that this is the start of a series of put messages
  // in fact there is only one in the series, the return will
  // always be of category endSeries

  setCategory(StartPut);

  // add calling client host and user
  
  addClientHost();
  addClientIpaddr();
  addClientUser();

  // add URL

  addURL(output_url);

  // add formats part

  _addWriteFormat(mdvx._writeFormat);
  _addInternalFormat(mdvx._internalFormat);

  // add write options and data

  _addAppName(mdvx.getAppName());
  _addWriteOptions(mdvx._writeAsForecast,
                   mdvx._writeLdataInfo,
                   mdvx._useExtendedPaths,
                   mdvx._ifForecastWriteAsForecast);

  if (mdvx._internalFormat == Mdvx::FORMAT_NCF) {
    _addNcfHdrAndData(mdvx);
  } else {
    _addHdrsAndData(mdvx);
  }
  
  // add netCDF translation
  
  if (mdvx._writeFormat == Mdvx::FORMAT_NCF) {
    _addConvertMdv2NcfPart(mdvx);
  }
  
  // assemble
  
  ui08 *msg = DsMessage::assemble();

  if (_debug) {
    cerr << "--->> assembleWrite <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

/////////////////////////////////////////////////////
// assemble write return message
// Returns assembled message.

void *DsMdvxMsg::assembleWriteReturn(msg_subtype_t subtype,
				     const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleWriteReturn <<-----" << endl;
    _printReturnHeaderType(cerr);
  }

  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(subtype);
  
  // indicate that this is the end of the series of messages
  
  setCategory(EndSeries);

  // echo back the write format for information, in case it has
  // been changed by the server

  _addWriteFormat(mdvx._writeFormat);

  // add path in use

  _addPathInUse(mdvx.getPathInUse().c_str());

  // assemble
  
  ui08 *msg = DsMessage::assemble();

  if (_debug) {
    cerr << "--->> assembleWriteReturn <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

////////////////////////////////
// assemble CompileTimeList message
// Returns assembled message.

void *DsMdvxMsg::assembleCompileTimeList(const DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> assembleCompileTimeList <<-----" << endl;
  }

  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REQUEST_MESSAGE);
  setSubType(MDVP_COMPILE_TIME_LIST);

  // indicate that this is the start of a series of get messages
  // in fact there is only one in the series, the return will
  // always be of category endSeries

  setCategory(StartGet);

  // add calling client host and user
  
  addClientHost();
  addClientIpaddr();
  addClientUser();

  // add formats part

  _addReadFormat(mdvx._readFormat);

  // add time list request

  _addTimeListOptions(mdvx._timeList.getMode(),
		      mdvx._timeListUrl,
		      mdvx._timeList.getStartTime(),
		      mdvx._timeList.getEndTime(),
		      mdvx._timeList.getGenTime(),
		      mdvx._timeList.getSearchTime(),
		      mdvx._timeList.getTimeMargin());
  
  // optional part to constrain forecast lead times
  
  _addConstrainLeadTimes(mdvx.getConstrainFcastLeadTimes(),
			 mdvx.getMinFcastLeadTime(),
			 mdvx.getMaxFcastLeadTime(),
			 mdvx.getSpecifyFcastByGenTime());
  
  // horizontal limits

  if (mdvx._readHorizLimitsSet) {
    _addReadHorizLimits(mdvx._readMinLat, mdvx._readMinLon,
			mdvx._readMaxLat, mdvx._readMaxLon);
  }

  // set latest mod time?

  if (mdvx._timeList.checkLatestValidModTime()) {
    _addReadLatestValidModTime(mdvx._timeList.getLatestValidModTime());
  }

  // assemble
  
  ui08 *msg = DsMessage::assemble();

  if (_debug) {
    cerr << "--->> assembleCompileTimeList <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

/////////////////////////////////////////////////////
// assemble write return message
// Returns assembled message.

void *DsMdvxMsg::assembleCompileTimeListReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleCompileTimeListReturn <<-----" << endl;
    _printReturnHeaderType(cerr);
  }

  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(MDVP_COMPILE_TIME_LIST);
  
  // indicate that this is the end of the series of messages
  
  setCategory(EndSeries);

  // add formats part

  _addInternalFormat(mdvx._internalFormat);
  _addReadFormat(mdvx._readFormat);

  // add the time list data

  _addTimeLists(mdvx);

  // assemble
  
  ui08 *msg = DsMessage::assemble();

  if (_debug) {
    cerr << "--->> assembleCompileTimeListReturn <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

//////////////////////////////////////////////////////////////
// assemble compileTimeHeight message
//
// Returns assembled message on success, NULL on error.
// getErrorStr() returns the error str for this call.

void *DsMdvxMsg::assembleCompileTimeHeight(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleCompileTimeHeight <<-----" << endl;
  }

  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REQUEST_MESSAGE);
  setSubType(MDVP_COMPILE_TIME_HEIGHT);

  // indicate that this is the start of a series of get messages
  // in fact there is only one in the series, the return will
  // always be of category endSeries

  setCategory(StartGet);
  
  // add calling client host and user
  
  addClientHost();
  addClientIpaddr();
  addClientUser();

  // add formats part

  _addReadFormat(mdvx._readFormat);

  // set the read qualifiers
  
  _addReadQualifiers(mdvx);
  
  // add the vert section information - a single waypt is expected
  
  _addReadVsectWayPts(mdvx._vsectWayPts);
  if (mdvx._vsectDisableInterp) {
    _addReadVsectInterpDisabled();
  }

  // add time list options
  
  _addTimeListOptions(mdvx._timeList.getMode(),
		      mdvx._timeListUrl,
		      mdvx._timeList.getStartTime(),
		      mdvx._timeList.getEndTime(),
		      mdvx._timeList.getGenTime(),
		      mdvx._timeList.getSearchTime(),
		      mdvx._timeList.getTimeMargin());
  
  // optional part to constrain forecast lead times
  
  _addConstrainLeadTimes(mdvx.getConstrainFcastLeadTimes(),
			 mdvx.getMinFcastLeadTime(),
			 mdvx.getMaxFcastLeadTime(),
			 mdvx.getSpecifyFcastByGenTime());
  
  // set latest mod time?
  
  if (mdvx._timeList.checkLatestValidModTime()) {
    _addReadLatestValidModTime(mdvx._timeList.getLatestValidModTime());
  }

  // assemble
  
  ui08 *msg = DsMessage::assemble();

  if (_debug) {
    cerr << "--->> assembleCompileTimeHeight <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

/////////////////////////////////////////////////////
// assemble compileVsectionReturn message
// Returns assembled message.

void *DsMdvxMsg::assembleCompileTimeHeightReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleCompileTimeHeightReturn <<-----" << endl;
    _printReturnHeaderType(cerr);
  }
  
  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(MDVP_COMPILE_TIME_HEIGHT);

  // indicate that this is the end of the series of messages
  
  setCategory(EndSeries);

  // add formats part

  _addInternalFormat(mdvx._internalFormat);
  _addReadFormat(mdvx._readFormat);

  if (mdvx._internalFormat == Mdvx::FORMAT_NCF) {
    
    _addNcfHdrAndData(mdvx);

  } else {
    
    if (mdvx._readFormat == Mdvx::FORMAT_XML) {
      
      // add XML parts
      
      _addXmlHeader(mdvx.getXmlHdr());
      _addXmlBuffer(mdvx.getXmlBuf());
      
    } else if (mdvx._readAsSingleBuffer) {
      
      // add object as single buffer
      
      MemBuf buf;
      if (_use32BitHeaders) {
        mdvx.writeToBuffer32(buf);
      } else {
        mdvx.writeToBuffer64(buf);
      }
      _addSingleBuffer(buf);
      
    } else {
      
      if (_use32BitHeaders) {
        // 32-bit
        _addMasterHeader32(mdvx.getMasterHeader(), MDVP_MASTER_HEADER_PART_32);
        for (size_t i = 0; i < mdvx.getNFields(); i++) {
          MdvxField *field = mdvx.getFieldByNum(i);
          field->compressIfRequested();
          Mdvx::field_header_t fhdr = field->getFieldHeader();
          Mdvx::vlevel_header_t vhdr = field->getVlevelHeader();
          _addFieldHeader32(fhdr, MDVP_FIELD_HEADER_PART_32);
          _addVlevelHeader32(vhdr, MDVP_VLEVEL_HEADER_PART_32,
                             fhdr.nz, fhdr.field_name);
          _addFieldData(*field);
        }
      } else {
        // 64-bit
        _addMasterHeader64(mdvx.getMasterHeader(), MDVP_MASTER_HEADER_PART_64);
        for (size_t i = 0; i < mdvx.getNFields(); i++) {
          MdvxField *field = mdvx.getFieldByNum(i);
          field->compressIfRequested();
          Mdvx::field_header_t fhdr = field->getFieldHeader();
          Mdvx::vlevel_header_t vhdr = field->getVlevelHeader();
          _addFieldHeader64(fhdr, MDVP_FIELD_HEADER_PART_64);
          _addVlevelHeader64(vhdr, MDVP_VLEVEL_HEADER_PART_64,
                             fhdr.nz, fhdr.field_name);
          _addFieldData64(*field);
        }
      }

    }

  }

  // add waypts
  
  _addReadVsectWayPts(mdvx._vsectWayPts);
  
  // add the time list data

  _addTimeLists(mdvx);

  // assemble
  
  ui08 *msg = DsMessage::assemble();

  if (_debug) {
    cerr << "--->> assembleCompileTimeHeightReturn <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

#ifdef NOTNOW
/////////////////////////////////////////////
// assemble message to convert mdv to ncf
// Returns assembled message, NULL on failure

void *DsMdvxMsg::assembleConvertMdv2Ncf(const DsMdvx &mdvx,
                                        const string &trans_url)

{

  if (_debug) {
    cerr << "--->> assembleConvertMdv2Ncf <<-----" << endl;
  }
  
  clearAll();
  _clearErrStr();
  
  if (mdvx._internalFormat != Mdvx::FORMAT_MDV) {
    TaStr::AddStr(_errStr, "ERROR - DsMdvxMsg::assembleConvertMdv2Ncf");
    TaStr::AddStr(_errStr, "  Format must be MDV");
    TaStr::AddStr(_errStr, "  Internal format is: ",
                  Mdvx::format2Str(mdvx._internalFormat));
    return NULL;
  }
  
  // set header attributes
  
  setType(MDVP_REQUEST_MESSAGE);
  setSubType(MDVP_CONVERT_MDV_TO_NCF);
  
  // indicate that this is the start of a series of put messages
  // in fact there is only one in the series, the return will
  // always be of category endSeries

  setCategory(StartPut);
  
  // add calling client host and user
  
  addClientHost();
  addClientIpaddr();
  addClientUser();
  _addAppName(mdvx.getAppName());
  
  // add URL
  
  addURL(trans_url);

  // add formats
  
  _addInternalFormat(mdvx._internalFormat);
  
  // add mdv data

  _addHdrsAndData(mdvx);

  // add netCDF translation
  
  _addConvertMdv2NcfPart(mdvx);
  
  // assemble
  
  ui08 *msg = DsMessage::assemble();
  
  if (_debug) {
    cerr << "--->> assembleConvertMdv2Ncf <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

/////////////////////////////////////////////////////
// assemble return message from convert 2 nc
// Returns assembled message.

void *DsMdvxMsg::assembleConvertMdv2NcfReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleConvertMdv2NcfReturn <<-----" << endl;
    _printReturnHeaderType(cerr);
  }

  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(MDVP_CONVERT_MDV_TO_NCF);

  // indicate that this is the end of the series of messages
  
  setCategory(EndSeries);

  // add formats part

  _addInternalFormat(mdvx._internalFormat);

  // add the ncf data
  
  _addNcfHdrAndData(mdvx);

  // assemble
  
  ui08 *msg = DsMessage::assemble();
  
  // print
  
  if (_debug) {
    cerr << "--->> assembleConvertMdv2NcfReturn <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

/////////////////////////////////////////////
// assemble message to convert ncf to mdv
// Returns assembled message, NULL on failure

void *DsMdvxMsg::assembleConvertNcf2Mdv(const DsMdvx &mdvx,
                                        const string &trans_url)

{

  if (_debug) {
    cerr << "--->> assembleConvertNcf2Mdv <<-----" << endl;
  }
  
  clearAll();
  _clearErrStr();
  
  if (mdvx._internalFormat != Mdvx::FORMAT_NCF) {
    TaStr::AddStr(_errStr, "ERROR - DsMdvxMsg::assembleConvertNcf2Mdv");
    TaStr::AddStr(_errStr, "  Format must be NCF");
    TaStr::AddStr(_errStr, "  Internal format is: ",
                  Mdvx::format2Str(mdvx._internalFormat));
    return NULL;
  }
  
  // set header attributes
  
  setType(MDVP_REQUEST_MESSAGE);
  setSubType(MDVP_CONVERT_NCF_TO_MDV);
  
  // indicate that this is the start of a series of put messages
  // in fact there is only one in the series, the return will
  // always be of category endSeries
  
  setCategory(StartPut);
  
  // add calling client host and user
  
  addClientHost();
  addClientIpaddr();
  addClientUser();
  _addAppName(mdvx.getAppName());
  
  // add URL
  
  addURL(trans_url);

  // add formats

  _addInternalFormat(mdvx._internalFormat);

  // add read qualifiers

  _addReadQualifiers(mdvx);

  // add vsect read qualifiers, in case applicable

  _addReadVsectQualifiers(mdvx);
  
  // add the ncf data
  
  _addNcfHdrAndData(mdvx);

  // assemble
  
  ui08 *msg = DsMessage::assemble();
  
  if (_debug) {
    cerr << "--->> assembleConvertNcf2Mdv <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

/////////////////////////////////////////////////////
// assemble return message from convert 2 nc
// Returns assembled message.

void *DsMdvxMsg::assembleConvertNcf2MdvReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleConvertNcf2MdvReturn <<-----" << endl;
    _printReturnHeaderType(cerr);
  }

  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(MDVP_CONVERT_NCF_TO_MDV);

  // indicate that this is the end of the series of messages
  
  setCategory(EndSeries);
  
  // add formats part
  
  _addInternalFormat(mdvx._internalFormat);
  
  // add mdv data

  _addHdrsAndDataExtended(mdvx);

  // add vsect info in case applicable

  _addReturnVsectInfo(mdvx);

  // assemble
  
  ui08 *msg = DsMessage::assemble();
  
  // print
  
  if (_debug) {
    cerr << "--->> assembleConvertNcf2MdvReturn <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

////////////////////////////////////////////////
// assemble message to read all headers from an NCF file
//
// Returns assembled message, NULL on failure

void *DsMdvxMsg::assembleReadAllHdrsNcf(const DsMdvx &mdvx,
					   const string &trans_url)

{

  if (_debug) {
    cerr << "--->> assembleReadAllHdrsNcf <<-----" << endl;
  }
  
  clearAll();
  _clearErrStr();
  
  if (mdvx._internalFormat != Mdvx::FORMAT_NCF) {
    TaStr::AddStr(_errStr, "ERROR - DsMdvxMsg::assembleReadAllHdrsNcf");
    TaStr::AddStr(_errStr, "  Format must be NCF");
    TaStr::AddStr(_errStr, "  Internal format is: ",
                  Mdvx::format2Str(mdvx._internalFormat));
    return NULL;
  }
  
  // set header attributes
  
  setType(MDVP_REQUEST_MESSAGE);
  setSubType(MDVP_READ_ALL_HDRS_NCF);
  
  // indicate that this is the start of a series of get messages
  // in fact there is only one in the series, the return will
  // always be of category endSeries
  
  setCategory(StartGet);
  
  // add calling client host and user
  
  addClientHost();
  addClientIpaddr();
  addClientUser();
  _addAppName(mdvx.getAppName());
  
  // add URL
  
  addURL(trans_url);

  // add formats
  
  _addInternalFormat(mdvx._internalFormat);
  _addReadFormat(mdvx._readFormat);
  
  // add path in use

  _addPathInUse(mdvx.getPathInUse().c_str());

  // assemble
  
  ui08 *msg = DsMessage::assemble();
  
  if (_debug) {
    cerr << "--->> assembleReadAllHdrsNcf <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

/////////////////////////////////////////////////////
// assemble return message from read all headers NCF
//
// Returns assembled message.

void *DsMdvxMsg::assembleReadAllHdrsNcfReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleReadAllHdrsNcfReturn <<-----" << endl;
    _printReturnHeaderType(cerr);
  }
  
  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(MDVP_READ_ALL_HDRS_NCF);

  // indicate that this is the end of the series of messages
  
  setCategory(EndSeries);
  
  // add formats part
  // echo back the read format in case it has been changed
  
  _addInternalFormat(mdvx._internalFormat);
  _addReadFormat(mdvx._readFormat);

  if (mdvx._internalFormat == Mdvx::FORMAT_NCF) {
    
    // add the ncf headers
    
    _addNcfHdr(mdvx);

  } else {
    
    // add header parts

    if (_use32BitHeaders) {
      // 32-bit headers
      _addMasterHeader32(mdvx._mhdrFile, MDVP_MASTER_HEADER_FILE_PART_32);
      size_t nFields = MIN(mdvx._fhdrsFile.size(), mdvx._vhdrsFile.size());
      for (size_t i = 0; i < nFields; i++) {
        Mdvx::field_header_t fhdr = mdvx._fhdrsFile[i];
        Mdvx::vlevel_header_t vhdr = mdvx._vhdrsFile[i];
        _addFieldHeader32(fhdr, MDVP_FIELD_HEADER_FILE_PART_32);
        _addVlevelHeader32(vhdr, MDVP_VLEVEL_HEADER_FILE_PART_32,
                           fhdr.nz, fhdr.field_name);
      }
      for (size_t i = 0; i < mdvx._chdrsFile.size(); i++) {
        _addChunkHeader32(mdvx._chdrsFile[i],
                          MDVP_CHUNK_HEADER_FILE_PART_32);
      }
    } else {
      // 64-bit headers
      _addMasterHeader64(mdvx._mhdrFile, MDVP_MASTER_HEADER_FILE_PART_64);
      size_t nFields = MIN(mdvx._fhdrsFile.size(), mdvx._vhdrsFile.size());
      for (size_t i = 0; i < nFields; i++) {
        Mdvx::field_header_t fhdr = mdvx._fhdrsFile[i];
        Mdvx::vlevel_header_t vhdr = mdvx._vhdrsFile[i];
        _addFieldHeader64(fhdr, MDVP_FIELD_HEADER_FILE_PART_64);
        _addVlevelHeader64(vhdr, MDVP_VLEVEL_HEADER_FILE_PART_64,
                           fhdr.nz, fhdr.field_name);
      }
      for (size_t i = 0; i < mdvx._chdrsFile.size(); i++) {
        _addChunkHeader64(mdvx._chdrsFile[i],
                          MDVP_CHUNK_HEADER_FILE_PART_64);
      }
    }

  }

  // add path in use

  _addPathInUse(mdvx.getPathInUse().c_str());

  // assemble
  
  ui08 *msg = DsMessage::assemble();
  
  // print
  
  if (_debug) {
    cerr << "--->> assembleReadAllHdrsNcfReturn <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

////////////////////////////////////////////////
// assemble message to read an NCF file
//
// Returns assembled message, NULL on failure

void *DsMdvxMsg::assembleReadNcf(const DsMdvx &mdvx,
                                 const string &trans_url)

{

  if (_debug) {
    cerr << "--->> assembleReadNcf <<-----" << endl;
  }
  
  clearAll();
  _clearErrStr();
  
  if (mdvx._internalFormat != Mdvx::FORMAT_NCF) {
    TaStr::AddStr(_errStr, "ERROR - DsMdvxMsg::assembleReadNcf");
    TaStr::AddStr(_errStr, "  Format must be NCF");
    TaStr::AddStr(_errStr, "  Internal format is: ",
                  Mdvx::format2Str(mdvx._internalFormat));
    return NULL;
  }
  
  // set header attributes
  
  setType(MDVP_REQUEST_MESSAGE);
  setSubType(MDVP_READ_NCF);
  
  // indicate that this is the start of a series of get messages
  // in fact there is only one in the series, the return will
  // always be of category endSeries
  
  setCategory(StartGet);
  
  // add calling client host and user
  
  addClientHost();
  addClientIpaddr();
  addClientUser();
  _addAppName(mdvx.getAppName());
  
  // add URL
  
  addURL(trans_url);

  // add formats
  
  _addInternalFormat(mdvx._internalFormat);
  
  // add read qualifiers
  
  _addReadQualifiers(mdvx);

  // add vsect read qualifiers, in case applicable

  _addReadVsectQualifiers(mdvx);
  
  // add path in use

  _addPathInUse(mdvx.getPathInUse().c_str());

  // assemble
  
  ui08 *msg = DsMessage::assemble();
  
  if (_debug) {
    cerr << "--->> assembleReadNcf <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

/////////////////////////////////////////////////////
// assemble return message from read NCF
//
// Returns assembled message.

void *DsMdvxMsg::assembleReadNcfReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleReadNcfReturn <<-----" << endl;
    _printReturnHeaderType(cerr);
  }

  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(MDVP_READ_NCF);

  // indicate that this is the end of the series of messages
  
  setCategory(EndSeries);
  
  // add formats part
  
  _addInternalFormat(mdvx._internalFormat);

  if (mdvx._internalFormat == Mdvx::FORMAT_NCF) {
    
    // add the ncf data
    
    _addNcfHdrAndData(mdvx);

  } else {
    
    // add mdv data
    
    _addHdrsAndDataExtended(mdvx);
    
    // add vsect info in case applicable
    
    _addReturnVsectInfo(mdvx);

  }

  // add path in use

  _addPathInUse(mdvx.getPathInUse().c_str());

  // assemble
  
  ui08 *msg = DsMessage::assemble();
  
  // print
  
  if (_debug) {
    cerr << "--->> assembleReadNcfReturn <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

////////////////////////////////////////////////
// assemble message to read all headers from an RADX file
//
// Returns assembled message, NULL on failure

void *DsMdvxMsg::assembleReadAllHdrsRadx(const DsMdvx &mdvx,
                                         const string &trans_url)

{

  if (_debug) {
    cerr << "--->> assembleReadAllHdrsRadx <<-----" << endl;
  }
  
  clearAll();
  _clearErrStr();
  
  if (mdvx._internalFormat != Mdvx::FORMAT_RADX) {
    TaStr::AddStr(_errStr, "ERROR - DsMdvxMsg::assembleReadAllHdrsRadx");
    TaStr::AddStr(_errStr, "  Format must be RADX");
    TaStr::AddStr(_errStr, "  Internal format is: ",
                  Mdvx::format2Str(mdvx._internalFormat));
    return NULL;
  }
  
  // set header attributes
  
  setType(MDVP_REQUEST_MESSAGE);
  setSubType(MDVP_READ_ALL_HDRS_RADX);
  
  // indicate that this is the start of a series of get messages
  // in fact there is only one in the series, the return will
  // always be of category endSeries
  
  setCategory(StartGet);
  
  // add calling client host and user
  
  addClientHost();
  addClientIpaddr();
  addClientUser();
  _addAppName(mdvx.getAppName());
  
  // add URL
  
  addURL(trans_url);

  // add formats
  
  _addInternalFormat(mdvx._internalFormat);
  _addReadFormat(mdvx._readFormat);
  
  // add path in use

  _addPathInUse(mdvx.getPathInUse().c_str());

  // assemble
  
  ui08 *msg = DsMessage::assemble();
  
  if (_debug) {
    cerr << "--->> assembleReadAllHdrsRadx <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

/////////////////////////////////////////////////////
// assemble return message from read all headers RADX
//
// Returns assembled message.

void *DsMdvxMsg::assembleReadAllHdrsRadxReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleReadAllHdrsRadxReturn <<-----" << endl;
    _printReturnHeaderType(cerr);
  }
  
  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(MDVP_READ_ALL_HDRS_RADX);

  // indicate that this is the end of the series of messages
  
  setCategory(EndSeries);
  
  // add formats part
  // echo back the read format in case it has been changed
  
  _addInternalFormat(mdvx._internalFormat);
  _addReadFormat(mdvx._readFormat);
  
  if (mdvx._internalFormat == Mdvx::FORMAT_NCF) {
    
    // add the ncf headers
    
    _addNcfHdr(mdvx);

  } else {
    
    // add header parts

    if (_use32BitHeaders) {
      // 32-bit headers
      _addMasterHeader32(mdvx._mhdrFile, MDVP_MASTER_HEADER_FILE_PART_32);
      size_t nFields = MIN(mdvx._fhdrsFile.size(), mdvx._vhdrsFile.size());
      for (size_t i = 0; i < nFields; i++) {
        Mdvx::field_header_t fhdr = mdvx._fhdrsFile[i];
        Mdvx::vlevel_header_t vhdr = mdvx._vhdrsFile[i];
        _addFieldHeader32(fhdr, MDVP_FIELD_HEADER_FILE_PART_32);
        _addVlevelHeader32(vhdr, MDVP_VLEVEL_HEADER_FILE_PART_32,
                           fhdr.nz, fhdr.field_name);
      }
      for (size_t i = 0; i < mdvx._chdrsFile.size(); i++) {
        _addChunkHeader32(mdvx._chdrsFile[i],
                          MDVP_CHUNK_HEADER_FILE_PART_32);
      }
    } else {
      // 64-bit headers
      _addMasterHeader64(mdvx._mhdrFile, MDVP_MASTER_HEADER_FILE_PART_64);
      size_t nFields = MIN(mdvx._fhdrsFile.size(), mdvx._vhdrsFile.size());
      for (size_t i = 0; i < nFields; i++) {
        Mdvx::field_header_t fhdr = mdvx._fhdrsFile[i];
        Mdvx::vlevel_header_t vhdr = mdvx._vhdrsFile[i];
        _addFieldHeader64(fhdr, MDVP_FIELD_HEADER_FILE_PART_64);
        _addVlevelHeader64(vhdr, MDVP_VLEVEL_HEADER_FILE_PART_64,
                           fhdr.nz, fhdr.field_name);
      }
      for (size_t i = 0; i < mdvx._chdrsFile.size(); i++) {
        _addChunkHeader64(mdvx._chdrsFile[i],
                          MDVP_CHUNK_HEADER_FILE_PART_64);
      }
    }

  } // if (mdvx._internalFormat == Mdvx::FORMAT_NCF)

  // add path in use

  _addPathInUse(mdvx.getPathInUse().c_str());

  // assemble
  
  ui08 *msg = DsMessage::assemble();
  
  // print
  
  if (_debug) {
    cerr << "--->> assembleReadAllHdrsRadxReturn <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

////////////////////////////////////////////////
// assemble message to read a RADX file
//
// Returns assembled message, NULL on failure

void *DsMdvxMsg::assembleReadRadx(const DsMdvx &mdvx,
                                  const string &trans_url)

{

  if (_debug) {
    cerr << "--->> assembleReadRadx <<-----" << endl;
  }
  
  clearAll();
  _clearErrStr();
  
  if (mdvx._internalFormat != Mdvx::FORMAT_RADX) {
    TaStr::AddStr(_errStr, "ERROR - DsMdvxMsg::assembleReadRadx");
    TaStr::AddStr(_errStr, "  Format must be RADX");
    TaStr::AddStr(_errStr, "  Internal format is: ",
                  Mdvx::format2Str(mdvx._internalFormat));
    return NULL;
  }
  
  // set header attributes
  
  setType(MDVP_REQUEST_MESSAGE);
  setSubType(MDVP_READ_RADX);
  
  // indicate that this is the start of a series of get messages
  // in fact there is only one in the series, the return will
  // always be of category endSeries
  
  setCategory(StartGet);
  
  // add calling client host and user
  
  addClientHost();
  addClientIpaddr();
  addClientUser();
  _addAppName(mdvx.getAppName());
  
  // add URL
  
  addURL(trans_url);
  
  // add formats
  
  _addInternalFormat(mdvx._internalFormat);

  // add read qualifiers

  _addReadQualifiers(mdvx);

  // add vsect read qualifiers, in case applicable

  _addReadVsectQualifiers(mdvx);
  
  // add path in use

  _addPathInUse(mdvx.getPathInUse().c_str());

  // assemble
  
  ui08 *msg = DsMessage::assemble();
  
  if (_debug) {
    cerr << "--->> assembleReadRadx <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;
  
}

/////////////////////////////////////////////////////
// assemble return message from read RADX file
//
// Returns assembled message.

void *DsMdvxMsg::assembleReadRadxReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleReadRadxReturn <<-----" << endl;
    _printReturnHeaderType(cerr);
  }

  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(MDVP_READ_RADX);

  // indicate that this is the end of the series of messages
  
  setCategory(EndSeries);
  
  // add formats part
  
  _addInternalFormat(mdvx._internalFormat);

  if (mdvx._internalFormat == Mdvx::FORMAT_NCF) {
    
    // add the ncf data
    
    _addNcfHdrAndData(mdvx);

  } else {
    
    // add mdv data
    
    _addHdrsAndDataExtended(mdvx);
    
    // add vsect info in case applicable
    
    _addReturnVsectInfo(mdvx);

  }

  // add path in use

  _addPathInUse(mdvx.getPathInUse().c_str());

  // assemble
  
  ui08 *msg = DsMessage::assemble();
  
  // print
  
  if (_debug) {
    cerr << "--->> assembleReadRadxReturn <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

/////////////////////////////////////////////
// assemble message to constrain ncf using
// read MDVX read constraints
// Returns assembled message, NULL on failure

void *DsMdvxMsg::assembleConstrainNcf(const DsMdvx &mdvx,
                                      const string &trans_url)

{

  if (_debug) {
    cerr << "--->> assembleConstrainNcf <<-----" << endl;
  }
  
  clearAll();
  _clearErrStr();
  
  if (mdvx._internalFormat != Mdvx::FORMAT_NCF) {
    TaStr::AddStr(_errStr, "ERROR - DsMdvxMsg::assembleConstrainNcf");
    TaStr::AddStr(_errStr, "  Format must be NCF");
    TaStr::AddStr(_errStr, "  Internal format is: ",
                  Mdvx::format2Str(mdvx._internalFormat));
    return NULL;
  }
  
  // set header attributes
  
  setType(MDVP_REQUEST_MESSAGE);
  setSubType(MDVP_CONSTRAIN_NCF);
  
  // indicate that this is the start of a series of put messages
  // in fact there is only one in the series, the return will
  // always be of category endSeries
  
  setCategory(StartPut);
  
  // add calling client host and user
  
  addClientHost();
  addClientIpaddr();
  addClientUser();
  _addAppName(mdvx.getAppName());
  
  // add URL
  
  addURL(trans_url);

  // add formats

  _addInternalFormat(mdvx._internalFormat);
  
  // add read qualifiers

  _addReadQualifiers(mdvx);

  // add vsect read qualifiers, in case applicable

  _addReadVsectQualifiers(mdvx);
  
  // add the ncf data
  
  _addNcfHdrAndData(mdvx);

  // assemble
  
  ui08 *msg = DsMessage::assemble();
  
  if (_debug) {
    cerr << "--->> assembleConstrainNcf <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

/////////////////////////////////////////////////////
// assemble return message from constrain ncf
// Returns assembled message.

void *DsMdvxMsg::assembleConstrainNcfReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleConstrainNcfReturn <<-----" << endl;
    _printReturnHeaderType(cerr);
  }

  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(MDVP_CONSTRAIN_NCF);

  // indicate that this is the end of the series of messages
  
  setCategory(EndSeries);

  // add formats part

  _addInternalFormat(mdvx._internalFormat);
  
  // add the ncf data
  
  _addNcfHdrAndData(mdvx);

  // assemble
  
  ui08 *msg = DsMessage::assemble();
  
  // print
  
  if (_debug) {
    cerr << "--->> assembleConstrainNcfReturn <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

#endif

///////////////////////////////////
// assemble an error return message
//

void *DsMdvxMsg::assembleErrorReturn(const int requestSubtype,
				     const string &errorStr,
				     bool noFilesFoundOnRead /* = false*/ )
  
{

  if (_debug) {
    cerr << "--->> assembleErrorReturn <<-----" << endl;
  }

  clearAll();
  _clearErrStr();

  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(requestSubtype);
  setCategory(EndSeries);
  _error = -1;
  if (errorStr.size() > 0) {
    addErrString(errorStr);
  }
   
  // add noFilesFoundOnRead flag

  if (noFilesFoundOnRead) {
    _addNoFilesFoundOnRead();
  }

  // assemble
  
  ui08 *msg = DsMessage::assemble();

  if (_debug) {
    print(cerr, "  ");
    cerr << "--->> assembleErrorReturn <<-----" << endl;
  }

  return msg;

}

/////////////////////////////////////////
// print type of headers being returned

void DsMdvxMsg::_printReturnHeaderType(ostream &out) const
{
  if (_use32BitHeaders) {
    out << "=====>> returning 32-bit headers <<=====" << endl;
  } else {
    out << "=====>> returning 64-bit headers <<=====" << endl;
  }
}

////////////////////////
// add headers and data

void DsMdvxMsg::_addHdrsAndData(const DsMdvx &mdvx)
{

  // add mdv headers and data

  if (_use32BitHeaders) {
    // 32-bit
    _addMasterHeader32(mdvx.getMasterHeader(), MDVP_MASTER_HEADER_PART_32);
    for (size_t i = 0; i < mdvx.getNFields(); i++) {
      MdvxField *field = mdvx.getFieldByNum(i);
      field->compressIfRequested();
      Mdvx::field_header_t fhdr = field->getFieldHeader();
      Mdvx::vlevel_header_t vhdr = field->getVlevelHeader();
      _addFieldHeader32(fhdr, MDVP_FIELD_HEADER_PART_32);
      _addVlevelHeader32(vhdr, MDVP_VLEVEL_HEADER_PART_32,
                         fhdr.nz, fhdr.field_name);
      _addFieldData(*field);
    }
    for (size_t i = 0; i < mdvx.getNChunks(); i++) {
      _addChunkHeader32(mdvx.getChunkByNum(i)->getHeader(),
                        MDVP_CHUNK_HEADER_PART_32);
      _addChunkData(*mdvx.getChunkByNum(i));
    }
  } else {
    // 64-bit
    _addMasterHeader64(mdvx.getMasterHeader(), MDVP_MASTER_HEADER_PART_64);
    for (size_t i = 0; i < mdvx.getNFields(); i++) {
      MdvxField *field = mdvx.getFieldByNum(i);
      field->compressIfRequested();
      Mdvx::field_header_t fhdr = field->getFieldHeader();
      Mdvx::vlevel_header_t vhdr = field->getVlevelHeader();
      _addFieldHeader64(fhdr, MDVP_FIELD_HEADER_PART_64);
      _addVlevelHeader64(vhdr, MDVP_VLEVEL_HEADER_PART_64,
                         fhdr.nz, fhdr.field_name);
      _addFieldData64(*field);
    }
    for (size_t i = 0; i < mdvx.getNChunks(); i++) {
      _addChunkHeader64(mdvx.getChunkByNum(i)->getHeader(),
                        MDVP_CHUNK_HEADER_PART_64);
      _addChunkData(*mdvx.getChunkByNum(i));
    }
  }

}
  
////////////////////////////////////////////////////
// add mdv headers and data, including file headers

void DsMdvxMsg::_addHdrsAndDataExtended(const DsMdvx &mdvx)
{

  if (mdvx._readFormat == Mdvx::FORMAT_XML) {
      
    // add XML parts
    
    _addXmlHeader(mdvx.getXmlHdr());
    _addXmlBuffer(mdvx.getXmlBuf());
    
  } else if (mdvx._readAsSingleBuffer) {
    
    // add single buffer for mdvx object
    
    MemBuf buf;
    if (_use32BitHeaders) {
      mdvx.writeToBuffer32(buf);
    } else {
      mdvx.writeToBuffer64(buf);
    }
    _addSingleBuffer(buf);
    
    // if requested, add headers exactly as in the file
    for (size_t i = 0; i < mdvx.getNFields(); i++) {
      const Mdvx::field_header_t *fhdrFile = mdvx.getFieldByNum(i)->getFieldHeaderFile();
      const Mdvx::vlevel_header_t *vhdrFile = mdvx.getFieldByNum(i)->getVlevelHeaderFile();
      if (fhdrFile != NULL && vhdrFile != NULL) {
        if (_use32BitHeaders) {
          // 32-bit
          _addFieldHeader32(*fhdrFile, MDVP_FIELD_HEADER_FILE_FIELD_PART_32);
          _addVlevelHeader32(*vhdrFile, MDVP_VLEVEL_HEADER_FILE_FIELD_PART_32,
                             fhdrFile->nz, fhdrFile->field_name);
        } else {
          // 64-bit
          _addFieldHeader64(*fhdrFile, MDVP_FIELD_HEADER_FILE_FIELD_PART_64);
          _addVlevelHeader64(*vhdrFile, MDVP_VLEVEL_HEADER_FILE_FIELD_PART_64,
                             fhdrFile->nz, fhdrFile->field_name);
        }
      }
    }
    
  } else {
    
    if (_use32BitHeaders) {

      // 32-bit
      // add master header
      _addMasterHeader32(mdvx.getMasterHeader(), MDVP_MASTER_HEADER_PART_32);
      // add fields
      for (size_t i = 0; i < mdvx.getNFields(); i++) {
        MdvxField *field = mdvx.getFieldByNum(i);
        field->compressIfRequested();
        // field headers
        Mdvx::field_header_t fhdr = field->getFieldHeader();
        Mdvx::vlevel_header_t vhdr = field->getVlevelHeader();
        _addFieldHeader32(fhdr, MDVP_FIELD_HEADER_PART_32);
        _addVlevelHeader32(vhdr, MDVP_VLEVEL_HEADER_PART_32,
                           fhdr.nz, fhdr.field_name);
        // field headers as in file
        const Mdvx::field_header_t *fhdrFile = field->getFieldHeaderFile();
        const Mdvx::vlevel_header_t *vhdrFile = field->getVlevelHeaderFile();
        if (fhdrFile != NULL && vhdrFile != NULL) {
          _addFieldHeader32(*fhdrFile, MDVP_FIELD_HEADER_FILE_FIELD_PART_32);
          _addVlevelHeader32(*vhdrFile, MDVP_VLEVEL_HEADER_FILE_FIELD_PART_32,
                             fhdrFile->nz, fhdrFile->field_name);
        }
        _addFieldData(*field);
      } // i
      // add chunks
      for (size_t i = 0; i < mdvx.getNChunks(); i++) {
        _addChunkHeader32(mdvx.getChunkByNum(i)->getHeader(),
                          MDVP_CHUNK_HEADER_PART_32);
        _addChunkData(*mdvx.getChunkByNum(i));
      }

    } else {

      // 64-bit
      // add master header
      _addMasterHeader64(mdvx.getMasterHeader(), MDVP_MASTER_HEADER_PART_64);
      // fields
      for (size_t i = 0; i < mdvx.getNFields(); i++) {
        MdvxField *field = mdvx.getFieldByNum(i);
        field->compressIfRequested();
        // field headers
        Mdvx::field_header_t fhdr = field->getFieldHeader();
        Mdvx::vlevel_header_t vhdr = field->getVlevelHeader();
        _addFieldHeader64(fhdr, MDVP_FIELD_HEADER_PART_64);
        _addVlevelHeader64(vhdr, MDVP_VLEVEL_HEADER_PART_64,
                           fhdr.nz, fhdr.field_name);
        // field headers as in file
        const Mdvx::field_header_t *fhdrFile = field->getFieldHeaderFile();
        const Mdvx::vlevel_header_t *vhdrFile = field->getVlevelHeaderFile();
        if (fhdrFile != NULL && vhdrFile != NULL) {
          _addFieldHeader64(*fhdrFile, MDVP_FIELD_HEADER_FILE_FIELD_PART_64);
          _addVlevelHeader64(*vhdrFile, MDVP_VLEVEL_HEADER_FILE_FIELD_PART_64,
                             fhdrFile->nz, fhdrFile->field_name);
        }
        _addFieldData64(*field);
      } // i
      // add chunks
      for (size_t i = 0; i < mdvx.getNChunks(); i++) {
        _addChunkHeader64(mdvx.getChunkByNum(i)->getHeader(),
                          MDVP_CHUNK_HEADER_PART_64);
        _addChunkData(*mdvx.getChunkByNum(i));
      }

    } // if (_use32BitHeaders)
    
  } // if (mdvx._readFormat == Mdvx::FORMAT_XML)

}


