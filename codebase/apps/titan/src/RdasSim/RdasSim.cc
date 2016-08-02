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
// RdasSim.cc
//
// RdasSim object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
///////////////////////////////////////////////////////////////
//
// RdasSim produces a forecast image based on (a) motion data
// provided in the form of (u,v) components on a grid and
// (b) image data on a grid.
//
///////////////////////////////////////////////////////////////

#include "RdasSim.hh"
#include <iostream>
#include <cstdlib>
#include <dataport/bigend.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/ServerSocket.hh>
#include <toolsa/TaArray.hh>
#include <rapmath/math_macros.h>
#include <rapmath/stats.h>
#include <rapformats/bprp.h>

using namespace std;


// Constructor

RdasSim::RdasSim(int argc, char **argv)

{

  OK = TRUE;
  gettimeofday(&_prevBeamTime, &_timezone);
  STATS_uniform_seed(0);

  _mainPower = false;
  _systemReady = false;
  _servoPower = false;
  _radiate = false;
  _startVol = false;

  _statusFlags = 0;
  _errorFlags = 2;
  _statusStr[0] = '\0';
  _autoResetTimeout = 0;
  _shutdownOpMode = SHUTDOWN_MODE_OFF;
  
  _prf = 1000.0;
  _azSlewRate = 30.0;
  _elSlewRate = 10.0;
  _nGates = 256;
  _startRange = 0.3;
  _gateSpacing = 0.6;
  
  _requestedEl = 0.0;
  _requestedAz = 0.0;
  _actualEl = 0.0;
  _actualAz = 0.0;
  
  _opMode = OP_OFF;
  _scanMode = SCAN_MANUAL;
  _inEscapeSeq = false;
  _inKeySeq = false;

  _sumAzPpi = 0.0;
  _volElevIndex = 0;
  _endOfPpi = false;

  _endOfTiltFlag = false;
  _endOfVolFlag = false;
  _bprpRayCount = 0;

  _kbdFd = fileno(stdin);

  // set programe name

  _progName = "RdasSim";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			    &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    OK = FALSE;
  }

  _pulseGateStart = _params.pulse_gate_start;
  _pulseGateEnd = _params.pulse_gate_end;
  _pulseHeightCount = _params.pulse_height_count;
  
  if (!OK) {
    return;
  }

  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

RdasSim::~RdasSim()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RdasSim::Run ()
{

  PMU_auto_register("RdasSim::Run");

  // read in input MDV file

  if (_readInputFile()) {
    return -1;
  }

  // open a server-side socket

  ServerSocket serv;
  if (serv.openServer(_params.server_port)) {
    cerr << "ERROR - RdasSim" << endl;
    cerr << "  Cannot open server socket, port: "
	 << _params.server_port << endl;
    cerr << serv.getErrStr() << endl;
    return -1;
  }
  if (_params.debug) {
    cerr << "server side socket opened" << endl;
  }
  
  // wait for a client
  
  int iret = 0;
  while (true) {
    PMU_auto_register("Waiting for client");
    Socket *client = serv.getClient(1000);
    if (client == NULL) {
      if (serv.getErrNum() == Socket::TIMED_OUT) {
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "Timed out waiting for client, port: "
	       << _params.server_port << endl;
	  cerr << serv.getErrStr() << endl;
	}
      } else {
	cerr << "ERROR - RdasSim" << endl;
	cerr << "  getting client..." << endl;
	cerr << serv.getErrStr() << endl;
	serv.close();
	return -1;
      }
    } else {
      if (_params.debug) {
	cerr << "Got client... " << endl;
      }
      // serve client
      if (_params.bprp_format) {
	if (_serveBprpClient(*client)) {
	  iret = -1;
	}
      } else {
	if (_serveClient(*client)) {
	  iret = -1;
	}
      }
      if (_params.debug) {
	cerr << "deleting client object" << endl;
        if (_shutdownOpMode == SHUTDOWN_MODE_OFF) {
          cerr << "Shutdown mode: OFF" << endl;
        } else {
          cerr << "Shutdown mode: STANDBY" << endl;
        }
      }
      delete client;
    }
  } // while
  
  serv.close();
  return iret;

}

//////////////////////////////////////////////////
// read input file

int RdasSim::_readInputFile()
{

  PMU_auto_register("Reading input file");

  // read in MDV file

  _mdvx.clearRead();
  _mdvx.setReadPath(_params.input_mdv_path);
  _mdvx.addReadField(_params.dbz_field_name);
  _mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  if (_mdvx.readVolume()) {
    cerr << _mdvx.getErrStr();
    return -1;
  }

  const Mdvx::master_header_t &mhdr = _mdvx.getMasterHeader();
  MdvxField *field = _mdvx.getFieldByName(_params.dbz_field_name);
  const Mdvx::field_header_t &fhdr = field->getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr = field->getVlevelHeader();
  MdvxProj proj(field->getFieldHeader());
  MdvxRadar mdvxRadar;
  if (mdvxRadar.loadFromMdvx(_mdvx) == 0) {
    DsRadarParams radar = mdvxRadar.getRadarParams();
    proj.setSensorPosn(radar.latitude, radar.longitude, radar.altitude);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=========== INPUT FILE CONTENTS ============" << endl;
    cerr << "Path: " << _mdvx.getPathInUse() << endl;
    _mdvx.printMasterHeader(mhdr, cerr);
    _mdvx.printFieldHeader(fhdr, cerr);
    _mdvx.printVlevelHeader(vhdr, fhdr.nz, fhdr.field_name, cerr);
    proj.print(cerr);
  }

  // check for polar data

  //   if (fhdr.proj_type != Mdvx::PROJ_POLAR_RADAR) {
  //     cerr << "ERROR - RdasSim" << endl;
  //     cerr << "  Input file must have POLAR RADAR data." << endl;
  //     cerr << "  Data projection found: "
  // 	 << Mdvx::projType2Str(fhdr.proj_type) << endl;
  //     return -1;
  //   }
  
  // check for 360 degrees
  
  //   if (fhdr.ny != 360) {
  //     cerr << "ERROR - RdasSim" << endl;
  //     cerr << "  Input file must have 360 degree radials." << endl;
  //     cerr << "  Number of radials found: " << fhdr.ny;
  //     return -1;
  //   }
 
  return 0;

}

//////////////////////////////////////////////////
// serve the client

int RdasSim::_serveClient(Socket &client)
{

  PMU_auto_register("Serving client");
  gettimeofday(&_prevBeamTime, &_timezone);

  while (true) {
    
    // get a command message

    if (_readCommand(client)) {
      cerr << "ERROR - RdasSim::_serveClient" << endl;
      cerr << "  Cannot read incoming command" << endl;
      return -1;
    }
    
    // send a beam
    
    if (_opMode == OP_RUN) {

      if (_sendBeam(client)) {
	cerr << "ERROR - RdasSim::_serveClient" << endl;
	cerr << "  Cannot send beam" << endl;
	return -1;
      }
      
    } else if (_opMode == OP_CALIBRATE) {

      if (_sendCalib(client)) {
	cerr << "ERROR - RdasSim::_serveClient" << endl;
	cerr << "  Cannot send calibration ray" << endl;
	return -1;
      }
      
    }

    // check for user input from keyboard

    if (_readKbdSelect(1) == 1) {
      _readKbd();
    }
    
  }

  return 0;

}

