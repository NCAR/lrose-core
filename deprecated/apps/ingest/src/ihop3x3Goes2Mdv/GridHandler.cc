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
//   $Date: 2016/03/07 01:23:08 $
//   $Id: GridHandler.cc,v 1.7 2016/03/07 01:23:08 dixon Exp $
//   $Revision: 1.7 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * GridHandler: GridHandler program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2003
 *
 * Nancy Rehak/Kay Levesque
 *
 *********************************************************************/

#include <toolsa/str.h>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <rapmath/math_macros.h>

#include "GridHandler.hh"

using namespace std;

// Global variables

const double GridHandler::MISSING_DATA_VALUE = -9999.0;

/*********************************************************************
 * Constructor
 */

GridHandler::GridHandler(const bool debug):
  _debug(debug),
  _dataFound(false)
{

}


/*********************************************************************
 * Destructor
 */

GridHandler::~GridHandler()
{

}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool GridHandler::init(const MdvxPjg& projection, const vector<double> pressureLevels)
{
  _dataFound = false;
  _projection = projection;
  _pressureLevels = pressureLevels;
  _startTime = DateTime::NEVER;
  _endTime = DateTime::NEVER;

  if ((_temperatureField = _createTemperatureField(projection, pressureLevels)) == 0)
    return false;

  if ((_temperatureCField = _createTemperatureCField(projection, pressureLevels)) == 0)
    return false;

  if ((_dewpointField = _createDewpointField(projection, pressureLevels)) == 0)
    return false;

  if ((_dewpointCField = _createDewpointCField(projection, pressureLevels)) == 0)
    return false;

  if ((_pressureField = _createPressureField(projection, pressureLevels)) == 0)
    return false;

  if ((_heightField = _createHeightField(projection, pressureLevels)) == 0)
    return false;

  return true;
}


/*********************************************************************
 * updateGrid() - update the corresponding grid values for the given sounding
 *
 * Returns true if successful, false otherwise.
 */

bool GridHandler::updateGrid(const sounder_data_t &sounder_data)
{
  int x_index, y_index;
  if (_projection.latlon2xyIndex(sounder_data.latitude, sounder_data.longitude,
			     x_index, y_index) != 0)
    {
      cerr << "WARNING: sounding is outside of grid." << endl;
      cerr << "       lat: <" << sounder_data.latitude << ">" << endl;
      cerr << "       lon: <" << sounder_data.longitude << ">" << endl;
      cerr << "       projection info: " << endl;
      _projection.print(cerr);
      return true;
    }

  int z_index = -1;
  for (size_t i = 0; i < _pressureLevels.size(); ++i)
    {
      if (sounder_data.pressure == _pressureLevels[i])
	{
	  z_index = i;
	  break;
	}
    }
  if (z_index < 0)
    {
      cerr << "WARNING: pressure level: <" << sounder_data.pressure << "> is outside of grid." << endl;
      return true;
    }
  int array_index = _projection.xyIndex2arrayIndex(x_index, y_index, z_index);
  if (array_index < 0)
    {
      cerr << "ERROR: programming error." << endl;
      cerr << "       Calculated array index out of range." << endl;
      cerr << "       x and y indices: " << endl;
      cerr << "x: " << x_index << " y: " << y_index << " z: " << z_index << endl;
      return false;
    }

  // Set the start and end times.

  if (_startTime == DateTime::NEVER ||
      sounder_data.date_time < _startTime)
    {
      _startTime = sounder_data.date_time;
    }
  
  if (_endTime == DateTime::NEVER ||
      sounder_data.date_time > _endTime)
    {
      _endTime = sounder_data.date_time;
    }

  // Update the Kelvin temperature grid value.

  fl32 *data = (fl32*)_temperatureField->getVol();

  if (data[array_index] != MISSING_DATA_VALUE)
    {
      if (_debug)
	cerr << "Warning! 2 soundings in same location." << endl;
    }

  data[array_index] = sounder_data.temp;

  // Update the Celsius temperature grid value.

  data = (fl32*)_temperatureCField->getVol();
  data[array_index] = TEMP_K_TO_C(sounder_data.temp);

  // Update the Kelvin dewpoint grid value.

  data = (fl32*)_dewpointField->getVol();
  data[array_index] = sounder_data.dewpoint;

  // Update the Celsius dewpoint grid value.

  data = (fl32*)_dewpointCField->getVol();
  data[array_index] = TEMP_K_TO_C(sounder_data.dewpoint);

  // Update the pressure grid value.

  data = (fl32*)_pressureField->getVol();
  data[array_index] = sounder_data.pressure;

  // Update the height grid value.

  data = (fl32*)_heightField->getVol();
  data[array_index] = sounder_data.height;

  _dataFound = true;
  return true;
}


