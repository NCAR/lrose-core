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
//////////////////////////////////////////////////////////////////////////
// HcrMomentsCombine.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2026
//
//////////////////////////////////////////////////////////////////////////
//
// HCR has the capability to transmit blocks of pulses with varying PRTs 
//   and pulse lengths.
//
// The latest version supports 3 block types:
//
//      (1) short-pulse and short-PRT
//      (2) long-pulse and long-PRT
//      (3) long-pulse and short-PRT.
//
// This sequence is repeated in time.
//
// HcrTs2Moments reads this interleaved time series, and computes the 
//   relevant moments for each block. Those moments are then written, in 
//   sequence, to a single output FMQ in Radx moments format.
//
// HcrMomentsCombine reads the Radx moments data stream, and combines the 
//   three blocks into a single block, naming the fields appropriately, 
//   and unfolding the velocity fields as appropriate. This allows us to 
//   unfold the velocity field using the staggered-PRT technique.
//
//////////////////////////////////////////////////////////////////////////

#include "HcrMomentsCombine.hh"
#include <Radx/RadxRay.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxStatusXml.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/pmu.h>
#include <toolsa/sincos.h>
using namespace std;

// Constructor

HcrMomentsCombine::HcrMomentsCombine(int argc, char **argv)
  
{

  OK = TRUE;
  _momReader = nullptr;
  _outputFmq = nullptr;
  _cachedRay = nullptr;

  // init staggered prt
  
  _wavelengthM = 0.003176;
  _prtShort = 0.000101376;
  _prtLong = _prtShort * 1.5;
  _nyquistShort = ((_wavelengthM / _prtShort) / 4.0);
  _nyquistLong = ((_wavelengthM / _prtLong) / 4.0);
  _stagM = 2;
  _stagN = 3;

  // set programe name

  _progName = "HcrMomentsCombine";
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list, &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

  _dwellLengthSecs = _params.dwell_length_secs;
  _dwellLengthSecsHalf = _dwellLengthSecs / 2.0;

  // set dwell stats method

  _globalMethod = _getDwellStatsMethod(_params.dwell_stats_method);
  
  if (_params.set_stats_method_for_individual_fields) {
    for (int ii = 0; ii < _params.stats_method_fields_n; ii++) {
      const Params::stats_method_field_t &paramsMethod = 
        _params._stats_method_fields[ii];
      string fieldName = paramsMethod.field_name;
      RadxField::StatsMethod_t method =
        _getDwellStatsMethod(paramsMethod.stats_method);
      RadxField::NamedStatsMethod namedMethod(fieldName, method);
      _namedMethods.push_back(namedMethod);
    } // ii
  }

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

}

// destructor

HcrMomentsCombine::~HcrMomentsCombine()

{

  if (_momReader) {
    delete _momReader;
  }

  if (_outputFmq) {
    delete _outputFmq;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int HcrMomentsCombine::Run()
{

  if (_params.compute_mean_location) {
    return _computeMeanLocation();
  } else if (_params.mode == Params::ARCHIVE) {
    return _runArchive();
  } else {
    return _runRealtime();
  }

}

//////////////////////////////////////////////////
// Run in REALTIME mode

int HcrMomentsCombine::_runRealtime()
{

  // Open the output fmq

  if (_openOutputFmq()) {
    return -1;
  }

  // Instantiate and initialize the input radar queue

  if (_openInputFmq()) {
    return -1;
  }

  // prepare the input rays at the start of the first output dwell
  
  if (_initializeInput()) {
    return -1;
  }

  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "====>> Unfold stagM, stagN: " << _stagM << ", " << _stagN << endl;
  }
  
  _nRaysRead = 0;
  _nRaysWritten = 0;

  // loop forever
  
  while (true) {

    // read in next dwell for short and long
    
    PMU_auto_register("Reading FMQ realtime");
    if (_readNextDwell()) {
      return -1;
    }

    // process the dwell, combining and writing out to FMQ

    _processDwell();
    
  } // while
  
  return 0;

}

//////////////////////////////////////////////////
// Run in archive mode

int HcrMomentsCombine::_runArchive()
{

  // Open the output fmq

  if (_openOutputFmq()) {
    return -1;
  }

  // Instantiate and initialize the input radar queues

  if (_openFileReader()) {
    return -1;
  }
  
  // prepare the input rays at the start of the first output dwell
  
  if (_initializeInput()) {
    return -1;
  }

  _nRaysRead = 0;
  _nRaysWritten = 0;

  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "====>> Unfold stagM, stagN: " << _stagM << ", " << _stagN << endl;
  }
  
  // loop until readers are empty
  
  while (true) {
    
    // read in next dwell for short and long
    
    if (_readNextDwell()) {
      return -1;
    }
    
    // process the dwell, combining and writing out to FMQ

    _processDwell();
    
  } // while
  
  return 0;

}

//////////////////////////////////////////////////
// Process the dwell

void HcrMomentsCombine::_processDwell()
{

  // check the dwells for consistent PRTs

  
  // combine short-short, long-long and long-short dwells
  
  RadxRay *rayCombined = nullptr;
  
  if (_dwellRaysSS.size() > 0 &&
      _dwellRaysLL.size() > 0 &&
      _dwellRaysLS.size() > 0) {
    
    rayCombined = _combineDwellTriple();

  } else if (_dwellRaysSS.size() > 0 &&
             _dwellRaysLL.size() > 0) {
    
    rayCombined = _combineDwellDual();

  } else if (_dwellRaysFixed.size() > 0) {

    rayCombined = _combineDwellFixed();

  }
  
  if (rayCombined == nullptr) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - HcrMomentsCombine::_processDwell" << endl;
      cerr << "  no combined ray created" << endl;
    }
    return;
  }

  // create output message from combined ray
  
  RadxMsg msg;
  rayCombined->serialize(msg);
  if ((_params.debug >= Params::DEBUG_VERBOSE) ||
      (_params.debug && (_nRaysWritten % 1000 == 0))) {
    cerr << "Writing ray, time, el, az, rayNum: "
         << rayCombined->getRadxTime().asString(3) << ", "
         << rayCombined->getElevationDeg() << ", "
         << rayCombined->getAzimuthDeg() << ", "
         << _nRaysWritten << endl;
  }
    
  // write the message
  
  if (_outputFmq) {
    if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                             msg.assembledMsg(), msg.lengthAssembled())) {
      cerr << "WARNING - HcrMomentsCombine::_processDwell()" << endl;
      cerr << "  Cannot write ray to output queue" << endl;
    }
  } else {
    _nRaysWritten++;
  }

  // free up memory
    
  delete rayCombined;
    
}
    
