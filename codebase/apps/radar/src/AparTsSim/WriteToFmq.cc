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
// convert to APAR UDP format,
// and write out to UDP stream
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
#include <radar/AparTsPulse.hh>

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

  _sampleSeqNum = 0;
  _pulseSeqNum = 0;
  _dwellSeqNum = 0;
  _rateStartTime.set(RadxTime::NEVER);
  _nBytesForRate = 0;
  _realtimeDeltaSecs = 0;
  _nPulsesOut = 0;

  // compute the scan strategy
  
  _simVolNum = 0;
  _simBeamNum = 0;
  _computeScanStrategy();

}

// destructor

WriteToFmq::~WriteToFmq()
  
{
  
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
  
  // compute number of pulses per dwell

  size_t nPulsesPerDwell = 
    _params.n_samples_per_visit *
    _params.n_visits_per_beam *
    _params.n_beams_per_dwell;

  if (_params.debug) {
    cerr << "  ==>> nPulsesPerDwell: " << nPulsesPerDwell << endl;
  }

  // read in all pulses

  IwrfTsPulse *iwrfPulse = reader.getNextPulse();
  while (iwrfPulse != NULL) {
    
    PMU_auto_register("reading pulse");
  
    // convert pulse data to si16 counts
    
    iwrfPulse->convertToFL32();
    
    // add pulse to dwell
    
    _dwellPulses.push_back(iwrfPulse);
    
    // if we have a full dwell, process the pulses in it

    if (_dwellPulses.size() == nPulsesPerDwell) {

      // process dwell
      
      _processDwell(_dwellPulses);
      _dwellSeqNum++;

      // delete pulses to free memory
      
      for (size_t ii = 0; ii < _dwellPulses.size(); ii++) {
        delete _dwellPulses[ii];
      }
      _dwellPulses.clear();

    }
      
    // read next one

    iwrfPulse = reader.getNextPulse();

  } // while

  return 0;
  
}

////////////////////////////////////////////
// process pulses in a dwell for UDP output

int WriteToFmq::_processDwell(vector<IwrfTsPulse *> &dwellPulses)
  
{

  int iret = 0;

  // compute the angles for the beams in the dwell

  double startAz = dwellPulses.front()->getAz();
  double endAz = dwellPulses.back()->getAz();
  double azRange = AparTsSim::conditionAngle360(endAz - startAz);
  double deltaAzPerBeam = azRange / _params.n_beams_per_dwell;

  double startEl = dwellPulses.front()->getEl();
  double endEl = dwellPulses.back()->getEl();
  double elRange = AparTsSim::conditionAngle360(endEl - startEl);
  double deltaElPerBeam = elRange / _params.n_beams_per_dwell;
  
  vector<double> beamAz, beamEl;
  vector<int> sweepNum, volNum;
  vector<Radx::SweepMode_t> sweepMode;
  
  for (int ii = 0; ii < _params.n_beams_per_dwell; ii++) {

    if (!_params.specify_scan_strategy) {

      double az = AparTsSim::conditionAngle360(startAz + 
                                               (ii + 0.5) * deltaAzPerBeam);
      double el = AparTsSim::conditionAngle180(startEl + 
                                               (ii + 0.5) * deltaElPerBeam);
      
      // for APAR, az ranges from -60 to +60
      // and el from -90 to +90
      // so we adjust accordingly
      
      beamAz.push_back((az / 3.0) - 60.0);
      beamEl.push_back(el / 1.5);

    } else {

      if (_simBeamNum >= _simEl.size()) {
        _simBeamNum = 0;
        _simVolNum++;
      }
      
      beamAz.push_back(_simAz[_simBeamNum]);
      beamEl.push_back(_simEl[_simBeamNum]);
      sweepNum.push_back(_simSweepNum[_simBeamNum]);
      volNum.push_back(_simVolNum);
      sweepMode.push_back(_simSweepMode[_simBeamNum]);

    } // if (_params.specify_scan_strategy) 

  } // ii

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-----------------------------------------------" << endl;
    cerr << "startAz, endAz, deltaAzPerBeam: "
         << startAz << ", " << endAz << ", " << deltaAzPerBeam << endl;
    cerr << "startEl, endEl, deltaElPerBeam: "
         << startEl << ", " << endEl << ", " << deltaElPerBeam << endl;
    for (int ii = 0; ii < _params.n_beams_per_dwell; ii++) {
      cerr << "  ii, az, el: "
           << ii << ", " << beamAz[ii] << ", " << beamEl[ii] << endl;
    }
    cerr << "-----------------------------------------------" << endl;
  }

  // loop through all of the pulses

  int pulseNumInDwell = 0;
  for (int ivisit = 0; ivisit < _params.n_visits_per_beam; ivisit++) {
    for (int ibeam = 0; ibeam < _params.n_beams_per_dwell; ibeam++) {
      for (int ipulse = 0; ipulse < _params.n_samples_per_visit; 
           ipulse++, pulseNumInDwell++) {
        
        // set the metadata
        
        IwrfTsPulse *iwrfPulse = dwellPulses[pulseNumInDwell];

        // change to realtime if appropriate

        if (_params.set_udp_time_to_now && _realtimeDeltaSecs == 0) {
          time_t now = time(NULL);
          _realtimeDeltaSecs = now - iwrfPulse->getTime();
          si64 newTime = iwrfPulse->getTime() + _realtimeDeltaSecs;
          if (_params.debug) {
            cerr << "====>> recomputing pulse time offset, newTime: "
                 << RadxTime::strm(newTime) << endl;
          }
        }
        
        si64 secondsTime = iwrfPulse->getTime() + _realtimeDeltaSecs;
        ui32 nanoSecs = iwrfPulse->getNanoSecs();
        
        double az = beamAz[ibeam];
        double el = beamEl[ibeam];

        // add metadata to outgoing message
        
        if (_nPulsesOut % _params.n_pulses_per_info == 0) {
          _addMetaDataToMsg();
        }
        
        // create the outgoing pulse
        
        AparTsPulse pulse(*_aparTsInfo, _aparTsDebug);
        pulse.setTime(secondsTime, nanoSecs);

        pulse.setBeamNumInDwell(ibeam);
        pulse.setVisitNumInBeam(ivisit);
        pulse.setPulseSeqNum(_pulseSeqNum);
        pulse.setDwellSeqNum(_dwellSeqNum);
        
        if (sweepMode[ibeam] == Radx::SWEEP_MODE_RHI) {
          pulse.setScanMode((int) apar_ts_scan_mode_t::RHI);
          pulse.setFixedAngle(az);
        } else {
          pulse.setScanMode((int) apar_ts_scan_mode_t::PPI);
          pulse.setFixedAngle(el);
        }
        pulse.setSweepNum(sweepNum[ibeam]);
        pulse.setVolumeNum(volNum[ibeam]);
        pulse.setElevation(el);
        pulse.setAzimuth(az);
        pulse.setPrt(iwrfPulse->getPrt());
        pulse.setPrtNext(iwrfPulse->get_prt_next());
        pulse.setPulseWidthUs(iwrfPulse->getPulseWidthUs());
        pulse.setNGates(iwrfPulse->getNGates());
        pulse.setNChannels(iwrfPulse->getNChannels());
        pulse.setIqEncoding((int) apar_ts_iq_encoding_t::FL32);
        pulse.setHvFlag(iwrfPulse->isHoriz());
        pulse.setPhaseCohered(iwrfPulse->get_phase_cohered());
        pulse.setStatus(iwrfPulse->get_status());
        pulse.setNData(iwrfPulse->get_n_data());
        pulse.setScale(1.0);
        pulse.setOffset(0.0);
        pulse.setStartRangeM(iwrfPulse->get_start_range_m());
        pulse.setGateGSpacineM(iwrfPulse->get_gate_spacing_m());
        
        // pulse.setStartOfSweep(false);
        // pulse.setStartOfVolume(false);
        // pulse.setEndOfSweep(false);
        // pulse.setEndOfVolume(false);

        _pulseSeqNum++;

        pulse.setIqFloats(iwrfPulse->getNGates(), iwrfPulse->getNChannels(), 
                          (fl32*) iwrfPulse->getPackedData());
        
        // add the pulse to the outgoing message
        
        MemBuf pulseBuf;
        pulse.assemble(pulseBuf);
        _outputMsg.addPart(APAR_TS_PULSE_HEADER_ID,
                           pulseBuf.getLen(),
                           pulseBuf.getPtr());
        
        // write to the queue if ready
        
        if (_writeToOutputFmq(false)) {
          cerr << "ERROR - WriteToFmq::_writePulseToFmq" << endl;
          iret = -1;
        }
        
        // clear the buffer
        
        // Apar.clear();
        
        // increment
        
        _nPulsesOut++;
  
      } // ipulse
    } // ibeam
  } // ivisit

  return iret;

}