/*********************************************************************
 * writeGrid() - write grid to given URL.
 *
 * Returns true if successful, false otherwise.
 */

bool GridHandler::writeGrid(const string &url, const string &dataSource)
{
  if(!_dataFound)
    {
      cerr << "No data in grid. Not writing file." << endl;
      return false;
    }

  DsMdvx mdvFile;
  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = _startTime.utime();
  master_hdr.time_end = _endTime.utime();
  master_hdr.time_centroid = _startTime.utime() + time_t((_endTime.utime() - _startTime.utime()) / 2);
  //  cerr << "centroid time: " << master_hdr.time_centroid << endl;
  //  cerr << time_t(_startTime.utime() + _endTime.utime()) / 2;
  master_hdr.time_expire = master_hdr.time_end;
  master_hdr.data_dimension = 3;
  master_hdr.data_collection_type = Mdvx::DATA_MEASURED;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  master_hdr.n_fields = 0;
  master_hdr.max_nx = 0;
  master_hdr.max_ny = 0;
  master_hdr.max_nz = 0;
  master_hdr.n_chunks = 0;
  STRcopy(master_hdr.data_set_info, 
	  "Goes3x3 Sounding Data generated by ihop3x3Goes2Mdv", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "Goes3x3 Sounding Data", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, dataSource.c_str(), MDV_NAME_LEN);
  
  mdvFile.setMasterHeader(master_hdr);

  if(_temperatureField->convertType(Mdvx::ENCODING_INT8,
				    Mdvx::COMPRESSION_BZIP,
				    Mdvx::SCALING_DYNAMIC) != 0)
    {
      cerr << "Unable to convert type for Kelvin temperature field" << endl;
      return false;
    }

  if(_temperatureCField->convertType(Mdvx::ENCODING_INT8,
				     Mdvx::COMPRESSION_BZIP,
				     Mdvx::SCALING_DYNAMIC) != 0)
    {
      cerr << "Unable to convert type for Celsius temperature field" << endl;
      return false;
    }

  if(_dewpointField->convertType(Mdvx::ENCODING_INT8,
				    Mdvx::COMPRESSION_BZIP,
				    Mdvx::SCALING_DYNAMIC) != 0)
    {
      cerr << "Unable to convert type for Kelvin dewpoint field" << endl;
      return false;
    }

  if(_dewpointCField->convertType(Mdvx::ENCODING_INT8,
				  Mdvx::COMPRESSION_BZIP,
				  Mdvx::SCALING_DYNAMIC) != 0)
    {
      cerr << "Unable to convert type for Celsius dewpoint field" << endl;
      return false;
    }

  if(_pressureField->convertType(Mdvx::ENCODING_INT8,
				    Mdvx::COMPRESSION_BZIP,
				    Mdvx::SCALING_DYNAMIC) != 0)
    {
      cerr << "Unable to convert type for pressure field" << endl;
      return false;
    }

  if(_heightField->convertType(Mdvx::ENCODING_INT8,
				    Mdvx::COMPRESSION_BZIP,
				    Mdvx::SCALING_DYNAMIC) != 0)
    {
      cerr << "Unable to convert type for height field" << endl;
      return false;
    }

  mdvFile.addField(_temperatureField);
  mdvFile.addField(_temperatureCField);
  mdvFile.addField(_dewpointField);
  mdvFile.addField(_dewpointCField);
  mdvFile.addField(_pressureField);
  mdvFile.addField(_heightField);

  mdvFile.setWriteLdataInfo();

  if(mdvFile.writeToDir(url.c_str()) != 0)
    {
      cerr << "Write to url unsuccessful: <" << url << ">" << endl;
      return false;
    }

  return true;
}