//////////////////////////////////////////////////
// Compute mean location

int HcrMomentsCombine::_computeMeanLocation()
{

  if (_params.debug) {
    cerr << "HcrMomentsCombine::_computeMeanLocation()" << endl;
  }

  // Instantiate and initialize the input radar queues
  
  if (_openFileReader()) {
    return -1;
  }
  
  // process short rays for the dwell

  if (_params.debug) {
    cerr << "  Computing mean location for moments data ...." << endl;
  }

  double sumLat = 0.0;
  double sumLon = 0.0;
  double sumAlt = 0.0;
  long nRays = 0;
  
  RadxRay *ray = _momReader->readNextRay();
  while (ray != nullptr) {
    const RadxGeoref *georef = ray->getGeoreference();
    if (georef != nullptr) {
      double lat = georef->getLatitude();
      double lon = georef->getLongitude();
      double alt = georef->getAltitudeKmMsl();
      if (lat >= -90.0 && lat <= 90.0 &&
          lon >= -360.0 && lon <= 360 &&
          alt > -1.0 && alt < 25.0) {
        sumLat += lat;
        sumLon += lon;
        sumAlt += alt;
        nRays += 1.0;
      }
    }
    if (nRays > 0 && nRays % 10000 == 0) {
      cerr << "  data time, n rays short processed: "
           << ray->getRadxTime().asString(6) << ", "
           << nRays << endl;
    }
    delete ray;
    ray = _momReader->readNextRay();
  } // while
  
  _meanLat = _meanLon = _meanAlt = -9999.0;
  if (nRays > 0) {
    _meanLat = sumLat / nRays;
    _meanLon = sumLon / nRays;
    _meanAlt = sumAlt / nRays;
  }
  
  fprintf(stderr, "HcrMomentsCombine::_computeMeanLocation()\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  nRays           : %ld\n", nRays);
  fprintf(stderr, "  meanLat (deg)   : %10.6f\n", _meanLat);
  fprintf(stderr, "  meanLon (deg)   : %10.6f\n", _meanLon);
  fprintf(stderr, "  meanAlt (kmMSL) : %10.6f\n", _meanAlt);
  fprintf(stderr, "\n");

  return 0;

}

//////////////////////////////////////////////////
// Open input fmqs

int HcrMomentsCombine::_openInputFmq()
{

  // Instantiate and initialize the input radar queues

  if (_params.debug) {
    cerr << "DEBUG - opening input fmq: "
         << _params.input_fmq_url << endl;
  }
  
  _momReader = new IwrfMomReaderFmq(_params.input_fmq_url);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _momReader->setDebug(IWRF_DEBUG_NORM);
  }

  // initialize reader - read one ray

  RadxRay *ray = _readRayNext();
  if (ray == nullptr) {
    cerr << "ERROR - _openInputFmq()" << endl;
    cerr << "Cannot read ray from input fmq: " << _params.input_fmq_url << endl;
    return -1;
  }

  // free memory
  
  delete ray;

  // position in queue
  
  if (_params.seek_to_end_of_input_fmq) {
    _momReader->seekToEnd();
  } else {
    _momReader->seekToStart();
  }

  return 0;

}

//////////////////////////////////////////////////
// Open readers from CfRadial files

int HcrMomentsCombine::_openFileReader()
{

  // Instantiate and initialize the input radar queues

  if (_params.debug) {
    cerr << "DEBUG - opening input dir for moments data: "
         << _params.input_dir << endl;
  }

  RadxTime startTime(_args.startTime);
  RadxTime endTime(_args.endTime);
  
  _momReader = new IwrfMomReaderFile(_params.input_dir, startTime, endTime);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _momReader->setDebug(IWRF_DEBUG_NORM);
  }

  // initialize reader - read one ray
  
  RadxRay *ray = _readRayNext();
  if (ray == nullptr) {
    cerr << "ERROR - HcrMomentsCombine::_openFileReader()" << endl;
    cerr << "  Cannot read rays from dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << startTime.asString(0) << endl;
    cerr << "  End time: " << endTime.asString(0) << endl;
    return -1;
  }

  // free memory
  
  delete ray;
  
  return 0;

}

//////////////////////////////////////////////////
// Open output fmq

int HcrMomentsCombine::_openOutputFmq()
{

  // create the output FMQ
  
  _outputFmq = new DsFmq;

  if (_outputFmq->init(_params.output_fmq_url,
                       _progName.c_str(),
                       _params.debug >= Params::DEBUG_VERBOSE,
                       DsFmq::READ_WRITE, DsFmq::END,
                       _params.output_fmq_compress,
                       _params.output_fmq_n_slots,
                       _params.output_fmq_buf_size)) {
    cerr << "ERROR - " << _progName << "::_openFmqs" << endl;
    cerr << "  Cannot open output fmq, URL: " << _params.output_fmq_url << endl;
    return -1;
  }
  if (_params.output_fmq_compress) {
    _outputFmq->setCompressionMethod(TA_COMPRESSION_GZIP);
  }
  if (_params.output_fmq_write_blocking) {
    _outputFmq->setBlockingWrite();
  }
  if (_params.output_fmq_data_mapper_report_interval > 0) {
    _outputFmq->setRegisterWithDmap(true, _params.output_fmq_data_mapper_report_interval);
  }
  // _outputFmq->setSingleWriter();

  return 0;

}

/////////////////////////////////////////////////////////////////
// Initialize the input rays at the start of the first output dwell

