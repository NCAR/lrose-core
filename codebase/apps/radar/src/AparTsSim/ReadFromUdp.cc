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
// ReadFromUdp.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2019
//
///////////////////////////////////////////////////////////////
//
// Read UDP stream that is created by the WRITE_TO_UDP mode
// of this application.
// Creates APAR time series format stream,
// and write out to an FMQ
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
#include <toolsa/sockutil.h>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsReader.hh>
#include <Radx/RadxTime.hh>

#include "ReadFromUdp.hh"
#include "AparTsSim.hh"

using namespace std;

// Constructor

ReadFromUdp::ReadFromUdp(const string &progName,
                         const Params &params,
                         vector<string> &inputFileList) :
        _progName(progName),
        _params(params),
        _inputFileList(inputFileList)
  
{

  _sampleSeqNum = 0;
  _pulseSeqNum = 0;
  _dwellSeqNum = 0;
  _udpFd = -1;
  _errCount = 0;

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Running ReadFromUdp - extra verbose debug mode" << endl;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running ReadFromUdp - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running ReadFromUdp - debug mode" << endl;
  }

}

// destructor

ReadFromUdp::~ReadFromUdp()
  
{
  
  // delete pulses to free memory
  
  for (size_t ii = 0; ii < _dwellPulses.size(); ii++) {
    delete _dwellPulses[ii];
  }
  _dwellPulses.clear();

}

//////////////////////////////////////////////////
// Run

int ReadFromUdp::Run ()
{
  
  PMU_auto_register("ReadFromUdp::Run");
  
  // initialize the metadata for later use

  if (_inputFileList.size() < 1) {
    cerr << "ERROR - ReadFromUdp::Run()" << endl;
    cerr << "  No IWRF input file specified" << endl;
    cerr << "  We need 1 file to initialize metadata" << endl;
    return -1;
  }
  if (_setMetadata(_inputFileList[0])) {
    return -1;
  }

  // read UDP packtes from the simulator
  // these are written by an instance of this app
  // but in WRITE_TO_UDP mode
  
  int iret = 0;
  ui08 pktBuf[65536];

  while (true) {

    if (_openUdpForReading()) {
      return -1;
    }
    
    // wait on socket for up to 1 sec at a time
    
    int ready = 0;
    while ((ready = SKU_read_select(_udpFd, 1000)) == -1) {
      // timeout
      PMU_auto_register("Waiting for udp data");
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        fprintf(stderr, "Waiting for udp data\n");
      }
    }
    
    if (ready == 1) {
      
      // data is available for reading
      
      PMU_auto_register("Reading udp data");
      
      errno = EINTR;
      int pktLen = 0;

      struct sockaddr_in from;   // address from which packet came
      socklen_t addrlen = sizeof(struct sockaddr_in);
      
      while (errno == EINTR ||
             errno == EWOULDBLOCK) {
        errno = 0;
        pktLen = recvfrom(_udpFd, (void *) pktBuf, 65536, 0, 
                          (struct sockaddr *)&from, &addrlen);
        if (errno == EINTR) {
          PMU_auto_register("Reading udp data - EINTR");
        } else if (errno == EWOULDBLOCK) {
          PMU_auto_register("Reading udp data - EWOULDBLOCK");
          umsleep(1000);
        }
      }
      
      if (pktLen > 0) {
        
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          fprintf(stderr, "Read packet, %d bytes\n", pktLen);
        }
        if (_handlePacket(pktBuf, pktLen)) {
          iret = -1;
        } else {
          iret = 0;
        }
        
      } else {
        
        fprintf(stderr, "ERROR - %s::ReadFromUdp::Run\n", _progName.c_str());
        fprintf(stderr, "Reading UDP - pktLen = %d\n", pktLen);
        perror("");
        iret = -1;
        
      }
      
    } // if (ready == 1)
    
  } // while
  
  return iret;

}

////////////////////////////////////////////////////
// Handle an incoming packet

int ReadFromUdp::_handlePacket(ui08 *pktBuf, int pktLen)

