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
/*********************************************************************
 * get_contour_intervals.c
 *
 * reads the parameters or x data base for contour intervals
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Rview.hh"
using namespace std;

void get_contour_intervals()

{

  si32 ifield, icontour;
  si32 ncontours;
  double val;

  if (Glob->verbose) {
    fprintf(stderr, "** get_contour_intervals **\n");
  }

  /*
   * set up contour structs
   */

  for (ifield = 0; ifield < Glob->nfields; ifield++) {

    field_control_t &fcontrol = Glob->fcontrol[ifield];
    val = fcontrol.contour_min;

    if (fcontrol.contour_int <= 0) {
      ncontours = 1;
    } else {
      ncontours = 0;
      while (val <= (fcontrol.contour_max + 1.0e-5)) {
	ncontours++;
	val += fcontrol.contour_int;
      }
    }

    fcontrol.contours.n = ncontours;
    fcontrol.contours.value = (double *) umalloc
      ((ui32) ncontours * sizeof(double));

    val = fcontrol.contour_min;

    for (icontour = 0; icontour < ncontours; icontour++) {
      fcontrol.contours.value[icontour] = (double) val;
      val += fcontrol.contour_int;
    }

  } /* ifield */

}

