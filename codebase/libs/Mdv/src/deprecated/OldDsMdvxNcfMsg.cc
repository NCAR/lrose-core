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
// OldDsMdvxNcfMsg.cc
//
// OldDsMdvxNcfMsg object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1999
//
///////////////////////////////////////////////////////////////
//
// The OldDsMdvxNcfMsg object provides the message protocol for
// the DsMdvx service.
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <Mdv/OldDsMdvxNcfMsg.hh>
#include <didss/DsMsgPart.hh>
#include <toolsa/mem.h>
#include <toolsa/TaStr.hh>
using namespace std;

//////////////////////////////////////////
// constructor

OldDsMdvxNcfMsg::OldDsMdvxNcfMsg(memModel_t mem_model /* = CopyMem */) :
  DsMdvxMsg(mem_model)
{
}

//////////////////////////////////////////
// destructor

OldDsMdvxNcfMsg::~OldDsMdvxNcfMsg()
{

}

/////////////////////////////////////////////
// assemble message to convert mdv to ncf
// Returns assembled message, NULL on failure

void *OldDsMdvxNcfMsg::assembleConvertMdv2Ncf(const DsMdvx &mdvx,
                                        const string &trans_url)

{

  if (_debug) {
    cerr << "--->> assembleConvertMdv2Ncf <<-----" << endl;
  }
  
  clearAll();
  _clearErrStr();
  
  if (mdvx._currentFormat != Mdvx::FORMAT_MDV) {
    TaStr::AddStr(_errStr, "ERROR - OldDsMdvxNcfMsg::assembleConvertMdv2Ncf");
    TaStr::AddStr(_errStr, "  Format must be MDV");
    TaStr::AddStr(_errStr, "  Current format is: ",
                  Mdvx::format2Str(mdvx._currentFormat));
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
  
  _addCurrentFormat(mdvx._currentFormat);
  
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

void *OldDsMdvxNcfMsg::assembleConvertMdv2NcfReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleConvertMdv2NcfReturn <<-----" << endl;
  }

  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(MDVP_CONVERT_MDV_TO_NCF);

  // indicate that this is the end of the series of messages
  
  setCategory(EndSeries);

  // add formats part

  _addCurrentFormat(mdvx._currentFormat);

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

void *OldDsMdvxNcfMsg::assembleConvertNcf2Mdv(const DsMdvx &mdvx,
                                        const string &trans_url)

{

  if (_debug) {
    cerr << "--->> assembleConvertNcf2Mdv <<-----" << endl;
  }
  
  clearAll();
  _clearErrStr();
  
  if (mdvx._currentFormat != Mdvx::FORMAT_NCF) {
    TaStr::AddStr(_errStr, "ERROR - OldDsMdvxNcfMsg::assembleConvertNcf2Mdv");
    TaStr::AddStr(_errStr, "  Format must be NCF");
    TaStr::AddStr(_errStr, "  Current format is: ",
                  Mdvx::format2Str(mdvx._currentFormat));
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

  _addCurrentFormat(mdvx._currentFormat);

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

void *OldDsMdvxNcfMsg::assembleConvertNcf2MdvReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleConvertNcf2MdvReturn <<-----" << endl;
  }

  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(MDVP_CONVERT_NCF_TO_MDV);

  // indicate that this is the end of the series of messages
  
  setCategory(EndSeries);
  
  // add formats part
  
  _addCurrentFormat(mdvx._currentFormat);
  
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

void *OldDsMdvxNcfMsg::assembleReadAllHdrsNcf(const DsMdvx &mdvx,
					   const string &trans_url)

{

  if (_debug) {
    cerr << "--->> assembleReadAllHdrsNcf <<-----" << endl;
  }
  
  clearAll();
  _clearErrStr();
  
  if (mdvx._currentFormat != Mdvx::FORMAT_NCF) {
    TaStr::AddStr(_errStr, "ERROR - OldDsMdvxNcfMsg::assembleReadAllHdrsNcf");
    TaStr::AddStr(_errStr, "  Format must be NCF");
    TaStr::AddStr(_errStr, "  Current format is: ",
                  Mdvx::format2Str(mdvx._currentFormat));
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
  
  _addCurrentFormat(mdvx._currentFormat);
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

void *OldDsMdvxNcfMsg::assembleReadAllHdrsNcfReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleReadAllHdrsNcfReturn <<-----" << endl;
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
  
  _addCurrentFormat(mdvx._currentFormat);
  _addReadFormat(mdvx._readFormat);

  if (mdvx._currentFormat == Mdvx::FORMAT_NCF) {
    
    // add the ncf headers
    
    _addNcfHdr(mdvx);

  } else {
    
    // add header parts

    if (_use32BitHeaders) {
      // 32-bit
      _addMasterHeader32(mdvx._mhdrFile, MDVP_MASTER_HEADER_FILE_PART_32);
      for (size_t i = 0; i < mdvx._fhdrsFile.size(); i++) {
        _addFieldHeader32(mdvx._fhdrsFile[i],
                          MDVP_FIELD_HEADER_FILE_PART_32);
      }
      for (size_t i = 0; i < mdvx._vhdrsFile.size(); i++) {
        _addVlevelHeader32(mdvx._vhdrsFile[i],
                           MDVP_VLEVEL_HEADER_FILE_PART_32);
      }
      for (size_t i = 0; i < mdvx._chdrsFile.size(); i++) {
        _addChunkHeader32(mdvx._chdrsFile[i],
                          MDVP_CHUNK_HEADER_FILE_PART_32);
      }
    } else {
      // 64-bit
      _addMasterHeader64(mdvx._mhdrFile, MDVP_MASTER_HEADER_FILE_PART_64);
      for (size_t i = 0; i < mdvx._fhdrsFile.size(); i++) {
        _addFieldHeader64(mdvx._fhdrsFile[i],
                          MDVP_FIELD_HEADER_FILE_PART_64);
      }
      for (size_t i = 0; i < mdvx._vhdrsFile.size(); i++) {
        _addVlevelHeader64(mdvx._vhdrsFile[i],
                           MDVP_VLEVEL_HEADER_FILE_PART_64);
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

void *OldDsMdvxNcfMsg::assembleReadNcf(const DsMdvx &mdvx,
                                 const string &trans_url)

{

  if (_debug) {
    cerr << "--->> assembleReadNcf <<-----" << endl;
  }
  
  clearAll();
  _clearErrStr();
  
  if (mdvx._currentFormat != Mdvx::FORMAT_NCF) {
    TaStr::AddStr(_errStr, "ERROR - OldDsMdvxNcfMsg::assembleReadNcf");
    TaStr::AddStr(_errStr, "  Format must be NCF");
    TaStr::AddStr(_errStr, "  Current format is: ",
                  Mdvx::format2Str(mdvx._currentFormat));
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
  
  _addCurrentFormat(mdvx._currentFormat);
  
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

void *OldDsMdvxNcfMsg::assembleReadNcfReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleReadNcfReturn <<-----" << endl;
  }

  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(MDVP_READ_NCF);

  // indicate that this is the end of the series of messages
  
  setCategory(EndSeries);
  
  // add formats part
  
  _addCurrentFormat(mdvx._currentFormat);

  if (mdvx._currentFormat == Mdvx::FORMAT_NCF) {
    
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

void *OldDsMdvxNcfMsg::assembleReadAllHdrsRadx(const DsMdvx &mdvx,
                                         const string &trans_url)

{

  if (_debug) {
    cerr << "--->> assembleReadAllHdrsRadx <<-----" << endl;
  }
  
  clearAll();
  _clearErrStr();
  
  if (mdvx._currentFormat != Mdvx::FORMAT_RADX) {
    TaStr::AddStr(_errStr, "ERROR - OldDsMdvxNcfMsg::assembleReadAllHdrsRadx");
    TaStr::AddStr(_errStr, "  Format must be RADX");
    TaStr::AddStr(_errStr, "  Current format is: ",
                  Mdvx::format2Str(mdvx._currentFormat));
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
  
  _addCurrentFormat(mdvx._currentFormat);
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

void *OldDsMdvxNcfMsg::assembleReadAllHdrsRadxReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleReadAllHdrsRadxReturn <<-----" << endl;
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
  
  _addCurrentFormat(mdvx._currentFormat);
  _addReadFormat(mdvx._readFormat);
  
  if (mdvx._currentFormat == Mdvx::FORMAT_NCF) {
    
    // add the ncf headers
    
    _addNcfHdr(mdvx);

  } else {
    
    // add header parts

    if (_use32BitHeaders) {
      // 32-bit
      _addMasterHeader32(mdvx._mhdrFile, MDVP_MASTER_HEADER_FILE_PART_32);
      for (size_t i = 0; i < mdvx._fhdrsFile.size(); i++) {
        _addFieldHeader32(mdvx._fhdrsFile[i],
                          MDVP_FIELD_HEADER_FILE_PART_32);
      }
      for (size_t i = 0; i < mdvx._vhdrsFile.size(); i++) {
        _addVlevelHeader32(mdvx._vhdrsFile[i],
                           MDVP_VLEVEL_HEADER_FILE_PART_32);
      }
      for (size_t i = 0; i < mdvx._chdrsFile.size(); i++) {
        _addChunkHeader32(mdvx._chdrsFile[i],
                          MDVP_CHUNK_HEADER_FILE_PART_32);
      }
    } else {
      // 64-bit
      _addMasterHeader64(mdvx._mhdrFile, MDVP_MASTER_HEADER_FILE_PART_64);
      for (size_t i = 0; i < mdvx._fhdrsFile.size(); i++) {
        _addFieldHeader64(mdvx._fhdrsFile[i],
                          MDVP_FIELD_HEADER_FILE_PART_64);
      }
      for (size_t i = 0; i < mdvx._vhdrsFile.size(); i++) {
        _addVlevelHeader64(mdvx._vhdrsFile[i],
                           MDVP_VLEVEL_HEADER_FILE_PART_64);
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
    cerr << "--->> assembleReadAllHdrsRadxReturn <<-----" << endl;
    print(cerr, "  ");
  }

  return msg;

}

////////////////////////////////////////////////
// assemble message to read a RADX file
//
// Returns assembled message, NULL on failure

void *OldDsMdvxNcfMsg::assembleReadRadx(const DsMdvx &mdvx,
                                  const string &trans_url)

{

  if (_debug) {
    cerr << "--->> assembleReadRadx <<-----" << endl;
  }
  
  clearAll();
  _clearErrStr();
  
  if (mdvx._currentFormat != Mdvx::FORMAT_RADX) {
    TaStr::AddStr(_errStr, "ERROR - OldDsMdvxNcfMsg::assembleReadRadx");
    TaStr::AddStr(_errStr, "  Format must be RADX");
    TaStr::AddStr(_errStr, "  Current format is: ",
                  Mdvx::format2Str(mdvx._currentFormat));
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
  
  _addCurrentFormat(mdvx._currentFormat);

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

void *OldDsMdvxNcfMsg::assembleReadRadxReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleReadRadxReturn <<-----" << endl;
  }

  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(MDVP_READ_RADX);

  // indicate that this is the end of the series of messages
  
  setCategory(EndSeries);
  
  // add formats part
  
  _addCurrentFormat(mdvx._currentFormat);

  if (mdvx._currentFormat == Mdvx::FORMAT_NCF) {
    
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

void *OldDsMdvxNcfMsg::assembleConstrainNcf(const DsMdvx &mdvx,
                                      const string &trans_url)

{

  if (_debug) {
    cerr << "--->> assembleConstrainNcf <<-----" << endl;
  }
  
  clearAll();
  _clearErrStr();
  
  if (mdvx._currentFormat != Mdvx::FORMAT_NCF) {
    TaStr::AddStr(_errStr, "ERROR - OldDsMdvxNcfMsg::assembleConstrainNcf");
    TaStr::AddStr(_errStr, "  Format must be NCF");
    TaStr::AddStr(_errStr, "  Current format is: ",
                  Mdvx::format2Str(mdvx._currentFormat));
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

  _addCurrentFormat(mdvx._currentFormat);
  
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

void *OldDsMdvxNcfMsg::assembleConstrainNcfReturn(const DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> assembleConstrainNcfReturn <<-----" << endl;
  }

  clearAll();
  _clearErrStr();
  
  // set header attributes
  
  setType(MDVP_REPLY_MESSAGE);
  setSubType(MDVP_CONSTRAIN_NCF);

  // indicate that this is the end of the series of messages
  
  setCategory(EndSeries);

  // add formats part

  _addCurrentFormat(mdvx._currentFormat);
  
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

