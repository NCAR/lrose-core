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
///////////////////////////////////////////////////////////////
// GridAdvect.cc
//
// GridAdvect class
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2001
//
///////////////////////////////////////////////////////////////

#include <advect/GridAdvect.hh>
#include <euclid/CircularTemplate.hh>
#include <euclid/EllipticalTemplate.hh>
#include <euclid/Pjg.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>


//////////////
// Constructor

GridAdvect::GridAdvect(const double image_val_min,
		       const double image_val_max,
		       const bool debug_flag) :
  _debugFlag(debug_flag),
  _imageValMin(image_val_min),
  _imageValMax(image_val_max),
  _checkImageValues(image_val_min < image_val_max),
  _forecastData(0)
{
  // Do nothing
}

/////////////
// destructor

GridAdvect::~GridAdvect()
{
  delete [] _forecastData;
}

////////////////////////////
// compute()
//
// Compute the forecast grid.
//
// Returns true on success, false on failure

bool GridAdvect::compute(Advector &advector,
			 const Pjg &projection,
			 const fl32 *image_data,
			 const fl32 missing_data_value)
{
  // Create the forecast field
  
  int nx, ny, nz;
  projection.getGridDims(nx, ny, nz);
  
  delete [] _forecastData;
  _forecastData = new fl32[nx * ny];
  for (int i = 0; i < nx * ny; ++i)
    _forecastData[i] = missing_data_value;
  
  _forecastProj = projection;
  
  // load up forecast grid using image data and forecast vectors

  fl32 *forecast = _forecastData;
  fl32 *image = (fl32 *)image_data;
  
  for (int x_index = 0; x_index < nx; x_index++)
  {
    PMU_auto_register("GridAdvect::compute...computing ...");

    for (int y_index = 0; y_index < ny; y_index++)
    {
      int fcst_index = advector.calcFcstIndex(x_index, y_index);
      
      if (fcst_index < 0)
	continue;
      
      int index = x_index + (y_index * nx);
      
      if (_checkImageValues)
      {
	if (image[fcst_index] >= _imageValMin &&
	    image[fcst_index] <= _imageValMax &&
	    image[fcst_index] != missing_data_value)
	  forecast[index] = image[fcst_index];
      }
      else
      {
	if (image[fcst_index] != missing_data_value)
	  forecast[index] = image[fcst_index];
      }
	
    } /* endfor - y_index */
  } /* endfor - x_index */
    
  return true;
}
