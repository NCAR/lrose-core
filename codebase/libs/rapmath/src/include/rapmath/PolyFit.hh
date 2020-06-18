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
// PolyFit.hh
//
// Polynomial fit to (x, y) data set
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2018
//
///////////////////////////////////////////////////////////////

#ifndef PolyFit_hh
#define PolyFit_hh

#include <rapmath/Distribution.hh>

class PolyFit {
  
public:
  
  // constructor

  PolyFit();
  
  // destructor
  
  virtual ~PolyFit();

  // set polynomial order

  void setOrder(size_t order);

  // clear the data values

  void clear();

  // add a data value
  
  void addValue(double xx, double yy);

  // set the data values
  
  void setValues(const vector<double> &xVals,
                 const vector<double> &yVals);

  // perform a fit
  // values must have been set
  
  virtual int performFit();
  
  // get order
  
  int getOrder() const { return _order; }

  // get number of values

  size_t getNVals() const { return _nObs; }
  
  // get coefficients after fit
  
  const vector<double> getCoeffs() const { return _coeffs; }
  
  // get single y value, given the x value

  double getYEst(double xx);

  // get single y value, given the index

  double getYEst(size_t index);

  // get vector of estimated y values

  vector<double> getYEst() const;

protected:
private:

  size_t _order;        // polynomial order
  size_t _orderPlus1;   // polynomial order plus 1

  vector<double> _coeffs; // coefficients
  
  vector<double> _xObs, _yObs; // observations
  size_t _nObs;

  double *_yEst; // regression estimate of y

  double **_vv;   // Vandermonde matrix
  double **_vvT;  // vv transpose
  double **_vvA;  // vvT * vv
  double **_vvB;  // vvT * vv - check
  double **_uu;   // from SVD of vvA: [uu, ss, wwT] = SVD(vvA)
  double **_uuT;  // uu transpose
  double *_ssVec; // from SVD of vvA - diagonal elements
  double **_ss;   // from SVD of vvA - diagonal matrix
  double **_ssInv; // from SVD of vvA - diagonal matrix
  double **_ww;   // from SVD of vvA
  double **_wwT;  // ww transpose
  
  double *_pp;   // regression coefficients - polynomial fit
  double **_cc;  // ww * diag * uuT * vvT
  double **_multa; // temporary matrix for intermediate results
  double **_multb; // temporary matrix for intermediate results

  mutable double _stdErrEst;
  
  // private methods

  void _doFit();
  
  void _init();

  void _allocDataArrays();
  void _allocPolyArrays();

  void _freeDataArrays();
  void _freePolyArrays();

  void _freeVec(double* &vec);
  void _freeMatrix(double** &array);

  void _computeCc();
  void _computeVandermonde();

  void _matrixMult(double **aa,
                   double **bb,
                   size_t nRowsAa,
                   size_t nColsAa,
                   size_t nColsBb,
                   double **xx) const;

  void _matrixVectorMult(double **aa,
                         double *bb,
                         size_t nRowsAa,
                         size_t nColsAa,
                         double *xx) const;

  void _matrixPrint(string name,
                    double **aa,
                    size_t nRowsAa,
                    size_t nColsAa,
                    FILE *out) const;
  
  void _vectorPrint(string name,
                    double *aa,
                    size_t sizeAa,
                    FILE *out) const;

};

#endif
