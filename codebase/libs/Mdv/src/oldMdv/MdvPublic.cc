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
/*********************************************************************
 * MdvPublic.cc: Mdv object code.  This object manipulates MDV data.
 *               This file contains the public member functions.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/


#include <cstdio>
#include <cerrno>
#include <cassert>

#include <toolsa/os_config.h>
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_macros.h>
#include <Mdv/mdv/mdv_user.h>
#include <toolsa/file_io.h>
#include <toolsa/ldata_info.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>

#include <Mdv/mdv/Mdv.h>
using namespace std;

/*
 * Global variables
 */



/*********************************************************************
 * Constructors
 */

Mdv::Mdv(MdvDebugLevel debug_level)
{
  static const char *routine_name = "Constructor";
  
  if (debug_level >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  // Save the debug level.  Do this first in case any other routines
  // need this.

  _debugLevel = debug_level;
  
  // Indicate that there was no input file

  _inputFilename = (char *)NULL;
  _inputFile = (FILE *)NULL;
  
  // Create the master header

  _masterHdr = _createInitialMasterHdr();

  // Initialize the offset values

  _masterHdr->field_hdr_offset = sizeof(MDV_master_header_t);
  _masterHdr->vlevel_hdr_offset = sizeof(MDV_master_header_t);
  _masterHdr->chunk_hdr_offset = sizeof(MDV_master_header_t);
  
  // Initialize the field list

  _fieldList = new SimpleList<MdvFieldData *> ();
  
  // Initialize the grid.  Use 0's since we don't have any other
  // information right now.

  _grid = new MdvGrid(0.0, 0.0, 0.0,
		      0.0, 0.0, 0.0,
		      0, 0, 0,
		      MDV_PROJ_FLAT,
		      debug_level);
  
}


Mdv::Mdv(const char *filename,
	 MdvDebugLevel debug_level)
{
  static const char *routine_name = "Constructor";
  
  if (debug_level >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  // Save the debug level.  Do this first in case any other routines
  // need this.

  _debugLevel = debug_level;

  // Save the filename

  _inputFilename = STRdup(filename);
  
  // Open the input file

  _inputFile = ta_fopen_uncompress(_inputFilename, "r");
  assert(_inputFile != NULL);
  
  // Read in the master header from the input file

  _masterHdr = (MDV_master_header_t *)umalloc(sizeof(MDV_master_header_t));
  assert(MDV_load_master_header(_inputFile,
				_masterHdr) == MDV_SUCCESS);
  
  // Initialize the field list

  _fieldList = new SimpleList<MdvFieldData *> ();
  
  // Initialize the grid information.  Since the grid dimensions are
  // no longer kept in the master header, set those values to 0.

  _grid = new MdvGrid(0.0, 0.0, 0.0,
		      0.0, 0.0, 0.0,
		      _masterHdr->max_nx,
		      _masterHdr->max_ny,
		      _masterHdr->max_nz,
		      MDV_PROJ_FLAT,
		      debug_level);
  
}


Mdv::Mdv(const Mdv *mdv_obj,
	 const int copy_fields)
{
  static const char *routine_name = "Constructor";
  
  if (mdv_obj->_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  // Save the debug level.  Do this first in case any other routines
  // need this.

  _debugLevel = mdv_obj->_debugLevel;
  
  // Copy the master header

  _masterHdr = (MDV_master_header_t *)umalloc(sizeof(MDV_master_header_t));
  *_masterHdr = *(mdv_obj->_masterHdr);
  
  // Initialize the field list

  _fieldList = new SimpleList<MdvFieldData *> ();
  
  // Copy the fields, if desired

  if (copy_fields)
  {
    for (int i = 0; i < _masterHdr->n_fields; i++)
    {
      // Make sure the field information has been read in

      int field_id = getFieldId(i);
      
      // Now copy the field information

      MdvFieldData *new_field = new MdvFieldData(field_id,
						 (*mdv_obj->_fieldList)[i]);
      
      // Insert the new field in the field list

      _fieldList->add(new_field);
      
    } /* endfor - i */
    
  }
  else
  {
    _masterHdr->n_fields = 0;
    _masterHdr->vlevel_included = FALSE;
    _masterHdr->field_hdr_offset = sizeof(MDV_master_header_t);
    _masterHdr->vlevel_hdr_offset = sizeof(MDV_master_header_t);
    _masterHdr->chunk_hdr_offset = sizeof(MDV_master_header_t);
  }
  
  // Copy the grid information

  _grid = new MdvGrid(mdv_obj->_grid);
  
  // Note that this MDV object doesn't have an associated input file

  _inputFilename = (char *)NULL;
  _inputFile = (FILE *)NULL;
  
}


/*********************************************************************
 * Destructor
 */

Mdv::~Mdv(void)
{
  static const char *routine_name = "Destructor";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  if (_debugLevel >= MDV_DEBUG_MSGS)
    fprintf(stdout,
	    "%s:  Destroying Mdv object\n",
	    _className());

  // Free the master header

  ufree(_masterHdr);
  
  // Close and free the input file stuff

  STRfree(_inputFilename);
  if (_inputFile != NULL)
    fclose(_inputFile);
  
  // Free the field list

  for (int i = 0; i < _fieldList->size(); i++)
    delete (*_fieldList)[i];

  delete _fieldList;

  // Free the grid information

  delete _grid;
  
}


/*********************************************************************
 * initKavourasMosaic() - Initialize the dataset assuming the data is in a
 *                        Kavouras mosaic grid.
 */

void Mdv::initKavourasMosaic(double min_lat,
			     double min_lon,
			     double lat_delta,
			     double lon_delta,
			     long   num_lat,
			     long   num_lon)
{
  static const char *routine_name = "initKavourasMosaic";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  assert(_masterHdr != NULL);
  
  // Set the master header values

  _masterHdr->data_dimension = 3;
  _masterHdr->data_collection_type = MDV_DATA_EXTRAPOLATED;
  _masterHdr->native_vlevel_type = MDV_VERT_TYPE_ELEV;
  _masterHdr->vlevel_type = MDV_VERT_TYPE_Z;
  _masterHdr->vlevel_included = FALSE;
  _masterHdr->grid_order_direction = MDV_ORIENT_SN_WE;
  _masterHdr->grid_order_indices = MDV_ORDER_XYZ;
  _masterHdr->max_nx = num_lon;
  _masterHdr->max_ny = num_lat;
  _masterHdr->max_nz = 1;
  _masterHdr->sensor_lon = min_lon + lon_delta / 2.0;
  _masterHdr->sensor_lat = min_lat + lat_delta / 2.0;
  _masterHdr->sensor_alt = 0.0;
  STRcopy((char *)_masterHdr->data_set_info,
	  "MDV gridded lightning data file",
	  MDV_INFO_LEN);
  STRcopy((char *)_masterHdr->data_set_name,
	  "Gridded Lightning",
	  MDV_NAME_LEN);
  
  // Update the grid information

  _grid->updateOrigin(min_lon + lon_delta / 2.0,
		      min_lat + lat_delta / 2.0,
		      0.0); // Changed from 0.5 to 0.0 by Niles Oct 1999
  _grid->updateDeltas(lon_delta,
		      lat_delta,
		      0.0); // Changed from 1.0 to 0.0 by Niles
  _grid->updateSize(_masterHdr->max_nx,
		    _masterHdr->max_ny,
		    _masterHdr->max_nz);
  
  _grid->updateProjection(MDV_PROJ_LATLON);
  
  return;
}


/*********************************************************************
 * updateTimes - Update the dataset times
 */

void Mdv::updateTimes(time_t gen_time,
		      time_t begin_time,
		      time_t centroid_time,
		      time_t end_time)
{
  static const char *routine_name = "updateTimes";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);

  _masterHdr->time_gen = gen_time;
  _masterHdr->time_begin = begin_time;
  _masterHdr->time_centroid = centroid_time;
  _masterHdr->time_end = end_time;
  
  return;
}    
/*********************************************************************
 * updateInfo - Update the dataset information
 */

void Mdv::updateInfo(char *data_set_info,
		     char *data_set_name,
		     char *data_set_source)
{
  static const char *routine_name = "updateInfo";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  STRcopy((char *)_masterHdr->data_set_info,
	  data_set_info, MDV_INFO_LEN);
  STRcopy((char *)_masterHdr->data_set_name,
	  data_set_name, MDV_NAME_LEN);
  STRcopy((char *)_masterHdr->data_set_source,
	  data_set_source, MDV_NAME_LEN);
  
  return;
}


/*********************************************************************
 * updateDataInfo - Update some general data information
 */

void Mdv::updateDataInfo(int data_dimension,
			 int data_collection_type,
			 int native_vlevel_type,
			 int vlevel_type)
{
  static const char *routine_name = "updateDataInfo";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  _masterHdr->data_dimension = data_dimension;
  _masterHdr->data_collection_type = data_collection_type;
  _masterHdr->native_vlevel_type = native_vlevel_type;
  _masterHdr->vlevel_type = vlevel_type;
  
  return;
}


/*********************************************************************
 * updateSensor - Update the sensor location
 */

void Mdv::updateSensor(double sensor_lat,
		       double sensor_lon,
		       double sensor_alt)
{
  static const char *routine_name = "updateSensor";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  _masterHdr->sensor_lat = sensor_lat;
  _masterHdr->sensor_lon = sensor_lon;
  _masterHdr->sensor_alt = sensor_alt;
  
  return;
}


/*********************************************************************
 * updateGridParams - Update the grid parameter values in the master
 *                    header
 */

void Mdv::updateGridParams(double minx,
			   double miny,
			   double minz,
			   double dx,
			   double dy,
			   double dz,
			   int max_nx,
			   int max_ny,
			   int max_nz,
			   int grid_order_direction,
			   int grid_order_indices)
{
  static const char *routine_name = "updateGridParams";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  _masterHdr->max_nx = max_nx;
  _masterHdr->max_ny = max_ny;
  _masterHdr->max_nz = max_nz;
  
  _masterHdr->grid_order_direction = grid_order_direction;
  _masterHdr->grid_order_indices = grid_order_indices;
  
  // Now update the grid object

  _grid->updateOrigin(minx, miny, minz);
  _grid->updateDeltas(dx, dy, dz);
  _grid->updateSize(max_nx, max_ny, max_nz);
  
  return;
}


/*********************************************************************
 * addField() - Add a field to the MDV dataset.  Returns a unique field
 *              id for referencing the field in other calls.
 *
 * Internally, the returned field id is the position in the _fieldList
 * vector for this field.  Note that if we allow field deletions in the
 * future, we should probably change the field list from a vector to
 * a list and increment an internal next field id variable for assigning
 * ids.
 */

int Mdv::addField(char *field_name_long,
		  char *field_name,
		  char *units,
		  char *transform,
		  int field_code)
{
  static const char *routine_name = "addField";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);

  int field_id = _fieldList->size();
  
  if (_debugLevel >= MDV_DEBUG_MSGS)
    fprintf(stderr,
	    "%s::%s Adding field number %d\n",
	    _className(), routine_name, field_id);
  
  // Create the field

  MdvFieldData *new_field = new MdvFieldData(field_id, 
					     field_name_long,
					     field_name,
					     units,
					     transform,
					     field_code,
					     _debugLevel);
  
  new_field->updateGridParams(_grid->getMinX(),
			      _grid->getMinY(),
			      _grid->getMinZ(),
			      _grid->getDeltaX(),
			      _grid->getDeltaY(),
			      _grid->getDeltaZ(),
			      _masterHdr->max_nx,
			      _masterHdr->max_ny,
			      _masterHdr->max_nz);
  
  new_field->updateProjectionParams(_grid->getProjection(),
				    _masterHdr->sensor_lat,
				    _masterHdr->sensor_lon,
				    0.0);
  
  // Add the field to the field list

  _fieldList->add(new_field);

  // Update the number of fields in the master header

  _masterHdr->n_fields++;
  
  // Update the offsets in the master header

  _masterHdr->vlevel_hdr_offset += sizeof(MDV_field_header_t);
  _masterHdr->chunk_hdr_offset += sizeof(MDV_field_header_t);
  
  return(field_id);
}  


int Mdv::addField(MdvFieldData *field_obj)
{
  static const char *routine_name = "addField";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);

  int field_id = _fieldList->size();
  
  if (_debugLevel >= MDV_DEBUG_MSGS)
    fprintf(stderr,
	    "%s::%s Adding field number %d\n",
	    _className(), routine_name, field_id);
  
  // Create the field

  MdvFieldData *new_field = new MdvFieldData(field_id, 
					     field_obj);
  
  // Add the field to the field list

  _fieldList->add(new_field);

  // Update the number of fields in the master header

  _masterHdr->n_fields++;
  
  // Update the offsets in the master header

  _masterHdr->vlevel_hdr_offset += sizeof(MDV_field_header_t);
  _masterHdr->chunk_hdr_offset += sizeof(MDV_field_header_t);
  
  return(field_id);
}  


