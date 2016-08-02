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
# ifndef    SIMPLE_SPLINE_H
# define    SIMPLE_SPLINE_H
//
// This class takes some x,y tag points and fills in a simple
// spline. The code originated with Jeff Copeland. Optionally
// files named 'tag.dat' and 'spline.dat' can be written - these
// are ASCII files of the tag points and the fitted spline,
// respectively. These can be loaded into MatLab and plotted to see
// how the spline fit the points.
//
// This spline system can have problems if the points are symetrical.
// A spline fit to y = cos (x) centered on x=0 that includes the point
// x = 0 with points equally spaced on either side will have issues.
// NaNs may be returned. This is not likely for the application this code was
// developed for, so we assume that this is OK.
//
// The tag points passed in should be monotonically increasing in X.
//
// Niles Oien March 2005.
//
class SimpleSpline {
  
public:

  SimpleSpline( const double *xTag, // Input. Tag points in X
		const double *yTag, // Input. Tag points in Y
		const int numTag,   // Input. Number of tag points
		const double x0,    // Input. First X value for fill.
		const double xStep, // Input. Step in X for fill.
		const int numX,     // Input. Number of steps in fill.
		double *y,          // Output array, caller to allocate.
		const int writeAsciiFiles // Input. Option to write ASCII files of tag points, spline.
		);

  ~SimpleSpline();

protected:



private:
  //
  // Simple struct to hold the four co-efficients needed for the spline.
  //
  struct _segment{
    double a,b,c,d;
  };

  //
  // Routine that does much of the work.
  //
  void _makeSplines(struct _segment *s, 
		    const double *x, 
		    const double *y,
		    int i,
		    const int numTag);

};


# endif  
