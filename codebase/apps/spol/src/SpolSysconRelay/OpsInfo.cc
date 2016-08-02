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
// OpsInfo.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2010
//
///////////////////////////////////////////////////////////////
//
// OpsInfo reads data from a SysCon server in legacy CHILL format,
// and converts to IWRF format
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <toolsa/pmu.h>
#include <toolsa/ushmem.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <radar/chill_to_iwrf.hh>
#include <dsserver/DmapAccess.hh>
#include "OpsInfo.hh"

using namespace std;

// Constructor

OpsInfo::OpsInfo(const string &progName,
                 const Params &params,
                 DsFmq &outputFmq) :
        _progName(progName),
        _params(params),
        _outputFmq(outputFmq),
        _iwrfSeqNum(0)
{

}

// destructor

OpsInfo::~OpsInfo()

{
  _closeSocket();
}

/////////////////////////////
// check for data
// waiting specified time

int OpsInfo::checkForData(int waitMsecs)
{

  if (!_sock.isOpen()) {
    if (_openSocket(_params.syscon_server_host,
                    _params.syscon_ops_info_port)) {
      cerr << "ERROR -  OpsInfo::checkForData" << endl;
      cerr <<  "  Cannot open socket, host, port: "
           << _params.syscon_server_host << ", "
           << _params.syscon_ops_info_port << endl;
      cerr << _sock.getErrStr() << endl;
      return -1;
    }
  }
  
  if (_sock.readSelect(waitMsecs)) {
    return -1;
  }

  return 0;
  
}


///////////////////////////////////////////////
// read data from the server
// returns 0 on success, -1 on failure
// also sets id of iwrf data read
// sets iwrfId to 0 if not a chill packet

int OpsInfo::read(int &iwrfId)
  
