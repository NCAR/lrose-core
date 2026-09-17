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
/////////////////////////////////////////////////////////////
// RadarTimeSeriesSim
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2026
//
///////////////////////////////////////////////////////////////
//
// Radar time series simulator.
// Converted from FORTRAN code from John Hubbert.
// C++ conversion with the help of ChatGpt.
//
///////////////////////////////////////////////////////////////

#include <radar/RadarTimeSeriesSim.hh>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
constexpr double PI = 3.1415926;       // preserve simulate.f constant
constexpr double PI_VEL = 3.14159265;  // preserve scalevelocity constant
}

RadarTimeSeriesSim::RadarTimeSeriesSim() :
        _fft(NSA),
        _rng(std::random_device{}()),
        _uniform(0.0, 1.0)
{
}

void RadarTimeSeriesSim::setSeed(uint64_t seed)
{
  _rng.seed(seed);
}

double RadarTimeSeriesSim::_uniformOpen()
{
  double x = _uniform(_rng);
  return (x == 0.0) ? 1.0e-30 : x;
}

void RadarTimeSeriesSim::simulate(std::vector<RadarComplex_t> &h,
                                  std::vector<RadarComplex_t> &v,
                                  double sigv, int nn,
                                  const std::complex<double> &rhohv,
                                  double zdr, double wavelengthM,
                                  double prtSec)
{
  if (nn < 1 || nn > NSA) {
    throw std::invalid_argument("RadarTimeSeriesSim::simulate: require 1 <= nn <= 256");
  }
  if (sigv <= 0.0 || zdr <= 0.0 || wavelengthM <= 0.0 || prtSec <= 0.0) {
    throw std::invalid_argument("RadarTimeSeriesSim::simulate: sigv, zdr, wavelength and PRT must be positive");
  }

  const int n = NSA;
  const double fn = static_cast<double>(n);
  const double fmax = 0.5 / prtSec;
  const double delf = 2.0 * fmax / fn;
  const double tpi = 2.0 * PI;
  const double sigf = 2.0 * sigv / wavelengthM;
  const double den1 = 2.0 * sigf * sigf;
  const double cons = fmax / (sigf * std::sqrt(tpi) * fn);

  std::vector<RadarComplex_t> specH(n, RadarComplex_t(0.0, 0.0));
  std::vector<RadarComplex_t> specV(n, RadarComplex_t(0.0, 0.0));

  double f = -fmax;
  for (int ii = 0; ii < n; ++ii, f += delf) {

    // Form Gaussian shape power spectra -- simulate.f lines around label 100.
    const double fe = f * f / den1;
    const double f11 = cons * std::exp(-fe);
    if (f11 == 0.0) {
      continue;
    }
    const double f22 = f11 / zdr;
    const std::complex<double> f12c = rhohv * std::sqrt(f11 * f22);
    const double c12 = f12c.real();
    const double q12 = f12c.imag();

    // Innovation algorithm, kept in the same algebraic form as John's code.
    const double v0 = f11;
    const double tet11 = 0.0;
    const double v1 = v0;
    const double tet22 = c12 / f11;
    const double tet21 = -q12 / f11;
    double v2 = f22 - v0 * tet22 * tet22 - v1 * tet21 * tet21;
    if (v2 == 0.0) {
      v2 = 1.0e-30;
    }
    const double tet33 = q12 / f11;
    const double tet32 = c12 / f11;
    const double tet31 = (-v0 * tet33 * tet22 - v1 * tet32 * tet21) / v2;
    double v3 = f22 - v0 * tet33 * tet33 - v1 * tet32 * tet32 -
                v2 * tet31 * tet31;

    // Four iid N(0,1) values using the same two Box-Muller pairs.
    double w[4];
    for (int jj = 0; jj < 2; ++jj) {
      const double x1 = _uniformOpen();
      const double x2 = _uniformOpen();
      const double rr = std::sqrt(-2.0 * std::log(x1));
      w[2 * jj] = rr * std::cos(2.0 * PI * x2);
      w[2 * jj + 1] = rr * std::sin(2.0 * PI * x2);
    }

    // Preserve the original protection against negative roundoff values.
    v2 = std::abs(v2);
    if (v2 == 0.0) {
      v3 = 0.0;
    } else {
      v3 = std::abs(v3);
    }

    const double a1 = std::sqrt(v0) * w[0];
    const double b1 = std::sqrt(v1) * w[1] + tet11 * std::sqrt(v0) * w[0];
    const double a2 = std::sqrt(v2) * w[2] + tet21 * std::sqrt(v1) * w[1] +
                      tet22 * std::sqrt(v0) * w[0];
    const double b2 = std::sqrt(v3) * w[3] + tet31 * std::sqrt(v2) * w[2] +
                      tet32 * std::sqrt(v1) * w[1] +
                      tet33 * std::sqrt(v0) * w[0];

    specH[ii].set(a1, b1);
    specV[ii].set(a2, b2);
  }

  // John's ifft_it() uses FFTPACK CFFT1B, then simulate.f divides by n.
  // RadarFft::inv() is unitary and divides the FFTW backward transform by sqrt(n),
  // so divide once more by sqrt(n) here to preserve the legacy net 1/n scaling.
  std::vector<RadarComplex_t> timeH(n), timeV(n);
  _fft.inv(specH.data(), timeH.data());
  _fft.inv(specV.data(), timeV.data());
  const double extraScale = 1.0 / std::sqrt(fn);

  h.resize(nn);
  v.resize(nn);
  for (int ii = 0; ii < nn; ++ii) {
    h[ii].set(timeH[ii].re * extraScale, timeH[ii].im * extraScale);
    v[ii].set(timeV[ii].re * extraScale, timeV[ii].im * extraScale);
  }
}

