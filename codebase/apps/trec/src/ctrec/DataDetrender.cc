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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/06 23:28:57 $
//   $Id: DataDetrender.cc,v 1.4 2016/03/06 23:28:57 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DataDetrender.cc: class implementing an algorithm to detrend data.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <iostream>
#include <math.h>

#include <toolsa/os_config.h>

#include "DataDetrender.hh"
using namespace std;



/**********************************************************************
 * Initialize static constants.
 */

const double DataDetrender::EPSILON = 0.0001;
  

/**********************************************************************
 * Constructor
 */

DataDetrender::DataDetrender(const double thr_dbz,
			     const bool debug_flag) :
  _debugFlag(debug_flag),
  _lsfGrid(0),
  _nx(0),
  _ny(0),
  _badDataValue(0),
  _thrDbz(thr_dbz)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

DataDetrender::~DataDetrender(void)
{
  delete [] _lsfGrid;
}
  

/**********************************************************************
 * run() - Detrend the data in the given image.
 *
 * Returns true if the detrending was successful, false otherwise.
 */

bool DataDetrender::run(const float *image,
			const int nx, const int ny,
			const double bad_data_value,
			const int min_x, const int max_x,
			const int min_y, const int max_y,
			const int x_center, const int y_center)
{
  // Allocate space for the least squares fit grid and save needed
  // values.

  if (_nx != nx || _ny != ny)
  {
    delete [] _lsfGrid;
    _lsfGrid = new double[nx * ny];
    
    _nx = nx;
    _ny = ny;
  }
  
  _badDataValue = bad_data_value;
  
  // Initialize summation variables for least squares plane fit

  int num_pts = 0;
  double sum_x = 0.0;
  double sum_y = 0.0;
  double sum_x2 = 0.0;
  double sum_y2 = 0.0;
  double sum_data = 0.0;
  double sum_data_x = 0.0;
  double sum_data_y = 0.0;
  double sum_xy = 0.0;
  
  // Loop through the data, ignoring data below the threshold

  for (int y = min_y; y <= max_y; ++y)
  {
    int y_dist = y - y_center;
    
    for (int x = min_x; x <= max_x; ++x)
    {
      int x_dist = x - x_center;
      int index = x + (_nx * y);
      
      _lsfGrid[index] = _badDataValue;
      
      if (image[index] < _thrDbz)
	continue;
      
      ++num_pts;
      sum_x += x_dist;
      sum_y += y_dist;
      
      sum_xy += (x_dist * y_dist);
      sum_x2 += (x_dist * x_dist);
      sum_y2 += (y_dist * y_dist);
      
      sum_data += image[index];
      sum_data_x += (x_dist * image[index]);
      sum_data_y += (y_dist * image[index]);
      
    } /* endfor - x */
  } /* endfor - y */
  
  // Return if there weren't enough points in the totals

  if (num_pts < 5)
    return false;
  
  double t1 = (sum_x2 * sum_y2) - (sum_xy * sum_xy);
  double t2 = (sum_x * sum_y2) - (sum_xy * sum_y);
  double t3 = (sum_x * sum_xy) - (sum_x2 * sum_y);
  double denom = ((double)num_pts * t1) - (sum_x * t2) + (sum_y * t3);
  
  // Make sure the denominator is big enough

  if (fabs(denom) < EPSILON)
    return false;
  
  double anum = (sum_data * t1) - (sum_data_x * t2) + (sum_data_y * t3);
  double bnum = ((double)num_pts *
    ((sum_data_x * sum_y2) - (sum_data_y * sum_xy))) -
    (sum_data * ((sum_x * sum_y2) - (sum_xy * sum_y))) +
    (sum_y * ((sum_x * sum_data_y) - (sum_y * sum_data_x)));
  double cnum = ((double)num_pts *
    ((sum_x2 * sum_data_y) - (sum_xy * sum_data_x))) -
    (sum_x * ((sum_x * sum_data_y) - (sum_y * sum_data_x))) +
    (sum_data * ((sum_x * sum_xy) - (sum_y * sum_x2)));
  
  double a = anum / denom;
  double b = bnum / denom;
  double c = cnum / denom;
  
  // Fill in the _lsfGrid array with a least squares fit to the data

  for (int y = min_y; y <= max_y; ++y)
  {
    int y_dist = y - y_center;
    
    for (int x = min_x; x <= max_x; ++x)
    {
      int x_dist = x - x_center;
      int index = x + (_nx * y);
      
      _lsfGrid[index] = a + (b * x_dist) + (c * y_dist);
      
    } /* endfor - x */
  } /* endfor - y */
  
  return true;
}

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
