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
// SunCpCompute.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2007
//
///////////////////////////////////////////////////////////////
//
// SunCpCompute computes S1 and S2 from sun scans, for the
// ZDR Cross-polar (CP) power calibration technique
//
////////////////////////////////////////////////////////////////

#include "SunCpCompute.hh"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <algorithm>
#include <toolsa/DateTime.hh>
#include <toolsa/uusleep.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/mem.h>
#include <radar/RadarComplex.hh>

using namespace std;

// Constructor

SunCpCompute::SunCpCompute(int argc, char **argv)
  
{

  isOK = true;
  _totalPulseCount = 0;
  _pulseSeqNum = 0;
  _startTime = 0;
  _endTime = 0;
  _nSamples = 64;
  _reader = NULL;

  // set programe name
  
  _progName = "SunCpCompute";

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

  // initialize from params

  _nSamples = _params.n_samples;
  _sunPosn.setLocation(_params.radar_lat, _params.radar_lon,
                       _params.radar_altitude_km * 1000);

  if (_nSamples < 8) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  nSamples too low: " << _nSamples << endl;
    cerr << "  Must be at least 8" << endl;
    isOK = false;
    return;
  }

  // create reader

  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug) {
    iwrfDebug = IWRF_DEBUG_NORM;
  }
  if (_params.input_mode == Params::TS_FILE_INPUT) {
    _reader = new IwrfTsReaderFile(_args.inputFileList, iwrfDebug);
  } else {
    _reader = new IwrfTsReaderFmq(_params.input_fmq_name, iwrfDebug);
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

SunCpCompute::~SunCpCompute()

{

  if (_reader) {
    delete _reader;
  }

  // clean up memory

  _clearPulseQueue();
  _clearMomentsArray();
  _clearNoiseMomentsArray();

}

//////////////////////////////////////////////////
// Run

int SunCpCompute::Run ()
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _printMomentsLabels();
  }

  while (true) {
    
    const IwrfTsPulse *pulse = _reader->getNextPulse(true);
    if (pulse == NULL) {
      break;
    }
    
    if (_params.invert_hv_flag) {
      pulse->setInvertHvFlag();
    }

    _processPulse(pulse);
    
  }

  // compute cal time as mean of start and end time
  
  _calTime = (_startTime + _endTime) / 2.0;

  // compute the noise power for each channel

  if (_computeNoisePower()) {
    return -1;
  }

  // compute power ratios

  _computePowerRatios();

  // write out results
  
  if (_writeResults()) {
    return -1;
  }

  return 0;
  
}

/////////////////////
// process a pulse

void SunCpCompute::_processPulse(const IwrfTsPulse *pulse)