/*********************************************************************
 * getFieldId() - Get the field id for the given field.  This is used
 *                to identify a field in an MDV object that was
 *                initialized by an input file.  Note that the first
 *                field in the MDV file has field position 0.
 *
 * Returns -1 if there is no field for the given position.
 */

int Mdv::getFieldId(const int field_position)
{
  static const char *routine_name = "getFieldId";
  
  MdvFieldData *new_field;
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // If the field already exists, just return the id (which is the
  // same as the field position right now).

  if (field_position < (int)_fieldList->size())
    return(field_position);
  
  // Now make sure there is really a field in the given position

  if (_inputFilename == NULL ||
      _masterHdr->n_fields < field_position)
    return(-1);
  
  // Load all of the fields up to the one desired

  for (int i = _fieldList->size(); i <= field_position; i++)
  {
    // Create the new field, reading the field information from
    // the input file

    new_field = new MdvFieldData(i,
				 _inputFile,
				 _masterHdr->vlevel_included,
				 _masterHdr->vlevel_hdr_offset,
				 _debugLevel);
  
    // Add it to the field list

    _fieldList->add(new_field);
    
  } /* endfor - i */
  
  // Right now the field id is the same as the field position

  return(field_position);
}


/*********************************************************************
 * updateFieldName() - Update the field name and associated information
 *                     for a given field.
 */

