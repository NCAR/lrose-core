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
#ifndef ONEBOUNDARY_H
#define ONEBOUNDARY_H


#include <vector>
#include <string>
#include <iostream>


using namespace std;

// Code copied from Soloii

class OneBoundary {

 public:
  OneBoundary();
  virtual ~OneBoundary();
  void addBoundaryPoint(int x, int y);

 private:

  // TODO: use a vector?
  OneBoundary *last;
  OneBoundary *next;

  // TODO: use a stack?
  // What do all of these mean?  top, x, y, first, next, last?
  BoundaryPointManagement *top_bpm;
  BoundaryPointManagement *x_mids;
  BoundaryPointManagement *y_mids;
  BoundaryPointManagement *first_intxn; // first intersection 
  BoundaryPointManagement *next_segment;
  BoundaryPointManagement *last_line;
  BoundaryPointManagement *last_point;

  float r0;
  float r1;
  int num_segments;
  int num_intxns;  // number of intersections
  int num_points;
  double min_x;                       /* meters */
  double max_x;                       /* meters */
  double min_y;                       /* meters */
  double max_y;                       /* meters */
  double min_z;                       /* meters */
  double max_z;                       /* meters */
  int radar_inside_boundary;  // boolean if radar is inside this boundary
  int open_boundary;
  BoundaryHeader *bh;


};

#endif
