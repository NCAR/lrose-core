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
//   $Date: 2016/03/04 02:22:10 $
//   $Id: MdvAreaCoverage.cc,v 1.5 2016/03/04 02:22:10 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvAreaCoverage: MdvAreaCoverage program object.
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
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "MdvAreaCoverage.hh"
#include "Params.hh"

#include "EQCoverTester.hh"
#include "GECoverTester.hh"
#include "GTCoverTester.hh"
#include "LECoverTester.hh"
#include "LTCoverTester.hh"
#include "MissingCoverTester.hh"
#include "NECoverTester.hh"
#include "ValidCoverTester.hh"

using namespace std;

// Global variables

MdvAreaCoverage *MdvAreaCoverage::_instance =
     (MdvAreaCoverage *)NULL;


/*********************************************************************
 * Constructor
 */

MdvAreaCoverage::MdvAreaCoverage(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvAreaCoverage::MdvAreaCoverage()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvAreaCoverage *)NULL);
  
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
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    
    okay = false;
    
    return;
  }
}


/*********************************************************************
 * Destructor
 */

MdvAreaCoverage::~MdvAreaCoverage()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  
  vector< CoverTester* >::iterator tester;
  for (tester = _testers.begin(); tester != _testers.end(); ++tester)
    delete *tester;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

MdvAreaCoverage *MdvAreaCoverage::Inst(int argc, char **argv)
{
  if (_instance == (MdvAreaCoverage *)NULL)
    new MdvAreaCoverage(argc, argv);
  
  return(_instance);
}