void Mdv::updateFieldName(int field_id,
			  char *field_name_long,
			  char *field_name,
			  char *units,
			  char *transform,
			  int field_code)
{
  static const char *routine_name = "updateFieldName";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  (*_fieldList)[field_id]->updateName(field_name_long,
				      field_name,
				      units,
				      transform,
				      field_code);
  
  return;
}


/*********************************************************************
 * updateFieldGridParams() - Update the grid parameters for a given
 *                           field.
 */

void Mdv::updateFieldGridParams(int field_id,
				double minx,
				double miny,
				double minz,
				double dx,
				double dy,
				double dz,
				int nx,
				int ny,
				int nz)
{
  static const char *routine_name = "updateFieldGridParams";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  (*_fieldList)[field_id]->updateGridParams(minx, miny, minz,
					    dx, dy, dz,
					    nx, ny, nz);
  
  return;
}


/*********************************************************************
 * updateFieldProjectionParams() - Update the projection parameters
 *                                 for a given field.
 */

void Mdv::updateFieldProjectionParams(int field_id,
				      int type,
				      double origin_lat,
				      double origin_lon,
				      double rotation,
				      double param0,
				      double param1,
				      double param2,
				      double param3,
				      double param4,
				      double param5,
				      double param6,
				      double param7)
{
  static const char *routine_name = "updateFieldProjectionParams";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  (*_fieldList)[field_id]->updateProjectionParams(type,
						  origin_lat,
						  origin_lon,
						  rotation,
						  param0,
						  param1,
						  param2,
						  param3,
						  param4,
						  param5,
						  param6,
						  param7);
  
  return;
}


