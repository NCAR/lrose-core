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
//   Module: DPbasic.hh
//
//   Author: Gerry Wiener
//
//   Date:   9/19/04
//
//   Description: 
//----------------------------------------------------------------------

#ifndef DPBASIC_HH
#define DPBASIC_HH

typedef double POINT[2];	/* Most data are cartesian points */
typedef double HOMOG[3];	/* Some partial calculations are homogeneous */


#define XX 0
#define YY 1
#define WW 2


// 2-d cartesian to homog cross product 
#define CROSSPROD_2CCH(p, q, r)  (r)[WW] = (p)[XX] * (q)[YY] - (p)[YY] * (q)[XX];\
 (r)[XX] = - (q)[YY] + (p)[YY];\
 (r)[YY] =   (q)[XX] - (p)[XX];

// 2-d cartesian to homog dot product 
#define DOTPROD_2CH(p, q)  (q)[WW] + (p)[XX]*(q)[XX] + (p)[YY]*(q)[YY]


// 2-d cartesian  dot product 
#define DOTPROD_2C(p, q) (p)[XX]*(q)[XX] + (p)[YY]*(q)[YY]

// 2-d cartesian linear combination 
#define LINCOMB_2C(a, p, b, q, r)  (r)[XX] = (a) * (p)[XX] + (b) * (q)[XX];\
 (r)[YY] = (a) * (p)[YY] + (b) * (q)[YY];


class DPbasic
{
 public:

  // Constructor
  DPbasic
  (
   POINT *Vert,			// I - array of input points
   int size			// I - size of Vert
   )
    {
      _V = Vert;
      _out_ct = 0;
      _out_pts = 0;
    }

  ~DPbasic()
    {
    }

  // The Douglas-Peucker algorithm
  int dp
  (
   int i,			// I - index of first input point
   int j,			// I - index of second input point
   double epsilon,		// I - tolerance used to determine whether to split or not
   int *out_points		// O - array of output point indices
   );



 private:
  int _out_ct;			// the number of points in out_pts
  int *_out_pts;		// array of output points 
  POINT *_V;			// array of input points

void findSplit
(
 int i,				// I - index of first input point
 int j,				// I - index of second input point
 int *split,			// O - index of the farthest point
 double *dist			// O - distance of farthest point
);

void initOutput
(
 int *out_points		// I - array of output points
);

void Output
(
 int i,				// I - index of first input point
 int j				// I - index of second input point
);

  int sizeOutput();
};




#endif /* DPBASIC_HH */
