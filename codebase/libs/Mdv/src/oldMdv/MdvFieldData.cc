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
 * MdvFieldData.cc: MdvFieldData object code.  This object manipulates
 *                  a single field of data in MDV format.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/


#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cassert>
#include <memory.h>

#include <Mdv/mdv/mdv_convert.h>
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_macros.h>
#include <Mdv/mdv/mdv_print.h>
#include <Mdv/mdv/mdv_user.h>
#include <Mdv/mdv/mdv_utils.h>
#include <Mdv/mdv/mdv_write.h>
#include <toolsa/file_io.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>

#include <Mdv/mdv/Mdv.h>
using namespace std;

/*
 * Global variables
 */



/*********************************************************************
 * Constructors
 */

MdvFieldData::MdvFieldData(void)
{
  // This constructor should never be called.  It was only added so we
  // could use the templates in <vector.h>.

  fprintf(stderr,
	  "Default MdvFieldData constructor called -- this should never be called.\n");
  exit(-1);
  
}


MdvFieldData::MdvFieldData(const int field_number,
			   char *field_name_long,
			   char *field_name,
			   char *units,
			   char *transform,
			   int field_code,
			   MdvDebugLevel debug_level)
{
  static const char *routine_name = "Constructor";
  
  if (debug_level >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  if (debug_level >= MDV_DEBUG_MSGS)
    fprintf(stdout,
	    "%s::%s:  Creating MdvFieldData object for field number %d\n",
	    _className(), routine_name, field_number);
  
  // Save the debug level.  Do this first in case any other routines
  // need this.

  _debugLevel = debug_level;
  
  // Save the field number

  _fieldNum = field_number;
  
  // Initialize the header information

  _fieldHdr = (MDV_field_header_t *)umalloc(sizeof(MDV_field_header_t));
  MDV_init_field_header(_fieldHdr);
  
  _fieldHdr->scale = 1.0;
  _fieldHdr->bad_data_value = -1.0;
  
  // Update the field information in the header

  if (field_name_long == NULL)
    _fieldHdr->field_name_long[0] = '\0';
  else
    STRcopy((char *)_fieldHdr->field_name_long,
	    field_name_long,
	    MDV_LONG_FIELD_LEN);

  if (field_name == NULL)
    _fieldHdr->field_name[0] = '\0';
  else
    STRcopy((char *)_fieldHdr->field_name,
	    field_name,
	    MDV_SHORT_FIELD_LEN);

  if (units == NULL)
    _fieldHdr->units[0] = '\0';
  else
    STRcopy((char *)_fieldHdr->units,
	    units,
	    MDV_UNITS_LEN);

  if (transform == NULL)
    _fieldHdr->transform[0] = '\0';
  else
    STRcopy((char *)_fieldHdr->transform,
	    transform,
	    MDV_TRANSFORM_LEN);

  _fieldHdr->field_code = field_code;
  
  // Initially, we don't have any vlevel information

  _vlevelHdr = (MDV_vlevel_header_t *)NULL;
  
  // Initially, we don't have any data

  _data = NULL;
  _dataSize = 0;
  _dataAlloc = 0;
  
  // Save the grid information (set to all 0's since the user
  // hasn't told us anything else

  _grid = new MdvGrid(0.0, 0.0, 0.0,
		      0.0, 0.0, 0.0,
		      0, 0, 0,
		      MDV_PROJ_FLAT,
		      debug_level);
  
  // Note that we didn't use an input file

  _inputFile = (FILE *)NULL;
}


MdvFieldData::MdvFieldData(const int field_number,
			   FILE *mdv_file,
			   const int load_vlevel_hdr,
			   const int vlevel_hdr_offset,
			   MdvDebugLevel debug_level)
{
  static const char *routine_name = "Constructor";
  
  if (debug_level >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  if (debug_level >= MDV_DEBUG_MSGS)
    fprintf(stdout,
	    "%s::%s:  Creating MdvFieldData object for field number %d\n",
	    _className(), routine_name, field_number);
  
  // Save the debug level.  Do this first in case any other routines
  // need this.

  _debugLevel = debug_level;
  
  // Save the field number

  _fieldNum = field_number;
  
  // Save the input file pointer

  _inputFile = mdv_file;
  
  // Read in the field information from the file

  _fieldHdr = (MDV_field_header_t *)umalloc(sizeof(MDV_field_header_t));
  assert(MDV_load_field_header(mdv_file,
			       _fieldHdr,
			       field_number) == MDV_SUCCESS);
  
  // Read in the vlevel header, if appropriate

  if (load_vlevel_hdr)
  {
    _vlevelHdr = (MDV_vlevel_header_t *)umalloc(sizeof(MDV_vlevel_header_t));
    assert(MDV_load_vlevel_header_offset(mdv_file,
					 _vlevelHdr,
					 vlevel_hdr_offset,
					 field_number) == MDV_SUCCESS);
  }
  else
    _vlevelHdr = (MDV_vlevel_header_t *)NULL;
  
  // Initially, we don't have any data

  _data = NULL;
  _dataSize = 0;
  _dataAlloc = 0;
  
  // Save the grid information

  _grid = new MdvGrid(_fieldHdr->grid_minx,
		      _fieldHdr->grid_miny,
		      _fieldHdr->grid_minz,
		      _fieldHdr->grid_dx,
		      _fieldHdr->grid_dy,
		      _fieldHdr->grid_dz,
		      _fieldHdr->nx,
		      _fieldHdr->ny,
		      _fieldHdr->nz,
		      _fieldHdr->proj_type,
		      debug_level);
  
}


MdvFieldData::MdvFieldData(const int field_number,
			   const MdvFieldData *field_obj)
{
  static const char *routine_name = "Constructor";
  
  if (field_obj->_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  // Save the debug level.  Do this first in case any other routines
  // need this.

  _debugLevel = field_obj->_debugLevel;
  
  // Save the field number

  _fieldNum = field_number;
  
  // Save the field header

  _fieldHdr = (MDV_field_header_t *)umalloc(sizeof(MDV_field_header_t));
  *_fieldHdr = *(field_obj->_fieldHdr);
  
  // Save the vlevel header

  if (field_obj->_vlevelHdr == NULL)
    _vlevelHdr = (MDV_vlevel_header_t *)NULL;
  else
  {
    _vlevelHdr = (MDV_vlevel_header_t *)umalloc(sizeof(MDV_vlevel_header_t));
    *_vlevelHdr = *(field_obj->_vlevelHdr);
  }
  
  // We don't want to copy the actual data here

  _data = NULL;
  _dataSize = 0;
  _dataAlloc = 0;
  
  // And there wasn't an input file for the new field

  _inputFile = (FILE *)NULL;
  
  // Copy the grid information

  _grid = new MdvGrid(field_obj->_grid);
  
}


/*********************************************************************
 * Destructor
 */

MdvFieldData::~MdvFieldData(void)
{
  static const char *routine_name = "Destructor";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  if (_debugLevel >= MDV_DEBUG_MSGS)
    fprintf(stdout,
	    "%s:  Destroying field number %d\n",
	    _className(), _fieldNum);

  // Delete the field header

  ufree(_fieldHdr);
  
  // Delete any existing vlevel header

  if (_vlevelHdr != NULL)
    ufree(_vlevelHdr);

  // Free any space allocated for data

  if (_data != NULL)
    free(_data);

  // Delete the grid information

  delete _grid;
  
}


/*********************************************************************
 * updateName - Update the field name and associated information in 
 *              the field header
 */

void MdvFieldData::updateName(char *field_name_long,
			      char *field_name,
			      char *units,
			      char *transform,
			      int field_code)
{
  static const char *routine_name = "updateName";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  if (_debugLevel >= MDV_DEBUG_MSGS)
    fprintf(stdout,
	    "%s::%s: Updating name information for field %d\n",
	    _className(), routine_name, _fieldNum);
  
  STRcopy((char *)_fieldHdr->field_name_long,
	  field_name_long, MDV_LONG_FIELD_LEN);
  STRcopy((char *)_fieldHdr->field_name,
	  field_name, MDV_SHORT_FIELD_LEN);
  STRcopy((char *)_fieldHdr->units,
	  units, MDV_UNITS_LEN);
  STRcopy((char *)_fieldHdr->transform,
	  transform, MDV_TRANSFORM_LEN);
  
  _fieldHdr->field_code = field_code;
  
  return;
}


/*********************************************************************
 * updateGridParams - Update the grid parameter values in the field
 *                    header
 */

void MdvFieldData::updateGridParams(double minx,
				    double miny,
				    double minz,
				    double dx,
				    double dy,
				    double dz,
				    int nx,
				    int ny,
				    int nz)
{
  static const char *routine_name = "updateGridParams";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  if (_debugLevel >= MDV_DEBUG_MSGS)
    fprintf(stdout,
	    "%s::%s: Updating grid parameters for field %d\n",
	    _className(), routine_name, _fieldNum);
  
  _fieldHdr->grid_minx = minx;
  _fieldHdr->grid_miny = miny;
  _fieldHdr->grid_minz = minz;

  _fieldHdr->grid_dx = dx;
  _fieldHdr->grid_dy = dy;
  _fieldHdr->grid_dz = dz;
  
  _fieldHdr->nx = nx;
  _fieldHdr->ny = ny;
  _fieldHdr->nz = nz;
  
  // Now update the grid object

  _grid->updateOrigin(minx, miny, minz);
  _grid->updateDeltas(dx, dy, dz);
  _grid->updateSize(nx, ny, nz);
  
  return;
}


/*********************************************************************
 * updateProjectionParams - Update the projection parameter values in
 *                          the field header
 */

void MdvFieldData::updateProjectionParams(int type,
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
  static const char *routine_name = "updateProjectionParams";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  if (_debugLevel >= MDV_DEBUG_MSGS)
    fprintf(stdout,
	    "%s::%s: Updating projection parameters for field %d\n",
	    _className(), routine_name, _fieldNum);
  
  // Make sure our assumption about the number of projection
  // parameters won't cause any problems

  assert(MDV_MAX_PROJ_PARAMS >= 8);
  
  _fieldHdr->proj_type = type;
  _fieldHdr->proj_origin_lat = origin_lat;
  _fieldHdr->proj_origin_lon = origin_lon;
  _fieldHdr->proj_rotation = rotation;
  
  _fieldHdr->proj_param[0] = param0;
  _fieldHdr->proj_param[1] = param1;
  _fieldHdr->proj_param[2] = param2;
  _fieldHdr->proj_param[3] = param3;
  _fieldHdr->proj_param[4] = param4;
  _fieldHdr->proj_param[5] = param5;
  _fieldHdr->proj_param[6] = param6;
  _fieldHdr->proj_param[7] = param7;
  
  // Make sure any extra projection parameters are set to 0.0

  for (int i = 8; i < MDV_MAX_PROJ_PARAMS; i++)
    _fieldHdr->proj_param[i] = 0.0;
  
  // Update the grid object

  _grid->updateProjection(type);
  
  return;
}


/*********************************************************************
 * updateDataParams - Update the data parameter values in the field
 *                    header
 */

void MdvFieldData::updateDataParams(int encoding_type,
				    int data_element_nbytes,
				    double scale,
				    double bias,
				    double bad_data_value,
				    double missing_data_value)
{
  static const char *routine_name = "updateDataParams";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  if (_debugLevel >= MDV_DEBUG_MSGS)
    fprintf(stdout,
	    "%s::%s: Updating data parameters for field %d\n",
	    _className(), routine_name, _fieldNum);
  
  _fieldHdr->encoding_type = encoding_type;
  _fieldHdr->data_element_nbytes = data_element_nbytes;

  _fieldHdr->scale = scale;
  _fieldHdr->bias = bias;

  _fieldHdr->bad_data_value = bad_data_value;
  _fieldHdr->missing_data_value = missing_data_value;
  
  return;
}


/*********************************************************************
 * updateData - Update the field data
 */

void MdvFieldData::updateData(time_t forecast_time,
			      int forecast_delta,
			      void *data,
			      int data_size,
			      int encoding_type)
{
  static const char *routine_name = "updateData";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  if (_debugLevel >= MDV_DEBUG_MSGS)
    fprintf(stdout,
	    "%s::%s: Updating data for field %d\n",
	    _className(), routine_name, _fieldNum);
  
  // Update the header values

  _fieldHdr->forecast_delta = forecast_delta;
  _fieldHdr->forecast_time = forecast_time;
  
  _fieldHdr->encoding_type = encoding_type;
  _fieldHdr->compression_type = 0;
  _fieldHdr->data_element_nbytes = MDV_data_element_size(encoding_type);
  _fieldHdr->volume_size = data_size;
  
  // Save the field data

  if (data_size > _dataAlloc)
  {
    if (_dataAlloc <= 0)
      _data = (void *)malloc(data_size);
    else
      _data = (void *)realloc(_data,
			      data_size);
    
    _dataAlloc = data_size;
  }
  
  memcpy(_data, data, data_size);
  _dataSize = data_size;
  
  return;
}


/*********************************************************************
 * getDataVolume - Retrieve the data volume for the field in the
 *                 desired format.
 */

void *MdvFieldData::getDataVolume(int *volume_size,
				  int mdv_encoding_type)
{
  static const char *routine_name = "getDataVolume";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  // Read the data from the data file, if necessary

  if (_data == NULL)
  {
    if (_inputFile == NULL)
    {
      *volume_size = 0;
      return(NULL);
    }
    else
    {
      _data = MDV_get_volume_size(_inputFile,
				  _fieldHdr,
				  _fieldHdr->encoding_type,
				  &_dataSize);
      _dataAlloc = _dataSize;
    }
  }
  
  // Convert the data to the desired format and return it to the
  // calling routine

  void *return_data = (void *)MDV_convert_volume((ui08 *)_data,
						 _dataSize,
						 _fieldHdr->nx,
						 _fieldHdr->ny,
						 _fieldHdr->nz,
						 _fieldHdr->encoding_type,
						 mdv_encoding_type,
						 volume_size);
  
  return(return_data);
}


/*********************************************************************
 * dump() - Dump the MDV field information in binary format to the
 *          indicated stream.  Returns the number of bytes of data
 *          written to the file (in case the output encoding type was
 *          different from the native encoding type) so that the next
 *          field's offset can be calculated.
 */

int MdvFieldData::dump(FILE *out_file,
		       int field_position,
		       int field_data_offset,
		       int output_encoding_type)
{
  static const char *routine_name = "dump";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
 
  // Dump the field header and data to the output file

  return(MDV_write_field(out_file,
			 _fieldHdr,
			 _data,
			 field_position,
			 field_data_offset,
			 output_encoding_type));
}


/*********************************************************************
 * print() - Print the MDV field information to the indicated stream.
 */

void MdvFieldData::print(char *filename,
			 int full_flag,
			 int data_flag)
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

  print(output_file, full_flag);
  
  // Close the output file

  fclose(output_file);
  
  return;
}


void MdvFieldData::print(FILE *stream,
			 int full_flag,
			 int data_flag)
{
  static const char *routine_name = "print";
  
  if (_debugLevel >= MDV_DEBUG_ROUTINES)
    fprintf(stdout,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Print the field header

  if (full_flag)
    MDV_print_field_header_full(_fieldHdr, stream);
  else
    MDV_print_field_header(_fieldHdr, stream);
  
  // Print the vlevel header, if there is one

  if (_vlevelHdr != NULL)
  {
    if (full_flag)
      MDV_print_vlevel_header_full(_vlevelHdr,
				   _fieldHdr->nz,
				   (char *)_fieldHdr->field_name_long,
				   stream);
    else
      MDV_print_vlevel_header(_vlevelHdr,
			      _fieldHdr->nz,
			      (char *)_fieldHdr->field_name_long,
			      stream);
  }
  
  // Print the data, if desired

  if (data_flag)
  {
  }
  
  return;
}
