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

using namespace std;

////////////////////////
// This class

class Stats {
  
public:

  // constructor

  Stats();

  // destructor
  
  ~Stats();

  // initialize - set memvbers to 0

  void init();

  // sum up simultaneous information
  
  void addToSim(double ii0, double qq0,
                bool haveChan1,
                double ii1, double qq1);
  
  // compute sim stats
  // Assumes data has been added
  
  void computeSim();

  // sum up alternating information
  
  void addToAlternating(double ii0, double qq0,
			bool haveChan1,
			double ii1, double qq1,
			bool isHoriz);
  
  // compute alternating stats
  // Assumes data has been added
  
  void computeAlternating(bool haveChan1);

  // data

  double nnH, nnV;
  
  double sumPowerHc, sumPowerHx;
  double sumPowerVc, sumPowerVx;
  
  double meanDbmHc, meanDbmHx;
  double meanDbmVc, meanDbmVx;

  double noiseDbmHc, noiseDbmHx;
  double noiseDbmVc, noiseDbmVx;
  
protected:
private:

};

#endif
