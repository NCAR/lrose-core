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
// ChillTsTcp2Fmq.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2009
//
///////////////////////////////////////////////////////////////
//
// ChillTsTcp2Fmq reads data from a server in TCP, and writes
// it out to an FMQ
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctime>
#include <dataport/swap.h>
#include <toolsa/udatetime.h>
#include <toolsa/uusleep.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <radar/chill_to_iwrf.hh>
#include "ChillTsTcp2Fmq.hh"

using namespace std;

// Constructor

ChillTsTcp2Fmq::ChillTsTcp2Fmq(int argc, char **argv) :
        _iwrfPulse(_iwrfInfo)
  
{

  isOK = true;
  _nPulses = 0;
  _packetSeqNum = 0;
  _pulseSeqNum = 0;
  _prevMetaTime = 0;

  _prevVolNum = -1;
  _volStartSweepNum = 0;
  _prevSweepNum = -1;
  _sweepNum = 0;

  _radarInfoAvail = false;
  _scanSegAvail = false;
  _procInfoAvail = false;
  _powerUpdateAvail = false;
  _eventNoticeAvail = false;
  _calTermsAvail = false;
  _xmitInfoAvail = false;
  _antCorrAvail = false;
  _xmitSampleAvail = false;
  _phaseCodeAvail = false;
  _calibAvail = false;
  _iwrfCalibAvail = false;

  // set programe name
 
  _progName = "ChillTsTcp2Fmq";

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

  // initialize input cal data

  if (_params.read_cal_from_file) {
    _calibLdata.setDir(_params.cal_file_dir);
  }

  // initialize the output FMQ
  
  if (_fmq.initReadWrite(_params.output_fmq_path,
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
    cerr << _fmq.getErrStr() << endl;
    isOK = false;
    return;
  }
  _fmq.setSingleWriter();
  if (_params.data_mapper_report_interval > 0) {
    _fmq.setRegisterWithDmap(true, _params.data_mapper_report_interval);
  }

  // initialize message
  
  _msg.clearAll();
  _msg.setType(0);

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
  return;
  
}

// destructor

ChillTsTcp2Fmq::~ChillTsTcp2Fmq()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int ChillTsTcp2Fmq::Run ()
{

  PMU_auto_register("Run");

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running ChillTsTcp2Fmq - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running ChillTsTcp2Fmq - debug mode" << endl;
  }
  if (_params.debug) {
    cerr << "  FMQ: " << _params.output_fmq_path << endl;
    cerr << "  nSlots: " << _params.output_fmq_nslots << endl;
    cerr << "  nBytes: " << _params.output_fmq_size << endl;
  }

  int iret = 0;

  while (true) {

    PMU_auto_register("Opening socket");
    
    // open socket to server
    
    Socket sock;
    if (sock.open(_params.ts_tcp_server_host,
                  _params.ts_tcp_server_port,
                  10000)) {
      if (sock.getErrNum() == Socket::TIMED_OUT) {
	if (_params.debug) {
	  cerr << "  Waiting for server to come up ..." << endl;
	}
      } else {
	if (_params.debug) {
	  cerr << "ERROR - ChillTsTcp2Fmq::Run" << endl;
	  cerr << "  Connecting to server" << endl;
	  cerr << "  " << sock.getErrStr() << endl;
	}
        iret = -1;
      }
      umsleep(5000);
      continue;
    }

    if (_params.debug) {
      cerr << "ERROR - ChillTsTcp2Fmq::Run" << endl;
      cerr << "  Connected to server, host, port: "
           << _params.ts_tcp_server_host << ", "
           << _params.ts_tcp_server_port << endl;
    }

    // send commands to server
 
    if (_sendCommands(sock)) {
      iret = -1;
      sock.close();
      umsleep(5000);
      continue;
    }
    
    // read from the server
    
    if (_readFromServer(sock)) {
      iret = -1;
      sock.close();
      umsleep(5000);
      continue;
    }
    
  } // while(true)

  return iret;
  
}

/////////////////////////////
// send commands to server

// defines from acq_svr_defs.h

#define CREQ_CMD_SET_FLAGS 0x00000002
#define CREQ_FLAG_HOUSEKEEPING_FULL 0x00010000
#define MK_GATE_COUNT(s,e) (((s)&0x0000FFFF)|(((e)&0x0000FFFF)<<16))

int ChillTsTcp2Fmq::_sendCommands(Socket &sock)
  
