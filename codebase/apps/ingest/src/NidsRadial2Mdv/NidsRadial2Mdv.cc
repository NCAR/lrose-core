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
// NidsRadial2Mdv.cc
//
// NidsRadial2Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <didss/DsInputPath.hh>
#include <pthread.h>
#include "NidsRadial2Mdv.hh"
#include "Remap.hh"
#include "ThreadArgs.hh"
using namespace std;

// Constructor

NidsRadial2Mdv::NidsRadial2Mdv(int argc, char **argv) 

{

  isOK = true;

  // set programe name

  _progName = "NidsRadial2Mdv";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char*)"unknown";
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

NidsRadial2Mdv::~NidsRadial2Mdv()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int NidsRadial2Mdv::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  if (_params.mode == Params::REALTIME) {

    if (_runRealtime()) {
      return (-1);
    }
    
  } else {

    if (_runArchive()) {
      return (-1);
    }

  }

  return (0);

}

//////////////////////////////////////////////////
// _runRealtime

int NidsRadial2Mdv::_runRealtime ()
{

  // register with procmap
  
  PMU_auto_register("_runRealtime");
  
  // start a thread for each directory
  
  for (int idata = 0; idata < _params.realtime_data_sets_n; idata++) {
    
    // create args for thread - will be freed up by thread
    
    ThreadArgs *threadArgs = new ThreadArgs(_progName, _params, idata);
    
    if (_params.no_threads) {
      
      // call _runThread directly
      
      _runThread(threadArgs);
      
    } else {
      
      // spawn a thread for processing
      
      pthread_t thread;
      int iret;
      if ((iret = pthread_create(&thread, NULL,
				 _runThread, (void *) threadArgs))) {
	cerr << "ERROR - " << _progName << ":_runRealtime()." << endl;
	cerr << "  Cannot create thread." << endl;
	cerr << "  " << strerror(iret) << endl;
	delete (threadArgs);
	return (-1);
      }
      pthread_detach(thread);
      
    }

  } // idata

  // loop forever, registering with procmap

  bool forever = true;
  while (forever) {
    PMU_auto_register("_runRealtime");
    sleep(1);
  }
  
  return (0);

}

//////////////////////////////////////////////////
// _runArchive

int NidsRadial2Mdv::_runArchive ()
{

  // register with procmap
  
  PMU_auto_register("_runArchive");

  DsInputPath input((char *) _progName.c_str(),
		    _params.debug >= Params::DEBUG_VERBOSE,
		    _args.filePaths);

  string outputDir;
  if (_params.specify_individual_output_dirs) {
    outputDir = _params.archive_data_set.output_dir;
  } else {
    outputDir = _params.global_output_dir;
    outputDir += PATH_DELIM;
    outputDir += _params.archive_data_set.radar_name;
  }

  Remap remap(_progName, _params,
	      _params.archive_data_set.radar_name,
	      _params.archive_data_set.is_dbz, outputDir,
	      _params.use_specified_fieldname,
	      _params.specified_fieldname);

  char *filePath;
  while ((filePath = input.next()) != NULL) {
    if (remap.processFile(filePath)) {
      cerr << "WARNING - could not process file: " << filePath << endl;
    }
  }
  
  return (0);

}

//////////////////////////////////
// function for processing thread

void *NidsRadial2Mdv::_runThread(void *args)
  
{

  // args is owned by the thread, so make copy and
  // free up to avoid a leak
  
  ThreadArgs threadArgs = *((ThreadArgs *) args);
  delete (ThreadArgs *) args;
  
  int dataSetNum = threadArgs.dataSetNum;
  string inputDir =
    threadArgs.params._realtime_data_sets[dataSetNum].input_dir;
  string radarName =
    threadArgs.params._realtime_data_sets[dataSetNum].radar_name;
  bool isDbz =
    threadArgs.params._realtime_data_sets[dataSetNum].is_dbz;
  string outputDir;
  if (threadArgs.params.specify_individual_output_dirs) {
    outputDir = threadArgs.params._realtime_data_sets[dataSetNum].output_dir;
  } else {
    outputDir = threadArgs.params.global_output_dir;
    outputDir += PATH_DELIM;
    outputDir += threadArgs.params._realtime_data_sets[dataSetNum].radar_name;
  }

  bool use_specified_fieldname =
    threadArgs.params.use_specified_fieldname;

  char *specified_fieldname =
    threadArgs.params.specified_fieldname;

  if (threadArgs.params.debug) {
    cerr << "----> Starting _runThread for dir " << inputDir << endl;
  }

  DsInputPath input((char *) threadArgs.progName.c_str(),
		    threadArgs.params.debug >= Params::DEBUG_VERBOSE,
		    (char *) inputDir.c_str(),
		    threadArgs.params.max_realtime_data_age,
		    NULL,
		    threadArgs.params.use_latest_data_info,
		    threadArgs.params.get_latest_file_only);
  
  Remap remap(threadArgs.progName, threadArgs.params,
	      radarName, isDbz, outputDir, use_specified_fieldname,
	      specified_fieldname);

  char *file_path;
  while ((file_path = input.next()) != NULL) {
    if (remap.processFile(file_path)) {
      cerr << "WARNING - could not process file: " << file_path << endl;
    }
  }
   
  return (NULL);

}



