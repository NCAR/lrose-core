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
////////////////////////////////////////////////////////////////////////
// DsrGrabber.cc
//
// DsrGrabber object
// Some methods are in transform.cc and geom.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2007
//
///////////////////////////////////////////////////////////////////////
//
// DsrGrabber reads an input radar FMQ, and
// writes CP2Moments UDP data
//
///////////////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>
#include <toolsa/utim.h>
#include <toolsa/ucopyright.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/file_io.h>
#include <dsserver/DsLdataInfo.hh>
#include <radar/RadarComplex.hh>
#include <iomanip>
#include "DsrGrabber.hh"

using namespace std;

// Constructor

DsrGrabber::DsrGrabber(int argc, char **argv)

{

  isOK = true;
  _inputContents = 0;
  _dsInput = NULL;
  _inFile = NULL;
  _done = false;
  _diffIndexFirst = -1;
  _diffIndexSecond = -1;
  _nBeamsVol = 0;
  _latestTime = 0;
  _volStartTime = 0;
  _volEndTime = 0;
  _fileTime = 0;

  // set programe name
  
  _progName = "DsrGrabber";
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

  // initialize sun posn object
  
  _sunPosn.setLocation(_params.radar_lat, _params.radar_lon,
                       _params.radar_alt_km);

  // Instantiate and initialize the DsRadar queues
  // Block on SBAND, not on XBAND

  if (_params.input_mode == Params::FMQ_INPUT) {

    DsFmq::openPosition startPos = DsFmq::START;
    if (_params.seek_to_end_of_input) {
      startPos = DsFmq::END;
    }
    
    if (_inputQueue.init(_params.input_fmq_url, _progName.c_str(),
			 _params.debug,
			 DsFmq::BLOCKING_READ_ONLY, startPos)) {
      cerr << "ERROR - DsrGrabber::_run" << endl;
      cerr << "  Cannot init radar queue: " << _params.input_fmq_url << endl;
      isOK = false;
    }

  } else {

    _dsInput = new DsInputPath(_progName,
				 _params.debug >= Params::DEBUG_VERBOSE,
				 _args.inputFileList);
    
  }

  // field differencing

  if (_params.compute_differences) {

    if (_params.stats_fields_n < 2) {
      cerr << "ERROR - DsrGrabber" << endl;
      cerr << "  Param file: " << _paramsPath << endl;
      cerr << "  Cannot compute differences - less than 2 fields in output" << endl;
      cerr << "  Check 'stats_fields' and 'compute_differences' in params file" << endl;
      isOK = false;
    }

    for (int ii = 0; ii < _params.stats_fields_n; ii++) {
      if (strcmp(_params.first_field_for_diff, _params._stats_fields[ii]) == 0) {
	_diffIndexFirst = ii;
      }
      if (strcmp(_params.second_field_for_diff, _params._stats_fields[ii]) == 0) {
	_diffIndexSecond = ii;
      }
    }
    
    if (_diffIndexFirst < 0) {
      cerr << "ERROR - DsrGrabber" << endl;
      cerr << "  Param file: " << _paramsPath << endl;
      cerr << "  Cannot find first_field_for_diff: "
	   << _params.first_field_for_diff << endl;
      isOK = false;
    }
    
    if (_diffIndexSecond < 0) {
      cerr << "ERROR - DsrGrabber" << endl;
      cerr << "  Param file: " << _paramsPath << endl;
      cerr << "  Cannot find second_field_for_diff: "
	   << _params.second_field_for_diff << endl;
      isOK = false;
    }

  }
  
  // field names

  for (int ii = 0; ii < _params.stats_fields_n; ii++) {
    _fieldNames.push_back(_params._stats_fields[ii]);
  }
  if (_params.compute_differences) {
    _fieldNames.push_back("Diff");
  }

  // create grid vector

  if (_params.write_grid_files) {

    _gridMinEl = _params.grid_min_el;
    _gridMaxEl = _params.grid_max_el;
    _gridDeltaEl = _params.grid_delta_el;
    _gridNEl = (int) ((_gridMaxEl - _gridMinEl) / _gridDeltaEl + 0.5) + 1;
    
    _gridMinAz = _params.grid_min_az;
    _gridMaxAz = _params.grid_max_az;
    _gridDeltaAz = _params.grid_delta_az;
    _gridNAz = (int) ((_gridMaxAz - _gridMinAz) / _gridDeltaAz + 0.5) + 1;
    
    for (int iel = 0; iel < _gridNEl; iel++) {
      for (int iaz = 0; iaz < _gridNAz; iaz++) {
	double el = _gridMinEl + iel * _gridDeltaEl;
	double az = _gridMinAz + iaz * _gridDeltaAz;
	GridLoc *loc = new GridLoc(_params, el, az);
	_gridLocs.push_back(loc);
      }
    }

  } else {

    _gridNEl = 0;
    _gridNAz = 0;

  }

  // create minima vector, with enough space for one diff field
  
  for (int ii = 0; ii < _params.stats_fields_n + 1; ii++) {
    _minima.push_back(-9999.0);
  }
  _initMinima();

  // create sun props vector
  
  for (int ii = 0; ii < _params.stats_fields_n; ii++) {
    sun_props_t props;
    memset(&props, 0, sizeof(props));
    _sunProps.push_back(props);
  }

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

}