{

  // at start, print headers

  if (_totalPulseCount == 0) {
    _startTime = pulse->getFTime();
  }
  _endTime = pulse->getFTime();
  _calTime = (_startTime + _endTime) / 2.0;

  // check that we start with a horizontal pulse

  if (_pulseQueue.size() == 0 && !pulse->isHoriz()) {
    return;
  }
  
  // add the pulse to the queue

  _addPulseToQueue(pulse);
  _totalPulseCount++;
  
  // do we have a full pulse queue?

  if ((int) _pulseQueue.size() < _nSamples) {
    return;
  }

  // does the pulse az or el change drastically?
  
  int qSize = (int) _pulseQueue.size();
  double azStart = _pulseQueue[0]->getAz();
  double elStart = _pulseQueue[0]->getEl();
  double azEnd = _pulseQueue[qSize-1]->getAz();
  double elEnd = _pulseQueue[qSize-1]->getEl();
    
  double azDiff = RadarComplex::diffDeg(azStart, azEnd);
  double elDiff = RadarComplex::diffDeg(elStart, elEnd);
  if (fabs(azDiff) > 2 || fabs(elDiff) > 0.1) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "====>> Clearing pulse queue" << endl;
      cerr << "  azStart, azEnd: " << azStart << ", " << azEnd << endl;
      cerr << "  elStart, elEnd: " << elStart << ", " << elEnd << endl;
    }
    _clearPulseQueue();
    return;
  }

  // find pulses around mid point of queue

  int midIndex0 = _nSamples / 2;
  int midIndex1 = midIndex0 + 1;
  const IwrfTsPulse *pulse0 = _pulseQueue[midIndex0];
  const IwrfTsPulse *pulse1 = _pulseQueue[midIndex1];
  
  // compute angles at mid pulse
  
  double az0 = pulse0->getAz();
  double az1 = pulse1->getAz();

  // adjust angle if they cross north

  _checkForNorthCrossing(az0, az1);
  
  // order the azimuths

  if (az0 > az1) {
    double tmp = az0;
    az0 = az1; 
    az1 = tmp;
  }

  double az = RadarComplex::meanDeg(az0, az1);
  double el = RadarComplex::meanDeg(pulse0->getEl(), pulse1->getEl());
  double cosel = cos(el * DEG_TO_RAD);

  // compute angles relative to sun
  
  _midTime = (pulse0->getFTime() + pulse1->getFTime()) / 2.0;
  double sunEl, sunAz;
  _sunPosn.computePosn(_midTime, sunEl, sunAz);
  _offsetEl = RadarComplex::diffDeg(el, sunEl);
  _offsetAz = RadarComplex::diffDeg(az, sunAz) * cosel;
  
  // set other properties at mid pulse

  _midPrt = pulse0->getPrt();
  _midEl = el;
  _midAz = az;

  // compute the moments - alternating mode

  Moments *moments = new Moments();
  _computeMomentsAlt(moments);
  if (fabs(_offsetEl) <= _params.max_delta_el &&
      fabs(_offsetAz) <= _params.max_delta_az) {
    // close to sun
    _moments.push_back(moments);
    // away from sun, noise moments
  } else {
    _momentsNoise.push_back(moments);
  }
  
  // clear the pulse queue, ready for the next beam
  
  _clearPulseQueue();
  
}

/////////////////////////////////////////////////
// add the pulse to the pulse queue
    
void SunCpCompute::_addPulseToQueue(const IwrfTsPulse *pulse)
  
{

  // manage the size of the pulse queue, popping off the back

  if ((int) _pulseQueue.size() >= _nSamples) {
    const IwrfTsPulse *oldest = _pulseQueue.front();
    if (oldest->removeClient() == 0) {
      delete oldest;
    }
    _pulseQueue.pop_front();
  }

  int qSize = (int) _pulseQueue.size();
  if (qSize > 1) {

    // check for big azimuth or elevation change
    // if so clear the queue
    
    double az = pulse->getAz();
    double el = pulse->getEl();
    double prevAz = _pulseQueue[qSize-1]->getAz();
    double prevEl = _pulseQueue[qSize-1]->getEl();
    
    double azDiff = RadarComplex::diffDeg(az, prevAz);
    double elDiff = RadarComplex::diffDeg(el, prevEl);
    if (fabs(azDiff) > 0.1 || fabs(elDiff) > 0.1) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "====>> Clearing pulse queue" << endl;
        cerr << "  az, prevAz: " << az << ", " << prevAz << endl;
        cerr << "  el, prevEl: " << el << ", " << prevEl << endl;
      }
      _clearPulseQueue();
      return;
    }

  }
    
  // push pulse onto front of queue
  
  pulse->addClient();
  _pulseQueue.push_back(pulse);

  // print missing pulses in verbose mode
  
  if ((int) pulse->getSeqNum() != _pulseSeqNum + 1) {
    if (_params.debug >= Params::DEBUG_VERBOSE && _pulseSeqNum != 0) {
      cerr << "**** Missing seq num: " << _pulseSeqNum
	   << " to " <<  pulse->getSeqNum() << " ****" << endl;
    }
  }
  _pulseSeqNum = pulse->getSeqNum();

}

/////////////////////////////////////////////////
// clear the pulse queue
    
void SunCpCompute::_clearPulseQueue()
  
{

  for (size_t ii = 0; ii < _pulseQueue.size(); ii++) {
    if (_pulseQueue[ii]->removeClient() == 0) {
      delete _pulseQueue[ii];
    }
  }
  _pulseQueue.clear();

}

