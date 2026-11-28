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
 * @file SweepFile.cc
 */
#include "SweepFile.hh"
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>

#include <Ncxx/Nc3File.hh>
#include <cmath>

using std::string;


// Define globals

const string SweepFile::NUM_AZIMUTHS_DIM_NAME = "Azimuth";
const string SweepFile::GATE_SPACING_VAR_NAME = "GateWidth";
const string SweepFile::AZIMUTH_VAR_NAME = "Azimuth";
const string SweepFile::AZIMUTH_SPACING_VAR_NAME = "AzimuthalSpacing";
const string SweepFile::VOL_NUM_GLOBAL_ATT_NAME = "Volume_Number";
const string SweepFile::VOLUME_START_TIME_GLOBAL_ATT_NAME = "Time";
const string SweepFile::LATITUDE_GLOBAL_ATT_NAME = "Latitude";
const string SweepFile::LONGITUDE_GLOBAL_ATT_NAME = "Longitude";
const string SweepFile::ALTITUDE_GLOBAL_ATT_NAME = "Height";
const string SweepFile::START_RANGE_GLOBAL_ATT_NAME = "RangeToFirstGate";
const string SweepFile::TARGET_ELEV_GLOBAL_ATT_NAME = "Elevation";
const string SweepFile::RADAR_NAME_GLOBAL_ATT_NAME = "radarName-value";
const float SweepFile::FLOAT_MISSING_DATA_VALUE = -32768.0f;
const int SweepFile::INT_MISSING_DATA_VALUE = -32768;

//-------------------------------------------------------------------
SweepFile::SweepFile(const NsslData &data, const FieldSpec &spec,
                     const string &num_gates_dim_name,
		     bool data_driven_beamwidth,
		     int num_allowed_output_beamwidth,
		     double *allowed_output_beamwidth,
		     double output_beamwidth,
		     bool force_negative_longitude) :
  _data(data), _spec(spec),
  _numGatesDimName(num_gates_dim_name),
  _numGates(-1),
  _numInputAzimuths(-1),
  _numOutputAzimuths((int)(360.0/output_beamwidth)),
  _dataDrivenBeamWidth(data_driven_beamwidth),
  _outputBeamWidth(output_beamwidth),
  _forceNegativeLongitude(force_negative_longitude),
  _objectInitialized(false)


{
  for (int i=0; i<num_allowed_output_beamwidth; ++i)
  {
    _allowedOutputBeamWidth.push_back(allowed_output_beamwidth[i]);
  }
  _azimuthLut.setBeamWidth(output_beamwidth);
}
  
//-------------------------------------------------------------------
SweepFile::~SweepFile()
{
  _sweepFile->close();
  delete _sweepFile;
}

//-------------------------------------------------------------------
bool SweepFile::getVolumeNumber(int &volume_number) const
{
  volume_number = _getGlobalIntAtt(VOL_NUM_GLOBAL_ATT_NAME);
  return true;
}

//-------------------------------------------------------------------
bool SweepFile::getElevAngle(double &elev) const
{
  elev = _getGlobalFloatAtt(TARGET_ELEV_GLOBAL_ATT_NAME);
  return true;
}

//-------------------------------------------------------------------
bool SweepFile::getVolumeStartTime(DateTime &volume_start_time) const
{
  volume_start_time.set(_getGlobalIntAtt(VOLUME_START_TIME_GLOBAL_ATT_NAME));
  return true;
}

//-------------------------------------------------------------------
bool SweepFile::getRadarName(std::string &name) const
{
  name = _getGlobalStringAtt(RADAR_NAME_GLOBAL_ATT_NAME);
  return true;
}

//-------------------------------------------------------------------
bool SweepFile::initialize(void)
{
  LOG(DEBUG_VERBOSE) << "Initializing SweepFile object...";

  _objectInitialized = false;
  
  // Create an error object so that the netCDF library doesn't exit when an
  // error is encountered.  This object is not explicitly used in the below
  // code, but is used implicitly by the netCDF library.

  _ncError = new Nc3Error(Nc3Error::silent_nonfatal);

  // Initialize the input netCDF file

  _sweepFile = new Nc3File(_data.getPath().c_str());
  if (!_sweepFile->is_valid())
  {
    LOG(ERROR) << "Sweep file isn't valid: " << _data.getPath();
    _sweepFile->close();
    return false;
  }
  
  _objectInitialized = true;
  
  LOG(DEBUG_VERBOSE) << "... SweepFile object successfully initialized";
  return true;
}