{

  if (_params.debug) {
    cerr << "Sending commands to server" << endl;
  }
  
  // server expects 2 command packets

  client_request_packet_t req_pkt;

  req_pkt.magic_word = CREQ_MAGIC;
  req_pkt.command = CREQ_CMD_SET_MODE;
  req_pkt.data[0] = CREQ_MODE_TIMESERIES;
  req_pkt.data[1] = MK_GATE_COUNT(0, 8192);                       ;
  
  if (sock.writeBuffer(&req_pkt, sizeof(req_pkt), 10000)) {
    cerr << "ERROR - ChillTsTcp2Fmq::_sendCommands" << endl;
    cerr << "  Failure sending commands to server" << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }
  
  req_pkt.magic_word = CREQ_MAGIC;
  req_pkt.command = CREQ_CMD_SET_FLAGS;
  req_pkt.data[0] = CREQ_FLAG_HOUSEKEEPING_FULL;
  req_pkt.data[1] = 0;
  
  if (sock.writeBuffer(&req_pkt, sizeof(req_pkt), 10000)) {
    cerr << "ERROR - ChillTsTcp2Fmq::_sendCommands" << endl;
    cerr << "  Failure sending commands to server" << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Sent commands to server - success" << endl;
  }
  
  return 0;

}


/////////////////////////////
// read data from the server

int ChillTsTcp2Fmq::_readFromServer(Socket &sock)
  
{

  if (_params.debug) {
    cerr << "Reading data from server" << endl;
  }
  
  // set number of pulses to 0
  
  _nPulses = 0;

  // read data
  
  while (true) {
    
    // read in the generic packet header
    
    generic_packet_header_t genHdr;
    if (sock.readBufferHb(&genHdr, sizeof(genHdr), 1024,
                          PMU_auto_register, 1000)) {
      cerr << "ERROR - ChillTsTcp2Fmq::_readFromServer" << endl;
      cerr << "  " << sock.getErrStr() << endl;
      return -1;
    }
    
    if (genHdr.magic_word == HS2_MAGIC) {
      
      // peek at id and length of payload
      
      int idLen[2];
      if (_peekAtBuffer(sock, idLen, sizeof(idLen))) {
        cerr << "ERROR - ChillTsTcp2Fmq::_readFromServer" << endl;
        cerr << "  " << sock.getErrStr() << endl;
        return -1;
      }
  
      int id = idLen[0];
      int len = idLen[1];
      
      // handle housekeeping packet

      switch (id) {
        
        case HSK_ID_RADAR_INFO:
          _readRadarInfo(sock);
          break;
        case HSK_ID_SCAN_SEG:
          _readScanSeg(sock);
          break;
        case HSK_ID_PROCESSOR_INFO:
          _readProcInfo(sock);
          break;
        case HSK_ID_PWR_UPDATE:
          _readPowerUpdate(sock);
          _readCalFromFile();
          break;
        case HSK_ID_EVENT_NOTICE:
          _readEventNotice(sock);
          break;
        case HSK_ID_CAL_TERMS:
          _readCalTerms(sock);
          break;
        case HSK_ID_XMIT_INFO:
          _readXmitInfo(sock);
          break;
        case HSK_ID_ANT_OFFSET:
          _readAntCorr(sock);
          break;
        case HSK_ID_XMIT_SAMPLE:
          _readXmitSample(sock);
          break;
        case HSK_ID_PHASE_CODE:
          _readPhaseCode(sock);
          break;
        case IWRF_XMIT_POWER_ID:
          _readIwrfXmitPower(sock);
          break;
        case IWRF_EVENT_NOTICE_ID:
          _readIwrfEventNotice(sock);
          break;
        default:
          if (_params.debug) {
            cerr << "===== NOT HANDLED YET ==================" << endl;
            fprintf(stderr, "  id: %x, %d\n", id, id);
            cerr << "  len: " <<  len << endl;
            cerr << "========================================" << endl;
          }
          _seekAhead(sock, genHdr.payload_length);
          
      } // switch (id)
      
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "+";
      }
    
    } else if (genHdr.magic_word == TS_MAGIC) {

      // read in pulse

      _readPulse(sock, genHdr);
      _nPulses++;
      
      // if the message is large enough, write to the FMQ
      
      _writeToFmq();

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << ".";
      }
    
    } else {

      // re-synchronize the data stream
      
      _reSync(sock);
      
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "#";
      }
    
    } // if (genHdr.magic_word == HS2_MAGIC) {
    
  } // while (true)
    
  return 0;

}

