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
// HcrShortLongCombine.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2023
//
//////////////////////////////////////////////////////////////////////////
//
// Combines 100Hz HCR moments stream containing both long and short pulses,
// and optionally long and short PRTs. Groups the long and short pulses
// into dwells (normally 10Hz). We write out the individual fields
// (i.e. long and short) and combined fields.
// If both long and short PRT data are present, the velocity fields
// are unfolded into a final velocity field.
//
//////////////////////////////////////////////////////////////////////////

#include "HcrShortLongCombine.hh"
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

HcrShortLongCombine::HcrShortLongCombine(int argc, char **argv)
  
{

  OK = TRUE;
  _readerShort = NULL;
  _readerLong = NULL;
  _cacheRayShort = NULL;
  _cacheRayLong = NULL;
  _outputFmq = NULL;

  // init staggered prt
  
  _wavelengthM = 0.003176;
  _prtShort = 0.000101376;
  _prtLong = _prtShort * 1.5;
  _nyquistShort = ((_wavelengthM / _prtShort) / 4.0);
  _nyquistLong = ((_wavelengthM / _prtLong) / 4.0);
  _stagM = 2;
  _stagN = 3;

  // set programe name

  _progName = "HcrShortLongCombine";
  
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

HcrShortLongCombine::~HcrShortLongCombine()

{

  if (_readerShort) {
    delete _readerShort;
  }

  if (_readerLong) {
    delete _readerLong;
  }

  if (_outputFmq) {
    delete _outputFmq;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int HcrShortLongCombine::Run()
{

  int iret = 0;

  switch (_params.mode) {
    case Params::ARCHIVE:
      iret = _runArchive();
      break;
    case Params::REALTIME:
    default:
      iret = _runRealtime();
  } // switch

  return iret;

}

//////////////////////////////////////////////////
// Run in REALTIME mode

int HcrShortLongCombine::_runRealtime()
{

  // Open the output fmq

  if (_openOutputFmq()) {
    return -1;
  }

  // Instantiate and initialize the input radar queues

  if (_openInputFmqs()) {
    return -1;
  }

  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "====>> Unfold stagM, stagN: " << _stagM << ", " << _stagN << endl;
  }
  
  // prepare the input rays at the start of the first output dwell
  
  if (_prepareInputRays()) {
    return -1;
  }

  _nRaysRead = 0;
  _nRaysWritten = 0;

  // loop forever
  
  int iret = 0;
  while (true) {

    // read in next dwell for short and long
    
    PMU_auto_register("Reading FMQ realtime");
    if (_readNextDwell()) {
      return -1;
    }

    // combine short and long

    RadxRay *rayCombined = _combineDwellRays();
    
    if (rayCombined != NULL) {

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
      
      if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                               msg.assembledMsg(), msg.lengthAssembled())) {
        cerr << "ERROR - HcrShortLongCombine::_runRealtime" << endl;
        cerr << "  Cannot write ray to output queue" << endl;
        iret = -1;
      }
      _nRaysWritten++;
      
      // free up memory
      
      delete rayCombined;

    } else {

      cerr << "ERROR - HcrShortLongCombine::_runRealtime" << endl;
      cerr << "  no combined ray created" << endl;
      iret = -1;
      
    }
    
  } // while (true)

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int HcrShortLongCombine::_runArchive()
{

  // Open the output fmq

  if (_openOutputFmq()) {
    return -1;
  }

  // Instantiate and initialize the input radar queues

  if (_openFileReaders()) {
    return -1;
  }
  
  // prepare the input rays at the start of the first output dwell
  
  if (_prepareInputRays()) {
    return -1;
  }

  _nRaysRead = 0;
  _nRaysWritten = 0;

  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "====>> Unfold stagM, stagN: " << _stagM << ", " << _stagN << endl;
  }
  
  // loop until readers are empty
  
  int iret = 0;
  while (true) {
    
    // read in next dwell for short and long
    
    if (_readNextDwell()) {
      return -1;
    }

    // combine short and long

    RadxRay *rayCombined = _combineDwellRays();
    
    if (rayCombined != NULL) {

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
      
      if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                               msg.assembledMsg(), msg.lengthAssembled())) {
        cerr << "ERROR - HcrShortLongCombine::_runRealtime" << endl;
        cerr << "  Cannot write ray to output queue" << endl;
        iret = -1;
      }
      _nRaysWritten++;
      
      // free up memory
      
      delete rayCombined;

    } else {

      cerr << "ERROR - HcrShortLongCombine::_runRealtime" << endl;
      cerr << "  no combined ray created" << endl;
      iret = -1;
      
    }
    
  } // while (true)

  return iret;

}

