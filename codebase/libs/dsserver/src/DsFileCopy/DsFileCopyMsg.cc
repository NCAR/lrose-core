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
// DsFileCopyMsg.cc
//
// DsFileCopyMsg object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////
//
// The DsFileCopyMsg object provides the message protocol for
// the DsFileCopy service.
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <dsserver/DsFileCopyMsg.hh>
#include <didss/DsMsgPart.hh>
using namespace std;

////////////////////////////////////////////////////////////
// Constructor

DsFileCopyMsg::DsFileCopyMsg(const memModel_t mem_model /* = PointToMem */) :
  DsServerMsg(mem_model)

{
  _forcePut = false;
}

////////////////////////////////////////////////////////////
// destructor

DsFileCopyMsg::~DsFileCopyMsg()

{
}

//////////////////////////////////////////////////
// assemble a DS_FILECOPY_ENQUIRE_FOR_PUT message

void *DsFileCopyMsg::assembleEnquireForPut(const DsURL &data_url,
					   const LdataInfo &ldata_info,
					   const string &file_name,
					   const time_t mod_time,
					   const int file_size,
					   const int overwrite_age /* = -1*/)
  
{
  
  _dataUrl = data_url;
  _fileName = file_name;
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_FILECOPY,
	     DS_FILECOPY_ENQUIRE_FOR_PUT);

  // indicate that this is the start of a series of put messages

  setCategory(StartPut);

  // compile file info

  _clearFileInfo(_fileInfo);
  _fileInfo.mod_time = mod_time;
  _fileInfo.size = file_size;
  _fileInfo.overwrite_age = overwrite_age;

  // make copy of info and byte-swap
  
  file_info_t fileInfo = _fileInfo;
  _BEfromFileInfo(fileInfo);

  // load ldata info
  // cancel displaced dir if it is set
  
  LdataInfo ldata_local(ldata_info);
  ldata_local.setDisplacedDirPath("");

  LdataInfo::info_t info;
  ldata_local.copyToInfo(info);
  ldata_local.BEfromInfo(info);
  MemBuf leadTimeBuf;
  if (ldata_local.isFcast()) {
    ti32 leadTime = ldata_local.getLeadTime();
    leadTimeBuf.add(&leadTime, sizeof(leadTime));
    BE_from_array_32(leadTimeBuf.getPtr(), leadTimeBuf.getLen());
  }
  ldata_local.assemble(true);
  const void *xmlBuf = ldata_local.getBufPtr();
  int xmlLen = ldata_local.getBufLen();

  // clear message parts

  clearParts();

  // add parts

  const string &urlStr = _dataUrl.getURLStr();
  addPart(DS_FILECOPY_URL_PART, urlStr.size() + 1, urlStr.c_str());
  addPart(DS_FILECOPY_FILENAME_PART, file_name.size() + 1, file_name.c_str());
  addPart(DS_FILECOPY_FILEINFO_PART, sizeof(file_info_t), &fileInfo);
  addPart(DS_FILECOPY_LDATA_XML_PART, xmlLen, xmlBuf);
  addPart(DS_FILECOPY_LDATAINFO_PART, sizeof(LdataInfo::info_t), &info);
  addPart(DS_FILECOPY_LDATAFCASTS_PART,
	  leadTimeBuf.getLen(), leadTimeBuf.getPtr());

  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble enquire_for_put return
//

void *DsFileCopyMsg::assembleEnquireForPutReturn
(const bool doPut,
 const bool errorOccurred /* = false*/,
 const char *errorStr /* = NULL*/ )
  
{

  // if error occurred, make sure doPut is false
  
  bool do_put = doPut;
  if (errorOccurred) {
    do_put = false;
  }

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_FILECOPY, // type
	     DS_FILECOPY_RETURN, // subType
	     DS_FILECOPY_ENQUIRE_FOR_PUT, // mode
	     do_put? DS_FILECOPY_YES_PUT : DS_FILECOPY_NO_PUT); // flags

  if (!do_put) {
    setCategory(EndSeries);
  }

  if (errorOccurred) {
    setError(true);
  }
  
  // clear message parts

  clearParts();

  // add parts
  
  if (errorOccurred && errorStr != NULL) {
    addPart(DS_FILECOPY_ERRORSTR_PART,
	    strlen(errorStr) + 1, errorStr);
  }

  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////////
// assemble a DS_FILECOPY_PUT_AFTER_ENQUIRE message
//

