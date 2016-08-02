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
 * @file DeTect2Dsr.cc
 *
 * @class DeTect2Dsr
 *
 * DeTect2Dsr is the top level application class.
 *  
 * @date 7/7/2010
 *
 */

#include <iostream>
#include <limits.h>
#include <string>
#include <vector>

#include <toolsa/os_config.h>
#include <DeTect/DeTectFile.hh>
#include <dsdata/DsFileListTrigger.hh>
#include <rapformats/DsRadarMsg.hh>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "DeTect2Dsr.hh"
#include "Params.hh"

using namespace std;


// Global variables

DeTect2Dsr *DeTect2Dsr::_instance = (DeTect2Dsr *)NULL;

const int DeTect2Dsr::MAX_COUNT = 4096;


/*********************************************************************
 * Constructor
 */

DeTect2Dsr::DeTect2Dsr(int argc, char **argv) :
  _currentTimeRangeIndex(-1),
  _countToPower(0),
  _volumeNum(0),
  _prevAzimuth(999),
  _firstBeamTime(DateTime::NEVER),
  _lastBeamTime(DateTime::NEVER)
{
  static const string method_name = "DeTect2Dsr::DeTect2Dsr()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (DeTect2Dsr *)NULL);
  
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

DeTect2Dsr::~DeTect2Dsr()
{
  // If requested, print out the times of the first and last beams processed

  if (_params->write_times_to_stdout)
    cout << _firstBeamTime.getStrPlain() << " "
	 << _lastBeamTime.getStrPlain() << endl;
  
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  delete _dataTrigger;
  
  delete [] _countToPower;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst()
 */

DeTect2Dsr *DeTect2Dsr::Inst(int argc, char **argv)
{
  if (_instance == (DeTect2Dsr *)NULL)
    new DeTect2Dsr(argc, argv);
  
  return(_instance);
}

DeTect2Dsr *DeTect2Dsr::Inst()
{
  assert(_instance != (DeTect2Dsr *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool DeTect2Dsr::init()
{
  static const string method_name = "DeTect2Dsr::init()";
  
  // Initialize the input trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the count to power look-up table

  if (!_initCountToPower())
    return false;
  
  // Initialize the radar queue

  if (_radarQueue.init(_params->output_fmq_url,
		       "DeTect2Dsr",
		       _params->debug,
		       DsFmq::READ_WRITE, DsFmq::END,
		       _params->output_fmq_compress,
		       _params->output_fmq_nslots,
		       _params->output_fmq_size,
		       1000))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error initializing radar queue: "
	 << _params->output_fmq_url << endl;
    
    return false;
  }
  
  if (_params->use_blocking_write)
    _radarQueue.setBlockingWrite();
  
  // Initialize the time list

  if (!_initTimeList())
    return false;
  
  // Create the colorscale files, if requested

  if (_params->create_count_colorscale)
    _createCountColorscale();

  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}
  


/*********************************************************************
 * run()
 */

void DeTect2Dsr::run()
{
  static const string method_name = "DeTect2Dsr::run()";
  
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

/*********************************************************************
 * _createCountColorscale()
 */

void DeTect2Dsr::_createCountColorscale() const
{
  static const string method_name = "DeTect2Dsr::_createCountColorscale()";
  
  FILE *file;

  if ((file = fopen(_params->count_colorscale_path, "w")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening colorscale file for output" << endl;
    perror(_params->count_colorscale_path);
    
    return;
  }
  
  double min_value = 0.0;
  double max_value;
  
  double step =
    (double)_params->count_colorscale_max_spread_value / 254.0;
  
  for (short int i = 0; i < 254; ++i, min_value = max_value)
  {
    max_value = min_value + step;

    fprintf(file, "%8d  %8d   #%02x%02x%02x\n",
            (int)min_value, (int)max_value, i, i, i);
  }
  
  fprintf(file, "%8d  %8d   #ffffff\n",
          (int)max_value, 4096);

  fclose(file);
}


/*********************************************************************
 * _initCountToPower()
 */

bool DeTect2Dsr::_initCountToPower()
{
  static const string method_name = "DeTect2Dsr::_initCountToPower()";
  
  // Make sure that some sort of a table was specified.  If it wasn't then
  // we can't process the data.

  if (_params->count_to_power_table_n == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "No count_to_power_table specified in the parameter file" << endl;
    cerr << "Cannot process data" << endl;
    
    return false;
  }
  
  // Calculate the power scale and bias

  if (_params->count_to_power_table_n == 1)
  {
    _powerScale = 1.0;
    _powerBias = 0.0;
  }
  else
  {
    double min_power = _params->_count_to_power_table[0].power;
    double max_power =
      _params->_count_to_power_table[_params->count_to_power_table_n].power;

    if (min_power >= max_power)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Power should increase with count in count_to_power_table" << endl;
      cerr << "You have:" << endl;
      cerr << "    count_to_power_table[0].power = " << min_power << endl;
      cerr << "    count_to_power_table[" << _params->count_to_power_table_n
	   << "] = " << max_power << endl;
      cerr << "Fix parameters and try again" << endl;
      
      return false;
    }

    _powerBias = min_power;
    _powerScale = (max_power - min_power) / 255.0;
  }
  
  cerr << "_powerScale = " << _powerScale << endl;
  cerr << "_powerBias = " << _powerBias << endl;
  
  // Allocate space for the table.

  _countToPower = new int[MAX_COUNT+1];
  
  int current_param_index = 0;
  
  for (int i = 0; i <= MAX_COUNT; ++i)
  {
    double power;
    
    // See if we need to move forward in the table in the parameter file

    while (current_param_index < _params->count_to_power_table_n &&
	   _params->_count_to_power_table[current_param_index].count < i)
      ++current_param_index;
    
    // Get the appropriate power value from the table

    if (current_param_index >= _params->count_to_power_table_n)
    {
      // If we've gone past the end of the parameter table, take the last
      // power value

      power = 
	_params->_count_to_power_table[_params->count_to_power_table_n].power;
    }
    else if (i == _params->_count_to_power_table[current_param_index].count ||
	     current_param_index == 0)
    {
      // If we are at the exact count value or we are at the beginning of the
      // parameter file table, pull the appropriate value from the table

      power =
	_params->_count_to_power_table[current_param_index].power;
    }
    else
    {
      // If we get here, we need to linearly interpolate to get the power value

      double next_count =
	(double)_params->_count_to_power_table[current_param_index].count;
      double prev_count =
	(double)_params->_count_to_power_table[current_param_index-1].count;
    
      double next_power =
	_params->_count_to_power_table[current_param_index].power;
      double prev_power =
	_params->_count_to_power_table[current_param_index-1].power;

      double ratio = ((double)i - prev_count) / (next_count - prev_count);
      power = (ratio * (next_power - prev_power)) + prev_power;
    }
    
    _countToPower[i] = (int)(((power - _powerBias) / _powerScale) + 0.5);
  } /* endfor - i */
  
  return true;
}


/*********************************************************************
 * _initTimeList()
 */

bool DeTect2Dsr::_initTimeList()
{
  static const string method_name = "DeTect2Dsr::_initTimeList()";
  
  // If we are not using a time list, don't do anything.

  if (!_params->use_time_list)
    return true;
  
  // If there are no times specified in the parameter file, then assume that
  // the user made a mistake in setting the use_time_list flag and process all
  // of the times in the input files.

  if (_params->time_list_n == 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "use_time_list parameter set to true, but no times specified" << endl;
    cerr << "Assuming you want to process all times in the input files" << endl;
    
    return true;
  }
  
  // Parse the times in the parameter file and put them in the list

  DateTime previous_end_time = DateTime::NEVER;
  
  for (int i = 0; i < _params->time_list_n; ++i)
  {
    // Parse the time strings and check for any errors

    pair< DateTime, DateTime > time_range;
    time_range.first = DateTime(_params->_time_list[i].start_time);
    time_range.second = DateTime(_params->_time_list[i].end_time);
    
    if (time_range.first == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing specified start time string: "
	   << _params->_time_list[i].start_time << endl;
      return false;
    }
    
    if (time_range.second == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing specified end time string: "
	   << _params->_time_list[i].end_time << endl;
      return false;
    }
    
    if (time_range.first >= time_range.second)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Specified start time after specified end time" << endl;
      cerr << "Start time specified: "
	   << _params->_time_list[i].start_time << endl;
      cerr << "End time specified: "
	   << _params->_time_list[i].end_time << endl;
      cerr << "Fix parameters and try again..." << endl;
      
      return false;
    }
    
    if (previous_end_time == DateTime::NEVER)
    {
      previous_end_time = time_range.second;
    }
    else
    {
      if (time_range.first < previous_end_time)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Specified time ranges not in chronological order" << endl;
	cerr << "Fix parameters and try again..." << endl;
	
	return false;
      }
    }
    
    // Add the time range to the list

    _timeList.push_back(time_range);
    
  } /* endfor - i */
  
  if (_params->debug)
  {
    cerr << "**** Time list:" << endl;
    vector< pair< DateTime, DateTime > >::const_iterator time_range;
    
    for (time_range = _timeList.begin(); time_range != _timeList.end();
	 ++time_range)
      cerr << "   " << time_range->first << "   " << time_range->second << endl;
  }
  
  // Set the current time range to the first time in the list

  _currentTimeRangeIndex = 0;
  
  return true;
}


/*********************************************************************
 * _initTrigger()
 */

bool DeTect2Dsr::_initTrigger()
{
  static const string method_name = "DeTect2Dsr::_initTrigger()";
  
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
    
    if (_params->debug)
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
  
  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _processData()
 */

bool DeTect2Dsr::_processData(const string &file_path)
{
  static const string method_name = "DeTect2Dsr::_processData()";
  
  PMU_auto_register("Processing input file");

  if (_params->debug)
    cerr << endl << endl
	 << "*** Processing file: " << file_path << endl << endl;
  
  // Read the file

  DeTectFile input_file(file_path, _params->debug);
  
  if (!input_file.openFile())
    return false;

  // Process the beginning archive label

  ArchiveLabel archive_label;
  
  if (!input_file.readArchiveLabel(archive_label))
    return false;
  
  if (_params->debug)
    archive_label.print(cerr, "");
  
  // Process the DataObjects

  while (!input_file.isEndOfData())
  {
    // Get the next data object from the file and process it

    DataObject data_object;
    
    if (!input_file.readNextDataObject(data_object))
      break;

    if (_params->resample_beams)
    {
      DataObject::resample_type_t resample_type;
    
      switch (_params->resample_type)
      {
      case Params::RESAMPLE_MIN :
	resample_type = DataObject::RESAMPLE_MIN;
	break;
      case Params::RESAMPLE_MAX :
	resample_type = DataObject::RESAMPLE_MAX;
	break;
      case Params::RESAMPLE_MEAN :
	resample_type = DataObject::RESAMPLE_MEAN;
	break;
      }
    
      data_object.setResampleBeams(_params->resampled_beam_width,
				   resample_type);
    }
  
    _processDataObject(data_object);
    
  } /* endwhile - !input_file.isEndOfData() */
  
  return true;
}


/*********************************************************************
 * _processDataObject()
 */

void DeTect2Dsr::_processDataObject(const DataObject &data_object)
{
  static const string method_name = "DeTectFile::_processDataObject()";
  
  DsRadarMsg radar_msg;
  
  // Fill in the radar attributes

  DsRadarParams &radar_params = radar_msg.getRadarParams();
  
  radar_params.radarId = _params->radar_id;
  radar_params.radarType = DS_RADAR_GROUND_TYPE;
  radar_params.numFields = 2;
  radar_params.numGates = data_object.getNumGates();
  radar_params.samplesPerBeam = data_object.getNumGates();
  radar_params.scanType = DS_RADAR_SURVEILLANCE_MODE;
  radar_params.scanMode = DS_RADAR_SURVEILLANCE_MODE;
  radar_params.followMode = DS_RADAR_FOLLOW_MODE_NONE;
  radar_params.polarization = DS_POLARIZATION_HORIZ_TYPE;
  radar_params.prfMode = DS_RADAR_PRF_MODE_FIXED;
  radar_params.altitude = _params->radar_altitude;
  radar_params.latitude = data_object.getLatitude();
  radar_params.longitude = data_object.getLongitude();
  radar_params.gateSpacing = data_object.getGateSpacingKm();
  radar_params.startRange = data_object.getStartRangeKm();
  radar_params.horizBeamWidth = _params->horiz_beam_width;
  radar_params.vertBeamWidth = _params->vert_beam_width;
  radar_params.radarName = _params->radar_name;
  
  // Fill in the field parameters

  DsFieldParams count_field_params;
  
  count_field_params.byteWidth = 2;
  count_field_params.missingDataValue = USHRT_MAX;
  count_field_params.scale = 1.0;
  count_field_params.bias = 0.0;
  count_field_params.name = "count";
  count_field_params.units = "none";
  
  DsFieldParams power_field_params;
  
  power_field_params.byteWidth = 2;
  power_field_params.missingDataValue = USHRT_MAX;
  power_field_params.scale = _powerScale;
  power_field_params.bias = _powerBias;
  power_field_params.name = "power";
  power_field_params.units = "dBm";
  
  radar_msg.addFieldParams(count_field_params);
  radar_msg.addFieldParams(power_field_params);
  
  // Send all of the beams.  We will send the radar params and field params
  // with the first message and then again periodically through the data
  // stream.

  for (size_t beam_num = 0, messages_sent = 0;
       beam_num < data_object.getNumBeams(); ++beam_num, ++messages_sent)
  {
    // Get the beam information.

    double azimuth;
    vector< int > beam_data;
    DateTime beam_time;
    
    if (!data_object.getBeam(beam_num, beam_time, azimuth, beam_data))
      continue;
    
    // Check to see if the beam time is within the specified time ranges.
    // If it is not, skip this beam.

    if (_currentTimeRangeIndex >= 0)
    {
      // If this beam time is before the current time range, we can just skip
      // the beam and move on.

      if (beam_time < _timeList[_currentTimeRangeIndex].first)
	continue;
      
      // If this beam time is later than the end of the current range, we need
      // to switch time ranges.

      if (beam_time > _timeList[_currentTimeRangeIndex].second)
      {
	// Look through the time list and find the next range that ends after
	// this beam

	int next_range_index = -1;
	
	for (size_t i = _currentTimeRangeIndex+1; i < _timeList.size(); ++i)
	{
	  if (beam_time < _timeList[i].second)
	  {
	    next_range_index = i;
	    break;
	  }
	} /* endfor - i */

	// If we are past the end of the last range, we can just exit here.
	// I should probably do a clean back out here, but don't feel like
	// going to the trouble since this executable probably won't be used
	// again after the USDA Bird project is over.

	if (next_range_index < 0)
	{
	  cerr << "---> Finished processing last time range, exiting" << endl;
	  
	  // If requested, print out the times of the first and last beams
	  // processed

	  if (_params->write_times_to_stdout)
	    cout << _firstBeamTime.getStrPlain() << " "
		 << _lastBeamTime.getStrPlain() << endl;
  
	  exit(0);
	}
	
	// Set the new _currentTimeRangeIndex and process the current beam
	// appropriately.

	_currentTimeRangeIndex = next_range_index;
	
	if (beam_time < _timeList[_currentTimeRangeIndex].first)
	  continue;
	
      }
    } /* endif - _currentTimeRangeIndex >= 0 */
    
    // Save the start and end beam times for use outside of this method

    if (_firstBeamTime == DateTime::NEVER)
      _firstBeamTime = beam_time;
    
    _lastBeamTime = beam_time;
    
    // See if we are at the beginning of a new tilt.  If we are, we need to
    // send an end-of-tilt/end-of-volume message.  We are arbitrarily defining
    // our tilts to run from 0-360 degrees.

    if (azimuth < _prevAzimuth)
    {
      // Send the end of tilt/volume messages to the FMQ

      _radarQueue.putEndOfTilt(0);
      _radarQueue.putEndOfVolume(_volumeNum);
      
      // Increment the volume number

      ++_volumeNum;

      // Now send the start of tilt/volume messages to the FMQ

      _radarQueue.putStartOfVolume(_volumeNum);
      _radarQueue.putStartOfTilt(0);
      
    }
    
    // Now construct the beam message and write it to the queue

    int msg_content = 0;
    
    if (messages_sent % _params->output_radar_param_freq == 0)
    {
      msg_content |= (int)DsRadarMsg::RADAR_PARAMS;
      msg_content |= (int)DsRadarMsg::FIELD_PARAMS;
    }
    
    msg_content |= (int)DsRadarMsg::RADAR_BEAM;
    
    DsRadarBeam &radar_beam = radar_msg.getRadarBeam();
    
    radar_beam.dataTime = beam_time.utime();
    radar_beam.azimuth = azimuth;
    radar_beam.elevation = 0.0;
    radar_beam.targetElev = 0.0;
    radar_beam.volumeNum = _volumeNum;
    radar_beam.tiltNum = 0;
    
    int beam_buffer_size = data_object.getNumGates() * 4;
    ui08 *beam_buffer = new ui08[beam_buffer_size];
    ui16 *beam_buffer_ptr = (ui16 *)beam_buffer;
    
    for (size_t gate = 0; gate < data_object.getNumGates(); ++gate)
    {
      // Load the count field

      *beam_buffer_ptr = (ui16)beam_data[gate];
      ++beam_buffer_ptr;
      
      // Load the power field.  Make sure we don't go outside of the bounds
      // of the _countToPower table.

      if (beam_data[gate] < 0)
	*beam_buffer_ptr = (ui16)_countToPower[0];
      else if (beam_data[gate] > MAX_COUNT)
	*beam_buffer_ptr = (ui16)_countToPower[MAX_COUNT];
      else
	*beam_buffer_ptr = (ui16)_countToPower[beam_data[gate]];
      ++beam_buffer_ptr;
      
    } /* endfor - gate */

    radar_beam.loadData(beam_buffer, beam_buffer_size, 2);
    
    if (_radarQueue.putDsBeam(radar_msg, msg_content) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing beam to FMQ" << endl;
    }
    
    // Increment things for the next iteration

    _prevAzimuth = azimuth;
    
    delete [] beam_buffer;
    
  } /* endfor - beam_num */
}
