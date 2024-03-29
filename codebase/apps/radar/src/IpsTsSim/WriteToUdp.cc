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
// WriteToUdp.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2019
//
///////////////////////////////////////////////////////////////
//
// Resample IWRF time series data,
// convert to IPS UDP format,
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
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsReader.hh>

#include "WriteToUdp.hh"
#include "IpsTsSim.hh"
#include "SimScanStrategy.hh"

using namespace std;

// Constructor

WriteToUdp::WriteToUdp(const string &progName,
                       const Params &params,
                       vector<string> &inputFileList) :
        _progName(progName),
        _params(params),
        _inputFileList(inputFileList)
  
{

  // debug print

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Running WriteToUdp - extra verbose debug mode" << endl;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running WriteToUdp - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running WriteToUdp - debug mode" << endl;
  }

  // init

  _sampleSeqNum = 0;
  _pulseSeqNum = 0;
  _dwellSeqNum = 0;
  _udpFd = -1;
  _errCount = 0;
  _rateStartTime.set(RadxTime::NEVER);
  _nBytesForRate = 0;
  _realtimeDeltaSecs = 0;

  // compute the scan strategy

  _strategy = new SimScanStrategy(_params);
  
  // compute packet header length
  // by creating a dummy header and
  // getting the length of the resulting buffer
  
  MemBuf buf;
  _addIpsHeader(0, 0, 0,
                 0, 0, 0,
                 0.0, 0.0,
                 true, true, true,
                 1000, buf);
  _nBytesHeader = buf.getLen();

  // compute number of IQ samples (gates) per packet
  // the number of samples per packet must be a multiple of 16
  // so round down to the nearest 16

  int maxNbytesDataPerPacket = _params.udp_max_packet_size - _nBytesHeader;
  _nGatesPacket = maxNbytesDataPerPacket / (2 * 2); // I/Q, short for each
  _nGatesPacket = (_nGatesPacket / 16) * 16;
  _nBytesData = _nGatesPacket * (2 * 2); // I/Q, short for each
  _nBytesPacket = _nBytesHeader + _nBytesData;
  _nGatesRemaining = 0;

  if (_params.debug) {
    cerr << "WriteToUdp constructor" << endl;
    cerr << "  _nBytesHeader: " << _nBytesHeader << endl;
    cerr << "  _nGatesPacket: " << _nGatesPacket << endl;
    cerr << "  _nBytesData: " << _nBytesData << endl;
    cerr << "  _nBytesPacket: " << _nBytesPacket << endl;
  }

}

// destructor

WriteToUdp::~WriteToUdp()
  
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

int WriteToUdp::Run ()
{

  PMU_auto_register("WriteToUdp::Run");
  
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
      if (_convertToUdp(_inputFileList[ii])) {
        iret = -1;
      }
    }

    umsleep(1000);

  } // while
  
  return iret;

}

////////////////////////////////////////////////////
// Convert 1 file to UDP

int WriteToUdp::_convertToUdp(const string &inputPath)
  
