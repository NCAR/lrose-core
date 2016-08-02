// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:30:34 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
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

  sumConjProd01H.re = 0.0;
  sumConjProd01H.im = 0.0;
  sumConjProd01V.re = 0.0;
  sumConjProd01V.im = 0.0;

}

////////////////////////////////////////////
// sum up alternating information

void Stats::addToAlternating(double ii0, double qq0,
			     bool haveChan1,
                             double ii1, double qq1,
                             bool isHoriz)

{

  double power0 = ii0 * ii0 + qq0 * qq0;
  sumPower0 += power0;
  nn0++;
    
  if (haveChan1) {

    double power1 = ii1 * ii1 + qq1 * qq1;
    sumPower1 += power1;
    nn1++;

    RapComplex c0, c1;
    c0.re = ii0;
    c0.im = qq0;
    c1.re = ii1;
    c1.im = qq1;
    RapComplex prod01 = RapComplex::computeConjProduct(c0, c1);

    if (isHoriz) {
      nnH++;
      sumPowerHc += power0;
      sumPowerVx += power1;
      sumConjProd01H = RapComplex::computeSum(sumConjProd01H, prod01);
    } else {
      nnV++;
      sumPowerVc += power0;
      sumPowerHx += power1;
      sumConjProd01V = RapComplex::computeSum(sumConjProd01V, prod01);
    }

  }
    
}

////////////////////////////
// compute alternating stats
//
// Assumes data has been added

void Stats::computeAlternating()
  
{
  
  meanDbm0 = -999.9;
  meanDbm1 = -999.9;
  meanDbmHc = -999.9;
  meanDbmHx = -999.9;
  meanDbmVx = -999.9;
  meanDbmVc = -999.9;
  
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

    double meanPowerVx = sumPowerVx / nnH;
    meanDbmVx = 10.0 * log10(meanPowerVx);

    double corrMagH = RapComplex::computeMag(sumConjProd01H) / nnH;
    corr01H = corrMagH / sqrt(meanPowerHc * meanPowerVx);
    arg01H = RapComplex::computeArgDeg(sumConjProd01H);
  
  }
  
  if (nnV > 0) {

    double meanPowerHx = sumPowerHx / nnV;
    meanDbmHx = 10.0 * log10(meanPowerHx);

    double meanPowerVc = sumPowerVc / nnV;
    meanDbmVc = 10.0 * log10(meanPowerVc);

    double corrMagV = RapComplex::computeMag(sumConjProd01V) / nnV;
    corr01V = corrMagV / sqrt(meanPowerVc * meanPowerHx);
    arg01V = RapComplex::computeArgDeg(sumConjProd01V);
    
  }
  
}