///////////////////////////////////////////
// re-sync the data stream
// returns 0 on success, -1 on error

int ChillTsTcp2Fmq::_reSync(Socket &sock)
  
{
  int sync_count = 0;

  if (_params.debug) {
    cerr << "Trying to resync ....." << endl;
  }
  
  generic_packet_header_t genHdr;

  while (true) {
    
    // peek at the generic header
    
    if (_peekAtBuffer(sock, &genHdr, sizeof(genHdr))) {
      cerr << "ERROR - ChillTsTcp2Fmq::_reSync" << endl;
      cerr << "  " << sock.getErrStr() << endl;
      return -1;
    }

    // check if we have a housekeeping header which matches
    
    if (genHdr.magic_word == HS2_MAGIC) {

      if (genHdr.payload_length == sizeof(radar_info_t) ||
          genHdr.payload_length == sizeof(scan_seg_t) ||
          genHdr.payload_length == sizeof(processor_info_t) ||
          genHdr.payload_length == sizeof(power_update_t) ||
          genHdr.payload_length == sizeof(xmit_sample_t) ||
          genHdr.payload_length == sizeof(event_notice_t) ||
          genHdr.payload_length == sizeof(cal_terms_t) ||
          genHdr.payload_length == sizeof(phasecode_t) ||
          genHdr.payload_length == sizeof(xmit_info_t) ||
          genHdr.payload_length == sizeof(antenna_correction_t)) {
        return 0;
      }

    }

    // no sync yet, read 1 byte and start again
    
    char byteVal;
    if (sock.readBufferHb(&byteVal, 1, 1, PMU_auto_register, 1000)) {
      cerr << "ERROR - ChillTsTcp2Fmq::_reSync" << endl;
      cerr << "  " << sock.getErrStr() << endl;
      return -1;
    }
    sync_count++;

  } // while

  return -1;

}

///////////////////////////////////////////////////////////////////
// Peek at buffer from socket
// Returns 0 on success, -1 on failure

int ChillTsTcp2Fmq::_peekAtBuffer(Socket &sock, void *buf, int nbytes)

{
  
  while (true) {
    PMU_auto_register("peekAtBuffer");
    if (sock.peek(buf, nbytes, 1000) == 0) {
      return 0;
    } else {
      if (sock.getErrNum() == Socket::TIMED_OUT) {
        PMU_auto_register("Timed out ...");
        continue;
      }
      cerr << "ERROR - ChillTsTcp2Fmq::_peekAtBuffer" << endl;
      cerr << "  " << sock.getErrStr() << endl;
      return -1;
    }
  }
  
  return -1;

}

//////////////////////////////////////////////////
// seek ahead

int ChillTsTcp2Fmq::_seekAhead(Socket &sock, int nBytes)
  
{

  TaArray<char> buf_;
  char *buf = buf_.alloc(1024);

  int nLeft = nBytes;
  while (nLeft > 0) {
    int nRead = 1024;
    if (nRead > nLeft) {
      nRead = nLeft;
    }
    if (sock.readBufferHb(buf, nRead, 1024,
                          PMU_auto_register, 1000)) {
      cerr << "ERROR - ChillTsTcp2Fmq::_seekAhead" << endl;
      cerr << "  Seeking ahead by nbytes: " << nBytes << endl;
      cerr << "  " << sock.getErrStr() << endl;
      return -1;
    }
    nLeft -= nRead;
  }
    
  return 0;

}

/////////////////////////////////////////////
// check if sweep num is correct

void ChillTsTcp2Fmq::_checkSweepNum(iwrf_pulse_header_t &pHdr)

{
  
  // adjust the sweep number
  // to be 0 at start of each volume
  
  int volNum = pHdr.volume_num;
  int sweepNum = pHdr.sweep_num;
  
  if (volNum != _prevVolNum) {
    _prevVolNum = volNum;
    _volStartSweepNum = sweepNum;
  }
  
  if (_params.zero_sweep_number_at_start_of_vol) {
    pHdr.sweep_num = sweepNum - _volStartSweepNum;
  }

}
      
//////////////////////////////////////////////////
// read radarInfo

int ChillTsTcp2Fmq::_readRadarInfo(Socket &sock)
  
