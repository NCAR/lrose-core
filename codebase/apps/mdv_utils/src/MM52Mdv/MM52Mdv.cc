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
// MM52Mdv.cc
//
// MM52Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#include "MM52Mdv.hh"
#include "InputPath.hh"
#include "InputFile.hh"
#include "OutputFile.hh"
#include "FlightLevel.hh"
#include "ModelGrid.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <mdv/mdv_grid.h>
#include <mdv/mdv_handle.h>
using namespace std;

// Constructor

MM52Mdv::MM52Mdv(int argc, char **argv)

{

  OK = TRUE;

  // set programe name

  _progName = STRdup("MM52Mdv");
  ucopyright(_progName);

  // get command line args

  _args = new Args(argc, argv, _progName);
  if (!_args->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }

  // get TDRP params

  _params = new Params();
  _paramsPath = "unknown";
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &_paramsPath)) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = FALSE;
    return;
  }

  // create flight level object
  
  _fLevel = new FlightLevel(_progName, _params);
  if (!_fLevel->OK) {
    OK = FALSE;
  }

  // create output file object

  _outFile = new OutputFile(_progName, _params);
  
  // initialize the ldata handle

  LDATA_init_handle(&_ldata, _progName, _params->debug);

  // init process mapper registration

  if (_params->mode == Params::REALTIME) {
    PMU_auto_init(_progName, _params->instance, PROCMAP_REGISTER_INTERVAL);
    PMU_auto_register("In MM52Mdv constructor");
  }

  return;

}

// destructor

MM52Mdv::~MM52Mdv()

{

  // free up LDATA handle

  LDATA_free_handle(&_ldata);

  // unregister process

  PMU_auto_unregister();

  // free up

  delete(_outFile);
  delete(_fLevel);
  delete(_params);
  delete(_args);
  STRfree(_progName);

}

//////////////////////////////////////////////////
// Run

int MM52Mdv::Run ()
{

  PMU_auto_register("MM52Mdv::Run");

  int iret;

  if (_params->mode == Params::REALTIME) {
    iret = _runRealtime();
  } else {
    iret = _runArchive();
  }

  if (iret) {
    return (-1);
  } else {
    return (0);
  }

}

//////////////////////////////////////////////////
// _runRealtime

int MM52Mdv::_runRealtime ()
{

  PMU_auto_register("MM52Mdv::_runRealtime");

  char filePath[MAX_PATH_LEN];
  sprintf(filePath, "%s%s%s", _params->realtime_input_dir,
	  PATH_DELIM, _params->realtime_input_file_name);

  int forever = TRUE;

  while (forever) {
    
    LDATA_info_read_blocking(&_ldata, _params->realtime_input_dir,
			     _params->max_realtime_valid_age, 1000,
			     PMU_auto_register);
    
    if (_params->debug >= Params::DEBUG_NORM) {
      fprintf(stderr, "Reading input file %s\n", filePath);
    }
    
    _inFile = new InputFile(_progName, _params, filePath);
    
    if (_inFile->OK) {
      PMU_auto_register("MM52Mdv::_runRealtime");
      _inFile->readHeaders();
      int dtime = _ldata.info.latest_time - _inFile->forecastTimes[0];
      if (dtime >= _params->min_forecast_dtime) {
	_processTime(_ldata.info.latest_time);
      }
    }
    
    delete (_inFile);

  } // while (forever)

  return (0);

}

//////////////////////////////////////////////////
// _runArchive

int MM52Mdv::_runArchive ()

{

  PMU_auto_register("MM52Mdv::_runArchive");

  // create input path object

  InputPath *path = new InputPath (_progName, _params->debug,
				     _args->nFiles, _args->filePaths);

  // loop through the input files

  char *filePath;
  while ((filePath = path->next()) != NULL) {

    if (_params->debug >= Params::DEBUG_NORM) {
      fprintf(stderr, "Processing file %s\n", filePath);
    }

    _inFile = new InputFile(_progName, _params, filePath);
    
    if (_params->find_headers) {
      
      _inFile->findHdrRecords();
      
    } else {
      
      if (_inFile->OK) {
	_inFile->readHeaders();
	for (int i = 0; i < _inFile->nForecasts; i++) {
	  _processTime(_inFile->forecastTimes[i]);
	}
      }

    }

    delete (_inFile);

  }

  // clean up

  delete (path);
    
  return (0);

}

