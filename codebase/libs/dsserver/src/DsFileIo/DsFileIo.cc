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
// DsFileIo.cc
//
// DsFileIo object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////
//
// The DsFileIo object performs simple buffered file io both directly
// to the local disk and via a DsFileIoServer.
//
///////////////////////////////////////////////////////////////

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <iostream>
#include <dsserver/DsFileIo.hh>
#include <dsserver/DsFileIoMsg.hh>
#include <dsserver/DsSvrMgrSocket.hh>
#include <didss/RapDataDir.hh>
#include <didss/DsURL.hh>
#include <toolsa/umisc.h>
#include <toolsa/mem.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/toolsa_macros.h>
using namespace std;

////////////////////////////////////////////////////////////
// Constructor

DsFileIo::DsFileIo(DsMessage::memModel_t mem_model /* = DsMessage::CopyMem*/ )
  : ThreadSocket()

{

  _isOpen = false;
  _filep = NULL;
  _url = NULL;
  _msg = NULL;
  _msg = new DsFileIoMsg(mem_model);

}

////////////////////////////////////////////////////////////
// destructor

DsFileIo::~DsFileIo()

{

  if (_filep) {
    fclose (_filep);
  }
  if (_url) {
    delete (_url);
  }
  if (_msg) {
    delete (_msg);
  }

}

///////////////////////////////////////
// fOpen()
// Returns 0 on success, -1 on error
//
// Error message available via getErrorStr()

int DsFileIo::fOpen(const char *url_str, const char *mode_str,
		    bool force_local /* = false*/ )
  
{

  _urlStr = url_str;
  _modeStr = mode_str;

  // check if already open

  if (_isOpen) {
    _errStr = "Cannot open - file already open.";
    _errStr += " URL: '";
    _errStr += url_str;
    _errStr += "'";
    return (-1);
  }

  // decode URL and check for validity

  _url = new DsURL(_urlStr);
  if (!_url->isValid()) {
    _errStr = "Invalid URL: '";
    _errStr += url_str;
    _errStr += "'";
    return (-1);
  }

  // determine if host is local

  if (force_local) {
    _isLocal = true;
  } else {
    string localHost = "localhost";
    if (_url->getHost() == localHost) {
      _isLocal = true;
    } else {
      _isLocal = false;
    }
  }

  int iret = 0;
  if (_isLocal) {
    iret = _fOpenLocal();
  } else {
    iret = _fOpenRemote();
  }

  if (iret) {
    return (-1);
  } else {
    _isOpen = true;
    return (0);
  }
    
}

////////////////////////////////////////
// fClose()
// Returns 0 on success, -1 on error
//
// Error message available via getErrorStr()

int DsFileIo::fClose()

{

  if (!_isOpen) {
    _errStr = "Cannot close - file not open.";
    return (-1);
  }

  int iret;
  if (_isLocal) {
    iret = _fCloseLocal();
  } else {
    iret = _fCloseRemote();
  }

  delete (_url);
  _url = NULL;
  _isOpen = false;

  return (iret);
  
}

////////////////////////////////////////
// fWrite()
//
// Returns number of elements written
//
// Error message available via getErrorStr()

size_t DsFileIo::fWrite(const void *ptr,
			const size_t size, const size_t n)

{

  if (!_isOpen) {
    _errStr = "Cannot fWrite - file not open";
    return (0);
  }

  if (_isLocal) {
    return (_fWriteLocal(ptr, size, n));
  } else {
    return (_fWriteRemote(ptr, size, n));
  }

}

////////////////////////////////////////
// fRead()
//
// Loads ptr - memory allocated by class.
// Returns number of elements read
//
// Error message available via getErrorStr()

size_t DsFileIo::fRead(void *ptr, const size_t size, const size_t n)

{

  if (!_isOpen) {
    _errStr = "Cannot fRead - file not open";
    return (0);
  }

  if (_isLocal) {
    return (_fReadLocal(ptr, size, n));
  } else {
    return (_fReadRemote(ptr, size, n));
  }
  
}

//////////////////////////////////////////
// fPrintf()
//
// This is implemented through a sprintf() and fPuts().
// String printed must not exceed 8K chars in length.
//
// Returns number of items printed.
//
// Error message available via getErrorStr()

int DsFileIo::fPrintf(const char *format, ... )

