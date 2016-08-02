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


#include "Process.hh"

#include <iostream>
#include <Mdv/MdvxField.hh>
#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <Mdv/MdvxProj.hh>
#include <euclid/boundary.h>
#include <euclid/geometry.h>


using namespace std;

//
// Constructor
//
Process::Process(){
 
}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
bool Process::Derive(Params *TDRP_params, bool erase_outside, time_t data_time,
		     double *bounding_lats, double *bounding_lons,
		     int num_bounds)
{

  if (TDRP_params->debug){
    cerr << "Processing for region at " << utimstr(data_time) << endl;
  }


  // Read in the appropriate dataset

  DsMdvx mdvx;

  mdvx.setReadTime(Mdvx::READ_FIRST_BEFORE, TDRP_params->InUrl, 0, data_time);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.addReadField(TDRP_params->fieldName);

  if (mdvx.readVolume())
  {
    cerr << "Read failed at " << utimstr(data_time) << " from ";
    cerr << TDRP_params->InUrl  << endl;
    return false;
  }     


  // Get the desired field - should be the only field we have read in.

  MdvxField *field = mdvx.getFieldByNum(0);
  if (field == 0)
  {
    cerr << "Could not locate specified field." << endl;
    return false;
  }

  Mdvx::master_header_t master_hdr = mdvx.getMasterHeader();
  Mdvx::field_header_t field_hdr = field->getFieldHeader();
  Mdvx::vlevel_header_t vlevel_hdr = field->getVlevelHeader();

  int bottom_level_num = 0;
  int top_level_num = field_hdr.nz-1;
 
  if (TDRP_params->levelNumLimitSet)
  {
    top_level_num = TDRP_params->topLevelNum;
    if (top_level_num > field_hdr.nz-1)
      top_level_num = field_hdr.nz-1;

    bottom_level_num= TDRP_params->bottomLevelNum;
    if (bottom_level_num < 0)
      bottom_level_num = 0;
  }

  if (TDRP_params->levelLimitSet)
  {
    while (vlevel_hdr.level[bottom_level_num] < TDRP_params->bottomLevel)
    {
      ++bottom_level_num;
      if (bottom_level_num > top_level_num)
	break;
    }
    
    while (vlevel_hdr.level[top_level_num] > TDRP_params->topLevel)
    {
      --top_level_num;
      if (top_level_num < bottom_level_num)
	break;
    }
  }

  if (bottom_level_num > top_level_num)
  {
    cerr << "Vertical levels specified in the parameter file result in "
	 << "no chosen levels in the data" << endl;
    cerr << "Calculated bottom level index = " << bottom_level_num << endl;
    cerr << "Calculated top level index = " << top_level_num << endl;
    
    return false;
  }
  
  MdvxProj proj(master_hdr, field_hdr);

  // Convert the bounding lat/lons to integer indicies on this grid.  Make
  // sure that the polygon is closed.

  Point_d *pt_array = new Point_d[num_bounds+1];

  for (int i = 0; i < num_bounds; i++)
  {
    int ix, iy;
    if (proj.latlon2xyIndex(bounding_lats[i], bounding_lons[i],
			    ix, iy) == 0)
    {
      pt_array[i].x = ix;
      pt_array[i].y = iy;
    }
    else
    {
      cerr << "Lat, lon " << bounding_lats[i] << ", "
	   << bounding_lons[i] << " out of bounds." << endl;
      return false;
    }
  }

  pt_array[num_bounds].x = pt_array[0].x;
  pt_array[num_bounds].y = pt_array[0].y;
  
  // Create a grid of points in the polygon.

  int plane_size = field_hdr.nx * field_hdr.ny;
  unsigned char *grid_array = new unsigned char[plane_size];
  memset(grid_array, 0, plane_size);
  
  int num_filled =
    EG_fill_polygon(pt_array, num_bounds + 1,
		    field_hdr.nx, field_hdr.ny,
		    0.0, 0.0, 1.0, 1.0,
		    grid_array, 1);
  
  if (num_filled < 0)
  {
    cerr << "Error returned from polygon gridding routine" << endl;
    return false;
  }
  
  // Now go through the grid and mask the appropriate values

  switch (field_hdr.encoding_type)
  {
  case Mdvx::ENCODING_INT8 :
  {
    ui08 mask_value;
  
    switch (TDRP_params->mask_value_type)
    {
    case Params::USE_MISSING_DATA_VALUE :
      mask_value = (ui08)field_hdr.missing_data_value;
      break;
    case Params::USE_BAD_DATA_VALUE :
      mask_value = (ui08)field_hdr.bad_data_value;
      break;
    case Params::USE_SPECIFIED_DATA_VALUE :
      if (field_hdr.scaling_type == Mdvx::SCALING_NONE)
	mask_value = (ui08)TDRP_params->mask_value;
      else
	mask_value =
	  (ui08)((TDRP_params->mask_value * field_hdr.scale) + field_hdr.bias);
      break;
    } /* endswitch - TDRP_params->mask_value_type */
  
    ui08 *data = (ui08 *)field->getVol();

    _updateGrid(data, grid_array, plane_size,
		bottom_level_num, top_level_num, erase_outside,
		mask_value);
    break;
  }
    
  case Mdvx::ENCODING_INT16 :
  {
    ui16 mask_value;
  
    switch (TDRP_params->mask_value_type)
    {
    case Params::USE_MISSING_DATA_VALUE :
      mask_value = (ui16)field_hdr.missing_data_value;
      break;
    case Params::USE_BAD_DATA_VALUE :
      mask_value = (ui16)field_hdr.bad_data_value;
      break;
    case Params::USE_SPECIFIED_DATA_VALUE :
      if (field_hdr.scaling_type == Mdvx::SCALING_NONE)
	mask_value = (ui16)TDRP_params->mask_value;
      else
	mask_value =
	  (ui16)((TDRP_params->mask_value * field_hdr.scale) + field_hdr.bias);
      break;
    } /* endswitch - TDRP_params->mask_value_type */
  
    ui16 *data = (ui16 *)field->getVol();

    _updateGrid(data, grid_array, plane_size,
		bottom_level_num, top_level_num, erase_outside,
		mask_value);
    break;
  }
  
  case Mdvx::ENCODING_FLOAT32 :
  {
    fl32 mask_value;
  
    switch (TDRP_params->mask_value_type)
    {
    case Params::USE_MISSING_DATA_VALUE :
      mask_value = (fl32)field_hdr.missing_data_value;
      break;
    case Params::USE_BAD_DATA_VALUE :
      mask_value = (fl32)field_hdr.bad_data_value;
      break;
    case Params::USE_SPECIFIED_DATA_VALUE :
      if (field_hdr.scaling_type == Mdvx::SCALING_NONE)
	mask_value = (fl32)TDRP_params->mask_value;
      else
	mask_value =
	  (fl32)((TDRP_params->mask_value * field_hdr.scale) + field_hdr.bias);
      break;
    } /* endswitch - TDRP_params->mask_value_type */
  
    fl32 *data = (fl32 *)field->getVol();

    _updateGrid(data, grid_array, plane_size,
		bottom_level_num, top_level_num, erase_outside,
		mask_value);
    break;
  }
  
  default:
  {
    cerr << "Cannot process data with the given encoding" << endl;
    return false;
  }
  
  } /* endswitch - field_hdr.encoding_type */
  
  delete [] pt_array;
  delete [] grid_array;
  
  // Write the file to the output url
  
  field->compress(Mdvx::COMPRESSION_BZIP);

  if (mdvx.writeToDir(TDRP_params->OutUrl))
  {
    cerr << "Failed to wite to " << TDRP_params->OutUrl << endl;
    exit(-1);
  }      
  
  if (TDRP_params->debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }

  return true;

}



////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){

}










