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
// TsOverlay.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2012
//
///////////////////////////////////////////////////////////////
//
// TsOverlay reads raw time-series data from two sets of files. It
// combines these time series by summing the I and Q data, to create
// an overlaid data set. Typically this is used for combining clutter
// and weather data together, for testing the clutter mitigation
// algorithms.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <toolsa/str.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/Path.hh>
#include <dsserver/DsLdataInfo.hh>
#include "TsOverlay.hh"

using namespace std;

// Constructor

TsOverlay::TsOverlay(int argc, char **argv)
  
{

  isOK = true;
  _volNum = 0;
  _sweepNum = 0;
  _prevAzPrint = 0;
  _alternatingMode = false;

  _primaryReader = NULL;
  _secondaryReader = NULL;
  
  _prevPrimaryWithinLimits = false;
  _prevSecondaryWithinLimits = false;
  
  _outputTime = -1;
  _outPrimary = NULL;
  _outSecondary = NULL;
  _outCombined = NULL;

  _primaryPulse = NULL;
  _secondaryPulse = NULL;
  _combinedPulse = NULL;

  // set programe name
 
  _progName = "TsOverlay";

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
  
  _startTime.set(_params.output_start_time.year,
                 _params.output_start_time.month,
                 _params.output_start_time.day,
                 _params.output_start_time.hour,
                 _params.output_start_time.min,
                 _params.output_start_time.sec);

  _outputAzimuthStart = _params.output_azimuth_start;
  _outputAzimuth = _params.output_azimuth_start;
  _deltaAz = 0.0;

  _pulseSeqNum = 0;
  _deltaTime = 0.0;

  return;
  
}

// destructor

TsOverlay::~TsOverlay()

{

  if (_primaryReader) {
    delete _primaryReader;
  }
  if (_secondaryReader) {
    delete _secondaryReader;
  }

  if (_primaryPulse) {
    delete _primaryPulse;
  }
  if (_secondaryPulse) {
    delete _secondaryPulse;
  }
  if (_combinedPulse) {
    delete _combinedPulse;
  }

}

//////////////////////////////////////////////////
// Run

int TsOverlay::Run()
{
  
  // open the time series readers

  if (_openReaders()) {
    cerr << "ERROR: TsOverlay::Run()" << endl;
    cerr << "  Cannot open readers" << endl;
    return -1;
  }

  // check for alternating mode

  if (_checkAlternating(_primaryReader)) {
    cerr << "ERROR - TsOverlay::Run" << endl;
    cerr << "  Cannot check for alternating from primary reader" << endl;
    return -1;
  }

  while (_volNum < _params.output_n_volumes) {

    // overlay the time series for the sector specified
    
    if (_overlayTimeSeries()) {
      cerr << "ERROR - TsOverlay::Run" << endl;
      return -1;
    }

    // write end of vol
    
    _addEndOfVol();
    
    // close output
    
    _closeOutputFiles();

    // increment

    _volNum++;
    _startTime += _params.output_delta_volume_time;
    _outputAzimuthStart = _params.output_azimuth_start;
    _outputAzimuth = _params.output_azimuth_start;
    _deltaAz = 0.0;
    
    _pulseSeqNum = 0;
    _deltaTime = 0.0;

  }

  return 0;

}

//////////////////////////////////////////////////
// Open the readers

