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
#include <radar/AparTsPulse.hh>
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
  _volNum = 0;
  _sweepNum = 0;
  _nPulsesOut = 0;

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Running ReadFromUdp - extra verbose debug mode" << endl;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running ReadFromUdp - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running ReadFromUdp - debug mode" << endl;
  }

  _aparTsDebug = APAR_TS_DEBUG_OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _aparTsDebug = APAR_TS_DEBUG_VERBOSE;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    _aparTsDebug = APAR_TS_DEBUG_NORM;
  }
  _aparTsInfo = new AparTsInfo(_aparTsDebug);

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
  if (_initMetaData(_inputFileList[0])) {
    return -1;
  }

  // initialize the output FMQ

  if (_openOutputFmq()) {
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
        
        if (_params.debug >= Params::DEBUG_EXTRA) {
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

  if (_params.debug >= Params::DEBUG_EXTRA) {
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

int ReadFromUdp::_initMetaData(const string &inputPath)
  
{
  
  if (_params.debug) {
    cerr << "Setting metadata from file: " << inputPath << endl;
  }
  
  // set up a vector with a single file entry
  
  vector<string> fileList;
  fileList.push_back(inputPath);

  // create reader for just that one file

  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  // if (_params.debug >= Params::DEBUG_EXTRA) {
  //   iwrfDebug = IWRF_DEBUG_VERBOSE;
  // } else if (_params.debug >= Params::DEBUG_VERBOSE) {
  //   iwrfDebug = IWRF_DEBUG_NORM;
  // } 
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
    cerr << "ERROR - ReadFromUdp::_setMetadata()" << endl;
    cerr << "Metadata missing for file: " << inputPath << endl;
    return -1;
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
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
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

  int iret = 0;

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "==>> Writing out pulse <<==" << endl;
    cerr << "  _nPulsesOut: " << _nPulsesOut << endl;
    cerr << "  _iqApar.size(): " << _iqApar.size() << endl;
  }

  // add metadata to outgoing message

  if (_nPulsesOut % _params.n_pulses_per_info == 0) {
    cerr << "11111111111111111111" << endl;
    _addMetaDataToMsg();
  }
  
  // create the outgoing pulse

  AparTsPulse pulse(*_aparTsInfo, _aparTsDebug);
  pulse.setTime(_secs, _nsecs);
  int nGates = _iqApar.size() / 2;
  pulse.setIqPacked(nGates, 1,
                    APAR_TS_IQ_ENCODING_SCALED_SI16,
                    _iqApar.data(),
                    1.0, 0.0);

  pulse.setBeamNumInDwell(_beamNumInDwell);
  pulse.setVisitNumInBeam(_visitNumInBeam);
  pulse.setPulseSeqNum(_pulseNum);
  pulse.setDwellSeqNum(_dwellNum);

  pulse.setScanMode(_aparScanSegment.scan_mode);
  pulse.setSweepNum(_sweepNum);
  pulse.setVolumeNum(_volNum);

  pulse.setElevation(_el);
  pulse.setAzimuth(_az);
  pulse.setFixedAngle(_el);
  pulse.setPrt(_aparTsProcessing.prt_us[0]);
  pulse.setPrtNext(_aparTsProcessing.prt_us[0]);
  pulse.setPulseWidthUs(_aparTsProcessing.pulse_width_us);
  pulse.setHvFlag(_isXmitH);
  pulse.setStartRangeM(_aparTsProcessing.start_range_m);
  pulse.setGateGSpacineM(_aparTsProcessing.gate_spacing_m);

  // add the pulse to the outgoing message

  MemBuf pulseBuf;
  pulse.assemble(pulseBuf);
  _outputMsg.addPart(APAR_TS_PULSE_HEADER_ID,
                     pulseBuf.getLen(),
                     pulseBuf.getPtr());

  // write to the queue if ready

  if (_writeToOutputFmq(false)) {
    cerr << "ERROR - ReadFromUdp::_writePulseToFmq" << endl;
    iret = -1;
  }
  
  // clear the buffer

  _iqApar.clear();

  // increment

  _nPulsesOut++;
  
  return iret;

}
  
///////////////////////////////////////
// open the output FMQ
// returns 0 on success, -1 on failure

int ReadFromUdp::_openOutputFmq()

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

void ReadFromUdp::_addMetaDataToMsg()
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

int ReadFromUdp::_writeToOutputFmq(bool force)

{

  // if the message is large enough, write to the FMQ
  
  int nParts = _outputMsg.getNParts();
  if (!force && nParts < _params.n_pulses_per_message) {
    return 0;
  }

  PMU_auto_register("writeToOutputFmq");

  // if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "========= Output Message =========" << endl;
    _outputMsg.printHeader(cerr);
    _outputMsg.printPartHeaders(cerr);
    cerr << "==================================" << endl;
    // }


  void *buf = _outputMsg.assemble();
  int len = _outputMsg.lengthAssembled();
  if (_outputFmq.writeMsg(0, 0, buf, len)) {
    cerr << "ERROR - ReadFromUdp" << endl;
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

int ReadFromUdp::_writeEndOfVol()

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

