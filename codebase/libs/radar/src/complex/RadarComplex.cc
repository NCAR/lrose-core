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
 * RadarComplex.cc
 *
 * RadarComplex arithmetic
 *
 * Mike Dixon, RAP, NCAR, Boulder CO
 *
 * April 2007
 *
 *********************************************************************/

#include <radar/RadarComplex.hh>
#include <toolsa/sincos.h>
#include <cmath>
#include <iomanip>

#ifndef RAD_TO_DEG
#define RAD_TO_DEG 57.29577951308092
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.01745329251994372
#endif

using namespace std;

// construct from angle in radians / degrees
RadarComplex_t::RadarComplex_t(double angle, bool isDegrees /* = false*/)
{
  if (isDegrees) {
    RadarComplex::setFromDegrees(angle, *this);
  } else {
    RadarComplex::setFromRadians(angle, *this);
  }
}

// all methods static

// set complex from degrees

void RadarComplex::setFromDegrees(double degrees, RadarComplex_t &comp)
  
{
  double sinVal, cosVal;
  ta_sincos(degrees * DEG_TO_RAD, &sinVal, &cosVal);
  comp.re = cosVal;
  comp.im = sinVal;
}

// set complex from degrees

void RadarComplex::setFromRadians(double radians, RadarComplex_t &comp)
  
{
  double sinVal, cosVal;
  ta_sincos(radians, &sinVal, &cosVal);
  comp.re = cosVal;
  comp.im = sinVal;
}

// compute complex product

RadarComplex_t RadarComplex::complexProduct(const RadarComplex_t &c1,
                                            const RadarComplex_t &c2)
  
{
  RadarComplex_t product;
  product.re = (c1.re * c2.re) - (c1.im * c2.im);
  product.im = (c1.im * c2.re) + (c1.re * c2.im);
  return product;
}

// normalized complex product - has magnitude of 1.0

RadarComplex_t RadarComplex::normComplexProduct(const RadarComplex_t &c1,
                                                const RadarComplex_t &c2)
  
{
  RadarComplex_t product;
  product.re = (c1.re * c2.re) - (c1.im * c2.im);
  product.im = (c1.im * c2.re) + (c1.re * c2.im);
  double pmag = mag(product);
  product.re /= pmag;
  product.im /= pmag;
  return product;
}

// compute mean complex product of series

RadarComplex_t RadarComplex::meanComplexProduct(const RadarComplex_t *c1,
                                                const RadarComplex_t *c2,
                                                int len)
  
{
  
  double sumRe = 0.0;
  double sumIm = 0.0;
  
  for (int ipos = 0; ipos < len; ipos++, c1++, c2++) {
    sumRe += ((c1->re * c2->re) - (c1->im * c2->im));
    sumIm += ((c1->im * c2->re) + (c1->re * c2->im));
  }

  RadarComplex_t meanProduct;
  meanProduct.re = sumRe / len;
  meanProduct.im = sumIm / len;

  return meanProduct;

}

// compute complex quotient
// i.e. c1 / c2

RadarComplex_t RadarComplex::complexQuotient(const RadarComplex_t &c1,
                                             const RadarComplex_t &c2)
  
{
  RadarComplex_t quotient;
  double num_real = (c1.re * c2.re) + (c1.im * c2.im);
  double num_imag = (c1.im * c2.re) - (c1.re * c2.im);
  double denom = (c2.re * c2.re) + (c2.im * c2.im);
  quotient.re = num_real / denom;
  quotient.im = num_imag / denom;
  return quotient;
}

// compute mean complex quotient of series

RadarComplex_t RadarComplex::meanComplexQuotient(const RadarComplex_t *c1,
                                                 const RadarComplex_t *c2,
                                                 int len)
  
{
  
  double sumRe = 0.0;
  double sumIm = 0.0;
  
  for (int ipos = 0; ipos < len; ipos++, c1++, c2++) {
    double num_real = (c1->re * c2->re) + (c1->im * c2->im);
    double num_imag = (c1->im * c2->re) - (c1->re * c2->im);
    double denom = (c2->re * c2->re) + (c2->im * c2->im);
    sumRe += (num_real / denom);
    sumIm += (num_imag / denom);
  }
  
  RadarComplex_t meanQuotient;
  meanQuotient.re = sumRe / len;
  meanQuotient.im = sumIm / len;

  return meanQuotient;

}

// compute conjugate product
// This is effectively c1 - c2

RadarComplex_t RadarComplex::conjugateProduct(const RadarComplex_t &c1,
                                              const RadarComplex_t &c2)
  