{

  if (pktLen < 86) {
    cerr << "ERROR - ReadFromUdp::_handlePacketr" << endl;
    cerr << "  pktLen too short: " << pktLen << endl;
    return -1;
  }
  ui08 *loc = pktBuf;
  
  // message type

  memcpy(&_messageType, loc, sizeof(_messageType));
  BE_to_array_16(&_messageType, sizeof(_messageType));
  loc += sizeof(_messageType);

  // AESA ID
  
  memcpy(&_aesaId, loc, sizeof(_aesaId));
  BE_to_array_16(&_aesaId, sizeof(_aesaId));
  loc += sizeof(_aesaId);

  // channel number
  
  memcpy(&_chanNum, loc, sizeof(_chanNum));
  BE_to_array_16(&_chanNum, sizeof(_chanNum));
  loc += sizeof(_chanNum);

  // flags
  
  memcpy(&_flags, loc, sizeof(_flags));
  BE_to_array_32(&_flags, sizeof(_flags));
  loc += sizeof(_flags);

  _isXmitH = ((_flags & 1) != 0);
  _isRxH = ((_flags & 2) != 0);
  _isCoPolRx = false;
  if (_isXmitH && _isRxH) {
    _isCoPolRx = true;
  } else if (!_isXmitH && !_isRxH) {
    _isCoPolRx = true;
  }
  _isFirstPktInPulse = ((_flags & 4) != 0);

  // beam index

  memcpy(&_beamIndex, loc, sizeof(_beamIndex));
  BE_to_array_32(&_beamIndex, sizeof(_beamIndex));
  loc += sizeof(_beamIndex);

  // sample number

  memcpy(&_sampleNum, loc, sizeof(_sampleNum));
  BE_to_array_64(&_sampleNum, sizeof(_sampleNum));
  loc += sizeof(_sampleNum);

  // pulse number

  memcpy(&_pulseNum, loc, sizeof(_pulseNum));
  BE_to_array_64(&_pulseNum, sizeof(_pulseNum));
  loc += sizeof(_pulseNum);

  // time

  memcpy(&_secs, loc, sizeof(_secs));
  BE_to_array_64(&_secs, sizeof(_secs));
  loc += sizeof(_secs);

  memcpy(&_nsecs, loc, sizeof(_nsecs));
  BE_to_array_32(&_nsecs, sizeof(_nsecs));
  loc += sizeof(_nsecs);

  _rtime.set((time_t) _secs, (double) _nsecs / 1.0e9);

  // start index

  memcpy(&_startIndex, loc, sizeof(_startIndex));
  BE_to_array_32(&_startIndex, sizeof(_startIndex));
  loc += sizeof(_startIndex);

  // angles

  memcpy(&_uu, loc, sizeof(_uu));
  BE_to_array_32(&_uu, sizeof(_uu));
  loc += sizeof(_uu);

  memcpy(&_vv, loc, sizeof(_vv));
  BE_to_array_32(&_vv, sizeof(_vv));
  loc += sizeof(_vv);

  double elRad = asin(_vv);
  double azRad = atan2(_uu, sqrt(1.0 - _uu * _uu - _vv * _vv));
  _el = elRad * RAD_TO_DEG;
  _az = 90.0 - (azRad * RAD_TO_DEG);
  if (_az < 0.0) {
    _az += 360.0;
  } else if (_az >= 360.0) {
    _az -= 360.0;
  }

  // dwell details

  memcpy(&_dwellNum, loc, sizeof(_dwellNum));
  BE_to_array_32(&_dwellNum, sizeof(_dwellNum));
  loc += sizeof(_dwellNum);

  memcpy(&_beamNumInDwell, loc, sizeof(_beamNumInDwell));
  BE_to_array_32(&_beamNumInDwell, sizeof(_beamNumInDwell));
  loc += sizeof(_beamNumInDwell);

  memcpy(&_visitNumInBeam, loc, sizeof(_visitNumInBeam));
  BE_to_array_32(&_visitNumInBeam, sizeof(_visitNumInBeam));
  loc += sizeof(_visitNumInBeam);

  // number of samples

  memcpy(&_nSamples, loc, sizeof(_nSamples));
  BE_to_array_32(&_nSamples, sizeof(_nSamples));
  loc += sizeof(_nSamples);

  // spares

  ui32 spares[3];
  loc += sizeof(spares);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===============================================" << endl;
    cerr << "==>> messageType: " << _messageType << endl;
    cerr << "==>> aesaId: " << _aesaId << endl;
    cerr << "==>> chanNum: " << _chanNum << endl;
    cerr << "==>> flags: " << _flags << endl;
    cerr << "==>> isXmitH: " << _isXmitH << endl;
    cerr << "==>> isRxH: " << _isRxH << endl;
    cerr << "==>> isCoPolRx: " << _isCoPolRx << endl;
    cerr << "==>> isFirstPktInPulse: " << _isFirstPktInPulse << endl;
    cerr << "==>> beamIndex: " << _beamIndex << endl;
    cerr << "==>> sampleNum: " << _sampleNum << endl;
    cerr << "==>> pulseNum: " << _pulseNum << endl;
    cerr << "==>> startIndex: " << _startIndex << endl;
    cerr << "==>> rtime: " << _rtime.asString(6) << endl;
    cerr << "==>> uu: " << _uu << endl;
    cerr << "==>> vv: " << _vv << endl;
    cerr << "==>> el: " << _el << endl;
    cerr << "==>> az: " << _az << endl;
    cerr << "==>> dwellNum: " << _dwellNum << endl;
    cerr << "==>> beamNumInDwell: " << _beamNumInDwell << endl;
    cerr << "==>> visitNumInBeam: " << _visitNumInBeam << endl;
    cerr << "==>> nSamples: " << _nSamples << endl;
    cerr << "===============================================" << endl;
  }

  // if this packet is the first in a pulse,
  // send out the existing pulse

  if (_isFirstPktInPulse) {
    _writePulseToFmq();
  }

  // swap the IQ data

  int nIq = _nSamples * 2;
  int nBytesIq = nIq * sizeof(si16);
  si16 *iqData = (si16 *) loc;
  BE_swap_array_16(iqData, nBytesIq);

  // add it to the IQ vector
  
  for (int ii = 0; ii < nIq; ii++) {
    _iqApar.push_back(iqData[ii]);
  }
  
  return 0;

}

