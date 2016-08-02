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
// Module: DPbasic_main.cc
//
// Author: Gerry Wiener
//
// Date:   9/19/04
//
// Description:
//     
//----------------------------------------------------------------------

// Include files 
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <euclid/DPbasic.hh>

// Constant, macro and type definitions 
#define TWO_PI 6.28318531

// Global variables 

// Functions and objects


int main(int argc, char **argv)
{
  POINT V[10000];
  int out_points[10000];


  // Establish input
  int n = 50;
  for (int i = 0; i < n; i++)
    {
      V[i][XX] = (n / 2.0) * cos((TWO_PI * i) / n);
      V[i][YY] = (n / 2.0) * sin((TWO_PI * i) / n);
      printf("%d: %g, %g\n", i, V[i][0], V[i][1]);
    }

  double EPSILON = 1.0;

  // Create dp_basic object
  DPbasic *dp_basic = new DPbasic(V, n);  

  // Call Douglas-Peucker algorithm
  int num_pts = dp_basic->dp(0, n - 1, EPSILON, out_points);

  // Print output
  printf("num_pts = %d\n", num_pts);
  for (int i=0; i<num_pts; i++)
    {
      printf("%d: %g, %g\n", out_points[i], V[out_points[i]][0], V[out_points[i]][1]);
    }


  // Clean-up
  delete dp_basic;
  return(0);
}

