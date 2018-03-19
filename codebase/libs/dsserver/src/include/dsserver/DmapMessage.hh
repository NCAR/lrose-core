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
 * dsserver/DmapMessage.hh
 *
 * DsMessage class for DataMapper
 ******************************************************************/

#ifndef DmapMessage_HH
#define DmapMessage_HH

#include <dataport/port_types.h>
#include <toolsa/umisc.h>
#include <dsserver/DsServerMsg.hh>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

/*
 * data set info struct
 */

#define DMAP_HOSTNAME_LEN 128
#define DMAP_IPADDR_LEN   64
#define DMAP_DATATYPE_LEN 32
#define DMAP_DIR_LEN 256
#define DMAP_STATUS_LEN 128
#define DMAP_SPARE_LEN 128
#define DMAP_INFO_NBYTES_32 32

typedef struct {

  ti32 start_time;    /* absolute start time */
  ti32 end_time;      /* absolute end time */
  ti32 latest_time;   /* time of latest entry added */
  ti32 last_reg_time; /* time of last registration by scout or client */
  fl32 nfiles;        /* total number of files */
  fl32 total_bytes;   /* total number of bytes */
  si32 forecast_lead_time;  /* lead time of forecast data - secs
			     * set to -1 if not used */
  ti32 check_time;    /* time at which the data mapper was checked */
  ui32 uspare[24];

  char hostname[DMAP_HOSTNAME_LEN];
  char ipaddr[DMAP_IPADDR_LEN];
  char datatype[DMAP_DATATYPE_LEN];
  char dir[DMAP_DIR_LEN];
  char status[DMAP_STATUS_LEN];
  char cspare[DMAP_SPARE_LEN];
  
} DMAP_info_t;

/*
 * Messages to and from the data mapper make use the generic
 * ds_message format - see ds_message.h
 *
 * Messages have a header containing a type and subtype,
 * and optinally a body comprising a number of parts.
 * The header contains the number of parts, which is 0 for
 * a message with no body.
 */

/****************************************************************
 * message types
 */

/*
 * messageType: DS_MESSAGE_TYPE_DMAP
 * messageSubType: DMAP_REG_LATEST_DATA_INFO
 *
 * A process uses this message to register the latest data time
 * of data as it is put into the store.
 *
 * The message body has a single DMAP_INFO_PART containing a DMAP_info_t
 * struct filled out with:
 *       latest_time
 *       hostname
 *       ipaddr
 *       datatype
 *       dir
 *
 * A DMAP_REPLY_REG_STATUS message is expected as a reply.
 */
  
/*
 * messageType: DS_MESSAGE_TYPE_DMAP
 * messageSubType: DMAP_REG_STATUS_INFO
 *
 * A process uses this message to register the latest status
 * of a data set
 *
 * The message body has a single DMAP_INFO_PART containing a DMAP_info_t
 * struct filled out with:
 *       status
 *       hostname
 *       ipaddr
 *       datatype
 *       dir
 *
 * A DMAP_REPLY_REG_STATUS message is expected as a reply.
 */
  
/*
 * messageType: DS_MESSAGE_TYPE_DMAP
 * messageSubType: DMAP_REG_DATA_SET_INFO
 *
 * The Scout, or a similar process, uses this message to register
 * info about a data set in the store.
 *
 * The message body has a single DMAP_INFO_PART containing a DMAP_info_t
 * struct filled out with:
 *       start_time
 *       end_time
 *       nfiles
 *       total_bytes (total of all bytes in all files)
 *       hostname
 *       ipaddr
 *       datatype
 *       dir
 *
 * A DMAP_REPLY_REG_STATUS message is expected as a reply.
 */
  
/*
 * messageType: DS_MESSAGE_TYPE_DMAP
 * messageSubType: DMAP_REG_FULL_INFO
 *
 * Register fully-qualified info structs.
 * DmapSync uses this to relay an array of info to the DataMapper.
 *
 * The message body has multiple DMAP_INFO_PART containing a DMAP_info_t
 * struct fully filled out.
 *
 * A DMAP_REPLY_REG_STATUS message is expected as a reply.
 */
  
