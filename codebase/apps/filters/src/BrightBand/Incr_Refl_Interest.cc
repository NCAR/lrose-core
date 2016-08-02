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
//////////////////////////////////////////////////////////
// Incr_Refl_Interest.h:  Interest derived from determining if
//                        reflectivity values are monotonically
//                        decreasing
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 1997
//
//////////////////////////////////////////////////////////

#include "Incr_Refl_Interest.h"

using namespace std;

//////////////
// Constructor

Incr_Refl_Interest::Incr_Refl_Interest (const double max_down,
					const double max_up,
					const bool debug_flag) :
  Interest(debug_flag),
  _maxDown(max_down),
  _maxUp(max_up)
{
  if (_debug)
    cerr << "Creating Incr_Refl_Interest object" << endl;
}


/////////////
// Destructor

Incr_Refl_Interest::~Incr_Refl_Interest()
{
   if (_debug)
     cerr << "Deleting Incr_Refl_Interest object" << endl;
}


//////////////
// calcInterestFields()

bool Incr_Refl_Interest::calcInterestFields(MdvxField *dbz_field)
{
  // Initialize the interest fields

  _initInterestFields(dbz_field);
  
  // Calculate the interest fields

  int increasing;
  int missing;
  int start_plane;
   
  fl32 *dbz_array = (fl32 *)_dbzField->getVol();
  int plane_size = _dbzFieldHdr.nx * _dbzFieldHdr.ny;
   
  for (int iy = 0; iy < _dbzFieldHdr.ny; iy++)
  {
    for (int ix = 0; ix < _dbzFieldHdr.nx; ix++)
    {
      int plane_index = ix+iy*_dbzFieldHdr.nx;
	
      increasing = FALSE;
      missing = FALSE;
      start_plane = 0;
	
      // find first plane with data for this grid point

      while (start_plane < _dbzFieldHdr.nz &&
	     (dbz_array[(start_plane*plane_size) + plane_index] == _dbzFieldHdr.missing_data_value ||
	      dbz_array[(start_plane*plane_size) + plane_index] == _dbzFieldHdr.bad_data_value))
      {
	start_plane++;
      }
	 
      if (start_plane*_dbzFieldHdr.grid_dz + _dbzFieldHdr.grid_minz < _maxDown)
      {
	for (int iz = start_plane + 1; iz < _maxUp; iz++)
	{ 
	  int volume_index = (iz * plane_size) + plane_index;
	  
	  if (dbz_array[volume_index] == _dbzFieldHdr.missing_data_value ||
	      dbz_array[volume_index] == _dbzFieldHdr.bad_data_value)
	  {
	    missing = TRUE;
	    ht[plane_index] = iz;
	    break;
	  }
               
	  if (dbz_array[volume_index] > 
	      dbz_array[volume_index - plane_size])
	  {
	    increasing = TRUE;
	    ht[plane_index] = iz;
	    break;
	  }
	}

	if (increasing || missing)
	{
	  interest_values[plane_index] = 1.0;
	}
	else
	{
	  interest_values[plane_index] = 0.0;
	  ht[ix+iy*_dbzFieldHdr.nx] = 0;
	}
	
      }
      else
      {
	interest_values[plane_index] = 1.0;
	ht[plane_index] = 0;
      }
    }
  }

  return true;
}
