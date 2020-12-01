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
// IpsTsPrint.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////
//
// Support for Independent Pulse Sampling.
//
// IpsTsPrint reads ips time-series data, processes it
// in various ways, and prints out the results
//
////////////////////////////////////////////////////////////////

#include "IpsTsPrint.hh"
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
#include <toolsa/TaArray.hh>
#include <toolsa/pmu.h>
#include <toolsa/ServerSocket.hh>
#include <radar/RadarComplex.hh>
#include <radar/ips_ts_data.h>

using namespace std;

// Constructor

IpsTsPrint::IpsTsPrint(int argc, char **argv)
  
{

  isOK = true;
  _pulseCount = 0;
  _totalPulseCount = 0;
  _printCount = 0;
  _pulseReader = NULL;
  _haveChan1 = false;
  _prevPulseSeqNum = 0;

  _prevAzimuth = -9999.0;
  _prevElevation = -9999.0;

  MEM_zero(_scanPrev);
  MEM_zero(_procPrev);
  MEM_zero(_calibPrev);
  _infoChanged = false;

  _gateForMax0 = 0;
  _gateForMax1 = 0;

  // set programe name
  
  _progName = "IpsTsPrint";

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

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _printStructSizes(cerr);
  }
  
  // create the pulse reader
  
  IpsTsDebug_t ipsDebug = IpsTsDebug_t::OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    ipsDebug = IpsTsDebug_t::EXTRAVERBOSE;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    ipsDebug = IpsTsDebug_t::VERBOSE;
  } else if (_params.debug) {
    ipsDebug = IpsTsDebug_t::NORM;
  } 
    
  if (_params.input_mode == Params::TS_FMQ_INPUT) {
    _pulseReader = new IpsTsReaderFmq(_params.input_fmq_name,
				       ipsDebug,
				       !_params.seek_to_end_of_input);
  } else if (_params.input_mode == Params::TS_TCP_INPUT) {
    _pulseReader = new IpsTsReaderTcp(_params.tcp_server_host,
                                       _params.tcp_server_port,
				       ipsDebug);
  } else {
    _pulseReader = new IpsTsReaderFile(_args.inputFileList, ipsDebug);
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
  _fastAlternating = _params.fast_alternating;
  _dualChannel = _params.dual_channel;
  if (_fastAlternating) {
    _dualChannel = true;
  }

}

// destructor

IpsTsPrint::~IpsTsPrint()

