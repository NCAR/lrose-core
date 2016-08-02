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
/////////////////////////////////////////////////////////
// Quasi.hh
////////////////////////////////////////////////////////
#ifndef _QUASI
#define _QUASI

#include <dataport/port_types.h>		// for fl32
using namespace std;

//
// Forward class declarations
//

/**
 *  Quasi
 *  This class converts WAFS quasi-rectangular grids to regular rectangular grids.
 *  The quasi-grids have shorter row lengths as you approach the poles.
 *  The code here was converted and adapted from Unidata code (quasi.c) in the
 *  grib decoder "gribtonc", dating from 1995.  The conversion to C++
 *  was done by Carl Drews in April 2004.
 */
class Quasi {

public:

  Quasi();
  virtual ~Quasi();

  /// Regrid data from one equally-spaced grid to a different equally-spaced grid.
  /// \param y values of a function defined on an equally-spaced domain, y[0], ..., y[n]
  /// \param n number of input y values
  /// \param v output values regridded onto new equally-spaced grid over same domain
  /// \param m number of output v values
  /// \param c m precomputed interpolation coefficients:
  ///                                 for (j=0; j<m; j++)
  ///                                     c[j] = (double) (m - j -1) / (m - 1);
  static void linear(fl32 y[], int n, fl32 v[], int m, double c[]);

  /// Convert an irregular grid to a regular grid.
  /// \param nrows number of rows in input
  /// \param ix row i starts at idat[ix[i]], and ix[nrows] is 1 after last elem of idat
  /// \param idat input quasi-regular data
  /// \param ni constant length of each output row
  /// \param nj number of output rows
  /// \param odat where to put ni*nj outputs (already allocated)
  static void qlin(int nrows, int ix[], fl32 idat[], int ni, int nj, fl32 odat[]);

};

#endif

