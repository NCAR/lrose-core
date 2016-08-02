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
 * dsserver/DmapAccess.hh
 *
 * Class for communicating with the DataMapper
 ******************************************************************/

#ifndef DmapAccess_HH
#define DmapAccess_HH

#include <cassert>
#include <string>

#include <toolsa/ThreadSocket.hh>
#include <dsserver/DmapMessage.hh>
using namespace std;

class DmapAccess : public ThreadSocket
{
public:
  
  // constructor

  DmapAccess();

  // destructor

  virtual ~DmapAccess();

  // check if the DataMapper is active
  // now it is always active

  bool isActive() { return true; }

  // set the option to respect RAP_DATA_DIR or DATA_DIR environment
  // variables true/false
  // default is true

  void setRespectDataDir(bool respect_data_dir);

  // Set checking for register interval on.
  // If this is set, will only register when the time since the
  // last registration exceeds _registerInterval.
  // If reg_interval is 0, the checking is turned off.
  // Default behavior is on with a reg_interval of 1

  void setCheckRegInterval(int reg_interval);

  // register latest data info
  // Returns 0 on success, -1 on failure
  
  int regLatestInfo(const time_t latest_time,
		    const string dir,
		    const string datatype = "",
		    const int forecast_lead_time = -1);
  
  // register data set info
  // Returns 0 on success, -1 on failure
  
  int regDataSetInfo(const time_t start_time,
		     const time_t end_time,
		     const double nfiles,
		     const double total_bytes,
		     const string dir,
		     const string datatype = "");
  
  // register status info
  // Returns 0 on success, -1 on failure
  
  int regStatusInfo(const string status,
		    const string dir,
		    const string datatype = "");
  
  // register full info
  // Returns 0 on success, -1 on failure
  
  int regFullInfo(const vector<DMAP_info_t> &infoArray);
  int regFullInfo(const DMAP_info_t &info);
  
  // delete an info entry
  // Returns 0 on success, -1 on failure

  int deleteInfo(const string hostname,
		 const string dir,
		 const string datatype = "");
  
  // request selected info
  // Returns 0 on success, -1 on failure
  
  int reqSelectedInfo(const string datatype,
		      const string dir,
		      const string dmapHost = "localhost");
  
  // request all info
  // Returns 0 on success, -1 on failure
  
  int reqAllInfo(const string dmapHost = "localhost");

  // get info from object after requesting it from DataMapper

  int getNInfo() const { return (_info.size()); }

  DMAP_info_t getInfo(int i) const;

  const vector<DMAP_info_t> &getInfoArray() const {
    return _info;
  }

protected:

  bool _active;
  bool _debug;

  DmapMessage _msg;
  int _port;

  vector<DMAP_info_t> _info;

  bool _respectDataDir;
  bool _checkRegisterInterval;
  time_t _lastRegisterTime;
  int _registerInterval;

  // open socket to DataMapper on specfied host
  int _open(const string dmapHost);

  // communicate with DataMapper on specfied host
  int _communicate(void *buf, const int buflen, const string dmapHost);

};

#endif


