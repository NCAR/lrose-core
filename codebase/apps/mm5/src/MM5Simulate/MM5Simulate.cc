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
// MM5Simulate.cc
//
// MM5Simulate object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#include "MM5Simulate.hh"
#include "OutputFile.hh"
#include "Trigger.hh"
#include <mm5/MM5Data.hh>
#include <toolsa/pmu.h>
#include <toolsa/TaFile.hh>
using namespace std;

// Constructor

MM5Simulate::MM5Simulate(int argc, char **argv)

{

  OK = TRUE;
  
  // set programe name

  _progName = "MM5Simulate";
  ucopyright((char *) _progName.c_str());
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    fprintf(stderr, "ERROR: %s\n", _progName.c_str());
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			    &_paramsPath)) {
    fprintf(stderr, "ERROR: %s\n", _progName.c_str());
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = FALSE;
    return;
  }

  // check start and end in ARCHIVE mode
  
  if ((_params.mode == Params::ARCHIVE) &&
      (_args.startTime == 0 || _args.endTime == 0)) {
    fprintf(stderr, "ERROR - %s\n", _progName.c_str());
    fprintf(stderr,
	    "In ARCHIVE mode start and end times must be specified.\n");
    fprintf(stderr, "Run '%s -h' for usage\n", _progName.c_str());
    OK = FALSE;
  }

  if (!OK) {
    return;
  }

  // create the trigger
  
  if (_params.mode == Params::REALTIME) {
    _trigger = new RealtimeTrigger(_progName, _params);
  } else {
    _trigger = new ArchiveTrigger(_progName, _params,
				  _args.startTime,  _args.endTime);
  }
  
  // init process mapper registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL * 2);

  return;

}

// destructor

MM5Simulate::~MM5Simulate()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  delete(_trigger);

}

//////////////////////////////////////////////////
// Run

int MM5Simulate::Run ()
{

  PMU_auto_register("MM5Simulate::Run");
  
  // loop through times
  
  time_t triggerTime;
  
  while ((triggerTime = _trigger->next()) >= 0) {
    
    PMU_force_register("Start of trigger loop");
  
    time_t triggerWallclock = time(NULL);
    time_t wakeTime = triggerWallclock;
    
    for (int ipath = 0; ipath < _params.input_file_paths_n; ipath++) {
      
      PMU_force_register("Start of path loop");
  
      // get the file path
      
      string inputPath = _params._input_file_paths[ipath];
      
      if (_params.debug >= Params::DEBUG_NORM) {
	fprintf(stderr, "Processing input file %s\n", inputPath.c_str());
      }

      // read in the file

      PMU_auto_register("Reading in file");

      TaFile in;
      if (in.fopen(inputPath.c_str(), "rb") == NULL) {
	fprintf(stderr, "ERROR - %s:Run\n", _progName.c_str());
	fprintf(stderr, "  Cannot open input file: %s\n", inputPath.c_str());
	fprintf(stderr, "  Will skip this file and continue\n");
	perror(inputPath.c_str());
	continue;
	//return (-1);
      }
      in.fstat();
      int fileSize = in.getStat().st_size;

      void *fileBuf = umalloc(fileSize);
      if (in.fread(fileBuf, 1, fileSize) != fileSize) {
	fprintf(stderr, "ERROR - %s:Run\n", _progName.c_str());
	fprintf(stderr, "  Cannot read input file: %s\n", inputPath.c_str());
	perror(inputPath.c_str());
	ufree(fileBuf);
	return (-1);
      }
      in.fclose();
      
      // get the version number

      int version;
      if (MM5Data::getVersion(inputPath, version)) {
	fprintf(stderr, "Getting version number from file %s\n", inputPath.c_str());
	ufree(fileBuf);
	return -1;
      }

      PMU_auto_register("Resetting times");

      // reset the times

      int leadTime = ipath * _params.forecast_interval;

      if (_params.debug >= Params::DEBUG_NORM) {
	fprintf(stderr, " Model start time: %s\n", utimstr(triggerTime));
	fprintf(stderr, " Lead time: %d\n", leadTime);
      }
      
      if (version == 2) {
	if (_resetTimesV2(fileBuf, fileSize, triggerTime, leadTime)) {
	  fprintf(stderr, "ERROR - %s:Run\n", _progName.c_str());
	  fprintf(stderr, "  Bad input file: %s\n", inputPath.c_str());
	  ufree(fileBuf);
	  return (-1);
	}
      } else {
 	if (_resetTimesV3(fileBuf, fileSize, triggerTime, leadTime)) {
 	  fprintf(stderr, "ERROR - %s:Run\n", _progName.c_str());
 	  fprintf(stderr, "  Bad input file: %s\n", inputPath.c_str());
	  ufree(fileBuf);
 	  return (-1);
 	}
      }
      
      PMU_auto_register("Write out");

      OutputFile outFile(_progName, _params);
      outFile.writeDataset(inputPath, fileBuf, fileSize,
			   triggerTime, leadTime);
      
      // free up
      ufree(fileBuf);
      
      // 'wait' for model time-step to run
      
      PMU_auto_register("Waiting");

      if (_params.debug >= Params::DEBUG_NORM) {
	fprintf(stderr, "Waiting %d secs for next model run\n",
		_params.forecast_step_wallclock_duration);
      }
      
      wakeTime += _params.forecast_step_wallclock_duration;
      _sleepToTime(wakeTime);
	
    } // ipath

  } // while
  
  return (0);

}