////////////////////////////////////////////////////
// Open UDP for reading
// Returns 0 on success, -1 on error

int ReadFromUdp::_openUdpForReading()
  
{

  if (_udpFd > 0) {
    // already open
    return 0;
  }

  // open socket
  
  if  ((_udpFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror ("Could not open UDP socket: ");
    _udpFd = -1;
    return -1;
  }
  
  // set the socket for reuse
  
  int val = 1;
  int valen = sizeof(val);
  setsockopt(_udpFd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, valen);

  // bind local address to the socket
  
  struct sockaddr_in localAddr;
  MEM_zero(localAddr);
  uint16_t destPort = _params.udp_dest_port;
  localAddr.sin_port = htons(destPort);
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl (INADDR_ANY);
  
  if (bind (_udpFd, (struct sockaddr *) &localAddr, 
	    sizeof (localAddr)) < 0) {
    perror ("bind error:");
    fprintf(stderr, "Could bind UDP socket, port %d\n", destPort);
    close (_udpFd);
    _udpFd = -1;
    return -1;
  }
  
  if (_params.debug) {
    fprintf(stderr, "Opened UDP socket for reading, port %d\n",
            _params.udp_dest_port);
  }
  
  return 0;

}

////////////////////////////////////////////////////
// Read the IWRF file, set the APAR-style metadata

int ReadFromUdp::_setMetadata(const string &inputPath)
  
{
  
  if (_params.debug) {
    cerr << "Reading input file: " << inputPath << endl;
  }

  // set up a vector with a single file entry
  
  vector<string> fileList;
  fileList.push_back(inputPath);

  // create reader for just that one file

  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    iwrfDebug = IWRF_DEBUG_VERBOSE;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrfDebug = IWRF_DEBUG_NORM;
  } 
  IwrfTsReaderFile reader(fileList, iwrfDebug);
  const IwrfTsInfo &tsInfo = reader.getOpsInfo();

  // read through pulses until we have current metadata

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
    cerr << "ERROR - WriteToFile::_setMetadata()" << endl;
    cerr << "Metadata missing for file: " << inputPath << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_radar_info_print(stderr, tsInfo.getRadarInfo());
    iwrf_scan_segment_print(stderr, tsInfo.getScanSegment());
    iwrf_ts_processing_print(stderr, tsInfo.getTsProcessing());
    if (tsInfo.isCalibrationActive()) {
      iwrf_calibration_print(stderr, tsInfo.getCalibration());
    }
  }

  // convert the metadata to APAR types
  // set the metadata in the info metadata queue

  _convertMeta2Apar(tsInfo);
  _aparTsInfo->setRadarInfo(_aparRadarInfo);
  _aparTsInfo->setScanSegment(_aparScanSegment);
  _aparTsInfo->setTsProcessing(_aparTsProcessing);
  if (tsInfo.isCalibrationActive()) {
    _aparTsInfo->setCalibration(_aparCalibration);
  }
  
  return 0;
  
}

