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
//   Module: kd_query.hh
//
//   Author: Gerry Wiener
//
//   Date:   9/26/04
//
//   Description: A wrapper class making it convenient to call the
//   underlying nearest neighbor functions
//
//----------------------------------------------------------------------

#ifndef KD_QUERY_HH
#define KD_QUERY_HH

#include "./kd.hh"

class Kd_query
{
public:
  Kd_query
  (
   KD_real *xdata,		// I - x coordinates of data points
   KD_real *ydata,		// I - y coordinates of data points
   int ndata,			// I - number of data points
   int metric,			// I - type of metric 
   int minkP			// I - on used with KD_L_P
   );

  virtual ~Kd_query();

  virtual int nnquery
  (
   KD_real query_pt[2],		// I - query point
   int nn_num,			// I - number of nearest neighbors desired
   int *found,			// O - array of size at least nn_num
   KD_real *dist		// O - array of size at least nn_num
   ) = 0;

  virtual void rectquery
  (
   const KD_real **RectQuery,
   vector<int> &pts_found
   ) = 0;

  virtual void get_xy
  (
   int ind,                     // index of point
   KD_real *px,                 // x coord
   KD_real *py                  // y coord
   )
  {
    *px = _A[ind][0];
    *py = _A[ind][1];
  }

protected:
  KD_real **_A;
  int _dim;
  int _metric;
  int _minkP;
  int _ndata;
};

class Kd_tree_query : public Kd_query
{
public:
  Kd_tree_query
  (
   KD_real *xdata,		// I - x coordinates of data points
   KD_real *ydata,		// I - y coordinates of data points
   int ndata,			// I - number of data points
   int metric,			// I - type of metric 
   int minkP			// I - on used with KD_L_P
   );    

  ~Kd_tree_query();

  int nnquery
  (
   KD_real query_pt[2],		// I - query point
   int nn_num,			// I - number of nearest neighbors desired
   int *found,			// O - array of size at least nn_num
   KD_real *dist			// O - array of size at least nn_num
   );

  void rectquery
  (
   const KD_real **RectQuery,
   vector<int> &pts_found
   );

protected:
  KD_tree *_kdtp;  
};

class Kd_naive_query : public Kd_query
{
public:
  Kd_naive_query
  (
   KD_real *xdata,		// I - x coordinates of data points
   KD_real *ydata,		// I - y coordinates of data points
   int ndata,			// I - number of data points
   int metric,			// I - type of metric 
   int minkP			// I - on used with KD_L_P
   );    

  ~Kd_naive_query();

  int nnquery
  (
   KD_real query_pt[2],		// I - query point
   int nn_num,			// I - number of nearest neighbors desired
   int *found,			// O - array of size at least nn_num
   KD_real *dist			// O - array of size at least nn_num
   );

  void rectquery
  (
   const KD_real **RectQuery,
   vector<int> &pts_found
   );

protected:
  KD_naive *_kdnp;  
};


#endif /* KD_QUERY_HH */
