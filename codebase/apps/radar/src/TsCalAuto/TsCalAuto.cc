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
// TsCalAuto.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2009
//
///////////////////////////////////////////////////////////////
//
// TsCalAuto analyses time series data from sun scans
//
////////////////////////////////////////////////////////////////

#include "TsCalAuto.hh"
#include "Stats.hh"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <algorithm>
#include <functional>
#include <toolsa/uusleep.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/TaFile.hh>
#include <toolsa/Path.hh>
#include <rapformats/DsRadarCalib.hh>
#include <toolsa/Socket.hh>

const double TsCalAuto::piCubed = pow(M_PI, 3.0);
const double TsCalAuto::lightSpeed = 299792458.0;

using namespace std;

// Constructor

TsCalAuto::TsCalAuto(int argc, char **argv)
  
{

  isOK = true;
  _pulseReader = NULL;
  _haveChan1 = false;
  _fmqMode = false;
  _prevFreq = -9999;

  // set programe name
  
  _progName = "TsCalAuto";

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

  // are we in filelist or FMQ mode?

  if (_args.inputFileList.size() < 1 &&
      _args.hChannelFileList.size() < 1 &&
      _args.vChannelFileList.size() < 1) {
    _fmqMode = true;
  } else {
    _fmqMode = false;
  }

  // initialize sampling details

  _nSamples = _params.n_samples;
  _startGateRequested = _params.start_gate;
  _nGatesRequested = _params.n_gates;
  double frequencyGhz = _params.siggen_frequency;
  if (_params.radar_frequency > 0) {
    frequencyGhz = _params.radar_frequency;
  }
  _wavelengthM = lightSpeed / (frequencyGhz * 1.0e9);
  _wavelengthCm = _wavelengthM * 100.0;

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
  return;
  
}

// destructor

TsCalAuto::~TsCalAuto()
  
{

  _closePulseReader();

  if (_params.debug) {
    cerr << "TsCalAuto done ..." << endl;
  }

}

//////////////////////////////////////////////////
// Run

int TsCalAuto::Run ()
{

  int iret = 0;
  
  if (_fmqMode) {

    iret = _runFmqMode();

  } else {
    
    iret = _runFileMode();

  }
  
  return iret;
  
}

///////////////////////////////////////////////////
// clear arrays

void TsCalAuto::_clearArrays()
{
  _siggenDbm.clear();
  _waveguideDbmH.clear();
  _waveguideDbmV.clear();
  _hcDbm.clear();
  _hxDbm.clear();
  _vcDbm.clear();
  _vxDbm.clear();
  _hcMinusVcDbm.clear();
  _hxMinusVxDbm.clear();
  _hcDbmNs.clear();
  _hxDbmNs.clear();
  _vcDbmNs.clear();
  _vxDbmNs.clear();
}

//////////////////////////////////////////////////
// Run in fmq mode

int TsCalAuto::_runFmqMode()
{

  // initialize

  _calTime = time(NULL);
  _clearArrays();

  int iret = 0;

  if (_params.suspend_test_pulse) {
    _suspendTestPulse();
  }

  // Turn on the Siggen and  loop through the siggen power settings

  _setSiggenRF(true);
  if(_params.set_sig_freq) {
    _setSiggenFreq(_params.siggen_frequency);
  }

  double powerDbm = 0.0;

  if (_params.siggen_specify_power_sequence) {
    
    // use specified power sequence

    for (int jj = 0; jj < _params.siggen_delta_power_sequence_n; jj++) {
      powerDbm = (_params.siggen_sequence_start_power +
                  _params._siggen_delta_power_sequence[jj]);
      _setSiggenPower(powerDbm);
      umsleep(500);

      // get received powers
      if (_sampleReceivedPowers(powerDbm)) {
        return -1;
      }
    } // jj

  } else {

    // create power sequence

    powerDbm = _params.siggen_max_power;
    while (powerDbm >= _params.siggen_min_power) {
    
      _setSiggenPower(powerDbm);
      umsleep(500);

      // get received powers
      
      if (_sampleReceivedPowers(powerDbm)) {
        return -1;
      }
      
      // reduce power
      
      powerDbm -= _params.siggen_delta_power;
      
    } // while

  }

  // Turn off the siggen and get 4 more data points

  _setSiggenRF(false);

  for (int ii = 0; ii < 4; ii++) {

    // get received powers
    
    if (_sampleReceivedPowers(powerDbm)) {
      return -1;
    }
    
    // reduce power
    
    powerDbm -= _params.siggen_delta_power;

  }
  
  _setSiggenRF(true);  // Turn it back on so the fron panel is active.

  // compute radar constants

  _radarConstH = _computeRadarConstant(_params.xmitPowerDbmH,
                                       _params.antGainDbH,
                                       _params.twoWayWaveguideLossDbH,
                                       _params.twoWayRadomeLossDbH);
  
  _radarConstV = _computeRadarConstant(_params.xmitPowerDbmV,
                                       _params.antGainDbV,
                                       _params.twoWayWaveguideLossDbV,
                                       _params.twoWayRadomeLossDbV);
    
  // compute cals
  
  _computeCal("Hc", _waveguideDbmH, _hcDbm, _hcDbmNs, _resultHc, _radarConstH);
  _computeCal("Hx", _waveguideDbmH, _hxDbm, _hxDbmNs, _resultHx, _radarConstH);
  _computeCal("Vc", _waveguideDbmV, _vcDbm, _vcDbmNs, _resultVc, _radarConstV);
  _computeCal("Vx", _waveguideDbmV, _vxDbm, _vxDbmNs, _resultVx, _radarConstV);

  if (_writeResults()) {
    iret = -1;
  }

  // reset siggen if requested

  if (_params.reset_siggen_power_after_cal) {

    if(_params.set_sig_freq) {
      _setSiggenFreq(_params.siggen_frequency);
    }
    _setSiggenPower(_params.siggen_power_val_after_cal);
    _setSiggenRF(true);

  }

  if (_params.suspend_test_pulse) {
    _resumeTestPulse();
  }

  return iret;
  
}

