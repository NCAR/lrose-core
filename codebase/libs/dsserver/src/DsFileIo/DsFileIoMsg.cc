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
// DsFileIoMsg.cc
//
// DsFileIoMsg object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////
//
// The DsFileIoMsg object provides the message protocol for
// the DsFileIo service.
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <dsserver/DsFileIoMsg.hh>
#include <didss/DsMsgPart.hh>
#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>
using namespace std;

////////////////////////////////////////////////////////////
// Constructor

DsFileIoMsg::DsFileIoMsg(memModel_t mem_model /* = CopyMem */) :
  DsServerMsg(mem_model)

{
}

////////////////////////////////////////////////////////////
// destructor

DsFileIoMsg::~DsFileIoMsg()

{
}

///////////////////////////////////////////////
// assemble a DS_FILEIO_FOPEN message
//

void *DsFileIoMsg::assemblefOpen(const string &url_str,
				 const string &mode_str)
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_FILEIO,
	     DS_FILEIO_FOPEN);

  // indicate that this is the start of a series of put or
  // get messages

  if (mode_str[0] == 'w') {
    setCategory(StartPut);
  } else {
    setCategory(StartGet);
  }

  // clear message parts

  clearParts();

  // add parts

  addPart(DS_FILEIO_URL_PART, url_str.size() + 1, url_str.c_str());
  addPart(DS_FILEIO_MODE_PART, mode_str.size() + 1, mode_str.c_str());

  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble fOpen return
//

void *DsFileIoMsg::assemblefOpenReturn(const bool errorOccurred /* = false*/,
				       const char *errorStr /* = NULL*/ )
  
{

  _clearInfo();
  return (assembleReturn(DS_FILEIO_FOPEN,
			 errorOccurred,
			 errorStr));

}

///////////////////////////////////////////////
// assemble a DS_FILEIO_FCLOSE message
//

void *DsFileIoMsg::assemblefClose()
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_FILEIO,
	     DS_FILEIO_FCLOSE);
  
  // clear message parts

  clearParts();

  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble fClose return
//

void *DsFileIoMsg::assemblefCloseReturn(const bool errorOccurred /* = false*/,
					const char *errorStr /* = NULL*/ )
  
{
  
  _clearInfo();

  void *buf = assembleReturn(DS_FILEIO_FCLOSE,
			     errorOccurred,
			     errorStr);
  
  setCategory(EndSeries);

  return (buf);

}

///////////////////////////////////////////////
// assemble a DS_FILEIO_FWRITE message
//

void *DsFileIoMsg::assemblefWrite(const void *ptr,
				  const size_t size, const size_t n)
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_FILEIO,
	     DS_FILEIO_FWRITE);
  
  // clear message parts

  clearParts();

  // load up info struct

  _clearInfo();
  _info.size = size;
  _info.nelements = n;
  _BEfromInfo();
  
  // add parts

  addPart(DS_FILEIO_INFO_PART, sizeof(info_t), &_info);
  addPart(DS_FILEIO_DATA_PART, size * n, ptr);

  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble fWrite return
//

void *DsFileIoMsg::assemblefWriteReturn(const int nwritten,
					const bool errorOccurred /* = false*/,
					const char *errorStr /* = NULL*/ )
  
{
  
  _clearInfo();
  _info.nelements = nwritten;
  return (assembleReturn(DS_FILEIO_FWRITE,
			 errorOccurred,
			 errorStr));

}

///////////////////////////////////////////////
// assemble a DS_FILEIO_FREAD message
//

void *DsFileIoMsg::assemblefRead(const size_t size, const size_t n)
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_FILEIO,
	     DS_FILEIO_FREAD);
  
  // clear message parts

  clearParts();

  // load up info struct

  _clearInfo();
  _info.size = size;
  _info.nelements = n;
  _BEfromInfo();
  
  // add parts

  addPart(DS_FILEIO_INFO_PART, sizeof(info_t), &_info);

  // assemble
  
  return (DsMessage::assemble());

}


///////////////////////////////////////////////
// assemble fRead return
//

void *DsFileIoMsg::assemblefReadReturn(const int nread,
				       const void *data /* = NULL*/,
				       const bool errorOccurred /* = false*/,
				       const char *errorStr /* = NULL*/ )
  