/////////////////////////////////////////////////////////
// destructor

DsrGrabber::~DsrGrabber()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  for (int ii = 0; ii < (int) _gridLocs.size(); ii++) {
    delete _gridLocs[ii];
  }

  if (_dsInput) {
    delete _dsInput;
  }

  _closeInputFile();

}

//////////////////////////////////////////////////
// Run

int DsrGrabber::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");
  
  while (!_done) {
    _run();
    if (!_done && _params.debug) {
      cerr << "DsrGrabber::Run:" << endl;
      cerr << "  Trying to contact input server at url: "
	   << _params.input_fmq_url << endl;
    }
    umsleep(1000);
  }

  return 0;

}

//////////////////////////////////////////////////
// _run

int DsrGrabber::_run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // read beams from the queue
  
  while (!_done) {

    // read input beam
    
    if (_readBeam() == 0) {
      
      // process this beam
      
      _processBeam();
	
    } //  if (_readBeam()
    
  } // while (!_done)
  
  return 0;

}

////////////////////////////////////////////////////////////////
// read a beam
// returns 0 on success, -1 on failure

int DsrGrabber::_readBeam()

{

  PMU_auto_register("Reading beam");

  if (_params.input_mode == Params::FMQ_INPUT) {
    return _readBeamFromFmq();
  } else {
    return _readBeamFromFile();
  }

}

////////////////////////////////////////////////////////////////
// read a beam from FMQ
// returns 0 on success, -1 on failure

int DsrGrabber::_readBeamFromFmq()

{
  
  PMU_auto_register("Reading fmq");
  
  while (true) {
    
    if (_inputQueue.getDsMsg(_inputMsg, &_inputContents)) {
      return -1;
    }
    
    // end of vol?
    
    if (_endOfVol()) {
      _handleEndOfVol();
    }
    
    if ((_inputContents & DsRadarMsg::RADAR_BEAM) && _inputMsg.allParamsSet()) {

      if (_loadLatestBeamFromFmq() == 0) {
	return 0;
      }
      
    }

  } // while
    
  return -1;

}

////////////////////////////////////////////////////////////////
// read a beam from a file
// returns 0 on success, -1 on failure

int DsrGrabber::_readBeamFromFile()

{

  PMU_auto_register("Reading file");

  if (_inFile == NULL || feof(_inFile)) {
    if (_openNextFile()) {
      _done = true;
      return -1;
    }
  }

  while (true) {

    // read a line from the file

    char line[BUFSIZ];
    if (fgets(line, BUFSIZ, _inFile) == NULL) {
      _handleEndOfVol();
      return -1;
    }
 
    // scan the line, load up latest beam

    int year, month, day, hour, min, sec;
    double julDay;
    int volNum, tiltNum;
    double el, az;

    if (sscanf(line, "%d %d %d %d %d %d %lg %d %d %lg %lg",
	       &year, &month, &day, &hour, &min, &sec,
	       &julDay, &volNum, &tiltNum, &el, &az) != 11) {
      continue;
    }
    
    DateTime btime(year, month, day, hour, min, sec);
    
    _latestBeam.time = btime.utime();
    _latestBeam.volNum = volNum;
    _latestBeam.tiltNum = tiltNum;
    _latestBeam.el = el;
    _latestBeam.az = az;

    // tokenize the line

    vector<string> toks;
    TaStr::tokenize(line, " ", toks);

    // fields

    _latestBeam.fields.clear();
    for (int ii = 11; ii < (int) toks.size(); ii++) {
      double val;
      if (sscanf(toks[ii].c_str(), "%lg", &val) == 1) {
	_latestBeam.fields.push_back(val);
      }
    }

    // success

    return 0;

  } // while
    
  return -1;

}

////////////////////////////
// open next available file
//
// Returns 0 on success, -1 on failure

int DsrGrabber::_openNextFile()

