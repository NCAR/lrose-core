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
// MM5Ingest.cc
//
// MM5Ingest object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#include "MM5Ingest.hh"
#include "InputPath.hh"
#include "OutputFile.hh"
#include <mm5/MM5DataV2.hh>
#include <mm5/MM5DataV3.hh>
#include <mm5/ItfaIndices.hh>
#include <toolsa/pmu.h>
#include <toolsa/pjg.h>
#include <toolsa/Path.hh>
#include <toolsa/toolsa_macros.h>
#include <Mdv/MdvxField.hh>
using namespace std;

// Constructor

MM5Ingest::MM5Ingest(int argc, char **argv)

{

  OK = TRUE;

  // set programe name

  _progName = "MM5Ingest";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    fprintf(stderr, "ERROR: %s\n", _progName.c_str());
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }

  // get TDRP params

  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath)) {
    fprintf(stderr, "ERROR: %s\n", _progName.c_str());
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = FALSE;
    return;
  }

  // make sure START_2D_FIELDS is not selected

  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    if (_params._output_fields[ifield].name == Params::START_2D_FIELDS) {
      fprintf(stderr, "ERROR: %s\n", _progName.c_str());
      fprintf(stderr, "  Do not select START_2D_FIELDS.\n");
      fprintf(stderr, "  This is only a marker.\n");
      OK = FALSE;
      return;
    }
  }

  // make sure CORNER fields are only selected for NATIVE projection

  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    if (_params._output_fields[ifield].name == Params::U_CORNER_FIELD ||
	_params._output_fields[ifield].name == Params::V_CORNER_FIELD) {
      if (_params.output_projection != Params::OUTPUT_PROJ_NATIVE) {
	fprintf(stderr, "ERROR: %s\n", _progName.c_str());
	fprintf(stderr, "  Output projection is not NATIVE.\n");
	fprintf(stderr, "  Therefore you cannot select U_CORNER or V_CORNER output fields.\n");
	OK = FALSE;
	return;
      }
    }
  } // ifield

  // set up sigma level interpolation

  switch (_params.output_levels) {

  case Params::FLIGHT_LEVELS: {
    double *levels = new double[_params.flight_levels_n];
    for (int i = 0; i < _params.flight_levels_n; i++) {
      levels[i] = _params._flight_levels[i];
    }
    _sigmaInterp.setFlightLevels(_params.flight_levels_n, levels);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      _sigmaInterp.printPressureArray(cerr);
    }
    delete[] levels;
    break;
  }

  case Params::PRESSURE_LEVELS: {
    double *levels = new double[_params.pressure_levels_n];
    for (int i = 0; i < _params.pressure_levels_n; i++) {
      levels[i] = _params._pressure_levels[i];
    }
    _sigmaInterp.setPressureLevels(_params.pressure_levels_n, levels);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      _sigmaInterp.printPressureArray(cerr);
    }
    delete[] levels;
    break;
  }

  case Params::HEIGHT_LEVELS: {
    double *levels = new double[_params.height_levels_n];
    for (int i = 0; i < _params.height_levels_n; i++) {
      levels[i] = _params._height_levels[i] * 1000.0;
    }
    _sigmaInterp.setHeightLevels(_params.height_levels_n, levels);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      _sigmaInterp.printPressureArray(cerr);
    }
    delete[] levels;
    break;
  }

  default: {}

  } // switch


  // init process mapper registration

  if (_params.mode == Params::REALTIME) {
    PMU_auto_init((char *) _progName.c_str(), _params.instance,
		  PROCMAP_REGISTER_INTERVAL);
    cout << "started procmap with progname: " << _progName.c_str()
         << " and instance: " << _params.instance << endl;

    PMU_auto_register("In MM5Ingest constructor");
  }

  return;

}

// destructor

MM5Ingest::~MM5Ingest()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MM5Ingest::Run ()
{

  PMU_auto_register("MM5Ingest::Run");

  // check the parameters

  if (_checkParams()) {
    return -1;
  }

  // create input path object

  InputPath *inputPath;
  if (_params.mode == Params::REALTIME) {
    inputPath = new InputPath(_progName, _params,
			      PMU_auto_register);
  } else {
    inputPath = new InputPath(_progName, _params,
			      _args.nFiles, _args.filePaths);
  }

  // run

  int iret = _run(*inputPath);

  // clean up

  delete(inputPath);

  if (iret) {
    return (-1);
  } else {
    return (0);
  }

}

//////////////////////////////////////////////////
// check the parameters
//
// returns 0 on success, -1 on failure

int MM5Ingest::_checkParams()

{

  int iret = 0;

  for (int i = 0; i < _params.itfa_derived_indices_n; i++) {

    int min_fl = _params._itfa_derived_indices[i].min_flight_level;
    int max_fl = _params._itfa_derived_indices[i].max_flight_level;

    if (min_fl > max_fl) {
      cerr << "TDRP ERROR" << endl;
      cerr << "  Min flight level must no exceed max flight level" << endl;
      cerr << "  Param: itfa_derived_indices[]" << endl;
      cerr << "    i: " << i << endl;
      iret = -1;
    }

  } // i

  for (int i = 0; i < _params.itfa_model_indices_n; i++) {

    int min_fl = _params._itfa_model_indices[i].min_flight_level;
    int max_fl = _params._itfa_model_indices[i].max_flight_level;

    if (min_fl > max_fl) {
      cerr << "TDRP ERROR" << endl;
      cerr << "  Min flight level must no exceed max flight level" << endl;
      cerr << "  Param: itfa_model_indices[]" << endl;
      cerr << "    i: " << i << endl;
      iret = -1;
    }

  } // i

  return iret;

}

//////////////////////////////////////////////////
// _run

int MM5Ingest::_run(InputPath &input_path)