//////////////////////////////////////////////////
// read incoming command

int RdasSim::_readCommand(Socket &client)
{
  
  PMU_auto_register("Reading command");

  if (client.readSelect(1)) {
    if (client.getErrNum() == Socket::TIMED_OUT) {
      PMU_auto_register("Timed out waiting for command");
      return 0;
    } else {
      cerr << "ERROR - RdasSim::_readCommand" << endl;
      cerr << "  on readSelect ..." << endl;
      cerr << client.getErrStr() << endl;
      return -1;
    }
  }

  // look for beginning of message - 0x5B5B5B5B
  
  char c1 = '\0', c2 = '\0', c3 = '\0', c4 = '\0';
  while (true) {
    if (client.readBuffer(&c4, 1, 50)) {
      cerr << "ERROR - RdasSim" << endl;
      cerr << "  on readBuffer for c4 ..." << endl;
      cerr << client.getErrStr() << endl;
      return -1;
    } else {
      if (c4 != 0x5B) {
	cerr << "Got extra byte" << endl;
      }
      if (c1 == 0x5B && c2 == 0x5B && c3 == 0x5B && c4 == 0x5B) {
	// got sequence for start of command
	break;
      }
      c1 = c2;
      c2 = c3;
      c3 = c4;
    }
  }

  // read command and length, swap bytes

  si32 command;
  if (client.readBuffer(&command, sizeof(command), 1)) {
    cerr << "ERROR - RdasSim" << endl;
    cerr << "  on readBuffer for command ..." << endl;
    cerr << client.getErrStr() << endl;
    return -1;
  }
  if (_params.big_endian) {
    BE_to_array_32(&command, sizeof(command));
  }

  si32 len;
  if (client.readBuffer(&len, sizeof(len), 1)) {
    cerr << "ERROR - RdasSim" << endl;
    cerr << "  on readBuffer for len ..." << endl;
    cerr << client.getErrStr() << endl;
    return -1;
  }
  if (_params.big_endian) {
    BE_to_array_32(&len, sizeof(len));
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Got command: " << command << endl;
    cerr << "        len: " << len << endl;
  }

  // read buffer, swap bytes

  TaArray<ui08> buf_;
  ui08 *buf = buf_.alloc(len);
  if (client.readBuffer(buf, len, 1)) {
    cerr << "ERROR - RdasSim" << endl;
    cerr << "  on readBuffer for buf ..." << endl;
    cerr << client.getErrStr() << endl;
    return -1;
  }
  if (_params.big_endian) {
    BE_to_array_32(buf, len);
  }

  // handle the command

  _handleCommand(command, len, buf);

  // send status if op change
  
  if (command == OP_MODE_COMMAND ||
      command == CONTROL_FLAGS_COMMAND ||
      command == CONFIG_COMMAND) {
    if (_sendStatus(client)) {
      cerr << "ERROR - RdasSim::_serveClient" << endl;
      cerr << "  Cannot send status ray" << endl;
      return -1;
    }
  }
  
  return 0;

}

//////////////////////////////////////////////////
// send a beam

int RdasSim::_sendBeam(Socket &client)

{

  struct timeval beamTime;
  gettimeofday(&beamTime, &_timezone);
  int diff =
    (beamTime.tv_sec - _prevBeamTime.tv_sec) * 1000000 +
    beamTime.tv_usec - _prevBeamTime.tv_usec;
  int delay = _params.beam_wait_msecs * 1000;
  while (diff < delay) {
    umsleep(1);
    gettimeofday(&beamTime, &_timezone);
    diff =
      (beamTime.tv_sec - _prevBeamTime.tv_sec) * 1000000 +
      beamTime.tv_usec - _prevBeamTime.tv_usec;
  }
  _prevBeamTime = beamTime;

  // adjust the antenna position

  double secsPassed = (double) diff / 1000000.0;
  _adjPosn(secsPassed);

  // set up the beam

  RdasBeam beam;
  beam.setTime(beamTime.tv_sec);
  if (_mainPower) {
    beam.setMainPower(true);
  }
  if (_systemReady) {
    beam.setSystemReady(true);
  }
  if (_servoPower) {
    beam.setServoPower(true);
  }
  if (_radiate) {
    beam.setRadiate(true);
  }
  beam.setStatusFlags(_statusFlags);
  beam.setErrorFlags(_errorFlags);
  beam.setEl(_actualEl);
  beam.setAz(_actualAz);
  beam.setGateSpacing(_gateSpacing);
  beam.setStartRange(_startRange);
  beam.setPulseWidth(2.0);
  beam.setPrf(_prf);
  beam.setBeamCount(0);
  beam.setTiltCount(0);
  beam.setEndOfTiltFlag(_endOfTiltFlag);
  beam.setEndOfVolFlag(_endOfVolFlag);
  _endOfTiltFlag = false;
  _endOfVolFlag = false;

  int az = (int) _actualAz;
  if (az % 720 == 0) {
    sprintf(_statusStr, "time, el, az: %s, %g, %g",
 	    DateTime::strn(beamTime.tv_sec).c_str(), _actualEl, _actualAz);
    if (_params.debug) {
      cerr << "StatusStr: " << _statusStr << endl;
    }
  }
  beam.setStatusString(_statusStr);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "time, el, az: " << DateTime::strn(beamTime.tv_sec) << ", "
	 << _actualEl << ", " << _actualAz << endl;
  }

  // get the radar file headers

  MdvxField *dbzFld = _mdvx.getFieldByName(_params.dbz_field_name);
  const Mdvx::field_header_t &fhdr = dbzFld->getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr = dbzFld->getVlevelHeader();

  // find closest elevation

  double minDiff = 180.0;
  int elIndex = 0;
  for (int ii = 0; ii < fhdr.nz; ii++) {
    double diff = fabs(_actualEl - vhdr.level[ii]);
    if (diff < minDiff) {
      elIndex = ii;
      minDiff = diff;
    }
  }

  // compute the index into the dbz array

  int beamIndex = elIndex * fhdr.ny + (int) _actualAz;
  int gateIndex = beamIndex * fhdr.nx;
  int maxIndex = fhdr.nz * fhdr.ny * fhdr.nx - 10 * _nGates - 1;
  if (gateIndex > maxIndex) {
    gateIndex = maxIndex;
  }
  fl32 *dbz = (fl32*) dbzFld->getVol();
  dbz += gateIndex;

  // compute counts

  double atmosAtten = 0.014;
  double atmosAttenNm = atmosAtten * 1.85196;
  double startRange = 0.125;
  double gateSpacing = 0.250;
  
  double sumCount = 0.0;
  TaArray<si16> counts_;
  si16 *counts = counts_.alloc(_nGates);
  for (int i = 0; i < _nGates; i++) {
    double range = startRange + i * gateSpacing;
    double rangeNm = range / 1.85196;
    double log10Range = log10(rangeNm);
    double rangeCorrection =
      20.0 * log10Range + rangeNm * atmosAttenNm;
    double count =
      _params.calib_offset_1km + (dbz[i] - rangeCorrection) * _params.calib_slope;
    if (count < 0) {
      count = 0;
    }
    if (count > 16383) {
      count = 16383;
    }
    if (count < 200) {
      count = 200;
    }
    double drand = (double) rand() / RAND_MAX;
    count += (int) ((drand -0.5) * 30.0);
    counts[i] = (si16) count;
    sumCount += counts[i];
  } // i
  beam.setCounts(counts, _nGates);

  // assemble the beam
  
  beam.assemble(_params.big_endian);
  
  // check if we can write a beam message
  
  if (client.writeSelect(1)) {
    if (client.getErrNum() == Socket::TIMED_OUT) {
      PMU_auto_register("Timed out waiting for client");
    } else {
      cerr << "ERROR - RdasSim" << endl;
      cerr << "  on writeSelect ..." << endl;
      cerr << client.getErrStr() << endl;
      return -1;
    }
  } else {
    // write beam message
    if (client.writeBuffer(beam.getBufPtr(), beam.getBufNbytes())) {
      cerr << "ERROR - RdasSim" << endl;
      cerr << "  on writeBuffer ..." << endl;
      cerr << "  Cannot write beam" << endl;
      return -1;
    }
  }

  return 0;

}
    
