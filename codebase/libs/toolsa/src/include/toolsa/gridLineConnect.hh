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

#ifndef GRID_LINE_CONNECT
#define GRID_LINE_CONNECT

#include <cmath>

using namespace std;
//
// This is a class that enables the caller to get the
// points in a line connecting two points in an integer
// grid. For instance, to get the connecting points on the
// line (x1 = -10, y1 = -5) to the point (x1 = 10, y1 = 5)
// the following code could be used :
//
//   gridLineConnect G(-10, -5, 10, 5);
//   int morePoints;
//   do {
//
//     int ix, iy;
//     double t;
//
//     morePoints = G.nextPoint(ix, iy, t);
//
//     fprintf(stderr,"x=%d y=%d fractionDone=%g\n", ix, iy, t);
//
//   } while (morePoints);
//
// Which results in the following output :
//
// x=-10 y=-5 fractionDone=0
// x=-9 y=-5 fractionDone=0.05
// x=-8 y=-4 fractionDone=0.1
// x=-7 y=-3 fractionDone=0.15
// x=-6 y=-3 fractionDone=0.2
// x=-5 y=-3 fractionDone=0.25
// x=-4 y=-2 fractionDone=0.3
// x=-3 y=-1 fractionDone=0.35
// x=-2 y=-1 fractionDone=0.4
// x=-1 y=-1 fractionDone=0.45
// x=0 y=0 fractionDone=0.5
// x=1 y=1 fractionDone=0.55
// x=2 y=1 fractionDone=0.6
// x=3 y=1 fractionDone=0.65
// x=4 y=2 fractionDone=0.7
// x=5 y=3 fractionDone=0.75
// x=6 y=3 fractionDone=0.8
// x=7 y=3 fractionDone=0.85
// x=8 y=4 fractionDone=0.9
// x=9 y=5 fractionDone=0.95
// x=10 y=5 fractionDone=1
//
// Niles Oien June 2004
//
class gridLineConnect {
  
public:
  //
  // Constructor. Pass in the two points to be connected.
  //
  gridLineConnect(int x1, int y1, int x2, int y2);
  // 
  // Method to return the integer x,y of the next point.
  // returns 0 if this is the last point, else 1.
  //
  int nextPoint(int &ix, int &iy, double &fractionDone);
  //
  // Destructor.
  //
  ~gridLineConnect();
  //
  //
protected:
  
private:

  int _x1, _y1, _x2, _y2;
  int _x, _y;
  int _steppingInX;
  int _inc;
  double _DyOnDx, _DxOnDy;

};

#endif