{

  int iret = 0;

  PMU_auto_register("MM5Ingest::_run");

  // loop through the input files

  string filePath;
  while ((filePath = input_path.next()) != "") {
    Path path(filePath);

    string base = path.getBase();
    if (_params.debug >= Params::DEBUG_NORM) {
      fprintf(stderr, "File base name = <%s>\n", base.c_str());
    }

    int numtokens;
    int year, month, day, hour;
    int min = 0;
    int sec = 0;

    time_t gen_time = 0;

    if(_params.use_filename_for_gen_time || _params.use_parent_path_for_gen_time) {
      if (_params.debug >= Params::DEBUG_NORM) {
        if(_params.use_filename_for_gen_time) {
          cerr << "Using filename date and time for gen_time" << endl;
        }
        else {
          cerr << "Using date and time in parent path for gen_time" << endl;
        }
      }

      string gen_time_string = base.c_str();
      if(_params.use_parent_path_for_gen_time) {
        Path parent(path.getDirectory());
        gen_time_string = parent.getBase();
      }
      numtokens = sscanf(gen_time_string.c_str(), "%4d%2d%2d%2d",
                      &year,&month,&day,&hour);
      if(numtokens != 4) {
        fprintf(stderr, "ERROR - Filename does not include needed date string\n");
        return -1;
      }

      if (_params.debug >= Params::DEBUG_NORM) {
        cerr << "Year = " << year << endl;
	    cerr << "Month = " << month << endl;
	    cerr << "day = " << day << endl;
        cerr << "hour = " << hour << endl;
      }

      DateTime datetime(year, month, day, hour, min, sec);

      // Define the model generation time using time from model output filename
      gen_time = datetime.utime();
    }


    if (_params.debug >= Params::DEBUG_NORM) {
      fprintf(stderr, "Processing file %s\n", filePath.c_str());
    }

    int version;
    if (MM5Data::getVersion(filePath, version)) {
      fprintf(stderr, "Getting version number from file %s\n",
	      filePath.c_str());
      fflush(stderr);
      iret = -1;
      continue;
    }

    MM5Data *inData;
    if (version == 2) {
      inData = new MM5DataV2(_progName,
			     filePath,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     PMU_auto_register,
			     _params.dbzConstantIntercepts);
    } else {
      inData = new MM5DataV3(_progName,
			     filePath,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     PMU_auto_register,
			     _params.dbzConstantIntercepts);
    }

    while (inData->more()) {
      if (_processInData(*inData, gen_time)) {
	fprintf(stderr, "ERROR - MM5Ingest::_run\n");
	fprintf(stderr, "  Processing file: %s\n", filePath.c_str());
	umsleep(2);
	break;
      }
    }

    delete inData;
    fflush(stderr);
  } // while

  return iret;

}

/////////////////////////////////////////
// _processInData()
//
// Process input data
//
// Returns 0 on success, -1 on failure

int MM5Ingest::_processInData(MM5Data &inData,
			      time_t gen_time)

{

  if (!inData.OK) {
    return -1;
  }

  // read in data set

  if (inData.read()) {
    return -1;
  }

  // check projection
  // native proj only OK for Lambert, stereographic, and Mercator

  if (_params.output_projection == Params::OUTPUT_PROJ_NATIVE) {
    if (inData.proj_type != MM5Data::LAMBERT_CONF &&
	inData.proj_type != MM5Data::STEREOGRAPHIC &&
        inData.proj_type != MM5Data::MERCATOR) {
      cerr << "ERROR - MM5Ingest::_run" << endl;
      cerr << "  Output projection is set to NATIVE." << endl;
      cerr << "  Input MM5 data file has projection UNKNOWN: "
	   << inData.proj_type << endl;
      cerr << "  Only Lambert, stereographic, and Mercator projections "
	   << "are supported." << endl;
      return -1;
    }
  }

  // round the lead_time if desired, and compute forecast time

  int lead_time = inData.forecastLeadTime;
  int rounded_lead_time = lead_time;
  int lead_time_offset = 0;

  if( _params.use_filename_for_gen_time || _params.use_parent_path_for_gen_time ) {
    lead_time_offset = (inData.modelTime - gen_time);
    rounded_lead_time += lead_time_offset;
  }
  else {
    gen_time = inData.modelTime;
  }

  if (_params.round_leads.round_lead_times){
	bool isNegativeValue = (0 > rounded_lead_time);
	if (isNegativeValue) {
      rounded_lead_time *= -1;
	}
    rounded_lead_time = ((int) (double(rounded_lead_time) / double(_params.round_leads.lead_time_resolution_seconds) + 0.5)) * _params.round_leads.lead_time_resolution_seconds;
	if (isNegativeValue) {
      rounded_lead_time *= -1;
	}
  }
  time_t forecast_time;
  forecast_time  = gen_time + rounded_lead_time;

  if (_params.debug) {
    cerr << "gen_time: " << utimstr(gen_time) << endl;
    cerr << "forecast_time: " << utimstr(forecast_time) << endl;
    cerr << "rounded_lead_time: " << rounded_lead_time << endl;
  }

  // check lead time

  if (!_acceptLeadTime((rounded_lead_time - lead_time_offset))) {
    if (_params.debug) {
      cerr << "Rejecting file at lead time (secs): " << rounded_lead_time << endl;
    }
    return 0;
  }

  // process this forecast

  _processForecast(inData, gen_time, rounded_lead_time, forecast_time);

  return 0;

}

/////////////////////////////////////////
// _acceptLeadTime()
//
// Check that this lead time is accepted

bool MM5Ingest::_acceptLeadTime(int lead_time)

{

  if (lead_time < _params.min_forecast_dtime) {
    return false;
  }

  if (!_params.specify_lead_times) {
    return true;
  }

  for (int i = 0; i < _params.lead_times_n; i++) {
    if (lead_time == _params._lead_times[i]) {
      return true;
    }
  } // i

  return false;

}

/////////////////////////////////////////
// _processForecast()
//
// Process data for a given forecast

int MM5Ingest::_processForecast(MM5Data &inData,
				time_t gen_time,
				int lead_time,
				time_t forecast_time)

