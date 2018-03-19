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

////////////////////////////////////////////////////////////////
// dsserver/DsFileIo.hh
//
// DsFileIo class
//
// This class handles the FileIo operations to both the local
// disk and via the DsFileIoServer.
//
////////////////////////////////////////////////////////////////

#ifndef DsFileIo_HH
#define DsFileIo_HH

#include <string>
#include <cstdio>
#include <toolsa/ThreadSocket.hh>
#include <didss/DsMessage.hh>
using namespace std;

class DsURL;
class DsFileIoMsg;

///////////////////////////////////////////////////////////////
// class definition

class DsFileIo : public ThreadSocket

{

public:
  
  // constructor

  DsFileIo(DsMessage::memModel_t mem_model = DsMessage::CopyMem);

  // destructor
  
  virtual ~DsFileIo();

  ///////////////////////////////////////
  // fopen()
  // Returns 0 on success, -1 on error
  //
  // Error message available via getErrorStr()
  
  int fOpen(const char *url_str, const char *mode_str,
	    bool force_local = false);

  ////////////////////////////////////////
  // fclose()
  // Returns 0 on success, -1 on error
  //
  // Error message available via getErrorStr()

  int fClose();

  ////////////////////////////////////////
  // fwrite()
  //
  // Returns number of elements written
  //
  // Error message available via getErrorStr()

  size_t fWrite(const void *ptr,
		const size_t size, const size_t n);

  ////////////////////////////////////////
  // fread()
  //
  // Loads ptr - memory allocated by caller.
  // Returns number of elements read.
  //
  // Error message available via getErrorStr()

  size_t fRead(void *ptr, const size_t size, const size_t n);

  //////////////////////////////////////////
  // fprintf()
  //
  // String printed must not exceed 8K chars in length.
  //
  // Returns number of items printed
  //
  // Error message available via getErrorStr()

  int fPrintf(const char *format, ... );

  //////////////////////////////////////////
  // fputs()
  //
  // Returns 0 on success, EOF on error
  //
  // Error message available via getErrorStr()

  int fPuts(const char *str);

  //////////////////////////////////////////
  // fgets()
  //
  // Returns pointer to string read, NULL on error.
  //
  // Error message available via getErrorStr()

  char *fGets(char *str, int size);

  //////////////////////////////////////////
  // fseek()
  //
  // Returns 0 on success, -1 on error
  //
  // Error message available via getErrorStr()

  int fSeek(const long offset, const int whence);

  //////////////////////////////////////////
  // ftell()
  //
  // Returns file pos on success, -1 on error
  //
  // Error message available via getErrorStr()

  long fTell();

  //////////////////////////////////////////
  // fStat()
  //
  // Fills out size and times vals.
  // Returns 0 on success, -1 on error.
  //
  // Error message available via getErrorStr()

  int fStat(off_t *size_p, time_t *mtime_p = NULL,
	    time_t *atime_p = NULL, time_t *ctime_p = NULL);
  
  // get error string
  
  const char *getErrorStr() { return (_errStr.c_str()); }
  
protected:

private:
  
  FILE *_filep;
  // bool _errorOccurred;
  string _errStr;
  string _urlStr;
  string _modeStr;
  DsURL *_url;
  DsFileIoMsg *_msg;
  bool _isOpen;
  bool _isLocal;

  int _fOpenLocal();
  int _fOpenRemote();

  int _fCloseLocal();
  int _fCloseRemote();

  size_t _fWriteLocal(const void *ptr,
		      const size_t size, const size_t n);

  size_t _fWriteRemote(const void *ptr,
		       const size_t size, const size_t n);

  size_t _fReadLocal(void *ptr, const size_t size, const size_t n);
  size_t _fReadRemote(void *ptr, const size_t size, const size_t n);

  int _fPutsLocal(const char *str);
  int _fPutsRemote(const char *str);

  char *_fGetsLocal(char *str, int size);
  char *_fGetsRemote(char *str, int size);

  int _fSeekLocal(const long offset, const int whence);
  int _fSeekRemote(const long offset, const int whence);

  long _fTellLocal();
  long _fTellRemote();

  int _fStatLocal(off_t *stat_size, time_t *stat_atime,
		  time_t *stat_mtime, time_t *stat_ctime);
  int _fStatRemote(off_t *stat_size, time_t *stat_atime,
		   time_t *stat_mtime, time_t *stat_ctime);

  int _communicate(void *buf, const int buflen);

};

#endif


