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
// TsMaxPower2Fmq.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2018
//
///////////////////////////////////////////////////////////////
//
// TsMaxPower2Fmq - max power monitoring for HCR.
// TsMaxPower2Fmq reads radar time series data from an FMQ,
// computes the max power at any location, and writes the result
// as XML text to an FMQ.
// The HCR control app reads this data and disables the transmitter
// if the received power is too high.
//
////////////////////////////////////////////////////////////////

#include "TsMaxPower2Fmq.hh"
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
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaXml.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/pmu.h>

using namespace std;

// Constructor

TsMaxPower2Fmq::TsMaxPower2Fmq(int argc, char **argv)
  
{

  isOK = true;
  _pulseCount = 0;
  _pulseReader = NULL;
  _haveChan1 = false;
  _prevPulseSeqNum = 0;

  _gateForMax0 = 0;
  _gateForMax1 = 0;

  _outputFmqOpen = false;

  // set programe name
  
  _progName = "TsMaxPower2Fmq";

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
    iwrfDebug = IWRF_DEBUG_VERBOSE;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrfDebug = IWRF_DEBUG_NORM;
  } 
    
  if (_params.input_mode == Params::TS_FMQ_INPUT) {
    _pulseReader = new IwrfTsReaderFmq(_params.input_fmq_path,
				       iwrfDebug, false);
  } else {
    _pulseReader = new IwrfTsReaderFile(_args.inputFileList, iwrfDebug);
  }
  if (_params.use_secondary_georeference) {
    _pulseReader->setGeorefUseSecondary(true);
  }

  // open output fmq

  _outputFmqPath = _params.output_fmq_path;
  if (_openOutputFmq() == 0) {
    _outputFmqOpen = true;
  } else {
    cerr << "ERROR - TsMaxPower2Fmq: Cannot initialize FMQ: "
         << _outputFmqPath << endl;
    isOK = false;
    return;
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

  if (_params.reg_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
  // initialize from params

  _nSamples = _params.n_samples;
  _startGateRequested = _params.start_gate;
  _nGates = 0;

}

// destructor

TsMaxPower2Fmq::~TsMaxPower2Fmq()

{

  if (_pulseReader) {
    delete _pulseReader;
  }

  if (_outputFmqOpen) {
    _outputFmq.closeMsgQueue();
  }

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int TsMaxPower2Fmq::Run ()
{

  char msg[1024];
  snprintf(msg, 1024, "Reading FMQ: %s", _params.input_fmq_path);
  
  while (true) {

    // reg with procmap
    
    PMU_auto_register(msg);
    
    // initialize
    
    _pulseCount = 0;
    _initMaxPowerStats();
    
    // accumulate max power stats
    
    for (int ipulse = 0; ipulse < _nSamples; ipulse++) {
      
      // read next pulse
      
      IwrfTsPulse *pulse = _getNextPulse();
      if (pulse == NULL) {
        if (_pulseReader->endOfFile()) {
          return 0;
        }
        return -1;
      }

      pulse->convertToFL32();
      _addToMaxPower(*pulse);
      _pulseCount++;
      
      delete pulse;
      
    } // ipulse

    // compute max power stats
    
    _computeMaxPowerStats();

    // compile XML
    
    string xmlStr;
    _compileXmlStr(xmlStr);
    
    // write xml to output fmq
    
    if (_writeToFmq(xmlStr)) {
      return -1;
    }

  } // while

  return 0;

}

////////////////////////////
// get next pulse
//
// returns NULL on failure

IwrfTsPulse *TsMaxPower2Fmq::_getNextPulse() 

{
  
  IwrfTsPulse *pulse = NULL;

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

  _startRangeM = pulse->get_start_range_m();
  _gateSpacingM = pulse->get_gate_spacing_m();

  // check for missing pulses

  si64 pulseSeqNum = pulse->getSeqNum();
  si64 nMissing = (pulseSeqNum - _prevPulseSeqNum) - 1;
  if (_prevPulseSeqNum != 0 && nMissing != 0) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - TsMaxPower2Fmq - n missing pulses: " << nMissing << endl;
      cerr << "  prev pulse seq num: " << _prevPulseSeqNum << endl;
      cerr << "  this pulse seq num: " << pulseSeqNum << endl;
      cerr << "  file: " << _pulseReader->getPathInUse() << endl;
    }
  }
  _prevPulseSeqNum = pulseSeqNum;

  return pulse;

}

////////////////////////////////////////////
// condition the gate range, to keep the
// numbers within reasonable limits

void TsMaxPower2Fmq::_conditionGateRange(const IwrfTsPulse &pulse)

{

  _startGate = _startGateRequested;
  if (_startGate < 0) {
    _startGate = 0;
  }

  _nGates = pulse.getNGates();

}
    
////////////////////////////////////////////
// set midpoint and other cardinal values

void TsMaxPower2Fmq::_saveCardinalValues(const IwrfTsPulse &pulse)

