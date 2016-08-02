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
// MM52Spdb.cc
//
// MM52Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2000
//
///////////////////////////////////////////////////////////////

#include "MM52Spdb.hh"
#include <mm5/MM5DataV2.hh>
#include <mm5/MM5DataV3.hh>
#include <physics/IcaoStdAtmos.hh>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <Spdb/DsSpdb.hh>
#include <didss/DsInputPath.hh>
using namespace std;

// Constructor

MM52Spdb::MM52Spdb(int argc, char **argv)

{

  OK = TRUE;
  _input = NULL;

  // set programe name

  _progName = "MM52Spdb";
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

  // create input path object

  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug,
			     _params.realtime_input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register);
  } else {
    _input = new DsInputPath(_progName,
			     _params.debug,
			     _args.nFiles,
			     _args.filePaths);
  }
  
  // init process mapper registration
  
  if (_params.mode == Params::REALTIME) {
    PMU_auto_init((char *) _progName.c_str(), _params.instance,
		  PROCMAP_REGISTER_INTERVAL);
    PMU_auto_register("In MM52Spdb constructor");
  }

  return;

}

// destructor

MM52Spdb::~MM52Spdb()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MM52Spdb::Run ()
{

  PMU_auto_register("MM52Spdb::Run");
  
  // run
  
  int iret = 0;

  while (true) {
    iret = _run();
  }

  return iret;

}

//////////////////////////////////////////////////
// _run

int MM52Spdb::_run()