{

  PMU_auto_register("Opening next file");

  if (_inFile) {
    _closeInputFile();
  }
  
  _inputPath = _dsInput->next();
  if (_inputPath == NULL) {
    // no more files
    return -1;
  }

  if (_params.debug) {
    cerr << "Opening file: " << _inputPath << endl;
  }

  // open file
  
  if ((_inFile = fopen(_inputPath, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - DsrGrabber::_openNextFile" << endl;
    cerr << "  Cannot open file: " << _inputPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  return 0;

}

////////////////////////////
// close input file

void DsrGrabber::_closeInputFile()

{
  if (_inFile) {
    fclose(_inFile);
    _inFile = NULL;
  }
}

////////////////////////////////////////////////////////////////
// process S-band beam

int DsrGrabber::_processBeam()

{
  
  int iret = 0;

  _latestTime = (time_t) _latestBeam.time;
  if (_volStartTime == 0) {
    _volStartTime = _latestTime;
  }

  // compute minima

  _computeMinima(_latestBeam.fields);

  // format output string
  
  string line = _formatOutputLine(_latestBeam);
  
  if (_params.monitor_input) {
    cerr << "Beam: " << line << endl;
  }

  // save output string
  
  _beamStrings.push_back(line);
  
  // create beam, add to grid

  _addBeamToGrid(_latestBeam);

  // keep track of number of beams in vol
  
  _nBeamsVol++;

  return iret;
  
}

////////////////////////////////////////////////////////////////
// load latest beam from the FMQ

int DsrGrabber::_loadLatestBeamFromFmq()

{
  
  const DsRadarParams &radarParams = _inputMsg.getRadarParams();
  const vector<DsFieldParams*> &fieldParamsArray = _inputMsg.getFieldParams();
  const DsRadarBeam &radarBeam = _inputMsg.getRadarBeam();

  _latestBeam.time = radarBeam.getDoubleTime();
  _latestBeam.volNum = radarBeam.volumeNum;
  _latestBeam.tiltNum = radarBeam.tiltNum;

  // loop through requested output fields, computing means
  
  _latestBeam.fields.clear();
  for (int ii = 0; ii < _params.stats_fields_n; ii++) {

    // compute mean
    
    double mean = -9999;
    string dsrname = _params._stats_fields[ii];
    for (int jj = 0; jj < (int) fieldParamsArray.size(); jj++) {
      const DsFieldParams &fieldParams = *(fieldParamsArray[jj]);
      if (dsrname == fieldParams.name) {
	mean = _computeFieldMean(dsrname, jj, radarParams, fieldParams, radarBeam);
	break;
      }
    } // jj
    
    _latestBeam.fields.push_back(mean);
    
  } // ii

  // compute diff field if requested
  
  if (_params.compute_differences &&
      _diffIndexFirst >= 0 && _diffIndexFirst < (int) fieldParamsArray.size() &&
      _diffIndexSecond >= 0 && _diffIndexSecond < (int) fieldParamsArray.size()) {
    double diff = _latestBeam.fields[_diffIndexFirst] - _latestBeam.fields[_diffIndexSecond];
    _latestBeam.fields.push_back(diff);
  }

  // compute sun relative angles if requested

  double el = radarBeam.elevation;
  double az = radarBeam.azimuth;
  double offsetEl = el;
  double offsetAz = az;
  
  if (_params.compute_sun_relative_angles) {
    double sunEl, sunAz;
    _sunPosn.computePosnNova(radarBeam.dataTime, sunEl, sunAz);
    offsetEl = RadarComplex::diffDeg(el, sunEl);
    offsetAz = RadarComplex::diffDeg(az, sunAz) * cos(el * DEG_TO_RAD);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Sun el, az: " << sunEl << ", " << sunAz << endl;
    }
  }
  
  _latestBeam.el = offsetEl;
  _latestBeam.az = offsetAz;

  if (_params.compute_sun_relative_angles && _params.only_save_when_close_to_sun) {
    if (fabs(offsetEl) > _params.max_sun_el_error ||
	fabs(offsetAz) > _params.max_sun_az_error) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	_latestBeam.print(cerr);
	cerr << "--->> Too far from sun, ignoring" << endl;
      }
      return -1;
    }
  }
  
  return 0;
  
}

////////////////////////////////////////////////////////////////
// handle end of volume

void DsrGrabber::_handleEndOfVol()

{

  _volEndTime = _latestTime;
  if (_params.timestamp == Params::TIMESTAMP_VOL_START) {
    _fileTime = _volStartTime;
  } else if (_params.timestamp == Params::TIMESTAMP_VOL_MID) {
    double diff = (double) _volEndTime - (double) _volStartTime;
    _fileTime = _volStartTime + (int) (diff / 2.0 + 0.5);
  } else {
    _fileTime = _volEndTime;
  }

  if (_nBeamsVol >= _params.min_beams_per_vol) {
    
    // write beam file
    
    if (_params.write_beam_files) {
      _writeBeamsFile();
    }
    
    // interpolate onto grid
    
    _interpolateGrid();
    
    // compute sun props
    
    if (_params.compute_sun_properties) {
      for (int ii = 0; ii < _params.stats_fields_n; ii++) {
	_computeSunProps(ii);
      }
    }
    
    // write grid files
    
    if (_params.write_grid_files) {
      _writeGridFiles();
    }

    // write sun props file
    
    if (_params.write_sunprops_files) {
      _writeSunpropsFile();
    }

  } else {

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Too few beams in volume, ignoring: " << _nBeamsVol << endl;
    }

  } // if (_nBeamsVol >= _params.min_beams_per_vol)
    
  // re-initialize
  
  _beamStrings.clear();
  _clearGridBeams();
  _initMinima();
  _nBeamsVol = 0;
  _volStartTime = _latestTime;
  
}
      
////////////////////////////////////////////////////////////////
// compute mean for this field, within the specified gate range

