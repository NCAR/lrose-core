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
//----------------------------------------------------------------------
// Module: kd.cc
//
// Author: Gerry Wiener
//
// Date:   11/10/01
//
// Description:
//       Adapted from ranger code - http://www.cs.sunysb.edu/~algorith/implement/ranger/implement.shtml
//
//            The Algorithm Design Manual 
//         by Steven S. Skiena, Steve Skiena 
//----------------------------------------------------------------------

// Include files 
#include <algorithm>
#include "../include/kd/kd.hh"
#include "../include/kd/metric.hh"
#include <vector>
#include "pqueue.hh"

using namespace std;

// Constant, macro and type definitions 

// Global variables 

// Functions and objects


static KD_real (*Distance)(const KD_real **Points, int Index, const KD_real *NNQPoint, int dimension, int MinkP);

KD_tree::KD_tree(const KD_real **points, int num_points, int dimension) : _points(points), _num_points(num_points), _dimension(dimension)
{
  int j;

  // initialize perm array 
  _perm = new int[_num_points];
  for (j=0; j < _num_points; j++)
    _perm[j]=j;

  _OptkdRoot = BuildkdTree(0, _num_points-1);
}


KD_tree::KD_tree(const KD_tree &kdt)	// copy constructor
{
  _num_points = kdt._num_points;
  _dimension = kdt._dimension;
  _points = kdt._points;
  
  _perm = new int[_num_points];
  for (int j=0; j < _num_points; j++)
    _perm[j]=j;

  _OptkdRoot = BuildkdTree(0, _num_points-1);
}

KD_tree & KD_tree::operator=(const KD_tree &kdt) // assignment operator
{
  if (this == &kdt)
    return *this;

  KillOptTree(_OptkdRoot);	// free up used resources

  _num_points = kdt._num_points;
  _dimension = kdt._dimension;
  _points = kdt._points;
  
  _perm = new int[_num_points];
  for (int j=0; j < _num_points; j++)
    _perm[j]=j;

  _OptkdRoot = BuildkdTree(0, _num_points-1);

  return *this;
}

KD_tree::~KD_tree()
{
  KillOptTree(_OptkdRoot);
  delete [] _perm;
}
  
optkdNode *KD_tree::BuildkdTree(int l, int r)
{
  optkdNode *p;
  int m;

  p = new optkdNode;

  if (r-l+1 <= KD_BUCKETSIZE)
    {
      p->bucket = 1;		// using bucket
      p->lopt = l;		// left 
      p->hipt = r;		// right
      p->lochild = 0;		// no children
      p->hichild = 0;
    }
  else
    {
      p->bucket =0;		// no bucket
      p->discrim = findmaxspread(l, r);	// find dimension having the maximum spread in values
      m = (l+r)/2;		// midpoint
      Selection(l, r, m, p->discrim); // partition points around the midpoint, m
      p->cutval = _points[_perm[m]][p->discrim];
      p->lochild = BuildkdTree(l, m); // make recursive call for left side of midpoint
      p->hichild = BuildkdTree(m+1, r);	// make recursive call for right side of midpoint
    }
  return(p);
}

/*  Kills a kd-tree to avoid memory leaks.   */
void KD_tree::KillOptTree(optkdNode *P)
{
  if (_perm != 0)
    {
      delete [] _perm;
      _perm = 0;
    }  /* delete permutation array */

  if (P==0)
    return;

  if (P->lochild != 0)
    KillOptTree(P->lochild);

  if (P->hichild != 0)
    KillOptTree(P->hichild);

  delete P;
}


void KD_tree::nnquery(KD_real *querpoint, int numNN, int Metric, int MinkP, int *found, KD_real *dist)
{
  int j;

  /* allocate memory optfound array -- note that optfound is global */
  _optfound = (int *) new int[numNN+1];
  _optfound[0]=1;  /* for now */

  /* nndist is a priority queue of the distances of the nearest neighbors found */
  _nndist = (KD_real *) new KD_real[numNN+1];

  for (j=0; j < numNN+1; j++)
    _nndist[j] = 99999999999.0;

  switch(Metric)
    {
    case KD_EUCLIDEAN:
      rnnEuclidean(_OptkdRoot, querpoint, numNN);
      break;

    case KD_MANHATTAN:
      Distance = KD_ManhattDist;
      rnnGeneral(_OptkdRoot, querpoint, numNN, MinkP);
      break;

    case KD_L_INFINITY:
      Distance = KD_LInfinityDist;
      rnnGeneral(_OptkdRoot, querpoint, numNN, MinkP);
      break;

    case KD_L_P:
      Distance = KD_LGeneralDist;
      rnnGeneral(_OptkdRoot, querpoint, numNN, MinkP);
      break;
    }

  // Get the found nodes and return distances and indices in reverse
  // order, i.e., larger distances are popped first
  for (j=0; j<numNN; j++)
    PQremove(&dist[numNN-1-j], _nndist, _optfound, &found[numNN-1-j]);

  delete [] _optfound;
  _optfound = 0;
  delete [] _nndist;
  _nndist = 0;
}