{
  
  PMU_auto_register("MM52Spdb::_run");
  
  // loop through the input files
  
  char *filePath;
  while ((filePath = _input->next()) != NULL) {
    
    if (_params.debug >= Params::DEBUG_NORM) {
      fprintf(stderr, "Processing file %s\n", filePath);
    }
    
    int version;
    if (MM5Data::getVersion(filePath, version)) {
      fprintf(stderr, "Getting version number from file %s\n",
	      filePath);
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

    if (inData->OK) {
      if (inData->read() == 0) {

	time_t gen_time, forecast_time;
	int lead_time;
	gen_time = inData->modelTime;
	lead_time = inData->forecastLeadTime;

	// round the lead_time to nearest 15 mins, and compute forecast time

	lead_time = ((int) (lead_time / 900.0 + 0.5)) * 900;
	forecast_time = gen_time + lead_time;
	
	if (_params.debug) {
	  cerr << "gen_time: " << utimstr(gen_time) << endl;
	  cerr << "forecast_time: " << utimstr(forecast_time) << endl;
	  cerr << "lead_time: " << lead_time << endl;
	}

	_processForecast(*inData, gen_time, lead_time, forecast_time);

      }

    }
    
    delete (inData);
    
  }

  return (0);

}

/////////////////////////////////////////
// _processForecast()
//
// Process data for a given forecast

int MM52Spdb::_processForecast(MM5Data &inData,
			       time_t gen_time,
			       int lead_time,
			       time_t forecast_time)

{

  PMU_auto_register("In MM52Spdb::_processForecast");
  DsSpdb spdb;
  spdb.setAppName(_progName);
  spdb.setLeadTimeStorage(Spdb::LEAD_TIME_IN_DATA_TYPE2);

  // compute the derived fields

  inData.computeDerivedFields();
  
  // set up grid remapping object
  
  MM5Grid mGrid(_progName, _params.debug >= Params::DEBUG_VERBOSE,
		inData.nLat, inData.nLon,
		inData.lat, inData.lon);

  // loop through the points

  for (int ii = 0; ii < _params.output_points_n; ii++) {
    
    PMU_auto_register("In MM52Spdb::_processForecast loop");

    char *name = _params._output_points[ii].name;
    double lat = _params._output_points[ii].lat;
    double lon = _params._output_points[ii].lon;
      
    // find the model position for this point
      
    if (mGrid.findModelLoc(lat, lon)) {
      // point not with model
      if (_params.debug) {
	cerr << "Model data does not contain point: lat, lon: "
	     << lat << lon << endl;
      }
      continue;
    }
      
    if (_params.debug) {
      cerr << "Processing point, time: " << utimstr(forecast_time) << endl;
      cerr << "  lat, lon, mGrid.latIndex, mGrid.lonIndex: "
	   << lat << ", " << lon << ", "
	   << mGrid.latIndex << ", " << mGrid.lonIndex << endl;
    }

    // set up generic point object for single-level data


    GenPt singleLevelPt;
    _initSingle(singleLevelPt, inData, name,
		forecast_time, lat, lon);
    
    // load up 2d data

    _process2d(inData, mGrid, singleLevelPt);
    if (!singleLevelPt.check()) {
      cerr << "ERROR - MM52Spdb::_processForecast" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  Bad single level point object" << endl;
      return -1;
    }

    // set up generic point object for multi-level data
    
    GenPt multiLevelPt;
    _initMulti(multiLevelPt, inData, name,
	       forecast_time, lat, lon);

    // load up 3d data

    for (int isig = 0;  isig < inData.nSigma; isig++) {
      _process3d(inData, mGrid, isig, multiLevelPt);
    } // isig
    if (!multiLevelPt.check()) {
      cerr << "ERROR - MM52Spdb::_processForecast" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  Bad multi level point object" << endl;
      return -1;
    }
    
    // create the combo point object, combining the single-level and
    // multi-level data. Assemble the buffer.
    
    ComboPt comboPt;
    comboPt.setProdInfo("MM5 point data.");
    comboPt.set1DPoint(singleLevelPt);
    comboPt.set2DPoint(multiLevelPt);
    comboPt.assemble();

    // add the chunk

    spdb.addPutChunk(Spdb::hash4CharsToInt32(name),
		     forecast_time,
		     forecast_time + inData.forecastDelta,
		     comboPt.getBufLen(),
		     comboPt.getBufPtr(),
		     lead_time);

  } // ii

  // Put the data to Spdb
  
  if (spdb.put(_params.output_url,
	       SPDB_COMBO_POINT_ID,
	       SPDB_COMBO_POINT_LABEL)) {
    cerr << "ERROR - MM52Spdb::_processForecast" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  return (0);

}

/////////////////////////////////////////////////////
// process sigma level for this point

void MM52Spdb::_process3d(const MM5Data &inData,
			  const MM5Grid &mGrid,
			  int isig,
			  GenPt &pt)

{
  
  double pres = inData.interp3dLevel(isig, mGrid.latIndex, mGrid.lonIndex,
				     "Pres", inData.pres,
				     mGrid.wtSW, mGrid.wtNW,
				     mGrid.wtNE, mGrid.wtSE);
  pt.addVal(pres);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "pres: " << pres;
    IcaoStdAtmos isa;
    cerr << ", FL: " << isa.pres2flevel(pres);
  }

  for (int ifield = 0; ifield < _params.output_fields_3d_n; ifield++) {

    switch (_params._output_fields_3d[ifield]) {
	  
    case Params::U_3D: {
      double u = inData.interp3dLevel(isig, mGrid.latIndex, mGrid.lonIndex,
				      "U", inData.uu,
				      mGrid.wtSW, mGrid.wtNW,
				      mGrid.wtNE, mGrid.wtSE);
      pt.addVal(u);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", U: " << u;
      }
      break;
    }
      
    case Params::V_3D: {
      double v = inData.interp3dLevel(isig, mGrid.latIndex, mGrid.lonIndex,
				      "V", inData.vv,
				      mGrid.wtSW, mGrid.wtNW,
				      mGrid.wtNE, mGrid.wtSE);
      pt.addVal(v);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", V: " << v;
      }
      break;
    }

    case Params::W_3D: {
      double w = inData.interp3dLevel(isig, mGrid.latIndex, mGrid.lonIndex,
				      "W", inData.ww,
				      mGrid.wtSW, mGrid.wtNW,
				      mGrid.wtNE, mGrid.wtSE);
      pt.addVal(w);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", W: " << w;
      }
      break;
    }

    case Params::TEMP_3D: {
      double temp = inData.interp3dLevel(isig, mGrid.latIndex, mGrid.lonIndex,
					 "TEMP", inData.tc,
					 mGrid.wtSW, mGrid.wtNW,
					 mGrid.wtNE, mGrid.wtSE);
      pt.addVal(temp);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", TEMP: " << temp;
      }
      break;
    }
      
    case Params::HUMIDITY_3D: {
      double rh = inData.interp3dLevel(isig, mGrid.latIndex, mGrid.lonIndex,
				       "RH", inData.rh,
				       mGrid.wtSW, mGrid.wtNW,
				       mGrid.wtNE, mGrid.wtSE);
      pt.addVal(rh);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", RH: " << rh;
      }
      break;
    }
      
    case Params::CLW_3D: {
      double clw = inData.interp3dLevel(isig, mGrid.latIndex, mGrid.lonIndex,
					"CLW", inData.clw,
					mGrid.wtSW, mGrid.wtNW,
					mGrid.wtNE, mGrid.wtSE);
      pt.addVal(clw);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", CLW: " << clw;
      }
      break;
    }
      
    case Params::RNW_3D: {
      double rnw = inData.interp3dLevel(isig, mGrid.latIndex, mGrid.lonIndex,
					"RNW", inData.rnw,
					mGrid.wtSW, mGrid.wtNW,
					mGrid.wtNE, mGrid.wtSE);
      pt.addVal(rnw);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", RNW: " << rnw;
      }
      break;
    }
      
    case Params::ICE_3D: {
      double ice = inData.interp3dLevel(isig, mGrid.latIndex, mGrid.lonIndex,
					"ICE", inData.ice,
					mGrid.wtSW, mGrid.wtNW,
					mGrid.wtNE, mGrid.wtSE);
      pt.addVal(ice);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", ICE: " << ice;
      }
      break;
    }
    
    case Params::SNOW_3D: {
      double snow = inData.interp3dLevel(isig, mGrid.latIndex, mGrid.lonIndex,
					 "SNOW", inData.snow,
					 mGrid.wtSW, mGrid.wtNW,
					 mGrid.wtNE, mGrid.wtSE);
      pt.addVal(snow);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", SNOW: " << snow;
      }
      break;
    }
    
    case Params::RAD_TEND_3D: {
      double rad_tend =
	inData.interp3dLevel(isig, mGrid.latIndex, mGrid.lonIndex,
			     "RAD_TEND", inData.rad_tend,
			     mGrid.wtSW, mGrid.wtNW,
			     mGrid.wtNE, mGrid.wtSE);
      pt.addVal(rad_tend);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", RAD_TEND: " << rad_tend;
      }
      break;
    }
    
    } // switch

  } // ifield

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << endl;
  }

}

