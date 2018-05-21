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
 * RapComplex.cc
 *
 * Complex arithmetic
 *
 * Mike Dixon, RAP, NCAR, Boulder CO
 *
 * July 2006
 *
 *********************************************************************/

#include <toolsa/toolsa_macros.h>
#include <rapmath/RapComplex.hh>
#include <rapmath/trig.h>
#include <iostream>

using namespace std;

// constructor

RapComplex::RapComplex()
{
  re = 0.0;
  im = 0.0;
}

RapComplex::RapComplex(double real, double imag)
{
  re = real;
  im = imag;
}

// destructor

RapComplex::~RapComplex()
{
}

// all other methods static

// compute complex product

RapComplex RapComplex::complexProduct(const RapComplex &c1,
                                      const RapComplex &c2)
  
{
  RapComplex product;
  product.re = (c1.re * c2.re) - (c1.im * c2.im);
  product.im = (c1.im * c2.re) + (c1.re * c2.im);
  return product;
}

// compute mean complex product of series

RapComplex RapComplex::meanProduct(const RapComplex *c1,
                                   const RapComplex *c2,
                                   int len)
  
{
  
  double sumRe = 0.0;
  double sumIm = 0.0;
  
  for (int ipos = 0; ipos < len; ipos++, c1++, c2++) {
    sumRe += ((c1->re * c2->re) - (c1->im * c2->im));
    sumIm += ((c1->im * c2->re) + (c1->re * c2->im));
  }

  RapComplex meanProduct;
  meanProduct.re = sumRe / len;
  meanProduct.im = sumIm / len;

  return meanProduct;

}

// compute conjugate product

RapComplex RapComplex::conjugateProduct(const RapComplex &c1,
					const RapComplex &c2)
  
{
  RapComplex product;
  product.re = (c1.re * c2.re) + (c1.im * c2.im);
  product.im = (c1.im * c2.re) - (c1.re * c2.im);
  return product;
}
  
// compute mean conjugate product of series

RapComplex RapComplex::meanConjugateProduct(const RapComplex *c1,
                                            const RapComplex *c2,
                                            int len)
  
{
  
  double sumRe = 0.0;
  double sumIm = 0.0;

  for (int ipos = 0; ipos < len; ipos++, c1++, c2++) {
    sumRe += ((c1->re * c2->re) + (c1->im * c2->im));
    sumIm += ((c1->im * c2->re) - (c1->re * c2->im));
  }

  RapComplex meanProduct;
  meanProduct.re = sumRe / len;
  meanProduct.im = sumIm / len;

  return meanProduct;

}

// compute sum

RapComplex RapComplex::complexSum(const RapComplex &c1,
				  const RapComplex &c2)
  
{
  RapComplex sum;
  sum.re = c1.re + c2.re;
  sum.im = c1.im + c2.im;
  return sum;
}

/////////////////////////////////////////
// mean of complex values

RapComplex RapComplex::complexMean(const RapComplex &c1, const RapComplex &c2)
  
{
  RapComplex mean;
  mean.re = (c1.re + c2.re) / 2.0;
  mean.im = (c1.im + c2.im) / 2.0;
  return mean;
}

 
// compute magnitude

double RapComplex::computeMag(const RapComplex &cc)
  
{
  return sqrt(cc.re * cc.re + cc.im * cc.im);
}
  
// compute arg in degrees

double RapComplex::computeArgDeg(const RapComplex &cc)
  
{
  double arg = 0.0;
  if (cc.re != 0.0 || cc.im != 0.0) {
    arg = atan2(cc.im, cc.re);
  }
  arg *= RAD_TO_DEG;
  return arg;
}

// compute arg in radians

double RapComplex::computeArgRad(const RapComplex &cc)
  
{
  double arg = 0.0;
  if (cc.re != 0.0 || cc.im != 0.0) {
    arg = atan2(cc.im, cc.re);
  }
  return arg;
}

// compute difference between two angles
// in degrees

double RapComplex::computeDiffDeg(double deg1, double deg2)
  
{
  
  double rad1 = deg1 * DEG_TO_RAD;
  double rad2 = deg2 * DEG_TO_RAD;
  
  RapComplex ang1, ang2;
  rap_sincos(rad1, &ang1.im, &ang1.re);
  rap_sincos(rad2, &ang2.im, &ang2.re);
  
  RapComplex conjProd = conjugateProduct(ang1, ang2);
  return computeArgDeg(conjProd);
  
}

// compute difference between two angles
// in radians

double RapComplex::computeDiffRad(double rad1, double rad2)
  
{
  
  RapComplex ang1, ang2;
  rap_sincos(rad1, &ang1.im, &ang1.re);
  rap_sincos(rad2, &ang2.im, &ang2.re);
  
  RapComplex conjProd = conjugateProduct(ang1, ang2);
  return computeArgRad(conjProd);
  
}

// compute sum of two angles in degrees

double RapComplex::computeSumDeg(double deg1, double deg2)
  
{
  
  double rad1 = deg1 * DEG_TO_RAD;
  double rad2 = deg2 * DEG_TO_RAD;
  
  RapComplex ang1, ang2;
  rap_sincos(rad1, &ang1.im, &ang1.re);
  rap_sincos(rad2, &ang2.im, &ang2.re);
  
  RapComplex compProd = complexProduct(ang1, ang2);
  double sum = computeArgDeg(compProd);
  return sum;
  
}

// compute sum between two angles
// in radians

double RapComplex::computeSumRad(double rad1, double rad2)
  
{
  
  RapComplex ang1, ang2;
  rap_sincos(rad1, &ang1.im, &ang1.re);
  rap_sincos(rad2, &ang2.im, &ang2.re);
  
  RapComplex compProd = complexProduct(ang1, ang2);
  double sum = computeArgRad(compProd);
  return sum;
  
}

// compute mean of two angles in degrees

double RapComplex::computeMeanDeg(double deg1, double deg2)
  
{

  double diff = computeDiffDeg(deg2, deg1);
  double mean = computeSumDeg(deg1, diff / 2.0);

  return mean;

}

// compute mean of two angles in radians

double RapComplex::computeMeanRad(double rad1, double rad2)
  
{

  double diff = computeDiffRad(rad2, rad1);
  double mean = computeSumRad(rad1, diff / 2.0);
  return mean;

}

