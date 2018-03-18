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
 * dsserver/DsFileIoMsg.hh
 *
 * DsMessage class for FileServer
 ******************************************************************/

#ifndef DsFileIoMsg_HH
#define DsFileIoMsg_HH

#include <dsserver/DsServerMsg.hh>
#include <iostream>
#include <sys/stat.h>
using namespace std;

////////////////////////////////////////////////////////////////////a
// Messages to and from the fileio server make use the generic
// ds_message format - see ds_message.h
//
// Messages have a header containing a type and subtype,
// and optinally a body comprising a number of parts.
// The header contains the number of parts, which is 0 for
// a message with no body.
//

///////////////////////////////////////////////////////////////////
// message type definitions
//

//////////
// fOpen()
//
// Type: DS_MESSAGE_TYPE_FILEIO
// SubType: DS_FILEIO_FOPEN
//
// The message body has the following parts:
//   DS_FILEIO_URL_PART
//   DS_FILEIO_MODE_PART.
//
// A DS_FILEIO_RETURN message is expected as a reply.
// Mode is set to DS_FILEIO_FOPEN.
//

///////////
// fClose()
//
// Type: DS_MESSAGE_TYPE_FILEIO
// SubType: DS_FILIO_FCLOSE
//
// The message has no body.
//
// A DS_FILEIO_RETURN message is expected as a reply.
// Mode is set to DS_FILEIO_FCLOSE.
//

///////////
// fWrite()
//
// Type: DS_MESSAGE_TYPE_FILEIO
// SubType: DS_FILEIO_FWRITE
//
// The message body has the following parts:
//   DS_FILEIO_DATA_PART - data to be written.
//   DS_FILEIO_INFO_PART - buffer size information.
//
// A DS_FILEIO_RETURN message is expected as a reply.
// Mode is set to DS_FILEIO_FWRITE.
//

//////////
// fRead()
//
// Type: DS_MESSAGE_TYPE_FILEIO
// SubType: DS_FILEIO_FREAD
//
// The message body has the following parts:
//   DS_FILEIO_INFO_PART - buffer size information
//
// A DS_FILEIO_RETURN message is expected as a reply.
// Mode is set to DS_FILEIO_FREAD.
// On success, the reply has one DS_FILEIO_DATA_PART containing
// the data which was read.
//

//
// fPuts()
//
// Type: DS_MESSAGE_TYPE_FILEIO
// SubType: DS_FILEIO_FPUTS
//
// The message body has the following parts:
//   DS_FILEIO_DATA_PART - data string to be written - null terminated.
//
// A DS_FILEIO_RETURN message is expected as a reply.
// Mode is set to DS_FILEIO_FPUTS.
//

//
// fGets()
//
// Type: DS_MESSAGE_TYPE_FILEIO
// SubType: DS_FILEIO_FGETS
//
// The message body has the following parts:
//   DS_FILEIO_INFO_PART - buffer size information
//
// A DS_FILEIO_RETURN message is expected as a reply.
// Mode is set to DS_FILEIO_FGETS.
// On success, the reply has one DS_FILEIO_DATA_PART containing
// the data which was read.
//

//
// fSeek()
//
// Type: DS_MESSAGE_TYPE_FILEIO
// SubType: DS_FILEIO_FSEEK
//
// The message body has the following parts:
//   DS_FILEIO_INFO_PART - position information.
//
// A DS_FILEIO_RETURN message is expected as a reply.
// Mode is set to DS_FILEIO_FSEEK.
//

//
// fTell()
//
// Type: DS_MESSAGE_TYPE_FILEIO
// SubType: DS_FILEIO_FTELL
//
// The message has no body.
//
// A DS_FILEIO_RETURN message is expected as a reply.
// Mode is set to DS_FILEIO_FTELL.
//

//
// fStat()
//
// Type: DS_MESSAGE_TYPE_FILEIO
// SubType: DS_FILEIO_FSTAT
//
// The message has no body.
//
// A DS_FILEIO_RETURN message is expected as a reply.
// Mode is set to DS_FILEIO_FTELL.
//

//
// Return
//
// Type: DS_MESSAGE_TYPE_FILEIO
// SubType: DS_FILEIO_RETURN
// Mode: set to subType of original message
// Flags: 0 on success, -1 on error.
//
// The return message has a DS_FILEIO_INFO_PART containing the
// return code and/or other information.
//
// If an error occurred, the return message will contain a
// DS_FILEIO_STRING_PART containing the error message.
//
// For get messages a DS_FILEIO_DATA_PART contains the data
// buffer holding the message which was read.

///////////////////////////////////////////////////////////////////
// class definition

class DsFileIoMsg : public DsServerMsg

{

public:
  
  //
  // message type definition
  //

  typedef enum {
    DS_MESSAGE_TYPE_FILEIO = 6464000
  } type_enum_t;

  //
  // message sub-type definitions
  //