{

  PMU_auto_register("In MM5Ingest::_processForecast");

  // compute the derived fields

  inData.computeDerivedFields(_params.wind_speed_in_knots);

  ItfaIndices *itfa = NULL;
  if (_needItfa()) {
    itfa = new ItfaIndices(inData, PMU_auto_register);
    if (_params.debug) {
      itfa->setDebug(true);
    }
    _computeItfa(inData, itfa);
    inData.loadTurbField(itfa->getCombined());
  }

  if (_needIcing()) {
    inData.loadIcingField(_params.trace_icing_clw,
			  _params.light_icing_clw,
			  _params.moderate_icing_clw,
			  _params.severe_icing_clw,
			  _params.clear_ice_temp);
  }

  // create the output file

  OutputFile outFile(_progName, _params,
		     gen_time, forecast_time, lead_time, inData);
  DsMdvx &mdvx = outFile.getDsMdvx();

  // load up fields on cross (center) grid points

  _loadCrossOutputFields(inData, itfa, mdvx);

  // load up fields on dot (corner) grid points

  _loadDotOutputFields(inData, mdvx);

  // trim out unwanted turbulence data

  _trimTurbField(mdvx);

  // convert to desired output types

  Mdvx::compression_type_t compression_type;
  switch (_params.output_compression) {
  case Params::_MDV_COMPRESSION_NONE:
    compression_type = Mdvx::COMPRESSION_NONE;
    break;
  case Params::_MDV_COMPRESSION_RLE:
    compression_type = Mdvx::COMPRESSION_RLE;
    break;
  case Params::_MDV_COMPRESSION_LZO:
    compression_type = Mdvx::COMPRESSION_LZO;
    break;
  case Params::_MDV_COMPRESSION_ZLIB:
    compression_type = Mdvx::COMPRESSION_ZLIB;
    break;
  case Params::_MDV_COMPRESSION_BZIP:
    compression_type = Mdvx::COMPRESSION_BZIP;
    break;
  default:
    compression_type = Mdvx::COMPRESSION_NONE;
  }

  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {

    MdvxField *field = mdvx.getFieldByNum(ifield);

    bool field_as_index = FALSE;
	for (int i = 0; i < _params.output_fields_as_index_n; i++) {
       if (_params._output_fields[ifield].name == _params._output_fields_as_index[i])
		 field_as_index = TRUE;
	}

    if (_params._output_fields[ifield].encoding == Params::OUT_INT8) {
	  if (field_as_index) {
        field->convertType(Mdvx::ENCODING_INT8, compression_type, Mdvx::SCALING_SPECIFIED, 1.0, 0.0);
	  } else if (_params.output_scaling == Params::SCALING_ROUNDED) {
        field->convertRounded(Mdvx::ENCODING_INT8, compression_type);
      } else {
        field->convertDynamic(Mdvx::ENCODING_INT8, compression_type);
      }
    } else if (_params._output_fields[ifield].encoding == Params::OUT_INT16) {
	  if (field_as_index) {
         field->convertType(Mdvx::ENCODING_INT16, compression_type, Mdvx::SCALING_SPECIFIED, 1.0, 0.0);
	  } else if (_params.output_scaling == Params::SCALING_ROUNDED) {
        field->convertRounded(Mdvx::ENCODING_INT16, compression_type);
      } else {
        field->convertDynamic(Mdvx::ENCODING_INT16, compression_type);
      }
    } else if (_params._output_fields[ifield].encoding
	       == Params::OUT_FLOAT32) {
      field->convertRounded(Mdvx::ENCODING_FLOAT32, compression_type);
    }

  } // ifield

  // write out volume

  if (outFile.writeVol()) {
    fprintf(stderr, "Cannot write output for time %s\n",
	    utimstr(inData.outputTime));
  }

  // free up

  if (itfa) {
    delete itfa;
  }

  return (0);

}

/////////////////////////////////////////////////
// Load up output fields on cross (center) points

void MM5Ingest::_loadCrossOutputFields(MM5Data &inData,
				       const ItfaIndices *itfa,
				       DsMdvx &mdvx)

