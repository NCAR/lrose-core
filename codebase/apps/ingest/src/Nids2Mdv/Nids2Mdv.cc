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
// Nids2Mdv.cc
//
// Nids2Mdv object
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// After Mike Dixon
// March 2001
//
///////////////////////////////////////////////////////////////

#include <ctime>

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <didss/DsInputPath.hh>
#include <dsserver/DmapAccess.hh>
#include "Nids2Mdv.hh"
#include "RemapRadial.hh"
#include "RemapRast.hh"
using namespace std;

// Constructor

Nids2Mdv::Nids2Mdv(int argc, char **argv) :
          _postProcessFmq(NULL)
{

  isOK = true;

  // set programe name

  _progName = "Nids2Mdv";
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

  if (_params.postProcessOutput) {
    string url = _params.postProcessFmqUrl;
    _postProcessFmq = new DsFmq();
    if (_postProcessFmq->init(url.c_str(),
                              _progName.c_str(),
                              (_params.debug > 0), // Debug On?
                              DsFmq::READ_WRITE,   // Open Mode
                              DsFmq::END,          // Open Position
                              true,                // Compression?
                              20000,               // Num Slots
                              4000000,             // Max size, bytes
                              1000,                // Sleep 
                              NULL) != 0) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem initializing postProcessFmq." << endl;
      isOK = FALSE;
    }
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
                _params.instance,
                PROCMAP_REGISTER_INTERVAL * 10);

  return;

}

// destructor

Nids2Mdv::~Nids2Mdv()