/////////////////////////////////
// print moments labels

void SunCpCompute::_printMomentsLabels()
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    
    fprintf(stderr,
            "#                  time "
            "%7s %7s %7s %9s %9s "
            "%8s %8s %8s %8s "
            "%13s %13s\n",
            "prf", "midEl", "midAz", "offsetEl", "offsetAz",
            "dbmHc", "dbmHx", "dbmVc", "dbmVx",
            "dbmVc-dbmHc", "dbmVx-dbmHx");

  }

}

////////////////////////////////////////////
// compute moments using pulses in queue,
// assuming fast alternating operation

void SunCpCompute::_computeMomentsAlt(Moments *moments)

{
  
  // set up data pointer array for each channel
  
  const fl32 **iqData0 = new const fl32*[_nSamples];
  const fl32 **iqData1 = new const fl32*[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    iqData0[ii] = _pulseQueue[ii]->getIq0();
    iqData1[ii] = _pulseQueue[ii]->getIq1();
  }
  
  // load up IQ data arrays for chan0 and chan1
  
  int nGates = _pulseQueue[0]->getNGates();
  RadarComplex_t **IQ0 = (RadarComplex_t **)
    ucalloc2(nGates, _nSamples, sizeof(RadarComplex));
  RadarComplex_t **IQ1 = (RadarComplex_t **)
    ucalloc2(nGates, _nSamples, sizeof(RadarComplex));
  
  for (int igate = 0, posn = 0; igate < nGates; igate++, posn += 2) {
    RadarComplex_t *iq0 = IQ0[igate];
    RadarComplex_t *iq1 = IQ1[igate];
    for (int isamp = 0; isamp < _nSamples; isamp++, iq0++, iq1++) {
      iq0->re = iqData0[isamp][posn];
      iq0->im = iqData0[isamp][posn + 1];
      iq1->re = iqData1[isamp][posn];
      iq1->im = iqData1[isamp][posn + 1];
    } // isamp
  } // igate

  // compute mean H and V power, averaging over all pulses
  // and all gates

  double sumPowerHc = 0.0;
  double sumPowerHx = 0.0;
  double sumPowerVc = 0.0;
  double sumPowerVx = 0.0;
  double nH = 0.0;
  double nV = 0.0;

  int startGate = _params.start_gate;
  int endGate = startGate + _params.n_gates;

  for (int igate = startGate; igate < endGate; igate++) {

    RadarComplex_t *iq0 = IQ0[igate];
    RadarComplex_t *iq1 = IQ1[igate];
    
    for (int isamp = 0; isamp < _nSamples; isamp++, iq0++, iq1++) {
      
      double power0 = iq0->re * iq0->re + iq0->im * iq0->im;
      double power1 = iq1->re * iq1->re + iq1->im * iq1->im;

      if ((isamp % 2) == 0) {
        // horiz flag, horiz is in chan 0
        sumPowerHc += power0;
        sumPowerVx += power1;
        nH++;
      } else {
        // vert flag, vert is in chan 0
        sumPowerVc += power0;
        sumPowerHx += power1;
        nV++;
      }

    }
    
  } // igate

  // free up

  delete[] iqData0;
  delete[] iqData1;
  ufree2((void **) IQ0);
  ufree2((void **) IQ1);

  // compute moments

  double powerHc = sumPowerHc / nH;
  double powerVc = sumPowerVc / nV;
  double powerVx = sumPowerVx / nH;
  double powerHx = sumPowerHx / nV;
  
  if (powerHc == 0) {
    powerHc = 1.0e-12;
  }
  if (powerHx == 0) {
    powerHx = 1.0e-12;
  }
  if (powerVc == 0) {
    powerVc = 1.0e-12;
  }
  if (powerVx == 0) {
    powerVx = 1.0e-12;
  }

  double dbmHc = 10.0  * log10(powerHc);
  double dbmVc = 10.0  * log10(powerVc);
  double dbmHx = 10.0  * log10(powerHx);
  double dbmVx = 10.0  * log10(powerVx);
  
  moments->time = _midTime;
  moments->prt =  _midPrt;
  moments->az = _midAz;
  moments->el = _midEl;
  moments->offsetAz = _offsetAz;
  moments->offsetEl = _offsetEl;
  
  moments->powerHc = powerHc;
  moments->powerVc = powerVc;
  moments->powerHx = powerHx;
  moments->powerVx = powerVx;
  
  moments->dbmHc = dbmHc;
  moments->dbmVc = dbmVc;
  moments->dbmHx = dbmHx;
  moments->dbmVx = dbmVx;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
  
    time_t midSecs = (time_t) _midTime;
    int midPartialMSecs = (int) ((_midTime - midSecs) * 1000.0 + 0.5);
    if (midPartialMSecs > 999) {
      midPartialMSecs = 999;
    }
    double prf = 1.0 / _midPrt;
    
    fprintf(stderr,
            "%s.%.3d "
            "%7.1f %7.2f %7.2f %9.2f %9.2f "
            "%8.3f %8.3f %8.3f %8.3f "
            "%13.3f %13.3f\n",
            DateTime::strm(midSecs).c_str(), midPartialMSecs,
            prf, _midEl, _midAz, _offsetEl, _offsetAz,
            dbmHc, dbmHx, dbmVc, dbmVx,
            dbmVc - dbmHc, dbmVx - dbmHx);

  }
          
}

