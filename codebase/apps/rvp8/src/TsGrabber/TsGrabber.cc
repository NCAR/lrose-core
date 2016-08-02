// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:30:34 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// TsGrabber.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////
//
// TsGrabber reads time-series data from RVP8,
// and processes it in various ways
//
////////////////////////////////////////////////////////////////

#include "TsGrabber.hh"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctime>
#include <rvp8_rap/DateTime.hh>
#include <rvp8_rap/uusleep.h>
#include <rvp8_rap/macros.h>
#include <rvp8_rap/ServerSocket.hh>
#include <rvp8_rap/TaXml.hh>
#include <rvp8_rap/pmu.h>

using namespace std;

// Constructor

TsGrabber::TsGrabber(int argc, char **argv)
  
{

  isOK = true;
  // _done = false;
  _pulseCount = 0;
  _totalPulseCount = 0;
  _printCount = 0;
  _input = NULL;

  // set programe name
  
  _progName = "TsGrabber";

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // initialize from args

  _nSamples = _args.nSamples;
  _startGate = _args.startGate;
  _nGates = _args.nGates;
  _fastAlternating = _args.fastAlternating;
  _dualChannel = _args.dualChannel;
  _onceOnly = _args.onceOnly;

  // set up input object

  _input = new Input(_args);

  // init process mapper registration
  
  if (_args.regWithProcmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _args.instance.c_str(),
                  PROCMAP_REGISTER_INTERVAL);
  }
  
}

// destructor

TsGrabber::~TsGrabber()

