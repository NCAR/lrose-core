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
// NexradPowerStats.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2011
//
///////////////////////////////////////////////////////////////
//
// NexradPowerStats reads and ASCII file with power stats from
// NEXRAD calibrations, and analyses the data.
//
////////////////////////////////////////////////////////////////

#include "NexradPowerStats.hh"

#include <iostream>
#include <iomanip>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <algorithm>
#include <functional>
#include <toolsa/uusleep.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/file_io.h>
#include <toolsa/TaFile.hh>
#include <toolsa/Path.hh>

using namespace std;

// Constructor

NexradPowerStats::NexradPowerStats(int argc, char **argv)
  
{

  isOK = true;

  // set programe name
  
  _progName = "NexradPowerStats";

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

  // check we have a file

  if (_args.inputFileList.size() < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Must specify at least 1 file on the command line" << endl;
    _args.usage(_progName, cerr);
    isOK = false;
    return;
  }

  return;
  
}

// destructor

NexradPowerStats::~NexradPowerStats()
  
{

  if (_params.debug) {
    cerr << "NexradPowerStats done ..." << endl;
  }

}

//////////////////////////////////////////////////
// Run

int NexradPowerStats::Run ()
{

  int iret = 0;
  
  for (size_t ii = 0; ii < _args.inputFileList.size(); ii++) {
    string filePath = _args.inputFileList[ii];
    if (_processFile(filePath)) {
      iret = -1;
    }
    if (_writeResults()) {
      iret = -1;
    }
  } // ii
    
  return iret;
  
}

//////////////////////////////////////////////////
// process a file
//
// Returns 0 on success, -1 on failure

int NexradPowerStats::_processFile(const string& filePath)