/*********************************************************************
 * updateFieldDataParams() - Update the data parameters for a given
 *                           field.
 */

void Mdv::updateFieldDataParams(int field_id,
				int encoding_type,
				int data_element_nbytes,
				double scale,
				double bias,
				double bad_data_value,
				double missing_data_value)
{
  static const char *routine_name = "updateFieldDataParams";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  (*_fieldList)[field_id]->updateDataParams(encoding_type,
					    data_element_nbytes,
					    scale, bias,
					    bad_data_value,
					    missing_data_value);
  
  return;
}


/*********************************************************************
 * updateFieldData() - Update the data for a given field.  Also update
 *                     the dataset times, if indicated.
 */

void Mdv::updateFieldData(int field_id,
			  time_t forecast_time,
			  void *data,
			  int data_size,
			  int encoding_type)
{
  static const char *routine_name = "updateFieldData";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Update the field

  (*_fieldList)[field_id]->updateData(forecast_time,
				      forecast_time - _masterHdr->time_centroid,
				      data,
				      data_size,
				      encoding_type);
  
  return;
}


/*********************************************************************
 * getFieldNx() - Retrieve the size of the grid in the X direction for
 *                the given field.
 */

int Mdv::getFieldNx(int field_id)
{
  static const char *routine_name = "getFieldNx";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Return the value to the caller

  return((*_fieldList)[field_id]->getNx());
}


