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
// Channel.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2006
//
///////////////////////////////////////////////////////////////

#include "Channel.hh"
#include <toolsa/toolsa_macros.h>
#include <toolsa/mem.h>
#include <rapmath/umath.h>
#include <iomanip>
#include <cmath>
using namespace std;
#define MAX_EIG_DIM 3

// Constructor

Channel::Channel(const Params &params,
                 const string &short_label,
                 const string &long_label,
                 double coupling_loss,
                 double peak_power) :
        _params(params),
        _shortLabel(short_label),
        _longLabel(long_label),
        _couplingLoss(coupling_loss),
        _peakPowerW(peak_power)
  
{

  _meanNoiseIfd = 0;
  _meanNoiseGuide = 0;
  _meanGain = 0;
  _computeRadarConstant();

}

// destructor

Channel::~Channel()

{

}

// add a data point

void Channel::addDataPoint(double siggen_dbm,
                           double ifd_dbm)

{

  _siggenDbm.push_back(siggen_dbm);
  _ifdDbm.push_back(ifd_dbm);

}

// compute the calibation fit
// returns 0 on success, -1 on failure

int Channel::computeFit()
  
{
  
  // first compute mean noise
  
  if (_computeMeanNoise()) {
    return -1;
  }

  // then subtract from the ifd powers

  _subtractNoise();

  // subtract coupler loss from siggen power to get waveguide power

  _subtractCouplerLoss();

  // compute regression fit between waveguide power and ifd power minus noise
  
  if (_computeRegression()) {
    return -1;
  }

  // compute the gain as the adjusted IFD minus waveguide power

  _computeGain();

  // compute mean gain

  if (_computeMeanGain()) {
    return -1;
  }

  // compute the gain error

  _computeGainError();

  return 0;

}

// compute the noise
// returns 0 on success, -1 on failure

int Channel::_computeMeanNoise()
  
{

  double nn = 0;
  double sum = 0;

  for (int ii = 0; ii < (int) _siggenDbm.size(); ii++) {
    double siggen = _siggenDbm[ii];
    if (siggen >= _params.min_siggen_power_for_noise &&
        siggen <= _params.max_siggen_power_for_noise) {
      double ifd = _ifdDbm[ii];
      double power = pow(10.0, ifd / 10.0);
      sum += power;
      nn++;
    }
  }
  
  if (nn < 1) {
    cerr << "ERROR - Channel::_computeMeanNoise()" << endl;
    cerr << "  Not enough data for noise calculations" << endl;
    this->print(cerr);
    return -1;
  }

  _meanNoiseIfd = 10.0 * log10(sum / nn);

  return 0;

}

// subtract the noise from the ifd data

void Channel::_subtractNoise()
  
{

  _ifdMinusNoise.clear();

  double noisePower = pow(10.0, _meanNoiseIfd / 10.0);

  for (int ii = 0; ii < (int) _siggenDbm.size(); ii++) {
    double dbm = _ifdDbm[ii];
    double power = pow(10.0, dbm / 10.0);
    double adjusted = power - noisePower;
    if (adjusted < 0) {
      adjusted = 1.0e-12;
    }
    double adjustedDbm = 10.0 * log10(adjusted);
    _ifdMinusNoise.push_back(adjustedDbm);
  }

}

// subtract the coupler loss from the siggen data

void Channel::_subtractCouplerLoss()
  
{

  _waveguideDbm.clear();

  for (int ii = 0; ii < (int) _siggenDbm.size(); ii++) {
    double siggen = _siggenDbm[ii];
    double waveguide = siggen - _couplingLoss;
    _waveguideDbm.push_back(waveguide);
  }

}

// compute the receiver gain

void Channel::_computeGain()
  
{

  _gain.clear();

  for (int ii = 0; ii < (int) _siggenDbm.size(); ii++) {
    double gain = _ifdMinusNoise[ii] - _waveguideDbm[ii];
    _gain.push_back(gain);
  }

}

// compute the mean gain
// returns 0 on success, -1 on failure

int Channel::_computeMeanGain()
  
