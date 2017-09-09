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
//   $Date: 2017/09/09 21:22:28 $
//   $Id: SweepFile.cc,v 1.16 2017/09/09 21:22:28 dixon Exp $
//   $Revision: 1.16 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SweepFile: Class for controlling access to the netCDF sweep files.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <string.h>
#include <cerrno>
#include <math.h>

#include <toolsa/os_config.h>
#include <Mdv/DsMdvx.hh>
#include <toolsa/str.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>

#include "SweepFile.hh"

using namespace std;


// Define globals

const string SweepFile::NUM_AZIMUTHS_DIM_NAME = "Time";
const string SweepFile::NUM_FIELDS_DIM_NAME = "fields";
const string SweepFile::FIELD_NAME_LEN_DIM_NAME = "short_string";

const string SweepFile::LATITUDE_VAR_NAME = "Latitude";
const string SweepFile::LONGITUDE_VAR_NAME = "Longitude";
const string SweepFile::ALTITUDE_VAR_NAME = "Altitude";
const string SweepFile::GATE_SPACING_VAR_NAME = "Cell_Spacing";
const string SweepFile::START_RANGE_VAR_NAME = "Range_to_First_Cell";
const string SweepFile::TARGET_ELEV_VAR_NAME = "Fixed_Angle";
const string SweepFile::BASE_TIME_VAR_NAME = "base_time";
const string SweepFile::TIME_OFFSET_VAR_NAME = "time_offset";
const string SweepFile::AZIMUTH_VAR_NAME = "Azimuth";
const string SweepFile::VOLUME_START_TIME_VAR_NAME = "volume_start_time";

const string SweepFile::FIELD_NAME_LONG_ATT_NAME = "long_name";
const string SweepFile::UNITS_ATT_NAME = "units";
const string SweepFile::SCALE_ATT_NAME = "scale_factor";
const string SweepFile::BIAS_ATT_NAME = "add_offset";

const string SweepFile::VOL_NUM_GLOBAL_ATT_NAME = "Volume_Number";

const float SweepFile::FLOAT_MISSING_DATA_VALUE = -32768.0f;
const int SweepFile::INT_MISSING_DATA_VALUE = -32768;


/*********************************************************************
 * Constructors
 */

SweepFile::SweepFile(const string &sweep_file_path,
                     const string &num_gates_dim_name,
		     const string &field_list_var_name,
		     const string &missing_data_value_att_name,
		     const bool bias_specified,
		     const double output_beamwidth,
		     const bool force_negative_longitude,
		     const bool override_file_missing_data_value,
		     const double file_missing_data_value,
		     const bool fix_missing_beams,
		     const bool debug_flag) :
  _debug(debug_flag),
  _numGatesDimName(num_gates_dim_name),
  _fieldListVarName(field_list_var_name),
  _missingDataValueAttName(missing_data_value_att_name),
  _biasSpecified(bias_specified),
  _filePath(sweep_file_path),
  _numGates(-1),
  _numInputAzimuths(-1),
  _numOutputAzimuths((int)(360.0/output_beamwidth)),
  _outputBeamWidth(output_beamwidth),
  _forceNegativeLongitude(force_negative_longitude),
  _overrideFileMissingDataValue(override_file_missing_data_value),
  _fileMissingDataValue(file_missing_data_value),
  _fixMissingBeams(fix_missing_beams),
  _objectInitialized(false)
{
  _azimuthLut.setBeamWidth(output_beamwidth);
}

  
/*********************************************************************
 * Destructor
 */

SweepFile::~SweepFile()
{
  _sweepFile->close();
  
  delete _sweepFile;
}


/*********************************************************************
 * getVolumeNumber() - Get the volume number from the sweep file.
 *
 * Returns true on success, false on failure
 */

bool SweepFile::getVolumeNumber(int &volume_number) const
{
  static const string method_name = "SweepFile::getVolumeNumber()";
  
  volume_number = _getGlobalIntAtt(VOL_NUM_GLOBAL_ATT_NAME);
  
  return true;
}


/*********************************************************************
 * getVolumeStartTime() - Get the volume start time from the sweep
 *                        file.
 *
 * Returns true on success, false on failure
 */

