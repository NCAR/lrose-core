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
// SpolTs2Fmq.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2011
//
///////////////////////////////////////////////////////////////
//
// SpolTs2Fmq reads time series data from a server in TCP,
// and writes it out to an FMQ. It also optionally merges in
// syscon scan information, and angles from the S2D system.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <cmath>
#include <sys/stat.h>
#include <toolsa/pmu.h>
#include <toolsa/uusleep.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/toolsa_macros.h>
#include <didss/DsMsgPart.hh>
#include <radar/iwrf_data.h>
#include <radar/iwrf_functions.hh>
#include "SpolTs2Fmq.hh"

using namespace std;

// Constructor

SpolTs2Fmq::SpolTs2Fmq(int argc, char **argv)
  
{

  isOK = true;

  _sockTimedOut = false;
  _timedOutCount = 0;
  _unknownMsgId = false;
  _unknownCount = 0;
  _pulseCount = 0;

  _testPulseLatestTime = 0;
  _xmitPowerLatestTime = 0;
  _angleErrorsLatestTime = 0;
  _statusXmlLatestTime = 0;
  _secondaryStatusLatestTime = 0;

  _sysconLateSet = false;
  _sysconLateSecs = 0.0;

  _prevVolNum = -1;
  _volNum = 0;
  _volStartSweepNum = 0;

  _sweepNum = 0;

  _sweepNumChangeInit = false;
  _sweepNumChangeInProgress = false;
  _sweepNumOld = 0;
  _sweepNumAzOld = 0;
  _sweepNumTransDirn = 0;

  iwrf_radar_info_init(_tsRadarInfo);
  iwrf_scan_segment_init(_tsScanSeg);
  iwrf_ts_processing_init(_tsTsProc);
  iwrf_xmit_power_init(_tsXmitPower);

  iwrf_radar_info_init(_sysconRadarInfo);
  iwrf_scan_segment_init(_sysconScanSeg);
  iwrf_ts_processing_init(_sysconTsProc);
  iwrf_xmit_power_init(_sysconXmitPower);

  iwrf_event_notice_init(_sysconStartOfSweep);
  iwrf_event_notice_init(_sysconStartOfSweepPrev);
  iwrf_event_notice_init(_sysconEndOfSweep);
  iwrf_event_notice_init(_sysconEndOfSweepPrev);
  iwrf_event_notice_init(_sysconStartOfVolume);
  iwrf_event_notice_init(_sysconEndOfVolume);

  _sysconRadarInfoActive = false;
  _sysconScanSegActive = false;
  _sysconTsProcActive = false;
  _sysconXmitPowerActive = false;
  _sysconStartOfSweepActive = false;
  _sysconEndOfSweepActive = false;
  _sysconStartOfVolumeActive = false;
  _sysconEndOfVolumeActive = false;
  
  _angleNParts = 0;
  _anglePos = 0;
  spol_init(_latestAngle);
  spol_init(_prevAngle);
  
  _nSamplesTestPulse = 0;
  _testIqHc = NULL;
  _testIqHx = NULL;
  _testIqVc = NULL;
  _testIqVx = NULL;

  _testPowerDbHc = -9999;
  _testPowerDbHx = -9999;
  _testPowerDbVc = -9999;
  _testPowerDbVx = -9999;

  _testVelHc = -9999;
  _testVelHx = -9999;
  _testVelVc = -9999;
  _testVelVx = -9999;

  _meanAzError = 0;
  _meanElError = 0;
  _sdevAzError = 0;
  _sdevElError = 0;

  _scaleWarningPrinted = false;

  // set programe name
 
  _progName = "SpolTs2Fmq";

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

  // set up hearbeat func for reading from socket

  _heartBeatFunc = PMU_auto_register;
  if (_params.do_not_register_on_read) {
    _heartBeatFunc = NULL;
  }

  // initialize the output FMQ

  if (_openOutputFmq()) {
    isOK = false;
    return;
  }

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

  // allocate arrays for test pulse if needed

  if (_params.monitor_test_pulse) {
    _testIqHc = new RadarComplex_t[_params.test_pulse_n_samples];
    _testIqHx = new RadarComplex_t[_params.test_pulse_n_samples];
    _testIqVc = new RadarComplex_t[_params.test_pulse_n_samples];
    _testIqVx = new RadarComplex_t[_params.test_pulse_n_samples];
  }

  // load up cal if needed

  if (_params.override_calibration) {
    string errStr;
    if (_calib.readFromXmlFile(_params.cal_xml_file_path, errStr)) {
      cerr << "ERROR - SpolTs2Fmq" << endl;
      cerr << "  Cannot read cal from file: " << _params.cal_xml_file_path << endl;
      cerr << errStr << endl;
      isOK = false;
      return;
    }
  }

  return;
  
}

// destructor

SpolTs2Fmq::~SpolTs2Fmq()

{

  // close socket

  _sock.close();

  // close FMQs

  _outputFmq.closeMsgQueue();
  _sysconFmq.closeMsgQueue();

  // unregister process

  PMU_auto_unregister();

  // free up

  if (_testIqHc) delete[] _testIqHc;
  if (_testIqHx) delete[] _testIqHx;
  if (_testIqVc) delete[] _testIqVc;
  if (_testIqVx) delete[] _testIqVx;

}

//////////////////////////////////////////////////
// Run

int SpolTs2Fmq::Run ()
{

  PMU_auto_register("Run");

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Running SpolTs2Fmq - extra verbose debug mode" << endl;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running SpolTs2Fmq - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running SpolTs2Fmq - debug mode" << endl;
  }

  if (_params.debug) {
    cerr << "  OUTPUT FMQ: " << _params.output_fmq_path << endl;
    cerr << "    nSlots: " << _params.output_fmq_nslots << endl;
    cerr << "    nBytes: " << _params.output_fmq_size << endl;
  }

  if (_params.input_mode == Params::INPUT_FMQ) {
    return _runFmqMode();
  } else {
    return _runTcpMode();
  }

}
  
//////////////////////////////////////////////////
// Run in FMQ mode

int SpolTs2Fmq::_runFmqMode()
{

  PMU_auto_register("_runFmqMode");

  if (_params.debug) {
    cerr << "  Input mode: FMQ" << endl;
  }

  while (true) {

    PMU_auto_register("Opening FMQ");
    if (_params.debug) {
      cerr << "  Opening Fmq: " << _params.input_fmq_path << endl;
    }

    _inputFmq.setHeartbeat(PMU_auto_register);
    int msecsSleepBlocking = 100;
    
    if (_inputFmq.initReadBlocking(_params.input_fmq_path,
                                   "SpolTs2Fmq",
                                   _params.debug >= Params::DEBUG_EXTRA,
                                   Fmq::END, msecsSleepBlocking)) {
      cerr << "ERROR - SpolTs2Fmq::_runFmqMode" << endl;
      cerr << "  Cannot init FMQ for reading" << endl;
      cerr << "  Fmq: " << _params.input_fmq_path << endl;
      cerr << _inputFmq.getErrStr() << endl;
      umsleep(1000);
      continue;
    }

    // read data from the FMQ, send to client
    
    if (_readFromFmq()) {
      _inputFmq.closeMsgQueue();
      return -1;
    }

  } // while(true)

}

//////////////////////////////////////////////////
// Run in TCP mode

int SpolTs2Fmq::_runTcpMode()
{

  PMU_auto_register("_runTcpMode");

  if (_params.debug) {
    cerr << "  Input mode: TCP" << endl;
  }

  int iret = 0;

  while (true) {
    
    PMU_auto_register("Opening socket");
    
    // open socket to server
    
    if (_sock.open(_params.ts_tcp_server_host,
                   _params.ts_tcp_server_port,
                   10000)) {
      if (_sock.getErrNum() == Socket::TIMED_OUT) {
	if (_params.debug) {
	  cerr << "  Waiting for time series server to come up ..." << endl;
          cerr << "    host: " << _params.ts_tcp_server_host << endl;
          cerr << "    port: " << _params.ts_tcp_server_port << endl;
	}
      } else {
	if (_params.debug) {
	  cerr << "ERROR - SpolTs2Fmq::Run" << endl;
	  cerr << "  Connecting to server" << endl;
	  cerr << "  " << _sock.getErrStr() << endl;
	}
        iret = -1;
      }
      umsleep(1000);
      continue;
    }

    // read from the server
    
    if (_readFromTcpServer()) {
      iret = -1;
    }

    _sock.close();
    
  } // while(true)

  return iret;
  
}

/////////////////////////////
// read data from the FMQ

int SpolTs2Fmq::_readFromFmq()

