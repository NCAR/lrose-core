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
 * @file SdsDataField.cc
 *
 * @class SdsDataField
 *
 * Base class for a data field that is stored in the SDS section of a
 * TRMM file.
 *  
 * @date 10/31/2008
 *
 */

#include <iostream>
#include <algorithm>
#include <string.h>

#include "SdsDataField.hh"

using namespace std;

const double SdsDataField::MDV_BAD_DATA_VALUE = -88888.0;
const double SdsDataField::MDV_MISSING_DATA_VALUE = -99999.0;


/*********************************************************************
 * Constructors
 */

SdsDataField::SdsDataField(const string &sds_field_name,
			   const vector< FieldInfo > field_info,
			   const int num_vert_levels,
			   const vector< double > vert_levels,
			   const Mdvx::vlevel_type_t vert_level_type,
			   const bool dz_constant,
			   const bool invert_vert_levels,
			   RadConvert &rad_convert,
			   const bool debug_flag,
			   const bool verbose_flag) :
  _debug(debug_flag),
  _verbose(verbose_flag),
  _radConvert(rad_convert),
  _sdsFieldName(sds_field_name),
  _fieldInfo(field_info),
  _numVertLevels(num_vert_levels),
  _vertLevels(vert_levels),
  _vertLevelType(vert_level_type),
  _dzConstant(dz_constant),
  _gridDz(1.0),
  _invertVertLevels(invert_vert_levels)
{
  // Set the vertical information for constant dz fields

  if (_dzConstant && vert_levels.size() > 1)
  {
    _gridDz = vert_levels[1] - vert_levels[0];
      
    if (_invertVertLevels)
      _gridDz *= -1.0;
  }
  
}


/*********************************************************************
 * Destructor
 */

SdsDataField::~SdsDataField()
{
}


/*********************************************************************
 * createMdvFields()
 */

bool SdsDataField::createMdvFields(vector< MdvxField* > &mdv_fields,
				   const MdvxPjg &output_proj,
				   const HdfFile &hdf_file)
{
  static const string method_name = "SdsDataField::createMdvFields()";
  
  // Read the data from the TRMM file

  fl64 *raw_data;
  vector< int > dimensions;
  
  if ((raw_data = hdf_file.readSdsData(_sdsFieldName.c_str(), dimensions)) == 0)
    return false;
  
  // Check the dimensions of the data

  if (!_checkDimensions(hdf_file, dimensions))
  {
    delete [] raw_data;
    return false;
  }
  
  // Create the MDV fields

  if (!_createMdvFields(mdv_fields, output_proj, hdf_file, raw_data))
  {
    delete [] raw_data;
    return false;
  }
  
  // Clean up memory

  delete [] raw_data;
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createBlankMdvField()
 */

MdvxField *SdsDataField::_createBlankMdvField(const string &field_name,
					      const string &field_units,
					      const MdvxPjg &output_proj,
					      const Mdvx::vlevel_type_t vlevel_type,
					      const vector< double > vert_levels)
{
  // Create the field header

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  output_proj.syncXyToFieldHdr(field_hdr);
  
  field_hdr.nz = vert_levels.size();
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size =
    field_hdr.nx * field_hdr.ny * field_hdr.nz * field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = vlevel_type;
  field_hdr.vlevel_type = vlevel_type;

  if (_dzConstant)
    field_hdr.dz_constant = 1;
  else
    field_hdr.dz_constant = 0;

  if (field_hdr.nz == 1)
    field_hdr.data_dimension = 2;
  else
    field_hdr.data_dimension = 3;

  field_hdr.grid_dz = _gridDz;
  field_hdr.grid_minz = vert_levels[0];
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = MDV_BAD_DATA_VALUE;
  field_hdr.missing_data_value = MDV_MISSING_DATA_VALUE;
  strncpy(field_hdr.field_name_long, field_name.c_str(), MDV_LONG_FIELD_LEN);
  strncpy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  strncpy(field_hdr.units, field_units.c_str(), MDV_UNITS_LEN);
  
  if (_verbose)
    Mdvx::printFieldHeader(field_hdr, cerr);
  
  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  int num_vert_levels = min((int) vert_levels.size(), MDV_MAX_VLEVELS);
  
  for (int i = 0; i < num_vert_levels; ++i)
  {
    vlevel_hdr.type[i] = vlevel_type;

    if (_invertVertLevels)
      vlevel_hdr.level[i] = vert_levels[vert_levels.size() - i - 1];
    else
      vlevel_hdr.level[i] = vert_levels[i];
    
  }
  
  if (_verbose)
    Mdvx::printVlevelHeader(vlevel_hdr, num_vert_levels,
			    field_hdr.field_name, cerr);
  
  // Create the field

  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}
