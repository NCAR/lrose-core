/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 2004
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization.
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

//----------------------------------------------------------------------
// Module: test_kd_query.cc
//
// Author: Gerry Wiener
//
// Date:   10/2/04
//
// Description:
//     Test new kd_query.cc module.
//----------------------------------------------------------------------

// Include files 
#include <kd/kd_query.hh>
#include <stdio.h>

// Constant, macro and type definitions 

// Global variables 

// Functions and objects
void test_file()
{
  FILE *fp = fopen("metar_points.txt", "r");
  int ret;
  KD_real xdata[10000];
  KD_real ydata[10000];


  int ct = 0;
  for (int i=0; ; i++)
    {
      ret = fscanf(fp, "%lg%lg", &xdata[i], &ydata[i]);
      if (ret != 2)
        break;
      ct++;
    }

  KD_real query[2];
  int found[4];
  KD_real dist[4];
  int foundn[4];
  KD_real distn[4];
  
  Kd_tree_query kdtq(xdata, ydata, ct, KD_EUCLIDEAN, 1);
  Kd_naive_query kdnq(xdata, ydata, ct, KD_EUCLIDEAN, 1);
  for (int j=0; j<225; j++)
    {
      for (int i=0; i<301; i++)
        {
          query[0] = i;
          query[1] = j;
          kdtq.nnquery(query, 1, found, dist);
          kdnq.nnquery(query, 1, foundn, distn);
          if (dist[0] <= 8)
            printf("%d %d dist %g\n", i, j, dist[0]);
          if (distn[0] <= 8)
            printf("%d %d distn %g\n", i, j, dist[0]);
          if (dist[0] != distn[0])
            printf("UNEQUAL\n");

        }

    }

  
}

int main(int argc, char **argv)
{
  // Setup points

  const int SIZE = 4;
  KD_real xdata[SIZE];			// I - x coordinates of data points
  KD_real ydata[SIZE];			// I - y coordinates of data points
  int ndata = SIZE;			// I - number of data points
  KD_real data_val[SIZE];		// I - values at data points

  test_file();
  xdata[0] = 1;
  ydata[0] = 1;
  xdata[1] = 1;
  ydata[1] = -1;
  
  xdata[2] = -1;
  ydata[2] = 1;
  xdata[3] = -1;
  ydata[3] = -1;
  
  
  for (int i=0; i<SIZE; i++)
    data_val[i] = i;


  Kd_tree_query kdtq(xdata, ydata, ndata, KD_EUCLIDEAN, 1);

  KD_real query[2];
  int found[4];
  KD_real dist[4];
  
  query[0] = 2;
  query[1] = 2;
  kdtq.nnquery(query, 4, found, dist);
  printf("query: %g, %g\n", query[0], query[1]);
  for (int i=0; i<4; i++)
    {
      printf("found[%d] = %d; point %g, %g; dist = %g\n", i, found[i], xdata[found[i]], ydata[found[i]], dist[i]);
    }

  query[0] = 0;
  query[1] = 0;
  kdtq.nnquery(query, 4, found, dist);
  printf("query: %g, %g\n", query[0], query[1]);
  for (int i=0; i<4; i++)
    {
      printf("found[%d] = %d; point %g, %g; dist = %g\n", i, found[i], xdata[found[i]], ydata[found[i]], dist[i]);
    }

  query[0] = -1;
  query[1] = -1;
  kdtq.nnquery(query, 4, found, dist);
  printf("query: %g, %g\n", query[0], query[1]);
  for (int i=0; i<4; i++)
    {
      printf("found[%d] = %d; point %g, %g; dist = %g\n", i, found[i], xdata[found[i]], ydata[found[i]], dist[i]);
    }


  query[0] = 0;
  query[1] = 10;
  kdtq.nnquery(query, 4, found, dist);
  printf("query: %g, %g\n", query[0], query[1]);
  for (int i=0; i<4; i++)
    {
      printf("found[%d] = %d; point %g, %g; dist = %g\n", i, found[i], xdata[found[i]], ydata[found[i]], dist[i]);
    }

  
  return(0);
}