// process this point

void MM52Spdb::_process2d(const MM5Data &inData,
			  const MM5Grid &mGrid,
			  GenPt &pt)
  
{
  
  for (int ifield = 0; ifield < _params.output_fields_2d_n; ifield++) {
    
    switch (_params._output_fields_2d[ifield]) {
      
    case Params::GROUND_T_2D: {
      double ground_t = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					     "GROUND_T_2D", inData.ground_t,
					     mGrid.wtSW, mGrid.wtNW,
					     mGrid.wtNE, mGrid.wtSE);
      pt.addVal(ground_t);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "GROUND_T: " << ground_t;
      }
      break;
    }
    case Params::RAIN_CON_2D: {
      double rain_con = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					     "RAIN_CON_2D", inData.rain_con,
					     mGrid.wtSW, mGrid.wtNW,
					     mGrid.wtNE, mGrid.wtSE);
      pt.addVal(rain_con);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", RAIN_CON: " << rain_con;
      }
      break;
    }
    case Params::RAIN_NON_2D: {
      double rain_non = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					     "RAIN_NON_2D", inData.rain_non,
					     mGrid.wtSW, mGrid.wtNW,
					     mGrid.wtNE, mGrid.wtSE);
      pt.addVal(rain_non);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", RAIN_NON: " << rain_non;
      }
      break;
    }
    case Params::TERRAIN_2D: {
      double terrain = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					    "TERRAIN_2D", inData.terrain,
					    mGrid.wtSW, mGrid.wtNW,
					    mGrid.wtNE, mGrid.wtSE);
      pt.addVal(terrain);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", TERRAIN: " << terrain;
      }
      break;
    }
    case Params::CORIOLIS_2D: {
      double coriolis = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					     "CORIOLIS_2D", inData.coriolis,
					     mGrid.wtSW, mGrid.wtNW,
					     mGrid.wtNE, mGrid.wtSE);
      pt.addVal(coriolis);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", CORIOLIS: " << coriolis;
      }
      break;
    }
    case Params::RES_TEMP_2D: {
      double res_temp = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					     "RES_TEMP_2D", inData.res_temp,
					     mGrid.wtSW, mGrid.wtNW,
					     mGrid.wtNE, mGrid.wtSE);
      pt.addVal(res_temp);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", RES_TEMP: " << res_temp;
      }
      break;
    }
    case Params::LAND_USE_2D: {
      double land_use = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					     "LAND_USE_2D", inData.land_use,
					     mGrid.wtSW, mGrid.wtNW,
					     mGrid.wtNE, mGrid.wtSE);
      pt.addVal(land_use);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", LAND_USE: " << land_use;
      }
      break;
    }
    case Params::SNOWCOVR_2D: {
      double snowcovr = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					     "SNOWCOVR_2D", inData.snowcovr,
					     mGrid.wtSW, mGrid.wtNW,
					     mGrid.wtNE, mGrid.wtSE);
      pt.addVal(snowcovr);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", SNOWCOVR: " << snowcovr;
      }
      break;
    }
    case Params::TSEASFC_2D: {
      double tseasfc = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					    "TSEASFC_2D", inData.tseasfc,
					    mGrid.wtSW, mGrid.wtNW,
					    mGrid.wtNE, mGrid.wtSE);
      pt.addVal(tseasfc);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", TSEASFC: " << tseasfc;
      }
      break;
    }
    case Params::PBL_HGT_2D: {
      double pbl_hgt = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					    "PBL_HGT_2D", inData.pbl_hgt,
					    mGrid.wtSW, mGrid.wtNW,
					    mGrid.wtNE, mGrid.wtSE);
      pt.addVal(pbl_hgt);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", PBL_HGT: " << pbl_hgt;
      }
      break;
    }
    case Params::REGIME_2D: {
      double regime = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					   "REGIME_2D", inData.regime,
					   mGrid.wtSW, mGrid.wtNW,
					   mGrid.wtNE, mGrid.wtSE);
      pt.addVal(regime);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", REGIME: " << regime;
      }
      break;
    }
    case Params::SHFLUX_2D: {
      double shflux = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					   "SHFLUX_2D", inData.shflux,
					   mGrid.wtSW, mGrid.wtNW,
					   mGrid.wtNE, mGrid.wtSE);
      pt.addVal(shflux);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", SHFLUX: " << shflux;
      }
      break;
    }
    case Params::LHFLUX_2D: {
      double lhflux = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					   "LHFLUX_2D", inData.lhflux,
					   mGrid.wtSW, mGrid.wtNW,
					   mGrid.wtNE, mGrid.wtSE);
      pt.addVal(lhflux);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", LHFLUX: " << lhflux;
      }
      break;
    }
    case Params::UST_2D: {
      double ust = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					"UST_2D", inData.ust,
					mGrid.wtSW, mGrid.wtNW,
					mGrid.wtNE, mGrid.wtSE);
      pt.addVal(ust);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", UST: " << ust;
      }
      break;
    }
    case Params::SWDOWN_2D: {
      double swdown = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					    "SWDOWN_2D", inData.swdown,
					    mGrid.wtSW, mGrid.wtNW,
					   mGrid.wtNE, mGrid.wtSE);
      pt.addVal(swdown);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", SWDOWN: " << swdown;
      }
      break;
    }
    case Params::LWDOWN_2D: {
      double lwdown = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					   "LWDOWN_2D", inData.lwdown,
					   mGrid.wtSW, mGrid.wtNW,
					   mGrid.wtNE, mGrid.wtSE);
      pt.addVal(lwdown);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", LWDOWN: " << lwdown;
      }
      break;
    }
    case Params::SOIL_T_1_2D: {
      double soil_t_1 = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					     "SOIL_T_1_2D", inData.soil_t_1,
					     mGrid.wtSW, mGrid.wtNW,
					     mGrid.wtNE, mGrid.wtSE);
      pt.addVal(soil_t_1);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", SOIL_T_1: " << soil_t_1;
      }
      break;
    }
    case Params::SOIL_T_2_2D: {
      double soil_t_2 = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					    "SOIL_T_2_2D", inData.soil_t_2,
					     mGrid.wtSW, mGrid.wtNW,
					     mGrid.wtNE, mGrid.wtSE);
      pt.addVal(soil_t_2);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", SOIL_T_2: " << soil_t_2;
      }
      break;
    }
    case Params::SOIL_T_3_2D: {
      double soil_t_3 = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					     "SOIL_T_3_2D", inData.soil_t_3,
					     mGrid.wtSW, mGrid.wtNW,
					     mGrid.wtNE, mGrid.wtSE);
      pt.addVal(soil_t_3);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", SOIL_T_3: " << soil_t_3;
      }
      break;
    }
    case Params::SOIL_T_4_2D: {
      double soil_t_4 = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					     "SOIL_T_4_2D", inData.soil_t_4,
					     mGrid.wtSW, mGrid.wtNW,
					     mGrid.wtNE, mGrid.wtSE);
      pt.addVal(soil_t_4);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", SOIL_T_4: " << soil_t_4;
      }
      break;
    }
    case Params::SOIL_T_5_2D: {
      double soil_t_5 = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					     "SOIL_T_5_2D", inData.soil_t_5,
					     mGrid.wtSW, mGrid.wtNW,
					     mGrid.wtNE, mGrid.wtSE);
      pt.addVal(soil_t_5);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", SOIL_T_5: " << soil_t_5;
      }
      break;
    }
    case Params::SOIL_T_6_2D: {
      double soil_t_6 = inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
					     "SOIL_T_6_2D", inData.soil_t_6,
					     mGrid.wtSW, mGrid.wtNW,
					     mGrid.wtNE, mGrid.wtSE);
      pt.addVal(soil_t_6);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << ", SOIL_T_6: " << soil_t_6;
      }
      break;
    }

    } // switch

  } // ifield
    
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << endl;
  }
      
}