//-------------------------------------------------------------------
bool SweepFile::addSweepToMdv(Mdvx &mdv_file)
{
  // Make sure the object was initialized

  if (!_objectInitialized)
  {
    LOG(ERROR) << "Object not initialized";
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
  {
    _updateMasterHeader(mdv_file);
  }
  
  return _createMdvFields(mdv_file);
}

//-------------------------------------------------------------------
bool SweepFile::_createAzimuthLookupTable(void)
{
  // Get the azimuth data

  Nc3Var *az_var;

  if ((az_var = _getFieldVar(AZIMUTH_VAR_NAME)) == 0)
    return false;

  LOG(DEBUG_VERBOSE) << "Getting azimuth values for variable: " 
		     << AZIMUTH_VAR_NAME;

  Nc3Values *az_values;
  if ((az_values = az_var->values()) == 0)
  {
    LOG(ERROR) << "extracting values for " << AZIMUTH_VAR_NAME
	       << " field from sweep file: " << _data.getPath();
    return false;
  }
  
  // Update the lookup table with the current azimuth locations

  _azimuthLut.clear();
  
  if (_dataDrivenBeamWidth)
  {
    if (_numInputAzimuths < 1)
    {
      _azimuthLut.setBeamWidth(_outputBeamWidth);
      _numOutputAzimuths = (int)(360.0/_outputBeamWidth);
    }
    else
    {
      Nc3Var *az_spacing_var;
      if ((az_spacing_var = _getFieldVar(AZIMUTH_SPACING_VAR_NAME)) == 0)
	return false;

      Nc3Values *az_spacing_values;
      if ((az_spacing_values = az_spacing_var->values()) == 0)
      {
	LOG(ERROR) << "extracting values for " << AZIMUTH_SPACING_VAR_NAME
		   << " field from sweep file: " << _data.getPath();
	return false;
      }
      double sum=0.0;
      for (int i=0; i<_numInputAzimuths; ++i)
      {
	sum += az_spacing_values->as_float(i);
      }
      sum /= (double)_numInputAzimuths;

      double best = _outputBeamWidth;
      double best_delta = 0;
      for (size_t i=0; i<_allowedOutputBeamWidth.size(); ++i)
      {
	if (i == 0)
	{
	  best_delta = fabs(_allowedOutputBeamWidth[i] - sum);
	  best = _allowedOutputBeamWidth[i];
	}
	else
	{
	  double delta = fabs(_allowedOutputBeamWidth[i] - sum);
	  if (delta < best_delta)
	  {
	    best_delta = delta;
	    best = _allowedOutputBeamWidth[i];
	  }
	}
      }
      
      LOG(DEBUG) << "Setting azimuthal spacing to " << best;
      _outputBeamWidth = best;
      _azimuthLut.setBeamWidth(best);
      _numOutputAzimuths = (int)(360.0/_outputBeamWidth);
    }
  }
  else
  {
    _azimuthLut.setBeamWidth(_outputBeamWidth);
    _numOutputAzimuths = (int)(360.0/_outputBeamWidth);
  }
  
  for (int i = 0; i < _numInputAzimuths; ++i)
    _azimuthLut.addAzIndex(az_values->as_float(i), i);
  
  return true;
}

//-------------------------------------------------------------------
/*********************************************************************
 * _updateMasterHeader() - Update the master header in the given MDV
 *                         file based on the information in the sweep
 *                         file.
 */

void SweepFile::_updateMasterHeader(Mdvx &mdv_file) const
{
  DsMdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = _getGlobalIntAtt(VOLUME_START_TIME_GLOBAL_ATT_NAME);
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
  master_hdr.sensor_lon = _getGlobalFloatAtt(LONGITUDE_GLOBAL_ATT_NAME);
  master_hdr.sensor_lat = _getGlobalFloatAtt(LATITUDE_GLOBAL_ATT_NAME);

  // convert meters to km here, which is obviously hardwired
  master_hdr.sensor_alt = _getGlobalFloatAtt(ALTITUDE_GLOBAL_ATT_NAME)/1000.0;

  STRcopy(master_hdr.data_set_info, "SweepNetCDF2Mdv", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "SweepNetCDF2Mdv", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, _data.getPath().c_str(), MDV_NAME_LEN);
  
  mdv_file.setMasterHeader(master_hdr);
}

//-------------------------------------------------------------------
bool SweepFile::_createMdvFields(Mdvx &mdv_file)
{
  if (_spec.field_name.empty())
  {
    LOG(DEBUG) << "    Skipping field with blank name";
    return true;
  }
    
  LOG(DEBUG_VERBOSE) << "    Processing field: <" << _spec.field_name << ">";
    
  MdvxField *field;
    
  if ((field = mdv_file.getField(_spec.field_name.c_str())) == 0)
  {
    LOG(DEBUG_VERBOSE) << "Field not found in MDV, trying to create new one";
    if ((field = _createMdvField()) == 0)
    {
      LOG(ERROR) << "creating MDV field for " << _spec.field_name;
      return false;
    }
    else
    {
      mdv_file.addField(field);
    }
  }
  return _updateMdvFieldData(*field);
}


//-------------------------------------------------------------------
MdvxField *SweepFile::_createMdvField(void) const
{
  // Access the field data

  Nc3Var *field_var;

  if ((field_var = _getFieldVar(_spec.field_name)) == 0)
    return 0;

  // Construct the field header
  LOG(DEBUG_VERBOSE) << "Constructing field header for field:" 
		     << _spec.field_name;

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
  field_hdr.proj_origin_lat = _getGlobalFloatAtt(LATITUDE_GLOBAL_ATT_NAME);
  field_hdr.proj_origin_lon = _getGlobalFloatAtt(LONGITUDE_GLOBAL_ATT_NAME);
  if (_forceNegativeLongitude &&
      field_hdr.proj_origin_lon > 0.0)
    field_hdr.proj_origin_lon = -field_hdr.proj_origin_lon;
  field_hdr.grid_dx = _getScalarVarFloat(GATE_SPACING_VAR_NAME) / 1000.0;
  field_hdr.grid_dy = _outputBeamWidth;
  field_hdr.grid_dz = 1.0;
  field_hdr.grid_minx = _getGlobalFloatAtt(START_RANGE_GLOBAL_ATT_NAME)/1000.0;
  field_hdr.grid_miny = 0.0;
  field_hdr.grid_minz = _getGlobalFloatAtt(TARGET_ELEV_GLOBAL_ATT_NAME);

  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;

  if (_spec.override_missing)
    field_hdr.bad_data_value = _spec.missing_data_value;
  else
  {
    if (_spec.missing_value_is_global)
    {
      field_hdr.bad_data_value =
	_getGlobalFloatAtt(_spec.missing_data_value_att_name);
    }
    else
    {
      field_hdr.bad_data_value =
	_getVarFloatAtt(*field_var, _spec.missing_data_value_att_name);
    }
  }
  field_hdr.missing_data_value = field_hdr.bad_data_value;
  STRcopy(field_hdr.field_name_long,
	  _spec.field_name.c_str(), 
	  // _getVarStringAtt(*field_var, FIELD_NAME_LONG_ATT_NAME).c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, _spec.field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units,
	  _getVarStringAtt(*field_var, _spec.units_att_name).c_str(),
	  MDV_UNITS_LEN);
  
  // Construct the vlevel header

  LOG(DEBUG_VERBOSE) << "Constructing vlevel header for field:" 
		     << _spec.field_name;

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  // Create the field

  LOG(DEBUG_VERBOSE) << "Creating MdvField object for field: " 
		     << _spec.field_name;

  MdvxField *mdv_field = new MdvxField(field_hdr, vlevel_hdr);
  return mdv_field;
}



//-------------------------------------------------------------------
bool SweepFile::_updateMdvFieldData(MdvxField &field)const
{
  // Get the field data

  Nc3Var *field_var;

  if ((field_var = _getFieldVar(_spec.field_name)) == 0)
    return false;

  LOG(DEBUG_VERBOSE) << "Getting field values for variable: " 
		     << _spec.field_name;

  Nc3Values *field_values;
  if ((field_values = field_var->values()) == 0)
  {
    LOG(ERROR) << "extracting values for " << _spec.field_name
	       << " field from sweep file: " << _data.getPath();
    
    return false;
  }
  
  // Check for consistency

  Mdvx::field_header_t field_hdr = field.getFieldHeader();
  
  if (_numOutputAzimuths != field_hdr.ny)
  {
    LOG(ERROR) << "Number of azimuths changed between sweeps in volume";
    LOG(ERROR) << "Current sweeps have " << field_hdr.ny << " azimuths";
    LOG(ERROR) << "New sweep has " << _numOutputAzimuths << " azimuths";
    LOG(ERROR) << "Cannot process sweep";
    return false;
  }
  
  // Determine the new data dimensions

  int nx, ny, nz;

  nx = (_numGates > field_hdr.nx) ? _numGates : field_hdr.nx;
  ny = field_hdr.ny;
  nz = field_hdr.nz + 1;
  
  int plane_size = nx * ny;
  int volume_size = plane_size * nz;
  
  fl32 sweep_missing_data_value;
  if (_spec.override_missing)
    sweep_missing_data_value = _spec.missing_data_value;
  else
  {
    if (_spec.missing_value_is_global)
    {
      sweep_missing_data_value = 
	_getGlobalFloatAtt(_spec.missing_data_value_att_name);
    }
    else
    {
      sweep_missing_data_value = 
	_getVarFloatAtt(*field_var, _spec.missing_data_value_att_name);
    }
  }

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
    double scale = _getVarFloatAtt(*field_var, _spec.scale_att_name);
    double bias;
    if (_spec.bias_specified)
      bias = _getVarFloatAtt(*field_var, _spec.bias_att_name);
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
    LOG(ERROR) << "Don't yet handle this netCDF data type";
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

  if (_spec.fix_missing_beams)
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

  LOG(DEBUG_VERBOSE) << "Updating MdvField data for field: " 
		     << _spec.field_name;

  field.setVolData((void *)new_field_data,
		   volume_size * field_hdr.data_element_nbytes,
		   Mdvx::ENCODING_FLOAT32, Mdvx::SCALING_NONE);
  
  delete [] new_field_data;
  
  Mdvx::vlevel_header_t upd_vlevel_hdr = field.getVlevelHeader();
  upd_vlevel_hdr.type[field_hdr.nz] = Mdvx::VERT_TYPE_ELEV;
  upd_vlevel_hdr.level[field_hdr.nz] =
    _getGlobalFloatAtt(TARGET_ELEV_GLOBAL_ATT_NAME);
  field.setVlevelHeader(upd_vlevel_hdr);
  
  Mdvx::field_header_t upd_field_hdr = field.getFieldHeader();
  upd_field_hdr.nx = nx;
  upd_field_hdr.ny = ny;
  upd_field_hdr.nz = nz;
  upd_field_hdr.volume_size = volume_size * upd_field_hdr.data_element_nbytes;
  upd_field_hdr.dz_constant = 0;
  field.setFieldHeader(upd_field_hdr);
  
  return true;
}

//-------------------------------------------------------------------
bool SweepFile::_getDimensions()
{
  Nc3Dim *gates_dim;
  if ((gates_dim = _sweepFile->get_dim(_numGatesDimName.c_str())) == 0)
  {
    LOG(ERROR) << "Error reading " << _numGatesDimName
	       << " dimension from input file";
    return false;
  }
  Nc3Dim *az_dim;
  if ((az_dim = _sweepFile->get_dim(NUM_AZIMUTHS_DIM_NAME.c_str())) == 0)
  {
    LOG(ERROR) << "Error reading " << NUM_AZIMUTHS_DIM_NAME
	       << " dimension from input file";
    return false;
  }
  _numGates = gates_dim->size();
  _numInputAzimuths = az_dim->size();
  return true;
}

//-------------------------------------------------------------------
Nc3Var *SweepFile::_getFieldVar(const string &field_name) const
{
  LOG(DEBUG_VERBOSE) << "Getting NC variable object for field: " << field_name;

  Nc3Var *field = 0;
  if ((field = _sweepFile->get_var(field_name.c_str())) == 0)
  {
    LOG(ERROR) << "extracting " << field_name 
	       << " variable from input file: " << _data.getPath();

    return 0;
  }

  LOG(DEBUG_VERBOSE) << "Checking validity of variable object for field: "
		     << field_name;

  if (!field->is_valid())
  {
    LOG(ERROR) << field_name << " var from input file is invalid: "
	       << _data.getPath();
    return 0;
  }

  return field;
}

//-------------------------------------------------------------------
int SweepFile::_getGlobalIntAtt(const string &att_name) const
{
  LOG(DEBUG_VERBOSE) << "Getting global attribute from NC file:" << att_name;

  Nc3Att *attribute;
  
  if ((attribute = _sweepFile->get_att(att_name.c_str())) == 0)
  {
    LOG(ERROR) << "getting " << att_name << " global attribute ";
    LOG(ERROR) << "Sweep file: " << _data.getPath();
    return INT_MISSING_DATA_VALUE;
  }
  
  return attribute->as_int(0);
}

//-------------------------------------------------------------------
string SweepFile::_getGlobalStringAtt(const string &att_name) const
{
  LOG(DEBUG_VERBOSE) << "Getting global attribute from NC file:" << att_name;

  Nc3Att *attribute;
  
  if ((attribute = _sweepFile->get_att(att_name.c_str())) == 0)
  {
    LOG(ERROR) << "getting " << att_name << " global attribute ";
    LOG(ERROR) << "Sweep file: " << _data.getPath();
    return "";
  }
  
  return attribute->as_string(0);
}

/*********************************************************************
 * _getGlobalFloatAtt() - Get the specified global attribute from the
 *                      netCDF file as an float
 *
 * Returns the attribute value retrieved from the netCDF file on
 * success, the global INT_MISSING_DATA_VALUE on failure.
 */

float SweepFile::_getGlobalFloatAtt(const string &att_name) const
{
  LOG(DEBUG_VERBOSE) << "Getting global attribute from NC file:" << att_name;

  Nc3Att *attribute;
  
  if ((attribute = _sweepFile->get_att(att_name.c_str())) == 0)
  {
    LOG(ERROR) << "getting " << att_name << " global attribute ";
    LOG(ERROR) << "Sweep file: " << _data.getPath();
    return FLOAT_MISSING_DATA_VALUE;
  }
  
  return attribute->as_float(0);
}

//-------------------------------------------------------------------
float SweepFile::_getScalarVarFloat(const string &variable_name,
				    const int data_index) const
{
  LOG(DEBUG_VERBOSE) << "Getting variable from NC file: " << variable_name;

  Nc3Var *variable = 0;

  if ((variable = _sweepFile->get_var(variable_name.c_str())) == 0)
  {
    LOG(ERROR) << "extracting " << variable_name 
	       << " variable from input file: " << _data.getPath();
    return FLOAT_MISSING_DATA_VALUE;
  }

  if (!variable->is_valid())
  {
    LOG(ERROR) << variable_name << " var from input file is invalid: "
	       <<  _data.getPath();

    return FLOAT_MISSING_DATA_VALUE;
  }

  Nc3Values *variable_values;
  
  if ((variable_values = variable->values()) == 0)
  {
    LOG(ERROR) << "getting value for " << variable_name
	       << " var from sweep file: " << _data.getPath();
    return FLOAT_MISSING_DATA_VALUE;
  }
  
  float variable_value = variable_values->as_float(data_index);
  delete variable_values;
  return variable_value;
}


//-------------------------------------------------------------------
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
  Nc3Var *variable = 0;

  if ((variable = _sweepFile->get_var(variable_name.c_str())) == 0)
  {
    LOG(ERROR) << "extracting " << variable_name 
	       << " variable from input file: " << _data.getPath();
    return INT_MISSING_DATA_VALUE;
  }
  else
  {
    if (!variable->is_valid())
    {
      LOG(ERROR) << variable_name << " var from input file is invalid: "
		 << _data.getPath();
      return INT_MISSING_DATA_VALUE;
    }
  }
  Nc3Values *variable_values;
  if ((variable_values = variable->values()) == 0)
  {
    LOG(ERROR) << "getting value for " << variable_name
	       << " var from sweep file: " << _data.getPath();
    return INT_MISSING_DATA_VALUE;
  }
  else
  {
    int variable_value = variable_values->as_int(data_index);
    delete variable_values;
    return variable_value;
  }

}