{

  // read in

  if (sock.readBufferHb(&_radarInfo, sizeof(_radarInfo), 1024,
                        PMU_auto_register, 1000)) {
    cerr << "ERROR - ChillTsTcp2Fmq::_readRadarInfo" << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_radar_info_print(cerr, _radarInfo);
  }
  
  _radarInfoAvail = true;

  // convert to IWRF

  chill_iwrf_radar_info_load(_radarInfo, _packetSeqNum, _iwrfRadarInfo);
  _packetSeqNum++;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_radar_info_print(stderr, _iwrfRadarInfo);
  }

  // add to message
  
  _msg.addPart(IWRF_RADAR_INFO_ID, sizeof(_iwrfRadarInfo), &_iwrfRadarInfo);

  return 0;

}

//////////////////////////////////////////////////
// read scan segment

int ChillTsTcp2Fmq::_readScanSeg(Socket &sock)
  
{

  // read in

  if (sock.readBufferHb(&_scanSeg, sizeof(_scanSeg), 1024,
                        PMU_auto_register, 1000)) {
    cerr << "ERROR - ChillTsTcp2Fmq::_readScanSeg" << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_scan_seg_print(cerr, _scanSeg);
  }
  
  _scanSegAvail = true;

  // convert to IWRF

  chill_iwrf_scan_seg_load(_scanSeg, _packetSeqNum, _iwrfScanSeg);
  _packetSeqNum++;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_scan_segment_print(stderr, _iwrfScanSeg);
  }
  
  // add to message
  
  _msg.addPart(IWRF_SCAN_SEGMENT_ID, sizeof(_iwrfScanSeg), &_iwrfScanSeg);

  // if available add in radar info and proc info

  if (_radarInfoAvail) {
    _msg.addPart(IWRF_RADAR_INFO_ID, sizeof(_iwrfRadarInfo), &_iwrfRadarInfo);
  }

  if (_procInfoAvail) {
    _msg.addPart(IWRF_TS_PROCESSING_ID, sizeof(_iwrfTsProc), &_iwrfTsProc);
  }

  return 0;

}

//////////////////////////////////////////////////
// read proc info

int ChillTsTcp2Fmq::_readProcInfo(Socket &sock)
  
{

  // read in

  if (sock.readBufferHb(&_procInfo, sizeof(_procInfo), 1024,
                        PMU_auto_register, 1000)) {
    cerr << "ERROR - ChillTsTcp2Fmq::_readProcInfo" << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_proc_info_print(cerr, _procInfo);
  }
  
  _procInfoAvail = true;

  // convert to IWRF

  chill_iwrf_ts_proc_load(_procInfo, _packetSeqNum, _iwrfTsProc);
  _packetSeqNum++;

  if (_params.remove_negative_range_gates) {
    _nGatesRemove =
      (int) (_iwrfTsProc.burst_range_offset_m / _iwrfTsProc.gate_spacing_m + 0.5);
    _iwrfTsProc.start_range_m += _nGatesRemove * _iwrfTsProc.gate_spacing_m;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "===>>> Removing negative range gates, n = " << _nGatesRemove << endl;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_ts_processing_print(stderr, _iwrfTsProc);
  }
  
  // add to message
  
  _msg.addPart(IWRF_TS_PROCESSING_ID, sizeof(_iwrfTsProc), &_iwrfTsProc);

  return 0;

}

//////////////////////////////////////////////////
// read power update

int ChillTsTcp2Fmq::_readPowerUpdate(Socket &sock)
  
{

  // read in

  if (sock.readBufferHb(&_powerUpdate, sizeof(_powerUpdate), 1024,
                        PMU_auto_register, 1000)) {
    cerr << "ERROR - ChillTsTcp2Fmq::_readPowerUpdate" << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_power_update_print(cerr, _powerUpdate);
  }
  
  _powerUpdateAvail = true;

  // convert to IWRF

  chill_iwrf_xmit_power_load(_powerUpdate, _packetSeqNum, _iwrfXmitPower);
  _packetSeqNum++;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_xmit_power_print(stderr, _iwrfXmitPower);
  }
  
  // add to message
  
  _msg.addPart(IWRF_XMIT_POWER_ID, sizeof(_iwrfXmitPower), &_iwrfXmitPower);

  return 0;

}


//////////////////////////////////////////////////
// read power update

int ChillTsTcp2Fmq::_readIwrfXmitPower(Socket &sock)
  
