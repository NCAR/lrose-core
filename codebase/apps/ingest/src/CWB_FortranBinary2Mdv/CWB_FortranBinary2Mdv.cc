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
 *
 * @file CWB_FortranBinary2Mdv.cc
 *
 * @class CWB_FortranBinary2Mdv
 *
 * CWB_FortranBinary2Mdv is the top level application class.
 *  
 * @date 8/25/2010
 *
 */

#include <cassert>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
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

#include "CWB_FortranBinary2Mdv.hh"

using namespace std;

// Global variables

CWB_FortranBinary2Mdv *CWB_FortranBinary2Mdv::_instance = (CWB_FortranBinary2Mdv *)NULL;


/*********************************************************************
 * Constructor
 */

CWB_FortranBinary2Mdv::CWB_FortranBinary2Mdv(int argc, char **argv)
{
  static const string method_name = "CWB_FortranBinary2Mdv::CWB_FortranBinary2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (CWB_FortranBinary2Mdv *)NULL);
  
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
  
  _params = new Params();
  char *params_path = new char[strlen("unknown") + 1];
  strcpy(params_path, "unknown");
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file <"
	 << params_path << ">" << endl;

    delete [] params_path;
    
    okay = false;
    
    return;
  }

}


/*********************************************************************
 * Destructor
 */

CWB_FortranBinary2Mdv::~CWB_FortranBinary2Mdv()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  delete _dataTrigger;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

CWB_FortranBinary2Mdv *CWB_FortranBinary2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (CWB_FortranBinary2Mdv *)NULL)
    new CWB_FortranBinary2Mdv(argc, argv);
  
  return(_instance);
}

CWB_FortranBinary2Mdv *CWB_FortranBinary2Mdv::Inst()
{
  assert(_instance != (CWB_FortranBinary2Mdv *)NULL);
  
  return(_instance);
}

/*********************************************************************
 * init()
 */

