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
#include <stdlib.h>
#include "../include/kd/metric.hh"
#include "../include/kd/naive.hh"

using namespace std;

// This module contains naive implementations of nearest neighbor
// and rectangular queries.


static KD_real (*Distance)(const KD_real **Points, int Index, const KD_real *NNQPoint, int dimension, int MinkP);

KD_naive::KD_naive(const KD_real **points, int num_points, int dimension) : _points(points), _num_points(num_points), _dimension(dimension)
{
}


KD_naive::KD_naive(const KD_naive &kdt)	// copy constructor
{
  _num_points = kdt._num_points;
  _dimension = kdt._dimension;
  _points = kdt._points;
}

KD_naive & KD_naive::operator=(const KD_naive &kdt) // assignment operator
{
  if (this == &kdt)
    return *this;

  _num_points = kdt._num_points;
  _dimension = kdt._dimension;
  _points = kdt._points;
  
  return *this;
}

KD_naive::~KD_naive()
{
}
  

// Returns a pointer to an array of array indices of points in the array  
// Points falling within the hyper-rectangle defined by RectQuery.        
// The first element of the array contains the number of points falling   
// inside the query.                                                       
void KD_naive::rectquery(const KD_real **RectQuery, vector<int> &found)
{
  for (int j=0; j < _num_points; j++)
    {
      if (KD_ptInRect((const KD_real *)_points[j], _dimension, RectQuery))
        {
          found.push_back(j);
        }
    }

  return;
}



// Makes the perm partition the array Values along the element k.
// Adapted from Sedgewick's Algorithms in C (p. 128)
void KD_naive::selection(KD_real *a, int *perm, int N, int k)
{
  KD_real v;
  int t,i,j,l,r;

  l=0;
  r=N-1;

  while(r>l)
    {
      v=a[perm[r]];
      i=l-1;
      j=r;
      for (;;)
	{
	  while (a[perm[++i]] < v)
	    ;

	  while (a[perm[--j]] > v && j>l)
	    ;
	  if (i >= j)
	    break;

	  t=perm[i];
	  perm[i] = perm[j];
	  perm[j]=t;
	}

      t=perm[i]; perm[i] = perm[r]; perm[r]=t;
      if (i>=k)
	r=i-1;
      if (i<=k)
	l=i+1;
    }
}


// Returns a pointer to an array of the NumNN indices of the array points 
// closest to the query point q.                                         
// The first element of the array contains the number of points asked for, NumNN.
void KD_naive::nnquery(KD_real *NNQPoint, int NumNN, int Metric, int MinkP, int *found, KD_real *dist_array)
{
  int j; 

  switch(Metric)
    {
    case KD_EUCLIDEAN:
      Distance = KD_EuclidDist2;
      break;

    case KD_MANHATTAN:
      Distance = KD_ManhattDist;
      break;

    case KD_L_INFINITY:
      Distance = KD_LInfinityDist;
      break;

    case KD_L_P:
      Distance = KD_LGeneralDist;
      break;

    default:
      Distance = KD_EuclidDist2;
      break;
    }

  int *perm = (int *) new int[_num_points];
  KD_real *dist = new KD_real[_num_points];


  for (j=0; j < _num_points; j++)
    {
      dist[j] = Distance((const KD_real **)_points, j, NNQPoint, _dimension, MinkP);
      perm[j] = j;
    }

  selection(dist, perm, _num_points, NumNN);

  for (j=0; j <NumNN; j++)
    {
      found[j] = perm[j];
      dist_array[j] = dist[perm[j]];
    }

  delete [] dist;
  delete [] perm;

  return;
}



