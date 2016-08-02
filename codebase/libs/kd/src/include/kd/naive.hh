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
 *   Module: naive.hh
 *
 *   Author: Gerry Wiener
 *
 *   Date:   11/13/01
 *
 *   Description: 
 */

#ifndef NAIVE_HH
#define NAIVE_HH


#include <vector>
#include "datatype.hh"

using namespace std;

class KD_naive
{
public:
  KD_naive(const KD_real **points, int num_points, int dimension); // standard constructor
  KD_naive(const KD_naive &kdt);	// copy constructor
  KD_naive & operator=(const KD_naive &kdt); // assignment operator
  ~KD_naive();

  // The user needs to supply the query point where
  // querpoint is a multidimensional array of KD_reals having dimension, _dimension.
  // numNN is the number of natural neighbors to be output (> 1)
  // Metric 
  void nnquery(KD_real *querpoint, int numNN, int Metric, int MinkP, int *found, KD_real *dist);
  void rectquery(const KD_real **RectQuery, vector<int> &ptsFound);

  int get_num_points() { return _num_points; }
  int get_dimension() { return _dimension; }
  const KD_real ** get_points() { return _points; }
  
private:
  const KD_real **_points;
  int _num_points;
  int _dimension;

  // Makes the perm partition the array Values along the element k.
  // Adapted from Sedgewick's Algorithms in C (p. 128)
  void selection(KD_real *a, int *perm, int N, int k);
};

#endif /* NAIVE_HH */