//////////////////////////////////////////////////
// Open input fmqs

int HcrShortLongCombine::_openInputFmqs()
{

  // Instantiate and initialize the input radar queues

  if (_params.debug) {
    cerr << "DEBUG - opening input fmq for short pulse: "
         << _params.input_fmq_url_short << endl;
    cerr << "DEBUG - opening input fmq for long  pulse: "
         << _params.input_fmq_url_long << endl;
  }
  
  _readerShort = new IwrfMomReaderFmq(_params.input_fmq_url_short);
  _readerLong = new IwrfMomReaderFmq(_params.input_fmq_url_long);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _readerShort->setDebug(IWRF_DEBUG_NORM);
    _readerLong->setDebug(IWRF_DEBUG_NORM);
  }

  // initialize reader - read one ray

  RadxRay *rayShort = _readRayShort();
  if (rayShort != NULL) {
    delete rayShort;
  }
  RadxRay *rayLong = _readRayLong();
  if (rayLong != NULL) {
    delete rayLong;
  }

  if (_params.seek_to_end_of_input_fmq) {
    _readerShort->seekToEnd();
    _readerLong->seekToEnd();
  } else {
    _readerShort->seekToStart();
    _readerLong->seekToStart();
  }

  return 0;

}

//////////////////////////////////////////////////
// Open output fmq

int HcrShortLongCombine::_openOutputFmq()
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
  _outputFmq->setSingleWriter();

  return 0;

}

//////////////////////////////////////////////////
// Open readers from CfRadial files

int HcrShortLongCombine::_openFileReaders()
{

  // Instantiate and initialize the input radar queues

  if (_params.debug) {
    cerr << "DEBUG - opening input dir for short pulse: "
         << _params.input_dir_short << endl;
    cerr << "DEBUG - opening input dir for long  pulse: "
         << _params.input_dir_long << endl;
  }

  RadxTime startTime(_args.startTime);
  RadxTime endTime(_args.endTime);
  
  _readerShort = new IwrfMomReaderFile(_params.input_dir_short, startTime, endTime);
  _readerLong = new IwrfMomReaderFile(_params.input_dir_long, startTime, endTime);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _readerShort->setDebug(IWRF_DEBUG_NORM);
    _readerLong->setDebug(IWRF_DEBUG_NORM);
  }

  // initialize reader - read one ray
  
  RadxRay *rayShort = _readRayShort();
  if (rayShort == NULL) {
    cerr << "ERROR - HcrShortLongCombine::_openFileReaders()" << endl;
    cerr << "  Cannot read rays from short pulse dir: " << _params.input_dir_short << endl;
    cerr << "  Start time: " << startTime.asString(0) << endl;
    cerr << "  End time: " << endTime.asString(0) << endl;
  }
  
  RadxRay *rayLong = _readRayLong();
  if (rayLong == NULL) {
    cerr << "ERROR - HcrShortLongCombine::_openFileReaders()" << endl;
    cerr << "  Cannot read rays from long pulse dir: " << _params.input_dir_long << endl;
    cerr << "  Start time: " << startTime.asString(0) << endl;
    cerr << "  End time: " << endTime.asString(0) << endl;
  }

  return 0;

}

/////////////////////////////////////////////////////////////////
// Initialize the input rays at the start of the first output dwell

