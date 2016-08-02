// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1992 - 2001 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Program(RAP) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2001/12/18 19:51:15 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*
 * Module: test_kd.cc
 *
 * Author: Gerry Wiener
 *
 * Date:   11/6/01
 *
 * Description:
 *     
 */

/* Include files */
#include <sys/time.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <kd/kd.hh>
#include <kd/metric.hh>
#include <kd/naive.hh>
#include <kd/kd_query.hh>

using namespace std;

/* Constant, macro and type definitions */

/* Global variables */

/* Functions */
int test_rectquery(KD_tree &kdt, KD_naive &kdn, const KD_real **rectquery)
{
  vector<int> ptsFound;
  vector<int> ptsFound1;
  int ret = 0;
  
  kdt.rectquery((const KD_real **)rectquery, ptsFound);
  kdn.rectquery((const KD_real **)rectquery, ptsFound1);

  if (ptsFound.size() != ptsFound1.size())
    {
      printf("failure sizes %d %d are not equal\n", ptsFound.size(), ptsFound1.size());
      ret = -1;
    }

  sort(ptsFound.begin(), ptsFound.end());
  sort(ptsFound1.begin(), ptsFound1.end());


  for (int j=0; j<(int)ptsFound.size(); j++)
    {
      if (ptsFound[j] != ptsFound1[j])
	{
	  printf("ptsFound[%d] = %d != ptsFound1[%d] = %d\n", j, ptsFound[j], j, ptsFound1[j]);
	  ret = -1;
	}
    }
  return(ret);
}