{
  RadarComplex_t product;
  product.re = (c1.re * c2.re) + (c1.im * c2.im);
  product.im = (c1.im * c2.re) - (c1.re * c2.im);
  return product;
}
  
// normalized conjugate product - has magnitude of 1.0

RadarComplex_t RadarComplex::normConjugateProduct(const RadarComplex_t &c1,
                                                  const RadarComplex_t &c2)
  
{
  RadarComplex_t product;
  product.re = (c1.re * c2.re) + (c1.im * c2.im);
  product.im = (c1.im * c2.re) - (c1.re * c2.im);
  double pmag = mag(product);
  product.re /= pmag;
  product.im /= pmag;
  return product;
}

// compute mean conjugate product of series

RadarComplex_t RadarComplex::meanConjugateProduct(const RadarComplex_t *c1,
                                                  const RadarComplex_t *c2,
                                                  int len)
  
{
  
  double sumRe = 0.0;
  double sumIm = 0.0;

  for (int ipos = 0; ipos < len; ipos++, c1++, c2++) {
    sumRe += ((c1->re * c2->re) + (c1->im * c2->im));
    sumIm += ((c1->im * c2->re) - (c1->re * c2->im));
  }

  RadarComplex_t meanProduct;
  meanProduct.re = sumRe / len;
  meanProduct.im = sumIm / len;

  return meanProduct;

}

// compute sum

RadarComplex_t RadarComplex::complexSum(const RadarComplex_t &c1,
                                        const RadarComplex_t &c2)
  
{
  RadarComplex_t sum;
  sum.re = c1.re + c2.re;
  sum.im = c1.im + c2.im;
  return sum;
}
 
/////////////////////////////////////////
// mean of complex values

RadarComplex_t RadarComplex::complexMean(RadarComplex_t c1, RadarComplex_t c2)

{
  RadarComplex_t mean;
  mean.re = (c1.re + c2.re) / 2.0;
  mean.im = (c1.im + c2.im) / 2.0;
  return mean;
}

RadarComplex_t RadarComplex::mean(RadarComplex_t sum, double nn)

{
  RadarComplex_t mean;
  mean.re = sum.re / nn;
  mean.im = sum.im / nn;
  return mean;
}

// normalize in place - set magnitude to 1.0

void RadarComplex::normalize(RadarComplex_t &cc)
  
{
  double cmag = mag(cc);
  cc.re /= cmag;
  cc.im /= cmag;
}

// normalize - set magnitude to 1.0

RadarComplex_t RadarComplex::norm(const RadarComplex_t &cc)
  
{
  RadarComplex_t norm;
  double cmag = mag(cc);
  norm.re = cc.re / cmag;
  norm.im = cc.im / cmag;
  return norm;
}

// compute magnitude

double RadarComplex::mag(const RadarComplex_t &cc)
  
{
  return sqrt(cc.re * cc.re + cc.im * cc.im);
}
  
// compute arg in degrees

double RadarComplex::argDeg(const RadarComplex_t &cc)
  
{
  double arg = 0.0;
  if (cc.re != 0.0 || cc.im != 0.0) {
    arg = atan2(cc.im, cc.re);
  }
  arg *= RAD_TO_DEG;
  return arg;
}

// compute arg in radians

double RadarComplex::argRad(const RadarComplex_t &cc)
  
{
  double arg = 0.0;
  if (cc.re != 0.0 || cc.im != 0.0) {
    arg = atan2(cc.im, cc.re);
  }
  return arg;
}

// compute difference between two angles
// in degrees.  Computes (deg1 - deg2)

double RadarComplex::diffDeg(double deg1, double deg2)
  
{
  
  RadarComplex_t ang1(deg1, true);
  RadarComplex_t ang2(deg2, true);

  RadarComplex_t conjProd = conjugateProduct(ang1, ang2);
  return argDeg(conjProd);
  
}

// compute difference between two angles
// in radians. Computes (rad1 - rad2)

double RadarComplex::diffRad(double rad1, double rad2)
  
{
  
  RadarComplex_t ang1(rad1);
  RadarComplex_t ang2(rad2);

  RadarComplex_t conjProd = conjugateProduct(ang1, ang2);
  return argRad(conjProd);
  
}

// compute sum of two angles in degrees

double RadarComplex::sumDeg(double deg1, double deg2)
  
{
  
  RadarComplex_t ang1(deg1, true);
  RadarComplex_t ang2(deg2, true);

  RadarComplex_t compProd = complexProduct(ang1, ang2);
  double sum = argDeg(compProd);
  return sum;
  
}

// compute sum between two angles
// in radians

double RadarComplex::sumRad(double rad1, double rad2)
  