{

  // initialize
  
  _msgNParts = 0;
  _msgPos = 0;
  
  // read data
  
  while (true) {
    
    PMU_auto_register("Reading data");
    
    // read next message

    const DsMsgPart *part = _getNextFromFmq();
    if (part == NULL) {
      cerr << "ERROR - TsFmq2Tcp::_readFromFmq" << endl;
      return -1;
    }
    
    // set ID and length

    _packetId = part->getType();
    _packetLen = part->getLength();

    if (iwrf_check_packet_id(_packetId)) {
      cerr << "ERROR - TsFmq2Tcp::_readFromFmq" << endl;
      cerr << "  Incorrect packet type - ignoring id: " << _packetId << endl;
      continue;
    }

    // load up buffer
    
    _msgBuf.reset();
    _msgBuf.add(part->getBuf(), part->getLength());

    // swap if needed
    
    iwrf_packet_swap(_msgBuf.getPtr(), _msgBuf.getLen());

    // check packet for validity
    
    if (_checkPacket()) {
      continue;
    }

    // handle the packet

    _handlePacket();

  } // while (true)

  return 0;

}

///////////////////////////////////////////
// get next message part from FMQ
// returns DsMsgPart object pointer on success, NULL on failure
// returns NULL at end of data, or error

const DsMsgPart *SpolTs2Fmq::_getNextFromFmq()
  
{
  
  while (_msgPos >= _msgNParts) {
    
    // we need a new message
    // blocking read registers with Procmap while waiting
    
    if (_inputFmq.readMsgBlocking()) {
      cerr << "ERROR - SpolTs2Fmq::_getNextPart" << endl;
      cerr << "  Cannot read message from FMQ" << endl;
      cerr << "  Fmq: " << _params.input_fmq_path << endl;
      return NULL;
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Got message from FMQ, len: " << _inputFmq.getMsgLen() << endl;
    }

    // disassemble the message
    
    if (_msg.disassemble(_inputFmq.getMsg(), _inputFmq.getMsgLen()) == 0) {
      _msgPos = 0;
      _msgNParts = _msg.getNParts();
    }
    
  } // while
  
  DsMsgPart *part = _msg.getPart(_msgPos);
  _msgPos++;

  return part;
  
}
/////////////////////////////
// read data from the server

int SpolTs2Fmq::_readFromTcpServer()

{

  // read data
  
  while (true) {
    
    if (!_params.do_not_register_on_read) {
      PMU_auto_register("Reading data");
    }

    // read packet from time series server server

    if (_readTcpMessage()) {
      if (!_sockTimedOut && !_unknownMsgId) {
        // error
        cerr << "ERROR - SpolTs2Fmq::_readFromServer" << endl;
        return -1;
      }
      // skip to next message
      continue;
    }
    
    // handle the packet

    _handlePacket();
    
  } // while (true)

  return 0;

}

///////////////////////////////////////////////////////////////////
// Read in next message from TCP, set id and load buffer.
// Returns 0 on success, -1 on failure

int SpolTs2Fmq::_readTcpMessage()
  
{

  while (true) {

    if (!_params.do_not_register_on_read) {
      PMU_auto_register("_readTcpMessage");
    }

    if (_readTcpPacket() == 0) {
      return 0;
    }
    
    if (!_sockTimedOut && !_unknownMsgId) {
      // socket error
      cerr << "ERROR - SpolTs2Fmq::_readTcpMessage" << endl;
      return -1;
    }

    if (_sockTimedOut && _params.write_end_of_vol_when_data_stops) {
      // if no data for a while, write end of volume event
      if (_timedOutCount == _params.nsecs_no_data_for_end_of_vol) {
        _writeEndOfVol();
      }
    }

    // read any pending syscon messages
    
    if (_params.merge_syscon_info) {
      _readSysconFromFmq();
    }

  } // while (true)

}

///////////////////////////////////////////////////////////////////
// Read in next packet, set id and load buffer.
// Returns 0 on success, -1 on failure

int SpolTs2Fmq::_readTcpPacket()

{

  si32 packetTop[2];

  // read the first 8 bytes (id, len), waiting for up to 1 second

  _unknownMsgId = false;
  _sockTimedOut = false;
    
  if (_sock.readBufferHb(packetTop, sizeof(packetTop),
                         sizeof(packetTop), _heartBeatFunc, 10000)) {

    // check for timeout
    if (_sock.getErrNum() == Socket::TIMED_OUT) {
      if (!_params.do_not_register_on_read) {
        PMU_auto_register("Timed out ...");
      }
      _sockTimedOut = true;
      _timedOutCount++;
      if (_params.debug) {
        cerr << "Timed out reading time series server, host, port: "
             << _params.ts_tcp_server_host << ", "
             << _params.ts_tcp_server_port << endl;
      }
    } else {
      _sockTimedOut = false;
      cerr << "ERROR - SpolTs2Fmq::_readTcpPacket" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
    }
    return -1;
  }
  _timedOutCount = 0;
  
  // check ID for packet, and get its length

  _packetId = packetTop[0];
  _packetLen = packetTop[1];

  // make sure the size is reasonable
  
  if (_packetLen > 1000000 ||
      _packetLen < (int) sizeof(packetTop)) {
    cerr << "ERROR - SpolTs2Fmq::_readTcpPacket" << endl;
    cerr << "  Bad packet length: " << _packetLen << endl;
    fprintf(stderr, "  len: 0x%x\n", _packetLen);
    fprintf(stderr, "  id: 0x%x\n", _packetId);
    cerr << "  Need to reconnect to server to resync" << endl;
    return -1;
  }
  
  // compute nbytes still to read in
  
  int nBytesRemaining = _packetLen - sizeof(packetTop);

  // reserve space in the buffer
  
  _msgBuf.reserve(_packetLen);
  
  // copy the packet top into the start of the buffer
  
  memcpy(_msgBuf.getPtr(), packetTop, sizeof(packetTop));
  char *readPtr = (char *) _msgBuf.getPtr() + sizeof(packetTop);
  
  // read in the remainder of the buffer
  
  if (_sock.readBufferHb(readPtr, nBytesRemaining, 1024,
                        _heartBeatFunc, 10000)) {
    cerr << "ERROR - SpolTs2Fmq::_readTcpPacket" << endl;
    cerr << "  " << _sock.getErrStr() << endl;
    return -1;
  }

  // check packet for validity

  if (_checkPacket()) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////
// check packet for validity
// returns 0 on success, -1 on error

int SpolTs2Fmq::_checkPacket()

{

  // check that we have a valid message

  if (iwrf_check_packet_id(_packetId, _packetLen)) {
    // unknown packet type, read it in and continue without processing
    _unknownCount++;
    if (_unknownCount > 1000) {
      cerr << "ERROR - SpolTs2Fmq::_readTcpPacket" << endl;
      cerr << "  Too many bad packets" << endl;
      _unknownCount = 0;
      return -1;
    }
    _unknownMsgId = true;
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_NORM &&
      _packetId != IWRF_PULSE_HEADER_ID &&
      _packetId != IWRF_BURST_HEADER_ID &&
      _packetId != IWRF_RVP8_PULSE_HEADER_ID) {
    cerr << "Read in TCP packet, id, len: "
         << iwrf_packet_id_to_str(_packetId) << ", "
         << _packetLen << endl;

    if(_pulseCount > 0 && _packetId == IWRF_SYNC_ID) {
      cerr << "Read " << _pulseCount
           << " Pulse packets since last sync" << endl;
      _pulseCount = 0;  
    }
  } else {
    _pulseCount++;  // Keep track of pulse packets
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Read in TCP packet, id, len: "
           << iwrf_packet_id_to_str(_packetId) << ", "
           << _packetLen << endl;
    } else if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "p";
    }
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA && 
      _packetId != IWRF_PULSE_HEADER_ID && _packetId != IWRF_SYNC_ID) {
    iwrf_packet_print(stderr, _msgBuf.getPtr(), _msgBuf.getLen());
  }
  
  if(_params.debug >=  Params::DEBUG_EXTRA && 
     (_packetId == IWRF_PULSE_HEADER_ID || _packetId == IWRF_SYNC_ID))   {
    iwrf_packet_print(stderr, _msgBuf.getPtr(), _msgBuf.getLen());
  }

  // copy in latest info

  memcpy(&_latestPacketInfo, _msgBuf.getPtr(), sizeof(iwrf_packet_info_t));

  return 0;

}

/////////////////////////////
// handle a packet

void SpolTs2Fmq::_handlePacket()

