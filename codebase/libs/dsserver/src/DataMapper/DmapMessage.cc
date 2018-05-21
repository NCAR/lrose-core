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

/*******************************************************************
 * dsserver/DmapMessage.cc
 *
 * DsMessage class for DataMapper
 ******************************************************************/

#include <dataport/bigend.h>
#include <dsserver/DmapMessage.hh>
#include <didss/DsMsgPart.hh>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <ctime>
using namespace std;

/////////////////
// constructor

DmapMessage::DmapMessage() : DsServerMsg()
{
}

/////////////////
// destructor

DmapMessage::~DmapMessage()
{
}

///////////////////////////////////////////////
// assemble a DMAP_REG_LATEST_DATA_INFO message
//

void *DmapMessage::assembleRegLatestInfo(const time_t latest_time,
					 const char *hostname,
					 const char *ipaddr,
					 const char *datatype,
					 const char *dir,
					 const int forecast_lead_time)
  
{

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_DMAP,
	     DMAP_REG_LATEST_DATA_INFO);
  
  // clear message parts

  clearParts();

  // fill out info struct

  DMAP_info_t info;
  memset(&info, 0, sizeof(DMAP_info_t));
  info.latest_time = latest_time;
  info.last_reg_time = time(NULL);
  info.forecast_lead_time = forecast_lead_time;
  STRncopy(info.hostname, hostname, DMAP_HOSTNAME_LEN);
  STRncopy(info.ipaddr, ipaddr, DMAP_IPADDR_LEN);
  STRncopy(info.datatype, datatype, DMAP_DATATYPE_LEN);
  STRncopy(info.dir, dir, DMAP_DIR_LEN);

  // swap to BE

  BE_from_dmap_info(info);

  // add part

  addPart(DMAP_INFO_PART, sizeof(DMAP_info_t), &info);

  // assemble

  return (DsMessage::assemble());

}


//////////////////////////////////////////
// assemble a DMAP_REG_STATUS_INFO message
//

void *DmapMessage::assembleRegStatusInfo(const char *status,
					 const char *hostname,
					 const char *ipaddr,
					 const char *datatype,
					 const char *dir)
  
{

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_DMAP,
	     DMAP_REG_STATUS_INFO);
  
  // clear message parts

  clearParts();

  // fill out info struct

  DMAP_info_t info;
  memset(&info, 0, sizeof(DMAP_info_t));
  info.last_reg_time = time(NULL);
  STRncopy(info.status, status, DMAP_STATUS_LEN);
  STRncopy(info.hostname, hostname, DMAP_HOSTNAME_LEN);
  STRncopy(info.ipaddr, ipaddr, DMAP_IPADDR_LEN);
  STRncopy(info.datatype, datatype, DMAP_DATATYPE_LEN);
  STRncopy(info.dir, dir, DMAP_DIR_LEN);

  // swap to BE

  BE_from_dmap_info(info);

  // add part

  addPart(DMAP_INFO_PART, sizeof(DMAP_info_t), &info);

  // assemble

  return (DsMessage::assemble());

}


///////////////////////////////////////////////
// assemble a DMAP_REG_DATA_SET_INFO message
//

void *DmapMessage::assembleRegDataSetInfo(const time_t start_time,
					  const time_t end_time,
					  const double nfiles,
					  const double total_bytes,
					  const char *hostname,
					  const char *ipaddr,
					  const char *datatype,
					  const char *dir)
  
{

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_DMAP,
	     DMAP_REG_DATA_SET_INFO);
  
  // clear message parts

  clearParts();

  // fill out info struct

  DMAP_info_t info;
  memset(&info, 0, sizeof(DMAP_info_t));
  info.start_time = start_time;
  info.end_time = end_time;
  info.last_reg_time = time(NULL);
  info.nfiles = nfiles;
  info.total_bytes = total_bytes;
  STRncopy(info.hostname, hostname, DMAP_HOSTNAME_LEN);
  STRncopy(info.ipaddr, ipaddr, DMAP_IPADDR_LEN);
  STRncopy(info.datatype, datatype, DMAP_DATATYPE_LEN);
  STRncopy(info.dir, dir, DMAP_DIR_LEN);

  // swap to BE

  BE_from_dmap_info(info);

  // add part

  addPart(DMAP_INFO_PART, sizeof(DMAP_info_t), &info);

  // assemble

  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble a DMAP_REG_FULL_INFO messahe
//

void *DmapMessage::assembleRegFullInfo(const vector<DMAP_info_t> &infoArray)
  
