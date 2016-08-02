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
//----------------------------------------------------------------------
//   Module: kd_interp.hh
//
//   Author: Gerry Wiener
//
//   Date:   8/27/02
//
//   Description: 
//----------------------------------------------------------------------

#ifndef KD_INTERP_HH
#define KD_INTERP_HH

#include "./kd.hh"

namespace KD
{
  const int MIN = 0;
  const int MAX = 1;
}


// Nearest neighbor interpolation for two dimensions.  Note that there
// are nx_grid * ny_grid grid points in the cartesian product
// determined by xgrid and ygrid.
void kd_interp
(
 KD_real *xdata,			// I - x coordinates of data points
 KD_real *ydata,			// I - y coordinates of data points
 int ndata,			// I - number of data points
 KD_real *data_val,		// I - values at data points
 KD_real *xgrid,			// I - x coordinates of interp grid
 KD_real *ygrid,			// I - y coordinates of interp grid
 int xdim,			// I - size of xgrid
 int ydim,			// I - size of ygrid
 KD_real *grid_val		// O - interp grid values (should have dimension nx_grid * ny_grid)
 );


// Rectangular nearest neighbor interpolation for two dimensions.  For
// each grid point, determine the set N of neighbors who are within
// dist_crit of the grid point.  Set the value of the grid point to
// either the min or max value of the neighbors in N.  Note that there
// are nx_grid * ny_grid grid points in the cartesian product
// determined by xgrid and ygrid. If there are no nearest neighbors to
// a grid point within dist_crit, do not assign a value to the grid
// point.
void kd_rect_interp
(
 KD_real *xdata,		// I - x coordinates of data points
 KD_real *ydata,		// I - y coordinates of data points
 int ndata,			// I - number of data points
 KD_real *data_val,		// I - values at data points
 KD_real *xgrid,		// I - x coordinates of interp grid
 KD_real *ygrid,		// I - y coordinates of interp grid
 int xdim,			// I - size of xgrid
 int ydim,			// I - size of ygrid
 KD_real dist_crit,             // I - distance criterion from grid point
 int sel,                       // I - selection criterion (KD::MIN, KD::MAX)
 KD_real *grid_val		// O - interp grid values (should have dimension nx_grid * ny_grid)
 );

#endif /* KD_INTERP_HH */