int HcrMomentsCombine::_initializeInput()
{

  // read a single ray
  
  RadxRay *rayFirst = _readRayNext();
  if (rayFirst == nullptr) {
    cerr << "ERROR - HcrMomentsCombine::_initializeInput()" << endl;
    if (_params.mode == Params::REALTIME) {
      cerr << "  Cannot read input fmq: " << _params.input_fmq_url << endl;
    } else {
      cerr << "  Cannot read input dir: " << _params.input_dir << endl;
    }
    return -1;
  }

  // initial time
  
  _firstRayTime = rayFirst->getRadxTime();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=======>> first ray read, time: " << _firstRayTime.asString(6) << endl;
  }
  
  // set initial dwell limits
  
  _setDwellTimeLimits(rayFirst);
  delete rayFirst;
  
  // read rays, prepare for first dwell
  
  _cachedRay = nullptr;
  while (true) {
    RadxRay *ray = _readRayNext();
    if (ray == nullptr) {
      cerr << "========>> input queue done <<==========" << endl;
      return -1;
    }
    if (ray->getRadxTime() >= _nextDwellStartTime) {
      // save for next dwell
      _cachedRay = ray;
      break;
    } else {
      // read ahead
      delete ray;
    }
  }
  
  return 0;

}

/////////////////////////////////////////////////////////////////
// add ray to dwell vector

void HcrMomentsCombine::_addDwellRay(RadxRay *ray)
{
  
  _dwellRays.push_back(ray);
  
  string dwellLabel = ray->getScanName();
  
  if (dwellLabel.find(_params.short_short_dwell_label) != string::npos) {
    _dwellRaysSS.push_back(ray);
    return;
  }
  
  if (dwellLabel.find(_params.long_long_dwell_label) != string::npos) {
    _dwellRaysLL.push_back(ray);
    return;
  }
  
  if (dwellLabel.find(_params.long_short_dwell_label) != string::npos) {
    _dwellRaysLS.push_back(ray);
    return;
  }
  
  if (dwellLabel.find(_params.short_fixed_dwell_label) != string::npos) {
    _dwellRaysFixed.push_back(ray);
    return;
  }
  
  if (dwellLabel.find(_params.long_fixed_dwell_label) != string::npos) {
    _dwellRaysFixed.push_back(ray);
    return;
  }
  
}

/////////////////////////////////////////////////////////////////
// clear the dwell rays

void HcrMomentsCombine::_clearDwellRays()
{

  _dwellRays.clear();

  for (size_t ii = 0; ii < _dwellRaysSS.size(); ii++) {
    delete _dwellRaysSS[ii];
  }
  _dwellRaysSS.clear();
  
  for (size_t ii = 0; ii < _dwellRaysLL.size(); ii++) {
    delete _dwellRaysLL[ii];
  }
  _dwellRaysLL.clear();
  
  for (size_t ii = 0; ii < _dwellRaysLS.size(); ii++) {
    delete _dwellRaysLS[ii];
  }
  _dwellRaysLS.clear();
  
  for (size_t ii = 0; ii < _dwellRaysFixed.size(); ii++) {
    delete _dwellRaysFixed[ii];
  }
  _dwellRaysFixed.clear();
  
}

/////////////////////////////////////////////////////////////////
// print dwell ray info

void HcrMomentsCombine::_printDwellRayInfo(ostream &out)
{

  if (_dwellRaysSS.size() > 0) {
    cerr << "=================>> short short ray count: " << _dwellRaysSS.size() << endl;
    for (size_t ii = 0; ii < _dwellRaysSS.size(); ii++) {
      cerr << "  short short ray time: "
           << _dwellRaysSS[ii]->getRadxTime().asString(6) << ", "
           <<  _dwellRaysSS[ii] << endl;
    }
    cerr << "========================================" << endl;
  }
  if (_dwellRaysLL.size() > 0) {
    cerr << "=================>> long long ray count: " << _dwellRaysLL.size() << endl;
    for (size_t ii = 0; ii < _dwellRaysLL.size(); ii++) {
      cerr << "  long long ray time: "
           << _dwellRaysLL[ii]->getRadxTime().asString(6) << ", "
           <<  _dwellRaysLL[ii] << endl;
    }
    cerr << "========================================" << endl;
  }
  if (_dwellRaysLS.size() > 0) {
    cerr << "=================>> long short ray count: " << _dwellRaysLS.size() << endl;
    for (size_t ii = 0; ii < _dwellRaysLS.size(); ii++) {
      cerr << "  long short ray time: "
           << _dwellRaysLS[ii]->getRadxTime().asString(6) << ", "
           <<  _dwellRaysLS[ii] << endl;
    }
    cerr << "========================================" << endl;
  }
  if (_dwellRaysFixed.size() > 0) {
    cerr << "=================>> fixed PRT ray count: " << _dwellRaysFixed.size() << endl;
    for (size_t ii = 0; ii < _dwellRaysFixed.size(); ii++) {
      cerr << "  long short ray time: "
           << _dwellRaysFixed[ii]->getRadxTime().asString(6) << ", "
           <<  _dwellRaysFixed[ii] << endl;
    }
    cerr << "========================================" << endl;
  }

}

/////////////////////////////////////////////////////////////////
// Read in rays for next dwell

int HcrMomentsCombine::_readNextDwell()
{

  // clear the dwell vectors

  _clearDwellRays();
  
  // add cached ray if applicable

  if (_cachedRay != nullptr) {
    _addDwellRay(_cachedRay);
    _cachedRay = nullptr;
  }    
  
  // read in short rays for the dwell
  
  while (true) {
    RadxRay *ray = _readRayNext();
    if (_checkForTimeGap(ray)) {
      return -1;
    }
    if (ray == nullptr) {
      cerr << "ERROR - HcrMomentsCombine::_readNextDwell()" << endl;
      if (_params.mode == Params::REALTIME) {
        cerr << "  Cannot read input fmq: " << _params.input_fmq_url << endl;
      } else {
        cerr << "  Cannot read input dir: " << _params.input_dir << endl;
      }
      return -1;
    }
    if (ray->getRadxTime() >= _nextDwellEndTime) {
      // save for start of next dwell
      _cachedRay = ray;
      // we have a full dwell
      break;
    } else {
      _addDwellRay(ray);
    }
  }
  
  // remove the corrected velocity field if it exists, since it will
  // be replaced by values computed in this app
  
  for (size_t ii = 0; ii < _dwellRays.size(); ii++) {
    RadxRay *ray = _dwellRays[ii];
    RadxField *velCorr = ray->getField(_params.input_vel_corr_field_name);
    if (velCorr != nullptr) {
      ray->removeField(_params.input_vel_corr_field_name);
    }
  }
  
  // debug prints
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _printDwellRayInfo(cerr);
  }

  // set to advance to next dwell
  
  _nextDwellStartTime = _nextDwellEndTime;
  _nextDwellMidTime = _nextDwellStartTime + _dwellLengthSecsHalf;
  _nextDwellEndTime = _nextDwellStartTime + _dwellLengthSecs;
  _thisDwellMidTime = _nextDwellMidTime - _dwellLengthSecs;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> thisDwellMidTime  : " << _thisDwellMidTime.asString(6) << endl;
    cerr << "====>> nextDwellStartTime: " << _nextDwellStartTime.asString(6) << endl;
    cerr << "====>> nextDwellMidTime  : " << _nextDwellMidTime.asString(6) << endl;
    cerr << "====>> nextDwellEndTime  : " << _nextDwellEndTime.asString(6) << endl;
  }

  return 0;

}