//////////////////////////////////////////////////
// send a calibration ray

int RdasSim::_sendCalib(Socket &client)

{
  
  struct timeval beamTime;
  gettimeofday(&beamTime, &_timezone);
  int diff =
    (beamTime.tv_sec - _prevBeamTime.tv_sec) * 1000000 +
    beamTime.tv_usec - _prevBeamTime.tv_usec;
  int delay = _params.beam_wait_msecs * 1000;
  while (diff < delay) {
    umsleep(1);
    gettimeofday(&beamTime, &_timezone);
    diff =
      (beamTime.tv_sec - _prevBeamTime.tv_sec) * 1000000 +
      beamTime.tv_usec - _prevBeamTime.tv_usec;
  }
  _prevBeamTime = beamTime;

  // adjust the antenna position
  
  double secsPassed = (double) diff / 1000000.0;
  _adjPosn(secsPassed);

  // set up the beam
  
  RdasBeam beam;
  beam.setTime(beamTime.tv_sec);
  if (_mainPower) {
    beam.setMainPower(true);
  }
  if (_systemReady) {
    beam.setSystemReady(true);
  }
  if (_servoPower) {
    beam.setServoPower(true);
  }
  if (_radiate) {
    beam.setRadiate(true);
  }
  beam.setStatusFlags(_statusFlags);
  beam.setErrorFlags(_errorFlags);
  beam.setEl(_actualEl);
  beam.setAz(_actualAz);
  beam.setGateSpacing(_gateSpacing);
  beam.setStartRange(_startRange);
  beam.setPulseWidth(2.0);
  beam.setPrf(_prf);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "el, az: " << _actualEl << ", " << _actualAz << endl;
  }

  // compute counts

  double sumCount = 0.0;
  TaArray<si16> counts_;
  si16 *counts = counts_.alloc(_nGates);
  for (int i = 0; i < _nGates; i++) {
    double count = 0;
    if (i >= _pulseGateStart && i <= _pulseGateEnd) {
      count += _pulseHeightCount;
    }
    if (count > 32000) {
      count = 32000;
    }
    if (count < 200) {
      count = 200;
    }
    double drand = (double) rand() / RAND_MAX;
    count += (int) ((drand - 0.5) * 30.0);
    counts[i] = (si16) count;
    sumCount += counts[i];
  } // i
  beam.setCounts(counts, _nGates);

  // assemble the beam

  beam.assemble(_params.big_endian);

  // check if we can write a beam message
  
  if (client.writeSelect(1)) {
    if (client.getErrNum() == Socket::TIMED_OUT) {
      PMU_auto_register("Timed out waiting for client");
    } else {
      cerr << "ERROR - RdasSim" << endl;
      cerr << "  on writeSelect ..." << endl;
      cerr << client.getErrStr() << endl;
      return -1;
    }
  } else {
    // write beam message
    if (client.writeBuffer(beam.getBufPtr(), beam.getBufNbytes())) {
      cerr << "ERROR - RdasSim" << endl;
      cerr << "  on writeBuffer ..." << endl;
      cerr << "  Cannot write beam" << endl;
      return -1;
    }
  }

  return 0;

}
    
//////////////////////////////////////////////////
// send a status ray

int RdasSim::_sendStatus(Socket &client)

{
  
  struct timeval beamTime;
  gettimeofday(&beamTime, &_timezone);
  int diff =
    (beamTime.tv_sec - _prevBeamTime.tv_sec) * 1000000 +
    beamTime.tv_usec - _prevBeamTime.tv_usec;
  int delay = _params.beam_wait_msecs * 1000;
  while (diff < delay) {
    umsleep(1);
    gettimeofday(&beamTime, &_timezone);
    diff =
      (beamTime.tv_sec - _prevBeamTime.tv_sec) * 1000000 +
      beamTime.tv_usec - _prevBeamTime.tv_usec;
  }
  _prevBeamTime = beamTime;

  // set up the beam
  
  RdasBeam beam;
  beam.setTime(beamTime.tv_sec);
  if (_mainPower) {
    beam.setMainPower(true);
  }
  if (_systemReady) {
    beam.setSystemReady(true);
  }
  if (_servoPower) {
    beam.setServoPower(true);
  }
  if (_radiate) {
    beam.setRadiate(true);
  }
  beam.setStatusFlags(_statusFlags);
  beam.setErrorFlags(_errorFlags);
  beam.setEl(_actualEl);
  beam.setAz(_actualAz);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "el, az: " << _actualEl << ", " << _actualAz << endl;
  }
  
  // set counts to 0

  TaArray<si16> counts_;
  si16 *counts = counts_.alloc(_nGates);
  memset(counts, 0, _nGates * sizeof(si16));
  beam.setCounts(counts, _nGates);

  // assemble the beam

  beam.assemble(_params.big_endian);
  
  // check if we can write a beam message
  
  if (client.writeSelect(1)) {
    if (client.getErrNum() == Socket::TIMED_OUT) {
      PMU_auto_register("Timed out waiting for client");
    } else {
      cerr << "ERROR - RdasSim" << endl;
      cerr << "  on writeSelect ..." << endl;
      cerr << client.getErrStr() << endl;
      return -1;
    }
  } else {
    // write beam message
    if (client.writeBuffer(beam.getBufPtr(), beam.getBufNbytes())) {
      cerr << "ERROR - RdasSim" << endl;
      cerr << "  on writeBuffer ..." << endl;
      cerr << "  Cannot write beam" << endl;
      return -1;
    }
  }

  return 0;

}
    
