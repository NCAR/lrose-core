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
//   $Date: 2016/03/04 02:22:11 $
//   $Id: MdvCreateRadarCoverageMap.cc,v 1.3 2016/03/04 02:22:11 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvCreateRadarCoverageMap: MdvCreateRadarCoverageMap program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2014
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <string>

#include <os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>

#include "MdvCreateRadarCoverageMap.hh"
#include "Params.hh"

using namespace std;

// Global variables

MdvCreateRadarCoverageMap *MdvCreateRadarCoverageMap::_instance =
     (MdvCreateRadarCoverageMap *)NULL;


/*********************************************************************
 * Constructor
 */

MdvCreateRadarCoverageMap::MdvCreateRadarCoverageMap(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvCreateRadarCoverageMap::MdvCreateRadarCoverageMap()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvCreateRadarCoverageMap *)NULL);
  
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
  char *params_path = new char[strlen("unknown") + 1];
  strcpy(params_path, "unknown");
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    
    okay = false;
    
    return;
  }
}


/*********************************************************************
 * Destructor
 */

MdvCreateRadarCoverageMap::~MdvCreateRadarCoverageMap()
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

MdvCreateRadarCoverageMap *MdvCreateRadarCoverageMap::Inst(int argc, char **argv)
{
  if (_instance == (MdvCreateRadarCoverageMap *)NULL)
    new MdvCreateRadarCoverageMap(argc, argv);
  
  return(_instance);
}

