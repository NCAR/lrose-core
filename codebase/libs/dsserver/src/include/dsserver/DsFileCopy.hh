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
// dsserver/DsFileCopy.hh
//
// DsFileCopy class
//
// This class handles the FileCopy operations to/from 
// the DsFileCopyServer.
//
////////////////////////////////////////////////////////////////

#ifndef DsFileCopy_HH
#define DsFileCopy_HH

#include <string>
#include <cstdio>
#include <toolsa/URL.hh>
#include <toolsa/compress.h>
#include <didss/DsMessage.hh>
#include <toolsa/ThreadSocket.hh>
#include <dsserver/DsFileCopyMsg.hh>
using namespace std;

///////////////////////////////////////////////////////////////
// class definition

class DsFileCopy

{

public:
  
  // constructor

  DsFileCopy(const DsMessage::memModel_t mem_model = DsMessage::PointToMem);

  // destructor
  
  virtual ~DsFileCopy();

  // debugging

  void setDebug(bool debug = true) { _debug = debug; }

  // set max file age (secs)

  void setMaxFileAge(int age) { _maxFileAge = age; }

  // set timeout for socket open calls, in msecs
  // default is blocking open

  void setOpenTimeoutMsecs(int msecs) { _openTimeoutMsecs = msecs; }

  ///////////////////////////////////////
  // enquireForPut()
  //
  // Enquire whether to put the named file.
  //
  // check DoPut() to decide whether to put or not.
  //
  // If force_copy is true, copy will be forced even if file exists
  // on target machine.
  // The file age on target machine will be checked.
  // If the age exceeds overwrite_age, file will be copied over existing
  // file. If overwrite_age is -1, file will never be overwritten.
  //
  // Returns 0 on success, -1 on error
  // Error message available via getErrorStr()
  
  int enquireForPut(const string &data_dir,
		    const DsURL &dest_url,
		    const LdataInfo &ldata_info,
		    const string &file_name,
		    const int overwrite_age = -1);
  
  ///////////////////////////////////////
  // putAfterEnquire()
  //
  // Put the file, using compression method specified.
  //
  // Returns 0 on success, -1 on error
  // Error message available via getErrorStr()
  
  int putAfterEnquire(ta_compression_method_t
		      compression_type = TA_COMPRESSION_NONE,
		      bool remove_after_copy = false);
  
  ///////////////////////////////////////
  // putForced()
  //
  // Put file without enquiring.
  //
  // Returns 0 on success, -1 on error
  // Error message available via getErrorStr()
  
  int putForced(const string &data_dir,
		const DsURL &dest_url,
		const LdataInfo &ldata_info,
		const string &file_name,
		ta_compression_method_t
		compression_type = TA_COMPRESSION_NONE,
		bool remove_after_copy = false);
  

  ///////////////////////////////////////
  // sendBufferToSocket()
  //
  // Sends the buffer to the server directly.
  //
  // Returns 0 on success, -1 on error
  // Error message available via getErrorStr()
  //
  // Added by Niles Oien October 2004

  int sendBufferToSocket(DsURL destUrl,
			 void *buf, 
			 int bufLen);


  // get error string
  
  const char *getErrorStr() const { return (_errStr.c_str()); }
  const char *getErrStr() const { return (_errStr.c_str()); }
  
  // member access

  bool DoPut() const { return _doPut; }
  const string &getFilePath() { return _filePath; }
  int getFileLen() { return _fileLen; }
  
protected:

private:

  bool _doPut;
  bool _debug;
  
  string _errStr;
  int _openTimeoutMsecs;
  
  DsURL _destUrl;
  DsFileCopyMsg _msg;

  string _dataDir;
  string _filePath;
  int _fileLen;
  int _maxFileAge;
  void *_fileBuf;

  int _communicate(void *buf, const int buflen);
  int _enquireCommunicate(void *buf, const int buflen);

};

#endif