//////////////////////////////////////////////////
// handle a command

void RdasSim::_handleCommand(si32 command, si32 len, ui08 *buf)
  
{

  if (_params.debug) {
    cerr << "----> command, len: " << _command2Str((command_t) command)
	 << ", " << len << endl;
  }
  
  ////////////////////////////////////////
  // first set the values from the buffer
  
  si32 iValue;
  si32 state;
  fl32 fValue;
  int fArrayLen = len / sizeof(fl32);
  TaArray<fl32> fArray_;
  fl32 *fArray = fArray_.alloc(fArrayLen);
  config_t config;
  si32 dacChannel;
  si32 dacValue;
  
  switch (command) {
	    
    // iValue commands
	    
  case OP_MODE_COMMAND:
  case SCAN_MODE_COMMAND:
  case N_GATES_COMMAND:
  case CONTROL_FLAGS_COMMAND:
  case DATA_DECIMATE_COMMAND:
    memcpy(&iValue, buf, len);
    break;

    // state commands
    
  case MAIN_POWER_COMMAND:
  case SERVO_POWER_COMMAND:
  case RADIATE_COMMAND:
    memcpy(&state, buf, len);
    break;

    // fValue commands
    
  case START_RANGE_COMMAND:
  case GATE_SPACING_COMMAND:
  case PRF_COMMAND:
  case AZ_COMMAND:
  case EL_COMMAND:
  case AZ_RATE_COMMAND:
  case EL_RATE_COMMAND:
  case MAX_AZ_RATE_COMMAND:
  case MAX_EL_RATE_COMMAND:
    memcpy(&fValue, buf, len);
    break;

    // fArray commands

  case EL_ARRAY_COMMAND:
    memcpy(fArray, buf, len);
    break;

    // config command
    
  case CONFIG_COMMAND:
    memcpy(&config, buf, len);
    break;

    // DAC command
    
  case DAC_COMMAND:
    {
      si32 *ibuf = (si32 *) buf;
      memcpy(&dacChannel, ibuf, sizeof(si32));
      memcpy(&dacValue, ibuf + 1, sizeof(si32));
    }
    break;
    

  } // switch (command)

  /////////////////////////////////////////////////
  // now act on each command
  
  switch (command) {
	    
  case OP_MODE_COMMAND:
    _opMode = (op_mode_t) iValue;
    if (_params.debug) {
      cerr << "  Setting opMode: " << _opMode2Str(_opMode) << endl;
    }
    return;
    
  case SCAN_MODE_COMMAND:
    _scanMode = (scan_mode_t) iValue;
    switch (_scanMode) {
    case SCAN_AUTO_VOL:
      _startVol = true;
      break;
    case SCAN_AUTO_PPI:
    case SCAN_STOP:
      _requestedEl = _actualEl;
      _requestedAz = _actualAz;
    default: {}
    }
    if (_params.debug) {
      cerr << "  Setting scanMode: " << _scanMode2Str(_scanMode) << endl;
    }
    return;
    
  case MAIN_POWER_COMMAND:
    if (_params.debug) {
      cerr << "  Setting main power: " << (bool) state << endl;
    }
    if (state) {
      _mainPower = true;
      _systemReady = true;
    } else {
      _mainPower = false;
      _systemReady = false;
    }
    return;

  case SERVO_POWER_COMMAND:
    if (_params.debug) {
      cerr << "  Setting servo power: " << (bool) state << endl;
    }
    _servoPower = state;
    return;

  case RADIATE_COMMAND:
    if (_params.debug) {
      cerr << "  Setting radiate: " << (bool) state << endl;
    }
    _radiate = state;
    return;
    
  case N_GATES_COMMAND:
    _nGates = iValue;
    if (_params.debug) {
      cerr << "  Setting nGates: " << _nGates << endl;
    }
    return;
	    
  case CONTROL_FLAGS_COMMAND:
    _statusFlags = iValue;
    if (_params.debug) {
      cerr << "  Setting status flags: ";
      for (int i = RDAS_BEAM_NSTATUS - 1; i >= 0; i--) {
	int one = 1;
	if (_statusFlags & (one << i)) {
	  cerr << " 1";
	} else {
	  cerr << " 0";
	}
      }
      cerr << endl;
    }
    return;
	    
  case START_RANGE_COMMAND:
    _startRange = fValue;
    if (_params.debug) {
      cerr << "  Setting startRange: " << _startRange << endl;
    }
    return;
	    
  case GATE_SPACING_COMMAND:
    _gateSpacing = fValue;
    if (_params.debug) {
      cerr << "  Setting gateSpacing: " << _gateSpacing << endl;
    }
    return;
	    
  case PRF_COMMAND:
    _prf = fValue;
    if (_params.debug) {
      cerr << "  Setting prf: " << _prf << endl;
    }
    return;
	    
  case AZ_COMMAND:
    _requestedAz = fValue;
    if (_params.debug) {
      cerr << "  Setting requestedAz: " << _requestedAz << endl;
    }
    return;
    
  case EL_COMMAND:
    _requestedEl = fValue;
    if (_params.debug) {
      cerr << "  Setting requestedEl: " << _requestedEl << endl;
    }
    return;
	    
  case AZ_RATE_COMMAND:
    _azSlewRate = fValue;
    if (_params.debug) {
      cerr << "  Setting azSlewRate: " << _azSlewRate << endl;
    }
    return;
    
  case EL_RATE_COMMAND:
    _elSlewRate = fValue;
    if (_params.debug) {
      cerr << "  Setting elSlewRate: " << _elSlewRate << endl;
    }
    return;
    
  case EL_ARRAY_COMMAND:
    _elList.clear();
    for (int i = 0; i < fArrayLen; i++) {
      _elList.push_back((double) fArray[i]);
    }
    if (_params.debug) {
      cerr << "  Setting elevation array" << endl;
      for (size_t ii = 0; ii < _elList.size(); ii++) {
	cerr << "    el[" << ii << "]: " << _elList[ii] << endl;
      }
    }
    return;
    
  case CONFIG_COMMAND:

    _version = config.version;
    _struct_len = config.struct_len;
    _nGates = config.ngates;
    _samplesPerAz = config.samples_per_az;
    _samplesPerGate = config.samples_per_gate;
    _polarizationCode = (polarization_t) config.polarization_code;
    _elVoltagePositive = config.el_voltage_positive;
    _azVoltagePositive = config.az_voltage_positive;
    _statusFlags = config.control_flags;
    _dac0 = config.dac0;
    _dac1 = config.dac1;
    _autoResetTimeout = config.auto_reset_timeout;
    _shutdownOpMode = config.shutdown_op_mode;
    
    _startRange = config.start_range;
    _gateSpacing = config.gate_spacing;
    _prf = config.prf;
    _pulseWidth = config.pulse_width;
    _antennaElCorr = config.antenna_el_corr;
    _antennaAzCorr = config.antenna_az_corr;
    _controlElCorr = config.control_el_corr;
    _controlAzCorr = config.control_az_corr;
    _antennaMinEl = config.antenna_min_el;
    _antennaMaxEl = config.antenna_max_el;
    _antennaElSlewRate = config.antenna_el_slew_rate;
    _antennaAzSlewRate = config.antenna_az_slew_rate;
    _antennaMaxElSlewRate = config.antenna_max_el_slew_rate;
    _antennaMaxAzSlewRate = config.antenna_max_az_slew_rate;
    _calibSlope = config.calib_slope;
    _calibOffset = config.calib_offset;
    _elevTolerance = config.elev_tolerance;
    _ppiAzOverlap = config.ppi_az_overlap;
    
    if (_params.debug) {
      cerr << "  Setting configuration values" << endl;
      cerr << "    version: " << _version << endl;
      cerr << "    struct_len: " << _struct_len << endl;
      cerr << "    sizeof(config_t): " << sizeof(config_t) << endl;
      cerr << "    nGates: " << _nGates << endl;
      cerr << "    samplesPerAz: " << _samplesPerAz << endl;
      cerr << "    samplesPerGate: " << _samplesPerGate << endl;
      cerr << "    polarizationCode: "
	   << _polarization2Str(_polarizationCode) << endl;
      cerr << "    elVoltagePositive: " << _elVoltagePositive << endl;
      cerr << "    azVoltagePositive: " << _azVoltagePositive << endl;
      cerr << "    statusFlags: " << _flags2Str(_statusFlags) << endl;
      cerr << "    errorFlags: " << _flags2Str(_errorFlags) << endl;
      cerr << "    dac0: " << _dac0 << endl;
      cerr << "    dac1: " << _dac1 << endl;
      cerr << "    autoResetTimeout: " << _autoResetTimeout << endl;
      if (_shutdownOpMode == SHUTDOWN_MODE_OFF) {
        cerr << "    shutdown mode: OFF" << endl;
      } else {
        cerr << "    shutdown mode: STANDBY" << endl;
      }
      cerr << "    startRange: " << _startRange << endl;
      cerr << "    gateSpacing: " << _gateSpacing << endl;
      cerr << "    prf: " << _prf << endl;
      cerr << "    pulseWidth: " << _pulseWidth << endl;
      cerr << "    antennaElCorr: " << _antennaElCorr << endl;
      cerr << "    antennaAzCorr: " << _antennaAzCorr << endl;
      cerr << "    controlElCorr: " << _controlElCorr << endl;
      cerr << "    controlAzCorr: " << _controlAzCorr << endl;
      cerr << "    antennaMinEl: " << _antennaMinEl << endl;
      cerr << "    antennaMaxEl: " << _antennaMaxEl << endl;
      cerr << "    antennaElSlewRate: " << _antennaElSlewRate << endl;
      cerr << "    antennaAzSlewRate: " << _antennaAzSlewRate << endl;
      cerr << "    antennaMaxElSlewRate: " << _antennaMaxElSlewRate << endl;
      cerr << "    antennaMaxAzSlewRate: " << _antennaMaxAzSlewRate << endl;
      cerr << "    calibSlope: " << _calibSlope << endl;
      cerr << "    calibOffset: " << _calibOffset << endl;
      cerr << "    elevTolerance: " << _elevTolerance << endl;
      cerr << "    ppiAzOverlap: " << _ppiAzOverlap << endl;
    }
    return;
    
  case DAC_COMMAND:
    if (_params.debug) {
      cerr << "  Setting DAC channel, value: "
	   << dacChannel << ", " << dacValue << endl;
    }
    if (dacChannel == 0) {
      _dac0 = dacValue;
    } else {
      _dac1 = dacValue;
    }
    return;
    
  case DATA_DECIMATE_COMMAND:
    if (_params.debug) {
      cerr << "  Received DATA_DECIMATE command" << endl;
    }
    return;
    
  default:
    return;
    
  } // switch (command)

}