/////////////////
// _sleepToTime()
//
// Sleep until the given wake time, registering while waiting
//

void MM5Simulate::_sleepToTime(time_t wake_time)

{

  time_t now = time(NULL);

  if (_params.debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "  time now is  %s\n", utimstr(now));
    fprintf(stderr, "  wake time is %s\n", utimstr(wake_time));
  }

  while (now < wake_time) {
    PMU_auto_register("In _sleepToTime() : Sleeping");
    sleep (2);
    now = time(NULL);
  }

  return;

}

/////////////////////////////////
// reset times in version 2 file

int MM5Simulate::_resetTimesV2(void *fileBuf,
			       int bufSize,
			       time_t modelTime,
			       int leadTime)

{

  if (bufSize < (int) (20000 * sizeof(si32))) {
    fprintf(stderr, "ERROR - %s:_resetTimesV2\n", _progName.c_str());
    fprintf(stderr, "  Input file too small\n");
    return -1;
  }

  date_time_t start, forecast;
  
  start.unix_time = modelTime;
  uconvert_from_utime(&start);
  forecast.unix_time = modelTime + leadTime;
  uconvert_from_utime(&forecast);
  
  si32 mdate = ((start.year % 100) * 1000000 +
		start.month * 10000 +
		start.day * 100 +
		start.hour);
  
  si32 istrtda = ((forecast.year % 100) * 1000000 +
		  forecast.month * 10000 +
		  forecast.day * 100 +
		  forecast.hour);

  si32 min = forecast.min;
  si32 hour = forecast.hour;
  si32 day = forecast.day;
  si32 month = forecast.month;
  si32 year = forecast.year % 100;
  si32 cent = forecast.year / 100;

  si32 *ibuf = (si32 *) fileBuf + 1; // skip Fort rec

  ibuf[1000] = BE_from_si32(mdate);
  ibuf[1003] = BE_from_si32(istrtda);
  ibuf[1020] = BE_from_si32(start.year);
  ibuf[1021] = BE_from_si32(start.month);
  ibuf[1022] = BE_from_si32(start.day);
  ibuf[1023] = BE_from_si32(start.hour);
  ibuf[1024] = BE_from_si32(start.min);
  ibuf[1030] = BE_from_si32(start.year);
  ibuf[1031] = BE_from_si32(start.month);
  ibuf[1032] = BE_from_si32(start.day);
  ibuf[1033] = BE_from_si32(start.hour);
  ibuf[1034] = BE_from_si32(start.min);

  ibuf[2000] = BE_from_si32(mdate);
  ibuf[2003] = BE_from_si32(istrtda);
  ibuf[2020] = BE_from_si32(start.year);
  ibuf[2021] = BE_from_si32(start.month);
  ibuf[2022] = BE_from_si32(start.day);
  ibuf[2023] = BE_from_si32(start.hour);
  ibuf[2024] = BE_from_si32(start.min);
  ibuf[2030] = BE_from_si32(start.year);
  ibuf[2031] = BE_from_si32(start.month);
  ibuf[2032] = BE_from_si32(start.day);
  ibuf[2033] = BE_from_si32(start.hour);
  ibuf[2034] = BE_from_si32(start.min);

  ibuf[4000] = BE_from_si32(mdate);
  ibuf[4003] = BE_from_si32(istrtda);

  ibuf[5000] = BE_from_si32(mdate);
  ibuf[5001] = BE_from_si32(istrtda);
  ibuf[5010] = BE_from_si32(min);
  ibuf[5011] = BE_from_si32(hour);
  ibuf[5012] = BE_from_si32(day);
  ibuf[5013] = BE_from_si32(month);
  ibuf[5014] = BE_from_si32(year);
  ibuf[5015] = BE_from_si32(cent);

  return 0;

}

