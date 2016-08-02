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
// AcSpdb2Rapic.cc
//
// AcSpdb2Rapic object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2008
//
/////////////////////////////////////////////////////////////
//
// AcSpdb2Rapic reads aircraft data from SPDB, and reformats it
// for use in RAPIC. RAPIC is a radar display tool from the
// Australian Bureau of Meteorology
//
/////////////////////////////////////////////////////////////

#include <iomanip>
#include <cerrno>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/file_io.h>
#include <toolsa/pjg.h>
#include <toolsa/DateTime.hh>
#include <rapformats/ac_posn.h>
#include <rapformats/ac_data.h>
#include "AcSpdb2Rapic.hh"
using namespace std;

// Constructor

AcSpdb2Rapic::AcSpdb2Rapic(int argc, char **argv)
  
{

  isOK = true;

  // set programe name

  _progName = "AcSpdb2Rapic";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

AcSpdb2Rapic::~AcSpdb2Rapic()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int AcSpdb2Rapic::Run ()
{

  int iret = 0;
  if (_params.mode == Params::REALTIME) {
    iret = _runRealtime();
  } else {
    iret = _runArchive();
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode

int AcSpdb2Rapic::_runRealtime()
{
  
  int iret = 0;

  // register with procmap
  
  PMU_auto_register("Run");
  
  // initialize trigger times

  time_t now = time(NULL);
  int dailyInt = _params.daily_file_output_interval;
  int latestInt = _params.latest_file_output_interval;
  time_t nextTimeDaily = (now / dailyInt + 1) * dailyInt;
  time_t nextTimeLatest = (now / latestInt + 1) * latestInt;
    
  while (true) {
    
    PMU_auto_register("Waiting for trigger time ...");

    now = time(NULL);

    if (now >= nextTimeLatest) {
      if (_writeLatest(nextTimeLatest)) {
	iret = -1;
      }
      nextTimeLatest = (now / latestInt + 1) * latestInt;
    }
    
    if (now >= nextTimeDaily) {
      if (_writeDaily(nextTimeDaily)) {
	iret = -1;
      }
      nextTimeDaily = (now / dailyInt + 1) * dailyInt;
    }

    PMU_auto_register("Zzzzz ...");
    umsleep(1000);
    
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int AcSpdb2Rapic::_runArchive()
{
  
  int iret = 0;

  int startTime = _args.startTime;
  int endTime = _args.endTime;

  if (_params.debug) {
    cerr << "=========================================" << endl;
    cerr << "startTime: " << DateTime::strm(startTime) << endl;
    cerr << "endTime: " << DateTime::strm(endTime) << endl;
  }

  int startDay = startTime / SECS_IN_DAY;
  int endDay = endTime / SECS_IN_DAY;

  for (int iday = startDay; iday <= endDay; iday++) {
    _writeDailyFile(iday);
  }

  return iret;

}

//////////////////////////////////////////////////
// Write daily files

int AcSpdb2Rapic::_writeDaily(time_t refTime)
{
  
  int iret = 0;

  if (_params.debug) {
    cerr << "Writing daily files, ref time: " << DateTime::strm(refTime) << endl;
  }

  int dayNum = refTime / SECS_IN_DAY;
  _writeDailyFile(dayNum);

  return iret;

}

//////////////////////////////////////////////////
// Write latest files

int AcSpdb2Rapic::_writeLatest(time_t refTime)
{
  
  int iret = 0;

  time_t endTime = refTime;
  time_t startTime = refTime - _params.latest_file_len_secs;
  
  if (_params.debug) {
    cerr << "Writing latest files, start time, end time: "
	 << DateTime::strm(startTime) << ", "
	 << DateTime::strm(endTime) << endl;
  }

  if (_loadPosnData(startTime, endTime)) {
    return -1;
  }
  
  // write file for each callsign
  
  set<string>::iterator ip;
  for (set<string>::iterator ip = _callSigns.begin(); ip != _callSigns.end(); ip++) {
    string callsign = *ip;
    if (_writeFile(callsign, startTime, LATEST_FILE)) {
      iret = -1;
    }
  }

  return iret;
  
  return iret;

}


//////////////////////////////////////////////////
// Write daily file

int AcSpdb2Rapic::_writeDailyFile(int dayNum)
{

  int iret = 0;

  time_t startThisDay = dayNum * SECS_IN_DAY;
  time_t endThisDay = startThisDay + SECS_IN_DAY - 1;

  if (_params.mode == Params::ARCHIVE) {
    if (startThisDay < _args.startTime) {
      startThisDay = _args.startTime;
    }
    if (endThisDay > _args.endTime) {
      endThisDay = _args.endTime;
    }
  }
  
  if (_params.debug) {
    cerr << "===>> writing file for day number: " << dayNum << endl;
    cerr << "      startThisDay: " << DateTime::strm(startThisDay) << endl;
    cerr << "      endThisDay: " << DateTime::strm(endThisDay) << endl;
  }

  if (_loadPosnData(startThisDay, endThisDay)) {
    return -1;
  }

  // write file for each callsign

  set<string>::iterator ip;
  for (set<string>::iterator ip = _callSigns.begin(); ip != _callSigns.end(); ip++) {
    string callsign = *ip;
    if (_writeFile(callsign, startThisDay, DAILY_FILE)) {
      iret = -1;
    }
    if (_params.mode == Params::REALTIME) {
      if (_writeFile(callsign, startThisDay, TODAY_FILE)) {
	iret = -1;
      }
    }
  }

  return iret;
  
}


//////////////////////////////////////////////////
// Write daily file for given callsign

int AcSpdb2Rapic::_writeFile(const string &callsign, time_t startThisDay,
			     loc_file_type_t fileType)
{

  // create local array, filtering on callsign
  
  vector<acLocation_t> locs;
  for (int ii = 0; ii < (int) _locArray.size(); ii++) {
    const acLocation_t &loc = _locArray[ii];
    if (loc.callsign == callsign) {
      locs.push_back(loc);
    }
  }
  
  // compute heading
  
  for (int ii = 1; ii < (int) locs.size(); ii++) {
    double dist, heading;
    PJGLatLon2RTheta(locs[ii-1].lat, locs[ii-1].lon,
		     locs[ii].lat, locs[ii].lon,
		     &dist, &heading);
    if (heading < 0) {
      heading += 360.0;
    }
    locs[ii].computedHeading = heading;
  }
  if (locs.size() > 1) {
    locs[0].computedHeading = locs[1].computedHeading;
  }
  
  // compute output sub dir

  char callsignDir[1024];
  sprintf(callsignDir, "%s%s%s", _params.output_dir, PATH_DELIM, callsign.c_str());

  // make sure dir exists

  if (ta_makedir_recurse(callsignDir)) {
    int errNum = errno;
    cerr << "ERROR - AcSpdb2Rapic::_writeDailyFile" << endl;
    cerr << "  Cannot create output dir: " << callsignDir << endl;
    cerr << strerror(errNum);
    return -1;
  }

  // compute output path

  char outputPath[1024];

  if (fileType == DAILY_FILE) {
    DateTime daytime(startThisDay);
    sprintf(outputPath, "%s%s%.4d%.2d%.2d.%s.%s",
	    callsignDir, PATH_DELIM,
	    daytime.getYear(), daytime.getMonth(), daytime.getDay(),
	    callsign.c_str(), _params.output_file_ext);
  } else if (fileType == TODAY_FILE) {
    sprintf(outputPath, "%s%stoday.%s.%s",
	    callsignDir, PATH_DELIM,
	    callsign.c_str(), _params.output_file_ext);
  } else {
    sprintf(outputPath, "%s%slatest.%s.%s",
	    callsignDir, PATH_DELIM,
	    callsign.c_str(), _params.output_file_ext);
  }

  if (_params.debug) {
    cerr << "Writing file: " << outputPath << endl;
  }

  // open output file

  FILE *out;
  if ((out = fopen(outputPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - AcSpdb2Rapic::_writeDailyFile" << endl;
    cerr << "  Cannot open file: " << outputPath << endl;
    cerr << strerror(errNum);
    return -1;
  }

  // write locations to file

  for (int ii = 0; ii < (int) locs.size(); ii++) {
    const acLocation_t &loc = locs[ii];
    DateTime datatime(loc.time);
    double alt = loc.alt;
    if (_params.altitude_units == Params::ALTITUDE_KM) {
      alt *= 1000.0;
    } else if (_params.altitude_units == Params::ALTITUDE_FEET) {
      alt *= 0.3048;
    }
    double heading = loc.heading;
    if (heading < -999) {
      heading = loc.computedHeading;
    }
    fprintf(out, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2dZ %10.3f %10.3f %10.3f %10.3f\n",
	    datatime.getYear(), datatime.getMonth(), datatime.getDay(),
	    datatime.getHour(), datatime.getMin(), datatime.getSec(),
	    loc.lon, loc.lat, alt, heading);
  }

  // close file

  fclose(out);

  return 0;
  
}


//////////////////////////////////////////////////
// load position data

int AcSpdb2Rapic::_loadPosnData(time_t startTime, time_t endTime)
{

  int iret = 0;

  // retrieve ac_posn data

  DsSpdb spdb;
  if (spdb.getInterval(_params.ac_posn_spdb_url, startTime, endTime)) {
    cerr << "ERROR - AcSpdb2Rapic::_writeDailyFile" << endl;
    cerr << "  Calling getInterval for url: " << _params.ac_posn_spdb_url << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }

  // load up position vector
  
  _locArray.clear();
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();

  for (int ii = 0; ii < (int) chunks.size(); ii++) {

    const Spdb::chunk_t &chunk = chunks[ii];
    switch (spdb.getProdId()) {
      
    case SPDB_AC_POSN_ID :
      if (chunk.data_type2 ==  SPDB_AC_POSN_WMOD_ID) {
	load_ac_posn_wmod(chunk);
      } else {
	load_ac_posn(chunk);
      }
      break;
     
    case SPDB_AC_POSN_WMOD_ID :
      load_ac_posn_wmod(chunk);
      break;
      
    case SPDB_AC_DATA_ID :
      load_ac_data(chunk);
      break;
      
    }

  } // ii

  // load up the set of callsigns
  
  _callSigns.clear();
  for (int ii = 0; ii < (int) _locArray.size(); ii++) {
    _callSigns.insert(_locArray[ii].callsign);
  }

  if (_params.debug) {
    set<string>::iterator ip;
    for (set<string>::iterator ip = _callSigns.begin(); ip != _callSigns.end(); ip++) {
      cerr << "found callsign: " << *ip << endl;
    }
  }

  return iret;
  
}

///////////////////////////////////////////////////////////////
// load from ac_posn_t

void AcSpdb2Rapic::load_ac_posn(const Spdb::chunk_t &chunk)
  
{
  
  ac_posn_t *ac_posn = (ac_posn_t *) chunk.data;
  int num_posn = chunk.len / sizeof(ac_posn_t);
  
  for (int ii = 0; ii < num_posn; ii++) {
    
    ac_posn_t posn = ac_posn[ii];
    BE_to_ac_posn(&posn);

    acLocation_t loc;
    loc.callsign = posn.callsign;
    loc.time = chunk.valid_time;
    loc.lat = posn.lat;
    loc.lon = posn.lon;
    loc.alt = posn.alt;
    loc.heading = -999.9;
    loc.computedHeading = -999.9;

    _locArray.push_back(loc);

  } // ii
  
}

///////////////////////////////////////////////////////////////
// load from ac_posn_wmod_t
 
void AcSpdb2Rapic::load_ac_posn_wmod(const Spdb::chunk_t &chunk)
{

  ac_posn_wmod_t *ac_posn_data = (ac_posn_wmod_t *) chunk.data;
  int num_posn = chunk.len / sizeof(ac_posn_wmod_t);
  
  for (int ii = 0; ii < num_posn; ii++) {
    
    ac_posn_wmod_t posn = ac_posn_data[ii];
    BE_to_ac_posn_wmod(&posn);

    acLocation_t loc;
    loc.callsign = posn.callsign;
    loc.time = chunk.valid_time;
    loc.lat = posn.lat;
    loc.lon = posn.lon;
    loc.alt = posn.alt;
    loc.heading = -999.9;
    loc.computedHeading = -999.9;

    _locArray.push_back(loc);

  } // ii
  
}

///////////////////////////////////////////////////////////////
// load from ac_data_t

void AcSpdb2Rapic::load_ac_data(const Spdb::chunk_t &chunk)
  
{
  
  ac_data_t *ac_data = (ac_data_t *) chunk.data;
  int num_posn = chunk.len / sizeof(ac_data_t);
  
  for (int ii = 0; ii < num_posn; ii++) {
    
    ac_data_t posn = ac_data[ii];
    ac_data_from_BE(&posn);

    acLocation_t loc;
    loc.callsign = posn.callsign;
    loc.time = chunk.valid_time;
    loc.lat = posn.lat;
    loc.lon = posn.lon;
    loc.alt = posn.alt;
    loc.heading = posn.heading;
    loc.computedHeading = -999.9;
    
    _locArray.push_back(loc);

  } // ii
  
}