{

  // read in

  if (sock.readBufferHb(&_iwrfXmitPower, sizeof(_iwrfXmitPower), 1024,
                        PMU_auto_register, 1000)) {
    cerr << "ERROR - ChillTsTcp2Fmq::_readIwrfXmitPower" << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }

  _powerUpdateAvail = true;
  _packetSeqNum++;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_xmit_power_print(stderr, _iwrfXmitPower);
  }
  
  // add to message
  
  _msg.addPart(IWRF_XMIT_POWER_ID, sizeof(_iwrfXmitPower), &_iwrfXmitPower);

  return 0;

}

//////////////////////////////////////////////////
// read event notice

int ChillTsTcp2Fmq::_readEventNotice(Socket &sock)
  
{

  // read in

  if (sock.readBufferHb(&_eventNotice, sizeof(_eventNotice), 1024,
                        PMU_auto_register, 1000)) {
    cerr << "ERROR - ChillTsTcp2Fmq::_readEventNotice" << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_event_notice_print(cerr, _eventNotice);
  }
  
  _eventNoticeAvail = true;

  if (_scanSegAvail) {

    // convert to IWRF

    chill_iwrf_event_notice_load(_eventNotice, _scanSeg,
                                 _packetSeqNum, _iwrfEventNotice);
    _packetSeqNum++;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      chill_event_notice_print(cerr, _eventNotice);
      iwrf_event_notice_print(stderr, _iwrfEventNotice);
    }

    // add to message
    
    _msg.addPart(IWRF_EVENT_NOTICE_ID, sizeof(_iwrfEventNotice),
                 &_iwrfEventNotice);

  }
  
  return 0;

}

//////////////////////////////////////////////////
// read iwrf event notice

int ChillTsTcp2Fmq::_readIwrfEventNotice(Socket &sock)
  
{

  // read in

  if (sock.readBufferHb(&_iwrfEventNotice, sizeof(_iwrfEventNotice), 1024,
                        PMU_auto_register, 1000)) {
    cerr << "ERROR - ChillTsTcp2Fmq::_readIwrfEventNotice" << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }

  _eventNoticeAvail = true;

  if (_scanSegAvail) {

    _packetSeqNum++;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      iwrf_event_notice_print(stderr, _iwrfEventNotice);
    }

    // add to message
    
    _msg.addPart(IWRF_EVENT_NOTICE_ID, sizeof(_iwrfEventNotice),
                 &_iwrfEventNotice);

  }
  
  return 0;

}

//////////////////////////////////////////////////
// read cal terms

int ChillTsTcp2Fmq::_readCalTerms(Socket &sock)
  
{

  // read in
  
  if (sock.readBufferHb(&_calTerms, sizeof(_calTerms), 1024,
                        PMU_auto_register, 1000)) {
    cerr << "ERROR - ChillTsTcp2Fmq::_readCalTerms" << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_cal_terms_print(cerr, _calTerms);
  }

  if (_radarInfoAvail && _powerUpdateAvail) {

    _calTermsAvail = true;

    // convert to IWRF

    chill_iwrf_calibration_load(_radarInfo, _calTerms, _powerUpdate,
                                _packetSeqNum, _iwrfCalib);
    _packetSeqNum++;
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      iwrf_calibration_print(stderr, _iwrfCalib);
    }

    // add to message
    
    if (!_params.read_cal_from_file) {
      _msg.addPart(IWRF_CALIBRATION_ID, sizeof(_iwrfCalib), &_iwrfCalib);
    }

  }
  
  return 0;

}

//////////////////////////////////////////////////
// read cal from file

int ChillTsTcp2Fmq::_readCalFromFile()
  
