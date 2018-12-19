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
// DistPolynomial.hh
//
// Polynomial-based distribution
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2018
//
///////////////////////////////////////////////////////////////

#ifndef DistPolynomial_hh
#define DistPolynomial_hh

#include <rapmath/Distribution.hh>

/////////////////////////////////////
// Normal distribution
// Derived class.

class DistPolynomial : public Distribution {
  
public:
  
  // constructor

  DistPolynomial();
  
  // destructor
  
  virtual ~DistPolynomial();

  // set polynomial order

  void setOrder(size_t order);

  // perform a fit
  // values must have been set
  
  virtual int performFit();
  
  // get the pdf for a given x

  virtual double getPdf(double xx);
  
  // get the cdf for a given x
  
  virtual double getCdf(double xx);
  
  // get order
  
  int getOrder() const { return _nPoly; }
  
  // get coefficients

  const vector<double> &getCoeffs() const { return _coeffs; }

protected:
private:

  size_t _nPoly;    // polynomial order
  size_t _nPoly1;   // polynomial order plus 1

  vector<double> _coeffs; // coefficients

  size_t _minValidIndex; // below this the PDF is not valid
  size_t _maxValidIndex; // above this the PDF is not valid

  double _minValidX; // if x is less than this is it set to 0
  double _maxValidX; // if x is greater than this it is set to 0
  
  double *_xx;    // x vector - observed
  double *_yyEst; // regression estimate of y

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

  virtual void _clearStats();

  void _init();
  void _alloc();
  void _free();
  void _doPolyFit();

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
