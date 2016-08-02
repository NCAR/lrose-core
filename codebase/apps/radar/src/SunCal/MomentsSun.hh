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
// MomentsSun.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2006
//
///////////////////////////////////////////////////////////////

#ifndef MomentsSun_HH
#define MomentsSun_HH

#include <vector>
#include <iostream>
#include <radar/RadarComplex.hh>
using namespace std;

////////////////////////
// This class

class MomentsSun {
  
public:
  
  // constructor
  
  MomentsSun();
  
  // destructor
  
  ~MomentsSun();
  
  // Set values to missing
  
  void init();
  
  // print

  void print(ostream &out);

  // copy

  void copy(const MomentsSun &orig);
  
  // interpolate
  
  void interp(const MomentsSun &stats1,
              const MomentsSun &stats2,
              double weight1,
              double weight2);

  // adjust power for noise

  void adjustForNoise(double noisePowerHc,
                      double noisePowerHx,
                      double noisePowerVc,
                      double noisePowerVx);

  // compute power ratios

  void computeRatios();

  // print

  void print(ostream &out) const;

  // data

  static const double missing;

  double time;
  double prt;
  double az;
  double el;
  double offsetAz;
  double offsetEl;
  
  double nn;
  
  double powerHc;
  double powerVc;
  double powerHx;
  double powerVx;
  
  double dbmHc;
  double dbmHx;
  double dbmVc;
  double dbmVx;

  double dbm;
  double dbBelowPeak;

  RadarComplex_t sumRvvhh0Hc;
  RadarComplex_t sumRvvhh0Vc;
  RadarComplex_t sumRvvhh0;
  
  RadarComplex_t Rvvhh0Hc;
  RadarComplex_t Rvvhh0Vc;
  RadarComplex_t Rvvhh0;

  double corrMagHc;
  double corrMagVc;
  double corrMag;

  double corr00Hc;
  double corr00Vc;
  double corr00;

  double arg00Hc;
  double arg00Vc;
  double arg00;

  double ratioDbmHcHx;
  double ratioDbmVcVx;

  double ratioDbmVxHc;
  double ratioDbmVcHx;

  double ratioDbmVcHc;
  double ratioDbmVxHx;

  double S1S2;
  double SS;

  double zdr;
  double phidp;
  double rhohv;
  double ncp;

protected:
private:
  
};

#ifdef __in_moments_cc__
const double MomentsSun::missing = -9999;
#endif

#endif

