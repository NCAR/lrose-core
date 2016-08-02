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
/*
 *   Module: kd.hh
 *
 *   Author: Gerry Wiener
 *
 *   Date:   11/10/01
 *
 *   Description: 
 *       Adapted from ranger code located at
 *
 *       http://www.cs.sunysb.edu/~algorith/implement/ranger/implement.shtml
 *
 *       Original code was extensively modified and converted to C++.
 *
 *   Also see:
 *
 *            The Algorithm Design Manual 
 *         by Steven S. Skiena, Steve Skiena
 */

#ifndef KD_HH
#define KD_HH

#ifdef SUNOS5_ETG
using namespace std;
#endif

// Include files 
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include "datatype.hh"
#include "fileoper.hh"
#include "naive.hh"
#include "metric.hh"

using namespace std;

// Constant, macro and type definitions 


typedef struct optkdnode
{
  int bucket;  // 1 if the node is a bucket node and 0 otherwise
  int discrim; // dimension where the maximum spread occurs (the splitting dimension)
  KD_real cutval; // partition value 
  struct optkdnode *lochild, *hichild; // low child and high child for node
  int lopt, hipt;  // low index and high index for this node 
} optkdNode;

const int KD_BUCKETSIZE = 50;



class KD_tree
{
public:
  //
  // For the constructor, the user must specify an array of points using a procedure like the following:
  // for (int k=0; k < num_points; k++)
  //    {
  //      A[k] = new KD_real[2];	// dimension is 2 here but can be > 2
  //      A[k][0] = xdata[k];
  //      A[k][1] = ydata[k];
  //    }
  //
  KD_tree(const KD_real **points, int num_points, int dimension); // standard constructor
  KD_tree(const KD_tree &kdt);	// copy constructor
  KD_tree & operator=(const KD_tree &kdt); // assignment operator
  ~KD_tree();
  
  // Find the nearest neighbors of a query point, querpoint.
  // querpoint: a multidimensional array of KD_reals having dimension, _dimension.
  // numNN:     the number of nearest neighbors to be output (> 1)
  // Metric:    can be one of KD_EUCLIDEAN, KD_MANHATTAN, KD_L_INFINITY, KD_L_P.
  // MinkP:     (the Minkowski parameter) should be 1 unless KD_L_P is used then MinkP should be set to a positive integer n > 1.
  // found:     the array of point indices found to be closest to the query point querpoint
  // dist:      the distances of nearest neighbor point from the query point 
  void nnquery(KD_real *querpoint, int numNN, int Metric, int MinkP, int *found, KD_real *dist);

  
  // Find the points in a rectangle specified by RectQuery. Note that the rectangle can have dimension larger than 2.
  // The dimension however must agree with the dimension specified by the user in the KD_tree constructor.
  // The indices of the points found will be returned in the vector ptsFound
  void rectquery(const KD_real **RectQuery, vector<int> &ptsFound);

  // Return the number of points specified by the user
  int get_num_points() { return _num_points; }

  // Return the number of dimensions specified by the user
  int get_dimension() { return _dimension; }

  // Return a pointer to the points specified by the user
  const KD_real ** get_points() { return _points; }

private:
  const KD_real **_points;
  int _num_points;
  int _dimension;
  KD_real *_nndist;
  optkdNode *_OptkdRoot;
  int *_optfound;
  int *_perm;  /* permutation array */

  optkdNode *BuildkdTree(int l, int u);

  void KillOptTree(optkdNode *P);

  // Makes the perm partition the array Values along the element k.
  // Adapted from Sedgewick's Algorithms in C (p. 128)
  void Selection(int l, int N, int k, int discrim);

  int findmaxspread(int l, int u);

  // special searching algorithm to take advantage of the fact that square roots
  // do not need to be evaluated
  void rnnEuclidean(const optkdNode *p, const KD_real *querpoint, int numNN);

  void rnnGeneral(const optkdNode *p, const KD_real *querpoint, int numNN, int MinkP);

  void optInRegion(const optkdNode *P, const KD_real **RectQuery, vector<int> &ptsFound);
  int optBoundsIntersectRegion(const KD_real *B, const KD_real **RectQuery);
  void optRangeSearch(const optkdNode *P, const KD_real **RectQuery, const KD_real *B, vector<int> &ptsFound);
};


#endif /* KD_HH */
