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
// RadarComplex.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////

#ifndef RadarComplex_hh
#define RadarComplex_hh

#include <iostream>
#include <cmath>
using namespace std;

// data class for use by main class

class RadarComplex_t {
public:
  // default constructor - magnitude of 1, phase of 0
  RadarComplex_t() :
          re(1.0),
          im(0.0) {}
  // constructor with values
  RadarComplex_t(double re_, double im_) :
          re(re_),
          im(im_) {}
  // construct from angle in radians / degrees
  // angle is in degrees if isDegrees is true
  // otherwise it is in radians
  RadarComplex_t(double angle, bool isDegrees = false);
  // set method
  void set(double re_, double im_) {
    re = re_;
    im = im_;
  }
  // data
  double re;
  double im;
};

class RadarComplex {

public:

  // all methods static
  
  // set complex from degrees
  
  static void setFromDegrees(double degrees, RadarComplex_t &comp);

  // set complex from degrees

  static void setFromRadians(double radians, RadarComplex_t &comp);
  
  // compute complex product
  
  static RadarComplex_t complexProduct(const RadarComplex_t &c1,
                                       const RadarComplex_t &c2);
  
  // normalized complex product - has magnitude of 1.0
  
  static RadarComplex_t normComplexProduct(const RadarComplex_t &c1,
                                           const RadarComplex_t &c2);

  // compute mean complex product of series
  
  static RadarComplex_t meanComplexProduct(const RadarComplex_t *c1,
                                           const RadarComplex_t *c2,
                                           int len);

  // compute complex quotient
  
  static RadarComplex_t complexQuotient(const RadarComplex_t &c1,
                                        const RadarComplex_t &c2);

  // compute mean complex quotient of series
  
  static RadarComplex_t meanComplexQuotient(const RadarComplex_t *c1,
                                            const RadarComplex_t *c2,
                                            int len);
  
  // compute conjugate product
  // This is effectively c1 - c2

  static RadarComplex_t conjugateProduct(const RadarComplex_t &c1,
                                         const RadarComplex_t &c2);
  // normalized conjugate product - has magnitude of 1.0
  
  static RadarComplex_t normConjugateProduct(const RadarComplex_t &c1,
                                             const RadarComplex_t &c2);
  
  // compute mean conjugate product of series
  
  static RadarComplex_t meanConjugateProduct(const RadarComplex_t *c1,
                                             const RadarComplex_t *c2,
                                             int len);
  
  // compute sum
  
  static RadarComplex_t complexSum(const RadarComplex_t &c1,
                                   const RadarComplex_t &c2);
  
  // mean of complex values
  
  static RadarComplex_t complexMean(RadarComplex_t c1, RadarComplex_t c2);
  
  // compute mean by dividing by the number of obs
  
  static RadarComplex_t mean(RadarComplex_t sum, double nn);
  
  // normalize - set magnitude to 1.0
  
  static void normalize(RadarComplex_t &cc); // in place
  static RadarComplex_t norm(const RadarComplex_t &cc);

  // compute magnitude
  
  static double mag(const RadarComplex_t &cc);
  
  // compute arg in degrees
  
  static double argDeg(const RadarComplex_t &cc);
  

  // compute arg in radians

  static double argRad(const RadarComplex_t &cc);
  

  // compute difference between two angles
  // in degrees.  Computes (deg1 - deg2)

  static double diffDeg(double deg1, double deg2);
  
  // compute difference between two angles
  // in radians. Computes (rad1 - rad2)

  static double diffRad(double rad1, double rad2);
  
  // compute sum of two angles in degrees

  static double sumDeg(double deg1, double deg2);
  
  // compute sum between two angles
  // in radians

  static double sumRad(double rad1, double rad2);
  
  // compute mean of two angles in degrees

  static double meanDeg(double deg1, double deg2);
  
  // compute mean of two angles in radians

  static double meanRad(double rad1, double rad2);

  // compute power

  static double power(const RadarComplex_t &cc);

  // compute mean power of series

  static double meanPower(const RadarComplex_t *c1, int len);
  static double meanPower(const double *pwr, int len);
   
  // load power from complex
  
  static void loadPower(const RadarComplex_t *in, double *power, int len);

  // load magnitudes from complex
  
  static void loadMag(const RadarComplex_t *in, double *mag, int len);

  // print IQ time series
  // 3 columns are printed: n, i, q
  // An optional header is printed, labelling the columns

  static void printIq(ostream &out,
                      int nSamples,
                      const RadarComplex_t *iq,
                      bool printHeading,
                      const string &heading);
  
  // print complex compoments
  // 3 columns are printed: n, re, im
  // An optional header is printed, labelling the columns

  static void printComplex(ostream &out,
                           int nSamples,
                           const RadarComplex_t *comp,
                           bool printHeading,
                           const string &heading,
                           bool reCenter = false);

  // print complex vector
  // 3 columns are printed: n, mag, phase
  // An optional header is printed, labelling the columns
  
  static void printVector(ostream &out,
                          int nSamples,
                          const RadarComplex_t *comp,
                          bool printHeading,
                          const string &heading,
                          bool reCenter = false);

private:

};

#endif

