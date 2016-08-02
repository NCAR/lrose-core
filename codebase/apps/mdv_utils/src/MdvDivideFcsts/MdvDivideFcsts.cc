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
//   $Id: MdvDivideFcsts.cc,v 1.7 2016/03/04 02:22:11 dixon Exp $
//   $Revision: 1.7 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvDivideFcsts: MdvDivideFcsts program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2007
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

#include "MdvDivideFcsts.hh"
#include "Params.hh"

using namespace std;

// Global variables

MdvDivideFcsts *MdvDivideFcsts::_instance =
     (MdvDivideFcsts *)NULL;


/*********************************************************************
 * Constructor
 */

MdvDivideFcsts::MdvDivideFcsts(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvDivideFcsts::MdvDivideFcsts()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvDivideFcsts *)NULL);
  
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

MdvDivideFcsts::~MdvDivideFcsts()
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

MdvDivideFcsts *MdvDivideFcsts::Inst(int argc, char **argv)
{
  if (_instance == (MdvDivideFcsts *)NULL)
    new MdvDivideFcsts(argc, argv);
  
  return(_instance);
}

MdvDivideFcsts *MdvDivideFcsts::Inst()
{
  assert(_instance != (MdvDivideFcsts *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvDivideFcsts::init()
{
  static const string method_name = "MdvDivideFcsts::init()";
  
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

void MdvDivideFcsts::run()
{
  static const string method_name = "MdvDivideFcsts::run()";
  
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
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool MdvDivideFcsts::_initTrigger(void)
{
  static const string method_name = "MdvDivideFcsts::_initTrigger()";
  
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
  
  case Params::LATEST_DATA :
  {
    if (_params->debug)
    {
      cerr << "Initializing LATEST_DATA trigger: " << endl;
      cerr << "   url: " << _params->input_url << endl;
    }
    
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_url,
		      _params->max_valid_secs,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger for url: " <<
	_params->input_url << endl;
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

bool MdvDivideFcsts::_processData(const DateTime &trigger_time)
{
  static const string method_name = "MdvDivideFcsts::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  // Read in the input file

  DsMdvx mdvx;
  
  if (!_readInputData(mdvx, _params->input_url, trigger_time))
    return false;
  
  Mdvx::master_header_t master_hdr = mdvx.getMasterHeader();
  
  // Process each of the forecast fields

  for (int i = 0; i < _params->fcst_info_n; ++i)
  {
    // Get the field

    MdvxField *field = 0;
    
    if (_params->_fcst_info[i].use_field_name)
    {
      if (_params->verbose)
	cerr << "--- Processing field <" << _params->_fcst_info[i].field_name
	     << ">" << endl;
      
      if ((field = mdvx.getField(_params->_fcst_info[i].field_name)) == 0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error retrieving field <"
	     << _params->_fcst_info[i].field_name << "> from input file" << endl;
	continue;
      }
    }
    else
    {
      if (_params->verbose)
	cerr << "--- Processing field number "
	     << _params->_fcst_info[i].field_num << endl;
      
      if ((field = mdvx.getField(_params->_fcst_info[i].field_num)) == 0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error retrieving field number "
	     << _params->_fcst_info[i].field_num << " from input file" << endl;
	continue;
      }
    }
    
    // Write the field

    _writeFcstField(*field, _params->_fcst_info[i].fcst_lead_secs,
		    _params->output_url, _params->output_field_name,
		    master_hdr);
    
  } /* endfor - i */
  
  return true;
}


/*********************************************************************
 * _readInputData() - Read the indicated input data.
 *
 * Returns true on success, false on failure.
 */

bool MdvDivideFcsts::_readInputData(DsMdvx &mdvx,
				     const string &url,
				     const DateTime &trigger_time) const
{
  static const string method_name = "MdvDivideFcsts::_readInputData()";
  
  // Set up the read request

  mdvx.setReadTime(Mdvx::READ_CLOSEST, url, 0, trigger_time.utime());
  
  // Read the data

  if (mdvx.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input volume from: " << url << endl;
    cerr << mdvx.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _writeFcstField() - Write the given field to the indicated URL as a 
 *                     forecast field with the indicated lead time.
 */

void MdvDivideFcsts::_writeFcstField(const MdvxField &input_field,
				     const int fcst_lead_secs,
				     const string fcst_url,
				     const string output_field_name,
				     const Mdvx::master_header_t input_master_hdr) const
{
  static const string method_name = "MdvDivideFcsts::_writeFcstField()";
  
  // Create the output file

  DsMdvx mdvx;
  
  Mdvx::master_header_t master_hdr = input_master_hdr;
  
  master_hdr.time_gen = input_master_hdr.time_centroid;
  master_hdr.time_centroid = input_master_hdr.time_centroid + fcst_lead_secs;
  master_hdr.forecast_time = master_hdr.time_centroid;
  master_hdr.data_collection_type = Mdvx::DATA_FORECAST;
  master_hdr.n_fields = 0;
  master_hdr.n_chunks = 0;
  master_hdr.max_nx = 0;
  master_hdr.max_ny = 0;
  master_hdr.max_nz = 0;
  
  mdvx.setMasterHeader(master_hdr);
  
  // Create the output field

  MdvxField *field = new MdvxField(input_field);
  
  Mdvx::field_header_t field_hdr = field->getFieldHeader();
  
  field_hdr.forecast_delta = fcst_lead_secs;
  field_hdr.forecast_time = master_hdr.time_gen + fcst_lead_secs;
  STRcopy(field_hdr.field_name_long, output_field_name.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, output_field_name.c_str(),
	  MDV_SHORT_FIELD_LEN);
  
  field->setFieldHeader(field_hdr);
  
  mdvx.addField(field);
  
  // Write the output file

  mdvx.setWriteLdataInfo();
  mdvx.setWriteAsForecast();
  
  if (mdvx.writeToDir(fcst_url.c_str()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing field <"
	 << input_field.getFieldHeader().field_name
	 << "> to output URL: " << fcst_url << endl;
  }
  
}