{
  
  PMU_auto_register("WriteToUdp::_convertToUdp");
  
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
      cerr << "ERROR - WriteToUdp::_convertToUdp()" << endl;
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
  
    // open output UDP as needed
    
    if (_openOutputUdp()) {
      cerr << "ERROR - WriteToUdp::_convertToUdp" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      cerr << "  Cannot open UDP output device" << endl;
      return -1;
    }

    // convert pulse data to si16 counts
    
    iwrfPulse->convertToScaledSi16(_params.udp_iq_scale_for_si16, 0.0);
    
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

int WriteToUdp::_processDwell(vector<IwrfTsPulse *> &dwellPulses)
  
{

  // compute the angles for the beams in the dwell

  double startAz = dwellPulses.front()->getAz();
  double endAz = dwellPulses.back()->getAz();
  double azRange = IpsTsSim::conditionAngle360(endAz - startAz);
  double deltaAzPerBeam = azRange / _params.n_beams_per_dwell;

  double startEl = dwellPulses.front()->getEl();
  double endEl = dwellPulses.back()->getEl();
  double elRange = IpsTsSim::conditionAngle360(endEl - startEl);
  double deltaElPerBeam = elRange / _params.n_beams_per_dwell;
  
  vector<double> beamAz, beamEl;
  vector<int> sweepNum, volNum;
  vector<Radx::SweepMode_t> sweepMode;
  
  for (int ii = 0; ii < _params.n_beams_per_dwell; ii++) {

    if (!_params.specify_scan_strategy) {

      double az = IpsTsSim::conditionAngle360(startAz + 
                                               (ii + 0.5) * deltaAzPerBeam);
      double el = IpsTsSim::conditionAngle180(startEl + 
                                               (ii + 0.5) * deltaElPerBeam);
      
      // for IPS, az ranges from -60 to +60
      // and el from -90 to +90
      // so we adjust accordingly
      
      beamAz.push_back((az / 3.0) - 60.0);
      beamEl.push_back(el / 1.5);

    } else {

      SimScanStrategy::angle_t angle = _strategy->getNextAngle();
      
      beamAz.push_back(angle.az);
      beamEl.push_back(angle.el);
      sweepNum.push_back(angle.sweepNum);
      volNum.push_back(angle.volNum);
      sweepMode.push_back(angle.sweepMode);

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
        
        ui64 dwellNum = _dwellSeqNum;
        ui32 beamNumInDwell = ibeam;
        ui32 visitNumInBeam = ivisit;

        double az = beamAz[ibeam];
        double el = beamEl[ibeam];
        double azRad = az * DEG_TO_RAD;
        double elRad = el * DEG_TO_RAD;
        double uu = cos(elRad) * sin(azRad);
        double vv = sin(elRad);
        
        bool isXmitH = iwrfPulse->isHoriz();
        bool isCoPolRx = true;

        if (_params.debug >= Params::DEBUG_EXTRA) {

          cerr << "================================================" << endl;
          cerr << "==>> UDP sending pulse, _pulseSeqNum: " << _pulseSeqNum << endl;
          cerr << "==>> el: " << el << endl;
          cerr << "==>> az: " << az << endl;
          cerr << "==>> uu: " << uu << endl;
          cerr << "==>> vv: " << vv << endl;
          
        }

        // fill out the IQ data array
        
        vector<si16> iqIps;
        _fillIqData(iwrfPulse, 0, iqIps); // channel 0, co-pol

        // create the buffer for co-polar packet
        
        if (_sendPulse(secondsTime, nanoSecs,
                       dwellNum,
                       beamNumInDwell,
                       visitNumInBeam,
                       uu, vv, 
                       isXmitH, isCoPolRx,
                       _params.udp_n_gates,
                       iqIps)) {
          cerr << "ERROR - _processDwell" << endl;
          return -1;
        }
        
        // optionally add a cross-pol pulse
        // at the end of the dwell
        
        if (_params.add_cross_pol_sample_at_end_of_visit &&
            ipulse == _params.n_samples_per_visit - 1) {
          
          isCoPolRx = false;
          
          _fillIqData(iwrfPulse, 1, iqIps); // channel 1, cross-pol
          
          if (_sendPulse(secondsTime, nanoSecs,
                         dwellNum,
                         beamNumInDwell,
                         visitNumInBeam,
                         uu, vv, 
                         isXmitH, isCoPolRx,
                         _params.udp_n_gates,
                         iqIps)) {
            cerr << "ERROR - _processDwell" << endl;
            return -1;
          }

        } // if (_params.add_cross_pol_sample_at_end_of_visit ...

      } // ipulse
    } // ibeam
  } // ivisit

  return 0;

}

////////////////////////////////////////////////////////////////////////
// fill out IQ data array

void WriteToUdp::_fillIqData(IwrfTsPulse *iwrfPulse,
                             int channelNum,
                             vector<si16> &iqIps)

{

  // init

  iqIps.clear();

  // determine how many copies we need to make for each
  // gate to fill out the ips array
  
  int nGatesIwrf = iwrfPulse->getNGates();
  int nGatesIps = _params.udp_n_gates;
  int nValsIps = nGatesIps * 2;
  int nCopyPerGate = ((nGatesIps - 1) / nGatesIwrf) + 1;
  
  // copy iwrf data into ips array
  
  const si16 *iqIwrf = (const si16 *) iwrfPulse->getPackedData() +
    channelNum * nGatesIwrf * 2;

  while ((int) iqIps.size() < nValsIps) {
    
    for (int icopy = 0; icopy < nCopyPerGate; icopy++) {
      iqIps.push_back(BE_from_si16(*iqIwrf));       // I
      iqIps.push_back(BE_from_si16(*(iqIwrf + 1))); // Q
      if ((int) iqIps.size() ==  nValsIps) {
        break;
      }
    }

    iqIwrf += 2;

  } // while
  
}

////////////////////////////////////////////////////////////////////////
// Send data for a pulse

int WriteToUdp::_sendPulse(si64 secondsTime,
                           ui32 nanoSecs,
                           ui64 dwellNum,
                           ui32 beamNumInDwell,
                           ui32 visitNumInBeam,
                           double uu,
                           double vv,
                           bool isXmitH,
                           bool isCoPolRx,
                           int nGates,
                           vector<si16> &iqIps)
  