{
  
  // if appropriate, merge in syscon data
  
  if (_params.merge_syscon_info) {
    _readSysconFromFmq();
  }
  
  // if appropriate, merge in status xml
  
  if (_params.merge_secondary_status_from_fmq) {
    _readSecondaryStatusFromFmq();
  }
  
  // handle packet types
  
  if (_packetId == IWRF_PULSE_HEADER_ID) {
    
    _handlePulse();
    
  } else if (_packetId == IWRF_RADAR_INFO_ID) {
    
    // radar info - make local copy
    
    iwrf_radar_info_t *radar = (iwrf_radar_info_t *) _msgBuf.getPtr();
    _tsRadarInfo = *radar;
    
    // add to FMQ
    
    _writeRadarInfoToFmq();
    
  } else if (_packetId == IWRF_SCAN_SEGMENT_ID) {
    
    // scan segment - make local copy
    
    iwrf_scan_segment_t *scan = (iwrf_scan_segment_t *) _msgBuf.getPtr();
    _tsScanSeg = *scan;
    
    // add to FMQ
    
    _writeScanSegmentToFmq();
    
  } else if (_packetId == IWRF_TS_PROCESSING_ID) {
    
    // ts processing - make local copy
    
    iwrf_ts_processing_t *proc = (iwrf_ts_processing_t *) _msgBuf.getPtr();
    _tsTsProc = *proc;
    
    // add to FMQ
    
    _writeTsProcessingToFmq();
    
  } else if (_packetId == IWRF_XMIT_POWER_ID) {
    
    // xmit power - make local copy
    
    iwrf_xmit_power_t *power = (iwrf_xmit_power_t *) _msgBuf.getPtr();
    _tsXmitPower = *power;
    
    // add to FMQ
    
    _writeXmitPowerToFmq();
    
  } else if (_packetId == IWRF_STATUS_XML_ID) {
    
    _handleStatusXml();
    
  } else if (_packetId == IWRF_CALIBRATION_ID) {
    
    _handleCalibration();
    
  } else {
    
    // other packet type
    // add to outgoing message
    
    _outputMsg.addPart(_packetId, _packetLen, _msgBuf.getPtr());
    
  }

  // if the message is large enough, write to the FMQ
  
  _writeToOutputFmq();
  
    // check that status XML is up to date
  
  if (_params.augment_status_xml) {
    _checkStatusXml();
  }

}

/////////////////////////////
// handle pulse data

void SpolTs2Fmq::_handlePulse()

{

  // pulse header and data
  
  iwrf_pulse_header_t &pHdr = *((iwrf_pulse_header_t *) _msgBuf.getPtr());

  // adjust time as required

  if (_params.pulse_time_adjustment_secs != 0) {
    double pulseTime =
      (double) pHdr.packet.time_secs_utc + pHdr.packet.time_nano_secs / 1.0e9;
    double correctedTime = pulseTime + _params.pulse_time_adjustment_secs;
    pHdr.packet.time_secs_utc = (si64) correctedTime;
    pHdr.packet.time_nano_secs =
      (int) ((correctedTime - pHdr.packet.time_secs_utc) * 1.0e9 + 0.5);
  }
  
  // scale data as applicable
  
  if(_params.apply_scale) {
    _applyIQScale();
  }
  
  // monitor test pulse as appropriate
  
  if (_params.monitor_test_pulse) {
    _monitorTestPulse();
  }
  
  // modify the pulse header
  
  if (_params.merge_syscon_info) {
    _modifyPulseHeaderFromSyscon(pHdr);
  }
  
  // merge angles?
  
  if (_params.merge_antenna_angles) {
    _mergeAntennaAngles(pHdr); 
  } else if (_params.check_angles_for_status_xml) {
    _checkAntennaAngles(pHdr); 
  }
  
  // modify the sweep number if appropriate
  
  _modifySweepNumber(pHdr); 
  
  // add to message
  
  _outputMsg.addPart(_packetId, _packetLen, _msgBuf.getPtr());
  
  // delay as needed
  
  _doReadDelay(pHdr.packet);

}

/////////////////////////////////////////////
// modify sweep number

void SpolTs2Fmq::_modifySweepNumber(iwrf_pulse_header_t &pHdr)

{
  
  _volNum = pHdr.volume_num;
  _sweepNum = pHdr.sweep_num;
  
  if (_volNum != _prevVolNum) {
    _volStartSweepNum = _sweepNum;
    _prevVolNum = _volNum;
  }
  
  if (_params.zero_sweep_number_at_start_of_vol) {
    pHdr.sweep_num = _sweepNum - _volStartSweepNum;
  }
  
  // delay sweep number if appropriate
  // this delays the change in the sweep number until the antenna
  // direction changes
  
  if (_params.delay_sweep_num_change_in_sector_scan) {
    _delaySweepNumChange(pHdr);
  }

}

/////////////////////////////////////////////////////////
// delay the sweep number change, in sector mode, until
// the antenna turns around

void SpolTs2Fmq::_delaySweepNumChange(iwrf_pulse_header_t &pHdr)

{

  if (pHdr.scan_mode != IWRF_SCAN_MODE_SECTOR) {
    return;
  }

  // initialization

  if (!_sweepNumChangeInit) {
    _sweepNumChangeInProgress = false;
    _sweepNumChangeInit = true;
    _sweepNumOld = pHdr.sweep_num;
    _sweepNumAzOld = pHdr.azimuth;
    return;
  }
  
  // check for sweep num change

  int sweepNum = pHdr.sweep_num;
  if (sweepNum == _sweepNumOld) {
    // no change
    _sweepNumChangeInProgress = false;
    _sweepNumAzOld = pHdr.azimuth;
    return;
  }

  // sweep number changed
  // are we already processing it?

  if (!_sweepNumChangeInProgress) {

    double az = pHdr.azimuth;
    if (az == _sweepNumAzOld) {
      // wait for measured az change
      pHdr.sweep_num = _sweepNumOld;
      return;
    }
    
    // first pulse of transition
    // compute delta az, get direction

    double deltaAz = RadarComplex::diffDeg(az, _sweepNumAzOld);
    _sweepNumAzOld = az;
    if (deltaAz >= 0) {
      _sweepNumTransDirn = 1.0;
    } else {
      _sweepNumTransDirn = -1.0;
    }

    _sweepNumChangeInProgress = true;
    pHdr.sweep_num = _sweepNumOld;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "==>> sweep number changed" << endl;
      cerr << "       old sweep num" << _sweepNumOld << endl;
      cerr << "       new sweep num" << sweepNum << endl;
      cerr << "       az: " << az << endl;
      cerr << "       deltaAz: " << deltaAz << endl;
      cerr << "       transDirn: " << _sweepNumTransDirn << endl;
      cerr << "       DELAYING" << endl;
    }

    return;
    
  }
  
  // waiting for measurable az change

  double az = pHdr.azimuth;
  if (az == _sweepNumAzOld) {
    // wait for measured az change
    pHdr.sweep_num = _sweepNumOld;
    return;
  }

  // waiting for turn around

  double deltaAz = RadarComplex::diffDeg(az, _sweepNumAzOld);
  _sweepNumAzOld = az;
  if (deltaAz * _sweepNumTransDirn < 0) {
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "==>> antenna changed dirn" << endl;
      cerr << "       new sweep num" << sweepNum << endl;
      cerr << "       az: " << az << endl;
      cerr << "       deltaAz: " << deltaAz << endl;
      cerr << "       CHANGING TO NEW SWEEP NUM" << endl;
    }
    
    // change in direction, so transition is complete
    // leave sweep number unaltered
    _sweepNumChangeInProgress = false;
    _sweepNumOld = sweepNum;
    return;

  }

  // no turnaround yet

  pHdr.sweep_num = _sweepNumOld;
  return;
 
}

///////////////////////////////////////
// open the output FMQ
// returns 0 on success, -1 on failure

int SpolTs2Fmq::_openOutputFmq()

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
// write to output FMQ if ready
// returns 0 on success, -1 on failure

int SpolTs2Fmq::_writeToOutputFmq(bool force)

{

  // if the message is large enough, write to the FMQ
  
  int nParts = _outputMsg.getNParts();
  if (!force && nParts < _params.n_pulses_per_message) {
    return 0;
  }

  PMU_auto_register("writeToFmq");

  void *buf = _outputMsg.assemble();
  int len = _outputMsg.lengthAssembled();
  if (_outputFmq.writeMsg(0, 0, buf, len)) {
    cerr << "ERROR - SpolTs2Fmq" << endl;
    cerr << "  Cannot write FMQ: " << _params.output_fmq_path << endl;
    _outputMsg.clearParts();
    return -1;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "  Wrote msg, nparts, len: "
         << nParts << ", " << len << endl;
  }
  _outputMsg.clearParts();

  return 0;

}
    
///////////////////////////////////////
// write end-of-vol to output FMQ
// returns 0 on success, -1 on failure

int SpolTs2Fmq::_writeEndOfVol()

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

///////////////////////////////////////
// open the angle FMQ
// returns 0 on success, -1 on failure

int SpolTs2Fmq::_openAngleFmq()

{
  
  Fmq::openPosition initPos = Fmq::END;
  if (_params.position_angle_fmq_at_start) {
    initPos = Fmq::START;
  }
  
  if (_angleFmq.initReadBlocking
      (_params.angle_fmq_path,
       _progName.c_str(),
       _params.debug >= Params::DEBUG_EXTRA, // set debug?
       initPos)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot initialize FMQ: " << _params.angle_fmq_path << endl;
    cerr << _angleFmq.getErrStr() << endl;
    return -1;
  }

  _angleMsg.clearAll();
  _angleMsg.setType(0);

  return 0;

}

/////////////////////////////////////////////////
// get next antenna angle message
//
// Returns 0 on success, -1 on failure

int SpolTs2Fmq::_getNextAngle()
  
