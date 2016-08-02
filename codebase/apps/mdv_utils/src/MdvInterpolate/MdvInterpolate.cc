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
//   $Id: MdvInterpolate.cc,v 1.4 2016/03/04 02:22:11 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvInterpolate: MdvInterpolate program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <dsdata/DsIntervalTrigger.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/umisc.h>

#include "MdvInterpolate.hh"
#include "Params.hh"

#include "GenptSpdbInput.hh"

#include "NaturalNeighborInterp.hh"

using namespace std;


// Global variables

MdvInterpolate *MdvInterpolate::_instance =
     (MdvInterpolate *)NULL;


/*********************************************************************
 * Constructor
 */

MdvInterpolate::MdvInterpolate(int argc, char **argv) :
  _dataTrigger(0),
  _inputHandler(0),
  _interpolator(0)
{
  static const string method_name = "MdvInterpolate::MdvInterpolate()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvInterpolate *)NULL);
  
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
  char *params_path = "unknown";
  
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

MdvInterpolate::~MdvInterpolate()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  delete _inputHandler;
  delete _interpolator;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

MdvInterpolate *MdvInterpolate::Inst(int argc, char **argv)
{
  if (_instance == (MdvInterpolate *)NULL)
    new MdvInterpolate(argc, argv);
  
  return(_instance);
}

MdvInterpolate *MdvInterpolate::Inst()
{
  assert(_instance != (MdvInterpolate *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvInterpolate::init()
{
  static const string method_name = "MdvInterpolate::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::INTERVAL_REALTIME :
  {
    if (_params->debug)
      cerr << "Initializing INTERVAL_REALTIME trigger" << endl;
    
    DsIntervalTrigger *trigger = new DsIntervalTrigger();
    if (trigger->init(_params->interval_realtime_trigger.interval_secs,
		      _params->interval_realtime_trigger.interval_start_secs,
		      1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INTERVAL_REALTIME trigger" << endl;
      
      return false;
    }

    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::INTERVAL_ARCHIVE :
  {
    if (_params->debug)
      cerr << "Initializing INTERVAL_ARCHIVE trigger" << endl;
    
    time_t start_time =
      DateTime::parseDateTime(_params->interval_archive_trigger.start_time);
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start time string: "
	   << _params->interval_archive_trigger.start_time << endl;
      cerr << "Cannot initialize INTERVAL_ARCHIVE trigger" << endl;
      
      return false;
    }
    
    time_t end_time =
      DateTime::parseDateTime(_params->interval_archive_trigger.end_time);
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end time string: "
	   << _params->interval_archive_trigger.end_time << endl;
      cerr << "Cannot initialize INTERVAL_ARCHIVE trigger" << endl;
      
      return false;
    }
    
    DsIntervalTrigger *trigger = new DsIntervalTrigger();
    if (trigger->init(_params->interval_archive_trigger.interval_secs,
		      start_time,
		      end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INTERVAL_ARCHIVE trigger" << endl;
      
      return false;
    }

    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

  // Create the input object

  switch (_params->input_dataset_type)
  {
  case Params::GENPT_SPDB_DATASET :
    _inputHandler =
      new GenptSpdbInput(_params->genpt_spdb_dataset_params.spdb_url,
			 _params->genpt_spdb_dataset_params.data_field_name,
			 _params->debug);
    break;
    
  } /* endswitch - _params->input_dataset_type */
  
  // Create the interpolator object

  switch (_params->interpolation_type)
  {
  case Params::NATURAL_NEIGHBOR_INTERP :
    _interpolator = new NaturalNeighborInterp(_params->debug);
    break;
  } /* endswitch - _params->interpolation_type */
  
  // Initialize the output projection

  switch (_params->output_proj.proj_type)
  {
  case Params::PROJ_LATLON :
    _outputProjection.initLatlon(_params->output_proj.nx,
				 _params->output_proj.ny,
				 1,
				 _params->output_proj.dx,
				 _params->output_proj.dy,
				 1.0,
				 _params->output_proj.minx,
				 _params->output_proj.miny,
				 0.0);
    break;
    
  case Params::PROJ_FLAT :
    _outputProjection.initFlat(_params->output_proj.origin_lat,
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
			       0.0);
    break;
    
  case Params::PROJ_LC :
    _outputProjection.initLc2(_params->output_proj.origin_lat,
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
			      0.0);
    break;
    
  } /* endswitch - _params->output_proj.proj_type */
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void MdvInterpolate::run()
{
  static const string method_name = "MdvInterpolate::run()";
  
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
 * _createOutputField() - Create the blank output field.
 */

MdvxField *MdvInterpolate::_createOutputField(const time_t data_time)
{
  // Create the field header

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.forecast_time = data_time;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.dz_constant = 1;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = _inputHandler->getBadDataValue();
  field_hdr.missing_data_value = _inputHandler->getMissingDataValue();
  STRcopy(field_hdr.field_name_long,
	  _inputHandler->getFieldNameLong().c_str(), MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, _inputHandler->getFieldName().c_str(),
	  MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, _inputHandler->getFieldUnits().c_str(),
	  MDV_UNITS_LEN);
  
  _outputProjection.syncToFieldHdr(field_hdr);
  
  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  // Create the return field

  return new MdvxField(field_hdr, vlevel_hdr,
		       (void *)NULL, true);
}


/*********************************************************************
 * _processData() - Process the data for the given time.
 */

bool MdvInterpolate::_processData(const DateTime &trigger_time)
{
  static const string method_name = "MdvInterpolate::_processData()";
  
  if (_params->debug)
    cerr << endl
	 << "*** Processing data for time: " << trigger_time << endl;
  
  // Read in the input data

  time_t begin_time = trigger_time.utime() - _params->input_begin_secs;
  time_t end_time = trigger_time.utime() + _params->input_end_secs;
  
  vector< DataPoint > points =
    _inputHandler->getData(begin_time, end_time);
  
  // Interpolate the data values into the output grid

  MdvxField *output_field = _createOutputField(trigger_time.utime());
  
  if (!_interpolator->interpolate(points, *output_field))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error interpolating data" << endl;
    
    delete output_field;
    
    return false;
  }
  
  // Write the interpolated grid to the output file

  DsMdvx output_mdvx;
  
  _setMasterHeader(output_mdvx,
		   begin_time, trigger_time.utime(), end_time);
  
  output_field->convertType(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_BZIP,
			    Mdvx::SCALING_DYNAMIC);
  
  output_mdvx.addField(output_field);
  
  if (output_mdvx.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing MDV file to output URL: "
	 << _params->output_url << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _setMasterHeader() - Set the values for the output master header.
 */

void MdvInterpolate::_setMasterHeader(DsMdvx &mdvx,
				      const time_t time_begin,
				      const time_t time_centroid,
				      const time_t time_end) const
{
  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = time_begin;
  master_hdr.time_end = time_end;
  master_hdr.time_centroid = time_centroid;
  master_hdr.time_expire = master_hdr.time_end;
  master_hdr.num_data_times = 1;
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  STRcopy(master_hdr.data_set_info,
	  "Created by MdvInterpolate", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "MdvInterpolate", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source,
	  _inputHandler->getDataSource().c_str(), MDV_NAME_LEN);
  
  mdvx.setMasterHeader(master_hdr);
}
