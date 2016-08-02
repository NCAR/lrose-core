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
//   $Id: Mdv3Dto2D.cc,v 1.6 2016/03/04 02:22:10 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Mdv3Dto2D: Mdv3Dto2D program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2008
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

#include "Mdv3Dto2D.hh"
#include "Params.hh"

#include "FirstComputer.hh"
#include "MaxComputer.hh"
#include "MeanComputer.hh"
#include "MinComputer.hh"
#include "SumComputer.hh"

using namespace std;

// Global variables

Mdv3Dto2D *Mdv3Dto2D::_instance =
     (Mdv3Dto2D *)NULL;


/*********************************************************************
 * Constructor
 */

Mdv3Dto2D::Mdv3Dto2D(int argc, char **argv) :
  _dataTrigger(0),
  _computer(0)
{
  static const string method_name = "Mdv3Dto2D::Mdv3Dto2D()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Mdv3Dto2D *)NULL);
  
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
  char *params_path = new char[sizeof("unknown")+1];
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

Mdv3Dto2D::~Mdv3Dto2D()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  delete _computer;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Mdv3Dto2D *Mdv3Dto2D::Inst(int argc, char **argv)
{
  if (_instance == (Mdv3Dto2D *)NULL)
    new Mdv3Dto2D(argc, argv);
  
  return(_instance);
}

Mdv3Dto2D *Mdv3Dto2D::Inst()
{
  assert(_instance != (Mdv3Dto2D *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool Mdv3Dto2D::init()
{
  static const string method_name = "Mdv3Dto2D::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the value computer

  if (!_initComputer())
    return false;
  
  // Initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void Mdv3Dto2D::run()
{
  static const string method_name = "Mdv3Dto2D::run()";
  
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
 * _initComputer() - Initialize the value computer.
 *
 * Returns true on success, false on failure.
 */

bool Mdv3Dto2D::_initComputer(void)
{
  static const string method_name = "Mdv3Dto2D::_initComputer()";

  switch (_params->computation)
  {
  case Params::COMPUTE_SUM :
    _computer = new SumComputer();
    break;

  case Params::COMPUTE_MEAN :
    _computer = new MeanComputer();
    break;

  case Params::COMPUTE_MAX :
    _computer = new MaxComputer();
    break;

  case Params::COMPUTE_MIN :
    _computer = new MinComputer();
    break;

  case Params::COMPUTE_FIRST :
    _computer = new FirstComputer();
    break;
  }
  
  return true;
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool Mdv3Dto2D::_initTrigger(void)
{
  static const string method_name = "Mdv3Dto2D::_initTrigger()";
  
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
      cerr << "   url: " << _params->input_field.url << endl;
      cerr << "   start time: " << DateTime::str(start_time) << endl;
      cerr << "   end time: " << DateTime::str(end_time) << endl;
    }
    
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->input_field.url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->input_field.url << endl;
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
	endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::LATEST_DATA :
  {
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_field.url,
		      _params->max_valid_age,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger for url: " <<
	_params->input_field.url << endl;
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
 * Returns true on success, false on failure.
 */

bool Mdv3Dto2D::_performRead(DsMdvx &input_file,
			     const string &url,
			     const DateTime &trigger_time,
			     const int max_input_secs) const
{
  static const string method_name = "Mdv3Dto2D::_performRead()";
  
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
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _processData() - Process data for the given trigger time.
 *
 * Returns true on success, false on failure.
 */

bool Mdv3Dto2D::_processData(const DateTime &trigger_time)
{
  static const string method_name = "Mdv3Dto2D::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  // Read in the input field

  DsMdvx input_file;
  bool read_successful;
  
  if (_params->input_field.use_field_name)
    read_successful = _readInputFile(input_file,
				     _params->input_field.url,
				     _params->input_field.field_name,
				     trigger_time,
				     _params->input_field.max_input_secs);
  else
    read_successful = _readInputFile(input_file,
				     _params->input_field.url,
				     _params->input_field.field_num,
				     trigger_time,
				     _params->input_field.max_input_secs);
  
  if (!read_successful)
    return false;
  
  MdvxField *field_3d = input_file.getField(0);
  if (field_3d == 0)
    return false;
  
  // Compute the 2D field

  MdvxField *field_2d;

  if ((field_2d = _computer->computeField(*field_3d)) == 0)
    return false;
  
  // Create and write the output file.  Note that in this method the
  // field_2d pointer is added to the output file, which reclaims the 
  // memory before exiting.  So don't reclaim this memory here.

  if (!_writeOutputFile(input_file.getMasterHeader(), field_2d))
    return false;
  
  return true;
}


/*********************************************************************
 * _readInputFile() - Read the indicated input field.
 *
 * Returns true on success, false on failure.
 */

bool Mdv3Dto2D::_readInputFile(DsMdvx &input_file,
			       const string &url,
			       const string &field_name,
			       const DateTime &trigger_time,
			       const int max_input_secs) const
{
  // Set up the read request

  input_file.addReadField(field_name);
  
  if (!_performRead(input_file, url, trigger_time, max_input_secs))
    return false;

  return true;
}


bool Mdv3Dto2D::_readInputFile(DsMdvx &input_file,
			       const string &url,
			       const int field_num,
			       const DateTime &trigger_time,
			       const int max_input_secs) const
{
  // Set up the read request

  input_file.addReadField(field_num);
  
  if (!_performRead(input_file, url, trigger_time, max_input_secs))
    return false;

  return true;
}


/*********************************************************************
 * _writeOutputFile() - Create and write the output file.
 *
 * Returns true on success, false on failure.
 */

bool Mdv3Dto2D::_writeOutputFile(const Mdvx::master_header_t input_master_hdr,
				 MdvxField *field_2d) const
{
  static const string method_name = "Mdv3Dto2D::_writeOutputFile()";
  
  // Create the output master header

  Mdvx::master_header_t output_master_hdr = input_master_hdr;
  output_master_hdr.data_dimension = 2;
  output_master_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  output_master_hdr.n_fields = 0;
  output_master_hdr.max_nx = 0;
  output_master_hdr.max_ny = 0;
  output_master_hdr.max_nz = 0;
  output_master_hdr.n_chunks = 0;
  
  // Create the output file

  DsMdvx output_file;
  
  output_file.setMasterHeader(output_master_hdr);

  field_2d->convertType(Mdvx::ENCODING_INT8,
			Mdvx::COMPRESSION_BZIP,
			Mdvx::SCALING_DYNAMIC);
  output_file.addField(field_2d);
  
  // Write the output file

  output_file.setWriteLdataInfo();
  
  if (output_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file to URL: " << _params->output_url << endl;
    cerr << output_file.getErrStr() << endl;

    return false;
  }
  
  return true;
}
