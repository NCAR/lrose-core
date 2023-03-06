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
/********************************************************
* Approximation of a discrete real function F(x) by     *
* least squares                                         *
* ----------------------------------------------------- *
* Ref.: "Méthodes de calcul numérique, Tome 2 by Claude *
*        Nowakowski, PSI Edition, 1984" [BIBLI 04].     *
* ----------------------------------------------------- *
* C++ version by J-P Moreau, Paris.                     *
* (www.jpmoreau.fr)                                     *
********************************************************/
/////////////////////////////////////////////////////////////
// ForsytheFit.hh
//
// Regression fit to (x, y) data set using Forsythe polynomials
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2020
//
///////////////////////////////////////////////////////////////

#ifndef ForsytheFit_hh
#define ForsytheFit_hh

#include <string>
#include <vector>
#include <rapmath/stats.h>
#include <cmath>
#include <cstdio>
using namespace std;

class ForsytheFit {
  
public:
  
  // constructor

  ForsytheFit();
  
  // destructor
  
  virtual ~ForsytheFit();

  // clear the data values

  void clear();

  // perform a fit
  // values must have been set
  
  int performFit(size_t order,
                 const vector<double> &xVals,
                 const vector<double> &yVals);

  // Prepare for a fit, specifying the X values.
  // This is done for efficiency, if the X values do not change.
  
  void prepareForFit(size_t order, const vector<double> &xObs);

  // prepare for fit, specifying nSamples, fixed PRT
  
  void prepareForFitFixedPrt(size_t order, size_t nSamples);

  // Prepare for a fit, specifying nSamples, staggered PRT
  
  void prepareForFitStaggeredPrt(size_t order, size_t nSamples,
                                 int staggeredM, int staggeredN);

  // perform the fit
  
  int performFit(const vector<double> &yObs);

  // get order
  
  int getOrder() const { return _order; }

  // get number of values

  size_t getNVals() const { return _nObs; }
  
  // get coefficients after fit
  
  const vector<double> getCoeffs() const { return _coeffs; }
  
  // get single y value, given the x value
  
  double getYEst(double xx);

  // get the full vector of estimated Y values
  
  const vector<double> &getYEstVector();

  // compute standard error of estimate for the fit
  
  double computeStdErrEst(double &rSquared);

#ifdef WITH_FORTRAN
  virtual int performFitFortran();
#endif
  
protected:
private:

  bool _prepActive;

  size_t _order;        // polynomial order

  vector<double> _coeffs; // polynomial coefficients (0-based)
  
  vector<double> _xObs, _yObs; // observations
  size_t _nObs; // number of obs
  
  vector<double> _yEst; // regression estimate of y, size _nObs

  // arrays for the fitting procedure

  vector<double> _aa, _bb, _ff, _cc; // size _order + 2 (1-based)
  vector<double> _dd, _ee; // size _nObs (0-based)
  vector<vector<double> > _bbSave, _eeSave; // if prepareForFit is used

  vector<vector<double> > _xPowers; // polynomial powers of x [_nObs][_order+1]

  // private methods

  void _allocDataArrays();
  void _allocPolyArrays();

};

#endif
