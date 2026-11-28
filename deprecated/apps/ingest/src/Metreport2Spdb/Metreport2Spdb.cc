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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: Metreport2Spdb.cc,v 1.6 2016/03/07 01:23:02 dixon Exp $
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	Metreport2Spdb
//
// Author:	G. M. Cunning
//
// Date:	Sat Mar 17 13:52 2012
//
// Description: This class creates manages reading, parsing and writing 
//		MET REPORT and SPECIAL bulletins.
//
//

// C++ include files
#include <iostream>
#include <cassert>
#include <algorithm>

// System/RAP include files
#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/Path.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <Spdb/DsSpdb.hh>

// Local include files
#include "Metreport2Spdb.hh"
#include "Input.hh"
#include "Params.hh"
#include "Args.hh"

using namespace std;

// the singleton itself
Metreport2Spdb *Metreport2Spdb::_instance = 0;
 
// define any constants
const string Metreport2Spdb::_className = "Metreport2Spdb";

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

Metreport2Spdb::Metreport2Spdb() :
  _isOK(true),
  _progName(""),
  _errStr(""),
  _args(0),
  _params(0)

{
  // Make sure the singleton wasn't already created.
  assert(_instance == 0);
 
  // Set the singleton instance pointer
  _instance = this; 
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
Metreport2Spdb::~Metreport2Spdb()
{
  // unregister process
  PMU_auto_unregister();

  // Free contained objects
  delete _params;
  delete _args;
  delete _inputPath;
  if(_input) {
    _input->close();
  }
  delete _input;
  delete _stationLoc;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Metreport2Spdb::instance
//
// Description:	this method creates instance of Metreport2Spdb object
//
// Returns:	returns pointer to self
//
// Notes:	this method implements the singleton pattern
//
//

Metreport2Spdb*
Metreport2Spdb::instance(int argc, char **argv)
{
  if (_instance == 0) {
    new Metreport2Spdb();
 
   if(_instance->_isOK) {
      _instance->_initialize(argc, argv);
    }
  }
  return(_instance);
}

Metreport2Spdb*
Metreport2Spdb::instance()
{
  assert(_instance != 0);
  
  return(_instance);
}


/////////////////////////////////////////////////////////////////////////
// run
//
int 
Metreport2Spdb::run()
{

  const string methodName = _className + string( "::run" );

  // register with procmap
  PMU_auto_register("Running");

  if(static_cast<int>(_params->debug) != static_cast<int>(Params::DEBUG_OFF)) {
    cerr << methodName << ": Running" << endl;
  } 

  char* inPath = NULL;
  while ((inPath = _inputPath->next(true)) != 0) {
    string inputFilePath(inPath);
    time_t _refTime = _inputPath->getDataTime(inputFilePath);
    
    if (_params->mode == Params::REALTIME) {
      _fileTime.set(time(0)); // now
      _fileTime.setSec(0); // truncate the seconds
    } else {
      _fileTime.set(_refTime);
    }

    string procmapString = "";
    Path path(inputFilePath);
    procmapString = "Processing file <" + path.getFile() + ">";  
    PMU_auto_register(procmapString.c_str());

    _processFile(inputFilePath);

  } // while

  return 0;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// _initialize
//
void 
Metreport2Spdb::_initialize(int argc, char **argv)
{
  const string methodName = _className + string( "::_initialize" );

  // set programe name
  Path pathParts(argv[0]);
  _progName = pathParts.getBase();
  ucopyright(const_cast<char*>(_progName.c_str()));
 
  // get command line args
  _args= new Args(argc, argv, _progName);

  // get TDRP params
  _params = new Params();
  char *paramsPath = const_cast<char*>(string("unknown").c_str());
  if(_params->loadFromArgs(argc, argv, _args->override.list,
			   &paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    _isOK = false;
  } // endif -- _params->loadFromArgs(argc, argv, _args.override.list, ...

  // file input object
  bool debugDs= false;
  if(static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_VERBOSE)) {
    debugDs= true;
  }

  _inputPath = 0;  
  if (_params->mode == Params::FILELIST) {
    
    // FILELIST mode
    
    _inputPath = new DsInputPath(_progName, debugDs, _args->inputFileList);
    
  } else if (_params->mode == Params::ARCHIVE) {
    
    // archive mode - start time to end time
    
    _inputPath = new DsInputPath(_progName, debugDs, _params->input_dir,
				 DateTime::parseDateTime(_params->start_time),
				 DateTime::parseDateTime(_params->end_time));
    
  } else if (_params->mode == Params::REALTIME) {
    
    // realtime mode - no latest_data_info file
    
    _inputPath = new DsInputPath(_progName, debugDs, _params->input_dir,
				 _params->max_realtime_valid_age,
				 PMU_auto_register,
				 _params->latest_data_info_avail,
				 true);
    
    if (_params->strict_subdir_check) {
      _inputPath->setStrictDirScan(true);
    }
    
    if (_params->file_name_check) {
      _inputPath->setSubString(_params->file_match_string);
    }
    
  }
  
  _input = new Input(_params);
  _input->setSOH('\001');
  _input->setEOH('\003');

  // get from params?
  _input->addTypeId("MET REPORT");
  _input->addTypeId("SPECIAL");

  _stationLoc = new StationLoc();
  if (_stationLoc->ReadData(_params->station_location_path)) {
    cerr << "ERROR: " << methodName << endl;
    cerr << "  Cannot load station locations, file : "
         << _params->station_location_path << endl;
    _isOK = false;
  }

  // init process mapper registration
  if (_params->mode == Params::REALTIME) {
    PMU_auto_init((char *) _progName.c_str(),
		  _params->instance,
		  PROCMAP_REGISTER_INTERVAL);
    PMU_auto_register("In Metreport2Spdb constructor");
  }

}


/////////////////////////////////////////////////////////////////////////
// _processFile
//
int 
Metreport2Spdb::_processFile(const string& file_path)  
{

  const string methodName = _className + string( "::_processFile" );

  // open file
  if(_input->open(file_path)) {
    cerr << "ERROR - " << methodName << endl;
    return -1;
  }

  if(static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_NORM)) {
    cerr << "Processing file: " << file_path << endl;
    cerr << "  File time: " << _fileTime.getStr() << endl << endl;
  }
  
  string reportStr;
  DsSpdb outSpdb;
  int nTotal = 0;
  int nDecoded = 0;

  while(_input->readNext(reportStr) == 0) {
    
    PMU_auto_register("reading next message");
    
    if(reportStr.empty() == true) {
      continue;
    }

    nTotal++;

    if(static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_EXTRA)) {
      cerr << "====== Count is " << nTotal << " =====" << endl;
      cerr << "========== Start REPORT ===========" << endl << reportStr 
	   << endl << "========== End REPORT ===========" << endl;
    }
      
    if(_decode(reportStr) == 0) {
      nDecoded++;
      _addPut(reportStr, outSpdb);
    } else {
      if (static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_NORM)) {
        cerr << "WARNING -- " << methodName<< endl;
        cerr << "Cannot decode REPORT:" << endl;
        cerr << reportStr;
       cerr << "***************************" << endl;
      }
    }

  } // while

  // close file
  
  _input->close();

  // no need to write
  if(nDecoded < 1) {
    return 0;
  }

  // Write out
  
  int iret = 0;

  if(outSpdb.put(_params->output_url,
		  SPDB_ASCII_ID, SPDB_ASCII_LABEL)) {
    cerr << "ERROR - " << methodName << endl;
    cerr << outSpdb.getErrStr() << endl;
    iret = -1;
  } 
  else {
    if (static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_NORM)) {
      cerr << "Wrote REPORT data to URL: "
	   << _params->output_url << endl;
    }
  }
  
  return iret;
}

