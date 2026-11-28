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
//   $Id: GridHandler.cc,v 1.3 2016/03/07 01:23:08 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * GridHandler: GridHandler program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2003
 *
 * Nancy Rehak/Kay Levesque
 *
 *********************************************************************/

#include <math.h>

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

  if ((_totalPrecipField = _createTotalPrecipField(projection, pressureLevels)) == 0)
    return false;

  if ((_lowerPrecipField = _createLowerPrecipField(projection, pressureLevels)) == 0)
    return false;

  if ((_middlePrecipField = _createMiddlePrecipField(projection, pressureLevels)) == 0)
    return false;

  if ((_upperPrecipField = _createUpperPrecipField(projection, pressureLevels)) == 0)
    return false;

  if ((_pressureField = _createPressureField(projection, pressureLevels)) == 0)
    return false;

  if ((_guessTotalPrecipField = _createGuessTotalPrecipField(projection, pressureLevels)) == 0)
    return false;

  if ((_liField = _createLiField(projection, pressureLevels)) == 0)
    return false;

  if ((_capeField = _createCapeField(projection, pressureLevels)) == 0)
    return false;

  if ((_guessLiField = _createGuessLiField(projection, pressureLevels)) == 0)
    return false;

  if ((_skinTempField = _createSkinTempField(projection, pressureLevels)) == 0)
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

  int z_index = 0;

  // fabs is floating absolute value since we don't need to worry about positive or negative

  double minPressureDiff = fabs(sounder_data.pressure - _pressureLevels[0]);
  
  for (size_t i = 1; i < _pressureLevels.size(); ++i)
    {
      double tmpMinPressureDiff = fabs(sounder_data.pressure - _pressureLevels[i]);

      if (tmpMinPressureDiff < minPressureDiff)
	{
	  z_index = i;
	  minPressureDiff = tmpMinPressureDiff;
	}
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

  // Update the total precipitation grid value.

  fl32 *data = (fl32*)_totalPrecipField->getVol();

  if (data[array_index] != MISSING_DATA_VALUE)
    {
      if (_debug)
	cerr << "Warning! 2 soundings in same location." << endl;
    }

  data[array_index] = sounder_data.total_precip;

  // Update the lower precipitation grid value.

  data = (fl32*)_lowerPrecipField->getVol();
  data[array_index] = sounder_data.lower_precip;

  // Update the middle precipitation grid value.

  data = (fl32*)_middlePrecipField->getVol();
  data[array_index] = sounder_data.middle_precip;

  // Update the upper precipitation grid value.

  data = (fl32*)_upperPrecipField->getVol();
  data[array_index] = sounder_data.upper_precip;

  // Update the pressure grid value.

  data = (fl32*)_pressureField->getVol();
  data[array_index] = sounder_data.pressure;

  // Update the guess total precipitation grid value.

  data = (fl32*)_guessTotalPrecipField->getVol();
  data[array_index] = sounder_data.guess_total_precip;

  // Update the lifted index grid value.

  data = (fl32*)_liField->getVol();
  data[array_index] = sounder_data.lifted_index;

  // Update the CAPE grid value.

  data = (fl32*)_capeField->getVol();
  data[array_index] = sounder_data.cape;

  // Update the guess lifted index grid value.

  data = (fl32*)_guessLiField->getVol();
  data[array_index] = sounder_data.guess_lifted_index;

  // Update the skin temperature grid value.

  data = (fl32*)_skinTempField->getVol();
  data[array_index] = sounder_data.skin_temp;

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
	  "Goes3x3 Sounding Data generated by ihop3x3GoesTpwc2Mdv", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "Goes3x3 Sounding Data", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, dataSource.c_str(), MDV_NAME_LEN);
  
  mdvFile.setMasterHeader(master_hdr);

  if(_totalPrecipField->convertType(Mdvx::ENCODING_INT8,
				    Mdvx::COMPRESSION_BZIP,
				    Mdvx::SCALING_DYNAMIC) != 0)
    {
      cerr << "Unable to convert type for total precipitation field" << endl;
      return false;
    }

  if(_lowerPrecipField->convertType(Mdvx::ENCODING_INT8,
				     Mdvx::COMPRESSION_BZIP,
				     Mdvx::SCALING_DYNAMIC) != 0)
    {
      cerr << "Unable to convert type for lower precipitation field" << endl;
      return false;
    }

  if(_middlePrecipField->convertType(Mdvx::ENCODING_INT8,
				    Mdvx::COMPRESSION_BZIP,
				    Mdvx::SCALING_DYNAMIC) != 0)
    {
      cerr << "Unable to convert type for middle precipitation field" << endl;
      return false;
    }

  if(_upperPrecipField->convertType(Mdvx::ENCODING_INT8,
				  Mdvx::COMPRESSION_BZIP,
				  Mdvx::SCALING_DYNAMIC) != 0)
    {
      cerr << "Unable to convert type for upper precipitation field" << endl;
      return false;
    }

  if(_pressureField->convertType(Mdvx::ENCODING_INT8,
				    Mdvx::COMPRESSION_BZIP,
				    Mdvx::SCALING_DYNAMIC) != 0)
    {
      cerr << "Unable to convert type for pressure field" << endl;
      return false;
    }

  if(_guessTotalPrecipField->convertType(Mdvx::ENCODING_INT8,
				    Mdvx::COMPRESSION_BZIP,
				    Mdvx::SCALING_DYNAMIC) != 0)
    {
      cerr << "Unable to convert type for guess total precipitation field" << endl;
      return false;
    }

  if(_liField->convertType(Mdvx::ENCODING_INT8,
				    Mdvx::COMPRESSION_BZIP,
				    Mdvx::SCALING_DYNAMIC) != 0)
    {
      cerr << "Unable to convert type for lifted index field" << endl;
      return false;
    }

  if(_capeField->convertType(Mdvx::ENCODING_INT8,
				    Mdvx::COMPRESSION_BZIP,
				    Mdvx::SCALING_DYNAMIC) != 0)
    {
      cerr << "Unable to convert type for CAPE field" << endl;
      return false;
    }

  if(_guessLiField->convertType(Mdvx::ENCODING_INT8,
				    Mdvx::COMPRESSION_BZIP,
				    Mdvx::SCALING_DYNAMIC) != 0)
    {
      cerr << "Unable to convert type for guess lifted index field" << endl;
      return false;
    }

  if(_skinTempField->convertType(Mdvx::ENCODING_INT8,
				    Mdvx::COMPRESSION_BZIP,
				    Mdvx::SCALING_DYNAMIC) != 0)
    {
      cerr << "Unable to convert type for skin temperature field" << endl;
      return false;
    }

  mdvFile.addField(_totalPrecipField);
  mdvFile.addField(_lowerPrecipField);
  mdvFile.addField(_middlePrecipField);
  mdvFile.addField(_upperPrecipField);
  mdvFile.addField(_pressureField);
  mdvFile.addField(_guessTotalPrecipField);
  mdvFile.addField(_liField);
  mdvFile.addField(_capeField);
  mdvFile.addField(_guessLiField);
  mdvFile.addField(_skinTempField);

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
 * _createTotalPrecipField() - Create the needed total precip field with
 * all of the data filled in with MISSING_DATA_VALUE.
 *
 * Returns the pointer to the created field on success, 0 on failure.
 */