//////////////////////////////////////////////
// initialize the multi level pt

void MM52Spdb::_initMulti(GenPt &pt,
			  const MM5Data &inData,
			  const char *name,
			  time_t valid_time,
			  double lat, double lon)
  
{

  pt.clear();
  pt.setName(name);
  pt.setTime(valid_time);
  pt.setLat(lat);
  pt.setLon(lon);
  pt.setNLevels(inData.nSigma);

  pt.addFieldInfo("PRESSURE", "mb");
  
  for (int ifield = 0; ifield < _params.output_fields_3d_n; ifield++) {

    switch (_params._output_fields_3d[ifield]) {
	  
    case Params::U_3D: {
      pt.addFieldInfo("U", "m/s");
      break;
    }
    case Params::V_3D: {
      pt.addFieldInfo("V", "m/s");
      break;
    }
    case Params::W_3D: {
      pt.addFieldInfo("W", "m/s");
      break;
    }
    case Params::TEMP_3D: {
      pt.addFieldInfo("Temp", "C");
      break;
    }
    case Params::HUMIDITY_3D: {
      pt.addFieldInfo("RH", "%");
      break;
    }
    case Params::CLW_3D: {
      pt.addFieldInfo("CLW", "kg/kg");
      break;
    }
    case Params::RNW_3D: {
      pt.addFieldInfo("RNW", "kg/kg");
      break;
    }
    case Params::ICE_3D: {
      pt.addFieldInfo("ICE", "kg/kg");
      break;
    }
    case Params::SNOW_3D: {
      pt.addFieldInfo("SNOW", "kg/kg");
      break;
    }
    case Params::RAD_TEND_3D: {
      pt.addFieldInfo("RAD_TEND", "K/Day");
      break;
    }
    
    } // switch

  } // ifield

}

