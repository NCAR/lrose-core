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

///////////////////////////////////////////
// <dsserver/DsTitanMsg.hh>
//
// DsMessage class for DsTitan class
///////////////////////////////////////////

#ifndef DsTitanMsg_hh
#define DsTitanMsg_hh


#include <dsserver/DsServerMsg.hh>
#include <titan/DsTitan.hh>
using namespace std;

class DsTitanMsg : public DsServerMsg {
  
public:
  
  typedef enum {
    DS_MESSAGE_TYPE_TITAN    = 130010
  } type_enum_t;
  
  typedef enum {
    DS_TITAN_READ_REQUEST     = 131010,
    DS_TITAN_READ_REPLY       = 131020
  } subtype_enum_t;

  typedef enum {
    DS_TITAN_READ_URL_PART              = DsServerMsg::DS_URL,          // 1
    DS_TITAN_ERR_STRING_PART            = DsServerMsg::DS_ERR_STRING,   // 8
    DS_TITAN_READ_REQUEST_PART          = 133010,
    DS_TITAN_READ_REPLY_PART            = 133020,
    DS_TITAN_STORM_PARAMS_PART          = 133030,
    DS_TITAN_TRACK_PARAMS_PART          = 133040,
    DS_TITAN_DIR_IN_USE_PART            = 133050,
    DS_TITAN_STORM_PATH_IN_USE_PART     = 133060,
    DS_TITAN_TRACK_PATH_IN_USE_PART     = 133070,
    DS_TITAN_COMPLEX_TRACK_PART         = 133080,
    DS_TITAN_CURRENT_ENTRIES_PART       = 133090
  } part_enum_t;

  ////////////////////////////
  // titan read request struct

  typedef struct {
    si32 readTimeMode;
    si32 trackSet;
    si32 requestTime;
    si32 readTimeMargin;
    si32 requestComplexNum;
    si32 readLprops;
    si32 readDbzHist;
    si32 readRuns;
    si32 readProjRuns;
    si32 startTime;
    si32 endTime;
    si32 readCompressed;
    si32 spare[4];
  } read_request_t;
   
  //////////////////////////
  // titan read reply struct

  typedef struct {
    si32 nComplexTracks;
    si32 nCurrentEntries;
    si32 scanInUse;
    si32 timeInUse;
    si32 dataStartTime;
    si32 dataEndTime;
    si32 idayInUse;
    si32 spare[9];
  } read_reply_t;
   
  // constructor

  DsTitanMsg();
  
  // destructor
  
  virtual ~DsTitanMsg();

  // Assemble read request message.
  // Returns assembled message on success, NULL on error.
  // getErrorStr() returns the error string.
  
  void *assembleReadRequest(const string &url,
			    const DsTitan &tserver);
  
  // assemble successful read reply message 
  // Returns assembled message.

  void *assembleReadReplySuccess(const DsTitan &tserver);
  
  // assemble an error-on-read reply message
  
  void *assembleReadReplyError(const DsTitan &tserver);
  
  // override the disassemble function
  // returns 0 on success, -1 on error

  int disassemble(const void *in_msg, const int msg_len,
		  DsTitan &tserver);

  // get error string

  string &getErrStr() { return (_errStr); }

protected:

  string _errStr;

private:

  // Private members with no bodies provided -- do not use until defined.
  DsTitanMsg(const DsTitanMsg & orig);
  void operator= (const DsTitanMsg & other);

};

#endif