/*********************************************************************
 * getFieldNy() - Retrieve the size of the grid in the Y direction for
 *                the given field.
 */

int Mdv::getFieldNy(int field_id)
{
  static const char *routine_name = "getFieldNy";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Return the value to the caller

  return((*_fieldList)[field_id]->getNy());
}


/*********************************************************************
 * getFieldNz() - Retrieve the size of the grid in the Z direction for
 *                the given field.
 */

int Mdv::getFieldNz(int field_id)
{
  static const char *routine_name = "getFieldNz";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Return the value to the caller

  return((*_fieldList)[field_id]->getNz());
}


/*********************************************************************
 * getFieldForecastTime() - Retrieve the data forecast time for the
 *                          given field.
 */

time_t Mdv::getFieldForecastTime(int field_id)
{
  static const char *routine_name = "getFieldForecastTime";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Return the value to the caller

  return((*_fieldList)[field_id]->getForecastTime());
}


/*********************************************************************
 * getFieldScale() - Retrieve the scale value for the given field.
 */

double Mdv::getFieldScale(int field_id)
{
  static const char *routine_name = "getFieldScale";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Return the value to the caller

  return((*_fieldList)[field_id]->getScale());
}


/*********************************************************************
 * getFieldBias() - Retrieve the bias value for the given field.
 */

double Mdv::getFieldBias(int field_id)
{
  static const char *routine_name = "getFieldBias";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Return the value to the caller

  return((*_fieldList)[field_id]->getBias());
}


/*********************************************************************
 * getFieldMissingValue() - Retrieve the missing data value for the
 *                          given field.
 */

double Mdv::getFieldMissingValue(int field_id)
{
  static const char *routine_name = "getFieldMissingValue";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Return the value to the caller

  return((*_fieldList)[field_id]->getMissingValue());
}


/*********************************************************************
 * getFieldBadValue() - Retrieve the bad data value for the given field.
 */

double Mdv::getFieldBadValue(int field_id)
{
  static const char *routine_name = "getFieldBadValue";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Return the value to the caller

  return((*_fieldList)[field_id]->getBadValue());
}


/*********************************************************************
 * getFieldDataVolume() - Retrieve the field data for the given field
 *                        in the desired format.
 */

void *Mdv::getFieldDataVolume(int field_id,
			      int *volume_size,
			      int mdv_encoding_type)
{
  static const char *routine_name = "getFieldDataVolume";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Return the field data to the caller

  return((*_fieldList)[field_id]->getDataVolume(volume_size,
						mdv_encoding_type));
}