{

  PMU_auto_register("Get next angle part");
  
  // check we have an open FMQ
  
  if (!_angleFmq.isOpen()) {
    if (_openAngleFmq()) {
      return -1;
    }
  }
  
  if (_anglePos >= _angleNParts) {
    
    // we need a new message
    
    if (_angleFmq.readMsgBlocking()) {
      cerr << "ERROR - SpolTs2Fmq::_getNextAngle" << endl;
      cerr << "  Cannot read message from FMQ" << endl;
      cerr << "  Fmq: " << _params.angle_fmq_path << endl;
      cerr << _angleFmq.getErrStr() << endl;
      _angleFmq.closeMsgQueue();
      return -1;
    }

    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "--->> Got angle FMQ message" << endl;
    }
    
    // disassemble the message
    
    const void *msg = _angleFmq.getMsg();
    int len = _angleFmq.getMsgLen();
    if (_angleMsg.disassemble(msg, len) == 0) {
      _anglePos = 0;
      _angleNParts = _angleMsg.getNParts();
    }
    
  } // if (_anglePos >= _angleNParts)

  _anglePart = _angleMsg.getPart(_anglePos);
  _anglePos++;

  if (_anglePart->getType() != SPOL_SHORT_ANGLE_ID ||
      _anglePart->getLength() != sizeof(spol_short_angle_t)) {
    cerr << "ERROR - SpolTs2Fmq::_getNextAngle" << endl;
    cerr << "  Bad angle message part" << endl;
    fprintf(stderr, "  id: 0x%x\n", _anglePart->getType());
    cerr << "  length: " << _anglePart->getLength() << endl;
    return -1;
  }

  memcpy(&_prevAngle, &_latestAngle, sizeof(spol_short_angle_t));
  memcpy(&_latestAngle, _anglePart->getBuf(), sizeof(spol_short_angle_t));

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "========== latest antenna angle ==============" << endl;
    spol_print(stderr, _latestAngle);
    cerr << "==============================================" << endl;
  }

  return 0;

}

/////////////////////////////////////////////
// merge antenna angles

void SpolTs2Fmq::_mergeAntennaAngles(iwrf_pulse_header_t &pHdr)

{

  /// compute time relative to pulse seconds, to preserve precision

  si64 baseSecs = pHdr.packet.time_secs_utc;

  double pulseTime =
    _getTimeAsDouble(baseSecs, pHdr.packet.time_secs_utc, pHdr.packet.time_nano_secs);
  double prevTime = 
    _getTimeAsDouble(baseSecs, _prevAngle.time_secs_utc, _prevAngle.time_nano_secs);

  if (pulseTime < prevTime) {
    // pulse is in the past, use the previous angle
    pHdr.elevation = _prevAngle.elevation;
    pHdr.azimuth = _prevAngle.azimuth;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "DEBUG - SpolTs2Fmq::_mergeAntennaAngles" << endl;
      cerr << "========== using prev angle ==============" << endl;
      spol_print(stderr, _prevAngle);
      cerr << "==========================================" << endl;
    }
    return;
  }
  
  // get more angles

  double latestTime = 
    _getTimeAsDouble(baseSecs, _latestAngle.time_secs_utc,
                     _latestAngle.time_nano_secs);
  int count = 0;
  while (true) {

    PMU_auto_register("merge antenna angles");

    if (pulseTime > latestTime) {
      if (_getNextAngle()) {
        // error - use latest available
        pHdr.elevation = _latestAngle.elevation;
        pHdr.azimuth = _latestAngle.azimuth;
        return;
      }
      latestTime = _getTimeAsDouble(baseSecs, _latestAngle.time_secs_utc,
                                    _latestAngle.time_nano_secs);
    }

    if (pulseTime <= latestTime) {

      // interpolate to get the angles

      double prevAz = _prevAngle.azimuth;
      double prevEl = _prevAngle.elevation;
      double latestAz = _latestAngle.azimuth;
      double latestEl = _latestAngle.elevation;

      double interpEl, interpAz;

      _interpAngles(prevEl, latestEl, prevAz, latestAz,
                    pulseTime, prevTime, latestTime,
                    interpEl, interpAz);
      
      pHdr.elevation = interpEl;
      pHdr.azimuth = interpAz;
      
      if (_params.debug >= Params::DEBUG_EXTRA) {
        cerr << "DEBUG - SpolTs2Fmq::_mergeAntennaAngles" << endl;
        cerr << "========== using interpolated angle ==============" << endl;
        cerr << "  el, az: " << interpEl << ", " << interpAz << endl;
        cerr << "============================================" << endl;
      }
      return;
    }

    count++;

    if (count % 1000 == 0) {
      if (_params.debug) {
        cerr << "WARNING - SpolTs2Fmq::_mergeAntennaAngles" << endl;
        cerr << "  Trying to merge with antenna angles" << endl;
        cerr << "  No luck so far" << endl;
      }
      return;
    }

  } // while (true)

}

/////////////////////////////////////////////
// check antenna angles for errors relative
// to the S2D

void SpolTs2Fmq::_checkAntennaAngles(iwrf_pulse_header_t &pHdr)

{

  /// compute time relative to pulse seconds, to preserve precision

  si64 baseSecs = pHdr.packet.time_secs_utc;
  
  double pulseTime =
    _getTimeAsDouble(baseSecs, pHdr.packet.time_secs_utc,
                     pHdr.packet.time_nano_secs);
  double prevTime = 
    _getTimeAsDouble(baseSecs, _prevAngle.time_secs_utc,
                     _prevAngle.time_nano_secs);

  if (pulseTime < prevTime) {
    // pulse is in the past
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "DEBUG - SpolTs2Fmq::_checkAntennaAngles" << endl;
      cerr << "========== using prev angle ==============" << endl;
      spol_print(stderr, _prevAngle);
      cerr << "==========================================" << endl;
    }
    return;
  }
  
  // get more angles

  string flagStr;
  double latestTime = 
    _getTimeAsDouble(baseSecs, _latestAngle.time_secs_utc,
                     _latestAngle.time_nano_secs);
  int count = 0;
  while (true) {

    PMU_auto_register("merge antenna angles");

    if (pulseTime > latestTime) {
      if (_getNextAngle()) {
        // error
        return;
      }
      flagStr += "-";
      latestTime = _getTimeAsDouble(baseSecs, _latestAngle.time_secs_utc,
                                    _latestAngle.time_nano_secs);
    } else {
      flagStr += "#";
    }

    if (pulseTime <= latestTime) {

      // interpolate to get the angles

      double prevAz = _prevAngle.azimuth;
      double prevEl = _prevAngle.elevation;
      double latestAz = _latestAngle.azimuth;
      double latestEl = _latestAngle.elevation;

      double s2dEl, s2dAz;

      _interpAngles(prevEl, latestEl, prevAz, latestAz,
                    pulseTime, prevTime, latestTime,
                    s2dEl, s2dAz);
      
      // compute errors

      double errorEl = pHdr.elevation - s2dEl;
      double errorAz = pHdr.azimuth - s2dAz;
      if (errorAz > 180) {
        errorAz -= 360.0;
      } else if (errorAz < -180) {
        errorAz += 360.0;
      }
      if (errorEl > 180) {
        errorEl -= 360.0;
      } else if (errorEl < -180) {
        errorEl += 360.0;
      }

      // accumulate for stats, compute if ready

      _azErrors.push_back(errorAz);
      _elErrors.push_back(errorEl);
      _computeAngleStats();

      if (_params.correct_data_angles_using_s2d_angles) {
        _correctAngles(pHdr, errorEl, errorAz, s2dEl, s2dAz);
      }
      
      if (fabs(errorEl) > 0.02 || fabs(errorAz) > 0.02) {
        flagStr += "*";
      }

      if (_params.print_angle_errors) {
        fprintf(stderr,
                "RVP8 el, az, S2D el, az, error el, az: "
                "%8.3f %8.3f %8.3f %8.3f %8.3f %8.3f %s\n",
                pHdr.elevation, pHdr.azimuth,
                s2dEl, s2dAz,
                errorEl, errorAz, flagStr.c_str());
      }
        
      if (_params.debug >= Params::DEBUG_EXTRA) {
        cerr << "DEBUG - SpolTs2Fmq::_checkAntennaAngles" << endl;
        cerr << "========== using interpolated angle ==============" << endl;
        cerr << "  el, az: " << s2dEl << ", " << s2dAz << endl;
        cerr << "============================================" << endl;
      }
      return;
    }

    count++;

    if (count % 1000 == 0) {
      if (_params.debug) {
        cerr << "WARNING - SpolTs2Fmq::_checkAntennaAngles" << endl;
        cerr << "  Trying to merge with antenna angles" << endl;
        cerr << "  No luck so far" << endl;
      }
      return;
    }

  } // while (true)

}

/////////////////////////////////////////////
// sum the angle stats
//
// optionally censor outliers outside 1 stddev from mean

void SpolTs2Fmq::_sumAngleStats(bool censorOutliers)