{
  
  // set header attributes - flags has error status
  
  setHdrAttr(DS_MESSAGE_TYPE_DMAP,
	     DMAP_REG_FULL_INFO);
  
  // clear message parts
  
  clearParts();

  // add info parts

  for (size_t ii = 0; ii < infoArray.size(); ii++) {
    DMAP_info_t localInfo;
    localInfo = infoArray[ii];
    BE_from_dmap_info(localInfo);
    addPart(DMAP_INFO_PART, sizeof(DMAP_info_t), &localInfo);
  }
    
  // assemble

  return (DsMessage::assemble());

}

void *DmapMessage::assembleRegFullInfo(const DMAP_info_t &info)
  
{

  vector<DMAP_info_t> infoArray;
  infoArray.push_back(info);
  return assembleRegFullInfo(infoArray);

}
  
///////////////////////////////////////////////
// assemble a DMAP_REG_DATA_SET_INFO message
//

void *DmapMessage::assembleDeleteInfo(const char *hostname,
				      const char *datatype,
				      const char *dir)
  
{

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_DMAP,
	     DMAP_DELETE_INFO);
  
  // clear message parts
  
  clearParts();

  // fill out info struct

  DMAP_info_t info;
  memset(&info, 0, sizeof(DMAP_info_t));
  STRncopy(info.hostname, hostname, DMAP_HOSTNAME_LEN);
  STRncopy(info.datatype, datatype, DMAP_DATATYPE_LEN);
  STRncopy(info.dir, dir, DMAP_DIR_LEN);

  // swap to BE

  BE_from_dmap_info(info);

  // add part

  addPart(DMAP_INFO_PART, sizeof(DMAP_info_t), &info);

  // assemble

  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble a DMAP_REQ_SELECTED_SETS_INFO message
//

void *DmapMessage::assembleReqSelectedSetsInfo(const char *datatype,
					       const char *dir,
					       const char *relay_hostlist /* = NULL*/ )
  
{

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_DMAP,
	     DMAP_REQ_SELECTED_SETS_INFO);

  // clear message parts

  clearParts();

  // fill out info struct

  DMAP_info_t info;
  memset(&info, 0, sizeof(DMAP_info_t));
  STRncopy(info.datatype, datatype, DMAP_DATATYPE_LEN);
  STRncopy(info.dir, dir, DMAP_DIR_LEN);
  
  // swap to BE

  BE_from_dmap_info(info);

  // add part

  addPart(DMAP_INFO_PART, sizeof(DMAP_info_t), &info);
  if (relay_hostlist) {
    addPart(DMAP_RELAY_HOST_PART, strlen(relay_hostlist) + 1, relay_hostlist);
  }

  // assemble

  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble a DMAP_REQ_ALL_SETS_INFO message
//

void *DmapMessage::assembleReqAllSetsInfo(const char *relay_hostlist /* = NULL*/ )
  
{

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_DMAP,
	     DMAP_REQ_ALL_SETS_INFO);

  // clear message parts

  clearParts();

  // add relay hostlist if set

  if (relay_hostlist) {
    addPart(DMAP_RELAY_HOST_PART, strlen(relay_hostlist) + 1, relay_hostlist);
  }

  // assemble

  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble a DMAP_REPLY_REG_STATUS
//

void *DmapMessage::assembleReplyRegStatus(const bool errorOccurred /* = FALSE*/,
					  const char *errorStr /* = NULL*/,
					  const DMAP_info_t *info /* = NULL*/ )
  