////////////////////////////////////////////////////////////////////////
// compute simulated scan strategy

void WriteToFmq::_computeScanStrategy()

{

  int beamsPerDwell = _params.n_beams_per_dwell;
  int sweepNum = 0;
  
  for (int iscan = 0; iscan < _params.sim_scans_n; iscan++) {
    
    const Params::sim_scan_t &scan = _params._sim_scans[iscan];
    
    if (scan.sim_type == Params::RHI_SIM) {
      
      Radx::SweepMode_t sweepMode = Radx::SWEEP_MODE_RHI;
      
      for (double az = scan.min_az;
           az <= scan.max_az;
           az += scan.delta_az) {
        for (int istride = 0; istride < beamsPerDwell; istride++) {
          for (double el = scan.min_el + istride * scan.delta_el;
               el <= scan.max_el; el += beamsPerDwell * scan.delta_el) {
            _simEl.push_back(el);
            _simAz.push_back(az);
            _simSweepNum.push_back(sweepNum);
            _simSweepMode.push_back(sweepMode);
          } // el
        } // istride
      } // az

    } else if (scan.sim_type == Params::PPI_SIM) {

      Radx::SweepMode_t sweepMode = Radx::SWEEP_MODE_SECTOR;
      
      for (double el = scan.min_el;
           el <= scan.max_el;
           el += scan.delta_el) {
        for (int istride = 0; istride < beamsPerDwell; istride++) {
          for (double az = scan.min_az + istride * scan.delta_az;
               az <= scan.max_az; az += beamsPerDwell * scan.delta_az) {
            _simEl.push_back(el);
            _simAz.push_back(az);
            _simSweepNum.push_back(sweepNum);
            _simSweepMode.push_back(sweepMode);
          } // az
        } // istride
      } // el
      
    } // if (scan.sim_type == Params::RHI_SIM)
    
    sweepNum++;
    
  } // iscan

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
    _nBytesForRate / (_params.udp_sim_data_rate * 1.0e6);
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
  _aparTsProcessing.start_range_m = _params.udp_gate_spacing_m / 2.0;
  _aparTsProcessing.gate_spacing_m = _params.udp_gate_spacing_m;
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

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  Wrote msg, nparts, len, path: "
         << nParts << ", " << len << ", "
         << _params.output_fmq_path << endl;
  }

  _outputMsg.clearParts();

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

