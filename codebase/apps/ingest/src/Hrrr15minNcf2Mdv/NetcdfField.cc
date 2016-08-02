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
 * @file NetcdfField.cc
 *
 * @class NetcdfField
 *
 * Class for dealing with a field in the netCDF file.
 *  
 * @date 3/31/2009
 *
 */

#include <iostream>

#include <toolsa/str.h>

#include "NetcdfField.hh"

using namespace std;


// Global constants

const string NetcdfField::LONG_FIELD_NAME_ATT_NAME = "description";
const string NetcdfField::UNITS_ATT_NAME = "units";


/*********************************************************************
 * Constructors
 */

NetcdfField::NetcdfField(const string &nc_field_name,
			 const double nc_missing_data_value,
			 const bool transform_data,
			 const double transform_multiplier,
			 const double transform_constant,
			 const string &transform_units,
			 const bool replace_data,
			 const double replace_nc_value,
			 const double replace_mdv_value,
			 const bool debug_flag, const bool verbose_flag) :
  _debug(debug_flag),
  _verbose(verbose_flag),
  _ncFieldName(nc_field_name),
  _ncMissingDataValue(nc_missing_data_value),
  _transformData(transform_data),
  _transformMultiplier(transform_multiplier),
  _transformConstant(transform_constant),
  _transformUnits(transform_units),
  _replaceData(replace_data),
  _replaceNcValue(replace_nc_value),
  _replaceMdvValue(replace_mdv_value),
  _remapOutput(false)
{
}


/*********************************************************************
 * Destructor
 */

NetcdfField::~NetcdfField()
{
}


/*********************************************************************
 * createMdvField()
 */

MdvxField *NetcdfField::createMdvField(const NcFile &nc_file,
				       const MdvxProj &input_proj,
				       const int forecast_index,
				       const int forecast_secs,
				       const DateTime &forecast_time)
{
  static const string method_name = "NetcdfField::createMdvField()";
  
  // Get a pointer to the variable in the netCDF file

  NcVar *var;

  if ((var = nc_file.get_var(_ncFieldName.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << _ncFieldName
	 << " variable from netCDF file" << endl;
    
    return 0;
  }
  
  // Make sure the variable is valid and is of the type we expect

  if (!var->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << _ncFieldName << " variable in netCDF file isn't valid." << endl;
    
    return 0;
  }
  
  if (var->type() != ncFloat)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << _ncFieldName << " variable is not of type ncFloat." << endl;
    cerr << "We can only handle float fields at this point." << endl;
    
    return 0;
  }
  
  // Get the needed variable attributes

  string field_name_long = _getVarAttAsString(*var, LONG_FIELD_NAME_ATT_NAME);
  
  if (field_name_long == "")
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << LONG_FIELD_NAME_ATT_NAME
	 << " attribute for NC field " << _ncFieldName << endl;
    
    return 0;
  }
  
  string units = _getVarAttAsString(*var, UNITS_ATT_NAME);
  
  if (units == "")
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << UNITS_ATT_NAME
	 << " attribute for NC field " << _ncFieldName << endl;
    
    return 0;
  }
  
  if (_debug)
  {
    cerr << "     field_name_long = " << field_name_long << endl;
    cerr << "     units = " << units << endl;
  }
  
  // Get the data for this field.  We put the data into a temporary
  // memory location since we don't know if float and fl32 are equivalent
  // on this architecture.

  if (!var->set_cur(0, forecast_index, 0, 0))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error setting beginning point of data in netCDF file" << endl;
    
    return 0;
  }
  
  int nx = input_proj.getCoord().nx;
  int ny = input_proj.getCoord().ny;
  
  float *nc_data = new float[nx * ny];
  
  if (!var->get(nc_data, 1, 1, ny, nx))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting data for field: " << _ncFieldName << endl;
    cerr << "forecast secs = " << forecast_secs << endl;
    
    delete [] nc_data;
    
    return 0;
  }
  
  // Create the MDV field

  Mdvx::field_header_t field_hdr;
  _setFieldHeader(field_hdr, input_proj, _ncFieldName, field_name_long, units,
		  forecast_time, forecast_secs);
  
  Mdvx::vlevel_header_t vlevel_hdr;
  _setVlevelHeader(vlevel_hdr);
  
  MdvxField *field;

  if ((field = new MdvxField(field_hdr, vlevel_hdr, (void *)0,
			     false, false)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating the MDV field for field " << _ncFieldName << endl;
    
    delete [] nc_data;
    
    return 0;
  }
  
  // Set the data values in the field

  fl32 *mdv_data = (fl32 *)field->getVol();
  
  for (int i = 0; i < nx * ny; ++i)
  {
    if (nc_data[i] == _ncMissingDataValue)
    {
    }
    else if (_replaceData &&
	     nc_data[i] == _replaceNcValue)
    {
      mdv_data[i] = (fl32)_replaceMdvValue;
    }
    else if (_transformData)
    {
      mdv_data[i] =
	(fl32)((nc_data[i] * _transformMultiplier) + _transformConstant);
    }
    else
    {
      mdv_data[i] = (fl32)nc_data[i];
    }
    
  }
  
  delete [] nc_data;
  
  // Remap the MDV field, if requested

  if (_remapOutput)
  {
    if (field->remap(_remapLut, _remapProj) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error remapping MDV field to output projection" << endl;
      
      delete field;
      
      return 0;
    }
    
  }

  return field;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _setFieldHeader()
 */

void NetcdfField::_setFieldHeader(Mdvx::field_header_t &field_hdr,
				  const MdvxProj &input_proj,
				  const string &field_name,
				  const string &field_name_long,
				  const string &units,
				  const DateTime &forecast_time,
				  const int forecast_secs) const
{
  memset(&field_hdr, 0, sizeof(field_hdr));
  input_proj.syncToFieldHdr(field_hdr);
  
  field_hdr.forecast_delta = forecast_secs;
  field_hdr.forecast_time = forecast_time.utime();
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size =
    field_hdr.nx * field_hdr.ny * field_hdr.nz * field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.vlevel_type = field_hdr.native_vlevel_type;
  field_hdr.dz_constant = 1;
  field_hdr.data_dimension = 2;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = _ncMissingDataValue;
  field_hdr.missing_data_value = _ncMissingDataValue;
  STRcopy(field_hdr.field_name_long, field_name_long.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  if (_transformData)
    STRcopy(field_hdr.units, _transformUnits.c_str(), MDV_UNITS_LEN);
  else
    STRcopy(field_hdr.units, units.c_str(), MDV_UNITS_LEN);
}

/*********************************************************************
 * _setVlevelHeader()
 */

void NetcdfField::_setVlevelHeader(Mdvx::vlevel_header_t &vlevel_hdr) const
{
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));

  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vlevel_hdr.level[0] = 0.0;
}
