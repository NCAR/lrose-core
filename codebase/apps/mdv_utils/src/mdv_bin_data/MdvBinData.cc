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
//   $Date: 2016/03/04 02:22:15 $
//   $Id: MdvBinData.cc,v 1.8 2016/03/04 02:22:15 dixon Exp $
//   $Revision: 1.8 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvBinData.cc: Program to convert "continuous" float data into
 *                binned byte data.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2000
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <signal.h>

#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Params.hh"
#include "ArchiveFileRetriever.hh"
#include "MdvBinData.hh"
#include "RealtimeFileRetriever.hh"
using namespace std;

// Global variables

MdvBinData *MdvBinData::_instance = (MdvBinData *)NULL;

// Global constants

const int FOREVER = true;


/*********************************************************************
 * Constructor
 */

MdvBinData::MdvBinData(int argc, char **argv)
{
  static const char *routine_name = "Constructor";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvBinData *)NULL);
  
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

  // Create the file retriever object

  switch (_params->mode)
  {
  case Params::ARCHIVE :
  {
    time_t start_time = _args->getStartTime();
    time_t end_time = _args->getEndTime();
    
    if (start_time == 0 || end_time == 0)
    {
      cerr << "ERROR: " << _className() << "::" << routine_name << endl;
      cerr << "ARCHIVE mode requires -starttime and -endtime on the command line." <<
	endl;

      exit(-1);
    }
    
    _fileRetriever = new ArchiveFileRetriever(_params->input_url,
					      start_time,
					      end_time,
					      _params->debug);
    break;
  }
  
  case Params::REALTIME :
    _fileRetriever = new RealtimeFileRetriever(_params->input_url,
					       _params->sleep_interval,
					       _params->debug);
    break;
  }
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);
}


/*********************************************************************
 * Destructor
 */

MdvBinData::~MdvBinData()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _fileRetriever;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

MdvBinData *MdvBinData::Inst(int argc, char **argv)
{
  if (_instance == (MdvBinData *)NULL)
    new MdvBinData(argc, argv);
  
  return(_instance);
}

