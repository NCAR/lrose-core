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
using namespace std;

// Constructor

HcrShortLongCombine::HcrShortLongCombine(int argc, char **argv)
  
{

  OK = TRUE;
  _readerFmqShort = NULL;
  _readerFmqLong = NULL;
  _cacheRayShort = NULL;
  _cacheRayLong = NULL;
  _outputFmq = NULL;

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

  if (_readerFmqShort) {
    delete _readerFmqShort;
  }

  if (_readerFmqLong) {
    delete _readerFmqLong;
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

  // if we are writing out on time boundaries, there
  // may be unwritten data, so write it now

  if (_params.write_output_files_on_time_boundaries) {
    if (_writeSplitVol()) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in REALTIME mode

int HcrShortLongCombine::_runRealtime()
{

  // Instantiate and initialize the input radar queues

  if (_openFmqs()) {
    return -1;
  }

  // prepare the input fmqs at the start of the first output dwell
  
  if (_prepareInputFmqs()) {
    return -1;
  }

  _nRaysRead = 0;
  _nRaysWritten = 0;

  while (true) {
    PMU_auto_register("Reading FMQ realtime");
    if (_readNextDwellFromFmq()) {
      return -1;
    }
    _combineDwellRays();
  }

  return 0;

}

//////////////////////////////////////////////////
// Open fmqs

int HcrShortLongCombine::_openFmqs()
{

  // Instantiate and initialize the input radar queues

  if (_params.debug) {
    cerr << "DEBUG - opening input fmq for short pulse: "
         << _params.input_fmq_url_short << endl;
    cerr << "DEBUG - opening input fmq for long  pulse: "
         << _params.input_fmq_url_long << endl;
  }
  
  _readerFmqShort = new IwrfMomReaderFmq(_params.input_fmq_url_short);
  _readerFmqLong = new IwrfMomReaderFmq(_params.input_fmq_url_long);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _readerFmqShort->setDebug(IWRF_DEBUG_NORM);
    _readerFmqLong->setDebug(IWRF_DEBUG_NORM);
  }

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
    _readerFmqShort->seekToEnd();
    _readerFmqLong->seekToEnd();
  } else {
    _readerFmqShort->seekToStart();
    _readerFmqLong->seekToStart();
  }

  return 0;

}

/////////////////////////////////////////////////////////////////
// Initialize the input fmqs at the start of the first output dwell

int HcrShortLongCombine::_prepareInputFmqs()
{

  // read a short and long ray

  RadxTime shortTime;
  RadxTime longTime;

  {
    
    RadxRay *rayShort = _readRayShort();
    if (rayShort == NULL) {
      cerr << "ERROR - HcrShortLongCombine::_prepareInputFmqs()" << endl;
      cerr << "  Cannot read input fmq short: " << _params.input_fmq_url_short << endl;
      return -1;
    }
    
    RadxRay *longRay = _readRayLong();
    if (longRay == NULL) {
      cerr << "ERROR - HcrShortLongCombine::_prepareInputFmqs()" << endl;
      cerr << "  Cannot read input fmq long: " << _params.input_fmq_url_long << endl;
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

int HcrShortLongCombine::_readNextDwellFromFmq()
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
      cerr << "ERROR - HcrShortLongCombine::_readNextDwellFromFmq()" << endl;
      cerr << "  Cannot read input fmq short: " << _params.input_fmq_url_short << endl;
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
      cerr << "ERROR - HcrShortLongCombine::_readNextDwellFromFmq()" << endl;
      cerr << "  Cannot read input fmq long: " << _params.input_fmq_url_long << endl;
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

int HcrShortLongCombine::_combineDwellRays()

{

  // short rays
  // sanity check
  
  size_t nRaysShort = _dwellRaysShort.size();
  if (nRaysShort < 1) {
    return -1;
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
    return -1;
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
    string newName = fld->getName() + "_short";
    fld->setName(newName);
  }

  // add long fields to short ray

  vector<RadxField *> fieldsLong = rayLong->getFields();
  for (size_t ifield = 0; ifield < fieldsLong.size(); ifield++) {
    RadxField *fld = new RadxField(*fieldsLong[ifield]);
    string newName = fld->getName() + "_long";
    fld->setName(newName);
    rayCombined->addField(fld);
  }

  // unfold the velocity, add unfolded field to ray

  _unfoldVel(rayCombined);

  // create message from combined ray

  RadxMsg msg;
  rayCombined->serialize(msg);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=========== Writing out ray =============" << endl;
    cerr << "  time, el, az: "
         << rayCombined->getRadxTime().asString(3) << ", "
         << rayCombined->getElevationDeg() << ", "
         << rayCombined->getAzimuthDeg() << endl;
    cerr << "=========================================" << endl;
  }
  
  // write the message

  int iret = 0;
  if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - HcrShortLongCombine::_combineDwellRays" << endl;
    cerr << "  Cannot write ray to output queue" << endl;
    iret = -1;
  }
  _nRaysWritten++;
  
  // free up memory

  _dwellVolShort.clear();
  _dwellVolLong.clear();
  delete rayCombined;
  delete rayLong;
  
  return iret;
    
}

////////////////////////////////////////////////////////////////
// unfold the velocity, add unfolded field to ray

void HcrShortLongCombine::_unfoldVel(RadxRay *rayCombined)

{

  RadxField *velShort = rayCombined->getField("VEL_short");
  RadxField *velLong = rayCombined->getField("VEL_long");

  velShort->convertToFl32();
  velLong->convertToFl32();

  RadxField *velUnfold = new RadxField(*velShort);
  velUnfold->setName("VEL_unfold");

  // compute the unfolded velocity

  int _staggeredM = 2;
  int _staggeredN = 3;
  
  double _nyquistPrtShort = 7.8;
  double _nyquistPrtLong = 5.2;

    int _LL;
  int _PP_[32];
  int *_PP;
  
  _LL = (_staggeredM + _staggeredN - 1) / 2;
  if (_LL > 5) {
    _LL = 2; // set to 2/3
  }
  _PP = _PP_ + _LL;

  int cc = 0;
  int pp = 0;
  _PP[0] = 0;
  for (int ll = 1; ll <= _LL; ll++) {
    if ((ll / 2 * 2) == ll) {
      // even - va1 transition
      cc -= _staggeredN;
      pp++;
    } else {
      // odd - va2 transition
      cc += _staggeredM;
    }
    _PP[cc] = pp;
    _PP[-cc] = -pp;
  }

  size_t nGates = velUnfold->getNPoints();
  Radx::fl32 *dataShort = velShort->getDataFl32();
  Radx::fl32 *dataLong = velLong->getDataFl32();
  Radx::fl32 *dataUnfold = velUnfold->getDataFl32();
  double nyquistDiff = _nyquistPrtShort - _nyquistPrtLong;

  for (size_t ii = 0; ii < nGates; ii++) {
    
    double vel_diff = dataShort[ii] - dataLong[ii];
    double nyquistIntervalShort = (vel_diff / nyquistDiff) / 2.0;
    int ll = (int) floor(nyquistIntervalShort + 0.5);
    if (ll < -_LL) {
      ll = -_LL;
    } else if (ll > _LL) {
      ll = _LL;
    }
    double unfoldedVel = dataShort[ii] + _PP[ll] * _nyquistPrtShort * 2;
    dataUnfold[ii] = unfoldedVel;

  } // ii
  
  rayCombined->addField(velUnfold);

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

/////////////////////////////////////////////////////////////////
// Read a short ray
// Creates ray, must be freed by caller.

RadxRay *HcrShortLongCombine::_readRayShort()
{

  // read next ray
  
  RadxRay *rayShort = _readerFmqShort->readNextRay();
  _nRaysRead++;
  
  // check for platform update
  
  if (_readerFmqShort->getPlatformUpdated()) {
    RadxPlatform platform = _readerFmqShort->getPlatform();
    _platformShort = platform;
    // create message
    RadxMsg msg;
    platform.serialize(msg);
    // write the platform to the output queue
    if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                             msg.assembledMsg(), msg.lengthAssembled())) {
      cerr << "ERROR - HcrShortLongCombine::_readRayShort" << endl;
      cerr << "  Cannot write platform to queue" << endl;
    }
  }

  // check for calibration update
  
  if (_readerFmqShort->getRcalibUpdated()) {
    const vector<RadxRcalib> &calibs = _readerFmqShort->getRcalibs();
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
  
  if (_readerFmqShort->getStatusXmlUpdated()) {
    const string statusXml = _readerFmqShort->getStatusXml();
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
  
  _eventsShort = _readerFmqShort->getEvents();
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
  
  RadxRay *rayLong = _readerFmqLong->readNextRay();
  _nRaysRead++;

  // check for platform update
  
  if (_readerFmqLong->getPlatformUpdated()) {
    const RadxPlatform &platform = _readerFmqLong->getPlatform();
    _platformLong = platform;
  }

  // check for calibration update
  
  if (_readerFmqLong->getRcalibUpdated()) {
    const vector<RadxRcalib> &calibs = _readerFmqLong->getRcalibs();
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

  if (_readerFmqLong->getRcalibUpdated()) {
    const vector<RadxRcalib> &calibs = _readerFmqLong->getRcalibs();
    _calibsLong = calibs;
  }

  // check for status xml update
  
  if (_readerFmqLong->getStatusXmlUpdated()) {
    const string statusXml = _readerFmqLong->getStatusXml();
    _statusXmlLong = statusXml;
  }

  // update events
  
  _eventsLong = _readerFmqLong->getEvents();

  return rayLong;

}

//////////////////////////////////////////////////
// Run in archive mode

int HcrShortLongCombine::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir_short);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - HcrShortLongCombine::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir_short << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - HcrShortLongCombine::_runFilelist()" << endl;
    cerr << "  No files found, dir: " << _params.input_dir_short << endl;
    return -1;
  }
  
  // loop through the input file list
  
  int iret = 0;
  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_processFile(paths[ii])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Process a file
// Returns 0 on success, -1 on failure

int HcrShortLongCombine::_processFile(const string &readPath)
{

  PMU_auto_register("Processing file");

  // check we have not already processed this file
  // in the file aggregation step
  
  RadxPath thisPath(readPath);
  for (size_t ii = 0; ii < _readPaths.size(); ii++) {
    RadxPath rpath(_readPaths[ii]);
    if (thisPath.getFile() == rpath.getFile()) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Skipping file: " << readPath << endl;
        cerr << "  Previously processed in aggregation step" << endl;
      }
      return 0;
    }
  }
  
  if (_params.debug) {
    cerr << "INFO - HcrShortLongCombine::Run" << endl;
    cerr << "  Input path: " << readPath << endl;
  }
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file

  RadxVol vol;
  if (inFile.readFromPath(readPath, vol)) {
    cerr << "ERROR - HcrShortLongCombine::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _readPaths = inFile.getReadPaths();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < _readPaths.size(); ii++) {
      cerr << "  ==>> read in file: " << _readPaths[ii] << endl;
    }
  }

  // remove unwanted fields
  
  if (_params.exclude_specified_fields) {
    for (int ii = 0; ii < _params.excluded_fields_n; ii++) {
      if (_params.debug) {
        cerr << "Removing field name: " << _params._excluded_fields[ii] << endl;
      }
      vol.removeField(_params._excluded_fields[ii]);
    }
  }

  // combine the dwells

  _combineDwellsCentered(vol);

  // set field type, names, units etc
  
  _convertFields(vol);

  if (_params.set_output_encoding_for_all_fields) {
    _convertAllFields(vol);
  }

  // set global attributes

  _setGlobalAttr(vol);

  // write the file

  if (_writeVol(vol)) {
    cerr << "ERROR - HcrShortLongCombine::_processFile" << endl;
    cerr << "  Cannot write volume to file" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// set up read

void HcrShortLongCombine::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.set_max_range) {
    file.setReadMaxRangeKm(_params.max_range_km);
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }

}