/**********************************************************************
 *              Private Methods
 **********************************************************************/


/*********************************************************************
 * _createTemperatureField() - Create the needed temperature field with
 * all of the data filled in with MISSING_DATA_VALUE.
 *
 * Returns the pointer to the created field on success, 0 on failure.
 */

MdvxField* GridHandler::_createTemperatureField(const MdvxPjg& projection,
						const vector<double> pressureLevels)
{

  // Create the field header
  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));

  projection.syncToFieldHdr(field_hdr);

  field_hdr.nz = pressureLevels.size();

  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz * 
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  field_hdr.dz_constant = FALSE;
  
  field_hdr.grid_dz = 0;

  field_hdr.grid_minz = 0;

  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;

  field_hdr.bad_data_value = MISSING_DATA_VALUE;
  field_hdr.missing_data_value = MISSING_DATA_VALUE;
  field_hdr.min_value = 0;
  field_hdr.max_value = 0;
  STRcopy(field_hdr.field_name_long, "temperature", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "temp", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "K", MDV_UNITS_LEN);

  // Create the vertical level header

  Mdvx::vlevel_header_t vLevel_hdr;
  memset(&vLevel_hdr, 0, sizeof(vLevel_hdr));

  for(size_t i = 0; i < pressureLevels.size(); ++i)
    {
      vLevel_hdr.type[i] = Mdvx::VERT_TYPE_PRESSURE;
      vLevel_hdr.level[i] = pressureLevels[i];
    }

  if(_debug)
    {
      Mdvx::printFieldHeader(field_hdr, cerr);
      Mdvx::printVlevelHeader(vLevel_hdr, field_hdr.nz, field_hdr.field_name_long, cerr);
    }

  // create an mdv field object,
  // initializing entire grid with field_hdr.missing_data_value

  return new MdvxField(field_hdr, vLevel_hdr, (void*)NULL, true);
}


/*********************************************************************
 * _createTemperatureCField() - Create the needed Celsius temperature field with
 * all of the data filled in with MISSING_DATA_VALUE.
 *
 * Returns the pointer to the created field on success, 0 on failure.
 */

MdvxField* GridHandler::_createTemperatureCField(const MdvxPjg& projection,
						 const vector<double> pressureLevels)
{

  // Create the field header
  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));

  projection.syncToFieldHdr(field_hdr);

  field_hdr.nz = pressureLevels.size();

  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz * 
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  field_hdr.dz_constant = FALSE;
  
  field_hdr.grid_dz = 0;

  field_hdr.grid_minz = 0;

  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;

  field_hdr.bad_data_value = MISSING_DATA_VALUE;
  field_hdr.missing_data_value = MISSING_DATA_VALUE;
  field_hdr.min_value = 0;
  field_hdr.max_value = 0;
  STRcopy(field_hdr.field_name_long, "temperatureC", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "tempC", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "C", MDV_UNITS_LEN);

  // Create the vertical level header

  Mdvx::vlevel_header_t vLevel_hdr;
  memset(&vLevel_hdr, 0, sizeof(vLevel_hdr));

  for(size_t i = 0; i < pressureLevels.size(); ++i)
    {
      vLevel_hdr.type[i] = Mdvx::VERT_TYPE_PRESSURE;
      vLevel_hdr.level[i] = pressureLevels[i];
    }

  if(_debug)
    {
      Mdvx::printFieldHeader(field_hdr, cerr);
      Mdvx::printVlevelHeader(vLevel_hdr, field_hdr.nz, field_hdr.field_name_long, cerr);
    }

  // create an mdv field object,
  // initializing entire grid with field_hdr.missing_data_value

  return new MdvxField(field_hdr, vLevel_hdr, (void*)NULL, true);
}


