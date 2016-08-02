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
// RadarCal.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2006
//
///////////////////////////////////////////////////////////////
//
// RadarCal analyses time series data from sun scans
//
////////////////////////////////////////////////////////////////

#include "RadarCal.hh"

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

using namespace std;

// Constructor

RadarCal::RadarCal(int argc, char **argv)
  
{

  isOK = true;

  // set programe name
  
  _progName = "RadarCal";

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

  // check params

  if (_args.inputFileList.size() < 1) {
    cerr << "ERROR - you must specify files to be read using -f" << endl;
    isOK = false;
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

RadarCal::~RadarCal()
  
{

  if (_params.debug) {
    cerr << "RadarCal done ..." << endl;
  }

}

//////////////////////////////////////////////////
// Run

int RadarCal::Run ()
{
  
  int iret = 0;
  _results.clear();
  
  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {
    
    const char *filePath = _args.inputFileList[ii].c_str();
    
    if (_processFile(filePath)) {
      iret = -1;
    }
    
  } // ii


  if (_writeResults()) {
    iret = -1;
  }

  return iret;
  
}

//////////////////////////////////////////////////
// process a file
//
// Returns 0 on success, -1 on failure

int RadarCal::_processFile(const char* filePath)

{
  
  if (_params.debug) {
    cerr << "Processing file: " << filePath << endl;
  }

  if (_readFile(filePath)) {
    cerr << "ERROR - RadarCal::_processFile";
    cerr << "  Cannot read file: " << filePath << endl;
    return -1;
  }

  if (_params.debug) {
    fprintf(stderr, "RadarCal input\n");
    fprintf(stderr, "=============\n");
    fprintf(stderr, "Data time: %s\n",
            DateTime::strm((time_t) _calTime).c_str());
    fprintf(stderr, "Splitter power (dBm): %g\n", _splitterPower);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "\n");
      fprintf(stderr, "%10s %10s %10s %10s %10s %10s %10s\n",
              "Input", "InputH", "InputV", "Hc", "Hx", "Vc", "Vx");
      for (int ii = 0; ii < (int) _rawInjectedDbm.size(); ii++) {
        fprintf(stderr, "%10g %10g %10g %10g %10g %10g %10g\n",
                _rawInjectedDbm[ii], _waveguideDbmH[ii], _waveguideDbmV[ii],
                _hcDbm[ii], _hxDbm[ii], _vcDbm[ii], _vxDbm[ii]);
      }
    }
  }

  // compute cals

  Result result;
  result.time = _calTime;
  result.switchPos = _switchPos;
  result.el = _el;
  result.az = _az;
  result.splitterPowerDbm = _splitterPower;
  result.tempPowerSensor = _tempPowerSensor;
  result.tempLnaH = _tempLnaH;
  result.tempLnaV = _tempLnaV;
  result.tempIfd = _tempIfd;
  result.tempSiggen = _tempSiggen;
  result.tempInside = _tempInside;
  result.tempAmp = _tempAmp;
  _computeCal("Hc", _waveguideDbmH, _hcDbm, result.hc);
  _computeCal("Hx", _waveguideDbmH, _hxDbm, result.hx);
  _computeCal("Vc", _waveguideDbmV, _vcDbm, result.vc);
  _computeCal("Vx", _waveguideDbmV, _vxDbm, result.vx);
  _results.push_back(result);

  // write out diffs

  _appendToDiffsFile("Hc-Hx", _hcDbm, _hxDbm);
  _appendToDiffsFile("Hc-Vc", _hcDbm, _vcDbm);
  _appendToDiffsFile("Hc-Vx", _hcDbm, _vxDbm);
  _appendToDiffsFile("Vc-Vx", _vcDbm, _vxDbm);
  _appendToDiffsFile("Vc-Hx", _vcDbm, _hxDbm);
  _appendToDiffsFile("Hx-Vx", _hxDbm, _vxDbm);
  
  return 0;

}

//////////////////////////////////////////////////
// read a file
//
// Returns 0 on success, -1 on failure

int RadarCal::_readFile(const char* filePath)

