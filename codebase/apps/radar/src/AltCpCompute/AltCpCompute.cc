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
// AltCpCompute.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2006
//
///////////////////////////////////////////////////////////////
//
// AltCpCompute analyses data from tsarchive time series files
// using the cross-polar technique to determine ZDR bias.
//
////////////////////////////////////////////////////////////////

#include "AltCpCompute.hh"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <toolsa/DateTime.hh>
#include <toolsa/uusleep.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/mem.h>
#include <toolsa/TaArray.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/TaStr.hh>
#include <radar/RadarMoments.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRcalib.hh>
#include <Spdb/DsSpdb.hh>
#include <rapformats/WxObs.hh>
#include <Mdv/GenericRadxFile.hh>

using namespace std;

// Constructor

AltCpCompute::AltCpCompute(int argc, char **argv)
  
{

  isOK = true;
  _pulseSeqNum = 0;
  _totalPulseCount = 0;
  _startTime = 0;
  _pulseTime = 0;
  _calTime = 0;
  _midPrt = 0;
  _midEl = 0;
  _midAz = 0;
  _prevAz = -999;
  _azMoved = 0;
  _nBeams = 0;
  _reader = NULL;
  _switching = false;
  _ratioFile = NULL;
  
  _clearRatio();

  // set programe name
  
  _progName = "AltCpCompute";

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
    return;
  }

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
  return;
  
}

// destructor

AltCpCompute::~AltCpCompute()

{

  // close reader

  if (_reader) {
    delete _reader;
  }

  // close files

  if (_ratioFile) {
    fclose(_ratioFile);
  }

  // clean up memory

  _freeGateData();
  _clearPulseQueue();
  
}

//////////////////////////////////////////////////
// Run

int AltCpCompute::Run ()
{

  // CfRadial mode?

  if (_params.input_mode == Params::CFRADIAL_INPUT) {
    return _cfradialRun();
  } else {
    return _timeSeriesRun();
  }

}

//////////////////////////////////////////////////
// Run in time series mode

int AltCpCompute::_timeSeriesRun()
{

  // initialize time series mode

  _writeCount = 0;
  _pairCount = 0;

  // set _nSamples - must be even

  _nSamplesHalf = _params.n_samples / 2;
  _nSamples = _nSamplesHalf * 2;

  // create reader

  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug) {
    iwrfDebug = IWRF_DEBUG_NORM;
  }
  if (_params.input_mode == Params::TS_FILE_INPUT) {
    _reader = new IwrfTsReaderFile(_args.inputFileList, iwrfDebug);
  } else {
    _reader = new IwrfTsReaderFmq(_params.input_fmq_name, iwrfDebug);
  }

  // switching receiver?

  if (_params.switching_receiver) {
    _switching = true;
  } else {
    _switching = false;
  }

 // calibration

  string errStr;
  if (_calib.readFromXmlFile(_params.cal_xml_file_path, errStr)) {
    cerr << "ERROR - AltCpCompute" << endl;
    cerr << "  Cannot decode cal file: "
	 << _params.cal_xml_file_path << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "From cal file noiseHxDbm: " << _calib.getNoiseDbmHx() << endl;
    cerr << "              noiseVxDbm: " << _calib.getNoiseDbmVx() << endl;
  }

  _noiseHc = pow(10.0, _calib.getNoiseDbmHc() / 10.0);
  _noiseHx = pow(10.0, _calib.getNoiseDbmHx() / 10.0);
  _noiseVc = pow(10.0, _calib.getNoiseDbmVc() / 10.0);
  _noiseVx = pow(10.0, _calib.getNoiseDbmVx() / 10.0);
  
  _minValidRatioDb = _params.min_valid_ratio_db;
  _maxValidRatioDb = _params.max_valid_ratio_db;

  if (_params.write_data_pairs_to_stdout) {
    _writePairHeader();
  }

  // run

  while (true) {
    
    const IwrfTsPulse *pulse = _reader->getNextPulse(true);
    if (pulse == NULL) {
      break;
    }
    
    if (_params.invert_hv_flag) {
      pulse->setInvertHvFlag();
    }
    if (_processPulse(pulse)) {
      return -1;
    }
    
  }

  if (_nBeams > 0) {
    RadxTime ptime(_pulseTime);
    if (_writeRatioToFile(ptime)) {
      return -1;
    }
  }

  return 0;

}

/////////////////////
// process a pulse

int AltCpCompute::_processPulse(const IwrfTsPulse *pulse)

{

  // at start, print headers

  _latestPulseTime = pulse->getTime();
  if (_totalPulseCount == 0) {
    _startTime = pulse->getFTime();
  }
  _pulseTime = pulse->getFTime();
  _calTime = (_startTime + _pulseTime) / 2.0;

  // force the queue to start on a horizontal pulse

  if (_pulseQueue.size() == 0 && !pulse->isHoriz()) {
    delete pulse;
    return 0;
  }

  // add the pulse to the queue

  _addPulseToQueue(pulse);
  _totalPulseCount++;
  
  // do we have a full pulse queue?

  if ((int) _pulseQueue.size() < _nSamples) {
    return 0;
  }

  // does the pulse az or el change drastically?
  
  int qSize = (int) _pulseQueue.size();
  double azStart = _pulseQueue[0]->getAz();
  double elStart = _pulseQueue[0]->getEl();
  double azEnd = _pulseQueue[qSize-1]->getAz();
  double elEnd = _pulseQueue[qSize-1]->getEl();
    
  double azDiff = RadarComplex::diffDeg(azStart, azEnd);
  double elDiff = RadarComplex::diffDeg(elStart, elEnd);
  if (fabs(azDiff) > 5 || fabs(elDiff) > 0.2) {
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "====>> Clearing pulse queue" << endl;
      cerr << "  azStart, azEnd: " << azStart << ", " << azEnd << endl;
      cerr << "  elStart, elEnd: " << elStart << ", " << elEnd << endl;
    }
    _clearPulseQueue();
    return 0;
  }

  // find pulse at mid point of queue
  
  int midIndex = _nSamples / 2;
  const IwrfTsPulse *midPulse = _pulseQueue[midIndex];
  _midPrt = midPulse->getPrt();
  _midEl = midPulse->getEl();
  _midAz = midPulse->getAz();

  // sum up movement in azimuth

  if (_prevAz < -900) {
    _prevAz = _midAz;
  } else {
    azDiff = RadarComplex::diffDeg(_prevAz, _midAz);
    _azMoved += fabs(azDiff);
    _prevAz = _midAz;
    _nBeams++;
  }
  
  // compute the moments
  
  if (_checkNGates() && _checkDual()) {
    if (_processBeam()) {
      return -1;
    }
  }

  // ready for analysis

  if (_params.nbeams_per_analysis > 0 &&
      _nBeams >= _params.nbeams_per_analysis) {
    RadxTime ptime(_pulseTime);
    if (_writeRatioToFile(ptime)) {
      return -1;
    }
    _clearRatio();
    _azMoved = 0;
    _nBeams = 0;
  }

  _clearPulseQueue();

  return 0;

}

/////////////////////////////////////////////////
// add the pulse to the pulse queue
    
void AltCpCompute::_addPulseToQueue(const IwrfTsPulse *pulse)
  