/////////////////////////////////////////////////////////////////
// set dwell time limits

void HcrMomentsCombine::_setDwellTimeLimits(RadxRay *ray)

{

  // get the ray time
  
  RadxTime rayTime = ray->getRadxTime();
  double raySecs = rayTime.asDouble();
  
  // compute the dwell limits
  
  double dwellMidSecs = (floor(raySecs / _dwellLengthSecs) + 1.0) * _dwellLengthSecs;
  
  _nextDwellMidTime.setFromDouble(dwellMidSecs);
  _nextDwellStartTime = _nextDwellMidTime - _dwellLengthSecsHalf;
  _nextDwellEndTime = _nextDwellMidTime + _dwellLengthSecsHalf;
  _thisDwellMidTime = _nextDwellMidTime - _dwellLengthSecs;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> thisDwellMidTime  : " << _thisDwellMidTime.asString(6) << endl;
    cerr << "====>> nextDwellStartTime: " << _nextDwellStartTime.asString(6) << endl;
    cerr << "====>> nextDwellMidTime  : " << _nextDwellMidTime.asString(6) << endl;
    cerr << "====>> nextDwellEndTime  : " << _nextDwellEndTime.asString(6) << endl;
  }

}

/////////////////////////////////////////////////////////////////
// check moments in dwell all have the same PRT

int HcrMomentsCombine::_checkDwellConstantPrt(const vector<RadxRay *> &dwellRays)
  
{

  if (dwellRays.size() < 1) {
    return -1;
  }
  
  double prt0 = dwellRays[0]->getPrtSec();

  for (size_t ii = 1; ii < dwellRays.size(); ii++) {
    
    double prt = dwellRays[ii]->getPrtSec();
    double diff = fabs(prt0 - prt);

    if (diff > 1.0e-9) {
      return -1;
    }
    
  } // ii

  return 0;

}

/////////////////////////////////////////////////////////////////
// check for a significant time gap
// if found, re-initialize

int HcrMomentsCombine::_checkForTimeGap(RadxRay *latestRayShort)

{

  // check for time gap
  
  RadxTime latestTimeShort = latestRayShort->getRadxTime();
  if ((latestTimeShort - _prevTimeShort) > _dwellLengthSecs * 5) {

    // start again
    _clearDwellRays();
    _cachedRay = nullptr;
    
    if (_initializeInput()) {
      _prevTimeShort = latestTimeShort;
      cerr << "HcrMomentsCombine::_checkForTimeGap" << endl;
      cerr << "  _initializeInput() failed" << endl;
      return -1;
    }
    
  }

  _prevTimeShort = latestTimeShort;
  return 0;
  
}

/////////////////////////////////////////////////////////////////
// compute mean moments for a dwell, return ray

RadxRay *HcrMomentsCombine::_computeMeanMoments(vector<RadxRay *> &dwellRays,
                                                RadxVol &dwellVol)
  
{
  
  // sanity check
  
  if (dwellRays.size() < 1) {
    return nullptr;
  }
  
  // add rays to vol
  
  dwellVol.clear();
  for (size_t iray = 0; iray < dwellRays.size(); iray++) {
    dwellVol.addRay(dwellRays[iray]);
  }
  dwellVol.loadVolumeInfoFromRays();

  // ownership of rays passed to vol, which will free them
  
  dwellRays.clear();

  // compute moments
  
  RadxRay *ray =
    dwellVol.computeFieldStats(_globalMethod, _namedMethods,
                               _params.dwell_stats_max_fraction_missing);

  return ray;

}
  
/////////////////////////////////////////////////////////////////
// combine dwell rays from triple pulse scheme
// returns pointer to combined ray - this must be freed by caller.

RadxRay *HcrMomentsCombine::_combineDwellTriple()

