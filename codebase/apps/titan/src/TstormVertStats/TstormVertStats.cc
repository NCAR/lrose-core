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
/*********************************************************************
 * TstormVertStats: TstormVertStats program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2007
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
#include <dsdata/Tstorm.hh>
#include <dsdata/TstormGroup.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "TstormVertStats.hh"
#include "Params.hh"

#include "MaxStat.hh"
#include "MeanStat.hh"
#include "MedianStat.hh"
#include "MinStat.hh"

#include "AsciiOutput.hh"

using namespace std;

// Global variables

TstormVertStats *TstormVertStats::_instance =
     (TstormVertStats *)NULL;


/*********************************************************************
 * Constructor
 */

TstormVertStats::TstormVertStats(int argc, char **argv) :
  _dataTrigger(0),
  _polygonGrid(0),
  _polygonGridSize(0),
  _output(0)
{
  static const string method_name = "TstormVertStats::TstormVertStats()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (TstormVertStats *)NULL);
  
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
  char *params_path = (char *) "unknown";
  
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

TstormVertStats::~TstormVertStats()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  delete _output;
  
  delete [] _polygonGrid;
  
  vector< Stat* >::iterator stat;
  for (stat = _statistics.begin(); stat != _statistics.end(); ++stat)
    delete *stat;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

TstormVertStats *TstormVertStats::Inst(int argc, char **argv)
{
  if (_instance == (TstormVertStats *)NULL)
    new TstormVertStats(argc, argv);
  
  return(_instance);
}

TstormVertStats *TstormVertStats::Inst()
{
  assert(_instance != (TstormVertStats *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool TstormVertStats::init()
{
  static const string method_name = "TstormVertStats::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the list of statistics

  if (!_initStatistics())
    return false;
  
  // Initialize the output object.  Note that this must be called AFTER
  // _initStatistics() since it needs the statistics vector.

  if (!_initOutput())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void TstormVertStats::run()
{
  static const string method_name = "TstormVertStats::run()";
  
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
 * _calcLevelStatistics() - Calculate the statistics for given vertical
 *                          level of data.
 *
 * Returns a vector of the calculated statistics.
 */

vector< double > TstormVertStats::_calcLevelStatistics(const Pjg &proj,
						       const fl32 *data_grid,
						       const int min_x,
						       const int min_y,
						       const int max_x,
						       const int max_y,
						       const fl32 missing_data_value,
						       const fl32 bad_data_value)
{
  static const string method_name = "TstormVertStats::_calcLevelStatistics()";

  vector< double > stats_list;
  
  // Create a vector of the valid data values

  vector< double > data_values;
  
  for (int x = min_x; x <= max_x; ++x)
  {
    for (int y = min_y; y <= max_y; ++y)
    {
      int index = proj.xyIndex2arrayIndex(x, y, 0);
      
      if (index < 0)
	continue;
      
      // Make sure this is a data point to include in the statistic

      if (_polygonGrid[index] == 0)
	continue;
      
      if (data_grid[index] == missing_data_value ||
	  data_grid[index] == bad_data_value)
	continue;
      
      // Use this value

      if (_params->verbose)
	cerr << "Grid point = " << data_grid[index] << endl;
      
      data_values.push_back(data_grid[index]);
      
    } /* endfor - y */
    
  } /* endfor - x */
  
  // Calculate the statistics on the data values

  vector< Stat* >::iterator stat_iter;
    
  double stat_value;
    
  for (stat_iter = _statistics.begin();
       stat_iter != _statistics.end(); ++stat_iter)
  {
    Stat *stat = *stat_iter;
      
    stat_value = stat->calcStatistic(data_values);
    
    if (_params->verbose)
      cerr << "        " << stat->getName() << " statistic has value "
	   << stat_value << endl;

    stats_list.push_back(stat_value);
    
  } /* endfor - stat_iter */

  return stats_list;
}


/*********************************************************************
 * _calcStatistics() - Calculate the statistics for this field
 *
 * Returns true on success, false on failure.
 */

bool TstormVertStats::_calcStatistics(const MdvxField &field)
{
  static const string method_name = "TstormVertStats::_calcStatistics()";
  
  // Process each of the storm groups from the database

  vector< TstormGroup* > &groups = _tstormMgr.getGroups();
  vector< TstormGroup* >::iterator group_iter;
  
  for (group_iter = groups.begin(); group_iter != groups.end(); ++group_iter)
  {
    TstormGroup *group = *group_iter;
    
    if (_params->verbose)
      cerr << "Processing tstorm group with "
	   << group->getNStorms() << " storms" << endl;
    
    vector< Tstorm* > &storms = group->getTstorms();
    vector< Tstorm* >::iterator storm_iter;
    
    int storm_num = 0;
    
    for (storm_iter = storms.begin();
	 storm_iter != storms.end(); ++storm_iter, ++storm_num)
    {
      if (_params->verbose)
	cerr << "   Processing storm number " << storm_num << endl;
      
      Tstorm *storm = *storm_iter;
      
      if (!storm->isForecastValid())
	continue;
      
      if (!_calcStormStatistics(field, *storm))
	return false;
      
    } /* endfor - storm_iter */
    
  } /* endfor - group_iter */
  
  return true;
}


/*********************************************************************
 * _calcStormStatistics() - Calculate the statistics for this field and
 *                          storm.
 *
 * Returns true on success, false on failure.
 */

bool TstormVertStats::_calcStormStatistics(const MdvxField &field,
					   const Tstorm &storm)
{
  static const string method_name = "TstormVertStats::_calcStormStatistics()";
  
  // Get the needed gridded field information

  Mdvx::field_header_t field_hdr = field.getFieldHeader();
  Mdvx::vlevel_header_t vlevel_hdr = field.getVlevelHeader();
  MdvxPjg proj(field_hdr);
  fl32 *gridded_data = (fl32 *)field.getVol();
  
  // Get the gridded polygon for this storm

  int grid_size = proj.getNx() * proj.getNy();
  if (_polygonGridSize < grid_size)
  {
    delete [] _polygonGrid;
    _polygonGrid = new unsigned char[grid_size];
    _polygonGridSize = grid_size;
  }
  
  memset(_polygonGrid, 0, _polygonGridSize * sizeof(unsigned char));
  
  int min_x, min_y, max_x, max_y;
  
  if (!storm.getPolygonGridGrowKm(proj,
				  _polygonGrid,
				  min_x, min_y, max_x, max_y,
				  _params->storm_growth_km))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error gridding storm polygon" << endl;
    
    return false;
  }
  
  if (_params->verbose)
  {
    cerr << "--> Gridded polygon:" << endl;
    cerr << "    min_x = " << min_x << ", min_y = " << min_y << endl;
    cerr << "    max_x = " << max_x << ", max_y = " << max_y << endl;
  }
  
  // Calculate the statistics for each vertical level

  for (int z = 0; z < proj.getNz(); ++z)
  {
    if (_params->verbose)
      cerr << "      Processing Z level " << z << endl;
    
    fl32 *vlevel_ptr = gridded_data + (z * grid_size);
    
    vector< double > stats_list =
      _calcLevelStatistics(proj, vlevel_ptr,
			   min_x, min_y, max_x, max_y,
			   field_hdr.missing_data_value,
			   field_hdr.bad_data_value);
    
    _output->addOutput(field_hdr.forecast_time,
		       storm.getSimpleTrack(), storm.getComplexTrack(),
		       vlevel_hdr.level[z], stats_list);
      
  } /* endfor - z */
  
  return true;
}


/*********************************************************************
 * _initOutput() - Initialize the object that creates the output.
 *
 * Returns true on success, false on failure.
 */

bool TstormVertStats::_initOutput(void)
{
  static const string method_name = "TstormVertStats::_initOutput()";
  
  switch (_params->output_type)
  {
  case Params::OUTPUT_ASCII :
  {
    vector< string > stat_names;
    vector< Stat* >::const_iterator stat_iter;
    
    for (stat_iter = _statistics.begin(); stat_iter != _statistics.end();
	 ++stat_iter)
      stat_names.push_back((*stat_iter)->getName());
    
    if (_params->accumulate_output)
    {
      _output = new AsciiOutput(_params->accumulation_file,
				_params->output_ascii.delimiter,
				stat_names,
				_params->output_ascii.include_header,
				_params->debug || _params->verbose);
    }
    else
    {
      _output = new AsciiOutput(_params->output_dir,
				_params->output_ext,
				_params->output_ascii.delimiter,
				stat_names,
				_params->output_ascii.include_header,
				_params->debug || _params->verbose);
    }
    
    break;
  }
  
  } /* endswitch - _params->output_type */
  
  return true;
}


/*********************************************************************
 * _initStatistics() - Initialize the list of statistics.
 *
 * Returns true on success, false on failure.
 */

bool TstormVertStats::_initStatistics(void)
{
  for (int i = 0; i < _params->statistics_n; ++i)
  {
    switch (_params->_statistics[i])
    {
    case Params::STAT_MEAN :
      _statistics.push_back(new MeanStat(_params->verbose));
      break;

    case Params::STAT_MAX :
      _statistics.push_back(new MaxStat(_params->verbose));
      break;

    case Params::STAT_MIN :
      _statistics.push_back(new MinStat(_params->verbose));
      break;

    case Params::STAT_MEDIAN :
      _statistics.push_back(new MedianStat(_params->verbose));
      break;
    }
    
  }
  
  return true;
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool TstormVertStats::_initTrigger(void)
{
  static const string method_name = "TstormVertStats::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug || _params->verbose)
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
    
    if (_params->debug || _params->verbose)
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

MdvxField *TstormVertStats::_performRead(DsMdvx &input_file,
					const string &url,
					const DateTime &trigger_time,
					const int max_input_secs) const
{
  static const string method_name = "TstormVertStats::_performRead()";
  
  // Finish setting up the read request

  input_file.setReadTime(Mdvx::READ_FIRST_BEFORE,
			 url,
			 max_input_secs,
			 trigger_time.utime());
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  // Read the data

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input volume from: " << url << endl;
    cerr << input_file.getErrStr() << endl;
    
    return 0;
  }
  
  // Create the field to be used for processing.  Set the forecast_time in
  // this field to the MDV file time so that this value can be used in the
  // output

  MdvxField *return_field = new MdvxField(*(input_file.getField(0)));
  
  Mdvx::field_header_t field_hdr = return_field->getFieldHeader();
  field_hdr.forecast_time = trigger_time.utime();
  return_field->setFieldHeader(field_hdr);
  
  return return_field;
}


/*********************************************************************
 * _processData() - Process data for the given trigger time.
 *
 * Returns true on success, false on failure.
 */

bool TstormVertStats::_processData(const DateTime &trigger_time)
{
  static const string method_name = "TstormVertStats::_processData()";
  
  if (_params->debug || _params->verbose)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  // Read in the gridded input field

  MdvxField *field;
  
  if (_params->gridded_field.use_field_name)
    field = _readInputField(_params->gridded_field.url,
			    _params->gridded_field.field_name,
			    trigger_time,
			    _params->gridded_field.max_input_secs);
  else
    field = _readInputField(_params->gridded_field.url,
			    _params->gridded_field.field_num,
			    trigger_time,
			    _params->gridded_field.max_input_secs);
  
  if (field == 0)
  {
    return false;
  }
  
  // Read in the storm data

  _tstormMgr.clearData();
  
  if (_tstormMgr.readTstorms(_params->storms_url,
			     trigger_time.utime(),
			     _params->storms_max_input_secs) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading storm data from URL: "
	 << _params->storms_url << endl;
    cerr << "    Search time: " << trigger_time << endl;
    cerr << "    Search margin: " << _params->storms_max_input_secs << endl;
    
    delete field;
    
    return false;
  }
  
  if (_tstormMgr.getNumGroups() < 1)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "No storm data found for time: " << trigger_time << endl;
    
    delete field;
    
    return true;
  }
  
  // Calculate the statistics

  if (!_calcStatistics(*field))
  {
    delete field;
    return false;
  }
  
  // Reclaim memory

  delete field;
  
  // Generate the output

  if (!_output->writeOutput(trigger_time))
    return false;
  
  return true;
}


/*********************************************************************
 * _readInputField() - Read the indicated input field.
 *
 * Returns the newly read field on success, 0 on failure.
 */

MdvxField *TstormVertStats::_readInputField(const string &url,
					    const string &field_name,
					    const DateTime &trigger_time,
					    const int max_input_secs) const
{
  static const string method_name = "TstormVertStats::_readInputField()";
  
  // Set up the read request

  DsMdvx input_file;
  
  input_file.addReadField(field_name);
  
  MdvxField *field =
    _performRead(input_file, url, trigger_time, max_input_secs);

  return field;
}


MdvxField *TstormVertStats::_readInputField(const string &url,
					    const int field_num,
					    const DateTime &trigger_time,
					    const int max_input_secs) const
{
  static const string method_name = "TstormVertStats::_readInputField()";
  
  // Set up the read request

  DsMdvx input_file;
  
  input_file.addReadField(field_num);
  
  MdvxField *field =
    _performRead(input_file, url, trigger_time, max_input_secs);

  return field;
}
