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
  Non-recursive implementation of the Douglas Peucker line simplification algorithm.

 				1-26-94      Jack Snoeyink

  See DPSimp at http://www.cs.ubc.ca/spider/snoeyink/papers/.

  Converted to C++ and revised by Gerry Wiener.
*/

#include <stack>
#include <euclid/DPbasic.hh>

using namespace std;

// Assumes that the polygonal line is in array V.  main()
// assumes also that a global variable n contains the number of points
// in V.

// Basic DP chain simplification from vertex i to vertex j. Output
// indices are placed in out_points. Returns -1 if i>=j. Otherwise
// returns the number of points in the output array, out_points.
int DPbasic::dp
(
 int i,			// I - index of first input point
 int j,			// I - index of second input point
 double epsilon,	// I - tolerance used to determine whether to split or not
 int *out_points	// O - array of output point indices
 )
{
  double dist_sq = -1;
  double epsilon_sq = epsilon * epsilon;
  int split; 
  stack<int> st;

  if (i >= j)
    return(-1);
  else if (j == i+1)
    {
      Output(i, j);
      return(2);
    }

  initOutput(out_points);
  st.push(j);
  do
    {
      findSplit(i, st.top(), &split, &dist_sq);
      if (dist_sq > epsilon_sq)
	{
	  st.push(split);
	}
      else
	{
	  Output(i, st.top()); // output segment Vi to Vtop 
	  i = st.top();
	  st.pop();
	}
    }
  while (!st.empty());

  return(sizeOutput());
}

// Linear search for farthest point from the segment Vi to Vj. returns
// squared distance in dist and the index of the farthest point in split.
void DPbasic::findSplit
(
 int i,				// I - index of first input point
 int j,				// I - index of second input point
 int *split,			// O - index of the farthest point
 double *dist			// O - distance of farthest point
 ) 
{
  int k;
  HOMOG q;
  double tmp;

  *dist = -1;
  if (i + 1 < j)
    {
      // out of loop portion of distance computation
      CROSSPROD_2CCH(_V[i], _V[j], q); 
				     
      for (k = i + 1; k < j; k++)
	{
	  tmp = DOTPROD_2CH(_V[k], q); // distance computation of a vertex with the line between _V[i] and _V[j]
	  if (tmp < 0)
	    tmp = - tmp; // calling fabs() slows us down 

	  if (tmp > *dist) 
	    {
	      *dist = tmp;	// record the maximum 
	      *split = k;
	    }
	}
      *dist *= *dist/(q[XX]*q[XX] + q[YY]*q[YY]); // correction for segment, length --- should be redone if can == 0 
    }				   
}

// Initialize output count and output array
void DPbasic::initOutput
(
 int *out_points		// I - array of output points
 )
{
  _out_ct = 0;
  _out_pts = out_points;
}

int DPbasic::sizeOutput()
{
  return _out_ct;
}

void DPbasic::Output
(
 int i,				// I - index of first input point
 int j				// I - index of second input point
 )
{
  if (_out_ct == 0)
    {
      _out_pts[_out_ct] = i;
      _out_ct++;
    }

  _out_pts[_out_ct] = j;
  _out_ct++;
}