int TsOverlay::_openReaders()
{

  // initialize the time series data reader objects
  
  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
  //   iwrfDebug = IWRF_DEBUG_VERBOSE;
  // } else if (_params.debug >= Params::DEBUG_NORM) {
    iwrfDebug = IWRF_DEBUG_NORM;
  }

  vector<string> primaryPaths;
  for (int ii = 0; ii < _params.primary_file_paths_n; ii++) {
    primaryPaths.push_back(_params._primary_file_paths[ii]);
  }
  if (primaryPaths.size() < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  No files specified for primary data set" << endl;
    cerr << "  You must set the primary_file_paths array to have at least 1 entry" << endl;
    return -1;
  }
  if (_params.debug) {
    for (size_t ii = 0; ii < primaryPaths.size(); ii++) {
      cerr << "Primary path list[" << ii << "]: " << primaryPaths[ii] << endl;
    }
  }

  _primaryReader = new IwrfTsReaderFile(primaryPaths, iwrfDebug);
  
  vector<string> secondaryPaths;
  for (int ii = 0; ii < _params.secondary_file_paths_n; ii++) {
    secondaryPaths.push_back(_params._secondary_file_paths[ii]);
  }
  if (secondaryPaths.size() < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  No files specified for secondary data set" << endl;
    cerr << "  You must set the secondary_file_paths array to have at least 1 entry" << endl;
    return -1;
  }
  if (_params.debug) {
    for (size_t ii = 0; ii < secondaryPaths.size(); ii++) {
      cerr << "Secondary path list[" << ii << "]: " << secondaryPaths[ii] << endl;
    }
  }
  _secondaryReader = new IwrfTsReaderFile(secondaryPaths, iwrfDebug);

  // read in calibration

  string errStr;
  if (_primaryCalib.readFromXmlFile(_params.primary_cal_file_path, errStr)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem reading primary calibration file" << endl;
    cerr << errStr << endl;
    return -1;
  }

  if (_secondaryCalib.readFromXmlFile(_params.secondary_cal_file_path, errStr)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem reading secondary calibration file" << endl;
    cerr << errStr << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Reading primary   cal file: " << _params.primary_cal_file_path << endl;
    cerr << "Reading secondary cal file: " << _params.secondary_cal_file_path << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// Check for alternating mode
// Read first 2 pulses

int TsOverlay::_checkAlternating(IwrfTsReader *reader)
{

  IwrfTsPulse *pulse0 = reader->getNextPulse();
  IwrfTsPulse *pulse1 = reader->getNextPulse();

  if (pulse0 == NULL || pulse1 == NULL) {
    return -1;
  }

  if (pulse0->get_hv_flag() != pulse1->get_hv_flag()) {
    _alternatingMode = true;
  } else {
    _alternatingMode = false;
  }

  delete pulse0;
  delete pulse1;

  return 0;

}

//////////////////////////////////////////////////
// Get next pulse from primary reader

IwrfTsPulse *TsOverlay::_getNextPulsePrimary()
{
  
  while (true) {
    
    IwrfTsPulse *pulse = _primaryReader->getNextPulse();
    if (pulse == NULL) {
      _primaryReader->reset();
      if (_params.debug) {
        cerr << "Restarting primary series" << endl;
      }
      _pulseSeqNum += 10;
      continue;
    }
    
    if (_params.debug) {
      if (_primaryReader->endOfFile()) {
        string prevPathInUse = _primaryReader->getPrevPathInUse();
        cerr << "====>> Reading primary data" << endl;
        cerr << "       done with file: " << prevPathInUse << endl;
      }
    }
    
    // check azimuth

    double startAz = _params.primary_start_azimuth;
    double endAz = _params.primary_end_azimuth;
    bool withinLimits = false;
    double az = pulse->getAz();
    if (startAz < endAz) {
      if (az >= startAz && az <= endAz) {
        withinLimits = true;
      }
    } else {
      if (az >= startAz || az <= endAz) {
        withinLimits = true;
      }
    }
    if (!withinLimits) {
      delete pulse;
      _prevPrimaryWithinLimits = false;
      continue;
    } else {
      if (!_prevPrimaryWithinLimits) {
        // starting new time series sector
        if (_params.debug) {
          cerr << "Starting new primary sector" << endl;
        }
        // _pulseSeqNum += 10;
      }
      _prevPrimaryWithinLimits = true;
    }
    
    return pulse;
    
  } // while
  
  if (_params.debug) {
    string pathInUse = _primaryReader->getPathInUse();
    if (_primaryReader->endOfFile()) {
      cerr << "End of primary data: " << pathInUse << endl;
    }
  }
  
  return NULL;

}

//////////////////////////////////////////////////
// Get next pulse from secondary reader

IwrfTsPulse *TsOverlay::_getNextPulseSecondary(int requiredHvFlag)
{

  int count = 0;
  bool movingSector = false;

  while (true) {
    
    IwrfTsPulse *pulse = _secondaryReader->getNextPulse();
    if (pulse == NULL) {
      _secondaryReader->reset();
      if (_params.debug) {
        cerr << "Restarting secondary series" << endl;
      }
      _pulseSeqNum += 10;
      continue;
    }
    
    if (_params.debug) {
      if (_secondaryReader->endOfFile()) {
        string prevPathInUse = _secondaryReader->getPrevPathInUse();
        cerr << "====>> Reading secondary data" << endl;
        cerr << "       done with file: " << prevPathInUse << endl;
      }
    }
    
    // check HV flag if required
    
    if (_alternatingMode && requiredHvFlag >= 0) {
      int hvFlag = pulse->get_hv_flag();
      if (requiredHvFlag != hvFlag) {
        delete pulse;
        // _pulseSeqNum += 10;
        if (_params.debug && !movingSector) {
          count++;
          cerr << "====>> Syncing secondary series in alternating mode, count: " << count << endl;
        }
        continue;
      }
    }

    // check azimuth
    
    double startAz = _params.secondary_start_azimuth;
    double endAz = _params.secondary_end_azimuth;
    bool withinLimits = false;
    double az = pulse->getAz();
    if (startAz < endAz) {
      if (az >= startAz && az <= endAz) {
        withinLimits = true;
      }
    } else {
      if (az >= startAz || az <= endAz) {
        withinLimits = true;
      }
    }
    if (!withinLimits) {
      delete pulse;
      _prevSecondaryWithinLimits = false;
      movingSector = true;
      continue;
    } else {
      if (!_prevSecondaryWithinLimits) {
        // starting new time series sector
        if (_params.debug) {
          cerr << "Starting new secondary sector" << endl;
        }
        // _pulseSeqNum += 10;
      }
      _prevSecondaryWithinLimits = true;
    }

    return pulse;
    
  } // while
  
  if (_params.debug) {
    string pathInUse = _secondaryReader->getPathInUse();
    if (_secondaryReader->endOfFile()) {
      cerr << "End of secondary data: " << pathInUse << endl;
    }
  }
  
  return NULL;

}

//////////////////////////////////////////////////
// overlay the time series

int TsOverlay::_overlayTimeSeries()
  
{
  
  _prevAzPrint = 0;
  
  while (true) {

    // get primary pulse
    
    if (_primaryPulse) {
      delete _primaryPulse;
    }
    
    _primaryPulse =  _getNextPulsePrimary();
    
    if (_primaryPulse == NULL) {
      if (_params.debug) {
        cerr << "End of data in primary queue" << endl;
      }
      return 0;
    }
    
    if (_firstPulseTime.utime() == 0) {
      _firstPulseTime.set(_primaryPulse->getTime(),
                          _primaryPulse->getNanoSecs() / 1.0e9);
    }

    // get secondary pulse
    
    if (_secondaryPulse) {
      delete _secondaryPulse;
    }
    
    _secondaryPulse = _getNextPulseSecondary(_primaryPulse->get_hv_flag());
    
    if (_secondaryPulse == NULL) {
      if (_params.debug) {
        cerr << "End of data in secondary queue" << endl;
      }
      return 0;
    }
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      RadxTime primaryTime(_primaryPulse->getTime(),
                           _primaryPulse->getNanoSecs() / 1.0e9);
      RadxTime secondaryTime(_secondaryPulse->getTime(),
                             _secondaryPulse->getNanoSecs() / 1.0e9);
      cerr << "======>> Primary time, az: "
           << primaryTime.asString(3) << ", "
           << _primaryPulse->getAz() << endl;
      cerr << "====>> Secondary time, az: "
           << secondaryTime.asString(3) << ", "
           << _secondaryPulse->getAz() << endl;
      cerr << "====>> HV flag prim, sec: "
           << _primaryPulse->get_hv_flag() << ", "
           << _secondaryPulse->get_hv_flag() << endl;
    }
    

    // debug print
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      _printPulseInfo();
    }

    // set the pulse metadata
    
    _setPulseMetaData();
    
    // open output files if needed
    
    if (_outPrimary == NULL) {
      if (_openOutputFiles(_primaryPulse)) {
        cerr << "ERROR - TsOverlay::_overlayTimeSeries" << endl;
        return -1;
      }
    }
    
    if (_overlayPulse()) {
      return -1;
    }

    if (_deltaAz > _params.output_azimuth_coverage) {
      // done
      return 0;
    }
    
  } // while
  
  return 0;

}