////////////////////////////////////////
// adjust position of antenna

void RdasSim::_adjPosn(double adjSecs) {

  if (!_servoPower) {
    return;
  }
  
  if (_scanMode == SCAN_STOP) {
    return;
  }
  
  if (_scanMode == SCAN_MANUAL || _scanMode == SCAN_TRACK_SUN) {
    _adjustElAz(_requestedEl, _requestedAz, adjSecs);
    return;
  }
  
  if (_scanMode == SCAN_AUTO_PPI) {
    _adjustElSlewAz(_requestedEl, adjSecs);
    // _slewAz(adjSecs);
    return;
  }
  
  if (_scanMode == SCAN_AUTO_VOL) {

    // at start, move to starting location
    
    if (_startVol) {
      _volElevIndex = 0;
      double el = _elList[_volElevIndex];
      double az = 0.0;
      if (_adjustElAz(el, az, adjSecs)) {
	if (_params.debug) {
	  cerr << "------> Starting new vol: ";
	  cerr << "el, az: " << _actualEl << ", " << _actualAz << endl;
	}
	_endOfVolFlag = true;
	_startVol = false;
	_sumAzPpi = 0.0;
      }
      return;
    }
    
    // if done with one ppi, move to next elevation
    
    if (_sumAzPpi >= 360.0) {
      if (_params.debug && !_endOfPpi) {
	cerr << "--------> End of current ppi: ";
	cerr << "el, az: " << _actualEl << ", " << _actualAz << endl;
      }
      _endOfPpi = true;
      if (_volElevIndex > (int) _elList.size() - 2) {
	// done with this volume
	_slewAz(adjSecs);
	_startVol = true;
	return;
      }
      // compute desired position
      double el = _elList[_volElevIndex + 1];
      // go there, slewing in azimuth
      if (_adjustElSlewAz(el, adjSecs)) {
	_sumAzPpi = 0.0;
	_volElevIndex++;
	_endOfPpi = false;
	_endOfTiltFlag = true;
	if (_params.debug) {
	  cerr << "--------> Starting new ppi: ";
	  cerr << "el, az: " << _actualEl << ", " << _actualAz << endl;
	}
      }
      return;
    }
    
    // move in azimuth, keeping track of how much we slewed
    
    double deltaAz = _slewAz(adjSecs);
    _sumAzPpi += fabs(deltaAz);
    
    return;
    
  }

} // adjPosn

/////////////////////////////////////
// adjust towards requested El and Az

bool RdasSim::_adjustElAz(double el, double az, double adjSecs)
  