int HcrShortLongCombine::_prepareInputRays()
{

  // read a short and long ray

  RadxTime shortTime;
  RadxTime longTime;

  {
    
    RadxRay *rayShort = _readRayShort();
    if (rayShort == NULL) {
      cerr << "ERROR - HcrShortLongCombine::_prepareInputRays()" << endl;
      if (_params.mode == Params::REALTIME) {
        cerr << "  Cannot read input fmq short: " << _params.input_fmq_url_short << endl;
      } else {
        cerr << "  Cannot read input dir short: " << _params.input_dir_short << endl;
      }
      return -1;
    }
    
    RadxRay *longRay = _readRayLong();
    if (longRay == NULL) {
      cerr << "ERROR - HcrShortLongCombine::_prepareInputRays()" << endl;
      if (_params.mode == Params::REALTIME) {
        cerr << "  Cannot read input fmq long: " << _params.input_fmq_url_long << endl;
      } else {
        cerr << "  Cannot read input dir long: " << _params.input_dir_long << endl;
      }
      delete rayShort;
      return -1;
    }
    
    shortTime = rayShort->getRadxTime();
    longTime = longRay->getRadxTime();
    
    delete rayShort;
    delete longRay;

  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=======>> first short ray read, time: " << shortTime.asString(6) << endl;
    cerr << "=======>> first long  ray read, time: " << longTime.asString(6) << endl;
  }
  
  // compute the latest time
  
  RadxTime latestTime = shortTime;
  if (longTime > latestTime) {
    latestTime = longTime;
  }
  double latestSecs = latestTime.asDouble();
  
  // compute the next dwell limits
  
  double dwellMidSecs = (floor(latestSecs / _dwellLengthSecs) + 1.0) * _dwellLengthSecs;
  
  _dwellMidTime.setFromDouble(dwellMidSecs);
  _dwellStartTime = _dwellMidTime - _dwellLengthSecsHalf;
  _dwellEndTime = _dwellMidTime + _dwellLengthSecsHalf;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> dwellStartTime: " << _dwellStartTime.asString(6) << endl;
    cerr << "====>> dwellMidTime  : " << _dwellMidTime.asString(6) << endl;
    cerr << "====>> dwellEndTime  : " << _dwellEndTime.asString(6) << endl;
  }

  // read short rays, prepare for first dwell
  
  _cacheRayShort = NULL;
  while (true) {
    RadxRay *ray = _readRayShort();
    if (ray == NULL) {
      cerr << "========>> short queue done <<==========" << endl;
      return -1;
    }
    if (ray->getRadxTime() >= _dwellStartTime) {
      // save for next dwell
      _cacheRayShort = ray;
      break;
    } else {
      // read ahead
      delete ray;
    }
  }

  // read long rays, prepare for first dwell
  
  _cacheRayLong = NULL;
  while (true) {
    RadxRay *ray = _readRayLong();
    if (ray == NULL) {
      cerr << "========>> long queue done <<==========" << endl;
      return -1;
    }
    if (ray->getRadxTime() >= _dwellStartTime) {
      // save for next dwell
      _cacheRayLong = ray;
      break;
    } else {
      // read ahead
      delete ray;
    }
  }

  return 0;

}

/////////////////////////////////////////////////////////////////
// Read in rays for next dwell

int HcrShortLongCombine::_readNextDwell()
{

  // clear the dwell vectors

  _clearDwellRays();
  
  // add in the cached rays already read in

  if (_cacheRayShort != NULL) {
    _dwellRaysShort.push_back(_cacheRayShort);
    _cacheRayShort = NULL;
  }
  if (_cacheRayLong != NULL) {
    _dwellRaysLong.push_back(_cacheRayLong);
    _cacheRayLong = NULL;
  }

  // read in short rays for the dwell
  
  while (true) {
    RadxRay *ray = _readRayShort();
    if (ray == NULL) {
      cerr << "ERROR - HcrShortLongCombine::_readNextDwell()" << endl;
      if (_params.mode == Params::REALTIME) {
        cerr << "  Cannot read input fmq short: " << _params.input_fmq_url_short << endl;
      } else {
        cerr << "  Cannot read input dir short: " << _params.input_dir_short << endl;
      }
      return -1;
    }
    if (ray->getRadxTime() >= _dwellEndTime) {
      // save for start of next dwell
      _cacheRayShort = ray;
      break;
    } else {
      _dwellRaysShort.push_back(ray);
    }
  }

  // read in long rays for the dwell
  
  while (true) {
    RadxRay *ray = _readRayLong();
    if (ray == NULL) {
      cerr << "ERROR - HcrShortLongCombine::_readNextDwell()" << endl;
      if (_params.mode == Params::REALTIME) {
        cerr << "  Cannot read input fmq long: " << _params.input_fmq_url_long << endl;
      } else {
        cerr << "  Cannot read input dir long: " << _params.input_dir_long << endl;
      }
      return -1;
    }
    if (ray->getRadxTime() >= _dwellEndTime) {
      // save for start of next dwell
      _cacheRayLong = ray;
      break;
    } else {
      _dwellRaysLong.push_back(ray);
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=================>> short ray count: " << _dwellRaysShort.size() << endl;
    for (size_t ii = 0; ii < _dwellRaysShort.size(); ii++) {
      cerr << "  short ray time: "
           << _dwellRaysShort[ii]->getRadxTime().asString(6) << ", "
           <<  _dwellRaysShort[ii] << endl;
    }
    cerr << "========================================" << endl;
    cerr << "=================>> long ray count: " << _dwellRaysLong.size() << endl;
    for (size_t ii = 0; ii < _dwellRaysLong.size(); ii++) {
      cerr << "  long ray time: "
           << _dwellRaysLong[ii]->getRadxTime().asString(6) << ", "
           <<  _dwellRaysLong[ii] << endl;
    }
    cerr << "========================================" << endl;
  }
  
  // set to advance to next dwell
  
  _dwellStartTime = _dwellEndTime;
  _dwellMidTime = _dwellStartTime + _dwellLengthSecsHalf;
  _dwellEndTime = _dwellStartTime + _dwellLengthSecs;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> dwellStartTime: " << _dwellStartTime.asString(6) << endl;
    cerr << "====>> dwellMidTime  : " << _dwellMidTime.asString(6) << endl;
    cerr << "====>> dwellEndTime  : " << _dwellEndTime.asString(6) << endl;
  }

  return 0;

}

