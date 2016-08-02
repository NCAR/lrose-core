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
//   $Date: 2016/03/07 18:28:25 $
//   $Id: ShapeGridPt2Mdv.cc,v 1.5 2016/03/07 18:28:25 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ShapeGridPt2Mdv : ShapeGridPt2Mdv program class
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>

#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Args.hh"
#include "Params.hh"
#include "ShapeGridPt2Mdv.hh"

using namespace std;


// Global variables

ShapeGridPt2Mdv *ShapeGridPt2Mdv::_instance = (ShapeGridPt2Mdv *)NULL;


/*********************************************************************
 * Constructor
 */

ShapeGridPt2Mdv::ShapeGridPt2Mdv(int argc, char **argv)
{
  const string method_name = "Constructor";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (ShapeGridPt2Mdv *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = (char *) "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file <" << params_path <<
      ">" << endl;
    
    okay = false;
    
    return;
  }

}


/*********************************************************************
 * Destructor
 */

ShapeGridPt2Mdv::~ShapeGridPt2Mdv()
{
  // Free contained objects

  delete _args;
  delete [] _inputLine;
  
  // Free included strings

  STRfree(_progName);

}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

ShapeGridPt2Mdv *ShapeGridPt2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (ShapeGridPt2Mdv *)NULL)
    new ShapeGridPt2Mdv(argc, argv);
  
  return(_instance);
}

ShapeGridPt2Mdv *ShapeGridPt2Mdv::Inst()
{
  assert(_instance != (ShapeGridPt2Mdv *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool ShapeGridPt2Mdv::init()
{
  // Allocate space for the input line

  _inputLine = new char[BUFSIZ];
  
  // Initialize the output projection

  switch (_params->output_proj.proj_type)
  {
  case Params::PROJ_LATLON :
    _outputProj.initLatlon(_params->output_proj.nx,
			   _params->output_proj.ny,
			   1,
			   _params->output_proj.dx,
			   _params->output_proj.dy,
			   1.0,
			   _params->output_proj.minx,
			   _params->output_proj.miny,
			   0.5);
    break;
  case Params::PROJ_FLAT :
    _outputProj.initFlat(_params->output_proj.origin_lat,
			 _params->output_proj.origin_lon,
			 _params->output_proj.rotation,
			 _params->output_proj.nx,
			 _params->output_proj.ny,
			 1,
			 _params->output_proj.dx,
			 _params->output_proj.dy,
			 1.0,
			 _params->output_proj.minx,
			 _params->output_proj.miny,
			 0.5);
    break;
  case Params::PROJ_LC :
    _outputProj.initLc2(_params->output_proj.origin_lat,
			_params->output_proj.origin_lon,
			_params->output_proj.lat1,
			_params->output_proj.lat2,
			_params->output_proj.nx,
			_params->output_proj.ny,
			1,
			_params->output_proj.dx,
			_params->output_proj.dy,
			1.0,
			_params->output_proj.minx,
			_params->output_proj.miny,
			0.5);
    break;
  } /* endswitch - _params->output_proj.proj_type */
  
  return true;
}


/*********************************************************************
 * run()
 */

void ShapeGridPt2Mdv::run()
{
  static const string method_name = "ShapeGridPt2Mdv::run()";

  // Create the field that will contain the grid

  DateTime file_creation_time;
  MdvxField *field;
  
  if ((field = _createField(file_creation_time)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating blank output field" << endl;
    
    return;
  }
  
  // Update the grid points in the field with the values from the
  // input file

  if (!_updateField(*field))
    return;
  
  // Create the Mdvx object

  Mdvx  mdvx;
  
  _setMasterHeader(mdvx, file_creation_time);
  mdvx.addField(field);
  
  // Write the output file

  mdvx.clearWriteLdataInfo();
  if (mdvx.writeToPath(_params->output_path) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file" << endl;
    perror(_params->output_path);
    
    return;
  }
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _addData() - Add the data from this line to the given field data.
 */

void ShapeGridPt2Mdv::_addData(fl32 *field_data,
			       const char *input_line) const
{
  static const string method_name = "ShapeGridPt2Mdv::_addData()";
  
  // Parse the input line

  double lat, lon, value;
  
  if (sscanf(input_line, "\"%lf , %lf , %lf\"",
	     &lon, &lat, &value) != 3 &&
      sscanf(input_line, "%lf , %lf , %lf",
	     &lon, &lat, &value) != 3)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Error parsing input line: <" << input_line << ">" << endl;
    cerr << "--- Skipping line ---" << endl;
    
    return;
  }
  
  // Update the data point

  int index;
  
  if (_outputProj.latlon2arrayIndex(lat, lon, index) == 0)
  {
    double output_value = value;
    
    for (int i = 0; i < _params->conversion_table_n; ++i)
    {
      if (_params->_conversion_table[i].input_value == value)
	output_value = _params->_conversion_table[i].output_value;
    }
    
    field_data[index] = output_value;
  }
}


/*********************************************************************
 * _createField() - Create the output field.  Initialize the field with
 *                  missing data values which will be overwritten with
 *                  data values as they are processed.
 *
 * Returns a pointer to the created field on success, 0 on failure.
 */

MdvxField *ShapeGridPt2Mdv::_createField(const DateTime &file_time) const
{
  // Create the field header

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.forecast_time = file_time.utime();
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.dz_constant = 1;
  field_hdr.data_dimension = 2;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = -9999.0;
  field_hdr.missing_data_value = field_hdr.bad_data_value;
  STRcopy(field_hdr.field_name_long, _params->output_field_name,
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, _params->output_field_name,
	  MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, _params->output_field_units, MDV_UNITS_LEN);
  
  _outputProj.syncToFieldHdr(field_hdr);
  
  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  
  // Create and return the field

  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}


/*********************************************************************
 * _setMasterHeader() - Set the master header in the output file.
 */

void ShapeGridPt2Mdv::_setMasterHeader(Mdvx &mdvx,
				       const DateTime &file_time) const
{
  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = file_time.utime();
  master_hdr.time_begin = file_time.utime();
  master_hdr.time_end = file_time.utime();
  master_hdr.time_centroid = file_time.utime();
  master_hdr.time_expire = file_time.utime();
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_MEASURED;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  STRcopy(master_hdr.data_set_info, "Generated by ShapeGridPt2Mdv",
	  MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "ShapeGridPt2Mdv", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, _params->input_path, MDV_NAME_LEN);
  
  mdvx.setMasterHeader(master_hdr);
}


/*********************************************************************
 * _updateField() - Update the given field with the data from the
 *                  input file.
 *
 * Returns true on success, false on failure.
 */

bool ShapeGridPt2Mdv::_updateField(MdvxField &field) const
{
  static const string method_name = "ShapeGridPt2Mdv::_updateField()";
  
  // Open the input file

  FILE *input_file;
  
  if ((input_file = fopen(_params->input_path, "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input file" << endl;
    perror(_params->input_path);
    
    return false;
  }
  
  // Process each of the lines in the input file.  Skip the first line since
  // it is just a comment with the line contents.

  fl32 *field_data = (fl32 *)field.getVol();
  
  fgets(_inputLine, BUFSIZ, input_file);
  
  while (fgets(_inputLine, BUFSIZ, input_file) != 0)
    _addData(field_data, _inputLine);
  
  // Close the input file

  fclose(input_file);
  
  return true;
}
