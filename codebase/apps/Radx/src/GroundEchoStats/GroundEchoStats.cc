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
//////////////////////////////////////////////////////////////////////
// GroundEchoStats.cc
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2013
//
//////////////////////////////////////////////////////////////////////
//
// GroundEchoStats compute stats from ground echo in 
// nadir-pointing airborne radar
//
//////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/TaXml.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/sincos.h>
#include <Fmq/DsRadarQueue.hh>
#include <rapmath/RapComplex.hh>
#include <radar/IwrfMomReader.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxGeoref.hh>
#include "GroundEchoStats.hh"
using namespace std;

// Constructor

GroundEchoStats::GroundEchoStats(int argc, char **argv)

{

  isOK = true;
  _reader = NULL;
  _sock = NULL;

  // set programe name

  _progName = "GroundEchoStats";
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

  // create reader

  if (_params.input_mode == Params::FMQ_INPUT) {
    
    _reader = new IwrfMomReaderFmq(_params.fmq_url);
    if (_params.seek_to_start_of_fmq) {
      _reader->seekToStart();
    }

  } else if (_params.input_mode == Params::TCP_INPUT) {

    _reader = new IwrfMomReaderTcp(_params.input_tcp_host,
                                   _params.input_tcp_port);
    
  } else if (_params.input_mode == Params::FILE_LIST) {

    _reader = new IwrfMomReaderFile(_args.inputFileList);
    
  } else if (_params.input_mode == Params::FILE_REALTIME) {

    _reader = new IwrfMomReaderFile(_params.files_input_dir);
    
  } else {

    cerr << "ERROR: " << _progName << endl;
    cerr << "  Unknown input mode: " << _params.input_mode << endl;
    isOK = FALSE;
    return;

  }

  if (_params.debug) {
    _reader->setDebug(IWRF_DEBUG_NORM);
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _reader->setDebug(IWRF_DEBUG_VERBOSE);
  }

  // initialize stats

  _initStats();
  _globalStartTime = 0.0;
  _globalSumVrCorrected = 0.0;
  _globalSumSqVrCorrected = 0.0;
  _globalSumTiltError = 0.0;
  _globalSumSqTiltError = 0.0;
  _globalCount = 0;

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

GroundEchoStats::~GroundEchoStats()

{

  if (_reader) {
    delete _reader;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int GroundEchoStats::Run()
{

  // register with procmap
  
  PMU_auto_register("Run");

  int iret = 0;
  while (true) {
    iret = _run();
    if (_params.input_mode == Params::FILE_LIST) {
      return iret;
    }
    sleep(1);
  }

  return iret;

}

//////////////////////////////////////////////////
// _run computing stats

int GroundEchoStats::_run()
{

  // register with procmap
  
  PMU_auto_register("_runPrint");

  while (true) { 
    
    PMU_auto_register("Reading moments data");
    
    // get the next ray
    
    RadxRay *ray = _reader->readNextRay();
    if (ray == NULL) {
      return -1;
    }
    ray->convertToFl32();

    // compute stats

    _computeStats(ray);

    // clean up

    delete ray;
    
  } // while

  return 0;

}

////////////////////////////////////////////
// print ray information

void GroundEchoStats::_debugPrint(const RadxRay *ray)
  
{

  RadxTime rtime(ray->getTimeSecs());
  
  ray->print(cout);
  
  if (_reader->getPlatformUpdated()) {
    _reader->getPlatform().print(cerr);
  }
  if (_reader->getStatusXmlUpdated()) {
    cout << "STATUS XML" << endl;
    cout << "----------" << endl;
    cout << _reader->getStatusXml() << endl;
    cout << "----------" << endl << endl;
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    ray->printWithFieldData(cout);
  } else {
    ray->printWithFieldMeta(cout);
  }
        
  
} 

/////////////////////////
// initialize statistics

void GroundEchoStats::_initStats()

{
  _sumDbz = 0.0;
  _sumVrPlatform = 0.0;
  _sumVrMeasured = 0.0;
  _sumSqVrMeasured = 0.0;
  _sumVrCorrected = 0.0;
  _sumRange = 0.0;
  _count = 0;
  _statsStartTime = 0;
}

////////////////////////////////////////////////////////////////
// compute stats

void GroundEchoStats::_computeStats(const RadxRay *ray)

{

  double rayTime = ray->getTimeDouble();

  if (_globalStartTime == 0) {
    _globalStartTime = rayTime;
  }

  // get the georeference info

  const RadxGeoref *georef = ray->getGeoreference();
  if (georef == NULL) {
    _initStats();
    return;
  }
  
  // initialize fields
  
  double startRange = ray->getStartRangeKm();
  double gateSpacing = ray->getGateSpacingKm();
  int startGateNum =
    (int) ((_params.min_range_for_ground - startRange) / gateSpacing + 0.5);
  if (startGateNum > (int) ray->getNGates() - 1) {
    startGateNum = ray->getNGates() - 1;
  }

  if (ray->getField(_params.dbz_field_name) == NULL) {
    cerr << "ERROR - GroundEchoStats::_computeStats" << endl;
    cerr << "  Cannot find field: " << _params.dbz_field_name << endl;
    return;
  }
  if (ray->getField(_params.vel_field_name) == NULL) {
    cerr << "ERROR - GroundEchoStats::_computeStats" << endl;
    cerr << "  Cannot find field: " << _params.vel_field_name << endl;
    return;
  }
  
  RadxField dbzField(*ray->getField(_params.dbz_field_name));
  dbzField.convertToFl32();
  RadxField velField(*ray->getField(_params.vel_field_name));
  velField.convertToFl32();

  // find the ground echo by
  // finding the location of max reflectivity

  const Radx::fl32 *dbzData = dbzField.getDataFl32();
  const Radx::fl32 *velData = velField.getDataFl32();

  double maxDbz = -9999;
  int groundRangeGate = 0;
  for (int igate = startGateNum; igate < (int) ray->getNGates(); igate++) {
    double dbz = dbzData[igate];
    if (dbz > maxDbz) {
      groundRangeGate = igate;
      maxDbz = dbz;
    }
  }
  if (maxDbz < _params.min_dbz_for_stats) {
    _initStats();
    return;
  }

  double vrMeasured = velData[groundRangeGate];
  double range = startRange + groundRangeGate * gateSpacing;
  
  // compute the georeferenced variables

  if (_params.debug >= Params::DEBUG_EXTRA) {
    ray->print(cerr);
  }

  double rotation = georef->getRotation() + _params.rotation_correction;
  double tilt = georef->getTilt() + _params.tilt_correction;
  double ewVelocity = georef->getEwVelocity();
  double nsVelocity = georef->getNsVelocity();
  double vertVelocity = georef->getVertVelocity();
  double heading = georef->getHeading();
  double roll = georef->getRoll();
  double pitch = georef->getPitch();
  double drift = georef->getDrift();

  // compute the platform velocity

  double speed = sqrt(ewVelocity * ewVelocity + nsVelocity * nsVelocity); 
  double track = atan2(ewVelocity, nsVelocity) * RAD_TO_DEG;
  double driftCheck = track - heading;
  if (driftCheck < -180) {
    driftCheck += 360;
  } else if (driftCheck > 180) {
    driftCheck -= 360.0;
  }

  double R = roll * DEG_TO_RAD;
  double P = pitch * DEG_TO_RAD;
  double H = heading * DEG_TO_RAD;
  double D = drift * DEG_TO_RAD;
  double T = H + D;
  
  double sinP = sin(P);
  double cosP = cos(P);
  double sinD = sin(D);
  double cosD = cos(D);
  
  double theta_a = rotation * DEG_TO_RAD;
  double tau_a = tilt * DEG_TO_RAD;
  double sin_tau_a = sin(tau_a);
  double cos_tau_a = cos(tau_a);
  double sin_theta_rc = sin(theta_a + R); /* roll corrected rotation angle */
  double cos_theta_rc = cos(theta_a + R); /* roll corrected rotation angle */
  
  double xsubt = (cos_theta_rc * sinD * cos_tau_a * sinP
                  + cosD * sin_theta_rc * cos_tau_a
                  -sinD * cosP * sin_tau_a);
  
  double ysubt = (-cos_theta_rc * cosD * cos_tau_a * sinP
                  + sinD * sin_theta_rc * cos_tau_a
                  + cosP * cosD * sin_tau_a);
  
  double zsubt = (cosP * cos_tau_a * cos_theta_rc
                  + sinP * sin_tau_a);

  double theta_subt = atan2(xsubt, zsubt);
  double tau_subt = asin(ysubt);
  double lambda_t = atan2(xsubt, ysubt);
  double azimuthRad = fmod(lambda_t + T, M_PI * 2.0);
  double phi_subt = asin(zsubt);

  double theta_subt_Deg = theta_subt * RAD_TO_DEG;
  double tau_subt_Deg = tau_subt * RAD_TO_DEG;
  double elev = phi_subt * RAD_TO_DEG;
  double az = azimuthRad * RAD_TO_DEG;
  if (az < 0) {
    az += 360.0;
  }

  double vrPlatform =  - (vertVelocity * zsubt) - (speed * ysubt);
  double vrCorrected = vrMeasured - vrPlatform;
  double tiltErrorDeg = - asin(vrCorrected / speed) * RAD_TO_DEG;
  
  // check limits

  bool process = true;
  
  if (_params.check_elevation) {
    if (elev > _params.max_elevation || elev < _params.min_elevation) {
      process = false;
    }
  }

  if (_params.check_roll) {
    if (fabs(roll) > _params.max_abs_roll) {
      process = false;
    }
  }

  if (_params.check_tilt) {
    if (fabs(tilt) > _params.max_abs_tilt) {
      process = false;
    }
  }

  if (_params.check_drift) {
    if (fabs(drift) < _params.min_abs_drift) {
      process = false;
    }
    if (fabs(drift) > _params.max_abs_drift) {
      process = false;
    }
  }

  if (_params.check_vert_velocity) {
    if (fabs(vertVelocity) > _params.max_abs_vert_velocity) {
      process = false;
    }
  }

  if (!process) {
    _initStats();
    return;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Ray time: " << RadxTime::strm(rayTime) << endl;
    cerr << "  elev: " << elev << endl;
    cerr << "  az: " << az << endl;
    cerr << "  rotation: " << rotation << endl;
    if (_params.rotation_correction != 0) {
      cerr << "  rotation_correction: " << _params.rotation_correction << endl;
    }
    cerr << "  tilt: " << tilt << endl;
    if (_params.tilt_correction != 0) {
      cerr << "  tilt_correction: " << _params.tilt_correction << endl;
    }
    cerr << "  ewVelocity: " << ewVelocity << endl;
    cerr << "  nsVelocity: " << nsVelocity << endl;
    cerr << "  vertVelocity: " << vertVelocity << endl;
    cerr << "  heading: " << heading << endl;
    cerr << "  roll: " << roll << endl;
    cerr << "  pitch: " << pitch << endl;
    cerr << "  drift: " << drift << endl;
    cerr << "  track: " << track << endl;
    cerr << "  speed: " << speed << endl;
    cerr << "  theta_subt_Deg: " << theta_subt_Deg << endl;
    cerr << "  tau_subt_Deg: " << tau_subt_Deg << endl;
    cerr << "  vrPlatform: " << vrPlatform << endl;
    cerr << "  vrMeasured: " << vrMeasured << endl;
    cerr << "  vrCorrected: " << vrCorrected << endl;
    cerr << "  tiltErrorDeg: " << tiltErrorDeg << endl;
  }

  // acumulate stats

  _sumDbz += maxDbz;
  _sumVrPlatform += vrPlatform;
  _sumVrMeasured += vrMeasured;
  _sumSqVrMeasured += vrMeasured * vrMeasured;
  _sumVrCorrected += vrCorrected;
  _sumRange += range;
  _count++;
  
  if (_statsStartTime == 0) {
    _statsStartTime = rayTime;
  }
  _statsEndTime = rayTime;

  // check we have enough data
  
  if (_count < _params.nrays_for_stats) {
    return;
  }
  
  // compute stats
  
  double meanDbz = _sumDbz / _count;
  double meanVrPlatform = _sumVrPlatform / _count;
  double meanVrMeasured = _sumVrMeasured / _count;

  double sdevVrMeasured = _computeSdev(_sumVrMeasured,
                                       _sumSqVrMeasured,
                                       _count);

  double meanVrCorrected = _sumVrCorrected / _count;
  double meanRange = _sumRange / _count;
  double meanTime =
    ((_statsStartTime + _statsEndTime) / 2.0) - _globalStartTime;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "meanTime: " << RadxTime::strm((time_t) meanTime) << endl;
    cerr << "  meanDbz: " << meanDbz << endl;
    cerr << "  meanVrPlatform: " << meanVrPlatform << endl;
    cerr << "  meanVrMeasured: " << meanVrMeasured << endl;
    cerr << "  sdevVrMeasured: " << sdevVrMeasured << endl;
    cerr << "  meanVrCorrected: " << meanVrCorrected << endl;
    cerr << "  meanRange: " << meanRange << endl;
  }

  // check for turbulence using sdev of measured vel
  
  if (_params.check_sdev_vr_measured) {
    if (sdevVrMeasured > _params.max_sdev_vr_measured) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Discarding entry, sdevVrMeasured: " << sdevVrMeasured << endl;
      }
      _initStats();
      return;
    }
  }

  RadxTime rtime(rayTime);
  double subSec = rtime.getSubSec();
  int msecs = (int) (subSec * 1000.0 + 0.5);

  if (_params.print_table_to_stdout) {
    fprintf(stdout,
            "%.4d %.2d %.2d %.2d %.2d %.2d %.3d "
            "%15.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f "
            "%10.3f %10.3f %10.3f\n",
            rtime.getYear(),
            rtime.getMonth(),
            rtime.getDay(),
            rtime.getHour(),
            rtime.getMin(),
            rtime.getSec(),
            msecs,
            meanTime,
            meanRange,
            meanDbz,
            meanVrPlatform,
            meanVrMeasured,
            meanVrCorrected,
            tiltErrorDeg,
            pitch,
            -tilt,
            sdevVrMeasured);
    fflush(stdout);
  }

  // re-initialize stats

  _initStats();

  // compute median

  _vrCorrected.push_back(meanVrCorrected);
  _tiltError.push_back(tiltErrorDeg);

  if ((int) _vrCorrected.size() == _params.n_results_for_median) {

    sort(_vrCorrected.begin(), _vrCorrected.end());
    sort(_tiltError.begin(), _tiltError.end());
    double medianVrCorrected = _vrCorrected[_vrCorrected.size()/2];
    double medianTiltError = _tiltError[_tiltError.size()/2];

    _globalSumVrCorrected += medianVrCorrected;
    _globalSumSqVrCorrected += medianVrCorrected * medianVrCorrected;
    _globalSumTiltError += medianTiltError;
    _globalSumSqTiltError += medianTiltError * medianTiltError;
    _globalCount++;

    double globalVrCorrected = _globalSumVrCorrected / _globalCount;
    double globalTiltError = _globalSumTiltError / _globalCount;

    double sdevVrCorrected = _computeSdev(_globalSumVrCorrected,
                                          _globalSumSqVrCorrected,
                                          _globalCount);
    
    double sdevTiltError = _computeSdev(_globalSumTiltError,
                                        _globalSumSqTiltError,
                                        _globalCount);
    
    fprintf(stderr,
            "medVrCorr globVrCorr sdevVrCorr "
            "medTiltErr globTiltErr sdevTiltErr: "
            "%10.3f %10.3f %10.5f %10.3f %10.3f %10.5f\n",
            medianVrCorrected,
            globalVrCorrected,
            sdevVrCorrected,
            medianTiltError,
            globalTiltError,
            sdevTiltError);
    
    _vrCorrected.clear();
    _tiltError.clear();

  }

}

////////////////////////////////////////////////////////////////
// compute sdev

double GroundEchoStats::_computeSdev(double sum,
                                     double sumSq,
                                     int count)

{

  double mean = sum / count;
  double sdev = 0.0;
  if (count > 2) {
    double term1 = sumSq / count;
    double term2 = mean * mean;
    if (term1 >= term2) {
      sdev = sqrt(term1 - term2);
    }
  }

  return sdev;

}


