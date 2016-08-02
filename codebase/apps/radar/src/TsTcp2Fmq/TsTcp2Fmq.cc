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
// TsTcp2Fmq.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// TsTcp2Fmq reads data from a server in TCP, and writes
// it out to an FMQ
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <sys/stat.h>
#include <toolsa/pmu.h>
#include <toolsa/uusleep.h>
#include <toolsa/DateTime.hh>
#include <radar/RadarComplex.hh>
#include <radar/iwrf_data.h>
#include <radar/iwrf_functions.hh>
#include <radar/IwrfTsPulse.hh>
#include "TsTcp2Fmq.hh"

using namespace std;

// Constructor

TsTcp2Fmq::TsTcp2Fmq(int argc, char **argv)
  
{

  isOK = true;

  _sockTimedOut = false;
  _timedOutCount = 0;
  _unknownMsgId = false;
  _unknownCount = 0;

  _pulseCount = 0;
  _posnLastChecked = 0;
  _posnLastModified = 0;
  _scaleWarningPrinted = false;

  _prevPulseTime = 0;

  _prevVolNum = -1;
  _volNum = 0;
  _volStartSweepNum = 0;
  
  _sweepNum = 0;
  _sweepNumChangeInit = false;
  _sweepNumChangeInProgress = false;
  _sweepNumOld = 0;
  _sweepNumAzOld = 0;
  _sweepNumTransDirn = 0;

  // set programe name
 
  _progName = "TsTcp2Fmq";

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

  // load up cal if needed

  if (_params.override_calibration) {
    string errStr;
    if (_calib.readFromXmlFile(_params.cal_xml_file_path, errStr)) {
      cerr << "ERROR - SpolTs2Fmq" << endl;
      cerr << "  Cannot read cal from file: "
           << _params.cal_xml_file_path << endl;
      cerr << errStr << endl;
      isOK = false;
      return;
    }
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

TsTcp2Fmq::~TsTcp2Fmq()

{

  // close socket

  _sock.close();

  // close FMQ

  _outputFmq.closeMsgQueue();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int TsTcp2Fmq::Run ()
{

  PMU_auto_register("Run");

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running TsTcp2Fmq - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running TsTcp2Fmq - debug mode" << endl;
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
	  cerr << "ERROR - TsTcp2Fmq::Run" << endl;
	  cerr << "  Connecting to server" << endl;
	  cerr << "  " << _sock.getErrStr() << endl;
	}
        iret = -1;
      }
      umsleep(1000);
      continue;
    }

    // read from the server
    
    if (_readFromServer()) {
      iret = -1;
    }

    _sock.close();
    
  } // while(true)

  return iret;
  
}

/////////////////////////////
// read data from the server

int TsTcp2Fmq::_readFromServer()