{

  if (_pulseCount == 0) {
    _startTime = pulse.getFTime();
  }

  if (_pulseCount == _nSamples / 2) {
    _midTime = pulse.getFTime();
    _midPrt = pulse.getPrt();
    _midEl = pulse.getEl();
    _midAz = pulse.getAz();
  }

  _endTime = pulse.getFTime();

}

////////////////////////////////////////////
// augment max power information

void TsMaxPower2Fmq::_addToMaxPower(const IwrfTsPulse &pulse)

{

  _saveCardinalValues(pulse);
  
  const fl32 *iqChan0 = pulse.getIq0();
  const fl32 *iqChan1 = pulse.getIq1();
  
  double maxPower0 = -9999;
  double maxPower1 = -9999;
  int gateForMax0 = 0;
  int gateForMax1 = 0;

  int index = _startGate * 2;
  for (int igate = _startGate; igate < _nGates; igate++, index += 2) {
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

//////////////////////////////////////////////////
// compile max power XML string

void TsMaxPower2Fmq::_compileXmlStr(string &xmlStr)
  
{
  
  xmlStr += TaXml::writeStartTag("TsMaxPower", 0);

  // set response
  
  time_t midSecs = (time_t) _midTime;
  int midMSecs = (int) ((_midTime - midSecs) * 1000.0 + 0.5);
  double prf = 1.0 / _midPrt;

  double dwellSecs = (_endTime - _startTime) * ((double) _nSamples / ((double) _nSamples - 1.0));

  xmlStr += TaXml::writeTime("time", 1, midSecs);
  xmlStr += TaXml::writeDouble("msecs", 1, midMSecs);
  xmlStr += TaXml::writeDouble("dwellSecs", 1, dwellSecs);
  xmlStr += TaXml::writeDouble("prf", 1, prf);
  xmlStr += TaXml::writeInt("nSamples", 1, _nSamples);
  xmlStr += TaXml::writeInt("startGate", 1, _startGate);
  xmlStr += TaXml::writeInt("nGates", 1, _nGates);
  xmlStr += TaXml::writeDouble("el", 1, _midEl);
  xmlStr += TaXml::writeDouble("az", 1, _midAz);
  xmlStr += TaXml::writeDouble("meanMaxDbm0", 1, 10.0 * log10(_meanMaxPower0));
  xmlStr += TaXml::writeDouble("meanMaxDbm1", 1, 10.0 * log10(_meanMaxPower1));
  xmlStr += TaXml::writeDouble("peakMaxDbm0", 1, 10.0 * log10(_peakMaxPower0));
  xmlStr += TaXml::writeDouble("peakMaxDbm1", 1, 10.0 * log10(_peakMaxPower1));
  xmlStr += TaXml::writeDouble("rangeToMax0", 1, _rangeToMaxPower0);
  xmlStr += TaXml::writeDouble("rangeToMax1", 1, _rangeToMaxPower1);
  
  xmlStr += TaXml::writeEndTag("TsMaxPower", 0);

  xmlStr += "\r\n";

}

/////////////////////////////////
// initialize max power data

void TsMaxPower2Fmq::_initMaxPowerStats()
  
{
  
  _midTime = -999.9;
  _midPrt = -999.9;
  _midEl = -999.9;
  _midAz = -999.9;

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

void TsMaxPower2Fmq::_computeMaxPowerStats()
  
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

double TsMaxPower2Fmq::_computeVel(const vector<RadarComplex_t> &iq)
  
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
// write xml to FMQ

int TsMaxPower2Fmq::_writeToFmq(const string &xmlStr)
  
{

  PMU_auto_register("writeToFmq");

  if (_outputFmq.writeMsg(0, 0, xmlStr.c_str(), xmlStr.size() + 1)) {
    cerr << "ERROR - TsMaxPower2Fmq::_writeToFmq" << endl;
    cerr << "  Cannot write FMQ: " << _outputFmqPath << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===================================" << endl;
    cerr << xmlStr;
    cerr << "===================================" << endl;
  }

  return 0;

}

///////////////////////////////////////
// open the output FMQ
// returns 0 on success, -1 on failure

int TsMaxPower2Fmq::_openOutputFmq()

{
  
  // initialize the output FMQ
  
  if (_outputFmq.initReadWrite
      (_outputFmqPath.c_str(), "TsMaxPower2Fmq",
       _params.debug >= Params::DEBUG_EXTRA, // set debug?
       Fmq::END, // start position
       false,    // compression
       _params.output_fmq_nslots,
       _params.output_fmq_size)) {
    cerr << "ERROR: TsMaxPower2Fmq::_openOutputFmq()" << endl;
    cerr << "  Cannot initialize FMQ: " << _outputFmqPath << endl;
    cerr << "  nSlots: " << _params.output_fmq_nslots << endl;
    cerr << "  nBytes: " << _params.output_fmq_size << endl;
    cerr << _outputFmq.getErrStr() << endl;
    return -1;
  }
  _outputFmq.setSingleWriter();
  if (_params.output_fmq_reg_with_datamapper) {
    _outputFmq.setRegisterWithDmap(true, _params.output_fmq_datamapper_reg_interval);
  }

  return 0;

}