{

  // manage the size of the pulse queue, popping off the back

  if ((int) _pulseQueue.size() >= _nSamples) {
    const IwrfTsPulse *oldest = _pulseQueue.front();
    if (oldest->removeClient() == 0) {
      delete oldest;
    }
    _pulseQueue.pop_front();
  }

  int qSize = (int) _pulseQueue.size();
  if (qSize > 1) {

    // check for big azimuth or elevation change
    // if so clear the queue
    
    double az = pulse->getAz();
    double el = pulse->getEl();
    double prevAz = _pulseQueue[qSize-1]->getAz();
    double prevEl = _pulseQueue[qSize-1]->getEl();
    
    double azDiff = RadarComplex::diffDeg(az, prevAz);
    double elDiff = RadarComplex::diffDeg(el, prevEl);
    if (fabs(azDiff) > 0.1 || fabs(elDiff) > 0.1) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "====>> Clearing pulse queue" << endl;
        cerr << "  az, prevAz: " << az << ", " << prevAz << endl;
        cerr << "  el, prevEl: " << el << ", " << prevEl << endl;
      }
      _clearPulseQueue();
      return;
    }

  }
    
  // push pulse onto front of queue
  
  pulse->addClient();
  _pulseQueue.push_back(pulse);

  // print missing pulses in verbose mode
  
  if ((int) pulse->getSeqNum() != _pulseSeqNum + 1) {
    if (_params.debug >= Params::DEBUG_VERBOSE && _pulseSeqNum != 0) {
      cerr << "**** Missing seq num: " << _pulseSeqNum
	   << " to " <<  pulse->getSeqNum() << " ****" << endl;
    }
  }
  _pulseSeqNum = pulse->getSeqNum();

  // check number of gates is constant

  qSize = (int) _pulseQueue.size();
  _nGates = _pulseQueue[0]->getNGates();
  for (int ii = 1; ii < qSize; ii++) {
    if (_pulseQueue[ii]->getNGates() != _nGates) {
      _clearPulseQueue();
      return;
    }
  }

}

/////////////////////////////////////////////////
// clear the pulse queue
    
void AltCpCompute::_clearPulseQueue()
  
{
  
  for (size_t ii = 0; ii < _pulseQueue.size(); ii++) {
    delete _pulseQueue[ii];
  }
  _pulseQueue.clear();

}

////////////////////////////////////////////
// process a beam of data
// compute moments using pulses in queue

int AltCpCompute::_processBeam()

{

  // get beam info

  int qSize = (int) _pulseQueue.size();
  const IwrfTsPulse *midPulse = _pulseQueue[qSize / 2];
  double beamAz = midPulse->getAz();
  double beamEl = midPulse->getEl();
  time_t beamTime = midPulse->getTime();
  int nanoSecs = midPulse->getNanoSecs();
  RadxTime rayTime(beamTime);
  rayTime.setSubSec((double) nanoSecs / 1.0e9);
    
  // load up the IQ data for each gate

  _loadGateData();

  // loop through gates to compute moments
  
  const IwrfTsInfo &opsInfo = _reader->getOpsInfo();
  double startRangeKm = opsInfo.get_proc_start_range_km();
  double gateSpacingKm = opsInfo.get_proc_gate_spacing_km();
  double rangeKm = startRangeKm + gateSpacingKm / 2.0;
  if (gateSpacingKm < 0) {
    return 0;
  }
  
  for (int igate = 0; igate < _nGates; igate++, rangeKm += gateSpacingKm) {
    
    GateData *gate = _gateData[igate];
    
    // power
    
    double power_hx = RadarComplex::meanPower(gate->iqhx, _nSamplesHalf);
    double power_vx = RadarComplex::meanPower(gate->iqvx, _nSamplesHalf);
    if (power_hx <= 0 || power_vx <= 0) {
      continue;
    }

    double dbmhx = 10.0 * log10(power_hx);
    double dbmvx = 10.0 * log10(power_vx);
    double ratioHxVxDb = dbmhx - dbmvx;

    if (ratioHxVxDb < _minValidRatioDb || ratioHxVxDb > _maxValidRatioDb) {
      continue;
    }

    // noise subtracted power and SNR
    
    double ns_power_hx = power_hx - _noiseHx;
    double ns_power_vx = power_vx - _noiseVx;

    // check mean snr in cross-polar channels
    
    if (ns_power_hx <= 0 || ns_power_vx <= 0) {
      continue;
    }

    double snrhx = 10.0 * log10(ns_power_hx / _noiseHx);
    double snrvx = 10.0 * log10(ns_power_vx / _noiseVx);
    
    if (snrhx < _params.min_snr || snrhx > _params.max_snr) {
      continue;
    }
    if (snrvx < _params.min_snr || snrvx > _params.max_snr) {
      continue;
    }
    
    // compute cpa
    
    double cpa =
      RadarMoments::computeCpaAlt(gate->iqhc, gate->iqvc, _nSamplesHalf - 1);
    
    // compute cross correlation
  
    double lag0_hx = RadarComplex::meanPower(gate->iqhx, _nSamplesHalf - 1);
    double lag0_vx = RadarComplex::meanPower(gate->iqvx, _nSamplesHalf - 1);
    RadarComplex_t lag0_vxhx =
      RadarComplex::meanConjugateProduct(gate->iqvx, gate->iqhx, _nSamplesHalf - 1);
    double rhoVxHx = RadarComplex::mag(lag0_vxhx) / sqrt(lag0_vx * lag0_hx);

    // for time series, set cmd to missing
    
    double cmd = -9999.0;

    // add to stats
    
    _addGateData(rayTime, beamEl, beamAz, igate, rangeKm,
                 cpa, cmd, snrhx, snrvx, rhoVxHx, dbmhx, dbmvx);

  } // igate

  return 0;

}

/////////////////////////////////////////////////
// check we have a constant number of gates

bool AltCpCompute::_checkNGates()
  
{

  if (_pulseQueue.size() < 2) {
    return false;
  }

  // set prt and nGates, assuming single PRT for now

  _nGates = _pulseQueue[0]->getNGates();
  
  // check we have constant nGates

  for (size_t ii = 1; ii < _pulseQueue.size(); ii++) {
    int nGates = _pulseQueue[ii]->getNGates();
    if (nGates != _nGates) {
      if (_params.debug) {
        cerr << "nGates differ, this, prev: "
             << nGates << ", " << _nGates << endl;
      }
      return false;
    }
  }

  return true;

}

/////////////////////////////////////////////////
// check we have dual channels

bool AltCpCompute::_checkDual()
  
{

  for (size_t ii = 1; ii < _pulseQueue.size(); ii++) {
    if (_pulseQueue[ii]->getIq1() == NULL) {
      return false;
    }
  }

  return true;

}

/////////////////////////////////////////////////////////////////
// Allocate or re-allocate gate data

void AltCpCompute::_allocGateData()

{
  int nNeeded = _nGates - (int) _gateData.size();
  if (nNeeded > 0) {
    for (int ii = 0; ii < nNeeded; ii++) {
      GateData *gate = new GateData();
      _gateData.push_back(gate);
    }
  }
  for (size_t ii = 0; ii < _gateData.size(); ii++) {
    _gateData[ii]->allocArrays(_nSamples, false, false, false);
  }
}

/////////////////////////////////////////////////////////////////
// Free gate data

void AltCpCompute::_freeGateData()

{
  for (int ii = 0; ii < (int) _gateData.size(); ii++) {
    delete _gateData[ii];
  }
  _gateData.clear();
}

/////////////////////////////////////////////////////////////////
// Load gate IQ data.
//
// Assumptions:
// 1. Data is in simultaneouse mode.
// 2. Non-switching dual receivers,
//    H is channel 0 and V channel 1.

void AltCpCompute::_loadGateData()
  