//////////////////////////////////////////////////
// rename fields as required

void HcrShortLongCombine::_convertFields(RadxVol &vol)
{

  if (!_params.set_output_fields) {
    return;
  }

  for (int ii = 0; ii < _params.output_fields_n; ii++) {

    const Params::output_field_t &ofld = _params._output_fields[ii];
    
    string iname = ofld.input_field_name;
    string oname = ofld.output_field_name;
    string lname = ofld.long_name;
    string sname = ofld.standard_name;
    string ounits = ofld.output_units;
    
    Radx::DataType_t dtype = Radx::ASIS;
    switch(ofld.encoding) {
      case Params::OUTPUT_ENCODING_FLOAT32:
        dtype = Radx::FL32;
        break;
      case Params::OUTPUT_ENCODING_INT32:
        dtype = Radx::SI32;
        break;
      case Params::OUTPUT_ENCODING_INT16:
        dtype = Radx::SI16;
        break;
      case Params::OUTPUT_ENCODING_INT08:
        dtype = Radx::SI08;
        break;
      case Params::OUTPUT_ENCODING_ASIS:
        dtype = Radx::ASIS;
      default: {}
    }

    if (ofld.output_scaling == Params::SCALING_DYNAMIC) {
      vol.convertField(iname, dtype, 
                       oname, ounits, sname, lname);
    } else {
      vol.convertField(iname, dtype, 
                       ofld.output_scale, ofld.output_offset,
                       oname, ounits, sname, lname);
    }
    
  }

}