/////////////////////////////////
// reset times in version 3 file

int MM5Simulate::_resetTimesV3(void *fileBuf,
			       int bufSize,
			       time_t modelTime,
			       int leadTime)

{

  if (bufSize < (int) (1400 * sizeof(si32))) {
    fprintf(stderr, "ERROR - %s:_resetTimesV3\n", _progName.c_str());
    fprintf(stderr, "  Input file too small\n");
    return -1;
  }

  date_time_t start, forecast;
  
  start.unix_time = modelTime;
  uconvert_from_utime(&start);
  forecast.unix_time = modelTime + leadTime;
  uconvert_from_utime(&forecast);
  
  // For integer array, skip 3 Fort recs plus 1 flag
  si32 *ibuf = (si32 *) fileBuf + 4;

  // regrid start time

  ibuf[54] = BE_from_si32(start.year);
  ibuf[55] = BE_from_si32(start.month);
  ibuf[56] = BE_from_si32(start.day);
  ibuf[57] = BE_from_si32(start.hour);
  ibuf[58] = BE_from_si32(start.min);
  ibuf[59] = BE_from_si32(start.sec);

  // little_r start time

  ibuf[104] = BE_from_si32(start.year);
  ibuf[105] = BE_from_si32(start.month);
  ibuf[106] = BE_from_si32(start.day);
  ibuf[107] = BE_from_si32(start.hour);
  ibuf[108] = BE_from_si32(start.min);
  ibuf[109] = BE_from_si32(start.sec);

  // interp start time

  ibuf[204] = BE_from_si32(start.year);
  ibuf[205] = BE_from_si32(start.month);
  ibuf[206] = BE_from_si32(start.day);
  ibuf[207] = BE_from_si32(start.hour);
  ibuf[208] = BE_from_si32(start.min);
  ibuf[209] = BE_from_si32(start.sec);

  // mm5 start time

  ibuf[504] = BE_from_si32(start.year);
  ibuf[505] = BE_from_si32(start.month);
  ibuf[506] = BE_from_si32(start.day);
  ibuf[507] = BE_from_si32(start.hour);
  ibuf[508] = BE_from_si32(start.min);
  ibuf[509] = BE_from_si32(start.sec);

  // for float array, skip 3 Fort recs plus 1 flag plus mif array
  fl32 *rbuf = (fl32 *) fileBuf + 1004;

  // forecast output interval (secs)

  rbuf[200] = _params.forecast_interval;
  BE_from_array_32(&rbuf[200], sizeof(fl32));

  // go through the buffer, looking for date/time strings

  char *cptr = (char *) fileBuf;
  int nsearch = bufSize / 4 - 10;
  for (int i = 0; i < nsearch; i++, cptr += 4) {
    if (cptr[4] == '-' &&
	cptr[7] == '-' &&
	cptr[10] == '_' &&
	cptr[13] == ':' &&
	cptr[16] == ':' &&
	cptr[19] == '.') {

      char datestr[32];
      memcpy(datestr, cptr, 24);
      datestr[24] = '\0';
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Orig date: " << datestr << endl;
      }

      // update the forecast time

      sprintf(datestr, "%.4d-%.2d-%.2d_%.2d:%.2d:%.2d.0000",
	      forecast.year,
	      forecast.month,
	      forecast.day,
	      forecast.hour,
	      forecast.min,
	      forecast.sec);

      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "New date: " << datestr << endl;
      }
      memcpy(cptr, datestr, 24);

      // update the forecast lead time, which occurs
      // before the current date in the header

      fl32 *flead = (fl32 *) (cptr - 12);
      *flead = (leadTime / 60.0);
      BE_from_array_32(flead, sizeof(fl32));

    }
  } 

  return 0;

}