//////////////////////////////////////////////
// initialize the single level pt

void MM52Spdb::_initSingle(GenPt &pt,
			   const MM5Data &inData,
			   const char *name,
			   time_t valid_time,
			   double lat, double lon)
  
{

  pt.clear();
  pt.setName(name);
  pt.setTime(valid_time);
  pt.setLat(lat);
  pt.setLon(lon);
  pt.setNLevels(1);
  
  for (int ifield = 0; ifield < _params.output_fields_2d_n; ifield++) {

    switch (_params._output_fields_2d[ifield]) {
      
    case Params::GROUND_T_2D: {
      pt.addFieldInfo("GROUND_T", "K");
      break;
    }
    case Params::RAIN_CON_2D: {
      pt.addFieldInfo("RAIN_CON", "cm");
      break;
    }
    case Params::RAIN_NON_2D: {
      pt.addFieldInfo("RAIN_NON", "cm");
      break;
    }
    case Params::TERRAIN_2D: {
      pt.addFieldInfo("TERRAIN", "m");
      break;
    }
    case Params::CORIOLIS_2D: {
      pt.addFieldInfo("CORIOLIS", "1/s");
      break;
    }
    case Params::RES_TEMP_2D: {
      pt.addFieldInfo("RES_TEMP", "K");
      break;
    }
    case Params::LAND_USE_2D: {
      pt.addFieldInfo("LAND_USE", "category");
      break;
    }
    case Params::SNOWCOVR_2D: {
      pt.addFieldInfo("SNOWCOVR", "");
      break;
    }
    case Params::TSEASFC_2D: {
      pt.addFieldInfo("TSEASFC", "K");
      break;
    }
    case Params::PBL_HGT_2D: {
      pt.addFieldInfo("PBL_HGT", "m");
      break;
    }
    case Params::REGIME_2D: {
      pt.addFieldInfo("REGIME", "");
      break;
    }
    case Params::SHFLUX_2D: {
      pt.addFieldInfo("SHFLUX", "W/m2");
      break;
    }
    case Params::LHFLUX_2D: {
      pt.addFieldInfo("LHFLUX", "W/m2");
      break;
    }
    case Params::UST_2D: {
      pt.addFieldInfo("UST", "m/s");
      break;
    }
    case Params::SWDOWN_2D: {
      pt.addFieldInfo("SWDOWN", "W/m2");
      break;
    }
    case Params::LWDOWN_2D: {
      pt.addFieldInfo("LWDOWN", "W/m2");
      break;
    }
    case Params::SOIL_T_1_2D: {
      pt.addFieldInfo("SOIL_T_1", "K");
      break;
    }
    case Params::SOIL_T_2_2D: {
      pt.addFieldInfo("SOIL_T_2", "K");
      break;
    }
    case Params::SOIL_T_3_2D: {
      pt.addFieldInfo("SOIL_T_3", "K");
      break;
    }
    case Params::SOIL_T_4_2D: {
      pt.addFieldInfo("SOIL_T_4", "K");
      break;
    }
    case Params::SOIL_T_5_2D: {
      pt.addFieldInfo("SOIL_T_5", "K");
      break;
    }
    case Params::SOIL_T_6_2D: {
      pt.addFieldInfo("SOIL_T_6", "K");
      break;
    }
    
    } // switch
    
  } // ifield

}


  