{

  // check that the dwells have constant PRT
  
  if (_checkDwellConstantPrt(_dwellRaysSS) ||
      _checkDwellConstantPrt(_dwellRaysLL) ||
      _checkDwellConstantPrt(_dwellRaysLS)) {
    return nullptr;
  }

  // compute the mean moments for short-pulse short-PRT
  
  RadxRay *raySS = _computeMeanMoments(_dwellRaysSS, _dwellVolSS);
  if (raySS == nullptr) {
    return nullptr;
  }
  
  // compute the mean moments for long-pulse long-PRT
  
  RadxRay *rayLL = _computeMeanMoments(_dwellRaysLL, _dwellVolLL);
  if (rayLL == nullptr) {
    delete raySS;
    return nullptr;
  }
  
  // compute the mean moments for long-pulse short-PRT
  
  RadxRay *rayLS = _computeMeanMoments(_dwellRaysLS, _dwellVolLS);
  if (rayLS == nullptr) {
    delete raySS;
    delete rayLL;
    return nullptr;
  }
  
  // rename short fields and add to the long moments ray

  vector<RadxField *> fieldsSS = raySS->getFields();
  for (size_t ifield = 0; ifield < fieldsSS.size(); ifield++) {
    RadxField *fld = fieldsSS[ifield];
    string newName = fld->getName() + _params.suffix_for_short_pulse_fields;
    fld->setName(newName);
    rayLL->addField(fld);
  }
  
  // unfold the velocity for long pulse, add unfolded field to ray
  // correct the velocity for platform motion

  _initDualPrt(raySS->getPrtSec(), rayLL->getPrtSec());

  {
    
    RadxField *velShort = rayLS->getField(_params.input_vel_raw_field_name);
    RadxField *velLong = rayLL->getField(_params.input_vel_raw_field_name);
    
    if (velShort != nullptr && velLong != nullptr) {
      RadxField *velUnfold = _unfoldVel(velShort, velLong);
      if (velUnfold != nullptr) {
        velUnfold->setName(_params.output_vel_unfolded_field_name_long_pulse);
        rayLL->addField(velUnfold);
        rayLL->setNyquistMps(_nyquistUnfolded);
        _computeVelCorrectedForVertMotion(rayLL, velShort, velLong, velUnfold);
      }
    } else {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - HcrMomentsCombine::_combineDwellTriple()" << endl;
        cerr << "  Cannot find long pulse velocity fields to unfold." << endl;
        cerr << "  Vel field name: " << _params.input_vel_raw_field_name << endl;
      }
    }

  }
  
  // unfold the velocity for short pulse, add unfolded field to ray
  // correct the velocity for platform motion
  
  {
    
    RadxField *velShort = raySS->getField(_params.input_vel_raw_field_name);
    RadxField *velLong = rayLL->getField(_params.input_vel_raw_field_name);
    
    if (velShort != nullptr && velLong != nullptr) {
      RadxField *velUnfold = _unfoldVel(velShort, velLong);
      if (velUnfold != nullptr) {
        velUnfold->setName(_params.output_vel_unfolded_field_name_short_pulse);
        rayLL->addField(velUnfold);
        rayLL->setNyquistMps(_nyquistUnfolded);
        _computeVelCorrectedForVertMotion(rayLL, velShort, nullptr, velUnfold);
      }
    } else {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - HcrMomentsCombine::_combineDwellTriple()" << endl;
        cerr << "  Cannot find short pulse velocity fields to unfold." << endl;
        cerr << "  Vel field name: " << _params.input_vel_raw_field_name << endl;
      }
    }

  }
  
  // set the combined time
  
  rayLL->setTime(_thisDwellMidTime);
  
  // free up memory

  _clearDwellRays();
  delete raySS;
  delete rayLS;

  // return combined ray
  
  return rayLL;

}

/////////////////////////////////////////////////////////////////
// combine dwell rays from dual pulse scheme
// returns pointer to combined ray - this must be freed by caller.

RadxRay *HcrMomentsCombine::_combineDwellDual()

{
  
  // check that the dwells have constant PRT
  
  if (_checkDwellConstantPrt(_dwellRaysSS) ||
      _checkDwellConstantPrt(_dwellRaysLL)) {
    return nullptr;
  }

  // compute the mean moments for short-pulse short-PRT
  
  RadxRay *raySS = _computeMeanMoments(_dwellRaysSS, _dwellVolSS);
  if (raySS == nullptr) {
    return nullptr;
  }
  
  // compute the mean moments for long-pulse long-PRT
  
  RadxRay *rayLL = _computeMeanMoments(_dwellRaysLL, _dwellVolLL);
  if (rayLL == nullptr) {
    delete raySS;
    return nullptr;
  }
  
  // rename short fields and add to the long moments ray

  vector<RadxField *> fieldsSS = raySS->getFields();
  for (size_t ifield = 0; ifield < fieldsSS.size(); ifield++) {
    RadxField *fld = fieldsSS[ifield];
    string newName = fld->getName() + _params.suffix_for_short_pulse_fields;
    fld->setName(newName);
    rayLL->addField(fld);
  }
  
  // unfold the velocity for long pulse, add unfolded field to ray
  // correct the velocity for platform motion

  _initDualPrt(raySS->getPrtSec(), rayLL->getPrtSec());

  RadxField *velShort = raySS->getField(_params.input_vel_raw_field_name);
  RadxField *velLong = rayLL->getField(_params.input_vel_raw_field_name);
    
  if (velShort != nullptr && velLong != nullptr) {
    RadxField *velUnfold = _unfoldVel(velShort, velLong);
    if (velUnfold != nullptr) {
      velUnfold->setName(_params.output_vel_unfolded_field_name_long_pulse);
      rayLL->addField(velUnfold);
      rayLL->setNyquistMps(_nyquistUnfolded);
      _computeVelCorrectedForVertMotion(rayLL, velShort, velLong, velUnfold);
    }
  } else {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - HcrMomentsCombine::_combineDwellTriple()" << endl;
      cerr << "  Cannot find long pulse velocity fields to unfold." << endl;
      cerr << "  Vel field name: " << _params.input_vel_raw_field_name << endl;
    }
  }
  
  // set the combined time
  
  rayLL->setTime(_thisDwellMidTime);
  
  // free up memory
  
  _clearDwellRays();
  delete raySS;
  
  // return combined ray
  
  return rayLL;

}

/////////////////////////////////////////////////////////////////
// combine dwell rays from fixed PRT
// returns pointer to combined ray - this must be freed by caller.

RadxRay *HcrMomentsCombine::_combineDwellFixed()

{

  // check that the dwell has constant PRT
  
  if (_checkDwellConstantPrt(_dwellRaysFixed)) {
    return nullptr;
  }

  // compute the mean moments for the fixed PRT dwell
  
  RadxRay *rayFixed = _computeMeanMoments(_dwellRaysFixed, _dwellVolFixed);
  if (rayFixed == nullptr) {
    return nullptr;
  }
  
  // correct the velocity for platform motion

  RadxField *velFixedPrt = rayFixed->getField(_params.input_vel_raw_field_name);
    
  if (velFixedPrt != nullptr) {
    _computeVelCorrectedForVertMotion(rayFixed, velFixedPrt, nullptr, nullptr);
  }
  
  // set the combined time
  
  rayFixed->setTime(_thisDwellMidTime);
  
  // free up memory

  _clearDwellRays();

  return rayFixed;

}

////////////////////////////////////////////////////////////////
// Unfold the velocity, add unfolded field to ray.
// We need to use the raw velocity - i.e. not corrected for
// the vertical platform motion.
// The plaform motion correction is applied AFTER unfolding.

RadxField *HcrMomentsCombine::_unfoldVel(RadxField *velShortPrt,
                                         RadxField *velLongPrt)
  
