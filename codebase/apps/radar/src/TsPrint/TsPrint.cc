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
// TsPrint.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////
//
// TsPrint reads time-series data from an FMQ
// and processes it in various ways
//
////////////////////////////////////////////////////////////////

#include "TsPrint.hh"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctime>
#include <toolsa/DateTime.hh>
#include <toolsa/uusleep.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaXml.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/pmu.h>
#include <toolsa/ServerSocket.hh>
#include <radar/RadarComplex.hh>

using namespace std;

// Constructor

TsPrint::TsPrint(int argc, char **argv)
  
{

  isOK = true;
  _pulseCount = 0;
  _totalPulseCount = 0;
  _printCount = 0;
  _pulseReader = NULL;
  _haveChan1 = false;
  _fastAlternating = false;
  _prevPulseSeqNum = 0;

  _nPulsesRead = 0;
  _firstPulse = NULL;
  _secondPulse = NULL;
  
  _prevAzimuth = -9999.0;
  _prevElevation = -9999.0;

  MEM_zero(_scanPrev);
  MEM_zero(_procPrev);
  MEM_zero(_calibPrev);
  _infoChanged = false;

  _gateForMax0 = 0;
  _gateForMax1 = 0;

  // set programe name
  
  _progName = "TsPrint";

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
  
  // create the pulse reader
  
  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    iwrfDebug = IWRF_DEBUG_EXTRA;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrfDebug = IWRF_DEBUG_VERBOSE;
  } else if (_params.debug) {
    iwrfDebug = IWRF_DEBUG_NORM;
  } 
    
  if (_params.input_mode == Params::TS_FMQ_INPUT) {
    _pulseReader = new IwrfTsReaderFmq(_params.input_fmq_name,
				       iwrfDebug,
				       !_params.seek_to_end_of_input);
  } else if (_params.input_mode == Params::TS_TCP_INPUT) {
    _pulseReader = new IwrfTsReaderTcp(_params.tcp_server_host,
                                       _params.tcp_server_port,
				       iwrfDebug);
  } else {
    _pulseReader = new IwrfTsReaderFile(_args.inputFileList, iwrfDebug);
  }
  if (_params.rvp8_legacy_unpacking) {
    _pulseReader->setSigmetLegacyUnpacking(true);
  }
  if (_params.cohere_iq_to_burst_phase) {
    _pulseReader->setCohereIqToBurst(true);
  }
  if (_params.check_radar_id) {
    _pulseReader->setRadarId(_params.radar_id);
  }
  if (_params.use_secondary_georeference) {
    _pulseReader->setGeorefUseSecondary(true);
  }

  // calibration

  _rxGainHc = 1.0;
  _rxGainVc = 1.0;
  _rxGainHx = 1.0;
  _rxGainVx = 1.0;

  if (_params.apply_calibration) {
    string errStr;
    if (_calib.readFromXmlFile(_params.cal_xml_file_path, errStr)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Cannot read cal file: " << _params.cal_xml_file_path << endl;
      cerr << "  " << errStr << endl;
      isOK = false;
      return;
    }
    if (_params.debug) {
      cerr << "Using calibration from file: " << _params.cal_xml_file_path << endl;
      _calib.print(cerr);
    }
    if (_calib.getReceiverGainDbHc() > -9990) {
      _rxGainHc = pow(10.0, _calib.getReceiverGainDbHc() / 10.0);
    }
    if (_calib.getReceiverGainDbVc() > -9990) {
      _rxGainVc = pow(10.0, _calib.getReceiverGainDbVc() / 10.0);
    }
    if (_calib.getReceiverGainDbHx() > -9990) {
      _rxGainHx = pow(10.0, _calib.getReceiverGainDbHx() / 10.0);
    }
    if (_calib.getReceiverGainDbVx() > -9990) {
      _rxGainVx = pow(10.0, _calib.getReceiverGainDbVx() / 10.0);
    }
  }

  _stats.setCalibration(_rxGainHc,
                        _rxGainVc,
                        _rxGainHx,
                        _rxGainVx);

  // init process mapper registration
  
  if (_params.reg_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
  // initialize from params
  // will be overwritten in server mode by requests from client

  _nSamples = _params.n_samples;
  _startGateRequested = _params.start_gate;
  _nGatesRequested = _params.n_gates;

}

// destructor

TsPrint::~TsPrint()

