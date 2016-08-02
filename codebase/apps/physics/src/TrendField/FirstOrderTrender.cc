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
//   $Date: 2016/03/06 23:15:37 $
//   $Id: FirstOrderTrender.cc,v 1.2 2016/03/06 23:15:37 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * FirstOrderTrender: Class that trends a field using first-order trending.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2002
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <string.h>

#include <toolsa/str.h>

#include "FirstOrderTrender.hh"


/*********************************************************************
 * Constructor
 */

FirstOrderTrender::FirstOrderTrender() :
  FieldTrender(),
  _prevField(0),
  _currField(0)
{
}


/*********************************************************************
 * Destructor
 */

FirstOrderTrender::~FirstOrderTrender()
{
  delete _prevField;
  delete _currField;
}


/*********************************************************************
 * updateField() - Put the given field into the trending list as the
 *                 current value of the field.
 */

void FirstOrderTrender::updateField(MdvxField &current_field)
{
  // Adjust the field pointers

  delete _prevField;
  _prevField = _currField;
  _currField = new MdvxField(current_field);
}
  

/*********************************************************************
 * createTrendedField() - Create a trended field using the field
 *                        information currently stored in the trender.
 *
 * Returns a pointer to the created trended field on success, or
 * 0 if there was an error.
 *
 * Note that the calling method is responsible for deleting the
 * returned pointer when it is no longer needed.
 */

MdvxField *FirstOrderTrender::createTrendedField(void)
{
  static const string method_name = "FirstOrderTrender::createTrendedField()";
  
  // Make sure we have enough history to do the trending

  if (_prevField == 0 || _currField == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Not enough history to create trended field." << endl;
    
    return 0;
  }
  
  // Make sure the projections are the same on the fields to be
  // used in the trending.

  
  // Create the field to be returned.  This field will start out
  // containing all missing data values.

  MdvxField *trended_field;
  
  if ((trended_field = _createTrendedField()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating blank field to be returned" << endl;
    
    return 0;
  }
  
  // Calculate the trended field.

  fl32 *prev_data = (fl32 *)_prevField->getVol();
  fl32 *curr_data = (fl32 *)_currField->getVol();
  fl32 *trended_data = (fl32 *)trended_field->getVol();
  
  Mdvx::field_header_t prev_field_hdr = _prevField->getFieldHeader();
  Mdvx::field_header_t curr_field_hdr = _currField->getFieldHeader();
  Mdvx::field_header_t field_hdr = trended_field->getFieldHeader();

  int grid_size = field_hdr.nx * field_hdr.ny * field_hdr.nz;
  
  for (int i = 0; i < grid_size; ++i)
  {
    // Leave the value as missing if any of the data value to be
    // trended are missing

    if (prev_data[i] == prev_field_hdr.missing_data_value ||
	prev_data[i] == prev_field_hdr.bad_data_value ||
	curr_data[i] == curr_field_hdr.missing_data_value ||
	curr_data[i] == curr_field_hdr.bad_data_value)
      continue;
    
    // Calculate the trended value

    trended_data[i] = (2.0 * curr_data[i]) - prev_data[i];
    
  } /* endfor - i */
  
  return trended_field;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createTrendedField() - Create the trended field object.  Note that
 *                         the returned field object will contain all
 *                         missing data values.
 *
 * Returns a pointer to the created field, or 0 if the field could
 * not be created for some reason.
 */

MdvxField *FirstOrderTrender::_createTrendedField(void) const
{
  // Get the headers for the current data field

  Mdvx::field_header_t input_field_hdr = _currField->getFieldHeader();
  Mdvx::vlevel_header_t input_vlevel_hdr = _currField->getVlevelHeader();
  
  // Create the new field header

  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.field_code = input_field_hdr.field_code;
  field_hdr.forecast_delta = input_field_hdr.forecast_delta;
  field_hdr.forecast_time = input_field_hdr.forecast_time;
  field_hdr.nx = input_field_hdr.nx;
  field_hdr.ny = input_field_hdr.ny;
  field_hdr.nz = input_field_hdr.nz;
  field_hdr.proj_type = input_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = input_field_hdr.native_vlevel_type;
  field_hdr.vlevel_type = input_field_hdr.vlevel_type;
  field_hdr.dz_constant = input_field_hdr.dz_constant;
  
  field_hdr.proj_origin_lat = input_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = input_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; ++i)
    field_hdr.proj_param[i] = input_field_hdr.proj_param[i];
  field_hdr.vert_reference = 0;
  field_hdr.grid_dx = input_field_hdr.grid_dx;
  field_hdr.grid_dy = input_field_hdr.grid_dy;
  field_hdr.grid_dz = input_field_hdr.grid_dz;
  field_hdr.grid_minx = input_field_hdr.grid_minx;
  field_hdr.grid_miny = input_field_hdr.grid_miny;
  field_hdr.grid_minz = input_field_hdr.grid_minz;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = input_field_hdr.missing_data_value;
  field_hdr.missing_data_value = input_field_hdr.missing_data_value;
  field_hdr.proj_rotation = input_field_hdr.proj_rotation;
  
  STRcopy(field_hdr.field_name_long,
	  input_field_hdr.field_name_long, MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, input_field_hdr.field_name,
	  MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, input_field_hdr.units, MDV_UNITS_LEN);
  STRcopy(field_hdr.transform, "trended", MDV_TRANSFORM_LEN);
  
  // Create the average field

  return new MdvxField(field_hdr, input_vlevel_hdr, (void *)0, true);
}