/////////////////////////////////////////////////////////////////
// combine dwell rays
// returns pointer to combined ray - this must be freed by caller.

RadxRay *HcrShortLongCombine::_combineDwellRays()

{

  // short rays
  // sanity check
  
  size_t nRaysShort = _dwellRaysShort.size();
  if (nRaysShort < 1) {
    return NULL;
  }

  // add short rays to vol
  
  _dwellVolShort.clear();
  for (size_t iray = 0; iray < nRaysShort; iray++) {
    _dwellVolShort.addRay(_dwellRaysShort[iray]);
  }

  // ownership of rays passed to vol, which will free them

  _dwellRaysShort.clear();

  // combine short rays into a single ray

  RadxRay *rayCombined = _dwellVolShort.computeFieldStats(_globalMethod,
                                                          _namedMethods);
  
  // long rays
  // sanity check
  
  size_t nRaysLong = _dwellRaysLong.size();
  if (nRaysLong < 1) {
    delete rayCombined;
    return NULL;
  }

  // add long rays to vol
  
  _dwellVolLong.clear();
  for (size_t iray = 0; iray < nRaysLong; iray++) {
    _dwellVolLong.addRay(_dwellRaysLong[iray]);
  }
  
  // ownership of rays passed to vol, which will free them

  _dwellRaysLong.clear();

  // combine long rays into a single ray
  
  RadxRay *rayLong = _dwellVolLong.computeFieldStats(_globalMethod, _namedMethods);
  
  // rename short fields

  vector<RadxField *> fieldsShort = rayCombined->getFields();
  for (size_t ifield = 0; ifield < fieldsShort.size(); ifield++) {
    RadxField *fld = fieldsShort[ifield];
    string newName = fld->getName() + _params.suffix_to_add_for_short_pulse_fields;
    fld->setName(newName);
  }

  // add long fields to short ray

  vector<RadxField *> fieldsLong = rayLong->getFields();
  for (size_t ifield = 0; ifield < fieldsLong.size(); ifield++) {
    RadxField *fld = new RadxField(*fieldsLong[ifield]);
    string newName = fld->getName() + _params.suffix_to_add_for_long_pulse_fields;
    fld->setName(newName);
    rayCombined->addField(fld);
  }
  delete rayLong;

  // unfold the velocity, add unfolded field to ray

  _unfoldVel(rayCombined);

  // free up memory

  _dwellVolShort.clear();
  _dwellVolLong.clear();

  // set sweep mode if required
  
  if (_params.override_sweep_mode) {
    rayCombined->setSweepMode((Radx::SweepMode_t) _params.sweep_mode);
  }
  
  // return combined ray
  
  return rayCombined;

}

/////////////////////////////////////////////////////////////////
// clear the dwell rays

void HcrShortLongCombine::_clearDwellRays()
{

  for (size_t ii = 0; ii < _dwellRaysShort.size(); ii++) {
    delete _dwellRaysShort[ii];
  }
  _dwellRaysShort.clear();

  for (size_t ii = 0; ii < _dwellRaysLong.size(); ii++) {
    delete _dwellRaysLong[ii];
  }
  _dwellRaysLong.clear();

}

////////////////////////////////////////////////////////////////
// unfold the velocity, add unfolded field to ray