{ 
  
  double upperAzErrorLimit = 1.0e99;
  double lowerAzErrorLimit = -1.0e99;
  double upperElErrorLimit = 1.0e99;
  double lowerElErrorLimit = -1.0e99;

  if (censorOutliers) {
    // censor data outside 1 std dev from mean
    upperAzErrorLimit = _meanAzError + _sdevAzError;
    lowerAzErrorLimit = _meanAzError - _sdevAzError;
    upperElErrorLimit = _meanElError + _sdevElError;
    lowerElErrorLimit = _meanElError - _sdevElError;
  }

  _maxAzError = 0;
  _maxElError = 0;
  _sumAzError = 0;
  _sumElError = 0;
  _sumsqAzError = 0;
  _sumsqElError = 0;
  _nErrorAz = 0;
  _nErrorEl = 0;

  for (size_t ii = 0; ii < _azErrors.size(); ii++) {
    
    double errorAz = _azErrors[ii];
    double errorEl = _elErrors[ii];

    if (errorAz > lowerAzErrorLimit && errorAz < upperAzErrorLimit) {
      _maxAzError = MAX(_maxAzError, fabs(errorAz));
      _sumAzError += errorAz;
      _sumsqAzError += errorAz * errorAz;
      _nErrorAz++;
    }

    if (errorEl > lowerElErrorLimit && errorEl < upperElErrorLimit) {
      _maxElError = MAX(_maxElError, fabs(errorEl));
      _sumElError += errorEl;
      _sumsqElError += errorEl * errorEl;
      _nErrorEl++;
    }

  }

}

/////////////////////////////////////////////
// compute angle stats

void SpolTs2Fmq::_computeAngleStats()
  
{ 

  if ((int) _azErrors.size() < _params.n_samples_for_angle_error_stats) {
    return;
  }

  _angleErrorsLatestTime = time(NULL);

  // sum stats using all entries

  _sumAngleStats(false);

  // compute mean and standard deviation

  _meanAzError = _sumAzError / _nErrorAz;
  _meanElError = _sumElError / _nErrorEl;
  _sdevAzError =
    sqrt(fabs((_sumsqAzError / _nErrorAz) - _meanAzError * _meanAzError));
  _sdevElError =
    sqrt(fabs((_sumsqElError / _nErrorEl) - _meanElError * _meanElError));

  // again sum stats, but do not include entries which
  // deviate from the mean by more than 1 std deviation

  _sumAngleStats(true);

  // recompute the mean using the censored data

  if (_nErrorAz > 0) {
    _meanAzError = _sumAzError / _nErrorAz;
  } else {
    _meanAzError = 0.0;
  }
  if (_nErrorEl > 0) {
    _meanElError = _sumElError / _nErrorEl;
  } else {
    _meanElError = 0.0;
  }

  // load up XML

  _angleErrorsXml.clear();
  _angleErrorsXml += TaXml::writeStartTag(_params.angle_errors_xml_tag, 0);
  _angleErrorsXml += TaXml::writeTime("Time", 1, _angleErrorsLatestTime);
  _angleErrorsXml += TaXml::writeDouble("AzErrorMax", 1, _maxAzError);
  _angleErrorsXml += TaXml::writeDouble("ElErrorMax", 1, _maxElError);
  _angleErrorsXml += TaXml::writeDouble("AzErrorMean", 1, _meanAzError);
  _angleErrorsXml += TaXml::writeDouble("ElErrorMean", 1, _meanElError);
  _angleErrorsXml += TaXml::writeDouble("AzErrorSdev", 1, _sdevAzError);
  _angleErrorsXml += TaXml::writeDouble("ElErrorSdev", 1, _sdevElError);
  _angleErrorsXml += TaXml::writeEndTag(_params.angle_errors_xml_tag, 0);
    
  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "======================================" << endl;
    cerr << _angleErrorsXml << endl;
    cerr << "======================================" << endl;
  }

  // clear vectors

  _azErrors.clear();
  _elErrors.clear();

}

////////////////////////////////////////////////////////////
// get time as a double, relative to the base seconds

double SpolTs2Fmq::_getTimeAsDouble(si64 baseSecs, si64 secs, si32 nanoSecs)
  
{
  double dsecs = (double) (secs - baseSecs);
  double dtime = dsecs + nanoSecs / 1.0e9;
  return dtime;
}
  
////////////////////////////////////////////////////////////
// interpolate the angle between the given times

void SpolTs2Fmq::_interpAngles(double prevEl, double latestEl,
                               double prevAz, double latestAz,
                               double pulseTime,
                               double prevTime, double latestTime,
                               double &interpEl, double &interpAz)
  
{

  // compute the interpolation fraction

  double timeFraction = (pulseTime - prevTime) / (latestTime - prevTime);

  // compute delta angles

  double deltaEl = latestEl - prevEl;
  double deltaAz = latestAz - prevAz;

  // constrain the differences from -180 to 180

  if (deltaEl < -180) {
    deltaEl += 360.0;
  } else if (deltaEl > 180.0) {
    deltaEl -= 360.0;
  }

  if (deltaAz < -180) {
    deltaAz += 360.0;
  } else if (deltaAz > 180.0) {
    deltaAz -= 360.0;
  }

  // compute the interpolated angles

  double el = prevEl + deltaEl * timeFraction;
  double az = prevAz + deltaAz * timeFraction;

  // constrain the angles from 0 to 360

  if (el < -180) {
    el += 360.0;
  } else if (el > 180.0) {
    el -= 360.0;
  }

  if (az < -180) {
    az += 360.0;
  } else if (az > 180.0) {
    az -= 360.0;
  }

  interpEl = el;
  interpAz = az;

}
  
////////////////////////////////////////////////////////////
// Correct the angles in the data stream, if in error,
// overwriting with the S2D angles.

void SpolTs2Fmq::_correctAngles(iwrf_pulse_header_t &pHdr,
                                double errorEl,
                                double errorAz,
                                double s2dEl,
                                double s2dAz)

{

  if (fabs(errorEl) > _params.angle_error_correction_threshold) {
    if (_params.debug) {
      fprintf(stderr,
              "FIXING el angle, time, badVal(time series), goodVal(s2D): "
              "%s.%.3d, %.4f, %.4f\n",
              DateTime::strm(pHdr.packet.time_secs_utc).c_str(),
              pHdr.packet.time_nano_secs / 1000000,
              pHdr.elevation,
              s2dEl);
    }
    pHdr.elevation = s2dEl;
  }

  if (fabs(errorAz) > _params.angle_error_correction_threshold) {
    if (s2dAz < 0) {
      s2dAz += 360.0;
    } else if (s2dAz > 360) {
      s2dAz -= 360.0;
    }
    if (_params.debug) {
      fprintf(stderr,
            "FIXING az angle, time, badVal(time series), goodVal(s2D): "
              "%s.%.3d, %.4f, %.4f\n",
              DateTime::strm(pHdr.packet.time_secs_utc).c_str(),
              pHdr.packet.time_nano_secs / 1000000,
              pHdr.azimuth,
              s2dAz);
    }
    pHdr.azimuth = s2dAz;
  }

}

////////////////////////////////////////////////////////////
// open the output FMQ for monitoring
// returns 0 on success, -1 on failure

int SpolTs2Fmq::_openSysconFmq()
  
{
  
  if (_sysconFmq.initReadOnly
      (_params.syscon_fmq_path,
       _progName.c_str(),
       _params.debug >= Params::DEBUG_EXTRA,
       Fmq::END, 0)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot open FMQ for syscon info: "
         << _params.syscon_fmq_path << endl;
    cerr << _sysconFmq.getErrStr() << endl;
    return -1;
  }

  return 0;

}

///////////////////////////////////////////
// read syscon information if available
// Returns 0 on success, -1 on failure

int SpolTs2Fmq::_readSysconFromFmq()
  