{

  if (_pulseReader) {
    delete _pulseReader;
  }

  _ascopeStats.clear();

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int TsPrint::Run ()
{

  if (_params.add_cols_from_status_xml) {
    _initExtraCols();
  }

  if (_params.print_format) {
    iwrf_print_all_formats(stdout);
    return 0;
  }

  if (_params.run_mode == Params::ASCOPE_MODE) {
    
    if (_params.once_only) {
      return _runAscopeMode();
    } else {
      while (_runAscopeMode() == 0) {
        if (_pulseReader->endOfFile()) {
          return 0;
        }
      }
      return -1;
    }

  } else if (_params.run_mode == Params::PRINT_MODE) {

    if (_params.print_prt_details) {
      return _printPrtDetails();
    } else {
      return _runPrintMode();
    }

  } else if (_params.run_mode == Params::SERVER_MODE) {

    return _runServerMode();

  } else if (_params.run_mode == Params::MAX_POWER_MODE) {
    
    if (_params.once_only) {
      return _runMaxPowerMode();
    } else {
      while (_runMaxPowerMode() == 0) {
        if (_pulseReader->endOfFile()) {
          return 0;
        }
      }
      return -1;
    }

  } else if (_params.run_mode == Params::MAX_POWER_SERVER_MODE) {

    return _runMaxPowerServerMode();

  }

  return 0;

}

//////////////////////////////////////////////////
// Run in print mode

int TsPrint::_runPrintMode()
  
{

  while (true) {
    
    // read next pulse
    
    IwrfTsPulse *pulse = _getNextPulse();
    if (pulse == NULL) {
      return 0;
    }

    // at start, print headers
    
    if (_totalPulseCount == 0) {
      _printRunDetails(cout);
      _printOpsInfo(cout, pulse);
      if (!_params.print_all_headers) {
        _printSummaryHeading(cout);
      }
    }

    // angle change check

    if (_params.check_angle_change) {
      _checkAngleChange(*pulse);
    }

    // all headers and data?

    if (_params.print_all_headers || _params.print_meta_headers) {
      _pulseReader->getOpsInfo().printMetaQueue(stdout, true);
    }

    if (_pulseReader->getBurst().getPulseSeqNum() > 0 ||
        _pulseReader->getBurst().getNSamples() > 0) {

      if (_pulseReader->isBurstForPreviousPulse()) {
        fprintf(stdout, "WARNING - burst is for previous pulse\n");
      } else if (!_pulseReader->isBurstForLatestPulse()) {
        fprintf(stdout, "ERROR - burst is out of sequence!!!!!!!!\n");
      }

      if (_params.print_all_headers) {
        _pulseReader->getBurst().printHeader(stdout);
      }
      if (_params.print_burst_iq) {
        _pulseReader->getBurst().printData(stdout);
      }

    }
    
    if (_params.print_all_headers && !_params.print_iq_data) {
      pulse->printHeader(stdout);
    }
    if (_params.print_iq_data) {
      pulse->printData(stdout, _startGate, _endGate);
    }
    
    // ops info when it changes

    _infoChanged = _checkInfoChanged(*pulse);
    if (_params.print_info_on_change) {
      if (_infoChanged) {
        _printOpsInfo(cout, pulse);
	_pulseReader->getOpsInfo().print(stdout);
      }
    }
    
    // print pulses
    
    if (_params.print_all_pulses) {
      if ((_printCount % _params.label_interval) == 0) {
        _printPulseLabels(cout);
      }
      _printCount++;
      cout << _pulseString(*pulse) << endl;
    }
    
    // add to data as appropriate

    pulse->convertToFL32();
    if (_pulseCount == 0) {
      _clearStats();
    }
    if (pulse->isHoriz()) {
      _latestIsH = true;
    } else {
      _latestIsH = false;
    }
    if (_fastAlternating) {
      _addToAlternating(*pulse);
    } else {
      _addToSummary(*pulse);
    }
    
    if (_params.add_cols_from_status_xml) {
      _decodeXmlForExtraCols();
    }

    _pulseCount++;
    _totalPulseCount++;
    
    if (_pulseCount == _nSamples) {
      // compute stats
      _computeStats();
      // compute extra col means
      if (_params.add_cols_from_status_xml) {
        _computeExtraColMeans();
      }
      if (!_params.print_all_headers) {
        if ((_printCount % _params.label_interval) == 0) {
          _printSummaryLabels(cout);
        }
        _printSummaryData(stdout);
        _printCount++;
      }
      _pulseCount = 0;
      if (_params.add_cols_from_status_xml) {
        _initExtraCols();
      }
      cout << flush;
      fflush(stdout);
      if (_params.once_only) {
        delete pulse;
        return 0;
      }
    } // if (_pulseCount == _nSamples) 
    
    delete pulse;
    
  } // while

}

//////////////////////////////////////////////////
// Print PRT details

int TsPrint::_printPrtDetails()
  
{

  iwrf_pulse_header prevPulseHdr;
  memset(&prevPulseHdr, 0, sizeof(prevPulseHdr));

  while (true) {
    
    // read next pulse
    
    IwrfTsPulse *pulse = _getNextPulse();
    if (pulse == NULL) {
      return 0;
    }
    
    // at start, print headers
    
    if (_totalPulseCount == 0) {
      _printRunDetails(cout);
      _printOpsInfo(cout, pulse);
      cout << "# PRINTING PRT DETAILS" << endl;
    }

    // all headers and data?

    _pulseReader->getOpsInfo().printMetaQueue(stdout, true);

    if (_pulseReader->getBurst().getPulseSeqNum() > 0 ||
        _pulseReader->getBurst().getNSamples() > 0) {

      if (_pulseReader->isBurstForPreviousPulse()) {
        fprintf(stdout, "WARNING - burst is for previous pulse\n");
      } else if (!_pulseReader->isBurstForLatestPulse()) {
        fprintf(stdout, "ERROR - burst is out of sequence!!!!!!!!\n");
      }

    }
    
    if (_params.print_all_headers) {
      pulse->printHeader(stdout);
    }
    
    // ops info when it changes
    
    _infoChanged = _checkInfoChanged(*pulse);
    if (_infoChanged) {
      _printOpsInfo(cout, pulse);
      if (_params.print_info_on_change) {
	_pulseReader->getOpsInfo().print(stdout);
      }
    }
    
    // print prt details
    
    iwrf_pulse_header pulseHdr = pulse->getHdr();

    DateTime pulseTime(pulseHdr.packet.time_secs_utc,
                       true,
                       (double) pulseHdr.packet.time_nano_secs / 1.0e9);

    DateTime prevTime(prevPulseHdr.packet.time_secs_utc,
                      true,
                      (double) prevPulseHdr.packet.time_nano_secs / 1.0e9);
    
    double timeDiffSecs = 0.0;
    if (_totalPulseCount > 0) {
      timeDiffSecs =
        (double) (prevPulseHdr.packet.time_secs_utc - pulseHdr.packet.time_secs_utc) +
        ((double) pulseHdr.packet.time_nano_secs - prevPulseHdr.packet.time_nano_secs) / 1.0e9;
    }

    fprintf(stdout,
            "Pulse seqNum, time, timeDiff, prt, prtNext: "
            "%10ld, %s.%.9d, %10.6f, %10.6f, %10.6f\n",
            pulseHdr.pulse_seq_num,
            DateTime::stru(pulseTime.utime()).c_str(), 
            pulseHdr.packet.time_nano_secs,
            timeDiffSecs,
            pulseHdr.prt,
            pulseHdr.prt_next);

    if (_params.print_all_pulses) {
      if ((_printCount % _params.label_interval) == 0) {
        _printPulseLabels(cout);
      }
      _printCount++;
      cout << _pulseString(*pulse) << endl;
    }
    
    prevPulseHdr = pulseHdr;
    _pulseCount++;
    _totalPulseCount++;
    
    delete pulse;

  } // while

}

//////////////////////////////////////////////////
// Run in ASCOPE mode

int TsPrint::_runAscopeMode()

{

  // allocate vector for Ascope stats

  _ascopeStats.clear();
  for (int ii = 0; ii < _nGatesRequested; ii++) {
    Stats stats;
    stats.setCalibration(_rxGainHc,
                         _rxGainVc,
                         _rxGainHx,
                         _rxGainVx);
    _ascopeStats.push_back(stats);
  }
  _pulseCount = 0;

  for (int ipulse = 0; ipulse < _nSamples; ipulse++) {

    // read next pulse

    IwrfTsPulse *pulse = _getNextPulse();
    if (pulse == NULL) {
      if (_pulseReader->endOfFile()) {
        return 0;
      }
      cerr << "ERROR - TsPrint::_runAscopeMode()" << endl;
      cerr << "  Cannot read enough samples for aScope" << endl;
      return -1;
    }

    if (_params.print_hv_flag) {
      cerr << pulse->isHoriz() << ",";
    }

    pulse->convertToFL32();
    _addToAscope(*pulse);
    _pulseCount++;
    _totalPulseCount++;
    
    delete pulse;

  } 

  if (_params.print_hv_flag) {
    cerr << endl;
  }

  // compute ascope stats

  for (int igate = 0; igate < _nGates; igate++) {
    _ascopeStats[igate].computeAlternating(_haveChan1);
  }
    
  time_t midSecs = (time_t) _midTime;
  int midNanoSecs = (int) ((_midTime - midSecs) * 1.0e9 + 0.5);
  double prf = 1.0 / _midPrt;

  // print
  
  cout << "# ASCOPE MODE:" << endl;
  cout << "#   Start gate: " << _startGate << endl;
  cout << "#   N gates: " << _nGates << endl;
  cout << "#   N samples: " << _nSamples << endl;
  cout << "#   time: " << DateTime::stru(midSecs) << "."
       << midNanoSecs << endl;
  cout << "#   PRF: " << prf << endl;
  cout << "#   EL (deg): " << _midEl << endl;
  cout << "#   AZ (deg): " << _midAz << endl;
  cout << "#   TRANS: " << (_midTransition?"Y":"N")  << endl;

  cout << "#   Gate num  RangeKm"
       << "       Hc       Hx    Hcorr     Harg"
       << "       Vc       Vx    Vcorr     Varg"
       << "     IFD0     IFD1"
       << endl;

  const IwrfTsInfo &info = _pulseReader->getOpsInfo();
  double startRangeKm = info.get_proc_start_range_km();
  double gateSpacingKm = info.get_proc_gate_spacing_km();

  double rangeKm = startRangeKm + _startGate * gateSpacingKm;
  for (int igate = 0; igate < _nGates; igate++, rangeKm += gateSpacingKm) {

    int gateNum = igate + _startGate;

    fprintf(stdout, "%12d %8.3f "
            "%8.3f %8.3f %8.3f %8.3f "
            "%8.3f %8.3f %8.3f %8.3f "
            "%8.3f %8.3f\n",
            gateNum, rangeKm,
            _ascopeStats[igate].meanDbmHc,
            _ascopeStats[igate].meanDbmHx,
            _ascopeStats[igate].corrH,
            _ascopeStats[igate].argH,
            _ascopeStats[igate].meanDbmVc,
            _ascopeStats[igate].meanDbmVx,
            _ascopeStats[igate].corrV,
            _ascopeStats[igate].argV,
            _ascopeStats[igate].meanDbm0,
            _ascopeStats[igate].meanDbm1);

  }

  return 0;

}

//////////////////////////////////////////////////
// Run in server mode

int TsPrint::_runServerMode()
{

  while (true) {


    char msg[1024];
    sprintf(msg, "Opening port: %d", _params.server_port);
    PMU_auto_register(msg);

    // set up server
    
    ServerSocket server;
    if (server.openServer(_params.server_port)) {
      if (_params.debug) {
        cerr << "ERROR - TsPrint" << endl;
        cerr << "  Cannot open server, port: " << _params.server_port << endl;
        cerr << "  " << server.getErrStr() << endl;
      }
      umsleep(100);
      continue;
    }
    
    if (_params.debug) {
      cerr << "Running as server, listening on port: "
           << _params.server_port << endl;
    }

    while (true) {

      // register with procmap
      sprintf(msg, "Getting clients, port: %d", _params.server_port);
      PMU_auto_register(msg);

      // get a client
      
      Socket *sock = NULL;
      while (sock == NULL) {
        // reap any children from previous client
        _reapChildren();
        // get client
        sock = server.getClient(100);
	// register with procmap
	PMU_auto_register(msg);
      }

      if (_params.debug) {
        cerr << "  Got client ..." << endl;
      }

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

        delete sock;

      }

    } // while
      
  } // while

  return 0;

}

//////////////////////////////////////////////////
// Run in max power mode

int TsPrint::_runMaxPowerMode()

{

  _initMaxPowerData();

  for (int ipulse = 0; ipulse < _nSamples; ipulse++) {
    
    // read next pulse
    
    IwrfTsPulse *pulse = _getNextPulse();
    if (pulse == NULL) {
      if (_pulseReader->endOfFile()) {
        return 0;
      }
      cerr << "ERROR - TsPrint::_runAscopeMode()" << endl;
      cerr << "  Cannot read enough samples for aScope" << endl;
      return -1;
    }

    pulse->convertToFL32();
    _addToMaxPower(*pulse);
    
    // at start, print headers
    
    if (_totalPulseCount == 0) {
      _printRunDetails(cout);
      _printOpsInfo(cout, pulse);
      _printMaxPowerHeading(cout);
    }

    _pulseCount++;
    _totalPulseCount++;

    delete pulse;

  } 

  if ((_printCount % _params.label_interval) == 0) {
    _printMaxPowerLabels(cout);
  }

  _printCount++;
  _computeMaxPowerData();
  _printMaxPowerData(stdout);

  return 0;

}