double DsrGrabber::_computeFieldMean(const string &fieldName,
				     int dsrFieldNum,
				     const DsRadarParams &radarParams,
				     const DsFieldParams &fieldParams,
				     const DsRadarBeam &radarBeam)
  
{

  // compute mean in gate range

  double count = 0.0;
  double sum = 0.0;
  int startGate = _params.start_gate;
  int endGate = startGate + _params.n_gates - 1;
  
  if(fieldParams.byteWidth == 1) {
    
    ui08 *dd = (ui08 *) radarBeam.getData()
      + startGate * radarParams.numFields + dsrFieldNum;
    ui08 missing = (ui08) fieldParams.missingDataValue;
    double scale = fieldParams.scale;
    double bias = fieldParams.bias;
    for (int ii = startGate; ii <= endGate; ii++, dd += radarParams.numFields) {
      if (*dd != missing) {
	double val = *dd * scale + bias;
	sum += val;
	count++;
      }
    }

  } else if (fieldParams.byteWidth == 2) {
    
    ui16 *dd = (ui16 *) radarBeam.getData()
      + startGate * radarParams.numFields + dsrFieldNum;
    ui16 missing = (ui16) fieldParams.missingDataValue;
    double scale = fieldParams.scale;
    double bias = fieldParams.bias;
    for (int ii = startGate; ii <= endGate; ii++, dd += radarParams.numFields) {
      if (*dd != missing) {
	double val = *dd * scale + bias;
	sum += val;
	count++;
      }
    }

  } else if (fieldParams.byteWidth == 4) {

    fl32 *dd = (fl32 *) radarBeam.getData()
      + startGate * radarParams.numFields + dsrFieldNum;
    fl32 missing = (fl32) fieldParams.missingDataValue;
    for (int ii = startGate; ii <= endGate; ii++, dd += radarParams.numFields) {
      if (*dd != missing) {
	double val = *dd;
	sum += val;
	count++;
      }
    }

  } else {

    cerr << "ERROR - invalid byte width: " << fieldParams.byteWidth << endl;
    return -9999;
    
  }
  
  double mean = -9999;
  if (count > 0) {
    mean = sum / count;
  }

  return mean;

}

////////////////////////////////////////////////////////////////
// check for end of volume

bool DsrGrabber::_endOfVol()

{

  // If we have an end of vol, close the output file
  
  if (_inputContents & DsRadarMsg::RADAR_FLAGS) {
    const DsRadarFlags& flags = _inputMsg.getRadarFlags();
    if (flags.endOfVolume) {
      return true;
    }
  }

  if (_nBeamsVol >= _params.max_beams_per_vol) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "End of vol triggered by number of beams: " << _nBeamsVol << endl;
    }
    return true;
  }

  return false;
  
}

////////////////////////////////////////////////////////////////
// format output line

string DsrGrabber::_formatOutputLine(const Beam &beam)
  

{
  
  char txt[1024];
  string str;
  
  DateTime btime((time_t) _latestBeam.time);
  double jday = (double) btime.utime() / 86400.0;
  sprintf(txt,
	  "%.4d %.2d %.2d %.2d %.2d %.2d "
	  "%11.4f %6d %4d %10.3f %10.3f ",
	  btime.getYear(),
	  btime.getMonth(),
	  btime.getDay(),
	  btime.getHour(),
	  btime.getMin(),
	  btime.getSec(),
	  jday,
	  _latestBeam.volNum,
	  _latestBeam.tiltNum,
	  _latestBeam.el,
	  _latestBeam.az);
  
  str += txt;
  for (int ii = 0; ii < (int) _latestBeam.fields.size(); ii++) {
    sprintf(txt, "%10.3f ", _latestBeam.fields[ii]);
    str += txt;
    if (ii != (int) _latestBeam.fields.size() - 1) {
      str += " ";
    }
  } // ii

  return str;
  
}
      
////////////////////////////////////////////////////////////////
// get azimuth, elevation, time

double DsrGrabber::_getAz(const DsRadarMsg &radarMsg)
  
{
  return radarMsg.getRadarBeam().azimuth;
}

double DsrGrabber::_getEl(const DsRadarMsg &radarMsg)
  
{
  return radarMsg.getRadarBeam().elevation;
}

time_t DsrGrabber::_getTime(const DsRadarMsg &radarMsg)
  
{
  return radarMsg.getRadarBeam().dataTime;
}

////////////////////////////////////////////////////////////////
// add a beam to the grid

void DsrGrabber::_addBeamToGrid(const Beam &beam)
  
{

  for (int ii = 0; ii < (int) _gridLocs.size(); ii++) {
    _gridLocs[ii]->addBeam(beam);
  }

}

////////////////////////////////////////////////////////////////
// interpolate grid

void DsrGrabber::_interpolateGrid()
  
{

  for (int ii = 0; ii < (int) _gridLocs.size(); ii++) {
    _gridLocs[ii]->interpolate(_minima);
  }

}

////////////////////////////////////////////////////////////////
// clear grid beams

void DsrGrabber::_clearGridBeams()
  
{

  for (int ii = 0; ii < (int) _gridLocs.size(); ii++) {
    _gridLocs[ii]->clearBeams();
  }

}