{

  PMU_auto_register("Reading syscon info");
  
  // check we have an open FMQ
  
  if (!_sysconFmq.isOpen()) {
    if (_openSysconFmq()) {
      return -1;
    }
  }

  // read in all available syscon messages

  while (true) {

    PMU_auto_register("Reading syscon info");
    
    // read in a message
    
    bool gotOne;
    if (_sysconFmq.readMsg(&gotOne)) {
      cerr << "ERROR -  SpolTs2Fmq::_readSysconInfo" << endl;
      cerr << "  Cannot read syscon from FMQ" << endl;
      cerr << "  Fmq: " << _params.syscon_fmq_path << endl;
      cerr << _sysconFmq.getErrStr() << endl;
      _sysconFmq.closeMsgQueue();
      return -1;
    }

    if (!gotOne) {
      // no data
      return 0;
    }
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "--->> Got Syscon message" << endl;
    }
    
    const void *msg = _sysconFmq.getMsg();
    int len = _sysconFmq.getMsgLen();
    int msgType = _sysconFmq.getMsgType();
    
    switch (msgType) {

      case IWRF_RADAR_INFO_ID:
        if (len == sizeof(iwrf_radar_info_t)) {
          _sysconRadarInfoActive = true;
          memcpy(&_sysconRadarInfo, msg, sizeof(iwrf_radar_info_t));
          iwrf_radar_info_print(stderr, _sysconRadarInfo);
          _setReadDelay(_sysconRadarInfo.packet);
          _writeRadarInfoToFmq();
        }
        break;

      case IWRF_SCAN_SEGMENT_ID:
        if (len == sizeof(iwrf_scan_segment_t)) {
          _sysconScanSegActive = true;
          memcpy(&_sysconScanSeg, msg, sizeof(_sysconScanSeg));
          _setReadDelay(_sysconScanSeg.packet);
          _writeScanSegmentToFmq();
        }
        break;
          
      case IWRF_TS_PROCESSING_ID:
        if (len == sizeof(iwrf_ts_processing_t)) {
          _sysconTsProcActive = true;
          memcpy(&_sysconTsProc, msg, sizeof(_sysconTsProc));
          _setReadDelay(_sysconTsProc.packet);
          _writeTsProcessingToFmq();
        }
        break;

      case IWRF_XMIT_POWER_ID:
        if (_params.merge_xmit_power_from_syscon) {
          if (len == sizeof(iwrf_xmit_power_t)) {
            _sysconXmitPowerActive = true;
            memcpy(&_sysconXmitPower, msg, sizeof(_sysconXmitPower));
            _setReadDelay(_sysconXmitPower.packet);
            _writeXmitPowerToFmq();
          }
        }
        break;

      case IWRF_EVENT_NOTICE_ID:
        if (len == sizeof(iwrf_event_notice_t)) {
          iwrf_event_notice_t event;
          memcpy(&event, msg, sizeof(event));
          if (event.start_of_sweep) {
            _sysconStartOfSweepPrev = _sysconStartOfSweep;
            _sysconStartOfSweep = event;
            _sysconStartOfSweepActive = true;
          }
          if (event.end_of_sweep) {
            _sysconEndOfSweepPrev = _sysconEndOfSweep;
            _sysconEndOfSweep = event;
            _sysconEndOfSweepActive = true;
          }
          if (event.start_of_volume) {
            _sysconStartOfVolume = event;
            _sysconStartOfVolumeActive = true;
          }
          if (event.end_of_volume) {
            _sysconEndOfVolume = event;
            _sysconStartOfVolumeActive = true;
          }
          _setReadDelay(event.packet);
          _writeSysconEventToFmq(event);
          if (_params.debug >= Params::DEBUG_EXTRA) {
            iwrf_event_notice_print(stderr, event);
          }
        }
        break;

      default: {}
        
    } // switch
    
  } // while (true)
  
  return 0;

}

/////////////////////////////////////////////////////
// set the read delay by comparing latest pulse time
// with latest syscon packet time

void SpolTs2Fmq::_setReadDelay(iwrf_packet_info &sysconPacket)

{

  if (!_params.sync_syscon_info_with_pulses) {
    return;
  }

  double sysconSecs = iwrf_get_packet_time_as_double(sysconPacket);
  double nowSecs = DateTime::getCurrentTimeAsDouble();

  double sysconLateSecs = nowSecs - sysconSecs;

  if (!_sysconLateSet) {
    _sysconLateSecs = sysconLateSecs;
  } else {
    double alpha = 0.1;
    _sysconLateSecs = alpha * sysconLateSecs + (1.0 - alpha) * _sysconLateSecs;
  }
  
  _sysconLateSet = true;

}

///////////////////////////////////////////////////////////
// delay reading pulses based on previously computed value

void SpolTs2Fmq::_doReadDelay(iwrf_packet_info &pulsePacket)

{

  if (!_params.merge_syscon_info) {
    // no delay needed
    return;
  }

  if (!_sysconLateSet) {
    // not yet set up
    return;
  }

  double pulseSecs = iwrf_get_packet_time_as_double(pulsePacket);
  double nowSecs = DateTime::getCurrentTimeAsDouble();
  double pulseLateSecs = nowSecs - pulseSecs;

  if (pulseLateSecs < _sysconLateSecs) {
    int usecsWait = (int) ((_sysconLateSecs - pulseLateSecs) * 1.5e6);
    cerr << "====>> usecsWait: " << usecsWait << endl;
    uusleep(usecsWait);
    PMU_auto_register("Delaying pulse");
  }

}

/////////////////////////////////////////////
// write radar info to FMQ

void SpolTs2Fmq::_writeRadarInfoToFmq()
  
{

  if (_params.debug) {
    cerr << "Writing radar info to FMQ" << endl;
  }

  iwrf_radar_info_t radarInfo;
  if (_sysconRadarInfoActive) {
    // merge syscon with time series
    radarInfo = _sysconRadarInfo;
    radarInfo.wavelength_cm = _tsRadarInfo.wavelength_cm;
    memcpy(radarInfo.site_name, _tsRadarInfo.site_name,
           IWRF_MAX_SITE_NAME);
  } else {
    // use time series
    radarInfo = _tsRadarInfo;
  }
  _info.setRadarInfo(radarInfo);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_radar_info_print(stderr, radarInfo);
  }
  
  // write to FMQ
  
  _outputMsg.addPart(IWRF_RADAR_INFO_ID, sizeof(radarInfo), &radarInfo);

}

/////////////////////////////////////////////
// write scan segment to FMQ

void SpolTs2Fmq::_writeScanSegmentToFmq()
  
{
  
  if (_params.debug) {
    cerr << "Writing scan segment to FMQ" << endl;
  }

  iwrf_scan_segment_t scanSeg;
  if (_sysconScanSegActive) {
    // use syscon
    scanSeg = _sysconScanSeg;
  } else {
    // use time series
    scanSeg = _tsScanSeg;
  }
  _info.setScanSegment(scanSeg);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_scan_segment_print(stderr, scanSeg);
  }
  
  // write to FMQ
  
  _outputMsg.addPart(IWRF_SCAN_SEGMENT_ID, sizeof(scanSeg), &scanSeg);

}

/////////////////////////////////////////////
// write ts processing to FMQ

void SpolTs2Fmq::_writeTsProcessingToFmq()
  
{

  if (_params.debug) {
    cerr << "Writing ts_processing to FMQ" << endl;
  }

  iwrf_ts_processing_t proc = _sysconTsProc;
  if (_sysconTsProcActive) {
    // merge syscon with time series
    proc = _sysconTsProc;
    proc.xmit_phase_mode = _tsTsProc.xmit_phase_mode;
    proc.pulse_width_us = _tsTsProc.pulse_width_us;
    proc.start_range_m = _tsTsProc.start_range_m;
    proc.gate_spacing_m = _tsTsProc.gate_spacing_m;
    proc.pol_mode = _tsTsProc.pol_mode;
  } else {
    // use time series
    proc = _tsTsProc;
  }
  _info.setTsProcessing(proc);
 
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_ts_processing_print(stderr, proc);
  }
  
  // write to FMQ
  
  _outputMsg.addPart(IWRF_TS_PROCESSING_ID, sizeof(proc), &proc);

}

/////////////////////////////////////////////
// write power info to FMQ

void SpolTs2Fmq::_writeXmitPowerToFmq()
  
{
  
  if (_params.debug) {
    cerr << "Writing xmit_power to FMQ" << endl;
  }
  
  iwrf_xmit_power_t power;
  if (_sysconXmitPowerActive) {
    // use syscon
    power = _sysconXmitPower;
  } else {
    // use time series
    power = _tsXmitPower;
  }
  _info.setXmitPower(power);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_xmit_power_print(stderr, power);
  }

  // write to FMQ
  
  _outputMsg.addPart(IWRF_XMIT_POWER_ID, sizeof(power), &power);
  
  if (_params.append_xmit_power_to_status_xml) {

    // create xmit power XML
    
    _xmitPowerLatestTime = power.packet.time_secs_utc;

    _xmitPowerXml.clear();
    _xmitPowerXml += TaXml::writeStartTag(_params.xmit_power_xml_tag, 0);
    _xmitPowerXml += TaXml::writeTime("Time", 1, _xmitPowerLatestTime);
    _xmitPowerXml +=
      TaXml::writeDouble("XmitPowerDbmH", 1, power.power_dbm_h);
    _xmitPowerXml +=
      TaXml::writeDouble("XmitPowerDbmV", 1, power.power_dbm_v);
    _xmitPowerXml += TaXml::writeEndTag(_params.xmit_power_xml_tag, 0);


    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "======================================" << endl;
      cerr << _xmitPowerXml << endl;
      cerr << "======================================" << endl;
    }

  }
  
}

/////////////////////////////////////////////
// write event to FMQ

void SpolTs2Fmq::_writeSysconEventToFmq(const iwrf_event_notice_t &event)
  
{
  
  // write to FMQ
  
  _outputMsg.addPart(IWRF_EVENT_NOTICE_ID, sizeof(event), &event);

}

/////////////////////////////////////////////
// modify pulse header using syscon data