/*********************************************************************
 * getFieldGrid() - Retrieve the grid object for the given field.
 */

MdvGrid Mdv::getFieldGrid(int field_id)
{
  static const char *routine_name = "getFieldGrid";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Return the field grid to the caller

  return((*_fieldList)[field_id]->getGrid());
}


/*********************************************************************
 * dump() - Dump the mdv information in the binary format to the
 *          indicated stream.
 */

void Mdv::dump(char *filename,
	       int output_encoding_type,
	       char *curr_file_index_dir)
{
  static const char *routine_name = "dump";
  
  FILE *output_file;
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  if (_debugLevel >= MDV_DEBUG_MSGS)
    fprintf(stdout, "%s::%s: Writing data to file <%s>\n",
	    _className(), routine_name, filename);

  // compute tmp file path

  char tmp_path[MAX_PATH_LEN];

  ta_tmp_path_from_final(filename, tmp_path,
			 MAX_PATH_LEN, "TMP_MDV");

  // Open the tmp output file

  if ((output_file = ta_fopen_uncompress(tmp_path, "w")) == NULL)
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n",
	    _className(), routine_name);
    fprintf(stderr,
	    "Error opening tmp output file\n");
    perror(tmp_path);
    return;
  }
  
  // Dump the information

  _dumpBinary(output_file, output_encoding_type);
  
  // Close the output file

  fclose(output_file);

  // rename tmp path to final file path

  if (rename(tmp_path, filename)) {
    fprintf(stderr,
	    "ERROR: %s::%s\n",
	    _className(), routine_name);
    fprintf(stderr,
	    "Error renaming tmp output file %s\n", tmp_path);
    perror(filename);
    return;
  }
  
  // Create the current index file

  if (curr_file_index_dir != NULL &&
      curr_file_index_dir[0] != '\0')
  {
    // Extract the output directory and extension from the filename
    
    char output_ext[MAX_PATH_LEN];
    
    char *dot_pos;
    
    if ((dot_pos = strrchr(filename, '.')) == NULL)
      output_ext[0] = '\0';
    else
      STRcopy(output_ext, dot_pos+1, MAX_PATH_LEN);
    
    // Write the current index

    _writeCurrentIndex(curr_file_index_dir,
		       output_ext,
		       _masterHdr->time_centroid);
  } /* endif - curr_file_index_dir != NULL */
    
  return;
}


void Mdv::dump(FILE *output_file,
	       int output_encoding_type)
{
  static const char *routine_name = "dump";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Dump the information

  _dumpBinary(output_file, output_encoding_type);
  
  return;
}


void Mdv::dump(char *output_dir,
	       char *output_filename,
	       int output_encoding_type,
	       char *curr_file_index_dir)
{
  static const char *routine_name = "dump";
  
  char full_output_filename[MAX_PATH_LEN];
    
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Make sure the output directory exists

  if (makedir(output_dir) != 0)
  {
    if (_debugLevel >= MDV_DEBUG_ERRORS)
    {
      fprintf(stderr,
	      "ERROR: %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr,
	      "Error creating output directory <%s>\n",
	      output_dir);
    }
	
    return;
  }
  
  // Construct the full output filename

  sprintf(full_output_filename,
	  "%s/%s",
	  output_dir, output_filename);
    
  // Dump the information

  dump(full_output_filename,
       output_encoding_type,
       curr_file_index_dir);
    
  return;
}


