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
// Refl_Interest.cc:  Interest derived from max reflectivity
//                    value in vertical profile
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 1997
//
//////////////////////////////////////////////////////////

#include "Refl_Interest.h"
using namespace std;

//////////////
// Constructor

Refl_Interest::Refl_Interest(const Params::input_template_info_t &input_template1_info,
			     const Params::input_template_info_t &input_template2_info,
			     const Params::input_template_info_t &input_template3_info,
			     const bool debug_flag) :
  Interest(debug_flag),
  _minThreshold(0.0),
  _minThresholdSet(false)
{
  static const string method_name = "Refl_Interest::Refl_Interest()";
  
  // variables used to process input data

  if(_debug)
  {
    cerr << "Creating Refl_Interest object" << endl;
  }

  if (input_template1_info.compute_interest && 
      (!_minThresholdSet ||
       input_template1_info.min_refl_in_band <=	_minThreshold))
  {
    _minThreshold = input_template1_info.min_refl_in_band;
    _minThresholdSet = true;
  }

  if (input_template2_info.compute_interest && 
      (!_minThresholdSet ||
       input_template2_info.min_refl_in_band <=	_minThreshold))
  {
    _minThreshold = input_template2_info.min_refl_in_band;
    _minThresholdSet = true;
  }

  if (input_template3_info.compute_interest && 
      (!_minThresholdSet ||
       input_template3_info.min_refl_in_band <= _minThreshold))
  {
    _minThreshold = input_template3_info.min_refl_in_band;
    _minThresholdSet = true;
  }

}

/////////////
// Destructor

Refl_Interest::~Refl_Interest()
{
   if (_debug)
     cerr << "Deleting Refl_Interest object" << endl;
}


//////////////
// calcInterestFields()

bool Refl_Interest::calcInterestFields(MdvxField *dbz_field)
{
  static const string method_name = "Refl_Interest::calcInterestFields()";
  
  // Initialize the interest fields

  _initInterestFields(dbz_field);
  
  // If the minimum threshold wasn't set, leave the interest values to
  // their default values and return.  This isn't an error.

  if (!_minThresholdSet)
    return true;
  
  fl32 *dbz_array = (fl32 *)_dbzField->getVol();

  // compute interest
    
  int plane_size = _dbzFieldHdr.nx * _dbzFieldHdr.ny;
	 
  for(int iy = 0; iy < _dbzFieldHdr.ny; iy++)
  {
    for(int ix = 0; ix < _dbzFieldHdr.nx; ix++)
    {
      size_t plane_index = ix+iy*_dbzFieldHdr.nx;
	      
      fl32 max_refl = _minThreshold;
      bool max_refl_found = false;
	       
      for (int iz = 0; iz < _dbzFieldHdr.nz; iz++)
      {
	int volume_index = (iz * plane_size) + plane_index;
		 
	if (dbz_array[volume_index] != _dbzFieldHdr.missing_data_value &&
	    dbz_array[volume_index] != _dbzFieldHdr.bad_data_value &&
	    dbz_array[volume_index] > max_refl)
	{
	  max_refl = dbz_array[volume_index];
	  ht[plane_index] = iz;
	  max_refl_found = true;
	}
      } /* endfor - iz */

      if (max_refl_found)
      {
	interest_values[plane_index] = 1.0;
      }
      else
      {
	interest_values[plane_index] = 0.0;
	ht[plane_index] = 0;
      }

    } /* endfor - ix */
  }/* endfor - iy */

  return true;
}