{

  // read data

  long count = 0;
  while (true) {
    
    if (!_params.do_not_register_on_read) {
      PMU_auto_register("Reading data");
    }

    // Check for a new Location if applicable
    
    _loadPosition();

    // read packet from time series server server

    if (_readMessage()) {
      if (!_sockTimedOut && !_unknownMsgId) {
        // error
        cerr << "ERROR - TsTcp2Fmq::_readFromServer" << endl;
        return -1;
      }
      // on timeout, skip to next message
      if (_sockTimedOut) {
        continue;
      }
    }

    // handle packet types

    if (_packetId == IWRF_PULSE_HEADER_ID) {
      
      // pulse header and data
      
      iwrf_pulse_header_t &pHdr = 
        *((iwrf_pulse_header_t *) _inputBuf.getPtr());

      count++;
      if (count % 1000 == 0) {
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "===>> 1000 pulses read, time, seqNum: " 
               << DateTime::strm(pHdr.packet.time_secs_utc) << ", "
               << pHdr.pulse_seq_num << endl;
        }
      }
      
      // scale data as applicable
      // this will convert the IQ data to floats
      
      if(_params.apply_scale) {
        _applyIQScale();
      }

      // Correct the azimuth angle from vehicle heading
      
      if(_params.update_position) {
        _modifyAzFromHeading(pHdr);
      }
      
      // Set the azimuth from the time
      
      if(_params.compute_azimuth_from_time) {
        _setAzFromTime(pHdr);
      }
      
      // modify the sweep number if appropriate

      _modifySweepNumber(pHdr); 

      // if appropriate, ignore pulses with time that goes backwards
      
      if (_params.ignore_pulse_if_time_goes_backwards) {
        if (_checkTimeGoesForward(pHdr) == false) {
          continue;
        }
      }

      // add to message

      _outputMsg.addPart(_packetId, _packetLen, _inputBuf.getPtr());
      
    } else if (_packetId == IWRF_RADAR_INFO_ID) {
      
      // add to FMQ

      _outputMsg.addPart(_packetId, _packetLen, _inputBuf.getPtr());

    } else if (_packetId == IWRF_SCAN_SEGMENT_ID) {
      
      // add to FMQ

      _outputMsg.addPart(_packetId, _packetLen, _inputBuf.getPtr());
      
    } else if (_packetId == IWRF_TS_PROCESSING_ID) {
      
      // add to FMQ

      _outputMsg.addPart(_packetId, _packetLen, _inputBuf.getPtr());

    } else if (_packetId == IWRF_CALIBRATION_ID) {
      
      _handleCalibration();
      
    } else {

      // other packet type
      // add to outgoing message
      
      _outputMsg.addPart(_packetId, _packetLen, _inputBuf.getPtr());

    }

    // if the message is large enough, write to the FMQ

    _writeToOutputFmq();
    
  } // while (true)

  return 0;

}

///////////////////////////////////////////////////////////////////
// Read in next message, set id and load buffer.
// Returns 0 on success, -1 on failure

int TsTcp2Fmq::_readMessage()
  
{

  while (true) {

    if (!_params.do_not_register_on_read) {
      PMU_auto_register("_readMessage");
    }

    if (_readTcpPacket() == 0) {
      return 0;
    }
    
    if (!_sockTimedOut && !_unknownMsgId) {
      // socket error
      cerr << "ERROR - TsTcp2Fmq::_readFromServer" << endl;
      return -1;
    }

    if (_sockTimedOut && _params.write_end_of_vol_when_data_stops) {
      // if no data for a while, write end of volume event
      if (_timedOutCount >= _params.nsecs_no_data_for_end_of_vol * 10) {
        _writeEndOfVol();
      }
    }

  } // while

}

///////////////////////////////////////////////////////////////////
// Read in next packet, set id and load buffer.
// Returns 0 on success, -1 on failure

int TsTcp2Fmq::_readTcpPacket()

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
      cerr << "ERROR - TsTcp2Fmq::_readTcpPacket" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
    }
    return -1;
  }
  _timedOutCount = 0;
  
  // check ID for packet, and get its length
  
  _packetId = packetTop[0];
  _packetLen = packetTop[1];

  bool isSwapped;
  if (iwrf_check_packet_id(_packetId, _packetLen, &isSwapped)) {
    if (_params.debug >= Params::DEBUG_EXTRA) {
      if (isSwapped) {
        cerr << "INFO - time series data is swapped" << endl;
      }
    }
  }

  // make sure the size is reasonable
  
  if (_packetLen > 1000000 ||
      _packetLen < (int) sizeof(packetTop)) {
    cerr << "ERROR - TsTcp2Fmq::_readTcpPacket" << endl;
    cerr << "  Bad packet length: " << _packetLen << endl;
    fprintf(stderr, "  id: 0x%x\n", _packetId);
    cerr << "  Need to reconnect to server to resync" << endl;
    return -1;
  }
  
  // compute nbytes still to read in
  
  int nBytesRemaining = _packetLen - sizeof(packetTop);

  // reserve space in the buffer
  
  _inputBuf.reserve(_packetLen);
  
  // copy the packet top into the start of the buffer
  
  memcpy(_inputBuf.getPtr(), packetTop, sizeof(packetTop));
  char *readPtr = (char *) _inputBuf.getPtr() + sizeof(packetTop);
  
  // read in the remainder of the buffer
  
  if (_sock.readBufferHb(readPtr, nBytesRemaining, 1024,
                         _heartBeatFunc, 10000)) {
    cerr << "ERROR - TsTcp2Fmq::_readTcpPacket" << endl;
    cerr << "  " << _sock.getErrStr() << endl;
    return -1;
  }
  
  // check that we have a valid message
  
  if (iwrf_check_packet_id(_packetId, _packetLen)) {
    // unknown packet type, read it in and continue without processing
    _unknownCount++;
    if (_unknownCount > 100) {
      cerr << "ERROR - TsTcp2Fmq::_readTcpPacket" << endl;
      cerr << "  Too many resync tries, need to reconnect" << endl;
      umsleep(1000);
      _sock.close();
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
    }
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA && 
      _packetId != IWRF_PULSE_HEADER_ID && _packetId != IWRF_SYNC_ID) {
    iwrf_packet_print(stderr, _inputBuf.getPtr(), _inputBuf.getLen());
  }
  
  if(_params.debug >=  Params::DEBUG_EXTRA && 
     (_packetId == IWRF_PULSE_HEADER_ID || _packetId == IWRF_SYNC_ID))   {
    iwrf_packet_print(stderr, _inputBuf.getPtr(), _inputBuf.getLen());
  }

  return 0;

}