void Mdv::dump(char *output_host,
	       char *output_dir,
	       char *output_filename,
	       char *local_tmp_dir,
	       int output_encoding_type,
	       char *curr_file_index_dir)
{
  static const char *routine_name = "dump";
  
  char tmp_filename[MAX_PATH_LEN];
  char remote_tmp_filename[MAX_PATH_LEN];
  char remote_filename[MAX_PATH_LEN];
  
  char call_str[BUFSIZ];
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // For the local host, just call the appropriate dump
  // routine

  if (STRequal_exact(output_host, "local") ||
      STRequal_exact(output_host, "localhost"))
  {
    dump(output_dir,
	 output_filename,
	 output_encoding_type,
	 curr_file_index_dir);
    
    return;
  } /* endif - dump on local host */
  
  // Determine all of the filenames and paths

  time_t now = time((time_t *)NULL);
  sprintf(tmp_filename, "%s/mdv_temp.%ld",
	  local_tmp_dir, now);
  sprintf(remote_tmp_filename, "%s/mdv_temp.%ld",
	  output_dir, now);
  
  sprintf(remote_filename, "%s/%s",
	  output_dir, output_filename);
  
  // Create the remote directories, if necessary

  sprintf(call_str, "rsh -n %s mkdir -p %s",
	  output_host, output_dir);
  
  usystem_call(call_str);
  
  // Dump the information to the temporary file on the
  // local machine

  dump(tmp_filename, output_encoding_type, (char *)NULL);
  
  // Copy the temp file into the temp area on the remote host

  sprintf(call_str, "rcp %s %s:%s",
	  tmp_filename, output_host, remote_tmp_filename);
  
  usystem_call(call_str);
  
  // Move the file to the correct location on the remote host

  sprintf(call_str, "rsh %s mv %s %s",
	  output_host, remote_tmp_filename, remote_filename);
  
  usystem_call(call_str);
  
  // Delete the temp file locally

  if (unlink(tmp_filename) != 0)
  {
    fprintf(stderr,
	    "ERROR: %s:%s\n",
	    _className(), routine_name);
    fprintf(stderr,
	    "Error deleting temporary file\n");
    perror(tmp_filename);
  }
  
  // Create the current file index

  if (curr_file_index_dir != NULL &&
      curr_file_index_dir[0] != '\0')
  {
    // Get the output extension

    char output_ext[MAX_PATH_LEN];
    char *dot_pos;
    
    if ((dot_pos = strrchr(output_filename, '.')) == NULL)
      output_ext[0] = '\0';
    else
      STRcopy(output_ext, dot_pos+1, MAX_PATH_LEN);
    
    // Write the index to the local temp directory
    
    char *index_path;
    
    if ((index_path = _writeCurrentIndex(local_tmp_dir,
					 output_ext,
					 _masterHdr->time_centroid)) != NULL)
    {
      // Copy the index file to the remote host

      sprintf(call_str, "rcp %s %s:%s",
	      index_path, output_host, curr_file_index_dir);
      usystem_call(call_str);

      // Delete the local copy of the file

      if (unlink(index_path) != 0)
      {
	fprintf(stderr,
		"ERROR: %s:%s\n",
		_className(), routine_name);
	fprintf(stderr,
		"Error deleting local index file\n");
	perror(index_path);
      }

    }
    
  } /* endif - write_current_index */
  
  return;
}


void Mdv::dump(char *output_host,
	       char *output_dir,
	       time_t data_time,
	       char *output_extension,
	       char *local_tmp_dir,
	       int output_encoding_type,
	       char *curr_file_index_dir)
{
  static const char *routine_name = "dump";
  
  char data_output_dir[MAX_PATH_LEN];
  char output_filename[MAX_PATH_LEN];
  
  UTIMstruct time_struct;
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Extract the parts of the time

  UTIMunix_to_date(data_time, &time_struct);
  
  // Construct the output directory path

  sprintf(data_output_dir, "%s/%04ld%02ld%02ld",
	  output_dir,
	  time_struct.year,
	  time_struct.month,
	  time_struct.day);
  
  // Construct the output filename

  sprintf(output_filename, "%02ld%02ld%02ld.%s",
	  time_struct.hour,
	  time_struct.min,
	  time_struct.sec,
	  output_extension);
  
  dump(output_host,
       data_output_dir,
       output_filename,
       local_tmp_dir,
       output_encoding_type,
       curr_file_index_dir);
  
  return;
}


