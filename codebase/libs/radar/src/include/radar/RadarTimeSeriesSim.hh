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

#ifndef RadarTimeSeriesSim_hh
#define RadarTimeSeriesSim_hh

#include <complex>
#include <cstdint>
#include <random>
#include <vector>
#include <radar/RadarComplex.hh>
#include <radar/RadarFft.hh>

class RadarTimeSeriesSim {
public:
  static const int NSA = 256;

  RadarTimeSeriesSim();

  // Phase-1 translation of Hubbert simulate.f.
  // zdr is linear, rhohv may be complex, sigv is m/s.
  void simulate(std::vector<RadarComplex_t> &h,
                std::vector<RadarComplex_t> &v,
                double sigv, int nn,
                const std::complex<double> &rhohv,
                double zdr, double wavelengthM, double prtSec);

  // Phase-1 translations of signal_helpers.f.
  static void scaleVelocity(std::vector<RadarComplex_t> &h,
                            std::vector<RadarComplex_t> &v,
                            double prtSec, double wavelengthM,
                            double velocityMps);
  static void scaleDb(std::vector<RadarComplex_t> &h,
                      std::vector<RadarComplex_t> &v,
                      double scaleDb);
  static void phaseShiftV(std::vector<RadarComplex_t> &v,
                          double phaseRadians);

  // Phase-1 translation of noise.f. Output has exactly unit mean power.
  void noise(std::vector<RadarComplex_t> &iq, int nn);

  // Added for reproducible C++ tests. If not called, construction seeds from random_device.
  void setSeed(uint64_t seed);

private:
  RadarFft _fft;
  std::mt19937_64 _rng;
  std::uniform_real_distribution<double> _uniform;

  double _uniformOpen();
};

#endif