// Partition the set of points determined by l, r around k.
// If k = (l+r)/2, the partitioning is done around the median. 
// Note that after the routine, _points[_perm[j]][discrim] <= _points[_perm[k]][discrim]
// for j < k.
// Similarly, _points[_perm[l]][discrim] >= _points[_perm[k]][discrim]
// for l > k.
// The parameter discrim is the discriminating dimension where the largest
// spread occurs.  Adapted from Sedgewick's Algorithms in C (p. 128)
// Note that the net effect of Selection is to modify the _perm array.
void KD_tree::Selection(int l, int r, int k, int discrim)
{
  KD_real v;
  int t, i, j;

  assert(k >= l && k <= r);

  while (r > l)
    {
      v=_points[_perm[r]][discrim];
      i=l-1;
      j=r;
      for (;;)
	{
	  while (_points[_perm[++i]][discrim] < v)
	    ;
	  while (_points[_perm[--j]][discrim] > v && j>l)
	    ; 
	  if (i >= j)
	    break;		// the points are now partitioned about v
	  
	  // swap points i and j
	  t=_perm[i];
	  _perm[i] = _perm[j];
	  _perm[j]=t;
	}

      // swap points i and r
      t=_perm[i]; _perm[i] = _perm[r]; _perm[r]=t;

      //
      if (i>=k)
	r=i-1;
      if (i<=k)
	l=i+1;
    }
}

// Find dimension where the maximum spread occurs
int KD_tree::findmaxspread(int l, int u)
{
  int i, j, maxdim = 0;
  KD_real max =-999999999.0;
  KD_real min = 999999999.0;
  KD_real maxspread =-999999999.0;

  for (i=0; i < _dimension; i++)
    {
      max =-999999999.0;
      min = 999999999.0;
      for (j=l; j <= u; j++)
	{
	  if (max < _points[_perm[j]][i])
	    { 
	      max = _points[_perm[j]][i];
	    }
	  if (min > _points[_perm[j]][i])
	    { 
	      min = _points[_perm[j]][i];
	    }
	  if (maxspread < fabs(max-min))
	    {
	      maxspread = fabs(max-min);
	      maxdim = i;
	    }
	}
    }

  return(maxdim);
}


/* special searching algorithm to take advantage of the fact that square roots
   do not need to be evaluated */
void KD_tree::rnnEuclidean(const optkdNode *p, const KD_real *querpoint, int numNN)
{
  int i;
  int j;
  KD_real d,thisdist,val;

  if (p->bucket)
    {
      // If we're at a bucket node, search through all points in the bucket
      for (i=p->lopt; i <= p->hipt; i++)
	{
	  thisdist=0.0;
	  for (j=0; j<_dimension; j++)
	    {
	      d = (querpoint[j] - _points[_perm[i]][j]);
	      thisdist = thisdist + d*d;
	    }        

	  if (_optfound[0] < numNN)
	    {
	      PQInsert(thisdist, _perm[i], _nndist, _optfound);
	    }
	  else
	    {
	      PQreplace(thisdist, _nndist, _optfound, _perm[i]);
	    }
	}
    }
  else
    {
      val = querpoint[p->discrim] - p->cutval;
      if (val < 0)
	{
	  // The query point's value at the p->discrim coordinate is lower than the cut value
	  // so nearer neighbors would most likely be on the lo subtree
	  rnnEuclidean(p->lochild, querpoint, numNN);

	  if (_nndist[1] >= val*val)
	    {
	      // In this case there may be some near neighbors on the hi subtree
	      rnnEuclidean(p->hichild, querpoint, numNN);
	    }
	}
      else
	{
	  // The query point's value at the p->discrim coordinate is greater than the cut value
	  // so nearer neighbors would be on the hi subtree
	  rnnEuclidean(p->hichild, querpoint, numNN);

	  if (_nndist[1] >= val*val)
	    {
	      // In this case there may be some near neighbors on the lo subtree
	      rnnEuclidean(p->lochild, querpoint, numNN);
	    }
	}
    }
}


