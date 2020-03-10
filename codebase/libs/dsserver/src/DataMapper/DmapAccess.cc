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
 * dsserver/DmapAccess.cc
 *
 * Class for communicating with the DataMapper
 ******************************************************************/

#include <dsserver/DmapAccess.hh>
#include <dsserver/DsLocator.hh>
#include <didss/RapDataDir.hh>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <toolsa/GetHost.hh>
using namespace std;

/////////////////
// constructor

DmapAccess::DmapAccess() : ThreadSocket()
{

  // set debug flag

  _debug = false;
  char *debug_str = getenv("DATA_MAPPER_DEBUG");
  if (debug_str && STRequal(debug_str, "true")) {
    _debug = true;
  }

  // set active flag

  _active = true;
  char *active_str = getenv("DATA_MAPPER_ACTIVE");
  if (active_str && STRequal(active_str, "false")) {
    _active = false;
  }

  // set up the port number

  _port = DsLocator.getDefaultPort("DataMapper");

  // by default _checkRegisterInterval is on with interval of 1

  _checkRegisterInterval = true;
  _lastRegisterTime = 0;
  _registerInterval = 1;
  _respectDataDir = true;

}

/////////////////
// destructor

DmapAccess::~DmapAccess()
{
}

//////////////////////////////////////////////////////////////
// set the option to respect RAP_DATA_DIR or DATA_DIR environment
// variables true/false
// default is true

void DmapAccess::setRespectDataDir(bool respect_data_dir)

{
  _respectDataDir = respect_data_dir;
}

/////////////////////////////////////////////////////////////
// Set checking for register interval on.
// If this is set, will only register when the time since the
// last registration exceeds _registerInterval

void DmapAccess::setCheckRegInterval(int reg_interval)

{
  _registerInterval = reg_interval;
  if (_registerInterval < 1) {
    _checkRegisterInterval = false;
  } else {
    _checkRegisterInterval = true;
  }
}

/////////////////////////////////////////////////////////////////////
// register latest data info
//
// Returns 0 on success, -1 on failure

int DmapAccess::regLatestInfo(const time_t latest_time,
			      const string dir,
			      const string datatype /* = ""*/,
			      const int forecast_lead_time /* = -1*/ )
  