/////////////////////////////////////////
// _processTime()
//
// Process data for a given forecast time

void MM52Mdv::_processTime(time_t forecast_time)

{

  double startTime, endTime;

  PMU_auto_register("In MM52Mdv::_processTime");

  // read in dataset
  
  startTime = (double) clock() / CLOCKS_PER_SEC;

  if (_inFile->readDataset(forecast_time)) {
    fprintf(stderr, "ERROR\n");
  }

  endTime = (double) clock() / CLOCKS_PER_SEC;
  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "Reading dataset took %g CPU secs\n",
	    endTime - startTime);
  }
  
  // set up grid remapping object
  
  _mGrid = new ModelGrid(_progName, _params,
			 _inFile->nLat, _inFile->nLon,
			 _inFile->lat, _inFile->lon);
  
  // initialize the output grid computations
  
  mdv_grid_comps_t gridComps;
  if (_params->output_projection == Params::OUTPUT_PROJ_FLAT) {
    MDV_init_flat(_params->output_origin.lat,
		  _params->output_origin.lon,
		  0.0, &gridComps);
  } else {
    MDV_init_latlon(&gridComps);
  }

  // set scale and bias for freezing level
  double scale, bias;
  if (_inFile->get2dScaleBias("FZLevel", _inFile->freeze,
			      &scale, &bias) == 0) {
    _outFile->setScaleAndBias("FZLevel", scale, bias);
  }
  
  ui08 ***fieldPlanes = _outFile->fieldPlanes();
  
  // loop through the grid

  startTime = (double) clock() / CLOCKS_PER_SEC;

  Params::output_grid_t *oGrid = &_params->output_grid;
  
  for (int iy = 0; iy < oGrid->ny; iy++) {
    
    PMU_auto_register("In MM52Mdv::_processTime loop");

    for (int ix = 0; ix < oGrid->nx; ix++) {

      // compute x and y, and plane offset
      
      double yy = oGrid->miny + iy * oGrid->dy;
      double xx = oGrid->minx + ix * oGrid->dx;
      int planeOffset = iy * oGrid->nx + ix;
      
      // compute latlon
      
      double lat, lon;
      MDV_xy2latlon(&gridComps, xx, yy, &lat, &lon);
      
      // find the model position for this point
      
      // fprintf(stderr, "grid point [%d][%d]: (%g, %g)\n",
      // iy, ix, lat, lon);
      
      if (_mGrid->findModelLoc(lat, lon) == 0) {

	// fprintf(stderr, "--> success, %g, %g, %d, %d\n",
	// lat, lon, _mGrid->latIndex, _mGrid->lonIndex);
	
	// load up interpolated sigma pressure array for this point
	
	double *presSigma =
	  _inFile->interp3dField(_mGrid->latIndex, _mGrid->lonIndex,
				 "press", _inFile->pres,
				 _mGrid->wtSW, _mGrid->wtNW,
				 _mGrid->wtNE, _mGrid->wtSE);
	
	// load up the sigma interpolation array
	
	_fLevel->prepareInterp(_inFile->nSigma, presSigma);
	
	// interp the fields for this point

	for (int ifield = 0; ifield < _params->output_fields_n; ifield++) {

	  double scale = _outFile->handle()->fld_hdrs[ifield].scale;
	  double bias  = _outFile->handle()->fld_hdrs[ifield].bias;
	  ui08 **planes = fieldPlanes[ifield];
	  
	  switch (_params->_output_fields[ifield]) {
	    
	  case Params::U_FIELD:
	    _interp3dField("U", _inFile->uu,
			   planes, planeOffset, scale, bias);
	    break;
	    
	  case Params::V_FIELD:
	    _interp3dField("V", _inFile->vv,
			   planes, planeOffset, scale, bias);
	    break;
	    
	  case Params::W_FIELD:
	    _interp3dField("W", _inFile->ww,
			   planes, planeOffset, scale, bias);
	    break;
	    
	  case Params::WSPD_FIELD:
	    _interp3dField("wspd", _inFile->wspd,
			   planes, planeOffset, scale, bias);
	    break;
	    
	  case Params::TEMP_FIELD:
	    _interp3dField("temp", _inFile->tc,
			   planes, planeOffset, scale, bias);
	    break;
	    
	  case Params::HUMIDITY_FIELD:
	    _interp3dField("rh", _inFile->rh,
			   planes, planeOffset, scale, bias);
	    break;
	    
	  case Params::CLOUD_FIELD:
	    _interp3dField("cloud", _inFile->cloud,
			   planes, planeOffset, scale, bias);
	    break;
	    
	  case Params::PRECIP_FIELD:
	    _interp3dField("precip", _inFile->precip,
			   planes, planeOffset, scale, bias);
	    break;
	    
	  case Params::ICING_FIELD:
	    _interp3dField("icing", _inFile->icing,
			   planes, planeOffset, scale, bias);
	    break;
	    
	  case Params::TURB_FIELD:
	    _interp3dField("turb", _inFile->turb,
			   planes, planeOffset, scale, bias);
	    break;

	  case Params::FZLEVEL_FIELD:
	    _interp2dField("fzlevel", _inFile->freeze,
			   planes, planeOffset, scale, bias);
	    break;
	    
	  } // switch

	} // ifield
	  
      } else {
	
	// printf(stderr, "------------------------> failure\n");

      }
      
    } // ix
    
  } // iy

  endTime = (double) clock() / CLOCKS_PER_SEC;

  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "Interpolation took %g CPU secs\n", endTime - startTime);
  }

  // write out volume

  if (_outFile->writeVol(forecast_time, forecast_time,
			 forecast_time + _inFile->forecastDelta)) {
    fprintf(stderr, "Cannot write output for time %s\n",
	    utimstr(forecast_time));
  }

  // clean up

  delete (_mGrid);

}