{
  
  _clearInfo();
  _info.nelements = nread;
  return (assembleReturn(DS_FILEIO_FREAD,
			 errorOccurred,
			 errorStr,
			 data,
			 nread));

}

///////////////////////////////////////////////
// assemble a DS_FILEIO_FPUTS message
//

void *DsFileIoMsg::assemblefPuts(const char *str)
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_FILEIO,
	     DS_FILEIO_FPUTS);
  
  // clear message parts

  clearParts();

  // add parts

  addPart(DS_FILEIO_DATA_PART, strlen(str) + 1, str);

  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble fPuts return
//

void *DsFileIoMsg::assemblefPutsReturn(const bool errorOccurred /* = false*/,
				       const char *errorStr /* = NULL*/ )
  
{
  
  _clearInfo();
  return (assembleReturn(DS_FILEIO_FPUTS,
			 errorOccurred,
			 errorStr));

}

///////////////////////////////////////////////
// assemble a DS_FILEIO_FGETS message
//

void *DsFileIoMsg::assemblefGets(const int size)
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_FILEIO,
	     DS_FILEIO_FGETS);
  
  // load up info struct

  _clearInfo();
  _info.size = size;
  _BEfromInfo();

  // clear message parts

  clearParts();

  // add parts

  addPart(DS_FILEIO_INFO_PART, sizeof(info_t), &_info);

  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble fGets return
//

void *DsFileIoMsg::assemblefGetsReturn(const char *str /* = NULL*/,
				       const bool errorOccurred /* = false*/,
				       const char *errorStr /* = NULL*/ )
  
{
  
  _clearInfo();
  if (str) {
    return (assembleReturn(DS_FILEIO_FGETS,
			   errorOccurred,
			   errorStr,
			   str,
			   strlen(str) + 1));
  } else {
    return (assembleReturn(DS_FILEIO_FGETS,
			   errorOccurred,
			   errorStr));
  }

}

///////////////////////////////////////////////
// assemble a DS_FILEIO_FSEEK message
//

void *DsFileIoMsg::assemblefSeek(const long offset, const int whence)
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_FILEIO,
	     DS_FILEIO_FSEEK);
  
  // clear message parts

  clearParts();

  // load up info struct

  _clearInfo();
  _info.offset = offset;
  if (whence == SEEK_CUR) {
    _info.whence = DS_FILEIO_SEEK_CUR;
  } else if (whence == SEEK_END) {
    _info.whence = DS_FILEIO_SEEK_END;
  } else {
    _info.whence = DS_FILEIO_SEEK_SET;
  }
  _BEfromInfo();

  // add parts

  addPart(DS_FILEIO_INFO_PART, sizeof(info_t), &_info);

  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble fSeek return
//

void *DsFileIoMsg::assemblefSeekReturn(const bool errorOccurred /* = false*/,
				       const char *errorStr /* = NULL*/ )
  
{
  
  _clearInfo();
  return (assembleReturn(DS_FILEIO_FSEEK,
			 errorOccurred,
			 errorStr));

}

///////////////////////////////////////////////
// assemble a DS_FILEIO_FTELL message
//

void *DsFileIoMsg::assemblefTell()
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_FILEIO,
	     DS_FILEIO_FTELL);
  
  // clear message parts

  clearParts();

  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble fTell return
//

void *DsFileIoMsg::assemblefTellReturn(const long filepos,
				       const bool errorOccurred /* = false*/,
				       const char *errorStr /* = NULL*/ )
  
{
  
  _clearInfo();
  _info.filepos = (si32) filepos;
  return (assembleReturn(DS_FILEIO_FTELL,
			 errorOccurred,
			 errorStr));

}

///////////////////////////////////////////////
// assemble a DS_FILEIO_FSTAT message
//

void *DsFileIoMsg::assemblefStat()
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_FILEIO,
	     DS_FILEIO_FSTAT);
  
  // clear message parts

  clearParts();

  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble fstat return
//

void *DsFileIoMsg::assemblefStatReturn(const off_t stat_size,
				       const time_t stat_atime,
				       const time_t stat_mtime,
				       const time_t stat_ctime,
				       const bool errorOccurred /* = false*/,
				       const char *errorStr /* = NULL*/ )
  
