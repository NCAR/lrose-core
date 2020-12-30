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
  _rxGainHc = 1.0;
  _rxGainVc = 1.0;
  _rxGainHx = 1.0;
  _rxGainVx = 1.0;

}

// destructor

Stats::~Stats()

{

}

// initialize

void Stats::init()
  
{
  
  sumPower0 = 0.0;
  sumPower1 = 0.0;
  nn0 = 0.0;
  nn1 = 0.0;
    
  nnH = 0;
  nnV = 0;

  sumPowerHc = 0.0;
  sumPowerHx = 0.0;
  sumPowerVc = 0.0;
  sumPowerVx = 0.0;

  sumConjProdH.re = 0.0;
  sumConjProdH.im = 0.0;
  sumConjProdV.re = 0.0;
  sumConjProdV.im = 0.0;

  iqHc.clear();
  iqVc.clear();
  iqHx.clear();
  iqVx.clear();

  _nGates = 0;

}

void Stats::setNGates(int nGates)
  
{

  if (nGates > _nGates) {
    _nGates = nGates;
    iqHc.resize(_nGates);
    iqVc.resize(_nGates);
    iqHx.resize(_nGates);
    iqVx.resize(_nGates);
  }
  
}

// set the calibration

void Stats::setCalibration(double gainHc,
                           double gainVc,
                           double gainHx,
                           double gainVx)

{
  _rxGainHc = gainHc;
  _rxGainVc = gainVc;
  _rxGainHx = gainHx;
  _rxGainVx = gainVx;
}

////////////////////////////////////////////
// sum up summary information

void Stats::addToSummary(const IwrfTsPulse &pulse,
                         double ii0, double qq0,
                         bool haveChan1,
                         double ii1, double qq1)
  
{

  double power0 = ii0 * ii0 + qq0 * qq0;
  power0 /= _rxGainHc;

  sumPower0 += power0;
  nn0++;
  
  sumPowerHc += power0;
  nnH++;
  
  if (haveChan1) {

    double power1 = ii1 * ii1 + qq1 * qq1;
    power1 /= _rxGainVc;
    sumPower1 += power1;
    nn1++;

    RadarComplex_t c0, c1;
    c0.re = ii0;
    c0.im = qq0;
    c1.re = ii1;
    c1.im = qq1;
    RadarComplex_t prod = RadarComplex::conjugateProduct(c0, c1);

    sumConjProdH = RadarComplex::complexSum(sumConjProdH, prod);

    nnV++;
    sumPowerVc += power1;
    sumConjProdV = RadarComplex::complexSum(sumConjProdV, prod);
    
  } // if (haveChan1)
    
}

////////////////////////////////////////////
// sum up alternating information

void Stats::addToAlternating(const IwrfTsPulse &pulse,
                             double ii0, double qq0,
			     bool haveChan1,
                             double ii1, double qq1,
                             bool isHoriz)

{

  if (!haveChan1) {

    double power0 = ii0 * ii0 + qq0 * qq0;
    if (isHoriz) {
      power0 /= _rxGainHc;
    } else {
      power0 /= _rxGainVc;
    }
    sumPower0 += power0;
    nn0++;
    
    if (isHoriz) {
      nnH++;
      sumPowerHc += power0;
    } else {
      nnV++;
      sumPowerVc += power0;
    }

  } else {
    
    double power0 = ii0 * ii0 + qq0 * qq0;
    if (isHoriz) {
      power0 /= _rxGainHc;
    } else {
      power0 /= _rxGainVc;
    }
    sumPower0 += power0;
    nn0++;
    
    double power1 = ii1 * ii1 + qq1 * qq1;
    if (isHoriz) {
      power1 /= _rxGainHx;
    } else {
      power1 /= _rxGainVx;
    }
    sumPower1 += power1;
    nn1++;

    RadarComplex_t c0, c1;
    c0.re = ii0;
    c0.im = qq0;
    c1.re = ii1;
    c1.im = qq1;
    RadarComplex_t prod = RadarComplex::conjugateProduct(c0, c1);

    if (isHoriz) {
      nnH++;
      sumPowerHc += power0;
      sumPowerVx += power1;
      sumConjProdH = RadarComplex::complexSum(sumConjProdH, prod);
    } else {
      nnV++;
      sumPowerVc += power0;
      sumPowerHx += power1;
      sumConjProdV = RadarComplex::complexSum(sumConjProdV, prod);
    }

  }
    
}

////////////////////////////
// compute summary stats
// Assumes data has been added

void Stats::computeSummary(bool haveChan1)
  