/////////////////////////////////////////////////////////////////////////
// _addPut
//
void 
Metreport2Spdb::_addPut(const string& report, DsSpdb& out_spdb)
{

  time_t testTime;
  if (_params->mode == Params::REALTIME) {
    testTime = time(0);
  }
  else {
    testTime = _fileTime.utime();
  }

  // check that issue time is not significantly in past
  time_t deltaT = testTime - _validTime;
  if((deltaT > _params->max_bulletin_valid_age) || (deltaT < 0)) {
    if (static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_EXTRA)) {
      cerr << "==============================" << endl;
      cerr << "Skipping MET REPORT: " << endl;
      cerr << "  Issue time: " << DateTime::str(_validTime) << endl;
      cerr << "  greater than maximum for a bulleting " << endl;
      cerr << "  " << report << endl;
      cerr << "==============================" << endl;
    }
    return;
  }

  si32 dataType = Spdb::hash4CharsToInt32(_name.c_str());
  time_t validTime = _validTime;
  time_t expireTime =  validTime + _params->expire_seconds;

  string asciiChunk;
  asciiChunk += report;
  out_spdb.addPutChunk(dataType, validTime, expireTime,
		       asciiChunk.size() + 1, asciiChunk.c_str());
    
  if (static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_VERBOSE)) {
    cerr << "==============================" << endl;
    cerr << "Adding MET REPORT chunk: " << endl;
    cerr << "  Issue time: " << DateTime::str(_validTime) << endl;
    cerr << "  " << report << endl;
    cerr << "==============================" << endl;
  }

}