/*********************************************************************
 * _createDewpointField() - Create the needed dewpoint field with
 * all of the data filled in with MISSING_DATA_VALUE.
 *
 * Returns the pointer to the created field on success, 0 on failure.
 */

MdvxField* GridHandler::_createDewpointField(const MdvxPjg& projection,
						const vector<double> pressureLevels)
{

  // Create the field header
  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));

  projection.syncToFieldHdr(field_hdr);

  field_hdr.nz = pressureLevels.size();

  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz * 
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  field_hdr.dz_constant = FALSE;
  
  field_hdr.grid_dz = 0;

  field_hdr.grid_minz = 0;

  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;

  field_hdr.bad_data_value = MISSING_DATA_VALUE;
  field_hdr.missing_data_value = MISSING_DATA_VALUE;
  field_hdr.min_value = 0;
  field_hdr.max_value = 0;
  STRcopy(field_hdr.field_name_long, "dewpoint", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "dewpoint", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "K", MDV_UNITS_LEN);

  // Create the vertical level header

  Mdvx::vlevel_header_t vLevel_hdr;
  memset(&vLevel_hdr, 0, sizeof(vLevel_hdr));

  for(size_t i = 0; i < pressureLevels.size(); ++i)
    {
      vLevel_hdr.type[i] = Mdvx::VERT_TYPE_PRESSURE;
      vLevel_hdr.level[i] = pressureLevels[i];
    }

  if(_debug)
    {
      Mdvx::printFieldHeader(field_hdr, cerr);
      Mdvx::printVlevelHeader(vLevel_hdr, field_hdr.nz, field_hdr.field_name_long, cerr);
    }

  // create an mdv field object,
  // initializing entire grid with field_hdr.missing_data_value

  return new MdvxField(field_hdr, vLevel_hdr, (void*)NULL, true);
}


/*********************************************************************
 * _createDewpointCField() - Create the needed Celsius dewpoint field with
 * all of the data filled in with MISSING_DATA_VALUE.
 *
 * Returns the pointer to the created field on success, 0 on failure.
 */

MdvxField* GridHandler::_createDewpointCField(const MdvxPjg& projection,
					      const vector<double> pressureLevels)
{

  // Create the field header
  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));

  projection.syncToFieldHdr(field_hdr);

  field_hdr.nz = pressureLevels.size();

  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz * 
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  field_hdr.dz_constant = FALSE;
  
  field_hdr.grid_dz = 0;

  field_hdr.grid_minz = 0;

  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;

  field_hdr.bad_data_value = MISSING_DATA_VALUE;
  field_hdr.missing_data_value = MISSING_DATA_VALUE;
  field_hdr.min_value = 0;
  field_hdr.max_value = 0;
  STRcopy(field_hdr.field_name_long, "dewpointC", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "dewpointC", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "C", MDV_UNITS_LEN);

  // Create the vertical level header

  Mdvx::vlevel_header_t vLevel_hdr;
  memset(&vLevel_hdr, 0, sizeof(vLevel_hdr));

  for(size_t i = 0; i < pressureLevels.size(); ++i)
    {
      vLevel_hdr.type[i] = Mdvx::VERT_TYPE_PRESSURE;
      vLevel_hdr.level[i] = pressureLevels[i];
    }

  if(_debug)
    {
      Mdvx::printFieldHeader(field_hdr, cerr);
      Mdvx::printVlevelHeader(vLevel_hdr, field_hdr.nz, field_hdr.field_name_long, cerr);
    }

  // create an mdv field object,
  // initializing entire grid with field_hdr.missing_data_value

  return new MdvxField(field_hdr, vLevel_hdr, (void*)NULL, true);
}


