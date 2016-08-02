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
/*********************************************************************
 * RadxComplex.cc
 *
 * Complex arithmetic
 *
 * Mike Dixon, RAP, NCAR, Boulder CO
 *
 * July 2006
 *
 *********************************************************************/

#include <Radx/Radx.hh>
#include <Radx/RadxComplex.hh>
#include <iostream>
#include <cmath>

using namespace std;

// constructor

RadxComplex::RadxComplex()
{
  re = 0.0;
  im = 0.0;
}

RadxComplex::RadxComplex(double real, double imag)
{
  re = real;
  im = imag;
}

// destructor

RadxComplex::~RadxComplex()
{
}

// all other methods static

// compute complex product

RadxComplex RadxComplex::complexProduct(const RadxComplex &c1,
                                        const RadxComplex &c2)
  
{
  RadxComplex product;
  product.re = (c1.re * c2.re) - (c1.im * c2.im);
  product.im = (c1.im * c2.re) + (c1.re * c2.im);
  return product;
}

// compute mean complex product of series

RadxComplex RadxComplex::meanProduct(const RadxComplex *c1,
                                     const RadxComplex *c2,
                                     int len)
  
{
  
  double sumRe = 0.0;
  double sumIm = 0.0;
  
  for (int ipos = 0; ipos < len; ipos++, c1++, c2++) {
    sumRe += ((c1->re * c2->re) - (c1->im * c2->im));
    sumIm += ((c1->im * c2->re) + (c1->re * c2->im));
  }

  RadxComplex meanProduct;
  meanProduct.re = sumRe / len;
  meanProduct.im = sumIm / len;

  return meanProduct;

}

// compute conjugate product

RadxComplex RadxComplex::conjugateProduct(const RadxComplex &c1,
                                          const RadxComplex &c2)
  
{
  RadxComplex product;
  product.re = (c1.re * c2.re) + (c1.im * c2.im);
  product.im = (c1.im * c2.re) - (c1.re * c2.im);
  return product;
}
  
// compute mean conjugate product of series

RadxComplex RadxComplex::meanConjugateProduct(const RadxComplex *c1,
                                              const RadxComplex *c2,
                                              int len)
  
{
  
  double sumRe = 0.0;
  double sumIm = 0.0;

  for (int ipos = 0; ipos < len; ipos++, c1++, c2++) {
    sumRe += ((c1->re * c2->re) + (c1->im * c2->im));
    sumIm += ((c1->im * c2->re) - (c1->re * c2->im));
  }

  RadxComplex meanProduct;
  meanProduct.re = sumRe / len;
  meanProduct.im = sumIm / len;

  return meanProduct;

}

// compute sum

RadxComplex RadxComplex::complexSum(const RadxComplex &c1,
                                    const RadxComplex &c2)
  
{
  RadxComplex sum;
  sum.re = c1.re + c2.re;
  sum.im = c1.im + c2.im;
  return sum;
}

/////////////////////////////////////////
// mean of complex values

RadxComplex RadxComplex::complexMean(const RadxComplex &c1, const RadxComplex &c2)
  
{
  RadxComplex mean;
  mean.re = (c1.re + c2.re) / 2.0;
  mean.im = (c1.im + c2.im) / 2.0;
  return mean;
}

 
// compute magnitude

double RadxComplex::computeMag(const RadxComplex &cc)
  
{
  return sqrt(cc.re * cc.re + cc.im * cc.im);
}
  
// compute arg in degrees

double RadxComplex::computeArgDeg(const RadxComplex &cc)
  
{
  double arg = 0.0;
  if (cc.re != 0.0 || cc.im != 0.0) {
    arg = atan2(cc.im, cc.re);
  }
  arg *= Radx::RadToDeg;
  return arg;
}

// compute arg in radians

double RadxComplex::computeArgRad(const RadxComplex &cc)
  
{
  double arg = 0.0;
  if (cc.re != 0.0 || cc.im != 0.0) {
    arg = atan2(cc.im, cc.re);
  }
  return arg;
}

// compute difference between two angles
// in degrees

double RadxComplex::computeDiffDeg(double deg1, double deg2)
  
{
  
  double rad1 = deg1 * Radx::DegToRad;
  double rad2 = deg2 * Radx::DegToRad;
  
  RadxComplex ang1, ang2;
  sinCos(rad1, ang1.im, ang1.re);
  sinCos(rad2, ang2.im, ang2.re);
  
  RadxComplex conjProd = conjugateProduct(ang1, ang2);
  return computeArgDeg(conjProd);
  
}

// compute difference between two angles
// in radians

double RadxComplex::computeDiffRad(double rad1, double rad2)
  
{
  
  RadxComplex ang1, ang2;
  sinCos(rad1, ang1.im, ang1.re);
  sinCos(rad2, ang2.im, ang2.re);
  
  RadxComplex conjProd = conjugateProduct(ang1, ang2);
  return computeArgRad(conjProd);
  
}

// compute sum of two angles in degrees

double RadxComplex::computeSumDeg(double deg1, double deg2)
  
{
  
  double rad1 = deg1 * Radx::DegToRad;
  double rad2 = deg2 * Radx::DegToRad;
  
  RadxComplex ang1, ang2;
  sinCos(rad1, ang1.im, ang1.re);
  sinCos(rad2, ang2.im, ang2.re);
  
  RadxComplex compProd = complexProduct(ang1, ang2);
  double sum = computeArgDeg(compProd);
  return sum;
  
}

// compute sum between two angles
// in radians

double RadxComplex::computeSumRad(double rad1, double rad2)
  
{
  
  RadxComplex ang1, ang2;
  sinCos(rad1, ang1.im, ang1.re);
  sinCos(rad2, ang2.im, ang2.re);
  
  RadxComplex compProd = complexProduct(ang1, ang2);
  double sum = computeArgRad(compProd);
  return sum;
  
}

// compute mean of two angles in degrees

double RadxComplex::computeMeanDeg(double deg1, double deg2)
  
{

  double diff = computeDiffDeg(deg2, deg1);
  double mean = computeSumDeg(deg1, diff / 2.0);

  return mean;

}

// compute mean of two angles in radians

double RadxComplex::computeMeanRad(double rad1, double rad2)
  
{

  double diff = computeDiffRad(rad2, rad1);
  double mean = computeSumRad(rad1, diff / 2.0);
  return mean;

}

///////////////////////////////////////////////////
// compute sin and cos together for efficiency

void RadxComplex::sinCos(double radians, double &sinVal, double &cosVal)
  
{
  
  double cosv, sinv, interval;

  /* compute cosine */

  cosv = cos(radians);
  cosVal = cosv;

  /* compute sine magnitude */

  sinv = sqrt(1.0 - cosv * cosv);

  /* set sine sign from location relative to PI */

  interval = floor(radians / M_PI);
  if (fabs(fmod(interval, 2.0)) == 0) {
    sinVal = sinv;
  } else {
    sinVal = -1.0 * sinv;
  }

}