/////////////////////////////////////////////////
// clear the moments array
    
void SunCpCompute::_clearMomentsArray()
  
{
  
  for (int ii = 0; ii < (int) _moments.size(); ii++) {
    delete _moments[ii];
  }
  _moments.clear();

}

/////////////////////////////////////////////////
// clear the noise moments array
    
void SunCpCompute::_clearNoiseMomentsArray()
  
{
  
  for (int ii = 0; ii < (int) _momentsNoise.size(); ii++) {
    delete _momentsNoise[ii];
  }
  _momentsNoise.clear();

}

/////////////////////////////////////////////////
// compute the min power for each channel
    
int SunCpCompute::_computeNoisePower()
  
{
  
  _noisePowerHc = Moments::missing;
  _noisePowerHx = Moments::missing;
  _noisePowerVc = Moments::missing;
  _noisePowerVx = Moments::missing;
  
  _noiseDbmHc = Moments::missing;
  _noiseDbmHx = Moments::missing;
  _noiseDbmVc = Moments::missing;
  _noiseDbmVx = Moments::missing;

  double sumPowerHc = 0.0;
  double sumPowerHx = 0.0;
  double sumPowerVc = 0.0;
  double sumPowerVx = 0.0;

  _nBeamsNoise = 0.0;
  
  for (int ii = 0; ii < (int) _momentsNoise.size(); ii++) {
    
    Moments *moments = _momentsNoise[ii];
    
    sumPowerHc += moments->powerHc;
    sumPowerHx += moments->powerHx;
    sumPowerVc += moments->powerVc;
    sumPowerVx += moments->powerVx;
    _nBeamsNoise++;
    
  } // ii

  if (_nBeamsNoise > 0) {

    _noisePowerHc = sumPowerHc / _nBeamsNoise;
    _noiseDbmHc = 10.0 * log10(_noisePowerHc);

    _noisePowerHx = sumPowerHx / _nBeamsNoise;
    _noiseDbmHx = 10.0 * log10(_noisePowerHx);

    _noisePowerVc = sumPowerVc / _nBeamsNoise;
    _noiseDbmVc = 10.0 * log10(_noisePowerVc);

    _noisePowerVx = sumPowerVx / _nBeamsNoise;
    _noiseDbmVx = 10.0 * log10(_noisePowerVx);

  } else {

    cerr << "ERROR - SunCpCompute" << endl;
    cerr << "  Cannot compute noise power" << endl;
    cerr << "  No data in the wings" << endl;
    return -1;

  }
    
  return 0;

}

/////////////////////////////////////////////////
// compute the power ratios
    
void SunCpCompute::_computePowerRatios()
  
