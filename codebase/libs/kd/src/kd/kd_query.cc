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
//------------------------------------------------------------
// Module: nn.cc
//
// Author: Gerry Wiener
//
// Date:   6/6/00
//
// Description: Nearest neighbor interpolation
//------------------------------------------------------------

// Include files 
#include "../include/kd/kd.hh"
#include "../include/kd/metric.hh"
#include "../include/kd/kd_query.hh"

// Constant, macro and type definitions 
#define ML_INDXY(nx, ny, x, y) ((nx) * ((int)y) + ((int)x))

   
// Global variables 

// Functions and objects

Kd_query::~Kd_query()
{
  for (int k=0; k < _ndata; k++)
    delete [] _A[k];

  delete [] _A;
}

Kd_query::Kd_query
(
 KD_real *xdata,		// I - x coordinates of data points
 KD_real *ydata,		// I - y coordinates of data points
 int ndata,			// I - number of data points
 int metric = KD_EUCLIDEAN,	// I - type of metric 
 int minkP = 1		// I - on used with KD_L_P
 ) 
{
  _dim = 2;
  _ndata = ndata;
  _metric = metric;
  _minkP = minkP;
  _A = new KD_real*[_ndata];
  for (int k=0; k < _ndata; k++)
    {
      _A[k] = new KD_real[_dim];
      _A[k][0] = xdata[k];
      _A[k][1] = ydata[k];
    }
}


Kd_tree_query::~Kd_tree_query()
{
  delete _kdtp;
}



Kd_tree_query::Kd_tree_query
(
 KD_real *xdata,		// I - x coordinates of data points
 KD_real *ydata,		// I - y coordinates of data points
 int ndata,			// I - number of data points
 int metric = KD_EUCLIDEAN,	// I - type of metric 
 int minkP = 1		// I - on used with KD_L_P
 ) :   Kd_query(xdata, ydata, ndata, metric, minkP)
{
  // Create KD tree
  _kdtp = new KD_tree((const KD_real **)_A, _ndata, _dim);
}

// Find nn_num nearest neighbors of the given query_pt. Store the
// indices of these neighbors in found and their distances from the
// query point in dist
int Kd_tree_query::nnquery
(
 KD_real query_pt[2],		// I - query point
 int nn_num,			// I - number of nearest neighbors desired
 int *found,			// O - array of size at least nn_num
 KD_real *dist			// O - array of size at least nn_num
)
{
  // Get nearest neighbor of query points of grid
  _kdtp->nnquery(query_pt, nn_num, _metric, _minkP, found, dist);
  if (nn_num > KD_BUCKETSIZE)
    return(KD_BUCKETSIZE);
  return(nn_num);
}


void Kd_tree_query::rectquery
(
 const KD_real **RectQuery,
 vector<int> &pts_found
 )
{
  _kdtp->rectquery(RectQuery, pts_found);
}


Kd_naive_query::~Kd_naive_query()
{
  delete _kdnp;
}


Kd_naive_query::Kd_naive_query
(
 KD_real *xdata,		// I - x coordinates of data points
 KD_real *ydata,		// I - y coordinates of data points
 int ndata,			// I - number of data points
 int metric = KD_EUCLIDEAN,	// I - type of metric 
 int minkP = 1		// I - on used with KD_L_P
 ) :   Kd_query(xdata, ydata, ndata, metric, minkP)
{
  // Create KD tree
  _kdnp = new KD_naive((const KD_real **)_A, _ndata, _dim);
}

// Find nn_num nearest neighbors of the given query_pt. Store the
// indices of these neighbors in found and their distances from the
// query point in dist
int Kd_naive_query::nnquery
(
 KD_real query_pt[2],		// I - query point
 int nn_num,			// I - number of nearest neighbors desired
 int *found,			// O - array of size at least nn_num
 KD_real *dist			// O - array of size at least nn_num
)
{
  // Get nearest neighbor of query points of grid
  _kdnp->nnquery(query_pt, nn_num, _metric, _minkP, found, dist);
  return(nn_num);
}


void Kd_naive_query::rectquery
(
 const KD_real **RectQuery,
 vector<int> &pts_found
 )
{
  _kdnp->rectquery(RectQuery, pts_found);
}