{

  double nn = 0;
  double sum = 0;

  for (int ii = 0; ii < (int) _siggenDbm.size(); ii++) {
    double siggen = _siggenDbm[ii];
    if (siggen >= _params.min_siggen_power_for_gain &&
        siggen <= _params.max_siggen_power_for_gain) {
      double gain = _gain[ii];
      sum += pow(10.0, gain / 10.0);
      nn++;
    }
  }

  if (nn < 1) {
    cerr << "ERROR - Channel::_computeMeanGain()" << endl;
    cerr << "  Not enough data for gain calculations" << endl;
    this->print(cerr);
    return -1;
  }

  _meanGain = 10.0 * log10(sum / nn);
  _meanNoiseGuide = _meanNoiseIfd - _meanGain;
  _dbz0 = _meanNoiseGuide - _radarConst;

  return 0;

}

// compute the gain error
// i.e. defferences between gain and mean gain

void Channel::_computeGainError()
  
{

  _gainError.clear();

  for (int ii = 0; ii < (int) _gain.size(); ii++) {
    double error = _meanGain - _gain[ii];
    _gainError.push_back(error);
  }

}

/////////////////////////////////////////////////
// compute the regression of ifd to siggen
// returns 0 on success, -1 on failure

int Channel::_computeRegression()

{

  _slope = 0;
  _slope_y_on_x = 0;
  _slope_x_on_y = 0;
  _intercept = 0;
  _corr = 0;

  int npts = (int) _waveguideDbm.size();
  if (npts < 3) {
    cerr << "ERROR - Channel::_computeRegression" << endl;
    cerr << "  Too few points for computing slope" << endl;
    return -1;
  }

  // sum up for regression
  
  double sumx = 0.0, sumx2 = 0.0;
  double sumy = 0.0, sumy2 = 0.0, sumxy = 0.0;
  double dn = 0;

  for (int ii = 0; ii < (int) _waveguideDbm.size(); ii++) {
    double xx = _ifdMinusNoise[ii];
    // double xx = _ifdDbm[ii];
    double yy = _waveguideDbm[ii];
    sumx += xx;
    sumx2 += xx * xx;
    sumy += yy;
    sumy2 += yy * yy;
    sumxy += xx * yy;
    dn++;
  }

  // compute the terms

  double term1x = dn * sumx2 - sumx * sumx;
  double term1y = dn * sumy2 - sumy * sumy;

  // double term2x = sumy * sumx2 - sumx * sumxy;
  // double term2y = sumx * sumy2 - sumy * sumxy;

  double term3 = dn * sumxy - sumx * sumy;

  double meanx = sumx / dn;
  double meany = sumy / dn;
  
  // compute slope and correlation from regression

  _slope_y_on_x = term3 / term1x;
  _slope_x_on_y = term3 / term1y;
  _slope = (_slope_y_on_x + (1.0 / _slope_x_on_y)) / 2.0;
  _corr = fabs(term3 / sqrt(fabs(term1x * term1y)));
  
  // try computing slope using principal components
  
  double means[MAX_EIG_DIM];
  double eigenvalues[MAX_EIG_DIM];
  double **eigenvectors =
    (double **) umalloc2 (MAX_EIG_DIM, MAX_EIG_DIM, sizeof(double));
  double **coords = (double **) umalloc2 (npts, 2, sizeof(double));
  
  for (int ii = 0; ii < npts; ii++) {
    double xx = _ifdMinusNoise[ii];
    double yy = _waveguideDbm[ii];
    coords[ii][0] = xx;
    coords[ii][1] = yy;
  }
  
  if (upct(2, npts, coords, means, eigenvectors, eigenvalues)) {
    cerr << "ERROR - Channel::_computeRegression" << endl;
    cerr << "  Cannot compute principal components, using regression instead." << endl;
    ufree2((void **) eigenvectors);
    ufree2((void **) coords);
    return -1;
  }

  _slope = eigenvectors[1][0] / eigenvectors[0][0];
  
  ufree2((void **) eigenvectors);
  ufree2((void **) coords);

  // compute intercept

  double dy = _slope * meanx;
  _intercept = meany - dy;
  
  return 0;

}

// compute the radar constant

void Channel::_computeRadarConstant()
  