{
  
  bool done = true;
  
  // adjust elevation
  
  double errorEl = fabs(el - _actualEl);
  if (errorEl < 0.0001) {
    _actualEl = el;
  } else {
    done = false;
    double slewRate = _elSlewRate;
    if (errorEl < 5.0) {
      slewRate *= (errorEl / 5.0);
    }
    if (slewRate < 0.25) {
      slewRate = 0.25;
    }
    double deltaEl = slewRate * adjSecs;
    if (deltaEl > errorEl) {
      deltaEl = errorEl;
    }
    if (el < _actualEl) {
      deltaEl *= -1.0;
    }
    _actualEl += deltaEl;
  }
  
  // adjust azimuth
  
  double errorAz = fabs(az - _actualAz);
  double sign = 1.0;
  if (errorAz > 180.0) {
    errorAz = 360.0 - errorAz;
    sign = -1.0;
  }
  if (errorAz < 0.0001) {
    _actualAz = az;
  } else {
    done = false;
    double slewRate = _azSlewRate;
    if (errorAz < 5.0) {
      slewRate *= (errorAz / 5.0);
    }
    if (slewRate < 0.25) {
      slewRate = 0.25;
    }
    double deltaAz = slewRate * adjSecs;
    if (deltaAz > errorAz) {
      deltaAz = errorAz;
    }
    if (az < _actualAz) {
      deltaAz *= -1.0;
    }
    deltaAz *= sign;
    _actualAz += deltaAz;
    if (_actualAz < 0) {
      _actualAz += 360.0;
    }
    if (_actualAz >= 360) {
      _actualAz -= 360.0;
    }
    if (fabs(_actualAz - 0) < 0.00001 || fabs(_actualAz - 360) < 0.00001) {
      _actualAz = 0.0;
    }
    
  }
  
  return done;
  
} // adjustElAz()

///////////////////////////////////////////////
// adjust towards requested El, slew in azimuth

bool RdasSim::_adjustElSlewAz(double el, double adjSecs)

{
  
  bool done = true;
  
  // adjust elevation
  
  double errorEl = fabs(el - _actualEl);
  if (errorEl < 0.0001) {
    _actualEl = el;
  } else {
    done = false;
    double slewRate = _elSlewRate;
    if (errorEl < 5.0) {
      slewRate *= (errorEl / 5.0);
    }
    if (slewRate < 0.25) {
      slewRate = 0.25;
    }
    double deltaEl = slewRate * adjSecs;
    if (deltaEl > errorEl) {
      deltaEl = errorEl;
    }
    if (el < _actualEl) {
      deltaEl *= -1.0;
    }
    _actualEl += deltaEl;
  }
  
  // slew azimuth
  
  _slewAz(adjSecs);
  
  return done;
  
} // adjustElSlewAz()

////////////////////////////////////////////
// slew in azimuth - return how much slewed

double RdasSim::_slewAz(double adjSecs) {
  
  // slew azimuth
  
  double deltaAz = _azSlewRate * adjSecs;
  _actualAz += deltaAz;
  if (_actualAz < 0) {
    _actualAz += 360.0;
  }
  if (_actualAz >= 360) {
    _actualAz -= 360.0;
  }
  
  return deltaAz;
  
} // slewAz	

//////////////////////////////////////////////////
// read keyboard input

void RdasSim::_readKbd()
{

  char cc;
  if (_readKbdChar(cc)) {
    return;
  }

  if (cc == '') {
    _inEscapeSeq = true;
    return;
  }

  if (_inEscapeSeq && cc == '[') {
    _inKeySeq = true;
    return;
  }
  
  if (_inEscapeSeq && cc == 'a') {
    // toggle servo power
    _servoPower = !_servoPower;
    _inEscapeSeq = false;
    _inKeySeq = false;
    _kbdCmd = "";
    return;
  }
  
  if (_inEscapeSeq && cc == 'b') {
    // toggle radiate power
    _radiate = !_radiate;
    _inEscapeSeq = false;
    _inKeySeq = false;
    _kbdCmd = "";
    return;
  }
  
  if (_inKeySeq && cc == 'A') {
    _pulseHeightCount += _params.pulse_height_delta;
    _inEscapeSeq = false;
    _inKeySeq = false;
    _kbdCmd = "";
    return;
  }
      
  if (_inKeySeq && cc == 'B') {
    _pulseHeightCount -= _params.pulse_height_delta;
    _inEscapeSeq = false;
    _inKeySeq = false;
    _kbdCmd = "";
    return;
  }
      
  if (_inKeySeq && cc == 'C') {
    _pulseGateStart += 10;
    _pulseGateEnd += 10;
    _inEscapeSeq = false;
    _inKeySeq = false;
    _kbdCmd = "";
    return;
  }
      
  if (_inKeySeq && cc == 'D') {
    _pulseGateStart -= 10;
    _pulseGateEnd -= 10;
    _inEscapeSeq = false;
    _inKeySeq = false;
    _kbdCmd = "";
    return;
  }
      
  if (_inKeySeq && cc == 'E') {
    _servoPower = !_servoPower;
    _inEscapeSeq = false;
    _inKeySeq = false;
    _kbdCmd = "";
    return;
  }
      
  if (_inKeySeq) {
    cerr << "key val: " << cc << endl;
    _inEscapeSeq = false;
    _inKeySeq = false;
    _kbdCmd = "";
    return;
  }
      
  if (cc == '\n' && _kbdCmd.size() > 0) {
    if (_kbdCmd == "wider") {
      _pulseGateStart -= 10;
      _pulseGateEnd += 10;
    } else {
      int count;
      if (sscanf(_kbdCmd.c_str(), "%d", &count) == 1) {
	_pulseHeightCount = count;
      }
    }
    _kbdCmd = "";
    return;
  }

  _kbdCmd += cc;
  return;
  
}

/////////////////////////////////////////////////////////////////
//
// readChar()
//
// reads a character
//
// returns 0 on success, -1 on failure
//

int RdasSim::_readKbdChar(char &cc)

{

  if (read(_kbdFd, &cc, 1) != 1) {
    return (-1);
  } else {
    return (0);
  }

}

///////////////////////////////////////////////////////
// readSelect - waits for read access on fd
//
// returns 1 on success, -1 on timeout, -2 on failure
//
// Blocks if wait_msecs == -1

int RdasSim::_readKbdSelect(long wait_msecs)

{
  
  int ret, maxfdp1;
  fd_set read_fd;
  
  struct timeval wait;
  struct timeval * waitp;
  
  waitp = &wait;
  
  // check only on _fd file descriptor

  FD_ZERO(&read_fd);
  FD_SET(_kbdFd, &read_fd);
  maxfdp1 = _kbdFd + 1;
  
 again:

  /*
   * set timeval structure
   */
  
  if (-1 == wait_msecs) {
    waitp = NULL;
  } else {
    wait.tv_sec = wait_msecs / 1000;
    wait_msecs -= wait.tv_sec * 1000;
    wait.tv_usec = wait_msecs * 1000;
  }
  
  errno = 0;
  if (0 > (ret = select(maxfdp1, &read_fd, NULL, NULL, waitp))) {
      if (errno == EINTR) /* system call was interrupted */
	goto again;
      fprintf(stderr,"Read select failed on _kbdFd %d; error = %d\n",
	      _kbdFd, errno);
      return -2; /* select failed */
    } 
  
  if (ret == 0) {
    return (-1); /* timeout */
  }
  
  return (1);

}