//-------------------------------------------------------------------

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
  LOG(DEBUG_VERBOSE) << "Getting variable attribute from NC file:" << att_name;

  Nc3Att *attribute;
  
  if ((attribute = variable.get_att(att_name.c_str())) == 0)
  {
    LOG(ERROR) << "getting " << att_name << " attribute for variable "
	       << variable.name();
    LOG(ERROR) << "Sweep file: " << _data.getPath();
    return FLOAT_MISSING_DATA_VALUE;
  }
  
  Nc3Values *att_values;
  
  if ((att_values = attribute->values()) == 0)
  {
    LOG(ERROR) << "getting " << att_name << " attribute value for variable "
	       << variable.name();
    LOG(ERROR) << "Sweep file: " << _data.getPath();
    return FLOAT_MISSING_DATA_VALUE;
  }
  
  float att_value = att_values->as_float(0);
  
  delete att_values;
  
  return att_value;
}


//-------------------------------------------------------------------
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
  Nc3Att *attribute;
  
  if ((attribute = variable.get_att(att_name.c_str())) == 0)
  {
    LOG(ERROR) << "getting " << att_name << " attribute for variable "
	       << variable.name();
    LOG(ERROR) << "Sweep file: " << _data.getPath();
    
    return "";
  }
  
  Nc3Values *att_values;
  
  if ((att_values = attribute->values()) == 0)
  {
    LOG(ERROR) << "getting " << att_name << " attribute value for variable "
	       << variable.name();
    LOG(ERROR) << "Sweep file: " << _data.getPath();
    
    return "";
  }
  
  string att_value = att_values->as_string(0);
  
  delete att_values;
  
  return att_value;
}