{

  if (_postProcessFmq != NULL) {
    delete _postProcessFmq;
    _postProcessFmq = NULL;
  }

  // Delete all Remap objects.
  vector<Remap *>::iterator it;
  for (it = _remapObjects.begin(); it != _remapObjects.end(); it++) {
    Remap * curr = *it;
    delete curr;
  }
  _remapObjects.clear();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Nids2Mdv::Run ()
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

int Nids2Mdv::_runRealtime ()
{

  // register with procmap
  
  PMU_auto_register("_runRealtime");
  
  // start a thread for each directory
  
  for (int idata = 0; idata < _params.realtime_data_sets_n; idata++) {
    
    // Call createRemapObject(...) with the proper station info.

    string outputDir;
    if (_params.specify_individual_output_dirs) {
      outputDir = _params._realtime_data_sets[idata].output_dir;
    } else {
      outputDir = _params.global_output_dir;
      outputDir += PATH_DELIM;
      outputDir += _params._realtime_data_sets[idata].radar_name;
    }

    _createRemapObject(_params._realtime_data_sets[idata].input_dir,
                       _params._realtime_data_sets[idata].radar_name,  
                       outputDir);

  } // idata

  // loop forever, registering with procmap

  time_t prevCycleStart = 0;
  bool forever = true;
  int  num_processed = 0;

  while (forever) {
    PMU_auto_register("_runRealtime starting cycle");

    prevCycleStart = time(0);
	num_processed = 0; // Count number of files processed
    
    vector<Remap *>::iterator it;

    for (it = _remapObjects.begin(); it != _remapObjects.end(); it++) {
      Remap * curr = *it;

      string msg = "_runRealtime checking station " + curr->getRadarName();
      PMU_auto_register(msg.c_str());

      string nextFile; 
      int success = curr->fetchNextFile(nextFile); // Returns 1 if newer file.

      if (success == 1) {
        if (_params.debug > 0) {
          cerr << "Processing file " << nextFile << endl;
        }

        if (curr->processFile(nextFile)) {
          // Error processing the file. Ignore

        } else {  // Successfully processed

		  num_processed++;

          // Get date from output file name:
          //   /d1/adds/system/data/nws_nids/BREF1/KTLX/20110308/170520.mdv
          string outputFile = curr->getLastOutputFile();
          string dateString;

          // regex r("/[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]/[0-9][0-9][0-9][0-9][0-9][0-9].mdv");
          // int idx = outputFile.index(r);
          // string dateString = outputFile.chop(idx);

          size_t idx = 0;
          size_t prev_idx = 0;
          while ( (idx = outputFile.find('/', prev_idx + 1)) != string::npos ) {
            // cerr << "    Processing string: " << outputFile.substr(idx).c_str() << endl;
            prev_idx = idx;

            size_t idx2 = outputFile.find_first_not_of("0123456789", idx + 1);
            // Do we have an 8-digit number?
            if ( (idx2 - idx) == 9 ) {
              // cerr << "        Got 8-digit number!!!" << endl;
              dateString = outputFile.substr(idx+1);
              break;
            }
          }

          time_t latestTime = 0;
          if (dateString.length() > 8) {
            // cerr << "got dateString: " << dateString << endl;
            date_time_t ltime;
            int result = sscanf(dateString.c_str(),
                                "%04d%02d%02d/%02d%02d%02d.mdv",
                                &ltime.year, &ltime.month, &ltime.day,   
                                &ltime.hour, &ltime.min, &ltime.sec);
            if ( result == 6 ) {
              uconvert_to_utime(&ltime);
              latestTime = ltime.unix_time;
              // cerr << "got latestTime: " << latestTime << endl;
            }
            else {
              cerr << "Could not parse file time. Result = " << result << ". Year = " << ltime.year << ". Could not determine latestTime from file name: " << outputFile.c_str() << endl;
            }
          }

          // Register with the DataMapper, if desired.
          if ( _params.dmapDir != "" && latestTime != 0 ) {
            DmapAccess dm;
            dm.setRespectDataDir(FALSE);

            int status = dm.regLatestInfo(latestTime, _params.dmapDir, "mdv");
          }

		  if (_params.postProcessOutput) {
          
            // Schedule Post-processing of the file.
            char cmd[4096]; // Way more than MAX_PATH_LEN
            sprintf(cmd,
					"%s %s %s %s",
					_params.postProcessCommand,
					curr->getRadarName().c_str(),
					curr->getLastOutputFile().c_str(),
					_params.postProcessCmdArguments);

            if ( _params.debug > 0 ) {
              cout << "Writing command to FMQ (" 
                   << _postProcessFmq->getURL().getURLStr()
                   << "): "
                   << cmd << endl;
            }

            _postProcessFmq->writeMsg(101, 202, cmd, strlen(cmd));
          }

	    } // Is successfully processed

      }  // Is New

    }  // For each radar

	// Report on cycle time 
    if ( num_processed > 1 && _params.debug > 0) {

        time_t currTime = time(0);
        time_t cycleTime = currTime - prevCycleStart;
        cerr << "Cycle processed " << num_processed 
			 << " files in " << cycleTime << " seconds,"
			 << endl;
    }

	if(num_processed == 0) sleep(_params.realtime_sleep_secs);
  }
  
  return (0);

}

//////////////////////////////////////////////////
// _runArchive

int Nids2Mdv::_runArchive ()

{

  // register with procmap
  
  PMU_auto_register("_runArchive");

  string outputDir;
  if (_params.specify_individual_output_dirs) {
    outputDir = _params.archive_data_set.output_dir;
  } else {
    outputDir = _params.global_output_dir;
    outputDir += PATH_DELIM;
    outputDir += _params.archive_data_set.radar_name;
  }

  Remap * remap = NULL;
  if (_params.preserve_input_geometry) {
    if (_params.is_radial) {
      remap = new RemapRadial(_progName,
                              _args.filePaths,
                              _params.archive_data_set.radar_name,
                              outputDir,
                              _params.compute_scale_and_bias,
                              _params.data_scale,
                              _params.data_bias,
                              _params.debug > 0,
                              _params.debug >= Params::DEBUG_VERBOSE,
                              _params.data_field_name_long,
                              _params.data_field_name,
                              _params.data_units,
                              _params.data_transform,
                              _params.data_field_code,
                              _params.processing_delay);
    }
    else {
        remap = new RemapRast(_progName,
                              _args.filePaths,
                              _params.archive_data_set.radar_name,
                              outputDir,
                              _params.compute_scale_and_bias,
                              _params.data_scale,
                              _params.data_bias,
                              _params.debug > 0,
                              _params.debug >= Params::DEBUG_VERBOSE,
                              _params.data_field_name_long,
                              _params.data_field_name,
                              _params.data_units,
                              _params.data_transform,
                              _params.data_field_code,
                              _params.processing_delay);
    }
  }
  else { 
    // Specify the output geometry explicitly
    if (_params.is_radial) {
      remap = new RemapRadial(_progName,
                              _args.filePaths,
                              _params.archive_data_set.radar_name,
                              outputDir,
                              _params.compute_scale_and_bias,
                              _params.data_scale,
                              _params.data_bias,
                              _params.debug > 0,
                              _params.debug >= Params::DEBUG_VERBOSE,
                              _params.output_grid.nx,
                              _params.output_grid.ny,
                              _params.output_grid.dx,
                              _params.output_grid.dy,
                              _params.output_grid.minx,
                              _params.output_grid.miny,
                              _params.data_field_name_long,
                              _params.data_field_name,
                              _params.data_units,
                              _params.data_transform,
                              _params.data_field_code,
                              _params.processing_delay);
    }
    else {
        remap = new RemapRast(_progName,
                              _args.filePaths,
                              _params.archive_data_set.radar_name,
                              outputDir,
                              _params.compute_scale_and_bias,
                              _params.data_scale,
                              _params.data_bias,
                              _params.debug > 0,
                              _params.debug >= Params::DEBUG_VERBOSE,
                              _params.output_grid.nx,
                              _params.output_grid.ny,
                              _params.output_grid.dx,
                              _params.output_grid.dy,
                              _params.output_grid.minx,
                              _params.output_grid.miny,
                              _params.data_field_name_long,
                              _params.data_field_name,
                              _params.data_units,
                              _params.data_transform,
                              _params.data_field_code,
                              _params.processing_delay);
    }
  }

  remap->initInputPaths();

  int success;
  string nextFile;;
  while ( (success = remap->fetchNextFile(nextFile) ) == 1) {
    if (_params.debug > 0) {
      cerr << "Processing file " << nextFile << endl;
    }

    if (remap->processFile(nextFile)) {
      cerr << "WARNING - could not process file: " << nextFile << endl;
    }
  }

  delete remap;
  remap = NULL;
  
  return (0);

}

//////////////////////////////////
// function for processing thread

int Nids2Mdv::_createRemapObject(const string & inputDir,
                                 const string & radarName,
                                 const string & outputDir)
  
{

  Remap * remap = NULL;
  if (_params.preserve_input_geometry) {
    if (_params.is_radial) {
      remap = new RemapRadial(_progName,
                              inputDir,
                              radarName,
                              outputDir,
                              _params.max_realtime_data_age,
                              _params.use_latest_data_info,
                              _params.get_latest_file_only,
                              _params.compute_scale_and_bias,
                              _params.data_scale,
                              _params.data_bias,
                              _params.debug > 0,
                              _params.debug >= Params::DEBUG_VERBOSE,
                              _params.data_field_name_long,
                              _params.data_field_name,
                              _params.data_units,
                              _params.data_transform,
                              _params.data_field_code,
                              _params.processing_delay);
    }
    else {
        remap = new RemapRast(_progName,
                              inputDir,
                              radarName,
                              outputDir,
                              _params.max_realtime_data_age,
                              _params.use_latest_data_info,
                              _params.get_latest_file_only,
                              _params.compute_scale_and_bias,
                              _params.data_scale,
                              _params.data_bias,
                              _params.debug > 0,
                              _params.debug >= Params::DEBUG_VERBOSE,
                              _params.data_field_name_long,
                              _params.data_field_name,
                              _params.data_units,
                              _params.data_transform,
                              _params.data_field_code,
                              _params.processing_delay);
    }
  }
  else {
    // Specify the output geometry explicitly
    if (_params.is_radial) {
      remap = new RemapRadial(_progName,
                              inputDir,
                              radarName,
                              outputDir,
                              _params.max_realtime_data_age,
                              _params.use_latest_data_info,
                              _params.get_latest_file_only,
                              _params.compute_scale_and_bias,
                              _params.data_scale,
                              _params.data_bias,
                              _params.debug > 0,
                              _params.debug >= Params::DEBUG_VERBOSE,
                              _params.output_grid.nx,
                              _params.output_grid.ny,
                              _params.output_grid.dx,
                              _params.output_grid.dy,
                              _params.output_grid.minx,
                              _params.output_grid.miny,
                              _params.data_field_name_long,
                              _params.data_field_name,
                              _params.data_units,
                              _params.data_transform,
                              _params.data_field_code,
                              _params.processing_delay);
    }
    else {
        remap = new RemapRast(_progName,
                              inputDir,
                              radarName,
                              outputDir,
                              _params.max_realtime_data_age,
                              _params.use_latest_data_info,
                              _params.get_latest_file_only,
                              _params.compute_scale_and_bias,
                              _params.data_scale,
                              _params.data_bias,
                              _params.debug > 0,
                              _params.debug >= Params::DEBUG_VERBOSE,
                              _params.output_grid.nx,
                              _params.output_grid.ny,
                              _params.output_grid.dx,
                              _params.output_grid.dy,
                              _params.output_grid.minx,
                              _params.output_grid.miny,
                              _params.data_field_name_long,
                              _params.data_field_name,
                              _params.data_units,
                              _params.data_transform,
                              _params.data_field_code,
                              _params.processing_delay);
    }
  }

  remap->initInputPaths();

  _remapObjects.push_back(remap);

  return (0);

}