///////////////////////////
// get string represtations

string RdasSim::_opMode2Str(op_mode_t mode)

{
  
  switch(mode) {
  case OP_OFF:
    return "OFF";
  case OP_STANDBY:
    return "STANDBY";
  case OP_CALIBRATE:
    return "CALIBRATE";
  case OP_RUN:
    return "RUN";
  }

  return "Unknown";

}

string RdasSim::_scanMode2Str(scan_mode_t mode)

{
  
  switch (mode) {
  case SCAN_STOP:
    return "STOP";
  case SCAN_MANUAL:
    return "MANUAL";
  case SCAN_AUTO_VOL:
    return "AUTO_VOL";
  case SCAN_AUTO_PPI:
    return "AUTO_PPI";
  case SCAN_TRACK_SUN:
    return "TRACK_SUN";
  }

  return "Unknown";

}

string RdasSim::_command2Str(command_t command)

{
  
  switch (command) {
  case OP_MODE_COMMAND:
    return "OP_MODE_COMMAND";
  case SCAN_MODE_COMMAND:
    return "SCAN_MODE_COMMAND";
  case MAIN_POWER_COMMAND:
    return "MAIN_POWER_COMMAND";
  case MAG_POWER_COMMAND:
    return "MAG_POWER_COMMAND";
  case SERVO_POWER_COMMAND:
    return "SERVO_POWER_COMMAND";
  case RADIATE_COMMAND:
    return "RADIATE_COMMAND";
  case CONTROL_FLAGS_COMMAND:
    return "CONTROL_FLAGS_COMMAND";
  case N_GATES_COMMAND:
    return "N_GATES_COMMAND";
  case START_RANGE_COMMAND:
    return "START_RANGE_COMMAND";
  case GATE_SPACING_COMMAND:
    return "GATE_SPACING_COMMAND";
  case PRF_COMMAND:
    return "PRF_COMMAND";
  case AZ_COMMAND:
    return "AZ_COMMAND";
  case EL_COMMAND:
    return "EL_COMMAND";
  case EL_ARRAY_COMMAND:
    return "EL_ARRAY_COMMAND";
  case AZ_RATE_COMMAND:
    return "AZ_RATE_COMMAND";
  case EL_RATE_COMMAND:
    return "EL_RATE_COMMAND";
  case MAX_AZ_RATE_COMMAND:
    return "MAX_AZ_RATE_COMMAND";
  case MAX_EL_RATE_COMMAND:
    return "MAX_EL_RATE_COMMAND";
  case CONFIG_COMMAND:
    return "CONFIG_COMMAND";
  case DAC_COMMAND:
    return "DAC_COMMAND";
  case DATA_DECIMATE_COMMAND:
    return "DATA_DECIMATE_COMMAND";
  }

  return "Unknown";

}

string RdasSim::_polarization2Str(polarization_t polarization)

{
  
  switch (polarization) {
  case POLARIZATION_HORIZONTAL:
    return "POLARIZATION_HORIZONTAL";
  case POLARIZATION_VERTICAL:
    return "POLARIZATION_VERTICAL";
  case POLARIZATION_CIRCULAR:
    return "POLARIZATION_CIRCULAR";
  }

  return "Unknown";

}

string RdasSim::_flags2Str(int flags)

{
  
  string str;

  for (int i = RDAS_BEAM_NSTATUS - 1; i >= 0; i--) {
    int one = 1;
    if (flags & (one << i)) {
      str += " 1";
    } else {
      str += " 0";
    }
  }

  return str;

}

//////////////////////////////////////////////////
// serve old BPRP client

int RdasSim::_serveBprpClient(Socket &client)
{

  PMU_auto_register("Serving BPRP client");
  gettimeofday(&_prevBeamTime, &_timezone);

  // set up BPRP simulator

  _setupBprpSim();
  
  while (true) {
    
    // send a beam
    
    if (_sendBprpBeam(client)) {
      cerr << "ERROR - RdasSim::_serveClient" << endl;
      cerr << "  Cannot send beam" << endl;
      return -1;
    }
      
    // check for user input from keyboard
    
    if (_readKbdSelect(1) == 1) {
      _readKbd();
    }
    
  }

  return 0;

}

//////////////////////////////////////////////////
// set up BPRP client

void RdasSim::_setupBprpSim()
{

  PMU_auto_register("Setting up BPRP simulator");

  _opMode = OP_RUN;
  _scanMode = SCAN_AUTO_VOL;
  _startVol = true;
  _requestedEl = 0.5;
  _requestedAz = 360.0;
  _mainPower = true;
  _systemReady = true;
  _servoPower = true;
  _radiate = true;
  _nGates = 224;
  _statusFlags = 0;
  _errorFlags = 0;
  _startRange = 0.3;
  _gateSpacing = 0.6;
  _prf = 250.0;
  _azSlewRate = 20.0;
  _elSlewRate = 10.0;

  _elList.clear();
  _elList.push_back(0.5);
  _elList.push_back(1.0);
  _elList.push_back(2.0);
  _elList.push_back(3.0);
  _elList.push_back(4.0);
  _elList.push_back(6.0);
  _elList.push_back(8.0);
  _elList.push_back(10.0);
  _elList.push_back(13.0);
  _elList.push_back(17.0);
  _elList.push_back(21.0);
  _elList.push_back(25.0);
  
  _version = 0;
  _samplesPerAz = 8;
  _samplesPerGate = 4;
  _polarizationCode = POLARIZATION_HORIZONTAL;
  _elVoltagePositive = true;
  _azVoltagePositive = true;
  _dac0 = 0;
  _dac1 = 0;
  _autoResetTimeout = 10;
    
  _pulseWidth = 2;
  _antennaElCorr = 0;
  _antennaAzCorr = 0;
  _controlElCorr = 0;
  _controlAzCorr = 0;
  _antennaMinEl = 0;
  _antennaMaxEl = 90.0;
  _antennaElSlewRate = 20.0;
  _antennaAzSlewRate = 10.0;
  _antennaMaxElSlewRate = 30.0;
  _antennaMaxAzSlewRate = 30.0;
  _calibSlope = 15;
  _calibOffset = 500;
  _elevTolerance = 0.2;
  _ppiAzOverlap = 10.0;
  
  if (_params.debug) {
    cerr << "  Setting configuration values" << endl;
    cerr << "    version: " << _version << endl;
    cerr << "    struct_len: " << _struct_len << endl;
    cerr << "    sizeof(config_t): " << sizeof(config_t) << endl;
    cerr << "    nGates: " << _nGates << endl;
    cerr << "    samplesPerAz: " << _samplesPerAz << endl;
    cerr << "    samplesPerGate: " << _samplesPerGate << endl;
    cerr << "    polarizationCode: "
	 << _polarization2Str(_polarizationCode) << endl;
    cerr << "    elVoltagePositive: " << _elVoltagePositive << endl;
    cerr << "    azVoltagePositive: " << _azVoltagePositive << endl;
    cerr << "    statusFlags: " << _flags2Str(_statusFlags) << endl;
    cerr << "    errorFlags: " << _flags2Str(_errorFlags) << endl;
    cerr << "    dac0: " << _dac0 << endl;
    cerr << "    dac1: " << _dac1 << endl;
    cerr << "    autoResetTimeout: " << _autoResetTimeout << endl;
    cerr << "    startRange: " << _startRange << endl;
    cerr << "    gateSpacing: " << _gateSpacing << endl;
    cerr << "    prf: " << _prf << endl;
    cerr << "    pulseWidth: " << _pulseWidth << endl;
    cerr << "    antennaElCorr: " << _antennaElCorr << endl;
    cerr << "    antennaAzCorr: " << _antennaAzCorr << endl;
    cerr << "    controlElCorr: " << _controlElCorr << endl;
    cerr << "    controlAzCorr: " << _controlAzCorr << endl;
    cerr << "    antennaMinEl: " << _antennaMinEl << endl;
    cerr << "    antennaMaxEl: " << _antennaMaxEl << endl;
    cerr << "    antennaElSlewRate: " << _antennaElSlewRate << endl;
    cerr << "    antennaAzSlewRate: " << _antennaAzSlewRate << endl;
    cerr << "    antennaMaxElSlewRate: " << _antennaMaxElSlewRate << endl;
    cerr << "    antennaMaxAzSlewRate: " << _antennaMaxAzSlewRate << endl;
    cerr << "    calibSlope: " << _calibSlope << endl;
    cerr << "    calibOffset: " << _calibOffset << endl;
    cerr << "    elevTolerance: " << _elevTolerance << endl;
    cerr << "    ppiAzOverlap: " << _ppiAzOverlap << endl;
  }

}

