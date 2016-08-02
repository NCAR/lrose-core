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
/*************
 * uNewtRaph.cc
 *
 * User must provide function funcd, which returns the funcion value
 * and derivative at point x.
 *
 * funcd: method that computes the value and derivate of the function.
 *   typedef void (*NewtRaphFunc)(double x, double *val, double *deriv);
 *
 * xLower, xUpper: values bracketing the desired root.
 * accuracy: accuracy of test for root.
 * root_p: pointer to root to be returned.
 */

#include <math.h>
#include <stdio.h>
#include <rapmath/umath.h>

#define MAX_ITERATIONS 10000

int uNewtRaph(NewtRaphFunc myFunc,
	      double xLower, double xUpper,
	      double accuracy,
	      double *root_p)
     
{

  // compute function values for the bracketing values

  double fUpper, fLower;
  double derivLower, derivUpper;

  myFunc(xLower,&fLower,&derivLower);
  myFunc(xUpper,&fUpper,&derivUpper);

  if (fLower*fUpper >= 0.0) {
    fprintf(stderr, "ERROR - uNewtRaph\n");
    fprintf(stderr, "  Root must be bracketed\n");
    fprintf(stderr, "  f(x) at lower limit %g is %g\n", xLower, fLower);
    fprintf(stderr, "  f(x) at upper limit %g is %g\n", xUpper, fUpper);
    return (-1);
  }

  double xxl, xxh;

  if (fLower < 0.0) {
    xxl = xLower;
    xxh = xUpper;
  } else {
    xxh = xLower;
    xxl = xUpper;
    double swap = fLower;
    fLower = fUpper;
    fUpper = swap;
  }

  // initialize with center of range

  double rts = 0.5 *(xLower + xUpper);
  double dxPrev = fabs(xUpper - xLower);
  double dx = dxPrev;
  double f, deriv;
  myFunc(rts,&f,&deriv);
  
  // now iterate, searching for root

  for (int j = 1; j <= MAX_ITERATIONS; j++) {

    double fac1 = (rts - xxh) * deriv - f;
    double fac2 = (rts - xxl) * deriv - f;
    
    if (((fac1 * fac2) >= 0.0) ||
        (fabs(2.0 * f) > fabs(dxPrev* deriv))) {

      dxPrev = dx;
      dx = 0.5 * (xxh - xxl);
      rts = xxl + dx;
      if (xxl == rts) {
	*root_p = rts;
	return 0;
      }

    } else {

      dxPrev = dx;
      dx = f / deriv;
      double temp = rts;
      rts -= dx;
      if (temp == rts) {
	*root_p = rts;
	return 0;
      }
      
    }

    if (fabs(dx) < accuracy) {
      *root_p = rts;
      return 0;
    }

    // another try

    myFunc(rts,&f,&deriv);
    if (f < 0.0) {
      xxl = rts;
      fLower = f;
    } else {
      xxh = rts;
      fUpper = f;
    }

  } // for

  fprintf(stderr, "ERROR - uNewtRaph\n");
  fprintf(stderr, "Maximum number of iterations exceeded\n");
  
  return -1;
  
}

