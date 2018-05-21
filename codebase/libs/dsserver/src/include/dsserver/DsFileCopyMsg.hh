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
 * dsserver/DsFileCopyMsg.hh
 *
 * DsMessage class for FileServer
 ******************************************************************/

#ifndef DsFileCopyMsg_HH
#define DsFileCopyMsg_HH

#include <iostream>
#include <dsserver/DsServerMsg.hh>
#include <didss/LdataInfo.hh>
using namespace std;

////////////////////////////////////////////////////////////////////a
// Messages to and from the FileCopy server make use the generic
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

////////////////////
// enquire_for_put
//
// Enquire whether to put named file in url dir 
//
// Type: DS_MESSAGE_TYPE_FILECOPY
// SubType: DS_FILECOPY_ENQUIRE_FOR_PUT
// Mode: set to DS_FILECOPY_NO_FORCE to allow server to arbitrate
//       on whether copy should proceed
//
// The message body has the following parts:
//   DS_FILECOPY_URL_PART
//   DS_FILECOPY_FILENAME_PART
//   DS_FILECOPY_FILEINFO_PART
//   DS_FILECOPY_LDATA_XML_PART
//   DS_FILECOPY_LDATAINFO_PART (deprecated)
//   DS_FILECOPY_LDATAFCASTS_PART (deprecated)
//
// A DS_FILECOPY_RETURN message is expected as a reply.
// Mode: set to DS_FILECOPY_ENQUIRE
// Flags: set as follows:
//    DS_FILECOPY_DO_PUT - proceed with copy
//    DS_FILECOPY_NO_PUT - do not proceed with copy
// Error: set to -1 if an error occurred, which usually means
//   that we cannot create the directory required.
//   DS_ERRORSTR_PART contains error message.

/////////////////////////
// enquire_for_put return
//
// Type: DS_MESSAGE_TYPE_FILECOPY
// SubType: DS_FILECOPY_RETURN
// Mode: DS_FILECOPY_ENQUIRE_FOR_PUT
// Flags: DS_FILECOPY_YES_PUT or DS_FILECOPY_NO_PUT
// Error: 0 on success, -1 on error.
//
// If no error occurred, the message has no body.
//
// If an error occurred, the return message will contain a
// DS_FILECOPY_ERRORSTR_PART containing the error message.
//

//////////////////////
// put_after_enquire
//
// Type: DS_MESSAGE_TYPE_FILECOPY
// SubType: DS_FILECOPY_PUT_AFTER_ENQUIRE
//
// The message body has the following parts:
//   DS_FILECOPY_FILEBUF_PART
//

///////////////////////////
// put_after_enquire return
//
// Type: DS_MESSAGE_TYPE_FILECOPY
// SubType: DS_FILECOPY_RETURN
// Mode: DS_FILECOPY_PUT_AFTER_ENQUIRE
//
// Error: 0 on success, -1 on error.
//
// If no error occurred, the message has no body.
//
// If an error occurred, the return message will contain a
// DS_FILECOPY_ERRORSTR_PART containing the error message.
//

////////////////////
// put_forced
//
// Put file of given name in url dir
//
// Type: DS_MESSAGE_TYPE_FILECOPY
// SubType: DS_FILECOPY_PUT_FORCED
//
// The message body has the following parts:
//   DS_FILECOPY_URL_PART
//   DS_FILECOPY_FILENAME_PART
//   DS_FILECOPY_FILEINFO_PART
//   DS_FILECOPY_FILEBUF_PART
//   DS_FILECOPY_LDATA_XML_PART
//   DS_FILECOPY_LDATAINFO_PART (deprecated)
//   DS_FILECOPY_LDATAFCASTS_PART (deprecated)
//
// A DS_FILECOPY_RETURN message is expected as a reply.
// Mode: set to DS_FILECOPY_PUT_FORCED
// Error: set to -1 if an error occurred, which usually means
//   that we cannot create the directory required.
//   DS_ERRORSTR_PART contains error message.

///////////////////////////
// put_forced return
//
// Type: DS_MESSAGE_TYPE_FILECOPY
// SubType: DS_FILECOPY_RETURN
// Mode: DS_FILECOPY_PUT_FORCED
//
// Error: 0 on success, -1 on error.
//
// If no error occurred, the message has no body.
//
// If an error occurred, the return message will contain a
// DS_FILECOPY_ERRORSTR_PART containing the error message.
//

///////////////////////////////////////////////////////////////////
// class definition

class DsFileCopyMsg : public DsServerMsg

{

public:
  
  //
  // message type definition
  //

