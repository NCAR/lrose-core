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
// Stats.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////
//
// Data accumulation and stats at a gate or over multiple gates
//
////////////////////////////////////////////////////////////////

#include "Stats.hh"
#include <cmath>
#include <iostream>

// Constructor

Stats::Stats()
  
{

  init();

}

// destructor

Stats::~Stats()

{

}

// initialize

void Stats::init()
  
{
  
  nnH = 0;
  nnV = 0;

  sumPowerHc = 0.0;
  sumPowerHx = 0.0;
  sumPowerVc = 0.0;
  sumPowerVx = 0.0;

}

////////////////////////////////////////////
// sum up simultaneous information

void Stats::addToSim(double ii0, double qq0,
                     bool haveChan1,
                     double ii1, double qq1)

{

  if (!haveChan1) {

    double power0 = ii0 * ii0 + qq0 * qq0;
    sumPowerHc += power0;
    nnH++;

  } else {
    
    double power0 = ii0 * ii0 + qq0 * qq0;
    double power1 = ii1 * ii1 + qq1 * qq1;

    sumPowerHc += power0;
    sumPowerVc += power1;
    
    nnH++;
    nnV++;

  }
    
}

/////////////////////////////
// compute simultaneous stats
//
// Assumes data has been added

void Stats::computeSim()
  
{
  
  meanDbmHc = -999.9;
  meanDbmHx = -999.9;
  meanDbmVx = -999.9;
  meanDbmVc = -999.9;
  
  if (nnH > 0) {

    double meanPowerHc = sumPowerHc / nnH;
    meanDbmHc = 10.0 * log10(meanPowerHc);
    
  }
  
  if (nnV > 0) {
    
    double meanPowerVc = sumPowerVc / nnV;
    meanDbmVc = 10.0 * log10(meanPowerVc);

  }

}

////////////////////////////////////////////
// sum up alternating information

void Stats::addToAlternating(double ii0, double qq0,
			     bool haveChan1,
                             double ii1, double qq1,
                             bool isHoriz)

{

  if (!haveChan1) {

    double power0 = ii0 * ii0 + qq0 * qq0;
    
    if (isHoriz) {
      nnH++;
      sumPowerHc += power0;
    } else {
      nnV++;
      sumPowerVc += power0;
    }

  } else {
    
    double power0 = ii0 * ii0 + qq0 * qq0;
    double power1 = ii1 * ii1 + qq1 * qq1;

    if (isHoriz) {
      nnH++;
      sumPowerHc += power0;
      sumPowerVx += power1;
    } else {
      nnV++;
      sumPowerVc += power0;
      sumPowerHx += power1;
    }

  }
    
}

////////////////////////////
// compute alternating stats
//
// Assumes data has been added

void Stats::computeAlternating(bool haveChan1)
  
{
  
  meanDbmHc = -999.9;
  meanDbmHx = -999.9;
  meanDbmVx = -999.9;
  meanDbmVc = -999.9;
  
  if (nnH > 0) {

    double meanPowerHc = sumPowerHc / nnH;
    meanDbmHc = 10.0 * log10(meanPowerHc);

    if (haveChan1) {
      
      double meanPowerVx = sumPowerVx / nnH;
      meanDbmVx = 10.0 * log10(meanPowerVx);
      
    }
  
  }
  
  if (nnV > 0) {

    double meanPowerVc = sumPowerVc / nnV;
    meanDbmVc = 10.0 * log10(meanPowerVc);

    if (haveChan1) {
      
      double meanPowerHx = sumPowerHx / nnV;
      meanDbmHx = 10.0 * log10(meanPowerHx);
      
    }
    
  }
  
}