//////////////////
// _interp3dField()
//
// Interpolate the 3d field data onto the output grid point
//

void MM52Mdv::_interp3dField(char *field_name,
			       fl32 ***field_data,
			       ui08 **planes,
			       int planeOffset,
			       double scale,
			       double bias)
  
{

  // load up sigma array for this point
  
  double *sigmaData =
    _inFile->interp3dField(_mGrid->latIndex, _mGrid->lonIndex,
			   field_name, field_data,
			   _mGrid->wtSW, _mGrid->wtNW,
			   _mGrid->wtNE, _mGrid->wtSE,
			   _fLevel->sigmaNeeded);
  
  // interpolate field onto flight levels
  
  double *flevelData = _fLevel->doInterp(sigmaData);
  
  //   fprintf(stderr, "----> Field %s\n", field_name);
  //   for (int i = 0; i < _fLevel->nLevels; i++) {
  //     fprintf(stderr, "%g ", flevelData[i]);
  //   }
  //   fprintf(stderr, "\n");

  // scale and put into field planes
  
  for (int i = 0; i < _fLevel->nLevels; i++) {
    int scaled;
    if (flevelData[i] == MISSING_DOUBLE) {
      scaled = 0;
    } else {
      scaled = (int) ((flevelData[i] - bias) / scale + 0.5);
      if (scaled < 1) {
	scaled = 1;
      } else if (scaled > 255) {
	scaled = 255;
      }
    }
    planes[i][planeOffset] = (ui08) scaled;
  }
  
}

//////////////////
// _interp2dField()
//
// Interpolate the 2d field data onto the output grid point
//

void MM52Mdv::_interp2dField(char *field_name,
			       fl32 **field_data,
			       ui08 **planes,
			       int planeOffset,
			       double scale,
			       double bias)
  
{

  // get interp value for point
  
  double interpVal =
    _inFile->interp2dField(_mGrid->latIndex, _mGrid->lonIndex,
			   field_name, field_data,
			   _mGrid->wtSW, _mGrid->wtNW,
			   _mGrid->wtNE, _mGrid->wtSE);
  
  int scaled;
  if (interpVal == MISSING_DOUBLE) {
    scaled = 0;
  } else {
    scaled = (int) ((interpVal - bias) / scale + 0.5);
    if (scaled < 1) {
      scaled = 1;
    } else if (scaled > 255) {
      scaled = 255;
    }
  }
  planes[0][planeOffset] = (ui08) scaled;
  
}