//////////////////////////////////////////////////
// Run in file mode

int TsCalAuto::_runFileMode()
{

  int iret = 0;

  if (_args.inputFileList.size() > 0) {

    for (size_t ii = 0; ii < _args.inputFileList.size(); ii++) {
      string filePath = _args.inputFileList[ii];
      if (_processFile(filePath)) {
        iret = -1;
      }
      if (_writeResults()) {
        iret = -1;
      }
    } // ii
    
  } else {
    
    for (size_t ii = 0; ii < _args.hChannelFileList.size(); ii++) {
      if (ii < _args.vChannelFileList.size()) {
        string hPath = _args.hChannelFileList[ii];
        string vPath = _args.vChannelFileList[ii];
        if (_processFiles(hPath, vPath)) {
          iret = -1;
        }
      }
      if (_writeResults()) {
        iret = -1;
      }
    } // ii

  }


  return iret;
  
}

//////////////////////////////////////////////////
// process a file
//
// Returns 0 on success, -1 on failure

int TsCalAuto::_processFile(const string& filePath)

{
  
  // initialize

  _calTime = time(NULL);
  _clearArrays();

  if (_params.debug) {
    cerr << "Processing file: " << filePath << endl;
  }

  if (_readFile(filePath)) {
    cerr << "ERROR - TsCalAuto::_processFile";
    cerr << "  Cannot read file: " << filePath << endl;
    return -1;
  }

  _doCal();
  return 0;

}

//////////////////////////////////////////////////
// process H and V file
//
// Returns 0 on success, -1 on failure

int TsCalAuto::_processFiles(const string &hPath, const string &vPath)

{
  
  // initialize

  _calTime = time(NULL);
  _clearArrays();

  if (_params.debug) {
    cerr << "Processing files" << endl;
    cerr << "  H: " << hPath << endl;
    cerr << "  V: " << vPath << endl;
  }

  if (_readFiles(hPath, vPath)) {
    cerr << "ERROR - TsCalAuto::_processFile";
    return -1;
  }

  _doCal();
  return 0;

}

//////////////////////////////////////////////////
// read a file
//
// Returns 0 on success, -1 on failure

int TsCalAuto::_readFile(const string& filePath)