//////////////////////////////////////////////////
// convert all fields to specified output encoding

void HcrShortLongCombine::_convertAllFields(RadxVol &vol)
{

  switch(_params.output_encoding) {
    case Params::OUTPUT_ENCODING_FLOAT32:
      vol.convertToFl32();
      return;
    case Params::OUTPUT_ENCODING_INT32:
      vol.convertToSi32();
      return;
    case Params::OUTPUT_ENCODING_INT16:
      vol.convertToSi16();
      return;
    case Params::OUTPUT_ENCODING_INT08:
      vol.convertToSi08();
      return;
    case Params::OUTPUT_ENCODING_ASIS:
    default:
      return;
  }

}

//////////////////////////////////////////////////
// set up write

void HcrShortLongCombine::_setupWrite(RadxFile &file)
{

  if (_params.debug) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.output_filename_mode == Params::START_TIME_ONLY) {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_TIME_ONLY);
  } else if (_params.output_filename_mode == Params::END_TIME_ONLY) {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_END_TIME_ONLY);
  } else {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_AND_END_TIMES);
  }

  file.setNcFormat(RadxFile::NETCDF4);
  file.setWriteCompressed(true);
  file.setCompressionLevel(4);

  // set output format

  switch (_params.output_format) {
    case Params::OUTPUT_FORMAT_CFRADIAL2:
      file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL2);
      break;
    case Params::OUTPUT_FORMAT_CFRADIAL:
    default:
      file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  }

  if (strlen(_params.output_filename_prefix) > 0) {
    file.setWriteFileNamePrefix(_params.output_filename_prefix);
  }

  file.setWriteInstrNameInFileName(_params.include_instrument_name_in_file_name);
  file.setWriteSubsecsInFileName(_params.include_subsecs_in_file_name);
  file.setWriteScanTypeInFileName(_params.include_scan_type_in_file_name);
  file.setWriteHyphenInDateTime(_params.use_hyphen_in_file_name_datetime_part);

}