{

  double fractionSecs = nanoSecs / 1.0e9;
  DateTime pulseTime(secondsTime, true, fractionSecs);

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "------------------------------------------------" << endl;
    cerr << "==>> UDP start of pulse" << endl;
    cerr << "==>>   pulseTime: " << pulseTime.asString(3) << endl;
    cerr << "==>>   nGates remaining from prev: " << _nGatesRemaining << endl;
    cerr << "==>>   nGates this pulse: " << nGates << endl;
    cerr << "==>>   nGates total: " << _nGatesRemaining + iqIps.size() / 2 << endl;
  }

  // add new IQ samples to unused IQ buffer

  for (size_t ii = 0; ii < iqIps.size(); ii++) {
    _iqQueue.push_back(iqIps[ii]);
  }
  
  // create the pulse packets

  ui32 pulseStartIndex = _nGatesRemaining;
  _nGatesRemaining += nGates;
  int nGatesSoFar = 0;
  int pktNum = 0;
  bool isFirstPktInPulse = true;

  while (_nGatesRemaining >= _nGatesPacket) {
  
    // create packet buffer
  
    MemBuf buf;
  
    // add header to buffer
    
    _addIpsHeader(secondsTime, nanoSecs,
                   pulseStartIndex,
                   dwellNum, beamNumInDwell, visitNumInBeam,
                   uu, vv,
                   isFirstPktInPulse, isXmitH, isCoPolRx,
                   _nGatesPacket,
                   buf);
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "=====>> adding packet <<=====" << endl;
      cerr << "  pktNum: " << pktNum << endl;
      cerr << "  isFirstPktInPulse: " << isFirstPktInPulse << endl;
      cerr << "  pulseStartIndex: " << pulseStartIndex << endl;
      cerr << "  starting sampleSeqNum: " << _sampleSeqNum << endl;
      cerr << "  nGatesPacket: " << _nGatesPacket << endl;
    }

    // add IQ data to buffer
    
    for (size_t ii = 0; ii < _nGatesPacket; ii++) {
      si16 ival = _iqQueue.front();
      _iqQueue.pop_front();
      si16 qval = _iqQueue.front();
      _iqQueue.pop_front();
      buf.add(&ival, sizeof(si16));
      buf.add(&qval, sizeof(si16));
    } // ii
      
    // write the packet to UDP
    
    if (_writeBufToUdp(buf)) {
      cerr << "ERROR - _sendPulse" << endl;
      return -1;
    }
    _nBytesForRate += buf.getLen();
    
    // increment / decrement

    isFirstPktInPulse = false;
    pulseStartIndex = 0;
    pktNum++;
    nGatesSoFar += _nGatesPacket;
    _nGatesRemaining -= _nGatesPacket;
    _sampleSeqNum += _nGatesPacket;
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "=====>> packet sent <<=====" << endl;
      cerr << "  nGatesSoFar: " << nGatesSoFar << endl;
      cerr << "  _nGatesRemaining: " << _nGatesRemaining << endl;
    }
    
  } // while

  // sent 1 pulse

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "==>> UDP done, _pulseSeqNum: " << _pulseSeqNum << endl;
    cerr << "==>> UDP done, nGatesSoFar: " << nGatesSoFar << endl;
    cerr << "================================================" << endl;
  }

  _pulseSeqNum++;

  _sleepForDataRate();

  return 0;

}

  
////////////////////////////////////////////////////////////////////////
// add header to packet

void WriteToUdp::_addIpsHeader(si64 secondsTime,
                                ui32 nanoSecs,
                                ui32 pulseStartIndex,
                                ui64 dwellNum,
                                ui32 beamNumInDwell,
                                ui32 visitNumInBeam,
                                double uu,
                                double vv,
                                bool isFirstPktInPulse,
                                bool isXmitH,
                                bool isCoPolRx,
                                int nSamples,
                                MemBuf &buf)
  
