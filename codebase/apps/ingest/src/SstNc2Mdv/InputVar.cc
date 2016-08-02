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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 01:23:05 $
//   $Id: InputVar.cc,v 1.4 2016/03/07 01:23:05 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * InputVar: Class for handling an input variable.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Mdv/MdvxPjg.hh>

#include "InputVar.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

InputVar::InputVar(const string &nc_var_name,
		   const string &missing_data_attr_name,
		   const string &units_attr_name,
		   const string &mdv_field_name,
		   const double mdv_missing_data_value,
		   const bool specify_mdv_scaling,
		   const double mdv_scale,
		   const double mdv_bias,
		   const bool debug_flag,
		   MsgLog *msg_log) :
  _debug(debug_flag),
  _msgLog(msg_log),
  _ncVarName(nc_var_name),
  _missingDataAttrName(missing_data_attr_name),
  _unitsAttrName(units_attr_name),
  _mdvFieldName(mdv_field_name),
  _mdvMissingDataValue(mdv_missing_data_value),
  _mdvUnits(""),
  _specifyMdvScaling(specify_mdv_scaling),
  _mdvScale(mdv_scale),
  _mdvBias(mdv_bias),
  _fieldData(0)
{
}

  
/*********************************************************************
 * Destructor
 */

InputVar::~InputVar()
{
}


/*********************************************************************
 * createMdvxField() - Create an MdvxField object using the information
 *                     from this field.
 *
 * Returns a pointer to the created field on success, 0 on failure.
 */

MdvxField *InputVar::createMdvxField(const MdvxPjg &projection,
				     time_t data_time,
				     const fl32 *lat_data,
				     const fl32 *lon_data) const
{
  const string METHOD_NAME = "InputVar::createMdvxField()";

  // Create a blank field object

  cerr << "Creating blank MDV field" << endl;
 
  MdvxField *field;
  
  if ((field = _createBlankMdvxField(projection,
				     data_time,
				     _mdvMissingDataValue,
				     _mdvMissingDataValue,
				     _mdvFieldName,
				     _mdvFieldName,
				     _mdvUnits)) == 0)
    return 0;
  
  // Fill in the field data

  cerr << "Filling in field data" << endl;
 
  if (!remapInput(field, lat_data, lon_data))
  {
    delete field;
    return 0;
  }
  
  // Compress the field

  cerr << "Compressing field" << endl;
 
  if (_specifyMdvScaling)
    field->convertType(Mdvx::ENCODING_INT8,
		       Mdvx::COMPRESSION_BZIP,
		       Mdvx::SCALING_SPECIFIED,
		       _mdvScale,
		       _mdvBias);
  else
    field->convertType(Mdvx::ENCODING_INT8,
		       Mdvx::COMPRESSION_BZIP,
		       Mdvx::SCALING_DYNAMIC);

  return field;
}


/*********************************************************************
 * readData() - Read the data for this variable from the given netCDF
 *              file.
 *
 * Returns true on success, false on failure.
 *
 * Note that this method assumes that the caller has already loaded all
 * of the data for this variable into the given nc_dataset object.
 */

bool InputVar::readData(NetcdfDataset *nc_dataset,
			const int num_input_rows, const int num_input_cols)
{
  const string METHOD_NAME = "InputVar::ReadInpSSTData()";

  // Save the number of rows and columns for later use

  _numInputRows = num_input_rows;
  _numInputCols = num_input_cols;
  
  // Retrieve the variable from the netCDF dataset

  NcdVar *nc_var;

  if ((nc_var = nc_dataset->get_var(_ncVarName)) == 0)
  {
    POSTMSG(ERROR, "Error retrieving %s variable from netCDF file",
	    _ncVarName.c_str());
    return false;
  }
  

  // Get variable data from input netCDF file
  
  _fieldData = (fl32 *) nc_var->get_data();

  // Get the missing values from the netCDF file and fill in the variable
  // values where needed

  if (_missingDataAttrName.size() > 0)
  {
    float nc_missing_value =
      *((float *) nc_var->get_att(_missingDataAttrName)->values());

    // Substitute *our* missing value for the missing value found in
    // the input data. This is done to prevent bizarre missing values
    // like "-3.402823e+38f" (found in NASA swath data) from messing-up
    // the INT8 encoding scale and bias and subsequent compression
    // (which was causing the output MDV file to appear empty).

    int num_data_pts = num_input_rows * num_input_cols;
    
    for (int i = 0; i < num_data_pts; ++i)
    {
      if (_fieldData[i] == nc_missing_value)
	_fieldData[i] = _mdvMissingDataValue;
    }
  }

  // Get the units name and save for use in creating the MDV field

  if (_unitsAttrName.size() > 0)
  {
    char *units =
      (char *)nc_var->get_att(_unitsAttrName)->values();
    
    _mdvUnits = units;
  }
  else
  {
    _mdvUnits = "none";
  }
  
  return true;
}


/*********************************************************************
 * remapInput() - Remap the input netCDF data to the output MDV field.
 */

bool InputVar::remapInput(MdvxField *field,
			  const fl32 *lat_data, const fl32 *lon_data) const
{
  const string METHOD_NAME = "InputVar::remapInput()";

  // Get the projection for the output field

  MdvxPjg projection(field->getFieldHeader());
  
  // Fill in the MDV field values with the data for this variable

  fl32 *field_data = (fl32 *) field->getVol();
  int num_data_pts = _numInputRows * _numInputCols;
 
  cerr << "Remapping " << num_data_pts << " data points" << endl;
  
  for (int inp_index = 0; inp_index < num_data_pts; ++inp_index)
  {
    double inp_lat = lat_data[inp_index];
    double inp_lon = lon_data[inp_index];

    // Check for errors in the lat/lon values.  We were getting some 
    // pretty bad values that would cause the remapping to take forever.

    if (inp_lat < -90.0 || inp_lat > 90.0 ||
	inp_lon < -360.0 || inp_lon > 360.0)
      continue;
    
    int out_index;

    if (projection.latlon2arrayIndex(inp_lat, inp_lon, out_index) == 0)
    {
      field_data[out_index] = _fieldData[inp_index];
    }
  } /* endfor - inp_index */

  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _createBlankMdvxField() - Create an empty MdvxField object.
 *
 * Returns a pointer to the created field on success, 0 on failure.
 */

MdvxField *InputVar::_createBlankMdvxField(const MdvxPjg &projection,
					   time_t data_time,
					   fl32 bad_data_value,
					   fl32 missing_data_value,
					   const string &field_name,
					   const string &field_name_long,
					   const string &units)
{
  const string METHOD_NAME = "InputVar::_createBlankMdvxField()";

  // Create the field header

  Mdvx::field_header_t field_hdr;

  // Set entire field header to all zero bytes

  memset(&field_hdr, 0, sizeof(field_hdr));

  field_hdr.field_code          = 0;
  field_hdr.forecast_delta      = 0;
  field_hdr.forecast_time       = data_time;
  field_hdr.encoding_type       = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.compression_type    = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type      = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type        = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type  = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.vlevel_type         = field_hdr.native_vlevel_type;
  field_hdr.dz_constant         = 1;
  field_hdr.scale               = 1.0;
  field_hdr.bias                = 0.0;
  field_hdr.bad_data_value      = bad_data_value;
  field_hdr.missing_data_value  = missing_data_value;

  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.field_name_long, field_name_long.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.units, units.c_str(), MDV_UNITS_LEN);

  projection.syncToFieldHdr(field_hdr);

  Mdvx::vlevel_header_t  vlevel_hdr;

  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));

  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vlevel_hdr.level[0] = 0.5;
 
  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}