bool SweepFile::getVolumeStartTime(DateTime &volume_start_time) const
{
  static const string method_name = "SweepFile::getVolumeStartTime()";
  
  volume_start_time.set(_getScalarVarInt(VOLUME_START_TIME_VAR_NAME));
  
  return true;
}


/*********************************************************************
 * initialize() - Initialize the SweepFile.
 *
 * Returns true on success, false on failure
 */

bool SweepFile::initialize()
{
  static const string method_name = "SweepFile::initialize()";
  
  if (_debug)
    cerr << "Initializing SweepFile object..." << endl;

  _objectInitialized = false;
  
  // Create an error object so that the netCDF library doesn't exit when an
  // error is encountered.  This object is not explicitly used in the below
  // code, but is used implicitly by the netCDF library.

  _ncError = new Nc3Error(Nc3Error::silent_nonfatal);

  // Initialize the input netCDF file

  _sweepFile = new Nc3File(_filePath.c_str());
  
  if (!_sweepFile->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Sweep file isn't valid: " << _filePath << endl;

    _sweepFile->close();
    return false;
  }

  _objectInitialized = true;
  
  if (_debug)
    cerr << "... SweepFile object successfully initialized" << endl;

  return true;
}


/*********************************************************************
 * addSweepToMdv() - Add the sweep from this file into the given MDV 
 *                   object.
 *
 * Returns true on success, false on failure
 */