{
  
  _clearInfo();
  _info.stat_size = (si32) stat_size;
  _info.stat_atime = (ti32) stat_atime;
  _info.stat_mtime = (ti32) stat_mtime;
  _info.stat_ctime = (ti32) stat_ctime;
  return (assembleReturn(DS_FILEIO_FSTAT,
			 errorOccurred,
			 errorStr));

}

//////////////////////////////////////
// assemble a DS_FILEIO_RETURN message
//

void *DsFileIoMsg::assembleReturn(const int requestSubType,
				  const bool errorOccurred /* = false*/,
				  const char *errorStr /* = NULL*/,
				  const void *data /* = NULL*/,
				  const int dataLen /* = 0*/ )

  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_FILEIO,
	     DS_FILEIO_RETURN,
	     requestSubType,
	     errorOccurred);
  
  // clear message parts

  clearParts();

  // swap the info struct

  _BEfromInfo();
  
  // add parts
  
  addPart(DS_FILEIO_INFO_PART, sizeof(info_t), &_info);
  if (errorOccurred && errorStr != NULL) {
    addPart(DS_FILEIO_ERRORSTR_PART,
	    strlen(errorStr) + 1, errorStr);
  }
  if (data) {
    addPart(DS_FILEIO_DATA_PART, dataLen, data);
  }

  // assemble
  
  return (DsMessage::assemble());

}

/////////////////////////////////////
// override the disassemble function
//

int DsFileIoMsg::disassemble(void *in_msg, const int msg_len)

{

  // initialize

  _urlStr = "\0";
  _modeStr = "\0";
  _errStr = "\0";
  _clearInfo();
  _data = NULL;

  // peek at the header to make sure we're looking at the
  // right type of message

  if (decodeHeader(in_msg, msg_len)) {
    cerr << "ERROR - DsFileIoMsg::disassemble" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Bad message header" << endl;
    cerr << "  Message len: " << msg_len << endl;
    return (-1);
  }

  if (_type != DS_MESSAGE_TYPE_FILEIO) {
    cerr << "ERROR - DsFileIoMsg::disassemble" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Unknown message type: " << _type << endl;
    cerr << "  Message len: " << msg_len << endl;
    printHeader(&cerr, "  ");
    return (-1);
  }

  // disassemble the message using the base-class routine
  
  if (DsMessage::disassemble(in_msg, msg_len)) {
    cerr << "ERROR - DsFileIoMsg::disassemble" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Error in DsMessage::disassemble()" << endl;
    return (-1);
  }

  // set data members to parts

  if (partExists(DS_FILEIO_URL_PART)) {
    _urlStr = (char *) getPartByType(DS_FILEIO_URL_PART)->getBuf();
  }
  if (partExists(DS_FILEIO_MODE_PART)) {
    _modeStr = (char *) getPartByType(DS_FILEIO_MODE_PART)->getBuf();
  }
  if (partExists(DS_FILEIO_ERRORSTR_PART)) {
    _errStr = (char *) getPartByType(DS_FILEIO_ERRORSTR_PART)->getBuf();
  }
  if (partExists(DS_FILEIO_INFO_PART)) {
    memcpy(&_info, getPartByType(DS_FILEIO_INFO_PART)->getBuf(),
	   sizeof(info_t));
    _BEtoInfo();
  }
  if (partExists(DS_FILEIO_DATA_PART)) {
    _data = (void *) getPartByType(DS_FILEIO_DATA_PART)->getBuf();
  }

  return (0);

}

////////////////
// print message
//

void DsFileIoMsg::print(ostream &out, const char *spacer) const