/////////////////////////////////////////////////////////////////////////
// _decode
//
int 
Metreport2Spdb::_decode(const string& report)  
{

  const string methodName = _className + string("::_parse");


  // pull out the station name and valid time    
  string line;
  char eol = '\n';
  size_t where = report.find(eol);
  line.assign(report, 0, where);

  vector<string> tokens;
  TaStr::tokenize(line, " ", tokens);

  // the line may have 3 or four tokens, depending on whether the line starts with
  // 'MET REPORT' or 'SPECIAL.' Traverse from end to get name and time 

  size_t nToks = tokens.size();
  _name = tokens[nToks-2];

  _validTime = _setTime(tokens[nToks-1]);

  // should we check acceptedStations?  
  if (!_acceptStation()) {
    return 1;
  }

  return 0;    

}


/////////////////////////////////////////////////////////////////////////
// _setTime
//
time_t 
Metreport2Spdb::_setTime(const string& rtime)
{
  // compare the hhmmss time to _fileTime and create difference

  // turn btime into number of seconds since start of day
  int day = strtol(rtime.substr(0,2).c_str(), NULL, 10);
  int hour = strtol(rtime.substr(2,2).c_str(), NULL, 10);
  int minute = strtol(rtime.substr(4,2).c_str(), NULL, 10);
  int rTimeSecs = 24*3600*day + 3600*hour + 60*minute;

  // turn time part (not date) into number of seconds since start of day
  day = _fileTime.getDay();
  hour = _fileTime.getHour();
  minute = _fileTime.getMin();
  int fTimeSecs = 24*3600*day + 3600*hour + 60*minute;
  
  int deltaSec = fTimeSecs - rTimeSecs;

  // force seconds to zero

  

  return _fileTime.utime() - deltaSec;
    
}


/////////////////////////////////////////////////////////////////
// _acceptStation
//
bool 
Metreport2Spdb::_acceptStation()
  
{

  // should we check acceptedStations?
  
  bool storeIt = true;
  if (_params->use_accepted_stations_list) {
    bool accept = false;
    for (int ii = 0; ii < _params->accepted_stations_n; ii++) {
      if (_name == _params->_accepted_stations[ii]) {
        accept = true;
        break;
      }
    }
    if (!accept) {
      if (static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_VERBOSE)) {
        cerr << endl;
        cerr << "Rejecting station: " << _name << endl;
        cerr << "  Not in acceptedStations list" << endl;
      }
      storeIt = false;
    }
  }
  
  // should we check rejectedStations?
  
  if (_params->use_rejected_stations_list) {
    bool reject = false;
    for (int ii = 0; ii < _params->rejected_stations_n; ii++) {
      if (_name == _params->_rejected_stations[ii]) {
        reject = true;
        break;
      }
    }
    if (reject) {
      if (static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_VERBOSE)) {
        cerr << endl;
        cerr << "Rejecting station: " << _name << endl;
        cerr << "  Station name is in rejectedStations list" << endl;
      }
      storeIt = false;
    }
  }

  return storeIt;

}