MdvxField* GridHandler::_createTotalPrecipField(const MdvxPjg& projection,
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
  STRcopy(field_hdr.field_name_long, "total_precipitation", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "precip", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "mm", MDV_UNITS_LEN);

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
 * _createLowerPrecipField() - Create the needed lower layer precip field with
 * all of the data filled in with MISSING_DATA_VALUE. (surface to 0.9 sigma)
 *
 * Returns the pointer to the created field on success, 0 on failure.
 */

MdvxField* GridHandler::_createLowerPrecipField(const MdvxPjg& projection,
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
  STRcopy(field_hdr.field_name_long, "lower_precipitation", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "low_precip", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "mm", MDV_UNITS_LEN);

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
 * _createMiddlePrecipField() - Create the needed middle layer precip field with
 * all of the data filled in with MISSING_DATA_VALUE. (0.9 to 0.7 sigma)
 *
 * Returns the pointer to the created field on success, 0 on failure.
 */

MdvxField* GridHandler::_createMiddlePrecipField(const MdvxPjg& projection,
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
  STRcopy(field_hdr.field_name_long, "middle_precipitation", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "mid_precip", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "mm", MDV_UNITS_LEN);

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
 * _createUpperPrecipField() - Create the needed upper layer precip field with
 * all of the data filled in with MISSING_DATA_VALUE. (0.7 to 0.3 sigma)
 *
 * Returns the pointer to the created field on success, 0 on failure.
 */

MdvxField* GridHandler::_createUpperPrecipField(const MdvxPjg& projection,
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
  STRcopy(field_hdr.field_name_long, "upper_precipitation", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "upper_precip", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "mm", MDV_UNITS_LEN);

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
 * _createGuessTotalPrecipField() - Create the needed guess total precip field with
 * all of the data filled in with MISSING_DATA_VALUE.
 *
 * Returns the pointer to the created field on success, 0 on failure.
 */

MdvxField* GridHandler::_createGuessTotalPrecipField(const MdvxPjg& projection,
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
  STRcopy(field_hdr.field_name_long, "guess_total_precip", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "guess_precip", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "mm", MDV_UNITS_LEN);

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
 * _createLiField() - Create the needed lifted index field with
 * all of the data filled in with MISSING_DATA_VALUE.
 *
 * Returns the pointer to the created field on success, 0 on failure.
 */

MdvxField* GridHandler::_createLiField(const MdvxPjg& projection,
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
  STRcopy(field_hdr.field_name_long, "lifted_index", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "LI", MDV_SHORT_FIELD_LEN);
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
 * _createCapeField() - Create the needed CAPE field with
 * all of the data filled in with MISSING_DATA_VALUE.
 *
 * Returns the pointer to the created field on success, 0 on failure.
 */

MdvxField* GridHandler::_createCapeField(const MdvxPjg& projection,
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
  STRcopy(field_hdr.field_name_long, "CAPE", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "CAPE", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "J/Kg", MDV_UNITS_LEN);

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
 * _createGuessLiField() - Create the needed guess lifted index field with
 * all of the data filled in with MISSING_DATA_VALUE.
 *
 * Returns the pointer to the created field on success, 0 on failure.
 */

MdvxField* GridHandler::_createGuessLiField(const MdvxPjg& projection,
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
  STRcopy(field_hdr.field_name_long, "guess_lifted_index", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "guess_li", MDV_SHORT_FIELD_LEN);
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
 * _createSkinTempField() - Create the needed skin temperature field with
 * all of the data filled in with MISSING_DATA_VALUE.
 *
 * Returns the pointer to the created field on success, 0 on failure.
 */

MdvxField* GridHandler::_createSkinTempField(const MdvxPjg& projection,
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
  STRcopy(field_hdr.field_name_long, "skin_temperature", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "skin_temp", MDV_SHORT_FIELD_LEN);
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