{
  switch (_subType) {
    
  case DS_FILEIO_FOPEN:
    out << spacer << "Message subType: DS_FILEIO_FOPEN" << endl;
    out << spacer << "  url: " << _urlStr << endl;
    out << spacer << "  mode: " << _modeStr << endl;
    break;

  case DS_FILEIO_FCLOSE:
    out << spacer << "Message subType: DS_FILEIO_FCLOSE" << endl;
    break;

  case DS_FILEIO_FWRITE:
    out << spacer << "Message subType: DS_FILEIO_FWRITE" << endl;
    out << spacer << "  size: " << _info.size << endl;
    out << spacer << "  nelements: " << _info.nelements << endl;
    break;

  case DS_FILEIO_FREAD:
    out << spacer << "Message subType: DS_FILEIO_FREAD" << endl;
    out << spacer << "  size: " << _info.size << endl;
    out << spacer << "  nelements: " << _info.nelements << endl;
    break;

  case DS_FILEIO_FPUTS:
    out << spacer << "Message subType: DS_FILEIO_FPUTS" << endl;
    break;

  case DS_FILEIO_FGETS:
    out << spacer << "Message subType: DS_FILEIO_FGETS" << endl;
    out << spacer << "  size: " << _info.size << endl;
    break;

  case DS_FILEIO_FSEEK:
    out << spacer << "Message subType: DS_FILEIO_FSEEK" << endl;
    out << spacer << "  offset: " << _info.offset << endl;
    if (_info.whence == DS_FILEIO_SEEK_SET) {
      out << spacer << "  whence: SEEK_SET" << endl;
    } else if (_info.whence == DS_FILEIO_SEEK_CUR) {
      out << spacer << "  whence: SEEK_CUR" << endl;
    } else if (_info.whence == DS_FILEIO_SEEK_END) {
      out << spacer << "  whence: SEEK_END" << endl;
    } 
    break;

  case DS_FILEIO_FTELL:
    out << spacer << "Message subType: DS_FILEIO_FTELL" << endl;
    break;

  case DS_FILEIO_FSTAT:
    out << spacer << "Message subType: DS_FILEIO_FSTAT" << endl;
    break;

  case DS_FILEIO_RETURN:

    out << spacer << "Message subType: DS_FILEIO_RETURN" << endl;

    switch (_mode) {
    
    case DS_FILEIO_FOPEN:
      out << spacer << "  Request type: DS_FILEIO_FOPEN" << endl;
      break;
      
    case DS_FILEIO_FCLOSE:
      out << spacer << "  Request type: DS_FILEIO_FCLOSE" << endl;
      break;
      
    case DS_FILEIO_FWRITE:
      out << spacer << "  Request type: DS_FILEIO_FWRITE" << endl;
      out << spacer << "    nwritten: " << _info.nelements << endl;
      break;
      
    case DS_FILEIO_FREAD:
      out << spacer << "  Request type: DS_FILEIO_FREAD" << endl;
      out << spacer << "    nread: " << _info.nelements << endl;
      break;
      
    case DS_FILEIO_FPUTS:
      out << spacer << "  Request type: DS_FILEIO_FPUTS" << endl;
      break;
      
    case DS_FILEIO_FGETS:
      out << spacer << "  Request type: DS_FILEIO_FGETS" << endl;
      break;
      
    case DS_FILEIO_FSEEK:
      out << spacer << "  Request type: DS_FILEIO_FSEEK" << endl;
      break;
      
    case DS_FILEIO_FTELL:
      out << spacer << "  Request type: DS_FILEIO_FTELL" << endl;
      out << spacer << "    filepos: " << _info.filepos << endl;
      break;

    case DS_FILEIO_FSTAT:
      out << spacer << "  Request type: DS_FILEIO_FSTAT" << endl;
      out << spacer << "    stat_size: " << _info.stat_size << endl;
      out << spacer << "    stat_atime: " << utimstr(_info.stat_atime) << endl;
      out << spacer << "    stat_mtime: " << utimstr(_info.stat_mtime) << endl;
      out << spacer << "    stat_ctime: " << utimstr(_info.stat_ctime) << endl;
      break;

    default:
      break;
      
    } // switch (_mode)

    if (getFlags()) {
      out << spacer << "ERROR OCCURRED" << endl;
      out << spacer << _errStr;
    }
      
    break;
    
  default:
    break;

  } // switch (_subType)

}

//////////////////////////////////
// Convert to BE from info struct
//

void DsFileIoMsg::_BEfromInfo()

{
  BE_from_array_32(&_info, sizeof(info_t));
}
  
//////////////////////////////////
// Convert from BE to info struct
//

void DsFileIoMsg::_BEtoInfo()

{
  BE_to_array_32(&_info, sizeof(info_t));
}

/////////////////////////  
// clear the info struct

void DsFileIoMsg::_clearInfo()

{
  memset(&_info, 0, sizeof(info_t));
}