MdvBinData *MdvBinData::Inst()
{
  assert(_instance != (MdvBinData *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run()
 */

void MdvBinData::run()
{
  // Process files forever

  while(true)
  {
    PMU_auto_register("Waiting for data");
    
    // Get the next file to process

    DsMdvx *input_file;
    
    if ((input_file = _fileRetriever->next()) == 0)
    {
      if (_params->mode == Params::ARCHIVE)
	break;
      
      continue;
    }
    
    PMU_auto_register("Processing a file");
    
    // Process the file

    _processFile(*input_file);
    
    // Release memory

    delete input_file;
    
    // Sleep for a second

    sleep(1);
    
  } /* endwhile - true */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createOutputField() - Create an output field based on the given
 *                        input field.
 */

MdvxField *MdvBinData::_createOutputField(const MdvxField& input_field)
{
  // Create the field header

  Mdvx::field_header_t field_hdr;
  Mdvx::field_header_t input_field_hdr = input_field.getFieldHeader();
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.field_code = input_field_hdr.field_code;
  field_hdr.user_time1 = input_field_hdr.user_time1;
  field_hdr.forecast_delta = input_field_hdr.forecast_delta;
  field_hdr.user_time2 = input_field_hdr.user_time2;
  field_hdr.user_time3 = input_field_hdr.user_time3;
  field_hdr.forecast_time = input_field_hdr.forecast_time;
  field_hdr.user_time4 = input_field_hdr.user_time4;
  field_hdr.nx = input_field_hdr.nx;
  field_hdr.ny = input_field_hdr.ny;
  field_hdr.nz = input_field_hdr.nz;
  field_hdr.proj_type = input_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_INT8;
  field_hdr.data_element_nbytes = 1;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;

  for (int i = 0; i < 10; i++)
    field_hdr.user_data_si32[i] = input_field_hdr.user_data_si32[i];
  
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = input_field_hdr.native_vlevel_type;
  field_hdr.vlevel_type = input_field_hdr.vlevel_type;
  field_hdr.dz_constant = input_field_hdr.dz_constant;
  
  field_hdr.proj_origin_lat = input_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = input_field_hdr.proj_origin_lon;

  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; ++i)
    field_hdr.proj_param[i] = input_field_hdr.proj_param[i];
  
  field_hdr.vert_reference = input_field_hdr.vert_reference;
  field_hdr.grid_dx = input_field_hdr.grid_dx;
  field_hdr.grid_dy = input_field_hdr.grid_dy;
  field_hdr.grid_dz = input_field_hdr.grid_dz;
  field_hdr.grid_minx = input_field_hdr.grid_minx;
  field_hdr.grid_miny = input_field_hdr.grid_miny;
  field_hdr.grid_minz = input_field_hdr.grid_minz;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = _params->bad_data_value;
  field_hdr.missing_data_value = _params->missing_data_value;
  field_hdr.proj_rotation = input_field_hdr.proj_rotation;
  
  for (int i = 0; i < 4; ++i)
    field_hdr.user_data_fl32[i] = input_field_hdr.user_data_fl32[i];
  
  STRcopy(field_hdr.field_name_long, input_field_hdr.field_name_long,
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, input_field_hdr.field_name,
	  MDV_SHORT_FIELD_LEN);
  field_hdr.units[0] = '\0';
  STRcopy(field_hdr.transform, "binned", MDV_TRANSFORM_LEN);
  
  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  Mdvx::vlevel_header_t input_vlevel_hdr = input_field.getVlevelHeader();
  
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  for (int i = 0; i < MDV_MAX_VLEVELS; ++i)
  {
    vlevel_hdr.type[i] = input_vlevel_hdr.type[i];
    vlevel_hdr.level[i] = input_vlevel_hdr.level[i];
  } /* endfor - i */
  
  // Create the output field

  MdvxField *output_field = new MdvxField(field_hdr, vlevel_hdr);
  
  // Set the binned data values in the output field

  fl32 *input_data = (fl32 *)input_field.getVol();
  ui08 *output_data = (ui08 *)output_field->getVol();
  
  for (int i = 0; i < field_hdr.nx * field_hdr.ny * field_hdr.nz; ++i)
  {
    // Check for bad or missing data in the input file

    if (input_data[i] == input_field_hdr.bad_data_value)
    {
      output_data[i] = _params->bad_data_value;
      continue;
    }
    
    if (input_data[i] == input_field_hdr.missing_data_value)
    {
      output_data[i] = _params->missing_data_value;
      continue;
    }
    
    // Initialize the output value to a bad data value

    output_data[i] = _params->bad_data_value;
    
    // See if we can bin this data value

    for (int bin = 0; bin < _params->bin_table_n; ++bin)
    {
      if (input_data[i] >= _params->_bin_table[bin].data_start &&
	  input_data[i] < _params->_bin_table[bin].data_end)
      {
	output_data[i] = _params->_bin_table[bin].bin_value;
	break;
      }
    } /* endfor - bin */
    
  } /* endfor - i */
  
  return output_field;
  
}


/*********************************************************************
 * _processFile() - Process the given input file.
 */

void MdvBinData::_processFile(const DsMdvx& input_file)
{
  // Create the output file

  DsMdvx output_file;
  
  _setMasterHeaderFields(input_file.getMasterHeader(),
			 input_file.getReadPath(), output_file);
  

  // Process each field in the input file and put it in the output
  // file.

  for (int i = 0; i < input_file.getMasterHeader().n_fields; ++i)
  {
    MdvxField *input_field = input_file.getFieldByNum(i);
    
    MdvxField *output_field = _createOutputField(*input_field);
    
    output_field->convertType(Mdvx::ENCODING_INT8,
			      Mdvx::COMPRESSION_RLE,
			      Mdvx::SCALING_NONE);
    
    output_file.addField(output_field);
    
  } /* endfor - i */
  
  // Write the output file.

  output_file.setWriteLdataInfo();
  output_file.writeToDir(_params->output_url);
  
}


/*********************************************************************
 * _setMasterHeaderFields() - Set the master header fields in the
 *                            output file based on those in the input
 *                            file.
 */

void MdvBinData::_setMasterHeaderFields(const Mdvx::master_header_t& input_hdr,
					const string& data_set_source,
					DsMdvx& output_file)
{
  Mdvx::master_header_t output_hdr;
  
  memset(&output_hdr, 0, sizeof(output_hdr));
  
  output_hdr.time_gen = time((time_t *)NULL);
  output_hdr.user_time = input_hdr.user_time;
  output_hdr.time_begin = input_hdr.time_begin;
  output_hdr.time_end = input_hdr.time_end;
  output_hdr.time_centroid = input_hdr.time_centroid;
  output_hdr.time_expire = input_hdr.time_expire;
  output_hdr.num_data_times = 0;
  output_hdr.index_number = 0;
  output_hdr.data_dimension = input_hdr.data_dimension;
  output_hdr.data_collection_type = input_hdr.data_collection_type;
  output_hdr.user_data = input_hdr.user_data;
  output_hdr.native_vlevel_type = input_hdr.native_vlevel_type;
  output_hdr.vlevel_type = input_hdr.vlevel_type;
  output_hdr.vlevel_included = input_hdr.vlevel_included;
  output_hdr.grid_orientation = input_hdr.grid_orientation;
  output_hdr.data_ordering = input_hdr.data_ordering;
  output_hdr.n_fields = 0;
  output_hdr.max_nx = input_hdr.max_nx;
  output_hdr.max_ny = input_hdr.max_ny;
  output_hdr.max_nz = input_hdr.max_nz;
  output_hdr.n_chunks = 0;
  output_hdr.field_grids_differ = input_hdr.field_grids_differ;
  
  for (int i = 0; i < 8; ++i)
    output_hdr.user_data_si32[i] = input_hdr.user_data_si32[i];
  
  for (int i = 0; i < 6; ++i)
    output_hdr.user_data_fl32[i] = input_hdr.user_data_fl32[i];
  
  output_hdr.sensor_lon = input_hdr.sensor_lon;
  output_hdr.sensor_lat = input_hdr.sensor_lat;
  output_hdr.sensor_alt = input_hdr.sensor_alt;
  
  STRcopy(output_hdr.data_set_info,
	  "mdv_bin_data output", MDV_INFO_LEN);
  STRcopy(output_hdr.data_set_name,
	  "binned data", MDV_NAME_LEN);
  STRcopy(output_hdr.data_set_source, data_set_source.c_str(), MDV_NAME_LEN);
  
  output_file.setMasterHeader(output_hdr);
  
}
