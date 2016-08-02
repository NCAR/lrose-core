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
//   $Id: MdvInterpMissing.cc,v 1.8 2016/03/04 02:22:11 dixon Exp $
//   $Revision: 1.8 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvInterpMissing: MdvInterpMissing program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2007
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
#include <Mdv/MdvxField.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "MdvInterpMissing.hh"
#include "Params.hh"

using namespace std;

// Global variables

MdvInterpMissing *MdvInterpMissing::_instance =
     (MdvInterpMissing *)NULL;



/*********************************************************************
 * Constructor
 */

MdvInterpMissing::MdvInterpMissing(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvInterpMissing::MdvInterpMissing()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvInterpMissing *)NULL);
  
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

MdvInterpMissing::~MdvInterpMissing()
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

MdvInterpMissing *MdvInterpMissing::Inst(int argc, char **argv)
{
  if (_instance == (MdvInterpMissing *)NULL)
    new MdvInterpMissing(argc, argv);
  
  return(_instance);
}

MdvInterpMissing *MdvInterpMissing::Inst()
{
  assert(_instance != (MdvInterpMissing *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvInterpMissing::init()
{
  static const string method_name = "MdvInterpMissing::init()";
  
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

void MdvInterpMissing::run()
{
  static const string method_name = "MdvInterpMissing::run()";
  
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
 * _fillMissingField() - Fill the in the missing data values for the given
 *                       field.
 *
 * Returns true on success, false on failure.
 */

void MdvInterpMissing::_fillMissingField(MdvxField &field) const
{
  static const string method_name = "MdvInterpMissing::_fillMissingField()";
  
  PMU_auto_register("Filling missing field");
    
  Mdvx::field_header_t field_hdr = field.getFieldHeader();
  fl32 *data = (fl32 *)field.getVol();
  
  int plane_size = field_hdr.nx * field_hdr.ny;
  int volume_size = plane_size * field_hdr.nz;
  
  // Copy the original data so that we aren't using interpolated values
  // in our interpolations

  fl32 *orig_data = new fl32[volume_size];
  memcpy(orig_data, data, volume_size * sizeof(fl32));
  
  // Update the template to match the current data projection

  MdvxPjg proj(field_hdr);
  
  _updateTemplate(proj);
  
  // Loop through the data, interpolating any missing data values

  for (int z = 0; z < field_hdr.nz; ++z)
  {
    PMU_auto_register("Filling level");
    
    for (int y = 0; y < field_hdr.ny; ++y)
    {
      PMU_auto_register("Filling line");
    
      for (int x = 0; x < field_hdr.nx; ++x)
      {
	int index = proj.xyIndex2arrayIndex(x, y, z);
      
	// If we have a data value already, no interpolation is needed

	if (data[index] != field_hdr.missing_data_value &&
	    data[index] != field_hdr.bad_data_value)
	  continue;
      
	// If we get here, we need to try to interpolate a new data value
	// so we loop through the template around the current grid square
	// calculating the percent filled and the interpolated value

	GridPoint *point;
	
	int npts_filled = 0;
	int npts_in_template = 0;
	double data_sum = 0.0;
	double min_data_val;
	double max_data_val;
	
	for (point = _template.getFirstInGrid(x, y,
					      field_hdr.nx, field_hdr.ny);
	     point != 0;
	     point = _template.getNextInGrid())
	{
	  ++npts_in_template;
	  
	  int template_index =
	    point->getIndex(field_hdr.nx, field_hdr.ny) + (z * plane_size);
	  
	  if (orig_data[template_index] == field_hdr.bad_data_value ||
	      orig_data[template_index] == field_hdr.missing_data_value)
	    continue;
	  
	  if (npts_filled <= 0)
	  {
	    min_data_val = orig_data[template_index];
	    max_data_val = orig_data[template_index];
	  }
	  else
	  {
	    if (orig_data[template_index] < min_data_val)
	      min_data_val = orig_data[template_index];
	    if (orig_data[template_index] > max_data_val)
	      max_data_val = orig_data[template_index];
	  }

	  ++npts_filled;
	  data_sum += orig_data[template_index];
	} /* endfor - point */
	
	// See if we have valid information for doing the interpolation

	if (npts_in_template == 0 || npts_filled == 0)
	  continue;
	
	if (_params->check_max_interp_data_range)
	{
	  double data_range = max_data_val - min_data_val;

	  if (data_range > _params->max_interp_data_range)
	    continue;
	}

	double percent_filled = (double)npts_filled / (double)npts_in_template;

	if (percent_filled < _params->fill_percent)
	  continue;
	
	// Update the missing value with the interpolated value.

	data[index] = data_sum / (double)npts_filled;
	
      } /* endfor - x */
      
    } /* endfor - y */
    
  } /* endfor - z */
  
  // Reclaim memory

  delete [] orig_data;
  
}


/*********************************************************************
 * _fillMissingFields() - Fill the in the missing data values for all of the
 *                        fields in the given file.
 *
 * Returns true on success, false on failure.
 */

void MdvInterpMissing::_fillMissingFields(Mdvx &mdvx) const
{
  for (int i = 0; i < mdvx.getNFields(); ++i)
  {
    MdvxField *field = mdvx.getField(i);
    
    _fillMissingField(*field);
    
  } /* endfor - i */
  
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool MdvInterpMissing::_initTrigger(void)
{
  static const string method_name = "MdvInterpMissing::_initTrigger()";
  
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

bool MdvInterpMissing::_processData(const DateTime &trigger_time)
{
  static const string method_name = "MdvInterpMissing::_processData()";
  
  // Register with the process mapper

  string register_string = "*** Processing data for time: " + trigger_time.getStr();
  PMU_force_register(register_string.c_str());
  
  if (_params->debug)
    cerr << endl << register_string << endl;
  
  // Read the new input data

  DsMdvx mdvx;
  
  if (!_readInputFile(mdvx, trigger_time))
    return false;
  
  PMU_auto_register("Data read successfully");
  
  // Fill in the missing data in the fields

  _fillMissingFields(mdvx);
  
  // Write the output file
  
  PMU_auto_register("Writing output file");
  
  for (int i = 0; i < mdvx.getNFields(); ++i)
  {
    MdvxField *field = mdvx.getField(i);
    
    field->convertType(Mdvx::ENCODING_INT8,
		       Mdvx::COMPRESSION_BZIP,
		       Mdvx::SCALING_DYNAMIC);
  } /* endfor - i */
  
  mdvx.setWriteLdataInfo();
  
  if (mdvx.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing accumulation file to URL: "
	 << _params->output_url << endl;
    cerr << mdvx.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _readInputFile() - Read the indicated input file.
 *
 * Returns true on success, false on failure.
 */

bool MdvInterpMissing::_readInputFile(Mdvx &input_file,
				   const DateTime &trigger_time) const
{
  static const string method_name = "MdvInterpMissing::_readInputFile()";
  
  // Set up the read request

  input_file.setReadTime(Mdvx::READ_FIRST_BEFORE,
			 _params->input_url,
			 0, trigger_time.utime());
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);

  // Read the data

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input volume from: " << _params->input_url << endl;
    cerr << input_file.getErrStr() << endl;
    
    return false;
  }

  if(_params->debug)
  {
    cerr << "Reading from " << input_file.getPathInUse() << endl;
  }
  
  return true;
}


/*********************************************************************
 * _updateTemplate() - Update the radius of influence template based on
 *                     the current grid projection.
 */

void MdvInterpMissing::_updateTemplate(const MdvxPjg &proj) const
{
  double radius_x = proj.km2xGrid(_params->radius_of_influence);
  double radius_y = proj.km2yGrid(_params->radius_of_influence);
  
  if (radius_x > radius_y)
    _template.setEllipse(90.0, radius_x, radius_y);
  else
    _template.setEllipse(0.0, radius_y, radius_x);
}