{

  if (_input) {
    delete _input;
  }

  _aStats.clear();

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int TsGrabber::Run ()
{

  // initialize for reading time series data

  if (_input->initForReading()) {
    cerr << "ERROR - TsGrabber::run()" << endl;
    cerr << "  Cannot initialize for reading" << endl;
    return -1;
  }

  if (_args.runMode == Args::CAL_MODE) {

    return _runCalMode();

  } else if (_args.runMode == Args::SERVER_MODE) {

    return _runServerMode();

  } else if (_args.runMode == Args::ASCOPE_MODE) {

    if (_onceOnly) {
      return _runAscopeMode();
    } else {
      while (_runAscopeMode() == 0) {
      }
      return -1;
    }

  } else if (_args.runMode == Args::PRINT_MODE) {

    return _runPrintMode();

  }

}

//////////////////////////////////////////////////
// Run in print mode

int TsGrabber::_runPrintMode()

{
  
  while (true) {

    // read next pulse

    TsPulse *pulse = _input->readNextPulse();
    if (pulse == NULL) {
      return 0;
    }

    if (_args.verbose) {
      pulse->print(cerr);
    }
    
    // at start, print headers
    
    if (_totalPulseCount == 0) {
      _printRunDetails(cout);
      _printOpsInfo(cout);
      if (_args.printMode == Args::PRINT_SUMMARY) {
        if (_fastAlternating) {
          _printAlternatingHeading(cout);
        } else {
          _printSummaryHeading(cout);
        }
      }
    }
    
    // print every pulse if needed

    if (_args.printMode == Args::PRINT_FULL) {
      if ((_printCount % _args.labelInterval) == 0) {
        _printPulseLabels(cout);
      }
      _printCount++;
      cout << _pulseString(*pulse) << endl;
    }
    
    // add to data as appropriate
    
    if (_fastAlternating) {
      if (_pulseCount == 0) {
        _clearStats();
      }
      _addToAlternating(*pulse);
    } else {
      if (_pulseCount == 0) {
        _clearStats();
      }
      _addToSummary(*pulse);
    }
    
    _pulseCount++;
    _totalPulseCount++;
    
    if (_pulseCount == _nSamples) {
      
      // compute stats
      
      if (_fastAlternating) {
        _computeAlternating();
      } else {
        _computeSummary();
      }
      
      if (_args.printMode == Args::PRINT_SUMMARY) {
        if (_fastAlternating) {
          if ((_printCount % _args.labelInterval) == 0) {
            _printAlternatingLabels(cout);
          }
	  if (_nSamples == 1) {
	    _printAlternatingData(stdout, pulse);
	  } else {
	    _printAlternatingData(stdout, NULL);
	  }
        } else {
          if ((_printCount % _args.labelInterval) == 0) {
            _printSummaryLabels(cout);
          }
          _printSummaryData(stdout);
        }
        _printCount++;
      }
      
      _pulseCount = 0;

      cout << flush;
      fflush(stdout);

      if (_onceOnly) {
        delete pulse;
        return 0;
      }
      
    }
    
    delete pulse;
    
  } // while

}

//////////////////////////////////////////////////
// Run in print mode

int TsGrabber::_runAscopeMode()

{

  // allocate vector for Ascope stats

  _aStats.clear();
  for (int ii = 0; ii < _nGates; ii++) {
    Stats stats;
    _aStats.push_back(stats);
  }
  _pulseCount = 0;

  for (int ipulse = 0; ipulse < _nSamples; ipulse++) {

    // read next pulse

    TsPulse *pulse = _input->readNextPulse();
    if (pulse == NULL) {
      cerr << "ERROR - TsGrabber::_runAscopeMode()" << endl;
      cerr << "  Cannot read enough samples for aScope" << endl;
      return -1;
    }

    if (_args.printHvFlag) {
      cerr << pulse->getiPolarBits() << ",";
    }

    _addToAscope(*pulse);
    _pulseCount++;
    _totalPulseCount++;
    
    delete pulse;

  } 

  if (_args.printHvFlag) {
    cerr << endl;
  }

  // compute ascope stats

  for (int igate = 0; igate < _nGatesAscope; igate++) {
    _aStats[igate].computeAlternating();
  }
    
  time_t midSecs = (time_t) _midTime;
  int midPartialMSecs = (int) ((_midTime - midSecs) * 1000.0 + 0.5);
  double prf = 1.0 / _midPrt;

  // print
  
  cout << "# ASCOPE MODE:" << endl;
  cout << "#   Start gate: " << _startGate << endl;
  cout << "#   N gates: " << _nGates << endl;
  cout << "#   N samples: " << _nSamples << endl;
  cout << "#   time: " << DateTime::strm(midSecs) << "."
       << midPartialMSecs << endl;
  cout << "#   PRF: " << prf << endl;
  cout << "#   EL (deg): " << _midEl << endl;
  cout << "#   AZ (deg): " << _midAz << endl;

  cout << "#   Gate num"
       << "       Hc       Hx    Hcorr     Harg"
       << "       Vc       Vx    Vcorr     Varg"
       << "     IFD0     IFD1"
       << endl;

  for (int igate = 0; igate < _nGatesAscope; igate++) {

    int gateNum = igate + _startGate;

    fprintf(stdout, "%12d "
            "%8.3f %8.3f %8.3f %8.3f "
            "%8.3f %8.3f %8.3f %8.3f "
            "%8.3f %8.3f\n",
            gateNum,
            _aStats[igate].meanDbmHc,
            _aStats[igate].meanDbmHx,
            _aStats[igate].corr01H,
            _aStats[igate].arg01H,
            _aStats[igate].meanDbmVc,
            _aStats[igate].meanDbmVx,
            _aStats[igate].corr01V,
            _aStats[igate].arg01V,
            _aStats[igate].meanDbm0,
            _aStats[igate].meanDbm1);

  }

  return 0;

}

//////////////////////////////////////////////////
// Run in server mode

int TsGrabber::_runServerMode()
{

  while (true) {


    char msg[1024];
    sprintf(msg, "Opening port: %d", _args.serverPort);
    PMU_auto_register(msg);

    // set up server
    
    ServerSocket server;
    if (server.openServer(_args.serverPort)) {
      if (_args.debug) {
        cerr << "ERROR - TsGrabber" << endl;
        cerr << "  Cannot open server, port: " << _args.serverPort << endl;
        cerr << "  " << server.getErrStr() << endl;
      }
      umsleep(100);
      continue;
    }
    
    if (_args.debug) {
      cerr << "Running as server, listening on port: "
           << _args.serverPort << endl;
    }

    while (true) {

      // register with procmap
      sprintf(msg, "Getting clients, port: %d", _args.serverPort);
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

      if (_args.debug) {
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
// Run in calibration mode

int TsGrabber::_runCalMode()
{

  // get cal name

  fprintf(stdout, "Running TsGrabber in calibration mode\n");
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
  char fileName[1024];
  sprintf(fileName, "rvp8cal_%.4d%.2d%.2d_%.2d%.2d%2d_%s.txt",
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

  fprintf(calFile, "#%10s %10s %10s %10s %10s\n",
	  "sgdBm", "Hc", "Hx", "Vc", "Vx");

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
      
      TsPulse *pulse = _input->readNextPulse();
      if (pulse == NULL) {
        return -1;
      }
      _addToAlternating(*pulse);
      _pulseCount++;
      delete pulse;
    }
    
    _computeAlternating();
    _printAlternatingLabels(cout);
    _printAlternatingData(stdout, NULL);

    fprintf(calFile, " %10.3f %10.3f %10.3f %10.3f %10.3f\n",
	    sgSetting, _stats.meanDbmHc, _stats.meanDbmHx,
            _stats.meanDbmVc, _stats.meanDbmVx);
    
  } // while (true)
  
  fclose(calFile);
  
  return 0;

}

////////////////////////////////////////////
// augment summary information

void TsGrabber::_addToSummary(const TsPulse &pulse)

{

  if (_pulseCount == _nSamples / 2) {
    _midTime = pulse.getFTime();
    _midPrt = pulse.getPrt();
    _midEl = pulse.getEl();
    _midAz = pulse.getAz();
    _midElBits = pulse.getiEl();
    _midAzBits = pulse.getiAz();
  }

  int maxGates = pulse.getNGates();

  int endGate = _startGate + _nGates - 1;
  if (endGate > maxGates - 1) {
    endGate = maxGates - 1;
  }

  const FLT4 *iqChan0 = pulse.getIqChan0();
  const FLT4 *iqChan1 = pulse.getIqChan1();

  int index = _startGate * 2;
  for (int igate = _startGate; igate <= endGate; igate++, index += 2) {

    double ii0 = iqChan0[index];
    double qq0 = iqChan0[index + 1];
    double power0 = ii0 * ii0 + qq0 * qq0;
    _stats.sumPower0 += power0;
    _stats.nn0++;

    if (_dualChannel && iqChan1) {
      double ii1 = iqChan1[index];
      double qq1 = iqChan1[index + 1];
      double power1 = ii1 * ii1 + qq1 * qq1;
      _stats.sumPower1 += power1;
      _stats.nn1++;
    }
    
  }
}

/////////////////////////
// compute summary stats

void TsGrabber::_computeSummary()
  
{
  
  _stats.meanDbm0 = -999.9;
  _stats.meanDbm1 = -999.9;
  
  const TsInfo &info = _input->getTsInfo();
  
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

void TsGrabber::_clearStats()

{

  _stats.init();
    
  _midTime = -999.9;
  _midPrt = -999.9;
  _midEl = -999.9;
  _midAz = -999.9;

}
  
////////////////////////////////////////////
// augment alternating information

void TsGrabber::_addToAlternating(const TsPulse &pulse)

{

  if (_pulseCount == _nSamples / 2) {
    _midTime = pulse.getFTime();
    _midPrt = pulse.getPrt();
    _midEl = pulse.getEl();
    _midAz = pulse.getAz();
    _midElBits = pulse.getiEl();
    _midAzBits = pulse.getiAz();
  }

  int maxGates = pulse.getNGates();
  int endGate = _startGate + _nGates - 1;
  if (endGate > maxGates - 1) {
    endGate = maxGates - 1;
  }

  const FLT4 *iqChan0 = pulse.getIqChan0();
  const FLT4 *iqChan1 = pulse.getIqChan1();
  bool isHoriz = pulse.isHoriz();

  int index = _startGate * 2;
  for (int igate = _startGate; igate <= endGate; igate++, index += 2) {

    double ii0 = iqChan0[index];
    double qq0 = iqChan0[index + 1];

    double ii1 = -9999;
    double qq1 = -9999;
    bool haveChan1 = false;
    if (_dualChannel && iqChan1) {
      haveChan1 = true;
      ii1 = iqChan1[index];
      qq1 = iqChan1[index + 1];
    }
    _stats.addToAlternating(ii0, qq0, haveChan1, ii1, qq1, isHoriz);

  }

}

/////////////////////////
// compute alternating stats

void TsGrabber::_computeAlternating()
  
{

  _stats.computeAlternating();

}

////////////////////////////////////////////
// augment ascope information

void TsGrabber::_addToAscope(const TsPulse &pulse)

{

  if (_pulseCount == _nSamples / 2) {
    _midTime = pulse.getFTime();
    _midPrt = pulse.getPrt();
    _midEl = pulse.getEl();
    _midAz = pulse.getAz();
    _midElBits = pulse.getiEl();
    _midAzBits = pulse.getiAz();
  }

  int maxGates = pulse.getNGates();
  int endGate = _startGate + _nGates - 1;
  if (endGate > maxGates - 1) {
    endGate = maxGates - 1;
  }
  _nGatesAscope = endGate - _startGate + 1;

  const FLT4 *iqChan0 = pulse.getIqChan0();
  const FLT4 *iqChan1 = pulse.getIqChan1();
  bool isHoriz = pulse.isHoriz();
  
  int index = _startGate * 2;
  for (int igate = 0; igate < _nGatesAscope; igate++, index += 2) {
    
    double ii0 = iqChan0[index];
    double qq0 = iqChan0[index + 1];
    double power0 = ii0 * ii0 + qq0 * qq0;
    _aStats[igate].sumPower0 += power0;
    _aStats[igate].nn0++;
    
    if (_dualChannel && iqChan1) {
      
      double ii1 = iqChan1[index];
      double qq1 = iqChan1[index + 1];
      double power1 = ii1 * ii1 + qq1 * qq1;
      _aStats[igate].sumPower1 += power1;
      _aStats[igate].nn1++;

      RapComplex c0, c1;
      c0.re = ii0;
      c0.im = qq0;
      c1.re = ii1;
      c1.im = qq1;
      RapComplex prod01 = RapComplex::computeConjProduct(c0, c1);

      if (isHoriz) {
        _aStats[igate].nnH++;
        _aStats[igate].sumPowerHc += power0;
        _aStats[igate].sumPowerVx += power1;
        _aStats[igate].sumConjProd01H =
          RapComplex::computeSum(_aStats[igate].sumConjProd01H, prod01);
      } else {
        _aStats[igate].nnV++;
        _aStats[igate].sumPowerVc += power0;
        _aStats[igate].sumPowerHx += power1;
        _aStats[igate].sumConjProd01V =
          RapComplex::computeSum(_aStats[igate].sumConjProd01V, prod01);
      }

    }
    
  }

}

//////////////////////////////////////////////////
// provide data to client

int TsGrabber::_handleClient(Socket *sock)

{

  if (_args.debug) {
    cerr << "  Child - handling client ..." << endl;
  }

  // read data coming in
  
  string xmlCommands;
  if (_readCommands(sock, xmlCommands)) {
    return -1;
  }
  _decodeCommands(xmlCommands);
  
  if (_args.debug) {
    cerr << "----------- Incoming XML commands --------------" << endl;
    cerr << xmlCommands << endl;
    cerr << "----------- Setting variables ------------------" << endl;
    cerr << "-->> nSamples: " << _nSamples << endl;
    cerr << "-->> nGates: " << _nGates << endl;
    cerr << "-->> startGate: " << _startGate << endl;
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

    TsPulse *pulse = _input->readNextPulse();
    if (pulse == NULL) {
      iret = -1;
      break;
    }

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
      
  if (_fastAlternating) {
    _computeAlternating();
  } else {
    _computeSummary();
  }

  if (_args.debug) {
    _printAlternatingHeading(cerr);
    _printAlternatingLabels(cerr);
    _printAlternatingData(stderr, NULL);
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
// reap children from fork()s which have exited

void TsGrabber::_reapChildren()

{
  
  pid_t dead_pid;
  int status;
  
  while ((dead_pid = waitpid((pid_t) -1,
                             &status,
                             (int) (WNOHANG | WUNTRACED))) > 0) {
    
    if (_args.debug) {
      cerr << "  child exited, pid: " << dead_pid << endl;
    }

  } // while

}

//////////////////////////////////////////////////
// Read incoming commands
//
// Returns 0 on success, -1 on failure

int TsGrabber::_readCommands(Socket *sock, string &xmlBuf)

{

  if (_args.verbose) {
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
      if (_args.verbose) {
	cerr << cc;
      }
      xmlBuf += cc;
      latestLine += cc;
      if (latestLine.find("</TsGrabberCommands>") != string::npos) {
	if (_args.verbose) {
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

int TsGrabber::_decodeCommands(string &xmlBuf)

{

  // decode commands

  int iret = 0;

  if (TaXml::readInt(xmlBuf, "nSamples", _nSamples)) {
    iret = -1;
  }
  if (TaXml::readInt(xmlBuf, "nGates", _nGates)) {
    iret = -1;
  };
  if (TaXml::readInt(xmlBuf, "startGate", _startGate)) {
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

void TsGrabber::_prepareXmlResponse(int iret,
                                    string &xmlResponse)
  
{
  
  xmlResponse += TaXml::writeStartTag("TsGrabberResponse", 0);

  if (iret) {
    
    xmlResponse += TaXml::writeBoolean("success", 1, false);
    
  } else {
    
    xmlResponse += TaXml::writeBoolean("success", 1, true);
    
    // set response
    
    time_t midSecs = (time_t) _midTime;
    int midPartialMSecs = (int) ((_midTime - midSecs) * 1000.0 + 0.5);
    double prf = 1.0 / _midPrt;
    
    xmlResponse += TaXml::writeTime("time", 1, midSecs);
    xmlResponse += TaXml::writeDouble("msecs", 1, midPartialMSecs);
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
      xmlResponse += TaXml::writeDouble("corr01H", 1, _stats.corr01H);
      xmlResponse += TaXml::writeDouble("arg01H", 1, _stats.arg01H);
      xmlResponse += TaXml::writeDouble("corr01V", 1, _stats.corr01V);
      xmlResponse += TaXml::writeDouble("arg01V", 1, _stats.arg01V);
      
    }
    
  }
  
  xmlResponse += TaXml::writeEndTag("TsGrabberResponse", 0);

}

//////////////////////////////////////////////////
// prepare labview XML response

void TsGrabber::_prepareLabviewResponse(int iret,
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

  xmlResponse += TaXml::writeString("Name", 1, "RVP8_power");
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

int TsGrabber::_writeResponse(Socket *sock, string &xmlBuf)

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

void TsGrabber::_printRunDetails(ostream &out)

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

void TsGrabber::_printOpsInfo(ostream &out)

{

  out << "# OPERATIONS INFO: "
      << "siteName,majorMode,polarization,phaseModSeq,prfMode,"
      << "pulseWidthUsec,dbzCal1km,ifdClockMhz,wavelengthCm,"
      << "satPowerDbm,gateSpacingM,"
      << "noiseLevelDbmI,noiseLevelDbmQ,"
      << "noiseSdevDbmI,noiseSdevDbmQ,"
      << "noiseRanngeKm,noisePrf,"
      << "RDAversion"
      << endl;

  const TsInfo &info = _input->getTsInfo();
  
  out << info.getSiteName() << ","
      << _majorMode2String(info.getMajorMode()) << ","
      << _polarization2String(info.getPolarization()) << ","
      << _phaseSeq2String(info.getPhaseModSeq()) << ","
      << _prfMode2String(info.getUnfoldMode()) << ","
      << info.getPulseWidthUs() << ","
      << info.getDbzCalibChan0() << ","
      << info.getClockMhz() << ","
      << info.getWavelengthCm() << ","
      << info.getSaturationDbm() << ","
      << info.getRangeMaskRes() << ","
      << info.getNoiseDbmChan0() << ","
      << info.getNoiseDbmChan1() << ","
      << info.getNoiseSdevChan0() << ","
      << info.getNoiseSdevChan1() << ","
      << info.getNoiseRangeKm() << ","
      << info.getNoisePrfHz() << ","
      << info.getVersionString() << endl;
  
}

/////////////////////////////////
// print the pulse labels

void TsGrabber::_printPulseLabels(ostream &out)
  
{

  out << "# PULSE HEADER: ";
  out << "seqNum,nGates,gateNum,time,prt,el,az,phaseDiff0,burstMag";
  out << ",iFlags,iPrevPRT,iNextPRT,iAqMode";
  out << ",iPolarBits,iTxPhase,iTgBank,iTgWave";
  out << ",iChan0,qChan0,power0,iChan1,qChan1,power1";

  out << endl;

}

////////////////////////////////////////////
// load up the text for this pulse

string TsGrabber::_pulseString(const TsPulse &pulse)

{

  string str;
  const TsInfo &info = _input->getTsInfo();
  
  // compute properties

  double ptime = pulse.getFTime();
  double prt = pulse.getPrt();
  
  double el = pulse.getEl();
  double az = pulse.getAz();
  double burstMag = pulse.getBurstMag0();
  double phaseDiff0 = pulse.getPhaseDiffChan0();
  
  int nGates = pulse.getNGates();
  unsigned int seqNum = pulse.getSeqNum();

  int gateNum = _startGate;
  if (gateNum > nGates - 1) {
    cerr << "ERROR - gate number too high: " << gateNum << endl;
    cerr << "  Using last gate, number: " << nGates - 1 << endl;
    gateNum = nGates - 1;
  }

  char text[4096];
  int index = gateNum * 2;

  sprintf(text,
          "%u,%d,%d"
          ",%.3f,%.5f,%.2f,%.2f"
          ",%.2f,%.3e",
          seqNum, nGates, gateNum,
          ptime, prt, el, az,
          phaseDiff0, burstMag);
  
  str += text;
  
  sprintf(text,
          ",%d,%d,%d,%d,%d,%d,%d,%d",
          pulse.getiFlags(),
          pulse.getiPrevPRT(),
          pulse.getiNextPRT(),
          pulse.getiAqMode(),
          pulse.getiPolarBits(),
          pulse.getiTxPhase(),
          pulse.getiTgBank(),
          pulse.getiTgWave());
  
  str += text;
  
  double ii0 = pulse.getIqChan0()[index];
  double qq0 = pulse.getIqChan0()[index + 1];
  sprintf(text, ",%.3e,%.3e", ii0, qq0);
  str += text;
  double power0 = ii0 * ii0 + qq0 * qq0;
  double dbm0 = 10.0 * log10(power0);
  sprintf(text, ",%.2f", dbm0);
  str += text;

  if (_dualChannel && pulse.getIqChan1() != NULL) {
    double ii1 = pulse.getIqChan1()[index];
    double qq1 = pulse.getIqChan1()[index + 1];
    sprintf(text, ",%.3e,%.3e", ii1, qq1);
    str += text;
    double power1 = ii1 * ii1 + qq1 * qq1;
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

void TsGrabber::_printSummaryHeading(ostream &out)
  
{

  out << "# SUMMARY MODE:" << endl;
  out << "#   Start gate: " << _startGate << endl;
  out << "#   N gates: " << _nGates << endl;
  out << "#   N samples: " << _nSamples << endl;

}

////////////////////////////////////
// print alternating heading labels

void TsGrabber::_printAlternatingHeading(ostream &out)
  
{

  out << "# ALTERNATING MODE:" << endl;
  out << "#   Start gate: " << _startGate << endl;
  out << "#   N gates: " << _nGates << endl;
  out << "#   N samples: " << _nSamples << endl;

}

/////////////////////////////////
// print summary labels

void TsGrabber::_printSummaryLabels(ostream &out)
  
{
  
  out << "#                  "
      << "time     prf      el      az";

  if (_args.printBinAngles) {
    out << "            elBits            azBits";
  }

  if (_dualChannel) {
    out << "  dbmChan0  dbmChan1";
  } else {
    out << "  dbmChan0";
  }

  out << endl;

}

/////////////////////////////////
// print summary data

void TsGrabber::_printSummaryData(FILE *out)
  
{
  
  time_t midSecs = (time_t) _midTime;
  int midPartialMSecs = (int) ((_midTime - midSecs) * 1000.0 + 0.5);
  double prf = 1.0 / _midPrt;
  
  fprintf(out, "%s.%.3d %7.1f %7.2f %7.2f",
	  DateTime::strm(midSecs).c_str(),
	  midPartialMSecs, prf, _midEl, _midAz);

  if (_args.printBinAngles) {
    fprintf(out, "%18s", _iangle2BitStr(_midElBits).c_str());
    fprintf(out, "%18s", _iangle2BitStr(_midAzBits).c_str());
  }

  if (_dualChannel) {
    fprintf(out, " %9.3f %9.3f\n", _stats.meanDbm0, _stats.meanDbm1);
  } else {
    fprintf(out, " %9.3f\n", _stats.meanDbm0);
  }
  
}

/////////////////////////////////
// print alternating labels

void TsGrabber::_printAlternatingLabels(ostream &out)
  
{

  out << "#                  "
      << "time     prf      el      az"
      << "       Hc       Hx    Hcorr     Harg"
      << "       Vc       Vx    Vcorr     Varg"
      << "     IFD0     IFD1";

  if (_nSamples == 1) {
    out << "  HV";
  }

  out << endl;

}

/////////////////////////////////
// print alternating data

void TsGrabber::_printAlternatingData(FILE *out,
				      const TsPulse *pulse)
  
{
  
  time_t midSecs = (time_t) _midTime;
  int midPartialMSecs = (int) ((_midTime - midSecs) * 1000.0 + 0.5);
  double prf = 1.0 / _midPrt;
  
  fprintf(out, "%s.%.3d %7.1f %7.2f %7.2f "
          "%8.3f %8.3f %8.3f %8.3f "
          "%8.3f %8.3f %8.3f %8.3f "
          "%8.3f %8.3f",
	  DateTime::strm(midSecs).c_str(),
	  midPartialMSecs,
	  prf, _midEl, _midAz,
          _stats.meanDbmHc,
          _stats.meanDbmHx,
          _stats.corr01H,
          _stats.arg01H,
          _stats.meanDbmVc,
          _stats.meanDbmVx,
          _stats.corr01V,
          _stats.arg01V,
	  _stats.meanDbm0,
          _stats.meanDbm1);
  
  if (pulse != NULL) {
    int hvFlag = pulse->getiPolarBits();
    fprintf(out, "  %2d", hvFlag);
  }

  fprintf(out, "\n");

}

//////////////////////////////////////
// convert enums to strings

string TsGrabber::_majorMode2String(int majorMode)
  
{
  
  switch (majorMode) {
    case PMODE_PPP: return "ppp";
    case PMODE_FFT: return "fft";
    case PMODE_RPH: return "rph";
    case PMODE_DPT2: return "dpt2";
    case PMODE_USER1: return "user1";
    case PMODE_USER2: return "user2";
    case PMODE_USER3: return "user3";
    case PMODE_USER4: return "user4";
    default: return "unknown";
  }

}

string TsGrabber::_prfMode2String(int prfMode)

{

  switch (prfMode) {
    case PRF_FIXED: return "fixed";
    case PRF_2_3: return "2_3";
    case PRF_3_4: return "3_4";
    case PRF_4_5: return "4_5";
    default: return "unknown";
  }

}

string TsGrabber::_phaseSeq2String(int phaseSeq)

{
  
  switch (phaseSeq) {
    case PHSEQ_FIXED: return "fixed";
    case PHSEQ_RANDOM: return "random";
    case PHSEQ_CUSTOM: return "custom";
    case PHSEQ_SZ8_64: return "sz8_64";
    default: return "unknown";
  }

}

string TsGrabber::_polarization2String(int polarization)

{
  
  switch (polarization) {
    case POL_HORIZ_FIX: return "horizontal";
    case POL_VERT_FIX: return "vertical";
    case POL_ALTERNATING: return "alternating";
    case POL_SIMULTANEOUS: return "simultaneous";
    default: return "unknown";
  }

}

// load bits in integer angle

string TsGrabber::_iangle2BitStr(int iang)
  
{
  
  char text[32];
  memset(text, 0, 32);

  int bit = 1;
  for (int ii = 0; ii < 16; ii++) {
    if (iang & bit) {
      text[16-ii-1] = '1';
    } else {
      text[16-ii-1] = '0';
    }
    bit <<= 1;
  }

  return text;

}