///////////////////////////////////////////////
// Convert the IWRF metadata to APAR structs

void ReadFromUdp::_convertMeta2Apar(const IwrfTsInfo &info)
  
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
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    apar_ts_radar_info_print(stderr, _aparRadarInfo);
    apar_ts_scan_segment_print(stderr, _aparScanSegment);
    apar_ts_processing_print(stderr, _aparTsProcessing);
    apar_ts_calibration_print(stderr, _aparCalibration);
  }
}

////////////////////////////////////////////////////
// Write the current pulse to an output FMQ

int ReadFromUdp::_writePulseToFmq()
  
{

  // write the pulse

  // clear the buffer

  _iqApar.clear();
  
  return 0;

}
  
#ifdef JUNK

////////////////////////////////////////////////////
// Convert 1 file to UDP

int ReadFromUdp::_convert2Udp(const string &inputPath)
  
{
  
  if (_params.debug) {
    cerr << "Reading input file: " << inputPath << endl;
  }

  // set up a vector with a single file entry
  
  vector<string> fileList;
  fileList.push_back(inputPath);

  // create reader for just that one file

  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    iwrfDebug = IWRF_DEBUG_VERBOSE;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrfDebug = IWRF_DEBUG_NORM;
  } 
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
      cerr << "ERROR - ReadFromUdp::_convert2Udp()" << endl;
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
    
    // open output UDP as needed
    
    if (_openUdpForReading()) {
      cerr << "ERROR - ReadFromUdp::_convert2Udp" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      cerr << "  Cannot open UDP output device" << endl;
      return -1;
    }
    
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

int ReadFromUdp::_processDwell(vector<IwrfTsPulse *> &dwellPulses)
  
