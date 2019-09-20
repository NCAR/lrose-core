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
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsReader.hh>

#include "WriteToUdp.hh"
#include "AparTsSim.hh"

using namespace std;

// Constructor

WriteToUdp::WriteToUdp(const string &progName,
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
    cerr << "Running WriteToUdp - extra verbose debug mode" << endl;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running WriteToUdp - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running WriteToUdp - debug mode" << endl;
  }

}

// destructor

WriteToUdp::~WriteToUdp()
  
{
  
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
    for (size_t ii = 0; ii < _inputFileList.size(); ii++) {
      if (_convert2Udp(_inputFileList[ii])) {
        iret = -1;
      }
    }
  }
  
  return iret;

}

////////////////////////////////////////////////////
// Convert 1 file to UDP

int WriteToUdp::_convert2Udp(const string &inputPath)
  
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
      cerr << "ERROR - WriteToUdp::_convert2Udp()" << endl;
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
    
    if (_openOutputUdp()) {
      cerr << "ERROR - WriteToUdp::_convert2Udp" << endl;
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

int WriteToUdp::_processDwell(vector<IwrfTsPulse *> &dwellPulses)
  
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

void WriteToUdp::_fillIqData(IwrfTsPulse *iwrfPulse,
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

int WriteToUdp::_sendPulse(ui64 sampleNumber,
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

  uusleep(100);

  return 0;

}

  
////////////////////////////////////////////////////////////////////////
// add header to packet

void WriteToUdp::_addAparHeader(ui64 sampleNumber,
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
// Open output UDP device
// Returns 0 on success, -1 on error

int WriteToUdp::_openOutputUdp()
  
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
  
  // bind local address to the socket
  
  struct sockaddr_in localAddr;
  MEM_zero(localAddr);
  uint16_t sourcePort = _params.udp_source_port;
  localAddr.sin_port = htons(sourcePort);
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl (INADDR_ANY);
  
  if (bind (_udpFd, (struct sockaddr *) &localAddr, 
	    sizeof (localAddr)) < 0) {
    perror ("bind error:");
    fprintf(stderr, "Could bind UDP socket, port %d\n", sourcePort);
    close (_udpFd);
    _udpFd = -1;
    return -1;
  }
  
  // set socket for broadcast
  
  int option = 1;
  if (setsockopt(_udpFd, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR,
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


