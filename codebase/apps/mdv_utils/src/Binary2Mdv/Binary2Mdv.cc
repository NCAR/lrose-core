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
//   $Date: 2016/03/04 02:22:09 $
//   $Id: Binary2Mdv.cc,v 1.12 2016/03/04 02:22:09 dixon Exp $
//   $Revision: 1.12 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Binary2Mdv.cc: unisysm2mdv program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2004
 *
 * Dan Megenhardt
 *
 *********************************************************************/

#include <cassert>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <dataport/bigend.h>
#include <dataport/smallend.h>

#include "Binary2Mdv.hh"

using namespace std;

// Global variables

Binary2Mdv *Binary2Mdv::_instance = (Binary2Mdv *)NULL;


/*********************************************************************
 * Constructor
 */

Binary2Mdv::Binary2Mdv(int argc, char **argv)
{
  static const string method_name = "Binary2Mdv::Binary2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Binary2Mdv *)NULL);
  
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

  _data = new fl32[_params->input_proj.nx * _params->input_proj.ny * _params->input_proj.nz];
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

}


/*********************************************************************
 * Destructor
 */

Binary2Mdv::~Binary2Mdv()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  delete [] _data;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Binary2Mdv *Binary2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (Binary2Mdv *)NULL)
    new Binary2Mdv(argc, argv);
  
  return(_instance);
}

Binary2Mdv *Binary2Mdv::Inst()
{
  assert(_instance != (Binary2Mdv *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run()
 */

void Binary2Mdv::run()
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

      if ((next_file = _getNextFilename()) != (char *)NULL)
      {
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
      }
      else
        sleep(2);

    } // forever

  } // if (_params->run_mode == ARCHIVE)
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
////////////////////////////////////////////////////////////////
// writeMDV() - writes imageData to output MDV file
///

DateTime Binary2Mdv::getDataTime(const string &input_file_name)
{
    Path file_path(input_file_name);
    string file_base = file_path.getBase();

    DateTime return_date;
    return_date.setYear(atoi(file_base.substr(6,4).c_str()));
    return_date.setMonth(atoi(file_base.substr(10,2).c_str()));
    return_date.setDay(atoi(file_base.substr(12,2).c_str()));
    return_date.setHour(atoi(file_base.substr(14,2).c_str()));
    return_date.setMin(atoi(file_base.substr(16,2).c_str()));
    return_date.setSec(0);
    return return_date;
}


////////////////////////////////////////////////////////////////
// writeMDV() - writes imageData to output MDV file
///

int Binary2Mdv::writeMDV(const string &input_file_name)
{
  static const string method_name = "Binary2Mdv::writeMDV()";
  
  DsMdvx output_file;

  DateTime data_time = getDataTime(input_file_name);

    
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
  master_hdr.data_dimension = 3;

  master_hdr.data_collection_type = Mdvx::DATA_MEASURED;
  master_hdr.user_data = 0;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  master_hdr.vlevel_included = 1;

  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;

  master_hdr.n_fields = 0;

  master_hdr.n_chunks = 0;
  master_hdr.field_grids_differ = 0;

  master_hdr.sensor_lon = 0;
  master_hdr.sensor_lat = 0;
  master_hdr.sensor_alt = 0.0; 
  
  STRcopy(master_hdr.data_set_info, "Output from Binary2Mdv", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "Binary2Mdv", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, input_file_name.c_str(),
          MDV_NAME_LEN);

  output_file.setMasterHeader(master_hdr);
  

  // Construct the field header

  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));

  field_hdr.forecast_time = master_hdr.time_centroid;
  field_hdr.nx = _params->input_proj.nx;
  field_hdr.ny = _params->input_proj.ny;
  field_hdr.nz = _params->input_proj.nz;
  switch(_params->input_proj.proj_type)
  {
    case Params::PROJ_TYPE_LAT_LON:
      field_hdr.proj_type = Mdvx::PROJ_LATLON;
      break;
    case Params::PROJ_TYPE_FLAT:
      field_hdr.proj_type = Mdvx::PROJ_FLAT;
      break;
  }
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz * field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  field_hdr.dz_constant = 1;
  field_hdr.proj_origin_lat = _params->input_proj.origin_lat;
  field_hdr.proj_origin_lon = _params->input_proj.origin_lon;
  field_hdr.grid_dx = _params->input_proj.dx;
  field_hdr.grid_dy = _params->input_proj.dy;
  field_hdr.grid_dz = _params->input_proj.dz;
  field_hdr.grid_minx = _params->input_proj.minx;
  field_hdr.grid_miny = _params->input_proj.miny;
  field_hdr.grid_minz = _params->input_proj.minz;
  field_hdr.scale = 0.5;
  field_hdr.bias =  -32.0;
  field_hdr.bad_data_value = _params->bad_data_value;
  field_hdr.missing_data_value = _params->missing_data_value;
  field_hdr.proj_rotation = _params->input_proj.rotation;
  STRcopy(field_hdr.field_name_long,_params->data_field_name_long,MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name,_params->data_field_name,MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units,_params->data_units,MDV_UNITS_LEN);
  
  // Construct the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
 for (int i = 0; i < _params->Vlevels_n; i++)
  {
      vlevel_hdr.type[i] = Mdvx::VERT_TYPE_Z;
      vlevel_hdr.level[i] = _params->_Vlevels[i];
  }
 
  
  MdvxField *output_field;

  if ((output_field =
       new MdvxField(field_hdr,vlevel_hdr,(void *)_data)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output field" << endl;
    
    return -1;
  }

  double output_scale = 0.5;
  double output_bias  = -32.0;

  output_field->convertType(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_BZIP,
			    Mdvx::SCALING_SPECIFIED,
			     output_scale,
			     output_bias);

  
  output_file.addField(output_field);

  // Write the output file

  output_file.setWriteLdataInfo();
  
  if (output_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file to URL: " << _params->output_url << endl;
    
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

char *Binary2Mdv::_getNextFilename(void)
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
    
    if (_params->debug_level > Params::DEBUG_WARNINGS)
      cerr << "DEBUG: Waiting for data" << endl;
    
    PMU_auto_register("Waiting for data");
    sleep(2);
  }

  return(filename);
}


