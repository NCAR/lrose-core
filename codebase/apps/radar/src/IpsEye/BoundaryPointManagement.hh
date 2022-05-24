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
#ifndef BOUNDARYPOINTMANAGEMENT_H
#define BOUNDARYPOINTMANAGEMENT_H


#include <vector>
#include <string>
#include <iostream>


using namespace std;

// Code copied from Soloii

class BoundaryPointManagement {

 public:
  BoundaryPointManagement();
  virtual ~BoundaryPointManagement();

 private:

    // TODO: double linked list
    BoundaryPointManagement *last;
    BoundaryPointManagement *next;
    long x;                     /* meters */
    long y;                     /* meters */
    long z;                     /* meters */
    long r;                     /* range in meters */

    // this is a linked list of intersections; traverse using next_intxn
    // the first intersection of the list is kept in OneBoundary
    BoundaryPointManagement *last_intxn; // last intersection
    BoundaryPointManagement *next_intxn; // next intersection

    BoundaryPointManagement *x_parent;
    BoundaryPointManagement *x_left;
    BoundaryPointManagement *x_right;

    BoundaryPointManagement *y_parent;
    BoundaryPointManagement *y_left;
    BoundaryPointManagement *y_right;

    float slope;
    float slope_90;             /* slope of line perpendicular                                                
                                 * to this line*/
    float len;

    long x_mid;                 /* midpoint of the line */
    long y_mid;                 /* midpoint of the line */
    long dy;                    /* last->y - this->y */
    long dx;                    /* last->x - this->x */
    long rx;                    /* intersection with a ray */

    int bnd_num;
    int segment_num;
    int what_happened;
    int which_frame;
    int which_side;
    int mid_level;
    struct point_in_space *pisp;
    long _x;            /* x shifted */
    long _y;            /* y shifted */
    long _z;            /* z shifted */
    /*                                                                                                        
     * time series stuff...range and time will be in pisp.                                                    
     */
    double t_mid;
    float dt;
    float r_mid;
    float dr;

    int screen_x;
    int screen_y;

};

#endif