void SpolTs2Fmq::_modifyPulseHeaderFromSyscon(iwrf_pulse_header_t &pHdr)

{
  
  if (!_params.merge_syscon_info) {
    return;
  }

  // decide which event to use for scan information

  const iwrf_event_notice_t &startPrev = _sysconStartOfSweepPrev;
  const iwrf_event_notice_t &start = _sysconStartOfSweep;
  const iwrf_event_notice_t &end = _sysconEndOfSweep;

  double startTime = iwrf_get_packet_time_as_double(start.packet);
  double endTime = iwrf_get_packet_time_as_double(end.packet);

  double ptime = iwrf_get_packet_time_as_double(pHdr.packet);

  iwrf_event_notice_t currentEvent = start;
  bool transition = false;

  if (startTime < endTime) {
    // from scan info point of view,
    // sweep has ended, scan is in transition
    if (ptime < startTime) {
      // from pulse point of view,
      // pulse is in transition region before start of sweep
      currentEvent = start;
      transition = true;
    } else if (ptime > endTime) {
      // from pulse point of view,
      // pulse is in transition region after end of sweep
      currentEvent = end;
      transition = true;
    } else {
      // from pulse point of view,
      // pulse is in sweep
      currentEvent = start;
      transition = false;
    }
  } else {
    // from scan info point of view,
    // sweep is in progress, scan is not in transition
    if (ptime > endTime && ptime < startTime) {
      // from pulse point of view,
      // pulse is in transition
      double meanTime = (startTime + endTime) / 2.0;
      if (ptime < meanTime) {
        currentEvent = end;
      } else {
        currentEvent = start;
      }
      transition = true;
    } else if (ptime < endTime) {
      // from pulse point of view,
      // pulse is in previous sweep
      currentEvent = startPrev;
      transition = false;
    } else {
      // from pulse point of view,
      // pulse is current sweep
      currentEvent = start;
      transition = false;
    }
  }

  // time in sync, set scan information
  
  pHdr.volume_num = currentEvent.volume_num;
  pHdr.sweep_num = currentEvent.sweep_num;
  if (pHdr.sweep_num < 0) {
    pHdr.sweep_num = 0;
  }
  pHdr.scan_mode = currentEvent.scan_mode;
  
  if (currentEvent.scan_mode == IWRF_SCAN_MODE_RHI ||
      currentEvent.scan_mode == IWRF_SCAN_MODE_MANRHI) {
    pHdr.fixed_az = currentEvent.current_fixed_angle;
    pHdr.fixed_el = -9999;
  } else {
    pHdr.fixed_el = currentEvent.current_fixed_angle;
    pHdr.fixed_az = -9999;
  }
  
  pHdr.antenna_transition = transition;

}

/////////////////////////////////////////////
// monitor test pulse characteristics

void SpolTs2Fmq::_monitorTestPulse()

{
  
  // create pulse object, load from buffer

  IwrfTsPulse pulse(_info);
  pulse.setFromBuffer(_msgBuf.getPtr(), _msgBuf.getLen(), false);
  
  // compute gate number
  
  int nGates = pulse.getNGates();
  double startRange = _info.get_proc_start_range_km();
  double gateSpacing = _info.get_proc_gate_spacing_km();

  int gateNum = (int)
    ((_params.test_pulse_range_km - startRange) / gateSpacing + 0.5);
  if (gateNum < 0) gateNum = 0;
  if (gateNum > nGates - 1) gateNum = nGates - 1;

  // load IQ values for test pulse
  
  if (_params.dual_pol_alternating_mode) {
    bool isHoriz = pulse.isHoriz();
    if (_params.dual_pol_switching_receivers) {
      if (isHoriz) {
        _loadTestPulseIq(pulse, 0, gateNum, _testIqHc);
        _loadTestPulseIq(pulse, 1, gateNum, _testIqVx);
      } else {
        _loadTestPulseIq(pulse, 0, gateNum, _testIqVc);
        _loadTestPulseIq(pulse, 1, gateNum, _testIqHx);
      }
    } else {
      if (isHoriz) {
        _loadTestPulseIq(pulse, 0, gateNum, _testIqHc);
        _loadTestPulseIq(pulse, 1, gateNum, _testIqVx);
      } else {
        _loadTestPulseIq(pulse, 1, gateNum, _testIqVc);
        _loadTestPulseIq(pulse, 0, gateNum, _testIqHx);
      }
    }
  } else {
    _loadTestPulseIq(pulse, 0, gateNum, _testIqHc);
    _loadTestPulseIq(pulse, 1, gateNum, _testIqVc);
  }
  _nSamplesTestPulse++;

  // compute mean power if ready

  if (_nSamplesTestPulse < _params.test_pulse_n_samples) {
    return;
  }

  int nSamples = _nSamplesTestPulse;
  if (_params.dual_pol_alternating_mode) {
    nSamples /= 2;
  }
  double meanPowerHc = RadarComplex::meanPower(_testIqHc, nSamples);
  double meanPowerHx = RadarComplex::meanPower(_testIqHx, nSamples);
  double meanPowerVc = RadarComplex::meanPower(_testIqVc, nSamples);
  double meanPowerVx = RadarComplex::meanPower(_testIqVx, nSamples);

  if (meanPowerHc > 0) {
    _testPowerDbHc = 10.0 * log10(meanPowerHc);
  }
  if (meanPowerHx > 0) {
    _testPowerDbHx = 10.0 * log10(meanPowerHx);
  }
  if (meanPowerVc > 0) {
    _testPowerDbVc = 10.0 * log10(meanPowerVc);
  }
  if (meanPowerVx > 0) {
    _testPowerDbVx = 10.0 * log10(meanPowerVx);
  }

  RadarComplex_t lag1Hc =
    RadarComplex::meanConjugateProduct(_testIqHc + 1, _testIqHc,
                                       nSamples-1);
  RadarComplex_t lag1Hx =
    RadarComplex::meanConjugateProduct(_testIqHx + 1, _testIqHx,
                                       nSamples-1);
  RadarComplex_t lag1Vc =
    RadarComplex::meanConjugateProduct(_testIqVc + 1, _testIqVc,
                                       nSamples-1);
  RadarComplex_t lag1Vx =
    RadarComplex::meanConjugateProduct(_testIqVx + 1, _testIqVx,
                                       nSamples-1);

  double argVelHc = RadarComplex::argRad(lag1Hc);
  double argVelHx = RadarComplex::argRad(lag1Hx);
  double argVelVc = RadarComplex::argRad(lag1Vc);
  double argVelVx = RadarComplex::argRad(lag1Vx);

  double wavelengthM = _info.get_radar_wavelength_cm() / 100.0;
  double prt = pulse.getPrt();
  if (_params.dual_pol_alternating_mode) {
    prt *= 2;
  }
  double nyquist = (wavelengthM / prt) / 4.0;

  _testVelHc = (argVelHc / M_PI) * nyquist;
  _testVelHx = (argVelHx / M_PI) * nyquist;
  _testVelVc = (argVelVc / M_PI) * nyquist;
  _testVelVx = (argVelVx / M_PI) * nyquist;

  // clear the test pulse stats to prepare for next average

  _nSamplesTestPulse = 0;

  // create test pulse XML

  _testPulseLatestTime = time(NULL);
  _testPulseXml.clear();
  _testPulseXml += TaXml::writeStartTag(_params.test_pulse_xml_tag, 0);
  _testPulseXml += TaXml::writeTime("Time", 1, _testPulseLatestTime);
  _testPulseXml += TaXml::writeDouble("RangeKm", 1, _params.test_pulse_range_km);
  _testPulseXml += TaXml::writeInt("GateNum", 1, gateNum);
  _testPulseXml += TaXml::writeDouble("StartRangeKm", 1, startRange);
  _testPulseXml += TaXml::writeDouble("GateSpacingKm", 1, gateSpacing);
  
  if (_testPowerDbHc > -9990) {
    _testPulseXml +=
      TaXml::writeDouble("TestPulsePowerDbHc", 1, _testPowerDbHc);
  }
  if (_testPowerDbVc > -9990) {
    _testPulseXml +=
      TaXml::writeDouble("TestPulsePowerDbVc", 1, _testPowerDbVc);
  }

  if (_params.dual_pol_alternating_mode) {
    if (_testPowerDbHx > -9990) {
      _testPulseXml +=
        TaXml::writeDouble("TestPulsePowerDbHx", 1, _testPowerDbHx);
    }
    if (_testPowerDbVx > -9990) {
      _testPulseXml +=
        TaXml::writeDouble("TestPulsePowerDbVx", 1, _testPowerDbVx);
    }
  }

  if (_testPowerDbHc > -9990) {
    _testPulseXml += TaXml::writeDouble("TestPulseVelHc", 1, _testVelHc);
  }
  if (_testPowerDbVc > -9990) {
    _testPulseXml += TaXml::writeDouble("TestPulseVelVc", 1, _testVelVc);
  }
  if (_params.dual_pol_alternating_mode) {
    if (_testPowerDbHx > -9990) {
      _testPulseXml += TaXml::writeDouble("TestPulseVelHx", 1, _testVelHx);
    }
    if (_testPowerDbVx > -9990) {
      _testPulseXml += TaXml::writeDouble("TestPulseVelVx", 1, _testVelVx);
    }
  }

  _testPulseXml += TaXml::writeEndTag(_params.test_pulse_xml_tag, 0);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "======================================" << endl;
    cerr << _testPulseXml << endl;
    cerr << "======================================" << endl;
  }

}