////////////////////////////////////////////////////////////////
// init minima

void DsrGrabber::_initMinima()

{

  for (int ii = 0; ii < (int) _minima.size(); ii++) {
    _minima[ii] = 1.0e99;
  }

}

////////////////////////////////////////////////////////////////
// compute minima

void DsrGrabber::_computeMinima(const vector<double> &fields)

{

  for (int ii = 0; ii < (int) _minima.size(); ii++) {
    if (_minima[ii] > fields[ii]) {
      _minima[ii] = fields[ii];
    }
  }

}

/////////////////////////////////////////////////
// compute the maximum sun power
    
double DsrGrabber::_computeMaxSunPower(int fieldNum)
  
{

  double maxSunPower = -200;
  
  int ii = 0;
  for (int iel = 0; iel < _gridNEl; iel++) {
    for (int iaz = 0; iaz < _gridNAz; iaz++, ii++) {
      const GridLoc *loc = _gridLocs[ii];
      double val = loc->getInterp()[fieldNum];
      if (val > maxSunPower) {
        maxSunPower = val;
      }
    } // iaz
  } // iel

  return maxSunPower;

}

/////////////////////////////////////////////////
// compute the sun properties
    
void DsrGrabber::_computeSunProps(int fieldNum)
  
{
  
  double maxSunPower = _computeMaxSunPower(fieldNum);

  if (_params.debug) {
    cerr << "--->> Computing sun centroid" << endl;
    cerr << "      Field: " << _params._stats_fields[fieldNum] << endl;
    cerr << "      maxSunPower: " << maxSunPower << endl;
  }

  
  double sumWtAz = 0.0;
  double sumWtEl = 0.0;
  double sumPower = 0.0;
  double edgePowerDbm = maxSunPower - _params.sun_edge_below_peak_db;
  double centroidAzOffset = 0;
  double centroidElOffset = 0;

  int ii = 0;
  for (int iel = 0; iel < _gridNEl; iel++) {
    for (int iaz = 0; iaz < _gridNAz; iaz++, ii++) {
      const GridLoc *loc = _gridLocs[ii];
      double dbm = loc->getInterp()[fieldNum];
      if (dbm >= edgePowerDbm) {
	double el = _gridMinEl + iel * _gridDeltaEl;
	double az = _gridMinAz + iaz * _gridDeltaAz;
        sumPower += dbm;
        sumWtAz += az * dbm;
        sumWtEl += el * dbm;
      }
    }
  }

  if (sumPower > 0) {
    centroidAzOffset = sumWtAz / sumPower;
    centroidElOffset = sumWtEl / sumPower;
  }
  
  if (centroidAzOffset < _gridMinAz ||
      centroidAzOffset > _gridMaxAz ||
      centroidElOffset < _gridMinEl ||
      centroidElOffset > _gridMaxEl) {
    if (_params.debug) {
      cerr << "Cannot estimate solar centroid:" << endl;
      cerr << "  Setting offsets to 0" << endl;
    }
    _sunProps[fieldNum].maxSunPower = maxSunPower;
    _sunProps[fieldNum].centroidAzOffset = 0;
    _sunProps[fieldNum].centroidElOffset = 0;
    _sunProps[fieldNum].quadSunPower = maxSunPower;
    return;
  }
  
  if (_params.debug) {
    cerr << "Initial estimates for solar centroid:" << endl;
    cerr << "  centroidAzOffset: " << centroidAzOffset << endl;
    cerr << "  centroidElOffset: " << centroidElOffset << endl;
  }

  // if possible, fit parabola to azimuth at solar peak
  // to refine the azimuth centroid

  vector<double> azArray;
  vector<double> azDbm;

  int elCentroidIndex =
    (int) ((centroidElOffset - _gridMinEl) /  _gridDeltaEl);
  
  for (int iaz = 0; iaz < _gridNAz; iaz++) {
    int ii = elCentroidIndex * _gridNAz + iaz;
    const GridLoc *loc = _gridLocs[ii];
    double dbm = loc->getInterp()[fieldNum];
    if (dbm >= edgePowerDbm) {
      double az = _gridMinAz + iaz * _gridDeltaAz;
      azArray.push_back(az);
      // add 200 to dbm to ensure real roots
      azDbm.push_back(dbm + 200);
    }
  }
  
  double ccAz, bbAz, aaAz, errEstAz, rSqAz;
  if (_quadFit((int) azArray.size(),
               azArray, azDbm,
               ccAz, bbAz, aaAz,
               errEstAz, rSqAz) == 0) {
    double rootTerm = bbAz * bbAz - 4.0 * aaAz * ccAz;
    if (rSqAz > 0.9 && rootTerm >= 0) {
      // good fit, real roots, so override centroid
      double root1 = (-bbAz - sqrt(rootTerm)) / (2.0 * aaAz); 
      double root2 = (-bbAz + sqrt(rootTerm)) / (2.0 * aaAz);
      centroidAzOffset = (root1 + root2) / 2.0;
    }
  }
  
  // if possible, fit parabola to azimuth at solar peak
  // to refine the azimuth centroid

  vector<double> elArray;
  vector<double> elDbm;
  
  int azCentroidIndex =
    (int) ((centroidAzOffset - _gridMinAz) /  _gridDeltaAz);
  for (int iel = 0; iel < _gridNEl; iel++) {
    int ii = iel * _gridNAz + azCentroidIndex;
    const GridLoc *loc = _gridLocs[ii];
    double dbm = loc->getInterp()[fieldNum];
    if (dbm >= edgePowerDbm) {
      double el = _gridMinEl + iel * _gridDeltaEl;
      elArray.push_back(el);
      // add 200 to dbm to ensure real roots
      elDbm.push_back(dbm + 200);
    }
  }
  
  double ccEl, bbEl, aaEl, errEstEl, rSqEl;
  if (_quadFit((int) elArray.size(),
               elArray, elDbm,
               ccEl, bbEl, aaEl,
               errEstEl, rSqEl) == 0) {
    double rootTerm = bbEl * bbEl - 4.0 * aaEl * ccEl;
    if (rSqEl > 0.9 && rootTerm >= 0) {
      // good fit, real roots, so override centroid
      double root1 = (-bbEl - sqrt(rootTerm)) / (2.0 * aaEl); 
      double root2 = (-bbEl + sqrt(rootTerm)) / (2.0 * aaEl);
      centroidElOffset = (root1 + root2) / 2.0;
    }
  }
  
  double quadSunPower = (ccAz + ccEl) / 2.0 - 200.0;
  
  if (_params.debug) {
    cerr << "Final estimates for solar centroid:" << fieldNum << endl;
    cerr << "  field: " << _params._stats_fields[fieldNum] << endl;
    cerr << "  centroidAzOffset: " << centroidAzOffset << endl;
    cerr << "  centroidElOffset: " << centroidElOffset << endl;
    cerr << "  maxSunPower: " << maxSunPower << endl;
    cerr << "  quadSunPower: " << quadSunPower << endl;
  }

  _sunProps[fieldNum].maxSunPower = maxSunPower;
  _sunProps[fieldNum].centroidAzOffset = centroidAzOffset;
  _sunProps[fieldNum].centroidElOffset = centroidElOffset;
  _sunProps[fieldNum].quadSunPower = quadSunPower;

}

