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
#ifndef BOUNDARYSTUFF_H
#define BOUNDARYSTUFF_H


#include <vector>
#include <string>
#include <iostream>


using namespace std;

// Code copied from Soloii

class BoundaryStuff {

 public:
  BoundaryStuff();
  virtual ~BoundaryStuff();

 private:

    OneBoundary *first_boundary;

    BoundaryHeader *bh;
    struct point_in_space *origin;

    int operate_outside_bnd;    /* zero implies inside the boundary                                           
                                 * non-zero implies outside the boundary */

    int num_points;             /* used to indicate the presence or                                           
                                 * absence of a boundary(s) */

    int line_thickness;         /* in pixels */
    int sizeof_mod_list;
    int num_mods;
    int last_operation;
    BoundaryPointManagement **mod_list;
    BoundaryPointManagement *last_line;
    BoundaryPointManagement *last_point;

    char directory_text[128];
    char file_name_text[128];
    char comment_text[88];
    char last_boundary_point_text[88];

    int num_boundaries;
    int total_boundaries;
    OneBoundary *current_boundary;
    int proximity_parameter;    /* the mouse is considered to be within                                       
                                 * the proximity of a line if it is within                                        
                                 * a rectangle centered on the line                                           
                                 * whose length is the length                                                 
                                 * of the line and whose width is two                                         
                                 * times the PP unless the line width is                                      
                                 * less than the PP. Then the PP is defined                                   
                                 * as .666 times the length of the line */
    struct point_in_space *pisp;
    long linked_windows[16];
    int view_bounds;
    int absorbing_boundary;
};
#endif