{
  
  RadarComplex_t ang1(rad1);
  RadarComplex_t ang2(rad2);

  RadarComplex_t compProd = complexProduct(ang1, ang2);
  double sum = argRad(compProd);
  return sum;
  
}

// compute mean of two angles in degrees

double RadarComplex::meanDeg(double deg1, double deg2)
  
{

  double diff = diffDeg(deg2, deg1);
  double mean = sumDeg(deg1, diff / 2.0);

  return mean;

}

// compute mean of two angles in radians

double RadarComplex::meanRad(double rad1, double rad2)
  
{

  double diff = diffRad(rad2, rad1);
  double mean = sumRad(rad1, diff / 2.0);
  return mean;

}

//////////////////////////////////////
// compute power

double RadarComplex::power(const RadarComplex_t &cc)
  
{
  return cc.re * cc.re + cc.im * cc.im;
}

/////////////////////////////////////////////
// compute mean power of series

double RadarComplex::meanPower(const RadarComplex_t *c1, int len)
  
{
  if (len < 1) {
    return 0.0;
  }
  double sum = 0.0;
  for (int ipos = 0; ipos < len; ipos++, c1++) {
    sum += ((c1->re * c1->re) + (c1->im * c1->im));
  }
  return sum / len;
}

double RadarComplex::meanPower(const double *pwr, int len)
  
{
  if (len < 1) {
    return 0.0;
  }
  double sum = 0.0;
  for (int ipos = 0; ipos < len; ipos++, pwr++) {
    sum += *pwr;
  }
  return sum / len;
}

////////////////////////////////////////
// load power from complex

void RadarComplex::loadPower(const RadarComplex_t *in, double *power, int len)

{
  
  for (int ii = 0; ii < len; ii++, in++, power++) {
    *power = in->re * in->re + in->im * in->im;
  }

}

////////////////////////////////////////
// load magnitudes from complex

void RadarComplex::loadMag(const RadarComplex_t *in, double *mag, int len)

{
  
  for (int ii = 0; ii < len; ii++, in++, mag++) {
    *mag = sqrt(in->re * in->re + in->im * in->im);
  }

}

///////////////////////////////
// print IQ time series
//
// 3 columns are printed: n, i, q
// An optional header is printed, labelling the columns

void RadarComplex::printIq(ostream &out,
                           int nSamples,
                           const RadarComplex_t *iq,
                           bool printHeading,
                           const string &heading)
  
{

  if (printHeading) {
    out << "---->> " << heading << " <<----" << endl;
    out << setw(5) << "n" << "  "
        << setw(10) << "I" << "  "
        << setw(10) << "Q" << endl;
  }
  for (int ii = 0; ii < nSamples; ii++) {
    out << setw(5) << ii << "  "
	<< setw(10) << iq[ii].re << "  "
	<< setw(10) << iq[ii].im << endl;
  }
  out.flush();

}

///////////////////////////////
// print complex compoments
//
// 3 columns are printed: n, re, im
// An optional header is printed, labelling the columns

void RadarComplex::printComplex(ostream &out,
                                int nSamples,
                                const RadarComplex_t *comp,
                                bool printHeading,
                                const string &heading,
                                bool reCenter /* = false */ )
  
{
  
  if (printHeading) {
    out << "---->> " << heading << " <<----" << endl;
    out << setw(5) << "n" << "  "
        << setw(10) << "re" << "  "
        << setw(10) << "im" << endl;
  }
  for (int ii = 0; ii < nSamples; ii++) {
    int jj = ii;
    if (reCenter) {
      jj += nSamples / 2;
      jj %= nSamples;
    }
    out << setw(5) << jj << "  "
	<< setw(10) << comp[jj].re << "  "
	<< setw(10) << comp[jj].im << endl;
  }
  out.flush();

}

///////////////////////////////
// print complex vector
//
// 3 columns are printed: n, mag, phase
// An optional header is printed, labelling the columns

void RadarComplex::printVector(ostream &out,
                               int nSamples,
                               const RadarComplex_t *comp,
                               bool printHeading,
                               const string &heading,
                               bool reCenter /* = false */ )
  
{
  
  if (printHeading) {
    out << "---->> " << heading << " <<----" << endl;
    out << setw(5) << "n" << "  "
        << setw(10) << "mag" << "  "
        << setw(10) << "phase" << endl;
  }
  for (int ii = 0; ii < nSamples; ii++) {
    int jj = ii;
    if (reCenter) {
      jj += nSamples / 2;
      jj %= nSamples;
    }
    double magn = mag(comp[jj]);
    double angle = argDeg(comp[jj]);
    out << setw(5) << ii << "  "
	<< setw(10) << magn << "  "
	<< setw(10) << angle << endl;
  }
  out.flush();

}