{

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
  for (int ii = 0; ii < _params.n_beams_per_dwell; ii++) {
    double az = AparTsSim::conditionAngle360(startAz + 
                                             (ii + 0.5) * deltaAzPerBeam);
    double el = AparTsSim::conditionAngle180(startEl + 
                                             (ii + 0.5) * deltaElPerBeam);
    beamAz.push_back(az);
    beamEl.push_back(el);
  }

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
        
        // convert to si16 as needed
        
        IwrfTsPulse *iwrfPulse = dwellPulses[pulseNumInDwell];
        if (iwrfPulse->getPackedEncoding() != IWRF_IQ_ENCODING_SCALED_SI16) {
          iwrfPulse->convertToPacked(IWRF_IQ_ENCODING_SCALED_SI16);
        }

        // set the metadata

        ui64 sampleNumber = _sampleSeqNum;
        ui64 pulseNumber = _pulseSeqNum;
        
        si64 secondsTime = iwrfPulse->getTime();
        ui32 nanoSecs = iwrfPulse->getNanoSecs();
        
        ui64 dwellNum = _dwellSeqNum;
        ui32 beamNumInDwell = ibeam;
        ui32 visitNumInBeam = ivisit;
        
        double azRad = beamAz[ibeam] * DEG_TO_RAD;
        double elRad = beamEl[ibeam] * DEG_TO_RAD;
        double uu = cos(elRad) * sin(azRad);
        double vv = sin(elRad);
        
        bool isXmitH = iwrfPulse->isHoriz();
        bool isCoPolRx = true;

        // fill out the IQ data array
        
        vector<si16> iqApar;
        _fillIqData(iwrfPulse, 0, iqApar); // channel 0, co-pol

        // create the buffer for co-polar packet
        
        if (_sendPulse(sampleNumber,
                       pulseNumber,
                       secondsTime, nanoSecs,
                       dwellNum,
                       beamNumInDwell,
                       visitNumInBeam,
                       uu, vv, 
                       isXmitH, isCoPolRx,
                       _params.udp_n_gates,
                       iqApar)) {
          cerr << "ERROR - _processDwell" << endl;
          return -1;
        }
        
        // optionally add a cross-pol pulse
        // at the end of the dwell
        
        if (_params.add_cross_pol_sample_at_end_of_visit &&
            ipulse == _params.n_samples_per_visit - 1) {
          
          isCoPolRx = false;
          sampleNumber = _sampleSeqNum;
          pulseNumber = _pulseSeqNum;
          
          _fillIqData(iwrfPulse, 1, iqApar); // channel 1, cross-pol

          if (_sendPulse(sampleNumber,
                         pulseNumber,
                         secondsTime, nanoSecs,
                         dwellNum,
                         beamNumInDwell,
                         visitNumInBeam,
                         uu, vv, 
                         isXmitH, isCoPolRx,
                         _params.udp_n_gates,
                         iqApar)) {
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

void ReadFromUdp::_fillIqData(IwrfTsPulse *iwrfPulse,
                              int channelNum,
                              vector<si16> &iqApar)

{

  // init

  iqApar.clear();

  // determine how many copies we need to make for each
  // gate to fill out the apar array
  
  int nGatesIwrf = iwrfPulse->getNGates();
  int nGatesApar = _params.udp_n_gates;
  int nValsApar = nGatesApar * 2;
  int nCopyPerGate = ((nGatesApar - 1) / nGatesIwrf) + 1;
  
  // copy iwrf data into apar array
  
  const si16 *iqIwrf = (const si16 *) iwrfPulse->getPackedData() +
    channelNum * nGatesIwrf * 2;

  
  while ((int) iqApar.size() < nValsApar) {
    
    for (int icopy = 0; icopy < nCopyPerGate; icopy++) {
      iqApar.push_back(*iqIwrf);       // I
      iqApar.push_back(*(iqIwrf + 1)); // Q
      if ((int) iqApar.size() ==  nValsApar) {
        break;
      }
    }

    iqIwrf += 2;

  } // while
  
}

////////////////////////////////////////////////////////////////////////
// create a packet buffer

int ReadFromUdp::_sendPulse(ui64 sampleNumber,
                            ui64 pulseNumber,
                            si64 secondsTime,
                            ui32 nanoSecs,
                            ui64 dwellNum,
                            ui32 beamNumInDwell,
                            ui32 visitNumInBeam,
                            double uu,
                            double vv,
                            bool isXmitH,
                            bool isCoPolRx,
                            int nGates,
                            vector<si16> &iqApar)
  
{

  // create packet buffer

  MemBuf buf;

  // add header to buffer, compute header length
  
  ui32 pulseStartIndex = 0;
  
  _addAparHeader(sampleNumber, pulseNumber,
                 secondsTime, nanoSecs,
                 pulseStartIndex,
                 dwellNum, beamNumInDwell, visitNumInBeam,
                 uu, vv,
                 true, isXmitH, isCoPolRx,
                 nGates,
                 buf);
  
  size_t headerLen = buf.getLen();

  // compute number of packets per pulse

  int maxNbytesDataPerPacket = _params.udp_max_packet_size - headerLen;
  int maxNGatesPerPacket = maxNbytesDataPerPacket / 4;
  int nPacketsPerPulse = (_params.udp_n_gates / maxNGatesPerPacket) + 1;
  int nGatesPerPacket = ((nGates - 1) / nPacketsPerPulse) + 1;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> UDP headerLen: " << headerLen << endl;
    cerr << "==>> UDP maxNbytesDataPerPacket: "
         << maxNbytesDataPerPacket << endl;
    cerr << "==>> UDP maxNGatesPerPacket: "
         << maxNGatesPerPacket << endl;
    cerr << "==>> UDP nPacketsPerPulse: "
         << nPacketsPerPulse << endl;
    cerr << "==>> UDP nGatesPerPacket: "
         << nGatesPerPacket << endl;
  }

  // create the pulse packets

  int nGatesRemaining = nGates;
  int nGatesSoFar = 0;

  for (int ipkt = 0; ipkt < nPacketsPerPulse; ipkt++) {

    //  init

    buf.clear();
    bool isFirstPacket = (ipkt == 0);

    // compute number of gates for this packet
  
    int nGatesThisPacket = nGatesPerPacket;
    if (nGatesThisPacket > nGatesRemaining) {
      nGatesThisPacket = nGatesRemaining;
    }

    // add header to buffer
  
    _addAparHeader(sampleNumber, pulseNumber,
                   secondsTime, nanoSecs,
                   pulseStartIndex,
                   dwellNum, beamNumInDwell, visitNumInBeam,
                   uu, vv,
                   isFirstPacket, isXmitH, isCoPolRx,
                   nGatesThisPacket,
                   buf);

    // add IQ data to buffer

    buf.add(iqApar.data() + nGatesSoFar * 2,
            nGatesThisPacket * 2 * 2);

    // write the packet to UDP
    
    if (_writeBufToUdp(buf)) {
      cerr << "ERROR - _sendPulse" << endl;
      return -1;
    }
    
    // increment / decrement

    nGatesSoFar += nGatesThisPacket;
    nGatesRemaining -= nGatesThisPacket;
    _sampleSeqNum += nGatesThisPacket;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "====>> UDP ipkt: " << ipkt << endl;
      cerr << "====>> UDP nGatesSoFar: " << nGatesSoFar << endl;
      cerr << "====>> UDP nGatesRemaining: " << nGatesRemaining << endl;
      cerr << "====>> UDP _sampleSeqNum: " << _sampleSeqNum << endl;
    }
    
  } // ipkt

  // sent 1 pulse

  _pulseSeqNum++;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> UDP _pulseSeqNum: " << _pulseSeqNum << endl;
  }
    
  return 0;

}

  
////////////////////////////////////////////////////////////////////////
// add header to packet

void ReadFromUdp::_addAparHeader(ui64 sampleNumber,
                                 ui64 pulseNumber,
                                 si64 secondsTime,
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
    // git 1 indicates H receive
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

  ui64 sampleNum = sampleNumber;
  BE_from_array_64(&sampleNum, sizeof(sampleNum));
  buf.add(&sampleNum, sizeof(sampleNum));
  
  // pulse number

  ui64 pulseNum = pulseNumber;
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
// Write buffer to UDP
// Returns 0 on success, -1 on error

int ReadFromUdp::_writeBufToUdp(const MemBuf &buf)
  
{

  // set up destination address structure
  
  struct sockaddr_in destAddr;
  MEM_zero(destAddr);
  if (inet_aton(_params.udp_dest_address, &destAddr.sin_addr) == 0) {
    fprintf(stderr, "Cannot translate address: %s - may be invalid\n",
            _params.udp_dest_address);
    close (_udpFd);
    _udpFd = -1;
    return -1;
  }
  destAddr.sin_family = AF_INET;
  uint16_t destPort = _params.udp_dest_port;
  destAddr.sin_port = htons(destPort);
  destAddr.sin_addr.s_addr = inet_addr(_params.udp_dest_address);
  
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


#endif