  typedef enum {
    DS_FILEIO_FOPEN = 6464200,
    DS_FILEIO_FCLOSE = 6464201,
    DS_FILEIO_FREAD = 6464202,
    DS_FILEIO_FWRITE = 6464203,
    DS_FILEIO_FPUTS = 6464204,
    DS_FILEIO_FGETS = 6464205,
    DS_FILEIO_FSEEK = 6464206,
    DS_FILEIO_FTELL = 6464207,
    DS_FILEIO_FSTAT = 6464208,
    DS_FILEIO_RETURN = 6464209
  } subtype_enum_t;

  //
  // message flags definitions
  //

  typedef enum {
    DS_FILEIO_RETURN_OK = 6464302,
    DS_FILEIO_RETURN_ERROR = 6464303
  } flags_enum_t;

  //
  // part definitions
  //
  
  typedef enum {
    DS_FILEIO_URL_PART = 6464400,
    DS_FILEIO_MODE_PART = 6464401,
    DS_FILEIO_DATA_PART = 6464402,
    DS_FILEIO_INFO_PART = 6464403,
    DS_FILEIO_ERRORSTR_PART = 6464404
  } part_enum_t;

  //
  // file seek flags
  //

  typedef enum {
    DS_FILEIO_SEEK_SET = 6464500,
    DS_FILEIO_SEEK_END = 6464501,
    DS_FILEIO_SEEK_CUR = 6464502
  } seek_enum_t;

  ///////////////////
  // file info struct
  
  typedef struct {
    si32 size;
    si32 nelements;
    si32 offset;
    si32 whence;
    si32 filepos;
    si32 stat_size;
    ti32 stat_atime;
    ti32 stat_mtime;
    ti32 stat_ctime;
    si32 spare[7];
  } info_t;
  
  // constructor

  DsFileIoMsg(memModel_t mem_model = CopyMem);

  // destructor
  
  virtual ~DsFileIoMsg();

  // assemble a DS_FILEIO_FOPEN message
  
  void *assemblefOpen(const string &url_str,
		      const string &mode_str);
  
  // assemble fOpen return

  void *assemblefOpenReturn(const bool errorOccurred = false,
			    const char *errorStr = NULL);

  // assemble a DS_FILIO_FCLOSE message

  void *assemblefClose();
  
  // assemble fClose return

  void *assemblefCloseReturn(const bool errorOccurred = false,
			     const char *errorStr = NULL);
  
  // assemble a DS_FILEIO_FWRITE message
  
  void *assemblefWrite(const void *ptr,
		       const size_t size, const size_t n);
  
  // assemble fWrite return

  void *assemblefWriteReturn(const int nwritten,
			     const bool errorOccurred = false,
			     const char *errorStr = NULL);

  // assemble a DS_FILEIO_FREAD message
  
  void *assemblefRead(const size_t size,
		      const size_t n);
  
  // assemble fRead return

  void *assemblefReadReturn(const int nread,
			    const void *data = NULL,
			    const bool errorOccurred = false,
			    const char *errorStr = NULL);
  
  // assemble a DS_FILEIO_FPUTS message
  
  void *assemblefPuts(const char *str);
  
  // assemble fPuts return

  void *assemblefPutsReturn(const bool errorOccurred = false,
			    const char *errorStr = NULL);
  
  // assemble a DS_FILEIO_FGETS message
  
  void *assemblefGets(const int size);
  
  // assemble fGets return

  void *assemblefGetsReturn(const char *str = NULL,
			    const bool errorOccurred = false,
			    const char *errorStr = NULL);
  
  // assemble a DS_FILEIO_FSEEK message
  
  void *assemblefSeek(const long offset, const int whence);
  
  // assemble fSeek return

  void *assemblefSeekReturn(const bool errorOccurred = false,
			    const char *errorStr = NULL);

  // assemble a DS_FILEIO_FTELL message
  
  void *assemblefTell();
  
  // assemble fTell return

  void *assemblefTellReturn(const long filepos,
			    const bool errorOccurred = false,
			    const char *errorStr = NULL);
  
  // assemble a DS_FILEIO_FSTAT message
  
  void *assemblefStat();
  
  // assemble fStat return

  void *assemblefStatReturn(const off_t stat_size,
			    const time_t stat_atime,
			    const time_t stat_mtime,
			    const time_t stat_ctime,
			    const bool errorOccurred = false,
			    const char *errorStr = NULL);

  // generic return message

  void *assembleReturn(const int requestSubType,
		       const bool errorOccurred = false,
		       const char *errorStr = NULL,
		       const void *data = NULL,
		       const int dataLen = 0);

  // overload disassemble
  
  int disassemble(void *in_msg, const int msg_len);

  // print

  virtual void print(ostream &out, const char *spacer = "") const;

  // access to parts

  string &getUrlStr() { return (_urlStr); }
  string &getModeStr() { return (_modeStr); }
  string &getErrorStr() { return (_errStr); }
  info_t &getInfo() { return (_info); }
  void *getData() { return (_data); }

protected:

private:

  string _urlStr;
  string _modeStr;
  string _errStr;
  void *_data;
  info_t _info;

  void _clearInfo();
  void _BEfromInfo();
  void _BEtoInfo();
  void _BEfromFStat();
  void _BEtoFStat();

};

#endif


