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
// TestDataMap.cc
//
// TestDataMap object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////
//
// TestDataMap tests the DataMapper
//
///////////////////////////////////////////////////////////////

#include "TestDataMap.hh"
#include "Args.hh"
#include "Params.hh"
#include <dsserver/DmapAccess.hh>

#include <toolsa/umisc.h>
#include <toolsa/str.h>
using namespace std;

// Constructor

TestDataMap::TestDataMap(int argc, char **argv)

{

  OK = TRUE;
  _progName = NULL;
  _args = NULL;
  _params = NULL;
  _access = NULL;

  // set programe name

  _progName = STRdup("TestDataMap");

  // get command line args
  
  _args = new Args(argc, argv, _progName);
  if (!_args->OK) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _params = new Params();
  _paramsPath = (char *) "unknown";
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

  // access

  _access = new DmapAccess();

  return;

}

// destructor

TestDataMap::~TestDataMap()

{

  // free up

  if (_progName) {
    STRfree(_progName);
  }
  if (_args) {
    delete(_args);
  }
  if (_params) {
    delete(_params);
  }
  if (_access) {
    delete (_access);
  }

}

//////////////////////////////////////////////////
// Run

int TestDataMap::Run()
{

  int forever = TRUE;

  while (forever) {
    if (_params->mode == Params::LATEST_DATA_INFO) {
      if (_sendLatestInfo()) {
	cerr << "_sendLatestInfo failed" << endl;
      }
    } else if (_params->mode == Params::STATUS_INFO) {
      if (_sendStatusInfo()) {
	cerr << "_sendStatusInfo failed" << endl;
      }
    } else if (_params->mode == Params::DATA_SET_INFO) {
      if (_sendDataSetInfo()) {
	cerr << "_sendDataSetInfo failed" << endl;
      }
    } else if (_params->mode == Params::DELETE_INFO) {
      if (_sendDeleteInfo()) {
	cerr << "_sendDeleteInfo failed" << endl;
      }
    }
    if (!_params->continuous_operation) {
      break;
    }
    umsleep(_params->delay_msecs);
  }
  
  return (0);

}

////////////////////
// _sendLatestInfo()
//

int TestDataMap::_sendLatestInfo()

{

  time_t latest_time;

  if (_params->use_now_for_latest_time) {
    latest_time = time(NULL);
  } else {
    latest_time = utime_compute(_params->latest_time.year,
				_params->latest_time.month,
				_params->latest_time.day,
				_params->latest_time.hour,
				_params->latest_time.min,
				_params->latest_time.sec);
  }
  
  if (_params->debug) {
    cout << "-----------------------------------------" << endl;
    cout << "hostname:" << PORThostname() << endl;
    cout << "IpAddr: " << PORThostIpAddr() << endl;
    cout << "Data type: " << _params->data_type << endl;
    cout << "Data set dir: " << _params->data_set_directory << endl;
    cout << "latest time: " << utimstr(latest_time) << endl;
    cout << "forecast lead time: " << _params->forecast_lead_time << endl;
  }

  if (_access->regLatestInfo(latest_time,
			     _params->data_set_directory,
			     _params->data_type,
			     _params->forecast_lead_time)) {
    cerr << "ERROR - TestDataMap::_sendLatestInfo" << endl;
    cerr << "Cannot send latest data info" << endl;
    return (-1);
  }
      
  return (0);

}

////////////////////
// _sendStatusInfo()
//

int TestDataMap::_sendStatusInfo()

{

  if (_params->debug) {
    cout << "-----------------------------------------" << endl;
    cout << "hostname:" << PORThostname() << endl;
    cout << "IpAddr: " << PORThostIpAddr() << endl;
    cout << "Data type: " << _params->data_type << endl;
    cout << "Data set dir: " << _params->data_set_directory << endl;
    cout << "status: " << _params->status << endl;
  }

  if (_access->regStatusInfo(_params->status,
			     _params->data_set_directory,
			     _params->data_type)) {
    cerr << "ERROR - TestDataMap::_sendStatusInfo" << endl;
    cerr << "Cannot send status info" << endl;
    return (-1);
  }
      
  return (0);

}

////////////////////
// _sendDataSetInfo()
//

int TestDataMap::_sendDataSetInfo()

{

  time_t start_time =
    utime_compute(_params->start_time.year,
		  _params->start_time.month,
		  _params->start_time.day,
		  _params->start_time.hour,
		  _params->start_time.min,
		  _params->start_time.sec);
  
  time_t end_time =
    utime_compute(_params->end_time.year,
		  _params->end_time.month,
		  _params->end_time.day,
		  _params->end_time.hour,
		  _params->end_time.min,
		  _params->end_time.sec);

  if (_params->debug) {
    cout << "-----------------------------------------" << endl;
    cout << "hostname:" << PORThostname() << endl;
    cout << "IpAddr: " << PORThostIpAddr() << endl;
    cout << "Data type: " << _params->data_type << endl;
    cout << "Data set dir: " << _params->data_set_directory << endl;
    cout << "start time: " << utimstr(start_time) << endl;
    cout << "end time: " << utimstr(end_time) << endl;
    cout << "nfiles: " << _params->nfiles << endl;
    cout << "total_bytes: " << _params->total_bytes << endl;
  }

  if (_access->regDataSetInfo(start_time,
			      end_time,
			      _params->nfiles,
			      _params->total_bytes,
			      _params->data_set_directory,
			      _params->data_type)) {
    cerr << "ERROR - TestDataMap::_sendLatestInfo" << endl;
    cerr << "Cannot send data set info" << endl;
    return (-1);
  }
      
  return (0);

}

////////////////////
// _sendDataSetInfo()
//

int TestDataMap::_sendDeleteInfo()
  
{
  
  if (_params->debug) {
    cout << "-----------------------------------------" << endl;
    cout << "Hostname:" << _params->data_set_host << endl;
    cout << "Data type: " << _params->data_type << endl;
    cout << "Data set dir: " << _params->data_set_directory << endl;
  }

  if (_access->deleteInfo(_params->data_set_host,
			  _params->data_set_directory,
			  _params->data_type)) {
    cerr << "ERROR - TestDataMap::_sendDeleteInfo" << endl;
    cerr << "Cannot send delete info" << endl;
    return (-1);
  }
      
  return (0);

}