{

  // convert to floats
  
  velShortPrt->convertToFl32();
  velLongPrt->convertToFl32();

  // copy long prt field to get unfolded
  
  RadxField *velUnfold = new RadxField(*velLongPrt);

  // compute the unfolded velocity

  int *PP = _PP_;
  _LL = (_stagM + _stagN - 1) / 2;
  if (_LL > 5) {
    _LL = 2; // set to 2/3
  }
  PP = _PP_ + _LL;
  
  int cc = 0;
  int pp = 0;
  PP[0] = 0;
  for (int ll = 1; ll <= _LL; ll++) {
    if ((ll / 2 * 2) == ll) {
      // even - va1 transition
      cc -= _stagN;
      pp++;
    } else {
      // odd - va2 transition
      cc += _stagM;
    }
    PP[cc] = pp;
    PP[-cc] = -pp;
  }
  
  size_t nGates = velUnfold->getNPoints();
  Radx::fl32 *dataShort = velShortPrt->getDataFl32();
  Radx::fl32 *dataLong = velLongPrt->getDataFl32();
  Radx::fl32 *dataUnfold = velUnfold->getDataFl32();
  double nyquistDiff = _nyquistShort - _nyquistLong;

  for (size_t ii = 0; ii < nGates; ii++) {
    
    double vel_diff = dataShort[ii] - dataLong[ii];
    double nyquistIntervalShort = (vel_diff / nyquistDiff) / 2.0;
    int ll = (int) floor(nyquistIntervalShort + 0.5);
    if (ll < -_LL) {
      ll = -_LL;
    } else if (ll > _LL) {
      ll = _LL;
    }
    double unfoldedVel = dataShort[ii] + PP[ll] * _nyquistShort * 2;
    dataUnfold[ii] = unfoldedVel;
    
  } // ii

  // correct vel for vertical motion
  
  _nyquistUnfolded = _nyquistShort * _LL;
  
#ifdef JUNK
  
  // rename vel fields
  
  string velCorrShortName = _params.output_vel_corr_field_name_short_prt;
  // velCorrShortName += _params.suffix_to_add_for_short_pulse_fields;
  string velCorrLongName = _params.output_vel_corr_field_name_long_prt;
  // velCorrLongName += _params.suffix_to_add_for_long_pulse_fields;

  velShort->setName(velCorrShortName);
  velLong->setName(velCorrLongName);
  
  // add field to ray
  
  rayCombined->addField(velUnfold);
  rayCombined->setNyquistMps(rayCombined->getNyquistMps() * _LL);

#endif

  return velUnfold;

}

///////////////////////////////////////////////////////////
// compute velocity corrected for platform motion
//
// NOTES from Ulrike's Matlab code
//  
// % Compute y_t following equation 9 Lee et al (1994)
// y_subt=-cosd(data.rotation+data.roll).*cosd(data.drift).*cosd(data.tilt).*sind(data.pitch)...
//     +sind(data.drift).*sind(data.rotation+data.roll).*cosd(data.tilt)...
//     +cosd(data.pitch).*cosd(data.drift).*sind(data.tilt);
//
// % Compute z following equation 9 Lee et al (1994)
// z=cosd(data.pitch).*cosd(data.tilt).*cosd(data.rotation+data.roll)+sind(data.pitch).*sind(data.tilt);
//
// % compute tau_t following equation 11 Lee et al (1994)
// tau_subt=asind(y_subt);
//
// % Compute phi following equation 17 Lee et al (1994)
// phi=asind(z);
//
// % Compute platform motion based on Eq 27 from Lee et al (1994)
// ground_speed=sqrt(data.eastward_velocity.^2 + data.northward_velocity.^2);
// % Use this equation when starting from VEL_RAW
// %vr_platform=-ground_speed.*sin(tau_subt)-vertical_velocity.*sin(phi);
// % Use this equation when starting from VEL
// vr_platform=-ground_speed.*sind(tau_subt).*sind(phi);
//
// velAngCorr=data.VEL+vr_platform;

void HcrMomentsCombine::_computeVelCorrectedForVertMotion(RadxRay *ray,
                                                          RadxField *velShort,
                                                          RadxField *velLong,
                                                          RadxField *velUnfolded)
  
{

  // no good if no georeference available
  
  const RadxGeoref *georef = ray->getGeoreference();
  if (georef == nullptr) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - _computeVelCorrectedForVertMotion" << endl;
      cerr << "  No georef information found" << endl;
      cerr << "  Correction will not be applied" << endl;
    }
    return;
  }

  // pre-compute sin / cosine

  double cosEl, sinEl;
  ta_sincos(ray->getElevationDeg() * Radx::DegToRad, &sinEl, &cosEl);

  double cosPitch, sinPitch;
  ta_sincos(georef->getPitch() * Radx::DegToRad, &sinPitch, &cosPitch);
  
  double cosRoll, sinRoll;
  ta_sincos(georef->getRoll() * Radx::DegToRad, &sinRoll, &cosRoll);
  
  double cosTilt, sinTilt;
  ta_sincos(georef->getTilt() * Radx::DegToRad, &sinTilt, &cosTilt);
  
  double cosDrift, sinDrift;
  ta_sincos(georef->getDrift() * Radx::DegToRad, &sinDrift, &cosDrift);
  
  double cosRotRoll, sinRotRoll;
  double rotPlusRoll = georef->getRotation() + georef->getRoll();
  ta_sincos(rotPlusRoll * Radx::DegToRad, &sinRotRoll, &cosRotRoll);

  // compute the vel correction from horiz platform motion, including drift
  // Compute y_t following equation 9 Lee et al (1994)
  
  double y_subt = ((-cosRotRoll * cosDrift * cosTilt * sinPitch) +
                   (sinDrift * sinRotRoll * cosTilt) +
                   (cosPitch * cosDrift * sinTilt));

  // Compute z following equation 9 Lee et al (1994)

  double zz = cosPitch * cosTilt * cosRotRoll + sinPitch * sinTilt;
  
  // Compute ground speed based on Eq 27 from Lee et al (1994)

  double ewVel = georef->getEwVelocity();
  double nsVel = georef->getNsVelocity();
  double ground_speed = sqrt(ewVel * ewVel + nsVel * nsVel);

  // compute the vert vel correction

  double vertCorr = 0.0;
  double vertVel = georef->getVertVelocity();
  if (vertVel > -9990) {
    vertCorr = vertVel * zz;
  }

  // compute the horiz vel correction

  double horizCorr = ground_speed * y_subt;

  // check
  
  if (vertCorr == 0.0 && horizCorr == 0.0) {
    // no change needed
    return;
  }

  if (velShort != nullptr) {

    Radx::fl32 missShort = velShort->getMissingFl32();
    Radx::fl32 *dataShort = velShort->getDataFl32();
    
    for (size_t ii = 0; ii < ray->getNGates(); ii++) {
      
      double valShort = dataShort[ii];
      if (valShort != missShort) {
        double shortCorrected = _correctForNyquist(valShort + vertCorr, _nyquistShort);
        dataShort[ii] = shortCorrected;
      }
      
    } // ii
    
  } // if (velShort != nullptr)
  
  if (velLong != nullptr) {

    Radx::fl32 missLong = velLong->getMissingFl32();
    Radx::fl32 *dataLong = velLong->getDataFl32();
    
    for (size_t ii = 0; ii < ray->getNGates(); ii++) {
      
      double valLong = dataLong[ii];
      if (valLong != missLong) {
        double longCorrected = _correctForNyquist(valLong + vertCorr, _nyquistLong);
        dataLong[ii] = longCorrected;
      }
      
    } // ii
    
  } // if (velLong != nullptr)
  
  if (velUnfolded != nullptr) {

    Radx::fl32 missUnfolded = velUnfolded->getMissingFl32();
    Radx::fl32 *dataUnfolded = velUnfolded->getDataFl32();
    
    for (size_t ii = 0; ii < ray->getNGates(); ii++) {
      
      double valUnfolded = dataUnfolded[ii];
      if (valUnfolded != missUnfolded) {
        double unfoldedCorrected = _correctForNyquist(valUnfolded + vertCorr, _nyquistUnfolded);
        dataUnfolded[ii] = unfoldedCorrected;
      }
      
    } // ii
    
  } // if (velUnfolded != nullptr)
  
}

