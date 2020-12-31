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
// NewNidsRadial2Mdv.cc
//
// NewNidsRadial2Mdv object
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
#include "NewNidsRadial2Mdv.hh"
#include "RemapRadial.hh"
#include "RemapRast.hh"
using namespace std;

// Constructor

NewNidsRadial2Mdv::NewNidsRadial2Mdv(int argc, char **argv) 

{

  isOK = true;

  // set programe name

  _progName = "NewNidsRadial2Mdv";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
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

NewNidsRadial2Mdv::~NewNidsRadial2Mdv()

{

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

int NewNidsRadial2Mdv::Run ()
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

int NewNidsRadial2Mdv::_runRealtime ()
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

  bool forever = true;
  while (forever) {
    PMU_auto_register("_runRealtime starting cycle");

    
    vector<Remap *>::iterator it;
    for (it = _remapObjects.begin(); it != _remapObjects.end(); it++) {
      Remap * curr = *it;
      string msg = "_runRealtime checking station " + curr->getRadarName();
      PMU_auto_register(msg.c_str());
      curr->checkNextFile();
    }

    // Todo: Put in logic to tell if we've run through the
    //       list recently.
    sleep(30);
  }
  
  return (0);

}

//////////////////////////////////////////////////
// _runArchive

int NewNidsRadial2Mdv::_runArchive ()

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

  cerr << "Debug is " << _params.debug << " verbose is " << Params::DEBUG_VERBOSE << " sending " << (_params.debug >= Params::DEBUG_VERBOSE) << endl;

  Remap * remap = NULL;
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

  remap->initInputPaths();

  // This blocks until all files are processed.
  remap->checkNextFile();

  delete remap;
  
  return (0);

}

//////////////////////////////////
// function for processing thread

int NewNidsRadial2Mdv::_createRemapObject(const string & inputDir,
                                          const string & radarName,
                                          const string & outputDir)
  
{

  cerr << "Debug is " << _params.debug << " verbose is " << Params::DEBUG_VERBOSE << " sending " << (_params.debug >= Params::DEBUG_VERBOSE) << endl;

  Remap * remap = NULL;
  if (_params.is_radial) {
    remap = new RemapRadial(_progName,
                            inputDir,
                            _params.archive_data_set.radar_name,
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
                          _params.archive_data_set.radar_name,
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

  remap->initInputPaths();

  _remapObjects.push_back(remap);

  return (0);

}



