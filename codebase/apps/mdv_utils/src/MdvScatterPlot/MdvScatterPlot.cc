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
//   $Date: 2016/03/04 02:22:12 $
//   $Id: MdvScatterPlot.cc,v 1.11 2016/03/04 02:22:12 dixon Exp $
//   $Revision: 1.11 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvScatterPlot: MdvScatterPlot program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>

#include "MdvScatterPlot.hh"
#include "Params.hh"

#include "AsciiTablePlotter.hh"
#include "NetcdfPlotter.hh"

using namespace std;

// Global variables

MdvScatterPlot *MdvScatterPlot::_instance =
     (MdvScatterPlot *)NULL;


/*********************************************************************
 * Constructor
 */

MdvScatterPlot::MdvScatterPlot(int argc, char **argv) :
  _dataTrigger(0),
  _plotter(0)
{
  static const string method_name = "MdvScatterPlot::MdvScatterPlot()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvScatterPlot *)NULL);
  
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

MdvScatterPlot::~MdvScatterPlot()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  delete _plotter;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

MdvScatterPlot *MdvScatterPlot::Inst(int argc, char **argv)
{
  if (_instance == (MdvScatterPlot *)NULL)
    new MdvScatterPlot(argc, argv);
  
  return(_instance);
}

MdvScatterPlot *MdvScatterPlot::Inst()
{
  assert(_instance != (MdvScatterPlot *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvScatterPlot::init()
{
  static const string method_name = "MdvScatterPlot::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the input projection

  if (!_initInputProj())
    return false;
  
  // Initialize the plotter object

  if (!_initPlotter())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void MdvScatterPlot::run()
{
  static const string method_name = "MdvScatterPlot::run()";
  
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
 * _applyThresholds() - Apply the given thresholds to the given data
 *                      field.
 */

void MdvScatterPlot::_applyThresholds(MdvxField *field,
				      const bool apply_min_thresh,
				      const double min_thresh,
				      const bool apply_max_thresh,
				      const double max_thresh) const
{
  // Check to make sure we have a field to process

  if (!field)
    return;
  
  // Check to see if thresholding was requested

  if (!(apply_min_thresh || apply_max_thresh))
    return;
  
  // Apply the requested thresholds to each data value

  Mdvx::field_header_t field_hdr = field->getFieldHeader();
  fl32 *data = (fl32 *)field->getVol();
  int volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz;
  
  for (int i = 0; i < volume_size; ++i)
  {
    if (data[i] == field_hdr.bad_data_value ||
	data[i] == field_hdr.missing_data_value)
      continue;
    
    if (apply_min_thresh && data[i] < min_thresh)
      data[i] = field_hdr.bad_data_value;
    
    if (apply_max_thresh && data[i] > max_thresh)
      data[i] = field_hdr.bad_data_value;
    
  } /* endfor - i */
}


/*********************************************************************
 * _getMdvReadSearchMode() - Convert the read search mode given in the
 *                           parameter file to the matching MDV value.
 *
 * Returns the matching MDV read search mode,
 */

Mdvx::read_search_mode_t MdvScatterPlot::_getMdvReadSearchMode(const Params::read_search_mode_t search_mode) const
{
  switch (search_mode)
  {
  case Params::READ_LAST :
    return Mdvx::READ_LAST;
    
  case Params::READ_CLOSEST :
    return Mdvx::READ_CLOSEST;
    
  case Params::READ_FIRST_BEFORE :
    return Mdvx::READ_FIRST_BEFORE;
    
  case Params::READ_FIRST_AFTER :
    return Mdvx::READ_FIRST_AFTER;
    
  case Params::READ_BEST_FORECAST :
    return Mdvx::READ_BEST_FORECAST;
    
  case Params::READ_SPECIFIED_FORECAST :
    return Mdvx::READ_SPECIFIED_FORECAST;
  }
  
  // We should never get here

  return Mdvx::READ_FIRST_BEFORE;
}


/*********************************************************************
 * _initInputProj() - Initialize the input projection.
 *
 * Returns true on success, false on failure.
 */

bool MdvScatterPlot::_initInputProj(void)
{
  static const string method_name = "MdvScatterPlot::_initInputProj()";
  
  switch (_params->proj_info.proj_type)
  {
  case Params::PROJ_LATLON :
    _inputProj.initLatlon(_params->proj_info.nx,
			  _params->proj_info.ny,
			  1,
			  _params->proj_info.dx,
			  _params->proj_info.dy,
			  1.0,
			  _params->proj_info.minx,
			  _params->proj_info.miny,
			  0.0);
    break;
  
  case Params::PROJ_FLAT :
    _inputProj.initFlat(_params->proj_info.origin_lat,
			_params->proj_info.origin_lon,
			_params->proj_info.rotation,
			_params->proj_info.nx,
			_params->proj_info.ny,
			1,
			_params->proj_info.dx,
			_params->proj_info.dy,
			1.0,
			_params->proj_info.minx,
			_params->proj_info.miny,
			0.0);
    break;
  
  case Params::PROJ_LAMBERT_CONFORMAL2 :
    _inputProj.initLc2(_params->proj_info.origin_lat,
		       _params->proj_info.origin_lon,
		       _params->proj_info.lat1,
		       _params->proj_info.lat2,
		       _params->proj_info.nx,
		       _params->proj_info.ny,
		       1,
		       _params->proj_info.dx,
		       _params->proj_info.dy,
		       1.0,
		       _params->proj_info.minx,
		       _params->proj_info.miny,
		       0.0);
    break;
    
  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _initPlotter() - Initialize the object that creates the scatter plots.
 *
 * Returns true on success, false on failure.
 */

bool MdvScatterPlot::_initPlotter(void)
{
  static const string method_name = "MdvScatterPlot::_initPlotter()";
  
  switch (_params->plotter_type)
  {
  case Params::PLOT_ASCII_TABLE :
    _plotter = new AsciiTablePlotter(_params->plot_ascii_table.delimiter,
				     _params->plot_ascii_table.include_header,
				     _params->output_dir,
				     _params->output_ext,
				     _params->_input_fields[0].output_field_name,
				     _params->_input_fields[1].output_field_name,
				     _params->debug || _params->verbose);
    break;
    
  case Params::PLOT_NETCDF_FILE :
    _plotter = new NetcdfPlotter(_params->output_dir,
				 _params->output_ext,
				 _params->_input_fields[0].output_field_name,
				 _params->_input_fields[1].output_field_name,
				 _params->debug || _params->verbose);
    break;
    
  } /* endswitch - _params->plotter_type */
  
  return true;
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool MdvScatterPlot::_initTrigger(void)
{
  static const string method_name = "MdvScatterPlot::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug)
      cerr << "Initializing LATEST_DATA trigger using url: " <<
	_params->trigger_url << endl;
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->trigger_url,
		      -1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger using url: " <<
	_params->trigger_url << endl;
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
      cerr << "   url: " << _params->trigger_url << endl;
      cerr << "   start time: " << DateTime::str(start_time) << endl;
      cerr << "   end time: " << DateTime::str(end_time) << endl;
    }
    
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->trigger_url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->trigger_url << endl;
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
 * _performRead() - Perform the read.  This method just consolidates
 *                  the parts of the reading of the input fields that
 *                  is common between fields.
 *
 * Returns the newly read field on success, 0 on failure.
 */

MdvxField *MdvScatterPlot::_performRead(DsMdvx &input_file,
					const string &url,
					const DateTime &trigger_time,
					const int max_input_secs,
					const Mdvx::read_search_mode_t search_mode,
					const int forecast_lead_secs,
					const bool process_single_level,
					const int level_num) const
{
  static const string method_name = "MdvScatterPlot::_performRead()";
  
  // Finish setting up the read request

  input_file.setReadTime(search_mode,
			 url,
			 max_input_secs,
			 trigger_time.utime(),
			 forecast_lead_secs);
  
  if (process_single_level)
  {
    if (level_num < 0)
      input_file.setReadComposite();
    else
      input_file.setReadPlaneNumLimits(level_num, level_num);
  }
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  input_file.setReadRemap(_inputProj);
  
  // Read the data

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input volume from: " << url << endl;
    cerr << input_file.getErrStr() << endl;
    
    return 0;
  }
  
  return new MdvxField(*(input_file.getField(0)));
}


/*********************************************************************
 * _processData() - Process data for the given trigger time.
 *
 * Returns true on success, false on failure.
 */

bool MdvScatterPlot::_processData(const DateTime &trigger_time)
{
  static const string method_name = "MdvScatterPlot::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  if (_params->use_hour_bounds)
    {
      DateTime processTime(trigger_time);
      int hour = processTime.getHour();
      if ( hour < _params->start_hour  ||   hour >= _params->end_hour)
	{
	  if (_params->debug)
	    {
	      cerr << "MdvScatterPlot::_processData(): Skipping processing of " 
		   << processTime.getW3cStr() << ". Hour falls outside of hour "
		   << "bounds.\n" << endl;
	    }
	  return true;
	}
    }

  // Read in the two input fields

  MdvxField *field_x;
  MdvxField *field_y;
  
  if (_params->_input_fields[0].use_field_name)
    field_x = _readInputField(_params->_input_fields[0].url,
			      _params->_input_fields[0].field_name,
			      trigger_time,
			      _params->_input_fields[0].max_input_secs,
			      _getMdvReadSearchMode(_params->_input_fields[0].read_search_mode),
			      _params->_input_fields[0].forecast_lead_secs,
			      _params->_input_fields[0].apply_min_thresh,
			      _params->_input_fields[0].min_thresh,
			      _params->_input_fields[0].apply_max_thresh,
			      _params->_input_fields[0].max_thresh,
			      _params->_input_fields[0].process_single_level,
			      _params->_input_fields[0].level_num);
  else
    field_x = _readInputField(_params->_input_fields[0].url,
			      _params->_input_fields[0].field_num,
			      trigger_time,
			      _params->_input_fields[0].max_input_secs,
			      _getMdvReadSearchMode(_params->_input_fields[0].read_search_mode),
			      _params->_input_fields[0].forecast_lead_secs,
			      _params->_input_fields[0].apply_min_thresh,
			      _params->_input_fields[0].min_thresh,
			      _params->_input_fields[0].apply_max_thresh,
			      _params->_input_fields[0].max_thresh,
			      _params->_input_fields[0].process_single_level,
			      _params->_input_fields[0].level_num);
  
  if (field_x == 0)
  {
    return false;
  }
  
  if (_params->_input_fields[1].use_field_name)
    field_y = _readInputField(_params->_input_fields[1].url,
			      _params->_input_fields[1].field_name,
			      trigger_time,
			      _params->_input_fields[1].max_input_secs,
			      _getMdvReadSearchMode(_params->_input_fields[1].read_search_mode),
			      _params->_input_fields[1].forecast_lead_secs,
			      _params->_input_fields[1].apply_min_thresh,
			      _params->_input_fields[1].min_thresh,
			      _params->_input_fields[1].apply_max_thresh,
			      _params->_input_fields[1].max_thresh,
			      _params->_input_fields[1].process_single_level,
			      _params->_input_fields[1].level_num);
  else
    field_y = _readInputField(_params->_input_fields[1].url,
			      _params->_input_fields[1].field_num,
			      trigger_time,
			      _params->_input_fields[1].max_input_secs,
			      _getMdvReadSearchMode(_params->_input_fields[1].read_search_mode),
			      _params->_input_fields[1].forecast_lead_secs,
			      _params->_input_fields[1].apply_min_thresh,
			      _params->_input_fields[1].min_thresh,
			      _params->_input_fields[1].apply_max_thresh,
			      _params->_input_fields[1].max_thresh,
			      _params->_input_fields[1].process_single_level,
			      _params->_input_fields[1].level_num);
  
  if (field_y == 0)
  {
    delete field_x;
    return false;
  }
  
  // Make sure that the fields have the same number of points

  Mdvx::field_header_t field_x_hdr = field_x->getFieldHeader();
  Mdvx::field_header_t field_y_hdr = field_y->getFieldHeader();
  
  if (field_x_hdr.nx != field_y_hdr.nx ||
      field_x_hdr.ny != field_y_hdr.ny ||
      field_x_hdr.nz != field_y_hdr.nz)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Fields have different numbers of points." << endl;
    cerr << "   X field: nx = " << field_x_hdr.nx
	 << ", ny = " << field_x_hdr.ny << ", nz = " << field_x_hdr.nz << endl;
    cerr << "   Y field: nx = " << field_y_hdr.nx
	 << ", ny = " << field_y_hdr.ny << ", nz = " << field_y_hdr.nz << endl;
    cerr << "Skipping fields...." << endl;
    
    return false;
  }
  
  // Generate the plot

  bool status;
  
  if (_params->accumulate_plots)
    status = _plotter->createPlot(_params->accumulation_file,
				  trigger_time, *field_x, *field_y);
  else
    status = _plotter->createPlot(trigger_time, *field_x, *field_y);

  if (!status)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error generating plot for time: " << trigger_time << endl;
    
    delete field_x;
    delete field_y;
    
    return false;
  }
  
  // Reclaim memory

  delete field_x;
  delete field_y;
  
  return true;
}


/*********************************************************************
 * _readInputField() - Read the indicated input field.
 *
 * Returns the newly read field on success, 0 on failure.
 */

MdvxField *MdvScatterPlot::_readInputField(const string &url,
					   const string &field_name,
					   const DateTime &trigger_time,
					   const int max_input_secs,
					   const Mdvx::read_search_mode_t search_mode,
					   const int forecast_lead_secs,
					   const bool apply_min_thresh,
					   const double min_thresh,
					   const bool apply_max_thresh,
					   const double max_thresh,
					   const bool process_single_level,
					   const int level_num) const
{
  static const string method_name = "MdvScatterPlot::_readInputField()";
  
  // Set up the read request

  DsMdvx input_file;
  
  input_file.addReadField(field_name);
  
  MdvxField *field =
    _performRead(input_file, url, trigger_time, max_input_secs,
		 search_mode, forecast_lead_secs,
		 process_single_level, level_num);

  _applyThresholds(field, apply_min_thresh, min_thresh,
		   apply_max_thresh, max_thresh);
  
  return field;
}


MdvxField *MdvScatterPlot::_readInputField(const string &url,
					   const int field_num,
					   const DateTime &trigger_time,
					   const int max_input_secs,
					   const Mdvx::read_search_mode_t search_mode,
					   const int forecast_lead_secs,
					   const bool apply_min_thresh,
					   const double min_thresh,
					   const bool apply_max_thresh,
					   const double max_thresh,
					   const bool process_single_level,
					   const int level_num) const
{
  static const string method_name = "MdvScatterPlot::_readInputField()";
  
  // Set up the read request

  DsMdvx input_file;
  
  input_file.addReadField(field_num);
  
  MdvxField *field =
    _performRead(input_file, url, trigger_time, max_input_secs,
		 search_mode, forecast_lead_secs,
		 process_single_level, level_num);

  _applyThresholds(field, apply_min_thresh, min_thresh,
		   apply_max_thresh, max_thresh);
  
  return field;
}