/////////////////////////////////////////////////
// correct velocity for nyquist
  
double HcrMomentsCombine::_correctForNyquist(double vel, double nyquist)

{
  while (vel > nyquist) {
    vel -= 2.0 * nyquist;
  }
  while (vel < -nyquist) {
    vel += 2.0 * nyquist;
  }
  return vel;
}

/////////////////////////////////////////////////////////////////
// Read a short ray
// Creates ray, must be freed by caller.

RadxRay *HcrMomentsCombine::_readRayNext()
{

  // read next ray
  
  RadxRay *ray = _momReader->readNextRay();
  if (ray == nullptr) {
    return nullptr;
  }
  _nRaysRead++;
  
  // check for platform update
  
  if (_momReader->getPlatformUpdated()) {

    RadxPlatform platform = _momReader->getPlatform();
    _platformShort = platform;
    _wavelengthM = _platformShort.getWavelengthM();
    if (_wavelengthM < 0) {
      _wavelengthM = 0.003176;
    }
    _prtShort = ray->getPrtSec();
    _nyquistShort = ((_wavelengthM / _prtShort) / 4.0);
    if (_params.fixed_location_mode) {
      _platformShort.setLatitudeDeg(_params.fixed_radar_location.latitudeDeg);
      _platformShort.setLongitudeDeg(_params.fixed_radar_location.longitudeDeg);
      _platformShort.setAltitudeKm(_params.fixed_radar_location.altitudeKm);
    }
    
    // create message
    RadxMsg msg;
    platform.serialize(msg);
    // write the platform to the output queue
    if (_outputFmq) {
      if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                               msg.assembledMsg(), msg.lengthAssembled())) {
        cerr << "ERROR - HcrMomentsCombine::_readRayNext" << endl;
        cerr << "  Cannot write platform to queue" << endl;
      }
    }

  } // if (_momReader->getPlatformUpdated())

  // check for calibration update
  
  if (_momReader->getRcalibUpdated()) {
    const vector<RadxRcalib> &calibs = _momReader->getRcalibs();
    _calibsShort = calibs;
    for (size_t ii = 0; ii < calibs.size(); ii++) {
      // create message
      RadxRcalib calib = calibs[ii];
      RadxMsg msg;
      calib.serialize(msg);
      // write to output queue
      if (_outputFmq) {
        if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                                 msg.assembledMsg(), msg.lengthAssembled())) {
          cerr << "ERROR - HcrMomentsCombine::_readRayNext" << endl;
          cerr << "  Cannot write calib to queue" << endl;
        }
      }
    } // ii
  }

  // check for status xml update
  
  if (_momReader->getStatusXmlUpdated()) {
    const string statusXml = _momReader->getStatusXml();
    _statusXmlShort = statusXml;
    // create RadxStatusXml object
    RadxStatusXml status;
    status.setXmlStr(statusXml);
    // create message
    RadxMsg msg;
    status.serialize(msg);
    // write to output queue
    if (_outputFmq) {
      if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                               msg.assembledMsg(), msg.lengthAssembled())) {
        cerr << "ERROR - HcrMomentsCombine::_readRayNext" << endl;
        cerr << "  Cannot write status xml to queue" << endl;
      }
    }
  }
  
  // update events
  
  _eventsShort = _momReader->getEvents();
  for (size_t ii = 0; ii < _eventsShort.size(); ii++) {
    RadxEvent event = _eventsShort[ii];
    RadxMsg msg;
    event.serialize(msg);
    // write to output queue
    if (_outputFmq) {
      if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                               msg.assembledMsg(), msg.lengthAssembled())) {
        cerr << "ERROR - HcrMomentsCombine::_readRayNext" << endl;
        cerr << "  Cannot write start of vol event to queue" << endl;
      }
    }
  } // ii

  // override location as required

  if (_params.fixed_location_mode) {
    RadxGeoref *georef = ray->getGeoreference();
    if (georef != nullptr) {
      georef->setLatitude(_params.fixed_radar_location.latitudeDeg);
      georef->setLongitude(_params.fixed_radar_location.longitudeDeg);
      georef->setAltitudeKmMsl(_params.fixed_radar_location.altitudeKm);
      georef->setEwVelocity(0.0);
      georef->setNsVelocity(0.0);
      georef->setVertVelocity(0.0);
      georef->setHeading(0.0);
      georef->setTrack(0.0);
      georef->setEwWind(0.0);
      georef->setNsWind(0.0);
      georef->setVertWind(0.0);
    }
  }
  
  return ray;

}