{

  _nBeamsUsed = 0;

  double sumRatioVcHc = 0.0;
  double sumRatioVxHx = 0.0;
  double sum2RatioVcHc = 0.0;
  double sum2RatioVxHx = 0.0;

  double sumDiffVcHc = 0.0;
  double sumDiffVxHx = 0.0;
  double sum2DiffVcHc = 0.0;
  double sum2DiffVxHx = 0.0;

  double sumDbmHc = 0.0;
  double sumDbmVc = 0.0;
  double sumDbmHx = 0.0;
  double sumDbmVx = 0.0;

  double minSnr = _params.min_snr;
  double maxSnr = _params.max_snr;
  
  for (int ii = 0; ii < (int) _moments.size(); ii++) {
    
    Moments *moments = _moments[ii];

    // compute power, check SNR, for each channel

    double powerHc = moments->powerHc - _noisePowerHc;
    if (powerHc <= 0) {
      continue;
    }
    double dbmHc = 10.0 * log10(powerHc);
    double snrHc = dbmHc - _noiseDbmHc;

    double powerVc = moments->powerVc - _noisePowerVc;
    if (powerVc <= 0) {
      continue;
    }
    double dbmVc = 10.0 * log10(powerVc);
    double snrVc = dbmVc - _noiseDbmVc;

    double powerHx = moments->powerHx - _noisePowerHx;
    if (powerHx <= 0) {
      continue;
    }
    double dbmHx = 10.0 * log10(powerHx);
    double snrHx = dbmHx - _noiseDbmHx;

    double powerVx = moments->powerVx - _noisePowerVx;
    if (powerVx <= 0) {
      continue;
    }
    double dbmVx = 10.0 * log10(powerVx);
    double snrVx = dbmVx - _noiseDbmVx;

    double snr = (snrHc + snrVc + snrHx + snrVx) / 4.0;
    if (snr < minSnr || snr > maxSnr) {
      continue;
    }

    sumDbmHc += dbmHc;
    sumDbmVc += dbmVc;
    sumDbmHx += dbmHx;
    sumDbmVx += dbmVx;

    double diffVcHc = dbmVc - dbmHc;
    double diffVxHx = dbmVx - dbmHx;

    sumDiffVcHc += diffVcHc;
    sumDiffVxHx += diffVxHx;

    sum2DiffVcHc += diffVcHc * diffVcHc;
    sum2DiffVxHx += diffVxHx * diffVxHx;

    double ratioVcHc = powerVc / powerHc;
    double ratioVxHx = powerVx / powerHx;

    sumRatioVcHc += ratioVcHc;
    sumRatioVxHx += ratioVxHx;

    sum2RatioVcHc += ratioVcHc * ratioVcHc;
    sum2RatioVxHx += ratioVxHx * ratioVxHx;

    _nBeamsUsed++;

  } // ii

  if (_nBeamsUsed < 5) {
    _meanDbmHc = Moments::missing;
    _meanDbmHx = Moments::missing;
    _meanDbmVc = Moments::missing;
    _meanDbmVx = Moments::missing;
    _meanDiffVcHc = Moments::missing;
    _meanDiffVxHx = Moments::missing;
    _sdevDiffVcHc = Moments::missing;
    _sdevDiffVxHx = Moments::missing;
    _twoSigmaDiffVcHc = Moments::missing;
    _twoSigmaDiffVxHx = Moments::missing;
    _meanRatioVcHc = Moments::missing;
    _meanRatioVxHx = Moments::missing;
    _sdevRatioVcHc = Moments::missing;
    _sdevRatioVxHx = Moments::missing;
    _twoSigmaRatioVcHc = Moments::missing;
    _twoSigmaRatioVxHx = Moments::missing;
    return;
  }

  // mean powers

  _meanDbmHc = sumDbmHc / _nBeamsUsed;
  _meanDbmHx = sumDbmHx / _nBeamsUsed;
  _meanDbmVc = sumDbmVc / _nBeamsUsed;
  _meanDbmVx = sumDbmVx / _nBeamsUsed;

  // mean of diffs

  _meanDiffVcHc = sumDiffVcHc / _nBeamsUsed;
  _meanDiffVxHx = sumDiffVxHx / _nBeamsUsed;

  // sdev of diffs

  double varDiffVcHc =
    (sum2DiffVcHc - (sumDiffVcHc * sumDiffVcHc) /  _nBeamsUsed) /
    (_nBeamsUsed - 1.0);
  if (varDiffVcHc >= 0.0) {
    _sdevDiffVcHc = sqrt(varDiffVcHc);
  } else {
    _sdevDiffVcHc = 0.00001;
  }
  
  double varDiffVxHx =
    (sum2DiffVxHx - (sumDiffVxHx * sumDiffVxHx) /  _nBeamsUsed) /
    (_nBeamsUsed - 1.0);
  if (varDiffVxHx >= 0.0) {
    _sdevDiffVxHx = sqrt(varDiffVxHx);
  } else {
    _sdevDiffVxHx = 0.00001;
  }

  // twoSigma of diffs

  _twoSigmaDiffVcHc = sqrt(varDiffVcHc / _nBeamsUsed) * 2.0;
  _twoSigmaDiffVxHx = sqrt(varDiffVxHx / _nBeamsUsed) * 2.0;

  // mean of ratios

  _meanRatioVcHc = sumRatioVcHc / _nBeamsUsed;
  _meanRatioVxHx = sumRatioVxHx / _nBeamsUsed;

  // sdev of ratios

  double varRatioVcHc =
    (sum2RatioVcHc - (sumRatioVcHc * sumRatioVcHc) /  _nBeamsUsed) /
    (_nBeamsUsed - 1.0);
  if (varRatioVcHc >= 0.0) {
    _sdevRatioVcHc = sqrt(varRatioVcHc);
  } else {
    _sdevRatioVcHc = 0.00001;
  }
  
  double varRatioVxHx =
    (sum2RatioVxHx - (sumRatioVxHx * sumRatioVxHx) /  _nBeamsUsed) /
    (_nBeamsUsed - 1.0);
  if (varRatioVxHx >= 0.0) {
    _sdevRatioVxHx = sqrt(varRatioVxHx);
  } else {
    _sdevRatioVxHx = 0.00001;
  }

  // twoSigma of ratios

  _twoSigmaRatioVcHc = sqrt(varRatioVcHc / _nBeamsUsed) * 2.0;
  _twoSigmaRatioVxHx = sqrt(varRatioVxHx / _nBeamsUsed) * 2.0;

}