bool CWB_FortranBinary2Mdv::init()
{
  static const string method_name = "CWB_FortranBinary2Mdv::init()";
  
  // Initialize the input trigger

  switch (_params->trigger_mode)
  {
  case Params::FILE_LIST :
  {
    const vector< string > file_list = _args->getFileList();
    
    if (file_list.size() == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Must specify file paths on command line" << endl;
      
      return false;
    }
    
    if (_params->debug_level > Params::DEBUG_OFF)
    {
      cerr << "Initializing FILE_LIST trigger: " << endl;
      vector< string >::const_iterator file;
      for (file = file_list.begin(); file != file_list.end(); ++file)
	cerr << "   " << *file << endl;
    }
    
    DsFileListTrigger *trigger = new DsFileListTrigger();
    if (trigger->init(file_list) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing FILE_LIST trigger:" << endl;
      vector< string >::const_iterator file;
      for (file = file_list.begin(); file != file_list.end(); ++file)
	cerr << "   " << *file << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  case Params::REALTIME :
  {

    if (_params->debug_level > Params::DEBUG_OFF)
    {
      cerr << "Initializing REALTIME trigger using directory: " <<
	_params->realtimeInput.InputDir << endl;
    }
    
    struct stat stat_info;
    if(stat(_params->realtimeInput.InputDir,&stat_info) != 0)
    {
	cerr << "Error: Directory " << _params->realtimeInput.InputDir 
	     << " does not exist." << endl;
	return false;
    }

    DsInputDirTrigger *trigger = new DsInputDirTrigger();
    if (trigger->init(_params->realtimeInput.InputDir,
		      _params->realtimeInput.IncludeSubString,
		      _params->realtimeInput.ProcessOldFiles,
		      PMU_auto_register,
		      false,
		      _params->realtimeInput.ExcludeSubString) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing REALTIME trigger for directory:" << 
	_params->realtimeInput.InputDir << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }

    _dataTrigger = trigger;

    if (_params->debug_level > Params::DEBUG_OFF)
      cerr << "Successfully initialized  REALTIME trigger for directory:" << 
	_params->realtimeInput.InputDir << endl;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */
 
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void CWB_FortranBinary2Mdv::run()
{

 static const string method_name = "CWB_FortranBinary2Mdv::run()";
  
  TriggerInfo trigger_info;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_info.getFilePath()))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing file: " << trigger_info.getFilePath() << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
////////////////////////////////////////////////////////////////
// _getDataTime() - Gets data date and time from input file name
///

DateTime CWB_FortranBinary2Mdv::_getDataTime(const string &input_file_name)
{
    Path file_path(input_file_name);
    string file_base = file_path.getBase();

    DateTime return_date;
    switch(_params->data_type)
    {
    case Params::SATELLITE:
    return_date.setYear(atoi(file_base.substr(0,4).c_str()));
    return_date.setMonth(atoi(file_base.substr(5,2).c_str()));
    return_date.setDay(atoi(file_base.substr(8,2).c_str()));
    return_date.setHour(atoi(file_base.substr(11,2).c_str()));
    return_date.setMin(atoi(file_base.substr(13,2).c_str()));
    return_date.setSec(0);
    break;
    case Params::CLIMATOLOGY:
    return_date.setYear(2011);
    return_date.setMonth(8);
    return_date.setDay(1);
    return_date.setHour(atoi(file_base.substr(0,2).c_str()));
    return_date.setMin(atoi(file_base.substr(2,2).c_str()));
    return_date.setSec(0);
    break;
    }

    return return_date;
}


////////////////////////////////////////////////////////////////
// _writeMDV() - writes Data to output MDV file
///

int CWB_FortranBinary2Mdv::_writeMDV(const string &input_file_name, fl32 *data)
{
  static const string method_name = "CWB_FortranBinary2Mdv::writeMDV()";
  
  DsMdvx output_file;

  DateTime data_time = _getDataTime(input_file_name);

    
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
  
  STRcopy(master_hdr.data_set_info, "Output from CWB_FortranBinary2Mdv", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "CWB_FortranBinary2Mdv", MDV_NAME_LEN);
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
  field_hdr.scale = _params->scaling_info.scale;
  field_hdr.bias =  _params->scaling_info.bias;
  field_hdr.bad_data_value = _params->bad_data_value;
  field_hdr.missing_data_value = _params->missing_data_value;
  field_hdr.proj_rotation = _params->input_proj.rotation;
  STRcopy(field_hdr.field_name_long,_params->data_field_name_long,MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name,_params->data_field_name,MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units,_params->data_units,MDV_UNITS_LEN);
  
  // Construct the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));

  if( _params->AssignVlevelValues)
  {
    for (int i = 0; i < _params->Vlevels_n; i++)
    {
      if( _params->Vlevels_n == 1)
      {
	vlevel_hdr.type[i] = Mdvx::VERT_TYPE_SURFACE;
      }
      else 
      {
	vlevel_hdr.type[i] = Mdvx::VERT_TYPE_Z;
      }
    
      vlevel_hdr.level[i] = _params->_Vlevels[i];
    }
  }
  else // Assigne Vlevels based on projection information
  {
    for (int i = 0; i < _params->input_proj.nz; i++)
    {
      if( _params->input_proj.nz == 1)
      {
	vlevel_hdr.type[i] = Mdvx::VERT_TYPE_SURFACE;
      }
      else 
      {
	vlevel_hdr.type[i] = Mdvx::VERT_TYPE_Z;
      }
      vlevel_hdr.level[i] = _params->input_proj.minz + ( i * _params->input_proj.dz );
    }
  }
  
   
  
  MdvxField *output_field;

  if ((output_field =
       new MdvxField(field_hdr,vlevel_hdr,(void *)data)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output field" << endl;
    
    return -1;
  }

  if( _params->use_scaling_info && _params->data_type == Params::SATELLITE)
  {
    double output_scale = _params->scaling_info.scale;
    double output_bias  = _params->scaling_info.bias;

    output_field->convertType(Mdvx::ENCODING_INT8,
			      Mdvx::COMPRESSION_BZIP,
			      Mdvx::SCALING_SPECIFIED,
			      output_scale,
			      output_bias);
  }
  else // set scaling to dynamic
  {
    switch(_params->data_type)
    {
    case Params::SATELLITE:
    output_field->convertType(Mdvx::ENCODING_INT8,
			      Mdvx::COMPRESSION_BZIP,
			      Mdvx::SCALING_DYNAMIC);
    break;
    case Params::CLIMATOLOGY:    
    output_field->convertType(Mdvx::ENCODING_FLOAT32,
			      Mdvx::COMPRESSION_GZIP);
    break;
    }
  }
  
  
  output_file.addField(output_field);

  // Write the output file

  output_file.setWriteLdataInfo();
  
  if (output_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file to URL: " << _params->output_url << endl;
    
    return -1;
  }
  
  return(0);
}



////////////////////////////////////////////////////////////////
// processData() - convert file to MDV format
///

bool CWB_FortranBinary2Mdv::_processData(const string &file_path)
{

  PMU_auto_register("Reading File");

  cerr << "*** Processing file: " << file_path << endl;
  
  fl32 * data = new fl32[_params->input_proj.nx * _params->input_proj.ny * _params->input_proj.nz ];

  if (!_readFile(file_path, data))
  {
    cerr << "Error reading  file <" << file_path << "> Skipping file" << endl;
    
    delete [] data;
    return false;
  }
  
  PMU_auto_register("Writing File");

  if (_writeMDV(file_path, data) != 0)
  {
    cerr << "Error writing MDV file for file <"
	 << file_path << "> Skipping file" << endl;
    
    delete [] data;
    return false;
  }

  delete [] data;

  return true;
}

////////////////////////////////////////////////////////////////
// _readFile() - read input file
///

bool CWB_FortranBinary2Mdv::_readFile(const string &filename, fl32 *data)
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

       if((num_flts - 2) != (_params->input_proj.nx * _params->input_proj.ny * _params->input_proj.nz))
       {
	   cerr << "wrong file size " << filename << endl;
	   cerr << "file contains " << (num_flts -2) << " integers" << endl;
	   cerr << "should contain " << (_params->input_proj.nx * _params->input_proj.ny * _params->input_proj.nz) << " integers" << endl;
	   return false;
       }
       
       fl32 *input_data = new fl32[num_flts];
    
       int num_flts_read = fread(input_data,sizeof(fl32),num_flts,file_pointer);
       if(num_flts_read != num_flts)
       {
	   cerr << "error reading data from file " << filename << endl;
	   cerr << "read " << num_flts_read << " integers " << endl;
	   cerr << "expecting " << num_flts << " integers " << endl;
	   fclose(file_pointer);
	   delete [] input_data;
	   return false;
       }
       
   // close file
       fclose(file_pointer);
       
       int nx = _params->input_proj.nx;
       int ny = _params->input_proj.ny;
       int nz = _params->input_proj.nz;

       switch(_params->data_type)
       {
       case Params::SATELLITE:
       for(int z = 0; z < nz; z++)
       {
	 // skip first and last 4byte float
	 int i = (z * nx * ny) + 1;
	 for( int y = (ny - 1); y >= 0; y-- )
	 {
	   for( int x = 0; x < nx; x++)
	   {
	     data[ (y * nx) + x ] = input_data[i];
	     i++;
	   }
	 }
       }
       break;
       case Params::CLIMATOLOGY:
       for(int z = 0; z < nz; z++)
       {
	 // skip first and last 4byte float
	 int i = (z * nx * ny) + 1;
	 for( int y = 0; y < ny; y++)
	 {
	   for( int x = 0; x < nx; x++)
	   {
	     data[ (y * nx) + x ] = input_data[i];
	     i++;
	   }
	 }
       }
       break;
       }

       delete [] input_data;	       

       int num_bytes;
       
       if(_params->is_bigend) 
       {
	   num_bytes =  BE_to_array_32(data,num_flts * sizeof(fl32));
       }
       else 
       {
	   num_bytes = SE_to_array_32(data,num_flts * sizeof(fl32));
       }
       
       if(_params->ConvertKelvin2Celsius)
       {
	 for(int x = 0; x <  nx * ny ;x++)
	 {
	   data[x] = data[x] - 273.16;
	 }
       }

       return true;
}