{

  if (!_isOpen) {
    _errStr = "Cannot fPrintf - file not open";
    return (-1);
  }

  char str[8192];

  va_list ap;
  va_start(ap, format);
  int nprint = vsprintf(str, format, ap);
  va_end(ap);

  if (nprint < 0) {
    _errStr = strerror(errno);
    return(nprint);
  }

  if (fPuts(str)) {
    return (-1);
  } else {
    return (nprint);
  }

}

//////////////////////////////////////////
// fPuts()
//
// Returns 0 on success, EOF on error
//
// Error message available via getErrorStr()

int DsFileIo::fPuts(const char *str)

{

  if (!_isOpen) {
    _errStr = "Cannot fPuts - file not open";
    return (-1);
  }

  if (_isLocal) {
    return (_fPutsLocal(str));
  } else {
    return (_fPutsRemote(str));
  }

}

//////////////////////////////////////////
// fGets()
//
// Returns pointer to string read, NULL on error.
//
// Error message available via getErrorStr()

char *DsFileIo::fGets(char *str, int size)

{
  
  if (!_isOpen) {
    _errStr = "Cannot fGets - file not open";
    return (NULL);
  }
  
  if (_isLocal) {
    return (_fGetsLocal(str, size));
  } else {
    return (_fGetsRemote(str, size));
  }

}

//////////////////////////////////////////
// fSeek()
//
// Returns 0 on success, -1 on error
//
// Error message available via getErrorStr()

int DsFileIo::fSeek(const long offset, const int whence)

{

  if (!_isOpen) {
    _errStr = "Cannot fSeek - file not open";
    return (-1);
  }
  
  if (_isLocal) {
    return (_fSeekLocal(offset, whence));
  } else {
    return (_fSeekRemote(offset, whence));
  }

}

//////////////////////////////////////////
// fTell()
//
// Returns file pos on success, -1 on error
//
// Error message available via getErrorStr()

long DsFileIo::fTell()

{

  if (!_isOpen) {
    _errStr = "Cannot fTell - file not open";
    return (-1);
  }
  
  if (_isLocal) {
    return (_fTellLocal());
  } else {
    return (_fTellRemote());
  }

}

//////////////////////////////////////////
// fStat()
//
// Fills out size and times vals.
// Returns 0 on success, -1 on error.
//
// Error message available via getErrorStr()

int DsFileIo::fStat(off_t *size_p,
		    time_t *mtime_p /* = NULL*/,
		    time_t *atime_p /* = NULL*/,
		    time_t *ctime_p /* = NULL*/ )
  
{

  if (!_isOpen) {
    _errStr = "Cannot fStat - file not open";
    return (-1);
  }
  
  if (_isLocal) {
    return (_fStatLocal(size_p, mtime_p, atime_p, ctime_p));
  } else {
    return (_fStatRemote(size_p, mtime_p, atime_p, ctime_p));
  }

  return (0);

}

////////////////
// _fOpenLocal()
//

int DsFileIo::_fOpenLocal()

{
  
  string path;
  RapDataDir.fillPath(_url->getFile(), path);

  if ((_filep = fopen(path.c_str(), _modeStr.c_str())) == NULL) {
    _errStr = (path + ": ") + strerror(errno);
    return (-1);
  }
    
  return (0);

}

////////////////
// _fOpenRemote()
//

int DsFileIo::_fOpenRemote()

{

  // resolve the port if needed

  if (_url->getPort() < 0) {
    DsSvrMgrSocket mgrSock;
    if (mgrSock.findPortForURL(_url->getHost().c_str(), *_url,
			       -1, _errStr)) {
      cerr << "ERROR - COMM - cannot resolve port from ServerMgr" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  " << _errStr << endl;
      return (-1);
    }
  }
  
  if (ThreadSocket::open(_url->getHost().c_str(),
			 _url->getPort())) {
      
    cerr << "ERROR - COMM - DsFileIo::_fOpenRemote" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Trying to connect to server" << endl;
    cerr << "  host: " << _url->getHost() << endl;
    cerr << "  port: " << _url->getPort() << endl;
    cerr << "  " << getErrString() << endl;
    return (-1);

  }

  // assemble the message

  void *buf = _msg->assemblefOpen(_url->getURLStr(), _modeStr);

  // communicate with server, read reply

  if (_communicate(buf, _msg->lengthAssembled())) {
    _errStr = "Error communicating with server.";
    return(-1);
  }

  // check for error

  if (_msg->getFlags()) {
    _errStr = _msg->getErrorStr();
    return (-1);
  }

  return (0);
 
}

