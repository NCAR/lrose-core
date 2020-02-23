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
//   $Date: 2016/03/06 23:53:43 $
//   $Id: Wsim2mdv.cc,v 1.17 2016/03/06 23:53:43 dixon Exp $
//   $Revision: 1.17 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Wsim2mdv.cc: wsim2mdv program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <signal.h>

#include <toolsa/os_config.h>
#include <didss/DsInputPath.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Params.hh"
#include "Wsim2mdv.hh"
using namespace std;

// Global variables

Wsim2mdv *Wsim2mdv::_instance = (Wsim2mdv *)NULL;

// Global constants

const int FOREVER = true;

/*********************************************************************
 * Constructor
 */

Wsim2mdv::Wsim2mdv(int argc, char **argv)
{
  static const char *routine_name = "Constructor";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Wsim2mdv *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  if (!_args->okay)
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr,
	    "Problem with command line arguments.\n");
    
    okay = false;
    
    return;
  }
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = (char *) "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr,
	    "Problem with TDRP parameters in file <%s>\n",
	    params_path);
    
    okay = false;
    
    return;
  }

  // Make sure the archive mode information is consistent

  if (_params->mode == Params::ARCHIVE &&
      _args->numInputFiles <= 0)
  {
    fprintf(stderr, "ERROR - %s\n", _progName);
    fprintf(stderr, "For ARCHIVE mode must specify input file list\n");

    okay = false;
    return;
  }

  // Create the WSI mosaic file object

  double value_table[15];
  
  for (int i = 0; i < 15; i++)
    value_table[i] = _params->_color2data[i];

  if (_params->resample_grid &&
      _params->output_projection == Params::PROJ_FLAT)
    _wsimFile = new WsimFile(value_table,
			     15,
			     _params->data_scale,
			     _params->data_bias,
			     _params->resample_grid,
			     WSIM_FILE_PROJ_FLAT,
			     _params->x_grid.min,
			     _params->y_grid.min,
			     _params->x_grid.delta,
			     _params->y_grid.delta,
			     _params->x_grid.n,
			     _params->y_grid.n,
			     _params->flat_origin.lat,
			     _params->flat_origin.lon,
			     _convertParamFilterType(_params->filter_type),
			     _params->coverage_threshold,
			     _params->debug_level);
  else
    _wsimFile = new WsimFile(value_table,
			     15,
			     _params->data_scale,
			     _params->data_bias,
			     _params->resample_grid,
			     WSIM_FILE_PROJ_LATLON,
			     _params->x_grid.min,
			     _params->y_grid.min,
			     _params->x_grid.delta,
			     _params->y_grid.delta,
			     _params->x_grid.n,
			     _params->y_grid.n,
			     _params->flat_origin.lat,
			     _params->flat_origin.lon,
			     _convertParamFilterType(_params->filter_type),
			     _params->coverage_threshold,
			     _params->debug_level);
  
  // Initialize process registration
  
  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // Initialize the input path object

  if (_params->mode == Params::REALTIME)
  {
    if (_params->use_ldata_info)
      _inputPath = new DsInputPath(_progName,
				   _params->debug_level >= Params::DEBUG_NORM,
				   _params->input_dir,
				   _params->max_input_data_age,
				   PMU_auto_register,
				   true, true);
    else
      _inputPath = new DsInputPath(_progName,
				   _params->debug_level >= Params::DEBUG_NORM,
				   _params->input_dir,
				   _params->max_input_data_age,
				   PMU_auto_register,
				   false, true);
  }
  else
  {
    _inputPath = new DsInputPath(_progName,
				 _params->debug_level >= Params::DEBUG_NORM,
				 _args->numInputFiles,
				 _args->inputFileList);
  }
  
}


/*********************************************************************
 * Destructor
 */

Wsim2mdv::~Wsim2mdv()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Wsim2mdv *Wsim2mdv::Inst(int argc, char **argv)
{
  if (_instance == (Wsim2mdv *)NULL)
    new Wsim2mdv(argc, argv);
  
  return(_instance);
}