{

  // initialize the output grid computations

  bool rotateOutputUV = false;
  MdvxProj proj;
  if (_params.output_projection == Params::OUTPUT_PROJ_FLAT) {
    proj.initFlat(_params.output_origin.lat,
		  _params.output_origin.lon,
		  0.0);
  } else if (_params.output_projection == Params::OUTPUT_PROJ_LAMBERT) {
    proj.initLc2(_params.output_origin.lat,
		 _params.output_origin.lon,
		 _params.lambert_lat1,
		 _params.lambert_lat2);
    rotateOutputUV = true;
  } else if (_params.output_projection == Params::OUTPUT_PROJ_MERCATOR) {
    proj.initMercator(_params.output_origin.lat,
		      _params.output_origin.lon);
  } else {
    proj.initLatlon();
  }

  if (rotateOutputUV) {
    _loadRotatedOutputUV(inData, proj);
  }

  // set up grid remapping object

  MM5Grid mGrid(_progName, _params.debug >= Params::DEBUG_VERBOSE,
                 inData,_params.stereographic_central_scale);

  // get representative field header

  const Mdvx::field_header_t *fhdr = NULL;
  for (int ifield = 0; ifield < mdvx.getNFields(); ifield++) {
    MdvxField *fld = mdvx.getField(ifield);
    // make sure we don't have a corner field
    if (strcmp(fld->getFieldName(), "_CORNER")) {
      fhdr = &fld->getFieldHeader();
      break;
    }
  } // ifields
  if (fhdr == NULL) {
    // no non-corner fields
    return;
  }

  // loop through the grid

  int nPointsPlane = fhdr->ny * fhdr->nx;

  for (int iy = 0; iy < fhdr->ny; iy++) {

    PMU_auto_register("In MM5Ingest::_loadCrossOutputFields loop");
    int planeOffset = iy * fhdr->nx;

    for (int ix = 0; ix < fhdr->nx; ix++, planeOffset++) {

      // set up grid interp object

      if (_params.output_projection == Params::OUTPUT_PROJ_NATIVE) {
	if (mGrid.setNonInterp(iy, ix)) {
	  // cannot process this grid point
	  continue;
	}
      } else {
	// compute x and y, and plane offset
	double yy = fhdr->grid_miny + iy * fhdr->grid_dy;
	double xx = fhdr->grid_minx + ix * fhdr->grid_dx;
	// compute latlon
	double lat, lon;
	proj.xy2latlon(xx, yy, lat, lon);
	// find the model position for this point
	if (mGrid.getGridIndices(lat, lon)) {
	  continue;
	}
      }

      // set up vert interp

      if (_params.output_levels != Params::NATIVE_SIGMA_LEVELS) {
	// load up interpolated sigma pressure array for this point
	vector<double> presSigma;
	inData.interp3dField(mGrid.latIndex, mGrid.lonIndex,
			     "press", inData.pres,
			     mGrid.wtSW, mGrid.wtNW,
			     mGrid.wtNE, mGrid.wtSE,
			     presSigma);
	// load up the sigma interpolation array
	_sigmaInterp.prepareInterp(presSigma);
      }

      // interp the fields for this point

      for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {

	switch (_params._output_fields[ifield].name) {

	  // raw 3d fields

	case Params::U_FIELD:
          if (_params.output_projection == Params::OUTPUT_PROJ_NATIVE) {
            _interp3dField(inData, "U", inData.uu, mdvx, planeOffset,
                           nPointsPlane, fhdr->missing_data_value, mGrid);
          } else if (rotateOutputUV) {
            _interp3dField(inData, "U", inData.uuOut, mdvx, planeOffset,
                           nPointsPlane, fhdr->missing_data_value, mGrid);
          } else {
            _interp3dField(inData, "U", inData.uuTn, mdvx, planeOffset,
                           nPointsPlane, fhdr->missing_data_value, mGrid);
          }
	  break;

	case Params::V_FIELD:
          if (_params.output_projection == Params::OUTPUT_PROJ_NATIVE) {
            _interp3dField(inData, "V", inData.vv, mdvx, planeOffset,
                           nPointsPlane, fhdr->missing_data_value, mGrid);
          } else if (rotateOutputUV) {
            _interp3dField(inData, "V", inData.vvOut, mdvx, planeOffset,
                           nPointsPlane, fhdr->missing_data_value, mGrid);
          } else {
            _interp3dField(inData, "V", inData.vvTn, mdvx, planeOffset,
                           nPointsPlane, fhdr->missing_data_value, mGrid);
          }
	  break;

	case Params::TK_FIELD:
	  _interp3dField(inData, "TK", inData.tk, mdvx, planeOffset,
			 nPointsPlane, fhdr->missing_data_value, mGrid);
	  break;

	case Params::Q_FIELD:
	  _interp3dField(inData, "Q", inData.qq, mdvx, planeOffset,
			 nPointsPlane, fhdr->missing_data_value, mGrid);
	  break;

	case Params::CLW_FIELD:
	  _interp3dField(inData, "clw", inData.clw, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid, 1000.0);
	  break;

	case Params::RNW_FIELD:
	  _interp3dField(inData, "rnw", inData.rnw, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid, 1000.0);
	  break;

	case Params::ICE_FIELD:
	  _interp3dField(inData, "ice", inData.ice, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid, 1000.0);
	  break;

	case Params::SNOW_FIELD:
	  _interp3dField(inData, "snow", inData.snow, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid, 1000.0);
	  break;

	case Params::GRAUPEL_FIELD:
	  _interp3dField(inData, "graupel", inData.graupel, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid, 1000.0);
	  break;

	case Params::NCI_FIELD:
	  _interp3dField(inData, "nci", inData.nci, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid, 1000.0);
	  break;

	case Params::RAD_TEND_FIELD:
	  _interp3dField(inData, "rad_tend", inData.rad_tend, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid, 1000.0);
	  break;

	case Params::W_FIELD:
	  _interp3dField(inData, "W", inData.ww, mdvx, planeOffset,
			 nPointsPlane, fhdr->missing_data_value, mGrid);
	  break;

	case Params::P_FIELD:
	  _interp3dField(inData, "P", inData.pp, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid, 1000.0);
	  break;

	case Params::TC_FIELD:
	  _interp3dField(inData, "Temp", inData.tc, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::WSPD_FIELD:
	  _interp3dField(inData, "Speed", inData.wspd, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::WDIR_FIELD:
	  _interp3dField(inData, "Wdir", inData.wdir, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::Z_FIELD:
	  _interp3dField(inData, "z", inData.zz, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::DIVERGENCE_FIELD:
	  _interp3dField(inData, "divergence", inData.divergence, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::PRESSURE_FIELD:
	  _interp3dField(inData, "pressure", inData.pres, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	  // derived 3d fields

	case Params::RH_FIELD:
	  _interp3dField(inData, "RH", inData.rh, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::DEWPT_FIELD:
	  _interp3dField(inData, "DewPt", inData.dewpt, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::TURB_FIELD:
	  _interp3dField(inData, "Turb", inData.turb, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::ICING_FIELD:
	  _interp3dField(inData, "Icing", inData.icing, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::CLW_G_FIELD:
	  _interp3dField(inData, "CLW_G", inData.clw_g, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::RNW_G_FIELD:
	  _interp3dField(inData, "RNW_G", inData.rnw_g, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::Q_G_FIELD:
	  _interp3dField(inData, "Q_G", inData.q_g, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::THETA_FIELD:
	  _interp3dField(inData, "THETA", inData.theta, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::THETAE_FIELD:
	  _interp3dField(inData, "THETAE", inData.thetae, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::THETAV_FIELD:
	  _interp3dField(inData, "THETAV", inData.thetav, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::DBZ_3D_FIELD:
	  _interp3dField(inData, "dbz_3d", inData.dbz_3d, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::TOT_CLD_CON_FIELD:
	  _interp3dField(inData, "tot_cld_con", inData.tot_cld_con, mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	  // ITFA debug 3d fields

	case Params::BROWN1_FIELD:
	  _interp3dField(inData, "Brown1", itfa->getBrown1(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::BROWN2_FIELD:
	  _interp3dField(inData, "Brown2", itfa->getBrown2(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::CCAT_FIELD:
	  _interp3dField(inData, "Ccat", itfa->getCcat(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::COLSON_PANOFSKY_FIELD:
	  _interp3dField(inData, "Colson_panofsky",
			 itfa->getColson_panofsky(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::DEF_SQR_FIELD:
	  _interp3dField(inData, "Def_sqr", itfa->getDef_sqr(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::ELLROD1_FIELD:
	  _interp3dField(inData, "Ellrod1", itfa->getEllrod1(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::ELLROD2_FIELD:
	  _interp3dField(inData, "Ellrod2", itfa->getEllrod2(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::DUTTON_FIELD:
	  _interp3dField(inData, "Dutton",
			 itfa->getDutton(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::ENDLICH_FIELD:
	  _interp3dField(inData, "Endlich", itfa->getEndlich(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::HSHEAR_FIELD:
	  _interp3dField(inData, "Hshear", itfa->getHshear(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::LAZ_FIELD:
	  _interp3dField(inData, "Laz", itfa->getLaz(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::PVORT_FIELD:
	  _interp3dField(inData, "Pvort", itfa->getPvort(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::PVORT_GRADIENT_FIELD:
	  _interp3dField(inData, "Pvort_gradient",
			 itfa->getPvort_gradient(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::NGM1_FIELD:
	  _interp3dField(inData, "Ngm1", itfa->getNgm1(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::NGM2_FIELD:
	  _interp3dField(inData, "Ngm2", itfa->getNgm2(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::RICHARDSON_FIELD:
	  _interp3dField(inData, "Richardson",
			 itfa->getRichardson(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::RIT_FIELD:
	  _interp3dField(inData, "Rit", itfa->getRit(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::SAT_RI_FIELD:
	  _interp3dField(inData, "Sat_ri", itfa->getSat_ri(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::STABILITY_FIELD:
	  _interp3dField(inData, "Stability", itfa->getStability(),
			 mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::VORT_SQR_FIELD:
	  _interp3dField(inData, "Vort_sqr", itfa->getVort_sqr(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::VWSHEAR_FIELD:
	  _interp3dField(inData, "Vwshear", itfa->getVwshear(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::TKE_KH3_FIELD:
	  _interp3dField(inData, "TkeKh3", itfa->getTke_kh3(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::TKE_KH4_FIELD:
	  _interp3dField(inData, "TkeKh4", itfa->getTke_kh4(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::TKE_KH5_FIELD:
	  _interp3dField(inData, "TkeKh5", itfa->getTke_kh5(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	  // raw 2d fields

	case Params::START_2D_FIELDS:
	  break;

	case Params::GROUND_T_FIELD:
	  _interp2dField(inData, "ground_t", inData.ground_t, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::LAT_FIELD:
	  _interp2dField(inData, "lat", inData.lat, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::LON_FIELD:
	  _interp2dField(inData, "lon", inData.lon, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::RAIN_CON_FIELD:
	  _interp2dField(inData, "rain_con", inData.rain_con, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::RAIN_NON_FIELD:
	  _interp2dField(inData, "rain_non", inData.rain_non, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::TERRAIN_FIELD:
	  _interp2dField(inData, "terrain", inData.terrain, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::CORIOLIS_FIELD:
	  _interp2dField(inData, "coriolis", inData.coriolis, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::RES_TEMP_FIELD:
	  _interp2dField(inData, "res_temp", inData.res_temp, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::LAND_USE_FIELD:
	  _closest2dField(inData, "land_use", inData.land_use, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SNOWCOVR_FIELD:
	  _interp2dField(inData, "snowcovr", inData.snowcovr, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::TSEASFC_FIELD:
	  _interp2dField(inData, "tseasfc", inData.tseasfc, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::PBL_HGT_FIELD:
	  _interp2dField(inData, "pbl_hgt", inData.pbl_hgt, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::REGIME_FIELD:
	  _interp2dField(inData, "regime", inData.regime, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SHFLUX_FIELD:
	  _interp2dField(inData, "shflux", inData.shflux, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::LHFLUX_FIELD:
	  _interp2dField(inData, "lhflux", inData.lhflux, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::UST_FIELD:
	  _interp2dField(inData, "ust", inData.ust, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SWDOWN_FIELD:
	  _interp2dField(inData, "swdown", inData.swdown, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::LWDOWN_FIELD:
	  _interp2dField(inData, "lwdown", inData.lwdown, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SOIL_T_1_FIELD:
	  _interp2dField(inData, "soil_t_1", inData.soil_t_1, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SOIL_T_2_FIELD:
	  _interp2dField(inData, "soil_t_2", inData.soil_t_2, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SOIL_T_3_FIELD:
	  _interp2dField(inData, "soil_t_3", inData.soil_t_3, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SOIL_T_4_FIELD:
	  _interp2dField(inData, "soil_t_4", inData.soil_t_4, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SOIL_T_5_FIELD:
	  _interp2dField(inData, "soil_t_5", inData.soil_t_5, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SOIL_T_6_FIELD:
	  _interp2dField(inData, "soil_t_6", inData.soil_t_6, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SOIL_M_1_FIELD:
	  _interp2dField(inData, "soil_m_1", inData.soil_m_1, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SOIL_M_2_FIELD:
	  _interp2dField(inData, "soil_m_2", inData.soil_m_2, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SOIL_M_3_FIELD:
	  _interp2dField(inData, "soil_m_3", inData.soil_m_3, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SOIL_M_4_FIELD:
	  _interp2dField(inData, "soil_m_4", inData.soil_m_4, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::T2_FIELD:
	  _interp2dField(inData, "T2", inData.t2, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::Q2_FIELD:
	  _interp2dField(inData, "Q2", inData.q2, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::U10_FIELD:
	  _interp2dField(inData, "U10", inData.u10, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::V10_FIELD:
	  _interp2dField(inData, "V10", inData.v10, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::WEASD_FIELD:
	  _interp2dField(inData, "weasd", inData.weasd, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SNOWH_FIELD:
	  _interp2dField(inData, "snowh", inData.snowh, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::HOURLY_CONV_RAIN_FIELD:
	  _interp2dField(inData, "hraincon", inData.hc_rain, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::HOURLY_NONC_RAIN_FIELD:
	  _interp2dField(inData, "hrainnon", inData.hn_rain, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	  // derived 2d fields

	case Params::FZLEVEL_FIELD:
	  _interp2dField(inData, "FZLevel", inData.fzlevel, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::RAIN_TOTAL_FIELD:
	  _interp2dField(inData, "rain_total", inData.rain_total, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

        case Params::HOURLY_RAIN_TOTAL_FIELD:
          _interp2dField(inData, "hrain_total", inData.hourly_rain_total, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
          break;

	case Params::DBZ_2D_FIELD:
	  _interp2dField(inData, "dbz_2d", inData.dbz_2d, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::RH2_FIELD:
	  _interp2dField(inData, "RH2", inData.rh2, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::DEWPT2_FIELD:
	  _interp2dField(inData, "DewPt2", inData.dewpt2, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::WSPD10_FIELD:
	  _interp2dField(inData, "Speed10", inData.wspd10, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::WDIR10_FIELD:
	  _interp2dField(inData, "Wdir10", inData.wdir10, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::MSLP2_FIELD:
	  _interp2dField(inData, "pressure2", inData.mslp2, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::T2C_FIELD:
	  _interp2dField(inData, "T2C", inData.t2c, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::Q2_G_FIELD:
	  _interp2dField(inData, "Q2_G", inData.q2_g, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::THETA2_FIELD:
	  _interp2dField(inData, "THETA2", inData.theta2, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::THETAE2_FIELD:
	  _interp2dField(inData, "THETAE2", inData.thetae2, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::THETAV2_FIELD:
	  _interp2dField(inData, "THETAV2", inData.thetav2, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::CLOUD_FRACT_FIELD:
	  _interp2dField(inData, "cloud_fract", inData.cloud_fract, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::TWP_FIELD:
	  _interp2dField(inData, "twp", inData.twp, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);

	  break;

	case Params::RWP_FIELD:
	  _interp2dField(inData, "rwp", inData.rwp, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::TOT_CLD_CONP_FIELD:
	  _interp2dField(inData, "tot_cld_conp", inData.tot_cld_conp, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::CLWP_FIELD:
	  _interp2dField(inData, "clwp", inData.clwp, mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

          default: {}

	} // switch

      } // ifield

    } // ix

  } // iy

}

/////////////////////////////////////////////////
// Load up output fields on dot (corner) points

void MM5Ingest::_loadDotOutputFields(MM5Data &inData,
				     DsMdvx &mdvx)

{

  if (_params.output_projection != Params::OUTPUT_PROJ_NATIVE) {
    // only applies to native projection
    return;
  }

  // set up grid remapping object

  MM5Grid mGrid(_progName, _params.debug >= Params::DEBUG_VERBOSE,
                 inData,_params.stereographic_central_scale);

  // get representative field header

  const Mdvx::field_header_t *fhdr = NULL;
  for (int ifield = 0; ifield < mdvx.getNFields(); ifield++) {
    MdvxField *fld = mdvx.getField(ifield);
    // make sure we don't have a corner field
    if (!strstr(fld->getFieldName(), "_CORNER")) {
      fhdr = &fld->getFieldHeader();
      break;
    }
  } // ifields
  if (fhdr == NULL) {
    // no corner fields
    return;
  }

  // loop through the grid

  int nPointsPlane = fhdr->ny * fhdr->nx;

  for (int iy = 0; iy < fhdr->ny; iy++) {

    PMU_auto_register("In MM5Ingest::_loadDotOutputFields loop");
    int planeOffset = iy * fhdr->nx;

    for (int ix = 0; ix < fhdr->nx; ix++, planeOffset++) {

      if (mGrid.setNonInterp(iy, ix)) {
	// cannot process this grid point
	continue;
      }

      if (_params.output_levels != Params::NATIVE_SIGMA_LEVELS) {
	// load up interpolated sigma pressure array for this point
	vector<double> presSigma;
	inData.interp3dField(mGrid.latIndex, mGrid.lonIndex,
			     "press", inData.pres,
			     mGrid.wtSW, mGrid.wtNW,
			     mGrid.wtNE, mGrid.wtSE,
			     presSigma);
	// load up the sigma interpolation array
	_sigmaInterp.prepareInterp(presSigma);
      }

      // interp the fields for this point

      for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {

        switch (_params._output_fields[ifield].name) {

          case Params::U_CORNER_FIELD:
            _interp3dField(inData, "U_CORNER", inData.uu_dot, mdvx,
                           planeOffset, nPointsPlane,
                           fhdr->missing_data_value, mGrid);
            break;

          case Params::V_CORNER_FIELD:
            _interp3dField(inData, "V_CORNER", inData.vv_dot, mdvx,
                           planeOffset, nPointsPlane,
                           fhdr->missing_data_value, mGrid);
            break;

          default: {}

	} // switch

      } // ifield

    } // ix

  } // iy

}

//////////////////
// _interp3dField()
//
// Interpolate the 3d field data onto the output grid point
//

void MM5Ingest::_interp3dField(MM5Data &inData,
			       const char *field_name,
			       fl32 ***field_data,
			       DsMdvx &mdvx,
			       int planeOffset,
			       int nPointsPlane,
			       fl32 missingDataVal,
			       const MM5Grid &mGrid,
			       double factor /* = 1.0*/ )

{

  if (field_data == NULL)
    return;

  // load up sigma array for this point

  vector<double> vlevelData;

  if (_params.output_levels == Params::NATIVE_SIGMA_LEVELS) {

    inData.interp3dField(mGrid.latIndex, mGrid.lonIndex,
			 field_name, field_data,
			 mGrid.wtSW, mGrid.wtNW,
			 mGrid.wtNE, mGrid.wtSE,
			 vlevelData);

  } else {

    vector<double> sigmaData;
    inData.interp3dField(mGrid.latIndex, mGrid.lonIndex,
			 field_name, field_data,
			 mGrid.wtSW, mGrid.wtNW,
			 mGrid.wtNE, mGrid.wtSE,
			 sigmaData,
			 &_sigmaInterp.getSigmaNeeded());

    // interpolate field onto flight levels

    _sigmaInterp.doInterp(sigmaData, vlevelData,
			  _params.copy_lowest_downwards);

  }

  // put into targetVol

  MdvxField *field = mdvx.getFieldByName(field_name);
  if (field == NULL) {
    cerr << "ERROR - MM5Ingest::_interp3dField" << endl;
    cerr << "  Cannot find field name: " << field_name << endl;
    return;
  }

  Mdvx::field_header_t fhdr = field->getFieldHeader();
  fl32 *targetVol = (fl32 *) field->getVol();
  fl32 *ffp = targetVol + planeOffset;

  for (size_t i = 0; i < vlevelData.size(); i++, ffp += nPointsPlane) {
    if (vlevelData[i] == MM5Data::MissingDouble) {
      *ffp = missingDataVal;
    } else {
      *ffp = vlevelData[i] * factor;
    }
  }

}

//////////////////
// _interp2dField()
//
// Interpolate the 2d field data onto the output grid point
//

void MM5Ingest::_interp2dField(MM5Data &inData,
			       const char *field_name,
			       fl32 **field_data,
			       DsMdvx &mdvx,
			       int planeOffset,
			       fl32 missingDataVal,
			       const MM5Grid &mGrid,
			       double factor /* = 1.0*/ )

{

  if (field_data == NULL)
    return;

  // get interp value for point

  double interpVal =
    inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
			 field_name, field_data,
			 mGrid.wtSW, mGrid.wtNW,
			 mGrid.wtNE, mGrid.wtSE);

  // put into targetVol

  MdvxField *field = mdvx.getFieldByName(field_name);
  if (field == NULL) {
    cerr << "ERROR - MM5Ingest::_interp2dField" << endl;
    cerr << "  Cannot find field name: " << field_name << endl;
    return;
  }
  Mdvx::field_header_t fhdr = field->getFieldHeader();
  fl32 *targetVol = (fl32 *) field->getVol();
  fl32 *ffp = targetVol + planeOffset;

  if (interpVal == MM5Data::MissingDouble) {
    *ffp = missingDataVal;
  } else {
    *ffp = interpVal * factor;
  }

}

//////////////////
// _closest2dField()
//
// Interpolate the 2d field data onto the output grid point
//

void MM5Ingest::_closest2dField(MM5Data &inData,
			       const char *field_name,
			       fl32 **field_data,
			       DsMdvx &mdvx,
			       int planeOffset,
			       fl32 missingDataVal,
			       const MM5Grid &mGrid,
			       double factor /* = 1.0*/ )

{

  if (field_data == NULL)
    return;

  // get interp value for point

  double interpVal =
    inData.closest2dField(mGrid.latIndex, mGrid.lonIndex,
			 field_name, field_data,
			 mGrid.wtSW, mGrid.wtNW,
			 mGrid.wtNE, mGrid.wtSE);

  // put into targetVol

  MdvxField *field = mdvx.getFieldByName(field_name);
  if (field == NULL) {
    cerr << "ERROR - MM5Ingest::_closest2dField" << endl;
    cerr << "  Cannot find field name: " << field_name << endl;
    return;
  }
  Mdvx::field_header_t fhdr = field->getFieldHeader();
  fl32 *targetVol = (fl32 *) field->getVol();
  fl32 *ffp = targetVol + planeOffset;

  if (interpVal == MM5Data::MissingDouble) {
    *ffp = missingDataVal;
  } else {
    *ffp = interpVal * factor;
  }

}

/////////////////////////////////////////////////
// Load up rotated outout UV fields

void MM5Ingest::_loadRotatedOutputUV(MM5Data &inData,
                                     const MdvxProj &proj)

{

  fl32 **outputGridRotation =
    (fl32 **) umalloc2(inData.nLat, inData.nLon, sizeof(fl32));

  for (int ilat = 0; ilat < inData.nLat; ilat++) {
    for (int ilon = 0; ilon < inData.nLon; ilon++) {

      // get lat/lon of point 0

      double lat0 = inData.lat[ilat][ilon];
      double lon0 = inData.lon[ilat][ilon];
      lon0 = proj.conditionLon2Origin(lon0);

      // compute x0, y0

      double x0, y0;
      proj.latlon2xy(lat0, lon0, x0, y0);

      // compute x1, y1 10 km north

      double x1 = x0;
      double y1 = y0 + 10.0;

      // compute lat1, lon1

      double lat1, lon1;
      proj.xy2latlon(x1, y1, lat1, lon1);

      // compute rotation theta

      double range, theta;
      PJGLatLon2RTheta(lat0, lon0, lat1, lon1,
                       &range, &theta);

      //       cerr << "lat0, lon0, x0, y0, x1, y1, lat1, lon1, rotation: "
      //            << lat0 << " " << lon0 << " "
      //            << x0 << " " << y0 << " "
      //            << x1 << " " << y1 << " "
      //            << lat1 << " " << lon1 << " "
      //            << theta << endl;

      outputGridRotation[ilat][ilon] = (fl32) theta;

    } // ilon

  } // ilat

  inData.loadUVOutput(outputGridRotation);
  ufree2((void **) outputGridRotation);

}

////////////////////////////////////////////////////
// do we need to compute ITFA turbulence probability

bool MM5Ingest::_needItfa()

{
  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    switch (_params._output_fields[ifield].name) {
    case Params::TURB_FIELD:
    case Params::BROWN1_FIELD:
    case Params::BROWN2_FIELD:
    case Params::CCAT_FIELD:
    case Params::COLSON_PANOFSKY_FIELD:
    case Params::DEF_SQR_FIELD:
    case Params::ELLROD1_FIELD:
    case Params::ELLROD2_FIELD:
    case Params::DUTTON_FIELD:
    case Params::ENDLICH_FIELD:
    case Params::HSHEAR_FIELD:
    case Params::LAZ_FIELD:
    case Params::PVORT_FIELD:
    case Params::PVORT_GRADIENT_FIELD:
    case Params::NGM1_FIELD:
    case Params::NGM2_FIELD:
    case Params::RICHARDSON_FIELD:
    case Params::RIT_FIELD:
    case Params::SAT_RI_FIELD:
    case Params::STABILITY_FIELD:
    case Params::VORT_SQR_FIELD:
    case Params::VWSHEAR_FIELD:
    case Params::TKE_KH3_FIELD:
    case Params::TKE_KH4_FIELD:
    case Params::TKE_KH5_FIELD:
      return true;
      break;
    default: {}
    }
  }
  return false;
}

////////////////////////////////////
// do we need to compute icing

bool MM5Ingest::_needIcing()

{
  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    if (_params._output_fields[ifield].name == Params::ICING_FIELD) {
      return true;
    }
  }
  return false;
}

void MM5Ingest::_computeItfa(MM5Data &inData,
			     ItfaIndices *itfa)

{

  // calculate the indices

  itfa->calculate_indices();

  // clear the combined field

  itfa->clearCombined();

  // add the itfa derived fields to the combined fields, by weight

  for (int index = 0; index < _params.itfa_derived_indices_n; index++) {

    ItfaIndices::test_sense_t sense = ItfaIndices::LESS_THAN;
    switch (_params._itfa_derived_indices[index].sense) {
    case Params::LESS_THAN:
      sense = ItfaIndices::LESS_THAN;
      break;
    case Params::GREATER_THAN:
      sense = ItfaIndices::GREATER_THAN;
      break;
    case Params::INSIDE_INTERVAL:
      sense = ItfaIndices::INSIDE_INTERVAL;
      break;
    case Params::OUTSIDE_INTERVAL:
      sense = ItfaIndices::OUTSIDE_INTERVAL;
      break;
    } // switch

    double threshold_1 = _params._itfa_derived_indices[index].threshold_1;
    double threshold_2 = _params._itfa_derived_indices[index].threshold_2;
    double weight = _params._itfa_derived_indices[index].weight;

    double min_level = _params._itfa_derived_indices[index].min_flight_level;
    double max_level = _params._itfa_derived_indices[index].max_flight_level;
    IcaoStdAtmos isa;
    double min_pressure = isa.flevel2pres(max_level);
    double max_pressure = isa.flevel2pres(min_level);

    switch (_params._itfa_derived_indices[index].name) {

    case Params::BROWN1:
      itfa->addToCombined("Brown1", itfa->getBrown1(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::BROWN2:
      itfa->addToCombined("Brown2", itfa->getBrown2(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::CCAT:
      itfa->addToCombined("Ccat", itfa->getCcat(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::COLSON_PANOFSKY:
      itfa->addToCombined("ColsonPanofsky", itfa->getColson_panofsky(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::DEF_SQR:
      itfa->addToCombined("DefSqr", itfa->getDef_sqr(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::ELLROD1:
      itfa->addToCombined("Ellrod1", itfa->getEllrod1(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::ELLROD2:
      itfa->addToCombined("Ellrod2", itfa->getEllrod2(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::DUTTON:
      itfa->addToCombined("dutton", itfa->getDutton(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::ENDLICH:
      itfa->addToCombined("endlich", itfa->getEndlich(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::HSHEAR:
      itfa->addToCombined("hshear", itfa->getHshear(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::LAZ:
      itfa->addToCombined("laz", itfa->getLaz(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::PVORT:
      itfa->addToCombined("pvort", itfa->getPvort(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::PVORT_GRADIENT:
      itfa->addToCombined("pvort_gradient", itfa->getPvort_gradient(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::NGM1:
      itfa->addToCombined("Ngm1", itfa->getNgm1(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::NGM2:
      itfa->addToCombined("Ngm2", itfa->getNgm2(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::RICHARDSON:
      itfa->addToCombined("richardson", itfa->getRichardson(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::RIT:
      itfa->addToCombined("rit", itfa->getRit(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::SAT_RI:
      itfa->addToCombined("sat_ri", itfa->getSat_ri(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::STABILITY:
      itfa->addToCombined("stability", itfa->getStability(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::VORT_SQR:
      itfa->addToCombined("vort_sqr", itfa->getVort_sqr(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::VWSHEAR:
      itfa->addToCombined("vwshear", itfa->getVwshear(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::TKE_GWB:
      itfa->addToCombined("tke_gwb", itfa->getTke_gwb(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::TKE_KH3:
      itfa->addToCombined("tke_kh3", itfa->getTke_kh3(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::TKE_KH4:
      itfa->addToCombined("tke_kh4", itfa->getTke_kh4(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::TKE_KH5:
      itfa->addToCombined("tke_kh5", itfa->getTke_kh5(),
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    } // switch

  } // index

  // add the itfa model fields to the combined fields, by weight

  for (int index = 0; index < _params.itfa_model_indices_n; index++) {

    ItfaIndices::test_sense_t sense = ItfaIndices::LESS_THAN;
    switch (_params._itfa_model_indices[index].sense) {
    case Params::LESS_THAN:
      sense = ItfaIndices::LESS_THAN;
      break;
    case Params::GREATER_THAN:
      sense = ItfaIndices::GREATER_THAN;
      break;
    case Params::INSIDE_INTERVAL:
      sense = ItfaIndices::INSIDE_INTERVAL;
      break;
    case Params::OUTSIDE_INTERVAL:
      sense = ItfaIndices::OUTSIDE_INTERVAL;
      break;
    } // switch

    double threshold_1 = _params._itfa_model_indices[index].threshold_1;
    double threshold_2 = _params._itfa_model_indices[index].threshold_2;
    double weight = _params._itfa_model_indices[index].weight;

    double min_level = _params._itfa_model_indices[index].min_flight_level;
    double max_level = _params._itfa_model_indices[index].max_flight_level;

    IcaoStdAtmos isa;
    double min_pressure = isa.flevel2pres(max_level);
    double max_pressure = isa.flevel2pres(min_level);

    switch (_params._itfa_model_indices[index].name) {

    case Params::W_ITFA:
      itfa->addToCombined("w", inData.ww,
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::WSPD_ITFA:
      itfa->addToCombined("wind speed", inData.wspd,
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    case Params::DIVERGENCE_ITFA:
      itfa->addToCombined("divergence", inData.divergence,
			  weight, sense,
			  threshold_1, threshold_2,
			  min_pressure, max_pressure);
      break;

    } // switch

  } // index

  // normalize the field

  itfa->normalizeCombined();

  // fill in edges if required

  if (_params.itfa_fill_edges) {
    itfa->fillEdgesCombined();
  }

}

//////////////////////////////////////////////////
// trim unwanted values from the turbulence field

void MM5Ingest::_trimTurbField(DsMdvx &mdvx)

{

  MdvxField *field = mdvx.getField("Turb");
  if (field == NULL) {
    return;
  }

  const Mdvx::field_header_t &fhdr = field->getFieldHeader();
  fl32 *turb = (fl32 *) field->getVol();
  int nPointsPlane = fhdr.nx * fhdr.ny;
  int nLevels = fhdr.nz;

  for (int iz = 0; iz < nLevels; iz++) {

    fl32 *plane = turb + iz * nPointsPlane;
    for (int i = 0; i < nPointsPlane; i++) {
      if (plane[i] < _params.min_turb_severity_threshold) {
	plane[i] = 0.0;
      }
    }

  } // iz

}