void HcrShortLongCombine::_unfoldVel(RadxRay *rayCombined)

{

  string velShortName = _params.input_vel_field_name;
  velShortName += _params.suffix_to_add_for_short_pulse_fields;
  string velLongName = _params.input_vel_field_name;
  velLongName += _params.suffix_to_add_for_long_pulse_fields;

  RadxField *velShort = rayCombined->getField(velShortName);
  RadxField *velLong = rayCombined->getField(velLongName);

  if (velShort == NULL || velLong == NULL) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - HcrShortLongCombine::_unfoldVel()" << endl;
      cerr << "Cannot find velocity fields to unfold." << endl;
    }
    return;
  }
  
  velShort->convertToFl32();
  velLong->convertToFl32();

  RadxField *velUnfold = new RadxField(*velShort);
  velUnfold->setName(_params.vel_unfolded_field_name);

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
  Radx::fl32 *dataShort = velShort->getDataFl32();
  Radx::fl32 *dataLong = velLong->getDataFl32();
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
  rayCombined->setNyquistMps(_nyquistUnfolded);
  _computeVelCorrectedForVertMotion(rayCombined, velShort, velLong, velUnfold);

  // add field to ray
  
  rayCombined->addField(velUnfold);
  rayCombined->setNyquistMps(rayCombined->getNyquistMps() * _LL);

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

void HcrShortLongCombine::_computeVelCorrectedForVertMotion(RadxRay *ray,
                                                            RadxField *velShort,
                                                            RadxField *velLong,
                                                            RadxField *velUnfolded)
  
{

  // no good if no georeference available
  
  const RadxGeoref *georef = ray->getGeoreference();
  if (georef == NULL) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - _computeVelocityCorrectedForMotion" << endl;
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

  Radx::fl32 missShort = velShort->getMissingFl32();
  Radx::fl32 missLong = velLong->getMissingFl32();
  Radx::fl32 missUnfolded = velUnfolded->getMissingFl32();
  
  Radx::fl32 *dataShort = velShort->getDataFl32();
  Radx::fl32 *dataLong = velLong->getDataFl32();
  Radx::fl32 *dataUnfolded = velUnfolded->getDataFl32();
  
  for (size_t ii = 0; ii < ray->getNGates(); ii++) {
    
    double valShort = dataShort[ii];
    if (valShort != missShort) {
      double shortCorrected = _correctForNyquist(valShort + vertCorr, _nyquistShort);
      dataShort[ii] = shortCorrected;
    }
    
    double valLong = dataLong[ii];
    if (valLong != missLong) {
      double longCorrected = _correctForNyquist(valLong + vertCorr, _nyquistLong);
      dataLong[ii] = longCorrected;
    }
    
    double valUnfolded = dataUnfolded[ii];
    if (valUnfolded != missUnfolded) {
      double unfoldedCorrected = _correctForNyquist(valUnfolded + vertCorr, _nyquistUnfolded);
      dataUnfolded[ii] = unfoldedCorrected;
    }
    
  } // ii

}

/////////////////////////////////////////////////
// correct velocity for nyquist
  
double HcrShortLongCombine::_correctForNyquist(double vel, double nyquist)

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

