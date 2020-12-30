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
// Stats.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////
//
// Data accumulation and stats at a gate or over multiple gates
//
////////////////////////////////////////////////////////////////

#ifndef Stats_HH
#define Stats_HH

#include <radar/RadarComplex.hh>
#include <radar/IwrfTsPulse.hh>

using namespace std;

////////////////////////
// This class

class Stats {
  
public:

  // constructor

  Stats();

  // destructor
  
  ~Stats();

  // set calibration

  void setCalibration(double gainHc,
                      double gainVc,
                      double gainHx,
                      double gainVx);

  // initialize - set members to 0

  void init();

  // resize arrays for number of gates

  void setNGates(int nGates);

  // sum up summary information
  
  void addToSummary(const IwrfTsPulse &pulse,
                    double ii0, double qq0,
                    bool haveChan1,
                    double ii1, double qq1);
  
  // sum up alternating information
  
  void addToAlternating(const IwrfTsPulse &pulse,
                        double ii0, double qq0,
			bool haveChan1,
			double ii1, double qq1,
			bool isHoriz);

  // compute summary stats
  // Assumes data has been added
  
  void computeSummary(bool haveChan1);

  // compute alternating stats
  // Assumes data has been added
  
  void computeAlternating(bool haveChan1);

  // data

  double nn0, nn1;
  double nnH, nnV;
  
  double sumPower0, sumPower1;
  double meanDbm0, meanDbm1;
  
  double sumPowerHc, sumPowerHx;
  double sumPowerVc, sumPowerVx;

  double meanDbmHc, meanDbmHx;
  double meanDbmVc, meanDbmVx;

  double lag1DbmHc, lag1DbmHx;
  double lag1DbmVc, lag1DbmVx;

  double noiseDbmHc, noiseDbmHx;
  double noiseDbmVc, noiseDbmVx;
  
  RadarComplex_t sumConjProdH;
  RadarComplex_t sumConjProdV;
  
  double corrH;
  double corrV;
  
  double argH;
  double argV;

  vector<vector<RadarComplex_t> > iqHc, iqVc, iqHx, iqVx;

protected:
  
private:

  int _nGates;

  // calibration

  double _rxGainHc;
  double _rxGainVc;
  double _rxGainHx;
  double _rxGainVx;

};

#endif