//////////////////////////////////////////////////
// Run in max power server mode

int TsPrint::_runMaxPowerServerMode()
{

  char msg[1024];
  sprintf(msg, "Opening port: %d", _params.server_port);
  PMU_auto_register(msg);
  
  // set up server
  
  ServerSocket server;
  if (server.openServer(_params.server_port)) {
    cerr << "ERROR - TsPrint" << endl;
    cerr << "  Cannot open server, port: " << _params.server_port << endl;
    cerr << "  " << server.getErrStr() << endl;
    return -1;
  }
    
  if (_params.debug) {
    cerr << "Running as server, listening on port: "
         << _params.server_port << endl;
  }

  while (true) {
    
    // register with procmap
    sprintf(msg, "Getting clients, port: %d", _params.server_port);
    PMU_auto_register(msg);
    
    // get a client
    
    Socket *sock = NULL;
    while (sock == NULL) {
      // reap any children from previous client
      _reapChildren();
      // get client
      sock = server.getClient(100);
      // register with procmap
      PMU_auto_register(msg);
    }
    
    if (_params.debug) {
      cerr << "  Got client ..." << endl;
    }
    
    // spawn a child to handle the connection
    
    int pid = fork();
    
    // check for parent or child
    
    if (pid == 0) {
      
      // child - provide service to client
      
      int iret = _handleMaxPowerClient(sock);
      sock->close();
      delete sock;
      
      // child exits
      
      _exit(iret);
      
    } else {
      
      // parent - clean up only
      
      delete sock;
      
    }
    
  } // while
      
  return 0;

}

////////////////////////////
// get next pulse
//
// returns NULL on failure

IwrfTsPulse *TsPrint::_getNextPulse() 

{

  // at the start, check for alternating mode
  
  if (_nPulsesRead == 0) {

    // read first 2 pulses
    // use for detecting alternating mode

    _firstPulse = _getNextPulseCheckTimeout();
    _secondPulse = _getNextPulseCheckTimeout();

    if (_firstPulse == NULL || _secondPulse == NULL) {
      // failure
      return NULL;
    }

    // check for fast alternating mode

    if (_firstPulse->get_hv_flag() == _secondPulse->get_hv_flag()) {
      _fastAlternating = false;
    } else {
      _fastAlternating = true;
      // in alternating mode, ensure we start on H
      if (!_firstPulse->get_hv_flag()) {
        // first pulse is V
        // discard it, read in another
        delete _firstPulse;
        _firstPulse = _secondPulse;
        _secondPulse = _getNextPulseCheckTimeout();
        if (_secondPulse == NULL) {
          return NULL;
        }
      }
    } // if (_firstPulse->get_hv_flag() == _secondPulse->get_hv_flag())

    // return the first pulse, set it to NULL for next time
    // this is a one-time event
    
    IwrfTsPulse *pulse = _firstPulse;
    _firstPulse = NULL;
    return pulse;

  } // if (_nPulsesRead == 0) 

  // has the second pulse been read, if so return it now
  
  if (_secondPulse != NULL) {
    // second pulse was read to determine alternating mode
    // return the second pulse now
    // this is a one-time event
    IwrfTsPulse *pulse = _secondPulse;
    _secondPulse = NULL;
    return pulse;
  }
  
  // read next pulse
  
  IwrfTsPulse *pulse = _getNextPulseCheckTimeout();
  if (pulse == NULL) {
    return NULL;
  }

  // check for missing pulses
  
  si64 pulseSeqNum = pulse->getSeqNum();
  si64 nMissing = (pulseSeqNum - _prevPulseSeqNum) - 1;
  if ((_prevPulseSeqNum != 0) &&
      (nMissing != 0) &&
      _params.print_missing_pulses) {
    cerr << "WARNING - TsPrint - n missing pulses: " << nMissing << endl;
    cerr << "  prev pulse seq num: " << _prevPulseSeqNum << endl;
    cerr << "  this pulse seq num: " << pulseSeqNum << endl;
    cerr << "  file: " << _pulseReader->getPathInUse() << endl;
  }
  _prevPulseSeqNum = pulseSeqNum;

  return pulse;

}

//////////////////////////////////////////////////
// get next pulse, checking for timeout condition
// returns NULL on failure

IwrfTsPulse *TsPrint::_getNextPulseCheckTimeout() 

{

  IwrfTsPulse *pulse = NULL;

  while (pulse == NULL) {
    pulse = _pulseReader->getNextPulse(false);
    if (pulse == NULL) {
      if (_pulseReader->getTimedOut()) {
	cout << "# reader timed out, retrying ..." << endl;
	continue;
      }
      if (_pulseReader->endOfFile()) {
        cout << "# end of file" << endl;
      }
      return NULL;
    }
  }
  _nPulsesRead++;
  
  // check for dual channel mode
  
  if (pulse->getIq1() != NULL) {
    _haveChan1 = true;
  } else {
    _haveChan1 = false;
  }

  // condition gate limits

  _conditionGateRange(*pulse);

  // set range geom

  _startRangeM = pulse->get_start_range_m();
  _gateSpacingM = pulse->get_gate_spacing_m();

  if (_params.change_packing) {
    // reformat pulse as needed
    if (_params.packing_type == Params::PACKING_FL32) {
      pulse->convertToPacked(IWRF_IQ_ENCODING_FL32);
    } else if (_params.packing_type == Params::PACKING_SCALED_SI16) {
      pulse->convertToPacked(IWRF_IQ_ENCODING_SCALED_SI16);
    } else if (_params.packing_type == Params::PACKING_DBM_PHASE_SI16) {
      pulse->convertToPacked(IWRF_IQ_ENCODING_DBM_PHASE_SI16);
    } else if (_params.packing_type == Params::PACKING_SIGMET_FL16) {
      pulse->convertToPacked(IWRF_IQ_ENCODING_SIGMET_FL16);
    }
  }

  if (_pulseReader->endOfFile()) {
    cout << "# end of file" << endl;
  }
  
  return pulse;
      
}

////////////////////////////////////////////
// condition the gate range, to keep the
// numbers within reasonable limits

void TsPrint::_conditionGateRange(const IwrfTsPulse &pulse)

{

  _startGate = _startGateRequested;
  if (_startGate < 0) {
    _startGate = 0;
  }

  _nGates = _nGatesRequested;
  _endGate = _startGate + _nGates - 1;
  if (_endGate > pulse.getNGates() - 1) {
    _endGate = pulse.getNGates() - 1;
    _nGates = _endGate - _startGate + 1;
  }

  _stats.setNGates(_nGates);

}
    
////////////////////////////////////////////
// set midpoint and other cardinal values

void TsPrint::_saveMetaData(const IwrfTsPulse &pulse)

{

  if (_pulseCount == 0) {
    _startTime = pulse.getFTime();
  }

  if (_pulseCount == _nSamples / 2) {
    _midTime = pulse.getFTime();
    _midPrt = pulse.getPrt();
    _midEl = pulse.getEl();
    _midAz = pulse.getAz();
    _midTransition = pulse.antennaTransition();
  }

  _endTime = pulse.getFTime();

}

////////////////////////////////////////////
// add to summary information

void TsPrint::_addToSummary(const IwrfTsPulse &pulse)

{

  _saveMetaData(pulse);

  const fl32 *iqChan0 = pulse.getIq0();

  int count = 0;
  int index = _startGate * 2;
  for (int igate = _startGate; igate <= _endGate; igate++, count++, index += 2) {
    
    double ii0 = iqChan0[index];
    double qq0 = iqChan0[index + 1];
    if (_params.print_lag1_coherent_power) {
      RadarComplex_t iq0(ii0, qq0);
      _stats.iqHc[count].push_back(iq0);
    }

    double ii1 = -9999;
    double qq1 = -9999;
    if (_haveChan1) {
      const fl32 *iqChan1 = pulse.getIq1();
      ii1 = iqChan1[index];
      qq1 = iqChan1[index + 1];
      if (_params.print_lag1_coherent_power) {
        RadarComplex_t iq1(ii1, qq1);
        _stats.iqVc[count].push_back(iq1);
      }
    }
    _stats.addToSummary(pulse, ii0, qq0, _haveChan1, ii1, qq1);
    
  }

}

////////////////////////////////////////////
// augment alternating information

void TsPrint::_addToAlternating(const IwrfTsPulse &pulse)