/////////////////////////////////////////////
// modify sweep number

void TsTcp2Fmq::_modifySweepNumber(iwrf_pulse_header_t &pHdr)

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

///////////////////////////////////////
// Modify the radar location

void TsTcp2Fmq::_modifyRadarLocation(iwrf_radar_info_t &radar)
{

  radar.latitude_deg = _radarLatitude;
  radar.longitude_deg = _radarLongitude;
  radar.altitude_m = _radarAltitude;

}

/////////////////////////////////////////////////
// correct the azimuth using the vehicle heading

void TsTcp2Fmq::_modifyAzFromHeading(iwrf_pulse_header_t &pHdr)

{

  pHdr.azimuth = pHdr.azimuth + _vehicleHeading;
  while(pHdr.azimuth >= 360.0) pHdr.azimuth -= 360.0;
  while(pHdr.azimuth < 0.0) pHdr.azimuth += 360.0;

}
     
/////////////////////////////////////////////////
// set the azimuth from the time

void TsTcp2Fmq::_setAzFromTime(iwrf_pulse_header_t &pHdr)

{

  const iwrf_packet_info_t &pkt = pHdr.packet;
  
  double degPerSec = 360.0 / _params.simulated_secs_per_rev;
  double timeSinceN = ((pkt.time_secs_utc % _params.simulated_secs_per_rev) +
                       (pkt.time_nano_secs * 1.0e-9));

  double az = timeSinceN * degPerSec;
  if (az >= 360.0) {
    az -= 360.0;
  }

  pHdr.azimuth = az;
  
}

/////////////////////////////////////////////////
// check the time goes forward

bool TsTcp2Fmq::_checkTimeGoesForward(iwrf_pulse_header_t &pHdr)

{

  const iwrf_packet_info_t &pkt = pHdr.packet;
  double ptime = pkt.time_secs_utc + (pkt.time_nano_secs * 1.0e-9);
  
  if (_prevPulseTime == 0) {
    _prevPulseTime = ptime;
    return true;
  }

  double tdiff = ptime - _prevPulseTime;

  if (tdiff <= 0) {
    return false;
  }

  _prevPulseTime = ptime;
  return true;

}

///////////////////////////////
// handle a calibration packet

void TsTcp2Fmq::_handleCalibration()

{

  if (!_params.override_calibration) {
    // pass packet through unchanged
    _outputMsg.addPart(_packetId, _packetLen, _inputBuf.getPtr());
    return;
  }

  // use calibration read in from file

  iwrf_calibration_t calib = _calib.getStruct();

  // write to FMQ
  
  _outputMsg.addPart(IWRF_CALIBRATION_ID, sizeof(calib), &calib);

}

/////////////////////////////////////////////////////////
// delay the sweep number change, in sector mode, until
// the antenna turns around

void TsTcp2Fmq::_delaySweepNumChange(iwrf_pulse_header_t &pHdr)

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

//////////////////////////////////////////////
// Load a Mobile radar's position and heading.