{

  // alloc gate data

  _allocGateData();

  // set up data pointer arrays

  TaArray<const fl32 *> iqChan0_, iqChan1_;
  const fl32* *iqChan0 = iqChan0_.alloc(_nSamples);
  const fl32* *iqChan1 = iqChan1_.alloc(_nSamples);
  for (int isamp = 0; isamp < _nSamples; isamp++) {
    iqChan0[isamp] = _pulseQueue[isamp]->getIq0();
    iqChan1[isamp] = _pulseQueue[isamp]->getIq1();
  }
  
  // load up IQ arrays, gate by gate

  for (int igate = 0, ipos = 0;  igate < _nGates; igate++, ipos += 2) {
    
    GateData *gate = _gateData[igate];
    RadarComplex_t *iqhc = gate->iqhc;
    RadarComplex_t *iqvc = gate->iqvc;
    RadarComplex_t *iqhx = gate->iqhx;
    RadarComplex_t *iqvx = gate->iqvx;
    
    // samples start on horiz pulse

    for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
         isamp++, iqhc++, iqvc++, iqhx++, iqvx++) {

      if (_switching) {

        // H co-polar, V cross-polar

        iqhc->re = iqChan0[jsamp][ipos];
        iqhc->im = iqChan0[jsamp][ipos + 1];
        iqvx->re = iqChan1[jsamp][ipos];
        iqvx->im = iqChan1[jsamp][ipos + 1];
        jsamp++;
        
        // V co-polar, H cross-polar

        iqvc->re = iqChan0[jsamp][ipos];
        iqvc->im = iqChan0[jsamp][ipos + 1];
        iqhx->re = iqChan1[jsamp][ipos];
        iqhx->im = iqChan1[jsamp][ipos + 1];
        jsamp++;

      } else {

        // H from chan 0, V from chan 1

        iqhc->re = iqChan0[jsamp][ipos];
        iqhc->im = iqChan0[jsamp][ipos + 1];
        iqvx->re = iqChan1[jsamp][ipos];
        iqvx->im = iqChan1[jsamp][ipos + 1];
        jsamp++;
        
        iqhx->re = iqChan0[jsamp][ipos];
        iqhx->im = iqChan0[jsamp][ipos + 1];
        iqvc->re = iqChan1[jsamp][ipos];
        iqvc->im = iqChan1[jsamp][ipos + 1];
        jsamp++;

      }
      
    } // isamp

  } // igate

}

//////////////////////////////////////////////////
// Run

int AltCpCompute::_cfradialRun()
{

  _writeCount = 0;
  _pairCount = 0;

  if (_params.write_data_pairs_to_stdout) {
    _writePairHeader();
  }

  // calibration
  
  string errStr;
  if (_calib.readFromXmlFile(_params.cal_xml_file_path, errStr)) {
    cerr << "ERROR - AltCpCompute::_cfradialRun" << endl;
    cerr << "  Cannot decode cal file: "
	 << _params.cal_xml_file_path << endl;
    cerr << "  " << errStr << endl;
    return -1;
  }

  if (_params.cfradial_mode == Params::CFRADIAL_ARCHIVE) {
    return _cfradialRunArchive();
  } else if (_params.cfradial_mode == Params::CFRADIAL_FILELIST) {
    return _cfradialRunFilelist();
  } else {
    if (_params.cfradial_latest_data_info_avail) {
      return _cfradialRunRealtimeWithLdata();
    } else {
      return _cfradialRunRealtimeNoLdata();
    }
  }
}

//////////////////////////////////////////////////
// Run in filelist mode

int AltCpCompute::_cfradialRunFilelist()
{

  // loop through the input file list

  int iret = 0;

  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {

    string inputPath = _args.inputFileList[ii];
    if (_cfradialProcessFile(inputPath)) {
      iret = -1;
    }

  }

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int AltCpCompute::_cfradialRunArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.cfradial_input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - AltCpCompute::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.cfradial_input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - AltCpCompute::_runFilelist()" << endl;
    cerr << "  No files found, dir: " << _params.cfradial_input_dir << endl;
    return -1;
  }
  
  // loop through the input file list
  
  int iret = 0;
  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_cfradialProcessFile(paths[ii])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode with latest data info

int AltCpCompute::_cfradialRunRealtimeWithLdata()
{

  // watch for new data to arrive

  LdataInfo ldata(_params.cfradial_input_dir,
                  _params.debug >= Params::DEBUG_VERBOSE);
  if (strlen(_params.cfradial_search_ext) > 0) {
    ldata.setDataFileExt(_params.cfradial_search_ext);
  }
  
  int iret = 0;
  int msecsWait = _params.cfradial_wait_between_checks * 1000;
  while (true) {
    ldata.readBlocking(_params.cfradial_max_realtime_data_age_secs,
                       msecsWait, PMU_auto_register);
    const string path = ldata.getDataPath();
    if (_cfradialProcessFile(path)) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode without latest data info

int AltCpCompute::_cfradialRunRealtimeNoLdata()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);
  
  // Set up input path

  DsInputPath input(_progName,
		    _params.debug >= Params::DEBUG_VERBOSE,
		    _params.cfradial_input_dir,
		    _params.cfradial_max_realtime_data_age_secs,
		    PMU_auto_register,
		    _params.cfradial_latest_data_info_avail,
		    false);

  input.setFileQuiescence(_params.cfradial_file_quiescence);
  input.setSearchExt(_params.cfradial_search_ext);
  input.setRecursion(_params.cfradial_search_recursively);
  input.setMaxRecursionDepth(_params.cfradial_max_recursion_depth);
  input.setMaxDirAge(_params.cfradial_max_realtime_data_age_secs);

  int iret = 0;

  while(true) {

    // check for new data
    
    char *path = input.next(false);
    
    if (path == NULL) {
      
      // sleep a bit
      
      PMU_auto_register("Waiting for data");
      umsleep(_params.cfradial_wait_between_checks * 1000);

    } else {

      // process the file

      if (_cfradialProcessFile(path)) {
        iret = -1;
      }
      
    }

  } // while

  return iret;

}

//////////////////////////////////////////////////
// Process a file
// Returns 0 on success, -1 on failure