//////////////////////////////////////////////////
// send a BPRP beam

int RdasSim::_sendBprpBeam(Socket &client)

{

  struct timeval beamTime;
  gettimeofday(&beamTime, &_timezone);
  int diff =
    (beamTime.tv_sec - _prevBeamTime.tv_sec) * 1000000 +
    beamTime.tv_usec - _prevBeamTime.tv_usec;
  int delay = _params.beam_wait_msecs * 1000;
  while (diff < delay) {
    umsleep(1);
    gettimeofday(&beamTime, &_timezone);
    diff =
      (beamTime.tv_sec - _prevBeamTime.tv_sec) * 1000000 +
      beamTime.tv_usec - _prevBeamTime.tv_usec;
  }
  _prevBeamTime = beamTime;

  // adjust the antenna position

  double secsPassed = (double) diff / 1000000.0;
  _adjPosn(secsPassed);

  // set up the beam

  bprp_beam_t beam;
  MEM_zero(beam);
  bprp_beam_hdr_t &hdr = beam.hdr;

  // date is based on jear and julian day

  date_time_t dtime;
  dtime.unix_time = beamTime.tv_sec;
  uconvert_from_utime(&dtime);

  date_time_t jtime = dtime;
  jtime.month = 1;
  jtime.day = 1;
  uconvert_to_utime(&jtime);
  int julianDay = (dtime.unix_time - jtime.unix_time) / 86400 + 1;

  hdr.date = (dtime.year - 2000) * 512 + julianDay;
  hdr.hour = dtime.hour;
  hdr.min = dtime.min * 60 + dtime.sec;

  // raycount

  if (_bprpRayCount < 511) {
    _bprpRayCount++;
  }
  if (_endOfTiltFlag) {
    _bprpRayCount = 0;
    _endOfTiltFlag = false;
  }
  if (_endOfVolFlag) {
    hdr.raycount = 0;
    _endOfVolFlag = false;
  } else {
    int tiltNum = _volElevIndex + 1;
    hdr.raycount = (_bprpRayCount + (tiltNum << 9));
  }

  // azimuth and elevation are binary coded decimal
  
  int iaz = (int) (_actualAz * 10.0 + 0.5);
  int azLeft = iaz;
  int azThousands = iaz / 1000;
  azLeft -= (azThousands * 1000);
  int azHundreds = azLeft / 100;
  azLeft -= (azHundreds * 100);
  int azTens = azLeft / 10;
  azLeft -= (azTens * 10);
  int azUnits = azLeft;
  hdr.azimuth =
    (azThousands << 12) + (azHundreds << 8) + (azTens << 4) + azUnits;
  
  int iel = (int) (_actualEl * 10.0 + 0.5);
  int elLeft = iel;
  int elThousands = iel / 1000;
  elLeft -= (elThousands * 1000);
  int elHundreds = elLeft / 100;
  elLeft -= (elHundreds * 100);
  int elTens = elLeft / 10;
  elLeft -= (elTens * 10);
  int elUnits = elLeft;
  hdr.elevation =
    (elThousands << 12) + (elHundreds << 8) + (elTens << 4) + elUnits;

  hdr.mds = _params.bprp_mds;
  hdr.noise = _params.bprp_mus;
  hdr.viphi = _params.bprp_viphi;
  hdr.viplo = _params.bprp_viplo;
  hdr.phi = _params.bprp_phi * 32;
  hdr.plo = _params.bprp_plo * 32;
  hdr.xmt = (int) floor(_params.bprp_xmt * 32.0 + 0.5 + _params.bprp_radar_id);
  hdr.site_blk = 4 * 256; // 600m gates = 4us

  // load counts

  for (int ii = 0; ii < BPRP_GATES_PER_BEAM; ii++) {
    beam.vip[ii] = _params.bprp_test_count * 8;
  }

  // load up response

  bprp_response_t packet;
  MEM_zero(packet);
  packet.length = BE_from_ui32(sizeof(packet));
  packet.magik = BE_from_ui32(RADAR_MAGIK);
  packet.version = BE_from_ui32(RADAR_VERSION);
  memcpy(packet.data, &beam, sizeof(beam));
  BE_from_array_16(packet.data, sizeof(beam));
  
  // check if we can write a beam message
  
  if (client.writeSelect(1)) {
    if (client.getErrNum() == Socket::TIMED_OUT) {
      PMU_auto_register("Timed out waiting for client");
    } else {
      cerr << "ERROR - RdasSim" << endl;
      cerr << "  on writeSelect ..." << endl;
      cerr << client.getErrStr() << endl;
      return -1;
    }
  } else {
    // write beam message
    if (client.writeBuffer(&packet, sizeof(packet))) {
      cerr << "ERROR - RdasSim" << endl;
      cerr << "  on writeBuffer ..." << endl;
      cerr << "  Cannot write beam" << endl;
      return -1;
    } else {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "  --> sent beam to client" << endl;
      }
    }
  }

  return 0;

}
    