{

  _saveMetaData(pulse);

  const fl32 *iqChan0 = pulse.getIq0();
  const fl32 *iqChan1 = pulse.getIq1();
  bool isHoriz = pulse.isHoriz();

  int index = _startGate * 2;
  int count = 0;
  for (int igate = _startGate; igate <= _endGate; igate++, count++, index += 2) {

    double ii0 = iqChan0[index];
    double qq0 = iqChan0[index + 1];
    if (_params.print_lag1_coherent_power) {
      RadarComplex_t iq0(ii0, qq0);
      if (isHoriz) {
        _stats.iqHc[count].push_back(iq0);
      } else {
        _stats.iqVc[count].push_back(iq0);
      }
    }

    double ii1 = -9999;
    double qq1 = -9999;
    if (iqChan1) {
      ii1 = iqChan1[index];
      qq1 = iqChan1[index + 1];
      if (_params.print_lag1_coherent_power) {
        RadarComplex_t iq1(ii1, qq1);
        if (isHoriz) {
          _stats.iqVx[count].push_back(iq1);
        } else {
          _stats.iqHx[count].push_back(iq1);
        }
      }
    }
    _stats.addToAlternating(pulse, ii0, qq0, _haveChan1, ii1, qq1, isHoriz);

  }

}

/////////////////////////
// compute summary stats

void TsPrint::_computeStats()
  
{
  if (_fastAlternating) {
    _stats.computeAlternating(_haveChan1);
  } else {
    _stats.computeSummary(_haveChan1);
  }
}

////////////////////////////////////////////
// clear stats info

void TsPrint::_clearStats()

{

  _stats.init();
  _stats.setNGates(_nGates);
    
  _midTime = -999.9;
  _midPrt = -999.9;
  _midEl = -999.9;
  _midAz = -999.9;
  _midTransition = false;

}
  
////////////////////////////////////////////
// augment ascope information

void TsPrint::_addToAscope(const IwrfTsPulse &pulse)

{

  _saveMetaData(pulse);
  
  const fl32 *iqChan0 = pulse.getIq0();
  const fl32 *iqChan1 = pulse.getIq1();
  bool isHoriz = pulse.isHoriz();

  int index = _startGate * 2;
  for (int igate = 0; igate < _nGates; igate++, index += 2) {
    
    if (!iqChan1) {
      
      double ii0 = iqChan0[index];
      double qq0 = iqChan0[index + 1];
      double power0 = ii0 * ii0 + qq0 * qq0;
      power0 /= _rxGainHc;
      _ascopeStats[igate].sumPower0 += power0;
      _ascopeStats[igate].nn0++;
    
      if (isHoriz) {
        _ascopeStats[igate].nnH++;
        _ascopeStats[igate].sumPowerHc += power0;
      } else {
        _ascopeStats[igate].nnV++;
        _ascopeStats[igate].sumPowerVc += power0;
      }

    } else {
      
      double ii0 = iqChan0[index];
      double qq0 = iqChan0[index + 1];
      double power0 = ii0 * ii0 + qq0 * qq0;
      power0 /= _rxGainHc;
      _ascopeStats[igate].sumPower0 += power0;
      _ascopeStats[igate].nn0++;
    
      double ii1 = iqChan1[index];
      double qq1 = iqChan1[index + 1];
      double power1 = ii1 * ii1 + qq1 * qq1;
      power1 /= _rxGainVc;
      _ascopeStats[igate].sumPower1 += power1;
      _ascopeStats[igate].nn1++;

      RadarComplex_t c0, c1;
      c0.re = ii0;
      c0.im = qq0;
      c1.re = ii1;
      c1.im = qq1;
      RadarComplex_t prod = RadarComplex::conjugateProduct(c0, c1);
      
      if (isHoriz) {
        _ascopeStats[igate].nnH++;
        _ascopeStats[igate].sumPowerHc += power0;
        _ascopeStats[igate].sumPowerVx += power1;
        _ascopeStats[igate].sumConjProdH =
          RadarComplex::complexSum(_ascopeStats[igate].sumConjProdH, prod);
      } else {
        _ascopeStats[igate].nnV++;
        _ascopeStats[igate].sumPowerVc += power0;
        _ascopeStats[igate].sumPowerHx += power1;
        _ascopeStats[igate].sumConjProdV =
          RadarComplex::complexSum(_ascopeStats[igate].sumConjProdV, prod);
      }

    }
    
  }

}

////////////////////////////////////////////
// augment max power information

void TsPrint::_addToMaxPower(const IwrfTsPulse &pulse)

{

  _saveMetaData(pulse);
  
  const fl32 *iqChan0 = pulse.getIq0();
  const fl32 *iqChan1 = pulse.getIq1();
  
  double maxPower0 = -9999;
  double maxPower1 = -9999;
  int gateForMax0 = 0;
  int gateForMax1 = 0;

  int index = _startGate * 2;
  for (int igate = _startGate; igate <= _endGate; igate++, index += 2) {
    double ii0 = iqChan0[index];
    double qq0 = iqChan0[index + 1];
    double power0 = ii0 * ii0 + qq0 * qq0;
    power0 /= _rxGainHc;
    if (power0 > maxPower0) {
      maxPower0 = power0;
      gateForMax0 = igate;
    }
    if (igate == _gateForMax0) {
      RadarComplex_t iq0(ii0, qq0);
      _iqForVelAtMaxPower0.push_back(iq0);
    }
    if (iqChan1) {
      double ii1 = iqChan1[index];
      double qq1 = iqChan1[index + 1];
      double power1 = ii1 * ii1 + qq1 * qq1;
      power1 /= _rxGainVc;
      if (power1 > maxPower1) {
        maxPower1 = power1;
        gateForMax1 = igate;
      }
      if (igate == _gateForMax1) {
        RadarComplex_t iq1(ii1, qq1);
        _iqForVelAtMaxPower1.push_back(iq1);
      }
    }
  } // igate

  _maxPowers0.push_back(maxPower0);
  _maxPowers1.push_back(maxPower1);

  _gatesForMax0.push_back(gateForMax0);
  _gatesForMax1.push_back(gateForMax1);

}

/////////////////////////////////////////
// Check if ops info has changed
// Returns true if changed, false if not

bool TsPrint::_checkInfoChanged(const IwrfTsPulse &pulse)

{
  
  const IwrfTsInfo &info = pulse.getTsInfo();
  const iwrf_scan_segment_t &scan = info.getScanSegment();
  const iwrf_ts_processing_t &proc = info.getTsProcessing();
  const iwrf_calibration_t &calib = info.getCalibration();

  bool changed = false;
  if (iwrf_compare(scan, _scanPrev)) {
    if (_params.print_info_on_change) {
      cout << "# scan info changed" << endl;
    }
    changed = true;
  }
  
  if (iwrf_compare(proc, _procPrev)) {
    if (_params.print_info_on_change) {
      cout << "# proc info changed" << endl;
    }
    changed = true;
  }
  
  if (iwrf_compare(calib, _calibPrev)) {
    if (_params.print_info_on_change) {
      cout << "# calibration changed" << endl;
    }
    changed = true;
  }

  _scanPrev = scan;
  _procPrev = proc;
  _calibPrev = calib;

  return changed;

}
  
//////////////////////////////////////////////////
// provide data to client

int TsPrint::_handleClient(Socket *sock)

{

  if (_params.debug) {
    cerr << "  Child - handling client ..." << endl;
  }

  // read commands coming in
  
  string xmlCommands;
  if (_readCommands(sock, xmlCommands)) {
    return -1;
  }
  _decodeCommands(xmlCommands);
  
  if (_params.debug) {
    cerr << "----------- Incoming XML commands --------------" << endl;
    cerr << xmlCommands << endl;
    cerr << "----------- Setting variables ------------------" << endl;
    cerr << "-->> nSamples: " << _nSamples << endl;
    cerr << "-->> nGatesRequested: " << _nGatesRequested << endl;
    cerr << "-->> startGateRequested: " << _startGateRequested << endl;
    cerr << "-->> dualChannel: " << _haveChan1 << endl;
    cerr << "-->> fastAlternating: " << _fastAlternating << endl;
    cerr << "-->> labviewRequest: " << _labviewRequest << endl;
    cerr << "------------------------------------------------" << endl;
  
  }

  // initialize
  
  _pulseCount = 0;
  _clearStats();

  // gather data

  int iret = 0;
  while (_pulseCount < _nSamples) {
    
    // read next pulse

    const IwrfTsPulse *pulse = _pulseReader->getNextPulse(true);
    if (pulse == NULL) {
      iret = -1;
      break;
    }
    if (pulse->getIq1() != NULL) {
      _haveChan1 = true;
    } else {
      _haveChan1 = false;
    }
    _conditionGateRange(*pulse);

    // add to data as appropriate
    
    if (_fastAlternating) {
      _addToAlternating(*pulse);
    } else {
      _addToSummary(*pulse);
    }
    _pulseCount++;
    _totalPulseCount++;

    // free up pulse memory

    delete pulse;
    
  } // while
      
  // compute stats
      
  _computeStats();
  
  // compile XML response

  string xmlResponse;

  if (_labviewRequest) {
    _prepareLabviewResponse(iret, xmlResponse);
  } else {
    _prepareXmlResponse(iret, xmlResponse);
  }
    
  // write response to client
    
  if (_writeResponse(sock, xmlResponse)) {
    return -1;
  }

  return iret;

}

