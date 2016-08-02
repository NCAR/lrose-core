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
 * @file QPESumBinary2Mdv.cc
 *
 * @class QPESumBinary2Mdv
 *
 * QPESumBinary2Mdv is the top level application class.
 *  
 * @date 5/7/2012
 *
 */

#include <cassert>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <zlib.h>
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

#include "QPESumBinary2Mdv.hh"

using namespace std;

// Global variables

QPESumBinary2Mdv *QPESumBinary2Mdv::_instance = (QPESumBinary2Mdv *)NULL;


/*********************************************************************
 * Constructor
 */

QPESumBinary2Mdv::QPESumBinary2Mdv(int argc, char **argv)
{
  static const string method_name = "QPESumBinary2Mdv::QPESumBinary2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (QPESumBinary2Mdv *)NULL);
  
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

QPESumBinary2Mdv::~QPESumBinary2Mdv()
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

QPESumBinary2Mdv *QPESumBinary2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (QPESumBinary2Mdv *)NULL)
    new QPESumBinary2Mdv(argc, argv);
  
  return(_instance);
}

QPESumBinary2Mdv *QPESumBinary2Mdv::Inst()
{
  assert(_instance != (QPESumBinary2Mdv *)NULL);
  
  return(_instance);
}

/*********************************************************************
 * init()
 */