{

  if (!_sock.isOpen()) {
    if (_openSocket(_params.syscon_server_host,
                    _params.syscon_ops_info_port)) {
      cerr << "ERROR -  OpsInfo::read" << endl;
      cerr <<  "  Cannot open socket, host, port: "
           << _params.syscon_server_host << ", "
           << _params.syscon_ops_info_port << endl;
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "reading OpsInfo data from syscon" << endl;
  }
  
  // read id and length of packet
  
  si32 packetId;
  si32 packetLen;
  si32 packetTop[2];
  
  // read the first 8 bytes (id, len)
  if (_sock.readBufferHb(packetTop, sizeof(packetTop),
                         sizeof(packetTop), PMU_auto_register, 1000)) {
    cerr << "ERROR - OpsInfo::read" << endl;
    cerr << "  " << _sock.getErrStr() << endl;
    _sock.close();
    return -1;
  }

  // get ID and length

  packetId = packetTop[0];
  packetLen = packetTop[1];

  // make the read buffer large enough
  
  _readBuf.reserve(packetLen);
  
  // copy the packet top into the start of the buffer

  memcpy(_readBuf.getPtr(), packetTop, sizeof(packetTop));
  
  // read in the remainder of the buffer

  char *startPtr = (char *) _readBuf.getPtr() + sizeof(packetTop);
  int nBytesRemaining = packetLen - sizeof(packetTop);
  
  if (nBytesRemaining < 0) {
    cerr << "ERROR - OpsInfo::read" << endl;
    cerr << "  bad buffer len: " << packetLen << endl;
    _sock.close();
    return -1;
  }

  if (_sock.readBufferHb(startPtr, nBytesRemaining, 1024,
                         PMU_auto_register, 1000)) {
    cerr << "ERROR - OpsInfo::read" << endl;
    cerr << "  " << _sock.getErrStr() << endl;
    _sock.close();
    return -1;
  }
  
  // process packet
    
  int iret = 0;

  switch (packetId) {

    // chill packets

    case HSK_ID_RADAR_INFO:
      iwrfId = IWRF_RADAR_INFO_ID;
      if (_processChillRadarInfo()) {
        iret = -1;
      }
      break;

    case HSK_ID_SCAN_SEG:
      iwrfId = IWRF_SCAN_SEGMENT_ID;
      if (_processChillScanSeg()) {
        iret = -1;
      }
      break;

    case HSK_ID_PROCESSOR_INFO:
      iwrfId = IWRF_TS_PROCESSING_ID;
      if (_processChillProcInfo()) {
        iret = -1;
      }
      break;

    case HSK_ID_PWR_UPDATE:
      iwrfId = IWRF_XMIT_POWER_ID;
      if (_processChillPowerUpdate()) {
        iret = -1;
      }
      break;

    case HSK_ID_EVENT_NOTICE:
      iwrfId = IWRF_EVENT_NOTICE_ID;
      if (_processChillEventNotice()) {
        iret = -1;
      }
      break;

    case HSK_ID_CAL_TERMS:
      iwrfId = IWRF_CALIBRATION_ID;
      if (_processChillCalTerms()) {
        iret = -1;
      }
      break;

    case HSK_ID_XMIT_INFO:
      iwrfId = IWRF_XMIT_INFO_ID;
      if (_processChillXmitInfo()) {
        iret = -1;
      }
      break;

    case HSK_ID_ANT_OFFSET:
      iwrfId = IWRF_ANTENNA_CORRECTION_ID;
      if (_processChillAntCorr()) {
        iret = -1;
      }
      break;

    case HSK_ID_XMIT_SAMPLE:
      iwrfId = IWRF_XMIT_SAMPLE_ID;
      if (_processChillXmitSample()) {
        iret = -1;
      }
      break;

    case HSK_ID_PHASE_CODE:
      iwrfId = IWRF_PHASECODE_ID;
      if (_processChillPhaseCode()) {
        iret = -1;
      }
      break;
      
    // iwrf packets

    case IWRF_RADAR_INFO_ID:
      iwrfId = IWRF_RADAR_INFO_ID;
      if (_processIwrfRadarInfo()) {
        iret = -1;
      }
      break;

    case IWRF_SCAN_SEGMENT_ID:
      iwrfId = IWRF_SCAN_SEGMENT_ID;
      if (_processIwrfScanSeg()) {
        iret = -1;
      }
      break;

    case IWRF_TS_PROCESSING_ID:
      iwrfId = IWRF_TS_PROCESSING_ID;
      if (_processIwrfProcInfo()) {
        iret = -1;
      }
      break;

    case IWRF_XMIT_POWER_ID:
      iwrfId = IWRF_XMIT_POWER_ID;
      if (_processIwrfXmitPower()) {
        iret = -1;
      }
      break;

    case IWRF_EVENT_NOTICE_ID:
      iwrfId = IWRF_EVENT_NOTICE_ID;
      if (_processIwrfEventNotice()) {
        iret = -1;
      }
      break;

    case IWRF_CALIBRATION_ID:
      iwrfId = IWRF_CALIBRATION_ID;
      if (_processIwrfCalib()) {
        iret = -1;
      }
      break;

    case IWRF_XMIT_INFO_ID:
      iwrfId = IWRF_XMIT_INFO_ID;
      if (_processIwrfXmitInfo()) {
        iret = -1;
      }
      break;

    case IWRF_ANTENNA_CORRECTION_ID:
      iwrfId = IWRF_ANTENNA_CORRECTION_ID;
      if (_processIwrfAntCorr()) {
        iret = -1;
      }
      break;

    case IWRF_XMIT_SAMPLE_ID:
      iwrfId = IWRF_XMIT_SAMPLE_ID;
      if (_processIwrfXmitSample()) {
        iret = -1;
      }
      break;

    case IWRF_PHASECODE_ID:
      iwrfId = IWRF_PHASECODE_ID;
      if (_processIwrfPhaseCode()) {
        iret = -1;
      }
      break;
      
    default: {
      if (_params.debug) {
        cerr << "DEBUG - OpsInfo::read" << endl;
        cerr << "  Unrecognized packet type" << endl;
        fprintf(stderr, "    Packet id: 0x%x\n", packetId);
        fprintf(stderr, "    Packet len: %d\n", packetLen);
        cerr << "  IWRF type: " << iwrf_packet_id_to_str(packetId) << endl;
      }
    }
      
  } // switch (id)
  
  if (iret) {
    _closeSocket();
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////
// open socket
// returns 0 on success, -1 on failure

int OpsInfo::_openSocket(const char *host, int port)

{

  if (_sock.open(host, port, 10000)) {
    
    if (_params.debug) {
      if (_sock.getErrNum() == Socket::TIMED_OUT) {
        cerr << "  Waiting for server to come up ..." << endl;
      } else {
        cerr << "ERROR - SpolSysconRelay::Run" << endl;
        cerr << "  Connecting to server" << endl;
        cerr << "  " << _sock.getErrStr() << endl;
      }
    }

    return -1;

  }

  return 0;

}

//////////////////////////////////////////////
// close socket

void OpsInfo::_closeSocket()
  
{
  _sock.close();
}

//////////////////////////////////////////////////
// process chill radarInfo

int OpsInfo::_processChillRadarInfo()
  
{
  
  // load chill struct

  if (_readBuf.getLen() != sizeof(radar_info_t)) {
    cerr << "ERROR - OpsInfo::_processChillRadarInfo" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(radar_info_t) << endl;
    return -1;
  }
  memcpy(&_chillRadarInfo, _readBuf.getPtr(), _readBuf.getLen());
  if (_params.debug >= Params::DEBUG_EXTRA) {
    chill_radar_info_print(cerr, _chillRadarInfo);
  }
  _chillRadarInfoAvail = true;

  // convert to IWRF

  chill_iwrf_radar_info_load(_chillRadarInfo, _iwrfSeqNum++, _iwrfRadarInfo);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv" << endl;
    cerr << "NOTE: following converted from CHILL" << endl;
    iwrf_radar_info_print(stderr, _iwrfRadarInfo);
    cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
  }

  if (_params.use_iwrf_syscon_packets && _iwrfRadarInfoAvail) {
    // use IWRF data instead
    return 0;
  }

  // write to FMQ

  if (_outputFmq.writeMsg(IWRF_RADAR_INFO_ID, 0,
                          &_iwrfRadarInfo, sizeof(_iwrfRadarInfo))) {
    cerr << "ERROR - OpsInfo::_processChillRadarInfo" << endl;
    cerr << "  Cannot write radar info to FMQ" << endl;
    cerr << "  Fmq path: " << _params.output_fmq_path << endl;
    cerr << _outputFmq.getErrStr();
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process chill scan segment

int OpsInfo::_processChillScanSeg()
  
{

  // load chill struct

  if (_readBuf.getLen() != sizeof(scan_seg_t)) {
    cerr << "ERROR - OpsInfo::_processChillScanSeg" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(scan_seg_t) << endl;
    return -1;
  }
  memcpy(&_chillScanSeg, _readBuf.getPtr(), _readBuf.getLen());
  if (_params.debug >= Params::DEBUG_EXTRA) {
    chill_scan_seg_print(cerr, _chillScanSeg);
  }
  _chillScanSegAvail = true;

  // convert to IWRF

  chill_iwrf_scan_seg_load(_chillScanSeg, _iwrfSeqNum++, _iwrfScanSeg);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv" << endl;
    cerr << "NOTE: following converted from CHILL" << endl;
    iwrf_scan_segment_print(stderr, _iwrfScanSeg);
    cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
  }
  
  if (_params.use_iwrf_syscon_packets && _iwrfScanSegAvail) {
    // use IWRF data instead
    return 0;
  }

  // write to FMQ
  
  if (_outputFmq.writeMsg(IWRF_SCAN_SEGMENT_ID, 0,
                          &_iwrfScanSeg, sizeof(_iwrfScanSeg))) {
    cerr << "ERROR - OpsInfo::_processChillScanSeg" << endl;
    cerr << "  Cannot write scan segment to FMQ" << endl;
    cerr << "  Fmq path: " << _params.output_fmq_path << endl;
    cerr << _outputFmq.getErrStr();
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process chill proc info

int OpsInfo::_processChillProcInfo()
  
{

  // load chill struct

  if (_readBuf.getLen() != sizeof(processor_info_t)) {
    cerr << "ERROR - OpsInfo::_processChillProcInfo" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(processor_info_t) << endl;
    return -1;
  }
  memcpy(&_chillProcInfo, _readBuf.getPtr(), _readBuf.getLen());
  if (_params.debug >= Params::DEBUG_EXTRA) {
    chill_proc_info_print(cerr, _chillProcInfo);
  }
  _chillProcInfoAvail = true;

  // convert to IWRF
  
  chill_iwrf_ts_proc_load(_chillProcInfo, _iwrfSeqNum++, _iwrfTsProc);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv" << endl;
    cerr << "NOTE: following converted from CHILL" << endl;
    iwrf_ts_processing_print(stderr, _iwrfTsProc);
    cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
  }
  
  if (_params.use_iwrf_syscon_packets && _iwrfTsProcAvail) {
    // use IWRF data instead
    return 0;
  }

  // write to FMQ
  
  if (_outputFmq.writeMsg(IWRF_TS_PROCESSING_ID, 0,
                          &_iwrfTsProc, sizeof(_iwrfTsProc))) {
    cerr << "ERROR - OpsInfo::_processChillProcInfo" << endl;
    cerr << "  Cannot write ts processing to FMQ" << endl;
    cerr << "  Fmq path: " << _params.output_fmq_path << endl;
    cerr << _outputFmq.getErrStr();
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process chill power update

int OpsInfo::_processChillPowerUpdate()
  
{

  // load chill struct

  if (_readBuf.getLen() != sizeof(power_update_t)) {
    cerr << "ERROR - OpsInfo::_processChillPowerUpdate" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(power_update_t) << endl;
    return -1;
  }
  memcpy(&_chillPowerUpdate, _readBuf.getPtr(), _readBuf.getLen());
  if (_params.debug >= Params::DEBUG_EXTRA) {
    chill_power_update_print(cerr, _chillPowerUpdate);
  }
  _chillPowerUpdateAvail = true;

  // convert to IWRF

  chill_iwrf_xmit_power_load(_chillPowerUpdate, _iwrfSeqNum++, _iwrfXmitPower);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv" << endl;
    cerr << "NOTE: following converted from CHILL" << endl;
    iwrf_xmit_power_print(stderr, _iwrfXmitPower);
    cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
  }
  
  if (_params.use_iwrf_syscon_packets && _iwrfXmitPowerAvail) {
    // use IWRF data instead
    return 0;
  }

  // write to FMQ
  
  if (_outputFmq.writeMsg(IWRF_XMIT_POWER_ID, 0,
                          &_iwrfXmitPower, sizeof(_iwrfXmitPower))) {
    cerr << "ERROR - OpsInfo::_processChillPowerUpdate" << endl;
    cerr << "  Cannot write xmit power to FMQ" << endl;
    cerr << "  Fmq path: " << _params.output_fmq_path << endl;
    cerr << _outputFmq.getErrStr();
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process chill event notice

int OpsInfo::_processChillEventNotice()
  
{

  // load chill struct

  if (_readBuf.getLen() != sizeof(event_notice_t)) {
    cerr << "ERROR - OpsInfo::_processChillEventNotice" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(event_notice_t) << endl;
    return -1;
  }
  memcpy(&_chillEventNotice, _readBuf.getPtr(), _readBuf.getLen());
  if (_params.debug >= Params::DEBUG_EXTRA) {
    chill_event_notice_print(cerr, _chillEventNotice);
  }
  _chillEventNoticeAvail = true;

  if (_chillScanSegAvail) {

    // convert to IWRF
    
    chill_iwrf_event_notice_load(_chillEventNotice, _chillScanSeg,
                                 _iwrfSeqNum++, _iwrfEventNotice);
    
    // *** CLUDGE *** until chill event notice has correct scan mode

    _iwrfEventNotice.scan_mode = _iwrfScanSeg.scan_mode;
    _iwrfEventNotice.current_fixed_angle = _iwrfScanSeg.current_fixed_angle;
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      
      cerr << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv" << endl;
      cerr << "NOTE: following converted from CHILL" << endl;
      iwrf_event_notice_print(stderr, _iwrfEventNotice);
      cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
      
      struct timeval tv;
      gettimeofday(&tv, NULL);

      DateTime now(tv.tv_sec);
      fprintf(stderr, "Event received at time: %s.%.6d\n", 
              DateTime::strm(tv.tv_sec).c_str(),
              (int) tv.tv_usec);

    }

  }
  
  if (_params.use_iwrf_syscon_packets && _iwrfEventNoticeAvail) {
    // use IWRF data instead
    return 0;
  }

  // write to FMQ
  
  if (_outputFmq.writeMsg(IWRF_EVENT_NOTICE_ID, 0,
                          &_iwrfEventNotice, sizeof(_iwrfEventNotice))) {
    cerr << "ERROR - OpsInfo::_processChillEventNotice" << endl;
    cerr << "  Cannot write event notice to FMQ" << endl;
    cerr << "  Fmq path: " << _params.output_fmq_path << endl;
    cerr << _outputFmq.getErrStr();
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process chill cal terms

int OpsInfo::_processChillCalTerms()
  
{
  
  // load chill struct

  if (_readBuf.getLen() != sizeof(cal_terms_t)) {
    cerr << "ERROR - OpsInfo::_processChillCalTerms" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(cal_terms_t) << endl;
    return -1;
  }
  memcpy(&_chillCalTerms, _readBuf.getPtr(), _readBuf.getLen());
  if (_params.debug >= Params::DEBUG_EXTRA) {
    chill_cal_terms_print(cerr, _chillCalTerms);
  }
  _chillCalTermsAvail = true;
  
  if (_chillRadarInfoAvail && _chillPowerUpdateAvail) {
    
    // convert to IWRF

    chill_iwrf_calibration_load(_chillRadarInfo, _chillCalTerms,
                                _chillPowerUpdate,
                                _iwrfSeqNum++, _iwrfCalib);
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv" << endl;
      cerr << "NOTE: following converted from CHILL" << endl;
      iwrf_calibration_print(stderr, _iwrfCalib);
      cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
    }

  }
  
  return 0;

}

//////////////////////////////////////////////////
// process chill xmit info

int OpsInfo::_processChillXmitInfo()
  
{

  // load chill struct

  if (_readBuf.getLen() != sizeof(xmit_info_t)) {
    cerr << "ERROR - OpsInfo::_processChillXmitInfo" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(xmit_info_t) << endl;
    return -1;
  }
  memcpy(&_chillXmitInfo, _readBuf.getPtr(), _readBuf.getLen());
  if (_params.debug >= Params::DEBUG_EXTRA) {
    chill_xmit_info_print(cerr, _chillXmitInfo);
  }
  _chillXmitInfoAvail = true;

  // convert to IWRF

  chill_iwrf_xmit_info_load(_chillXmitInfo, _iwrfSeqNum++, _iwrfXmitInfo);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv" << endl;
    cerr << "NOTE: following converted from CHILL" << endl;
    iwrf_xmit_info_print(stderr, _iwrfXmitInfo);
    cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process chill antenna correction

int OpsInfo::_processChillAntCorr()
  
{

  // load chill struct

  if (_readBuf.getLen() != sizeof(antenna_correction_t)) {
    cerr << "ERROR - OpsInfo::_processChillAntCorr" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(antenna_correction_t) << endl;
    return -1;
  }
  memcpy(&_chillAntCorr, _readBuf.getPtr(), _readBuf.getLen());
  if (_params.debug >= Params::DEBUG_EXTRA) {
    chill_ant_corr_print(cerr, _chillAntCorr);
  }
  _chillAntCorrAvail = true;
 
  // convert to IWRF
  
  chill_iwrf_ant_corr_load(_chillAntCorr, _iwrfSeqNum++, _iwrfAntCorr);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv" << endl;
    cerr << "NOTE: following converted from CHILL" << endl;
    iwrf_antenna_correction_print(stderr, _iwrfAntCorr);
    cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process chill xmit sample

int OpsInfo::_processChillXmitSample()
  
{

  // load chill struct

  if (_readBuf.getLen() != sizeof(xmit_sample_t)) {
    cerr << "ERROR - OpsInfo::_processChillXmitSample" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(xmit_sample_t) << endl;
    return -1;
  }
  memcpy(&_chillXmitSample, _readBuf.getPtr(), _readBuf.getLen());
  if (_params.debug >= Params::DEBUG_EXTRA) {
    chill_xmit_sample_print(cerr, _chillXmitSample);
  }
  _chillXmitSampleAvail = true;

  // convert to IWRF

  chill_iwrf_xmit_sample_load(_chillXmitSample, _iwrfSeqNum++, _iwrfXmitSample);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Read xmit sample - chill packet" << endl;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv" << endl;
      cerr << "NOTE: following converted from CHILL" << endl;
      iwrf_xmit_sample_print(stderr, _iwrfXmitSample);
      cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
    }
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process chill phase code

int OpsInfo::_processChillPhaseCode()
  
{

  // load chill struct

  if (_readBuf.getLen() != sizeof(phasecode_t)) {
    cerr << "ERROR - OpsInfo::_processChillPhaseCode" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(phasecode_t) << endl;
    return -1;
  }
  memcpy(&_chillPhaseCode, _readBuf.getPtr(), _readBuf.getLen());
  if (_params.debug >= Params::DEBUG_EXTRA) {
    chill_phasecode_print(cerr, _chillPhaseCode);
  }
  _chillPhaseCodeAvail = true;

  // convert to IWRF

  chill_iwrf_phasecode_load(_chillPhaseCode, _iwrfSeqNum++, _iwrfPhaseCode);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv" << endl;
    cerr << "NOTE: following converted from CHILL" << endl;
    iwrf_phasecode_print(stderr, _iwrfPhaseCode);
    cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process iwrf radarInfo

int OpsInfo::_processIwrfRadarInfo()
  
{
  
  // load iwrf struct

  if (_readBuf.getLen() != sizeof(iwrf_radar_info_t)) {
    cerr << "ERROR - OpsInfo::_processIwrfRadarInfo" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(iwrf_radar_info_t) << endl;
    return -1;
  }
  memcpy(&_iwrfRadarInfo, _readBuf.getPtr(), _readBuf.getLen());
  _iwrfRadarInfoAvail = true;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_radar_info_print(stderr, _iwrfRadarInfo);
  }
  
  // write to FMQ

  if (_params.use_iwrf_syscon_packets) {
    if (_outputFmq.writeMsg(IWRF_RADAR_INFO_ID, 0,
                            &_iwrfRadarInfo, sizeof(_iwrfRadarInfo))) {
      cerr << "ERROR - OpsInfo::_processIwrfRadarInfo" << endl;
      cerr << "  Cannot write radar info to FMQ" << endl;
      cerr << "  Fmq path: " << _params.output_fmq_path << endl;
      cerr << _outputFmq.getErrStr();
      return -1;
    }
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process iwrf scan segment

int OpsInfo::_processIwrfScanSeg()
  
{

  // load iwrf struct

  if (_readBuf.getLen() != sizeof(iwrf_scan_segment_t)) {
    cerr << "ERROR - OpsInfo::_processIwrfScanSeg" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(iwrf_scan_segment_t) << endl;
    return -1;
  }
  memcpy(&_iwrfScanSeg, _readBuf.getPtr(), _readBuf.getLen());
  _iwrfScanSegAvail = true;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_scan_segment_print(stderr, _iwrfScanSeg);
  }
  
  // write to FMQ
  
  if (_params.use_iwrf_syscon_packets) {
    if (_outputFmq.writeMsg(IWRF_SCAN_SEGMENT_ID, 0,
                            &_iwrfScanSeg, sizeof(_iwrfScanSeg))) {
      cerr << "ERROR - OpsInfo::_processIwrfScanSeg" << endl;
      cerr << "  Cannot write scan segment to FMQ" << endl;
      cerr << "  Fmq path: " << _params.output_fmq_path << endl;
      cerr << _outputFmq.getErrStr();
      return -1;
    }
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process iwrf proc info

int OpsInfo::_processIwrfProcInfo()
  
{

  // load iwrf struct

  if (_readBuf.getLen() != sizeof(iwrf_ts_processing_t)) {
    cerr << "ERROR - OpsInfo::_processIwrfProcInfo" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(iwrf_ts_processing_t) << endl;
    return -1;
  }
  memcpy(&_iwrfTsProc, _readBuf.getPtr(), _readBuf.getLen());
  _iwrfTsProcAvail = true;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_ts_processing_print(stderr, _iwrfTsProc);
  }
  
  // write to FMQ
  
  if (_params.use_iwrf_syscon_packets) {
    if (_outputFmq.writeMsg(IWRF_TS_PROCESSING_ID, 0,
                            &_iwrfTsProc, sizeof(_iwrfTsProc))) {
      cerr << "ERROR - OpsInfo::_processIwrfProcInfo" << endl;
      cerr << "  Cannot write ts processing to FMQ" << endl;
      cerr << "  Fmq path: " << _params.output_fmq_path << endl;
      cerr << _outputFmq.getErrStr();
      return -1;
    }
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process iwrf power update

int OpsInfo::_processIwrfXmitPower()
  
{

  // load iwrf struct

  if (_readBuf.getLen() != sizeof(iwrf_xmit_power_t)) {
    cerr << "ERROR - OpsInfo::_processIwrfXmitPower" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(iwrf_xmit_power_t) << endl;
    return -1;
  }
  memcpy(&_iwrfXmitPower, _readBuf.getPtr(), _readBuf.getLen());
  _iwrfXmitPowerAvail = true;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_xmit_power_print(stderr, _iwrfXmitPower);
  }
  
  // write to FMQ
  
  if (_params.use_iwrf_syscon_packets) {
    if (_outputFmq.writeMsg(IWRF_XMIT_POWER_ID, 0,
                            &_iwrfXmitPower, sizeof(_iwrfXmitPower))) {
      cerr << "ERROR - OpsInfo::_processIwrfPowerUpdate" << endl;
      cerr << "  Cannot write xmit power to FMQ" << endl;
      cerr << "  Fmq path: " << _params.output_fmq_path << endl;
      cerr << _outputFmq.getErrStr();
      return -1;
    }
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process iwrf event notice

int OpsInfo::_processIwrfEventNotice()
  
{

  // load iwrf struct

  if (_readBuf.getLen() != sizeof(iwrf_event_notice_t)) {
    cerr << "ERROR - OpsInfo::_processIwrfEventNotice" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(iwrf_event_notice_t) << endl;
    return -1;
  }
  memcpy(&_iwrfEventNotice, _readBuf.getPtr(), _readBuf.getLen());
  _iwrfEventNoticeAvail = true;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_event_notice_print(stderr, _iwrfEventNotice);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    DateTime now(tv.tv_sec);
    fprintf(stderr, "Event received at time: %s.%.6d\n", 
            DateTime::strm(tv.tv_sec).c_str(),
            (int) tv.tv_usec);
  }
  
  // write to FMQ
  
  if (_params.use_iwrf_syscon_packets) {
    if (_outputFmq.writeMsg(IWRF_EVENT_NOTICE_ID, 0,
                            &_iwrfEventNotice, sizeof(_iwrfEventNotice))) {
      cerr << "ERROR - OpsInfo::_processIwrfEventNotice" << endl;
      cerr << "  Cannot write event notice to FMQ" << endl;
      cerr << "  Fmq path: " << _params.output_fmq_path << endl;
      cerr << _outputFmq.getErrStr();
      return -1;
    }
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process iwrf calibration

int OpsInfo::_processIwrfCalib()
  
{
  
  // load iwrf struct
  
  if (_readBuf.getLen() != sizeof(iwrf_calibration_t)) {
    cerr << "ERROR - OpsInfo::_processIwrfCalibration" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(iwrf_calibration_t) << endl;
    return -1;
  }
  memcpy(&_iwrfCalib, _readBuf.getPtr(), _readBuf.getLen());
  _iwrfCalibAvail = true;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_calibration_print(stderr, _iwrfCalib);
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process iwrf xmit info

int OpsInfo::_processIwrfXmitInfo()
  
{

  // load iwrf struct
  
  if (_readBuf.getLen() != sizeof(iwrf_xmit_info_t)) {
    cerr << "ERROR - OpsInfo::_processIwrfXmitInfo" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(iwrf_xmit_info_t) << endl;
    return -1;
  }
  memcpy(&_iwrfXmitInfo, _readBuf.getPtr(), _readBuf.getLen());
  _iwrfXmitInfoAvail = true;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_xmit_info_print(stderr, _iwrfXmitInfo);
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process iwrf antenna correction

int OpsInfo::_processIwrfAntCorr()
  
{

  // load iwrf struct

  if (_readBuf.getLen() != sizeof(iwrf_antenna_correction_t)) {
    cerr << "ERROR - OpsInfo::_processIwrfAntCorr" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(iwrf_antenna_correction_t) << endl;
    return -1;
  }

  memcpy(&_iwrfAntCorr, _readBuf.getPtr(), _readBuf.getLen());
  _iwrfAntCorrAvail = true;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_antenna_correction_print(stderr, _iwrfAntCorr);
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process iwrf xmit sample

int OpsInfo::_processIwrfXmitSample()
  
{

  // load iwrf struct

  if (_readBuf.getLen() != sizeof(iwrf_xmit_sample_t)) {
    cerr << "ERROR - OpsInfo::_processIwrfXmitSample" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(iwrf_xmit_sample_t) << endl;
    return -1;
  }
  memcpy(&_iwrfXmitSample, _readBuf.getPtr(), _readBuf.getLen());
  _iwrfXmitSampleAvail = true;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Read xmit sample" << endl;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      iwrf_xmit_sample_print(stderr, _iwrfXmitSample);
    }
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process iwrf phase code

int OpsInfo::_processIwrfPhaseCode()
  
{

  // load iwrf struct

  if (_readBuf.getLen() != sizeof(iwrf_phasecode_t)) {
    cerr << "ERROR - OpsInfo::_processIwrfPhaseCode" << endl;
    cerr << "  Incorrect packet length: " << _readBuf.getLen() << endl;
    cerr << "  Should be: " << sizeof(iwrf_phasecode_t) << endl;
    return -1;
  }
  memcpy(&_iwrfPhaseCode, _readBuf.getPtr(), _readBuf.getLen());
  _iwrfPhaseCodeAvail = true;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_phasecode_print(stderr, _iwrfPhaseCode);
  }
  
  return 0;

}