{

  // initialize

  _calTime = 0;
  _switchPos = true;
  _splitterPower = -9999;
  _tempLnaH = -9999;
  _tempLnaV = -9999;
  _tempPowerSensor = -9999;
  _rawInjectedDbm.clear();
  _waveguideDbmH.clear();
  _waveguideDbmV.clear();
  _hcDbm.clear();
  _hxDbm.clear();
  _vcDbm.clear();
  _vxDbm.clear();

  // open file
  
  TaFile _in;
  FILE *in;
  if ((in = _in.fopenUncompress(filePath, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - RadarCal::_readFile";
    cerr << "  Cannot open file for reading: " << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  char line[BUFSIZ];
  
  // read date/time etc from first line in file
  
  if (fgets(line, BUFSIZ, in) == NULL) {
    int errNum = errno;
    cerr << "ERROR - RadarCal::_readFile";
    cerr << "  Cannot read data/time from file: " << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  int year, month, day, hour, min, sec;

  if (sscanf(line, "%4d %2d %2d %2d %2d %2d", 
             &year, &month, &day, &hour, &min, &sec) != 6) {
    cerr << "ERROR - RadarCal::_readFile";
    cerr << "  Cannot decode data/time etc from file: " << filePath << endl;
    cerr << "  line: " << line;
    return -1;
  }
  DateTime ctime(year, month, day, hour, min, sec);
  _calTime = ctime.utime();

  // read switchPos, splitter volts and temps
  
  if (fgets(line, BUFSIZ, in) == NULL) {
    int errNum = errno;
    cerr << "ERROR - RadarCal::_readFile";
    cerr << "  Cannot read second header line from file: " << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  int normal;
  double el, az;
  double spower;
  double temp_lna_h, temp_lna_v, temp_power_sensor;
  double temp_ifd, temp_siggen, temp_inside, temp_amp;
  if (sscanf(line, "%d %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg", 
             &normal, &el, &az, &spower,
             &temp_power_sensor, &temp_lna_h, &temp_lna_v,
             &temp_ifd, &temp_siggen, &temp_inside, &temp_amp) != 11) {
    cerr << "ERROR - RadarCal::_readFile";
    cerr << "  Cannot decode data/time etc from file: " << filePath << endl;
    cerr << "  line: " << line;
    return -1;
  }
  if (normal == 0) {
    _switchPos = false;
  } else {
    _switchPos = true;
  }
  _el = el;
  _az = az;
  _splitterPower = spower;
  _tempPowerSensor = temp_power_sensor;
  _tempLnaH = temp_lna_h;
  _tempLnaV = temp_lna_v;
  _tempIfd = temp_ifd;
  _tempSiggen = temp_siggen;
  _tempInside = temp_inside;
  _tempAmp = temp_amp;

  // read cal results

  while (fgets(line, BUFSIZ, in) != NULL) {
    double injected, hc, hx, vc, vx;
    if (sscanf(line, "%lg%lg%lg%lg%lg", &injected, &hc, &hx, &vc, &vx) == 5) {
      _rawInjectedDbm.push_back(injected);
      _hcDbm.push_back(hc);
      _hxDbm.push_back(hx);
      _vcDbm.push_back(vc);
      _vxDbm.push_back(vx);
    }
  }
  
  _in.fclose();

  // compute waveguide power adjusting injected power
  // for circuit gain and coupling factors

  for (int ii = 0; ii < (int) _rawInjectedDbm.size(); ii++) {
    double adjustedH =
      _rawInjectedDbm[ii] + _params.circuit_gain_h + _params.coupling_factor_h;
    _waveguideDbmH.push_back(adjustedH);
    double adjustedV =
      _rawInjectedDbm[ii] + _params.circuit_gain_v + _params.coupling_factor_v;
    _waveguideDbmV.push_back(adjustedV);
  }

  return 0;

}

//////////////////////////////////////////////////
// compute the cal for a data set

void RadarCal::_computeCal(const string &label,
                           const vector<double> &inputDbm,
                           const vector<double> &outputDbm,
                           ChannelResult &chanResult)

{
  
  // compute noise

  double sumNoise = 0.0;
  double countNoise = 0.0;
  double noiseDbm;

  for (int ii = 0; ii < (int) inputDbm.size(); ii++) {
    if (_rawInjectedDbm[ii] <= _params.noise_region_max_dbm) {
      sumNoise += pow(10.0, outputDbm[ii] / 10.0);
      countNoise++;
    }
  }

  if (countNoise > 0) {
    double meanNoise = sumNoise / countNoise;
    noiseDbm = 10.0 * log10(meanNoise);
  } else {
    noiseDbm = -9999;
  }

  // compute gain

  double sumGain = 0.0;
  double countGain = 0.0;
  double gainDbm;

  for (int ii = 0; ii < (int) inputDbm.size(); ii++) {
    if (_rawInjectedDbm[ii] >= _params.linear_region_min_dbm &&
        _rawInjectedDbm[ii] <= _params.linear_region_max_dbm) {
      double gain = outputDbm[ii] - inputDbm[ii];
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

  // compute linear fit to data

  vector<double> xx, yy;

  for (int ii = 0; ii < (int) inputDbm.size(); ii++) {
    if (_rawInjectedDbm[ii] >= _params.linear_region_min_dbm &&
        _rawInjectedDbm[ii] <= _params.linear_region_max_dbm) {
      xx.push_back(inputDbm[ii]);
      yy.push_back(outputDbm[ii]);
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
  

  if (_params.debug) {
    cerr << "Cal for: " << label << endl;
    cerr << "  noiseDbm: " << noiseDbm << endl;
    cerr << "  gain: " << gain << endl;
    cerr << "  gainDbm: " << gainDbm << endl;
    cerr << "  slope: " << slope << endl;
    cerr << "  corr: " << corr << endl;
    cerr << "  stdErrEst: " << stdErrEst << endl;
    cerr << "  rSquared: " << rSquared << endl;
  }

  chanResult.noiseDbm = noiseDbm;
  chanResult.gainDbm = gainDbm;
  chanResult.slope = slope;
  chanResult.corr = corr;
  chanResult.stdErrEst = stdErrEst;

}

//////////////////////////
// write out results data

int RadarCal::_writeResults()
  
{
  
  if (_results.size() < 1) {
    cerr << "ERROR - no results to print" << endl;
    return -1;
  }
  
  // create the directory for the output file
  
  if (ta_makedir_recurse(_params.output_dir)) {
    int errNum = errno;
    cerr << "ERROR - RadarCal::_writeResults";
    cerr << "  Cannot create output dir: " << _params.output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute file path
  
  const DateTime &ctime = _results[0].time;
  char path[1024];
  sprintf(path, "%s/RadarCal.out.%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
          _params.output_dir,
          ctime.getYear(),
          ctime.getMonth(),
          ctime.getDay(),
          ctime.getHour(),
          ctime.getMin(),
          ctime.getSec());
  
  cerr << "writing output file: " << path << endl;

  // open file
  
  FILE *out;
  if ((out = fopen(path, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - RadarCal::_writeResultsFile";
    cerr << "  Cannot create file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  fprintf(out, "RadarCal results\n");
  fprintf(out, "================\n");
  fprintf(out, "\n");
  fprintf(out, "Start time: %s\n",
          _results[0].time.getStrn().c_str());
  fprintf(out, "End   time: %s\n",
          _results[_results.size()-1].time.getStrn().c_str());
  fprintf(out, "Circuit gain    H (dB): %g\n", _params.circuit_gain_h);
  fprintf(out, "Circuit gain    V (dB): %g\n", _params.circuit_gain_v);
  fprintf(out, "Coupling factor H (dB): %g\n", _params.coupling_factor_h);
  fprintf(out, "Coupling factor V (dB): %g\n", _params.coupling_factor_v);
  fprintf(out, "Linear region min power (dBm): %g\n",
          _params.linear_region_min_dbm);
  fprintf(out, "Linear region max power (dBm): %g\n",
          _params.linear_region_max_dbm);
  fprintf(out, "Noise region max power (dBm): %g\n",
          _params.noise_region_max_dbm);
  fprintf(out, "\n");

  fprintf(out,
          "%10s %10s"
          " %4s %2s %2s %2s %2s %2s"
          " %10s %9s %9s %9s"
          " %9s %9s %9s"
          " %9s %9s %9s %9s"
          " %9s %9s %9s %9s"
          " %9s %9s %9s %9s"
          " %9s %9s %9s %9s"
          " %9s %9s %9s %9s"
          "\n",
          "unixtime", "unixdays",
          "yyyy", "mm", "dd", "hh", "mm", "ss",
          "switchPos", "el", "az", "splitPwr",
          "tempSens", "tempLnaH", "tempLnaV",
          "tempIfd", "tempSiggen", "tempInside", "tempAmp",
          "hcNoise", "hxNoise", "vcNoise", "vxNoise",
          "hcGain", "hxGain", "vcGain", "vxGain",
          "hcSlope", "hxSlope", "vcSlope", "vxSlope",
          "hcCorr", "hxCorr", "vcCorr", "vxCorr");
  
  for (int ii = 0; ii < (int) _results.size(); ii++) {
    const Result &rr = _results[ii];
    long utime = (long) rr.time.utime();
    double udays = (double) utime / 86400.0;
    fprintf(out,
            "%.10ld %10.4f"
            " %.4d %.2d %.2d %.2d %.2d %.2d"
            " %10d %9.4f %9.4f %9.4f"
            " %9.4f %9.4f %9.4f"
            " %9.4f %9.4f %9.4f %9.4f"
            " %9.4f %9.4f %9.4f %9.4f"
            " %9.4f %9.4f %9.4f %9.4f"
            " %9.4f %9.4f %9.4f %9.4f"
            " %9.4f %9.4f %9.4f %9.4f"
            "\n",
            utime,
            udays,
            rr.time.getYear(),
            rr.time.getMonth(),
            rr.time.getDay(),
            rr.time.getHour(),
            rr.time.getMin(),
            rr.time.getSec(),
            rr.switchPos,  rr.el, rr.az, rr.splitterPowerDbm,
            rr.tempPowerSensor, rr.tempLnaH, rr.tempLnaV,
            rr.tempIfd, rr.tempSiggen, rr.tempInside, rr.tempAmp,
            rr.hc.noiseDbm, rr.hx.noiseDbm, rr.vc.noiseDbm, rr.vx.noiseDbm,
            rr.hc.gainDbm, rr.hx.gainDbm, rr.vc.gainDbm, rr.vx.gainDbm,
            rr.hc.slope, rr.hx.slope, rr.vc.slope, rr.vx.slope,
            rr.hc.corr, rr.hx.corr, rr.vc.corr, rr.vx.corr);
  }
  
  fclose(out);

  return 0;

}

////////////////////////////////////////
// write out power differences to a file

int RadarCal::_appendToDiffsFile(const string &label,
                                 const vector<double> &first,
                                 const vector<double> &second)
  
{
  
  if (_results.size() < 1) {
    cerr << "ERROR - writeDiffsFile, label: " << label << endl;
    cerr << "  No results to print" << endl;
    return -1;
  }
  
  // create the directory for the output file
  
  if (ta_makedir_recurse(_params.output_dir)) {
    int errNum = errno;
    cerr << "ERROR - RadarCal::_writeDiffsFile";
    cerr << "  Cannot create output dir: " << _params.output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute file path
  
  const DateTime &ftime = _results[0].time;
  char path[1024];
  sprintf(path, "%s/%s.%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
          _params.output_dir,
          label.c_str(),
          ftime.getYear(),
          ftime.getMonth(),
          ftime.getDay(),
          ftime.getHour(),
          ftime.getMin(),
          ftime.getSec());

  if (_params.debug) {
    cerr << "writing diffs file: " << path << endl;
  }

  // open file
  
  FILE *out;
  if (_results.size() == 1) {
    if ((out = fopen(path, "w")) == NULL) {
      int errNum = errno;
      cerr << "ERROR - RadarCal::_writeDiffsFile";
      cerr << "  Cannot create file: " << path << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  } else {
    if ((out = fopen(path, "a")) == NULL) {
      int errNum = errno;
      cerr << "ERROR - RadarCal::_writeDiffsFile";
      cerr << "  Cannot open file for appending: " << path << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  DateTime ctime(_calTime);
  long utime = (long) ctime.utime();
  double udays = (double) utime / 86400.0;

  fprintf(out,
          "%.10ld %10.4f"
          " %.4d %.2d %.2d %.2d %.2d %.2d",
          utime,
          udays,
          ctime.getYear(),
          ctime.getMonth(),
          ctime.getDay(),
          ctime.getHour(),
          ctime.getMin(),
          ctime.getSec());

  fprintf(out,
          " %10d %9.4f %9.4f %9.4f"
          " %9.4f %9.4f %9.4f"
          " %9.4f %9.4f %9.4f %9.4f",
          _switchPos,  _el, _az, _splitterPower,
          _tempPowerSensor, _tempLnaH, _tempLnaV,
          _tempIfd, _tempSiggen, _tempInside, _tempAmp);

  for (int ii = 0; ii < (int) _rawInjectedDbm.size(); ii++) {
    if (_rawInjectedDbm[ii] < -19.9 && _rawInjectedDbm[ii] > -80.1) {
      double diff = first[ii] - second[ii];
      fprintf(out, " %9.4f", diff);
    }
  }

  fprintf(out, "\n");

  fclose(out);

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
 
int RadarCal::_linearFit(const vector<double> &x,
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