void *DsFileCopyMsg::assemblePutAfterEnquire(const void *fileBuf,
					     const int bufLen)
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_FILECOPY,
	     DS_FILECOPY_PUT_AFTER_ENQUIRE);
  
  // clear message parts

  clearParts();

  // add parts
  
  addPart(DS_FILECOPY_FILEBUF_PART, bufLen, fileBuf);

  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble put after enquire return
//

void *DsFileCopyMsg::assemblePutAfterEnquireReturn
(const bool errorOccurred /* = false*/,
 const char *errorStr /* = NULL*/)
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_FILECOPY, // type
	     DS_FILECOPY_RETURN, // subType
	     DS_FILECOPY_PUT_AFTER_ENQUIRE); // mode

  setCategory(EndSeries);
  
  if (errorOccurred) {
    setError(true);
  }
  
  // clear message parts

  clearParts();

  // add parts
  
  if (errorOccurred && errorStr != NULL) {
    addPart(DS_FILECOPY_ERRORSTR_PART,
	    strlen(errorStr) + 1, errorStr);
  }

  // assemble
  
  return (DsMessage::assemble());

}

/////////////////////////////////////////////
// assemble a DS_FILECOPY_PUT_FORCED message
  
void *DsFileCopyMsg::assemblePutForced(const DsURL &data_url,
				       const LdataInfo &ldata_info,
				       const string &file_name,
				       const time_t mod_time,
				       const int file_size,
				       const void *fileBuf,
				       const int bufLen)

{

  _dataUrl = data_url;
  _fileName = file_name;

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_FILECOPY,
	     DS_FILECOPY_PUT_FORCED);

  // indicate that this is the start of a series of put messages

  setCategory(StartPut);
  
  // compile file info

  _clearFileInfo(_fileInfo);
  _fileInfo.mod_time = mod_time;
  _fileInfo.size = file_size;

  // make copy of info and byte-swap
  
  file_info_t fileInfo = _fileInfo;
  _BEfromFileInfo(fileInfo);

  // load ldata info
  // cancel displaced dir if it is set
  
  LdataInfo ldata_local(ldata_info);
  ldata_local.setDisplacedDirPath("");

  LdataInfo::info_t info;
  ldata_local.copyToInfo(info);
  ldata_local.BEfromInfo(info);
  MemBuf leadTimeBuf;
  if (ldata_local.isFcast()) {
    ti32 leadTime = ldata_local.getLeadTime();
    leadTimeBuf.add(&leadTime, sizeof(leadTime));
    BE_from_array_32(leadTimeBuf.getPtr(), leadTimeBuf.getLen());
  }
  ldata_local.assemble(true);
  const void *xmlBuf = ldata_local.getBufPtr();
  int xmlLen = ldata_local.getBufLen();

  // clear message parts

  clearParts();

  // add parts

  const string &urlStr = _dataUrl.getURLStr();
  addPart(DS_FILECOPY_URL_PART, urlStr.size() + 1, urlStr.c_str());
  addPart(DS_FILECOPY_FILENAME_PART, file_name.size() + 1, file_name.c_str());
  addPart(DS_FILECOPY_FILEINFO_PART, sizeof(file_info_t), &fileInfo);
  addPart(DS_FILECOPY_FILEBUF_PART, bufLen, fileBuf);
  addPart(DS_FILECOPY_LDATA_XML_PART, xmlLen, xmlBuf);
  addPart(DS_FILECOPY_LDATAINFO_PART, sizeof(LdataInfo::info_t), &info);
  addPart(DS_FILECOPY_LDATAFCASTS_PART,
	  leadTimeBuf.getLen(), leadTimeBuf.getPtr());
  
  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble put forced return
//

void *DsFileCopyMsg::assemblePutForcedReturn
(const bool errorOccurred /* = false*/,
 const char *errorStr /* = NULL*/)
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_FILECOPY, // type
	     DS_FILECOPY_RETURN, // subType
	     DS_FILECOPY_PUT_FORCED); // mode

  setCategory(EndSeries);
  
  if (errorOccurred) {
    setError(true);
  }
  
  // clear message parts

  clearParts();

  // add parts
  
  if (errorOccurred && errorStr != NULL) {
    addPart(DS_FILECOPY_ERRORSTR_PART,
	    strlen(errorStr) + 1, errorStr);
  }

  // assemble
  
  return (DsMessage::assemble());

}

/////////////////////////////////////
// override the disassemble function
//

int DsFileCopyMsg::disassemble(void *inMsg, const int msgLen)