bool QPESumBinary2Mdv::init()
{
  static const string method_name = "QPESumBinary2Mdv::init()";
  
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

void QPESumBinary2Mdv::run()
{

 static const string method_name = "QPESumBinary2Mdv::run()";
  
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

DateTime QPESumBinary2Mdv::_getDataTime(const string &input_file_name)
{
    Path file_path(input_file_name);
    string file_base = file_path.getBase();
    string file_ext = file_path.getExt();

    DateTime return_date;
    switch(_params->data_time_location)
    {
    case Params::PRECIP_FILE_NAME:
    return_date.setYear(atoi(file_base.substr(17,4).c_str()));
    return_date.setMonth(atoi(file_base.substr(21,2).c_str()));
    return_date.setDay(atoi(file_base.substr(23,2).c_str()));
    return_date.setHour(atoi(file_ext.substr(0,2).c_str()));
    return_date.setMin(atoi(file_ext.substr(2,2).c_str()));
    return_date.setSec(0);
    break;
    }

    return return_date;
}


////////////////////////////////////////////////////////////////
// _writeMDV() - writes Data to output MDV file
///

int QPESumBinary2Mdv::_writeMDV(const string &input_file_name, fl32 *data)
{
  static const string method_name = "QPESumBinary2Mdv::writeMDV()";
  
  DsMdvx output_file;

  DateTime data_time;

  if(_params->data_time_location == Params::HEADER)
  {
    data_time.setYear(_year);
    data_time.setMonth(_month);
    data_time.setDay(_day);
    data_time.setHour(_hour);
    data_time.setMin(_minute);
    data_time.setSec(_second);
  }
  else // get time from the file name
  {
    data_time = _getDataTime(input_file_name);
  }
  
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
  
  STRcopy(master_hdr.data_set_info, "Output from QPESumBinary2Mdv", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "QPESumBinary2Mdv", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, input_file_name.c_str(),
          MDV_NAME_LEN);

  output_file.setMasterHeader(master_hdr);
  

  // Construct the field header

  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));

  field_hdr.forecast_time = master_hdr.time_centroid;
  field_hdr.nx = _nx;
  field_hdr.ny = _ny;
  field_hdr.nz = _nz;

  // For now going to assume latlon projection
  // Can use the projection field from the header
  // but so far all the data is LL.
  field_hdr.proj_type = Mdvx::PROJ_LATLON;

  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz * field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  field_hdr.dz_constant = 1;
  field_hdr.proj_origin_lat = _miny;
  field_hdr.proj_origin_lon = _minx;
  field_hdr.grid_dx = _dx;
  field_hdr.grid_dy = _dy;
  field_hdr.grid_dz = _dz;
  field_hdr.grid_minx = _minx;
  field_hdr.grid_miny = _miny;
  field_hdr.grid_minz = _minz;
  field_hdr.scale = _params->scaling_info.scale;
  field_hdr.bias =  _params->scaling_info.bias;
  field_hdr.bad_data_value = _missingDataValue;
  field_hdr.missing_data_value = _missingDataValue;
  //  field_hdr.proj_rotation = _params->input_proj.rotation;
  STRcopy(field_hdr.field_name_long,_field_name_long.c_str(),MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name,_field_name.c_str(),MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units,_units.c_str(),MDV_UNITS_LEN);
  
  // Construct the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));

  // May need to revisit when/if we ingest 3D data
  for (int i = 0; i < _nz; i++)
  {
    if( _nz == 1)
    {
      vlevel_hdr.type[i] = Mdvx::VERT_TYPE_SURFACE;
    }
    else 
    {
      vlevel_hdr.type[i] = Mdvx::VERT_TYPE_Z;
    }
    vlevel_hdr.level[i] = _minz + ( i * _dz );
  }
  
  MdvxField *output_field;

  if ((output_field =
       new MdvxField(field_hdr,vlevel_hdr,(void *)data)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output field" << endl;
    
    return -1;
  }

  if( _params->use_scaling_info )
  {
    double output_scale = _params->scaling_info.scale;
    double output_bias  = _params->scaling_info.bias;

    output_field->convertType(Mdvx::ENCODING_INT8,
			      Mdvx::COMPRESSION_BZIP,
			      Mdvx::SCALING_SPECIFIED,
			      output_scale,
			      output_bias);
  }
  else // set output as float's
  {
    output_field->convertType(Mdvx::ENCODING_FLOAT32,
			      Mdvx::COMPRESSION_GZIP);
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

  if (_params->debug_level > Params::DEBUG_OFF)
  {
    cerr << "Writing output for time " << data_time << endl;
  }
  
  return(0);
}



////////////////////////////////////////////////////////////////
// processData() - convert file to MDV format
///

bool QPESumBinary2Mdv::_processData(const string &file_path)
{

  PMU_auto_register("Reading File");

  cerr << "*** Processing file: " << file_path << endl;
  
  fl32 * data = new fl32[921*881*1];

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

bool QPESumBinary2Mdv::_readFile(const string &filename, fl32 *data)
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
       
    si32 *header_input = new si32[10];
    char projection[5];
        
    int num_pts_read = fread(header_input,sizeof(si32),9,file_pointer);
    _year = header_input[0];
    _month = header_input[1];
    _day = header_input[2];
    _hour = header_input[3];
    _minute = header_input[4];
    _second = header_input[5];
    _nx = header_input[6];
    _ny = header_input[7];
    _nz = header_input[8];
    if(_nz == 1)
    {
      _dz = 1;
    }
    
    if (_params->debug_level == Params::DEBUG_EXTRA)
    {
      cerr << "_year = " << _year << endl;
      cerr << "_month = " << _month << endl;
      cerr << "_day = " << _day << endl;
      cerr << "_hour = " << _hour << endl;
      cerr << "_minute = " << _minute << endl;
      cerr << "_second = " << _second << endl;
      cerr << "_nx = " << _nx << endl;
      cerr << "_ny = " << _ny << endl;
      cerr << "_nz = " << _nz << endl;
    }
    
    num_pts_read = fread(&projection,sizeof(ui08),4,file_pointer);

    if (_params->debug_level == Params::DEBUG_EXTRA)
    {
      cerr << "projection = " << projection << endl;
    }
    
    int map_scale, nw_lon1, nw_lat1, xy_scale, dx1, dy1, dxy_scale;
    num_pts_read =  fread(header_input,sizeof(si32),10,file_pointer);
    map_scale = header_input[0];
    nw_lon1 = header_input[4];
    float nw_lon = float(nw_lon1) / float(map_scale);

    nw_lat1 = header_input[5];
    float nw_lat = float(nw_lat1) / float(map_scale);

    xy_scale = header_input[6];
    dx1 = header_input[7];
    dy1 = header_input[8];
    dxy_scale = header_input[9];

    _dx = float(dx1) / float(dxy_scale);
    _dy = float(dy1) / float(dxy_scale);
    _minx = nw_lon;
    _miny = nw_lat - ( (_ny -1) * _dy);
    
    if (_params->debug_level == Params::DEBUG_EXTRA)
    {
      cerr << "map_scale = " << map_scale << endl;
      cerr << "nw_lon = " << nw_lon << endl;
      cerr << "nw_lat = " << nw_lat << endl;
      cerr << "xy_scale = " << xy_scale << endl;
      cerr << "_dx = " << _dx << endl;
      cerr << "_dy = " << _dy << endl;
      cerr << "dxy_scale  = " << dxy_scale << endl;
      cerr << "_minx = " << _minx << endl;
      cerr << "_miny = " << _miny << endl;
    }
    
    // More work to do to ingest 3D mosaic data
    si32 *zhgt = new si32[_nz];
    fl32 *zlevel = new fl32[_nz];
    int z_scale, i_bb_mode;

    num_pts_read =  fread(zhgt,sizeof(si32),_nz,file_pointer);
    num_pts_read =  fread(&z_scale,sizeof(si32),1,file_pointer);
    num_pts_read =  fread(&i_bb_mode,sizeof(si32),1,file_pointer);

    if (_params->debug_level == Params::DEBUG_EXTRA)
    {
      cerr << "z_scale = " << z_scale << endl;
      cerr << "i_bb_mode = " << i_bb_mode << endl;
    }

    for( int i=0; i < _nz; i++)
    {
      zlevel[i] = float(zhgt[i]) / float(z_scale);
      if (_params->debug_level == Params::DEBUG_EXTRA)
      {
	cerr << "zlevel[" << i << "] = " << zlevel[i] << endl;
      }
    }
    _minz = zlevel[0];
    delete zhgt;
    delete zlevel;

    // Future use
    num_pts_read =  fread(header_input,sizeof(si32),9,file_pointer);
    
    // get field name
    num_pts_read = fread(&_varname,sizeof(ui08),20,file_pointer);
    if (_params->debug_level == Params::DEBUG_EXTRA)
    {
      cerr << "_varname = " << _varname << endl;
    }
    string varname = string(_varname);
    if( varname == "composite refl")
      _field_name = "crefl";
    else
      _field_name = _varname;

    if(_params->set_field_name)
    {
      _field_name = _params->data_field_name;
      _field_name_long = _params->data_field_name_long;
    }
    else
    {
      // _field_name alread set in previous if statement
      _field_name_long = _varname;
    }

    // get field units
    num_pts_read = fread(&_varunit,sizeof(ui08),6,file_pointer);
    if (_params->debug_level == Params::DEBUG_EXTRA)
    {
      cerr << "_varunit = " << _varunit << endl;
    }
    if (_params->set_data_units)
    {
      _units = _params->data_units;
    }
    else
    {
      _units = string(_varunit);
    }

    si32 var_scale, nradars, imissing;
    num_pts_read =  fread(&var_scale,sizeof(si32),1,file_pointer);
    num_pts_read =  fread(&imissing,sizeof(si32),1,file_pointer);
    _missingDataValue = float(imissing);

    num_pts_read =  fread(&nradars,sizeof(si32),1,file_pointer);

    if (_params->debug_level == Params::DEBUG_EXTRA)
    {
      cerr << "var_scale = " << var_scale << endl;
      cerr << "_missingDataValue = " << _missingDataValue << endl;
      cerr << "nradars = " << nradars << endl;
    }

    char radarnames[nradars][4];
    for( int i=0; i<nradars; i++)
    {
      num_pts_read = fread(&radarnames[i],4 * sizeof(ui08),1,file_pointer);
      radarnames[i][4] = '\0';
      if (_params->debug_level == Params::DEBUG_EXTRA)
      {
	cerr << "radarname = " << radarnames[i] << endl;
      }
    }

    // check to see if file size is what we expect it to be
    int header_size =( sizeof(si32) * (33 + _nz)) + (30 * sizeof(ui08)) + (4 * sizeof(ui08) * nradars);
    int expected_data_size = _nx * _ny * _nz * sizeof(ui16);
    int expected_file_size = header_size + expected_data_size;
    if( expected_file_size != stat_info.st_size)
    {
      cerr << "wrong file size " << filename << endl;
      cerr << "file size = " << stat_info.st_size << " bite's" << endl;
      cerr << "Expected file size = " << expected_file_size << " bite's" << endl;
      return false;
    }
    delete header_input;

    // fill input data array
    ui16 *input_data =   new ui16[_nx*_ny*_nz];
    num_pts_read = fread(input_data,sizeof(ui16),_nx*_ny*_nz,file_pointer);	      
       
    // close file
    fclose(file_pointer);
       
    for( int i = 0; i < (_nx *_ny *_nz); i++)
    {
      data[i] = fl32(input_data[i]) / fl32(var_scale);
      // I believe values in the 5000's represents areas that
      // are in the scanning range of the radar that are missing.
      // Values in the 6000's represents values that are in
      // the domain but outside the radar scanning range.
      if( data[i] > 5000)
      {
	data[i] = _missingDataValue;
      }
    }

    delete [] input_data;	       

    return true;
}

