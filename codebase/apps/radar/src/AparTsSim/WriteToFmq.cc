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
// WriteToFmq.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2020
//
///////////////////////////////////////////////////////////////
//
// Resample IWRF time series data,
// convert to APAR format,
// and write out to FMQ
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <cstdio>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#ifdef __linux
#include <arpa/inet.h>
#endif

#include <dataport/bigend.h>
#include <dataport/swap.h>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <toolsa/file_io.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/str.h>
#include <toolsa/MemBuf.hh>

#include "WriteToFmq.hh"
#include "AparTsSim.hh"

using namespace std;

// Constructor

WriteToFmq::WriteToFmq(const string &progName,
                       const Params &params,
                       vector<string> &inputFileList) :
        _progName(progName),
        _params(params),
        _inputFileList(inputFileList)
  
{

  // debug print

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Running WriteToFmq - extra verbose debug mode" << endl;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running WriteToFmq - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running WriteToFmq - debug mode" << endl;
  }

  _aparTsDebug = AparTsDebug_t::OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _aparTsDebug = AparTsDebug_t::VERBOSE;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    _aparTsDebug = AparTsDebug_t::NORM;
  }
  _aparTsInfo = new AparTsInfo(_aparTsDebug);

  // init

  _pulseSeqNum = 0;
  _dwellSeqNum = 0;
  _rateStartTime.set(RadxTime::NEVER);
  _nBytesForRate = 0;
  _realtimeDeltaSecs = 0;
  _metaCount = 0;
  _volNum = 0;
  _sweepNum = 0;

  // compute the scan strategy

  _strategy = new SimScanStrategy(_params);
  
}

// destructor

WriteToFmq::~WriteToFmq()
  
{
  
  delete _strategy;
  
  // delete pulses to free memory
  
  for (size_t ii = 0; ii < _dwellPulses.size(); ii++) {
    delete _dwellPulses[ii];
  }
  _dwellPulses.clear();

}

//////////////////////////////////////////////////
// Run

int WriteToFmq::Run ()
{

  PMU_auto_register("WriteToFmq::Run");
  
  // initialize the output FMQ

  if (_openOutputFmq()) {
    return -1;
  }

  // this is a simulation mode
  // loop through the input files, and repeat
  
  int iret = 0;
  while (true) {

    if (_params.debug) {
      cerr << "N input files: " << _inputFileList.size() << endl;
    }
    
    // initialize time delta for realtime correction
    // this is redone every time we go through the file list

    _realtimeDeltaSecs = 0;

    // loop through files

    for (size_t ii = 0; ii < _inputFileList.size(); ii++) {
      if (_convertToFmq(_inputFileList[ii])) {
        iret = -1;
      }
    }

    umsleep(1000);

  } // while
  
  return iret;

}

////////////////////////////////////////////////////
// Convert 1 file to FMQ

int WriteToFmq::_convertToFmq(const string &inputPath)
  
