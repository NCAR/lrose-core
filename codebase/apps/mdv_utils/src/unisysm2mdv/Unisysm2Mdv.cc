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
//   $Date: 2016/03/04 02:22:16 $
//   $Id: Unisysm2Mdv.cc,v 1.10 2016/03/04 02:22:16 dixon Exp $
//   $Revision: 1.10 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Unisysm2Mdv.cc: unisysm2mdv program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <dataport/bigend.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/pjg.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>

#include "Unisysm2Mdv.hh"

using namespace std;

// Global variables

Unisysm2Mdv *Unisysm2Mdv::_instance = (Unisysm2Mdv *)NULL;


/*********************************************************************
 * Constructor
 */

Unisysm2Mdv::Unisysm2Mdv(int argc, char **argv)
{
  static const string method_name = "Unisysm2Mdv::Unisysm2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Unisysm2Mdv *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  if (!_args->okay)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with command line arguments." << endl;
    
    okay = false;
    
    return;
  }
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file <"
	 << params_path << ">" << endl;
    
    okay = false;
    
    return;
  }

  // Initialize the Unisys file handler

  Pjg output_projection;
  output_projection.initLatlon(_params->x_grid.n,
			       _params->y_grid.n,
			       1,
			       _params->x_grid.delta,
			       _params->y_grid.delta,
			       1.0,
			       _params->x_grid.min,
			       _params->y_grid.min,
			       0.5);
  
  if (!_unisysFile.init(_params->std_parallel_1,
			_params->std_parallel_2,
			_params->valid_source_id,
			_params->data_field_name_long,
			_params->data_field_name,
			_params->data_units,
			_params->data_scale,
			_params->data_bias,
			_params->missing_data_value,
			output_projection,
			_params->debug_level >= Params::DEBUG_NORM))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error initializing Unisys file handler" << endl;
    
    exit(0);
  }
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

}


/*********************************************************************
 * Destructor
 */

Unisysm2Mdv::~Unisysm2Mdv()
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

Unisysm2Mdv *Unisysm2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (Unisysm2Mdv *)NULL)
    new Unisysm2Mdv(argc, argv);
  
  return(_instance);
}