int TsTcp2Fmq::_loadPosition()
{

  if(!_params.update_position) {
    return 0;
  }

  // do we need to check it now?

  time_t now = time(NULL);
  int tdiff = now - _posnLastChecked;
  if (tdiff < _params.position_check_interval) {
    return 0;
  }
  _posnLastChecked = now;

  // Check for existence of position file

  struct stat sbuf;
  if(stat(_params.position_file_path,&sbuf) != 0) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - TsTcp2Fmq::_loadPosition" << endl;
      cerr << "  File does not exist: " << _params.position_file_path << endl;
    }
    return -1;
  }

  // Has the file been updated since we last looked?

  if(sbuf.st_mtime <= _posnLastModified) {
    return 0;
  }
  _posnLastModified = sbuf.st_mtime; 
    
  // Open the file
  
  FILE *infile;
  if((infile = fopen(_params.position_file_path,"r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - TsTcp2Fmq::_loadPosition" << endl;
    cerr << "  Cannot open file: " << _params.position_file_path << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // seek close to the end of the file so we don't waste time
  // on long files

  fseek(infile, -1024, SEEK_END);
  
  // read a line at a time, and stop reading when we are
  // less that 10 characters from the end of the file
  // this will ensure we have the last line

  char line[1024];
  while (!feof(infile)) {
    long pos = ftell(infile);
    if (sbuf.st_mtime - pos < 10) {
      break;
    }
    if (fgets(line, 1024, infile) == NULL) {
      break;
    }
  }
  fclose(infile);
  
  double lat,lon,alt,head,t;
  if(sscanf(line,"%lf %lf %lf %lf %lf",&head,&lat,&lon,&alt,&t) != 5) {
    cerr << "ERROR - TsTcp2Fmq::_loadPosition" << endl;
    cerr << "  Cannot decode last line" << endl;
    cerr << "  File: " << _params.position_file_path << endl;
    cerr << "  Text: '" << line << "'" << endl;
    return -1;
  }
  
  // QA the values for sanity.

  bool error = false;

  if (head < -180.0 || head > 360.0) {
    error = true;
    cerr << "  Bad heading: " << head << endl;
  }

  if (lat < -90.0 || lat > 90.0)  {
    error = true;
    cerr << "  Bad latitude: " << lat << endl;
  }

  if (lon < -180.0 || lon > 360.0)  {
    error = true;
    cerr << "  Bad longitude: " << lon << endl;
  }

  if (alt < -1000.0 || alt > 30000.0) {
    error = true;
    cerr << "  Bad altitude: " << alt << endl;
  }
  
  if (error) {
    cerr << "ERROR - TsTcp2Fmq::_loadPosition" << endl;
    cerr << "  File: " << _params.position_file_path << endl;
    cerr << "  Text: '" << line << "'" << endl;
    return -1;  
  }

  // Now set the global values.

  _radarLatitude = lat;
  _radarLongitude = lon;
  _radarAltitude = alt;
  _vehicleHeading = head;
  
  if (_params.debug) {
    cerr << "DEBUG - TsTcp2Fmq::_loadPosition" << endl;
    cerr << "  ++>> Loading new location:" << endl;
    cerr << "  ++>>   _radarLatitude: " << _radarLatitude << endl;
    cerr << "  ++>>   _radarLongitude: " << _radarLongitude << endl;
    cerr << "  ++>>   _radarAltitude: " << _radarAltitude << endl;
    cerr << "  ++>>   _vehicleHeading: " << _vehicleHeading << endl;
  }

  return 0;
  
}
      
///////////////////////////////////////
// open the output FMQ
// returns 0 on success, -1 on failure

int TsTcp2Fmq::_openOutputFmq()

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

int TsTcp2Fmq::_writeToOutputFmq(bool force)

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
    cerr << "ERROR - TsTcp2Fmq" << endl;
    cerr << "  Cannot write FMQ: " << _params.output_fmq_path << endl;
    _outputMsg.clearParts();
    return -1;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
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

int TsTcp2Fmq::_writeEndOfVol()

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

////////////////////////////////////
// Scale IQ data in pulse buffer
// Only applies to FLOAT data

void TsTcp2Fmq::_applyIQScale()
  
{

  ui08 *start = (ui08 *) _inputBuf.getPtr();
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