void RadarTimeSeriesSim::scaleVelocity(std::vector<RadarComplex_t> &h,
                                       std::vector<RadarComplex_t> &v,
                                       double prtSec, double wavelengthM,
                                       double velocityMps)
{
  if (h.size() != v.size() || h.empty()) {
    throw std::invalid_argument("RadarTimeSeriesSim::scaleVelocity: H/V sizes must match and be nonzero");
  }
  const int n = static_cast<int>(h.size());
  const double scalevel = 2.0 * (wavelengthM / (4.0 * prtSec)) / n;
  const double vel = velocityMps / scalevel;

  // Fortran indices start at 1, so preserve i = 1..n in the phase expression.
  for (int jj = 0; jj < n; ++jj) {
    const int i = jj + 1;
    const double phase = PI_VEL * (static_cast<double>(i) + 2.0 * vel * i / n);
    const double cs = std::cos(phase);
    const double sn = std::sin(phase);
    auto rotate = [cs, sn](RadarComplex_t &x) {
      const double re = x.re * cs - x.im * sn;
      const double im = x.re * sn + x.im * cs;
      x.set(re, im);
    };
    rotate(h[jj]);
    rotate(v[jj]);
  }
}

void RadarTimeSeriesSim::scaleDb(std::vector<RadarComplex_t> &h,
                                 std::vector<RadarComplex_t> &v,
                                 double scaleDbVal)
{
  if (h.size() != v.size()) {
    throw std::invalid_argument("RadarTimeSeriesSim::scaleDb: H/V sizes must match");
  }
  // Exact Phase-1 equivalent of avglin=10**(-4.81585) in scaledb().
  const double avgLin = std::pow(10.0, -4.81585);
  const double sclLin = std::pow(10.0, scaleDbVal / 10.0);
  const double s = std::sqrt(sclLin / avgLin);
  for (size_t ii = 0; ii < h.size(); ++ii) {
    h[ii].re *= s; h[ii].im *= s;
    v[ii].re *= s; v[ii].im *= s;
  }
}

void RadarTimeSeriesSim::phaseShiftV(std::vector<RadarComplex_t> &v,
                                     double phaseRadians)
{
  const double cs = std::cos(phaseRadians);
  const double sn = std::sin(phaseRadians);
  for (auto &x : v) {
    const double re = x.re * cs - x.im * sn;
    const double im = x.re * sn + x.im * cs;
    x.set(re, im);
  }
}

void RadarTimeSeriesSim::noise(std::vector<RadarComplex_t> &iq, int nn)
{
  if (nn < 1 || nn > 64) {
    throw std::invalid_argument("RadarTimeSeriesSim::noise: legacy Phase-1 noise requires 1 <= nn <= 64");
  }
  iq.resize(nn);
  double sumPower = 0.0;
  for (int ii = 0; ii < nn; ++ii) {
    const double a1 = _uniformOpen();
    const double a2 = _uniformOpen();
    const double rr = std::sqrt(-2.0 * std::log(a1));
    const double rn1 = rr * std::cos(2.0 * PI_VEL * a2);
    const double rn2 = rr * std::sin(2.0 * PI_VEL * a2);
    iq[ii].set(rn1, rn2);
    sumPower += rn1 * rn1 + rn2 * rn2;
  }
  const double rms = std::sqrt(sumPower / nn);
  for (auto &x : iq) {
    x.re /= rms;
    x.im /= rms;
  }
}
