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
/**
 * @file ParmsMdvPlayback.cc
 */
#include "ParmsMdvPlayback.hh"
#include "Params.hh"
#include <toolsa/DateTime.hh>

//------------------------------------------------------------------
ParmsMdvPlayback::ParmsMdvPlayback(int argc, char **argv)
{
  // set things up to read in the parameters specific to this app:
  Params appParms;
  char *ppath;
  if (appParms.loadFromArgs(argc, argv, NULL, &ppath))
  {
    printf("ERROR loading parms");
    exit(1);
  }

  for (int i=0; i<appParms.inputData_n; ++i)
  {
    ParmData D(appParms._inputData[i].url, appParms._inputData[i].isObs,
	       appParms._inputData[i].useDataTime,
	       appParms._inputData[i].latencyHoursMax);
    pInput.push_back(D);
  }

  for (int i=0; i<appParms.path_n; ++i)
  {
    pair<string,string> p(appParms._path[i].input_path,
			  appParms._path[i].output_path);
    pInputOutputPath.push_back(p);
  }

  switch (appParms.mode)
  {
  case Params::PRE_PLAYBACK:
    pMode = ParmsMdvPlayback::PRE_PLAYBACK;
    break;
  case Params::PLAYBACK:
  default:
    pMode = ParmsMdvPlayback::PLAYBACK;
    break;
  }

  DateTime dt(appParms._start_time[0], appParms._start_time[1],
	      appParms._start_time[2], appParms._start_time[3],
	      appParms._start_time[4], appParms._start_time[5]);
  pTime0 = dt.utime();

  dt = DateTime(appParms._end_time[0], appParms._end_time[1],
		appParms._end_time[2], appParms._end_time[3],
		appParms._end_time[4], appParms._end_time[5]);
  pTime1 = dt.utime();
  pDebug = appParms.debug;
  pSpeedup = appParms.speedup;
  if (pSpeedup <= 0.0)
  {
    printf("ERROR speedup must be > 0\n");
    exit(1);
  }
  pShowRealtime = appParms.show_realtime;

  pSecondsDelayBeforeExit = 
    static_cast<int>(appParms.minutes_delay_before_exit*60.0);

  pInputSyncUrl = appParms.input_sync_url;
  for (int i=0; i<appParms.output_sync_url_n; ++i)
  {
    pOutputSyncUrl.push_back(appParms._output_sync_url[i]);
  }

  pMaxWaitSeconds = static_cast<int>(appParms.max_sync_wait_minutes*60.0);
  pSyncToTimeWritten = appParms.sync_to_time_written;
  pResolutionMinutes = appParms.resolution_minutes_sync;
  pDebugSync = appParms.debug_sync;
  pNumThreads = appParms.num_threads;
  pThreadDebug = appParms.thread_debug;
}

//------------------------------------------------------------------
ParmsMdvPlayback::~ParmsMdvPlayback()
{
}