/////////////////////////////////////////////////////////////////
// quadFit : fit a quadratic to a data series
//
// Mike Dixon  RAP, NCAR, Boulder, Colorado
//
// October 1990
//
//  n: number of points in (x, y) data set
//  x: array of x data
//  y: array of y data
//  a? - quadratic coefficients (cc - bias, bb - linear, aa - squared)
//  std_error - standard error of estimate
//  r_squared - correlation coefficient squared
//
// Returns 0 on success, -1 on error.
//
/////////////////////////////////////////////////////////////////

int DsrGrabber::_quadFit(int n,
			 const vector<double> &x,
			 const vector<double> &y,
			 double &cc,
			 double &bb,
			 double &aa,
			 double &std_error_est,
			 double &r_squared)
  
{
  
  long i;

  double sumx = 0.0, sumx2 = 0.0, sumx3 = 0.0, sumx4 = 0.0;
  double sumy = 0.0, sumxy = 0.0, sumx2y = 0.0;
  double dn;
  double term1, term2, term3, term4, term5;
  double diff;
  double ymean, sum_dy_squared = 0.0;
  double sum_of_residuals = 0.0;
  double xval, yval;

  if (n < 4)
    return (-1);

  dn = (double) n;
  
  // sum the various terms

  for (i = 0; i < n; i++) {

    xval = x[i];
    yval = y[i];

    sumx = sumx + xval;
    sumx2 += xval * xval;
    sumx3 += xval * xval * xval;
    sumx4 += xval * xval * xval * xval;
    sumy += yval;
    sumxy += xval  * yval;
    sumx2y += xval * xval * yval;

  }

  ymean = sumy / dn;

  // compute the coefficients

  term1 = sumx2 * sumy / dn - sumx2y;
  term2 = sumx * sumx / dn - sumx2;
  term3 = sumx2 * sumx / dn - sumx3;
  term4 = sumx * sumy / dn - sumxy;
  term5 = sumx2 * sumx2 / dn - sumx4;

  aa = (term1 * term2 / term3 - term4) / (term5 * term2 / term3 - term3);
  bb = (term4 - term3 * aa) / term2;
  cc = (sumy - sumx * bb  - sumx2 * aa) / dn;

  // compute the sum of the residuals

  for (i = 0; i < n; i++) {
    xval = x[i];
    yval = y[i];
    diff = (yval - cc - bb * xval - aa * xval * xval);
    sum_of_residuals += diff * diff;
    sum_dy_squared += (yval - ymean) * (yval - ymean);
  }

  // compute standard error of estimate and r-squared
  
  std_error_est = sqrt(sum_of_residuals / (dn - 3.0));
  r_squared = ((sum_dy_squared - sum_of_residuals) /
	       sum_dy_squared);
  
  return 0;

}