{

  // set header attributes - flags has error status
  
  setHdrAttr(DS_MESSAGE_TYPE_DMAP,
	     DMAP_REPLY_REG_STATUS, -1,
	     errorOccurred? DMAP_REPLY_ERROR : DMAP_REPLY_OK);

  // clear message parts
  
  clearParts();

  // if error, add error part

  if (errorOccurred && errorStr != NULL) {
    if (info == NULL) {
      addPart(DMAP_ERROR_STRING_PART, strlen(errorStr) + 1, errorStr);
    } else {
      string combo = errorStr;
      combo += ":";
      combo += info->datatype;
      combo += ":";
      combo += info->dir;
      combo += ":";
      combo += info->hostname;
      addPart(DMAP_ERROR_STRING_PART, combo.size() + 1, combo.c_str());
    }
  }
    
  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble a DMAP_REPLY_WITH_INFO
//

void *DmapMessage::assembleReplyWithInfo(const int nInfo,
					 const DMAP_info_t *info,
					 const bool errorOccurred /* = FALSE*/,
					 const char *errorStr /* = NULL*/ )
  
{

  // set header attributes - flags has error status
  
  setHdrAttr(DS_MESSAGE_TYPE_DMAP,
	     DMAP_REPLY_WITH_INFO, -1,
	     errorOccurred? DMAP_REPLY_ERROR : DMAP_REPLY_OK);
  
  // clear message parts
  
  clearParts();

  // if error, add error part

  if (errorOccurred && errorStr != NULL) {
    addPart(DMAP_ERROR_STRING_PART, strlen(errorStr) + 1, errorStr);
  }

  // add info parts

  for (int i = 0; i < nInfo; i++) {
    DMAP_info_t localInfo;
    localInfo = info[i];
    BE_from_dmap_info(localInfo);
    addPart(DMAP_INFO_PART, sizeof(DMAP_info_t), &localInfo);
  }
    
  // assemble

  return (DsMessage::assemble());

}

/////////////////////////////////////
// override the disassemble function
//

int DmapMessage::disassemble(void *in_msg, const int msg_len)

{

  // initialize

  _isRegLatestDataInfo = FALSE;
  _isRegStatusInfo = FALSE;
  _isRegDataSetInfo = FALSE;
  _isRegFullInfo = FALSE;
  _isReqSelectedSetsInfo = FALSE;
  _isReqAllSetsInfo = FALSE;
  _isReplyRegStatus = FALSE;
  _isReplyWithInfo = FALSE;
  _isDeleteInfo = FALSE;
  _errorOccurred = FALSE;
  _errorStr = "unknown error";
  _info.clear();

  // peek at the header to make sure we're looking at the
  // right type of message

  if (decodeHeader(in_msg, msg_len)) {
    cerr << "ERROR - DmapMessage::disassemble" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Bad message header" << endl;
    cerr << "  Message len: " << msg_len << endl;
    return (-1);
  }

  if (_type != DS_MESSAGE_TYPE_DMAP) {
    cerr << "ERROR - DmapMessage::disassemble" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Unknown message type: " << _type << endl;
    cerr << "  Message len: " << msg_len << endl;
    printHeader(&cerr, "");
    return (-1);
  }

  switch (_subType) {
    
  case DMAP_REG_LATEST_DATA_INFO:
    _isRegLatestDataInfo = TRUE;
    break;

  case DMAP_REG_STATUS_INFO:
    _isRegStatusInfo = TRUE;
    break;

  case DMAP_REG_DATA_SET_INFO:
    _isRegDataSetInfo = TRUE;
    break;
    
  case DMAP_REG_FULL_INFO:
    _isRegFullInfo = TRUE;
    break;
    
  case DMAP_DELETE_INFO:
    _isDeleteInfo = TRUE;
    break;
    
  case DMAP_REQ_SELECTED_SETS_INFO:
    _isReqSelectedSetsInfo = TRUE;
    break;
    
  case DMAP_REQ_ALL_SETS_INFO:
    _isReqAllSetsInfo = TRUE;
    break;
    
  case DMAP_REPLY_REG_STATUS:
    _isReplyRegStatus = TRUE;
    break;
    
  case DMAP_REPLY_WITH_INFO:
    _isReplyWithInfo = TRUE;
    break;
    
  default:
    cerr << "ERROR - DmapMessage::disassemble" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Unknown message subType: " << _subType << endl;
    return (-1);

  } // switch

  // disassemble the message using the base-class routine

  if (DsMessage::disassemble(in_msg, msg_len)) {
    cerr << "ERROR - DmapMessage::disassemble" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  In DsMessage::disassemble()" << endl;
    return (-1);
  }

  // Error occurred?

  if (_subType == DMAP_REPLY_ERROR) {
    _errorOccurred = TRUE;
    if (partExists(DMAP_ERROR_STRING_PART)) {
      _errorStr = (char *) getPartByType(DMAP_ERROR_STRING_PART)->getBuf();
    }
  }
    
  // info parts

  int nInfo = partExists(DMAP_INFO_PART);

  if (nInfo > 0) {
    for (int i = 0; i < nInfo; i++) {
      DMAP_info_t info = 
	*((DMAP_info_t *) getPartByType(DMAP_INFO_PART, i)->getBuf());
      BE_to_dmap_info(info);
      _info.push_back(info);
    }
  }
  
  if (partExists(DMAP_RELAY_HOST_PART)) {
    _relayHostList = (char *) getPartByType(DMAP_RELAY_HOST_PART)->getBuf();
  } else {
    _relayHostList = "";
  }
  
  // check consistency of message

  if (_isRegLatestDataInfo) {
    if (nInfo != 1) {
      cerr << "ERROR - DmapMessage::disassemble" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  DMAP_REG_LATEST_DATA_INFO message" << endl;
      cerr << "  Message must have 1 info struct, " << nInfo << " found"
	   << endl;
      return (-1);
    }
  }

  if (_isDeleteInfo) {
    if (nInfo != 1) {
      cerr << "ERROR - DmapMessage::disassemble" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  DMAP_DELETE_INFO message" << endl;
      cerr << "  Message must have 1 info struct, " << nInfo << " found"
	   << endl;
      return (-1);
    }
  }

  if (_isRegDataSetInfo) {
    if (nInfo != 1) {
      cerr << "ERROR - DmapMessage::disassemble" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  DMAP_REG_DATA_SET_INFO message" << endl;
      cerr << "  Message must have 1 info struct, " << nInfo << " found"
	   << endl;
      return (-1);
    }
  }

  if (_isReqSelectedSetsInfo) {
    if (nInfo != 1) {
      cerr << "ERROR - DmapMessage::disassemble" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  DMAP_REQ_SELECTED_SETS_INFO message" << endl;
      cerr << "  Message must have 1 info struct, " << nInfo << " found"
	   << endl;
      return (-1);
    }
  }

  return (0);

}

/////////////////////////////////////
// print
//

void DmapMessage::print(ostream &out , const char *spacer) const
  
{

  switch (_subType) {
    
  case DMAP_REG_LATEST_DATA_INFO:
    out << spacer << "Message type: DMAP_REG_LATEST_DATA_INFO" << endl;
    out << spacer << "  latest_time: " << utimstr(_info[0].latest_time) << endl;
    out << spacer << "  last_reg_time: " << utimstr(_info[0].last_reg_time) << endl;
    if (_info[0].forecast_lead_time >= 0) {
      out << spacer << "  forecast_lead_time: " << _info[0].forecast_lead_time << endl;
    }
    out << spacer << "  hostname: " << _info[0].hostname << endl;
    out << spacer << "  ipaddr: " << _info[0].ipaddr << endl;
    out << spacer << "  datatype: " << _info[0].datatype << endl;
    out << spacer << "  dir: " << _info[0].dir << endl;
    break;

  case DMAP_REG_DATA_SET_INFO:
    out << spacer << "Message type: DMAP_REG_DATA_SET_INFO" << endl;
    out << spacer << "  start_time: " << utimstr(_info[0].start_time) << endl;
    out << spacer << "  end_time: " << utimstr(_info[0].end_time) << endl;
    out << spacer << "  last_reg_time: " << utimstr(_info[0].last_reg_time) << endl;
    out << spacer << "  nfiles: " << _info[0].nfiles << endl;
    out << spacer << "  total_bytes: " << _info[0].total_bytes << endl;
    out << spacer << "  hostname: " << _info[0].hostname << endl;
    out << spacer << "  ipaddr: " << _info[0].ipaddr << endl;
    out << spacer << "  datatype: " << _info[0].datatype << endl;
    out << spacer << "  dir: " << _info[0].dir << endl;
    break;
    
  case DMAP_REQ_SELECTED_SETS_INFO:
    out << spacer << "Message type: DMAP_REQ_SELECTED_SETS_INFO" << endl;
    out << spacer << "  datatype: " << _info[0].datatype << endl;
    out << spacer << "  dir: " << _info[0].dir << endl;
    break;
    
  case DMAP_REQ_ALL_SETS_INFO:
    out << spacer << "Message type: DMAP_REQ_ALL_SETS_INFO" << endl;
    break;
    
  case DMAP_REPLY_REG_STATUS:
    out << spacer << "Message type: DMAP_REPLY_REG_STATUS" << endl;
    break;
    
  case DMAP_REPLY_WITH_INFO:
    out << spacer << "Message type: DMAP_REPLY_WITH_INFO" << endl;
    break;
    
  default:
    break;

  } // switch
  
}

/////////////////////////////////////
// swap
//

void DmapMessage::BE_to_dmap_info(DMAP_info_t &info)
{
  BE_to_array_32(&info, DMAP_INFO_NBYTES_32);
}

void DmapMessage::BE_from_dmap_info(DMAP_info_t &info)
{
  BE_from_array_32(&info, DMAP_INFO_NBYTES_32);
}
