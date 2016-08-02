// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1992 - 2001 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Program(RAP) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2001/12/18 19:51:15 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
//----------------------------------------------------------------------
// Module: time_testkd.cc
//
// Author: Gerry Wiener
//
// Date:   11/14/01
//
// Description:
//     
//----------------------------------------------------------------------

// Include files 
#include <sys/time.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <kd/kd.hh>
#include <kd/metric.hh>

// Constant, macro and type definitions 

// Global variables 

// Functions and objects

// Regular grid test
// A test will consist of an input grid and a set of query points. The output will be a set
// of nearest neighbors corresponding to the query points
// Use regular grids for input and query points
int test_rectquery(int xdim, int ydim, int zdim)
{
  int dimension = 3;
  const int ndata = xdim * ydim * zdim;
  int ind = 0;
  int ret = 0;

  KD_real **A = new KD_real*[ndata];
  KD_real *xdata = new KD_real[ndata];
  KD_real *ydata = new KD_real[ndata];
  KD_real *zdata = new KD_real[ndata];
  KD_real *xgrid = new KD_real[ndata];
  KD_real *ygrid = new KD_real[ndata];
  KD_real *zgrid = new KD_real[ndata];

  for (unsigned int k=0; k<; k++)
    {
      for (int j=0; j<ydim; j+=10)
	{
	  for (int i=0; i<xdim; i+=10)
	    {

	      xdata[ind] = i;
	      ydata[ind] = j;
	      xgrid[ind] = i + 0.25;
	      ygrid[ind] = j + 0.75;
	      ind++;
	    }
	}
    }

  // Allocate and initialize array
  for (int k=0; k < ndata; k++)
    {
      A[k] = new KD_real[dimension];
      A[k][0] = xdata[k];
      A[k][1] = ydata[k];
      A[k][2] = zdata[k];
    }

  // Create KD tree
  KD_tree kdt((const KD_real **)A, ndata, dimension);
  
  // Allocate rectangular query consisting of x bounds, y bounds, etc.
  KD_real **rectquery = new (KD_real*)[dimension];

  for (int k=0; k < dimension; k++)
    rectquery[k] = new KD_real[2]; // for low and high bounds for each dimension

  // Run through 100 query rectangles
  for (unsigned int i=0; i<100; i++)
    {
      for (unsigned int j=0; j<dimension; j++)
	{
	  rectquery[j][0] = i;		// x bounds
	  rectquery[j][1] = i+10;
	  kdt.rectquery((const KD_real **)rectquery, ptsFound);
	}
    }


  for (int k=0; k < dimension; k++)
    delete [] rectquery[k];

  delete [] rectquery;

  for (int k=0; k < ndata; k++)
    delete [] A[k];

  delete [] A;
  delete [] xdata;
  delete [] ydata;
  delete [] xgrid;
  delete [] ygrid;

  return(ret);
}


// Regular grid test
// A test will consist of an input grid and a set of query points. The output will be a set
// of nearest neighbors corresponding to the query points
// Use regular grids for input and query points
int test_nnquery_reg(int xdim, int ydim)
{
  int dimension = 2;
  const int ndata = xdim * ydim;
  const int num_neighbor = 3;
  int found[num_neighbor];
  KD_real dist[num_neighbor];

  KD_real *xdata = new KD_real[ndata];
  KD_real *ydata = new KD_real[ndata];
  KD_real *xgrid = new KD_real[ndata];
  KD_real *ygrid = new KD_real[ndata];
  KD_real **A;

  int Metric = KD_EUCLIDEAN;
  int MinkP = 1;
  KD_real querpoint[2];

  int ind = 0;
  for (int j=0; j<ydim; j++)
    {
      for (int i=0; i<xdim; i++)
	{
	  xdata[ind] = i;
	  ydata[ind] = j;
	  xgrid[ind] = i + 0.25;
	  ygrid[ind] = j + 0.75;
	  ind++;
	}
    }

  A = new KD_real*[ndata];
  for (int k=0; k < ndata; k++)
    {
      A[k] = new KD_real[2];	// dimension is 2
      A[k][0] = xdata[k];
      A[k][1] = ydata[k];
    }

  // Create KD tree
  KD_tree kdt((const KD_real **)A, ndata, dimension);
  
  // Get nearest neighbor of query points
  for (int i=0; i<ndata; i++)
    {
      querpoint[0] = xgrid[i];
      querpoint[1] = ygrid[i];
      kdt.nnquery(querpoint, num_neighbor, Metric, MinkP, found, dist);
    }

  for (int k=0; k < ndata; k++)
    delete [] A[k];

  delete [] xdata;
  delete [] ydata;
  delete [] xgrid;
  delete [] ygrid;
  return(0);
}


int main(int argc, char **argv)
{
  int ret;
  
  // Find the nearest neighbors of 1000 x 1000 points
  ret = test_nnquery_reg(1000, 1000);
  return 0;
}
