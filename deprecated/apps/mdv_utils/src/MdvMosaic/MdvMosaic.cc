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
// MdvMosaic.cc
//
// MdvMosaic object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1998
//
///////////////////////////////////////////////////////////////

#include "MdvMosaic.hh"
#include "Trigger.hh"
#include "InputFile.hh"
#include "OutputFile.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <rapmath/math_macros.h>
using namespace std;

// Constructor

MdvMosaic::MdvMosaic(int argc, char **argv)

{

  OK = TRUE;
  Done = FALSE;

  // set programe name

  _progName = STRdup("MdvMosaic");
  ucopyright(_progName);

  // get command line args

  _args = new Args(argc, argv, _progName);
  if (!_args->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }
  if (_args->Done) {
    Done = TRUE;
    return;
  }

  // get TDRP params

  Params *params = new Params(_args->paramsFilePath,
			      &_args->override,
			      _progName,
			      _args->checkParams,
			      _args->printParams,
			      _args->printShort);
  
  if (!params->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = FALSE;
    return;
  }
  if (params->Done) {
    Done = TRUE;
    return;
  }
  _params = &params->p;

  // check start and end in ARCHIVE mode

  if ((_params->mode == ARCHIVE) &&
      (_args->startTime == 0 || _args->endTime == 0)) {
    fprintf(stderr, "ERROR - %s\n", _progName);
    fprintf(stderr, "In ARCHIVE mode start and end times must be specified.\n");
    fprintf(stderr, "Run '%s -h' for usage\n", _progName);
    OK = FALSE;
  }

  if (!OK) {
    return;
  }

  // setup output grid params

  _loadGrid();

  // init process mapper registration

  PMU_auto_init(_progName, _params->instance, PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

MdvMosaic::~MdvMosaic()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  ufree(_locArray);
  delete _params;
  delete _args;
  STRfree(_progName);

}

//////////////////////////////////////////////////
// Run

int MdvMosaic::Run ()
{

  PMU_auto_register("MdvMosaic::Run");
  
  // create the trigger
  
  Trigger *trigger = _createTrigger();
  
  // create the input file objects

  int nInput = _params->input_dirs.len;
  InputFile **inFiles = new InputFile*[nInput];
  for (int i = 0; i < nInput; i++) {
    inFiles[i] = new InputFile(_progName, _params,
			       _params->input_dirs.val[i],
			       (i == 0),
			       &_outGrid, _locArray);
  }
  
  // create output file object

  OutputFile *outFile = new OutputFile(_progName, _params, &_outGrid);
  MDV_handle_t *outHandle = outFile->handle();
  int outInit = FALSE;
  
  // loop through times

  time_t triggerTime;

  while ((triggerTime = trigger->next()) >= 0) {
    
    if (_params->debug) {
      fprintf(stderr, "----> Trigger time: %s\n", utimstr(triggerTime));
    }
    
    // read relevant input files
    
    int dataAvail = FALSE;
    for (int input = 0; input < nInput; input++) {
      inFiles[input]->read(triggerTime);
      if (inFiles[input]->readSuccess) {
	dataAvail = TRUE;
	// on first successful read, initialize the output file headers
	if (!outInit) {
	  outFile->initHeaders(inFiles[input]->handle());
	  outInit = TRUE;
	}
      }
    }
    
    if (!dataAvail) {
      continue;
    }
    
    // clear vol
    outFile->clearVol();

    // set info string
    outFile->addToInfo("Merged data set from the following files:\n");
    for (int input = 0; input < nInput; input++) {
      if (inFiles[input]->readSuccess) {
	outFile->addToInfo(inFiles[input]->path());
	outFile->addToInfo("\n");
      }
    }

    // set the times

    time_t startTime = -1;
    time_t endTime = -1;
    for (int input = 0; input < nInput; input++) {
      if (inFiles[input]->readSuccess) {
	inFiles[input]->updateTimes(&startTime, &endTime);
      }
    } // input

    // loop through fields
    
    for (int out_field = 0; out_field < _params->field_list.len; out_field++) {

      int in_field = _params->field_list.val[out_field];
      
      // compute scale and bias - if error returned there was no data in
      // this field in any of the input files, so jump to end of loop

      double out_scale, out_bias;

      if (_computeScaleAndBias(in_field, nInput, inFiles,
			       &out_scale, &out_bias) == 0) {
	if (_params->debug) {
	  fprintf(stderr, "out_scale, out_bias: %g, %g\n", out_scale, out_bias);
	}
	outFile->loadScaleAndBias(out_field, out_scale, out_bias);
      } else {
	if (_params->debug) {
	  fprintf(stderr, "No data found, field %d\n", in_field);
	}
	outFile->loadScaleAndBias(out_field, 1.0, 0.0);
	continue;
      }
      
      // for each input file, merge the field data into the output grid

      for (int input = 0; input < nInput; input++) {
	if (inFiles[input]->readSuccess) {
	  inFiles[input]->mergeField(in_field,
				     outHandle, &_outGrid, out_field,
				     out_scale, out_bias);
	}
      } // input
      
    } // out_field

    // write out volume

    outFile->writeVol(triggerTime, startTime, endTime);

  } // while

  // free up

  delete (trigger);
  for (int i = 0; i < nInput; i++) {
    delete (inFiles[i]);
  }
  delete inFiles;
  delete outFile;

  return (0);

}

//////////////////////////////////////////////////
// _createTrigger

Trigger *MdvMosaic::_createTrigger()
{

  Trigger *trigger;

  // create the trigger

  if (_params->mode == REALTIME) {
    // REALTIME mode
    if (_params->trigger == TIME_TRIGGER) {
      trigger = new RealtimeTimeTrigger(_progName, _params);
    } else {
      trigger = new RealtimeFileTrigger(_progName, _params);
    }
  } else {
    // ARCHIVE mode
    if (_params->trigger == TIME_TRIGGER) {
      trigger = new ArchiveTimeTrigger(_progName, _params,
				       _args->startTime,  _args->endTime);
    } else {
      trigger = new ArchiveFileTrigger(_progName, _params,
				       _args->startTime, _args->endTime);
    }
  }

  return (trigger);

}

//////////////////////////////////////////////////
// _loadGrid

void MdvMosaic::_loadGrid()
{

  memset(&_outGrid, 0, sizeof(mdv_grid_t));

  // load up grid params

  _outGrid.nx = _params->output_grid.nx;
  _outGrid.ny = _params->output_grid.ny;
  _outGrid.nz = _params->output_grid.nz;
  _outGrid.minx = _params->output_grid.minx;
  _outGrid.miny = _params->output_grid.miny;
  _outGrid.minz = _params->output_grid.minz;
  _outGrid.dx = _params->output_grid.dx;
  _outGrid.dy = _params->output_grid.dy;
  _outGrid.dz = _params->output_grid.dz;

  if (_params->output_projection == OUTPUT_PROJ_FLAT) {
    _outGrid.proj_origin_lat = _params->output_origin.lat;
    _outGrid.proj_origin_lon = _params->output_origin.lon;
    _outGrid.proj_type = MDV_PROJ_FLAT;
  } else {
    _outGrid.proj_type = MDV_PROJ_LATLON;
  }

  // alloc location array for (lat, lon) pairs
  
  _locArray = (lat_lon_t *) umalloc(_outGrid.nx * _outGrid.ny * sizeof(lat_lon_t));
  
  // load up location array - the mdv_grid routines take care of
  // the projection geometry
  
  mdv_grid_comps_t comps;
  MDV_init_proj(&_outGrid, &comps);
  
  lat_lon_t *loc = _locArray;
  for (int iy = 0; iy < _outGrid.ny; iy++) {
    double yy = _outGrid.miny + _outGrid.dy * iy;
    for (int ix = 0; ix < _outGrid.nx; ix++, loc++) {
      double xx = _outGrid.minx + _outGrid.dx * ix;
      double lat, lon;
      MDV_xy2latlon(&comps, xx, yy, &lat, &lon);
      loc->lat = lat;
      loc->lon = lon;
    } // ix
  } // iy
  
}

//////////////////////////////////////////////////
// _computeScaleAndBias
//
// Compute scale and bias for a field.
//
// Returns 0 on success, -1 on failure.
//

int MdvMosaic::_computeScaleAndBias(int in_field,
				    int nInput, InputFile **inFiles,
				    double *scale_p, double *bias_p)

{
  
  // get min and max for field 
  
  double min_val = 1.0e99;
  double max_val = -1.0e99;
  
  for (int i = 0; i < nInput; i++) {
    if (inFiles[i]->readSuccess) {
      double file_min, file_max;
      if (inFiles[i]->getMinAndMax(in_field, &file_min, &file_max) == 0) {
	min_val = MIN(min_val, file_min);
	max_val = MAX(max_val, file_max);
      }
    }
  } // i
  
  if (min_val > max_val) {
    // no valid data found
    return (-1);
  }
  
  // compute scale and bias
  double range = max_val - min_val;
  double scale = range / 250.0;
  double bias = min_val - scale * 2.0;

  // assign a scale and bias

  if (_params->compute_scale_and_bias)
  {
    *scale_p = scale;
    *bias_p = bias;
  }
  else
  {
    *scale_p = _params->data_scale;
    *bias_p = _params->data_bias;
  }


  return (0);

}
