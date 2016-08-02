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
// ChillCal2Xml.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2009
//
///////////////////////////////////////////////////////////////
//
// ChillCal2Xml reads a CHILL cal file, and reformats to
// an XML file in DsRadarCal format
//
////////////////////////////////////////////////////////////////

#include "ChillCal2Xml.hh"

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

const double ChillCal2Xml::piCubed = pow(M_PI, 3.0);
const double ChillCal2Xml::lightSpeed = 299792458.0;
const double ChillCal2Xml::kSquared = 0.93;

using namespace std;

// Constructor

ChillCal2Xml::ChillCal2Xml(int argc, char **argv)
  
{

  isOK = true;
  _reader = NULL;

  // set programe name
  
  _progName = "ChillCal2Xml";

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
                              _params.chill_cal_dir,
                              864000,
                              PMU_auto_register,
                              false,
                              true);
    _reader->setSubString(_params.current_cal_file_name);
    
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

ChillCal2Xml::~ChillCal2Xml()
  
{

  if (_reader) {
    delete _reader;
  }

  if (_params.debug) {
    cerr << "ChillCal2Xml done ..." << endl;
  }

}

//////////////////////////////////////////////////
// Run

int ChillCal2Xml::Run ()
{

  int iret = 0;
  
  // loop until end of data
    
  char *path;
  while ((path = _reader->next()) != NULL) {
    if (_processFile(path)) {
      cerr << "WARNING - ChillCal2Xml::Run" << endl;
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

int ChillCal2Xml::_processFile(const char* filePath)

{
  
  if (_params.debug) {
    cerr << "Processing file: " << filePath << endl;
  }

  if (_readChill(filePath)) {
    cerr << "ERROR - ChillCal2Xml::_processFile";
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

  // write the XML file

  if (_writeXml()) {
    cerr << "ERROR - ChillCal2Xml::_processFile";
    cerr << "  Cannot write XML file" << endl;
    return -1;
  }
    
  return 0;

}

//////////////////////////////////////////////////
// read a CHILL cal file
//
// Returns 0 on success, -1 on failure

int ChillCal2Xml::_readChill(const char* filePath)

{

  // stat the file to get the cal time

  struct stat fileStat;
  if (ta_stat(filePath, &fileStat)) {
    int errNum = errno;
    cerr << "ERROR - ChillCal2Xml::_readChill";
    cerr << "  Cannot stat file file: " << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  _calTime = fileStat.st_mtime;

  // open file
  
  TaFile _in;
  FILE *in;
  if ((in = _in.fopenUncompress(filePath, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - ChillCal2Xml::_readChill";
    cerr << "  Cannot open file for reading: " << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  char line[BUFSIZ];
  while (fgets(line, BUFSIZ, in) != NULL) {
    if (line[0] == '#') {
      continue;
    }
    vector<string> toks;
    TaStr::tokenize(line, " ", toks);
    if (toks.size() == 3) {
      if (toks[0] == "noise_v_rx_1") {
        _noise_v_rx_1 = atof(toks[2].c_str());
      }
      if (toks[0] == "noise_h_rx_2") {
        _noise_h_rx_2 = atof(toks[2].c_str());
      }
      if (toks[0] == "noise_v_rx_2") {
        _noise_v_rx_2 = atof(toks[2].c_str());
      }
      if (toks[0] == "noise_h_rx_1") {
        _noise_h_rx_1 = atof(toks[2].c_str());
      }
      if (toks[0] == "zdr_bias_db") {
        _zdr_bias_db = atof(toks[2].c_str());
      }
      if (toks[0] == "ldr_bias_h_db") {
        _ldr_bias_h_db = atof(toks[2].c_str());
      }
      if (toks[0] == "ldr_bias_v_db") {
        _ldr_bias_v_db = atof(toks[2].c_str());
      }
      if (toks[0] == "gain_v_rx_1_db") {
        _gain_v_rx_1_db = atof(toks[2].c_str());
      }
      if (toks[0] == "gain_v_rx_2_db") {
        _gain_v_rx_2_db = atof(toks[2].c_str());
      }
      if (toks[0] == "gain_h_rx_1_db") {
        _gain_h_rx_1_db = atof(toks[2].c_str());
      }
      if (toks[0] == "gain_h_rx_2_db") {
        _gain_h_rx_2_db = atof(toks[2].c_str());
      }
      if (toks[0] == "zdr_cal_base_vhs") {
        _zdr_cal_base_vhs = atof(toks[2].c_str());
      }
      if (toks[0] == "zdr_cal_base_vh") {
        _zdr_cal_base_vh = atof(toks[2].c_str());
      }
      if (toks[0] == "sun_pwr_v_rx_1_db") {
        _sun_pwr_v_rx_1_db = atof(toks[2].c_str());
      }
      if (toks[0] == "sun_pwr_h_rx_2_db") {
        _sun_pwr_h_rx_2_db = atof(toks[2].c_str());
      }
      if (toks[0] == "sun_pwr_v_rx_2_db") {
        _sun_pwr_v_rx_2_db = atof(toks[2].c_str());
      }
      if (toks[0] == "sun_pwr_h_rx_1_db") {
        _sun_pwr_h_rx_1_db = atof(toks[2].c_str());
      }
    }
    
  }

  _in.fclose();

  _noise_v_rx_1_dbm = 10.0 * log10(_noise_v_rx_1);
  _noise_v_rx_2_dbm = 10.0 * log10(_noise_v_rx_2);
  _noise_h_rx_1_dbm = 10.0 * log10(_noise_h_rx_1);
  _noise_h_rx_2_dbm = 10.0 * log10(_noise_h_rx_2);

  if (_params.debug) {
    cerr << "_noise_v_rx_1: " << _noise_v_rx_1 << endl;
    cerr << "_noise_h_rx_2: " << _noise_h_rx_2 << endl;
    cerr << "_noise_v_rx_2: " << _noise_v_rx_2 << endl;
    cerr << "_noise_h_rx_1: " << _noise_h_rx_1 << endl;
    cerr << "_noise_v_rx_1_dbm: " << _noise_v_rx_1_dbm << endl;
    cerr << "_noise_h_rx_2_dbm: " << _noise_h_rx_2_dbm << endl;
    cerr << "_noise_v_rx_2_dbm: " << _noise_v_rx_2_dbm << endl;
    cerr << "_noise_h_rx_1_dbm: " << _noise_h_rx_1_dbm << endl;
    cerr << "_zdr_bias_db: " << _zdr_bias_db << endl;
    cerr << "_ldr_bias_h_db: " << _ldr_bias_h_db << endl;
    cerr << "_ldr_bias_v_db: " << _ldr_bias_v_db << endl;
    cerr << "_gain_v_rx_1_db: " << _gain_v_rx_1_db << endl;
    cerr << "_gain_v_rx_2_db: " << _gain_v_rx_2_db << endl;
    cerr << "_gain_h_rx_1_db: " << _gain_h_rx_1_db << endl;
    cerr << "_gain_h_rx_2_db: " << _gain_h_rx_2_db << endl;
    cerr << "_zdr_cal_base_vhs: " << _zdr_cal_base_vhs << endl;
    cerr << "_zdr_cal_base_vh: " << _zdr_cal_base_vh << endl;
    cerr << "_sun_pwr_v_rx_1_db: " << _sun_pwr_v_rx_1_db << endl;
    cerr << "_sun_pwr_h_rx_2_db: " << _sun_pwr_h_rx_2_db << endl;
    cerr << "_sun_pwr_v_rx_2_db: " << _sun_pwr_v_rx_2_db << endl;
    cerr << "_sun_pwr_h_rx_1_db: " << _sun_pwr_h_rx_1_db << endl;
  }

  return 0;

}

//////////////////////////
// write out results data

int ChillCal2Xml::_writeXml()
  
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

  if (_params.switching_receiver) {

    calib.setNoiseDbmHc(_noise_h_rx_2_dbm);
    calib.setNoiseDbmVc(_noise_v_rx_2_dbm);
    
    calib.setNoiseDbmHx(_noise_h_rx_1_dbm);
    calib.setNoiseDbmVx(_noise_v_rx_1_dbm);
    
    calib.setReceiverGainDbHc(_gain_h_rx_2_db);
    calib.setReceiverGainDbVc(_gain_v_rx_2_db);
    
    calib.setReceiverGainDbHx(_gain_h_rx_1_db);
    calib.setReceiverGainDbVx(_gain_v_rx_1_db);
    
    calib.setReceiverSlopeDbHc(1.0);
    calib.setReceiverSlopeDbVc(1.0);

    calib.setReceiverSlopeDbHx(1.0);
    calib.setReceiverSlopeDbVx(1.0);

    calib.setBaseDbz1kmHc(_noise_h_rx_2_dbm - _gain_h_rx_2_db + _radarConstH);
    calib.setBaseDbz1kmHx(_noise_h_rx_1_dbm - _gain_h_rx_1_db + _radarConstH);
    calib.setBaseDbz1kmVc(_noise_v_rx_2_dbm - _gain_v_rx_2_db + _radarConstV);
    calib.setBaseDbz1kmVx(_noise_v_rx_1_dbm - _gain_v_rx_1_db + _radarConstV);
    
    calib.setSunPowerDbmHc(-9999);
    calib.setSunPowerDbmHx(-9999);
    calib.setSunPowerDbmVc(-9999);
    calib.setSunPowerDbmVx(-9999);

  } else {

    calib.setNoiseDbmHc(_noise_h_rx_2_dbm);
    calib.setNoiseDbmVc(_noise_v_rx_1_dbm);
    
    calib.setNoiseDbmHx(_noise_h_rx_2_dbm);
    calib.setNoiseDbmVx(_noise_v_rx_1_dbm);
    
    calib.setReceiverGainDbHc(_gain_h_rx_2_db);
    calib.setReceiverGainDbVc(_gain_v_rx_1_db);
    
    calib.setReceiverGainDbHx(_gain_h_rx_2_db);
    calib.setReceiverGainDbVx(_gain_v_rx_1_db);
    
    calib.setReceiverSlopeDbHc(1.0);
    calib.setReceiverSlopeDbVc(1.0);

    calib.setReceiverSlopeDbHx(1.0);
    calib.setReceiverSlopeDbVx(1.0);

    calib.setBaseDbz1kmHc(_noise_h_rx_2_dbm - _gain_h_rx_2_db + _radarConstH);
    calib.setBaseDbz1kmVc(_noise_v_rx_1_dbm - _gain_v_rx_1_db + _radarConstV);

    calib.setBaseDbz1kmHx(_noise_h_rx_2_dbm - _gain_h_rx_2_db + _radarConstH);
    calib.setBaseDbz1kmVx(_noise_v_rx_1_dbm - _gain_v_rx_1_db + _radarConstV);
    
    calib.setSunPowerDbmHc(-9999);
    calib.setSunPowerDbmHx(-9999);
    calib.setSunPowerDbmVc(-9999);
    calib.setSunPowerDbmVx(-9999);

  }

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
  
  if (ta_makedir_recurse(_params.xml_output_dir)) {
    int errNum = errno;
    cerr << "ERROR - ChillCal2Xml::_writeResults";
    cerr << "  Cannot create output dir: " << _params.xml_output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute XML file path
  
  const DateTime ctime(_calTime);
  char xmlPath[1024];
  sprintf(xmlPath, "%s/ChillCal2Xml.%.4d%.2d%.2d_%.2d%.2d%.2d.xml",
          _params.xml_output_dir,
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
    cerr << "ERROR - ChillCal2Xml::_writeResultsFile";
    cerr << "  Cannot create file: " << xmlPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  fprintf(outXml, "%s", calibXml.c_str());
  fclose(outXml);

  return 0;

}

//////////////////////////////////////////////////
// compute radar constant, in meter units

double ChillCal2Xml::_computeRadarConstant(double xmitPowerDbm,
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