MdvCreateRadarCoverageMap *MdvCreateRadarCoverageMap::Inst()
{
  assert(_instance != (MdvCreateRadarCoverageMap *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvCreateRadarCoverageMap::init()
{
  static const string method_name = "MdvCreateRadarCoverageMap::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void MdvCreateRadarCoverageMap::run()
{
  static const string method_name = "MdvCreateRadarCoverageMap::run()";
  
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_time))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: " << trigger_time << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _addRadar() - Add the current radar to the coverage map using the
 *               given parameters.
 */

void MdvCreateRadarCoverageMap::_addRadar(MdvxField &coverage_field,
					  const double lat, const double lon,
					  const double max_range_km,
					  const double map_value) const
{
  static const string method_name = "MdvCreateRadarCoverageMap::_addRadar()";

  // Get the field projection

  Mdvx::field_header_t field_hdr = coverage_field.getFieldHeader();
  ui08 *coverage_data = (ui08 *)coverage_field.getVol();
  
  MdvxPjg proj(field_hdr);
  
  // Loop through the grid, setting the map value if the grid point is
  // within range of the radar
  // NOTE: Improve effiency by only looking in rectangle around radar.
  // Could really improve efficiency by craeting circle template for area
  // covered by each radar and saving between volumes, checking for changes
  // in the field grid.

  for (int iy = 0; iy < proj.getNy(); ++iy)
  {
    for (int ix = 0; ix < proj.getNx(); ++ix)
    {
      int index = (iy * proj.getNx()) + ix;
      
      double grid_lat, grid_lon;
      
      proj.xyIndex2latlon(ix, iy, grid_lat, grid_lon);
      
      double distance_km, theta;
      
      proj.latlon2RTheta(lat, lon,
			 grid_lat, grid_lon,
			 distance_km, theta);
      
      if (distance_km < max_range_km)
	coverage_data[index] = map_value;
    } /* endfor - ix */
  } /* endfor - iy */
  
}


/*********************************************************************
 * _createCoverageField() - Create the blank coverage field based on
 *                          the given master header.
 *
 * Returns a pointer to the created field on success, 0 on failure.
 */

MdvxField *MdvCreateRadarCoverageMap::_createCoverageField(const Mdvx::field_header_t input_field_hdr) const
{
  static const string method_name = "MdvCreateRadarCoverageMap::_createCoverageField()";

  // Create the field header for the new field

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.nx = input_field_hdr.nx;
  field_hdr.ny = input_field_hdr.ny;
  field_hdr.nz = 1;
  field_hdr.proj_type = input_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_INT8;
  field_hdr.data_element_nbytes = 1;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = input_field_hdr.native_vlevel_type;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.dz_constant = 1;
  field_hdr.data_dimension = 2;
  field_hdr.proj_origin_lat = input_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = input_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; ++i)
    field_hdr.proj_param[i] = input_field_hdr.proj_param[i];
  field_hdr.grid_dx = input_field_hdr.grid_dx;
  field_hdr.grid_dy = input_field_hdr.grid_dy;
  field_hdr.grid_dz = 0.0;
  field_hdr.grid_minx = input_field_hdr.grid_minx;
  field_hdr.grid_miny = input_field_hdr.grid_miny;
  field_hdr.grid_minz = 0.0;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = 0;
  field_hdr.missing_data_value = 0;
  field_hdr.proj_rotation = input_field_hdr.proj_rotation;
  STRcopy(field_hdr.field_name_long, "Coverage Map", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "Cov", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "none", MDV_UNITS_LEN);
  
  // Create the vlevel header for the new field

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vlevel_hdr.level[0] = 0.0;
  
  // Create and return the new field

  return new MdvxField(field_hdr, vlevel_hdr,
		       (void *)0, true);
}

/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool MdvCreateRadarCoverageMap::_initTrigger(void)
{
  static const string method_name = "MdvCreateRadarCoverageMap::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug)
      cerr << "Initializing LATEST_DATA trigger using url: " <<
	_params->input_url << endl;
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_url,
		      -1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger using url: " <<
	_params->input_url << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }

    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::TIME_LIST :
  {
    time_t start_time =
      DateTime::parseDateTime(_params->time_list_trigger.start_time);
    time_t end_time
      = DateTime::parseDateTime(_params->time_list_trigger.end_time);
    
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for TIME_LIST trigger: " <<
	_params->time_list_trigger.start_time << endl;
      
      return false;
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for TIME_LIST trigger: " <<
	_params->time_list_trigger.end_time << endl;
      
      return false;
    }
    
    if (_params->debug)
    {
      cerr << "Initializing TIME_LIST trigger: " << endl;
      cerr << "   url: " << _params->input_url << endl;
      cerr << "   start time: " << DateTime::str(start_time) << endl;
      cerr << "   end time: " << DateTime::str(end_time) << endl;
    }
    
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->input_url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->input_url << endl;
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
	endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _processData() - Process data for the given trigger time.
 *
 * Returns true on success, false on failure.
 */

bool MdvCreateRadarCoverageMap::_processData(const DateTime &trigger_time)
{
  static const string method_name = "MdvCreateRadarCoverageMap::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  // Read in the input field

  DsMdvx mdvx;
  
  if (!_readInputFile(mdvx, trigger_time))
    return false;
  
  // Create the output field

  MdvxField *coverage_field;
  
  if ((coverage_field = _createCoverageField(mdvx.getField(0)->getFieldHeader())) == 0)
    return false;
  
  // Loop through the radars, adding any found in the info string

  string info_string(mdvx.getMasterHeader().data_set_info);
  
  if (_params->verbose)
    cerr << "Info string:" << info_string << endl;
  
  for (int i = 0; i < _params->radar_info_n; ++i)
  {
    if (_params->verbose)
      cerr << "    Looking for: " << _params->_radar_info[i].radar_string << endl;
    
    if (info_string.find(_params->_radar_info[i].radar_string) != string::npos)
    {
      if (_params->verbose)
	cerr << "      FOUND" << endl;

      _addRadar(*coverage_field,
		_params->_radar_info[i].lat, _params->_radar_info[i].lon,
		_params->_radar_info[i].max_range_km,
		_params->_radar_info[i].map_value);
    }
  } /* endfor - i */

  // Write the output file

  if (!_writeOutputFile(*coverage_field, mdvx.getMasterHeader()))
    return false;
  
  // Clean up memory

  delete coverage_field;
  
  return true;
}


/*********************************************************************
 * _readInputFile() - Read the input file for the given time.
 *
 * Returns true on success, false on failure.
 */

bool MdvCreateRadarCoverageMap::_readInputFile(DsMdvx &input_file,
					       const DateTime &trigger_time) const
{
  static const string method_name = "MdvCreateRadarCoverageMap::_readInputFile()";
  
  // Set up the read request.  We go ahead and read the entire field even though
  // we just need the header so that we don't have to dig through all of the headers
  // to find the one we need.

  input_file.clearRead();

  input_file.setReadTime(Mdvx::READ_CLOSEST,
			 _params->input_url,
			 0,
			 trigger_time.utime());

  input_file.addReadField(_params->input_field_name);
  
  // Read the data

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input field from: " << _params->input_url << endl;
    cerr << input_file.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _writeOutputFile() - Create the output file containing the given
 *                      coverage field and write it to the output URL.
 *
 * Returns true on success, false on failure.
 */

bool MdvCreateRadarCoverageMap::_writeOutputFile(const MdvxField &coverage_field,
						 const Mdvx::master_header_t &input_master_hdr) const
{
  static const string method_name = "MdvCreateRadarCoverageMap::_writeOutputFile()";
  
  // Create the master header

  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = input_master_hdr.time_begin;
  master_hdr.time_end = input_master_hdr.time_end;
  master_hdr.time_centroid = input_master_hdr.time_centroid;
  master_hdr.time_expire = input_master_hdr.time_expire;
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr.native_vlevel_type = input_master_hdr.native_vlevel_type;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = input_master_hdr.grid_orientation;
  master_hdr.data_ordering = input_master_hdr.data_ordering;
  master_hdr.sensor_lon = input_master_hdr.sensor_lon;
  master_hdr.sensor_lat = input_master_hdr.sensor_lat;
  master_hdr.sensor_alt = input_master_hdr.sensor_alt;
  STRcopy(master_hdr.data_set_info, input_master_hdr.data_set_info,
	  MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "Radar Coverage Map", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, "MdvCreateRadarCoverageMap", MDV_NAME_LEN);
  
  // Create the output file

  DsMdvx output_file;
  
  output_file.setMasterHeader(master_hdr);
  
  // Add the field

  MdvxField *output_field = new MdvxField(coverage_field);
  output_field->compress(Mdvx::COMPRESSION_RLE);
  
  output_file.addField(output_field);
  
  // Write the file

  if (output_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file to URL: " << _params->output_url << endl;
    cerr << output_file.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}

