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
//   $Id: ClutterRemover.cc,v 1.8 2016/03/06 23:28:57 dixon Exp $
//   $Revision: 1.8 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ClutterRemover.cc: Class implementing the clutter removal algorithm
 *                    used by ctrec.
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

#include <toolsa/os_config.h>

#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/globals.h>

#include "ClutterRemover.hh"
#include "DataDetrender.hh"
using namespace std;

/**********************************************************************
 * Constructor - Sets up the values used in the clutter removal algorithm.
 *
 *      variance_thresh = variance threshhold.  Grid squares with a
 *                        variance greater than this value will be set
 *                        to missing.
 *      variance_radius = defines the number of grid squares around the
 *                        current grid square to be included in the
 *                        variance calculations.  If this is less than
 *                        2, clutter removal will NOT be performed.
 *      min_data_val =    minimum data value to be included in the
 *                        variance calculations.
 *      
 */

ClutterRemover::ClutterRemover(const double variance_thresh,
			       const int variance_radius,
			       const double min_data_value,
			       const double max_data_value,
			       DataDetrender *detrender,
			       const bool debug_flag) :
  _debugFlag(debug_flag),
  _minDataValue(min_data_value),
  _maxDataValue(max_data_value),
  _varianceThresh(variance_thresh),
  _varianceRadius(variance_radius),
  _detrender(detrender),
  _variance(0),
  _nx(0),
  _ny(0)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

ClutterRemover::~ClutterRemover(void)
{
  delete [] _variance;
}
  

/**********************************************************************
 * run() - Remove the clutter from the given image.
 */

void ClutterRemover::run(MdvxField &field,
			 const double bad_data_value)
{
  static const string method_name = "ClutterRemover::run()";
  
  // Make sure the data is encoded in FLOAT32 format.

  if (field.getFieldHeader().encoding_type != Mdvx::ENCODING_FLOAT32)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Data must be in FLOAT32 format to do clutter removal" << endl;
    cerr << "Clutter not removed" << endl;
    
    return;
  }
  
  // Call the run() method that does the actual work.

  run ((fl32 *)field.getVol(),
       field.getFieldHeader().nx, field.getFieldHeader().ny,
       bad_data_value);
  
}


void ClutterRemover::run(fl32 *image, const int nx, const int ny,
			 const double bad_data_value)
{
  static const string method_name = "ClutterRemover::run()";
  
  // Don't do anything to the image if the variance radius is too
  // small.

  if (_varianceRadius < 2)
    return;
  
  // Allocate space for the variance grid.

  if (nx != _nx || ny != _ny)
  {
    delete [] _variance;
    _variance = new double[nx * ny];
    
    _nx = nx;
    _ny = ny;
  }
  
  /*
   * Do ground clutter removal.
   */

  // Fill data void or weak signal regions with random values
  // if requested.

  if (_debugFlag)
    cout << "thresholding dz on standard deviation......" << endl;
  
  if (_debugFlag && _detrender != 0)
    cout << "will detrend data" << endl;
  
  // Process each point in the grid

  for (int x = 0; x < _nx; ++x)
  {
    for (int y = 0; y < _ny; ++y)
    {
      int index = x + (y * _nx);
      
      _variance[index] = bad_data_value;
      
      int min_x = MAX(x - _varianceRadius, 0);
      int max_x = MIN(x + _varianceRadius, _nx - 1);
      int min_y = MAX(y - _varianceRadius, 0);
      int max_y = MIN(y + _varianceRadius, _ny - 1);
      
      int num_pts = 0;
      double sum = 0.0;
      double sumsq = 0.0;
      
      if (_detrender != 0)
      {
	if (!_detrender->run(image, _nx, _ny, bad_data_value,
			     min_x, max_x, min_y, max_y, x, y))
	{
	  continue;
	}
      }
      
      // Compute the variance around the grid point

      for (int xx = min_x; xx <= max_x; ++xx)
      {
	for (int yy = min_y; yy <= max_y; ++yy)
	{
	  double data_value = image[xx + (yy * _nx)];
	  
	  if (data_value < _minDataValue ||
	      data_value > _maxDataValue)
	    continue;
	  
	  if (_detrender != 0)
	    data_value -= _detrender->getLsfValue(xx, yy);
	  
	  num_pts++;
	  sum += data_value;
	  sumsq += (data_value * data_value);
	  
	} /* endfor - yy */
      } /* endfor - xx */
      
      if (num_pts >= 8)
      {
	double num_pts_dbl = (double)num_pts;
	
	_variance[index] = (sumsq / num_pts_dbl) -
	  ((sum * sum) / (num_pts_dbl * num_pts_dbl));
      }
      
    } /* endfor - y */
  } /* endfor - x */
  
  // Replace data with bad value if variance greater than threshold
  
  for (int i = 0; i < _nx * _ny; ++i)
  {
    if (_variance[i] > _varianceThresh)
      image[i] = bad_data_value;
  }
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