{

  double piCubed = pow(M_PI, 3.0);
  double lightSpeed = 299792458.0;
  double kSquared = 0.93;

  double wavelengthCm = _params.wavelength_cm;
  double wavelengthM = wavelengthCm / 100.0;
  double horizBeamWidthDeg = _params.horiz_beam_width;
  double vertBeamWidthDeg = _params.vert_beam_width;
  double antGainDb = _params.antenna_gain_db;
  double pulseWidthUs = _params.pulse_width_us;
  double waveguideLoss = _params.waveguide_loss_db;
  double radomeLoss = _params.radome_loss_db;
  double receiverLoss = _params.receiver_loss_db;

  double antGainPower = pow(10.0, antGainDb / 10.0);
  double gainSquared = antGainPower * antGainPower;
  double lambdaSquared = wavelengthM * wavelengthM;
  double pulseMeters = pulseWidthUs * 1.0e-6 * lightSpeed;
  
  double hBeamWidthRad = horizBeamWidthDeg * DEG_TO_RAD;
  double vBeamWidthRad = vertBeamWidthDeg * DEG_TO_RAD;

  double peakPowerMilliW = _peakPowerW * 1000;

  double num = (peakPowerMilliW * piCubed * pulseMeters * gainSquared *
                hBeamWidthRad * vBeamWidthRad * kSquared * 1.0e-24);

  double denom = (1024.0 * log(2.0) * lambdaSquared);
  
  double factor = num / denom;
  
  _radarConst = 10.0 * log10(factor) - radomeLoss - receiverLoss;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Computing radar constant" << endl;
    cerr << "  wavelengthCm: " << wavelengthCm << endl;
    cerr << "  horizBeamWidthDeg: " << horizBeamWidthDeg << endl;
    cerr << "  vertBeamWidthDeg: " << vertBeamWidthDeg << endl;
    cerr << "  antGainDb: " << antGainDb << endl;
    cerr << "  peakPowerW: " << _peakPowerW << endl;
    cerr << "  pulseWidthUs: " << pulseWidthUs << endl;
    cerr << "  waveguideLoss: " << waveguideLoss << endl;
    cerr << "  RadarConst: " << _radarConst << endl;
  }

}

// print

void Channel::print(ostream &out)

{

  out << endl;
  out << "  shortLabel: " << _shortLabel << endl;
  out << "  longLabel: " << _longLabel << endl;
  out << "  couplingLoss: " << _couplingLoss << endl;
  out << "  peakPower: " << _peakPowerW << endl;
  out << "  radarConstant: " << _radarConst << endl;
  out << "  siggen min power for computing noise: "
      << _params.min_siggen_power_for_noise << endl;
  out << "  siggen max power for computing noise: "
      << _params.max_siggen_power_for_noise << endl;
  out << "  siggen min power for computing gain: "
      << _params.min_siggen_power_for_gain << endl;
  out << "  siggen max power for computing gain: "
      << _params.max_siggen_power_for_gain << endl;


  out << endl;

  out << setw(15) << "siggen_dBm"
      << setw(15) << "ifd_dBm"
      << setw(15) << "waveguide_dBm"
      << setw(15) << "ifd-noise"
      << setw(15) << "gain"
      << setw(15) << "gain_error"
      << endl;

  for (int ii = 0; ii < (int) _siggenDbm.size(); ii++) {
    double siggen = _siggenDbm[ii];
    double ifd = _ifdDbm[ii];
    double guide = -9999;
    if (ii < (int) _waveguideDbm.size()) {
      guide = _waveguideDbm[ii];
    }
    double ifdMinus = -9999;
    if (ii < (int) _ifdMinusNoise.size()) {
      ifdMinus = _ifdMinusNoise[ii];
    }
    double gain = -9999;
    if (ii < (int) _gain.size()) {
      gain = _gain[ii];
    }
    double error = -9999;
    if (ii < (int) _gainError.size()) {
      error = _gainError[ii];
    }
    char text[1024];
    sprintf(text, "%15.3f%15.3f%15.3f%15.3f%15.3f%15.3f",
            siggen, ifd, guide, ifdMinus, gain, error);
    out << text << endl;
  }
  out << endl;
  out << "  mean noise at IFD  : " << _meanNoiseIfd << endl;
  out << "  mean gain          : " << _meanGain << endl;
  out << "  mean noise wv guide: " << _meanNoiseGuide << endl;
  out << "  SNR0 dbz at 1km    : " << _dbz0 << endl;
  out << endl;
  out << "  slope              : " << _slope << endl;
  out << "  slope Y on X       : " << _slope_y_on_x << endl;
  out << "  slope X on Y       : " << _slope_x_on_y << endl;
  out << "  intercept          : " << _intercept << endl;
  out << "  correlation        : " << _corr << endl;
  
}