  typedef enum {
    DS_MESSAGE_TYPE_FILECOPY = 7575000
  } type_enum_t;

  //
  // message sub-type definitions
  //

  typedef enum {
    DS_FILECOPY_ENQUIRE_RETURN = 7575100, // deprecated
    DS_FILECOPY_ENQUIRE_FOR_PUT = 7575101,
    DS_FILECOPY_ENQUIRE_BY_TIME = 7575102, // deprecated
    DS_FILECOPY_PUT_AFTER_ENQUIRE = 7575103,
    DS_FILECOPY_RETURN = 7575104,
    DS_FILECOPY_PUT_BY_TIME = 7575105, // deprecated
    DS_FILECOPY_PUT_FORCED = 7575106
  } subtype_enum_t;

  //
  // part definitions
  //
  
  typedef enum {
    DS_FILECOPY_URL_PART = 7575200,
    DS_FILECOPY_FILENAME_PART = 7575201,
    DS_FILECOPY_LDATAINFO_PART = 7575202, // deprecated
    DS_FILECOPY_LDATAFCASTS_PART = 7575203, // deprecated
    DS_FILECOPY_FILEINFO_PART = 7575204,
    DS_FILECOPY_FILEBUF_PART = 7575205,
    DS_FILECOPY_ERRORSTR_PART = 7575206,
    DS_FILECOPY_LDATA_XML_PART = 7575207
  } part_enum_t;

  //
  // copy flag definitions
  //

  typedef enum {
    DS_FILECOPY_YES_PUT = 7575400,
    DS_FILECOPY_NO_PUT = 7575401
  } flags_enum_t;

  ///////////////////
  // file info struct
  
  typedef struct {
    si32 size;
    ti32 mod_time;
    ti32 data_time;
    si32 overwrite_age;
    si32 spare[4];
  } file_info_t;
  
  // constructor

  DsFileCopyMsg(const memModel_t mem_model = PointToMem);

  // destructor
  
  virtual ~DsFileCopyMsg();

  // assemble a DS_FILECOPY_ENQUIRE_FOR_PUT message
  
  void *assembleEnquireForPut(const DsURL &dir_url,
			      const LdataInfo &ldata_info,
			      const string &file_name,
			      const time_t mod_time,
			      const int file_size,
			      const int overwrite_age = -1);
  
  // assemble enquire for put return
  
  void *assembleEnquireForPutReturn(const bool doPut,
				    const bool errorOccurred = false,
				    const char *errorStr = NULL);
  
  // assemble a DS_FILECOPY_PUT_AFTER_ENQUIRE message
  
  void *assemblePutAfterEnquire(const void *fileBuf,
				const int bufLen);
  
  // assemble a DS_FILECOPY_PUT_AFTER_ENQUIRE return message
  
  void *assemblePutAfterEnquireReturn(const bool errorOccurred = false,
				      const char *errorStr = NULL);
  
  // assemble a DS_FILECOPY_PUT_FORCED message
  
  void *assemblePutForced(const DsURL &dir_url,
			  const LdataInfo &ldata_info,
			  const string &file_name,
			  const time_t mod_time,
			  const int file_size,
			  const void *fileBuf,
			  const int bufLen);
  
  // assemble put forced return
  
  void *assemblePutForcedReturn(const bool errorOccurred = false,
				const char *errorStr = NULL);

  // overload disassemble
  
  int disassemble(void *inMsg, const int msgLen);

  // print

  virtual void print(ostream &out, const char *spacer = "") const;

  // access to parts
  
  const DsURL &getDataUrl() const { return (_dataUrl); }
  const string &getFileName() const { return (_fileName); }
  const string &getErrorStr() const { return (_errStr); }
  const file_info_t &getFileInfo() const { return (_fileInfo); }
  const LdataInfo &getLdataInfo() const { return (_ldataInfo); }
  const void *getFileBuf() const { return (_fileBuf); }
  int getFileLen() const { return (_fileLen); }
  bool DoPut() const { return (_doPut); }

protected:

private:

  bool _doPut;
  bool _forcePut;
  string _fileName;
  string _errStr;
  DsURL _dataUrl;
  file_info_t _fileInfo;
  LdataInfo _ldataInfo;
  void *_fileBuf;
  int _fileLen;

  void _clearFileInfo(file_info_t &info);
  void _BEfromFileInfo(file_info_t &info);
  void _BEtoFileInfo(file_info_t &info);

  void _intErr(const char *err_str, const int iarg);
  void _strErr(const char *err_str, const string &sarg);

  bool ForcePut() const { return (_forcePut); } // deprecated

};

#endif