{

  if (_pulseReader) {
    delete _pulseReader;
  }

  _aStats.clear();

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int IpsTsPrint::Run ()
{

  if (_params.print_format) {
    ips_ts_print_all_formats(stdout);
    return 0;
  }

  if (_params.run_mode == Params::CAL_MODE) {

    return _runCalMode();
    
  } else if (_params.run_mode == Params::ASCOPE_MODE) {
    
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

int IpsTsPrint::_runPrintMode()
  
{

  while (true) {
    
    // read next pulse
    
    IpsTsPulse *pulse = _getNextPulse();
    if (pulse == NULL) {
      return 0;
    }

    // at start, print headers
    
    if (_totalPulseCount == 0) {
      _printRunDetails(cout);
      _printOpsInfo(cout, pulse);
      if (!_params.print_all_headers) {
        if (_fastAlternating) {
          _printAlternatingHeading(cout);
        } else if (_dualChannel) {
          _printDualHeading(cout);
        } else {
          _printSummaryHeading(cout);
        }
      }
    }

    // all headers and data?

    if (_params.print_all_headers || _params.print_meta_headers) {
      _pulseReader->getOpsInfo().printMetaQueue(stdout, true);
    }

    if (_params.print_all_headers && !_params.print_iq_data) {
      pulse->printHeader(stdout);
    }
    if (_params.print_iq_data) {
      pulse->printData(stdout, _startGate, _endGate);
    }
    
    // ops info when it changes

    _infoChanged = _checkInfoChanged(*pulse);
    if (_infoChanged) {
      _printOpsInfo(cout, pulse);
      if (_params.print_info_on_change) {
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
    if (_fastAlternating) {
      _addToAlternating(*pulse);
    } else if (_dualChannel) {
      _addToDual(*pulse);
    } else {
      _addToSummary(*pulse);
    }
    
    _pulseCount++;
    _totalPulseCount++;
    
    if (_pulseCount == _nSamples) {
      // compute stats
      if (_fastAlternating) {
        _computeAlternating();
      } else if (_dualChannel) {
        _computeDual();
      } else {
        _computeSummary();
      }
      if (!_params.print_all_headers) {
        if (_fastAlternating) {
          if ((_printCount % _params.label_interval) == 0) {
            _printAlternatingLabels(cout);
          }
          _printAlternatingData(stdout);
        } else if (_dualChannel) {
          if ((_printCount % _params.label_interval) == 0) {
            _printDualLabels(cout);
          }
          _printDualData(stdout);
        } else {
          if ((_printCount % _params.label_interval) == 0) {
            _printSummaryLabels(cout);
          }
          _printSummaryData(stdout);
        }
        _printCount++;
      }
      _pulseCount = 0;
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

int IpsTsPrint::_printPrtDetails()
  
{

  ips_ts_pulse_header prevPulseHdr;
  memset(&prevPulseHdr, 0, sizeof(prevPulseHdr));

  while (true) {
    
    // read next pulse
    
    IpsTsPulse *pulse = _getNextPulse();
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
    
    ips_ts_pulse_header pulseHdr = pulse->getHdr();

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
            DateTime::strm(pulseTime.utime()).c_str(), 
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

int IpsTsPrint::_runAscopeMode()

{

  // allocate vector for Ascope stats

  _aStats.clear();
  for (int ii = 0; ii < _nGatesRequested; ii++) {
    Stats stats;
    stats.setCalibration(_rxGainHc,
                         _rxGainVc,
                         _rxGainHx,
                         _rxGainVx);
    _aStats.push_back(stats);
  }
  _pulseCount = 0;

  for (int ipulse = 0; ipulse < _nSamples; ipulse++) {

    // read next pulse

    IpsTsPulse *pulse = _getNextPulse();
    if (pulse == NULL) {
      if (_pulseReader->endOfFile()) {
        return 0;
      }
      cerr << "ERROR - IpsTsPrint::_runAscopeMode()" << endl;
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
    _aStats[igate].computeAlternating(_haveChan1);
  }
    
  time_t midSecs = (time_t) _midTime;
  int midNanoSecs = (int) ((_midTime - midSecs) * 1.0e9 + 0.5);
  double prf = 1.0 / _midPrt;

  // print
  
  cout << "# ASCOPE MODE:" << endl;
  cout << "#   Start gate: " << _startGate << endl;
  cout << "#   N gates: " << _nGates << endl;
  cout << "#   N samples: " << _nSamples << endl;
  cout << "#   time: " << DateTime::strm(midSecs) << "."
       << midNanoSecs << endl;
  cout << "#   PRF: " << prf << endl;
  cout << "#   EL (deg): " << _midEl << endl;
  cout << "#   AZ (deg): " << _midAz << endl;

  cout << "#   Gate num  RangeKm"
       << "       Hc       Hx    Hcorr     Harg"
       << "       Vc       Vx    Vcorr     Varg"
       << "     IFD0     IFD1"
       << endl;

  const IpsTsInfo &info = _pulseReader->getOpsInfo();
  double startRangeKm = info.getProcStartRangeKm();
  double gateSpacingKm = info.getProcGateSpacingKm();

  double rangeKm = startRangeKm + _startGate * gateSpacingKm;
  for (int igate = 0; igate < _nGates; igate++, rangeKm += gateSpacingKm) {

    int gateNum = igate + _startGate;

    fprintf(stdout, "%12d %8.3f "
            "%8.3f %8.3f %8.3f %8.3f "
            "%8.3f %8.3f %8.3f %8.3f "
            "%8.3f %8.3f\n",
            gateNum, rangeKm,
            _aStats[igate].meanDbmHc,
            _aStats[igate].meanDbmHx,
            _aStats[igate].corrH,
            _aStats[igate].argH,
            _aStats[igate].meanDbmVc,
            _aStats[igate].meanDbmVx,
            _aStats[igate].corrV,
            _aStats[igate].argV,
            _aStats[igate].meanDbm0,
            _aStats[igate].meanDbm1);

  }

  return 0;

}

//////////////////////////////////////////////////
// Run in calibration mode

int IpsTsPrint::_runCalMode()
{

  // get cal name

  fprintf(stdout, "Running IpsTsPrint in calibration mode\n");
  fprintf(stdout, "=====================================\n");
  fprintf(stdout, "Enter cal name: ");
  fflush(stdout);
  char calName[1024];
  if (fscanf(stdin, "%s", calName) != 1) {
    cerr << "ERROR - you must input a cal name for the output file." << endl;
    return -1;
  }

  // compute output file name

  DateTime now(time(NULL));
  char fileName[2048];
  snprintf(fileName, 2048,
           "tscal_%.4d%.2d%.2d_%.2d%.2d%2d_%s.txt",
           now.getYear(), now.getMonth(), now.getDay(),
           now.getHour(), now.getMin(), now.getSec(),
           calName);

  // open file

  FILE *calFile;
  if ((calFile = fopen(fileName, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - cannot open output file: " << fileName << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  fprintf(calFile, "#%10s %10s %10s %10s %10s %10s %10s\n",
	  "sgdBm", "Hc", "Hx", "Vc", "Vx", "Hc-Vc", "Hx-Vx");
  
  while (true) {

    fprintf(stdout, "Siggen setting in dBm, (q to quit): ");
    fflush(stdout);
    char input[1024];
    if (fscanf(stdin, "%s", input) != 1) {
      cerr << "ERROR - try again ..." << endl;
      continue;
    }
    if (input[0] == 'q') {
      break;
    }
    double sgSetting;
    if (sscanf(input, "%lg", &sgSetting) != 1) {
      cerr << "ERROR - enter a number ..." << endl;
      continue;
    }
    
    _clearStats();
    _pulseCount = 0;

    while (_pulseCount < _nSamples) {
      
      // read next pulse
      
      IpsTsPulse *pulse = _getNextPulse();
      if (pulse == NULL) {
        return -1;
      }

      _addToAlternating(*pulse);
      _pulseCount++;
      delete pulse;
    }
    
    _computeAlternating();
    _printAlternatingLabels(cout);
    _printAlternatingData(stdout);

    fprintf(calFile, " %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f\n",
	    sgSetting,
	    _stats.meanDbmHc, _stats.meanDbmHx,
            _stats.meanDbmVc, _stats.meanDbmVx,
	    _stats.meanDbmHc - _stats.meanDbmVc,
	    _stats.meanDbmHx - _stats.meanDbmVx);
    
  } // while (true)
  
  fclose(calFile);
  
  return 0;

}

//////////////////////////////////////////////////
// Run in server mode

int IpsTsPrint::_runServerMode()
{

  while (true) {


    char msg[1024];
    sprintf(msg, "Opening port: %d", _params.server_port);
    PMU_auto_register(msg);

    // set up server
    
    ServerSocket server;
    if (server.openServer(_params.server_port)) {
      if (_params.debug) {
        cerr << "ERROR - IpsTsPrint" << endl;
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

int IpsTsPrint::_runMaxPowerMode()

{

  _initMaxPowerData();

  for (int ipulse = 0; ipulse < _nSamples; ipulse++) {
    
    // read next pulse
    
    IpsTsPulse *pulse = _getNextPulse();
    if (pulse == NULL) {
      if (_pulseReader->endOfFile()) {
        return 0;
      }
      cerr << "ERROR - IpsTsPrint::_runAscopeMode()" << endl;
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

int IpsTsPrint::_runMaxPowerServerMode()
{

  char msg[1024];
  sprintf(msg, "Opening port: %d", _params.server_port);
  PMU_auto_register(msg);
  
  // set up server
  
  ServerSocket server;
  if (server.openServer(_params.server_port)) {
    cerr << "ERROR - IpsTsPrint" << endl;
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

IpsTsPulse *IpsTsPrint::_getNextPulse() 

{
  
  IpsTsPulse *pulse = NULL;

  while (pulse == NULL) {
    pulse = _pulseReader->getNextPulse(false);
    if (pulse == NULL) {
      if (_pulseReader->getTimedOut()) {
	cout << "# NOTE: read timed out, retrying ..." << endl;
	continue;
      }
      if (_pulseReader->endOfFile()) {
        cout << "# NOTE: end of file encountered" << endl;
      }
      return NULL;
    }
  }

  if (_pulseReader->endOfFile()) {
    cout << "# NOTE: end of file encountered" << endl;
  }
  if (pulse->getIq1() != NULL) {
    _haveChan1 = true;
  } else {
    _haveChan1 = false;
  }

  // condition gate limits

  _conditionGateRange(*pulse);

  // set range geom

  _startRangeM = pulse->getStartRangeM();
  _gateSpacingM = pulse->getGateSpacingM();

  // check for missing pulses

  si64 pulseSeqNum = pulse->getSeqNum();
  si64 nMissing = (pulseSeqNum - _prevPulseSeqNum) - 1;
  if (_prevPulseSeqNum != 0 && nMissing != 0 && _params.print_missing_pulses) {
    cerr << "WARNING - IpsTsPrint - n missing pulses: " << nMissing << endl;
    cerr << "  prev pulse seq num: " << _prevPulseSeqNum << endl;
    cerr << "  this pulse seq num: " << pulseSeqNum << endl;
    cerr << "  file: " << _pulseReader->getPathInUse() << endl;
  }
  _prevPulseSeqNum = pulseSeqNum;

  if (_params.change_packing) {

    // reformat pulse as needed
    
    if (_params.packing_type == Params::PACKING_FL32) {
      pulse->convertToPacked(ips_ts_iq_encoding_t::FL32);
    } else if (_params.packing_type == Params::PACKING_SCALED_SI16) {
      pulse->convertToPacked(ips_ts_iq_encoding_t::SCALED_SI16);
    } else if (_params.packing_type == Params::PACKING_DBM_PHASE_SI16) {
      pulse->convertToPacked(ips_ts_iq_encoding_t::DBM_PHASE_SI16);
    }

  }

  return pulse;

}

////////////////////////////////////////////
// condition the gate range, to keep the
// numbers within reasonable limits

void IpsTsPrint::_conditionGateRange(const IpsTsPulse &pulse)

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

}
    
////////////////////////////////////////////
// set midpoint and other cardinal values

void IpsTsPrint::_saveCardinalValues(const IpsTsPulse &pulse)

{

  if (_pulseCount == 0) {
    _startTime = pulse.getFTime();
  }

  if (_pulseCount == _nSamples / 2) {
    _midTime = pulse.getFTime();
    _midPrt = pulse.getPrt();
    _midEl = pulse.getElevation();
    _midAz = pulse.getAzimuth();
  }

  _endTime = pulse.getFTime();

}

////////////////////////////////////////////
// augment summary information

void IpsTsPrint::_addToSummary(const IpsTsPulse &pulse)

{

  _saveCardinalValues(pulse);

  const fl32 *iqChan0 = pulse.getIq0();
  const fl32 *iqChan1 = pulse.getIq1();

  int index = _startGate * 2;
  for (int igate = _startGate; igate <= _endGate; igate++, index += 2) {

    double ii0 = iqChan0[index];
    double qq0 = iqChan0[index + 1];
    double power0 = ii0 * ii0 + qq0 * qq0;
    power0 /= _rxGainHc;
    _stats.sumPower0 += power0;
    _stats.nn0++;

    if (iqChan1) {
      double ii1 = iqChan1[index];
      double qq1 = iqChan1[index + 1];
      double power1 = ii1 * ii1 + qq1 * qq1;
      power1 /= _rxGainVc;
      _stats.sumPower1 += power1;
      _stats.nn1++;
    }
    
  }
}

/////////////////////////
// compute summary stats

void IpsTsPrint::_computeSummary()
  
{
  
  _stats.meanDbm0 = -999.9;
  _stats.meanDbm1 = -999.9;
  
  if (_stats.nn0 > 0) {
    double meanPower0 = _stats.sumPower0 / _stats.nn0;
    _stats.meanDbm0 = 10.0 * log10(meanPower0);
  }

  if (_stats.nn1 > 0) {
    double meanPower1 = _stats.sumPower1 / _stats.nn1;
    _stats.meanDbm1 = 10.0 * log10(meanPower1);
  }

}

////////////////////////////////////////////
// clear stats info

void IpsTsPrint::_clearStats()

{

  _stats.init();
    
  _midTime = -999.9;
  _midPrt = -999.9;
  _midEl = -999.9;
  _midAz = -999.9;

}
  
////////////////////////////////////////////
// augment alternating information

void IpsTsPrint::_addToAlternating(const IpsTsPulse &pulse)

{

  _saveCardinalValues(pulse);

  const fl32 *iqChan0 = pulse.getIq0();
  const fl32 *iqChan1 = pulse.getIq1();
  bool isHoriz = pulse.isHoriz();

  int index = _startGate * 2;
  for (int igate = _startGate; igate <= _endGate; igate++, index += 2) {

    double ii0 = iqChan0[index];
    double qq0 = iqChan0[index + 1];

    double ii1 = -9999;
    double qq1 = -9999;
    if (iqChan1) {
      ii1 = iqChan1[index];
      qq1 = iqChan1[index + 1];
    }
    _stats.addToAlternating(pulse, ii0, qq0, _haveChan1, ii1, qq1, isHoriz);

  }

}

////////////////////////////////////////////
// augment dual information

void IpsTsPrint::_addToDual(const IpsTsPulse &pulse)

{

  _saveCardinalValues(pulse);

  const fl32 *iqChan0 = pulse.getIq0();
  const fl32 *iqChan1 = pulse.getIq1();
  
  int index = _startGate * 2;
  for (int igate = _startGate; igate <= _endGate; igate++, index += 2) {
    
    double ii0 = iqChan0[index];
    double qq0 = iqChan0[index + 1];

    double ii1 = -9999;
    double qq1 = -9999;
    if (iqChan1) {
      ii1 = iqChan1[index];
      qq1 = iqChan1[index + 1];
    }
    _stats.addToDual(pulse, ii0, qq0, _haveChan1, ii1, qq1);

  }

}

/////////////////////////
// compute alternating stats

void IpsTsPrint::_computeAlternating()
  
{

  _stats.computeAlternating(_haveChan1);

}

/////////////////////////
// compute dual stats

void IpsTsPrint::_computeDual()
  
{

  _stats.computeDual(_haveChan1);

}

////////////////////////////////////////////
// augment ascope information

void IpsTsPrint::_addToAscope(const IpsTsPulse &pulse)

{

  _saveCardinalValues(pulse);
  
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
      _aStats[igate].sumPower0 += power0;
      _aStats[igate].nn0++;
    
      if (isHoriz) {
        _aStats[igate].nnH++;
        _aStats[igate].sumPowerHc += power0;
      } else {
        _aStats[igate].nnV++;
        _aStats[igate].sumPowerVc += power0;
      }

    } else {
      
      double ii0 = iqChan0[index];
      double qq0 = iqChan0[index + 1];
      double power0 = ii0 * ii0 + qq0 * qq0;
      power0 /= _rxGainHc;
      _aStats[igate].sumPower0 += power0;
      _aStats[igate].nn0++;
    
      double ii1 = iqChan1[index];
      double qq1 = iqChan1[index + 1];
      double power1 = ii1 * ii1 + qq1 * qq1;
      power1 /= _rxGainVc;
      _aStats[igate].sumPower1 += power1;
      _aStats[igate].nn1++;

      RadarComplex_t c0, c1;
      c0.re = ii0;
      c0.im = qq0;
      c1.re = ii1;
      c1.im = qq1;
      RadarComplex_t prod = RadarComplex::conjugateProduct(c0, c1);
      
      if (isHoriz) {
        _aStats[igate].nnH++;
        _aStats[igate].sumPowerHc += power0;
        _aStats[igate].sumPowerVx += power1;
        _aStats[igate].sumConjProdH =
          RadarComplex::complexSum(_aStats[igate].sumConjProdH, prod);
      } else {
        _aStats[igate].nnV++;
        _aStats[igate].sumPowerVc += power0;
        _aStats[igate].sumPowerHx += power1;
        _aStats[igate].sumConjProdV =
          RadarComplex::complexSum(_aStats[igate].sumConjProdV, prod);
      }

    }
    
  }

}

////////////////////////////////////////////
// augment max power information

void IpsTsPrint::_addToMaxPower(const IpsTsPulse &pulse)

{

  _saveCardinalValues(pulse);
  
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

bool IpsTsPrint::_checkInfoChanged(const IpsTsPulse &pulse)

{
  
  const IpsTsInfo &info = pulse.getTsInfo();
  const ips_ts_scan_segment_t &scan = info.getScanSegment();
  const ips_ts_processing_t &proc = info.getTsProcessing();
  const ips_ts_calibration_t &calib = info.getCalibration();

  bool changed = false;
  if (ips_ts_compare(scan, _scanPrev)) {
    changed = true;
  }
  
  if (ips_ts_compare(proc, _procPrev)) {
    changed = true;
  }
  
  if (ips_ts_compare(calib, _calibPrev)) {
    changed = true;
  }

  _scanPrev = scan;
  _procPrev = proc;
  _calibPrev = calib;

  return changed;

}
  
//////////////////////////////////////////////////
// provide data to client

int IpsTsPrint::_handleClient(Socket *sock)

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
    cerr << "-->> dualChannel: " << _dualChannel << endl;
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

    const IpsTsPulse *pulse = _pulseReader->getNextPulse(true);
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
    } else if (_dualChannel) {
      _addToDual(*pulse);
    } else {
      _addToSummary(*pulse);
    }
    _pulseCount++;
    _totalPulseCount++;

    // free up pulse memory

    delete pulse;
    
  } // while
      
  // compute stats
      
  if (_fastAlternating) {
    _computeAlternating();
  } else if (_dualChannel) {
    _computeDual();
  } else {
    _computeSummary();
  }
  
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

int IpsTsPrint::_handleMaxPowerClient(Socket *sock)

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
      
      IpsTsPulse *pulse = _getNextPulse();
      if (pulse == NULL) {
        if (_pulseReader->endOfFile()) {
          return -1;
        }
        cerr << "ERROR - IpsTsPrint::_runAscopeMode()" << endl;
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

void IpsTsPrint::_reapChildren()

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

int IpsTsPrint::_readCommands(Socket *sock, string &xmlBuf)

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
      if (latestLine.find("</IpsTsPrintCommands>") != string::npos) {
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

int IpsTsPrint::_decodeCommands(string &xmlBuf)

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
  
  TaXml::readBoolean(xmlBuf, "dualChannel", _dualChannel);

  TaXml::readBoolean(xmlBuf, "fastAlternating", _fastAlternating);
  if (_fastAlternating) {
    _dualChannel = true;
  }

  _labviewRequest = false;
  TaXml::readBoolean(xmlBuf, "labviewRequest", _labviewRequest);

  return iret;

}

//////////////////////////////////////////////////
// prepare normal XML response

void IpsTsPrint::_prepareXmlResponse(int iret,
				  string &xmlResponse)
  
{
  
  xmlResponse += TaXml::writeStartTag("IpsTsPrintResponse", 0);

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
  
  xmlResponse += TaXml::writeEndTag("IpsTsPrintResponse", 0);

}

//////////////////////////////////////////////////
// prepare max power XML response

void IpsTsPrint::_prepareMaxPowerResponse(string &xmlResponse)
  
{
  
  xmlResponse += TaXml::writeStartTag("IpsTsPrintMaxPower", 0);

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
  
  xmlResponse += TaXml::writeEndTag("IpsTsPrintMaxPower", 0);

  xmlResponse += "\r\n";

}

//////////////////////////////////////////////////
// prepare labview XML response

void IpsTsPrint::_prepareLabviewResponse(int iret,
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

int IpsTsPrint::_writeResponse(Socket *sock, string &xmlBuf)

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

void IpsTsPrint::_printRunDetails(ostream &out)

{

  // get time

  DateTime now(time(NULL));
  
  out << "# RUN INFO: "
      << "year,month,day,hour,min,sec,nSamples,startGate,nGates"
      << endl;
  
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

void IpsTsPrint::_printOpsInfo(ostream &out, const IpsTsPulse *pulse)

{
  
  out << "# OPS INFO: "
      << "siteName,radarName,"
      << "alt,lat,lon,"
      << "startRange,gateSpacing,"
      << "scanMode,prfMode,"
      << "wavelenCm,beamWidthH,beamWidthV"
      << endl;

  const IpsTsInfo &info = _pulseReader->getOpsInfo();
  
  out << info.getSiteName() << ","
      << info.getRadarName() << ","
      << info.getRadarAltitudeM() << ","
      << info.getRadarLatitudeDeg() << ","
      << info.getRadarLongitudeDeg() << ","
      << info.getProcStartRangeM() << ","
      << info.getProcGateSpacingM() << ","
      << info.getScanMode() << ","
      << static_cast<int>(info.getProcPrfMode()) << ","
      << info.getRadarWavelengthCm() << ","
      << info.getRadarBeamwidthDegH() << ","
      << info.getRadarBeamwidthDegV() << endl;

  if (_params.print_packing) {
    const ips_ts_pulse_header_t &hdr = pulse->getHdr();
    out << "# IQ packing: "
        << ips_ts_iq_encoding_to_str(hdr.iq_encoding) << endl;
  }
  
}

/////////////////////////////////
// print the pulse labels

void IpsTsPrint::_printPulseLabels(ostream &out)
  
{

  out << "# PULSE HEADER: ";
  out << "seqNum,nGates,gateNum,time,prt,vol,tilt,el,az,hvFlag";
  out << ",iChan0,qChan0,power0,iChan1,qChan1,power1";

  out << endl;

}

////////////////////////////////////////////
// load up the text for this pulse

string IpsTsPrint::_pulseString(const IpsTsPulse &pulse)

{

  string str;
  
  // compute properties

  double ptime = pulse.getFTime();
  double prt = pulse.getPrt();
  
  double el = pulse.getElevation();
  double az = pulse.getAzimuth();
  int volNum = pulse.getVolumeNum();
  int sweepNum = pulse.getSweepNum();
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
  
  sprintf(text,
          "%lu,%d,%d"
          ",%.3f,%.5f,%d,%d,%.2f,%.2f"
          ",%d",
          pulseSeqNum, nGates, gateNum,
          ptime, prt, volNum, sweepNum, el, az,
          (int) horiz);
  
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

void IpsTsPrint::_printSummaryHeading(ostream &out)
  
{

  out << "# SUMMARY MODE:" << endl;
  out << "#   Start gate: " << _startGate << endl;
  out << "#   N gates: " << _nGates << endl;
  out << "#   N samples: " << _nSamples << endl;

}

////////////////////////////////////
// print alternating heading labels

void IpsTsPrint::_printAlternatingHeading(ostream &out)
  
{

  out << "# ALTERNATING MODE:" << endl;
  out << "#   Start gate: " << _startGate << endl;
  out << "#   N gates: " << _nGates << endl;
  out << "#   N samples: " << _nSamples << endl;

}

/////////////////////////////////////
// print dual channel heading labels

void IpsTsPrint::_printDualHeading(ostream &out)
  
{

  out << "# DUAL CHANNEL MODE:" << endl;
  out << "#   Start gate: " << _startGate << endl;
  out << "#   N gates: " << _nGates << endl;
  out << "#   N samples: " << _nSamples << endl;

}

/////////////////////////////////
// print summary labels

void IpsTsPrint::_printSummaryLabels(ostream &out)
  
{

  if (_dualChannel) {
    out << "#                  "
	<< "time           prf      el      az  dbmChan0  dbmChan1" << endl;
  } else {
    out << "#                  "
	<< "time           prf      el      az  dbmChan0" << endl;
  }

}

/////////////////////////////////
// print summary data

void IpsTsPrint::_printSummaryData(FILE *out)
  
{

  if (_params.debug) {
    fprintf(out, "Current time: %s\n", DateTime::str().c_str());
  }

  time_t midSecs = (time_t) _midTime;
  int midNanoSecs = (int) ((_midTime - midSecs) * 1.0e9 + 0.5);
  double prf = 1.0 / _midPrt;
  const char *transStr = "";

  if (_dualChannel) {
    fprintf(out, "%s.%.9d %7.1f %7.2f %7.2f %9.3f %9.3f%s\n",
	    DateTime::strm(midSecs).c_str(),
	    midNanoSecs,
	    prf, _midEl, _midAz, _stats.meanDbm0, _stats.meanDbm1, transStr);
  } else {
    fprintf(out, "%s.%.9d %7.1f %7.2f %7.2f %9.3f%s\n",
	    DateTime::strm(midSecs).c_str(),
	    midNanoSecs,
	    prf, _midEl, _midAz, _stats.meanDbm0, transStr);
  }
  
}

/////////////////////////////////
// print alternating labels

void IpsTsPrint::_printAlternatingLabels(ostream &out)
  
{

  out << "#                  "
      << "time           prf      el      az"
      << "       Hc       Hx    Hcorr     Harg"
      << "       Vc       Vx    Vcorr     Varg"
      << "     IFD0     IFD1"
      << endl;

}

/////////////////////////////////
// print dual labels

void IpsTsPrint::_printDualLabels(ostream &out)
  
{

  out << "#                  "
      << "time           prf      el      az"
      << "       Hc    Hcorr     Harg"
      << "       Vc    Vcorr     Varg"
      << "     IFD0     IFD1"
      << endl;

}

/////////////////////////////////
// print alternating data

void IpsTsPrint::_printAlternatingData(FILE *out)
  
{
  
  if (_params.debug) {
    fprintf(out, "Current time: %s\n", DateTime::str().c_str());
  }

  time_t midSecs = (time_t) _midTime;
  int midNanoSecs = (int) ((_midTime - midSecs) * 1.0e9 + 0.5);
  double prf = 1.0 / _midPrt;
  
  const char *transStr = "";

  fprintf(out, "%s.%.9d %7.1f %7.2f %7.2f "
          "%8.3f %8.3f %8.3f %8.3f "
          "%8.3f %8.3f %8.3f %8.3f "
          "%8.3f %8.3f%s\n",
          DateTime::strm(midSecs).c_str(),
          midNanoSecs,
          prf, _midEl, _midAz,
          _stats.meanDbmHc,
          _stats.meanDbmHx,
          _stats.corrH,
          _stats.argH,
          _stats.meanDbmVc,
          _stats.meanDbmVx,
          _stats.corrV,
          _stats.argV,
          _stats.meanDbm0,
          _stats.meanDbm1,
          transStr);
  
}

/////////////////////////////////
// print dual data

void IpsTsPrint::_printDualData(FILE *out)
  
{
  
  if (_params.debug) {
    fprintf(out, "Current time: %s\n", DateTime::str().c_str());
  }
  
  time_t midSecs = (time_t) _midTime;
  int midNanoSecs = (int) ((_midTime - midSecs) * 1.0e9 + 0.5);
  double prf = 1.0 / _midPrt;

  const char *transStr = "";

  fprintf(out, "%s.%.9d %7.1f %7.2f %7.2f "
          "%8.3f %8.3f %8.3f "
          "%8.3f %8.3f %8.3f "
          "%8.3f %8.3f%s\n",
	  DateTime::strm(midSecs).c_str(),
	  midNanoSecs,
	  prf, _midEl, _midAz,
          _stats.meanDbmHc,
          _stats.corrH,
          _stats.argH,
          _stats.meanDbmVc,
          _stats.corrV,
          _stats.argV,
	  _stats.meanDbm0,
          _stats.meanDbm1,
          transStr);
          
}

/////////////////////////////////////
// print max power heading

void IpsTsPrint::_printMaxPowerHeading(ostream &out)
  
{

  out << "# MAX POWER MODE:" << endl;
  out << "#   Start gate: " << _startGate << endl;
  out << "#   N gates: " << _nGates << endl;
  out << "#   N samples: " << _nSamples << endl;

}

/////////////////////////////////
// print max power labels

void IpsTsPrint::_printMaxPowerLabels(ostream &out)
  
{
  
  if (_dualChannel) {
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
    if (_dualChannel) {
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

void IpsTsPrint::_initMaxPowerData()
  
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

void IpsTsPrint::_computeMaxPowerData()
  
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

double IpsTsPrint::_computeVel(const vector<RadarComplex_t> &iq)
  
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
  
  const IpsTsInfo &info = _pulseReader->getOpsInfo();
  double wavelengthMeters = info.getRadarWavelengthCm() / 1.0e2;
  double prt = info.getProcPrtUs(0) / 1.0e6;
  double nyquist = ((wavelengthMeters / prt) / 4.0);

  // compute velocity
  
  double vel = (argVel / M_PI) * nyquist * -1.0;

  // correct for platform motion
  
  if (_params.use_secondary_georeference) {
    if (info.isPlatformGeoref1Active()) {
      const ips_ts_platform_georef_t &georef1 = info.getPlatformGeoref1();
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
    const ips_ts_platform_georef_t &georef = info.getPlatformGeoref();
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

void IpsTsPrint::_printMaxPowerData(FILE *out)
  
{
  
  time_t midSecs = (time_t) _midTime;
  int midMilliSecs = (int) ((_midTime - midSecs) * 1.0e3 + 0.5);
  double prf = 1.0 / _midPrt;

  fprintf(out, "%s.%.3d %7.1f %7.2f %8.2f %7.2f %10.0f",
          DateTime::strm(midSecs).c_str(),
          midMilliSecs,
          prf,
          _midEl, _midAz,
          10.0 * log10(_meanMaxPower0),
          _rangeToMaxPower0);
  if (_dualChannel) {
    fprintf(out, " %7.2f %10.0f",
            10.0 * log10(_meanMaxPower1),
            _rangeToMaxPower1);
  }

  if (_params.print_radial_velocity) {
    fprintf(out, " %7.2f", _velAtMaxPower0);
    if (_dualChannel) {
      fprintf(out, " %7.2f", _velAtMaxPower1);
    }
  }
  
  const IpsTsInfo &info = _pulseReader->getOpsInfo();
  double lat = info.getRadarLatitudeDeg();
  double lon = info.getRadarLongitudeDeg();
  double alt = info.getRadarAltitudeM();
  if (_params.use_secondary_georeference &&
      info.isPlatformGeoref1Active()) {
    const ips_ts_platform_georef_t &georef1 = info.getPlatformGeoref();
      lat = georef1.latitude;
      lon = georef1.longitude;
      alt = georef1.altitude_msl_km * 1000.0;
  } else if (info.isPlatformGeorefActive()) {
    const ips_ts_platform_georef_t &georef = info.getPlatformGeoref();
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
// print sizes of structures

void IpsTsPrint::_printStructSizes(ostream &out)

{

  cerr << "sizeof(ips_ts_packet_info_t): " << sizeof(ips_ts_packet_info_t) << endl;
  cerr << "sizeof(ips_ts_sync_t): " << sizeof(ips_ts_sync_t) << endl;
  cerr << "sizeof(ips_ts_version_t): " << sizeof(ips_ts_version_t) << endl;
  cerr << "sizeof(ips_ts_radar_info_t): " << sizeof(ips_ts_radar_info_t) << endl;
  cerr << "sizeof(ips_ts_scan_segment_t): " << sizeof(ips_ts_scan_segment_t) << endl;
  cerr << "sizeof(ips_ts_processing_t): " << sizeof(ips_ts_processing_t) << endl;
  cerr << "sizeof(ips_ts_status_xml_t): " << sizeof(ips_ts_status_xml_t) << endl;
  cerr << "sizeof(ips_ts_calibration_t): " << sizeof(ips_ts_calibration_t) << endl;
  cerr << "sizeof(ips_ts_event_notice_t): " << sizeof(ips_ts_event_notice_t) << endl;
  cerr << "sizeof(ips_ts_pulse_header_t): " << sizeof(ips_ts_pulse_header_t) << endl;
  cerr << "sizeof(ips_ts_platform_georef_t): " << sizeof(ips_ts_platform_georef_t) << endl;
  cerr << "sizeof(ips_ts_georef_correction_t): " << sizeof(ips_ts_georef_correction_t) << endl;

}

  