//////////////////////////////////////////////////
// overlay the primary and secondary pulse data

int TsOverlay::_overlayPulse()
  
{

  // compute gate geometry and number of gates
  
  _computeGateGeometry();

  // ensure data are floats

  _primaryPulse->convertToFL32();
  _secondaryPulse->convertToFL32();

  // get the IQ data for channel 0

  vector<RadarComplex_t> primaryIq0; // channel 0
  vector<RadarComplex_t> secondaryIq0; // channel 0
  primaryIq0.resize(_nGates);
  secondaryIq0.resize(_nGates);

  const fl32 *pIq0 = _primaryPulse->getIq0() + _primaryStartGate * 2;
  const fl32 *sIq0 = _secondaryPulse->getIq0() + _secondaryStartGate * 2;
  
  int index = 0;
  for (int ii = 0; ii < _nGates; ii++, index += 2) {
    primaryIq0[ii].re = pIq0[index];
    primaryIq0[ii].im = pIq0[index+1];
    secondaryIq0[ii].re = sIq0[index];
    secondaryIq0[ii].im = sIq0[index+1];
  }

  // get the IQ data for channel 1 if available

  int nChannels = 1;
  if (_primaryPulse->getIq0() != NULL && 
      _secondaryPulse->getIq0() != NULL) {
    nChannels = 2;
  }

  vector<RadarComplex_t> primaryIq1; // channel 1
  vector<RadarComplex_t> secondaryIq1; // channel 1
  
  if (nChannels == 2) {
    primaryIq1.resize(_nGates);
    secondaryIq1.resize(_nGates);
    const fl32 *pIq1 = _primaryPulse->getIq1() + _primaryStartGate * 2;
    const fl32 *sIq1 = _secondaryPulse->getIq1() + _secondaryStartGate * 2;
    index = 0;
    for (int ii = 0; ii < _nGates; ii++, index += 2) {
      primaryIq1[ii].re = pIq1[index];
      primaryIq1[ii].im = pIq1[index+1];
      secondaryIq1[ii].re = sIq1[index];
      secondaryIq1[ii].im = sIq1[index+1];
    }
  }

  // get HV flag

  int hvFlag = _primaryPulse->get_hv_flag();

  // compute the combined IQ - channel 0

  double primaryNoiseDbm0 = _primaryCalib.getNoiseDbmHc();
  double secondaryNoiseDbm0 = _secondaryCalib.getNoiseDbmHc();
  if (_alternatingMode && hvFlag == 0) {
    primaryNoiseDbm0 = _primaryCalib.getNoiseDbmVx();
    secondaryNoiseDbm0 = _secondaryCalib.getNoiseDbmVx();
  }
  vector<RadarComplex_t> comboIq0; // channel 0
  comboIq0.resize(_nGates);
  _combineIq(primaryIq0, secondaryIq0, primaryNoiseDbm0, secondaryNoiseDbm0, comboIq0);

  // compute the combined IQ - channel 1

  double primaryNoiseDbm1 = _primaryCalib.getNoiseDbmVc();
  double secondaryNoiseDbm1 = _secondaryCalib.getNoiseDbmVc();
  if (_alternatingMode && hvFlag == 0) {
    primaryNoiseDbm1 = _primaryCalib.getNoiseDbmHx();
    secondaryNoiseDbm1 = _secondaryCalib.getNoiseDbmHx();
  }
  vector<RadarComplex_t> comboIq1; // channel 1
  if (nChannels == 2) {
    comboIq1.resize(_nGates);
    _combineIq(primaryIq1, secondaryIq1, primaryNoiseDbm1, secondaryNoiseDbm1, comboIq1);
  }

  // set the IQ data in the primary and secondary pulses

  TaArray<fl32> primaryIq_;
  fl32 *primaryIq = primaryIq_.alloc(_nGates * nChannels * 2);
  index = 0;
  for (int ii = 0; ii < _nGates; ii++, index += 2) {
    primaryIq[index] = primaryIq0[ii].re;
    primaryIq[index+1] = primaryIq0[ii].im;
  }
  if (nChannels == 2) {
    for (int ii = 0; ii < _nGates; ii++, index += 2) {
      primaryIq[index] = primaryIq1[ii].re;
      primaryIq[index+1] = primaryIq1[ii].im;
    }
  }
  _primaryPulse->setIqFloats(_nGates, nChannels, primaryIq);

  TaArray<fl32> secondaryIq_;
  fl32 *secondaryIq = secondaryIq_.alloc(_nGates * nChannels * 2);
  index = 0;
  for (int ii = 0; ii < _nGates; ii++, index += 2) {
    secondaryIq[index] = secondaryIq0[ii].re;
    secondaryIq[index+1] = secondaryIq0[ii].im;
  }
  if (nChannels == 2) {
    for (int ii = 0; ii < _nGates; ii++, index += 2) {
      secondaryIq[index] = secondaryIq1[ii].re;
      secondaryIq[index+1] = secondaryIq1[ii].im;
    }
  }
  _secondaryPulse->setIqFloats(_nGates, nChannels, secondaryIq);

  // set the IQ data in the combined pulse

  TaArray<fl32> comboIq_;
  fl32 *comboIq = comboIq_.alloc(_nGates * nChannels * 2);
  index = 0;
  for (int ii = 0; ii < _nGates; ii++, index += 2) {
    comboIq[index] = comboIq0[ii].re;
    comboIq[index+1] = comboIq0[ii].im;
  }
  if (nChannels == 2) {
    for (int ii = 0; ii < _nGates; ii++, index += 2) {
      comboIq[index] = comboIq1[ii].re;
      comboIq[index+1] = comboIq1[ii].im;
    }
  }

  // create an output pulse, based on primary pulse

  if (_combinedPulse) {
    delete _combinedPulse;
    _combinedPulse = NULL;
  }

  _combinedPulse = new IwrfTsPulse(*_primaryPulse);
  _combinedPulse->set_volume_num(_volNum);
  _combinedPulse->set_sweep_num(0);

  // set the IQ data on the combined pulse
  
  _combinedPulse->setIqFloats(_nGates, nChannels, comboIq);

  // add pulse to output files

  if (_writeToOutput()) {
    cerr << "ERROR - TsOverlay::_overlayPulse" << endl;
    cerr << "  Cannot write output to files" << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////
// set the metadata on primary and secondary pulses

void TsOverlay::_setPulseMetaData()

{

  // compute times
  
  _pulseTime.set(_primaryPulse->getTime(),
                 _primaryPulse->getNanoSecs() / 1.0e9);

  _pulseSeqNum++;
  _prt = _primaryPulse->getPrt();
  _deltaTime += _prt;
  _outputTime = _startTime + _deltaTime;

  // compute azimuth
  
  _deltaAz = _deltaTime * _params.output_scan_rate;
  _outputAzimuth = _outputAzimuthStart + _deltaAz;
  if (_outputAzimuth > 360.0) {
    _outputAzimuth -= 360.0;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "_deltaTime, _outputAz: " << _deltaTime << ", " << _outputAzimuth << endl;
  }
  
  // set pulse headers

  int nanoSecs = (int) (_outputTime.getSubSec() * 1.0e9 + 0.5);
  _primaryPulse->setTime(_outputTime.utime(), nanoSecs);
  _secondaryPulse->setTime(_outputTime.utime(), nanoSecs);

  _primaryPulse->set_azimuth(_outputAzimuth);
  _secondaryPulse->set_azimuth(_outputAzimuth);

  _primaryPulse->set_elevation(_params.output_elevation_angle);
  _secondaryPulse->set_elevation(_params.output_elevation_angle);

  _primaryPulse->set_fixed_el(_params.output_elevation_angle);
  _secondaryPulse->set_fixed_el(_params.output_elevation_angle);

  _primaryPulse->set_pulse_seq_num(_pulseSeqNum);
  _secondaryPulse->set_pulse_seq_num(_pulseSeqNum);

  _primaryPulse->set_volume_num(_volNum);
  _secondaryPulse->set_volume_num(_volNum);

  _primaryPulse->set_sweep_num(0);
  _secondaryPulse->set_sweep_num(0);

}
    
//////////////////////////////////////////////////
// combine IQ data for one channel
// Also attenuates primary and secondary data

void TsOverlay::_combineIq(vector<RadarComplex_t> &primaryIq,
                           vector<RadarComplex_t> &secondaryIq,
                           double primaryNoiseDbm,
                           double secondaryNoiseDbm,
                           vector<RadarComplex_t> &comboIq)

{

  double smallPower = 1.0e-20;
#ifdef NOTNOW
  double primaryNoise = pow(10.0, primaryNoiseDbm / 10.0);
  double secondaryNoise = pow(10.0, secondaryNoiseDbm / 10.0);
  double pAtten = pow(10.0, _params.primary_attenuation_db / 10.0);
  double sAtten = pow(10.0, _params.secondary_attenuation_db / 10.0);
#endif
  
  for (int ii = 0; ii < _nGates; ii++) {

    RadarComplex_t piq = primaryIq[ii];
    RadarComplex_t siq = secondaryIq[ii];

    if (piq.re == 0.0 && piq.im == 0.0) {
      piq.re = smallPower;
    }
    if (siq.re == 0.0 && siq.im == 0.0) {
      siq.re = smallPower;
    }

#ifdef NOTNOW

    // compute primary power and noise removal

    double pMeasPower = RadarComplex::power(piq);
    double pSignalPower = pMeasPower - primaryNoise;
    double pAttenPower = pSignalPower * pAtten;
    double pRatioIq = sqrt(pAttenPower / pMeasPower);
    double pNoiseRemoved = 0.0;

    // compute modified primary IQ
    
    if (pAtten != 1.0 && pSignalPower > 0) {
      piq.re *= pRatioIq;
      piq.im *= pRatioIq;
      pNoiseRemoved = primaryNoise;
    }
    
    // compute secondary power and noise removal
    
    double sMeasPower = RadarComplex::power(siq);
    double sSignalPower = sMeasPower - secondaryNoise;
    double sAttenPower = sSignalPower * sAtten;
    double sRatioIq = sqrt(sAttenPower / sMeasPower);
    double sNoiseRemoved = 0.0;

    // compute modified secondary IQ
    
    if (sAtten != 1.0 && sSignalPower > 0) {
      siq.re *= sRatioIq;
      siq.im *= sRatioIq;
      sNoiseRemoved = secondaryNoise;
    }

#endif
    
    // sum the parts

    RadarComplex_t ciq;
    ciq.re = piq.re + siq.re;
    ciq.im = piq.im + siq.im;

#ifdef NOTNOW
    // compute the combined power and add the noise back in
    
    double cPower_ns = RadarComplex::power(ciq);
    if (cPower_ns == 0.0) {
      cPower_ns = smallPower;
    }
    double cPower = cPower_ns + (pNoiseRemoved + sNoiseRemoved) / 2.0;
    double cNoiseRatio = sqrt(cPower_ns / cPower);
    ciq.re /= cNoiseRatio;
    ciq.im /= cNoiseRatio;
#endif

    comboIq[ii] = ciq;

#ifdef NOTNOW
    // add noise back in and 
    // save attenuated data in primary and secondaryu pulses

    if (pAtten != 1.0) {
      double pModPower = RadarComplex::power(piq);
      double pFinalPower = pModPower + pNoiseRemoved;
      double pFinalRatio = sqrt(pFinalPower / pModPower);
      piq.re /= pFinalRatio;
      piq.im /= pFinalRatio;
    }
#endif
    primaryIq[ii] = piq;

#ifdef NOTNOW
    if (sAtten != 1.0) {
      double sModPower = RadarComplex::power(siq);
      double sFinalPower = sModPower + sNoiseRemoved;
      double sFinalRatio = sqrt(sFinalPower / sModPower);
      siq.re /= sFinalRatio;
      siq.im /= sFinalRatio;
    }
#endif
    secondaryIq[ii] = siq;

  } // ii

}

//////////////////////////////////////////////////
// compute number of gates for each stream, etc

void TsOverlay::_computeGateGeometry()
  
{


  double primaryStartRange = _params.primary_start_range;
  double primaryStartRangeData = _primaryPulse->get_start_range_km();
  if (primaryStartRange < primaryStartRangeData) {
    primaryStartRange = primaryStartRangeData;
  }

  double primaryEndRange = _params.primary_end_range;
  double primaryEndRangeData = _primaryPulse->get_start_range_km() + 
    (_primaryPulse->getNGates() - 1) * _primaryPulse->get_gate_spacing_km();
  if (primaryEndRange > primaryEndRangeData) {
    primaryEndRange = primaryEndRangeData;
  }

  int primaryStartGate = (int) 
    ((primaryStartRange - primaryStartRangeData) / 
     _primaryPulse->get_gate_spacing_km() + 0.5);
  if (primaryStartGate < 0) {
    primaryStartGate = 0;
  }

  int primaryEndGate = (int) 
    ((primaryEndRange - primaryStartRangeData) / 
     _primaryPulse->get_gate_spacing_km() + 0.5);
  if (primaryEndGate >= _primaryPulse->getNGates()) {
    primaryEndGate = _primaryPulse->getNGates() - 1;
  }

  int primaryNGates = (primaryEndGate - primaryStartGate) + 1;

  // compute start and end gates for secondary data

  double secondaryStartRange = _params.secondary_start_range;
  double secondaryStartRangeData = _secondaryPulse->get_start_range_km();
  if (secondaryStartRange < secondaryStartRangeData) {
    secondaryStartRange = secondaryStartRangeData;
  }

  double secondaryEndRange = _params.secondary_end_range;
  double secondaryEndRangeData = _secondaryPulse->get_start_range_km() + 
    (_secondaryPulse->getNGates() - 1) * _secondaryPulse->get_gate_spacing_km();
  if (secondaryEndRange > secondaryEndRangeData) {
    secondaryEndRange = secondaryEndRangeData;
  }

  int secondaryStartGate = (int) 
    ((secondaryStartRange - secondaryStartRangeData) / 
     _secondaryPulse->get_gate_spacing_km() + 0.5);
  if (secondaryStartGate < 0) {
    secondaryStartGate = 0;
  }

  int secondaryEndGate = (int) 
    ((secondaryEndRange - secondaryStartRangeData) / 
     _secondaryPulse->get_gate_spacing_km() + 0.5);
  if (secondaryEndGate >= _secondaryPulse->getNGates()) {
    secondaryEndGate = _secondaryPulse->getNGates() - 1;
  }

  int secondaryNGates = (secondaryEndGate - secondaryStartGate) + 1;

  // ensure we have same number of gates

  if (primaryNGates > secondaryNGates) {
    primaryNGates = secondaryNGates;
    primaryEndGate = primaryStartGate + primaryNGates - 1;
  } else if (primaryNGates < secondaryNGates) {
    secondaryNGates = primaryNGates;
    secondaryEndGate = secondaryStartGate + secondaryNGates - 1;
  }

  _nGates = primaryNGates;
  _startRangeKm = _primaryPulse->get_start_range_km();
  _gateSpacingKm = _primaryPulse->get_gate_spacing_km();
  _primaryStartGate = primaryStartGate;
  _secondaryStartGate = secondaryStartGate;

}

//////////////////////////////////////////////////
// print pulse info for debugging

void TsOverlay::_printPulseInfo()
  
{

  // check hv flag for consistency

  int primaryHv = _primaryPulse->get_hv_flag();
  int secondaryHv = _secondaryPulse->get_hv_flag();
  
  // debug print

  double azPrimary = _primaryPulse->getAz();
  double azSecondary = _secondaryPulse->getAz();
  double elPrimary = _primaryPulse->getEl();
  double elSecondary = _secondaryPulse->getEl();

  double deltaAz = fabs(azPrimary - _prevAzPrint);
  if (deltaAz > 180) deltaAz = fabs(deltaAz - 360.0);

  if (deltaAz < 0.5) {
    return;
  }

  cerr << "  overlay in progress, az/el primary, az/el secondary: "
       << azPrimary << "/" << elPrimary << ", "
       << azSecondary << "/" << elSecondary << endl;
  cerr << "    nGates, start gate primary, secondary: "
       << _nGates << ", "
       << _primaryStartGate << ", "
       << _secondaryStartGate << endl;
  if (primaryHv != secondaryHv) {
    cerr << "WARNING - hv flags differ:" << endl;
    cerr << "  Primary HV flag: " << primaryHv << endl;
    cerr << "  Secondary HV flag: " << secondaryHv << endl;
  }
  _prevAzPrint = azPrimary;

}

////////////////////////////////////////////
// write to output files
// returns 0 on success, -1 on failure

int TsOverlay::_writeToOutput()

{

  // write ops info to file, if info has changed since last write
  
  IwrfTsInfo &info = _primaryPulse->getTsInfo();
  if (info.writeMetaQueueToFile(_outPrimary, false)) {
    return -1;
  }
  if (info.writeMetaQueueToFile(_outSecondary, false)) {
    return -1;
  }
  if (info.writeMetaQueueToFile(_outCombined, true)) {
    return -1;
  }

  // write burst if applicable
  
  if (_primaryReader->isBurstForLatestPulse()) {
    if (_primaryReader->getBurst().writeToFile(_outPrimary)) {
      return -1;
    }
    if (_primaryReader->getBurst().writeToFile(_outSecondary)) {
      return -1;
    }
    if (_primaryReader->getBurst().writeToFile(_outCombined)) {
      return -1;
    }
  }

  // write pulse to files
  
  if (_primaryPulse->writeToFile(_outPrimary)) {
    return -1;
  }
  if (_secondaryPulse->writeToFile(_outSecondary)) {
    return -1;
  }
  if (_combinedPulse->writeToFile(_outCombined)) {
    return -1;
  }

  return 0;

}
    
///////////////////////////////////////
// write end-of-sweep to output FMQ
// returns 0 on success, -1 on failure

int TsOverlay::_writeEndOfSweep()

{

  iwrf_event_notice_t notice;
  iwrf_event_notice_init(notice);

  notice.end_of_sweep = true;
  notice.volume_num = _volNum;
  notice.sweep_num = _sweepNum;

  if (_writeToFiles(&notice, sizeof(notice))) {
    cerr << "TsOverlay::_writeEndOfSweep" << endl;
    cerr << " Writing end of sweep event" << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Writing end of sweep event" << endl;
    iwrf_event_notice_print(stderr, notice);
  }

  return 0;

}

///////////////////////////////////////
// write end-of-vol to output FMQ
// returns 0 on success, -1 on failure

int TsOverlay::_writeEndOfVol()

{

  iwrf_event_notice_t notice;
  iwrf_event_notice_init(notice);

  notice.end_of_volume = true;
  notice.volume_num = _volNum;
  notice.sweep_num = _sweepNum;

  if (_writeToFiles(&notice, sizeof(notice))) {
    cerr << "TsOverlay::_writeEndOfVol" << endl;
    cerr << " Writing end of volume event" << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Writing end of volume event" << endl;
    iwrf_event_notice_print(stderr, notice);
  }

  return 0;

}

///////////////////////////////////////
// add an end-of-vol to output message

int TsOverlay::_addEndOfVol()
  
{
  
  iwrf_event_notice_t notice;
  iwrf_event_notice_init(notice);

  notice.end_of_volume = true;
  notice.volume_num = _volNum;
  notice.sweep_num = _sweepNum;

  if (_writeToFiles(&notice, sizeof(notice))) {
    cerr << "TsOverlay::_addEndOfVol" << endl;
    cerr << " Writing end of volume event" << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Adding end of volume event" << endl;
    iwrf_event_notice_print(stderr, notice);
  }

  return 0;

}

///////////////////////////////////////
// write buffer to files

int TsOverlay::_writeToFiles(const void *buf, int nbytes)

{

  if (fwrite(buf, nbytes, 1, _outPrimary) != 1) {
    int errNum = errno;
    cerr << "ERROR - TsOverlay::_writeToFiles" << endl;
    cerr << "  Cannot write to file: " << _primaryOutputPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (fwrite(buf, nbytes, 1, _outSecondary) != 1) {
    int errNum = errno;
    cerr << "ERROR - TsOverlay::_writeToFiles" << endl;
    cerr << "  Cannot write to file: " << _secondaryOutputPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (fwrite(buf, nbytes, 1, _outCombined) != 1) {
    int errNum = errno;
    cerr << "ERROR - TsOverlay::_writeToFiles" << endl;
    cerr << "  Cannot write to file: " << _combinedOutputPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  return 0;

}
  
/////////////////////////////////
// open new files

int TsOverlay::_openOutputFiles(IwrfTsPulse *pulse)

{
  
  // close out old files
  
  _closeOutputFiles();

  // open new ones
  
  if ((_outPrimary = _openOutputFile(pulse,
                                     _params.primary_output_dir,
                                     _primaryOutputPath)) == NULL) {
    cerr << "ERROR - TsOverlay::_openFiles" << endl;
    cerr << "Cannot open new primary file" << endl;
    return -1;
  }
  
  if ((_outSecondary = _openOutputFile(pulse,
                                       _params.secondary_output_dir,
                                       _secondaryOutputPath)) == NULL) {
    cerr << "ERROR - TsOverlay::_openFiles" << endl;
    cerr << "Cannot open new secondary file" << endl;
    return -1;
  }
  
  if ((_outCombined = _openOutputFile(pulse,
                                      _params.combined_output_dir,
                                      _combinedOutputPath)) == NULL) {
    cerr << "ERROR - TsOverlay::_openFiles" << endl;
    cerr << "Cannot open new combined file" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////
// open new file
//
// Returns FILE on success, NULL on failure

FILE *TsOverlay::_openOutputFile(IwrfTsPulse *pulse,
                                 const string &output_dir,
                                 string &outputPath)
  
{
  
  // get time

  time_t ptime = pulse->getTime();
  int nanoSecs = pulse->getNanoSecs();
  int milliSecs = nanoSecs / 1000000;
  if (milliSecs > 999) {
    milliSecs = 999;
  }

  date_time_t ttime;
  ttime.unix_time = ptime;
  _outputTime = ptime;
  uconvert_from_utime(&ttime);

  // compute antenna pos strings
  
  iwrf_scan_mode_t scanMode = pulse->getScanMode();

  char fixedAngleStr[64];
  char movingAngleStr[64];
  
  if (scanMode == IWRF_SCAN_MODE_RHI ||
      scanMode == IWRF_SCAN_MODE_MANRHI) {
    double el = pulse->getEl();
    if (el < 0) {
      el += 360;
    }
    double az = pulse->getAz();
    if (_params.use_fixed_angle_for_file_name) {
      az = pulse->getFixedAz();
      if (az < -9990) {
        az = 999; // missing
      }
    }
    sprintf(fixedAngleStr, "_%.3d", (int) (az * 10.0 + 0.5));
    sprintf(movingAngleStr, "_%.3d", (int) (el + 0.5));
  } else {
    double az = pulse->getAz();
    if (az < 0) {
      az += 360;
    }
    double el = pulse->getEl();
    if (_params.use_fixed_angle_for_file_name) {
      el = pulse->getFixedEl();
      if (el < -9990) {
        el = 99.9; // missing
      }
    }
    sprintf(fixedAngleStr, "_%.3d", (int) (el * 10.0 + 0.5));
    sprintf(movingAngleStr, "_%.3d", (int) (az + 0.5));
  }

  // compute scan mode string
  
  string scanModeStr;
  if (_params.add_scan_mode_to_file_name) {
    switch (scanMode) {
    case IWRF_SCAN_MODE_SECTOR:
      scanModeStr = ".sec";
      break;
    case IWRF_SCAN_MODE_COPLANE:
      scanModeStr = ".coplane";
      break;
    case IWRF_SCAN_MODE_RHI:
      scanModeStr = ".rhi";
      break;
    case IWRF_SCAN_MODE_VERTICAL_POINTING:
      scanModeStr = ".vert";
      break;
    case IWRF_SCAN_MODE_IDLE:
      scanModeStr = ".idle";
      break;
    case IWRF_SCAN_MODE_AZ_SUR_360:
      scanModeStr = ".sur";
      break;
    case IWRF_SCAN_MODE_EL_SUR_360:
      scanModeStr = ".elsur";
      break;
    case IWRF_SCAN_MODE_SUNSCAN:
      scanModeStr = ".sun";
      break;
    case IWRF_SCAN_MODE_POINTING:
      scanModeStr = ".point";
      break;
    case IWRF_SCAN_MODE_MANPPI:
      scanModeStr = ".manppi";
      break;
    case IWRF_SCAN_MODE_MANRHI:
      scanModeStr = ".manrhi";
      break;
    default: {}
    }
  }

  if (pulse->antennaTransition()) {
    scanModeStr += "_trans";
  }
    
  // make the output dir

  char subdir[1024];
  sprintf(subdir, "%s/%.4d%.2d%.2d", output_dir.c_str(),
          ttime.year, ttime.month, ttime.day);
  
  if (ta_makedir_recurse(subdir)) {
    int errNum = errno;
    cerr << "ERROR - TsSmartSave" << endl;
    cerr << "  Cannot make output directory: " << subdir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return NULL;
  }
  
  // compute output path

  string format = ".iwrf_ts";

  iwrf_iq_encoding_t encoding = pulse->getPackedEncoding();

  string packing;
  if (encoding == IWRF_IQ_ENCODING_FL32) {
    packing = ".fl32";
  } else if (encoding == IWRF_IQ_ENCODING_SCALED_SI16) {
    packing = ".scaled";
  } else if (encoding == IWRF_IQ_ENCODING_SCALED_SI32) {
    packing = ".scaled32";
  } else if (encoding == IWRF_IQ_ENCODING_DBM_PHASE_SI16) {
    packing = ".dbm_phase";
  } else if (encoding == IWRF_IQ_ENCODING_SIGMET_FL16) {
    packing = ".sigmet";
  }

  char name[1024];
  sprintf(name, "%.4d%.2d%.2d_%.2d%.2d%.2d.%.3d%s%s%s%s%s",
          ttime.year, ttime.month, ttime.day,
          ttime.hour, ttime.min, ttime.sec, milliSecs,
	  fixedAngleStr, movingAngleStr, scanModeStr.c_str(),
          packing.c_str(), format.c_str());
  
  char relPath[1024];
  sprintf(relPath, "%.4d%.2d%.2d/%s",
          ttime.year, ttime.month, ttime.day, name);

  char path[1024];
  sprintf(path, "%s/%s", output_dir.c_str(), relPath);
  
  // open file
  
  if (_params.debug) {
    cerr << "====>>> opening new output file: " << path << endl;
  }

  FILE *out;
  if ((out = fopen(path, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - TsOverlay" << endl;
    cerr << "  Cannot open output file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return NULL;
  }

  outputPath = path;

  // write metadata at start of file

  IwrfTsInfo &info = pulse->getTsInfo();
  info.writeMetaToFile(out, 0);

  return out;

}

///////////////////////////////////////////
// close all output files

void TsOverlay::_closeOutputFiles()

{

  _closeOutputFile(_outPrimary,
                   _params.primary_output_dir,
                   _primaryOutputPath);

  _closeOutputFile(_outSecondary,
                   _params.secondary_output_dir,
                   _secondaryOutputPath);
  
  _closeOutputFile(_outCombined,
                   _params.combined_output_dir,
                   _combinedOutputPath);
  
}
  
///////////////////////////////////////////
// close file

void TsOverlay::_closeOutputFile(FILE* &out,
                                 const string &outputDir,
                                 const string &outputPath)
  
{
  
  // close out old file
  
  if (out != NULL) {

    fclose(out);
    out = NULL;
    
    if (_params.debug) {
      cerr << "====>>> closed output file: " << outputPath << endl;
    }
    
    // write latest data info file
    
    string relPath;
    Path::stripDir(outputDir, outputPath, relPath);
    DsLdataInfo ldata(outputDir,
		      _params.debug >= Params::DEBUG_VERBOSE);
    ldata.setLatestTime(_outputTime.utime());
    ldata.setRelDataPath(relPath);
    ldata.setWriter("TsOverlay");
    ldata.setDataType("iwrf_ts");
    if (ldata.write(_outputTime.utime())) {
      cerr << "ERROR - cannot write LdataInfo" << endl;
      cerr << " outputDir: " << outputDir << endl;
    }
    
  }

}
  
