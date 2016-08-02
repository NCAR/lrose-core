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
// Complex.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////

#ifndef Complex_hh
#define Complex_hh

typedef struct {
  double re;
  double im;
} Complex_t;

class Complex {

public:

  // all methods static
  
  // compute complex product
  
  static Complex_t complexProduct(const Complex_t &c1,
                                  const Complex_t &c2);
  
  static Complex_t meanComplexProduct(const Complex_t *c1,
                                      const Complex_t *c2,
                                      int len);
  // compute conjugate product
  
  static Complex_t conjugateProduct(const Complex_t &c1,
                                    const Complex_t &c2);
  
  // compute mean conjugate product of series
  
  static Complex_t meanConjugateProduct(const Complex_t *c1,
                                        const Complex_t *c2,
                                        int len);
  
  // compute sum

  static Complex_t complexSum(const Complex_t &c1,
                              const Complex_t &c2);
  
  // mean of complex values
  
  static Complex_t complexMean(Complex_t c1, Complex_t c2);

  // compute magnitude

  static double mag(const Complex_t &cc);
  
  // compute arg in degrees

  static double argDeg(const Complex_t &cc);
  

  // compute arg in radians

  static double argRad(const Complex_t &cc);
  

  // compute difference between two angles
  // in degrees

  static double diffDeg(double deg1, double deg2);
  
  // compute difference between two angles
  // in radians

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

  static double power(const Complex_t &cc);

  // compute mean power of series

  static double meanPower(const Complex_t *c1, int len);
  
};

#endif