{
  
  if (_params.debug) {
    cerr << "Processing file: " << filePath << endl;
  }

  // open file
  
  TaFile _in;
  FILE *in;
  if ((in = _in.fopenUncompress(filePath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - NexradPowerStats::_readFile";
    cerr << "  Cannot open file for reading: " << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  char line[BUFSIZ];
  
  // read cal results

  _xmitMode = SPLIT_50_50;
  _prevHTime = 0;
  _prevVTime = 0;
  _prevHPower = 0;
  _prevVPower = 0;
  _sumDiff = 0;
  _nDiff = 0;

  while (fgets(line, BUFSIZ, in) != NULL) {

    // remove line feed

    line[strlen(line)-1] = '\0';

    // process the line

    _processLine(line);

  }
  
  _in.fclose();
  
  return 0;

}

//////////////////////////////////////////////////
// process a line
//
// Returns 0 on success, -1 on failure

int NexradPowerStats::_processLine(const string& line)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << line << endl;
  }

#ifdef JUNK
  bool modeFound = false;
  if (line.find("V-only") != string::npos) {
    _xmitMode = V_ONLY;
    modeFound = true;
  } else if (line.find("H-only") != string::npos) {
    _xmitMode = H_ONLY;
    modeFound = true;
  } else if (line.find("50/50") != string::npos) {
    _xmitMode = SPLIT_50_50;
    modeFound = true;
  }

  if (_params.debug && modeFound) {
    switch (_xmitMode) {
      case H_ONLY:
        cerr << "Xmit mode is H_ONLY" << endl;
        break;
      case V_ONLY:
        cerr << "Xmit mode is V_ONLY" << endl;
        break;
    case SPLIT_50_50:
      cerr << "Xmit mode is SPLIT_50_50" << endl;
      break;
    }
  }
#endif

  int year, month, day, hour, min, sec;
  double xmitPower, hPower, vPower, imbalance;

  if (sscanf(line.c_str(),
             "%2d/%2d/%4d %2d:%2d:%2d XMTR Peak Power: %lg, H Power Sense: %lg, V Power Sense: %lg, Tx Imbalance: %lg",
             &month, &day, &year, &hour, &min, &sec,
             &xmitPower, &hPower, &vPower, &imbalance) == 10) {

    DateTime ptime(year, month, day, hour, min, sec);

    double hLoss = xmitPower - hPower;
    double vLoss = xmitPower - vPower;
    double delta = hPower - vPower;

    if (_params.debug) {
      cerr << "==>> time: " << DateTime::strm(ptime.utime()) << endl;
      cerr << "====>> xmitPower, hPower, vPower, imbalance: "
           << xmitPower << ", " << hPower << ", " << vPower << ", " << imbalance << endl;
      cerr << "====>> hLoss, vLoss, delta: " << hLoss << ", " << vLoss << ", " << delta << endl;
    }

    if (delta > 20) {
      _xmitMode = H_ONLY;
    } else if (delta < -20) {
      _xmitMode = V_ONLY;
    } else {
      _xmitMode = SPLIT_50_50;
    }

    if (_params.debug) {
      switch (_xmitMode) {
        case H_ONLY:
          cerr << "Xmit mode is H_ONLY" << endl;
          break;
        case V_ONLY:
          cerr << "Xmit mode is V_ONLY" << endl;
          break;
        case SPLIT_50_50:
          cerr << "Xmit mode is SPLIT_50_50" << endl;
          break;
      }
    }

    if (_xmitMode == H_ONLY && _prevVPower != 0) {
      int tDiff = ptime.utime() - _prevVTime;
      double pDiff = hPower - _prevVPower;
      if (_params.debug) {
        cerr << "==>> H tDiff, pDiff: " << tDiff << ", " << pDiff << endl;
      }
      if (tDiff < _params.max_time_diff_for_power_diff) {
        cout << "========>> H time, time Diff (sec), power Diff H-V (dB): "
             << DateTime::strm(ptime.utime()) << ", "
             << tDiff << ", " << pDiff << endl;
      }
      _sumDiff += pDiff;
      _nDiff++;
    } else if (_xmitMode == V_ONLY && _prevHPower != 0) {
      int tDiff = ptime.utime() - _prevHTime;
      double pDiff = _prevHPower - vPower;
      if (_params.debug) {
        cerr << "==>> V tDiff, pDiff: " << tDiff << ", " << pDiff << endl;
      }
      if (tDiff < _params.max_time_diff_for_power_diff) {
        cout << "========>> H time, time Diff (sec), power Diff H-V (dB): "
             << DateTime::strm(ptime.utime()) << ", "
             << tDiff << ", " << pDiff << endl;
      }
      _sumDiff += pDiff;
      _nDiff++;
    } else if (_xmitMode == SPLIT_50_50) {
      if (_nDiff > 0) {
        double meanDiff = _sumDiff / _nDiff;
        _sumDiff = 0;
        _nDiff = 0;
        cout << "=====================================================>> "
             << "mean power diff H-V (dB): "
             << meanDiff << endl;
      }
      cout << "========>> 50/50 time, hPower, vPower, power diff H-V (dB): "
           << DateTime::strm(ptime.utime()) << ", "
           << hPower << ", " << vPower << ", "
           << hPower - vPower << endl;
    }
    
    if (_xmitMode == H_ONLY) {
      _prevHTime = ptime.utime();
      _prevHPower = hPower;
    } else if (_xmitMode == V_ONLY) {
      _prevVTime = ptime.utime();
      _prevVPower = vPower;
    }
    
  }
             

  return 0;

}
  
//////////////////////////
// write out results data

int NexradPowerStats::_writeResults()
  
{

#ifdef JUNK

  // compute ASCII text file path
  
  char textPath[1024];
  sprintf(textPath, "%s/NexradPowerStats.%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
          _params.output_dir,
          ctime.getYear(),
          ctime.getMonth(),
          ctime.getDay(),
          ctime.getHour(),
          ctime.getMin(),
          ctime.getSec());
  
  if (_params.debug) {
    cout << "writing output calib TEXT file: " << textPath << endl;
  }

  // write to TEXT file
  
  FILE *outText;
  if ((outText = fopen(textPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - NexradPowerStats::_writeResultsFile";
    cerr << "  Cannot create file: " << textPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, " %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s\n",
	    "siggen",
            "Hc", "Vc", "Hx", "Vx",
            "HcmHx", "VcmVx", "wgH", "wgV",
            "HcNs", "VcNs", "HxNs", "VxNs");
  }
  
  fclose(outText);

#endif

  return 0;

}

