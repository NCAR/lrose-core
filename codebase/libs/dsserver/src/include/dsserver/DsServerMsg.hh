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

//////////////////////////////////////////////////////////
// DsServerMsg.hh
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
//
// Jan 1999
//
////////////////////////////////////////////////////////////

#ifndef _DS_SERVER_MSG_INC_
#define _DS_SERVER_MSG_INC_

#include <string>
#include <didss/DsMessage.hh>
#include <didss/DsURL.hh>
using namespace std;

#define DS_DEFAULT_PING_TIMEOUT_MSECS 10000
#define DS_DEFAULT_COMM_TIMEOUT_MSECS 30000

//////////////////////////////
// forward class declarations
//

class DsServerMsg : public DsMessage

{

public:

  // part types

  enum partType {
      DS_URL = 1,
      DS_VERSION = 2,
      DS_SERIAL = 4,
      DS_ERR_STRING = 8,
      DS_STRING = 16,
      DS_INT = 32,
      DS_CLIENT_USER = 64,
      DS_CLIENT_HOST = 128,
      DS_CLIENT_IPADDR = 256
  };

  // Categories as follows:
  //
  // ServerStatus: requesting server status
  // StartPut: start of a series of messages for a put
  // StartGet: start of a series of messages for a get
  // EndSeries: end of a series of messages
  // LastCategory: not used in messages,
  //               only in checking for valid categories
  //
  // Be sure to update printHeader() when adding categories!

  typedef enum { Generic = 8389420,
		 ServerStatus,
		 StartPut,
		 StartGet,
		 EndSeries,
		 LastCategory
  } category_t;

  // Reserved error codes.
  //   Maskable. Use Top order bits only to avoid conflicts.
  // 
  //   Only DsServer and DsServerMgr errors are reserved. 
  //     Other errors should be defined in client/server protocols.
  // 
  //   Integers smaller than 16^4 (65,536) are not reserved.
  //
  //   Be sure to update printHeader() when adding errors!
  // 
  enum msgErr {
      // DsServer and all subclasses.
      BAD_MESSAGE         = 0x10000000, // 16^7 = 268,435,456
      SERVER_ERROR        = 0x20000000,
      UNKNOWN_COMMAND     = 0x40000000,
      NOT_SUPPORTED       = 0x80000000,

      // DsServerMgr Errors.
      BAD_URL_PROVIDED    = 0x01000000, // 16^6 = 16,777,216
      BAD_HOST_PROVIDED   = 0x02000000, // Unexpected but recoverable.
      BAD_PORT_PROVIDED   = 0x04000000, // Unexpected but recoverable.
      SERVICE_DENIED      = 0x08000000,
      NO_SERVICE_AVAIL    = 0x00100000,
      DSS_MSG_NEXT        = 0x00200000,
      DSS_MSG_LAST        = 0x00080000,
      DSS_MSG_SUCCESS     = 0
  };

  // Reserved server commands -- used to get info about the server itself.
  // 
  // These are sent in the type field of the message,
  //   when the message has a category equal to DsServerMsg::ServerStatus.
  // 
  // All clients can use these commands with the DsServerMgr.
  //   Other commands should be defined as part of the client/server
  //   protocol.
  //  
  enum svrCmd {
    // DsServer commands.
    IS_ALIVE,              // Returns empty message.
    GET_NUM_CLIENTS,       // Returns integer.
    SHUTDOWN,              // Returns empty message, then calls exit(0).

    // DsServerMgr commands.
    GET_NUM_SERVERS,       // Returns integer.
    GET_SERVER_INFO,       // Returns int and formatted string, list of servers.
    GET_FAILURE_INFO,      // Returns int and formatted string, failure list.
    GET_DENIED_SERVICES    // Returns int and formatted string, executable list.
  };

  //////////////
  // constructor
  //
  // Memory model defaults to local copy

  DsServerMsg(memModel_t mem_model = CopyMem );

  // copy constructor
  
  DsServerMsg(const DsServerMsg &rhs);

  /////////////
  // destructor

  virtual ~DsServerMsg();

  // assignment
  
  DsServerMsg & operator=(const DsServerMsg &rhs);

  //////////////////////////                                                    
  // decode a message header
  //
  // This is used if you just want to peek at the header before       
  // deciding how to handle the message.
  //
  // If msg_len is set, checks that the msg is big enough
  //   to hold at least a DsMsgHdr_t. Otherwise, assumes            
  //   that the message is big enough and blindly copies            
  //   memory.
  //
  // Subclasses *must* call this version if they define
  //   this method and want checking.
  //                                        
  // Returns: -1 If msg_len is set and is smaller than a DsMsgHdr_t.
  //               or if the category of the message doesn't match
  //               one of the valid categories.
  //           0 Otherwise.          
    
  virtual int decodeHeader(const void *in_msg, ssize_t msg_len = -1);

  // get category
  // 
  category_t getMessageCat() const { return ((category_t) _category); }

  // get error
  msgErr getMessageErr()     const { return ((msgErr) _error); }

  string getFirstURLStr() const;
  DsURL * getFirstURL() const;
  int addURL(const string & url_str);
  int addURL(const DsURL & url);

  // add URL, stripping off any forwarding info
  int addURLNoFwd(const DsURL & url);

  string getFirstErrString() const;
  int addErrString(const string & err);

  string getFirstString() const;
  int addString(const string & str);

  int getFirstInt() const;
  int addInt(const int & integer);

  int addClientUser();
  string getClientUser() const;

  int addClientHost();
  string getClientHost() const;

  int addClientIpaddr();
  string getClientIpaddr() const;

  // set the message header attributes
  //
  // These overwrite the existing attributes.
  // 
  void setHdrAttr(const int type,
		  const int sub_type = -1,
		  const int mode = -1,
		  const int flags = 0,
		  const int major_version = 1,
		  const int minor_version = 0,
		  const int serial_num = -1,
		  const category_t category = Generic);

  void setCategory(const category_t category) { _category = category; }
  void setErr(const msgErr error)             { _error = (int) error; }
  
  //
  // Set everything to initial state
  //
  virtual void clearAll();

  //////////////////////////////////////////
  // print out main header and parts headers

  virtual void print(ostream &out, const char *spacer = "") const;

  // print out the message header
  // 
  virtual void printHeader(ostream &out, const char *spacer = "") const;
  virtual void printHeader(ostream *out, const char *spacer = "") const;

  // check that a url is secure
  //
  // This checks for absolute paths and .. relative paths in a
  // url. If these are present, the url may point to data which
  // is not below DATA_DIR, making the request insecure.
  //
  // returns true if secure, false if not.
  // Appends error information to errorStr.

  static bool urlIsSecure(const string &urlStr,
			  string &errorStr);
    
protected:

  DsServerMsg &_copy(const DsServerMsg &rhs);

private:

};

#endif