//////////////////////////////////////////////////
// provide max power data to client

int TsPrint::_handleMaxPowerClient(Socket *sock)

{

  if (_params.debug) {
    cerr << "  Child - handling max power client ..." << endl;
  }

  while (true) {

    _initMaxPowerData();

    // accumulate data
    
    _pulseReader->seekToEnd();
          
    for (int ipulse = 0; ipulse < _nSamples; ipulse++) {
      
      // read next pulse
      
      IwrfTsPulse *pulse = _getNextPulse();
      if (pulse == NULL) {
        if (_pulseReader->endOfFile()) {
          return -1;
        }
        cerr << "ERROR - TsPrint::_runAscopeMode()" << endl;
        cerr << "  Cannot read enough samples for aScope" << endl;
        return -1;
      }

      // add to stats

      pulse->convertToFL32();
      _addToMaxPower(*pulse);
      _pulseCount++;
      _totalPulseCount++;
      
      delete pulse;
      
    } 

    // compute max power and range to max

    _computeMaxPowerData();
    
    // compile XML response
    
    string xmlResponse;
    _prepareMaxPowerResponse(xmlResponse);
    
    // write response to client
    
    if (_writeResponse(sock, xmlResponse)) {
      return -1;
    }

  } // while

  return 0;

}

//////////////////////////////////////////////////
// reap children from fork()s which have exited

void TsPrint::_reapChildren()

{
  
  pid_t dead_pid;
  int status;
  
  while ((dead_pid = waitpid((pid_t) -1,
                             &status,
                             (int) (WNOHANG | WUNTRACED))) > 0) {
    
    if (_params.debug) {
      cerr << "  child exited, pid: " << dead_pid << endl;
    }

  } // while

}

//////////////////////////////////////////////////
// Read incoming commands
//
// Returns 0 on success, -1 on failure

int TsPrint::_readCommands(Socket *sock, string &xmlBuf)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading commands" << endl;
  }

  string latestLine;

  while (true) {
    
    // check for available data
    
    int iret = sock->readSelect(100);
    if (iret != 0) {
      if (sock->getErrNum() == SockUtil::TIMED_OUT) {
        return 0;
      } else {
        cerr << "ERROR - _readCommands()" << endl;
        cerr << "  readSelect() failed" << endl;
        cerr << "  " << sock->getErrStr() << endl;
        return -1;
      }
    }

    // read all available data

    char cc;
    if (sock->readBuffer(&cc, 1, 100) == 0) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << cc;
      }
      xmlBuf += cc;
      latestLine += cc;
      if (latestLine.find("</TsPrintCommands>") != string::npos) {
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << endl;
	}
	return 0;
      }
      if (cc == '\n') {
	latestLine.clear();
      }
    } else {
      if (sock->getErrNum() != SockUtil::TIMED_OUT) {
        cerr << "ERROR - _readCommands()" << endl;
        cerr << "  readBuffer() failed" << endl;
        cerr << "  " << sock->getErrStr() << endl;
        return -1;
      }
    }

  }

  return -1;

}

//////////////////////////////////////////////////
// Decode incoming commands
//
// Returns 0 on success, -1 on failure

int TsPrint::_decodeCommands(string &xmlBuf)

{

  // decode commands

  int iret = 0;

  if (TaXml::readInt(xmlBuf, "nSamples", _nSamples)) {
    iret = -1;
  }
  if (TaXml::readInt(xmlBuf, "nGates", _nGatesRequested)) {
    iret = -1;
  };
  if (TaXml::readInt(xmlBuf, "startGate", _startGateRequested)) {
    iret = -1;
  };
  
  TaXml::readBoolean(xmlBuf, "dualChannel", _haveChan1);

  TaXml::readBoolean(xmlBuf, "fastAlternating", _fastAlternating);
  if (_fastAlternating) {
    _haveChan1 = true;
  }

  _labviewRequest = false;
  TaXml::readBoolean(xmlBuf, "labviewRequest", _labviewRequest);

  return iret;

}

//////////////////////////////////////////////////
// prepare normal XML response

void TsPrint::_prepareXmlResponse(int iret,
				  string &xmlResponse)
  
{
  
  xmlResponse += TaXml::writeStartTag("TsPrintResponse", 0);

  if (iret) {
    
    xmlResponse += TaXml::writeBoolean("success", 1, false);
    
  } else {
    
    xmlResponse += TaXml::writeBoolean("success", 1, true);
    
    // set response
    
    time_t midSecs = (time_t) _midTime;
    int midMSecs = (int) ((_midTime - midSecs) * 1000.0 + 0.5);
    double prf = 1.0 / _midPrt;
    
    xmlResponse += TaXml::writeTime("time", 1, midSecs);
    xmlResponse += TaXml::writeDouble("msecs", 1, midMSecs);
    xmlResponse += TaXml::writeDouble("prf", 1, prf);
    xmlResponse += TaXml::writeInt("nSamples", 1, _nSamples);
    xmlResponse += TaXml::writeInt("startGate", 1, _startGate);
    xmlResponse += TaXml::writeInt("nGates", 1, _nGates);
    xmlResponse += TaXml::writeDouble("el", 1, _midEl);
    xmlResponse += TaXml::writeDouble("az", 1, _midAz);
    xmlResponse += TaXml::writeString("transition", 1, (_midTransition?"true":"false"));
    xmlResponse += TaXml::writeDouble("dbm0", 1, _stats.meanDbm0);
    xmlResponse += TaXml::writeDouble("dbm1", 1, _stats.meanDbm1);
    
    if (_fastAlternating) {
      
      xmlResponse += TaXml::writeDouble("dbmHc", 1, _stats.meanDbmHc);
      xmlResponse += TaXml::writeDouble("dbmHx", 1, _stats.meanDbmHx);
      xmlResponse += TaXml::writeDouble("dbmVc", 1, _stats.meanDbmVc);
      xmlResponse += TaXml::writeDouble("dbmVx", 1, _stats.meanDbmVx);
      xmlResponse += TaXml::writeDouble("corrH", 1, _stats.corrH);
      xmlResponse += TaXml::writeDouble("argH", 1, _stats.argH);
      xmlResponse += TaXml::writeDouble("corrV", 1, _stats.corrV);
      xmlResponse += TaXml::writeDouble("argV", 1, _stats.argV);
      
    }
    
  }
  
  xmlResponse += TaXml::writeEndTag("TsPrintResponse", 0);

}

//////////////////////////////////////////////////
// prepare max power XML response

void TsPrint::_prepareMaxPowerResponse(string &xmlResponse)
  
{
  
  xmlResponse += TaXml::writeStartTag("TsPrintMaxPower", 0);

  // set response
  
  time_t midSecs = (time_t) _midTime;
  int midMSecs = (int) ((_midTime - midSecs) * 1000.0 + 0.5);
  double prf = 1.0 / _midPrt;

  double dwellSecs = (_endTime - _startTime) * ((double) _nSamples / ((double) _nSamples - 1.0));

  
  xmlResponse += TaXml::writeTime("time", 1, midSecs);
  xmlResponse += TaXml::writeDouble("msecs", 1, midMSecs);
  xmlResponse += TaXml::writeDouble("dwellSecs", 1, dwellSecs);
  xmlResponse += TaXml::writeDouble("prf", 1, prf);
  xmlResponse += TaXml::writeInt("nSamples", 1, _nSamples);
  xmlResponse += TaXml::writeInt("startGate", 1, _startGate);
  xmlResponse += TaXml::writeInt("nGates", 1, _nGates);
  xmlResponse += TaXml::writeDouble("el", 1, _midEl);
  xmlResponse += TaXml::writeDouble("az", 1, _midAz);
  xmlResponse += TaXml::writeDouble("meanMaxDbm0", 1, 10.0 * log10(_meanMaxPower0));
  xmlResponse += TaXml::writeDouble("meanMaxDbm1", 1, 10.0 * log10(_meanMaxPower1));
  xmlResponse += TaXml::writeDouble("peakMaxDbm0", 1, 10.0 * log10(_peakMaxPower0));
  xmlResponse += TaXml::writeDouble("peakMaxDbm1", 1, 10.0 * log10(_peakMaxPower1));
  xmlResponse += TaXml::writeDouble("rangeToMax0", 1, _rangeToMaxPower0);
  xmlResponse += TaXml::writeDouble("rangeToMax1", 1, _rangeToMaxPower1);
  
  xmlResponse += TaXml::writeEndTag("TsPrintMaxPower", 0);

  xmlResponse += "\r\n";

}

//////////////////////////////////////////////////
// prepare labview XML response

void TsPrint::_prepareLabviewResponse(int iret,
				      string &xmlResponse)
  
