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
// ChillSdbServer.cc
//
// ChillSdbServer for ChillSdbServer
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2007
//
///////////////////////////////////////////////////////////////
//
// The ChillSdbServer sets up the server object which does the real work.
//
///////////////////////////////////////////////////////////////

#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <sys/wait.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/Socket.hh>
#include <toolsa/ServerSocket.hh>
#include <toolsa/umisc.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaArray.hh>
#include <toolsa/sincos.h>
#include "Args.hh"
#include "Params.hh"
#include "ChillSdbServer.hh"
using namespace std;

// Constructor

ChillSdbServer::ChillSdbServer(int argc, char **argv)

{

  // init

  OK = true;
  _progName = "ChillSdbServer";
  _lastPrintTime = 0;
  _numClients = 0;
  _prevAz = -999;
  _prevEl = -999;

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = false;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = NULL;
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }
  // init process mapper registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);

}

// destructor

ChillSdbServer::~ChillSdbServer()

{

  _clearFieldParams();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int ChillSdbServer::Run()
{

  // initialize from the radar queue
  // before we even consider taking clients

  if (_initialize()) {
    return -1;
  }

  while (true) {
    
    PMU_auto_register("Setting up server");

    // set up server
    
    ServerSocket server;
    if (server.openServer(_params.port)) {
      if (_params.debug) {
        cerr << "ERROR - ChillSdbServer" << endl;
        cerr << "  Cannot open server, port: " << _params.port << endl;
        cerr << "  " << server.getErrStr() << endl;
      }
      umsleep(100);
      continue;
    }

    // register
    
    char msg[1024];
    sprintf(msg, "Listening on port %d ....", _params.port);
    PMU_auto_register(msg);
    
    while (true) {

      // get a client
      
      Socket *sock = NULL;
      while (sock == NULL) {

	PMU_auto_register(msg);

        // print?
        if (_params.debug) {
          time_t now = time(NULL);
          if (now - _lastPrintTime > 30) {
            cerr << msg << endl;
            _lastPrintTime = now;
          }
        }

        // reap any children from previous client
        _reapChildren();

        // get client
        sock = server.getClient(100);

      }

      if (_params.debug) {
        cerr << "  Got client ..." << endl;
      }
      
      if (_params.max_clients > 0 && _numClients >= _params.max_clients) {
        if (_params.debug) {
          cerr << "  Too many clients: " << _numClients << endl;
          cerr << "  Ignoring this one" << endl;
        }
        delete sock;
        continue;
      }

      // in no_threads mode, do not fork

      if (_params.no_threads) {
        _handleClient(sock);
        sock->close();
        delete sock;
        continue;
      } // if (_params.no_threads)

      // spawn a child to handle the connection

      int pid = fork();
      
      // check for parent or child
      
      if (pid == 0) {

        // child - provide service to client

        int iret = _handleClient(sock);
        sock->close();
        delete sock;

        // child exits

        _exit(iret);

      } else {

        // parent - clean up only

        _numClients++;
        if (_params.debug) {
          cerr << "Parent - num clients: " << _numClients << endl;
        }
        delete sock;

      } // if (pid == 0)
        
    } // while
      
  } // while

  return 0;

}

//////////////////////////////////////////////////
// initialize
//
// Read radar params and field params

int ChillSdbServer::_initialize()

{

  if (_params.debug) {
    cerr << "Initializing using FMQ at: " << _params.input_fmq_url << endl;
  }

  // Instantiate and initialize the DsRadar queue and message
  // read through the queue getting the radarParams and fieldParams
  
  DsRadarQueue radarQ;
  DsRadarMsg msg;
  
  if (radarQ.init(_params.input_fmq_url, _progName.c_str(),
                  _params.debug >= Params::DEBUG_VERBOSE,
                  DsFmq::READ_ONLY, DsFmq::START)) {
    cerr << "ERROR - ChillSdbServer::_initialize" << endl;
    cerr << "  Could not initialize radar queue: "
	 << _params.input_fmq_url << endl;
    return -1;
  }

  bool foundRadarParams = false;
  bool foundFieldParams = false;

  while (!foundRadarParams || !foundFieldParams) { 
    
    int contents = 0;
    if (radarQ.getDsMsg(msg, &contents) == 0) {
      
      if (contents & DsRadarMsg::RADAR_PARAMS) {
        _rParams = msg.getRadarParams();
        foundRadarParams = true;
      }

      if (contents & DsRadarMsg::FIELD_PARAMS) {
	_saveFieldParams(msg);
        foundFieldParams = true;
      }

    } else {

      // sleep

      umsleep(1000);
      PMU_auto_register("Inializing");
      if (_params.debug) {
        cerr << "Initializing, waiting for radar params ..." << endl;
      }

    }

  } // while

  if (_params.debug) {
    cerr << "Initializing complete" << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// provide data to client

int ChillSdbServer::_handleClient(Socket *sock)

{

  _rayNumber = 0;
  _sweepNum = 0;
  _volNum = 0;

  // Instantiate and initialize the DsRadar queue and message
  
  DsRadarQueue radarQ;
  DsRadarMsg msg;
  
  if (radarQ.init(_params.input_fmq_url, _progName.c_str(),
                  _params.debug >= Params::DEBUG_VERBOSE,
                  DsFmq::READ_ONLY, DsFmq::END)) {
    cerr << "ERROR - ChillSdbServer::_handleClient" << endl;
    cerr << "  Could not initialize radar queue: "
	 << _params.input_fmq_url << endl;
    return -1;
  }

  // first write out the version header

  if(_writeVersion(sock)) {
    return -1;
  }

  // write out initial headers

  if(_writeInitialHeaders(sock)) {
    return -1;
  }

  // get data from the queue

  double timeSlept = 0.0;
  while (true) { 

    // get a message from the radar queue
    
    int contents = 0;
    if (radarQ.getDsMsg(msg, &contents) == 0) {
      
      if (contents & DsRadarMsg::RADAR_PARAMS) {
        _rParams = msg.getRadarParams();
        // write packets which depend on radar params
	if(_writeRadarInfo(sock)) {
          return -1;
        }
        if (_writeProcessorInfo(sock)) {
          return -1;
        }
        if (_writeXmitInfo(sock)) {
          return -1;
        }
      }
      
      if (contents & DsRadarMsg::FIELD_PARAMS) {
	_saveFieldParams(msg);
      }
      
      if (contents & DsRadarMsg::RADAR_BEAM) {
	if(_writeBeam(msg, sock)) {
          return -1;
        }
      }

      if (contents & DsRadarMsg::RADAR_FLAGS) {
	if(_handleFlags(msg, sock)) {
          return -1;
        }
      }

    } else {

      // sleep
      
      umsleep(100);
      timeSlept += 0.1;
      
      // if we are not doing anything else, send out version
      
      if (timeSlept > 1) {
        if(_writeVersion(sock)) {
          return -1;
        }
        timeSlept = 0.0;
      }
      
    } // if (radarQ ...
    
  } // while
  
  return 0;

}

//////////////////////////////////////////////////
// Write version struct
// Returns 0 on success, -1 on failure

int ChillSdbServer::_writeVersion(Socket *sock)

{

  sdb_version_hdr_t version;
  version.id = HSK_ID_VERSION;
  version.length = sizeof(sdb_version_hdr_t);
  version.version = SDB_FORMAT_VERSION;
  version.creator_version = 0;

  if (sock->writeBuffer(&version, sizeof(sdb_version_hdr_t))) {
    cerr << "ERROR - ChillSdbServer::_writeVersion" << endl;
    cerr << "  Error writing version to client" << endl;
    cerr << "  " << sock->getErrStr() << endl;
    return -1;
  }


  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Wrote version" << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// Write initial headers
// Returns 0 on success, -1 on failure

int ChillSdbServer::_writeInitialHeaders(Socket *sock)

{

  if (_writeRadarInfo(sock)) {
    return -1;
  }

  if (_writeScanSegment(sock)) {
    return -1;
  }
  
  if (_writeProcessorInfo(sock)) {
    return -1;
  }
  
  if (_writePowerUpdate(sock)) {
    return -1;
  }
  
  if (_writeCalTerms(sock)) {
    return -1;
  }
  
  if (_writeXmitInfo(sock)) {
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// Write radar info
//
// Returns 0 on success, -1 on failure

int ChillSdbServer::_writeRadarInfo(Socket *sock)

{

  radar_info_t rinfo;
  MEM_zero(rinfo);
  rinfo.id = HSK_ID_RADAR_INFO;
  rinfo.length = sizeof(radar_info_t);
  STRncopy(rinfo.radar_name, _rParams.radarName.c_str(), MAX_RADAR_NAME);
  rinfo.latitude_d = _rParams.latitude;
  rinfo.longitude_d = _rParams.longitude;
  rinfo.altitude_m = _rParams.altitude * 1000.0;
  rinfo.beamwidth_d = (_rParams.horizBeamWidth + _rParams.vertBeamWidth) / 2.0;
  rinfo.wavelength_cm = _rParams.wavelength;
  rinfo.gain_ant_h_db = _rParams.antennaGain;
  rinfo.gain_ant_v_db = _rParams.antennaGain;
  rinfo.zdr_cal_base_db = 0.0;
  rinfo.phidp_rot_d = 0.0;
  rinfo.base_radar_constant_db = _rParams.radarConstant;
  rinfo.power_measurement_loss_h_db = 0;
  rinfo.power_measurement_loss_v_db = 0;
  rinfo.zdr_cal_base_vhs_db = 0;
  rinfo.test_power_h_db = 0;
  rinfo.test_power_v_db = 0;
  rinfo.dc_loss_h_db = 0;
  rinfo.dc_loss_v_db = 0;

  if (sock->writeBuffer(&rinfo, sizeof(radar_info_t))) {
    cerr << "ERROR - ChillSdbServer::_writeRadarInfo" << endl;
    cerr << "  " << sock->getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Wrote radar info" << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// Write scan segment
//
// Returns 0 on success, -1 on failure

int ChillSdbServer::_writeScanSegment(Socket *sock)

{

  scan_seg_t seg;
  MEM_zero(seg);
  seg.id = HSK_ID_SCAN_SEG;
  seg.length = sizeof(scan_seg_t);
  seg.az_manual = 0.0;
  seg.el_manual = 0.0;
  seg.az_start = 400.0;
  seg.el_start = 400.0;
  seg.scan_rate = 10.0;
  STRncopy(seg.segname, "unknown", MAX_SEGNAME_LENGTH);
  seg.opt.rmax_km = 0.0;
  seg.opt.htmax_km = 0.0;
  seg.opt.res_m = 0.0;
  seg.follow_mode = FOLLOW_MODE_NONE;

  if (_rParams.scanMode == DS_RADAR_RHI_MODE) {
    seg.scan_type = SCAN_TYPE_RHI;
  } else if (_rParams.scanMode == DS_RADAR_EL_SURV_MODE) {
    seg.scan_type = SCAN_TYPE_RHI;
  } else if (_rParams.scanMode == DS_RADAR_SECTOR_MODE) {
    seg.scan_type = SCAN_TYPE_PPI;
  } else if (_rParams.scanMode == DS_RADAR_SURVEILLANCE_MODE) {
    seg.scan_type = SCAN_TYPE_PPI;
  } else if (_rParams.scanMode == DS_RADAR_MANPPI_MODE) {
    seg.scan_type = SCAN_TYPE_MANPPI;
  } else if (_rParams.scanMode == DS_RADAR_MANRHI_MODE) {
    seg.scan_type = SCAN_TYPE_MANRHI;
  } else if (_rParams.scanMode == DS_RADAR_IDLE_MODE) {
    seg.scan_type = SCAN_TYPE_IDLE;
  }

  seg.scan_flags = 0;
  seg.volume_num = _volNum;
  seg.sweep_num = _sweepNum;
  seg.time_limit = 0;
  seg.webtilt = 0;
  seg.left_limit = 0;
  seg.right_limit = 0;
  seg.up_limit = 0;
  seg.down_limit = 0;
  seg.step = 0;
  seg.max_sweeps = 0;
  seg.filter_break_sweep = 0;
  seg.clutter_filter1 = 0;
  seg.clutter_filter2 = 0;
  STRncopy(seg.project, "unknown", MAX_PRJNAME_LENGTH);
  seg.current_fixed_angle = 0.0;

  // write radar params
  
  if (sock->writeBuffer(&seg, sizeof(scan_seg_t))) {
    cerr << "ERROR - ChillSdbServer::_writeScanSegment" << endl;
    cerr << "  " << sock->getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Wrote scan segment" << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// Write processor info
//
// Returns 0 on success, -1 on failure

int ChillSdbServer::_writeProcessorInfo(Socket *sock)

{

  processor_info_t pinfo;
  MEM_zero(pinfo);
  pinfo.id = HSK_ID_PROCESSOR_INFO;
  pinfo.length = sizeof(processor_info_t);

  switch (_params.polarization_mode) {
    case Params::POL_MODE_H:
      pinfo.polarization_mode = POL_MODE_H;
      break;
    case Params::POL_MODE_V:
      pinfo.polarization_mode = POL_MODE_V;
      break;
    case Params::POL_MODE_VHS:
      pinfo.polarization_mode = POL_MODE_VHS;
      break;
    case Params::POL_MODE_VH:
    default:
      pinfo.polarization_mode = POL_MODE_VH;
      break;
  }

  pinfo.processing_mode = PROC_MODE_INDEXEDBEAM_MSK;
  pinfo.pulse_type = PULSE_TYPE_RECT_1US;
  pinfo.test_type = TEST_TYPE_NONE;
  
  pinfo.integration_cycle_pulses = _rParams.samplesPerBeam;
  pinfo.clutter_filter_number = 0;
  pinfo.range_gate_averaging = 1;
  pinfo.indexed_beam_width_d = 1.0;
  
  pinfo.gate_spacing_m = _rParams.gateSpacing * 1000.0;
  pinfo.prt_usec = (1.0 / _rParams.pulseRepFreq) * 1000000.0;
  pinfo.range_start_km = _rParams.startRange;
  pinfo.range_stop_km =
    _rParams.startRange + _rParams.numGates * _rParams.gateSpacing;
  pinfo.max_gate = _rParams.numGates;
  pinfo.test_power_dbm = 0;
  pinfo.test_pulse_range_km = 0;
  pinfo.test_pulse_length_usec = 0;
  pinfo.prt2_usec = (1.0 / _rParams.pulseRepFreq) * 1000000.0;
  pinfo.range_offset_m = _rParams.startRange * 1000.0;

  if (sock->writeBuffer(&pinfo, sizeof(processor_info_t))) {
    cerr << "ERROR - ChillSdbServer::_writeProcessorInfo" << endl;
    cerr << "  " << sock->getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Wrote processor info" << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// Write power update
//
// Returns 0 on success, -1 on failure

int ChillSdbServer::_writePowerUpdate(Socket *sock)

{

  power_update_t pup;
  MEM_zero(pup);
  pup.id = HSK_ID_PWR_UPDATE;
  pup.length = sizeof(power_update_t);
  pup.h_power_dbm = 0.0;
  pup.v_power_dbm = 0.0;

  if (sock->writeBuffer(&pup, sizeof(power_update_t))) {
    cerr << "ERROR - ChillSdbServer::_writePowerUpdate" << endl;
    cerr << "  " << sock->getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Wrote power update" << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// Write cal terms
//
// Returns 0 on success, -1 on failure

int ChillSdbServer::_writeCalTerms(Socket *sock)

{

  cal_terms_t cal;
  MEM_zero(cal);
  cal.id = HSK_ID_CAL_TERMS;
  cal.length = sizeof(cal_terms_t);

  cal.noise_v_rx_1 = -114.0;
  cal.noise_h_rx_2 = -114.0;
  cal.noise_v_rx_2 = -114.0;
  cal.noise_h_rx_1 = -114.0;
  
  cal.zcon_h_db = -35.0;
  cal.zcon_v_db = -35.0;  
  cal.zdr_bias_db = 0.0;
  cal.ldr_bias_h_db = 0.0;
  cal.ldr_bias_v_db = 0.0;
  
  cal.noise_source_h_db = -20.0;
  cal.noise_source_v_db = -20.0;
  
  cal.gain_v_rx_1_db = 35.0;
  cal.gain_h_rx_2_db = 35.0;
  cal.gain_v_rx_2_db = 35.0;
  cal.gain_h_rx_1_db = 35.0;
  
  cal.sun_pwr_v_rx_1_db = -66.0;
  cal.sun_pwr_h_rx_2_db = -66.0;
  cal.sun_pwr_v_rx_2_db = -66.0;
  cal.sun_pwr_h_rx_1_db = -66.0;

  if (sock->writeBuffer(&cal, sizeof(cal_terms_t))) {
    cerr << "ERROR - ChillSdbServer::_writeCalTerms" << endl;
    cerr << "  " << sock->getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Wrote cal" << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// Write xmit info
//
// Returns 0 on success, -1 on failure

int ChillSdbServer::_writeXmitInfo(Socket *sock)

{

  xmit_info_t xmit;
  MEM_zero(xmit);
  xmit.id = HSK_ID_XMIT_INFO;
  xmit.length = sizeof(xmit_info_t);

  xmit.xmit_enables = 1;

  switch (_params.polarization_mode) {
    case Params::POL_MODE_H:
      xmit.polarization_mode = POL_MODE_H;
      break;
    case Params::POL_MODE_V:
      xmit.polarization_mode = POL_MODE_V;
      break;
    case Params::POL_MODE_VHS:
      xmit.polarization_mode = POL_MODE_VHS;
      break;
    case Params::POL_MODE_VH:
    default:
      xmit.polarization_mode = POL_MODE_VH;
      break;
  }

  xmit.pulse_type = PULSE_TYPE_RECT_1US;
  xmit.prt_usec = (1.0 / _rParams.pulseRepFreq) * 1000000.0;
  xmit.prt2_usec = (1.0 / _rParams.pulseRepFreq) * 1000000.0;

  if (sock->writeBuffer(&xmit, sizeof(xmit_info_t))) {
    cerr << "ERROR - ChillSdbServer::_writeXmitInfo" << endl;
    cerr << "  " << sock->getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Wrote xmit info" << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// Write beam
//
// Returns 0 on success, -1 on failure

int ChillSdbServer::_writeBeam(DsRadarMsg &msg, Socket *sock)

{

  if (_params.beam_wait_msecs > 0) {
    umsleep(_params.beam_wait_msecs);
  }

  // allocate output moments array

  TaArray<gate_mom_data_t> momOutput_;
  gate_mom_data_t *momOutput = momOutput_.alloc(_rParams.numGates);
  _initMomOutput(momOutput);

  // fill out beam data

  _fillBeam(msg, momOutput);

  // compute el and az
  // these are at the end of the dwell

  DsRadarBeam &beam = msg.getRadarBeam();

  double az = beam.azimuth;
  double azWidth = 1.0;
  double deltaAz = 1.0;
  if (_prevAz >= -360) {
    deltaAz = az - _prevAz;
    if (deltaAz < -180.0) {
      deltaAz += 360.0;
    } else if (deltaAz > 180.0) {
      deltaAz -= 360.0;
    }
    azWidth = fabs(deltaAz);
  }
  double endAz = az + signof(deltaAz) * azWidth / 2.0;
  if (endAz < 0) {
    endAz += 360.0;
  } else if (endAz >= 360.0) {
    endAz -= 360.0;
  }

  double el = beam.elevation;
  double elWidth = 1.0;
  double deltaEl = 1.0;
  if (_prevEl >= -360) {
    deltaEl = el - _prevEl;
    if (deltaEl < -180.0) {
      deltaEl += 360.0;
    } else if (deltaEl > 180.0) {
      deltaEl -= 360.0;
    }
    elWidth = fabs(deltaEl);
  }
  double endEl = el + signof(deltaEl) * elWidth / 2.0;

  _prevAz = az;
  _prevEl = el;
  
  // prepare ray header

  ray_header_t ray;
  MEM_zero(ray);
  ray.id = MOM_ID_ALL_DATA;
  ray.length = sizeof(ray_header_t);
  ray.azimuth = endAz;
  ray.elevation = endEl;
  ray.azimuth_width = azWidth;
  ray.elevation_width = elWidth;
  ray.gates = _rParams.numGates;
  ray.beam_index = _rayNumber % 360;
  ray.ray_number = _rayNumber;
  ray.time = beam.dataTime;
  ray.ns_time = 0;
  ray.num_pulses = _rParams.samplesPerBeam;
  ray.bytes_per_gate = sizeof(gate_mom_data_t);

  _rayNumber++;

  // write ray header
  
  if (sock->writeBuffer(&ray, sizeof(ray_header_t))) {
    cerr << "ERROR - ChillSdbServer::_writeBeam" << endl;
    cerr << "  Writing ray header" << endl;
    cerr << "  " << sock->getErrStr() << endl;
    return -1;
  }

  // write beam data
  
  if (sock->writeBuffer(momOutput, _rParams.numGates * sizeof(gate_mom_data_t))) {
    cerr << "ERROR - ChillSdbServer::_writeBeam" << endl;
    cerr << "  Writing beam moments data" << endl;
    cerr << "  " << sock->getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Wrote ray" << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// Handle flags for end-of-tilt and end-of-vol
//
// Returns 0 on success, -1 on failure

int ChillSdbServer::_handleFlags(DsRadarMsg &msg, Socket *sock)

{

  const DsRadarFlags &flags = msg.getRadarFlags();
  bool changed = false;
  if (_volNum != flags.volumeNum ||
      _sweepNum != flags.tiltNum) {
    changed = true;
  }
  _volNum = flags.volumeNum;
  _sweepNum = flags.tiltNum;
  
  if (flags.startOfTilt) {
    if (_writeEventNotice(sock, EN_START_SWEEP)) {
      return -1;
    }
  }

  if (flags.endOfTilt) {
    if (_writeEventNotice(sock, EN_END_SWEEP)) {
      return -1;
    }
  }

  if (flags.endOfVolume) {
    if (_writeEventNotice(sock, EN_END_VOLUME)) {
      return -1;
    }
  }

  if (changed) {
    if (_writeScanSegment(sock)) {
      return -1;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// Write event notice
//
// Returns 0 on success, -1 on failure

int ChillSdbServer::_writeEventNotice(Socket *sock,
                                      event_notice_flags_t flags)

{
  
  event_notice_t event;
  MEM_zero(event);
  event.id = HSK_ID_EVENT_NOTICE;
  event.length = sizeof(event_notice_t);
  event.flags = flags;
  event.cause = END_NOTICE_CAUSE_DONE;
  
  if (sock->writeBuffer(&event, sizeof(event_notice_t))) {
    cerr << "ERROR - ChillSdbServer::_writeEventNotice" << endl;
    cerr << "  " << sock->getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Wrote event notice, flags: " << flags << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// Save the field params for later use
// Returns 0 on success, -1 on failure

void ChillSdbServer::_saveFieldParams(DsRadarMsg &msg)

{

  _clearFieldParams();

  const vector<DsFieldParams *> &fparams = msg.getFieldParams();
  for (int ii = 0; ii < (int) fparams.size(); ii++) {
    DsFieldParams *fp = new DsFieldParams();
    *fp = *fparams[ii];
    _fParams.push_back(fp);
  }

  // set up the indices for the various moments

  _momIndex.Z = _findFieldIndex(_params.DBZ_name);
  _momIndex.V = _findFieldIndex(_params.VEL_name);
  _momIndex.W = _findFieldIndex(_params.WIDTH_name);
  _momIndex.NCP = _findFieldIndex(_params.NCP_name);
  _momIndex.ZDR = _findFieldIndex(_params.ZDR_name);
  _momIndex.PHIDP = _findFieldIndex(_params.PHIDP_name);
  _momIndex.RHOHV = _findFieldIndex(_params.RHOHV_name);
  _momIndex.LDR_H = _findFieldIndex(_params.LDRH_name);
  _momIndex.LDR_V = _findFieldIndex(_params.LDRV_name);
  _momIndex.KDP = _findFieldIndex(_params.KDP_name);
  _momIndex.Zc = _findFieldIndex(_params.DBZc_name);
  _momIndex.ZDRc = _findFieldIndex(_params.ZDRc_name);
  _momIndex.PHIDPf = _findFieldIndex(_params.PHIDPf_name);
  _momIndex.avg_v_mag = _findFieldIndex(_params.AVG_V_MAG_name);
  _momIndex.avg_v_phase = _findFieldIndex(_params.AVG_V_PHASE_name);
  _momIndex.avg_h_mag = _findFieldIndex(_params.AVG_H_MAG_name);
  _momIndex.avg_h_phase = _findFieldIndex(_params.AVG_H_PHASE_name);
  _momIndex.lag0_hc = _findFieldIndex(_params.LAG0_HC_name);
  _momIndex.lag0_vc = _findFieldIndex(_params.LAG0_VC_name);
  _momIndex.lag0_hx = _findFieldIndex(_params.LAG0_HX_name);
  _momIndex.lag0_vx = _findFieldIndex(_params.LAG0_VX_name);
  _momIndex.lag1_hc_mag = _findFieldIndex(_params.LAG1_HC_MAG_name);
  _momIndex.lag1_hc_phase = _findFieldIndex(_params.LAG1_HC_PHASE_name);
  _momIndex.lag1_vc_mag = _findFieldIndex(_params.LAG1_VC_MAG_name);
  _momIndex.lag1_vc_phase = _findFieldIndex(_params.LAG1_VC_PHASE_name);
  _momIndex.lag2_hc_mag = _findFieldIndex(_params.LAG2_HC_MAG_name);
  _momIndex.lag2_hc_phase = _findFieldIndex(_params.LAG2_HC_PHASE_name);
  _momIndex.lag2_vc_mag = _findFieldIndex(_params.LAG2_VC_MAG_name);
  _momIndex.lag2_vc_phase = _findFieldIndex(_params.LAG2_VC_PHASE_name);
  _momIndex.lag0_hv_mag = _findFieldIndex(_params.LAG0_HV_MAG_name);
  _momIndex.lag0_hv_phase = _findFieldIndex(_params.LAG0_HV_PHASE_name);
  _momIndex.rhohv_hcx = _findFieldIndex(_params.RHOHV_HCX_name);
  _momIndex.rhohv_vcx = _findFieldIndex(_params.RHOHV_VCX_name);
  
}

///////////////////////////
// find field index in FMQ
// Returns -1 if not found

int ChillSdbServer::_findFieldIndex(const string &name)
  
{
  
  int index = -1;
  for (int ii = 0; ii < (int) _fParams.size(); ii++) {
    const DsFieldParams *fp = _fParams[ii];
    if (fp->name == name) {
      index = ii;
      break;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Field, index: " << name << ", " << index << endl;
  }

  return index;
    
}

//////////////////////////////////////////////////
// reap children from fork()s which have exited

void ChillSdbServer::_reapChildren()

{
  
  pid_t dead_pid;
  int status;
  
  while ((dead_pid = waitpid((pid_t) -1,
                             &status,
                             (int) (WNOHANG | WUNTRACED))) > 0) {
    
    _numClients--;
    if (_params.debug) {
      cerr << "  child exited, pid: " << dead_pid << endl;
      cerr << "    num clients: " << _numClients << endl;
    }

  } // while

}

/////////////////////////
// clear the field params

void ChillSdbServer::_clearFieldParams()

{
  
  for (int ii = 0; ii < (int) _fParams.size(); ii++) {
    delete _fParams[ii];
  }
  _fParams.clear();

}

//////////////////////////////////
// convert string version to int


int ChillSdbServer::_getVersionFromString(const char * str)

{
  char *ver_cpy = strdup(str);
  char *save;
  char *major = strtok_r(ver_cpy, ".", &save);
  char *minor = strtok_r(NULL, ".", &save);
  int rv = (atoi(major) << 16) | atoi(minor);
  free(ver_cpy);
  return rv;
}

//////////////////////////////////////////////////
// initialize moments

void ChillSdbServer::_initMomOutput(gate_mom_data_t *momOutput)

{

  gate_mom_data_t *mom = momOutput;
  for (int ii = 0; ii < _rParams.numGates; ii++, mom++) {
    mom->Z = NAN;
    mom->V = NAN;
    mom->W = NAN;
    mom->NCP = NAN;
    mom->ZDR = NAN;
    mom->PHIDP = NAN;
    mom->RHOHV = NAN;
    mom->LDR_H = NAN;
    mom->LDR_V = NAN;
    mom->KDP = NAN;
    mom->Zc = NAN;
    mom->ZDRc = NAN;
    mom->PHIDPf = NAN;
    mom->avg_h = float_complex(NAN, NAN);
    mom->avg_v = float_complex(NAN, NAN);
    mom->lag0_h = NAN;
    mom->lag0_v = NAN;
    mom->lag0_hx = NAN;
    mom->lag0_vx = NAN;
    mom->lag1_h = float_complex(NAN, NAN);
    mom->lag1_v = float_complex(NAN, NAN);
    mom->lag2_h = float_complex(NAN, NAN);
    mom->lag2_v = float_complex(NAN, NAN);
    mom->lag0_hv = float_complex(NAN, NAN);
    mom->RHOHV_HCX = NAN;
    mom->RHOHV_VCX = NAN;

  }

}

//////////////////////////////////////////////////
// Fill beam

void ChillSdbServer::_fillBeam(DsRadarMsg &msg, gate_mom_data_t *momOutput)

{
  
  DsRadarBeam &beam = msg.getRadarBeam();
  void *fieldsIn = beam.getData();

  // use example as a template for computing offsets for storing data in
  // the mom_data struct

  gate_mom_data_t example;
  int offset;

  // load up individual fields

  _loadScalar(fieldsIn, momOutput, _momIndex.Z,
              (char *) &example.Z - (char *) &example.Z, -9999);

  _loadScalar(fieldsIn, momOutput, _momIndex.V,
              (char *) &example.V - (char *) &example.Z, -9999);

  _loadScalar(fieldsIn, momOutput, _momIndex.W,
              (char *) &example.W - (char *) &example.Z, -9999);

  _loadScalar(fieldsIn, momOutput, _momIndex.NCP,
              (char *) &example.NCP - (char *) &example.Z, -9999);

  _loadScalar(fieldsIn, momOutput, _momIndex.ZDR,
              (char *) &example.ZDR - (char *) &example.Z, -9999);

  _loadScalar(fieldsIn, momOutput, _momIndex.PHIDP,
              (char *) &example.PHIDP - (char *) &example.Z, -9999);

  _loadScalar(fieldsIn, momOutput, _momIndex.RHOHV,
              (char *) &example.RHOHV - (char *) &example.Z, -9999);

  _loadScalar(fieldsIn, momOutput, _momIndex.LDR_H,
              (char *) &example.LDR_H - (char *) &example.Z, -9999);

  _loadScalar(fieldsIn, momOutput, _momIndex.LDR_V,
              (char *) &example.LDR_V - (char *) &example.Z, -9999);

  _loadScalar(fieldsIn, momOutput, _momIndex.KDP,
              (char *) &example.KDP - (char *) &example.Z, -9999);

  _loadScalar(fieldsIn, momOutput, _momIndex.Zc,
              (char *) &example.Zc - (char *) &example.Z, -9999);

  _loadScalar(fieldsIn, momOutput, _momIndex.ZDRc,
              (char *) &example.ZDRc - (char *) &example.Z, -9999);

  _loadScalar(fieldsIn, momOutput, _momIndex.PHIDPf,
              (char *) &example.PHIDPf - (char *) &example.Z, -9999);
  
  offset = (char *) &example.avg_v - (char *) &example.Z;
  _loadVector(fieldsIn, momOutput,
              _momIndex.avg_v_mag, _momIndex.avg_v_phase, 
              offset, offset + sizeof(float), -9999);
  
  offset = (char *) &example.avg_h - (char *) &example.Z;
  _loadVector(fieldsIn, momOutput,
              _momIndex.avg_h_mag, _momIndex.avg_h_phase, 
              offset, offset + sizeof(float), -9999);
  
  _loadScalar(fieldsIn, momOutput, _momIndex.lag0_hc,
              (char *) &example.lag0_h - (char *) &example.Z, -9999);

  _loadScalar(fieldsIn, momOutput, _momIndex.lag0_vc,
              (char *) &example.lag0_v - (char *) &example.Z, -9999);

  _loadScalar(fieldsIn, momOutput, _momIndex.lag0_hx,
              (char *) &example.lag0_hx - (char *) &example.Z, -9999);

  _loadScalar(fieldsIn, momOutput, _momIndex.lag0_vx,
              (char *) &example.lag0_vx - (char *) &example.Z, -9999);

  offset = (char *) &example.lag1_h - (char *) &example.Z;
  _loadVector(fieldsIn, momOutput,
              _momIndex.lag1_hc_mag, _momIndex.lag1_hc_phase, 
              offset, offset + sizeof(float), -9999);
  
  offset = (char *) &example.lag1_v - (char *) &example.Z;
  _loadVector(fieldsIn, momOutput,
              _momIndex.lag1_vc_mag, _momIndex.lag1_vc_phase, 
              offset, offset + sizeof(float), -9999);
  
  offset = (char *) &example.lag2_h - (char *) &example.Z;
  _loadVector(fieldsIn, momOutput,
              _momIndex.lag2_hc_mag, _momIndex.lag2_hc_phase, 
              offset, offset + sizeof(float), -9999);
  
  offset = (char *) &example.lag2_v - (char *) &example.Z;
  _loadVector(fieldsIn, momOutput,
              _momIndex.lag2_vc_mag, _momIndex.lag2_vc_phase, 
              offset, offset + sizeof(float), -9999);
  
  offset = (char *) &example.lag0_hv - (char *) &example.Z;
  _loadVector(fieldsIn, momOutput,
              _momIndex.lag0_hv_mag, _momIndex.lag0_hv_phase, 
              offset, offset + sizeof(float), -9999);
  
  _loadScalar(fieldsIn, momOutput, _momIndex.rhohv_hcx,
              (char *) &example.RHOHV_HCX - (char *) &example.Z, -9999);

  _loadScalar(fieldsIn, momOutput, _momIndex.rhohv_vcx,
              (char *) &example.RHOHV_VCX - (char *) &example.Z, -9999);

}

//////////////////////////////////////////////////
// load data for a selected field

void ChillSdbServer::_loadScalar(void *fieldsIn,
                                 gate_mom_data_t *momOutput,
                                 int inIndex,
                                 int outOffset,
                                 float outMissing)

{

  // find the field in the input data

  if (inIndex < 0) {
    // field not present in input data
    return;
  }
  const DsFieldParams *fParams = _fParams[inIndex];
  int byteWidth = fParams->byteWidth;

  // create local float field data array

  TaArray<float> fdata_;
  float *fdata = fdata_.alloc(_rParams.numGates);

  // fill local float array

  if (byteWidth == 1) {
    
    int stride = _rParams.numFields;
    float scale = fParams->scale;
    float bias = fParams->bias;
    ui08 missing = fParams->missingDataValue;
    ui08 *ip = (ui08 *) fieldsIn + inIndex;
    float *fp = fdata;
    for (int ii = 0; ii < _rParams.numGates; ii++, ip+= stride, fp++) {
      ui08 uval = *ip;
      if (uval != missing) {
        *fp = uval * scale + bias;
      } else {
        *fp = outMissing;
      }
    }
    
  } else if (byteWidth == 2) {

    int stride = _rParams.numFields;
    float scale = fParams->scale;
    float bias = fParams->bias;
    ui16 missing = fParams->missingDataValue;
    ui16 *ip = (ui16 *) fieldsIn + inIndex;
    float *fp = fdata;
    for (int ii = 0; ii < _rParams.numGates; ii++, ip+= stride, fp++) {
      ui16 uval = *ip;
      if (uval != missing) {
        *fp = uval * scale + bias;
      } else {
        *fp = outMissing;
      }
    }
    
  } else if (byteWidth == 4) {

    int stride = _rParams.numFields;
    fl32 missing = fParams->missingDataValue;
    float *ip = (float *) fieldsIn + inIndex;
    float *fp = fdata;
    for (int ii = 0; ii < _rParams.numGates; ii++, ip+= stride, fp++) {
      float fval = *ip;
      if (fval != missing) {
        *fp = fval;
      } else {
        *fp = outMissing;
      }
    }
  
  } // if (byteWidth == 1)

  // copy float array to output moments

  int stride = sizeof(gate_mom_data_t);
  float *fp = fdata;
  char *outp = ((char *) momOutput) + outOffset;
  for (int ii = 0; ii < _rParams.numGates; ii++, outp += stride, fp++) {
    *((float *) outp) = *fp;
  }

}

//////////////////////////////////////////////////
// load vector data for a selected field

void ChillSdbServer::_loadVector(void *fieldsIn,
                                 gate_mom_data_t *momOutput,
                                 int inIndexMag,
                                 int inIndexPhase,
                                 int outOffsetRe,
                                 int outOffsetIm,
                                 float outMissing)
  
{

  // find the field in the input data

  if (inIndexMag < 0 || inIndexPhase < 0) {
    // vector field not present in input data
    return;
  }

  const DsFieldParams *paramsMag = _fParams[inIndexMag];
  int byteWidthMag = paramsMag->byteWidth;

  const DsFieldParams *paramsPhase = _fParams[inIndexPhase];
  int byteWidthPhase = paramsPhase->byteWidth;

  // create local field data arrays to hold float data
  
  TaArray<float> fMag_, fPhase_;
  float *fMag = fMag_.alloc(_rParams.numGates);
  float *fPhase = fPhase_.alloc(_rParams.numGates);
  
  // fill magnitude array
  
  if (byteWidthMag == 1) {
    
    int stride = _rParams.numFields;
    float scale = paramsMag->scale;
    float bias = paramsMag->bias;
    ui08 missing = paramsMag->missingDataValue;
    ui08 *ip = (ui08 *) fieldsIn + inIndexMag;
    float *fp = fMag;
    for (int ii = 0; ii < _rParams.numGates; ii++, ip+= stride, fp++) {
      ui08 uval = *ip;
      if (uval != missing) {
        *fp = uval * scale + bias;
      } else {
        *fp = outMissing;
      }
    }
    
  } else if (byteWidthMag == 2) {

    int stride = _rParams.numFields;
    float scale = paramsMag->scale;
    float bias = paramsMag->bias;
    ui16 missing = paramsMag->missingDataValue;
    ui16 *ip = (ui16 *) fieldsIn + inIndexMag;
    float *fp = fMag;
    for (int ii = 0; ii < _rParams.numGates; ii++, ip+= stride, fp++) {
      ui16 uval = *ip;
      if (uval != missing) {
        *fp = uval * scale + bias;
      } else {
        *fp = outMissing;
      }
    }
    
  } else if (byteWidthMag == 4) {

    int stride = _rParams.numFields;
    fl32 missing = paramsMag->missingDataValue;
    float *ip = (float *) fieldsIn + inIndexMag;
    float *fp = fMag;
    for (int ii = 0; ii < _rParams.numGates; ii++, ip+= stride, fp++) {
      float fval = *ip;
      if (fval != missing) {
        *fp = fval;
      } else {
        *fp = outMissing;
      }
    }
  
  } // if (byteWidthMag == 1)

  // fill phase array
  
  if (byteWidthPhase == 1) {
    
    int stride = _rParams.numFields;
    float scale = paramsPhase->scale;
    float bias = paramsPhase->bias;
    ui08 missing = paramsPhase->missingDataValue;
    ui08 *ip = (ui08 *) fieldsIn + inIndexPhase;
    float *fp = fPhase;
    for (int ii = 0; ii < _rParams.numGates; ii++, ip+= stride, fp++) {
      ui08 uval = *ip;
      if (uval != missing) {
        *fp = uval * scale + bias;
      } else {
        *fp = outMissing;
      }
    }
    
  } else if (byteWidthPhase == 2) {

    int stride = _rParams.numFields;
    float scale = paramsPhase->scale;
    float bias = paramsPhase->bias;
    ui16 missing = paramsPhase->missingDataValue;
    ui16 *ip = (ui16 *) fieldsIn + inIndexPhase;
    float *fp = fPhase;
    for (int ii = 0; ii < _rParams.numGates; ii++, ip+= stride, fp++) {
      ui16 uval = *ip;
      if (uval != missing) {
        *fp = uval * scale + bias;
      } else {
        *fp = outMissing;
      }
    }
    
  } else if (byteWidthPhase == 4) {

    int stride = _rParams.numFields;
    fl32 missing = paramsPhase->missingDataValue;
    float *ip = (float *) fieldsIn + inIndexPhase;
    float *fp = fPhase;
    for (int ii = 0; ii < _rParams.numGates; ii++, ip+= stride, fp++) {
      float fval = *ip;
      if (fval != missing) {
        *fp = fval;
      } else {
        *fp = outMissing;
      }
    }
  
  } // if (byteWidthPhase == 1)

  // cmopute real and imaginary components from the magnitude and phase
  // copy to output buffer

  int stride = sizeof(gate_mom_data_t);
  float *fpMag = fMag;
  float *fpPhase = fPhase;
  char *outpRe = ((char *) momOutput) + outOffsetRe;
  char *outpIm = ((char *) momOutput) + outOffsetIm;
  for (int ii = 0; ii < _rParams.numGates;
       ii++, outpRe += stride, outpIm += stride, fpMag++, fpPhase++) {
    float mag = *fpMag;
    float phase = *fpPhase;
    double sinPhase, cosPhase;
    ta_sincos(phase * DEG_TO_RAD, &sinPhase, &cosPhase);
    float re = mag * cosPhase;
    float im = mag * sinPhase;
    *((float *) outpRe) = re;
    *((float *) outpIm) = im;
  }

}