{

  char tmpStr[128];
  // initialize

  _fileName = "\0";
  _errStr = "\0";
  _clearFileInfo(_fileInfo);
  _fileBuf = NULL;

  // peek at the header to make sure we're looking at the
  // right type of message

  if (decodeHeader(inMsg, msgLen)) {
    _errStr = "ERROR - DsFileCopyMsg::disassemble\n";
    TaStr::AddStr(_errStr, "  ", DateTime::str());
    _errStr += "  Bad message header\n";
    _errStr += "  Message len: ";
    sprintf(tmpStr, "%d\n", msgLen);
    _errStr += tmpStr;
    return (-1);
  }

  if (_type != DS_MESSAGE_TYPE_FILECOPY) {
    _errStr = "ERROR - DsFileCopyMsg::disassemble\n";
    TaStr::AddStr(_errStr, "  ", DateTime::str());
    _errStr += "  Unknown message type: \n";
    sprintf(tmpStr, "%d\n", _type);
    _errStr += tmpStr;
    _errStr += "  Message len: ";
    sprintf(tmpStr, "%d\n", msgLen);
    _errStr += tmpStr;
    return (-1);
  }

  // disassemble the message using the base-class routine
  
  if (DsMessage::disassemble(inMsg, msgLen)) {
    _errStr = "ERROR - DsFileCopyMsg::disassemble\n";
    TaStr::AddStr(_errStr, "  ", DateTime::str());
    _errStr += "ERROR in DsMessage::disassemble()\n";
    return (-1);
  }

  // set data members to parts

  // url
  if (partExists(DS_FILECOPY_URL_PART)) {

    string urlStr = (char *) getPartByType(DS_FILECOPY_URL_PART)->getBuf();
    _dataUrl.setURLStr(urlStr);
    
    if (_ldataInfo.setDir(_dataUrl.getFile())) {
      _errStr = "ERROR - DsFileCopyMsg::disassemble\n";
      TaStr::AddStr(_errStr, "  ", DateTime::str());
      _errStr += "  Invalid URL: '";
      _errStr += urlStr;
      _errStr += "'\n";
      return (-1);
    }

  }

  // file name
  if (partExists(DS_FILECOPY_FILENAME_PART)) {
    _fileName = (char *) getPartByType(DS_FILECOPY_FILENAME_PART)->getBuf();
  }

  // file info
  if (partExists(DS_FILECOPY_FILEINFO_PART)) {
    memcpy(&_fileInfo, getPartByType(DS_FILECOPY_FILEINFO_PART)->getBuf(),
	   sizeof(file_info_t));
    _BEtoFileInfo(_fileInfo);
  }

  // ldata info
  if (partExists(DS_FILECOPY_LDATA_XML_PART)) {
    DsMsgPart *xmlPart = getPartByType(DS_FILECOPY_LDATA_XML_PART);
    const void *xmlBuf = xmlPart->getBuf();
    int xmlLen = xmlPart->getLength();
    _ldataInfo.disassemble(xmlBuf, xmlLen);
  } else if (partExists(DS_FILECOPY_LDATAINFO_PART)) {
    LdataInfo::info_t info;
    memcpy(&info, getPartByType(DS_FILECOPY_LDATAINFO_PART)->getBuf(),
	   sizeof(LdataInfo::info_t));
    _ldataInfo.BEtoInfo(info);
    _ldataInfo.setFromInfo(info);
    if (_ldataInfo.isFcast()) {
      DsMsgPart *part = getPartByType(DS_FILECOPY_LDATAFCASTS_PART);
      if (part == NULL) {
	_errStr = "ERROR - DsFileCopyMsg::disassemble\n";
	TaStr::AddStr(_errStr, "  ", DateTime::str());
	_errStr += "  No DS_FILECOPY_LDATAFCASTS_PART.\n";
	return -1;
      }
      MemBuf leadTimeBuf;
      leadTimeBuf.add(part->getBuf(), part->getLength());
      BE_to_array_32(leadTimeBuf.getPtr(), leadTimeBuf.getLen());
      si32 *leadTime = (si32 *) leadTimeBuf.getPtr();
      _ldataInfo.setLeadTime(*leadTime);
    }
  }

  // file buffer
  if (partExists(DS_FILECOPY_FILEBUF_PART)) {
    DsMsgPart *fileBufPart = getPartByType(DS_FILECOPY_FILEBUF_PART);
    _fileBuf = (void *) fileBufPart->getBuf();
    _fileLen = fileBufPart->getLength();
  }

  // error string
  if (partExists(DS_FILECOPY_ERRORSTR_PART)) {
    _errStr = (char *) getPartByType(DS_FILECOPY_ERRORSTR_PART)->getBuf();
  }

  if (_subType == DS_FILECOPY_RETURN) {
    if (_mode == DS_FILECOPY_ENQUIRE_FOR_PUT ||
	_mode == DS_FILECOPY_ENQUIRE_RETURN) {
      if (_flags == DS_FILECOPY_YES_PUT) {
	_doPut = true;
      } else {
	_doPut = false;
      }
    }
  }

  return (0);

}

