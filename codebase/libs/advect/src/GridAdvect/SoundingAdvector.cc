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
// SoundingAdvector.cc
//
// SoundingAdvector class
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2001
//
///////////////////////////////////////////////////////////////

#include <advect/SoundingAdvector.hh>
#include <euclid/Pjg.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/str.h>


//////////////
// Constructor

SoundingAdvector::SoundingAdvector(const bool debug_flag) :
  _debugFlag(debug_flag)
{
  if (_debugFlag) {
    cerr << "In debug mode" << endl;
  }
}

/////////////
// destructor

SoundingAdvector::~SoundingAdvector()

{
  // Do nothing
}

////////////////////////////
// loadSounding()
//
// Load the sounding to be used for the forecast.
//
// Returns true on success, false on failure

bool SoundingAdvector::loadSounding(const double u_comp, const double v_comp)
{
  _uComp = u_comp;
  _vComp = v_comp;
  
  return true;
}

////////////////////////////
// precompute()
//
// Precompute the sounding advection
//
// Returns true on success, false on failure

bool SoundingAdvector::precompute(const Pjg &projection,
				  const int lead_time_secs)
{
  // Save the motion projection and lead time

  _motionProjection = projection;
  
  // Initialize the offset values.

  _xOffset = 0;
  _yOffset = 0;
  
  // Determine the distance for advection from sounding winds

  if (lead_time_secs > 0)
  {
    // Get the advection motion based on the sounding

    double translate_km_x = _uComp * (double)lead_time_secs / 1000.0;
    double translate_km_y = _vComp * (double)lead_time_secs / 1000.0;
    
    _xOffset = (int)(projection.km2xGrid(translate_km_x) + 0.5);
    _yOffset = (int)(projection.km2yGrid(translate_km_y) + 0.5);
  }
 
  return true;
}


////////////////////////////
// calcFcstIndex()
//
// Calculate the grid index of the original grid location from this
// forcast grid location.
//
// Returns the calculated grid index if successful, returns -1 if
// the original location is outside of the grid or if there is no
// motion in that location.

int SoundingAdvector::calcFcstIndex(const int x_index,
				    const int y_index)
{
  int nx, ny, nz;
  
  _motionProjection.getGridDims(nx, ny, nz);
  
  int fcst_x = x_index - _xOffset;
  int fcst_y = y_index - _yOffset;
	
  if (fcst_x < 0 || fcst_x >= nx ||
      fcst_y < 0 || fcst_y >= ny)
    return -1;
	
  return fcst_x + (fcst_y * nx);
}