int AltCpCompute::_cfradialProcessFile(const string &readPath)
{

  PMU_auto_register("Processing CfRadial file");

  if (_params.debug) {
    cerr << "INFO - AltCpCompute::_cfradialProcessFile" << endl;
    cerr << "  Input path: " << readPath << endl;
  }
  
  if (_params.write_data_pairs_to_stdout) {
    _writePairHeader();
  }

  GenericRadxFile inFile;
  _cfradialSetupRead(inFile);
  
  // read in file

  RadxVol vol;
  if (inFile.readFromPath(readPath, vol)) {
    cerr << "ERROR - AltCpCompute::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _startTime = vol.getStartTimeSecs();
  
  // retrieve data from XML status
  if (_params.retrieve_vals_from_xml_status) {
    _retrieveValsFromXmlStatus(vol);
  }

  // optionally retieve site temperature

  _siteTempC = NAN;
  _timeForSiteTemp = 0;
  if (_params.read_site_temp_from_spdb) {
    if (_retrieveSiteTempFromSpdb(vol, _siteTempC, _timeForSiteTemp) == 0) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> site tempC: " 
             << _siteTempC << " at " << RadxTime::strm(_timeForSiteTemp) << endl;
      }
    }
  }

  // optionally retieve vert pointing results

  _meanZdrmVert = NAN;
  if (_params.read_vert_point_from_spdb) {
    if (_retrieveVertPointingFromSpdb(vol, _meanZdrmVert) == 0) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> vert point meanZdrm: " << _meanZdrmVert << endl;
      }
    }
  }

  // optionally retieve sunscan results

  _sunscanZdrm = NAN;
  _sunscanS1S2 = NAN;
  _sunscanQuadPowerDbm = NAN;
  if (_params.read_sunscan_from_spdb) {
    if (_retrieveSunscanFromSpdb(vol, _sunscanS1S2, _sunscanQuadPowerDbm) == 0) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> sunscan S1S2, quadPowerDbm: "
             << _sunscanS1S2 << ", "
             << _sunscanQuadPowerDbm << endl;
      }
    }
  }

  // use floats

  vol.convertToFl32();

  // initialize

  _nPairsClutter = 0;
  _sumRatioHxVxDbClutter = 0.0;
  _nPairsWeather = 0;
  _sumRatioHxVxDbWeather = 0.0;

  _minValidRatioDb = _params.min_valid_ratio_db;
  _maxValidRatioDb = _params.max_valid_ratio_db;

  // get receiver gain from calibration

  double receiverGainDbHx = _calib.getReceiverGainDbHx();
  double receiverGainDbVx = _calib.getReceiverGainDbVx();
  
  _calXmitPowerDbmH = NAN;
  _calXmitPowerDbmV = NAN;
  
  if (vol.getNRcalibs() > 0) {
    const vector<RadxRcalib *> &calibs = vol.getRcalibs();
    receiverGainDbHx = calibs[0]->getReceiverGainDbHx();
    receiverGainDbVx = calibs[0]->getReceiverGainDbVx();
    _calXmitPowerDbmH = calibs[0]->getXmitPowerDbmH();
    _calXmitPowerDbmV = calibs[0]->getXmitPowerDbmV();
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> From cal:" << endl;
    cerr << "    receiverGainDbHx: " << receiverGainDbHx << endl;
    cerr << "    receiverGainDbVx: " << receiverGainDbVx << endl;
  }
  
  // loop through rays

  for (size_t iray = 0; iray < vol.getNRays(); iray++) {

    RadxRay *ray = vol.getRays()[iray];
    
    RadxField *dbmhxField = ray->getField(_params.cfradial_dbmhx_field_name);
    RadxField *dbmvxField = ray->getField(_params.cfradial_dbmvx_field_name);
    RadxField *snrhxField = ray->getField(_params.cfradial_snrhx_field_name);
    RadxField *snrvxField = ray->getField(_params.cfradial_snrvx_field_name);
    RadxField *cpaField = ray->getField(_params.cfradial_cpa_field_name);
    RadxField *cmdField = ray->getField(_params.cfradial_cmd_field_name);
    RadxField *rhoVxHxField = ray->getField(_params.cfradial_rho_vx_hx_field_name);
    
    if (!dbmhxField) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING, no dbmhx field: "
             << _params.cfradial_dbmhx_field_name << ", skipping ray" << endl;
      }
      continue;
    }
    
    if (!dbmvxField) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING, no dbmvx field: "
             << _params.cfradial_dbmvx_field_name << ", skipping ray" << endl;
      }
      continue;
    }
    
    if (!snrhxField) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING, no snrhx field: "
             << _params.cfradial_snrhx_field_name << ", skipping ray" << endl;
      }
      continue;
    }
    
    if (!snrvxField) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING, no snrvx field: "
             << _params.cfradial_snrvx_field_name << ", skipping ray" << endl;
      }
      continue;
    }
    
    if (!cpaField) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING, no cpa field: "
             << _params.cfradial_cpa_field_name << ", skipping ray" << endl;
      }
      continue;
    }
    
    if (!cmdField) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING, no cmd field: "
             << _params.cfradial_cmd_field_name << ", skipping ray" << endl;
      }
      continue;
    }
    
    if (!rhoVxHxField) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING, no rho_vx_hx field: "
             << _params.cfradial_rho_vx_hx_field_name << ", skipping ray" << endl;
      }
      continue;
    }
    
    _cfradialProcessRay(receiverGainDbHx,
                        receiverGainDbVx,
                        ray, 
                        dbmhxField, dbmvxField, 
                        snrhxField, snrvxField, 
                        cpaField, cmdField, rhoVxHxField);
    
  } // iray

  // compute mean ratio and write mean ratio to file

  _writeRatioToFile(vol.getStartTimeSecs());

  if (_params.debug) {
    cerr << "===>> time, nPairsClutter, meanCpRatioClutter, nPairsWx, meanCpRatioWx: " 
         << RadxTime::strm(vol.getStartTimeSecs()) << ", "
         << _nPairsClutter << ", " 
         << _meanRatioHxVxDbClutter << ", "
         << _nPairsWeather << ", " 
         << _meanRatioHxVxDbWeather << endl;
  }

  return 0;

}

///////////////////////////////////////////////////
// process a ray

void AltCpCompute::_cfradialProcessRay(double receiverGainDbHx,
                                       double receiverGainDbVx,
                                       RadxRay *ray, 
                                       RadxField *dbmhxField,
                                       RadxField *dbmvxField,
                                       RadxField *snrhxField,
                                       RadxField *snrvxField,
                                       RadxField *cpaField,
                                       RadxField *cmdField,
                                       RadxField *rhoVxHxField)

{

  RadxTime rayTime(ray->getRadxTime());

  // set up field pointers
  
  const Radx::fl32 *cpaArray = cpaField->getDataFl32();
  const Radx::fl32 *cmdArray = cmdField->getDataFl32();
  const Radx::fl32 *rhoVxHxArray = rhoVxHxField->getDataFl32();

  const Radx::fl32 *dbmhxArray = dbmhxField->getDataFl32();
  const Radx::fl32 *dbmvxArray = dbmvxField->getDataFl32();
  const Radx::fl32 *snrhxArray = snrhxField->getDataFl32();
  const Radx::fl32 *snrvxArray = snrvxField->getDataFl32();

  // loop through gates
  
  for (size_t igate = 0; igate < ray->getNGates(); igate++) {

    // compute ratio

    Radx::fl32 dbmhx = dbmhxArray[igate] + receiverGainDbHx;
    Radx::fl32 dbmvx = dbmvxArray[igate] + receiverGainDbVx;
    double ratioHxVxDb = dbmhx - dbmvx;
    if (ratioHxVxDb < _minValidRatioDb || ratioHxVxDb > _maxValidRatioDb) {
      continue;
    }
    
    // check snr

    Radx::fl32 snrhx = snrhxArray[igate];
    Radx::fl32 snrvx = snrvxArray[igate];
    
    if (snrhx < _params.min_snr || snrhx > _params.max_snr) {
      continue;
    }
    if (snrvx < _params.min_snr || snrvx > _params.max_snr) {
      continue;
    }

    // get other data

    double rangeKm = ray->getStartRangeKm() + igate * ray->getGateSpacingKm();
    double elevationDeg = ray->getElevationDeg();
    double azimuthDeg = ray->getAzimuthDeg();
    
    Radx::fl32 cpa = cpaArray[igate];
    Radx::fl32 cmd = cmdArray[igate];
    Radx::fl32 rhoVxHx = rhoVxHxArray[igate];
    
    // add to stats

    _addGateData(rayTime, elevationDeg, azimuthDeg, igate, rangeKm,
                 cpa, cmd, snrhx, snrvx, rhoVxHx, dbmhx, dbmvx);

  } // igate

}

//////////////////////////////////////////////////
// set up read

void AltCpCompute::_cfradialSetupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }

  file.addReadField(_params.cfradial_dbmhx_field_name);
  file.addReadField(_params.cfradial_dbmvx_field_name);
  file.addReadField(_params.cfradial_snrhx_field_name);
  file.addReadField(_params.cfradial_snrvx_field_name);
  file.addReadField(_params.cfradial_cpa_field_name);
  file.addReadField(_params.cfradial_cmd_field_name);
  file.addReadField(_params.cfradial_rho_vx_hx_field_name);

}

//////////////////////////////////////////////////
// add analysis point

void AltCpCompute::_addGateData(RadxTime rayTime,
                                double elevationDeg,
                                double azimuthDeg,
                                int gateNum,
                                double rangeKm,
                                double cpa,
                                double cmd,
                                double snrhx,
                                double snrvx,
                                double rhoVxHx,
                                double dbmhx,
                                double dbmvx)
  