// Regular grid test
// A test will consist of an input grid and a set of query points. The output will be a set
// of nearest neighbors corresponding to the query points
// Use regular grids for input and query points
int test_rectquery_reg(int xdim, int ydim)
{
  int dimension = 2;
  const int ndata = xdim * ydim;
  int ind = 0;
  int ret = 0;

  KD_real **A = new KD_real*[ndata];
  KD_real *xdata = new KD_real[ndata];
  KD_real *ydata = new KD_real[ndata];
  KD_real *xgrid = new KD_real[ndata];
  KD_real *ygrid = new KD_real[ndata];

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

  // Allocate and initialize array
  for (int k=0; k < ndata; k++)
    {
      A[k] = new KD_real[dimension];
      A[k][0] = xdata[k];
      A[k][1] = ydata[k];
    }

  // Create KD tree
  KD_tree kdt((const KD_real **)A, ndata, dimension);
  
  // Create KD naive
  KD_naive kdn((const KD_real **)A, ndata, dimension);
  
  // Allocate rectangular query consisting of x bounds, y bounds, etc.
  KD_real **rectquery = new KD_real*[dimension];

  for (int k=0; k < dimension; k++)
    rectquery[k] = new KD_real[2];

  rectquery[0][0] = 0;		// x bounds
  rectquery[0][1] = 4;
  rectquery[1][0] = 0;		// y bounds
  rectquery[1][1] = 10;

  if (test_rectquery(kdt, kdn, (const KD_real **)rectquery) < 0)
    {
      ret = -1;
      printf("failed on rectquery %f, %f, %f, %f\n", rectquery[0][0], rectquery[0][1], rectquery[1][0], rectquery[1][1]);
    }

  
  rectquery[0][0] = 1000;	// x bounds
  rectquery[0][1] = 2000;
  rectquery[1][0] = 1000;	// y bounds
  rectquery[1][1] = 2000;
  if (test_rectquery(kdt, kdn, (const KD_real **)rectquery) < 0)
    {
      ret = -1;
      printf("failed on rectquery %f, %f, %f, %f\n", rectquery[0][0], rectquery[0][1], rectquery[1][0], rectquery[1][1]);
    }

  rectquery[0][0] = 10;		// x bounds
  rectquery[0][1] = 20;
  rectquery[1][0] = 10;		// y bounds
  rectquery[1][1] = 20;
  if (test_rectquery(kdt, kdn, (const KD_real **)rectquery) < 0)
    {
      ret = -1;
      printf("failed on rectquery %f, %f, %f, %f\n", rectquery[0][0], rectquery[0][1], rectquery[1][0], rectquery[1][1]);
    }

  
  rectquery[0][0] = 5;		// x bounds
  rectquery[0][1] = 45;
  rectquery[1][0] = 10;		// y bounds
  rectquery[1][1] = 20;
  if (test_rectquery(kdt, kdn, (const KD_real **)rectquery) < 0)
    {
      ret = -1;
      printf("failed on rectquery %f, %f, %f, %f\n", rectquery[0][0], rectquery[0][1], rectquery[1][0], rectquery[1][1]);
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
  int found1[num_neighbor];
  KD_real dist[num_neighbor];
  KD_real dist1[num_neighbor];
  int ret = 0;

  KD_real **A = new KD_real*[ndata];
  KD_real *xdata = new KD_real[ndata];
  KD_real *ydata = new KD_real[ndata];
  KD_real *xgrid = new KD_real[ndata];
  KD_real *ygrid = new KD_real[ndata];

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

  for (int k=0; k < ndata; k++)
    {
      A[k] = new KD_real[dimension];
      A[k][0] = xdata[k];
      A[k][1] = ydata[k];
    }

  // Create KD tree
  KD_tree kdt((const KD_real **)A, ndata, dimension);
  
  // Create KD naive
  KD_naive kdn((const KD_real **)A, ndata, dimension);

  // Get nearest neighbor of query points
  for (int i=0; i<ndata; i++)
    {
      querpoint[0] = xgrid[i];
      querpoint[1] = ygrid[i];
      kdt.nnquery(querpoint, num_neighbor, Metric, MinkP, found, dist);
      kdn.nnquery(querpoint, num_neighbor, Metric, MinkP, found1, dist1);
      vector <KD_real> dist2;

      for (int j=0; j<num_neighbor; j++)
	dist2.push_back(dist1[j]);

      sort(dist2.begin(), dist2.end());

      
      for (int j=0; j<num_neighbor; j++)
	if (dist[j] != dist2[j])
	  {
	    printf("dist[%d] = %f, dist2[%d] = %f\n", j, dist[j], j, dist2[j]);
	    ret = -1;
	  }
    }

  for (int k=0; k < ndata; k++)
    delete [] A[k];

  delete [] A;
  delete [] xdata;
  delete [] ydata;
  delete [] xgrid;
  delete [] ygrid;
  return(ret);
}


// Circle Test:
// A test will consist of an input grid and a set of query points. The output will be a set
// of nearest neighbors corresponding to the query points
// Use circular grids for input and query points. Query point will be along same vector from origin
// as neareast grid point.
int test_nnquery_circle(int ndim)
{
  int dimension = 2;
  KD_real angle;
  const int ndata = ndim;
  const int num_neighbor = 3;
  int found[num_neighbor];
  KD_real dist[num_neighbor];
  int ret = 0;

  KD_real **A = new KD_real*[ndata];
  KD_real *xdata = new KD_real[ndata];
  KD_real *ydata = new KD_real[ndata];
  KD_real *xgrid = new KD_real[ndata];
  KD_real *ygrid = new KD_real[ndata];

  int Metric = KD_EUCLIDEAN;
  int MinkP = 1;
  KD_real querpoint[2];


  int ind = 0;
  KD_real radius = 100;
  KD_real radius1 = 200;
  angle = 0;
  for (int i=0; i<ndim; i++)
    {
      xdata[ind] = radius * cos(angle);
      ydata[ind] = radius * sin(angle);
      xgrid[ind] = radius1 * cos(angle);
      ygrid[ind] = radius1 * sin(angle);
      ind++;
      angle += 2 * M_PI / ndim;
    }

  for (int k=0; k < ndata; k++)
    {
      A[k] = new KD_real[2];	// dimension is 2
      A[k][0] = xdata[k];
      A[k][1] = ydata[k];
    }


  // Create kd tree query object
  Kd_tree_query kdtq(xdata, ydata, ndata, Metric, MinkP);


  // Create KD tree
  KD_tree kdt((const KD_real **)A, ndata, dimension);

  // Get nearest neighbor of query points
  for (int i=0; i<ndata; i++)
    {
      querpoint[0] = xgrid[i];
      querpoint[1] = ygrid[i];
      kdt.nnquery(querpoint, num_neighbor, Metric, MinkP, found, dist);
      if (found[0] != i)
	{
	  printf("found[%d] = %d != %d\n", 0, found[0], i);
	  ret = -1;
	}
      kdtq.nnquery(querpoint, num_neighbor, found, dist);
      if (found[0] != i)
	{
	  printf("kdtq found[%d] = %d != %d\n", 0, found[0], i);
	  ret = -1;
	}
   }

  for (int k=0; k < ndata; k++)
    delete [] A[k];

  delete [] A;
  delete [] xdata;
  delete [] ydata;
  delete [] xgrid;
  delete [] ygrid;

  return(ret);
}


// Simple Test:
int test_nnquery_simple()
{
  int ndim = 3;
  int dimension = ndim;
  const int ndata = 4;
  const int num_neighbor = 3;
  int found[num_neighbor];
  KD_real dist[num_neighbor];
  int ret = 0;

  KD_real **A = new KD_real*[ndata];
  KD_real *xdata = new KD_real[ndata];
  KD_real *ydata = new KD_real[ndata];
  KD_real *zdata = new KD_real[ndata];

  int Metric = KD_EUCLIDEAN;
  int MinkP = 1;
  KD_real querpoint[ndim];


  xdata[0] = 0.0;
  ydata[0] = 0.0;
  zdata[0] = 0.0;

  xdata[1] = 1.0;
  ydata[1] = 1.0;
  zdata[1] = 1.0;

  xdata[2] = 2.0;
  ydata[2] = 2.0;
  zdata[2] = 2.0;

  for (int k=0; k < ndata; k++)
    {
      A[k] = new KD_real[ndim];	// dimension is 3
      A[k][0] = xdata[k];
      A[k][1] = ydata[k];
      A[k][2] = zdata[k];
    }

  // Create KD tree
  KD_tree kdt((const KD_real **)A, ndata, dimension);

  querpoint[0] = -1.0;
  querpoint[1] = -1.0;
  querpoint[2] = -1.0;

  kdt.nnquery(querpoint, num_neighbor, Metric, MinkP, found, dist);
  for (int i=0; i<num_neighbor; i++)
    {
      printf("found[%d] = %d, dist**2 = %g\n", i, found[i], dist[i]);
    }

  printf("ret = %d\n", ret);

  for (int k=0; k < ndata; k++)
    delete [] A[k];

  delete [] A;
  delete [] xdata;
  delete [] ydata;
  delete [] zdata;
  return(ret);
}


// Simple Test:
int test_nnquery_simple1()
{
  int ndim = 3;
  int dimension = ndim;
  const int ndata = 10000;
  const int num_neighbor = 3;
  KD_real dist[num_neighbor];
  int ret = 0;

  KD_real **A = new KD_real*[ndata];
  KD_real *xdata = new KD_real[ndata];
  KD_real *ydata = new KD_real[ndata];
  KD_real *zdata = new KD_real[ndata];

  int Metric = KD_EUCLIDEAN;
  int MinkP = 1;
  KD_real querpoint[ndim];


  int ct = 0;
  for (int i=0; i<50; i++)
    for (int j=0; j<50; j++)
      for (int k=0; k<4; k++)
	{
	  xdata[ct] = k;
	  ydata[ct] = j;
	  zdata[ct] = i;
	  ct++;
	}

  for (int k=0; k < ndata; k++)
    {
      A[k] = new KD_real[ndim];	// dimension is 3
      A[k][0] = xdata[k];
      A[k][1] = ydata[k];
      A[k][2] = zdata[k];
    }

  // Create KD tree
  KD_tree kdt((const KD_real **)A, ndata, dimension);

  // Allocate rectangular query consisting of x bounds, y bounds, etc.
  KD_real **rectquery = new KD_real*[dimension];

  for (int k=0; k < dimension; k++)
    rectquery[k] = new KD_real[2];

  rectquery[0][0] = 0;		// x bounds
  rectquery[0][1] = 4;
  rectquery[1][0] = 0;		// y bounds
  rectquery[1][1] = 5;
  rectquery[2][0] = 0;		// z bounds
  rectquery[2][1] = 1;


  vector<int> ptsFound;
  
  for (unsigned int i=0; i<1000000; i++)
    {
      kdt.rectquery((const KD_real **)rectquery, ptsFound);
    }


#ifdef NOTNOW  
  for (unsigned int j=0; j<ptsFound.size(); j++)
    {
      printf("ptsFound[%d] = %d \n", j, ptsFound[j]);
      printf("x,y,z = %g, %g, %g\n", xdata[ptsFound[j]], ydata[ptsFound[j]], zdata[ptsFound[j]]);
    }
#endif

  for (int k=0; k < ndata; k++)
    delete [] A[k];

  delete [] A;
  delete [] xdata;
  delete [] ydata;
  delete [] zdata;
  return(ret);
}

// Simple Test:
int test_nnquery_simple2()
{
  int ndim = 2;
  int dimension = ndim;
  const int ndata = 2500;
  const int num_neighbor = 3;
  int found[num_neighbor];
  KD_real dist[num_neighbor];
  int ret = 0;

  KD_real **A = new KD_real*[ndata];
  KD_real *xdata = new KD_real[ndata];
  KD_real *ydata = new KD_real[ndata];

  int Metric = KD_EUCLIDEAN;
  int MinkP = 1;
  KD_real querpoint[ndim];


  int ct = 0;
  for (int i=0; i<50; i++)
    for (int j=0; j<50; j++)
	{
	  xdata[ct] = j;
	  ydata[ct] = i;
	  ct++;
	}

  for (int k=0; k < ndata; k++)
    {
      A[k] = new KD_real[ndim];
      A[k][0] = xdata[k];
      A[k][1] = ydata[k];
    }

  // Create KD tree
  KD_tree kdt((const KD_real **)A, ndata, dimension);

  // Allocate rectangular query consisting of x bounds, y bounds, etc.
  KD_real **rectquery = new KD_real*[dimension];

  for (int k=0; k < dimension; k++)
    rectquery[k] = new KD_real[2];

  rectquery[0][0] = 0;		// x bounds
  rectquery[0][1] = 4;
  rectquery[1][0] = 0;		// y bounds
  rectquery[1][1] = 5;


  vector<int> ptsFound;
  
  kdt.rectquery((const KD_real **)rectquery, ptsFound);

  for (unsigned int j=0; j<ptsFound.size(); j++)
    {
      printf("ptsFound[%d] = %d \n", j, ptsFound[j]);
      printf("x,y = %g, %g\n", xdata[ptsFound[j]], ydata[ptsFound[j]]);
    }

  for (int k=0; k < ndata; k++)
    delete [] A[k];

  delete [] A;
  delete [] xdata;
  delete [] ydata;
  return(ret);
}


// Sin Test:
// A test will consist of an input grid and a set of query points. The output will be a set
// of nearest neighbors corresponding to the query points
// Use sin wave grid for input points. Query points will be on x = k * pi/2 (where k = 4n+1), y = 10.
int test_nnquery_sin()
{
  int ndim = 200;
  int dimension = 2;
  KD_real angle;
  const int ndata = ndim;
  const int num_neighbor = 10;
  int found[num_neighbor];
  KD_real dist[num_neighbor];
  int ret = 0;

  KD_real **A = new KD_real*[ndata];
  KD_real *xdata = new KD_real[ndata];
  KD_real *ydata = new KD_real[ndata];

  int Metric = KD_EUCLIDEAN;
  int MinkP = 1;
  KD_real querpoint[2];

  int ind = 0;
  angle = 0;
  for (int i=0; i<ndim; i++)
    {
      xdata[ind] = angle;
      ydata[ind] = sin(angle);
      ind++;
      angle += 2 * M_PI / ndim;
    }

  for (int k=0; k < ndata; k++)
    {
      A[k] = new KD_real[2];	// dimension is 2
      A[k][0] = xdata[k];
      A[k][1] = ydata[k];
    }

  // Create KD naive
  KD_naive kdn((const KD_real **)A, ndata, dimension);

  // Create KD tree
  KD_tree kdt((const KD_real **)A, ndata, dimension);

  // Get nearest neighbor of query points
  querpoint[0] = M_PI / 2.0;
  querpoint[1] = 10;
  kdt.nnquery(querpoint, num_neighbor, Metric, MinkP, found, dist);

  if (found[0] != 50)
    {
      printf("found[%d] = %d != %d\n", 0, found[0], 50);
      ret = -1;
    }


  // Allocate rectangular query consisting of two diagonal points
  KD_real **rectquery = new KD_real*[dimension];
  for (int k=0; k < dimension; k++)
    rectquery[k] = new KD_real[2];

  rectquery[0][0] = -10;	// x bounds
  rectquery[0][1] = 0;
  rectquery[1][0] = -10;	// y bounds
  rectquery[1][1] = 0;

  if (test_rectquery(kdt, kdn, (const KD_real **)rectquery) < 0)
    {
      ret = -1;
      printf("failed on rectquery %f, %f, %f, %f\n", rectquery[0][0], rectquery[0][1], rectquery[1][0], rectquery[1][1]);
    }

  for (int k=0; k < dimension; k++)
    delete [] rectquery[k];

  delete [] rectquery;

  for (int k=0; k < ndata; k++)
    delete [] A[k];

  delete [] A;
  delete [] xdata;
  delete [] ydata;

  return(ret);
}


int main(int argc, char **argv)
{
  int ret;
  int success = 1;
  
  test_nnquery_simple1();

#ifdef NOTNOW
  ret = test_rectquery_reg(0, 100);
  if (ret != 0)
    {
      printf("test_rectquery_reg failed\n");
      success = 0;
    }
  

  ret = test_nnquery_circle(100);
  if (ret != 0)
    {
      printf("test_nnquery_circle failed\n");
      success = 0;
    }

  ret = test_nnquery_reg(30, 30);
  if (ret != 0)
    {
      printf("test_nnquery_reg failed\n");
      success = 0;
    }

  ret = test_nnquery_sin();
  if (ret != 0)
    {
      printf("test_nnquery_sin failed\n");
      success = 0;
    }

  if (success)
    printf("success\n");
#endif

  return 0;
}

