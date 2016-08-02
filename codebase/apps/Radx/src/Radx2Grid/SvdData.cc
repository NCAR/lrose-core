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
#include "SvdData.hh"
#include <rapmath/usvd.h>
#include <rapmath/RMmalloc.h>

SvdData::SvdData() : _ok(false)
{
  _nrow = 0;
  _ncol = 0;
  _a = NULL;
  _u = NULL;
  _w = NULL;
  _b = NULL;
  _x = NULL;
  _v = NULL;
}

SvdData::SvdData(const std::vector<ReorderInterp::radar_point_t> &pts,
		 double x0, double y0, double z0) : _ok(true)
{
  _alloc(pts, x0,  y0, z0);
  if (usvd(_a, _nrow, _ncol, _u, _v, _w) != 0) {
    cerr << "ERROR SvdData constructor - in call to usvd" << endl;
    _ok = false;
  }
}

SvdData::~SvdData(void)
{
  _free();
}

void SvdData::init(const std::vector<ReorderInterp::radar_point_t> &pts,
		   double x0, double y0, double z0)
{
  _free();
  _alloc(pts, x0,  y0, z0);
  if (usvd(_a, _nrow, _ncol, _u, _v, _w) != 0) {
    cerr << "ERROR SvdData::init - in call to usvd" << endl;
    _ok = false;
  } else {
    _ok = true;
  }
}

bool SvdData::compute(const std::vector<double> &b)
{
  if (static_cast<int>(b.size()) != _nrow) {
    cerr << "ERROR SvdData::compute" << endl;
    cerr << "Dimensions inconsistent " << b.size() << " " << _nrow << endl;
    return false;
  } else {
    for (int i=0; i<_nrow; ++i) {
      _b[i] = b[i];
    }
    _apply(_u, _w, _v, _nrow, _ncol, _b, _x);
    return true;
  }
}

bool SvdData::computeWithMissing(const std::vector<double> &b, double missing)
{
  if (static_cast<int>(b.size()) != _nrow) {
    cerr << "ERROR SvdData::computeWithMissing" << endl;
    cerr << "Dimensions inconsistent " << b.size() << " " << _nrow << endl;
    return false;
  } else {
    for (int i=0; i<_nrow; ++i) {
      _b[i] = b[i];
    }
    _apply(_u, _w, _v, _nrow, _ncol, _b, missing, _x);
    return true;
  }
}

double SvdData::getTerm(int index) const
{
  return _x[index];
}

void SvdData::print(void) const
{
  printf("A:\n");
  for (int i=0; i<_nrow; ++i) {
    for (int j=0; j<_ncol; ++j) {
      printf("%6.3lf ", _a[i][j]);
    }
    printf("\n");
  }

  printf("U:\n");
  for (int i=0; i<_nrow; ++i) {
    for (int j=0; j<_ncol; ++j) {
      printf("%6.3lf ", _u[i][j]);
    }
    printf("\n");
  }

  printf("V:\n");
  for (int i=0; i<_ncol; ++i) {
    for (int j=0; j<_ncol; ++j) {
      printf("%6.3lf ", _v[i][j]);
    }
    printf("\n");
  }

  printf("W:\n");
  for (int i=0; i<_ncol; ++i) {
    printf("%6.3lf ", _w[i]);
  }
  printf("\n");


  printf("B:\n");
  for (int i=0; i<_nrow; ++i) {
    printf("%6.3lf\n", _b[i]);
  }

  printf("X:\n");
  for (int i=0; i<_ncol; ++i) {
    printf("%6.3lf ", _x[i]);
  }
  printf("\n");
}


/************************************************************************

Function Name: 	usvd_apply

Description:	use the singular value decomposition of a matrix A 
                to solve A X = B for X

Returns:	none

Globals:	none

Notes: 		X = V [1 / w] ( Utranspose B )
   
                usvd decomposes A into U, V, and W.

************************************************************************/