{

  // check general conditions

  double ratioHxVxDb = dbmhx - dbmvx;
  if (ratioHxVxDb < _minValidRatioDb || ratioHxVxDb > _maxValidRatioDb) {
    return;
  }

  if (snrhx < _params.min_snr || snrhx > _params.max_snr) {
    return;
  }
  if (snrvx < _params.min_snr || snrvx > _params.max_snr) {
    return;
  }
  if (rhoVxHx < _params.min_rho_vx_hx || rhoVxHx > _params.max_rho_vx_hx) {
    return;
  }

  // check for clutter conditions

  if (cmd > -9990) {

    // cmd available so use it

    if (elevationDeg >= _params.min_elevation_deg_for_clutter &&
        elevationDeg <= _params.max_elevation_deg_for_clutter &&
        rangeKm >= _params.min_range_km_for_clutter &&
        rangeKm <= _params.max_range_km_for_clutter &&
        cmd >= _params.min_cmd_for_clutter &&
        cmd <= _params.max_cmd_for_clutter) {
      _sumRatioHxVxDbClutter += ratioHxVxDb;
      _nPairsClutter++;
    }

  } else {

    // cmd missing so use cpa instead

    if (elevationDeg >= _params.min_elevation_deg_for_clutter &&
        elevationDeg <= _params.max_elevation_deg_for_clutter &&
        rangeKm >= _params.min_range_km_for_clutter &&
        rangeKm <= _params.max_range_km_for_clutter &&
        cpa >= _params.min_cpa_for_clutter &&
        cpa <= _params.max_cpa_for_clutter) {
      _sumRatioHxVxDbClutter += ratioHxVxDb;
      _nPairsClutter++;
    }

  }

  // check for weather conditions

  if (cmd > -9990) {

    // cmd available so use it

    if (elevationDeg >= _params.min_elevation_deg_for_weather &&
        elevationDeg <= _params.max_elevation_deg_for_weather &&
        rangeKm >= _params.min_range_km_for_weather &&
        rangeKm <= _params.max_range_km_for_weather &&
        cmd >= _params.min_cmd_for_weather &&
        cmd <= _params.max_cmd_for_weather) {
      _sumRatioHxVxDbWeather += ratioHxVxDb;
      _nPairsWeather++;
    }

  } else {

    // cmd missing so use cpa instead

    if (elevationDeg >= _params.min_elevation_deg_for_weather &&
        elevationDeg <= _params.max_elevation_deg_for_weather &&
        rangeKm >= _params.min_range_km_for_weather &&
        rangeKm <= _params.max_range_km_for_weather &&
        cpa >= _params.min_cpa_for_weather &&
        cpa <= _params.max_cpa_for_weather) {
      _sumRatioHxVxDbWeather += ratioHxVxDb;
      _nPairsWeather++;
    }

  }

  if (elevationDeg >= _params.min_elevation_deg_for_weather &&
      elevationDeg <= _params.max_elevation_deg_for_weather &&
      rangeKm >= _params.min_range_km_for_weather &&
      rangeKm <= _params.max_range_km_for_weather &&
      cpa >= _params.min_cpa_for_weather &&
      cmd <= _params.max_cmd_for_weather) {
    
    _sumRatioHxVxDbWeather += ratioHxVxDb;
    _nPairsWeather++;

  }

  if (_params.write_data_pairs_to_stdout) {
    _writePairData(rayTime,
                   elevationDeg,
                   azimuthDeg,
                   gateNum,
                   rangeKm,
                   snrhx,
                   snrvx,
                   dbmhx,
                   dbmvx,
                   cpa,
                   cmd,
                   rhoVxHx,
                   ratioHxVxDb);
  }
    

}


////////////////////////////////////////////
// compute results

void AltCpCompute::_computeRatio()

{

  if (_nPairsClutter > _params.min_n_pairs_for_clutter) {
    _meanRatioHxVxDbClutter = _sumRatioHxVxDbClutter / _nPairsClutter;
  } else {
    _meanRatioHxVxDbClutter = NAN;
  }

  if (_nPairsWeather > _params.min_n_pairs_for_weather) {
    _meanRatioHxVxDbWeather = _sumRatioHxVxDbWeather / _nPairsWeather;
  } else {
    _meanRatioHxVxDbWeather = NAN;
  }

  _sunscanZdrm = -1.0 * (_sunscanS1S2 + _meanRatioHxVxDbClutter);

}
  
////////////////////////////////////////////
// clear stats info

void AltCpCompute::_clearRatio()

{

  _nPairsClutter = 0;
  _sumRatioHxVxDbClutter = 0;
  _meanRatioHxVxDbClutter = NAN;

  _nPairsWeather = 0;
  _sumRatioHxVxDbWeather = 0;
  _meanRatioHxVxDbWeather = NAN;

}
  
///////////////////////////////
// write out results to files

int AltCpCompute::_writeRatioToFile(const RadxTime &rtime)

{

  _computeRatio();

  if (_ratioFile == NULL) {
    if (_openRatioFile()) {
      return -1;
    }
    if (_params.write_metadata_results_to_file) {
      _writeRatioHeader();
    }
  }
  
  _doWriteRatio(rtime);

  return 0;

}

///////////////////////////////
// open results file

int AltCpCompute::_openRatioFile()