void KD_tree::rnnGeneral(const optkdNode *p, const KD_real *querpoint, int numNN, int MinkP)
{
  int i;
  KD_real thisdist,val,thisx;

  if (p->bucket)
    {
      for (i=p->lopt; i <= p->hipt; i++)
	{
	  thisdist=Distance(_points, _perm[i], querpoint, _dimension, MinkP);

	  if (_optfound[0] < numNN)
	    {
	      PQInsert(thisdist, _perm[i], _nndist, _optfound);
	    }
	  else
	    {
	      PQreplace(thisdist, _nndist, _optfound, _perm[i]);
	    }
	}
    }
  else
    {
      val = p->cutval;
      thisx=querpoint[p->discrim];
      if (thisx < val)
	{
	  rnnGeneral(p->lochild, querpoint, numNN, MinkP);
	  if (thisx + _nndist[1] > val)
	    {
	      rnnGeneral(p->hichild, querpoint, numNN, MinkP);
	    }
	}
      else
	{
	  rnnGeneral(p->hichild, querpoint, numNN, MinkP);
	  if (thisx - _nndist[1] < val)
	    {
	      rnnGeneral(p->lochild, querpoint, numNN, MinkP);
	    }
	}
    }
}

// Determines if the treenode P falls inside the rectangular query
// RectQuery.  If so, adds the array index of the point to the found array.
void KD_tree::optInRegion(const optkdNode *P, const KD_real **RectQuery, vector<int> &ptsFound)
{
  int index;
  
  for (index=P->lopt; index<=P->hipt; index++)
    {
      if (KD_ptInRect(_points[_perm[index]], _dimension, (const KD_real **)RectQuery))
	ptsFound.push_back(_perm[index]);
    }
}

// Returns true iff the hyper-rectangle defined by bounds array B
// intersects the rectangular query RectQuery.
int KD_tree::optBoundsIntersectRegion(const KD_real *B, const KD_real **RectQuery)
{
  int dc;

  for (dc=0; dc < _dimension; dc++)
    {
      if (B[2*dc] > RectQuery[dc][1] || B[2*dc+1] < RectQuery[dc][0])
	return(0);
    }

  return(1);
}


void KD_tree::optRangeSearch(const optkdNode *P, const KD_real **RectQuery, const KD_real *B, vector<int> &ptsFound)
{
  int dc, disc;
  KD_real *BHigh,*BLow;

  assert (P!=0);

  if (P->bucket)
    {   
      // P is a bucket node
      optInRegion(P, RectQuery, ptsFound);
      return;
    }

  // P is not a bucket node
  disc = P->discrim;
  BLow =  (KD_real *) new KD_real[2*_dimension];
  BHigh = (KD_real *) new KD_real[2*_dimension];

  /* copy the region B into BLow, BHigh */
  for (dc=0; dc < 2*_dimension; dc++)
    {
      BLow[dc]  = B[dc];
      BHigh[dc] = B[dc];
    }

  // Improve the Bounds for the subtrees 
  BLow[2*disc+1] = P->cutval;
  BHigh[2*disc] = P->cutval;

  if (optBoundsIntersectRegion(BLow, RectQuery))
    optRangeSearch(P->lochild, RectQuery, BLow, ptsFound);

  delete [] BLow;

  if (optBoundsIntersectRegion(BHigh, RectQuery))
    optRangeSearch(P->hichild, RectQuery, BHigh, ptsFound);

  delete [] BHigh;
}


void KD_tree::rectquery(const KD_real **RectQuery, vector<int> &pts_found)
{
  KD_real *B;
  int dc;
  vector<int> pts_found_init;

  // Allocate bounds array for two points
  B = (KD_real *) new KD_real[2*_dimension];

  // Initialize B with RectQuery. Note that B contains first the low and high x coordinates,
  // the low and high y coordinates, etc.
  for (dc =0; dc < _dimension; dc++)
    {
      B[2*dc]   = RectQuery[dc][0];
      B[2*dc+1] = RectQuery[dc][1];
    }

  optRangeSearch(_OptkdRoot, RectQuery, B, pts_found_init);

  // Sort pts_found_init
  sort(pts_found_init.begin(), pts_found_init.end());
  
  // Remove repeat points
  if (pts_found_init.size() > 0)
    pts_found.push_back(pts_found_init[0]);

  for (int i=1; i<(int)pts_found_init.size(); i++)
    {
      if (pts_found_init[i] != pts_found_init[i-1])
	pts_found.push_back(pts_found_init[i]);
    }

  delete [] B;
}
