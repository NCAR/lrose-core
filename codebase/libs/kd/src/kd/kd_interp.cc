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
// Module: kd_interp.cc
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
#include "../include/kd/kd_interp.hh"
#include <algorithm>
#include <math.h>

// Constant, macro and type definitions 
#define ML_INDXY(nx, ny, x, y) ((nx) * ((int)y) + ((int)x))

   
// Global variables 

// Functions and objects


// Nearest neighbor interpolation for two dimensions.  Note that there
// are xdim * ydim grid points in the cartesian product
// determined by xgrid and ygrid.
void kd_interp
(
 KD_real *xdata,		// I - x coordinates of data points
 KD_real *ydata,		// I - y coordinates of data points
 int ndata,			// I - number of data points
 KD_real *data_val,		// I - values at data points
 KD_real *xgrid,		// I - x coordinates of interp grid
 KD_real *ygrid,		// I - y coordinates of interp grid
 int xdim,			// I - size of xgrid
 int ydim,			// I - size of ygrid
 KD_real *grid_val		// O - interp grid values (should have dimension xdim * ydim)
)
{
  KD_real **A;
  int Metric = KD_EUCLIDEAN;
  int MinkP = 1;
  const int dim = 2;
  const int nn_num = 3;			// number of nearest neighbors to search for
  int found[nn_num];
  KD_real dist[nn_num];
  KD_real querpoint[2];
  
  A = new KD_real*[ndata];
  for (int k=0; k < ndata; k++)
    {
      A[k] = new KD_real[dim];
      A[k][0] = xdata[k];
      A[k][1] = ydata[k];
    }

  // Create KD tree
  KD_tree kdt((const KD_real **)A, ndata, dim);
  
  // Get nearest neighbor of query points of grid
  int ct = 0;
  for (int j=0; j<ydim; j++)
    {
      for (int i=0; i<xdim; i++)
	{
	  querpoint[0] = xgrid[i];
	  querpoint[1] = ygrid[j];
	  kdt.nnquery(querpoint, nn_num, Metric, MinkP, found, dist);

	  grid_val[ct] = data_val[found[0]];
	  ct++;
	}
    }

  for (int k=0; k < ndata; k++)
    delete [] A[k];

  delete [] A;
}

// Rectangular nearest neighbor interpolation for two dimensions.  For
// each grid point, determine the set N of neighbors that are within
// dist_crit of the grid point.  Set the value of the grid point to
// either the min or max value of the neighbors in N.  Note that there
// are xdim * ydim grid points in the cartesian product
// determined by xgrid and ygrid. If there are no nearest neighbors to
// a grid point within dist_crit, do not assign a value to the grid
// point.
void kd_rect_interp
(
 KD_real *xdata,		// I - x coordinates of data points
 KD_real *ydata,		// I - y coordinates of data points
 int ndata,			// I - number of data points
 KD_real *data_val,		// I - values at data points
 KD_real *xgrid,		// I - x coordinates of interp grid
 KD_real *ygrid,		// I - y coordinates of interp grid
 int xdim,			// I - size of xgrid
 int ydim,			// I - size of ygrid
 KD_real dist_crit,             // I - distance criterion from grid point
 int sel,                       // I - selection criterion (KD::MIN, KD::MAX)
 KD_real *grid_val		// O - interp grid values (should have dimension xdim * ydim)
)
{
  KD_real **A;
  const int dim = 2;
  // KD_real querpoint[dim];
  vector<int> ptsFound;
  vector<KD_real> values;

  A = new KD_real*[ndata];
  for (int k=0; k < ndata; k++)
    {
      A[k] = new KD_real[dim];
      A[k][0] = xdata[k];
      A[k][1] = ydata[k];
    }

  // Create KD tree
  KD_tree kdt((const KD_real **)A, ndata, dim);
  //KD_naive kdn((const KD_real **)A, ndata, dim);
  
  // Allocate rectangular query consisting of x bounds, y bounds, etc.
  KD_real **rectquery = new KD_real *[dim];

  for (int k=0; k < dim; k++)
    rectquery[k] = new KD_real[dim];


  // Get nearest neighbor of query points of grid
  int ct = 0;
  for (int j=0; j<ydim; j++)
    {
      for (int i=0; i<xdim; i++)
	{
	  // querpoint[0] = xgrid[i];
	  // querpoint[1] = ygrid[j];

          rectquery[0][0] = xgrid[i] - dist_crit;		// x bounds
          rectquery[0][1] = xgrid[i] + dist_crit;
          rectquery[1][0] = ygrid[j] - dist_crit;		// y bounds
          rectquery[1][1] = ygrid[j] + dist_crit;

          ptsFound.clear();
          kdt.rectquery((const KD_real **)rectquery, ptsFound);

          if (ptsFound.size() != 0)
            {
              values.clear();
              for (int k=0; k<(int)ptsFound.size(); k++)
                {
                  // Determine if distance of neighbor is within dist_crit
                  if (hypot((xgrid[i] - xdata[ptsFound[k]]), (ygrid[j] - ydata[ptsFound[k]])) < dist_crit)
                    {
                      values.push_back(data_val[ptsFound[k]]);
                    }
                }
              
              if (values.size() != 0)
                {
                  if (sel == KD::MIN)
                    {
                      grid_val[ct] = *min_element(values.begin(), values.end());
                    }
                  else if (sel == KD::MAX)
                    {
                      grid_val[ct] = *max_element(values.begin(), values.end());
                    }
                }
            }
	  ct++;
	}
    }

  for (int k=0; k < dim; k++)
    delete [] rectquery[k];

  delete [] rectquery;

  for (int k=0; k < ndata; k++)
    delete [] A[k];

  delete [] A;
}


