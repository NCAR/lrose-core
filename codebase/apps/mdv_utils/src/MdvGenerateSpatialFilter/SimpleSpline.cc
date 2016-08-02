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

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <cstdio>
#include "SimpleSpline.hh"

void SimpleSpline::_makeSplines(struct SimpleSpline::_segment *s, 
				const double *x, 
				const double *y,
				int i,
				const int numTag) {

  double fa,fb,ffa,ffb; //these are the first and second derivatives
  
  // calculate first derivatives
  if (i == 0) {
    fb = 2.0/( ((x[2]-x[1])/(y[2]-y[1])) + ((x[1]-x[0])/(y[1]-y[0])) );
    fa = 1.5*((y[1]-y[0])/(x[1]-x[0])) - 0.5*fb;
  } else if (i == numTag-2) {
    fa = 2.0/( ((x[i+1]-x[i])/(y[i+1]-y[i])) + ((x[i]-x[i-1])/(y[i]-y[i-1])) );
    fb = 1.5*((y[i+1]-y[i])/(x[i+1]-x[i])) - 0.5*fa;
  } else {
    fa = 2.0/( ((x[i+1]-x[i])/(y[i+1]-y[i])) + ((x[i]-x[i-1])/(y[i]-y[i-1])) );
    fb = 2.0/( ((x[i+2]-x[i+1])/(y[i+2]-y[i+1])) + ((x[i+1]-x[i])/(y[i+1]-y[i])) );
  }

  // now second derivatives
  ffa = -2.0*(fb + 2.0*fa)/(x[i+1]-x[i]) + 6.0*(y[i+1]-y[i])/((x[i+1]-x[i])*(x[i+1]-x[i]));
  ffb = 2.0*(2.0*fb + fa)/(x[i+1]-x[i]) - 6.0*(y[i+1]-y[i])/((x[i+1]-x[i])*(x[i+1]-x[i]));

  //now compute the cubic parameters
  s->d = (ffb - ffa)/(6.0*(x[i+1]-x[i]));
  s->c = 0.5*(x[i+1]*ffa - x[i]*ffb)/(x[i+1]-x[i]);
  s->b = ((y[i+1]-y[i]) - s->c*(x[i+1]*x[i+1]-x[i]*x[i]) - s->d*(x[i+1]*x[i+1]*x[i+1]-x[i]*x[i]*x[i]))/(x[i+1]-x[i]);
  s->a = y[i] - s->b*x[i] - s->c*x[i]*x[i] - s->d*x[i]*x[i]*x[i];
}


SimpleSpline::SimpleSpline( const double *xTag, // Input. Tag points in X
			    const double *yTag, // Input. Tag points in Y
			    const int numTag,   // Input. Number of tag points
			    const double x0,    // Input. First X value for fill.
			    const double xStep, // Input. Step in X for fill.
			    const int numX,     // Input. Number of steps in fill.
			    double *y,          // Output array, caller to allocate.
			    const int writeAsciiFiles // Input. Option to write ASCII files of tag points, spline.
			    ){
  //
  // If desired, write ASCII files of tag points.
  //
  FILE *fp;
  if (writeAsciiFiles){
    fp = fopen("tag.dat","w");
    for (int itag = 0; itag < numTag; itag++){
      fprintf(fp,"%g %g\n",xTag[itag],yTag[itag]);
    }
    fclose (fp);
  }
  //
  // Allocate space for the co-efficients.
  //
  struct SimpleSpline::_segment *spline = (struct SimpleSpline::_segment *) malloc((numTag-1)*sizeof(struct SimpleSpline::_segment));
  //
  // Get the co-efficients.
  //
  for (int is = 0; is < numTag-1; is++){
    _makeSplines(&spline[is], xTag, yTag, is, numTag);
  }
  //
  // Init the y values to 0.0
  //
  for (int iy=0; iy < numX; iy++){
    y[iy]=0.0;
  }
  //
  // OK - now you have to loop through the X values, find what segment
  // you are in, and get the value. Write these to an ASCII file if requested.
  //
  if (writeAsciiFiles){
    fp = fopen("spline.dat","w");
  }
  for (int ix = 0; ix < numX; ix++){
    double x = x0 + ix * xStep;
    int iSegment = -1;
    do {
      iSegment++;
    } while ((iSegment != numTag-1) && (xTag[iSegment] < x));

    if (iSegment > 0) iSegment --;

    y[ix] = spline[iSegment].a + spline[iSegment].b*x + 
      spline[iSegment].c*x*x + spline[iSegment].d*x*x*x;

    if (writeAsciiFiles){
      fprintf(fp,"%g %g\n",x, y[ix]);
    }
  }

  if (writeAsciiFiles){
    fclose (fp);
  }

  free(spline);

  return;

}

SimpleSpline::~SimpleSpline(){
  return;
}