{

  if (!_params.read_cal_from_file) {
    return 0;
  }

  // is there a new file?

  if (_calibLdata.read()) {
    if (_params.debug) {
      cerr << "INFO - _readCalFromFile" << endl;
      cerr << "       no new cal file available" << endl;
      return -1;
    }
  }

  // read ladest data info
  
  string calPath = _calibLdata.getDataPath();
  FILE *in;
  if ((in = fopen(calPath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "WARNING - _readCalFromFile" << endl;
    cerr << "  Cannot open cal file path: " << calPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  long fileLen = ta_stat_get_len(calPath.c_str());
  if (fileLen < 100) {
    cerr << "WARNING - _readCalFromFile" << endl;
    cerr << "  Cal file not properly written, path: " << calPath << endl;
    cerr << "  File len: " << fileLen << endl;
    return -1;
  }

  char *xmlBuf = new char[fileLen + 1];
  if ((long) fread(xmlBuf, 1, fileLen, in) != fileLen) {
    int errNum = errno;
    cerr << "WARNING - _readCalFromFile" << endl;
    cerr << "  Cannot read cal file path: " << calPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    fclose(in);
    delete[] xmlBuf;
    return -1;
  }
  fclose(in);

  // populate struct

  IwrfCalib calib;
  string errStr;
  if (calib.setFromXml(xmlBuf, errStr)) {
    cerr << "WARNING - _readCalFromFile" << endl;
    cerr << "  Cannot interpret data from file path: " << calPath << endl;
    cerr << "XML:" << endl;
    cerr << xmlBuf << "==============" << endl;
    cerr << errStr << endl;
    delete[] xmlBuf;
    return -1;
  }
  delete[] xmlBuf;

  // set calib

  _iwrfCalib = calib.getStruct();
  _iwrfCalibAvail = true;

  if(_params.debug) {
    cerr << "Read in cal from file: " << calPath << endl;
    if(_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "====================================" << endl;
      cerr << xmlBuf << endl;
      cerr << "====================================" << endl;
    }
  }
  
  // add to message
  
  _msg.addPart(IWRF_CALIBRATION_ID, sizeof(_iwrfCalib), &_iwrfCalib);
  
  return 0;

}

//////////////////////////////////////////////////
// read xmit info

int ChillTsTcp2Fmq::_readXmitInfo(Socket &sock)
  
{

  // read in

  if (sock.readBufferHb(&_xmitInfo, sizeof(_xmitInfo), 1024,
                        PMU_auto_register, 1000)) {
    cerr << "ERROR - ChillTsTcp2Fmq::_readXmitInfo" << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_xmit_info_print(cerr, _xmitInfo);
  }

  _xmitInfoAvail = true;

  // convert to IWRF

  chill_iwrf_xmit_info_load(_xmitInfo, _packetSeqNum, _iwrfXmitInfo);
  _packetSeqNum++;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_xmit_info_print(cerr, _xmitInfo);
    iwrf_xmit_info_print(stderr, _iwrfXmitInfo);
  }
  
  // add to message
  
  _msg.addPart(IWRF_XMIT_INFO_ID, sizeof(_iwrfXmitInfo), &_iwrfXmitInfo);

  return 0;

}

//////////////////////////////////////////////////
// read antenna correction

int ChillTsTcp2Fmq::_readAntCorr(Socket &sock)
  
{

  // read in

  if (sock.readBufferHb(&_antCorr, sizeof(_antCorr), 1024,
                        PMU_auto_register, 1000)) {
    cerr << "ERROR - ChillTsTcp2Fmq::_readAntCorr" << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    chill_ant_corr_print(cerr, _antCorr);
  }

  _antCorrAvail = true;
 
  // convert to IWRF

  chill_iwrf_ant_corr_load(_antCorr, _packetSeqNum, _iwrfAntCorr);
  _packetSeqNum++;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_antenna_correction_print(stderr, _iwrfAntCorr);
  }
  
  // add to message
  
  _msg.addPart(IWRF_ANTENNA_CORRECTION_ID, sizeof(_iwrfAntCorr), &_iwrfAntCorr);

  return 0;

}

//////////////////////////////////////////////////
// read xmit sample

int ChillTsTcp2Fmq::_readXmitSample(Socket &sock)
  
{

  // read in
  
  if (sock.readBufferHb(&_xmitSample, sizeof(_xmitSample), 1024,
                        PMU_auto_register, 1000)) {
    cerr << "ERROR - ChillTsTcp2Fmq::_readXmitSample" << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    chill_xmit_sample_print(cerr, _xmitSample);
  }

  _xmitSampleAvail = true;

  // convert to IWRF

  chill_iwrf_xmit_sample_load(_xmitSample, _packetSeqNum, _iwrfXmitSample);
  _packetSeqNum++;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Read xmit sample" << endl;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      iwrf_xmit_sample_print(stderr, _iwrfXmitSample);
    }
  }
  
  // add to message
  
  _msg.addPart(IWRF_XMIT_SAMPLE_ID, sizeof(_iwrfXmitSample), &_iwrfXmitSample);
        
  return 0;

}

//////////////////////////////////////////////////
// read phase code

int ChillTsTcp2Fmq::_readPhaseCode(Socket &sock)
  