{

  if (!_params.write_results_to_file) {
    _ratioFile = stdout;
    return 0;
  }

  if (_ratioFile != NULL) {
    return -1;
  }

  // create the directory for the output files, if needed
  
  if (ta_makedir_recurse(_params.output_dir)) {
    int errNum = errno;
    cerr << "ERROR - AltCpCompute::_openRatioFile";
    cerr << "  Cannot create output dir: " << _params.output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute results file path

  time_t startTime = (time_t) _startTime;
  DateTime ctime(startTime);
  char outPath[1024];
  sprintf(outPath, "%s%s%s_%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
          _params.output_dir,
          PATH_DELIM,
          _params.file_name_prefix,
          ctime.getYear(),
          ctime.getMonth(),
          ctime.getDay(),
          ctime.getHour(),
          ctime.getMin(),
          ctime.getSec());
  
  if (_params.debug) {
    cerr << "-->> Opening results file: " << outPath << endl;
  }

  // open file
  
  if ((_ratioFile = fopen(outPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - AltCpCompute::_openRatioFile";
    cerr << "  Cannot create file: " << outPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  return 0;

}


/////////////////////////////////////////////////
// write header at top of results file

void AltCpCompute::_writeRatioHeader()

{

  // first line indicates column names

  const char *delim = _params.column_delimiter_for_output_files;

  fprintf(_ratioFile, "#%s", delim);
  fprintf(_ratioFile, "obsNum%s", delim);
  fprintf(_ratioFile, "year%s", delim);
  fprintf(_ratioFile, "month%s", delim);
  fprintf(_ratioFile, "day%s", delim);
  fprintf(_ratioFile, "hour%s", delim);
  fprintf(_ratioFile, "min%s", delim);
  fprintf(_ratioFile, "sec%s", delim);
  fprintf(_ratioFile, "unixDay%s", delim);
  fprintf(_ratioFile, "npairsClut%s", delim);
  fprintf(_ratioFile, "cpRatioClut%s", delim);
  fprintf(_ratioFile, "npairsWx%s", delim);
  fprintf(_ratioFile, "cpRatioWx%s", delim);
  fprintf(_ratioFile, "calTxPwrH%s", delim);
  fprintf(_ratioFile, "calTxPwrV%s", delim);

  if (_params.retrieve_vals_from_xml_status) {
    fprintf(_ratioFile, "TxPwrRatioHV%s", delim);
    for (int ii = 0; ii < _params.xml_status_entries_n; ii++) {
      fprintf(_ratioFile, "%s%s", _params._xml_status_entries[ii].label, delim);
    }
  }

  if (_params.read_site_temp_from_spdb) {
    fprintf(_ratioFile, "TempSite%s", delim);
  }
  if (_params.read_vert_point_from_spdb) {
    fprintf(_ratioFile, "ZdrmVert%s", delim);
  }
  if (_params.read_sunscan_from_spdb) {
    fprintf(_ratioFile, "SunscanS1S2%s", delim);
    fprintf(_ratioFile, "SunscanPowerDbm%s", delim);
    fprintf(_ratioFile, "SunscanZdrm%s", delim);
  }
    
  fprintf(_ratioFile, "\n");
  fflush(_ratioFile);

  // followed by metadata

  fprintf(_ratioFile, "#========================================\n");
  fprintf(_ratioFile, "# CP ratio for ZDR calibration\n");
  fprintf(_ratioFile, "#  Start time: %s\n",
          DateTime::strm((time_t) _startTime).c_str());
  if (_params.input_mode != Params::CFRADIAL_INPUT) {
    fprintf(_ratioFile, "#  n samples           : %d\n", _params.n_samples);
  }
  fprintf(_ratioFile, "#  min valid ratio (dB): %g\n", _params.min_valid_ratio_db);
  fprintf(_ratioFile, "#  max valid ratio (dB): %g\n", _params.max_valid_ratio_db);
  fprintf(_ratioFile, "#  min snr (dB)        : %g\n", _params.min_snr);
  fprintf(_ratioFile, "#  max snr (dB)        : %g\n", _params.max_snr);
  fprintf(_ratioFile, "#  min rho             : %g\n", _params.min_rho_vx_hx);
  fprintf(_ratioFile, "#  max rho             : %g\n", _params.max_rho_vx_hx);
  if (_params.input_mode == Params::CFRADIAL_INPUT) {
    fprintf(_ratioFile, "#  min cmd for clut    : %g\n", _params.min_cmd_for_clutter);
    fprintf(_ratioFile, "#  max cmd for clut    : %g\n", _params.max_cmd_for_clutter);
    fprintf(_ratioFile, "#  min cmd for wx      : %g\n", _params.min_cmd_for_weather);
    fprintf(_ratioFile, "#  max cmd for wx      : %g\n", _params.max_cmd_for_weather);
  } else {
    fprintf(_ratioFile, "#  min cpa for clut    : %g\n", _params.min_cpa_for_clutter);
    fprintf(_ratioFile, "#  max cpa for clut    : %g\n", _params.max_cpa_for_clutter);
    fprintf(_ratioFile, "#  min cpa for wx      : %g\n", _params.min_cpa_for_weather);
    fprintf(_ratioFile, "#  max cpa for wx      : %g\n", _params.max_cpa_for_weather);
  }
  fprintf(_ratioFile, "#  min range for clut  : %g\n", _params.min_range_km_for_clutter);
  fprintf(_ratioFile, "#  max range for clut  : %g\n", _params.max_range_km_for_clutter);
  fprintf(_ratioFile, "#  min range for wx    : %g\n", _params.min_range_km_for_weather);
  fprintf(_ratioFile, "#  max range for wx    : %g\n", _params.max_range_km_for_weather);
  fprintf(_ratioFile, "#  min elev for clut   : %g\n", _params.min_elevation_deg_for_clutter);
  fprintf(_ratioFile, "#  max elev for clut   : %g\n", _params.max_elevation_deg_for_clutter);
  fprintf(_ratioFile, "#  min elev for wx     : %g\n", _params.min_elevation_deg_for_weather);
  fprintf(_ratioFile, "#  max elev for wx     : %g\n", _params.max_elevation_deg_for_weather);
  fprintf(_ratioFile, "#========================================\n");

  fflush(_ratioFile);

}

//////////////////////////////////
// write out ratio to results file

void AltCpCompute::_doWriteRatio(const RadxTime &rtime)

{

  if (_nPairsClutter < _params.min_n_pairs_for_clutter &&
      _nPairsWeather < _params.min_n_pairs_for_weather) {
    return;
  }

  _writeCount++;

  const char *delim = _params.column_delimiter_for_output_files;
  fprintf(_ratioFile, "%6d%s", _writeCount, delim);
  fprintf(_ratioFile, "%.4d%s", rtime.getYear(), delim);
  fprintf(_ratioFile, "%.2d%s", rtime.getMonth(), delim);
  fprintf(_ratioFile, "%.2d%s", rtime.getDay(), delim);
  fprintf(_ratioFile, "%.2d%s", rtime.getHour(), delim);
  fprintf(_ratioFile, "%.2d%s", rtime.getMin(), delim);
  fprintf(_ratioFile, "%.2d%s", rtime.getSec(), delim);
  fprintf(_ratioFile, "%12.6f%s", rtime.utime() / 86400.0, delim);
  fprintf(_ratioFile, "%6d%s", _nPairsClutter, delim);
  fprintf(_ratioFile, "%10.4f%s", _meanRatioHxVxDbClutter, delim);
  fprintf(_ratioFile, "%6d%s", _nPairsWeather, delim);
  fprintf(_ratioFile, "%10.4f%s", _meanRatioHxVxDbWeather, delim);
  fprintf(_ratioFile, "%10.4f%s", _calXmitPowerDbmH, delim);
  fprintf(_ratioFile, "%10.4f%s", _calXmitPowerDbmV, delim);
  
  if (_params.retrieve_vals_from_xml_status) {

    // get the transmit powers
    
    _getXmitPowers(rtime);
    fprintf(_ratioFile, "%10.4f%s", _measTxPwrRatioHV, delim);
    
    for (size_t ii = 0; ii < _statusVals.size(); ii++) {
      fprintf(_ratioFile, "%10.4f%s", _statusVals[ii], delim);
    } // ii

  }

  if (_params.read_site_temp_from_spdb) {
    fprintf(_ratioFile, "%10.4f%s", _siteTempC, delim);
  }
  if (_params.read_vert_point_from_spdb) {
    fprintf(_ratioFile, "%10.4f%s", _meanZdrmVert, delim);
  }
  if (_params.read_sunscan_from_spdb) {
    fprintf(_ratioFile, "%10.4f%s", _sunscanS1S2, delim);
    fprintf(_ratioFile, "%10.4f%s", _sunscanQuadPowerDbm, delim);
    fprintf(_ratioFile, "%10.4f%s", _sunscanZdrm, delim);
  }
    
  fprintf(_ratioFile, "\n");
  fflush(_ratioFile);

  if (_params.write_ratio_to_spdb) {
    _writeRatioToSpdb(rtime);
  }

}

//////////////////////////////////////////////
// fix transmit powers
// for certain dates these are switched

void AltCpCompute::_getXmitPowers(const RadxTime &rtime)

{

  // get the transmit powers

  double txPwrH = NAN;
  double txPwrV = NAN;
  _measTxPwrRatioHV = NAN;
  
  for (int ii = 0; ii < _params.xml_status_entries_n; ii++) {
    if (strcmp(_params._xml_status_entries[ii].label, "TxPwrH") == 0) {
      if(ii < (int) _statusVals.size() - 1) {
        txPwrH = _statusVals[ii];
      }
    }
    if (strcmp(_params._xml_status_entries[ii].label, "TxPwrV") == 0) {
      if(ii < (int) _statusVals.size() - 1) {
        txPwrV = _statusVals[ii];
      }
    }
  }

  // sanity check
  if (txPwrH > 80 && txPwrV > 80 &&
      txPwrH < 95 && txPwrV < 95) {
    _measTxPwrRatioHV = txPwrH - txPwrV;
  } else {
    txPwrH = NAN;
    txPwrV = NAN;
  }

  // The tranmit power measurements were switched between May 1 and Jun 6 2015
  // so we correct for that here

  RadxTime switchedPeriodStart = RadxTime(2015, 05, 01, 00, 00, 00);
  RadxTime SwitchedPeriodEnd = RadxTime(2015, 06, 07, 00, 00, 00);

  if (rtime > switchedPeriodStart && rtime < SwitchedPeriodEnd) {

    double tmp = txPwrH;
    txPwrH = txPwrV;
    txPwrV = tmp;
    _measTxPwrRatioHV = txPwrH - txPwrV;

  } // if (rtime ...

  // set xmit powers back in status values

  for (int ii = 0; ii < _params.xml_status_entries_n; ii++) {
    if (strcmp(_params._xml_status_entries[ii].label, "TxPwrH") == 0) {
      if(ii < (int) _statusVals.size() - 1) {
        _statusVals[ii] = txPwrH;
      }
    }
    if (strcmp(_params._xml_status_entries[ii].label, "TxPwrV") == 0) {
      if(ii < (int) _statusVals.size() - 1) {
        _statusVals[ii] = txPwrV;
      }
    }
  }
    
}


/////////////////////////////////////////////////
// write running summary of results

int AltCpCompute::_writeRunningSummary()
  
{
  
  _computeRatio();
  
  char resultsStr[10000];

  sprintf(resultsStr,
          "Time,nPairsClutter,ratioClutter,nPairsWeather,ratioWeather: %s %6d %10.3f %6d %10.3f",
          DateTime::strm(_latestPulseTime).c_str(),
          _nPairsClutter,
          _meanRatioHxVxDbClutter,
          _nPairsWeather,
          _meanRatioHxVxDbWeather);
  
  if (_params.write_running_results_to_stdout) {
    fprintf(stdout, "%s\n", resultsStr);
  }
  
  if (_ratioFile == NULL) {
    if (_openRatioFile()) {
      return -1;
    }
  }
    
  fprintf(_ratioFile, "%s\n", resultsStr);
  fflush(_ratioFile);

  _clearRatio();

  return 0;

}

/////////////////////////////////////////////////
// write pair header line to stdout

void AltCpCompute::_writePairHeader()
  
{

  if (!_params.write_column_header_for_pair_data) {
    return;
  }

  const char *delim = _params.column_delimiter_for_output_files;
  
  cout << "year" << delim;
  cout << "month" << delim;
  cout << "day" << delim;
  cout << "hour" << delim;
  cout << "min" << delim;
  cout << "sec" << delim;
  cout << "subsec" << delim;
  cout << "elev" << delim;
  cout << "az" << delim;
  cout << "gateNum" << delim;
  cout << "rangeKm" << delim;
  cout << "snrHx" << delim;
  cout << "snrVx" << delim;
  cout << "dbmHx" << delim;
  cout << "dbmVx" << delim;
  cout << "cpa" << delim;
  cout << "cmd" << delim;
  cout << "rhoVxHx" << delim;
  cout << "ratioHxVxDb" << endl;
  
  cout << flush;

}

/////////////////////////////////////////////////
// write pair data to stdout

void AltCpCompute::_writePairData(RadxTime &rayTime,
                                  double el,
                                  double az,
                                  int gateNum,
                                  double rangeKm,
                                  double snrHx,
                                  double snrVx,
                                  double dbmHx,
                                  double dbmVx,
                                  double cpa,
                                  double cmd,
                                  double rhoVxHx,
                                  double ratioHxVxDb)
  
{

  const char *delim = _params.column_delimiter_for_output_files;

  fprintf(stdout, "%.4d%s", rayTime.getYear(), delim);
  fprintf(stdout, "%.2d%s", rayTime.getMonth(), delim);
  fprintf(stdout, "%.2d%s", rayTime.getDay(), delim);
  fprintf(stdout, "%.2d%s", rayTime.getHour(), delim);
  fprintf(stdout, "%.2d%s", rayTime.getMin(), delim);
  fprintf(stdout, "%.2d%s", rayTime.getSec(), delim);
  fprintf(stdout, "%6.3f%s", rayTime.getSubSec(), delim);

  fprintf(stdout, "%8.3f%s", el, delim);
  fprintf(stdout, "%8.3f%s", az, delim);
  fprintf(stdout, "%8d%s", gateNum, delim);
  fprintf(stdout, "%8.3f%s", rangeKm, delim);
  fprintf(stdout, "%8.3f%s", snrHx, delim);
  fprintf(stdout, "%8.3f%s", snrVx, delim);
  fprintf(stdout, "%8.3f%s", dbmHx, delim);
  fprintf(stdout, "%8.3f%s", dbmVx, delim);
  fprintf(stdout, "%8.3f%s", cpa, delim);
  if (cmd < 0.0) {
    cmd = 0.0;
  }
  fprintf(stdout, "%8.3f%s", cmd, delim);
  fprintf(stdout, "%8.3f%s", rhoVxHx, delim);
  fprintf(stdout, "%8.3f%s", ratioHxVxDb, delim);

  fprintf(stdout, "\n");

  fflush(stdout);

}

/////////////////////////////////////////////////////////////
// write ratio results to SPDB in XML

int AltCpCompute::_writeRatioToSpdb(const RadxTime &rtime)

{

  string xml;

  xml += TaXml::writeStartTag("CrossPolarData", 0);

  xml += TaXml::writeInt("nPairsClutter", 1, _nPairsClutter);
  xml += TaXml::writeDouble("ratioHxVxDbClutter", 1, _meanRatioHxVxDbClutter);

  xml += TaXml::writeInt("nPairsWeather", 1, _nPairsWeather);
  xml += TaXml::writeDouble("ratioHxVxDbWeather", 1, _meanRatioHxVxDbWeather);

  xml += TaXml::writeDouble("calTxPwrH", 1, _calXmitPowerDbmH);
  xml += TaXml::writeDouble("calTxPwrV", 1, _calXmitPowerDbmV);

  if (_params.retrieve_vals_from_xml_status) {
    for (size_t ii = 0; ii < _statusVals.size(); ii++) {
      xml += TaXml::writeDouble(_statusLabels[ii], 1, _statusVals[ii]);
    } // ii
  }

  if (_params.read_site_temp_from_spdb) {
    xml += TaXml::writeDouble("TempSite", 1, _siteTempC);
  }
  if (_params.read_vert_point_from_spdb) {
    xml += TaXml::writeDouble("ZdrmVert", 1, _meanZdrmVert);
  }
  if (_params.read_sunscan_from_spdb) {
    xml += TaXml::writeDouble("SunscanS1S2", 1, _sunscanS1S2);
    xml += TaXml::writeDouble("SunscanPowerDbm", 1, _sunscanQuadPowerDbm);
    xml += TaXml::writeDouble("SunscanZdrm", 1, _sunscanZdrm);
  }


  xml += TaXml::writeEndTag("CrossPolarData", 0);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Writing XML results to SPDB:" << endl;
    cerr << xml << endl;
  }

  DsSpdb spdb;
  time_t validTime = rtime.utime();
  spdb.addPutChunk(0, validTime, validTime, xml.size() + 1, xml.c_str());
  if (spdb.put(_params.spdb_output_url,
               SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - AltCpCompute::_writeSummaryToSpdb" << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Wrote to spdb, url: " << _params.spdb_output_url << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// retrieve values from XML status

void AltCpCompute::_retrieveValsFromXmlStatus(const RadxVol &vol)

{

  _statusVals.clear();
  _statusLabels.clear();

  // set the xml string from the status xml in the volume

  string xml(vol.getStatusXml());

  // loop through XML entries

  for (int ii = 0; ii < _params.xml_status_entries_n; ii++) {
    
    // delimiter
    // fprintf(out, "%s", _params.column_delimiter);

    const Params::xml_entry_t &entry = _params._xml_status_entries[ii];
    
    double val = NAN;
    string label = entry.label;

    // get tag list
    
    vector<string> tags;
    TaStr::tokenize(entry.xml_tag_list, "<>", tags);
    if (tags.size() == 0) {
      continue;
    }

    // set label from tags if required

    if (!entry.specify_label) {
      label.clear();
      for (size_t jj = 0; jj < tags.size(); jj++) {
        label.append(tags[jj]);
        if (jj != tags.size() - 1) {
          label.append("-");
        }
      }
    }

    // drill down to inner tag block we want
    
    string buf(xml);
    bool foundInnerTag = true;
    for (size_t jj = 0; jj < tags.size() - 1; jj++) {
      string valStr;
      if (TaXml::readString(buf, tags[jj], valStr) == 0) {
        buf = valStr;
      } else {
        foundInnerTag = false;
        break;
      }
    }
    if (!foundInnerTag) {
      // missing
      _statusVals.push_back(val);
      _statusLabels.push_back(label);
      continue;
    }

    // find value
    
    string valStr;
    if (TaXml::readString(buf, tags[tags.size()-1], valStr)) {
      // missing
      _statusVals.push_back(val);
      _statusLabels.push_back(label);
      continue;
    }

    if (STRequal(valStr.c_str(), "TRUE")) {
      val = 1.0;
    } else if (STRequal(valStr.c_str(), "FALSE")) {
      val = 0.0;
    } else{
      double tmp;
      if (sscanf(valStr.c_str(), "%lg", &tmp) == 1) {
        val = tmp;
      }
    }

    _statusVals.push_back(val);
    _statusLabels.push_back(label);
    
  } // ii

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < _statusVals.size(); ii++) {
      cerr << "====>> status Label, Val: " << _statusLabels[ii] << ", " << _statusVals[ii] << endl;
    } // ii
  }
  
}

//////////////////////////////////////////////////
// retrieve site temp from SPDB for volume time

int AltCpCompute::_retrieveSiteTempFromSpdb(const RadxVol &vol,
                                            double &tempC,
                                            time_t &timeForTemp)
  
{

  // get surface data from SPDB

  DsSpdb spdb;
  si32 dataType = 0;
  if (strlen(_params.site_temp_station_name) > 0) {
    dataType =  Spdb::hash4CharsToInt32(_params.site_temp_station_name);
  }
  time_t searchTime = vol.getStartTimeSecs();

  if (spdb.getClosest(_params.site_temp_spdb_url,
                      searchTime,
                      _params.site_temp_search_margin_secs,
                      dataType)) {
    cerr << "WARNING - AltCpCompute::_retrieveSiteTempFromSpdb" << endl;
    cerr << "  Cannot get temperature from URL: " << _params.site_temp_spdb_url << endl;
    cerr << "  Station name: " << _params.site_temp_station_name << endl;
    cerr << "  Search time: " << RadxTime::strm(searchTime) << endl;
    cerr << "  Search margin (secs): " << _params.site_temp_search_margin_secs << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  // got chunks
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  if (chunks.size() < 1) {
    cerr << "WARNING - AltCpCompute::_retrieveSiteTempFromSpdb" << endl;
    cerr << "  No suitable temp data from URL: " << _params.site_temp_spdb_url << endl;
    cerr << "  Search time: " << RadxTime::strm(searchTime) << endl;
    cerr << "  Search margin (secs): " << _params.site_temp_search_margin_secs << endl;
    return -1;
  }

  const Spdb::chunk_t &chunk = chunks[0];
  WxObs obs;
  if (obs.disassemble(chunk.data, chunk.len)) {
    cerr << "WARNING - AltCpCompute::_retrieveSiteTempFromSpdb" << endl;
    cerr << "  SPDB data is of incorrect type, prodLabel: " << spdb.getProdLabel() << endl;
    cerr << "  Should be station data type" << endl;
    cerr << "  URL: " << _params.site_temp_spdb_url << endl;
    return -1;
  }

  tempC = obs.getTempC();
  timeForTemp = obs.getObservationTime();

  return 0;

}

//////////////////////////////////////////////////
// retrieve vertical pointing results from SPDB

int AltCpCompute::_retrieveVertPointingFromSpdb(const RadxVol &vol,
                                                double &meanZdrm)
  
{

  // initialize

  meanZdrm = NAN;

  // get data from SPDB, for interval around vol time

  DsSpdb spdb;
  si32 dataType = 0;
  if (strlen(_params.vert_point_radar_name) > 0) {
    dataType =  Spdb::hash4CharsToInt32(_params.vert_point_radar_name);
  }
  time_t startTime = vol.getStartTimeSecs() - _params.vert_point_search_margin_secs;
  time_t endTime = vol.getStartTimeSecs() + _params.vert_point_search_margin_secs;

  if (spdb.getInterval(_params.vert_point_spdb_url,
                       startTime, endTime,
                       dataType)) {
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "WARNING - AltCpCompute::_retrieveVertPointingFromSpdb" << endl;
      cerr << "  Cannot get vert point data: " << _params.vert_point_spdb_url << endl;
      cerr << "  StartTime: " << RadxTime::strm(startTime) << endl;
      cerr << "  EndTime: " << RadxTime::strm(endTime) << endl;
      cerr << spdb.getErrStr() << endl;
    }
    return -1;
  }
  
  // got chunks
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  if (chunks.size() < 1) {
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "WARNING - AltCpCompute::_retrieveVertPointingFromSpdb" << endl;
      cerr << "  No vert point data found: " << _params.vert_point_spdb_url << endl;
      cerr << "  StartTime: " << RadxTime::strm(startTime) << endl;
      cerr << "  EndTime: " << RadxTime::strm(endTime) << endl;
    }
    return -1;
  }
  
  // loop through chunks to compute mean

  double sum = 0.0;
  double count = 0.0;

  for (size_t ii = 0; ii < chunks.size(); ii++) {

    const Spdb::chunk_t &chunk = chunks[ii];
    string xml((const char *) chunk.data);

    double zdrm = 0.0;
    if (TaXml::readDouble(xml, "meanZdrm", zdrm)) {
      continue;
    }

    int nn = 0;
    if (TaXml::readInt(xml, "countZdrm", nn)) {
      continue;
    }

    if (nn < _params.vert_point_min_valid_count) {
      continue;
    }
    
    sum += zdrm * nn;
    count += nn;

  }
  
  if (count < _params.vert_point_min_valid_count) {
    return -1;
  }

  meanZdrm = sum / count;

  return 0;

}

//////////////////////////////////////////////////
// retrieve sunscan results from SPDB

int AltCpCompute::_retrieveSunscanFromSpdb(const RadxVol &vol,
                                           double &S1S2,
                                           double &quadPowerDbm)
  
{

  // initialize
  
  S1S2 = NAN;
  quadPowerDbm = NAN;

  // get data from SPDB, for interval around vol time

  DsSpdb spdb;
  si32 dataType = 0;
  time_t startTime = vol.getStartTimeSecs() - _params.sunscan_search_margin_secs;
  time_t endTime = vol.getStartTimeSecs() + _params.sunscan_search_margin_secs;
  
  if (spdb.getInterval(_params.sunscan_spdb_url,
                       startTime, endTime,
                       dataType)) {
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "WARNING - AltCpCompute::_retrieveSunscanFromSpdb" << endl;
      cerr << "  Cannot get sunscan data: " << _params.sunscan_spdb_url << endl;
      cerr << "  StartTime: " << RadxTime::strm(startTime) << endl;
      cerr << "  EndTime: " << RadxTime::strm(endTime) << endl;
      cerr << spdb.getErrStr() << endl;
    }
    return -1;
  }
  
  // got chunks
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  if (chunks.size() < 1) {
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "WARNING - AltCpCompute::_retrieveSunscanFromSpdb" << endl;
      cerr << "  No sunscan data available: " << _params.sunscan_spdb_url << endl;
      cerr << "  StartTime: " << RadxTime::strm(startTime) << endl;
      cerr << "  EndTime: " << RadxTime::strm(endTime) << endl;
    }
    return -1;
  }
  
  // loop through chunks to compute mean

  double sumS1S2 = 0.0;
  double sumPower = 0.0;
  double count = 0.0;
  
  for (size_t ii = 0; ii < chunks.size(); ii++) {
    
    const Spdb::chunk_t &chunk = chunks[ii];
    string xml((const char *) chunk.data);
    
    double s1s2 = 0.0;
    if (TaXml::readDouble(xml, "S1S2", s1s2)) {
      continue;
    }
    double power = 0.0;
    if (TaXml::readDouble(xml, "quadPowerDbm", power)) {
      continue;
    }
    int nn = 0;
    if (TaXml::readInt(xml, "nXpolPoints", nn)) {
      continue;
    }
    if (nn < _params.sunscan_min_valid_count) {
      continue;
    }

    sumS1S2 += s1s2 * nn;
    sumPower += power * nn;
    count += nn;

  }
  
  if (count < _params.sunscan_min_valid_count) {
    return -1;
  }

  S1S2 = sumS1S2 / count;
  quadPowerDbm = sumPower / count;

  return 0;

}