/*
 * messageType: DS_MESSAGE_TYPE_DMAP
 * messageSubType: DMAP_DELETE_INFO
 *
 * This is used by applications to delete a data set entry
 *
 * The message body has a single DMAP_INFO_PART containing a DMAP_info_t
 * struct filled out with:
 *       hostname
 *       datatype
 *       dir
 *
 * A DMAP_REPLY_REG_STATUS message is expected as a reply.
 */
  
/*
 * messageType: DS_MESSAGE_TYPE_DMAP
 * messageSubType: DMAP_REQ_SELECTED_SETS_INFO
 *
 * A client uses this message to request information about selected
 * data sets in the store.
 *
 * The message body has a single DMAP_INFO_PART containing a DMAP_info_t
 * struct filled out with:
 *       datatype
 *       dir
 * If the request goes through a relay host, the message will have
 * an optional DMAP_RELAY_HOST_PART.
 * 
 * If datatype is empty, all data types are matched.
 * If dir is empty, all dirs are matched.
 *
 * A DMAP_REPLY_WITH_INFO is expected.
 */
  
/*
 * messageType: DS_MESSAGE_TYPE_DMAP
 * messageSubType: DMAP_REQ_ALL_SETS_INFO
 *
 * A client uses this message to request information about all
 * data sets in the store.
 *
 * If the request goes through a relay host, the message will have
 * an optional DMAP_RELAY_HOST_PART.
 * Otherwise, the message has no body.
 *
 * A DMAP_REPLY_WITH_INFO is expected.
 */
  
/*
 * messageType: DS_MESSAGE_TYPE_DMAP
 * messageSubType: DMAP_REPLY_REG_STATUS
 *
 * This message is sent back to the client after a registration.
 *
 * messageFlags is:
 *
 *   DMAP_REPLY_OK if no error occurred.
 *   DMAP_REPLY_ERROR if an error occurred.
 *
 * If no error occurred the message has no body.
 *
 * If an error occurred, the message body contains a single
 * DMAP_ERROR_STRING_PART, a null-terminated string which
 * describes the error.
 */
  
/*
 * messageType: DS_MESSAGE_TYPE_DMAP
 * messageSubType: DMAP_REPLY_WITH_INFO
 *
 * This message is sent back to the client in response to a
 * request for data set information.
 *
 * messageFlags is:
 *
 *   DMAP_REPLY_OK if no error occurred.
 *   DMAP_REPLY_ERROR if an error occurred.
 *
 * If no error occurred the message contains an array of DMAP_INFO_PARTS,
 * one for each of the data sets about which information is returned.
 *
 * If an error occurred, the message body contains a single
 * DMAP_ERROR_STRING_PART, a null-terminated string which
 * describes the error.
 */
  
/*
 * message type definition
 */

#define DS_MESSAGE_TYPE_DMAP 3818000

/*
 * message sub-type definitions
 */

#define DMAP_REG_STATUS_INFO        3818100
#define DMAP_REG_LATEST_DATA_INFO   3818200
#define DMAP_REG_DATA_SET_INFO      3818201
#define DMAP_REQ_SELECTED_SETS_INFO 3818202
#define DMAP_REQ_ALL_SETS_INFO      3818203
#define DMAP_REPLY_REG_STATUS       3818204
#define DMAP_REPLY_WITH_INFO        3818205
#define DMAP_DELETE_INFO            3818206
#define DMAP_REG_FULL_INFO          3818207

/*
 * message flags definitions
 */

#define DMAP_REPLY_OK               3818302
#define DMAP_REPLY_ERROR            3818303

/*
 * part definitions
 */

#define DMAP_INFO_PART              3818400
#define DMAP_ERROR_STRING_PART      3818401
#define DMAP_RELAY_HOST_PART        3818402

class DmapMessage : public DsServerMsg
{
public:
  
  // constructor

  DmapMessage();

  // destructor
  
  virtual ~DmapMessage();