{
  
  PMU_auto_register("WriteToFmq::_convertToFmq");
  
  if (_params.debug) {
    cerr << "Reading input file: " << inputPath << endl;
  }

  // set up a vector with a single file entry
  
  vector<string> fileList;
  fileList.push_back(inputPath);

  // create reader for just that one file

  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  IwrfTsReaderFile reader(fileList, iwrfDebug);
  const IwrfTsInfo &tsInfo = reader.getOpsInfo();

  // read through pulses until we have current metadata
  
  {
    IwrfTsPulse *iwrfPulse = reader.getNextPulse();
    // change to realtime if appropriate
    if (_params.fmq_set_times_to_now && _realtimeDeltaSecs == 0) {
      time_t now = time(NULL);
      _realtimeDeltaSecs = now - iwrfPulse->getTime();
      si64 newTime = iwrfPulse->getTime() + _realtimeDeltaSecs;
      if (_params.debug) {
        cerr << "====>> recomputing pulse time offset, newTime: "
             << RadxTime::strm(newTime) << endl;
      }
    }
    bool haveMetadata = false;
    while (iwrfPulse != NULL) {
      if (tsInfo.isRadarInfoActive() &&
          tsInfo.isScanSegmentActive() &&
          tsInfo.isTsProcessingActive()) {
        // we have the necessary metadata
        haveMetadata = true;
        delete iwrfPulse;
        break;
      }
      delete iwrfPulse;
    }
    if (!haveMetadata) {
      cerr << "ERROR - WriteToFmq::_convertToFmq()" << endl;
      cerr << "Metadata missing for file: " << inputPath << endl;
      return -1;
    }
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_radar_info_print(stderr, tsInfo.getRadarInfo());
    iwrf_scan_segment_print(stderr, tsInfo.getScanSegment());
    iwrf_ts_processing_print(stderr, tsInfo.getTsProcessing());
    if (tsInfo.isCalibrationActive()) {
      iwrf_calibration_print(stderr, tsInfo.getCalibration());
    }
  }
  _initMetaData(tsInfo);

  // reset reader queue to start

  reader.reset();

  // reset scan strategy to start
  
  _strategy->resetToStart();
  
  // loop through all pulses in file

  while (true) {

    // get a new sim angle
    
    SimScanStrategy::angle_t angle = _strategy->getNextAngle();
    if (angle.startOfDwell) {
      _dwellSeqNum++;
    }
    if (angle.sweepNum < _sweepNum) {
      // end of volume
      _volNum++;
    }
    _sweepNum = angle.sweepNum;

    // loop through pulses for visiting this angle

    for (int ipulse = 0; ipulse < _params.n_samples_per_visit; ipulse++) {
    
      PMU_auto_register("reading pulse");
      IwrfTsPulse *iwrfPulse = reader.getNextPulse();
      if (iwrfPulse == NULL) {
        return 0;
      }
      
      // convert pulse data to floats
    
      iwrfPulse->convertToFL32();

      // process this pulse

      _processPulse(ipulse, iwrfPulse, angle);

    } // ipulse

  } // while (true)

  // should not reach here

  return -1;

}
    
////////////////////////////////////////////
// process pulse

int WriteToFmq::_processPulse(int pulseIndex, IwrfTsPulse *iwrfPulse,
                              SimScanStrategy::angle_t &angle)
  