///////////////////////////////////////////////////
// load test pulse IQ

void SpolTs2Fmq::_loadTestPulseIq(IwrfTsPulse &pulse,
                                  int channelNum,
                                  int gateNum,
                                  RadarComplex_t *iq)

{
  fl32 ival, qval;
  pulse.getIq(channelNum, gateNum, ival, qval);
  int index = _nSamplesTestPulse;
  if (_params.dual_pol_alternating_mode) {
    index /= 2;
  }
  iq[index].re = ival;
  iq[index].im = qval;
}

/////////////////////////////
// handle a status xml packet

void SpolTs2Fmq::_handleStatusXml()

{

  // get len
  
  iwrf_status_xml_t *statusHdr = (iwrf_status_xml_t *) _msgBuf.getPtr();
  int xmlLen = statusHdr->xml_len;
  if (xmlLen > (int) _msgBuf.getLen() - (int) sizeof(iwrf_status_xml_t)) {
    xmlLen = (int) _msgBuf.getLen() - (int) sizeof(iwrf_status_xml_t);
  }
  
  // create xml string
  
  char *xmlBuf = new char[xmlLen + 1];
  memcpy(xmlBuf,
         (char *) _msgBuf.getPtr() + sizeof(iwrf_status_xml_t),
         xmlLen);

  // ensure null termination
  xmlBuf[xmlLen] = '\0';
  string xmlStr(xmlBuf);
  delete[] xmlBuf;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_status_xml_print(stderr, *statusHdr, xmlStr);
  }
  
  // augment as required
  
  if (_params.augment_status_xml) {
    _augmentStatusXml(statusHdr->packet, xmlStr);
  } else {
    // add to FMQ as is
    _outputMsg.addPart(_packetId, _packetLen, _msgBuf.getPtr());
  }

  // record time

  _statusXmlLatestTime = statusHdr->packet.time_secs_utc;

}

///////////////////////////////////////////////////
// check for recent status XML
// if late, create a status packet and send it

void SpolTs2Fmq::_checkStatusXml()

{

  time_t now = time(NULL);
  int timeSinceLastXml = now - _statusXmlLatestTime;

  if (timeSinceLastXml > _params.status_xml_interval) {
    _augmentStatusXml(_latestPacketInfo, "");
    _statusXmlLatestTime = now;
  }

}

///////////////////////////////////////////////////
// augment the status XML

void SpolTs2Fmq::_augmentStatusXml(const iwrf_packet_info_t &packet,
                                   const string &xmlStr)

{

  // initialize augmented string with that passed in

  string augXml(xmlStr);

  // get time

  struct timeval tv;
  gettimeofday(&tv, NULL);

  // augment with xmit power?

  int timeSinceXmitPower = tv.tv_sec - _xmitPowerLatestTime;
  if (timeSinceXmitPower <= _params.status_xml_max_age) {
    augXml += _xmitPowerXml;
  }

  // augment with test pulse?

  int timeSinceTestPulse = tv.tv_sec - _testPulseLatestTime;
  if (timeSinceTestPulse <= _params.status_xml_max_age) {
    augXml += _testPulseXml;
  }

  // augment with angle errors?

  int timeSinceAngleErrors = tv.tv_sec - _angleErrorsLatestTime;
  if (timeSinceAngleErrors <= _params.status_xml_max_age) {
    augXml += _angleErrorsXml;
  }

  // augment with secondary status?

  int timeSinceSecondaryStatus = tv.tv_sec - _secondaryStatusLatestTime;
  if (timeSinceSecondaryStatus <= _params.status_xml_max_age) {
    augXml += _secondaryStatusXml;
  }

  // do we have augmented XML?

  if (augXml.size() == 0 ) {
    // no xml info - return now
    return;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "============== Augmented XML ==============" << endl;
    cerr << augXml << endl;
    cerr << "===========================================" << endl;
  }

  // create packet

  iwrf_status_xml_t hdr;
  hdr.packet = packet;
  hdr.packet.id = IWRF_STATUS_XML_ID;
  int xmlLen = augXml.size() + 1;
  hdr.packet.len_bytes = sizeof(hdr) + xmlLen;
  hdr.packet.time_secs_utc = tv.tv_sec;
  hdr.packet.time_nano_secs = tv.tv_usec * 1000;
  hdr.xml_len = xmlLen;

  MemBuf buf;
  buf.add(&hdr, sizeof(hdr));
  buf.add(augXml.c_str(), xmlLen);

  // add to FMQ

  _outputMsg.addPart(IWRF_STATUS_XML_ID, buf.getLen(), buf.getPtr());

}

///////////////////////////////
// handle a calibration packet

void SpolTs2Fmq::_handleCalibration()

{

  if (!_params.override_calibration) {
    // pass packet through unchanged
    _outputMsg.addPart(_packetId, _packetLen, _msgBuf.getPtr());
    return;
  }

  // use calibration read in from file

  iwrf_calibration_t calib = _calib.getStruct();

  // write to FMQ
  
  _outputMsg.addPart(IWRF_CALIBRATION_ID, sizeof(calib), &calib);

}

///////////////////////////////////////
// open the secondary Xml FMQ
// returns 0 on success, -1 on failure

int SpolTs2Fmq::_openSecondaryStatusFmq()

{
  
  Fmq::openPosition initPos = Fmq::END;

  if (_secondaryStatusFmq.initReadOnly
      (_params.secondary_status_fmq_path,
       _progName.c_str(),
       _params.debug >= Params::DEBUG_EXTRA, // set debug?
       initPos)) {
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Cannot initialize FMQ: " << _params.secondary_status_fmq_path << endl;
      cerr << _secondaryStatusFmq.getErrStr() << endl;
    }
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////
// read secondary status from FMQ
//
// Returns 0 on success, -1 on failure

int SpolTs2Fmq::_readSecondaryStatusFromFmq()
  
{

  PMU_auto_register("readSecondaryStatusFromFmq");
  
  // check we have an open FMQ
  
  if (!_secondaryStatusFmq.isOpen()) {
    cerr << "ooooooooooooooooooooooooooooooooooooooo" << endl;
    if (_openSecondaryStatusFmq()) {
      return -1;
    }
  }
  
  // read in a new message
  
  bool gotOne;
  if (_secondaryStatusFmq.readMsg(&gotOne)) {
    cerr << "ERROR -  SpolTs2Fmq::_readSysconInfo" << endl;
    cerr << "  Cannot read syscon from FMQ" << endl;
    cerr << "  Fmq: " << _params.syscon_fmq_path << endl;
    cerr << _sysconFmq.getErrStr() << endl;
    _sysconFmq.closeMsgQueue();
    return -1;
  }
  
  if (!gotOne) {
    // no data
    return 0;
  }
  
  // get the xml, ensure null termination
  
  const void *msg = _secondaryStatusFmq.getMsg();
  int len = _secondaryStatusFmq.getMsgLen();
  TaArray<char> xml_;
  char *xml = xml_.alloc(len);
  memcpy(xml, msg, len);
  xml[len-1] = '\0';
  _secondaryStatusXml = xml;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "========== latest secondary status XML ==============" << endl;
    cerr << _secondaryStatusXml << endl;
    cerr << "=====================================================" << endl;
  }

  return 0;

}

////////////////////////////////////
// Scale IQ data in pulse buffer
// Only applies to FLOAT data

void SpolTs2Fmq::_applyIQScale()
  
{

  ui08 *start = (ui08 *) _msgBuf.getPtr();
  iwrf_pulse_header_t *pHdr = (iwrf_pulse_header_t *) start;

  // check that the IQ data is floats

  iwrf_iq_encoding encoding = (iwrf_iq_encoding) pHdr->iq_encoding;
  if (encoding != IWRF_IQ_ENCODING_FL32) {
    if (!_scaleWarningPrinted) {
      cerr << "WARNING - TsTcp2Fmq::_applyIQScale" << endl;
      cerr << "  Parameter apply_scale is set" << endl;
      cerr << "  However, this only applies to float IQ data" << endl;
      cerr << "  Therefore, no scaling will be done" << endl;
    }
    _scaleWarningPrinted = true;
    return;
  }

  double scale = _params.scale;
  double bias = _params.bias;
  
  fl32 *fptr = (fl32 *) (start + sizeof(iwrf_pulse_header_t));
  for(int ii = 0; ii < pHdr->n_data; ii++) {
    fptr[ii] = fptr[ii] * scale + bias;
  }

}

