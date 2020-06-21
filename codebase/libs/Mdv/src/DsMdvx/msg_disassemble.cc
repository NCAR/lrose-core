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
// msg_disassemble.cc
//
// disassemble methods for DsMdvxMsg object
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

/////////////////////////////////////
// override the disassemble function
// returns 0 on success, -1 on error

int DsMdvxMsg::disassemble(const void *in_msg, const ssize_t msg_len,
			   DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassemble, msg_len: " << msg_len << "  <<-----" << endl;
  }
  
  // initialize
  
  _clearErrStr();
  
  // peek at the header to make sure we're looking at the
  // right type of message

  if (decodeHeader(in_msg, msg_len)) {
    _errStr += "ERROR - DsMdvxMsg::disassemble\n";
    _errStr += "  Bad message header\n";
    TaStr::AddInt(_errStr, "  Message len: ", msg_len);
    return -1;
  }
  
  if (_type != MDVP_REQUEST_MESSAGE && _type != MDVP_REPLY_MESSAGE) {
    _errStr += "ERROR - DsMdvxMsg::disassemble\n";
    TaStr::AddInt(_errStr, "  Unknown message type: ", _type);
    TaStr::AddInt(_errStr, "  Message len: ", msg_len);
    printHeader(cerr, "");
    return -1;
  }

  // disassemble the message using the base-class routine
  
  if (DsMessage::disassemble(in_msg, msg_len)) {
    _errStr += "ERROR - DsMdvxMsg::disassemble\n";
    _errStr += "  Error in DsMessage::disassemble()\n";
    return -1;
  }

  if (_debug) {
    print(cerr, "  ");
  }

  // check we are not mixing 32-bit and 64-bit header parts
  // this also sets _use32BitHeaders

  check64BitHeaders();

  // error message?
  
  mdvx.clearErrStr();

  if (_error) {
    if (partExists(DsServerMsg::DS_ERR_STRING)) {
      mdvx._errStr = (char *)
	getPartByType(DsServerMsg::DS_ERR_STRING)->getBuf();
    }
    _getNoFilesFoundOnRead(mdvx);
    return 0;
  }

  if (_type == MDVP_REQUEST_MESSAGE) {

    // request message
    
    switch (_subType) {
      case MDVP_READ_ALL_HDRS:
        if (_disassembleReadAllHdrs(mdvx)) {
          return -1;
        }
        break;
      case MDVP_READ_VOLUME:
        if (_disassembleReadVolume(mdvx)) {
          return -1;
        }
        break;
      case MDVP_READ_VSECTION:
        if (_disassembleReadVsection(mdvx)) {
          return -1;
        }
        break;
      case MDVP_WRITE_TO_DIR:
        if (_disassembleWrite(mdvx)) {
          return -1;
        }
        break;
      case MDVP_WRITE_TO_PATH:
        if (_disassembleWrite(mdvx)) {
          return -1;
        }
        break;
      case MDVP_COMPILE_TIME_LIST:
        if (_disassembleCompileTimeList(mdvx)) {
          return -1;
        }
        break;
      case MDVP_COMPILE_TIME_HEIGHT:
        if (_disassembleCompileTimeHeight(mdvx)) {
          return -1;
        }
        break;
      case MDVP_CONVERT_MDV_TO_NCF:
        if (_disassembleConvertMdv2Ncf(mdvx)) {
          return -1;
        }
        break;
#ifdef NOTNOW
      case MDVP_CONVERT_NCF_TO_MDV:
        if (_disassembleConvertNcf2Mdv(mdvx)) {
          return -1;
        }
        break;
      case MDVP_READ_NCF:
        if (_disassembleReadNcf(mdvx)) {
          return -1;
        }
        break;
      case MDVP_READ_RADX:
        if (_disassembleReadRadx(mdvx)) {
          return -1;
        }
        break;
      case MDVP_READ_ALL_HDRS_NCF:
        if (_disassembleReadAllHdrsNcf(mdvx)) {
          return -1;
        }
        break;
      case MDVP_READ_ALL_HDRS_RADX:
        if (_disassembleReadAllHdrsRadx(mdvx)) {
          return -1;
        }
        break;
#endif
    } // switch
    
  } else {
    
    // return message
    
    switch (_subType) {
      case MDVP_READ_ALL_HDRS:
        if (_disassembleReadAllHdrsReturn(mdvx)) {
          return -1;
        }
        break;
      case MDVP_READ_VOLUME:
        if (_disassembleReadVolumeReturn(mdvx)) {
          return -1;
        }
        break;
      case MDVP_READ_VSECTION:
        if (_disassembleReadVsectionReturn(mdvx)) {
          return -1;
        }
        break;
      case MDVP_WRITE_TO_DIR:
      case MDVP_WRITE_TO_PATH:
        if (_disassembleWriteReturn(mdvx)) {
          return -1;
        }
        break;
      case MDVP_COMPILE_TIME_LIST:
        if (_disassembleCompileTimeListReturn(mdvx)) {
          return -1;
        }
        break;
      case MDVP_COMPILE_TIME_HEIGHT:
        if (_disassembleCompileTimeHeightReturn(mdvx)) {
          return -1;
        }
        break;
      case MDVP_CONVERT_MDV_TO_NCF:
        if (_disassembleConvertMdv2NcfReturn(mdvx)) {
          return -1;
        }
        break;
#ifdef NOTNOW
      case MDVP_CONVERT_NCF_TO_MDV:
        if (_disassembleConvertNcf2MdvReturn(mdvx)) {
          return -1;
        }
        break;
      case MDVP_READ_NCF:
        if (_disassembleReadNcfReturn(mdvx)) {
          return -1;
        }
        break;
      case MDVP_READ_RADX:
        if (_disassembleReadRadxReturn(mdvx)) {
          return -1;
        }
        break;
      case MDVP_READ_ALL_HDRS_NCF:
        if (_disassembleReadAllHdrsNcfReturn(mdvx)) {
          return -1;
        }
        break;
      case MDVP_READ_ALL_HDRS_RADX:
        if (_disassembleReadAllHdrsRadxReturn(mdvx)) {
          return -1;
        }
        break;
#endif
    } // switch
    
  }

  return 0;

}