Unisysm2Mdv *Unisysm2Mdv::Inst()
{
  assert(_instance != (Unisysm2Mdv *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run()
 */

void Unisysm2Mdv::run()
{
  // Process files based on run_mode setting

  if (_params->run_mode == Params::ARCHIVE)
  {
    vector< string > file_list = _args->getInputFiles();
    vector< string >::iterator file_ptr;
    
    for (file_ptr = file_list.begin(); file_ptr != file_list.end(); ++file_ptr)
      processFile(*file_ptr);
  }
  else
  {

    // Initialize input directory object

    _inputDir = new InputDir(_params->input_dir,
                             _params->input_file_substring,
                             TRUE);
  
    while (true)
    {
      char *next_file = NULL;

      // Register with the process mapper

      PMU_auto_register("Waiting for data");

      // Check for new data

      if ((next_file = _getNextFilename()) == (char *)NULL)
      {
	sleep(2);
	continue;
      }
      
      // Don't process files whose names start with "."

      Path file_path(next_file);
      string file_base = file_path.getBase();
      if (file_base[0] == '.')
	continue;
      
      // If we get here, we can process the file

      char pmu_msg[1024];

      sprintf(pmu_msg, "Processing file <%s>",
	      next_file);

      PMU_force_register(pmu_msg);

      // Wait if required
      
      if (_params->processing_delay > 0)
	sleep(_params->processing_delay);

      // Process the file
      
      processFile(next_file);

      delete [] next_file;

    } // forever

  } // if (_params->run_mode == ARCHIVE)
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

////////////////////////////////////////////////////////////////
// writeMDV() - writes imageData to output MDV file
///

int Unisysm2Mdv::writeMDV(const string &input_file_name)
{
  static const string method_name = "Unisysm2Mdv::writeMDV()";
  
  DsMdvx output_file;

  DateTime data_time = _unisysFile.getDataTime();
  
  // Construct the master header

  Mdvx::master_header_t master_hdr;
  
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.user_time = 0;
  master_hdr.time_begin = data_time.utime() - _params->start_time_offset;
  master_hdr.time_end = data_time.utime() + _params->end_time_offset;
  master_hdr.time_centroid = data_time.utime() + _params->mid_time_offset;
  master_hdr.time_expire = master_hdr.time_end;

  master_hdr.num_data_times = 0;
  master_hdr.index_number = 0;
  master_hdr.data_dimension = 2;

  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr.user_data = 0;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  master_hdr.vlevel_included = 1;

  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;

  master_hdr.n_fields = 0;

  master_hdr.n_chunks = 0;
  master_hdr.field_grids_differ = 0;

  master_hdr.sensor_lon = _unisysFile.getCenterLongitude() / 1000; 
  master_hdr.sensor_lat = _unisysFile.getCenterLatitude() / 1000;   
  master_hdr.sensor_alt = 0.5; 
  
  STRcopy(master_hdr.data_set_info, "Output from unisysm2mdv", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "unisysm2mdv", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, input_file_name.c_str(),
          MDV_NAME_LEN);

  output_file.setMasterHeader(master_hdr);
  

  MdvxField *output_field;

  if ((output_field =
       _unisysFile.createMdvxField(DateTime(master_hdr.time_centroid))) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output field" << endl;
    
    return -1;
  }

  if (output_field->remap2Latlon(_remapLut,
				 _params->x_grid.n, _params->y_grid.n,
				 _params->x_grid.min, _params->y_grid.min,
				 _params->x_grid.delta, _params->y_grid.delta) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error remapping output field" << endl;
    
    return -1;
  }
  
  output_field->convertType(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_BZIP,
			    Mdvx::SCALING_SPECIFIED,
			    _params->data_scale,
			    _params->data_bias);
  
  output_file.addField(output_field);

  // Write the output file

  output_file.setWriteLdataInfo();
  
  if (output_file.writeToDir(_params->output_dir) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file to URL: " << _params->output_dir << endl;
    
    return -1;
  }
  
  if (_params->debug_level > Params::DEBUG_OFF &&
      _params->run_mode == Params::ARCHIVE)
    cerr << "Output_time: " << DateTime::str(master_hdr.time_centroid) << endl;

  return(0);
}


////////////////////////////////////////////////////////////////
// _getNextFilename()
///

char *Unisysm2Mdv::_getNextFilename(void)
{
  char *filename;

  while (true)
  {
    filename = _inputDir->getNextFilename(TRUE,
                                          _params->max_input_data_age);

    if (filename != NULL)
    {
      if (_params->debug_level > Params::DEBUG_WARNINGS)
        cerr << "DEBUG: filename is " << filename << endl;

      break;
    }
    
    if (_params->debug_level > Params::DEBUG_EXTRA)
      cerr << "DEBUG: Waiting for data" << endl;
    
    PMU_auto_register("Waiting for data");
    sleep(2);
  }

  return(filename);
}


////////////////////////////////////////////////////////////////
// processFile() - convert file to MDV format
///

int Unisysm2Mdv::processFile(const string &filename)
{
  static const string method_name = "Unisysm2Mdv::processFile()";

  if (_params->debug_level > Params::DEBUG_OFF)
  {
    cerr << endl;
    cerr << "Processing file: " << filename << endl;
  }

  // Make sure the file is uncompressed

  char local_filename[filename.length() + 1];

  STRcopy(local_filename, filename.c_str(), filename.length() + 1);

  if (ta_file_uncompress(local_filename) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error uncompressing file: " << filename << endl;

    return -1;
  }

  if (!_unisysFile.readFile(local_filename))
  {
    cerr << "Error reading Unisys file <" << local_filename << "> Skipping file" << endl;
    
    return -1;
  }

  if (_params->debug_level > Params::DEBUG_OFF)
    _unisysFile.printHeader();

  if (writeMDV(filename) != 0)
  {
    cerr << "Error writing MDV file for Unsisys file <"
	 << filename << "> Skipping file" << endl;
    
    return(-1);
  }

  return(0);
}