{
  
  xmlResponse += TaXml::writeStartTag("Cluster", 0);

  int numElts = 6;
  if (_fastAlternating) {
    numElts += 4;
  }

  double labviewSecs = (double) _midTime;
  DateTime ltime(1904, 1, 1, 12, 0, 0);
  labviewSecs += (-1.0 * ltime.utime());
  char timeStr[128];
  sprintf(timeStr, "%.10e", labviewSecs);

  xmlResponse += TaXml::writeString("Name", 1, "TS_power");
  xmlResponse += TaXml::writeInt("NumElts", 1, numElts);
  xmlResponse += TaXml::writeStartTag("Boolean", 1);
  xmlResponse += TaXml::writeString("Name", 2, "success");
  xmlResponse += TaXml::writeInt("Val", 2, (iret == 0));
  xmlResponse += TaXml::writeEndTag("Boolean", 1);

  xmlResponse += TaXml::writeStartTag("DBL", 1);
  xmlResponse += TaXml::writeString("Name", 2, "time");
  xmlResponse += TaXml::writeString("Val", 2, timeStr);
  xmlResponse += TaXml::writeEndTag("DBL", 1);
  
  xmlResponse += TaXml::writeStartTag("DBL", 1);
  xmlResponse += TaXml::writeString("Name", 2, "el");
  xmlResponse += TaXml::writeDouble("Val", 2, _midEl);
  xmlResponse += TaXml::writeEndTag("DBL", 1);
  
  xmlResponse += TaXml::writeStartTag("DBL", 1);
  xmlResponse += TaXml::writeString("Name", 2, "az");
  xmlResponse += TaXml::writeDouble("Val", 2, _midAz);
  xmlResponse += TaXml::writeEndTag("DBL", 1);
  
  xmlResponse += TaXml::writeStartTag("DBL", 1);
  xmlResponse += TaXml::writeString("Name", 2, "dbm0");
  xmlResponse += TaXml::writeDouble("Val", 2, _stats.meanDbm0);
  xmlResponse += TaXml::writeEndTag("DBL", 1);
  
  xmlResponse += TaXml::writeStartTag("DBL", 1);
  xmlResponse += TaXml::writeString("Name", 2, "dbm1");
  xmlResponse += TaXml::writeDouble("Val", 2, _stats.meanDbm1);
  xmlResponse += TaXml::writeEndTag("DBL", 1);
  
  if (_fastAlternating) {
    
    xmlResponse += TaXml::writeStartTag("DBL", 1);
    xmlResponse += TaXml::writeString("Name", 2, "dbmHc");
    xmlResponse += TaXml::writeDouble("Val", 2, _stats.meanDbmHc);
    xmlResponse += TaXml::writeEndTag("DBL", 1);
    
    xmlResponse += TaXml::writeStartTag("DBL", 1);
    xmlResponse += TaXml::writeString("Name", 2, "dbmHx");
    xmlResponse += TaXml::writeDouble("Val", 2, _stats.meanDbmHx);
    xmlResponse += TaXml::writeEndTag("DBL", 1);
    
    xmlResponse += TaXml::writeStartTag("DBL", 1);
    xmlResponse += TaXml::writeString("Name", 2, "dbmVc");
    xmlResponse += TaXml::writeDouble("Val", 2, _stats.meanDbmVc);
    xmlResponse += TaXml::writeEndTag("DBL", 1);
    
    xmlResponse += TaXml::writeStartTag("DBL", 1);
    xmlResponse += TaXml::writeString("Name", 2, "dbmVx");
    xmlResponse += TaXml::writeDouble("Val", 2, _stats.meanDbmVx);
    xmlResponse += TaXml::writeEndTag("DBL", 1);
    
  }
    
  xmlResponse += TaXml::writeEndTag("Cluster", 0);

}

//////////////////////////////////////////////////
// write response to client
//
// Returns 0 on success, -1 on failure

int TsPrint::_writeResponse(Socket *sock, string &xmlBuf)

