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
// RadxComplex.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////
//
// Complex arithmetic
//
///////////////////////////////////////////////////////////////

#ifndef RadxComplex_hh
#define RadxComplex_hh

class RadxComplex {

public:

  RadxComplex();
  RadxComplex(double real, double imag);

  ~RadxComplex();

  // data members
  
  double re;
  double im;

  // static functions

  // compute complex product
  
  static RadxComplex complexProduct(const RadxComplex &c1,
                                    const RadxComplex &c2);
  
  // compute mean complex product of series
  
  static RadxComplex meanProduct(const RadxComplex *c1,
                                 const RadxComplex *c2,
                                 int len);
  
  // compute conjugate product
  
  static RadxComplex conjugateProduct(const RadxComplex &c1,
                                      const RadxComplex &c2);
  
  // compute mean conjugate product of series
  
  static RadxComplex meanConjugateProduct(const RadxComplex *c1,
                                          const RadxComplex *c2,
                                          int len);

  // compute sum
  
  static RadxComplex complexSum(const RadxComplex &c1,
                                const RadxComplex &c2);
  
  // mean of complex values
  
  RadxComplex complexMean(const RadxComplex &c1, const RadxComplex &c2);
  
  // compute magnitude
  
  static double computeMag(const RadxComplex &cc);
  
  // compute arg in degrees
  
  static double computeArgDeg(const RadxComplex &cc);
  
  // compute arg in radians

  static double computeArgRad(const RadxComplex &cc);
  
  // compute difference between two angles
  // in degrees

  static double computeDiffDeg(double deg1, double deg2);

  // compute difference between two angles
  // in radians
  
  static double computeDiffRad(double rad1, double rad2);
  
  // compute sum of two angles in degrees
  
  static double computeSumDeg(double deg1, double deg2);

  // compute sum between two angles
  // in radians
  
  static double computeSumRad(double rad1, double rad2);
  
  // compute mean of two angles in degrees

  static double computeMeanDeg(double deg1, double deg2);
  
  // compute mean of two angles in radians

  static double computeMeanRad(double rad1, double rad2);
  
  // compute sin and cos together for efficiency
  
  static void sinCos(double radians, double &sinVal, double &cosVal);
  
};

#endif