/////////////////////////////////////////////////////////////////
// Read a long ray
// Creates ray, must be freed by caller.

RadxRay *HcrMomentsCombine::_readRayLong()
{

  // read next ray
  
  RadxRay *rayLong = _momReader->readNextRay();
  if (rayLong == nullptr) {
    return nullptr;
  }
  _nRaysRead++;

  // check for platform update
  
  if (_momReader->getPlatformUpdated()) {

    const RadxPlatform &platform = _momReader->getPlatform();
    _platformLong = platform;
    _prtLong = rayLong->getPrtSec();
    _nyquistLong = ((_wavelengthM / _prtLong) / 4.0);
    if (_params.fixed_location_mode) {
      _platformLong.setLatitudeDeg(_params.fixed_radar_location.latitudeDeg);
      _platformLong.setLongitudeDeg(_params.fixed_radar_location.longitudeDeg);
      _platformLong.setAltitudeKm(_params.fixed_radar_location.altitudeKm);
    }

    double prtRatio = _prtShort / _prtLong;
    int ratio60 = (int) (prtRatio * 60.0 + 0.5);
    if (ratio60 == 40) {
      // 2/3
      _stagM = 2;
      _stagN = 3;
    } else if (ratio60 == 45) {
      // 3/4
      _stagM = 3;
      _stagN = 4;
    } else if (ratio60 == 48) {
      // 4/5
      _stagM = 4;
      _stagN = 5;
    } else {
      // assume 2/3
      cerr << "WARNING - HcrMomentsCombine::_readRayLong" << endl;
      cerr << "  No support for prtRatio: " << prtRatio << endl;
      cerr << "  Assuming 2/3 stagger" << endl;
      _stagM = 2;
      _stagN = 3;
    }
  }

  // check for calibration update
  
  if (_momReader->getRcalibUpdated()) {
    const vector<RadxRcalib> &calibs = _momReader->getRcalibs();
    _calibsLong = calibs;
    for (size_t ii = 0; ii < calibs.size(); ii++) {
      // create message
      RadxRcalib calib = calibs[ii];
      RadxMsg msg;
      calib.serialize(msg);
      // write to output queue
      if (_outputFmq) {
        if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                                 msg.assembledMsg(), msg.lengthAssembled())) {
          cerr << "ERROR - HcrLongLongCombine::_readRayLong" << endl;
          cerr << "  Cannot write calib to queue" << endl;
        }
      }
    } // ii
  }

  if (_momReader->getRcalibUpdated()) {
    const vector<RadxRcalib> &calibs = _momReader->getRcalibs();
    _calibsLong = calibs;
  }

  // check for status xml update
  
  if (_momReader->getStatusXmlUpdated()) {
    const string statusXml = _momReader->getStatusXml();
    _statusXmlLong = statusXml;
  }

  // update events
  
  _eventsLong = _momReader->getEvents();

  // override location as required

  if (_params.fixed_location_mode) {
    RadxGeoref *georef = rayLong->getGeoreference();
    if (georef != nullptr) {
      georef->setLatitude(_params.fixed_radar_location.latitudeDeg);
      georef->setLongitude(_params.fixed_radar_location.longitudeDeg);
      georef->setAltitudeKmMsl(_params.fixed_radar_location.altitudeKm);
      georef->setEwVelocity(0.0);
      georef->setNsVelocity(0.0);
      georef->setVertVelocity(0.0);
      georef->setHeading(0.0);
      georef->setTrack(0.0);
      georef->setEwWind(0.0);
      georef->setNsWind(0.0);
      georef->setVertWind(0.0);
    }
  }
  
  return rayLong;

}

////////////////////////////////////////////////////////
// set dwell stats method from params

RadxField::StatsMethod_t
  HcrMomentsCombine::_getDwellStatsMethod(Params::dwell_stats_method_t method)
  
{

  switch (method) {

    case Params::DWELL_STATS_MEAN:
      return RadxField::STATS_METHOD_MEAN;
      break;
    case Params::DWELL_STATS_MEDIAN:
      return RadxField::STATS_METHOD_MEDIAN;
      break;
    case Params::DWELL_STATS_DISCRETE_MODE:
      return RadxField::STATS_METHOD_DISCRETE_MODE;
      break;
    case Params::DWELL_STATS_MAXIMUM:
      return RadxField::STATS_METHOD_MAXIMUM;
      break;
    case Params::DWELL_STATS_MINIMUM:
      return RadxField::STATS_METHOD_MINIMUM;
      break;
    case Params::DWELL_STATS_MIDDLE:
    default:
      return RadxField::STATS_METHOD_MIDDLE;

  }

}

///////////////////////////////////
// initialize dual PRT mode

void HcrMomentsCombine::_initDualPrt(double prtShort,
                                     double prtLong)

{
  
  _prtShort = prtShort;
  _prtLong = prtLong;
  
  double prtRatio = _prtShort / _prtLong;
  int ratio60 = (int) (prtRatio * 60.0 + 0.5);
  if (ratio60 == 40) {
    // 2/3
    _stagM = 2;
    _stagN = 3;
  } else if (ratio60 == 45) {
    // 3/4
    _stagM = 3;
    _stagN = 4;
  } else if (ratio60 == 48) {
    // 4/5
    _stagM = 4;
    _stagN = 5;
  } else {
    // assume 2/3
    cerr << "WARNING - HcrMomentsCombine::_initDualPrtq2Dsr" << endl;
    cerr << "  No support for prtRatio: " << prtRatio << endl;
    cerr << "  Assuming 2/3 stagger" << endl;
    _stagM = 2;
    _stagN = 3;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===>> staggered PRT, ratio: "
         << _stagM << "/" << _stagN << " <<===" << endl;
  }
  
  _nyquistShort = ((_wavelengthM / _prtShort) / 4.0);
  _nyquistLong = ((_wavelengthM / _prtLong) / 4.0);
  _nyquistUnfolded = _nyquistShort * _stagM;

}


