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
// MdvPull.cc
//
// MdvPull object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2011
//
///////////////////////////////////////////////////////////////
//
// MdvPull reads data from a remote server, and duplicates it on
// a local host or a target server.
//
////////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/Path.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRemapLut.hh>
#include <dsserver/DsLdataInfo.hh>
#include "MdvPull.hh"
using namespace std;

// Constructor

MdvPull::MdvPull(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "MdvPull";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
    }
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		_params.reg_interval);

  return;

}

//////////////////////////
// destructor

MdvPull::~MdvPull()
{
  // unregister process

  PMU_auto_unregister();
}

//////////////////////////////////////////////////
// Run

int MdvPull::Run()

{

  if (_params.mode == Params::REALTIME) {
    return _runRealtime();
  } else {
    return _runArchive();
  }

}

//////////////////////////////////////////////////
// Run in realtime mode

int MdvPull::_runRealtime()

{

  int iret = 0;

  // register with procmap
  
  PMU_auto_register("Run realtime");
  
  // initialize the next search time

  time_t now = time(NULL);
  _nextSearchTime = ((now / _params.new_data_poll_interval)
                     * _params.new_data_poll_interval);

  // poll for data

  while (true) {

    now = time(NULL);
    if (now >= _nextSearchTime) {

      PMU_auto_register("Searching for data");

      // past the required time, look for new data

      _startTime = _nextSearchTime - _params.realtime_lookback_secs;
      _endTime = _nextSearchTime + _params.realtime_lookforward_secs;
      
      if (_retrieveData()) {
        iret = -1;
      }
      
      _nextSearchTime += _params.new_data_poll_interval;

    } else {

      // sleep for a sec

      PMU_auto_register("zzz...");
      umsleep(1000);

    }

  } // while

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int MdvPull::_runArchive()

{

  // register with procmap
  
  PMU_auto_register("Run archive");
  
  _startTime = _args.startTime;
  _endTime = _args.endTime;
  
  if (_retrieveData()) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// retrieve data, store it

int MdvPull::_retrieveData()

{

  // compile time list for source

  if (_compileSourceTimeList()) {
    return -1;
  }

  // compile time list for source

  if (_compileOutputTimeList()) {
    return -1;
  }

  if (_params.search_mode == Params::VALID_TIME) {
    return _retrieveValidData();
  } else {
    return _retrieveForecastData();
  }

  // retrieve data

  return 0;

}

//////////////////////////////////////////////////
// retrieve valid data, store it

int MdvPull::_retrieveValidData()

{

  // check which times are missing

  vector<time_t> missingTimes;
  for (size_t ii = 0; ii < _sourceValidTimes.size(); ii++) {
    time_t validTime = _sourceValidTimes[ii];
    bool alreadyHere = false;
    for (size_t jj = 0; jj < _outputValidTimes.size(); jj++) {
      if (_outputValidTimes[jj] == validTime) {
        alreadyHere = true;
        break;
      }
    }
    if (!alreadyHere) {
      missingTimes.push_back(validTime);
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      if (alreadyHere) {
        cerr << "Already got data for valid time: "
             << DateTime::strm(validTime) << endl;
      } else {
        cerr << "Need to get data for valid time: "
             << DateTime::strm(validTime) << endl;
      }
    }
  }

  int iret = 0;
  for (size_t ii = 0; ii < missingTimes.size(); ii++) {
    if (_retrieveForValidTime(missingTimes[ii])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// retrieve forecast data, store it

int MdvPull::_retrieveForecastData()

{

  // check which times are missing

  vector<time_t> genMissing, forecastMissing;
  vector<time_t> genHere, forecastHere;
  
  for (size_t ii = 0; ii < _sourceGenTimes.size(); ii++) {
    size_t nSourceFcasts = _sourceForecastTimesArray[ii].size();
    time_t genTime = _sourceGenTimes[ii];
    size_t genIndex = 0;
    bool genFound = false;
    for (size_t jj = 0; jj < _outputGenTimes.size(); jj++) {
      if (_outputGenTimes[jj] == genTime) {
        genIndex = jj;
        genFound = true;
        break;
      }
    }
    if (!genFound) {
      // all forecasts are missing for this gen time
      for (size_t kk = 0; kk < nSourceFcasts; kk++) {
        genMissing.push_back(genTime);
        time_t forecastTime = _sourceForecastTimesArray[ii][kk];
        forecastMissing.push_back(forecastTime);
      }
    } else {
      // gen time is here
      // search for which forecast times may be missing
      size_t nOutputFcasts = _outputForecastTimesArray[genIndex].size();
      for (size_t kk = 0; kk < nSourceFcasts; kk++) {
        bool gotForecast = false;
        time_t forecastTime = _sourceForecastTimesArray[ii][kk];
        for (size_t mm = 0; mm < nOutputFcasts; mm++) {
          if (_outputForecastTimesArray[genIndex][mm] == forecastTime) {
            gotForecast = true;
            break;
          }
        }
        if (gotForecast) {
          genHere.push_back(genTime);
          forecastHere.push_back(forecastTime);
        } else {
          genMissing.push_back(genTime);
          forecastMissing.push_back(forecastTime);
        }
      }
    } // if (genIndex < 0)
  } // ii

  // debug print

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < genHere.size(); ii++) {
      time_t leadSecs = forecastHere[ii] - genHere[ii];
      cerr << "Already got data for gen, forecast, leadSecs: "
           << DateTime::strm(genHere[ii]) << ", "
           << DateTime::strm(forecastHere[ii]) << ", "
           << leadSecs << endl;
    }
    for (size_t ii = 0; ii < genMissing.size(); ii++) {
      time_t leadSecs = forecastMissing[ii] - genMissing[ii];
      cerr << "Data missing for gen, forecast, leadSecs: "
           << DateTime::strm(genMissing[ii]) << ", "
           << DateTime::strm(forecastMissing[ii]) << ", "
           << leadSecs << endl;
    }
  }

  int iret = 0;
  for (size_t ii = 0; ii < genMissing.size(); ii++) {
    if (_retrieveForGenAndForecastTime(genMissing[ii],
                                       forecastMissing[ii])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// retrieve for specified valid time

int MdvPull::_retrieveForValidTime(time_t validTime)

{

  PMU_auto_register("Retrieving valid");
    
  // create DsMdvx object
  
  DsMdvx mdvx;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.setDebug(true);
  }
 
  // set up the Mdvx read
  
  mdvx.clear();
  _setupRead(mdvx);
     
  mdvx.setReadTime(Mdvx::READ_CLOSEST,
                   _params.source_url,
                   0, validTime);
    
  PMU_auto_register("Before read");
    
  // do the read
  
  if (mdvx.readVolume()) {
    cerr << "ERROR - MdvPull::_retrieveForValidTime" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Reading from url: " << _params.source_url << endl;
    cerr << "            file: " << mdvx.getPathInUse() << endl;
  }
  
  // set up the write

  _setupWrite(mdvx);
  
  // perform write

  if (_performWrite(mdvx)) {
    cerr << "ERROR - MdvPull::_retrieveForValidTime" << endl;
    cerr << "  Cannot write data set." << endl;
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// retrieve for specified gen and forecast time

int MdvPull::_retrieveForGenAndForecastTime(time_t genTime,
                                            time_t forecastTime)

{

  PMU_auto_register("Retrieving forecast");
    
  // create DsMdvx object
  
  DsMdvx mdvx;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.setDebug(true);
  }
 
  // set up the Mdvx read
  
  mdvx.clear();
  _setupRead(mdvx);

  time_t leadSecs = forecastTime - genTime;
     
  mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                   _params.source_url,
                   0, genTime, (int) leadSecs);
    
  PMU_auto_register("Before read");
    
  // do the read
  
  if (mdvx.readVolume()) {
    cerr << "ERROR - MdvPull::_retrieveForGenAndForecastTime" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Reading from url: " << _params.source_url << endl;
    cerr << "            file: " << mdvx.getPathInUse() << endl;
  }
  
  // set up the write

  _setupWrite(mdvx);
  mdvx.setWriteAsForecast();
  
  // perform write
  
  if (_performWrite(mdvx)) {
    cerr << "ERROR - MdvPull::_retrieveForGenAndForecastTime" << endl;
    cerr << "  Cannot write data set." << endl;
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// compile source time list

int MdvPull::_compileSourceTimeList()

{

  // get time list for source data

  DsMdvx mdvxIn;
  if (_params.search_mode == Params::VALID_TIME) {
    mdvxIn.setTimeListModeValid(_params.source_url,
                                _startTime, _endTime);
  } else {
    mdvxIn.setTimeListModeGenPlusForecasts(_params.source_url,
                                           _startTime, _endTime);
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Source time list request:" << endl;
    mdvxIn.printTimeListRequest(cerr);
  }
  
  if (mdvxIn.compileTimeList()) {
    cerr << "ERROR - MdvPull::_retrieveData()" << endl;
    cerr << "  Cannot compile time list for source data" << endl;
    cerr << "  url: " << _params.source_url << endl;
    cerr << mdvxIn.getErrStr() << endl;
    return -1;
  }

  // store time lists

  if (_params.search_mode == Params::VALID_TIME) {
    _sourceValidTimes = mdvxIn.getValidTimes();
  } else {
    _sourceGenTimes = mdvxIn.getGenTimes();
    _sourceForecastTimesArray = mdvxIn.getForecastTimesArray();
  }

  // debug print

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    if (_params.search_mode == Params::VALID_TIME) {
      cerr << "Source valid times:" << endl;
      for (size_t ii = 0; ii < _sourceValidTimes.size(); ii++) {
        cerr << "  " << DateTime::strm(_sourceValidTimes[ii]) << endl;
      }
    } else {
      cerr << "Source gen times:" << endl;
      for (size_t ii = 0; ii < _sourceGenTimes.size(); ii++) {
        time_t genTime = _sourceGenTimes[ii];
        cerr << "  " << DateTime::strm(genTime) << endl;
        cerr << "  Forecast times (lead secs):" << endl;
        for (size_t jj = 0; jj < _sourceForecastTimesArray[ii].size(); jj++) {
          time_t forecastTime = _sourceForecastTimesArray[ii][jj];
          int leadTime = forecastTime - genTime;
          cerr << "    " << DateTime::strm(forecastTime)
               << " (" << leadTime << ")" << endl;
        }
      }
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// compile output time list

int MdvPull::_compileOutputTimeList()

{

  // get time list for output data
  
  DsMdvx mdvxOut;
  if (_params.search_mode == Params::VALID_TIME) {
    mdvxOut.setTimeListModeValid(_params.output_url,
                                _startTime, _endTime);
  } else {
    mdvxOut.setTimeListModeGenPlusForecasts(_params.output_url,
                                           _startTime, _endTime);
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Output time list request:" << endl;
    mdvxOut.printTimeListRequest(cerr);
  }
  
  if (mdvxOut.compileTimeList()) {
    cerr << "ERROR - MdvPull::_retrieveData()" << endl;
    cerr << "  Cannot compile time list for output data" << endl;
    cerr << "  url: " << _params.output_url << endl;
    cerr << mdvxOut.getErrStr() << endl;
    return -1;
  }

  // store time lists

  if (_params.search_mode == Params::VALID_TIME) {
    _outputValidTimes = mdvxOut.getValidTimes();
  } else {
    _outputGenTimes = mdvxOut.getGenTimes();
    _outputForecastTimesArray = mdvxOut.getForecastTimesArray();
  }

  // debug print

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    if (_params.search_mode == Params::VALID_TIME) {
      cerr << "Output valid times:" << endl;
      for (size_t ii = 0; ii < _outputValidTimes.size(); ii++) {
        cerr << "  " << DateTime::strm(_outputValidTimes[ii]) << endl;
      }
    } else {
      cerr << "Output gen times:" << endl;
      for (size_t ii = 0; ii < _outputGenTimes.size(); ii++) {
        time_t genTime = _outputGenTimes[ii];
        cerr << "  " << DateTime::strm(genTime) << endl;
        cerr << "  Forecast times and lead times:" << endl;
        for (size_t jj = 0; jj < _outputForecastTimesArray[ii].size(); jj++) {
          time_t forecastTime = _outputForecastTimesArray[ii][jj];
          int leadTime = forecastTime - genTime;
          cerr << "    " << DateTime::strm(forecastTime) << ":" << leadTime << endl;
        }
      }
    }
  }

  return 0;

}

////////////////////////////////////////////
// set up the read

void MdvPull::_setupRead(DsMdvx &mdvx)

{

  // set up the Mdvx read
  
  mdvx.clearRead();
  
  // get file headers, to save encoding and compression info

  mdvx.setReadFieldFileHeaders();
  
  if (_params.set_horiz_limits) {
    mdvx.setReadHorizLimits(_params.horiz_limits.min_lat,
			    _params.horiz_limits.min_lon,
			    _params.horiz_limits.max_lat,
			    _params.horiz_limits.max_lon);
  }
  
  if (_params.set_vlevel_limits) {
    mdvx.setReadVlevelLimits(_params.lower_vlevel,
			     _params.upper_vlevel);
  }
  
  if (_params.composite) {
    mdvx.setReadComposite();
  }

  if (_params.set_field_names) {
    for (int i = 0; i < _params.field_names_n; i++) {
      mdvx.addReadField(_params._field_names[i]);
    }
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.printReadRequest(cerr);
  }

}

////////////////////////////////////////////
// remap

void MdvPull::_remap(DsMdvx &mdvx)

{

  for (size_t ifld = 0; ifld < mdvx.getNFields(); ifld++) {
    
    MdvxField *field = mdvx.getField(ifld);
    
    if (field == NULL) {
      cerr << "ERROR - MdvxConvert::_remap" << endl;
      cerr << "  Error remapping field #" << ifld <<
	" in output file" << endl;
      return;
    }
    
    if (_params.remap_projection == Params::PROJ_LATLON) {
      field->remap2Latlon(_remapLut,
			  _params.remap_grid.nx,
			  _params.remap_grid.ny,
			  _params.remap_grid.minx,
			  _params.remap_grid.miny,
			  _params.remap_grid.dx,
			  _params.remap_grid.dy);
    } else if (_params.remap_projection == Params::PROJ_FLAT) {
      field->remap2Flat(_remapLut,
			_params.remap_grid.nx,
			_params.remap_grid.ny,
			_params.remap_grid.minx,
			_params.remap_grid.miny,
			_params.remap_grid.dx,
			_params.remap_grid.dy,
			_params.remap_origin_lat,
			_params.remap_origin_lon,
			_params.remap_rotation,
                        _params.remap_false_northing,
                        _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_LAMBERT_CONF)	{
      field->remap2LambertConf(_remapLut,
			       _params.remap_grid.nx,
			       _params.remap_grid.ny,
			       _params.remap_grid.minx,
			       _params.remap_grid.miny,
			       _params.remap_grid.dx,
			       _params.remap_grid.dy,
			       _params.remap_origin_lat,
			       _params.remap_origin_lon,
			       _params.remap_lat1,
			       _params.remap_lat2,
                               _params.remap_false_northing,
                               _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_POLAR_STEREO) {
      Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH;
      if (!_params.remap_pole_is_north) {
	poleType = Mdvx::POLE_SOUTH;
      }
      field->remap2PolarStereo(_remapLut,
			       _params.remap_grid.nx,
			       _params.remap_grid.ny,
			       _params.remap_grid.minx,
			       _params.remap_grid.miny,
			       _params.remap_grid.dx,
			       _params.remap_grid.dy,
			       _params.remap_origin_lat,
			       _params.remap_origin_lon,
			       _params.remap_tangent_lon,
			       poleType,
			       _params.remap_central_scale,
                               _params.remap_false_northing,
                               _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_OBLIQUE_STEREO) {
      field->remap2ObliqueStereo(_remapLut,
				 _params.remap_grid.nx,
				 _params.remap_grid.ny,
				 _params.remap_grid.minx,
				 _params.remap_grid.miny,
				 _params.remap_grid.dx,
				 _params.remap_grid.dy,
				 _params.remap_origin_lat,
				 _params.remap_origin_lon,
				 _params.remap_tangent_lat,
				 _params.remap_tangent_lon,
                                 _params.remap_false_northing,
                                 _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_MERCATOR) {
      field->remap2Mercator(_remapLut,
			    _params.remap_grid.nx,
			    _params.remap_grid.ny,
			    _params.remap_grid.minx,
			    _params.remap_grid.miny,
			    _params.remap_grid.dx,
			    _params.remap_grid.dy,
			    _params.remap_origin_lat,
			    _params.remap_origin_lon,
                            _params.remap_false_northing,
                            _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_TRANS_MERCATOR) {
      field->remap2TransverseMercator(_remapLut,
				      _params.remap_grid.nx,
				      _params.remap_grid.ny,
				      _params.remap_grid.minx,
				      _params.remap_grid.miny,
				      _params.remap_grid.dx,
				      _params.remap_grid.dy,
				      _params.remap_origin_lat,
				      _params.remap_origin_lon,
				      _params.remap_central_scale,
                                      _params.remap_false_northing,
                                      _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_ALBERS) {
      field->remap2Albers(_remapLut,
			  _params.remap_grid.nx,
			  _params.remap_grid.ny,
			  _params.remap_grid.minx,
			  _params.remap_grid.miny,
			  _params.remap_grid.dx,
			  _params.remap_grid.dy,
			  _params.remap_origin_lat,
			  _params.remap_origin_lon,
			  _params.remap_lat1,
			  _params.remap_lat2,
                          _params.remap_false_northing,
                          _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_LAMBERT_AZIM) {
      field->remap2LambertAzimuthal(_remapLut,
				    _params.remap_grid.nx,
				    _params.remap_grid.ny,
				    _params.remap_grid.minx,
				    _params.remap_grid.miny,
				    _params.remap_grid.dx,
				    _params.remap_grid.dy,
				    _params.remap_origin_lat,
				    _params.remap_origin_lon,
                                    _params.remap_false_northing,
                                    _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_VERT_PERSP) {
      field->remap2VertPersp(_remapLut,
                             _params.remap_grid.nx,
                             _params.remap_grid.ny,
                             _params.remap_grid.minx,
                             _params.remap_grid.miny,
                             _params.remap_grid.dx,
                             _params.remap_grid.dy,
                             _params.remap_origin_lat,
                             _params.remap_origin_lon,
                             _params.remap_persp_radius,
                             _params.remap_false_northing,
                             _params.remap_false_easting);
     }
  }
  
}

////////////////////////////////////////////
// auto remap to latlon grid
//
// Automatically picks the grid resolution and extent
// from the existing data.

void MdvPull::_autoRemapToLatLon(DsMdvx &mdvx)

{
  
  for (size_t ifld = 0; ifld < mdvx.getNFields(); ifld++) {
    
    MdvxField *field = mdvx.getField(ifld);
    
    if (field == NULL) {
      cerr << "ERROR - MdvxConvert::_autoRemapToLatLon" << endl;
      cerr << "  Error remapping field #" << ifld <<
	" in output file" << endl;
      return;
    }
    
    field->autoRemap2Latlon(_remapLut);
  }
  
}

////////////////////////////////////////////
// convert to output types

void MdvPull::_convertOutput(DsMdvx &mdvx)
  
{

  for (size_t ii = 0; ii < mdvx.getNFields(); ii++) {
    
    MdvxField *field = mdvx.getField(ii);
    // Mdvx::field_header_t fhdr = field->getFieldHeader();
    const Mdvx::field_header_t *fhdrFile = field->getFieldHeaderFile();

    Mdvx::encoding_type_t encoding =
      (Mdvx::encoding_type_t) fhdrFile->encoding_type;
    if (_params.encoding_type != Params::ENCODING_ASIS) {
      encoding = (Mdvx::encoding_type_t)_params.encoding_type;
    }

    Mdvx::compression_type_t compression =
      (Mdvx::compression_type_t) fhdrFile->compression_type;
    if (_params.compression_type != Params::COMPRESSION_ASIS) {
      compression = (Mdvx::compression_type_t)_params.compression_type;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Setting output compression: "
             << Mdvx::compressionType2Str(compression) << endl;
      }
    }

    if (field->convertType(encoding, compression)) {
      cerr << "ERROR - MdvPull::_convertOutput" << endl;
      cerr << field->getErrStr() << endl;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "--->>> output field <<<---" << endl;
      field->printHeaders(cerr);
      cerr << "--------------------------" << endl;
    }
    
  }

}

//////////////////////////////////////////////////////////
// rename fields

void MdvPull::_renameFields(DsMdvx &mdvx)
{

  for (size_t ii = 0; ii < mdvx.getNFields(); ii++) {

    MdvxField *field = mdvx.getField(ii);
      
    for (int jj = 0; jj < _params.new_names_n; jj++) {
      if ((strcmp(field->getFieldName(),
                 _params._new_names[jj].old_field_name)==0) ||
	(strcmp(field->getFieldNameLong(),
		_params._new_names[jj].old_field_name)==0)) {
	  field->setFieldName(_params._new_names[jj].new_field_name);
	  field->setFieldNameLong(_params._new_names[jj].new_field_name);
      }
    } // jj
    
  } // ii

}

///////////////////////////////////////////////////////////
// set up the write, performing conversions as appropriate

void MdvPull::_setupWrite(DsMdvx &mdvx)

{

  // Remap the data if requested
  
  if (_params.auto_remap_to_latlon) {
    _autoRemapToLatLon(mdvx);
  } else if (_params.remap_xy) {
    _remap(mdvx);
  }
  
  if (_params.rename_fields) {
    _renameFields(mdvx);
  }
  
  // Convert to output encoding, type and scaling
  
  _convertOutput(mdvx);
  
  // set up write
  
  mdvx.clearWrite(); 
  
  // write using extended paths?
  
  if (_params.write_using_extended_paths) {
    mdvx.setWriteUsingExtendedPath();
  }
  
  if (_params.output_format == Params::OUTPUT_FORMAT_XML) {
    mdvx.setWriteFormat(Mdvx::FORMAT_XML);
    if (_params.debug) {
      cerr << "Will write output as xml" << endl;
    }    
  } else if (_params.output_format == Params::OUTPUT_FORMAT_NCF) {
    mdvx.setWriteFormat(Mdvx::FORMAT_NCF);
    if (_params.debug) {
      cerr << "Will write output as netCDF CF" << endl;
    }    
  }
  
  if (_params.write_latest_data_info) {
    mdvx.setWriteLdataInfo();
  } else {
    mdvx.clearWriteLdataInfo();
  }

  mdvx.setAppName(_progName);
  
  // netCDF MDV to NCF
  
  if (_params.output_format == Params::OUTPUT_FORMAT_NCF) {
    
    mdvx.clearMdv2Ncf();
    
    if (_params.ncf_set_global_attributes) {
      mdvx.setMdv2NcfAttr(_params.ncf_global_attributes.institution,
                          _params.ncf_global_attributes.references,
                          _params.ncf_global_attributes.comment);
    }
    
    if (_params.ncf_transform_fields) {
      for (int ii = 0; ii < _params.ncf_field_transforms_n; ii++) {
        const Params::ncf_field_transform_t &trans =
          _params._ncf_field_transforms[ii];
        DsMdvx::ncf_pack_t packing = DsMdvx::NCF_PACK_ASIS;
        if (trans.packed_data_type == Params::DATA_PACK_FLOAT) {
          packing = DsMdvx::NCF_PACK_FLOAT;
        } else if (trans.packed_data_type == Params::DATA_PACK_BYTE) {
          packing = DsMdvx::NCF_PACK_BYTE;
        } else if (trans.packed_data_type == Params::DATA_PACK_SHORT) {
          packing = DsMdvx::NCF_PACK_SHORT;
        }
        mdvx.addMdv2NcfTrans(trans.mdv_field_name,
                             trans.ncf_field_name,
                             trans.ncf_standard_name,
                             trans.ncf_long_name,
                             trans.ncf_units,
                             trans.do_linear_transform,
                             trans.linear_multiplier,
                             trans.linear_const,
                             packing);
      } // ii
    }
    
    if (_params.ncf_file_format == Params::CLASSIC) {
      mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_CLASSIC);
    } else if (_params.ncf_file_format == Params::NC64BIT) {
      mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_OFFSET64BITS);
    } else if  (_params.ncf_file_format == Params::NETCDF4_CLASSIC) {
      mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_NETCFD4_CLASSIC);
    } else {
      mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_NETCDF4);
    }
    
    if (_params.ncf_polar_radar_file_type == Params::FILE_TYPE_CF_RADIAL) {
      mdvx.setRadialFileType(DsMdvx::RADIAL_TYPE_CF_RADIAL);
    } else if (_params.ncf_polar_radar_file_type == Params::FILE_TYPE_DORADE) {
      mdvx.setRadialFileType(DsMdvx::RADIAL_TYPE_DORADE);
    } else if (_params.ncf_polar_radar_file_type == Params::FILE_TYPE_UF) {
      mdvx.setRadialFileType(DsMdvx::RADIAL_TYPE_UF);
    } else {
      mdvx.setRadialFileType(DsMdvx::RADIAL_TYPE_CF);
    }
  
    mdvx.setMdv2NcfCompression(_params.ncf_compress_data,
                               _params.ncf_compression_level);
    
    mdvx.setMdv2NcfOutput(_params.ncf_output_latlon_arrays,
                          _params.ncf_output_mdv_attributes,
                          _params.ncf_output_mdv_chunks,
                          _params.ncf_output_start_end_times);
    
    mdvx.setNcfFileSuffix(_params.ncf_file_suffix);
    
  } // if (_params.output_format == Params::OUTPUT_FORMAT_NCF)
    
}

//////////////////////////////////////////////////
// Perform the write

int MdvPull::_performWrite(DsMdvx &mdvx)

{

  PMU_auto_register("Before write");
  
  if (_params.output_format != Params::OUTPUT_FORMAT_NCF) {

    // not NetCDF

    if(mdvx.writeToDir(_params.output_url)) {
      cerr << "ERROR - MdvPull::_retrieveForValidTime" << endl;
      cerr << "  Cannot write data set." << endl;
      cerr << mdvx.getErrStr() << endl;
      return -1;
    }
  
    if (_params.debug) {
      cerr << "--->> Wrote to url:" << _params.output_url << endl;
      cerr << "              file:" << mdvx.getPathInUse() << endl;
    }

    return 0;

  }
  
  // perform the translation to NetCDF

  DsURL outputUrl(_params.output_url);
  string outputDir = outputUrl.getFile();

  if (mdvx.getProjection() == Mdvx::PROJ_POLAR_RADAR && 
      _params.ncf_polar_radar_file_type != Params::FILE_TYPE_CF) {
    
    // specified radial data type
    
    Mdv2NcfTrans trans;
    trans.setDebug(_params.debug);
    if (_params.ncf_polar_radar_file_type == Params::FILE_TYPE_DORADE) {
      trans.setRadialFileType(DsMdvx::RADIAL_TYPE_DORADE);
    } else if (_params.ncf_polar_radar_file_type == Params::FILE_TYPE_UF) {
      trans.setRadialFileType(DsMdvx::RADIAL_TYPE_UF);
    } else {
      trans.setRadialFileType(DsMdvx::RADIAL_TYPE_CF_RADIAL);
    }
    if (trans.writeCfRadial(mdvx, outputDir)) {
      cerr << "ERROR - Mdv2NetCDF::_processData()" << endl;
      cerr << trans.getErrStr() << endl;
      return 1;
    }
    string outputPath = trans.getNcFilePath();
    // write latest data info
    _writeNcfLdataInfo(mdvx, outputDir, outputPath);

  } else {

    // basic CF

    string outputPath = _computeNcfOutputPath(mdvx, outputDir);
    Mdv2NcfTrans trans;
    trans.setDebug(_params.debug);
    trans.setRadialFileType(DsMdvx::RADIAL_TYPE_CF);
    if (trans.writeCf(mdvx, outputPath)) {
      cerr << "ERROR - Mdv2NetCDF::_processData()" << endl;
      cerr << trans.getErrStr() << endl;
      return 1;
    }
    
    // write latest data info
    _writeNcfLdataInfo(mdvx, outputDir, outputPath);
    
  }
    
  return 0;

}

//////////////////////////////////////
// Compute output path for netCDF file

string MdvPull::_computeNcfOutputPath(const DsMdvx &mdvx,
                                      string &outputDir)
{

  // Get the proper time to assign to filename

  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();
  DateTime validTime(mhdr.time_centroid);
  DateTime genTime(mhdr.time_gen);
  
  bool isForecast = false;
  int year, month, day;
  // int hour, minute, seconds;
  
  if (mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED ||
      mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
      mhdr.forecast_time > 0) {
    year = genTime.getYear();
    month = genTime.getMonth();
    day =  genTime.getDay();
    // hour = genTime.getHour();
    // minute = genTime.getMin();
    // seconds = genTime.getSec();
    isForecast = true;
  } else {
    year = validTime.getYear();
    month = validTime.getMonth();
    day =  validTime.getDay();
    // hour = validTime.getHour();
    // minute = validTime.getMin();
    // seconds = validTime.getSec();
  }

  if(_params.search_mode != Params::GEN_TIME){
    isForecast = false;
  }

  // determine if we have polar radar data

  bool isPolar = false;
  bool isRhi = false;
  bool isSector = false;
  int nSweeps = 1;
  double fixedAngle = 0.0;
  if (mdvx.getNFields() > 0) {
    const MdvxField *field = mdvx.getField(0);
    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    if (fhdr.proj_type == Mdvx::PROJ_POLAR_RADAR) {
      isPolar = true;
      if (fhdr.vlevel_type == Mdvx::VERT_TYPE_AZ) {
        isRhi = true;
      } else {
        double angleCoverage = fhdr.ny * fhdr.grid_dy;
        if (angleCoverage < 330) {
          isSector = true;
        }
      }
      nSweeps = fhdr.nz;
      if (nSweeps == 1) {
        const Mdvx::vlevel_header_t &vhdr = field->getVlevelHeader();
        fixedAngle = vhdr.level[0];
      }
    }
  } // if (mdvx.getNFields() > 0) 

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "isPolar: " << isPolar << endl;
    cerr << "isRhi: " << isRhi << endl;
    cerr << "isSector: " << isSector << endl;
    cerr << "fixedAngle: " << fixedAngle << endl;
  }
  
  // compute output dir

  char dayStr[128];
  sprintf(dayStr, "%.4d%.2d%.2d", year, month, day);
  outputDir += PATH_DELIM;
  outputDir += dayStr;

  // ensure output dir exists
  
  if (ta_makedir_recurse(outputDir.c_str())) {
    cerr << "ERROR - MdvPull::_computeNcfOutputPath()" << endl;
    cerr << "  Cannot make output dir: " << outputDir;
  }

  // Create output filepath

  char outputPath[1024];
  
  string basename = string(_params.ncf_base_name);
  string v_yyyymmdd = validTime.getDateStrPlain();
  string v_hhmmss = validTime.getTimeStrPlain();
  char filename[1024];
  
  if(basename.empty()) {
    sprintf(filename, "%s%s_%s%s.nc",
            _params.ncf_file_prefix,
            v_yyyymmdd.c_str(), v_hhmmss.c_str(),
            _params.ncf_file_suffix);
  } else {
    sprintf(filename, "%s%s%s_%s%s.nc",
            _params.ncf_file_prefix, _params.ncf_base_name,  
            v_yyyymmdd.c_str(), v_hhmmss.c_str(),
            _params.ncf_file_suffix);
  }
  
  if (isForecast) { 
    string g_hhmmss = genTime.getTimeStrPlain();
    sprintf(outputPath, "%s/g_%s/%s",
            outputDir.c_str(), g_hhmmss.c_str(), filename);
  } else {
    sprintf(outputPath, "%s/%s", outputDir.c_str(), filename);
  }
  
  return outputPath;
  
}

//////////////////////////////////////

void MdvPull::_writeNcfLdataInfo(const DsMdvx &mdvx,
                                 const string &outputDir,
                                 const string &outputPath)

{
  
  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();

  // Write LdataInfo file

  DsLdataInfo ldata(outputDir, _params.debug);

  ldata.setWriter("Mdv2NetCDF");
  ldata.setDataFileExt("nc");
  ldata.setDataType("netCDF");

  string fileName;
  Path::stripDir(outputDir, outputPath, fileName);
  ldata.setRelDataPath(fileName);
  
  if (mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED ||
      mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
      mhdr.forecast_time > 0)
  {
    ldata.setIsFcast(true);
    int leadtime = mhdr.forecast_delta;
    ldata.setLeadTime(leadtime);
    ldata.write(mhdr.time_gen);
  }
  else
  {
    ldata.setIsFcast(false);
    ldata.write(mhdr.time_centroid);
  }
  
  if (_params.debug) {
    cerr << "Mdv2NetCDF::_writeLdataInfo(): Data written to "
         << outputPath << endl;
  }

}

