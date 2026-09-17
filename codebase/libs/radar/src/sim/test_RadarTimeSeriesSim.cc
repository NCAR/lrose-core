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

#include "RadarTimeSeriesSim.hh"
#include <cmath>
#include <iomanip>
#include <iostream>

static double meanPower(const std::vector<RadarComplex_t> &x) {
  double sum = 0.0;
  for (const auto &z : x) sum += z.re*z.re + z.im*z.im;
  return sum / x.size();
}

int main() {
  constexpr int nn = 64;
  constexpr double prt = 0.001;
  constexpr double lam = 0.1067;
  constexpr double pi = 3.1415926;

  RadarTimeSeriesSim sim;
  sim.setSeed(1234567);

  std::vector<RadarComplex_t> wh, wv, ch, cv, nh, nv;
  sim.simulate(wh, wv, 4.0, nn, {0.997, 0.0}, 1.1221, lam, prt);
  RadarTimeSeriesSim::phaseShiftV(wv, 45.0*pi/180.0);
  RadarTimeSeriesSim::scaleDb(wh, wv, 40.0);
  RadarTimeSeriesSim::scaleVelocity(wh, wv, prt, lam, 8.0);

  sim.simulate(ch, cv, 0.05, nn, {0.7, 0.0}, 0.31622, lam, prt);
  RadarTimeSeriesSim::phaseShiftV(cv, 135.0*pi/180.0);
  RadarTimeSeriesSim::scaleVelocity(ch, cv, prt, lam, 0.0);
  RadarTimeSeriesSim::scaleDb(ch, cv, -10.0);

  sim.noise(nh, nn);
  sim.noise(nv, nn);

  std::cout << std::setprecision(12)
            << "weather H power dB: " << 10.0*std::log10(meanPower(wh)) << '\n'
            << "clutter H power dB: " << 10.0*std::log10(meanPower(ch)) << '\n'
            << "noise H power: " << meanPower(nh) << '\n'
            << "noise V power: " << meanPower(nv) << '\n';
  return 0;
}