  // access to data members
  
  bool isRegLatestDataInfo() { return (_isRegLatestDataInfo); }
  bool isRegStatusInfo() { return (_isRegStatusInfo); }
  bool isRegDataSetInfo() { return (_isRegDataSetInfo); }
  bool isRegFullInfo() { return (_isRegFullInfo); }
  bool isDeleteInfo() { return (_isDeleteInfo); }
  bool isReqSelectedSetsInfo() { return (_isReqSelectedSetsInfo); }
  bool isReqAllSetsInfo() { return (_isReqAllSetsInfo); }
  bool isReplyRegStatus() { return (_isReplyRegStatus); }
  bool isReplyWithInfo() { return (_isReplyWithInfo); }

  const string &getRelayHostList() { return _relayHostList; }

  bool errorOccurred() { return (_errorOccurred); }
  const char *errorStr() { return (_errorStr.c_str()); }
  const string &getErrorStr() { return _errorStr; }

  int getNInfo() const { return (int) _info.size(); }
  const DMAP_info_t &getInfo(int i) const { return _info[i]; }
  const vector<DMAP_info_t> &getInfo() const { return _info; }

  // assemble a DMAP_REG_LATEST_DATA_INFO message
  
  void *assembleRegLatestInfo(const time_t latest_time,
			      const char *hostname,
			      const char *ipaddr,
			      const char *datatype,
			      const char *dir,
			      const int forecast_lead_time);
  
  // assemble a DMAP_REG_STATUS_INFO message
  
  void *assembleRegStatusInfo(const char *status,
			      const char *hostname,
			      const char *ipaddr,
			      const char *datatype,
			      const char *dir);
  
  // assemble a DMAP_REG_DATA_SET_INFO message

  void *assembleRegDataSetInfo(const time_t start_time,
			       const time_t end_time,
			       const double nfiles,
			       const double total_bytes,
			       const char *hostname,
			       const char *ipaddr,
			       const char *datatype,
			       const char *dir);
  
  // assemble a DMAP_REG_FULL_INFO message

  void *assembleRegFullInfo(const DMAP_info_t &info);
  void *assembleRegFullInfo(const vector<DMAP_info_t> &infoArray);
  
  // assemble a DMAP_DELETE_INFO message

  void *assembleDeleteInfo(const char *hostname,
			   const char *datatype,
			   const char *dir);
  
  // assemble a DMAP_REQ_SINGLE_SET_INFO message

  void *assembleReqSelectedSetsInfo(const char *datatype,
				    const char *dir,
				    const char *relay_hostlist = NULL);

  // assemble a DMAP_REQ_ALL_SETS_INFO message

  void *assembleReqAllSetsInfo(const char *relay_hostlist = NULL);
  
  // assemble a DMAP_REPLY_REG_STATUS

  void *assembleReplyRegStatus(const bool errorOccurred = FALSE,
			       const char *errorStr = NULL,
			       const DMAP_info_t *info = NULL);

  // assemble a DMAP_REPLY_WITH_INFO

  void *assembleReplyWithInfo(const int nInfo,
			      const DMAP_info_t *info,
			      const bool errorOccurred = FALSE,
			      const char *errorStr = NULL);
  
  // overload disassemble
  
  int disassemble(void *in_msg, const int msg_len);

  // print

  virtual void print(ostream &out, const char * spacer = "") const;

  // byte swapping

  static void BE_to_dmap_info(DMAP_info_t &info);
  static void BE_from_dmap_info(DMAP_info_t &info);

private:

  bool _isRegLatestDataInfo;
  bool _isRegStatusInfo;
  bool _isRegDataSetInfo;
  bool _isRegFullInfo;
  bool _isDeleteInfo;
  bool _isReqSelectedSetsInfo;
  bool _isReqAllSetsInfo;
  bool _isReplyRegStatus;
  bool _isReplyWithInfo;

  bool _errorOccurred;
  string _errorStr;
  string _relayHostList;

  vector<DMAP_info_t> _info;
  
};

#endif


