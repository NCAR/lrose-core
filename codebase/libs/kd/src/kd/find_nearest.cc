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
// Module: find_nearest.cc
//
// Author: Gerry Wiener
//
// Date:   5/20/04
//
// Description:
//     Given two files, one consisting of a set of points and another
//     consisting of query points. Find the k-nearest neighbors of each
//     query point.
//----------------------------------------------------------------------

// Include files 
#include <sys/time.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <kd/kd.hh>
#include <kd/metric.hh>
#include <kd/naive.hh>

#include <kd/tokenize.hh>

using namespace std;

// Constant, macro and type definitions 

// Global variables 

// Functions and objects


// All arrays must be allocated by the caller. Finds num_nearest
// points in point_set nearest to each point in query_points.
// Distances of query point to num_nearest neighbors are stored in
// dist array. Returns 0 on success and -11 if num_nearest is not
// reasonable.
int find_nearest
(
 int dimension,			// I - dimension of points (usually 2)
 KD_real **point_set,		// I - array containing set of points
 int point_set_size,		// I - size of point_set
 KD_real **query_points,	// I - array of query points
 int query_points_size,		// I - size of query_points
 int num_nearest,		// I - number of nearest neighbors to find
 int *found,		        // O - array of indices of size num_nearest * query_points_size
 KD_real *dist			// O - array of distances of query point from nearest neighbor (size is num_nearest * query_points_size)
)
{
  int Metric = KD_EUCLIDEAN;
  int MinkP = 1;

  if (num_nearest <= 0 || num_nearest > point_set_size)
    return(-1);

  // Create KD tree
  KD_tree kdt((const KD_real **)point_set, point_set_size, dimension);

  // Get nearest neighbor of query points
  for (int i=0; i<query_points_size; i++)
    {
      kdt.nnquery(query_points[i], num_nearest, Metric, MinkP, &found[i*num_nearest], &dist[i*num_nearest]);
    }

  return(0);
}

// Read points from file and store in vector
int read_data(char *in_file, vector<KD_real> pts)
{
  vector<string> fstr;
  ifstream in(in_file);		// Open input file
  string s;

  // Check that input file has been opened
  if (!in.is_open())
    return(-1);
  
  // Read input and store in fstr vector
  while (getline(in, s))
    {
      vector<string> tokens;

      // Split each line into string tokens
      tokenize(s, tokens, " \t");

      printf("token size = %d\n", tokens.size());
    }

  return(0);
}

int main(int argc, char **argv)
{

  int dimension = 2;
  int num_nearest = 2;
  int point_set_size = 5;
  int query_size = 3;

  int *found = new int[num_nearest * query_size];
  KD_real *dist = new KD_real[num_nearest * query_size];
    
  KD_real **point_set = new (KD_real *)[point_set_size];
  KD_real **query = new (KD_real *)[query_size];

  // Allocate array
  for (int k=0; k<point_set_size; k++)
    {
      point_set[k] = new KD_real[dimension];
    }

  // Allocate array
  for (int k=0; k<query_size; k++)
    {
      query[k] = new KD_real[dimension];
    }

  query[0][0] = 6;
  query[0][1] = 6;
  query[1][0] = 2.1;
  query[1][1] = 2.1;
  query[2][0] = -5.0;
  query[2][1] = 0.0;

  for (int k=0; k<query_size; k++)
    {
      printf("query[%d] = %g, %g\n", k, query[k][0], query[k][1]);
    }

  // Initialize array
  for (int k=0; k<point_set_size; k++)
    {
      point_set[k][0] = k;
      point_set[k][1] = k;
      printf("point[%d] = %g, %g\n", k, point_set[k][0], point_set[k][1]);
    }


  find_nearest(dimension, point_set, point_set_size, query, query_size, num_nearest, found, dist);

  for (int i=0; i<query_size; i++)
    {
      for (int j=0; j<num_nearest; j++)
	{
	  printf("found[%d][%d] = %d, dist = %g\n", i, j, found[i*num_nearest + j], dist[i*num_nearest + j]);
	}

    }

  
}