{

  if (!_active) {
    return 0;
  }

  if (_checkRegisterInterval) {
    time_t now = time(NULL);
    int timeSinceLastReg = now - _lastRegisterTime;
    if (timeSinceLastReg < _registerInterval) {
      return 0;
    }
    _lastRegisterTime = now;
  }
  
  // strip RAP_DATA_DIR from the dir if required
  
  string file;
  if (_respectDataDir) {
    RapDataDir.stripPath(dir, file);
  } else {
    file = dir;
  }

  // assemble the message

  GetHost localHost;
  void *buf = _msg.assembleRegLatestInfo(latest_time,
                                         localHost.getName().c_str(),
                                         localHost.getIpAddr().c_str(),
					 datatype.c_str(),
					 file.c_str(),
					 forecast_lead_time);
  
  const string dmapHost = "localhost";
  if (_communicate(buf, _msg.lengthAssembled(), dmapHost)) {
    if (_debug) {
      cerr << "WARNING - COMM - DmapAccess::regLatestInfo" << endl;
    }
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////
// register data set info
//
// Returns 0 on success, -1 on failure

int DmapAccess::regDataSetInfo(const time_t start_time,
			       const time_t end_time,
			       const double nfiles,
			       const double total_bytes,
			       const string dir,
			       const string datatype /* = ""*/ )
  
{
  
  if (!_active) {
    return 0;
  }

  // strip RAP_DATA_DIR from the dir if required

  string file;
  if (_respectDataDir) {
    RapDataDir.stripPath(dir, file);
  } else {
    file = dir;
  }

  // assemble the message

  GetHost localHost;
  void *buf = _msg.assembleRegDataSetInfo(start_time,
					  end_time,
					  nfiles,
					  total_bytes,
                                          localHost.getName().c_str(),
                                          localHost.getIpAddr().c_str(),
					  datatype.c_str(),
					  file.c_str());
  
  const string dmapHost = "localhost";
  if (_communicate(buf, _msg.lengthAssembled(), dmapHost)) {
    if (_debug) {
      cerr << "WARNING - COMM - DmapAccess::regDataSetInfo" << endl;
    }
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////
// register status info
//
// Returns 0 on success, -1 on failure

int DmapAccess::regStatusInfo(const string status,
			      const string dir,
			      const string datatype /* = ""*/ )
  
{

  if (!_active) {
    return 0;
  }

  // strip RAP_DATA_DIR from the dir if required

  string file;
  if (_respectDataDir) {
    RapDataDir.stripPath(dir, file);
  } else {
    file = dir;
  }

  // assemble the message

  GetHost localHost;

  void *buf = _msg.assembleRegStatusInfo(status.c_str(),
                                         localHost.getName().c_str(),
                                         localHost.getIpAddr().c_str(),
					 datatype.c_str(),
					 file.c_str());
  
  const string dmapHost = "localhost";
  if (_communicate(buf, _msg.lengthAssembled(), dmapHost)) {
    if (_debug) {
      cerr << "WARNING - COMM - DmapAccess::regStatusInfo" << endl;
    }
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////
// register full info
//
// Returns 0 on success, -1 on failure

int DmapAccess::regFullInfo(const vector<DMAP_info_t> &infoArray)
  
{

  if (!_active) {
    return 0;
  }

  // assemble the message
  
  void *buf = _msg.assembleRegFullInfo(infoArray);
  
  const string dmapHost = "localhost";
  if (_communicate(buf, _msg.lengthAssembled(), dmapHost)) {
    if (_debug) {
      cerr << "WARNING - COMM - DmapAccess::regFullInfo" << endl;
    }
    return -1;
  }

  return 0;

}

int DmapAccess::regFullInfo(const DMAP_info_t &info)
  
{
  vector<DMAP_info_t> infoArray;
  infoArray.push_back(info);
  return regFullInfo(infoArray);
}

///////////////////////////////////////////////
// delete info
//
// Returns 0 on success, -1 on failure

int DmapAccess::deleteInfo(const string hostname,
			   const string dir,
			   const string datatype /* = ""*/ )
  
{
  
  if (!_active) {
    return 0;
  }

  // strip RAP_DATA_DIR from the dir if required

  string file;
  if (_respectDataDir) {
    RapDataDir.stripPath(dir, file);
  } else {
    file = dir;
  }

  // assemble the message

  void *buf = _msg.assembleDeleteInfo(hostname.c_str(),
				      datatype.c_str(),
				      file.c_str());
  
  const string dmapHost = "localhost";
  if (_communicate(buf, _msg.lengthAssembled(), dmapHost)) {
    if (_debug) {
      cerr << "WARNING - COMM - DmapAccess::deleteInfo" << endl;
    }
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////
// request selected info
//
// Returns 0 on success, -1 on failure

int DmapAccess::reqSelectedInfo(const string datatype,
				const string dir,
				const string dmapHost /* = "localhost"*/ )
  
{

  _info.erase(_info.begin(), _info.end());
  
  // strip RAP_DATA_DIR from the dir if required

  string file;
  if (_respectDataDir) {
    RapDataDir.stripPath(dir, file);
  } else {
    file = dir;
  }

  // assemble the message
  
  void *buf;
  string hostToContact;
  size_t colonPos = dmapHost.find(":", 0);
  
  if (colonPos == string::npos) {
    
    // no colon, so no relay host
    
    hostToContact = dmapHost;
    buf = _msg.assembleReqSelectedSetsInfo(datatype.c_str(),
					   file.c_str());
    
  } else {
    
    // colon found in host name, so no relay host list applies
    
    string remainingHosts;
    hostToContact.assign(dmapHost, 0, colonPos);
    remainingHosts.assign(dmapHost, colonPos + 1, string::npos);
    buf = _msg.assembleReqSelectedSetsInfo(datatype.c_str(),
					   file.c_str(),
					   remainingHosts.c_str());
    
  }
  
  if (_communicate(buf, _msg.lengthAssembled(), hostToContact)) {
    if (_debug) {
      cerr << "WARNING - COMM - DmapAccess::reqSelectedInfo" << endl;
    }
    return -1;
  }
  
  // copy info from msg
  
  for (int i = 0; i < _msg.getNInfo(); i++) {
    _info.push_back(_msg.getInfo(i));
  }
  
  return 0;

}

///////////////////////////////////////////////
// request all info
//
// Returns 0 on success, -1 on failure

int DmapAccess::reqAllInfo(const string dmapHost /* = "localhost"*/ )
  
{

  _info.erase(_info.begin(), _info.end());

  // assemble the message
  
  void *buf;
  string hostToContact;
  size_t colonPos = dmapHost.find(":", 0);
  
  if (colonPos == string::npos) {
    
    // no colon, so no relay host
    
    hostToContact = dmapHost;
    buf = _msg.assembleReqAllSetsInfo();
    
  } else {
    
    // colon found in host name, so no relay host list applies
    
    string remainingHosts;
    hostToContact.assign(dmapHost, 0, colonPos);
    remainingHosts.assign(dmapHost, colonPos + 1, string::npos);
    buf = _msg.assembleReqAllSetsInfo(remainingHosts.c_str());
    
  }
  
  if (_communicate(buf, _msg.lengthAssembled(), hostToContact)) {
    if (_debug) {
      cerr << "WARNING - COMM - DmapAccess::reqAllInfo" << endl;
    }
    return -1;
  }
  
  // copy info from msg
  
  for (int i = 0; i < _msg.getNInfo(); i++) {
    _info.push_back(_msg.getInfo(i));
  }
  
  return 0;

}

/////////////////////////////////////////////
// get info

DMAP_info_t DmapAccess::getInfo(int i) const

{

  if (i < (int) _info.size()) {
    return _info[i];
  } else {
    // should not happen, but just in case, send back empty struct
    DMAP_info_t info;
    MEM_zero(info);
    return info;
  }

}

/////////////////////////////////////////////
// open socket to DataMapper on specfied host
//
// Returns 0 on success, -1 on error.

int DmapAccess::_open(const string dmapHost)

{

  if (open(dmapHost.c_str(), _port)) {
    
    if (_debug) {
      cerr << "WARNING - COMM - DmapAccess::_open" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  Cannot contact DataMapper" << endl;
      cerr << "  Host: " << dmapHost << endl;
    }

    return -1;

  }

  return 0;

}

/////////////////////////////////////////////
// communicate with DataMapper on specfied host
//
// Returns 0 on success, -1 on error.

int DmapAccess::_communicate(void *buf,
			     const int buflen,
			     const string dmapHost)

{

  // open socket

  if (_open(dmapHost)) {
    return -1;
  }

  // get comm timeout
  
  ssize_t commTimeoutMsecs = DS_DEFAULT_COMM_TIMEOUT_MSECS;
  char *DS_COMM_TIMEOUT_MSECS = getenv("DS_COMM_TIMEOUT_MSECS");
  if (DS_COMM_TIMEOUT_MSECS != NULL) {
    ssize_t timeout;
    if (sscanf(DS_COMM_TIMEOUT_MSECS, "%ld", &timeout) == 1) {
      commTimeoutMsecs = timeout;
    }
  }

  // write the message

  if (writeMessage(DS_MESSAGE_TYPE_DMAP,
		   buf, buflen, commTimeoutMsecs)) {
    if (_debug) {
      cerr << "ERROR - COMM - DmapAccess::_communicate" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  Cannot send message to DataMapper on " << dmapHost << endl;
    }
    close();
    return -1;
  }
  
  // read the reply

  if (readMessage(commTimeoutMsecs)) {
    if (_debug) {
      cerr << "ERROR - COMM - DmapAccess::_communicate" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  Cannot read reply from DataMapper on "  << dmapHost << endl;
      cerr << "  " << getErrString() << endl;
    }
    close();
    return -1;
  }

  // close the socket

  close();

  // disassemble the reply

  if (_msg.disassemble((void *) getData(), getNumBytes())) {
    if (_debug) {
      cerr << "ERROR - COMM - DmapAccess::_communicate" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  Invalid reply" << endl;
    }
    return -1;
  }

  return 0;

}

