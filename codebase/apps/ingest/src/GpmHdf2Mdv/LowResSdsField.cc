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
/**
 *
 * @file LowResSdsField.cc
 *
 * @class LowResSdsField
 *
 * Class representing a TRMM SDS field that is stored at low resolution.
 *  
 * @date 11/3/2008
 *
 */

#include <iostream>
#include <algorithm>

#include "LowResSdsField.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

LowResSdsField::LowResSdsField(const string &sds_field_name,
			       const vector< FieldInfo > field_info,
			       const int num_vert_levels,
			       const vector< double > vert_levels,
			       const Mdvx::vlevel_type_t vert_level_type,
			       const bool dz_constant,
			       const bool invert_vert_levels,
			       RadConvert &rad_convert,
			       const bool debug_flag,
			       const bool verbose_flag) :
  SdsDataField(sds_field_name, field_info,
	       num_vert_levels, vert_levels,
	       vert_level_type, dz_constant, invert_vert_levels,
	       rad_convert, 
	       debug_flag, verbose_flag)
{
}


/*********************************************************************
 * Destructor
 */

LowResSdsField::~LowResSdsField()
{
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _checkDimensions()
 */

bool LowResSdsField::_checkDimensions(const HdfFile &hdf_file,
				       const vector< int > &dimensions)
{
  static const string method_name = "LowResSdsField::_checkDimensions()";
  
  // Determine the dimensions to use

  int scans_dim = 0;
  int pixels_dim = 1;
  int levels_dim = -1;
  int fields_dim = -1;
  
  size_t num_dims = 2;

  if (_numVertLevels > 1)
  {
    levels_dim = num_dims;
    ++num_dims;
  }

  if (_fieldInfo.size() > 1)
  {
    fields_dim = num_dims;
    ++num_dims;
  }
    
  // Check the number of dimensions

  if (num_dims != dimensions.size())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong number of dimensions in the SDS data array for field: "
	 << _sdsFieldName << endl;
    cerr << "Expected " << num_dims << " dimensions" << endl;
    cerr << "Found " << dimensions.size() << " dimensions" << endl;
    
    return false;
  }
  
  // Check the number of scans

  if (dimensions[scans_dim] != hdf_file.getNumScans())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong number of scans in SDS data array for field: "
	 << _sdsFieldName << endl;
    cerr << "Expected " << hdf_file.getNumScans() << " scans" << endl;
    cerr << "Found " << dimensions[scans_dim] << " scans" << endl;
    
    return false;
  }
  
  // Check the number of pixels

  if (dimensions[pixels_dim] != hdf_file.getNumPixels() / 2)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong number of pixels in SDS data array for field: "
	 << _sdsFieldName << endl;
    cerr << "Expected " << (hdf_file.getNumPixels()/2) << " pixels" << endl;
    cerr << "Found " << dimensions[pixels_dim] << " pixels" << endl;
    
    return false;
  }
  
  // Check the number of levels

  if (_numVertLevels > 1 && (dimensions[levels_dim] != _numVertLevels))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong number of vertical levels in SDS data array for field: "
	 << _sdsFieldName << endl;
    cerr << "Expected " << _numVertLevels << " vertical levels" << endl;
    cerr << "Found " << dimensions[levels_dim] << " vertical levels" << endl;
    
    return false;
  }
  
  // Check the number of fields

  if (_fieldInfo.size() > 1 &&
      (dimensions[fields_dim] != (int)_fieldInfo.size()))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Wrong number of fields in SDS data array for field: "
	 << _sdsFieldName << endl;
    cerr << "Expected " << _fieldInfo.size() << " fields" << endl;
    cerr << "Found " << dimensions[fields_dim] << " fields" << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _createMdvFields()
 */

bool LowResSdsField::_createMdvFields(vector< MdvxField* > &mdv_fields,
				      const MdvxPjg &output_proj,
				      const HdfFile &hdf_file,
				      const fl64 *raw_data)
{
  static const string method_name = "LowResSdsField::_createMdvFields()";
  
  // Create the specified MDV fields

  for (size_t field_num = 0; field_num < _fieldInfo.size(); ++field_num)
  {
    FieldInfo field_info = _fieldInfo[field_num];
    
    // Create the blank field

    MdvxField *field = _createBlankMdvField(field_info.getMdvFieldName(),
					    field_info.getMdvFieldUnits(),
					    output_proj,
					    _vertLevelType,
					    _vertLevels);
    
    // Fill in the field data

    int num_elements = hdf_file.getNumScans() *
      (hdf_file.getNumPixels() / 2);
    int num_vert_levels = min(_numVertLevels, MDV_MAX_VLEVELS);

    fl32 *mdv_data = (fl32 *)field->getVol();
    
    for (int output_z = 0; output_z < num_vert_levels; ++output_z)
    {
      int input_z = output_z;
      
      if (_invertVertLevels)
	input_z = _vertLevels.size() - output_z - 1;
      
      for (int i = 0; i < num_elements; ++i)
      {
	// Get the index into the raw data and don't do anything if it's
	// a missing value.

	int raw_data_index =
	  (i * _numVertLevels * _fieldInfo.size()) +
	  (input_z * _fieldInfo.size()) + field_num;

	// Get the lat/lon for the raw data point and normalize the lon to
	// the output grid

	double raw_data_lat = hdf_file.getLat(i*2);
	double raw_data_lon = hdf_file.getLon(i*2);
	while (raw_data_lon < output_proj.getMinx())
	  raw_data_lon += 360.0;
	while (raw_data_lon >= output_proj.getMinx() + 360.0)
	  raw_data_lon -= 360.0;
      
	// Get the index into the MDV grid

	int mdv_data_index;

	if (output_proj.latlon2arrayIndex(raw_data_lat, raw_data_lon,
					  mdv_data_index) < 0)
	{
	  if (_verbose)
	  {
	    cerr << "WARNING: " << method_name << endl;
	    cerr << "Data point outside of output grid." << endl;
	    cerr << "lat = " << hdf_file.getLat(i)
		 << ", lon = " << hdf_file.getLon(i) << endl;
	  }
	
	  continue;
	}
      
	mdv_data_index += output_z * output_proj.getNx() * output_proj.getNy();
	
	// Update the MDV data value

	mdv_data[mdv_data_index] =
	  field_info.calcMdvValue(raw_data[raw_data_index], _radConvert,
				  hdf_file.getSunMag(i),
				  hdf_file.getSolarZenith(i),
				  hdf_file.getCosSolarZenith(i));
      
      } /* endfor - i */
      
    } /* endfor - output_z */
    
    if (_debug)
      cerr << field->getFieldName() << " field successfully created" << endl;
    
    // Add the new field to the field list

    mdv_fields.push_back(field);
    
  } /* endfor - field_num */
  
  return true;
}