/////////////////////////////////////////////////////
// disassemble readAllHeaders message,
// load into Mdvx object
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleReadAllHdrs(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleReadAllHdrs <<-----" << endl;
  }

  mdvx.clearRead();
  if (_use32BitHeaders) {
    mdvx.setRead32BitHeaders(true);
  }

  // get format
  
  _getReadFormat(mdvx);

  if (_getReadSearch(mdvx)) {
    return -1;
  }

  _getReadTimeListAlso(mdvx);
  if (mdvx._readTimeListAlso) {
    mdvx.clearTimeListMode();
    if (_getTimeListOptions(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadAllHdrs.\n";
      return -1;
    }
  }

  _getReadLatestValidModTime(mdvx);

  return 0;

}

/////////////////////////////////////////////////////
// disassemble readAllHeadersReturn message,
// load into Mdvx object
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleReadAllHdrsReturn(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleReadAllHdrsReturn <<-----" << endl;
  }

  // get formats
  
  _getReadFormat(mdvx);
  _getInternalFormat(mdvx);

  if (_getHeaders(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadAllHdrsReturn\n";
    return -1;
  }
  
  if (_getPathInUse(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadAllHdrsReturn\n";
    return -1;
  }

  // get time list data if available

  _getTimeLists(mdvx);
  
  return 0;

}

//////////////////////////////////////////////////////////////
// disassemble readVolume message,
// load into Mdvx object
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleReadVolume(DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> disassembleReadVolume <<-----" << endl;
  }

  mdvx.clearRead();
  if (_use32BitHeaders) {
    mdvx.setRead32BitHeaders(true);
  }

  // get format
  
  _getReadFormat(mdvx);
  
  if (_getReadSearch(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadVolume.\n";
    return -1;
  }

  if (_getReadQualifiers(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadVolume.\n";
    return -1;
  }

  if (_getClimoQualifiers(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadVolume.\n";
    return -1;
  }

  _getReadTimeListAlso(mdvx);
  if (mdvx._readTimeListAlso) {
    mdvx.clearTimeListMode();
    if (_getTimeListOptions(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadVolume.\n";
      return -1;
    }
  }

  _getReadLatestValidModTime(mdvx);

  return 0;

}

/////////////////////////////////////////////////////
// disassemble readVolumeReturn message,
// load into Mdvx object
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleReadVolumeReturn(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleReadVolumeReturn <<-----" << endl;
  }

  // get formats
  
  _getReadFormat(mdvx);
  _getInternalFormat(mdvx);

  if (partExists(MDVP_XML_HEADER_PART)) {

    // mdvx object is XML

    if (_getXmlHdrAndBuf(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadVolumeReturn\n";
      return -1;
    }

  } else if (partExists(MDVP_SINGLE_BUFFER_PART)) {

    // mdvx object is in single buffer

    if (_getSingleBuffer(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadVolumeReturn\n";
      return -1;
    }

  } else if (mdvx._internalFormat == Mdvx::FORMAT_NCF) {
    
    if (_getNcfParts(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadVolumeReturn\n";
      return -1;
    }
    
  } else {
    
    // mdvx object is in separate parts
    
    if (_getHeadersAndData(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadVolumeReturn\n";
      return -1;
    }
    
  }

  if (_getPathInUse(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadVolumeReturn\n";
    return -1;
  }

  // get time list data if available

  _getTimeLists(mdvx);
  
  return 0;

}

//////////////////////////////////////////////////////////////
// disassemble readVsection message,
// load into Mdvx object
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleReadVsection(DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> disassembleReadVsection <<-----" << endl;
  }

  mdvx.clearRead();
  if (_use32BitHeaders) {
    mdvx.setRead32BitHeaders(true);
  }

  // get format
  
  _getReadFormat(mdvx);

  if (_getReadSearch(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadVsection.\n";
    return -1;
  }

  if (_getReadQualifiers(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadVsection.\n";
    return -1;
  }

  if (_getReadVsectQualifiers(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadVsection.\n";
    return -1;
  }
  
  _getReadTimeListAlso(mdvx);
  if (mdvx._readTimeListAlso) {
    mdvx.clearTimeListMode();
    if (_getTimeListOptions(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadVsection.\n";
      return -1;
    }
  }

  _getReadLatestValidModTime(mdvx);

  return 0;

}

/////////////////////////////////////////////////////
// disassemble readVsectionReturn message,
// load into Mdvx object
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleReadVsectionReturn(DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> disassembleReadVsectionReturn <<-----" << endl;
  }

  // get formats
  
  _getReadFormat(mdvx);
  _getInternalFormat(mdvx);

  if (partExists(MDVP_XML_HEADER_PART)) {
    
    // mdvx object is XML
    
    if (_getXmlHdrAndBuf(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadVolumeReturn\n";
      return -1;
    }

  } else if (partExists(MDVP_SINGLE_BUFFER_PART)) {

    // mdvx object is in single part

    if (_getSingleBuffer(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadVsectionReturn\n";
      return -1;
    }

  } else if (mdvx._internalFormat == Mdvx::FORMAT_NCF) {
    
    if (_getNcfParts(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadVsectionReturn\n";
      return -1;
    }
    
  } else {
  
    // mdvx object is in multiple parts

    if (_getHeadersAndData(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadVolumeReturn\n";
      return -1;
    }
    
  }

  if (_getReturnVsectInfo(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadVsectionReturn.\n";
    return -1;
  }

  if (_getPathInUse(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadVsectionReturn\n";
    return -1;
  }

  // get time list data if available

  _getTimeLists(mdvx);
  
  return 0;

}

/////////////////////////////////////////////////////
// disassemble write message,
// load into Mdvx object
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleWrite(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleWrite <<-----" << endl;
  }

  // get formats
  
  _getWriteFormat(mdvx);
  _getInternalFormat(mdvx);

  if (_getWriteUrl(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleWrite\n";
    return -1;
  }

  if (_getWriteOptions(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleWrite\n";
    return -1;
  }
  
  if (partExists(MDVP_SINGLE_BUFFER_PART)) {

    // mdvx object is in single buffer

    if (_getSingleBuffer(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleWrite\n";
      return -1;
    }

  } else if (mdvx._internalFormat == Mdvx::FORMAT_NCF) {
    
    if (_getNcfParts(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleWrite\n";
      return -1;
    }
    
  } else {

    // mdvx object is in multiple parts

    if (_getHeadersAndData(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleWrite\n";
      return -1;
    }
    
  }
  
  if (mdvx._internalFormat == Mdvx::FORMAT_NCF ||
      mdvx._writeFormat == Mdvx::FORMAT_NCF) {
    if (_getConvertMdv2Ncf(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleWrite\n";
      return -1;
    }
  }
  
  if (partExists(MDVP_APP_NAME_PART)) {
    if (_getAppName(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleWrite\n";
      return -1;
    }
  }

  return 0;

}

/////////////////////////////////////////////////////
// disassemble writeReturn message,
// load into Mdvx object
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleWriteReturn(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleWriteReturn <<-----" << endl;
  }

  // get format as written
  
  _getWriteFormat(mdvx);

  if (_getPathInUse(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleWriteReturn\n";
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////////////////
// disassemble CompileTimeList message,
// load into Mdvx object
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleCompileTimeList(DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> disassembleCompileTimeList <<-----" << endl;
  }

  mdvx.clearTimeListMode();

  // get format
  
  _getReadFormat(mdvx);

  if (_getTimeListOptions(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleCompileTimeList.\n";
    return -1;
  }

  _getReadLatestValidModTime(mdvx);

  // optional part to constrain forecast lead times
  
  if (_getConstrainLeadTimes(mdvx)) {
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////
// disassemble CompileTimeListReturn message,
// load into Mdvx object
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleCompileTimeListReturn(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleCompileTimeListReturn <<-----" << endl;
  }

  // get formats
  
  _getReadFormat(mdvx);
  _getInternalFormat(mdvx);

  if (_getTimeLists(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleCompileTimeListReturn\n";
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////////////////
// disassemble compileTimeHeight message,
// load into Mdvx object
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleCompileTimeHeight(DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> disassembleCompileTimeHeight <<-----" << endl;
  }
  
  mdvx.clearRead();
  if (_use32BitHeaders) {
    mdvx.setRead32BitHeaders(true);
  }
  mdvx.clearTimeListMode();

  // get format
  
  _getReadFormat(mdvx);

  if (_getReadQualifiers(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleCompileTimeHeight.\n";
    return -1;
  }

  if (_getReadVsectWaypts(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleCompileTimeHeight.\n";
    return -1;
  }

  if (_getReadVsectInterpDisabled(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleCompileTimeHeight.\n";
    return -1;
  }

  if (_getTimeListOptions(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleCompileTimeHeight.\n";
    return -1;
  }

  _getReadLatestValidModTime(mdvx);

  // optional part to constrain forecast lead times
  
  if (_getConstrainLeadTimes(mdvx)) {
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////
// disassemble compileTimeHeightReturn message,
// load into Mdvx object
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleCompileTimeHeightReturn(DsMdvx &mdvx)
  
{
  
  if (_debug) {
    cerr << "--->> disassembleCompileTimeHeightReturn <<-----" << endl;
  }
  
  // get formats
  
  _getReadFormat(mdvx);
  _getInternalFormat(mdvx);

  if (partExists(MDVP_XML_HEADER_PART)) {
    
    // mdvx object is XML
    
    if (_getXmlHdrAndBuf(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadVolumeReturn\n";
      return -1;
    }

  } else if (partExists(MDVP_SINGLE_BUFFER_PART)) {
    
    // mdvx object is in single part
    
    if (_getSingleBuffer(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleCompileTimeHeightReturn\n";
      return -1;
    }
    
  } else {
    
    // mdvx object is in multiple parts

    if (_use32BitHeaders) {
      // 32-bit
      if (_getMasterHeader(mdvx._mhdr, MDVP_MASTER_HEADER_PART_32)) {
        _errStr += "ERROR - DsMdvxMsg::_disassembleCompileTimeHeightReturn\n";
        _errStr += "  Cannot find 32-bit master header part\n";
        return -1;
      }
    } else {
      // 64-bit
      if (_getMasterHeader(mdvx._mhdr, MDVP_MASTER_HEADER_PART_64)) {
        _errStr += "ERROR - DsMdvxMsg::_disassembleCompileTimeHeightReturn\n";
        _errStr += "  Cannot find 64-bit master header part\n";
        return -1;
      }
    }

    int n_fields = mdvx._mhdr.n_fields;
    mdvx.clearFields();
    for (int i = 0; i < n_fields; i++) {
      if (_getField(mdvx, i)) {
	_errStr += "ERROR - DsMdvxMsg::_disassembleCompileTimeHeightReturn\n";
	return -1;
      }
    }
    
  }

  if (_getReadVsectWaypts(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleCompileTimeHeightReturn.\n";
    return -1;
  }

  // get time list data if available

  _getTimeLists(mdvx);
  
  return 0;

}

/////////////////////////////////////////////////////
// disassemble convert MDV to NCF message,
// load into Mdvx object
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleConvertMdv2Ncf(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleConvertMdv2Ncf <<-----" << endl;
  }
  
  // get format
  
  _getInternalFormat(mdvx);

  if (_getHeadersAndData(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleConvertMdv2Ncf\n";
    return -1;
  }
  
  if (partExists(MDVP_APP_NAME_PART)) {
    if (_getAppName(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleConvertMdv2Ncf\n";
      return -1;
    }
  }

  if (_getConvertMdv2Ncf(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleConvertMdv2Ncf\n";
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////////////
// disassemble convert MDV to NCF return message,
// load into Mdvx object
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleConvertMdv2NcfReturn(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleConvertMdv2NcfReturn <<-----" << endl;
  }
  
  // get format
  
  _getInternalFormat(mdvx);

  if (_getNcfParts(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleConvertMdv2NcfReturn\n";
    return -1;
  }
  
  return 0;

}

#ifdef NOTNOW

/////////////////////////////////////////////////////
// disassemble convert NCF to MDV message,
// load into Mdvx object
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleConvertNcf2Mdv(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleConvertNcf2Mdv <<-----" << endl;
  }
  
  // get format
  
  _getInternalFormat(mdvx);

  if (_getReadQualifiers(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleConvertNcf2Mdv.\n";
    return -1;
  }

  if (_getReadVsectQualifiers(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleConvertNcf2Mdv.\n";
    return -1;
  }
  
  if (_getNcfParts(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleConvertNcf2Mdv\n";
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////////////
// disassemble convert NCF to MDV return message,
// load into Mdvx object
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleConvertNcf2MdvReturn(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleConvertNcf2MdvReturn <<-----" << endl;
  }
  
  // get format
  
  _getInternalFormat(mdvx);

  if (partExists(MDVP_SINGLE_BUFFER_PART)) {
    if (_getSingleBuffer(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleConvertNcf2MdvReturn\n";
      return -1;
    }
  } else {
    if (_getHeadersAndData(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleConvertNcf2MdvReturn\n";
      return -1;
    }
  }
    
  if (_getReturnVsectInfo(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleConvertNcf2MdvReturn.\n";
    return -1;
  }

  return 0;

}


/////////////////////////////////////////////////////
// disassemble read NCF headers message,
// load into Mdvx object
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleReadAllHdrsNcf(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleReadAllHdrsNcf <<-----" << endl;
  }
  
  // get format
  
  _getInternalFormat(mdvx);
  
  if (_getReadQualifiers(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadAllHdrsNcf.\n";
    return -1;
  }

  if (_getPathInUse(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadNcf\n";
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////
// disassemble read NCF headers return message,
// load into Mdvx object
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleReadAllHdrsNcfReturn(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleReadAllHdrsNcfReturn <<-----" << endl;
  }
  
  // get format
  
  _getInternalFormat(mdvx);
  
  if (mdvx._internalFormat == Mdvx::FORMAT_NCF) {
    
    if (_getNcfHeaderParts(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadAllHdrsNcfReturn\n";
      return -1;
    }
    
  } else {

    if (_getHeaders(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadAllHdrsNcfReturn\n";
      return -1;
    }
    
  }
  
  if (_getPathInUse(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadAllHdrsNcfReturn\n";
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////////////
// disassemble read NCF file message,
// load into Mdvx object
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleReadNcf(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleReadNcf <<-----" << endl;
  }
  
  // get format
  
  _getInternalFormat(mdvx);
  
  if (_getReadQualifiers(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadNcf.\n";
    return -1;
  }

  if (_getReadVsectQualifiers(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadNcf.\n";
    return -1;
  }
  
  if (_getPathInUse(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadNcf\n";
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////
// disassemble read NCF file return message,
// load into Mdvx object
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleReadNcfReturn(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleReadNcfReturn <<-----" << endl;
  }
  
  // get format
  
  _getInternalFormat(mdvx);
  
  if (mdvx._internalFormat == Mdvx::FORMAT_NCF) {
    
    if (_getNcfParts(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadNcfReturn\n";
      return -1;
    }
    
  } else {

    if (partExists(MDVP_SINGLE_BUFFER_PART)) {
      if (_getSingleBuffer(mdvx)) {
	_errStr += "ERROR - DsMdvxMsg::_disassembleReadNcfReturn\n";
	return -1;
      }
    } else {
      if (_getHeadersAndData(mdvx)) {
	_errStr += "ERROR - DsMdvxMsg::_disassembleReadNcfReturn\n";
	return -1;
      }
    }
    
    if (_getReturnVsectInfo(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadNcfReturn.\n";
      return -1;
    }

  }
  
  if (_getPathInUse(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadNcfReturn\n";
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////////////
// disassemble read RADX headers message,
// load into Mdvx object
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleReadAllHdrsRadx(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleReadAllHdrsRadx <<-----" << endl;
  }
  
  // get format
  
  _getInternalFormat(mdvx);
  
  if (_getReadQualifiers(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadAllHdrsRadx.\n";
    return -1;
  }

  if (_getPathInUse(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadRadx\n";
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////
// disassemble read RADX headers return message,
// load into Mdvx object
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleReadAllHdrsRadxReturn(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleReadAllHdrsRadxReturn <<-----" << endl;
  }
  
  // get format
  
  _getInternalFormat(mdvx);
  
  if (mdvx._internalFormat == Mdvx::FORMAT_NCF) {
    
    if (_getNcfHeaderParts(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadAllHdrsRadxReturn\n";
      return -1;
    }
    
  } else {

    if (_getHeaders(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadAllHdrsRadxReturn\n";
      return -1;
    }
    
  }
  
  if (_getPathInUse(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadAllHdrsRadxReturn\n";
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////////////
// disassemble read RADX file message,
// load into Mdvx object
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleReadRadx(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleReadRadx <<-----" << endl;
  }
  
  // get format
  
  _getInternalFormat(mdvx);

  if (_getReadQualifiers(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadRadx.\n";
    return -1;
  }

  if (_getReadVsectQualifiers(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadRadx.\n";
    return -1;
  }
  
  if (_getPathInUse(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleReadRadx\n";
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////
// disassemble read RADX file return message,
// load into Mdvx object
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_disassembleReadRadxReturn(DsMdvx &mdvx)
  
{

  if (_debug) {
    cerr << "--->> disassembleReadRadxReturn <<-----" << endl;
  }
  
  // get format
  
  _getInternalFormat(mdvx);

  if (mdvx._internalFormat == Mdvx::FORMAT_NCF) {
    
    if (_getNcfParts(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadRadxReturn\n";
      return -1;
    }
    
  } else {
    
    if (partExists(MDVP_SINGLE_BUFFER_PART)) {
      if (_getSingleBuffer(mdvx)) {
	_errStr += "ERROR - DsMdvxMsg::_disassembleReadRadxReturn\n";
	return -1;
      }
    } else {
      if (_getHeadersAndData(mdvx)) {
	_errStr += "ERROR - DsMdvxMsg::_disassembleReadRadxReturn\n";
	return -1;
      }
    }
    
    if (_getReturnVsectInfo(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_disassembleReadRadxReturn.\n";
      return -1;
    }

  }
  
  if (_getPathInUse(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_disassembleWriteReturn\n";
    return -1;
  }
  
  return 0;

}

#endif
