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
//   $Date: 2016/03/04 02:22:13 $
//   $Id: MdvVectorsTemporalSmooth.cc,v 1.7 2016/03/04 02:22:13 dixon Exp $
//   $Revision: 1.7 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvVectorsTemporalSmooth: MdvVectorsTemporalSmooth program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <rapmath/math_macros.h>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>

#include "MdvVectorsTemporalSmooth.hh"
#include "Params.hh"

using namespace std;


// Global variables

MdvVectorsTemporalSmooth *MdvVectorsTemporalSmooth::_instance =
     (MdvVectorsTemporalSmooth *)NULL;



/*********************************************************************
 * Constructor
 */

MdvVectorsTemporalSmooth::MdvVectorsTemporalSmooth(int argc, char **argv) :
  _dataTrigger(0),
  _prevGridTime(-1),
  _prevSpeed(0),
  _prevDir(0)
{
  static const string method_name = "MdvVectorsTemporalSmooth::MdvVectorsTemporalSmooth()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvVectorsTemporalSmooth *)NULL);
  
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
  char *params_path = (char *)"unknown";
  
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

MdvVectorsTemporalSmooth::~MdvVectorsTemporalSmooth()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  
  delete [] _prevSpeed;
  delete [] _prevDir;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

MdvVectorsTemporalSmooth *MdvVectorsTemporalSmooth::Inst(int argc, char **argv)
{
  if (_instance == (MdvVectorsTemporalSmooth *)NULL)
    new MdvVectorsTemporalSmooth(argc, argv);
  
  return(_instance);
}

MdvVectorsTemporalSmooth *MdvVectorsTemporalSmooth::Inst()
{
  assert(_instance != (MdvVectorsTemporalSmooth *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvVectorsTemporalSmooth::init()
{
  static const string method_name = "MdvVectorsTemporalSmooth::init()";
  
  // Initialize the data trigger

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
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */
  
  // Convert the units as needed for the limitations specified by the user.

  _speedDeltaMax = _params->speed_delta_max / MPERSEC_TO_KMPERHOUR;
  _dirDeltaMax = _params->direction_delta_max * DEG_TO_RAD;
  _minOutputSpeed = _params->min_output_speed / MPERSEC_TO_KMPERHOUR;
  _minRecoverySpeed = _params->min_recovery_speed / MPERSEC_TO_KMPERHOUR;

  if (_params->min_output_speed > _params->min_recovery_speed)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Error in parameter file." << endl;
    cerr << "min_recovery_speed must be greater than or equal to min_output_speed" << endl;
    cerr << "Got min_recovery_speed = " << _params->min_recovery_speed
	 << " and min_output_speed = " << _params->min_output_speed << endl;
    cerr << "Settin min_recovery_speed to " << _params->min_output_speed << endl;
    
    _minRecoverySpeed = _minOutputSpeed;
  }
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void MdvVectorsTemporalSmooth::run()
{
  static const string method_name = "MdvVectorsTemporalSmooth::run()";
  
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
 * _calcSpeedDir() - Compute the speed and direction values for the
 *                   grid based on the current U/V values.
 */

void MdvVectorsTemporalSmooth::_calcSpeedDir(const MdvxField &u_field,
					     const MdvxField &v_field,
					     fl32 *speed, fl32 *dir) const
{
  Mdvx::field_header_t u_field_hdr = u_field.getFieldHeader();
  Mdvx::field_header_t v_field_hdr = v_field.getFieldHeader();
  
  fl32 *u_data = (fl32 *)u_field.getVol();
  fl32 *v_data = (fl32 *)v_field.getVol();
  
  // Loop through each grid point calculating speed and direction.
  // We can assume that the grids have the same projection because that
  // was checked before calling this method.

  int volume_size = u_field_hdr.nx * u_field_hdr.ny * u_field_hdr.nz;
  
  for (int i = 0; i < volume_size; ++i)
  {
    // If the U or V data is missing, set the speed and direction to 0.

    if (u_data[i] == u_field_hdr.bad_data_value ||
	u_data[i] == u_field_hdr.missing_data_value ||
	v_data[i] == v_field_hdr.bad_data_value ||
	v_data[i] == v_field_hdr.missing_data_value)
    {
      speed[i] = 0;
      dir[i] = 0;
      continue;
    }

    if (u_data[i] == 0.0)
    {
      if (v_data[i] >= 0.0)
	dir[i] = 0.0;
      else
	dir[i] = M_PI;
    }
    else if (v_data[i] == 0.0)
    {
      if (u_data[i] >= 0.0)
	dir[i] = M_PI_2;
      else
	dir[i] = -M_PI_2;
    }
    else
      dir[i] = atan2(u_data[i], v_data[i]);
      
    speed[i] = sqrt((u_data[i] * u_data[i]) +
		    (v_data[i] * v_data[i]));
  } /* endfor - i */
  
}

/*********************************************************************
 * _processData() - Process the data for the given time.
 */

bool MdvVectorsTemporalSmooth::_processData(DateTime &trigger_time)
{
  static const string method_name = "MdvVectorsTemporalSmooth::_processData()";
  
  if (_params->debug)
    cerr << endl
	 << "*** Processing data for time: " << trigger_time << endl;
  
  // Read in the input file

  if (_params->debug)
    cerr << "Reading input file from URL: " << _params->input_url << endl;
  
  DsMdvx input_mdv;
  
  if (!_readMdvFile(input_mdv, _params->input_url, trigger_time))
    return false;
  
  // Read in the previous state file, if necessary

  _readPrevData(input_mdv.getMasterHeader().time_centroid);
  
  // Pull out the U and V fields.  We know that the U field is field 0
  // and the V field is field 1 because of the way we specified the fields
  // in the MDV request.

  if (_params->debug)
    cerr << "Extracting U/V fields from input file" << endl;
  
  MdvxField *u_field;
  
  if ((u_field = input_mdv.getField(0)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting U field from input file" << endl;
    
    return false;
  }
  
  MdvxField *v_field;
  
  if ((v_field = input_mdv.getField(1)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting V field from input file" << endl;
    
    return false;
  }
  
  // Make sure that the fields have matching projections

  if (_params->debug)
    cerr << "Checking field projections" << endl;
  
  Mdvx::master_header_t master_hdr = input_mdv.getMasterHeader();
  
  Mdvx::field_header_t u_field_hdr = u_field->getFieldHeader();
  Mdvx::field_header_t v_field_hdr = v_field->getFieldHeader();
  
  MdvxPjg u_proj(master_hdr, u_field_hdr);
  MdvxPjg v_proj(master_hdr, v_field_hdr);
  
  if (u_proj != v_proj)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Cannot process current U/V fields" << endl;
    cerr << "Projections for the fields aren't the same" << endl;
    
    return false;
  }
  
  // Compute the speed and direction for the input fields.

  if (_params->debug)
    cerr << "Computing input speed and direction values" << endl;
  
  int volume_size = u_field_hdr.nx * u_field_hdr.ny * u_field_hdr.nz;
  
  fl32 *curr_speed = new fl32[volume_size];
  fl32 *curr_dir = new fl32[volume_size];
  
  _calcSpeedDir(*u_field, *v_field, curr_speed, curr_dir);
  
  // Update the current speed and direction based on the previous
  // values and on the limitations specified in the parameter file.

  if (_params->debug)
    cerr << "Smoothing input speed and direction values" << endl;
  
  _smoothSpeedDir(u_proj, master_hdr.time_centroid, curr_speed, curr_dir);
  
  // Save the current speed/direction values to use next time.

  if (_params->debug)
    cerr << "Updating previous state with current values" << endl;
  
  delete [] _prevSpeed;
  delete [] _prevDir;
  
  _prevSpeed = curr_speed;
  _prevDir = curr_dir;
  _prevProj = u_proj;
  _prevGridTime = master_hdr.time_centroid;
  
  // Update the U/V values in the input fields to match the smoothed
  // speed and direction values.

  if (_params->debug)
    cerr << "Computing new U/V values from smoothed speed and direction values" << endl;
  
  _updateUV(*u_field, *v_field, curr_speed, curr_dir);
  
  // Write the output file

  if (_params->debug)
    cerr << "Writing output file to URL: " << _params->output_url << endl;
  
  for (int i = 0; i < master_hdr.n_fields; ++i)
  {
    MdvxField *field = input_mdv.getField(i);
    field->convertType(Mdvx::ENCODING_INT8,
		       Mdvx::COMPRESSION_BZIP,
		       Mdvx::SCALING_DYNAMIC);
  }
  
  input_mdv.setAppName("MdvVectorsTemporalSmooth");
  if (input_mdv.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing smoothed vectors to output URL: " << _params->output_url << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _readMdvFile() - Read the MDV file for the given time.
 */

bool MdvVectorsTemporalSmooth::_readMdvFile(DsMdvx &input_mdv,
					    const string &input_url,
					    const DateTime &request_time,
					    const int search_margin) const
{
  return _readMdvFile(input_mdv, input_url,
		      request_time.utime(), search_margin);
}


bool MdvVectorsTemporalSmooth::_readMdvFile(DsMdvx &input_mdv,
					    const string &input_url,
					    const time_t request_time,
					    const int search_margin) const
{
  static const string method_name = "MdvVectorsTemporalSmooth::_readMdvFile()";
  
  // Set up the read request

  input_mdv.setReadTime(Mdvx::READ_FIRST_BEFORE,
			input_url,
			search_margin, request_time);

  // Adding the fields in this order will cause the U field to be field
  // number 0 and the V field to be field number 1 in the DsMdvx object.

  if (_params->use_field_names)
  {
    input_mdv.addReadField(_params->u_field.field_name);
    input_mdv.addReadField(_params->v_field.field_name);
  }
  else
  {
    input_mdv.addReadField(_params->u_field.field_num);
    input_mdv.addReadField(_params->v_field.field_num);
  }
  
  input_mdv.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_mdv.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_mdv.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_params->debug)
    input_mdv.printReadRequest(cerr);
  
  // Read the MDV file

  if (input_mdv.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input MDV file:" << endl;
    cerr << "   URL: " << input_url << endl;
    cerr << "   Request time: " << DateTime::str(request_time) << endl;
    cerr << "   msg: " << input_mdv.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _readPrevData() - If we don't currently have previous data (usually
 *                   because we just started or restarted), try to read
 *                   some in.
 */

void MdvVectorsTemporalSmooth::_readPrevData(const time_t curr_grid_time)
{
  static const string method_name = "MdvVectorsTemporalSmooth::_readPrevData()";
  
  // If we currently have data, then we don't need to look for any
  
  if (_prevSpeed != 0 && _prevDir != 0 &&
      _prevGridTime > 0)
    return;
  
  // First look in the output directory to see if we have a smoothed
  // vector file for the previous time.

  if (_params->debug)
    cerr << "Looking for previous data at URL: " << _params->output_url << endl;
  
  DsMdvx prev_mdv;
  
  if (!_readMdvFile(prev_mdv, _params->output_url,
		    curr_grid_time - 1, _params->max_grid_offset))
  {
    if (_params->debug)
      cerr << "Looking for previous data at URL: " << _params->input_url << endl;
    
    // We didn't find a file in the output directory, so now look in the
    // input directory so we at least have a starting point.

    if (!_readMdvFile(prev_mdv, _params->input_url,
		      curr_grid_time - 1, _params->max_grid_offset))
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "No previous data found, beginning without smoothing" << endl;
      
      // We couldn't find any previous data so exit.  Note that this
      // isn't an error condition.  It just means that we're starting
      // at the beginning.
      
      return;
    }
  }

  /////////////////////////////////////////////////////////////////////
  // If we get here, we read in some data that we're going to use as //
  // our previous state.                                             //
  /////////////////////////////////////////////////////////////////////

  // Get pointers to the U and V fields

  MdvxField *u_field;
  
  if ((u_field = prev_mdv.getField(0)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting U field from previous state file" << endl;
    
    return;
  }
  
  MdvxField *v_field;
  
  if ((v_field = prev_mdv.getField(1)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting V field from previous state file" << endl;
    
    return;
  }
  
  // Get the projection

  Mdvx::master_header_t master_hdr = prev_mdv.getMasterHeader();
  
  Mdvx::field_header_t u_field_hdr = u_field->getFieldHeader();
  Mdvx::field_header_t v_field_hdr = v_field->getFieldHeader();
  
  MdvxPjg u_proj(master_hdr, u_field_hdr);
  MdvxPjg v_proj(master_hdr, v_field_hdr);
  
  if (u_proj != v_proj)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Cannot initialize previous vectors" << endl;
    cerr << "Projections of the U and V fields don't match" << endl;
    
    return;
  }
  
  _prevProj = u_proj;
  _prevGridTime = master_hdr.time_centroid;
  
  int volume_size = u_field_hdr.nx * u_field_hdr.ny * u_field_hdr.nz;
  
  _prevSpeed = new fl32[volume_size];
  _prevDir = new fl32[volume_size];
  
  _calcSpeedDir(*u_field, *v_field, _prevSpeed, _prevDir);
}


/*********************************************************************
 * _smoothSpeedDir() - Smooth the speed and direction values based on
 *                     the previous values and on the limitations specified
 *                     in the parameter file.
 */

void MdvVectorsTemporalSmooth::_smoothSpeedDir(const MdvxPjg &curr_proj,
					       const time_t curr_grid_time,
					       fl32 *curr_speed,
					       fl32 *curr_dir) const
{
  static const string method_name = "MdvVectorsTemporalSmooth::_smoothSpeedDir()";
  
  // First check to see if we can use the previous data to smooth the
  // vectors.  If we can't, we leave the values unchanged and use them
  // to smooth the next grid.

  if (_prevSpeed == 0 || _prevDir == 0 ||
      _prevGridTime < 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Cannot smooth grid" << endl;
    cerr << "Don't have previous grid information" << endl;
    
    return;
  }
  
  if (_prevProj != curr_proj)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Cannot smooth grid" << endl;
    cerr << "Projection of new data doesn't match projection of previous data" << endl;
    
    return;
  }
  
  if (curr_grid_time > (_prevGridTime + _params->max_grid_offset))
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Cannot smooth grid" << endl;
    cerr << "Previous data too old to use for smoothing" << endl;
    cerr << "Current grid time: " << DateTime::str(curr_grid_time) << endl;
    cerr << "Previous grid time: " << DateTime::str(_prevGridTime) << endl;
    cerr << "Maximum allowed grid offset: " << _params->max_grid_offset
	 << " secs" << endl;
    
    return;
  }
  
  // Loop through each of the grid points and update as directed

  int volume_size = curr_proj.getNx() * curr_proj.getNy() * curr_proj.getNz();
  
  for (int i = 0; i < volume_size; ++i)
  {
    // Only smooth if we have a previous vector.  If the previous speed
    // is 0, then the previous vector is either missing or didn't meet
    // our post-processing speed criteria.

    if (_prevSpeed[i] > 0.0)
    {
      // Check the speed change

      double speed_delta = fabs(curr_speed[i] - _prevSpeed[i]);
    
      if (speed_delta > _speedDeltaMax)
      {
	if (curr_speed[i] > _prevSpeed[i])
	  curr_speed[i] = _prevSpeed[i] + _speedDeltaMax;
	else
	  curr_speed[i] = _prevSpeed[i] - _speedDeltaMax;
      }
    
      // Check the direction change

      double dir_diff = curr_dir[i] - _prevDir[i];
    
      while (dir_diff < -M_PI)
	dir_diff += 2.0 * M_PI;
      while (dir_diff >= M_PI)
	dir_diff -= 2.0 * M_PI;
    
      if (fabs(dir_diff) > _dirDeltaMax)
      {
	// Calculate the output direction and normalize it to between
	// pi and -pi.

	if (dir_diff > 0.0)
	  curr_dir[i] = _prevDir[i] + _dirDeltaMax;
	else
	  curr_dir[i] = _prevDir[i] - _dirDeltaMax;

	while (curr_dir[i] < -M_PI)
	  curr_dir[i] += 2.0 * M_PI;
	while (curr_dir[i] >= M_PI)
	  curr_dir[i] -= 2.0 * M_PI;
      }
    }
    
    // Check the minimum output speed

    if (curr_speed[i] < _minOutputSpeed)
    {
      curr_speed[i] = 0.0;
      curr_dir[i] = 0.0;
    }
    
    // Check the minimum recovery speed

    if (_prevSpeed[i] <= 0.0 && curr_speed[i] < _minRecoverySpeed)
    {
      curr_speed[i] = 0.0;
      curr_dir[i] = 0.0;
    }
    
  } /* endfor - i */
  
}


/*********************************************************************
 * _updateUV() - Update the U and V field values based on the smoothed
 *               speed and direction values.  Don't update grid squares
 *               where the original U or V value was missing.
 */

void MdvVectorsTemporalSmooth::_updateUV(MdvxField &u_field,
					 MdvxField &v_field,
					 const fl32 *speed,
					 const fl32 *dir) const
{
  Mdvx::field_header_t u_field_hdr = u_field.getFieldHeader();
  Mdvx::field_header_t v_field_hdr = v_field.getFieldHeader();
  
  fl32 *u_data = (fl32 *)u_field.getVol();
  fl32 *v_data = (fl32 *)v_field.getVol();
  
  int volume_size = u_field_hdr.nx * u_field_hdr.ny * u_field_hdr.nz;
  
  for (int i = 0; i < volume_size; ++i)
  {
    if (u_data[i] == u_field_hdr.bad_data_value ||
	v_data[i] == v_field_hdr.bad_data_value)
    {
      u_data[i] = u_field_hdr.bad_data_value;
      v_data[i] = v_field_hdr.bad_data_value;
      
      continue;
    }
    
    if (u_data[i] == u_field_hdr.missing_data_value ||
	v_data[i] == v_field_hdr.missing_data_value)
    {
      u_data[i] = u_field_hdr.missing_data_value;
      v_data[i] = v_field_hdr.missing_data_value;
      
      continue;
    }
    
    u_data[i] = speed[i] * sin(dir[i]);
    v_data[i] = speed[i] * cos(dir[i]);
  }
  
  // Set the min and max data values to 0 so that they will be recalculated
  // for data scaling

  u_field_hdr.min_value = 0.0;
  u_field_hdr.max_value = 0.0;
  
  v_field_hdr.min_value = 0.0;
  v_field_hdr.max_value = 0.0;
  
  u_field.setFieldHeader(u_field_hdr);
  v_field.setFieldHeader(v_field_hdr);
  
}
