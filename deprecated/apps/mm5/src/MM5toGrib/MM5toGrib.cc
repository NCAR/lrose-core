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
// MM5toGrib.cc
//
// MM5toGrib object
//
// Carl Drews, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 31, 2004
//
///////////////////////////////////////////////////////////////

#include "MM5toGrib.hh"
#include "InputPath.hh"
#include "OutputFile.hh"
#include <mm5/MM5DataV2.hh>
#include <mm5/MM5DataV3.hh>
#include <mm5/ItfaIndices.hh>
#include <toolsa/pmu.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include "GribWriter.hh"
#include "NullDataConverter.hh"
#include <stdlib.h>

using namespace std;

// Constructor

MM5toGrib::MM5toGrib(int argc, char **argv)

{

  OK = TRUE;

  // set application name
  _progName = "MM5toGrib";
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

  // set up sigma level interpolation 
  switch (_params.output_levels) {

  case Params::FLIGHT_LEVELS: {
    double *levels = new double[_params.flight_levels_n];
    for (int i = 0; i < _params.flight_levels_n; i++) {
      levels[i] = _params._flight_levels[i];
    }
    _sigmaInterp.setFlightLevels(_params.flight_levels_n, levels);
    if (_params.debug) {
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
    if (_params.debug) {
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
    if (_params.debug) {
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
    PMU_auto_register("In MM5toGrib constructor");
  }

  return;

}

// destructor

MM5toGrib::~MM5toGrib()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MM5toGrib::Run ()
{

  PMU_auto_register("MM5toGrib::Run");

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
  
int MM5toGrib::_checkParams()
  
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

int MM5toGrib::_run(InputPath &input_path)

{
  
  PMU_auto_register("MM5toGrib::_run");
  
  // loop through the input files
  
  string filePath;
  while ((filePath = input_path.next()) != "") {
    
    if (_params.debug >= Params::DEBUG_NORM) {
      fprintf(stderr, "Processing file %s\n", filePath.c_str());
    }
    
    int version;
    if (MM5Data::getVersion(filePath, version)) {
      fprintf(stderr, "Getting version number from file %s\n",
	      filePath.c_str());
      return -1;
    }

    MM5Data *inData;
    if (version == 2) {
      inData = new MM5DataV2(_progName,
			     filePath,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     PMU_auto_register);
    } else {
      inData = new MM5DataV3(_progName,
			     filePath,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     PMU_auto_register);
    }

    while (inData->more()) {
      if (_processInData(*inData)) {
	fprintf(stderr, "ERROR - MM5toGrib::_run\n");
	fprintf(stderr, "  Processing file: %s\n", filePath.c_str());
	umsleep(2);
	break;
      }
    }
    
    delete inData;
    
  } // while

  return 0;

}


void MM5toGrib::writeGrib(DsMdvx &inputMdv, string outputFile)
{
  string methodName = "MM5toGrib::writeGrib";

  // make object to do the work
  GribWriter gribber;
  gribber.setDebugFlag(_params.debug);

  // open output file
  if (!gribber.openFile(outputFile)) {
    fprintf(stderr, "Cannot open grib output file %s", outputFile.c_str());
  }

  // Extract each of the fields in the MDV file
  // and write them to the output GRIB file.
  Mdvx::master_header_t master_hdr = inputMdv.getMasterHeader();

  for (int field_num = 0; field_num < master_hdr.n_fields; ++field_num)
  {
    MdvxField *field = inputMdv.getField(field_num);
    if (field == 0)
    {
      cerr << "ERROR: " << methodName << endl;
      cerr << "Error extracting field number " << field_num
           << " from MDV file." << endl;
      cerr << "Skipping field....." << endl;

      continue;
    }

    if (field->isCompressed()) {
      field->decompress();
    }

    DataConverter *converter;
    converter = new NullDataConverter();

/*
    if (!gribber.writeField(master_hdr, *field, *converter,
                                _params.grib_tables_version,
                                _params.originating_center,
                                _params.generating_process_id,
                                _params.grid_id,
                                _params._output_fields[field_num].grib_code,
                                _params._output_fields[field_num].precision,
                                _params.forecast_interval_type,
                                _params.time_range_id))
*/
    if (!gribber.writeField(master_hdr, *field, *converter,
                                2,
                                60,	// NCAR
                                0,
                                0,
                                _params._output_fields[field_num].grib_code,
                                _params._output_fields[field_num].precision,
                                _params.forecast_interval_type,
                                0))
    {
      cerr << "Failed to write field in " << methodName << endl;
      // \todo: Throw exception here.
    }

    delete converter;
  }

  // close output file
  gribber.closeFile();
}


/////////////////////////////////////////
// _processInData()
//
// Process input data
//
// Returns 0 on success, -1 on failure

int MM5toGrib::_processInData(MM5Data &inData)

{

  if (!inData.OK) {
    return -1;
  }

  // read in data set

  if (inData.read()) {
    return -1;
  }
    
  // check projection - native proj only OK for Lambert, stereographic, and Mercator
    
  if (_params.output_projection == Params::OUTPUT_PROJ_NATIVE) {
    if (inData.proj_type != MM5Data::LAMBERT_CONF &&
	inData.proj_type != MM5Data::STEREOGRAPHIC &&
        inData.proj_type != MM5Data::MERCATOR) {
      cerr << "ERROR - MM5toGrib::_run" << endl;
      cerr << "  Output projection is set to NATIVE." << endl;
      cerr << "  Input MM5 data file has projection UNKNOWN: " << inData.proj_type << endl;
      cerr << "  Only Lambert, stereographic, and Mercator projections are supported." << endl;
      return -1;
    }
  }
  
  // round the lead_time to nearest 15 mins, and compute forecast time
  
  int lead_time = inData.forecastLeadTime;
  time_t gen_time = inData.modelTime;
  int rounded_lead_time = ((int) (lead_time / 900.0 + 0.5)) * 900;
  time_t forecast_time = gen_time + rounded_lead_time;

  if (_params.debug) {
    cerr << "gen_time: " << utimstr(gen_time) << endl;
    cerr << "forecast_time: " << utimstr(forecast_time) << endl;
    cerr << "rounded_lead_time: " << rounded_lead_time << endl;
  }

  // check lead time

  if (!_acceptLeadTime(rounded_lead_time)) {
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

bool MM5toGrib::_acceptLeadTime(int lead_time)

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

int MM5toGrib::_processForecast(MM5Data &inData,
				time_t gen_time,
				int lead_time,
				time_t forecast_time)

{
  PMU_auto_register("In MM5toGrib::_processForecast");

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

  // set up grid remapping object
  
  MM5Grid mGrid(_progName, _params.debug >= Params::DEBUG_VERBOSE,
		inData.nLat, inData.nLon,
		inData.lat, inData.lon);
  
  // initialize the output grid computations
 
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
  } else if (_params.output_projection == Params::OUTPUT_PROJ_MERCATOR) {
    proj.initMercator(_params.output_origin.lat,
		_params.output_origin.lon);
  } else if (_params.output_projection == Params::OUTPUT_PROJ_NATIVE) {
    proj.initLc2(inData.center_lat,
		 inData.center_lon,
		 OutputFile::getLambertLat1(inData.center_lat, inData.true_lat1, inData.true_lat2),
		 OutputFile::getLambertLat2(inData.center_lat, inData.true_lat1, inData.true_lat2));
  } else {
    proj.initLatlon();
  }

  // create the output file

  OutputFile outFile(_progName, _params,
		     gen_time, forecast_time, lead_time, inData);
  DsMdvx &mdvx = outFile.getDsMdvx();
  const Mdvx::field_header_t &fhdr = mdvx.getField(0)->getFieldHeader();
  
  // loop through the grid

  int nPointsPlane = fhdr.ny * fhdr.nx;
  int turbFieldNum = -1;

  for (int iy = 0; iy < fhdr.ny; iy++) {
    
    PMU_auto_register("In MM5toGrib::_processForecast loop");
    int planeOffset = iy * fhdr.nx;

    for (int ix = 0; ix < fhdr.nx; ix++, planeOffset++) {

      int iret;
      if (_params.output_projection == Params::OUTPUT_PROJ_NATIVE) {

	iret = mGrid.setNonInterp(iy, ix);

      } else {

	// compute x and y, and plane offset
	
	double yy = fhdr.grid_miny + iy * fhdr.grid_dy;
	double xx = fhdr.grid_minx + ix * fhdr.grid_dx;
	
	// compute latlon
	
	double lat, lon;
	proj.xy2latlon(xx, yy, lat, lon);
	
	// find the model position for this point
	
	// fprintf(stderr, "grid point [%d][%d]: (%g, %g)\n",
	//  iy, ix, lat, lon);
	
	iret = mGrid.findModelLoc(lat, lon);

      }

      if (iret == 0) {

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
	    _interp3dField(inData, "U", inData.uu, mdvx, planeOffset,
			   nPointsPlane, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::V_FIELD:
	    _interp3dField(inData, "V", inData.vv, mdvx, planeOffset,
			   nPointsPlane, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::TK_FIELD:
	    _interp3dField(inData, "TK", inData.tk, mdvx, planeOffset,
			   nPointsPlane, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::Q_FIELD:
	    _interp3dField(inData, "Q", inData.qq, mdvx, planeOffset,
			   nPointsPlane, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::CLW_FIELD:
	    _interp3dField(inData, "clw", inData.clw, mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid, 1000.0);
	    break;
	    
	  case Params::RNW_FIELD:
	    _interp3dField(inData, "rnw", inData.rnw, mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid, 1000.0);
	    break;
	    
	  case Params::ICE_FIELD:
	    _interp3dField(inData, "ice", inData.ice, mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid, 1000.0);
	    break;
	    
	  case Params::SNOW_FIELD:
	    _interp3dField(inData, "snow", inData.snow, mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid, 1000.0);
	    break;
	    
	  case Params::GRAUPEL_FIELD:
	    _interp3dField(inData, "graupel", inData.graupel, mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid, 1000.0);
	    break;
	    
	  case Params::NCI_FIELD:
	    _interp3dField(inData, "nci", inData.nci, mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid, 1000.0);
	    break;
	    
	  case Params::RAD_TEND_FIELD:
	    _interp3dField(inData, "rad_tend", inData.rad_tend, mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid, 1000.0);
	    break;
	    
	  case Params::W_FIELD:
	    _interp3dField(inData, "W", inData.ww, mdvx, planeOffset,
			   nPointsPlane, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::P_FIELD:
	    _interp3dField(inData, "P", inData.pp, mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid, 1000.0);
	    break;
	    
	  case Params::TC_FIELD:
	    _interp3dField(inData, "Temp", inData.tc, mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::WSPD_FIELD:
	    _interp3dField(inData, "Speed", inData.wspd, mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::WDIR_FIELD:
	    _interp3dField(inData, "Wdir", inData.wdir, mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::Z_FIELD:
	    _interp3dField(inData, "z", inData.zz, mdvx,
			   planeOffset, nPointsPlane, 
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::DIVERGENCE_FIELD:
	    _interp3dField(inData, "divergence", inData.divergence, mdvx,
			   planeOffset, nPointsPlane, 
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::PRESSURE_FIELD:
	    _interp3dField(inData, "pressure", inData.pres, mdvx,
			   planeOffset, nPointsPlane, 
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	    // derived 3d fields

	  case Params::RH_FIELD:
	    _interp3dField(inData, "RH", inData.rh, mdvx,
			   planeOffset, nPointsPlane, 
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::DEWPT_FIELD:
	    _interp3dField(inData, "DewPt", inData.dewpt, mdvx,
			   planeOffset, nPointsPlane, 
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::TURB_FIELD:
	    turbFieldNum = ifield;
	    _interp3dField(inData, "Turb", inData.turb, mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::ICING_FIELD:
	    _interp3dField(inData, "Icing", inData.icing, mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	    // ITFA debug 3d fields

	  case Params::BROWN1_FIELD:
	    _interp3dField(inData, "Brown1", itfa->getBrown1(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::BROWN2_FIELD:
	    _interp3dField(inData, "Brown2", itfa->getBrown2(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::CCAT_FIELD:
	    _interp3dField(inData, "Ccat", itfa->getCcat(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::COLSON_PANOFSKY_FIELD:
	    _interp3dField(inData, "Colson_panofsky",
			   itfa->getColson_panofsky(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::DEF_SQR_FIELD:
	    _interp3dField(inData, "Def_sqr", itfa->getDef_sqr(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::ELLROD1_FIELD:
	    _interp3dField(inData, "Ellrod1", itfa->getEllrod1(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::ELLROD2_FIELD:
	    _interp3dField(inData, "Ellrod2", itfa->getEllrod2(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::DUTTON_FIELD:
	    _interp3dField(inData, "Dutton",
			   itfa->getDutton(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::ENDLICH_FIELD:
	    _interp3dField(inData, "Endlich", itfa->getEndlich(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::HSHEAR_FIELD:
	    _interp3dField(inData, "Hshear", itfa->getHshear(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::LAZ_FIELD:
	    _interp3dField(inData, "Laz", itfa->getLaz(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::PVORT_FIELD:
	    _interp3dField(inData, "Pvort", itfa->getPvort(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::PVORT_GRADIENT_FIELD:
	    _interp3dField(inData, "Pvort_gradient",
			   itfa->getPvort_gradient(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::NGM1_FIELD:
	    _interp3dField(inData, "Ngm1", itfa->getNgm1(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::NGM2_FIELD:
	    _interp3dField(inData, "Ngm2", itfa->getNgm2(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::RICHARDSON_FIELD:
	    _interp3dField(inData, "Richardson",
			   itfa->getRichardson(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::RIT_FIELD:
	    _interp3dField(inData, "Rit", itfa->getRit(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::SAT_RI_FIELD:
	    _interp3dField(inData, "Sat_ri", itfa->getSat_ri(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::STABILITY_FIELD:
	    _interp3dField(inData, "Stability", itfa->getStability(),
			   mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::VORT_SQR_FIELD:
	    _interp3dField(inData, "Vort_sqr", itfa->getVort_sqr(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::VWSHEAR_FIELD:
	    _interp3dField(inData, "Vwshear", itfa->getVwshear(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::TKE_KH3_FIELD:
	    _interp3dField(inData, "TkeKh3", itfa->getTke_kh3(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::TKE_KH4_FIELD:
	    _interp3dField(inData, "TkeKh4", itfa->getTke_kh4(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::TKE_KH5_FIELD:
	    _interp3dField(inData, "TkeKh5", itfa->getTke_kh5(), mdvx,
			   planeOffset, nPointsPlane,
			   fhdr.missing_data_value, mGrid);
	    break;
	    
	    // raw 2d fields

	  case Params::START_2D_FIELDS:
	    break;

	  case Params::GROUND_T_FIELD:
	    _interp2dField(inData, "ground_t", inData.ground_t, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::RAIN_CON_FIELD:
	    _interp2dField(inData, "rain_con", inData.rain_con, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::RAIN_NON_FIELD:
	    _interp2dField(inData, "rain_non", inData.rain_non, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::TERRAIN_FIELD:
	    _interp2dField(inData, "terrain", inData.terrain, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::CORIOLIS_FIELD:
	    _interp2dField(inData, "coriolis", inData.coriolis, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::RES_TEMP_FIELD:
	    _interp2dField(inData, "res_temp", inData.res_temp, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::LAND_USE_FIELD:
	    _interp2dField(inData, "land_use", inData.land_use, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::SNOWCOVR_FIELD:
	    _interp2dField(inData, "snowcovr", inData.snowcovr, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::TSEASFC_FIELD:
	    _interp2dField(inData, "tseasfc", inData.tseasfc, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::PBL_HGT_FIELD:
	    _interp2dField(inData, "pbl_hgt", inData.pbl_hgt, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::REGIME_FIELD:
	    _interp2dField(inData, "regime", inData.regime, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::SHFLUX_FIELD:
	    _interp2dField(inData, "shflux", inData.shflux, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::LHFLUX_FIELD:
	    _interp2dField(inData, "lhflux", inData.lhflux, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::UST_FIELD:
	    _interp2dField(inData, "ust", inData.ust, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::SWDOWN_FIELD:
	    _interp2dField(inData, "swdown", inData.swdown, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::LWDOWN_FIELD:
	    _interp2dField(inData, "lwdown", inData.lwdown, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::SOIL_T_1_FIELD:
	    _interp2dField(inData, "soil_t_1", inData.soil_t_1, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::SOIL_T_2_FIELD:
	    _interp2dField(inData, "soil_t_2", inData.soil_t_2, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::SOIL_T_3_FIELD:
	    _interp2dField(inData, "soil_t_3", inData.soil_t_3, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::SOIL_T_4_FIELD:
	    _interp2dField(inData, "soil_t_4", inData.soil_t_4, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::SOIL_T_5_FIELD:
	    _interp2dField(inData, "soil_t_5", inData.soil_t_5, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::SOIL_T_6_FIELD:
	    _interp2dField(inData, "soil_t_6", inData.soil_t_6, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

          case Params::T2_FIELD:
            _interp2dField(inData, "T2", inData.t2, mdvx,
                         planeOffset, fhdr.missing_data_value, mGrid);
            break;

          case Params::Q2_FIELD:
            _interp2dField(inData, "Q2", inData.q2, mdvx,
                         planeOffset, fhdr.missing_data_value, mGrid);
            break;

          case Params::U10_FIELD:
            _interp2dField(inData, "U10", inData.u10, mdvx,
                         planeOffset, fhdr.missing_data_value, mGrid);
            break;

          case Params::V10_FIELD:
            _interp2dField(inData, "V10", inData.v10, mdvx,
                         planeOffset, fhdr.missing_data_value, mGrid);
            break;

	  case Params::SFCRNOFF_FIELD:
	    _interp2dField(inData, "sfcrnoff", inData.sfcrnoff, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::SWFRAC_FIELD:
	    _interp2dField(inData, "swfrac", inData.swfrac, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::SUNALT_FIELD:
	    _interp2dField(inData, "sunalt", inData.sunalt, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::SUNAZM_FIELD:
	    _interp2dField(inData, "sunazm", inData.sunazm, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::MOONALT_FIELD:
	    _interp2dField(inData, "moonalt", inData.moonalt, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::MOONAZM_FIELD:
	    _interp2dField(inData, "moonazm", inData.moonazm, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::SUNILL_FIELD:
	    _interp2dField(inData, "sunill", inData.sunill, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::MOONILL_FIELD:
	    _interp2dField(inData, "moonill", inData.moonill, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::TOTALILL_FIELD:
	    _interp2dField(inData, "totalill", inData.totalill, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::CLWI_FIELD:
	    _interp2dField(inData, "clwi", inData.clwi, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::RNWI_FIELD:
	    _interp2dField(inData, "rnwi", inData.rnwi, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::ICEI_FIELD:
	    _interp2dField(inData, "icei", inData.icei, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::SNOWI_FIELD:
	    _interp2dField(inData, "snowi", inData.snowi, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::PWV_FIELD:
	    _interp2dField(inData, "pwv", inData.pwv, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::SUN_BTW:
	    _interp2dField(inData, "sun_btw", inData.sun_btw, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::SUN_ETW:
	    _interp2dField(inData, "sun_etw", inData.sun_etw, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::SUN_ABTW:
	    _interp2dField(inData, "sun_abtw", inData.sun_abtw, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::SUN_AETW:
	    _interp2dField(inData, "sun_aetw", inData.sun_aetw, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::SUN_RISE:
	    _interp2dField(inData, "sun_rise", inData.sun_rise, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::SUN_SET:
	    _interp2dField(inData, "sun_set", inData.sun_set, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::SUN_ARIS:
	    _interp2dField(inData, "sun_aris", inData.sun_aris, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::SUN_ASET:
	    _interp2dField(inData, "sun_aset", inData.sun_aset, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::MOON_RIS:
	    _interp2dField(inData, "moon_ris", inData.moon_ris, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::MOON_SET:
	    _interp2dField(inData, "moon_set", inData.moon_set, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::MOON_ARI:
	    _interp2dField(inData, "moon_ari", inData.moon_ari, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::MOON_ASE:
	    _interp2dField(inData, "moon_ase", inData.moon_ase, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::NLAT_FIELD:
	    _interp2dField(inData, "latitude", inData.lat, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	  case Params::ELON_FIELD:
	    _interp2dField(inData, "longitude", inData.lon, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;

	    // derived 2d fields
	    
	  case Params::FZLEVEL_FIELD:
	    _interp2dField(inData, "FZLevel", inData.fzlevel, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	    
	  case Params::RAIN_TOTAL_FIELD:
	    _interp2dField(inData, "rain_total", inData.rain_total, mdvx,
			   planeOffset, fhdr.missing_data_value, mGrid);
	    break;
	
          case Params::RH2_FIELD:
            _interp2dField(inData, "RH2", inData.rh2, mdvx,
                         planeOffset, fhdr.missing_data_value, mGrid);
            break;

          case Params::DEWPT2_FIELD:
            _interp2dField(inData, "DewPt2", inData.dewpt2, mdvx,
                         planeOffset, fhdr.missing_data_value, mGrid);
            break;

          case Params::MSLP2_FIELD:
            _interp2dField(inData, "pressure2", inData.mslp2, mdvx,
                         planeOffset, fhdr.missing_data_value, mGrid, 100.);
            break;

	  } // switch

	} // ifield loop
	
      } // (iret == 0)
      
    } // ix
    
  } // iy

  // trim out unwanted turbulence data
  if (turbFieldNum >= 0) {
    _trimTurbField(mdvx, turbFieldNum);
  }

  // write out the GRIB file
  string outfileForecast = _params.output_file;
  if (lead_time > 0) {
    // add hour suffix to forecasts
    char hourSuffix[30];
    sprintf(hourSuffix, ".%02d", lead_time / 3600);
    outfileForecast += hourSuffix;
  }

  if (_params.debug) {
    cerr << "Writing out the GRIB file " << outfileForecast << " . . .\n" << endl;
  }
  writeGrib(mdvx, outfileForecast);

  if (_params.mdv == Params::MDV_ON) {
    // Prepare MDV output.
    // convert to desired output types for MDV
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

    // convert and compress each field
    for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    
      MdvxField *field = mdvx.getFieldByNum(ifield);

      if (_params._output_fields[ifield].encoding == Params::OUT_INT8) {
        field->convertRounded(Mdvx::ENCODING_INT8, compression_type);
      } else if (_params._output_fields[ifield].encoding == Params::OUT_INT16) {
        field->convertRounded(Mdvx::ENCODING_INT16, compression_type);
      }
    } // ifield

    // write out the MDV
    if (_params.debug) {
      cerr << "Writing out the MDV volume . . .\n" << endl;
    }
    if (outFile.writeVol()) {
      fprintf(stderr, "Cannot write output for time %s\n",
	    utimstr(inData.outputTime));
    }
  }

  // free up allocated objects
  if (itfa) {
    delete itfa;
  }

  return (0);

}

//////////////////
// _interp3dField()
//
// Interpolate the 3d field data onto the output grid point
//

void MM5toGrib::_interp3dField(MM5Data &inData,
			       const char *field_name,
			       fl32 ***field_data,
			       DsMdvx &mdvx,
			       int planeOffset,
			       int nPointsPlane,
			       fl32 missingDataVal,
			       const MM5Grid &mGrid,
			       double factor /* = 1.0*/ )
  
{

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
    cerr << "ERROR - MM5toGrib::_interp3dField" << endl;
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

void MM5toGrib::_interp2dField(MM5Data &inData,
			       const char *field_name,
			       fl32 **field_data,
			       DsMdvx &mdvx,
			       int planeOffset,
			       fl32 missingDataVal,
			       const MM5Grid &mGrid,
			       double factor /* = 1.0*/ )
  
{
  // get interpolated value for point
  double interpVal =
    inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
			 field_name, field_data,
			 mGrid.wtSW, mGrid.wtNW,
			 mGrid.wtNE, mGrid.wtSE);

  // check for error
  if (interpVal == MM5Data::MissingDouble) {
    cerr << "Field " << field_name << " was probably not present in the MM5 output file."
	    << endl;
    return;
  }

  // put into targetVol
  MdvxField *field = mdvx.getFieldByName(field_name);
  if (field == NULL) {
    cerr << "ERROR - MM5toGrib::_interp2dField" << endl;
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

////////////////////////////////////////////////////
// do we need to compute ITFA turbulence probability

bool MM5toGrib::_needItfa()

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

bool MM5toGrib::_needIcing()

{
  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    if (_params._output_fields[ifield].name == Params::ICING_FIELD) {
      return true;
    }
  }
  return false;
}

void MM5toGrib::_computeItfa(MM5Data &inData,
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
  
void MM5toGrib::_trimTurbField(DsMdvx &mdvx, int turbFieldNum)

{

  MdvxField *field = mdvx.getFieldByNum(turbFieldNum);
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
  
  return;

}
      