MdvAreaCoverage *MdvAreaCoverage::Inst()
{
  assert(_instance != (MdvAreaCoverage *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvAreaCoverage::init()
{
  static const string method_name = "MdvAreaCoverage::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the coverage testers

  if (!_initTesters())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void MdvAreaCoverage::run()
{
  static const string method_name = "MdvAreaCoverage::run()";
  
  // Print out the header

  _printHeader();
  
  // Process each of the input files, printing out the coverage values
  // for each as they are processed.

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
 * _calcCoverages() - Calculate the specified coverage values and print
 *                    them to stdout.
 */

void MdvAreaCoverage::_calcCoverages(const MdvxField &field)
{
  Mdvx::field_header_t field_hdr = field.getFieldHeader();
  fl32 *data = (fl32 *)field.getVol();
  
  int volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz;
  
  // Output the date and time values

  DateTime data_time(field_hdr.forecast_time);
  
  cout << data_time.getYear() << " " << data_time.getMonth()
       << " " << data_time.getDay()
       << " " << data_time.getHour() << " " << data_time.getMin()
       << " " << data_time.getSec();
  
  // Output the coverage values

  vector< CoverTester* >::iterator test_iter;
  
  for (test_iter = _testers.begin(); test_iter != _testers.end(); ++test_iter)
  {
    CoverTester *tester = *test_iter;
    
    tester->setBadValues(field_hdr.bad_data_value,
			 field_hdr.missing_data_value);
    
    int num_passing = 0;

    for (int i = 0; i < volume_size; ++i)
    {
      if (tester->isIncluded(data[i]))
	++num_passing;
    } /* endfor - i */
    
    cout << " " << num_passing;
    
  } /* endfor - test_iter */
  
  cout << endl;
  
}


/*********************************************************************
 * _initTesters() - Initialize the coverage tester objects.
 *
 * Returns true on success, false on failure.
 */

bool MdvAreaCoverage::_initTesters(void)
{
  for (int i = 0; i < _params->coverage_values_n; ++i)
  {
    switch (_params->_coverage_values[i].type)
    {
    case Params::COVERAGE_GREATER_THAN :
      _testers.push_back(new GTCoverTester(_params->_coverage_values[i].threshold));
      break;
    
    case Params::COVERAGE_GREATER_THAN_OR_EQUAL_TO :
      _testers.push_back(new GECoverTester(_params->_coverage_values[i].threshold));
      break;
    
    case Params::COVERAGE_LESS_THAN :
      _testers.push_back(new LTCoverTester(_params->_coverage_values[i].threshold));
      break;
    
    case Params::COVERAGE_LESS_THAN_OR_EQUAL_TO :
      _testers.push_back(new LECoverTester(_params->_coverage_values[i].threshold));
      break;
    
    case Params::COVERAGE_EQUAL_TO :
      _testers.push_back(new EQCoverTester(_params->_coverage_values[i].threshold));
      break;
    
    case Params::COVERAGE_NOT_EQUAL_TO :
      _testers.push_back(new NECoverTester(_params->_coverage_values[i].threshold));
      break;
    
    case Params::COVERAGE_MISSING :
      _testers.push_back(new MissingCoverTester(_params->_coverage_values[i].threshold));
      break;
    
    case Params::COVERAGE_NON_MISSING :
      _testers.push_back(new ValidCoverTester(_params->_coverage_values[i].threshold));
      break;
    } /* endswitch */
    
  } /* endfor - i */
  
  return true;
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool MdvAreaCoverage::_initTrigger(void)
{
  static const string method_name = "MdvAreaCoverage::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
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

MdvxField *MdvAreaCoverage::_performRead(DsMdvx &input_file,
					 const string &url,
					 const DateTime &trigger_time,
					 const int max_input_secs,
					 const int level_num) const
{
  static const string method_name = "MdvAreaCoverage::_performRead()";
  
  // Finish setting up the read request

  input_file.setReadTime(Mdvx::READ_FIRST_BEFORE,
			 url,
			 max_input_secs,
			 trigger_time.utime());
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  // Set the level number to read.  If level_num is negative but not -1,
  // we'll read the whole volume.  Otherwise, do the read as specified below.

  if (level_num == -1)
    input_file.setReadComposite();
  else if (level_num >= 0)
    input_file.setReadPlaneNumLimits(level_num, level_num);
  
  // Read the data

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input volume from: " << url << endl;
    cerr << input_file.getErrStr() << endl;
    
    return 0;
  }
  
  // Create the return field as a copy of the read field since the read
  // field will go away when this method exits.  Set the forecast time in
  // the field header to be the file time centroid so we can use that in
  // the output later on.

  MdvxField *field = new MdvxField(*(input_file.getField(0)));
  
  Mdvx::field_header_t field_hdr = field->getFieldHeader();
  field_hdr.forecast_time = input_file.getMasterHeader().time_centroid;
  field->setFieldHeader(field_hdr);
  
  return field;
}


/*********************************************************************
 * _printHeader() - Print the output header to stdout
 */

void MdvAreaCoverage::_printHeader() const
{
  cout << "Year,Month,Day,Hour,Minute,Second";
  
  for (int i = 0; i < _params->coverage_values_n; ++i)
    cout << "," << _params->_coverage_values[i].column_label;
  
  cout << endl;
}


/*********************************************************************
 * _processData() - Process data for the given trigger time.
 *
 * Returns true on success, false on failure.
 */

bool MdvAreaCoverage::_processData(const DateTime &trigger_time)
{
  static const string method_name = "MdvAreaCoverage::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  // Read in the input field

  MdvxField *field;
  
  if (_params->input_field.use_field_name)
    field = _readInputField(_params->input_field.url,
			    _params->input_field.field_name,
			    _params->input_field.level_num,
			    trigger_time,
			    _params->input_field.max_input_secs);
  else
    field = _readInputField(_params->input_field.url,
			    _params->input_field.field_num,
			    _params->input_field.level_num,
			    trigger_time,
			    _params->input_field.max_input_secs);
  
  if (field == 0)
  {
    return false;
  }
  
  // Calculate and print the coverages

  _calcCoverages(*field);
  
  // Reclaim memory

  delete field;
  
  return true;
}


/*********************************************************************
 * _readInputField() - Read the indicated input field.
 *
 * Returns the newly read field on success, 0 on failure.
 */

MdvxField *MdvAreaCoverage::_readInputField(const string &url,
					    const string &field_name,
					    const int level_num,
					    const DateTime &trigger_time,
					    const int max_input_secs) const
{
  static const string method_name = "MdvAreaCoverage::_readInputField()";
  
  // Set up the read request

  DsMdvx input_file;
  
  input_file.addReadField(field_name);
  
  MdvxField *field =
    _performRead(input_file, url, trigger_time, max_input_secs, level_num);

  return field;
}


MdvxField *MdvAreaCoverage::_readInputField(const string &url,
					    const int field_num,
					    const int level_num,
					    const DateTime &trigger_time,
					    const int max_input_secs) const
{
  static const string method_name = "MdvAreaCoverage::_readInputField()";
  
  // Set up the read request

  DsMdvx input_file;
  
  input_file.addReadField(field_num);
  
  MdvxField *field =
    _performRead(input_file, url, trigger_time, max_input_secs, level_num);

  return field;
}