////////////////
// print message
//

void DsFileCopyMsg::print(ostream &out, const char *spacer) const

{

  out << spacer << "  dataUrl: " << _dataUrl.getURLStr() << endl;

  switch (_subType) {
    
  case DS_FILECOPY_ENQUIRE_FOR_PUT:
    out << spacer << "Message subType: DS_FILECOPY_ENQUIRE_FOR_PUT" << endl;
    out << spacer << "  file name: " << _fileName << endl;
    out << spacer << "  file size: " << _fileInfo.size << endl;
    out << spacer << "  file mod time: " << utimstr(_fileInfo.mod_time) << endl;
    out << spacer << "  overwrite age: " << _fileInfo.overwrite_age << endl;
    break;

  case DS_FILECOPY_PUT_AFTER_ENQUIRE:
    out << spacer << "Message subType: DS_FILECOPY_PUT_AFTER_ENQUIRE" << endl;
    out << spacer << "  File buf len: " << _fileLen << endl;
    out << spacer << "LdataInfo:" << endl;
    _ldataInfo.printFull(out);
    break;

  case DS_FILECOPY_PUT_FORCED:
    out << spacer << "Message subType: DS_FILECOPY_PUT_FORCED" << endl;
    out << spacer << "  file name: " << _fileName << endl;
    out << spacer << "  file size: " << _fileInfo.size << endl;
    out << spacer << "  file mod time: " << utimstr(_fileInfo.mod_time) << endl;
    break;

  case DS_FILECOPY_RETURN:

    out << spacer << "Message subType: DS_FILECOPY_RETURN" << endl;

    switch (_mode) {
      
    case DS_FILECOPY_ENQUIRE_FOR_PUT:
    case DS_FILECOPY_ENQUIRE_RETURN: // deprecated
      out << spacer << "  Request type: DS_FILECOPY_ENQUIRE_FOR_PUT" << endl;
      if (_flags == DS_FILECOPY_YES_PUT) {
	out << spacer << "    YES_PUT" << endl;
      } else {
	out << spacer << "    NO_PUT" << endl;
      }
      break;
      
    case DS_FILECOPY_PUT_AFTER_ENQUIRE:
      out << spacer << "  Request type: DS_FILECOPY_PUT_AFTER_ENQUIRE" << endl;
      break;
      
    case DS_FILECOPY_PUT_FORCED:
      out << spacer << "  Request type: DS_FILECOPY_PUT_FORCED" << endl;
      break;
      
    default:
      break;
      
    } // switch (_mode)
    
    if (getError()) {
      out << spacer << "ERROR OCCURRED" << endl;
      out << spacer << _errStr;
    }
      
    break;
    
  default:
    break;

  } // switch (_subType)

  return;

}

///////////////////////////////////////
// Convert to BE from file info struct
//

void DsFileCopyMsg::_BEfromFileInfo(file_info_t &info)

{
  BE_from_array_32(&info, sizeof(file_info_t));
}
  
///////////////////////////////////////
// Convert from BE to file info struct
//

void DsFileCopyMsg::_BEtoFileInfo(file_info_t &info)

{
  BE_to_array_32(&info, sizeof(file_info_t));
}

//////////////////////////////  
// clear the file info struct

void DsFileCopyMsg::_clearFileInfo(file_info_t &info)

{
  memset(&info, 0, sizeof(file_info_t));
}


/////////////////////////////////////
// add error string with int argument

void DsFileCopyMsg::_intErr(const char *err_str, const int iarg)
{
  _errStr += err_str;
  char str[32];
  sprintf(str, "%d\n", iarg);
  _errStr += str;
}

////////////////////////////////////////
// add error string with string argument

void DsFileCopyMsg::_strErr(const char *err_str, const string &sarg)
{
  _errStr += err_str;
  _errStr += sarg;
  _errStr += "\n";
}