{
  
  si64 secondsTime = iwrfPulse->getTime() + _realtimeDeltaSecs;
  ui32 nanoSecs = iwrfPulse->getNanoSecs();
  
  // add metadata to outgoing message
  
  if (_metaCount % _params.n_pulses_per_info == 0) {
    _addMetaDataToMsg();
  }
  _metaCount++;
        
  // create the outgoing co-polar pulse
  
  AparTsPulse copolPulse(*_aparTsInfo, _aparTsDebug);
  copolPulse.setTime(secondsTime, nanoSecs);
        
  copolPulse.setBeamNumInDwell(angle.beamNumInDwell);
  copolPulse.setVisitNumInBeam(angle.visitNumInBeam);
  copolPulse.setPulseSeqNum(_pulseSeqNum);
  copolPulse.setDwellSeqNum(_dwellSeqNum);
  
  if (angle.sweepMode == Radx::SWEEP_MODE_RHI) {
    copolPulse.setScanMode((int) apar_ts_scan_mode_t::RHI);
    copolPulse.setFixedAngle(angle.az);
  } else {
    copolPulse.setScanMode((int) apar_ts_scan_mode_t::PPI);
    copolPulse.setFixedAngle(angle.el);
  }
  
  copolPulse.setSweepNum(_sweepNum);
  copolPulse.setVolumeNum(_volNum);
  copolPulse.setElevation(angle.el);
  copolPulse.setAzimuth(angle.az);
  copolPulse.setPrt(iwrfPulse->getPrt());
  copolPulse.setPrtNext(iwrfPulse->get_prt_next());
  copolPulse.setPulseWidthUs(iwrfPulse->getPulseWidthUs());
  copolPulse.setNGates(iwrfPulse->getNGates());
  copolPulse.setNChannels(1);
  copolPulse.setIqEncoding((int) apar_ts_iq_encoding_t::FL32);
  copolPulse.setHvFlag(iwrfPulse->isHoriz());
  copolPulse.setChanIsCopol(0, true);
  copolPulse.setPhaseCohered(iwrfPulse->get_phase_cohered());
  copolPulse.setStatus(iwrfPulse->get_status());
  copolPulse.setNData(iwrfPulse->get_n_data());
  copolPulse.setScale(1.0);
  copolPulse.setOffset(0.0);
  copolPulse.setStartRangeM(iwrfPulse->get_start_range_m());
  copolPulse.setGateGSpacineM(iwrfPulse->get_gate_spacing_m());
  
  _pulseSeqNum++;
  
  // add the co-pol IQ channel data as floats
  
  {
    MemBuf iqBuf;
    fl32 **iqChans = iwrfPulse->getIqArray();
    iqBuf.add(iqChans[0], iwrfPulse->getNGates() * 2 * sizeof(fl32));
    copolPulse.setIqFloats(iwrfPulse->getNGates(), 1,
                           (const fl32 *) iqBuf.getPtr());
  }
  
  // add the pulse to the outgoing message
  
  MemBuf copolBuf;
  copolPulse.assemble(copolBuf);
  _outputMsg.addPart(APAR_TS_PULSE_HEADER_ID,
                     copolBuf.getLen(),
                     copolBuf.getPtr());
  
  // optionally add a cross-pol pulse
  
  if (_params.add_cross_pol_sample_at_end_of_visit &&
      pulseIndex == _params.n_samples_per_visit - 1) {
    
    AparTsPulse xpolPulse(*_aparTsInfo, _aparTsDebug);
    xpolPulse.setTime(secondsTime, nanoSecs);
    
    xpolPulse.setBeamNumInDwell(angle.beamNumInDwell);
    xpolPulse.setVisitNumInBeam(angle.visitNumInBeam);
    xpolPulse.setPulseSeqNum(_pulseSeqNum);
    xpolPulse.setDwellSeqNum(_dwellSeqNum);
    
    if (angle.sweepMode == Radx::SWEEP_MODE_RHI) {
      xpolPulse.setScanMode((int) apar_ts_scan_mode_t::RHI);
      xpolPulse.setFixedAngle(angle.az);
    } else {
      xpolPulse.setScanMode((int) apar_ts_scan_mode_t::PPI);
      xpolPulse.setFixedAngle(angle.el);
    }
    
    xpolPulse.setSweepNum(_sweepNum);
    xpolPulse.setVolumeNum(_volNum);
    xpolPulse.setElevation(angle.el);
    xpolPulse.setAzimuth(angle.az);
    xpolPulse.setPrt(iwrfPulse->getPrt());
    xpolPulse.setPrtNext(iwrfPulse->get_prt_next());
    xpolPulse.setPulseWidthUs(iwrfPulse->getPulseWidthUs());
    xpolPulse.setNGates(iwrfPulse->getNGates());
    xpolPulse.setNChannels(1);
    xpolPulse.setIqEncoding((int) apar_ts_iq_encoding_t::FL32);
    xpolPulse.setHvFlag(iwrfPulse->isHoriz());
    xpolPulse.setChanIsCopol(0, false);
    xpolPulse.setPhaseCohered(iwrfPulse->get_phase_cohered());
    xpolPulse.setStatus(iwrfPulse->get_status());
    xpolPulse.setNData(iwrfPulse->get_n_data());
    xpolPulse.setScale(1.0);
    xpolPulse.setOffset(0.0);
    xpolPulse.setStartRangeM(iwrfPulse->get_start_range_m());
    xpolPulse.setGateGSpacineM(iwrfPulse->get_gate_spacing_m());
    
    _pulseSeqNum++;
    
    // add the co-pol IQ channel data as floats
    
    {
      MemBuf iqBuf;
      fl32 **iqChans = iwrfPulse->getIqArray();
      iqBuf.add(iqChans[1], iwrfPulse->getNGates() * 2 * sizeof(fl32));
      xpolPulse.setIqFloats(iwrfPulse->getNGates(), 1,
                            (const fl32 *) iqBuf.getPtr());
    }
    
    // add the pulse to the outgoing message
    
    MemBuf xpolBuf;
    xpolPulse.assemble(xpolBuf);
    _outputMsg.addPart(APAR_TS_PULSE_HEADER_ID,
                       xpolBuf.getLen(),
                       xpolBuf.getPtr());
    
  } // if (_params.add_cross_pol_sample_at_end_of_visit ...
  
  // write to the queue if ready
  
  if (_writeToOutputFmq(false)) {
    cerr << "ERROR - WriteToFmq::_writePulseToFmq" << endl;
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////////////
// Sleep as required to achieve the desired data rate
// Returns 0 on success, -1 on error

void WriteToFmq::_sleepForDataRate()
  
{

  // get current time

  RadxTime now(RadxTime::NOW);
  double elapsedSecs = now - _rateStartTime;
  if (elapsedSecs < 0.01) {
    return;
  }
  
  // compute time for data sent since last check

  double targetDuration =
    _nBytesForRate / (_params.fmq_sim_data_rate * 1.0e6);
  double sleepSecs = targetDuration - elapsedSecs;

  if (sleepSecs <= 0) {
    // we are not keeping up, so don't sleep
    _rateStartTime = now;
    _nBytesForRate = 0;
    return;
  }
  
  // sleep

  int uSecsSleep = (int) (sleepSecs * 1.0e6 + 0.5);
  uusleep(uSecsSleep);
  
  // reset

  _rateStartTime = now;
  _nBytesForRate = 0;

}

////////////////////////////////////////////////////
// Read the IWRF file, set the APAR-style metadata
// convert the metadata to APAR types
// set the metadata in the info metadata queue

int WriteToFmq::_initMetaData(const IwrfTsInfo &tsInfo)
  
{
  
  _convertMeta2Apar(tsInfo);
  _aparTsInfo->setRadarInfo(_aparRadarInfo);
  _aparTsInfo->setScanSegment(_aparScanSegment);
  _aparTsProcessing.start_range_m = _params.fmq_gate_spacing_m / 2.0;
  _aparTsProcessing.gate_spacing_m = _params.fmq_gate_spacing_m;
  _aparTsInfo->setTsProcessing(_aparTsProcessing);
  if (tsInfo.isCalibrationActive()) {
    _aparTsInfo->setCalibration(_aparCalibration);
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    apar_ts_radar_info_print(stderr, _aparRadarInfo);
    apar_ts_scan_segment_print(stderr, _aparScanSegment);
    apar_ts_processing_print(stderr, _aparTsProcessing);
    if (tsInfo.isCalibrationActive()) {
      apar_ts_calibration_print(stderr, _aparCalibration);
    }
  }

  return 0;
  
}

///////////////////////////////////////////////
// Convert the IWRF metadata to APAR structs

void WriteToFmq::_convertMeta2Apar(const IwrfTsInfo &info)
  
{

  // initialize the apar structs
  
  apar_ts_radar_info_init(_aparRadarInfo);
  apar_ts_scan_segment_init(_aparScanSegment);
  apar_ts_processing_init(_aparTsProcessing);
  apar_ts_calibration_init(_aparCalibration);

  // copy over the metadata members

  AparTsSim::copyIwrf2Apar(info.getRadarInfo(), _aparRadarInfo);
  AparTsSim::copyIwrf2Apar(info.getScanSegment(), _aparScanSegment);
  AparTsSim::copyIwrf2Apar(info.getTsProcessing(), _aparTsProcessing);
  if (info.isCalibrationActive()) {
    AparTsSim::copyIwrf2Apar(info.getCalibration(), _aparCalibration);
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    apar_ts_radar_info_print(stderr, _aparRadarInfo);
    apar_ts_scan_segment_print(stderr, _aparScanSegment);
    apar_ts_processing_print(stderr, _aparTsProcessing);
    apar_ts_calibration_print(stderr, _aparCalibration);
  }

}

///////////////////////////////////////
// open the output FMQ
// returns 0 on success, -1 on failure

int WriteToFmq::_openOutputFmq()

{

  // initialize the output FMQ
  
  if (_outputFmq.initReadWrite
      (_params.output_fmq_path,
       _progName.c_str(),
       _params.debug >= Params::DEBUG_EXTRA, // set debug?
       Fmq::END, // start position
       false,    // compression
       _params.output_fmq_nslots,
       _params.output_fmq_size)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot initialize FMQ: " << _params.output_fmq_path << endl;
    cerr << "  nSlots: " << _params.output_fmq_nslots << endl;
    cerr << "  nBytes: " << _params.output_fmq_size << endl;
    cerr << _outputFmq.getErrStr() << endl;
    return -1;
  }
  _outputFmq.setSingleWriter();
  if (_params.data_mapper_report_interval > 0) {
    _outputFmq.setRegisterWithDmap(true, _params.data_mapper_report_interval);
  }
  if (_params.output_fmq_write_blocking) {
    _outputFmq.setBlockingWrite();
  }

  // initialize message
  
  _outputMsg.clearAll();
  _outputMsg.setType(0);

  return 0;

}

///////////////////////////////////////
// Add metadata to the outgoing message

void WriteToFmq::_addMetaDataToMsg()
{


  // radar info

  _outputMsg.addPart(APAR_TS_RADAR_INFO_ID,
                     sizeof(_aparRadarInfo),
                     &_aparRadarInfo);

  // scan segment

  _outputMsg.addPart(APAR_TS_SCAN_SEGMENT_ID,
                     sizeof(_aparScanSegment),
                     &_aparScanSegment);

  // processing info

  _outputMsg.addPart(APAR_TS_PROCESSING_ID,
                     sizeof(_aparTsProcessing),
                     &_aparTsProcessing);

  // calibration

  _outputMsg.addPart(APAR_TS_CALIBRATION_ID,
                     sizeof(_aparCalibration),
                     &_aparCalibration);


}

///////////////////////////////////////
// write to output FMQ if ready
// returns 0 on success, -1 on failure

int WriteToFmq::_writeToOutputFmq(bool force)

{

  // if the message is large enough, write to the FMQ
  
  int nParts = _outputMsg.getNParts();
  if (!force && nParts < _params.n_pulses_per_message) {
    return 0;
  }

  PMU_auto_register("writeToOutputFmq");

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "========= Output Message =========" << endl;
    _outputMsg.printHeader(cerr);
    _outputMsg.printPartHeaders(cerr);
    cerr << "==================================" << endl;
  }


  void *buf = _outputMsg.assemble();
  int len = _outputMsg.lengthAssembled();
  if (_outputFmq.writeMsg(0, 0, buf, len)) {
    cerr << "ERROR - WriteToFmq" << endl;
    cerr << "  Cannot write FMQ: " << _params.output_fmq_path << endl;
    _outputMsg.clearParts();
    return -1;
  }
  _nBytesForRate += len;
   
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  Wrote msg, nparts, len, path: "
         << nParts << ", " << len << ", "
         << _params.output_fmq_path << endl;
  }

  _outputMsg.clearParts();

  if (_nBytesForRate > 1000000) {
    _sleepForDataRate();
  }

  return 0;

}
    
///////////////////////////////////////
// write end-of-vol to output FMQ
// returns 0 on success, -1 on failure

int WriteToFmq::_writeEndOfVol()

{

  iwrf_event_notice_t notice;
  iwrf_event_notice_init(notice);

  notice.end_of_volume = true;
  notice.volume_num = _volNum;
  notice.sweep_num = _sweepNum;

  _outputMsg.addPart(IWRF_EVENT_NOTICE_ID, sizeof(notice), &notice);

  if (_params.debug) {
    cerr << "Writing end of volume event" << endl;
    iwrf_event_notice_print(stderr, notice);
  }

  if (_writeToOutputFmq(true)) {
    return -1;
  }

  return 0;

}