////////////////////////////////////////////////////////////////
// write beams file
//
// returns 0 on success, -1 on failure

int DsrGrabber::_writeBeamsFile()

{

  // compute paths
  
  DateTime btime(_fileTime);

  char subdirName[1024];
  sprintf(subdirName, "%.4d%.2d%.2d",
	  btime.getYear(),
	  btime.getMonth(),
	  btime.getDay());
  
  char subdirPath[1024];
  sprintf(subdirPath, "%s%s%s",
	  _params.beams_output_dir, PATH_DELIM, subdirName);

  char relFilePath[1024];
  sprintf(relFilePath, "%s%s%.4d%.2d%.2d_%.2d%.2d%.2d_%s.%s",
	  subdirName, PATH_DELIM,
	  btime.getYear(),
	  btime.getMonth(),
	  btime.getDay(),
	  btime.getHour(),
	  btime.getMin(),
	  btime.getSec(),
	  _params.radar_name,
	  _params.beam_file_ext);

  char filePath[1024];
  sprintf(filePath, "%s%s%s",
	  _params.beams_output_dir, PATH_DELIM, relFilePath);
  
  // create the directory for the output file
  
  if (ta_makedir_recurse(subdirPath)) {
    int errNum = errno;
    cerr << "ERROR - DsrGrabber::_writeBeamsFile";
    cerr << "  Cannot create output dir: " << subdirPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // open file
  
  FILE *out;
  if ((out = fopen(filePath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - DsrGrabber::_writeBeamsFile";
    cerr << "  Cannot create file: " << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "---> writing beams file: " << filePath << endl;
  }

  if (_params.write_nlines_at_start_of_beam_files) {
    fprintf(out, "%s %d\n",
	    _params.beams_nlines_label,
	    (int) _beamStrings.size());
  }
  
  for (int ii = 0; ii < (int) _beamStrings.size(); ii++) {
    fprintf(out, "%s\n", _beamStrings[ii].c_str());
  }

  fclose(out);
  
  // write latest data info file
  
  if (_params.write_ldata_info_for_beam_files) {
    DsLdataInfo ldata(_params.beams_output_dir,
		      _params.debug >= Params::DEBUG_VERBOSE);
    ldata.setLatestTime(btime.utime());
    ldata.setRelDataPath(relFilePath);
    ldata.setWriter("DsrGrabber");
    ldata.setDataType("raw");
    ldata.setDataFileExt(_params.beam_file_ext);
    if (ldata.write(btime.utime())) {
      cerr << "ERROR - cannot write LdataInfo" << endl;
      cerr << " outputDir: " << _params.beams_output_dir << endl;
    }
  }
   
  return 0;

}

///////////////////////////////
// write grid files

int DsrGrabber::_writeGridFiles()

{
  
  // compute paths
  
  DateTime btime(_fileTime);
  
  char subdirName[1024];
  sprintf(subdirName, "%.4d%.2d%.2d",
	  btime.getYear(),
	  btime.getMonth(),
	  btime.getDay());
  
  char subdirPath[1024];
  sprintf(subdirPath, "%s%s%s",
	  _params.grid_output_dir, PATH_DELIM, subdirName);
  
  // create the sub-directory
  
  if (ta_makedir_recurse(subdirPath)) {
    int errNum = errno;
    cerr << "ERROR - DsrGrabber::_writeGridFiles";
    cerr << "  Cannot create output dir: " << subdirPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // write out fields
  
  for (int ii = 0; ii < (int) _fieldNames.size(); ii++) {
    _writeGridField(subdirName, subdirPath, btime, ii, _fieldNames[ii]);
  }
  return 0;

}

///////////////////////////////////////////////////////////
// write out field data files

int DsrGrabber::_writeGridField(const char *subdirName,
				const char *subdirPath,
				const DateTime &btime,
				int fieldNum,
				const string &fieldName)
  
{
  
  // compute file path
  
  char relFilePath[1024];
  sprintf(relFilePath, "%s%s%.4d%.2d%.2d_%.2d%.2d%.2d_%s_%s.%s",
	  subdirName, PATH_DELIM,
	  btime.getYear(),
	  btime.getMonth(),
	  btime.getDay(),
	  btime.getHour(),
	  btime.getMin(),
	  btime.getSec(),
	  _params.radar_name,
	  fieldName.c_str(),
	  _params.grid_file_ext);
  
  char filePath[1024];
  sprintf(filePath, "%s%s%s",
	  _params.beams_output_dir, PATH_DELIM, relFilePath);
  
  if (_params.debug) {
    cerr << "-->> writing grid file: " << filePath << endl;
  }
  
  // open file
  
  FILE *out;
  if ((out = fopen(filePath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SunCal::_writeGriddedField";
    cerr << "  Cannot create file: " << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // write out grid details
  
  if (_params.write_dimensions_at_start_of_grid_files) {
    fprintf(out, "%s%g %g %d\n", _params.grid_azimuths_label,
	    _gridMinAz, _gridDeltaAz, _gridNAz);
    fprintf(out, "%s%g %g %d\n", _params.grid_elevations_label,
	    _gridMinEl, _gridDeltaEl, _gridNEl);
  }

  // write out sun centroid

  if (fieldNum >= (int) _sunProps.size()) {
    fprintf(out, "%s-9999 -9999 -9999 -9999\n",
	    _params.sunprops_label);
  } else {
    fprintf(out, "%s%g %g %g %g\n",
	    _params.sunprops_label,
	    _sunProps[fieldNum].centroidAzOffset,
	    _sunProps[fieldNum].centroidElOffset,
	    _sunProps[fieldNum].maxSunPower,
	    _sunProps[fieldNum].quadSunPower);
  }

  // write out grid data

  int ii = 0;
  for (int iel = 0; iel < _gridNEl; iel++) {
    for (int iaz = 0; iaz < _gridNAz; iaz++, ii++) {
      const GridLoc *loc = _gridLocs[ii];
      double val = loc->getInterp()[fieldNum];
      fprintf(out, " %10.3f", val);
    } // iaz
    fprintf(out, "\n");
  } // iel

  fclose(out);

  // write latest data info file
  
  if (_params.write_ldata_info_for_grid_files) {
    DsLdataInfo ldata(_params.grid_output_dir,
		      _params.debug >= Params::DEBUG_VERBOSE);
    ldata.setLatestTime(btime.utime());
    ldata.setRelDataPath(relFilePath);
    ldata.setWriter("DsrGrabber");
    ldata.setDataType("raw");
    ldata.setDataFileExt(_params.grid_file_ext);
    if (ldata.write(btime.utime())) {
      cerr << "ERROR - cannot write LdataInfo" << endl;
      cerr << " outputDir: " << _params.grid_output_dir << endl;
    }
  }
   
  return 0;

}

////////////////////////////////////////////////////////////////
// write sunprops file
//
// returns 0 on success, -1 on failure

int DsrGrabber::_writeSunpropsFile()

{

  // compute paths
  
  DateTime btime(_fileTime);

  char subdirName[1024];
  sprintf(subdirName, "%.4d%.2d%.2d",
	  btime.getYear(),
	  btime.getMonth(),
	  btime.getDay());
  
  char subdirPath[1024];
  sprintf(subdirPath, "%s%s%s",
	  _params.sunprops_output_dir, PATH_DELIM, subdirName);
  
  char relFilePath[1024];
  sprintf(relFilePath, "%s%s%.4d%.2d%.2d_%.2d%.2d%.2d_%s.%s",
	  subdirName, PATH_DELIM,
	  btime.getYear(),
	  btime.getMonth(),
	  btime.getDay(),
	  btime.getHour(),
	  btime.getMin(),
	  btime.getSec(),
	  _params.radar_name,
	  _params.sunprops_file_ext);

  char filePath[1024];
  sprintf(filePath, "%s%s%s",
	  _params.sunprops_output_dir, PATH_DELIM, relFilePath);
  
  // create the directory for the file
  
  if (ta_makedir_recurse(subdirPath)) {
    int errNum = errno;
    cerr << "ERROR - DsrGrabber::_writeSunpropsFile";
    cerr << "  Cannot create output dir: " << subdirPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // open file
  
  FILE *out;
  if ((out = fopen(filePath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - DsrGrabber::_writeSunpropsFile";
    cerr << "  Cannot create file: " << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "---> writing sunprops file: " << filePath << endl;
  }
  
  fprintf(out, "Sun scan properties\n");
  fprintf(out, "  Time: %s\n", DateTime::strm(_fileTime).c_str());
  fprintf(out, "  Radar name: %s\n", _params.radar_name);

  for (int ii = 0; ii < (int) _sunProps.size(); ii++) {
    fprintf(out, "  Field: %s\n", _params._stats_fields[ii]);
    fprintf(out, "    centroid az offset (deg): %g\n",
	    _sunProps[ii].centroidAzOffset);
    fprintf(out, "    centroid el offset (deg): %g\n",
	    _sunProps[ii].centroidElOffset);
    fprintf(out, "    max sun power      (dBm): %g\n",
	    _sunProps[ii].maxSunPower);
    fprintf(out, "    quad fit sun power (dBm): %g\n",
	    _sunProps[ii].quadSunPower);
  }

  fclose(out);
  
  // write latest data info file
  
  if (_params.write_ldata_info_for_sunprops_files) {
    DsLdataInfo ldata(_params.sunprops_output_dir,
		      _params.debug >= Params::DEBUG_VERBOSE);
    ldata.setLatestTime(btime.utime());
    ldata.setRelDataPath(relFilePath);
    ldata.setWriter("DsrGrabber");
    ldata.setDataType("raw");
    ldata.setDataFileExt(_params.sunprops_file_ext);
    if (ldata.write(btime.utime())) {
      cerr << "ERROR - cannot write LdataInfo" << endl;
      cerr << " outputDir: " << _params.sunprops_output_dir << endl;
    }
  }
   
  return 0;

}