void
SvdData::_apply(double** u, // I - U matrix from SVD decomposition
		double *w,  // I - W diag array from SVD decomposition
		double **v, // I - V matrix from SVD decomposition
		int ndata,  // I - number of rows in U
		int nvar,   // I - number of cols in U, order of V,W size
		double* b,  // I - known values to solve for
		double *x   // O - values solved for
		)
{
  double *utb;
  int idat, ivar, jvar;

  // first calculate Utranspose B
  utb = (double *) RMcalloc( nvar, sizeof(double) );  
  for ( ivar=0; ivar<nvar; ++ivar ) {
    utb[ivar] = 0.0;
    if ( w[ivar] != 0.0 ) {
      for ( idat=0; idat<ndata; ++idat )
	utb[ivar] += u[idat][ivar] * b[idat];

      // multiply by diagonal 1/w
      utb[ivar] /= w[ivar];
    }
  }

  // matrix multiply by V 
  for ( ivar=0; ivar<nvar; ++ivar ) {
    x[ivar] = 0.0;
    for ( jvar=0; jvar<nvar; ++jvar )
      x[ivar] += v[ivar][jvar] * utb[jvar];
  }
  
  RMfree( utb );
}

void
SvdData::_apply(double** u,	// I - U matrix from SVD decomposition
		double *w, 	// I - W diag array from SVD decomposition
		double **v,     // I - V matrix from SVD decomposition
		int ndata, 	// I - number of rows in U
		int nvar,	// I - number of cols in U, order of V, W size
		double* b, 	// I - known values to solve for
		double missing,
		double *x 	// O - values solved for
		)
{
  double *utb;
  int idat, ivar, jvar;

  // first calculate Utranspose B
  utb = (double *) RMcalloc( nvar, sizeof(double) );  
  for ( ivar=0; ivar<nvar; ++ivar )
  {
    utb[ivar] = 0.0;
    if ( w[ivar] != 0.0 )
    {
      for ( idat=0; idat<ndata; ++idat )
      {
	if (b[idat] != missing)
	{
	  utb[ivar] += u[idat][ivar] * b[idat];
	}
      }

      // multiply by diagonal 1/w 
      utb[ivar] /= w[ivar];
    }
  }

  // matrix multiply by V 
  for ( ivar=0; ivar<nvar; ++ivar )
  {
    x[ivar] = 0.0;
    for ( jvar=0; jvar<nvar; ++jvar )
      x[ivar] += v[ivar][jvar] * utb[jvar];
  }
  
  RMfree( utb );
}

void SvdData::_free(void)
{
  if (_nrow > 0 && _ncol > 0)
  {
    for (int i=0; i<_nrow; ++i)
    {
      delete [] _a[i];
      delete [] _u[i];
    }
  
    for (int i=0; i<_ncol; ++i)
    {
      delete [] _v[i];
    }

    delete [] _a;
    delete [] _u;
    delete [] _w;
    delete [] _b;
    delete [] _x;
    delete [] _v;
  }
}

void SvdData::_alloc(const std::vector<ReorderInterp::radar_point_t> &pts,
		     double x0, double y0, double z0)
{
  _nrow = static_cast<int>(pts.size());
  _ncol = 4;
  _a = new double * [_nrow];
  _u = new double * [_nrow];
  _w = new double[_ncol];
  _b = new double[_nrow];
  _x = new double[_ncol];
  for (int i=0; i<_nrow; ++i)
  {
    _a[i] = new double[_ncol];
    _u[i] = new double[_ncol];
    _b[i] = 0;
  }
  
  _v = new double * [_ncol];
  for (int i=0; i<_ncol; ++i)
  {
    _v[i] = new double[_ncol];
    _x[i] = 0;
  }

  for (int i=0; i<_nrow; ++i)
  {
    _a[i][0] = pts[i].xx - x0;
    _a[i][1] = pts[i].yy - y0;
    _a[i][2] = pts[i].zz - z0;
    _a[i][3] = 1.0;
  }
}
