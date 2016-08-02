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
// SpolCal2Xml.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2010
//
///////////////////////////////////////////////////////////////
//
// SpolCal2Xml reads an SPOL ATE calibration file, analyzes it,
// and produces an XML file in DsRadarCal format
//
////////////////////////////////////////////////////////////////

#include "SpolCal2Xml.hh"

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
#include <toolsa/TaStr.hh>
#include <rapformats/DsRadarCalib.hh>
#include <toolsa/Socket.hh>
#include <didss/DataFileNames.hh>

const double SpolCal2Xml::piCubed = pow(M_PI, 3.0);
const double SpolCal2Xml::lightSpeed = 299792458.0;
const double SpolCal2Xml::kSquared = 0.93;

using namespace std;

// Constructor

SpolCal2Xml::SpolCal2Xml(int argc, char **argv)
  
{

  isOK = true;
  _reader = NULL;

  // set programe name
  
  _progName = "SpolCal2Xml";

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

  // initialize file reader

  if (_params.mode == Params::REALTIME) {
    
    // realtime mode - scan the directory
    
    _reader = new DsInputPath(_progName,
                              _params.debug >= Params::DEBUG_VERBOSE,
                              _params.input_dir,
                              864000,
                              PMU_auto_register,
                              false,
                              true);
    _reader->setSubString(_params.required_sub_str);
    
  } else {
    
    // ARCHIVE mode

    if (_args.inputFileList.size() < 1) {
      cerr << "ERROR - " << _progName << endl;
      cerr << "  ARCHIVE mode" << endl;
      cerr << "  No files specified" << endl;
      cerr << "  You must specify a file list on the command line" << endl;
      isOK = false;
      return;
    }
    
    _reader = new DsInputPath(_progName,
                              _params.debug >= Params::DEBUG_VERBOSE,
                              _args.inputFileList);
    
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

SpolCal2Xml::~SpolCal2Xml()
  
{

  if (_reader) {
    delete _reader;
  }

  if (_params.debug) {
    cerr << "SpolCal2Xml done ..." << endl;
  }

}

//////////////////////////////////////////////////
// Run

int SpolCal2Xml::Run ()
{

  int iret = 0;
  
  // loop until end of data
    
  char *path;
  while ((path = _reader->next()) != NULL) {
    if (_processFile(path)) {
      cerr << "WARNING - SpolCal2Xml::Run" << endl;
      cerr << "  Errors in processing file: " << path << endl;
      iret = -1;
    }
  } // while ((filePath ...
  
  return iret;
  
}

//////////////////////////////////////////////////
// process a file
//
// Returns 0 on success, -1 on failure

int SpolCal2Xml::_processFile(const char* filePath)

{
  
  if (_params.debug) {
    cerr << "Processing file: " << filePath << endl;
  }

  if (_readCal(filePath)) {
    cerr << "ERROR - SpolCal2Xml::_processFile";
    cerr << "  Cannot read file: " << filePath << endl;
    return -1;
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

  // write the XML file

  if (_writeResults()) {
    cerr << "ERROR - SpolCal2Xml::_processFile";
    cerr << "  Cannot write XML file" << endl;
    return -1;
  }
    
  return 0;

}

//////////////////////////////////////////////////
// read a cal file
//
// Returns 0 on success, -1 on failure

int SpolCal2Xml::_readCal(const char* filePath)

{

  // get the cal time from the file path

  time_t calTime;
  bool dateOnly;
  if (DataFileNames::getDataTime(filePath, calTime, dateOnly)) {
    cerr << "ERROR - SpolCal2Xml::_readCal()" << endl;
    cerr << "  Cannot get time from path: " << filePath << endl;
    cerr << "  Using file mod time instead" << endl;
    struct stat fileStat;
    if (ta_stat(filePath, &fileStat)) {
      int errNum = errno;
      cerr << "ERROR - SpolCal2Xml::_readChill";
      cerr << "  Cannot stat file file: " << filePath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    _calTime = fileStat.st_mtime;
  } else {
    _calTime = calTime;
  }

  // open file
  
  TaFile _in;
  FILE *in;
  if ((in = _in.fopenUncompress(filePath, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SpolCal2Xml::_readFile";
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
    double siggen, hc, hx, vc, vx;
    if (sscanf(line, "%lg%lg%lg%lg%lg",
               &siggen, &hc, &hx, &vc, &vx) == 5) {
      _siggenDbm.push_back(siggen);
      _hcDbm.push_back(hc);
      _hxDbm.push_back(hx);
      _vcDbm.push_back(vc);
      _vxDbm.push_back(vx);
      double hcMinusVc = hc - vc;
      double hxMinusVx = hx - vx;
      _hcMinusVcDbm.push_back(hcMinusVc);
      _hxMinusVxDbm.push_back(hxMinusVx);
    }
  }
  _in.fclose();
  
  // compute waveguide power adjusting injected power
  // for circuit gain and coupling factors
  
  for (int ii = 0; ii < (int) _siggenDbm.size(); ii++) {
    double waveguideH = _siggenDbm[ii] - 
      (_params.powerMeasLossDbH + _params.couplerForwardLossDbH);
    _waveguideDbmH.push_back(waveguideH);
    double waveguideV = _siggenDbm[ii] - 
      (_params.powerMeasLossDbV + _params.couplerForwardLossDbV);
    _waveguideDbmV.push_back(waveguideV);
  }
  
  return 0;

}

//////////////////////////////////////////////////
// compute radar constant, in meter units

double SpolCal2Xml::_computeRadarConstant(double xmitPowerDbm,
                                          double antennaGainDb,
                                          double twoWayWaveguideLossDb,
                                          double twoWayRadomeLossDb)
                                     
                                     
{

  double antGainPower = pow(10.0, antennaGainDb / 10.0);
  double gainSquared = antGainPower * antGainPower;
  double _wavelengthM = _params.wavelengthCm / 100.0;
  double lambdaSquared = _wavelengthM * _wavelengthM;
  double pulseMeters = _params.pulseWidthUs * 1.0e-6 * lightSpeed;
  
  double hBeamWidthRad = _params.beamWidthDegH * DEG_TO_RAD;
  double vBeamWidthRad = _params.beamWidthDegV * DEG_TO_RAD;

  double peakPowerMilliW = pow(10.0, xmitPowerDbm / 10.0);

  double theoreticalG = (M_PI * M_PI) / (hBeamWidthRad * vBeamWidthRad);
  double theoreticalGdB = 10.0 * log10(theoreticalG);
  
  double receiverMismatchLossDb = _params.receiverMismatchLossDb;
  
  double denom = (peakPowerMilliW * piCubed * pulseMeters * gainSquared *
                  hBeamWidthRad * vBeamWidthRad * kSquared * 1.0e-24);

  double num = (1024.0 * log(2.0) * lambdaSquared);
  
  double factor = num / denom;
  
  double radarConst = (10.0 * log10(factor)
                       + twoWayWaveguideLossDb
                       + twoWayRadomeLossDb
                       + receiverMismatchLossDb);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==== Computing radar constant ====" << endl;
    cerr << "  wavelengthCm: " << _params.wavelengthCm << endl;
    cerr << "  horizBeamWidthDeg: " << _params.beamWidthDegH << endl;
    cerr << "  vertBeamWidthDeg: " << _params.beamWidthDegV << endl;
    cerr << "  antGainDb: " << antennaGainDb << endl;
    cerr << "  theoretical antGainDb: " << theoreticalGdB << endl;
    cerr << "  xmitPowerDbm: " << xmitPowerDbm << endl;
    cerr << "  pulseWidthUs: " << _params.pulseWidthUs << endl;
    cerr << "  waveguideLoss: " << twoWayWaveguideLossDb << endl;
    cerr << "  radomeLoss: " << twoWayRadomeLossDb << endl;
    cerr << "  receiverLoss: " << receiverMismatchLossDb << endl;
    cerr << "  antGainPower: " << antGainPower << endl;
    cerr << "  gainSquared: " << gainSquared << endl;
    cerr << "  lambdaSquared: " << lambdaSquared << endl;
    cerr << "  pulseMeters: " << pulseMeters << endl;
    cerr << "  hBeamWidthRad: " << hBeamWidthRad << endl;
    cerr << "  vBeamWidthRad: " << vBeamWidthRad << endl;
    cerr << "  peakPowerMilliW: " << peakPowerMilliW << endl;
    cerr << "  piCubed: " << piCubed << endl;
    cerr << "  kSquared: " << kSquared << endl;
    cerr << "  num: " << num << endl;
    cerr << "  denom: " << denom << endl;
    cerr << "  factor: " << factor << endl;
    cerr << "  RadarConst: " << radarConst << endl;
  }

  return radarConst;

}

//////////////////////////////////////////////////
// compute the cal for a data set

void SpolCal2Xml::_computeCal(const string &label,
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

  double meanNoise;
  double noiseDbm;
  if (countNoise > 0) {
    meanNoise = sumNoise / countNoise;
    noiseDbm = 10.0 * log10(meanNoise);
  } else {
    meanNoise = 1.0e-20;
    noiseDbm = -9999;
  }

  // compute noise-subtracted powers

  vector<double> powers;
  double minPower = 1.0e99;
  for (int ii = 0; ii < (int) inputDbm.size(); ii++) {
    double powerPlusNoise = pow(10.0, outputDbm[ii] / 10.0);
    double power = powerPlusNoise - meanNoise;
    if (power > 0) {
      if (power < minPower) {
        minPower = power;
      }
    } 
    powers.push_back(power);
  }

  outputDbmNs.clear();
  for (int ii = 0; ii < (int) inputDbm.size(); ii++) {
    double power = powers[ii];
    if (powers[ii] < 0) {
      power = minPower;
    }
    outputDbmNs.push_back(10.0 * log10(power));
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
    cerr << "Cal for: " << label << endl;
    cerr << "  noiseDbm: " << noiseDbm << endl;
    cerr << "  gain: " << gain << endl;
    cerr << "  gainDbm: " << gainDbm << endl;
    cerr << "  dbz0: " << dbz0 << endl;
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
  chanResult.dbz0 = dbz0;

}

//////////////////////////
// write out results data

int SpolCal2Xml::_writeResults()
  
{

  // fill out DsRadarCalib object

  DsRadarCalib calib;

  calib.setRadarName(_params.radarName);
  calib.setCalibTime(_calTime);

  calib.setWavelengthCm(_params.wavelengthCm);

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

  calib.setRadarConstH(_radarConstH);
  calib.setRadarConstV(_radarConstV);

  calib.setNoiseDbmHc(_resultHc.noiseDbm);
  calib.setNoiseDbmVc(_resultVc.noiseDbm);

  if (_params.switching_receivers) {
    calib.setNoiseDbmHx(_resultHx.noiseDbm);
    calib.setNoiseDbmVx(_resultVx.noiseDbm);
  } else {
    calib.setNoiseDbmHx(_resultHc.noiseDbm);
    calib.setNoiseDbmVx(_resultVc.noiseDbm);
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

  calib.setZdrCorrectionDb(_params.zdrCorrectionDb);
  calib.setLdrCorrectionDbH(_params.ldrCorrectionDbH);
  calib.setLdrCorrectionDbV(_params.ldrCorrectionDbV);
  calib.setSystemPhidpDeg(_params.systemPhidpDeg);

  string calibXml;
  calib.convert2Xml(calibXml);

  if (_params.debug) {
    cerr << "============== Calibration XML =================" << endl;
    cerr << calibXml;
    cerr << "================================================" << endl;
  }
  
  // create the directory for the output file
  
  if (ta_makedir_recurse(_params.output_dir)) {
    int errNum = errno;
    cerr << "ERROR - SpolCal2Xml::_writeResults";
    cerr << "  Cannot create output dir: " << _params.output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute XML file path
  
  const DateTime ctime(_calTime);
  char xmlPath[1024];
  sprintf(xmlPath, "%s/SpolCal2Xml.%.4d%.2d%.2d_%.2d%.2d%.2d.xml",
          _params.output_dir,
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
    cerr << "ERROR - SpolCal2Xml::_writeResultsFile";
    cerr << "  Cannot create file: " << xmlPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  fprintf(outXml, "%s", calibXml.c_str());
  fclose(outXml);

  // compute ASCII text file path
  
  char textPath[1024];
  sprintf(textPath, "%s/SpolCal2Xml.%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
          _params.output_dir,
          ctime.getYear(),
          ctime.getMonth(),
          ctime.getDay(),
          ctime.getHour(),
          ctime.getMin(),
          ctime.getSec());
  
  if (_params.debug) {
    cerr << "writing output calib TEXT file: " << textPath << endl;
  }

  // write to TEXT file
  
  FILE *outText;
  if ((outText = fopen(textPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SpolCal2Xml::_writeResultsFile";
    cerr << "  Cannot create file: " << textPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (_params.debug) {
    fprintf(stderr, " %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s\n",
	    "siggen", "hc", "vc", "hx", "vx", "hcNs", "vcNs", "hxNs", "vxNs",
	    "hcmhx", "vcmvx", "wgh", "wgv");
  }
  
  for (int ii = 0; ii < (int) _siggenDbm.size(); ii++) {

    double siggen = _siggenDbm[ii];
    double hc = _hcDbm[ii];
    double hx = _hxDbm[ii];
    double vc = _vcDbm[ii];
    double vx = _vxDbm[ii];
    double hcNs = _hcDbmNs[ii];
    double hxNs = _hxDbmNs[ii];
    double vcNs = _vcDbmNs[ii];
    double vxNs = _vxDbmNs[ii];
    double hcmhx = _hcMinusVcDbm[ii];
    double vcmvx = _hxMinusVxDbm[ii];
    double wgh = _waveguideDbmH[ii];
    double wgv = _waveguideDbmV[ii];

    char text[1024];
    sprintf(text,
            " %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f "
            "%10.3f %10.3f %10.3f %10.3f %10.3f %10.3f\n",
	    siggen, hc, vc, hx, vx, hcNs, vcNs, hxNs, vxNs, hcmhx, vcmvx, wgh, wgv);

    fprintf(outText, "%s", text);
    if (_params.debug) {
      fprintf(stderr, "%s", text);
    }
    
  }

  fclose(outText);
  if(strlen(_params.plot_script) > 2) {
    char cmd[1024];
    sprintf(cmd,"%s %s\n",_params.plot_script,textPath);
    system(cmd);
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
 
int SpolCal2Xml::_linearFit(const vector<double> &x,
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