{

  // read in

  if (sock.readBufferHb(&_phaseCode, sizeof(_phaseCode), 1024,
                        PMU_auto_register, 1000)) {
    cerr << "ERROR - ChillTsTcp2Fmq::_readPhaseCode" << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    chill_phasecode_print(cerr, _phaseCode);
  }

  _phaseCodeAvail = true;

  // convert to IWRF

  chill_iwrf_phasecode_load(_phaseCode, _packetSeqNum, _iwrfPhasecode);
  _packetSeqNum++;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrf_phasecode_print(stderr, _iwrfPhasecode);
  }
  
  // add to message
  
  _msg.addPart(IWRF_PHASECODE_ID, sizeof(_iwrfPhasecode), &_iwrfPhasecode);

  return 0;

}

//////////////////////////////////////////////////
// read pulse

int ChillTsTcp2Fmq::_readPulse(Socket &sock,
                               const generic_packet_header &genHdr)
  
{

  timeseries_packet_header pulseHdr;
  memcpy(&pulseHdr, &genHdr, sizeof(pulseHdr));
  
  double elev = pulseHdr.antenna_elevation * 360.0 / 65536.0;
  if (elev > 180) {
    elev -= 360.0;
  }
  double az = pulseHdr.antenna_azimuth * 360.0 / 65536.0;
  int nanoSecs = pulseHdr.timestamp * 25;
  
  unsigned int timeOfDay = (pulseHdr.hdr_word3 << 15 ) >> 15;
  unsigned int hvFlag = (pulseHdr.hdr_word3 << 13) >> 30;
  unsigned int endOfInt = (pulseHdr.hdr_word3 << 12) >> 31;
  unsigned int pulseCount = (pulseHdr.hdr_word3 >> 23);
  
  int nGatesChill = (int) pulseHdr.gatecount;
  int nGatesOut = nGatesChill - _nGatesRemove;
  int nBytesData = nGatesChill * sizeof(timeseries_sample_t);
  time_t midnight = pulseHdr.hdr_word7;
  time_t pulseTimeSecs = midnight + timeOfDay;

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "===============================================" << endl;
    fprintf(stderr, "CHILL PULSE HEADER:\n");
    fprintf(stderr, "  Time: %s.%.6d\n",
            DateTime::strm(pulseTimeSecs).c_str(),
            nanoSecs / 1000);
    fprintf(stderr, "  elev: %g\n", elev);
    fprintf(stderr, "  az: %g\n", az);
    fprintf(stderr, "  hvFlag: %d\n", hvFlag);
    fprintf(stderr, "  endOfInt: %d\n", endOfInt);
    fprintf(stderr, "  pulseCount: %d\n", pulseCount);
    fprintf(stderr, "  nGatesChill: %d\n", nGatesChill);
    fprintf(stderr, "  nGatesOut: %d\n", nGatesOut);
    fprintf(stderr, "  nBytesData: %d\n", nBytesData);
  }

  if (nGatesChill > 10000) {
    cerr << "ERROR - ChillTsTcp2Fmq::_readPulse" << endl;
    cerr << "  Too many gates: " << nGatesChill << endl;
    return -1;
  }

  // read in IQ data

  TaArray<timeseries_sample_t> gateData_;
  timeseries_sample_t *gateData = gateData_.alloc(nGatesChill);
  int nBytes = nGatesChill * sizeof(timeseries_sample_t);

  if (sock.readBufferHb(gateData, nBytes, 1024,
                        PMU_auto_register, 1000)) {
    cerr << "ERROR - ChillTsTcp2Fmq::_readPulse" << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }

  // set up IWRF pulse object
  
  _iwrfPulse.clear();
  _iwrfPulse.setPktSeqNum(_packetSeqNum);
  _packetSeqNum++;
  _iwrfPulse.setTime(pulseTimeSecs, nanoSecs);

  _iwrfPulse.set_pulse_seq_num(_pulseSeqNum);
  _pulseSeqNum++;
  _iwrfPulse.set_scan_mode(_iwrfScanSeg.scan_mode);
  _iwrfPulse.set_follow_mode(_iwrfScanSeg.follow_mode);
  _iwrfPulse.set_sweep_num(_iwrfScanSeg.sweep_num);
  _iwrfPulse.set_volume_num(_iwrfScanSeg.volume_num);
  if (_iwrfScanSeg.scan_mode == IWRF_SCAN_MODE_RHI) {
    _iwrfPulse.set_fixed_az(_iwrfScanSeg.current_fixed_angle);
  } else {
    _iwrfPulse.set_fixed_el(_iwrfScanSeg.current_fixed_angle);
  }
  _iwrfPulse.set_elevation(elev);
  _iwrfPulse.set_azimuth(az);
  _iwrfPulse.set_prt(_iwrfTsProc.prt_usec / 1.0e6);
  _iwrfPulse.set_prt_next(_iwrfTsProc.prt_usec / 1.0e6);
  _iwrfPulse.set_pulse_width_us(_iwrfTsProc.pulse_width_us);
  _iwrfPulse.set_n_gates(nGatesOut);
  _iwrfPulse.set_n_channels(2);
  _iwrfPulse.set_iq_encoding(IWRF_IQ_ENCODING_SCALED_SI16);
  if (hvFlag == 1) {
    _iwrfPulse.set_hv_flag(true);
  } else {
    _iwrfPulse.set_hv_flag(false);
  }
  _iwrfPulse.set_antenna_transition(false);
  _iwrfPulse.set_phase_cohered(true);
  _iwrfPulse.set_iq_offset(0, 0);
  _iwrfPulse.set_iq_offset(1, nGatesOut * 2);
  _iwrfPulse.set_scale(1.0);
  _iwrfPulse.set_offset(0.0);
  _iwrfPulse.set_n_gates_burst(0);
  _iwrfPulse.set_start_range_m(_iwrfTsProc.start_range_m);
  _iwrfPulse.set_gate_spacing_m(_iwrfTsProc.gate_spacing_m);

  // load IQ data into array for IWRF

  TaArray<si16> packed_;
  si16 *packed = packed_.alloc(nGatesOut * 4);
  int index = 0;
  for (int ii = _nGatesRemove; ii < nGatesChill; ii++) {
    packed[index++] = gateData[ii].i2;
    packed[index++] = gateData[ii].q2;
  }
  for (int ii = _nGatesRemove; ii < nGatesChill; ii++) {
    packed[index++] = gateData[ii].i1;
    packed[index++] = gateData[ii].q1;
  }

  _iwrfPulse.setIqPacked(nGatesOut, 2,
                         IWRF_IQ_ENCODING_SCALED_SI16, packed,
                         _params.iq_data_scale, _params.iq_data_offset);

  // assemble into a membuf

  MemBuf pulseBuf;
  _iwrfPulse.assemble(pulseBuf);
  _msg.addPart(IWRF_PULSE_HEADER_ID, pulseBuf.getLen(), pulseBuf.getPtr());

  return 0;

}