/*********************************************************************
 * print() - Print the mdv information to the indicated stream.
 */

void Mdv::print(FILE *stream, int full_flag)
{
  static const char *routine_name = "print";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Print the information

  _dumpAscii(stream, full_flag);
  
  return;
}


void Mdv::print(char *filename, int full_flag)
{
  static const char *routine_name = "print";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  FILE *output_file;
  
  // Open the output file.
  
  if ((output_file = ta_fopen_uncompress(filename, "w")) == (FILE *)NULL)
  {
    fprintf(stderr,
	    "%s::%s: ERROR:  Error opening file <%s> for output\n",
	    _className(), routine_name, filename);
    return;
  }
  
  // Print the information

  _dumpAscii(output_file, full_flag);
  
  // Close the output file

  fclose(output_file);
  
  return;
}

/*********************************************************************
 * _createInitialMasterHdr() - Creates an initial master header with
 *                             default values for the dataset.
 */

MDV_master_header_t *Mdv::_createInitialMasterHdr(void)
{
  MDV_master_header_t *master_hdr;
  
  master_hdr = (MDV_master_header_t *)umalloc(sizeof(MDV_master_header_t));
  
  MDV_init_master_header(master_hdr);
  
  return(master_hdr);
}

/*********************************************************************
 * _readFile() - Read the input file into the internal structure.
 */

void Mdv::_readFile(void)
{
  static const char *routine_name = "_readFile";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  fprintf(stderr,
	  "ERROR: %s::%s called but not written\n",
	  _className(), routine_name);
  
  return;
}

/*********************************************************************
 * _dumpBinary() - Dump the information to the indicated stream in
 *                 binary format.
 */

void Mdv::_dumpBinary(FILE *out_file,
		      int output_encoding_type)
{
  static const char *routine_name = "_dumpBinary";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Write the master header

  if (MDV_write_master_header(out_file,
			      _masterHdr) != MDV_SUCCESS)
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n",
	    _className(), routine_name);
    fprintf(stderr,
	    "Error writing master header to output file\n");
    return;
  }
  
  // Write each of the fields

  int next_offset = MDV_get_first_field_offset(_masterHdr);
  int field_num;
  
  for (field_num = 0; field_num < (int)_fieldList->size(); field_num++)
  {
    int bytes_written = (*_fieldList)[field_num]->dump(out_file,
						       field_num,
						       next_offset,
						       output_encoding_type);
    
    next_offset += bytes_written + (2 * sizeof(si32));
  } /* endfor - field_num */
  
  return;
}


/*********************************************************************
 * _dumpAscii() - Dump the information to the indicated stream in
 *                ASCII format.
 */

void Mdv::_dumpAscii(FILE *stream,
		     int full_flag)
{
  static const char *routine_name = "_dumpAscii";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  // Print the master header

  if (full_flag)
    MDV_print_master_header_full(_masterHdr, stream);
  else
    MDV_print_master_header(_masterHdr, stream);
  
  // Print out each of the fields

  for (int i = 0; i < _fieldList->size(); i++)
    (*_fieldList)[i]->print(stream, full_flag);

  return;
}


/*********************************************************************
 * _writeCurrentIndex() - Write the current index file for the data
 *                        file that was just written.
 *
 * Returns the full path for the written index file.
 */

char *Mdv::_writeCurrentIndex(char *output_dir,
			      char *output_ext,
			      time_t data_time)
{
  static const char *routine_name = "_writeCurrentIndex";
  static int first_call = TRUE;
  static LDATA_handle_t ldata;
  char calling_sequence[BUFSIZ];

  if (first_call) {
    sprintf(calling_sequence, "%s::%s",
	    _className(), routine_name);
    LDATA_init_handle(&ldata, calling_sequence, 0);
    first_call = FALSE;
  }

  // Write the info file

  if (LDATA_info_write(&ldata, output_dir, data_time, output_ext,
		       (char *)NULL, (char *)NULL, 0, (int *)NULL)) {
    return (NULL);
  } else {
    return (ldata.file_path);
  }

}