///////////////////////////////
// write out results to files

int SunCpCompute::_writeResults()

{
  
  // create the directory for the output file

  if (ta_makedir_recurse(_params.output_dir)) {
    int errNum = errno;
    cerr << "ERROR - SunCpCompute::_writeResults";
    cerr << "  Cannot create output dir: " << _params.output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // compute file path
  
  time_t calTime = (time_t) _calTime;
  DateTime ctime(calTime);
  char path[1024];
  sprintf(path, "%s/SunCpCompute.out.%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
          _params.output_dir,
          ctime.getYear(),
          ctime.getMonth(),
          ctime.getDay(),
          ctime.getHour(),
          ctime.getMin(),
          ctime.getSec());
  
  // if (_params.debug) {
  cerr << "writing output file: " << path << endl;
  // }

  // open file

  FILE *out;
  if ((out = fopen(path, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SunCpCompute::_writeResults";
    cerr << "  Cannot create file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  fprintf(out, "Sun Cross-polar power analysis\n");
  fprintf(out, "==============================\n");
  fprintf(out, "\n");

  fprintf(out, "Data time: %s\n",
          DateTime::strm((time_t) _calTime).c_str());
  fprintf(out, "\n");
  
  fprintf(out, "Radar lat (deg): %g\n", _params.radar_lat); 
  fprintf(out, "Radar lon (deg): %g\n", _params.radar_lon); 
  fprintf(out, "\n");

  fprintf(out, "Max az delta from sun center (deg): %g\n",
          _params.max_delta_az); 
  fprintf(out, "Max el delta from sun center (deg): %g\n",
          _params.max_delta_el); 
  fprintf(out, "\n");

  fprintf(out, "Min SNR (db): %g\n", _params.min_snr); 
  fprintf(out, "Max SNR (db): %g\n", _params.max_snr); 
  fprintf(out, "\n");

  fprintf(out, "N samples per beam: %d\n", _nSamples); 
  fprintf(out, "\n");

  fprintf(out, "N beams Noise: %g\n", _nBeamsNoise);
  fprintf(out, "noiseDbmHc: %10.4f dBm\n", _noiseDbmHc);
  fprintf(out, "noiseDbmHx: %10.4f dBm\n", _noiseDbmHx);
  fprintf(out, "noiseDbmVc: %10.4f dBm\n", _noiseDbmVc);
  fprintf(out, "noiseDbmVx: %10.4f dBm\n", _noiseDbmVx);
  fprintf(out, "\n");

  fprintf(out, "N beams used: %g\n", _nBeamsUsed); 
  fprintf(out, "meanDbmHc: %10.4f dBm\n", _meanDbmHc);
  fprintf(out, "meanDbmHx: %10.4f dBm\n", _meanDbmHx);
  fprintf(out, "meanDbmVc: %10.4f dBm\n", _meanDbmVc);
  fprintf(out, "meanDbmVx: %10.4f dBm\n", _meanDbmVx);
  fprintf(out, "\n");

  fprintf(out, "Means of the diffs\n");
  fprintf(out, "  Vc - Hc: %10.4f dB\n", _meanDiffVcHc);
  fprintf(out, "  Vx - Hx: %10.4f dB\n", _meanDiffVxHx);
  fprintf(out, "  S1S2:    %10.4f dB\n", _meanDiffVxHx + _meanDiffVcHc);
  fprintf(out, "\n");

  fprintf(out, "Means of the ratios\n");
  fprintf(out, "  Vc / Hc: %10.4f\n", _meanRatioVcHc);
  fprintf(out, "  Vx / Hx: %10.4f\n", _meanRatioVxHx);
  fprintf(out, "  Vc / Hc: %10.4f dB\n", 10.0 * log10(_meanRatioVcHc));
  fprintf(out, "  Vx / Hx: %10.4f dB\n", 10.0 * log10(_meanRatioVxHx));
  fprintf(out, "  S1S2:    %10.4f dB\n",
          10.0 * log10(_meanRatioVcHc * _meanRatioVxHx));
  fprintf(out, "\n");

  fprintf(out, "Standard deviations of the diffs\n");
  fprintf(out, "  Vc - Hc: %10.4f dB\n", _sdevDiffVcHc);
  fprintf(out, "  Vx - Hx: %10.4f dB\n", _sdevDiffVxHx);
  fprintf(out, "\n");

  fprintf(out, "Standard deviations of the ratios\n");
  fprintf(out, "  Vc / Hc: %10.4f\n", _sdevRatioVcHc);
  fprintf(out, "  Vx / Hx: %10.4f\n", _sdevRatioVxHx);
  fprintf(out, "\n");

  fprintf(out, "Uncertainty of the diffs (2 sigma)\n");
  fprintf(out, "  Vc - Hc: %10.4f dB\n", _twoSigmaDiffVcHc);
  fprintf(out, "  Vx - Hx: %10.4f dB\n", _twoSigmaDiffVxHx);
  fprintf(out, "\n");

  fprintf(out, "Uncertainty of the ratios (2 sigma)\n");
  fprintf(out, "  Vc / Hc: %10.4f\n", _twoSigmaRatioVcHc);
  fprintf(out, "  Vx / Hx: %10.4f\n", _twoSigmaRatioVxHx);
  fprintf(out, "\n");

  fclose(out);
  return 0;

}

///////////////////////////
// check for north crossing
// and adjust accordingly

void SunCpCompute::_checkForNorthCrossing(double &az0, double &az1)

{

  if (az0 - az1 > 180) {
    az0 -= 360.0;
  } else if (az0 - az1 < -180) {
    az1 -= 360.0;
  }

}