///////////////////////////////////////
// write to output FMQ if ready
// returns 0 on success, -1 on failure

int ChillTsTcp2Fmq::_writeToFmq()

{

  // if the message is large enough, write to the FMQ
  
  int nParts = _msg.getNParts();
  if (nParts < _params.n_pulses_per_message) {
    return 0;
  }

  // regularly, add in the metadata

  time_t now = time(NULL);
  int64_t timeSinceLastMeta = now - _prevMetaTime;
  if (timeSinceLastMeta > _params.metadata_interval_secs) {
    if (_radarInfoAvail) {
      _msg.addPart(IWRF_RADAR_INFO_ID, sizeof(_iwrfRadarInfo), &_iwrfRadarInfo);
    }
    if (_scanSegAvail) {
      _msg.addPart(IWRF_SCAN_SEGMENT_ID, sizeof(_iwrfScanSeg), &_iwrfScanSeg);
    }
    if (_procInfoAvail) {
      _msg.addPart(IWRF_TS_PROCESSING_ID, sizeof(_iwrfTsProc), &_iwrfTsProc);
    }
    if (_calTermsAvail) {
      _msg.addPart(IWRF_CALIBRATION_ID, sizeof(_iwrfCalib), &_iwrfCalib);
    }
    if (_xmitInfoAvail) {
      _msg.addPart(IWRF_XMIT_INFO_ID, sizeof(_iwrfXmitInfo), &_iwrfXmitInfo);
    }
    if (_antCorrAvail) {
      _msg.addPart(IWRF_ANTENNA_CORRECTION_ID, sizeof(_iwrfAntCorr), &_iwrfAntCorr);
    }
    _prevMetaTime = now;
  }

  // assemble message

  void *buf = _msg.assemble();
  int len = _msg.lengthAssembled();
  if (_fmq.writeMsg(0, 0, buf, len)) {
    cerr << "ERROR - ChillTsTcp2Fmq" << endl;
    cerr << "  Cannot write FMQ: " << _params.output_fmq_path << endl;
    _msg.clearParts();
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "  Wrote msg, nparts, len: "
         << nParts << ", " << len << endl;
  }
  _msg.clearParts();

  return 0;

}
    