//////////////////////////////////////////////////
// set selected global attributes

void HcrShortLongCombine::_setGlobalAttr(RadxVol &vol)
{

  vol.setDriver("HcrShortLongCombine(NCAR)");
  RadxTime now(RadxTime::NOW);
  vol.setCreated(now.asString());

}

//////////////////////////////////////////////////
// write out the volume

int HcrShortLongCombine::_writeVol(RadxVol &vol)
{

  // are we writing files on time boundaries

  if (_params.write_output_files_on_time_boundaries) {
    return _writeVolOnTimeBoundary(vol);
  }

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {

    string outPath = _params.output_dir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path
  
    if (outFile.writeToPath(vol, outPath)) {
      cerr << "ERROR - HcrShortLongCombine::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
      
  } else {

    // write to dir
  
    if (outFile.writeToDir(vol, _params.output_dir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - HcrShortLongCombine::_writeVol" << endl;
      cerr << "  Cannot write file to dir: " << _params.output_dir << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }

  }

  string outputPath = outFile.getPathInUse();

  // write latest data info file if requested 
  
  if (_params.write_latest_data_info) {
    DsLdataInfo ldata(_params.output_dir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(_params.output_dir, outputPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(vol.getEndTimeSecs())) {
      cerr << "WARNING - HcrShortLongCombine::_writeVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// write out the data splitting on time

int HcrShortLongCombine::_writeVolOnTimeBoundary(RadxVol &vol)
{
  
  // check for time gap

  RadxTime newVolStart = vol.getStartRadxTime();
  RadxTime splitVolEnd = _splitVol.getEndRadxTime();
  double gapSecs = newVolStart - splitVolEnd;
  if (gapSecs > _params.output_file_time_interval_secs * 2) {
    if (_params.debug) {
      cerr << "==>> Found time gap between volumes" << endl;
      cerr << "  splitVolEnd: " << splitVolEnd.asString(3) << endl;
      cerr << "  newVolStart: " << newVolStart.asString(3) << endl;
    }
    _writeSplitVol();
    _setNextEndOfVolTime(newVolStart);
    // clear out rays from previous file
    _dwellVolShort.clearRays();
    // clear any rays before the new vol start
    // these could have been introduced during the merge
  }

  // add rays to the output vol

  _splitVol.copyMeta(vol);
  vector<RadxRay *> &volRays = vol.getRays();
  for (size_t ii = 0; ii < volRays.size(); ii++) {
    RadxRay *ray = volRays[ii];
    if (ray->getRadxTime() > _nextEndOfVolTime) {
      if (_writeSplitVol()) {
        return -1;
      }
    }
    RadxRay *splitRay = new RadxRay(*ray);
    _splitVol.addRay(splitRay);
  } // ii

  return 0;

}

//////////////////////////////////////////////////
// write out the split volume

int HcrShortLongCombine::_writeSplitVol()
{

  // sanity check

  if (_splitVol.getNRays() < 1) {
    return 0;
  }

  // load the sweep information from the rays
  
  _splitVol.loadSweepInfoFromRays();

  // load the volume information from the rays
  
  _splitVol.loadVolumeInfoFromRays();
  
  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  // write out

  if (outFile.writeToDir(_splitVol, _params.output_dir,
                         _params.append_day_dir_to_output_dir,
                         _params.append_year_dir_to_output_dir)) {
    cerr << "ERROR - HcrShortLongCombine::_writeSplitVol" << endl;
    cerr << "  Cannot write file to dir: " << _params.output_dir << endl;
    cerr << outFile.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote file: " << outFile.getPathInUse() << endl;
    cerr << "  StartTime: " << _splitVol.getStartRadxTime().asString(3) << endl;
    cerr << "  EndTime  : " << _splitVol.getEndRadxTime().asString(3) << endl;
  }

  // write latest data info file if requested 
  
  if (_params.write_latest_data_info) {
    string outputPath = outFile.getPathInUse();
    DsLdataInfo ldata(_params.output_dir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(_params.output_dir, outputPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(_splitVol.getEndTimeSecs())) {
      cerr << "WARNING - HcrShortLongCombine::_writeSplitVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  // update next end of vol time

  RadxTime nextVolStart(_splitVol.getEndTimeSecs() + 1);
  _setNextEndOfVolTime(nextVolStart);

  // clear

  _splitVol.clearRays();

  return 0;

}

//////////////////////////////////////////////////
// Compute next end of vol time

void HcrShortLongCombine::_setNextEndOfVolTime(RadxTime &refTime)
{
  _nextEndOfVolTime.set
    (((refTime.utime() / _params.output_file_time_interval_secs) + 1) *
     _params.output_file_time_interval_secs);
  if (_params.debug) {
    cerr << "==>> Next end of vol time: " << _nextEndOfVolTime.asString(3) << endl;
  }
}

/////////////////////////////////////////////////////////////////////////
// Combine the dwells in this volume

int HcrShortLongCombine::_combineDwells(RadxVol &vol)

{
  
  if (_params.debug) {
    cerr << "INFO - combineDwells: nrays left from previous file: "
         << _dwellVolShort.getNRays() << endl;
  }

  // create a volume for stats
  
  vector<RadxRay *> combRays;
  
  const vector<RadxRay *> &fileRays = vol.getRays();
  for (size_t iray = 0; iray < fileRays.size(); iray++) {
    
    // add rays to stats vol
    
    RadxRay *ray = new RadxRay(*fileRays[iray]);
    if (_dwellVolShort.getNRays() == 0) {
      _dwellStartTime = ray->getRadxTime();
    }
    _dwellVolShort.addRay(ray);
    int nRaysDwell = _dwellVolShort.getNRays();
    _dwellEndTime = ray->getRadxTime();
    double dwellSecs = (_dwellEndTime - _dwellStartTime);
    if (nRaysDwell > 1) {
      dwellSecs *= ((double) nRaysDwell / (nRaysDwell - 1.0));
    }
    
    // dwell time exceeded, so compute dwell ray and add to volume
    
    if (dwellSecs >= _params.dwell_length_secs) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "INFO: _combineDwells, using nrays: " << nRaysDwell << endl;
      }
      RadxRay *dwellRay =
        _dwellVolShort.computeFieldStats(_globalMethod,
                                    _namedMethods,
                                    _params.dwell_stats_max_fraction_missing);
      if (dwellRay->getRadxTime() >= vol.getStartRadxTime() - 60) {
        combRays.push_back(dwellRay);
      } else {
        RadxRay::deleteIfUnused(dwellRay);
      }
      // clear out stats vol
      _dwellVolShort.clearRays();
    }
      
  } // iray

  // move combination rays into volume

  vol.clearRays();
  for (size_t ii = 0; ii < combRays.size(); ii++) {
    vol.addRay(combRays[ii]);
  }
  
  // compute volume metadata

  vol.loadSweepInfoFromRays();

  return 0;

}

/////////////////////////////////////////////////////////////////////////
// Combine the dwells centered on time

int HcrShortLongCombine::_combineDwellsCentered(RadxVol &vol)

{
  
  if (_params.debug) {
    cerr << "INFO - combineDwellsCentered: nrays left from previous file: "
         << _dwellVolShort.getNRays() << endl;
  }

  // create a volume for combined dwells
  
  vector<RadxRay *> combRays;
  
  const vector<RadxRay *> &fileRays = vol.getRays();
  for (size_t iray = 0; iray < fileRays.size(); iray++) {

    RadxRay *ray = new RadxRay(*fileRays[iray]);
    _latestRayTime = ray->getRadxTime();
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "==>> got new ray, latestRayTime: " << _latestRayTime.asString(3) << endl;
    }

    // at the start of reading a volume, we always need 
    // at least 1 ray in the dwell

    if (_dwellVolShort.getNRays() == 0) {
      _dwellVolShort.addRay(ray);
      continue;
    }
    
    // set dwell time limits if we have just 1 ray in the dwell so far
    
    if (_dwellVolShort.getNRays() == 1) {

      _dwellStartTime = _dwellVolShort.getRays()[0]->getRadxTime();

      RadxTime volStartTime(vol.getStartTimeSecs());
      double dsecs = _latestRayTime - volStartTime;
      double roundedSecs =
        ((int) (dsecs / _params.dwell_length_secs) + 1.0) * _params.dwell_length_secs;
      _dwellMidTime = volStartTime + roundedSecs;
      _dwellEndTime = _dwellMidTime + _params.dwell_length_secs / 2.0;

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> starting new dwell <<==" << endl;
        cerr << "  _dwellStartTime: " << _dwellStartTime.asString(3) << endl;
        cerr << "  _dwellMidTime: " << _dwellMidTime.asString(3) << endl;
        cerr << "  _dwellEndTime: " << _dwellEndTime.asString(3) << endl;
      }
        
    }
    
    // dwell time exceeded, so compute dwell ray stats
    // and add results to volume
    
    if (_latestRayTime > _dwellEndTime) {

      // beyond end of dwell, process this one
      
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "INFO: _combineDwellsCentered, using nrays: "
             << _dwellVolShort.getNRays() << endl;
        const vector<RadxRay *> &dwellRays = _dwellVolShort.getRays();
        for (size_t jray = 0; jray < dwellRays.size(); jray++) {
          const RadxRay *dray = dwellRays[jray];
          cerr << "INFO: using ray at time: "
               << dray->getRadxTime().asString(3) << endl;
        }
      } // debug

      // compute ray for dwell
      
      RadxRay *dwellRay =
        _dwellVolShort.computeFieldStats(_globalMethod, _namedMethods,
                                    _params.dwell_stats_max_fraction_missing);

      // add it to the combination

      if (dwellRay) {
        if (dwellRay->getRadxTime() >= vol.getStartRadxTime() - 60) {
          dwellRay->setTime(_dwellMidTime);
          combRays.push_back(dwellRay);
        } else {
          RadxRay::deleteIfUnused(dwellRay);
        }
      }

      // clear out stats vol

      _dwellVolShort.clearRays();

    } // if (_latestRayTime > _dwellEndTime) {
      
    // add the latest ray to the next dwell vol
    
    _dwellVolShort.addRay(ray);
    
  } // iray

  // move combination rays into volume
  
  vol.clearRays();
  for (size_t ii = 0; ii < combRays.size(); ii++) {
    vol.addRay(combRays[ii]);
  }
  
  // compute volume metadata

  vol.loadSweepInfoFromRays();

  return 0;

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


//////////////////////////////////////////////////
// write radar and field parameters

int HcrShortLongCombine::_writeParams(const RadxRay *ray)

{

  // radar parameters
  
  DsRadarMsg msg;
  DsRadarParams &rparams = msg.getRadarParams();
  rparams = _rparams;
  rparams.numFields = ray->getFields().size();
  
  // field parameters - all fields are fl32
  
  vector<DsFieldParams* > &fieldParams = msg.getFieldParams();
  vector<RadxField *> flds = ray->getFields();
  for (size_t ifield = 0; ifield < flds.size(); ifield++) {
    
    const RadxField &fld = *flds[ifield];
    double dsScale = 1.0, dsBias = 0.0;
    int dsMissing = (int) floor(fld.getMissingFl32() + 0.5);
    
    DsFieldParams *fParams = new DsFieldParams(fld.getName().c_str(),
                                               fld.getUnits().c_str(),
                                               dsScale, dsBias, sizeof(fl32),
                                               dsMissing);
    fieldParams.push_back(fParams);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fParams->print(cerr);
    }

  } // ifield

  // put params
  
  // int content = DsRadarMsg::FIELD_PARAMS | DsRadarMsg::RADAR_PARAMS;
  // if(_outputFmq.putDsMsg(msg, content)) {
  //   cerr << "ERROR - HcrShortLongCombine::_writeParams()" << endl;
  //   cerr << "  Cannot write field params to FMQ" << endl;
  //   cerr << "  URL: " << _params.output_fmq_url << endl;
  //   return -1;
  // }
        
  return 0;

}

//////////////////////////////////////////////////
// write ray

int HcrShortLongCombine::_writeRay(const RadxRay *ray)
  
{

  // write params if needed

  int nGates = ray->getNGates();
  const vector<RadxField *> &fields = ray->getFields();
  int nFields = ray->getFields().size();
  int nPoints = nGates * nFields;
  
  // meta-data
  
  DsRadarMsg msg;
  DsRadarBeam &beam = msg.getRadarBeam();

  beam.dataTime = ray->getTimeSecs();
  beam.nanoSecs = (int) (ray->getNanoSecs() + 0.5);
  beam.referenceTime = 0;

  beam.byteWidth = sizeof(fl32); // fl32

  beam.volumeNum = ray->getVolumeNumber();
  beam.tiltNum = ray->getSweepNumber();
  Radx::SweepMode_t sweepMode = ray->getSweepMode();
  beam.scanMode = _getDsScanMode(sweepMode);
  beam.antennaTransition = ray->getAntennaTransition();
  
  beam.azimuth = ray->getAzimuthDeg();
  beam.elevation = ray->getElevationDeg();
  
  if (sweepMode == Radx::SWEEP_MODE_RHI ||
      sweepMode == Radx::SWEEP_MODE_MANUAL_RHI) {
    beam.targetAz = ray->getFixedAngleDeg();
  } else {
    beam.targetElev = ray->getFixedAngleDeg();
  }

  beam.beamIsIndexed = ray->getIsIndexed();
  beam.angularResolution = ray->getAngleResDeg();
  beam.nSamples = ray->getNSamples();

  beam.measXmitPowerDbmH = ray->getMeasXmitPowerDbmH();
  beam.measXmitPowerDbmV = ray->getMeasXmitPowerDbmV();

  // 4-byte floats
  
  fl32 *data = new fl32[nPoints];
  for (int ifield = 0; ifield < nFields; ifield++) {
    fl32 *dd = data + ifield;
    const RadxField *fld = fields[ifield];
    const Radx::fl32 *fd = (Radx::fl32 *) fld->getData();
    if (fd == NULL) {
      cerr << "ERROR - Radx2Dsr::_writeBeam" << endl;
      cerr << "  NULL data pointer, field name, elev, az: "
           << fld->getName() << ", "
           << ray->getElevationDeg() << ", "
           << ray->getAzimuthDeg() << endl;
      delete[] data;
      return -1;
    }
    for (int igate = 0; igate < nGates; igate++, dd += nFields, fd++) {
      *dd = *fd;
    }
  }
  beam.loadData(data, nPoints * sizeof(fl32), sizeof(fl32));
  delete[] data;
    
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    beam.print(cerr);
  }
  
  // add georeference if applicable

  int contents = (int) DsRadarMsg::RADAR_BEAM;
  if (_georefs.size() > 0) {
    DsPlatformGeoref &georef = msg.getPlatformGeoref();
    // use mid georef
    georef = _georefs[_georefs.size() / 2];
    contents |= DsRadarMsg::PLATFORM_GEOREF;
  }
    
  // put beam
  
  // if(_outputFmq.putDsMsg(msg, contents)) {
  //   cerr << "ERROR - HcrShortLongCombine::_writeBeam()" << endl;
  //   cerr << "  Cannot write beam to FMQ" << endl;
  //   cerr << "  URL: " << _params.output_fmq_url << endl;
  //   return -1;
  // }

  // debug print
  
  _nRaysWritten++;
  if (_params.debug) {
    if (_nRaysWritten % 100 == 0) {
      cerr << "====>> wrote nRays, latest time, el, az: "
           << _nRaysWritten << ", "
           << utimstr(ray->getTimeSecs()) << ", "
           << ray->getElevationDeg() << ", "
           << ray->getAzimuthDeg() << endl;
    }
  }
  
  return 0;

}

////////////////////////////
// is this an output field?

bool HcrShortLongCombine::_isOutputField(const string &name)

{

  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    string inputFieldName = _params._output_fields[ifield].input_field_name;
    if (name == inputFieldName) {
      return true;
    }
  }
  
  return false;

}

/////////////////////////////////////////////
// convert from DsrRadar enums to Radx enums

Radx::SweepMode_t HcrShortLongCombine::_getRadxSweepMode(int dsrScanMode)

{

  switch (dsrScanMode) {
    case DS_RADAR_SECTOR_MODE:
    case DS_RADAR_FOLLOW_VEHICLE_MODE:
      return Radx::SWEEP_MODE_SECTOR;
    case DS_RADAR_COPLANE_MODE:
      return Radx::SWEEP_MODE_COPLANE;
    case DS_RADAR_RHI_MODE:
      return Radx::SWEEP_MODE_RHI;
    case DS_RADAR_VERTICAL_POINTING_MODE:
      return Radx::SWEEP_MODE_VERTICAL_POINTING;
    case DS_RADAR_MANUAL_MODE:
      return Radx::SWEEP_MODE_POINTING;
    case DS_RADAR_SURVEILLANCE_MODE:
      return Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
    case DS_RADAR_EL_SURV_MODE:
      return Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE;
    case DS_RADAR_SUNSCAN_MODE:
      return Radx::SWEEP_MODE_SUNSCAN;
    case DS_RADAR_SUNSCAN_RHI_MODE:
      return Radx::SWEEP_MODE_SUNSCAN_RHI;
    case DS_RADAR_POINTING_MODE:
      return Radx::SWEEP_MODE_POINTING;
    default:
      return Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
  }
}

Radx::PolarizationMode_t HcrShortLongCombine::_getRadxPolarizationMode(int dsrPolMode)

{
  switch (dsrPolMode) {
    case DS_POLARIZATION_HORIZ_TYPE:
      return Radx::POL_MODE_HORIZONTAL;
    case DS_POLARIZATION_VERT_TYPE:
      return Radx::POL_MODE_VERTICAL;
    case DS_POLARIZATION_DUAL_TYPE:
      return Radx::POL_MODE_HV_SIM;
    case DS_POLARIZATION_DUAL_HV_ALT:
      return Radx::POL_MODE_HV_ALT;
    case DS_POLARIZATION_DUAL_HV_SIM:
      return Radx::POL_MODE_HV_SIM;
    case DS_POLARIZATION_RIGHT_CIRC_TYPE:
    case DS_POLARIZATION_LEFT_CIRC_TYPE:
      return Radx::POL_MODE_CIRCULAR;
    case DS_POLARIZATION_ELLIPTICAL_TYPE:
    default:
      return Radx::POL_MODE_HORIZONTAL;
  }
}

Radx::FollowMode_t HcrShortLongCombine::_getRadxFollowMode(int dsrMode)

{
  switch (dsrMode) {
    case DS_RADAR_FOLLOW_MODE_SUN:
      return Radx::FOLLOW_MODE_SUN;
    case DS_RADAR_FOLLOW_MODE_VEHICLE:
      return Radx::FOLLOW_MODE_VEHICLE;
    case DS_RADAR_FOLLOW_MODE_AIRCRAFT:
      return Radx::FOLLOW_MODE_AIRCRAFT;
    case DS_RADAR_FOLLOW_MODE_TARGET:
      return Radx::FOLLOW_MODE_TARGET;
    case DS_RADAR_FOLLOW_MODE_MANUAL:
      return Radx::FOLLOW_MODE_MANUAL;
    default:
      return Radx::FOLLOW_MODE_NONE;
  }
}

Radx::PrtMode_t HcrShortLongCombine::_getRadxPrtMode(int dsrMode)

{
  switch (dsrMode) {
    case DS_RADAR_PRF_MODE_FIXED:
      return Radx::PRT_MODE_FIXED;
    case DS_RADAR_PRF_MODE_STAGGERED_2_3:
    case DS_RADAR_PRF_MODE_STAGGERED_3_4:
    case DS_RADAR_PRF_MODE_STAGGERED_4_5:
      return Radx::PRT_MODE_STAGGERED;
    default:
      return Radx::PRT_MODE_FIXED;
  }
}

//////////////////////////////////////////////////
// get Dsr enums from Radx enums

int HcrShortLongCombine::_getDsScanMode(Radx::SweepMode_t mode)

{
  switch (mode) {
    case Radx::SWEEP_MODE_SECTOR:
      return DS_RADAR_SECTOR_MODE;
    case Radx::SWEEP_MODE_COPLANE:
      return DS_RADAR_COPLANE_MODE;
    case Radx::SWEEP_MODE_RHI:
      return DS_RADAR_RHI_MODE;
    case Radx::SWEEP_MODE_VERTICAL_POINTING:
      return DS_RADAR_VERTICAL_POINTING_MODE;
    case Radx::SWEEP_MODE_IDLE:
      return DS_RADAR_IDLE_MODE;
    case Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE:
      return DS_RADAR_SURVEILLANCE_MODE;
    case Radx::SWEEP_MODE_SUNSCAN:
      return DS_RADAR_SUNSCAN_MODE;
    case Radx::SWEEP_MODE_POINTING:
      return DS_RADAR_POINTING_MODE;
    case Radx::SWEEP_MODE_MANUAL_PPI:
      return DS_RADAR_MANUAL_MODE;
    case Radx::SWEEP_MODE_MANUAL_RHI:
      return DS_RADAR_MANUAL_MODE;
    case Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE:
    default:
      return DS_RADAR_SURVEILLANCE_MODE;
  }
}