{

  // message type

  ui16 messageType = 0x0001;
  BE_from_array_16(&messageType, sizeof(messageType));
  buf.add(&messageType, sizeof(messageType));

  // AESA ID

  ui16 aesaId = 0;
  BE_from_array_16(&aesaId, sizeof(aesaId));
  buf.add(&aesaId, sizeof(aesaId));

  // channel number

  ui16 chanNum = 0;
  BE_from_array_16(&chanNum, sizeof(chanNum));
  buf.add(&chanNum, sizeof(chanNum));

  // flags
  
  ui32 flags = 0;
  if (isXmitH) {
    // bit 0 indicates H transmit
    flags |= 1;
  }
  if ((isXmitH && isCoPolRx) || (!isXmitH && !isCoPolRx)) {
    // bit 1 indicates H receive
    flags |= 2;
  }
  // bit 3 is pulse start flag
  if (isFirstPktInPulse) {
    flags |= 4;
  }

  BE_from_array_32(&flags, sizeof(flags));
  buf.add(&flags, sizeof(flags));

  // beam index - always 200 for now

  ui32 beamIndex = 200;
  BE_from_array_32(&beamIndex, sizeof(beamIndex));
  buf.add(&beamIndex, sizeof(beamIndex));
  
  // sample number - first sample in this packet

  ui64 sampleNum = _sampleSeqNum;
  BE_from_array_64(&sampleNum, sizeof(sampleNum));
  buf.add(&sampleNum, sizeof(sampleNum));
  
  // pulse number

  ui64 pulseNum = _pulseSeqNum;
  BE_from_array_64(&pulseNum, sizeof(pulseNum));
  buf.add(&pulseNum, sizeof(pulseNum));

  // time

  si64 secs = secondsTime;
  BE_from_array_64(&secs, sizeof(secs));
  buf.add(&secs, sizeof(secs));

  ui32 nsecs = nanoSecs;
  BE_from_array_32(&nsecs, sizeof(nsecs));
  buf.add(&nsecs, sizeof(nsecs));

  // start index

  ui32 startIndex = pulseStartIndex;
  BE_from_array_32(&startIndex, sizeof(startIndex));
  buf.add(&startIndex, sizeof(startIndex));

  // angles

  fl32 u = uu;
  BE_from_array_32(&u, sizeof(u));
  buf.add(&u, sizeof(u));

  fl32 v = vv;
  BE_from_array_32(&v, sizeof(v));
  buf.add(&v, sizeof(v));

  // dwell, beam, visit

  ui64 dNum = dwellNum;
  BE_from_array_64(&dNum, sizeof(dNum));
  buf.add(&dNum, sizeof(dNum));

  ui32 bNum = beamNumInDwell;
  BE_from_array_32(&bNum, sizeof(bNum));
  buf.add(&bNum, sizeof(bNum));

  ui32 vNum = visitNumInBeam;
  BE_from_array_32(&vNum, sizeof(vNum));
  buf.add(&vNum, sizeof(vNum));

  // number of samples = number of gates

  ui32 nSamp = nSamples;
  BE_from_array_32(&nSamp, sizeof(nSamp));
  buf.add(&nSamp, sizeof(nSamp));

  // 3 spares

  ui32 spare = 0;
  buf.add(&spare, sizeof(spare));
  buf.add(&spare, sizeof(spare));
  buf.add(&spare, sizeof(spare));

}

  
////////////////////////////////////////////////////
// Open output UDP device
// Returns 0 on success, -1 on error

int WriteToUdp::_openOutputUdp()
  
{

  if (_udpFd > 0) {
    // already open
    return 0;
  }

  // open socket
  
  if  ((_udpFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror ("Could not open UDP socket: ");
    _udpFd = -1;
    return -1;
  }
  
  // set socket for broadcast
  
  int option = 1;
  // if (setsockopt(_udpFd, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR,
  if (setsockopt(_udpFd, SOL_SOCKET, SO_REUSEADDR,
		 (char *) &option, sizeof(option)) < 0) {
    perror ("Could not set broadcast on - setsockopt error");
    close (_udpFd);
    _udpFd = -1;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////
// Write buffer to UDP
// Returns 0 on success, -1 on error

int WriteToUdp::_writeBufToUdp(const MemBuf &buf)
  
{

  // set up destination address structure
  
  struct sockaddr_in destAddr;
  MEM_zero(destAddr);

  destAddr.sin_family = AF_INET;
  uint16_t destPort = _params.udp_dest_port;
  destAddr.sin_port = htons(destPort);
  destAddr.sin_addr.s_addr = inet_addr(_params.udp_dest_address);
  memset(destAddr.sin_zero, '\0', sizeof(destAddr.sin_zero));

  if (sendto(_udpFd, buf.getPtr(), buf.getLen(), 0,
             (struct sockaddr *) &destAddr,
             sizeof(destAddr)) != (ssize_t) buf.getLen()) {
    if (_errCount % 1000 == 0) {
      perror("_writeBufToUdp: ");
      cerr << "Cannot write UDP packet to port: "
           << _params.udp_dest_port << endl;
      cerr << "  Pkt len: " << buf.getLen() << endl;
    }
    _errCount++;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Sent UDP packet, len: " << buf.getLen() << endl;
  }
  
  return 0;

}

////////////////////////////////////////////////////
// Sleep as required to achieve the desired data rate
// Returns 0 on success, -1 on error

void WriteToUdp::_sleepForDataRate()
  
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