{
  
  meanDbm0 = -999.9;
  meanDbm1 = -999.9;

  meanDbmHc = -999.9;
  meanDbmVc = -999.9;

  lag1DbmHc = -999.9;
  lag1DbmHx = -999.9;
  
  if (nn0 > 0) {
    double meanPower0 = sumPower0 / nn0;
    meanDbm0 = 10.0 * log10(meanPower0);
  }

  if (nn1 > 0) {
    double meanPower1 = sumPower1 / nn1;
    meanDbm1 = 10.0 * log10(meanPower1);
  }

  if (nnH > 0) {

    double meanPowerHc = sumPowerHc / nnH;
    meanDbmHc = 10.0 * log10(meanPowerHc);

    if (haveChan1) {
      
      double meanPowerVc = sumPowerVc / nnH;
      meanDbmVc = 10.0 * log10(meanPowerVc);
      
      double corrMagH = RadarComplex::mag(sumConjProdH) / nnH;
      corrH = corrMagH / sqrt(meanPowerHc * meanPowerVc);
      argH = RadarComplex::argDeg(sumConjProdH);

    }
  
  }
  
  if (nnV > 0) {

    double meanPowerVc = sumPowerVc / nnV;
    meanDbmVc = 10.0 * log10(meanPowerVc);

    if (haveChan1) {
      
      double meanPowerHc = sumPowerHc / nnV;
      meanDbmHc = 10.0 * log10(meanPowerHc);
      
      double corrMagV = RadarComplex::mag(sumConjProdV) / nnV;
      corrV = corrMagV / sqrt(meanPowerVc * meanPowerHc);
      argV = RadarComplex::argDeg(sumConjProdV);

    }
    
  }
  
  // compute lag1 powers if appropriate

  if (iqHc[0].size() > 0) {
    double sumCpHc = 0.0;
    for (int ii = 0; ii < _nGates; ii++) {
      RadarComplex_t cpHc =
        RadarComplex::meanConjugateProduct(iqHc[ii].data() + 1,
                                           iqHc[ii].data(),
                                           iqHc[ii].size() - 1);
      sumCpHc += RadarComplex::mag(cpHc);
    }
    lag1DbmHc = 10.0 * log10(sumCpHc / _nGates);
  }
  
  if (iqVc[0].size() > 0) {
    double sumCpVc = 0.0;
    for (int ii = 0; ii < _nGates; ii++) {
      RadarComplex_t cpVc =
        RadarComplex::meanConjugateProduct(iqVc[ii].data() + 1,
                                           iqVc[ii].data(),
                                           iqVc[ii].size() - 1);
      sumCpVc += RadarComplex::mag(cpVc);
    }
    lag1DbmVc = 10.0 * log10(sumCpVc / _nGates);
  }
  
}

////////////////////////////
// compute alternating stats
//
// Assumes data has been added

void Stats::computeAlternating(bool haveChan1)
  
{
  
  meanDbm0 = -999.9;
  meanDbm1 = -999.9;

  meanDbmHc = -999.9;
  meanDbmHx = -999.9;
  meanDbmVc = -999.9;
  meanDbmVx = -999.9;

  lag1DbmHc = -999.9;
  lag1DbmHx = -999.9;
  lag1DbmVc = -999.9;
  lag1DbmVx = -999.9;

  if (nn0 > 0) {
    double meanPower0 = sumPower0 / nn0;
    meanDbm0 = 10.0 * log10(meanPower0);
  }

  if (nn1 > 0) {
    double meanPower1 = sumPower1 / nn1;
    meanDbm1 = 10.0 * log10(meanPower1);
  }

  if (nnH > 0) {

    double meanPowerHc = sumPowerHc / nnH;
    meanDbmHc = 10.0 * log10(meanPowerHc);

    if (haveChan1) {
      
      double meanPowerVx = sumPowerVx / nnH;
      meanDbmVx = 10.0 * log10(meanPowerVx);
      
      double corrMagH = RadarComplex::mag(sumConjProdH) / nnH;
      corrH = corrMagH / sqrt(meanPowerHc * meanPowerVx);
      argH = RadarComplex::argDeg(sumConjProdH);

    }
  
  }
  
  if (nnV > 0) {

    double meanPowerVc = sumPowerVc / nnV;
    meanDbmVc = 10.0 * log10(meanPowerVc);

    if (haveChan1) {
      
      double meanPowerHx = sumPowerHx / nnV;
      meanDbmHx = 10.0 * log10(meanPowerHx);
      
      double corrMagV = RadarComplex::mag(sumConjProdV) / nnV;
      corrV = corrMagV / sqrt(meanPowerVc * meanPowerHx);
      argV = RadarComplex::argDeg(sumConjProdV);

    }
    
  }

  // compute lag1 powers if appropriate

  if (iqHc[0].size() > 0) {
    double sumCpHc = 0.0;
    for (int ii = 0; ii < _nGates; ii++) {
      RadarComplex_t cpHc =
        RadarComplex::meanConjugateProduct(iqHc[ii].data() + 1,
                                           iqHc[ii].data(),
                                           iqHc[ii].size() - 1);
      sumCpHc += RadarComplex::mag(cpHc);
    }
    lag1DbmHc = 10.0 * log10(sumCpHc / _nGates);
  }
  
  if (iqVc[0].size() > 0) {
    double sumCpVc = 0.0;
    for (int ii = 0; ii < _nGates; ii++) {
      RadarComplex_t cpVc =
        RadarComplex::meanConjugateProduct(iqVc[ii].data() + 1,
                                           iqVc[ii].data(),
                                           iqVc[ii].size() - 1);
      sumCpVc += RadarComplex::mag(cpVc);
    }
    lag1DbmVc = 10.0 * log10(sumCpVc / _nGates);
  }
  
  if (iqHx[0].size() > 0) {
    double sumCpHx = 0.0;
    for (int ii = 0; ii < _nGates; ii++) {
      RadarComplex_t cpHx =
        RadarComplex::meanConjugateProduct(iqHx[ii].data() + 1,
                                           iqHx[ii].data(),
                                           iqHx[ii].size() - 1);
      sumCpHx += RadarComplex::mag(cpHx);
    }
    lag1DbmHx = 10.0 * log10(sumCpHx / _nGates);
  }
  
  if (iqVx[0].size() > 0) {
    double sumCpVx = 0.0;
    for (int ii = 0; ii < _nGates; ii++) {
      RadarComplex_t cpVx =
        RadarComplex::meanConjugateProduct(iqVx[ii].data() + 1,
                                           iqVx[ii].data(),
                                           iqVx[ii].size() - 1);
      sumCpVx += RadarComplex::mag(cpVx);
    }
    lag1DbmVx = 10.0 * log10(sumCpVx / _nGates);
  }
  
}