////////////////////////////////////////////////////////////////
// processFile() - convert file to MDV format
///

bool Binary2Mdv::processFile(const string &filename)
{
  if (!_readFile(filename))
  {
    cerr << "Error reading  file <" << filename << "> Skipping file" << endl;
    
    return false;
  }
  
  if (writeMDV(filename) != 0)
  {
    cerr << "Error writing MDV file for Unsisys file <"
	 << filename << "> Skipping file" << endl;
    
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////
// _readFile() - read input file
///

bool Binary2Mdv::_readFile(const string &filename)
{
// open file
    struct stat stat_info;
    if(stat(filename.c_str(),&stat_info) != 0)
    {
	cerr << "error stating" << filename << endl;
	return false;
    }

    FILE *file_pointer;
    if((file_pointer = fopen(filename.c_str(),"r")) == 0)
    {
	cerr << "Error opeining file " << filename << endl;
	return false;
    }
// read file
       
       int num_flts = stat_info.st_size / sizeof(fl32);
       cout << "num_flts = " << num_flts << endl;
       

       if((num_flts - 2) != (_params->input_proj.nx * _params->input_proj.ny * _params->input_proj.nz))
       {
	   cerr << "wrong file size " << filename << endl;
	   cerr << "file contains " << (num_flts -2) << " integers" << endl;
	   cerr << "should contain " << (_params->input_proj.nx * _params->input_proj.ny * _params->input_proj.nz) << " integers" << endl;
	   return false;
       }
       
       fl32 *data = new fl32[num_flts];
       int num_flts_read = fread(data,sizeof(fl32),num_flts,file_pointer);
       if(num_flts_read != num_flts)
       {
	   cerr << "error reading data from file " << filename << endl;
	   cerr << "read " << num_flts_read << " integers " << endl;
	   cerr << "expecting " << num_flts << " integers " << endl;
	   fclose(file_pointer);
	   delete [] data;
	   return false;
       }
       
   // close file
       fclose(file_pointer);
       
       int i=0;
       int nx = _params->input_proj.nx;
       int ny = _params->input_proj.ny;
       int nz = _params->input_proj.nz;
       
       for(int z = (nz - 1); z >= 0; z--)
       {
	 // skip first and last 4byte float
	 int start_grid_pt = (z * nx * ny) + 1;
	 int end_grid_pt = ((z + 1) * nx * ny);
	 cout << "z = " << z << endl;
	 cout << "start_grid_pt = " << start_grid_pt << endl;
	 cout << "end_grid_pt = " << end_grid_pt << endl;
	 for(int x = start_grid_pt; x <=  end_grid_pt ;x++)
	   {
	       _data[i] = data[x];
	       i++;
	   }
	   
       }
   //       cerr << "number of flts = " << num_flts-2 << endl;
   //       cerr << "number processed = " << i << endl;
       delete [] data;	       

       int num_bytes;
       
       if(_params->is_bigend) 
       {
	   num_bytes =  BE_to_array_32(_data,num_flts * sizeof(fl32));
       }
       else 
       {
	   num_bytes = SE_to_array_32(_data,num_flts * sizeof(fl32));
       }
       
       cerr << "Bytes converted :" << num_bytes << endl;
       
       return true;
}