////////////////
// _fCloseLocal()
//

int DsFileIo::_fCloseLocal()

{

  if (fclose (_filep)) {
    _errStr = strerror(errno);
    _filep = NULL;
    return (-1);
  }

  _filep = NULL;
  return (0);

}

////////////////
// _fCloseRemote()
//

int DsFileIo::_fCloseRemote()

{

  // assemble the message

  void *buf = _msg->assemblefClose();

  // communicate with server, read reply

  int iret = 0;
  if (_communicate(buf, _msg->lengthAssembled())) {
    _errStr = "Error communicating with server.";
    iret = -1;
  } else if (_msg->getFlags()) {
    _errStr = _msg->getErrorStr();
    iret = -1;
  }

  Socket::close();
  return (iret);

}

/////////////////
// _fWriteLocal()
//

size_t DsFileIo::_fWriteLocal(const void *ptr,
			      const size_t size, const size_t n)

{

  return (ufwrite((void *) ptr, size, n, _filep));

}

//////////////////
// _fWriteRemote()
//

size_t DsFileIo::_fWriteRemote(const void *ptr,
			       const size_t size, const size_t n)

{

  // assemble the message

  void *buf = _msg->assemblefWrite(ptr, size, n);

  if (_communicate(buf, _msg->lengthAssembled())) {
    _errStr = "Error communicating with server.";
    return (0);
  } else if (_msg->getFlags()) {
    _errStr = _msg->getErrorStr();
  }

  return (_msg->getInfo().nelements);
  
}

/////////////////
// _fReadLocal()
//

size_t DsFileIo::_fReadLocal(void *ptr, const size_t size, const size_t n)

{

  return (ufread(ptr, size, n, _filep));

}

//////////////////
// _fReadRemote()
//

size_t DsFileIo::_fReadRemote(void *ptr, const size_t size, const size_t n)
  
{
  
  // assemble the message

  void *buf = _msg->assemblefRead(size, n);

  if (_communicate(buf, _msg->lengthAssembled())) {
    _errStr = "Error communicating with server.";
    return (0);
  } else if (_msg->getFlags()) {
    _errStr = _msg->getErrorStr();
    return (_msg->getInfo().nelements);
  }

  memcpy(ptr, _msg->getData(),
	 MAX(_msg->getInfo().nelements, (int)(size * n)));

  return (_msg->getInfo().nelements);
  
}

/////////////////
// _fPutsLocal()
//

int DsFileIo::_fPutsLocal(const char *str)

{

  if (fputs(str, _filep) == EOF) {
    _errStr = strerror(errno);
    return (EOF);
  } else {
    fflush(_filep);
    return (0);
  }

}

/////////////////
// _fPutsRemote()
//

int DsFileIo::_fPutsRemote(const char *str)

{
  
  // assemble the message

  void *buf = _msg->assemblefPuts(str);

  if (_communicate(buf, _msg->lengthAssembled())) {
    _errStr = "Error communicating with server.";
    return (EOF);
  } else if (_msg->getFlags()) {
    _errStr = _msg->getErrorStr();
    return (EOF);
  }

  return (0);
  
}

/////////////////
// _fGetsLocal()
//

char *DsFileIo::_fGetsLocal(char *str, int size)

{

  if (fgets(str, size, _filep) == NULL) {
    _errStr = strerror(errno);
    return (NULL);
  } else {
    return (str);
  }

}

/////////////////
// _fGetsRemote()
//

char *DsFileIo::_fGetsRemote(char *str, int size)

{

  // assemble the message

  void *buf = _msg->assemblefGets(size);

  if (_communicate(buf, _msg->lengthAssembled())) {
    _errStr = "Error communicating with server.";
    return (NULL);
  } else if (_msg->getFlags()) {
    _errStr = _msg->getErrorStr();
    return (NULL);
  }

  memcpy(str, _msg->getData(), MAX(_msg->getInfo().nelements, size));

  return (str);

}