{
  
  // write xml buffer
  
  if (sock->writeBuffer((void *) xmlBuf.c_str(), xmlBuf.size() + 1)) {
    cerr << "ERROR - writeResponse" << endl;
    cerr << "  Error writing response to client" << endl;
    cerr << "  " << sock->getErrStr() << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////
// print the info about this run

void TsPrint::_printRunDetails(ostream &out)

{

  // get time

  DateTime now(time(NULL));
  
  out << "# RUN INFO: "
      << "year,month,day,hour,min,sec,nSamples,startGate,nGates"
      << endl;

  out << "# ";
  out << setfill('0');
  out << setw(4) << now.getYear() << ","
      << setw(2) << now.getMonth() << ","
      << setw(2) << now.getDay() << ","
      << setw(2) << now.getHour() << ","
      << setw(2) << now.getMin() << ","
      << setw(2) << now.getSec() << ","
      << _nSamples << ","
      << _startGate << ","
      << _nGates << endl;
  out << setfill(' ');
  
}

////////////////////////
// print the ops info

void TsPrint::_printOpsInfo(ostream &out, const IwrfTsPulse *pulse)

{
  
  out << "# OPS INFO: "
      << "siteName,radarName,"
      << "alt,lat,lon,"
      << "startRange,gateSpacing,"
      << "scanMode,xmitRcvMode,prfMode,xmitPhaseMode,"
      << "wavelenCm,beamWidthH,beamWidthV"
      << endl;

  const IwrfTsInfo &info = _pulseReader->getOpsInfo();
  
  out << "# ";
  out << info.get_radar_site_name() << ","
      << info.get_radar_name() << ","
      << info.get_radar_altitude_m() << ","
      << info.get_radar_latitude_deg() << ","
      << info.get_radar_longitude_deg() << ","
      << info.get_proc_start_range_m() << ","
      << info.get_proc_gate_spacing_m() << ","
      << info.get_scan_mode() << ","
      << info.get_proc_xmit_rcv_mode() << ","
      << info.get_proc_prf_mode() << ","
      << info.get_proc_xmit_phase_mode() << ","
      << info.get_radar_wavelength_cm() << ","
      << info.get_radar_beamwidth_deg_h() << ","
      << info.get_radar_beamwidth_deg_v() << endl;

  if (_params.print_packing) {
    const iwrf_pulse_header_t &hdr = pulse->getHdr();
    out << "# IQ packing: "
        << iwrf_iq_encoding_to_str(hdr.iq_encoding) << endl;
  }
  
}

/////////////////////////////////
// print the pulse labels

void TsPrint::_printPulseLabels(ostream &out)
  
{

  out << "# PULSE HEADER: ";
  out << "seqNum,nGates,gateNum,time,prt,vol,tilt,el,az,hvFlag,burstMag";
  out << ",iChan0,qChan0,power0,iChan1,qChan1,power1";

  out << endl;

}

////////////////////////////////////////////
// load up the text for this pulse

string TsPrint::_pulseString(const IwrfTsPulse &pulse)

{

  string str;
  
  // compute properties

  double ptime = pulse.getFTime();
  double prt = pulse.getPrt();
  
  double el = pulse.getEl();
  double az = pulse.getAz();
  int volNum = pulse.getVolNum();
  int sweepNum = pulse.getSweepNum();
  double burstMag = pulse.getBurstIq0()[0];
  // double phaseDiff0 = pulse.getPhaseDiff0();
  bool horiz = pulse.isHoriz();
  
  int nGates = pulse.getNGates();
  si64 pulseSeqNum = pulse.getSeqNum();
  int gateNum = _startGate;
  if (gateNum > nGates - 1) {
    cerr << "ERROR - gate number too high: " << gateNum << endl;
    cerr << "  Using last gate, number: " << nGates - 1 << endl;
    gateNum = nGates - 1;
  }

  char text[4096];
  int index = gateNum * 2;
  
  snprintf(text, 4096,
          "%lu,%d,%d"
          ",%.3f,%.5f,%d,%d,%.2f,%.2f"
          ",%d,%.3e",
          pulseSeqNum, nGates, gateNum,
          ptime, prt, volNum, sweepNum, el, az,
          (int) horiz, burstMag);
  
  str += text;
  
  double ii0 = pulse.getIq0()[index];
  double qq0 = pulse.getIq0()[index + 1];
  sprintf(text, ",%.3e,%.3e", ii0, qq0);
  str += text;
  double power0 = ii0 * ii0 + qq0 * qq0;
  power0 /= _rxGainHc;
  double dbm0 = 10.0 * log10(power0);
  sprintf(text, ",%.2f", dbm0);
  str += text;

  if (pulse.getIq1() != NULL) {
    double ii1 = pulse.getIq1()[index];
    double qq1 = pulse.getIq1()[index + 1];
    sprintf(text, ",%.3e,%.3e", ii1, qq1);
    str += text;
    double power1 = ii1 * ii1 + qq1 * qq1;
    power0 /= _rxGainVc;
    double dbm1 = 10.0 * log10(power1);
    sprintf(text, ",%.2f", dbm1);
    str += text;
  } else {
    str += ",-999,-999,-999";
  }
  
  return str;

}

/////////////////////////////////
// print summary heading labels

void TsPrint::_printSummaryHeading(ostream &out)
  
{

  if (_fastAlternating) {
    out << "# ALTERNATING MODE:" << endl;
  } else if (_haveChan1) {
    out << "# DUAL CHANNEL MODE:" << endl;
  } else {
    out << "# SUMMARY MODE:" << endl;
  }

  out << "#   Start gate: " << _startGate << endl;
  out << "#   N gates: " << _nGates << endl;
  out << "#   N samples: " << _nSamples << endl;

}

/////////////////////////////////
// print summary labels

void TsPrint::_printSummaryLabels(ostream &out)
  
{

  if (_fastAlternating) {
    out << "#                  "
        << "time           prf      el      az"
        << "       Hc       Hx    Hcorr     Harg"
        << "       Vc       Vx    Vcorr     Varg"
        << "     IFD0     IFD1";
  } else if (_haveChan1) {
    out << "#                  "
        << "time           prf      el      az"
        << "       Hc    Hcorr     Harg"
        << "       Vc    Vcorr     Varg"
        << "     IFD0     IFD1";
  } else {
    out << "#                  "
        << "time           prf      el      az     IFD0";
  }
  
  if (_params.print_lag1_coherent_power) {
    if (_fastAlternating) {
      out << "   Lag1Hc   Lag1Hx   Lag1Vc   Lag1Vx";
    } else if (_haveChan1) {
      out << "   Lag1Hc   Lag1Vc";
    } else {
      out << "   Lag1Hc";
    }
  }

  for (size_t ii = 0; ii < _extraColLabels.size(); ii++) {
    out << setw(12) << _extraColLabels[ii];
  }

  out << endl;

}

/////////////////////////////////
// print summary data

void TsPrint::_printSummaryData(FILE *out)
  
{

  if (_params.debug) {
    fprintf(out, "Current time: %s\n", DateTime::str().c_str());
  }

  time_t midSecs = (time_t) _midTime;
  int midNanoSecs = (int) ((_midTime - midSecs) * 1.0e9 + 0.5);
  double prf = 1.0 / _midPrt;

  fprintf(out,
          "%s.%.9d %7.1f %7.2f %7.2f",
          DateTime::stru(midSecs).c_str(),
          midNanoSecs,
          prf, _midEl, _midAz);

  if (_fastAlternating) {
    fprintf(out,
            " %8.3f %8.3f %8.3f %8.3f "
            "%8.3f %8.3f %8.3f %8.3f "
            "%8.3f %8.3f",
            _stats.meanDbmHc,
            _stats.meanDbmHx,
            _stats.corrH,
            _stats.argH,
            _stats.meanDbmVc,
            _stats.meanDbmVx,
            _stats.corrV,
            _stats.argV,
            _stats.meanDbm0,
            _stats.meanDbm1);
  } else if (_haveChan1) {
    fprintf(out,
            " %8.3f %8.3f %8.3f "
            "%8.3f %8.3f %8.3f "
            "%8.3f %8.3f",
            _stats.meanDbmHc,
            _stats.corrH,
            _stats.argH,
            _stats.meanDbmVc,
            _stats.corrV,
            _stats.argV,
            _stats.meanDbm0,
            _stats.meanDbm1);
  } else {
    fprintf(out, " %8.3f",
            _stats.meanDbm0);
  }
  
  if (_params.print_lag1_coherent_power) {
    if (_fastAlternating) {
      fprintf(out,
              " %8.3f %8.3f %8.3f %8.3f",
              _stats.lag1DbmHc,
              _stats.lag1DbmHx,
              _stats.lag1DbmVc,
              _stats.lag1DbmVx);
    } else if (_haveChan1) {
      fprintf(out,
              " %8.3f %8.3f",
              _stats.lag1DbmHc,
              _stats.lag1DbmVc);
    } else {
      fprintf(out,
              " %8.3f",
              _stats.lag1DbmHc);
    }
  }
  
  for (size_t ii = 0; ii < _extraColMeans.size(); ii++) {
    fprintf(out, "%12g", _extraColMeans[ii]);
  }

  if (_nSamples == 1 && _fastAlternating) {
    if (_latestIsH) {
      fprintf(out, " H");
    } else {
      fprintf(out, " V");
    }
  }

  if (_midTransition) {
    fprintf(out, " *");
  }
  
  fprintf(out, "\n");

}

/////////////////////////////////////
// print max power heading

void TsPrint::_printMaxPowerHeading(ostream &out)
  
{

  out << "# MAX POWER MODE:" << endl;
  out << "#   Start gate: " << _startGate << endl;
  out << "#   N gates: " << _nGates << endl;
  out << "#   N samples: " << _nSamples << endl;

}

/////////////////////////////////
// print max power labels

void TsPrint::_printMaxPowerLabels(ostream &out)
  
{
  
  if (_haveChan1) {
    if (_params.distance_units == Params::DISTANCE_IN_METERS) {
      out << "#                  "
          << "time     prf      el       az maxDbm0  range0(m) maxDbm1  range1(m)";
    } else {
      out << "#                  "
          << "time     prf      el       az maxDbm0 range0(ft) maxDbm1 range1(ft)";
    }
  } else {
    if (_params.distance_units == Params::DISTANCE_IN_METERS) {
      out << "#                  "
          << "time     prf      el       az maxDbm0  range0(m)";
    } else {
      out << "#                  "
          << "time     prf      el       az maxDbm0 range0(ft)";
    }
  }

  if (_params.print_radial_velocity) {
    out << "    vel0";
    if (_haveChan1) {
      out << "    vel1";
    }
  }

  if (_params.print_lat_lon) {
    out << "        lat        lon";
  }

  if (_params.print_altitude) {
    if (_params.distance_units == Params::DISTANCE_IN_METERS) {
      out << "    alt(m)";
    } else {
      out << "   alt(ft)";
    }
  }

  out << endl;

  fflush(stdout);

}

/////////////////////////////////
// initialize max power data

void TsPrint::_initMaxPowerData()
  
{
  
  _pulseCount = 0;

  _meanMaxPower0 = -9999.0;
  _meanMaxPower1 = -9999.0;
  _peakMaxPower0 = -9999.0;
  _peakMaxPower1 = -9999.0;
  _maxPowers0.clear();
  _maxPowers1.clear();
  
  _gatesForMax0.clear();
  _gatesForMax1.clear();

  _rangeToMaxPower0 = -9999;
  _rangeToMaxPower1 = -9999;

  _iqForVelAtMaxPower0.clear();
  _iqForVelAtMaxPower1.clear();

  _velAtMaxPower0 = -9999;
  _velAtMaxPower1 = -9999;

}

/////////////////////////////////
// compute max power data

void TsPrint::_computeMaxPowerData()
  
{
  
  // compute the mean and peak of the max power values for each channel
  // along with the mean range to max power

  if (_maxPowers0.size() > 0) {
    _peakMaxPower0 = -9999.0;
    double sumMaxPower0 = 0.0;
    for (size_t ii = 0; ii < _maxPowers0.size(); ii++) {
      double maxPower0 = _maxPowers0[ii];
      sumMaxPower0 += maxPower0;
      if (maxPower0 > _peakMaxPower0) {
        _peakMaxPower0 = maxPower0;
      }
    }
    _meanMaxPower0 = sumMaxPower0 / (double) _maxPowers0.size();
  }

  if (_maxPowers1.size() > 0) {
    _peakMaxPower1 = -9999.0;
    double sumMaxPower1 = 0.0;
    for (size_t ii = 0; ii < _maxPowers1.size(); ii++) {
      double maxPower1 = _maxPowers1[ii];
      sumMaxPower1 += maxPower1;
      if (maxPower1 > _peakMaxPower1) {
        _peakMaxPower1 = maxPower1;
      }
    }
    _meanMaxPower1 = sumMaxPower1 / (double) _maxPowers1.size();
  }

  if (_gatesForMax0.size() > 0) {
    double sumGateForMax0 = 0.0;
    for (size_t ii = 0; ii < _gatesForMax0.size(); ii++) {
      sumGateForMax0 += _gatesForMax0[ii];
    }
    double meanGateForMax0 = sumGateForMax0 / (double) _gatesForMax0.size();
    _rangeToMaxPower0 = meanGateForMax0 * _gateSpacingM + _startRangeM;
    _gateForMax0 = (int) floor(meanGateForMax0 + 0.5);
  }

  if (_gatesForMax1.size() > 0) {
    double sumGateForMax1 = 0.0;
    for (size_t ii = 0; ii < _gatesForMax1.size(); ii++) {
      sumGateForMax1 += _gatesForMax1[ii];
    }
    double meanGateForMax1 = sumGateForMax1 / (double) _gatesForMax1.size();
    _rangeToMaxPower1 = meanGateForMax1 * _gateSpacingM + _startRangeM;
    _gateForMax1 = (int) floor(meanGateForMax1 + 0.5);
  }

  if (_params.distance_units == Params::DISTANCE_IN_FEET) {
    _rangeToMaxPower0 /= M_PER_FT;
    _rangeToMaxPower1 /= M_PER_FT;
  }

  if (_iqForVelAtMaxPower0.size() > 0) {
    _velAtMaxPower0 = _computeVel(_iqForVelAtMaxPower0);
  }
  if (_iqForVelAtMaxPower1.size() > 0) {
    _velAtMaxPower1 = _computeVel(_iqForVelAtMaxPower1);
  }

}

/////////////////////////////////
// compute vel at max power gate

double TsPrint::_computeVel(const vector<RadarComplex_t> &iq)
  
{

  if ((int) iq.size() != _nSamples) {
    return -9999.0;
  }

  // put IQ data into array form

  TaArray<RadarComplex_t> IQ_;
  RadarComplex_t *IQ = IQ_.alloc(_nSamples);
  for (int ii = 0; ii < _nSamples; ii++) {
    IQ[ii] = iq[ii];
  }

  // compute lag1 covariance

  RadarComplex_t lag1 =
    RadarComplex::meanConjugateProduct(IQ + 1, IQ, _nSamples - 1);

  // compute phase

  double argVel = RadarComplex::argRad(lag1);

  // get nyquist
  
  const IwrfTsInfo &info = _pulseReader->getOpsInfo();
  double wavelengthMeters = info.get_radar_wavelength_cm() / 1.0e2;
  double prt = info.get_proc_prt_usec() / 1.0e6;
  double nyquist = ((wavelengthMeters / prt) / 4.0);

  // compute velocity
  
  double vel = (argVel / M_PI) * nyquist * -1.0;

  // correct for platform motion
  
  if (_params.use_secondary_georeference) {
    if (info.isPlatformGeoref1Active()) {
      const iwrf_platform_georef_t &georef1 = info.getPlatformGeoref1();
      if (georef1.vert_velocity_mps > -9990) {
        double correction = georef1.vert_velocity_mps * sin(_midEl * DEG_TO_RAD);
        vel += correction;
        // check nyquist interval
        while (vel > nyquist) {
          vel -= 2.0 * nyquist;
        }
        while (vel < -nyquist) {
          vel += 2.0 * nyquist;
        }
        return vel;
      }
    }
  }

  if (info.isPlatformGeorefActive()) {
    const iwrf_platform_georef_t &georef = info.getPlatformGeoref();
    if (georef.vert_velocity_mps > -9990) {
      double correction = georef.vert_velocity_mps * sin(_midEl * DEG_TO_RAD);
      vel += correction;
      // check nyquist interval
      while (vel > nyquist) {
        vel -= 2.0 * nyquist;
      }
      while (vel < -nyquist) {
        vel += 2.0 * nyquist;
      }
    }
  }

  return vel;

}

  
/////////////////////////////////
// print max power data

void TsPrint::_printMaxPowerData(FILE *out)
  
{
  
  time_t midSecs = (time_t) _midTime;
  int midMilliSecs = (int) ((_midTime - midSecs) * 1.0e3 + 0.5);
  double prf = 1.0 / _midPrt;

  fprintf(out, "%s.%.3d %7.1f %7.2f %8.2f %7.2f %10.0f",
          DateTime::stru(midSecs).c_str(),
          midMilliSecs,
          prf,
          _midEl, _midAz,
          10.0 * log10(_meanMaxPower0),
          _rangeToMaxPower0);
  if (_haveChan1) {
    fprintf(out, " %7.2f %10.0f",
            10.0 * log10(_meanMaxPower1),
            _rangeToMaxPower1);
  }

  if (_params.print_radial_velocity) {
    fprintf(out, " %7.2f", _velAtMaxPower0);
    if (_haveChan1) {
      fprintf(out, " %7.2f", _velAtMaxPower1);
    }
  }
  
  const IwrfTsInfo &info = _pulseReader->getOpsInfo();
  double lat = info.get_radar_latitude_deg();
  double lon = info.get_radar_longitude_deg();
  double alt = info.get_radar_altitude_m();
  if (_params.use_secondary_georeference &&
      info.isPlatformGeoref1Active()) {
    const iwrf_platform_georef_t &georef1 = info.getPlatformGeoref();
      lat = georef1.latitude;
      lon = georef1.longitude;
      alt = georef1.altitude_msl_km * 1000.0;
  } else if (info.isPlatformGeorefActive()) {
    const iwrf_platform_georef_t &georef = info.getPlatformGeoref();
    lat = georef.latitude;
    lon = georef.longitude;
    alt = georef.altitude_msl_km * 1000.0;
  }

  if (_params.distance_units == Params::DISTANCE_IN_FEET) {
    alt /= M_PER_FT;
  }

  if (_params.print_lat_lon) {
    fprintf(out, " %10.5f %11.5f", lat, lon);
  }

  if (_params.print_altitude) {
    fprintf(out, " %8.0f", alt);
  }

  fprintf(out, "\n");

}


////////////////////////////////////////////
// check for angle changes

void TsPrint::_checkAngleChange(const IwrfTsPulse &pulse)

{

  if (_prevElevation < -9990 && _prevAzimuth < -9990) {
    _prevElevation = pulse.getEl();
    _prevAzimuth = pulse.getAz();
    return;
  }

  double el = pulse.getEl();
  double az = pulse.getAz();

  double deltaEl = fabs(el - _prevElevation);
  if (deltaEl > 180) {
    deltaEl = fabs(deltaEl - 360.0);
  }

  double deltaAz = fabs(az - _prevAzimuth);
  if (deltaAz > 180) {
    deltaAz = fabs(deltaAz - 360.0);
  }

  if (deltaEl > _params.max_angle_change) {
    cerr << "WARNING - large elevation change: " << deltaEl << endl;
    cerr << "  ====>> prev elevation: " << _prevElevation << endl;
    cerr << "  ====>> this elevation: " << el << endl;
  }

  if (deltaAz > _params.max_angle_change) {
    cerr << "WARNING - large azimuth change: " << deltaAz << endl;
    cerr << "  ====>> prev azimuth: " << _prevAzimuth << endl;
    cerr << "  ====>> this azimuth: " << az << endl;
  }

  _prevElevation = pulse.getEl();
  _prevAzimuth = pulse.getAz();

}
    
/////////////////////////////////////////////////////////////
// init computations for extra columns

void TsPrint::_initExtraCols()

{

  _extraColLabels.clear();
  _extraColMeans.clear();
  _extraColSums.clear();
  _extraColCounts.clear();
  _statusXmlPktSeqNum = -1;

  for (int ii = 0; ii < _params.xml_entries_for_extra_cols_n; ii++) {
    
    const Params::status_xml_entry_t *entry =
      _params._xml_entries_for_extra_cols + ii;
    
    // add the column label
    
    _extraColLabels.push_back(entry->col_label);
    _extraColMeans.push_back(-9999.0);
    _extraColSums.push_back(0.0);
    _extraColCounts.push_back(0.0);

  }


}

/////////////////////////////////////////////////////////////
// decode the XML status block, to get data for extra columns

void TsPrint::_decodeXmlForExtraCols()

{

  // check we have new xml status

  si64 statusXmlPktSeqNum = _pulseReader->getOpsInfo().getStatusXmlPktSeqNum();
  if (statusXmlPktSeqNum == _statusXmlPktSeqNum) {
    // have already used this value
    return;
  }
  _statusXmlPktSeqNum = statusXmlPktSeqNum;

  string xml = _pulseReader->getOpsInfo().getStatusXmlStr();

  for (int ii = 0; ii < _params.xml_entries_for_extra_cols_n; ii++) {

    const Params::status_xml_entry_t *entry =
      _params._xml_entries_for_extra_cols + ii;
    
    // get tags in list
    
    string tagList = entry->xml_tag_list;
    vector<string> tags;
    TaStr::tokenize(tagList, "<>", tags);
    if (tags.size() == 0) {
      // tags not found
      cerr << "ERROR - TsPrint::_decodeXmlForExtraCols()" << endl;
      cerr << "  Cannot find XML tags in string: " << entry->xml_tag_list << endl;
      continue;
    }
    
    // read through the outer tags in status XML
    
    string valStr(xml);
    for (size_t jj = 0; jj < tags.size(); jj++) {
      string tmp;
      if (TaXml::readString(valStr, tags[jj], tmp)) {
        // tags not found
        continue;
      }
      valStr = tmp;
    }

    if (valStr.size() < 1) {
      continue;
    }

    // add to sum and count

    _extraColSums[ii] += atof(valStr.c_str());
    _extraColCounts[ii] += 1.0;

  } // ii

}

/////////////////////////////////////////////////////////////
// compute means for extra columns

void TsPrint::_computeExtraColMeans()

{

  for (int ii = 0; ii < _params.xml_entries_for_extra_cols_n; ii++) {
    if (_extraColCounts[ii] > 0) {
      _extraColMeans[ii] = _extraColSums[ii] / _extraColCounts[ii];
    } else {
      _extraColMeans[ii] = -9999.0;
    }
  }

}