RadxRay *HcrShortLongCombine::_readRayShort()
{

  // read next ray
  
  RadxRay *rayShort = _readerShort->readNextRay();
  if (rayShort == NULL) {
    return NULL;
  }
  _nRaysRead++;
  
  // check for platform update
  
  if (_readerShort->getPlatformUpdated()) {
    RadxPlatform platform = _readerShort->getPlatform();
    _platformShort = platform;
    _setPlatformMetadata(_platformShort);
    _wavelengthM = _platformShort.getWavelengthM();
    if (_wavelengthM < 0) {
      _wavelengthM = 0.003176;
    }
    _prtShort = rayShort->getPrtSec();
    _nyquistShort = ((_wavelengthM / _prtShort) / 4.0);
    
    // create message
    RadxMsg msg;
    platform.serialize(msg);
    // write the platform to the output queue
    if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                             msg.assembledMsg(), msg.lengthAssembled())) {
      cerr << "ERROR - HcrShortLongCombine::_readRayShort" << endl;
      cerr << "  Cannot write platform to queue" << endl;
    }

  } // if (_readerShort->getPlatformUpdated())

  // check for calibration update
  
  if (_readerShort->getRcalibUpdated()) {
    const vector<RadxRcalib> &calibs = _readerShort->getRcalibs();
    _calibsShort = calibs;
    for (size_t ii = 0; ii < calibs.size(); ii++) {
      // create message
      RadxRcalib calib = calibs[ii];
      RadxMsg msg;
      calib.serialize(msg);
      // write to output queue
      if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                               msg.assembledMsg(), msg.lengthAssembled())) {
        cerr << "ERROR - HcrShortLongCombine::_readRayShort" << endl;
        cerr << "  Cannot write calib to queue" << endl;
      }
    } // ii
  }

  // check for status xml update
  
  if (_readerShort->getStatusXmlUpdated()) {
    const string statusXml = _readerShort->getStatusXml();
    _statusXmlShort = statusXml;
    // create RadxStatusXml object
    RadxStatusXml status;
    status.setXmlStr(statusXml);
    // create message
    RadxMsg msg;
    status.serialize(msg);
    // write to output queue
    if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                             msg.assembledMsg(), msg.lengthAssembled())) {
      cerr << "ERROR - HcrShortLongCombine::_readRayShort" << endl;
      cerr << "  Cannot write status xml to queue" << endl;
    }
  }
  
  // update events
  
  _eventsShort = _readerShort->getEvents();
  for (size_t ii = 0; ii < _eventsShort.size(); ii++) {
    RadxEvent event = _eventsShort[ii];
    RadxMsg msg;
    event.serialize(msg);
    // write to output queue
    if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                             msg.assembledMsg(), msg.lengthAssembled())) {
      cerr << "ERROR - HcrShortLongCombine::_readRayShort" << endl;
      cerr << "  Cannot write start of vol event to queue" << endl;
    }
  } // ii

  return rayShort;

}

/////////////////////////////////////////////////////////////////
// Read a long ray
// Creates ray, must be freed by caller.

RadxRay *HcrShortLongCombine::_readRayLong()
{

  // read next ray
  
  RadxRay *rayLong = _readerLong->readNextRay();
  if (rayLong == NULL) {
    return NULL;
  }
  _nRaysRead++;

  // check for platform update
  
  if (_readerLong->getPlatformUpdated()) {
    const RadxPlatform &platform = _readerLong->getPlatform();
    _platformLong = platform;
    _setPlatformMetadata(_platformLong);
    _prtLong = rayLong->getPrtSec();
    _nyquistLong = ((_wavelengthM / _prtLong) / 4.0);
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
      cerr << "WARNING - HcrShortLongCombine::_readRayLong" << endl;
      cerr << "  No support for prtRatio: " << prtRatio << endl;
      cerr << "  Assuming 2/3 stagger" << endl;
      _stagM = 2;
      _stagN = 3;
    }
  }

  // check for calibration update
  
  if (_readerLong->getRcalibUpdated()) {
    const vector<RadxRcalib> &calibs = _readerLong->getRcalibs();
    _calibsLong = calibs;
    for (size_t ii = 0; ii < calibs.size(); ii++) {
      // create message
      RadxRcalib calib = calibs[ii];
      RadxMsg msg;
      calib.serialize(msg);
      // write to output queue
      if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                               msg.assembledMsg(), msg.lengthAssembled())) {
        cerr << "ERROR - HcrLongLongCombine::_readRayLong" << endl;
        cerr << "  Cannot write calib to queue" << endl;
      }
    } // ii
  }

  if (_readerLong->getRcalibUpdated()) {
    const vector<RadxRcalib> &calibs = _readerLong->getRcalibs();
    _calibsLong = calibs;
  }

  // check for status xml update
  
  if (_readerLong->getStatusXmlUpdated()) {
    const string statusXml = _readerLong->getStatusXml();
    _statusXmlLong = statusXml;
  }

  // update events
  
  _eventsLong = _readerLong->getEvents();

  return rayLong;

}

////////////////////////////////////////////////////////
// set dwell stats method from params

RadxField::StatsMethod_t
  HcrShortLongCombine::_getDwellStatsMethod(Params::dwell_stats_method_t method)
  
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

////////////////////////////////////////////////////////
// modify platform metadata

void HcrShortLongCombine::_setPlatformMetadata(RadxPlatform &platform)
  
{

  if (_params.override_platform_type) {
    platform.setPlatformType((Radx::PlatformType_t) _params.platform_type);
  }

  if (_params.override_primary_axis) {
    platform.setPrimaryAxis((Radx::PrimaryAxis_t) _params.primary_axis);
  }

}