Wsim2mdv *Wsim2mdv::Inst()
{
  assert(_instance != (Wsim2mdv *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run()
 */

void Wsim2mdv::run()
{
  /*
   * process the files
   */

  while (FOREVER)
  {
    char *next_file;
      
    /*
     * Register with the process mapper
     */

    PMU_auto_register("Waiting for data");

    /*
     * Check for new data
     */
      
    while ((next_file = _inputPath->next()) != (char *)NULL)
    {
      char pmu_msg[1024];
	
      sprintf(pmu_msg, "Processing file <%s>",
	      next_file);
	
      PMU_force_register(pmu_msg);
	
      /*
       * wait if required
       */

      if (_params->mode == Params::REALTIME &&
	  _params->processing_delay > 0)
	sleep(_params->processing_delay);

      /*
       * process the file
       */
      
      _processFile(next_file);
    }

    if (_params->mode == Params::ARCHIVE)
    {
      fprintf(stdout, "----------> Archive mode -- EXITING\n");
      break;
    }
    
    sleep(2);
      
  } /* endwhile - FOREVER */

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**************************************************************
 * _convertParamFilterType()
 */

WsimFileFilterType_t Wsim2mdv::_convertParamFilterType(int filter_type)
{
  switch(filter_type)
  {
  case Params::MAX_FILTER :
    return(WSIM_FILE_FILTER_MAX);
    
  case Params::MEAN_DBZ_FILTER :
    return(WSIM_FILE_FILTER_MEAN_DBZ);
    
  case Params::MEAN_Z_FILTER :
    return(WSIM_FILE_FILTER_MEAN_Z);
    
  default:
    return(WSIM_FILE_FILTER_MAX);
  }
}


/**************************************************************
 * _processFile()
 */

void Wsim2mdv::_processFile(char *file_name)
{
  static const string method_name = "Wsim2mdv::_processFile()";
  
  if (_params->debug_level >= Params::DEBUG_NORM)
    fprintf(stderr, "Processing file <%s>\n", file_name);

  // Read in the WSI file

  if (_wsimFile->read(file_name) != WSIM_FILE_OKAY)
  {
    fprintf(stderr, "*** Error processing file <%s>\n",
	    file_name);
    
    return;
  }

  if (_params->debug_level >= Params::DEBUG_EXTRA)
    _wsimFile->printHeader(stderr);
  
  time_t data_time = _wsimFile->getDataTime();
  
  // Create the MDV object

  DsMdvx mdv_obj;
  
  Mdvx::master_header_t master_hdr;
  
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = data_time + _params->start_time_offset;
  master_hdr.time_end = data_time + _params->end_time_offset;
  master_hdr.time_centroid = data_time + _params->mid_time_offset;
  master_hdr.time_expire = master_hdr.time_end;
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  master_hdr.vlevel_included = 0;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  master_hdr.n_fields = 0;
  master_hdr.n_chunks = 0;
  master_hdr.sensor_lon = _wsimFile->getOriginLon();
  master_hdr.sensor_lat = _wsimFile->getOriginLat();
  master_hdr.sensor_alt = 0.5;
  master_hdr.data_set_info[0] = '\0';
  master_hdr.data_set_name [0] = '\0';
  STRcopy(master_hdr.data_set_source, file_name, MDV_NAME_LEN);
  
  mdv_obj.setMasterHeader(master_hdr);
  

  // Create the field

  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.forecast_time = master_hdr.time_centroid;
  field_hdr.nx = _wsimFile->getNx();
  field_hdr.ny = _wsimFile->getNy();
  field_hdr.nz = 1;
  if (_params->output_projection == Params::PROJ_LATLON)
    field_hdr.proj_type = Mdvx::PROJ_LATLON;
  else
    field_hdr.proj_type = Mdvx::PROJ_FLAT;
  field_hdr.encoding_type = Mdvx::ENCODING_INT8;
  field_hdr.data_element_nbytes = 1;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_SPECIFIED;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  field_hdr.proj_origin_lat = _wsimFile->getOriginLat();
  field_hdr.proj_origin_lon = _wsimFile->getOriginLon();
  field_hdr.grid_dx = _wsimFile->getDx();
  field_hdr.grid_dy = _wsimFile->getDy();
  field_hdr.grid_dz = 1.0;
  field_hdr.grid_minx = _wsimFile->getMinX();
  field_hdr.grid_miny = _wsimFile->getMinY();
  field_hdr.grid_minz = 0.5;
  field_hdr.scale = _params->data_scale;
  field_hdr.bias = _params->data_bias;
  field_hdr.bad_data_value = 0.0;
  field_hdr.missing_data_value = 0.0;
  field_hdr.proj_rotation = 0.0;
  STRcopy(field_hdr.field_name_long, _params->data_field_name_long,
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, _params->data_field_name, MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, _params->data_units, MDV_UNITS_LEN);
  STRcopy(field_hdr.transform, "wsim2mdv", MDV_TRANSFORM_LEN);
  
  Mdvx::vlevel_header_t vlevel_hdr;
  
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_Z;
  vlevel_hdr.level[0] = 0.5;
  
  MdvxField *dbz_field = new MdvxField(field_hdr,
				       vlevel_hdr,
				       _wsimFile->getData());
  
  dbz_field->requestCompression(Mdvx::COMPRESSION_GZIP);
  mdv_obj.addField(dbz_field);
  
  // Write the MDV file

  mdv_obj.setWriteLdataInfo();
  mdv_obj.writeToDir(_params->output_url);
  
  return;
}