{

  // if possible, get time from file name

  int year, month, day, hour, min, sec;
  Path fpath(filePath);
  string fname(fpath.getFile());

  for (size_t ii = 0; ii < fname.size(); ii++) {
    if (sscanf(fname.c_str() + ii, "%4d%2d%2d_%2d%2d%2d",
               &year, &month, &day, &hour, &min, &sec) == 6) {
      DateTime ctime(year, month, day, hour, min, sec);
      _calTime = ctime.utime();
      break;
    }
  }

  // open file
  
  TaFile _in;
  FILE *in;
  if ((in = _in.fopenUncompress(filePath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - TsCalAuto::_readFile";
    cerr << "  Cannot open file for reading: " << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  char line[BUFSIZ];
  
  // read cal results
  
  while (fgets(line, BUFSIZ, in) != NULL) {
    if (line[0] == '#') {
      continue;
    }
    double siggen, val1, val2, val3, val4;
    if (sscanf(line, "%lg%lg%lg%lg%lg",
               &siggen, &val1, &val2, &val3, &val4) == 5) {
      _siggenDbm.push_back(siggen);
      if (_params.read_data_in_alt_column_order) {
        _hcDbm.push_back(val1);
        _hxDbm.push_back(val2);
        _vcDbm.push_back(val3);
        _vxDbm.push_back(val4);
      } else {
        _hcDbm.push_back(val1);
        _vcDbm.push_back(val2);
        _hxDbm.push_back(val3);
        _vxDbm.push_back(val4);
      }
    } else if (sscanf(line, "%lg%lg%lg",
                      &siggen, &val1, &val2) == 3) {
      _siggenDbm.push_back(siggen);
      _hcDbm.push_back(val1);
      _vcDbm.push_back(val2);
      _hxDbm.push_back(-999.9);
      _vxDbm.push_back(-999.9);
    } else if (sscanf(line, "%lg%lg",
                      &siggen, &val1) == 2) {
      _siggenDbm.push_back(siggen);
      _hcDbm.push_back(val1);
      _vcDbm.push_back(-999.9);
      _hxDbm.push_back(-999.9);
      _vxDbm.push_back(-999.9);
    }
  }
  
  _in.fclose();
  
  // compute power ratios and
  // waveguide power adjusting injected power
  // for circuit gain and coupling factors
  
  for (int ii = 0; ii < (int) _siggenDbm.size(); ii++) {
    double hcMinusVc = _hcDbm[ii] - _vcDbm[ii];
    double hxMinusVx = _hxDbm[ii] - _vxDbm[ii];
    double waveguideH = _siggenDbm[ii] - 
      (_params.powerMeasLossDbH + _params.couplerForwardLossDbH);
    double waveguideV = _siggenDbm[ii] - 
      (_params.powerMeasLossDbV + _params.couplerForwardLossDbV);
    _hcMinusVcDbm.push_back(hcMinusVc);
    _hxMinusVxDbm.push_back(hxMinusVx);
    _waveguideDbmH.push_back(waveguideH);
    _waveguideDbmV.push_back(waveguideV);
  }
  
  return 0;

}

//////////////////////////////////////////////////
// read H and V files
//
// Returns 0 on success, -1 on failure

int TsCalAuto::_readFiles(const string &hPath, const string &vPath)

{

  // if possible, get time from file name
  
  int year, month, day, hour, min, sec;
  Path fpath(hPath);
  string fname(fpath.getFile());
  if (sscanf(fname.c_str(), "TsCalAuto.%4d%2d%2d_%2d%2d%2d",
             &year, &month, &day, &hour, &min, &sec) == 6) {
    DateTime ctime(year, month, day, hour, min, sec);
    _calTime = ctime.utime();
  }
  
  // open files
  
  TaFile _inH;
  FILE *inH;
  if ((inH = _inH.fopenUncompress(hPath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - TsCalAuto::_readFiles";
    cerr << "  Cannot open H channel file for reading: " << hPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  TaFile _inV;
  FILE *inV;
  if ((inV = _inV.fopenUncompress(vPath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - TsCalAuto::_readFiles";
    cerr << "  Cannot open V channel file for reading: " << vPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  char lineH[BUFSIZ];
  char lineV[BUFSIZ];
  
  // read cal results
  
  while (fgets(lineH, BUFSIZ, inH) != NULL) {
    if (fgets(lineV, BUFSIZ, inV) != NULL) {

      if (lineH[0] == '#') {
        continue;
      }
      if (lineV[0] == '#') {
        continue;
      }

      double siggenH, hcH, hxH, vcH, vxH;
      double siggenV, hcV, hxV, vcV, vxV;
      
      if (sscanf(lineH, "%lg%lg%lg%lg%lg",
                 &siggenH, &hcH, &vcH, &hxH, &vxH) == 5) {
        
        if (sscanf(lineV, "%lg%lg%lg%lg%lg",
                   &siggenV, &hcV, &vcV, &hxV, &vxV) == 5) {
          
          if (fabs(siggenH -siggenV) < 0.01) {
            _siggenDbm.push_back(siggenH);
            _hcDbm.push_back(hcH);
            _hxDbm.push_back(hxH);
            _vcDbm.push_back(vcV);
            _vxDbm.push_back(vxV);
          }

        }

      } else if (sscanf(lineH, "%lg%lg%lg",
                        &siggenH, &hcH, &vcH) == 3) {

        if (sscanf(lineV, "%lg%lg%lg",
                   &siggenV, &hcV, &vcV) == 3) {
          
          if (fabs(siggenH -siggenV) < 0.01) {
            _siggenDbm.push_back(siggenH);
            _hcDbm.push_back(hcH);
            _vcDbm.push_back(vcV);
            _hxDbm.push_back(-999.9);
            _vxDbm.push_back(-999.9);
          }

        }
        
      }
      
    } // if (fgets ...
  } // while (fgets ...
  
  _inH.fclose();
  _inV.fclose();
  
  // compute power ratios and
  // waveguide power adjusting injected power
  // for circuit gain and coupling factors
  
  for (int ii = 0; ii < (int) _siggenDbm.size(); ii++) {
    double hcMinusVc = _hcDbm[ii] - _vcDbm[ii];
    double hxMinusVx = _hxDbm[ii] - _vxDbm[ii];
    double waveguideH = _siggenDbm[ii] - 
      (_params.powerMeasLossDbH + _params.couplerForwardLossDbH);
    double waveguideV = _siggenDbm[ii] - 
      (_params.powerMeasLossDbV + _params.couplerForwardLossDbV);
    _hcMinusVcDbm.push_back(hcMinusVc);
    _hxMinusVxDbm.push_back(hxMinusVx);
    _waveguideDbmH.push_back(waveguideH);
    _waveguideDbmV.push_back(waveguideV);
  }
  
  return 0;

}

//////////////////////////////////////////////////
// do the calibration
 
void TsCalAuto::_doCal()
   
{
  
  if (_params.debug) {
    fprintf(stderr, "TsCalAuto input\n");
    fprintf(stderr, "=============\n");
    fprintf(stderr, "Data time: %s\n",
            DateTime::strm((time_t) _calTime).c_str());
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "\n");
      fprintf(stderr, "%10s %10s %10s %10s %10s %10s %10s %10s %10s\n",
              "Siggen", "Hc", "Vc", "Hx", "Vx", "Hc-Nx", "Vc-Vx", "wgH", "wgV");
      for (int ii = 0; ii < (int) _siggenDbm.size(); ii++) {
        fprintf(stderr, "%10g %10g %10g %10g %10g %10g %10g %10g %10g\n",
                _siggenDbm[ii],
                _hcDbm[ii], _vcDbm[ii], _hxDbm[ii], _vxDbm[ii],
                _hcMinusVcDbm[ii], _hxMinusVxDbm[ii],
                _waveguideDbmH[ii], _waveguideDbmV[ii]);
      }
    }
  }

  // compute radar constants

  _radarConstH = _computeRadarConstant(_params.xmitPowerDbmH,
                                       _params.antGainDbH,
                                       _params.twoWayWaveguideLossDbH,
                                       _params.twoWayRadomeLossDbH);
    
  _radarConstV = _computeRadarConstant(_params.xmitPowerDbmV,
                                       _params.antGainDbV,
                                       _params.twoWayWaveguideLossDbV,
                                       _params.twoWayRadomeLossDbV);
    
  // compute cals
  
  _computeCal("Hc", _waveguideDbmH, _hcDbm, _hcDbmNs, _resultHc, _radarConstH);
  _computeCal("Hx", _waveguideDbmH, _hxDbm, _hxDbmNs, _resultHx, _radarConstH);
  _computeCal("Vc", _waveguideDbmV, _vcDbm, _vcDbmNs, _resultVc, _radarConstV);
  _computeCal("Vx", _waveguideDbmV, _vxDbm, _vxDbmNs, _resultVx, _radarConstV);

}

//////////////////////////////////////////////////
// compute the cal for a data set

void TsCalAuto::_computeCal(const string &label,
                            const vector<double> &inputDbm,
                            const vector<double> &outputDbm,
                            vector<double> &outputDbmNs,
                            ChannelResult &chanResult,
                            double radarConst)

{
  
  // compute noise

  double sumNoise = 0.0;
  double countNoise = 0.0;

  for (int ii = 0; ii < (int) inputDbm.size(); ii++) {
    if (_siggenDbm[ii] <= _params.noise_region_max_power) {
      sumNoise += pow(10.0, outputDbm[ii] / 10.0);
      countNoise++;
    }
  }

  double meanNoise = 1.0e-20;
  double noiseDbm = -9999;
  if (countNoise > 0) {
    meanNoise = sumNoise / countNoise;
    noiseDbm = 10.0 * log10(meanNoise);
  }

  // compute noise-subtracted powers

  outputDbmNs.clear();
  double prevDbmNs = 0.0;
  bool belowNoise = false;
  if (noiseDbm > -9990) {
    for (int ii = 0; ii < (int) outputDbm.size(); ii++) {
      if (outputDbm[ii] > -990) {
        double outputPower = pow(10.0, outputDbm[ii] / 10.0);
        double outputPowerNs = outputPower - meanNoise;
        double dbmNs = prevDbmNs;
        if (outputPowerNs > 0 && !belowNoise) {
          dbmNs = 10.0 * log10(outputPowerNs);
          prevDbmNs = dbmNs;
        } else {
          belowNoise = true;
        }
        outputDbmNs.push_back(dbmNs);
      } else {
        outputDbmNs.push_back(-999.9);
      }
    }
  } else {
    for (int ii = 0; ii < (int) outputDbm.size(); ii++) {
      outputDbmNs.push_back(-999.9);
    }
  }
  
  // compute gain

  double sumGain = 0.0;
  double countGain = 0.0;
  double gainDbm;

  for (int ii = 0; ii < (int) inputDbm.size(); ii++) {
    if (_siggenDbm[ii] >= _params.linear_region_min_power &&
        _siggenDbm[ii] <= _params.linear_region_max_power) {
      double gain = outputDbmNs[ii] - inputDbm[ii];
      sumGain += pow(10.0, gain / 10.0);
      countGain++;
    }
  }
  
  if (countGain > 0) {
    double meanGain = sumGain / countGain;
    gainDbm = 10.0 * log10(meanGain);
  } else {
    gainDbm = -9999;
  }
  
  // Find the maximum output power

  double satDbm = -9999;
  for (int ii = 0; ii < (int) inputDbm.size(); ii++) {
    if (_siggenDbm[ii] >= _params.noise_region_max_power) {
      if(outputDbmNs[ii] > satDbm) satDbm = outputDbmNs[ii];
    }
  }

  // dynamic range

  double dynRange = -9999;
  if (noiseDbm > -9990 && satDbm > -9990) {
    dynRange = satDbm - noiseDbm - _params.snr_for_mds;
  }

  // I0
  
  double i0Dbm;
  if (noiseDbm > -9990 && gainDbm > -9990) {
    i0Dbm = noiseDbm - gainDbm;
  } else {
    i0Dbm = -9999;
  }

  // compute linear fit to data

  vector<double> xx, yy;

  for (int ii = 0; ii < (int) inputDbm.size(); ii++) {
    if (_siggenDbm[ii] >= _params.linear_region_min_power &&
        _siggenDbm[ii] <= _params.linear_region_max_power) {
      xx.push_back(inputDbm[ii]);
      yy.push_back(outputDbmNs[ii]);
    }
  }
  
  double gain, slope;
  double xmean, ymean;
  double xsdev, ysdev;
  double corr, stdErrEst, rSquared;
  
  _linearFit(xx, yy,
             gain, slope,
             xmean, ymean,
             xsdev, ysdev,
             corr, stdErrEst, rSquared);
  
  double dbz0 = noiseDbm - gain + radarConst;

  if (_params.debug) {
    cout << "**** Cal for: " << label  << " ****" << endl;
    cout << label << "  noiseDbm: " << noiseDbm << endl;
    cout << label << "  gain: " << gain << endl;
    cout << label << "  gainDbm: " << gainDbm << endl;
    cout << label << "  i0Dbm: " << i0Dbm << endl;
    cout << label << "  dbz0: " << dbz0 << endl;
    cout << label << "  slope: " << slope << endl;
    cout << label << "  corr: " << corr << endl;
    cout << label << "  stdErrEst: " << stdErrEst << endl;
    cout << label << "  rSquared: " << rSquared << endl;
    cout << label << "  SatDBM: " << satDbm << endl;
    cout << label << "  Dyn Range: " << dynRange << endl;
  }

  chanResult.noiseDbm = noiseDbm;
  chanResult.gainDbm = gainDbm;
  chanResult.i0Dbm = i0Dbm;
  chanResult.slope = slope;
  chanResult.corr = corr;
  chanResult.stdErrEst = stdErrEst;
  chanResult.dbz0 = dbz0;
  chanResult.satDbm = satDbm;
  chanResult.dynamicRangeDb = dynRange;

}

//////////////////////////
// write out results data

int TsCalAuto::_writeResults()
  
{

  // fill out DsRadarCalib object

  DsRadarCalib calib;

  calib.setRadarName(_params.radarName);
  calib.setCalibTime(_calTime);

  calib.setWavelengthCm(_wavelengthCm);

  calib.setBeamWidthDegH(_params.beamWidthDegH);
  calib.setBeamWidthDegV(_params.beamWidthDegV);

  calib.setAntGainDbH(_params.antGainDbH);
  calib.setAntGainDbV(_params.antGainDbV);

  calib.setPulseWidthUs(_params.pulseWidthUs);

  calib.setXmitPowerDbmH(_params.xmitPowerDbmH);
  calib.setXmitPowerDbmV(_params.xmitPowerDbmV);

  calib.setTwoWayWaveguideLossDbH(_params.twoWayWaveguideLossDbH);
  calib.setTwoWayWaveguideLossDbV(_params.twoWayWaveguideLossDbV);

  calib.setTwoWayRadomeLossDbH(_params.twoWayRadomeLossDbH);
  calib.setTwoWayRadomeLossDbV(_params.twoWayRadomeLossDbV);

  calib.setReceiverMismatchLossDb(_params.receiverMismatchLossDb);

  calib.setKSquaredWater(_params.k_squared);

  calib.setRadarConstH(_radarConstH);
  calib.setRadarConstV(_radarConstV);

  calib.setNoiseDbmHc(_resultHc.noiseDbm);
  calib.setNoiseDbmVc(_resultVc.noiseDbm);
  calib.setI0DbmHc(_resultHc.i0Dbm);
  calib.setI0DbmVc(_resultVc.i0Dbm);
  calib.setDynamicRangeDbHc(_resultHc.dynamicRangeDb);
  calib.setDynamicRangeDbVc(_resultVc.dynamicRangeDb);

  if (_params.switching_receivers) {
    calib.setNoiseDbmHx(_resultHx.noiseDbm);
    calib.setNoiseDbmVx(_resultVx.noiseDbm);
    calib.setI0DbmHx(_resultHx.i0Dbm);
    calib.setI0DbmVx(_resultVx.i0Dbm);
    calib.setDynamicRangeDbHx(_resultHx.dynamicRangeDb);
    calib.setDynamicRangeDbVx(_resultVx.dynamicRangeDb);
  } else {
    calib.setNoiseDbmHx(_resultHc.noiseDbm);
    calib.setNoiseDbmVx(_resultVc.noiseDbm);
    calib.setI0DbmHx(_resultHc.i0Dbm);
    calib.setI0DbmVx(_resultVc.i0Dbm);
    calib.setDynamicRangeDbHx(_resultHc.dynamicRangeDb);
    calib.setDynamicRangeDbVx(_resultVc.dynamicRangeDb);
  }

  calib.setReceiverGainDbHc(_resultHc.gainDbm);
  calib.setReceiverGainDbVc(_resultVc.gainDbm);
  
  if (_params.switching_receivers) {
    calib.setReceiverGainDbHx(_resultHx.gainDbm);
    calib.setReceiverGainDbVx(_resultVx.gainDbm);
  } else {
    calib.setReceiverGainDbHx(_resultHc.gainDbm);
    calib.setReceiverGainDbVx(_resultVc.gainDbm);
  }

  calib.setReceiverSlopeDbHc(_resultHc.slope);
  calib.setReceiverSlopeDbVc(_resultVc.slope);

  if (_params.switching_receivers) {
    calib.setReceiverSlopeDbHx(_resultHx.slope);
    calib.setReceiverSlopeDbVx(_resultVx.slope);
  } else {
    calib.setReceiverSlopeDbHx(_resultHc.slope);
    calib.setReceiverSlopeDbVx(_resultVc.slope);
  }

  calib.setBaseDbz1kmHc(_resultHc.dbz0);
  calib.setBaseDbz1kmHx(_resultHx.dbz0);
  calib.setBaseDbz1kmVc(_resultVc.dbz0);
  calib.setBaseDbz1kmVx(_resultVx.dbz0);

  if (_params.switching_receivers) {
    calib.setBaseDbz1kmHx(_resultHx.dbz0);
    calib.setBaseDbz1kmVx(_resultVx.dbz0);
  } else {
    calib.setBaseDbz1kmHx(_resultHc.dbz0);
    calib.setBaseDbz1kmVx(_resultVc.dbz0);
  }

  calib.setSunPowerDbmHc(-9999);
  calib.setSunPowerDbmHx(-9999);
  calib.setSunPowerDbmVc(-9999);
  calib.setSunPowerDbmVx(-9999);

  calib.setNoiseSourcePowerDbmH(_params.noiseSourcePowerDbmH);
  calib.setNoiseSourcePowerDbmV(_params.noiseSourcePowerDbmV);

  calib.setPowerMeasLossDbH(_params.powerMeasLossDbH);
  calib.setPowerMeasLossDbV(_params.powerMeasLossDbV);

  calib.setCouplerForwardLossDbH(_params.couplerForwardLossDbH);
  calib.setCouplerForwardLossDbV(_params.couplerForwardLossDbV);

  //   double zdrCorrectionDb =
  //     (_resultHc.dbz0 - _resultVc.dbz0) - (_resultHc.noiseDbm - _resultVc.noiseDbm);
  double zdrCorrectionDb = 0.0;
  calib.setZdrCorrectionDb(zdrCorrectionDb);

  //   double ldrCorrectionDbH =
  //     (_resultVx.dbz0 - _resultHc.dbz0) - (_resultVx.noiseDbm - _resultHc.noiseDbm);
  double ldrCorrectionDbH = 0.0;
  calib.setLdrCorrectionDbH(ldrCorrectionDbH);

  //   double ldrCorrectionDbV =
  //     (_resultHx.dbz0 - _resultVc.dbz0) - (_resultHx.noiseDbm - _resultVc.noiseDbm);
  double ldrCorrectionDbV = 0.0;
  calib.setLdrCorrectionDbV(ldrCorrectionDbV);

  calib.setSystemPhidpDeg(_params.systemPhidpDeg);

  string calibXml;
  calib.convert2Xml(calibXml);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cout << "============== Calibration XML =================" << endl;
    cout << calibXml;
    cout << "================================================" << endl;
  }
  
  // create the directory for the output file
  
  if (ta_makedir_recurse(_params.output_dir)) {
    int errNum = errno;
    cerr << "ERROR - TsCalAuto::_writeResults";
    cerr << "  Cannot create output dir: " << _params.output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute XML file path
  
  const DateTime ctime(_calTime);
  char xmlPath[1024];
  string label;
  if (strlen(_params.output_file_label) > 0) {
    label += _params.output_file_label;
    label += "_";
  }
  sprintf(xmlPath, "%s/TsCalAuto_%s%.4d%.2d%.2d_%.2d%.2d%.2d.xml",
          _params.output_dir,
          label.c_str(),
          ctime.getYear(),
          ctime.getMonth(),
          ctime.getDay(),
          ctime.getHour(),
          ctime.getMin(),
          ctime.getSec());
  
  if (_params.debug) {
    cerr << "writing output calib XML file: " << xmlPath << endl;
  }

  // write to XML file
  
  FILE *outXml;
  if ((outXml = fopen(xmlPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - TsCalAuto::_writeResultsFile";
    cerr << "  Cannot create file: " << xmlPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  fprintf(outXml, "%s", calibXml.c_str());
  fclose(outXml);

  // compute ASCII text file path
  
  char textPath[1024];
  sprintf(textPath, "%s/TsCalAuto_%s%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
          _params.output_dir,
          label.c_str(),
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
    cerr << "ERROR - TsCalAuto::_writeResultsFile";
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
  
  for (int ii = 0; ii < (int) _siggenDbm.size(); ii++) {
    double siggen = _siggenDbm[ii];

    double hc = _hcDbm[ii];
    double hx = _hxDbm[ii];
    double vc = _vcDbm[ii];
    double vx = _vxDbm[ii];
    double hcmhx = _hcMinusVcDbm[ii];
    double vcmvx = _hxMinusVxDbm[ii];
    double wgh = _waveguideDbmH[ii];
    double wgv = _waveguideDbmV[ii];
    double hcNs = _hcDbmNs[ii];
    double hxNs = _hxDbmNs[ii];
    double vcNs = _vcDbmNs[ii];
    double vxNs = _vxDbmNs[ii];

    fprintf(outText,
            " %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f "
            "%10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f\n",
	    siggen, hc, vc, hx, vx, hcmhx, vcmvx, wgh, wgv, hcNs, vcNs, hxNs, vxNs);
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr,
              " %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f "
              "%10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f\n",
              siggen, hc, vc, hx, vx, hcmhx, vcmvx, wgh, wgv, hcNs, vcNs, hxNs, vxNs);
    }
    
  }

  fclose(outText);
  if(strlen(_params.plot_script) > 2) {
      char cmd[1024];
      sprintf(cmd,"%s %s %s\n",_params.plot_script, textPath, _paramsPath);
      int iret = system(cmd);
      if (_params.debug >= Params::DEBUG_EXTRA) {
        cerr << "Calling cmd: " << cmd << endl;
        cerr << "  iret: " << iret << endl;
      }
  }
  return 0;

}

//////////////////////////////////////////////////////////////////////////
// _linearFit: fit a line to a data series
//
//  n: number of points in (x, y) data set
//  x: array of x data
//  y: array of y data
//  xmean, ymean: means
//  xsdev, ysdev: standard deviations
//  corr: correlation coefficient
//  a[] - linear coefficients (a[0] - bias, a[1] - scale)
//
// Returns 0 on success, -1 on error.
//
//////////////////////////////////////////////////////////////////////////
 
int TsCalAuto::_linearFit(const vector<double> &x,
                         const vector<double> &y,
                         double &gain,
                         double &slope,
                         double &xmean,
                         double &ymean,
                         double &xsdev,
                         double &ysdev,
                         double &corr,
                         double &stdErrEst,
                         double &rSquared)

{

  // initialize

  gain = -9999;
  slope = -9999;
  xmean = -9999;
  ymean = -9999;
  xsdev = -9999;
  ysdev = -9999;
  corr = -9999;
  stdErrEst = -9999;
  rSquared = -9999;

  double sumx = 0.0, sumx2 = 0.0;
  double sumy = 0.0, sumy2 = 0.0, sumxy = 0.0;
  double dn = (double) x.size();
  double sum_dy_squared = 0.0;
  double sum_of_residuals = 0.0;

  if (x.size() < 2) {
    return -1;
  }

  // sum the various terms

  for (int i = 0; i < (int) x.size(); i++) {
    double xval = x[i];
    double yval = y[i];
    sumx += xval;
    sumx2 += xval * xval;
    sumy += yval;
    sumy2 += yval * yval;
    sumxy += xval * yval;
  }
  
  // compute the terms

  // double term1 = dn * sumx2  - sumx * sumx;
  // double term2 = sumy * sumx2 - sumx * sumxy;
  double term3 = dn * sumxy - sumx * sumy;
  double term4 = (dn * sumx2 - sumx * sumx);
  double term5 = (dn * sumy2 - sumy * sumy);
  
  // compute mean and standard deviation
  
  xmean = sumx / dn;
  ymean = sumy / dn;

  gain = ymean - xmean;
  
  xsdev = sqrt(fabs(term4)) / (dn - 1.0);
  ysdev = sqrt(fabs(term5)) / (dn - 1.0);
  
  // compute correlation coefficient

  corr = term3 / sqrt(fabs(term4 * term5));

 // get y-on-x slope

  double num = dn * sumxy - sumx * sumy;
  double denom = dn * sumx2 - sumx * sumx;
  double slope_y_on_x;
  
  if (denom != 0.0) {
    slope_y_on_x = num / denom; // mm/s
  } else {
    slope_y_on_x = 0.0;
  }

  // get x-on-y slope

  denom = dn * sumy2 - sumy * sumy;
  double slope_x_on_y;
  
  if (denom != 0.0) {
    slope_x_on_y = num / denom; // mm/s
  } else {
    slope_x_on_y = 0.0;
  }

  // combine slopes for final slope

  if (slope_y_on_x != 0.0 && slope_x_on_y != 0.0) {
    slope = (slope_y_on_x + 1.0 / slope_x_on_y) / 2.0;
  } else if (slope_y_on_x != 0.0) {
    slope = slope_y_on_x;
  } else if (slope_x_on_y != 0.0) {
    slope = 1.0 / slope_x_on_y;
  } else {
    slope = 0.0;
  }

  // compute the sum of the residuals

  for (int i = 0; i < (int) x.size(); i++) {
    double xval = x[i];
    double yval = y[i];
    double yEst = (xval - xmean) * slope + ymean;
    double error = yEst - yval;
    sum_of_residuals += error * error;
    sum_dy_squared += (yval - ymean) * (yval - ymean);
  }

  // compute standard error of estimate and r-squared
  
  stdErrEst = sqrt(sum_of_residuals / (dn - 3.0));
  rSquared = ((sum_dy_squared - sum_of_residuals) /
              sum_dy_squared);

  return 0;
  
}

//////////////////////////////////////////////////
// compute radar constant, in meter units

double TsCalAuto::_computeRadarConstant(double xmitPowerDbm,
                                        double antennaGainDb,
                                        double twoWayWaveguideLossDb,
                                        double twoWayRadomeLossDb)
                                     
                                     
{

  // double antennaGainDb = (_params.antGainDbH + _params.antGainDbV) / 2.0;
  double antGainLinear = pow(10.0, antennaGainDb / 10.0);
  double gainSquared = antGainLinear * antGainLinear;
  double lambdaSquared = _wavelengthM * _wavelengthM;
  double pulseMeters = _params.pulseWidthUs * 1.0e-6 * lightSpeed;
  
  double hBeamWidthRad = _params.beamWidthDegH * DEG_TO_RAD;
  double vBeamWidthRad = _params.beamWidthDegV * DEG_TO_RAD;

  double peakPowerMilliW = pow(10.0, xmitPowerDbm / 10.0);

  double theoreticalG = (M_PI * M_PI) / (hBeamWidthRad * vBeamWidthRad);
  double theoreticalGdB = 10.0 * log10(theoreticalG);
  
  double receiverMismatchLossDb = _params.receiverMismatchLossDb;

  double kSquared = _params.k_squared;
  double denom = (peakPowerMilliW * piCubed * pulseMeters * gainSquared *
                  hBeamWidthRad * vBeamWidthRad * kSquared * 1.0e-24);

  double num = (1024.0 * log(2.0) * lambdaSquared);
  
  double factor = num / denom;
  
  double radarConst = (10.0 * log10(factor)
                       + twoWayWaveguideLossDb
                       + twoWayRadomeLossDb
                       + receiverMismatchLossDb);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cout << "==== Computing radar constant ====" << endl;
    cout << "  wavelengthCm: " << _wavelengthCm << endl;
    cout << "  horizBeamWidthDeg: " << _params.beamWidthDegH << endl;
    cout << "  vertBeamWidthDeg: " << _params.beamWidthDegV << endl;
    cout << "  antGainDb: " << antennaGainDb << endl;
    cout << "  theoretical antGainDb: " << theoreticalGdB << endl;
    cout << "  xmitPowerDbm: " << xmitPowerDbm << endl;
    cout << "  pulseWidthUs: " << _params.pulseWidthUs << endl;
    cout << "  waveguideLoss: " << twoWayWaveguideLossDb << endl;
    cout << "  radomeLoss: " << twoWayRadomeLossDb << endl;
    cout << "  receiverLoss: " << receiverMismatchLossDb << endl;
    cout << "  antGainLinear: " << antGainLinear << endl;
    cout << "  gainSquared: " << gainSquared << endl;
    cout << "  lambdaSquared: " << lambdaSquared << endl;
    cout << "  pulseMeters: " << pulseMeters << endl;
    cout << "  hBeamWidthRad: " << hBeamWidthRad << endl;
    cout << "  vBeamWidthRad: " << vBeamWidthRad << endl;
    cout << "  peakPowerMilliW: " << peakPowerMilliW << endl;
    cout << "  piCubed: " << piCubed << endl;
    cout << "  kSquared: " << kSquared << endl;
    cout << "  num: " << num << endl;
    cout << "  denom: " << denom << endl;
    cout << "  factor: " << factor << endl;
    cout << "  RadarConst: " << radarConst << endl;
  }

  return radarConst;

}

//////////////////////////////////////////////////
// set siggen power and frequency

void TsCalAuto::_setSiggenPower(double powerDbm)

{
  char input[1024];

  double delta = powerDbm - _params.siggen_max_power;

  if (_params.use_manual_siggen_control) {
    cerr << "Set siggen power  to " << powerDbm  << " (dBm), delta: " << delta << " (dB)" << endl;
    fprintf(stdout, "Manual control - hit return when ready ...");
    fgets(input,1023,stdin);
    // const char *notused = fgets(input,1023,stdin);
    // notused = NULL;
   } else {
    char buf[1024];
    if (_params.debug) {
      cerr << "======================================================" << endl;
      cerr << "Setting siggen power (dBm): " << powerDbm << endl;
    }
    sprintf(buf,"POW %gDBM\n",powerDbm);  // Build SCPI command
    _sendSiggenCmd(buf,input,1024);
  }

}
////////////////////////////////////////////////////
void TsCalAuto::_setSiggenFreq(double freqGhz)

{

  if (freqGhz == _prevFreq) return;  // Do nothing on no change
  
  char input[1024];

  if (_params.use_manual_siggen_control) {
    fprintf(stdout, "Manual control - Set frequency to: %g GHz\n", freqGhz);
    fprintf(stdout, "..............   hit return when ready ...");
    fgets(input,1023,stdin);
    // const char *notused = fgets(input,1023,stdin);
    // notused = NULL;
  } else {
    char buf[1024];
    if (_params.debug) {
      cerr << "Setting Sig Gen frequency (gHz): " << freqGhz << endl;
    }
    sprintf(buf,"FREQ %gGhz\n",freqGhz);  // Build SCPI command
    _sendSiggenCmd(buf,input,1024); 
  }

  _prevFreq = freqGhz;
  
}

////////////////////////////////////////////////////
void TsCalAuto::_setSiggenRF(bool on)

{

  if (_params.use_manual_siggen_control) {
    if (on) {
      cerr << "Set siggen Output On" << endl;
    } else {
      cerr << "Set siggen Output OFF" << endl;
    }
    return;
  }

  char input[1024];
  if (on) {
    if (_params.debug ) cerr << "Setting siggen Output On" << endl;
    _sendSiggenCmd("OUTP ON\n", input, 1024);
  } else {
    if (_params.debug ) cerr << "Setting siggen Output OFF" << endl;
    _sendSiggenCmd("OUTP OFF\n", input, 1024);
  }
}

////////////////////////////////////////////////////
void TsCalAuto::_sendSiggenCmd(const char* cmd,char* recv_buf, int len)

{
  Socket S;

  if(S.open(_params.siggen_ip_address,_params.siggen_tcp_port)) {
   cerr << "Problem Sending Command to " << _params.siggen_ip_address << endl;
   return;
  }
  // Write the comand and read a possible reply.
  S.writeBuffer((void *) cmd,strlen(cmd),100);
  S.readBuffer(recv_buf,len,100);
  S.close();
  if (_params.debug ) cerr << "Sent: " << cmd  << endl;
  umsleep(_params.siggen_cmd_delay);
 }

////////////////////////////////////////////////////
// sample received powers
//
// Returs 0 on success, -1 if no more data

int TsCalAuto::_sampleReceivedPowers(double powerDbm)

{                              
             
  int warningCount = 0;
  int pulseCount = 0;
  Stats stats;

  // open pulse reader to sample powers
  // opening every time forces us to get updated powers,
  // i.e. there is no chance of reading powers which were saved
  // to the FMQ before the siggen stabilized

  _openPulseReader();

  while (pulseCount < _params.n_samples) {
      
    // read next pulse
    
    IwrfTsPulse *pulse = _getNextPulse();
    if (pulse == NULL) {
      cerr << "WARNING - end of pulse data" << endl;
      _closePulseReader();
      return -1;
    }

    if (_params.specify_pulse_width) {
      warningCount++;
      // check for valid pulse width
      if (fabs(_params.fixed_pulse_width_us - pulse->getPulseWidthUs()) > 2.0e-3) {
        if (warningCount == 100000) {
          cerr << "WARNING - cannot find pulse with width: "
               << _params.fixed_pulse_width_us << endl;
          warningCount = 0;
        }
        // Go back to get next pulse
        continue;
      }
    }
    
    const fl32 *iqChan0 = pulse->getIq0();
    const fl32 *iqChan1 = pulse->getIq1();
    bool isHoriz = pulse->isHoriz();
    
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

      if (_params.fast_alternating_mode) {
        stats.addToAlternating(ii0, qq0, _haveChan1, ii1, qq1, isHoriz);
      } else {
        stats.addToSim(ii0, qq0, _haveChan1, ii1, qq1);
      }
      
    }

    delete pulse;

    pulseCount++;

  } // while
    
  if (_params.fast_alternating_mode) {
    stats.computeAlternating(_haveChan1);
  } else {
    stats.computeSim();
  }

  _siggenDbm.push_back(powerDbm);
  _hcDbm.push_back(stats.meanDbmHc);
  _vcDbm.push_back(stats.meanDbmVc);
  _hxDbm.push_back(stats.meanDbmHx);
  _vxDbm.push_back(stats.meanDbmVx);
  _hcMinusVcDbm.push_back(stats.meanDbmHc - stats.meanDbmVc);
  _hxMinusVxDbm.push_back(stats.meanDbmHx - stats.meanDbmVx);

  double waveguideH = powerDbm - 
    (_params.powerMeasLossDbH + _params.couplerForwardLossDbH);
  double waveguideV = powerDbm - 
    (_params.powerMeasLossDbV + _params.couplerForwardLossDbV);

  _waveguideDbmH.push_back(waveguideH);
  _waveguideDbmV.push_back(waveguideV);

  if (_params.debug) {
    cerr << "powers hc, vc, hx, vx, hc-vc, hx-vx: "
	 << stats.meanDbmHc << ", "
	 << stats.meanDbmVc << ", "
	 << stats.meanDbmHx << ", "
	 << stats.meanDbmVx << ", "
	 << stats.meanDbmHc - stats.meanDbmVc << ", "
	 << stats.meanDbmHx - stats.meanDbmVx << endl;
  }

  // close reader

  _closePulseReader();

  return 0;

}

////////////////////////////
// get next pulse
//
// returns NULL on failure

IwrfTsPulse *TsCalAuto::_getNextPulse() 

{
  
  IwrfTsPulse *pulse = _pulseReader->getNextPulse(true);
  if (pulse == NULL) {
    return NULL;
  }
  if (pulse->getIq1() != NULL) {
    _haveChan1 = true;
  } else {
    _haveChan1 = false;
  }
  
  _conditionGateRange(*pulse);

  // reformat pulse as needed
    
  pulse->convertToFL32();

  return pulse;

}

////////////////////////////
// open FMQ pulse reader

void TsCalAuto::_openPulseReader() 

{

  if (_pulseReader) {
    return;
  }

  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrfDebug = IWRF_DEBUG_NORM;
  } 
  
  _pulseReader = new IwrfTsReaderFmq(_params.input_fmq_path,
                                     iwrfDebug);

  if (_params.check_radar_id) {
    _pulseReader->setRadarId(_params.radar_id);
  }
  
}

////////////////////////////
// close FMQ pulse reader

void TsCalAuto::_closePulseReader() 

{
  
  if (_pulseReader) {
    delete _pulseReader;
    _pulseReader = NULL;
  }

}

////////////////////////////////////////////
// condition the gate range, to keep the
// numbers within reasonable limits

void TsCalAuto::_conditionGateRange(const IwrfTsPulse &pulse)

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

/////////////////////////////////////////////////    
// suspend the test pulse while cal proceeds

void TsCalAuto::_suspendTestPulse()
  
{
  
  _testPulsePid = 0;
  FILE *pidFile = NULL;
  
  if((pidFile = fopen(_params.TestPulse_pid_file,"r")) != NULL ) {
    int numf = fscanf(pidFile, "%d\n", &_testPulsePid);
    if(numf == 1 && _testPulsePid != 0) {
      kill(_testPulsePid, SIGUSR1);  // Tell the utility to Suspend.
      if (_params.debug) {
        cerr << "Suspending Test Pulse Manager Utility Ops" << endl;
      }
      // Wait for the Test Pulse Manager Utility to sut off modulation
      umsleep(_params.siggen_cmd_delay * 10);
    }
  }

}

//////////////////////////////////////////////////
// resume test pulse

void TsCalAuto::_resumeTestPulse()

{
 
  if(_testPulsePid != 0) {
    kill(_testPulsePid, SIGUSR2);  // Tell the utility to Resume
    if (_params.debug) {
      cerr << "Resuming Test Pulse Manager Utility Ops " << endl;
    }
  }

  _testPulsePid = 0;

}