bool SweepFile::addSweepToMdv(Mdvx &mdv_file)
{
  static const string method_name = "SweepFile::addSweepToMdv()";
  
  // Make sure the object was initialized

  if (!_objectInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Object not initialized" << endl;
    
    return false;
  }
  
  // Get the dimensions of the data

  if (!_getDimensions())
    return false;
    
  // Create the azimuth look-up table

  if (!_createAzimuthLookupTable())
    return false;
  
  // Update the MDV file

  if (mdv_file.getMasterHeader().n_fields == 0)
    _updateMasterHeader(mdv_file);
  
  if (!_createMdvFields(mdv_file))
    return false;
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _createAzimuthLookupTable() - Create the azimuth lookup table, used
 *                               to map the netCDF data index to the
 *                               appropriate azimuth location.
 *
 * Returns true on success, false on failure.
 */

bool SweepFile::_createAzimuthLookupTable()
{
  static const string method_name = "SweepFile::_createAzimuthLookupTable()";
  
  // Get the azimuth data

  Nc3Var *az_var;

  if ((az_var = _getFieldVar(AZIMUTH_VAR_NAME)) == 0)
    return 0;

  if (_debug)
    cerr << "Getting azimuth values for variable: "
	 << AZIMUTH_VAR_NAME << endl;

  Nc3Values *az_values;
  if ((az_values = az_var->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting values for " << AZIMUTH_VAR_NAME
	 << " field from sweep file: " << _filePath << endl;
    
    return false;
  }
  
  // Update the lookup table with the current azimuth locations

  _azimuthLut.clear();
  
  for (int i = 0; i < _numInputAzimuths; ++i)
    _azimuthLut.addAzIndex(az_values->as_float(i), i);
  
  return true;
}


/*********************************************************************
 * _createMdvField() - Create the MDV field with the given name based
 *                     on the data in the sweep file.
 *
 * Returns a pointer to the created field on success, 0 on failure.
 */

MdvxField *SweepFile::_createMdvField(const string &field_name) const
{
  static const string method_name = "SweepFile::_createMdvField()";
  
  // Access the field data

  Nc3Var *field_var;

  if ((field_var = _getFieldVar(field_name)) == 0)
    return 0;

  // Construct the field header

  if (_debug)
    cerr << "Constructing field header for field: " << field_name << endl;

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.nx = _numGates;
  field_hdr.ny = _numOutputAzimuths;
  field_hdr.nz = 0;
  field_hdr.proj_type = Mdvx::PROJ_POLAR_RADAR;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  field_hdr.dz_constant = 1;
  field_hdr.data_dimension = 2;
  field_hdr.proj_origin_lat = _getScalarVarFloat(LATITUDE_VAR_NAME);
  field_hdr.proj_origin_lon = _getScalarVarFloat(LONGITUDE_VAR_NAME);
  if (_forceNegativeLongitude &&
      field_hdr.proj_origin_lon > 0.0)
    field_hdr.proj_origin_lon = -field_hdr.proj_origin_lon;
  field_hdr.grid_dx = _getScalarVarFloat(GATE_SPACING_VAR_NAME) / 1000.0;
  field_hdr.grid_dy = _outputBeamWidth;
  field_hdr.grid_dz = 1.0;
  field_hdr.grid_minx = _getScalarVarFloat(START_RANGE_VAR_NAME) / 1000.0;
  field_hdr.grid_miny = 0.0;
  field_hdr.grid_minz = _getScalarVarFloat(TARGET_ELEV_VAR_NAME);
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  if (_overrideFileMissingDataValue)
    field_hdr.bad_data_value = _fileMissingDataValue;
  else
    field_hdr.bad_data_value = _getVarFloatAtt(*field_var,
					       _missingDataValueAttName);
  field_hdr.missing_data_value = field_hdr.bad_data_value;
  STRcopy(field_hdr.field_name_long,
	  _getVarStringAtt(*field_var, FIELD_NAME_LONG_ATT_NAME).c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units,
	  _getVarStringAtt(*field_var, UNITS_ATT_NAME).c_str(),
	  MDV_UNITS_LEN);
  
  // Construct the vlevel header

  if (_debug)
    cerr << "Constructing vlevel header for field: " << field_name << endl;

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  // Create the field

  if (_debug)
    cerr << "Creating MdvField object for field: " << field_name << endl;

  MdvxField *mdv_field =
    new MdvxField(field_hdr, vlevel_hdr, (void *)0);

  if (_debug)
    cerr << endl;

  return mdv_field;
}


/*********************************************************************
 * _updateMdvFieldData() - Add the new sweep to the MDV data volume.
 *
 * Returns true on success, false on failure.
 */

bool SweepFile::_updateMdvFieldData(const string &field_name,
				    MdvxField &field)const
{
  static const string method_name = "SweepFile::_updateMdvFieldData()";
  
  // Get the field data

  Nc3Var *field_var;

  if ((field_var = _getFieldVar(field_name)) == 0)
    return false;

  if (_debug)
    cerr << "Getting field values for variable: " << field_name << endl;

  Nc3Values *field_values;
  if ((field_values = field_var->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting values for " << field_name
	 << " field from sweep file: " << _filePath << endl;
    
    return false;
  }
  
  // Check for consistency

  Mdvx::field_header_t field_hdr = field.getFieldHeader();
  
  if (_numOutputAzimuths != field_hdr.ny)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Number of azimuths changed between sweeps in volume" << endl;
    cerr << "Current sweeps have " << field_hdr.ny << " azimuths" << endl;
    cerr << "New sweep has " << _numOutputAzimuths << " azimuths" << endl;
    cerr << "Cannot process sweep" << endl;
    
    return false;
  }
  
  // Determine the new data dimensions

  int nx, ny, nz;

  nx = (_numGates > field_hdr.nx) ? _numGates : field_hdr.nx;
  ny = field_hdr.ny;
  nz = field_hdr.nz + 1;
  
  int plane_size = nx * ny;
  int volume_size = plane_size * nz;
  
  // Set the data values

  fl32 sweep_missing_data_value;

  if (_overrideFileMissingDataValue)
    sweep_missing_data_value = _fileMissingDataValue;
  else
    sweep_missing_data_value = _getVarFloatAtt(*field_var,
					       _missingDataValueAttName);

  fl32 mdv_missing_data_value = field_hdr.missing_data_value;
  
  fl32 *curr_field_data = (fl32 *)field.getVol();
  fl32 *new_field_data = new fl32[volume_size];
  fl32 *plane_data;
  
  if (curr_field_data == 0)
  {
    plane_data = new_field_data;
  }
  else
  {
    if (nx == field_hdr.nx)
    {
      memcpy(new_field_data, curr_field_data,
	     (volume_size - plane_size) * sizeof(fl32));
    }
    else
    {
      for (int z = 0; z < field_hdr.nz; ++z)
      {
	fl32 *new_plane_ptr = new_field_data + (nx * ny * z);
	fl32 *curr_plane_ptr = curr_field_data + (field_hdr.nx * ny * z);
	
	for (int y = 0; y < ny; ++y)
	{
	  fl32 *new_beam_ptr = new_plane_ptr + (y * nx);
	  fl32 *curr_beam_ptr = curr_plane_ptr + (y * field_hdr.nx);
	  
	  memcpy(new_beam_ptr, curr_beam_ptr, field_hdr.nx * sizeof(fl32));
	  
	  for (int x = field_hdr.nx; x < nx; ++x)
	    new_beam_ptr[x] = mdv_missing_data_value;
	  
	} /* endfor - y */
      } /* endfor - z */
    }
    
    plane_data = new_field_data + (volume_size - plane_size);
  }
  
  switch (field_var->type())
  {
  case nc3Float :
  {
    for (int az = 0; az < ny; ++az)
    {
      int input_az_index = _azimuthLut.getAzIndex(az);
      
      if (input_az_index < 0)
	continue;

      for (int gate = 0; gate < nx; ++gate)
      {
	int input_index = (input_az_index * _numGates) + gate;
	int output_index = (az * nx) + gate;
	
	if (gate >= _numGates)
	{
	  plane_data[output_index] = mdv_missing_data_value;
	}
	else
	{
	  plane_data[output_index] = (fl32)field_values->as_float(input_index);
	
	  if (plane_data[output_index] == sweep_missing_data_value)
	    plane_data[output_index] = mdv_missing_data_value;
	}
	
      } /* endfor - gate */
      
    } /* endfor - az */
    
    break;
  }
  
  case nc3Short :
  {
    double scale = _getVarFloatAtt(*field_var, SCALE_ATT_NAME);
    double bias;
    if (_biasSpecified)
      bias = _getVarFloatAtt(*field_var, BIAS_ATT_NAME);
    else
      bias = 0.0;

    sweep_missing_data_value =
      (sweep_missing_data_value * scale) + bias;
    
    for (int az = 0; az < ny; ++az)
    {
      int input_az_index = _azimuthLut.getAzIndex(az);
	
      if (input_az_index < 0)
	continue;

      for (int gate = 0; gate < nx; ++gate)
      {
	int output_index = (az * nx) + gate;
	int input_index = (input_az_index * _numGates) + gate;
	
	if (gate >= _numGates)
	{
	  plane_data[output_index] = mdv_missing_data_value;
	}
	else
	{
	  plane_data[output_index] =
	    (fl32)(((double)field_values->as_short(input_index) *
		    scale) + bias);

	  if (plane_data[output_index] == sweep_missing_data_value)
	    plane_data[output_index] = mdv_missing_data_value;
	}
	
      } /* endfor - gate */
    } /* endfor - az */
    break;
  }
  
  case nc3NoType :
  case nc3Byte :
  case nc3Char :
  case nc3Int :
  case nc3Double :
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Don't yet handle this netCDF data type" << endl;

    delete field_values;
    delete [] new_field_data;
    
    return false;
  }
  
  } /* endswitch - field_var->type */

  // We're finished with the field values from the sweep file, so delete the
  // memory so we don't forget.

  delete field_values;

  // Fix missing beams, if requested.  Note that we only need to check the
  // current scan because the other scans were checked when received.

  if (_fixMissingBeams)
  {
    fl32 *new_scan_ptr = new_field_data + (nx * ny * field_hdr.nz);
	
    for (int y = 0; y < ny; ++y)
    {
      fl32 *new_beam_ptr = new_scan_ptr + (y * nx);
      bool data_found = false;
      
      for (int x = 0; x < nx; ++x)
      {
	if (new_beam_ptr[x] != mdv_missing_data_value &&
	    new_beam_ptr[x] != 0.0)
	{
	  data_found = true;
	  break;
	}
      } /* endfor - x */
      
      if (!data_found)
      {
	for (int x = 0; x < nx; ++x)
	  new_beam_ptr[x] = mdv_missing_data_value;
      }
      
    } /* endfor - y */
  }

  // Update the field

  if (_debug)
    cerr << "Updating MdvField data for field: " << field_name << endl;

  field.setVolData((void *)new_field_data,
		   volume_size * field_hdr.data_element_nbytes,
		   Mdvx::ENCODING_FLOAT32, Mdvx::SCALING_NONE);
  
  delete [] new_field_data;
  
  Mdvx::vlevel_header_t upd_vlevel_hdr = field.getVlevelHeader();
  upd_vlevel_hdr.type[field_hdr.nz] = Mdvx::VERT_TYPE_ELEV;
  upd_vlevel_hdr.level[field_hdr.nz] =
    _getScalarVarFloat(TARGET_ELEV_VAR_NAME);
  field.setVlevelHeader(upd_vlevel_hdr);
  
  Mdvx::field_header_t upd_field_hdr = field.getFieldHeader();
  upd_field_hdr.nx = nx;
  upd_field_hdr.ny = ny;
  upd_field_hdr.nz = nz;
  upd_field_hdr.volume_size = volume_size * upd_field_hdr.data_element_nbytes;
  upd_field_hdr.dz_constant = 0;
  field.setFieldHeader(upd_field_hdr);
  
  if (_debug)
    cerr << endl;

  return true;
}


/*********************************************************************
 * _createMdvFields() - Create the MDV fields from the data in the
 *                      sweep file.
 *
 * Returns true on success, false on failure.  The MDV fields are
 * stored in the class members.
 */

bool SweepFile::_createMdvFields(Mdvx &mdv_file)
{
  static const string method_name = "SweepFile::_createMdvFields()";
  
  // Get the list of fields to process

  vector< string > field_list = _getFieldList();
  
  if (field_list.size() == 0)
    return false;
  
  // Create each field in the list.  Add the field to the MDV file as
  // it is created.

  vector< string >::const_iterator field_name;
  for (field_name = field_list.begin(); field_name != field_list.end();
       ++field_name)
  {
    if (*field_name == "")
    {
      if (_debug)
	cerr << "    Skipping field with blank name" << endl;
      
      continue;
    }
    
    if (_debug)
      cerr << "    Processing field: <" << *field_name << ">" << endl;
    
    MdvxField *field;
    
    if ((field = mdv_file.getField(field_name->c_str())) == 0)
    {
      if (_debug)
	cerr << "Field not found in MDV file, trying to create new one" << endl;
      
      if ((field = _createMdvField(*field_name)) == 0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error creating MDV field for " << *field_name << endl;
	
	return false;
      }
      else
      {
	mdv_file.addField(field);
      }
    }
  
    if (!_updateMdvFieldData(*field_name, *field))
      return false;
    
  }
  
  return true;
}


/*********************************************************************
 * _getDimensions() - Get the data dimensions from the sweep file.
 *
 * Returns true on success, false on failure.  The data dimensions are
 * stored in the class members.
 */

bool SweepFile::_getDimensions()
{
  static const string method_name = "SweepFile::_getDimensions()";
  
  // Get the number of gates

  Nc3Dim *gates_dim;
  if ((gates_dim = _sweepFile->get_dim(_numGatesDimName.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading " << _numGatesDimName
	 << " dimension from input file: " << _filePath << endl;

    return false;
  }

  _numGates = gates_dim->size();

  // Get the number of azimuths

  Nc3Dim *az_dim;
  if ((az_dim = _sweepFile->get_dim(NUM_AZIMUTHS_DIM_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading " << NUM_AZIMUTHS_DIM_NAME
	 << " dimension from input file: " << _filePath << endl;

    return false;
  }

  _numInputAzimuths = az_dim->size();

  return true;
}


/*********************************************************************
 * _getFieldList() - Get the list of fields in the sweep file.
 *
 * Returns the list of fields retrieved from the sweep file.
 */

vector< string> SweepFile::_getFieldList() const
{
  static const string method_name = "SweepFile::_getFieldList()";
  
  vector< string > field_list;
  
  // Get the number of fields

  Nc3Dim *fields_dim;
  if ((fields_dim = _sweepFile->get_dim(NUM_FIELDS_DIM_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading " << NUM_FIELDS_DIM_NAME
	 << " dimension from input file: " << _filePath << endl;

    return field_list;
  }

  int num_fields = fields_dim->size();

  // Get the length of the field names

  Nc3Dim *field_name_len_dim;
  if ((field_name_len_dim =
       _sweepFile->get_dim(FIELD_NAME_LEN_DIM_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading " << NUM_FIELDS_DIM_NAME
	 << " dimension from input file: " << _filePath << endl;

    return field_list;
  }

  int num_field_chars = field_name_len_dim->size();

  // Get the list of fields

  Nc3Var *field_list_var;
  
  if ((field_list_var =
       _sweepFile->get_var(_fieldListVarName.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting " << _fieldListVarName
	 << " variable from input file: " << _filePath << endl;

    return field_list;
  }

  if (!field_list_var->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << _fieldListVarName << " var from input file is invalid: "
	 << _filePath << endl;

    return field_list;
  }

  Nc3Values *field_values;

  if ((field_values = field_list_var->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting values for " << _fieldListVarName
	 << " variable from input file: " << _filePath << endl;
    
    return field_list;
  }
  
  // Save the list of fields

  for (int field_num = 0; field_num < num_fields; ++field_num)
  {
    // Strip any trailing blanks from the field name

    string field_name = field_values->as_string(field_num * num_field_chars);
    int blank_index = field_name.find(' ');
    if (blank_index > 0)
      field_name = field_name.substr(0, blank_index);

    field_list.push_back(field_name);
  }

  delete field_values;
  
  return field_list;
}


/*********************************************************************
 * _getFieldVar() - Get the specified variable from the netCDF file.
 *
 * Returns a pointer to the variable on success, 0 on failure.
 */

Nc3Var *SweepFile::_getFieldVar(const string &field_name) const
{
  static const string method_name = "SweepFile::_getFieldVar()";

  if (_debug)
    cerr << "Getting NC variable object for field: " << field_name << endl;

  Nc3Var *field = 0;

  if ((field = _sweepFile->get_var(field_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting " << field_name 
	 << " variable from input file: " << _filePath << endl;

    return 0;
  }

  if (_debug)
    cerr << "Checking validity of variable object for field: "
	 << field_name << endl;

  if (!field->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << field_name << " var from input file is invalid: "
	 << _filePath << endl;

    return 0;
  }

  return field;
}


/*********************************************************************
 * _getGlobalIntAtt() - Get the specified global attribute from the
 *                      netCDF file as an integer.
 *
 * Returns the attribute value retrieved from the netCDF file on
 * success, the global INT_MISSING_DATA_VALUE on failure.
 */

int SweepFile::_getGlobalIntAtt(const string &att_name) const
{
  static const string method_name = "SweepFile::_getGlobalIntAtt()";
  
  if (_debug)
    cerr << "Getting global attribute from NC file:"
	 << att_name << endl;

  Nc3Att *attribute;
  
  if ((attribute = _sweepFile->get_att(att_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << att_name << " global attribute " << endl;
    cerr << "Sweep file: " << _filePath << endl;
    
    return INT_MISSING_DATA_VALUE;
  }
  
  return attribute->as_int(0);
}


/*********************************************************************
 * _getScalarVarFloat() - Get the specified scalar variable from the
 *                        netCDF file as a float value.
 *
 * Returns the variable value retrieved from the netCDF file on
 * success, the global FLOAT_MISSING_DATA_VALUE on failure.
 */

float SweepFile::_getScalarVarFloat(const string &variable_name,
				    const int data_index) const
{
  static const string method_name = "SweepFile::_getScalarVarFloat()";

  if (_debug)
    cerr << "Getting variable from NC file: " << variable_name << endl;

  Nc3Var *variable = 0;

  if ((variable = _sweepFile->get_var(variable_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting " << variable_name 
	 << " variable from input file: " << _filePath << endl;

    return FLOAT_MISSING_DATA_VALUE;
  }

  if (!variable->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << variable_name << " var from input file is invalid: "
	 << _filePath << endl;

    return FLOAT_MISSING_DATA_VALUE;
  }

  Nc3Values *variable_values;
  
  if ((variable_values = variable->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting value for " << variable_name
	 << " var from sweep file: " << _filePath << endl;
    
    return FLOAT_MISSING_DATA_VALUE;
  }
  
  float variable_value = variable_values->as_float(data_index);
  delete variable_values;
  
  return variable_value;
}


/*********************************************************************
 * _getScalarVarInt() - Get the specified scalar variable from the
 *                      netCDF file as an integer value.
 *
 * Returns the variable value retrieved from the netCDF file on
 * success, the global INT_MISSING_DATA_VALUE on failure.
 */

int SweepFile::_getScalarVarInt(const string &variable_name,
				const int data_index) const
{
  static const string method_name = "SweepFile::_getScalarVarInt()";

  Nc3Var *variable = 0;

  if ((variable = _sweepFile->get_var(variable_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting " << variable_name 
	 << " variable from input file: " << _filePath << endl;

    return INT_MISSING_DATA_VALUE;
  }

  if (!variable->is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << variable_name << " var from input file is invalid: "
	 << _filePath << endl;

    return INT_MISSING_DATA_VALUE;
  }

  Nc3Values *variable_values;
  
  if ((variable_values = variable->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting value for " << variable_name
	 << " var from sweep file: " << _filePath << endl;
    
    return INT_MISSING_DATA_VALUE;
  }
  
  int variable_value = variable_values->as_int(data_index);
  delete variable_values;
  
  return variable_value;
}


/*********************************************************************
 * _getVarFloatAtt() - Get the specified attribute from the given
 *                     netCDF variable as a float.
 *
 * Returns the attribute value retrieved from the netCDF file on
 * success, the global FLOAT_MISSING_DATA_VALUE on failure.
 */

float SweepFile::_getVarFloatAtt(const Nc3Var &variable,
				 const string &att_name) const
{
  static const string method_name = "SweepFile::_getVarFloatAtt()";
  
  if (_debug)
    cerr << "Getting variable attribute from NC file:"
	 << att_name << endl;

  Nc3Att *attribute;
  
  if ((attribute = variable.get_att(att_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << att_name << " attribute for variable "
	 << variable.name() << endl;
    cerr << "Sweep file: " << _filePath << endl;
    
    return FLOAT_MISSING_DATA_VALUE;
  }
  
  Nc3Values *att_values;
  
  if ((att_values = attribute->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << att_name << " attribute value for variable "
	 << variable.name() << endl;
    cerr << "Sweep file: " << _filePath << endl;
    
    return FLOAT_MISSING_DATA_VALUE;
  }
  
  float att_value = att_values->as_float(0);
  
  delete att_values;
  
  return att_value;
}


/*********************************************************************
 * _getVarStringAtt() - Get the specified attribute from the given
 *                      netCDF variable as a string.
 *
 * Returns the attribute value retrieved from the netCDF file on
 * success, "" on failure.
 */

string SweepFile::_getVarStringAtt(const Nc3Var &variable,
				   const string &att_name) const
{
  static const string method_name = "SweepFile::_getVarStringAtt()";
  
  Nc3Att *attribute;
  
  if ((attribute = variable.get_att(att_name.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << att_name << " attribute for variable "
	 << variable.name() << endl;
    cerr << "Sweep file: " << _filePath << endl;
    
    return "";
  }
  
  Nc3Values *att_values;
  
  if ((att_values = attribute->values()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << att_name << " attribute value for variable "
	 << variable.name() << endl;
    cerr << "Sweep file: " << _filePath << endl;
    
    return "";
  }
  
  string att_value = att_values->as_string(0);
  
  delete att_values;
  
  return att_value;
}


/*********************************************************************
 * _updateMasterHeader() - Update the master header in the given MDV
 *                         file based on the information in the sweep
 *                         file.
 */

void SweepFile::_updateMasterHeader(Mdvx &mdv_file) const
{
  static const string method_name = "SweepFile::initMasterHeader()";
  
  DsMdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = _getScalarVarInt(BASE_TIME_VAR_NAME);
  master_hdr.time_end = master_hdr.time_begin;
  master_hdr.time_centroid = master_hdr.time_begin;
  master_hdr.time_expire = master_hdr.time_end;
  master_hdr.data_dimension = 3;
  master_hdr.data_collection_type = Mdvx::DATA_MEASURED;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  master_hdr.sensor_lon = _getScalarVarFloat(LONGITUDE_VAR_NAME);
  master_hdr.sensor_lat = _getScalarVarFloat(LATITUDE_VAR_NAME);
  master_hdr.sensor_alt = _getScalarVarFloat(ALTITUDE_VAR_NAME);
  STRcopy(master_hdr.data_set_info, "SweepNetCDF2Mdv", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "SweepNetCDF2Mdv", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, _filePath.c_str(), MDV_NAME_LEN);
  
  mdv_file.setMasterHeader(master_hdr);
}