/////////////////
// _fSeekLocal()
//

int DsFileIo::_fSeekLocal(const long offset, const int whence)

{

  int iret = fseek(_filep, offset, whence);

  if (iret < 0) {
    _errStr = strerror(errno);
    return (-1);
  } else {
    return (iret);
  }

}

/////////////////
// _fSeekRemote()
//

int DsFileIo::_fSeekRemote(const long offset, const int whence)

{

  // assemble the message

  void *buf = _msg->assemblefSeek(offset, whence);

  if (_communicate(buf, _msg->lengthAssembled())) {
    _errStr = "Error communicating with server.";
    return (-1);
  } else if (_msg->getFlags()) {
    _errStr = _msg->getErrorStr();
    return (-1);
  }

  return (0);

}

/////////////////
// _fTellLocal()
//

long DsFileIo::_fTellLocal()

{

  long filepos = ftell(_filep);

  if (filepos < 0) {
    _errStr = strerror(errno);
    return (-1);
  } else {
    return (filepos);
  }

}

/////////////////
// _fTellRemote()
//

long DsFileIo::_fTellRemote()

{

  // assemble the message

  void *buf = _msg->assemblefTell();

  if (_communicate(buf, _msg->lengthAssembled())) {
    _errStr = "Error communicating with server.";
    return (-1);
  } else if (_msg->getFlags()) {
    _errStr = _msg->getErrorStr();
    return (-1);
  }

  return (_msg->getInfo().filepos);
  
}

/////////////////
// _fStatLocal()
//

int DsFileIo::_fStatLocal(off_t *size_p, time_t *mtime_p,
			  time_t *atime_p, time_t *ctime_p)

{

  struct stat fileStat;

  if (fstat(fileno(_filep), &fileStat)) {
    _errStr = strerror(errno);
    return (-1);
  } else {
    *size_p = fileStat.st_size;
    if (mtime_p) {
      *mtime_p = fileStat.st_mtime;
    }
    if (atime_p) {
      *atime_p = fileStat.st_atime;
    }
    if (ctime_p) {
      *ctime_p = fileStat.st_ctime;
    }
    return (0);
  }

}

/////////////////
// _fStatRemote()
//

int DsFileIo::_fStatRemote(off_t *size_p, time_t *mtime_p,
			   time_t *atime_p, time_t *ctime_p)

{

  // assemble the message

  void *buf = _msg->assemblefStat();
  
  if (_communicate(buf, _msg->lengthAssembled())) {
    _errStr = "Error communicating with server.";
    return (-1);
  } else if (_msg->getFlags()) {
    _errStr = _msg->getErrorStr();
    return (-1);
  }
  
  *size_p = (off_t) _msg->getInfo().stat_size;
  if (mtime_p) {
    *mtime_p = (off_t) _msg->getInfo().stat_mtime;
  }
  if (atime_p) {
    *atime_p = (off_t) _msg->getInfo().stat_atime;
  }
  if (ctime_p) {
    *ctime_p = (off_t) _msg->getInfo().stat_ctime;
  }
  return (0);
  
}

/////////////////////////////////////////////
// communicate with server
//
// Returns 0 on success, -1 on error.

int DsFileIo::_communicate(void *buf,
			   const int buflen)

{

  // write the message
  
  if (writeMessage(DsFileIoMsg::DS_MESSAGE_TYPE_FILEIO,
		   buf, buflen, 10000)) {
    cerr << "ERROR - COMM - DsFileIo::_communicate" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Cannot send message to server." << endl;
    cerr << "  host: " << _url->getHost() << endl;
    cerr << "  port: " << _url->getPort() << endl;
    cerr << "  " << getErrString() << endl;
    return(-1);
  }
  
  // read the reply

  if (readMessage(30000)) {
    cerr << "ERROR - COMM - DsFileIo::_communicate" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Cannot read reply from server." << endl;
    cerr << "  host: " << _url->getHost() << endl;
    cerr << "  port: " << _url->getPort() << endl;
    cerr << "  " << getErrString() << endl;
    return(-1);
  }

  // disassemble the reply
  
  if (_msg->disassemble((void *) getData(), getNumBytes())) {
    cerr << "ERROR - DsFileIo::_communicate" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Invalid reply" << endl;
    return(-1);
  }

  return (0);

}