/*********************************************************************
 * _createPressureField() - Create the needed pressure field with
 * all of the data filled in with MISSING_DATA_VALUE.
 *
 * Returns the pointer to the created field on success, 0 on failure.
 */

MdvxField* GridHandler::_createPressureField(const MdvxPjg& projection,
						const vector<double> pressureLevels)
{

  // Create the field header
  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));

  projection.syncToFieldHdr(field_hdr);

  field_hdr.nz = pressureLevels.size();

  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz * 
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  field_hdr.dz_constant = FALSE;
  
  field_hdr.grid_dz = 0;

  field_hdr.grid_minz = 0;

  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;

  field_hdr.bad_data_value = MISSING_DATA_VALUE;
  field_hdr.missing_data_value = MISSING_DATA_VALUE;
  field_hdr.min_value = 0;
  field_hdr.max_value = 0;
  STRcopy(field_hdr.field_name_long, "pressure", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "pressure", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "MB", MDV_UNITS_LEN);

  // Create the vertical level header

  Mdvx::vlevel_header_t vLevel_hdr;
  memset(&vLevel_hdr, 0, sizeof(vLevel_hdr));

  for(size_t i = 0; i < pressureLevels.size(); ++i)
    {
      vLevel_hdr.type[i] = Mdvx::VERT_TYPE_PRESSURE;
      vLevel_hdr.level[i] = pressureLevels[i];
    }

  if(_debug)
    {
      Mdvx::printFieldHeader(field_hdr, cerr);
      Mdvx::printVlevelHeader(vLevel_hdr, field_hdr.nz, field_hdr.field_name_long, cerr);
    }

  // create an mdv field object,
  // initializing entire grid with field_hdr.missing_data_value

  return new MdvxField(field_hdr, vLevel_hdr, (void*)NULL, true);
}


/*********************************************************************
 * _createHeightField() - Create the needed height field with
 * all of the data filled in with MISSING_DATA_VALUE.
 *
 * Returns the pointer to the created field on success, 0 on failure.
 */

MdvxField* GridHandler::_createHeightField(const MdvxPjg& projection,
						const vector<double> pressureLevels)
{

  // Create the field header
  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));

  projection.syncToFieldHdr(field_hdr);

  field_hdr.nz = pressureLevels.size();

  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz * 
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  field_hdr.dz_constant = FALSE;
  
  field_hdr.grid_dz = 0;

  field_hdr.grid_minz = 0;

  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;

  field_hdr.bad_data_value = MISSING_DATA_VALUE;
  field_hdr.missing_data_value = MISSING_DATA_VALUE;
  field_hdr.min_value = 0;
  field_hdr.max_value = 0;
  STRcopy(field_hdr.field_name_long, "height", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "height", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "M", MDV_UNITS_LEN);

  // Create the vertical level header

  Mdvx::vlevel_header_t vLevel_hdr;
  memset(&vLevel_hdr, 0, sizeof(vLevel_hdr));

  for(size_t i = 0; i < pressureLevels.size(); ++i)
    {
      vLevel_hdr.type[i] = Mdvx::VERT_TYPE_PRESSURE;
      vLevel_hdr.level[i] = pressureLevels[i];
    }

  if(_debug)
    {
      Mdvx::printFieldHeader(field_hdr, cerr);
      Mdvx::printVlevelHeader(vLevel_hdr, field_hdr.nz, field_hdr.field_name_long, cerr);
    }

  // create an mdv field object,
  // initializing entire grid with field_hdr.missing_data_value

  return new MdvxField(field_hdr, vLevel_hdr, (void*)NULL, true);
}
