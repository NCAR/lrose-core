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
//   $Id: UpdateMdvVlevelTypes.cc,v 1.3 2016/03/04 02:22:13 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * UpdateMdvVlevelTypes: UpdateMdvVlevelTypes program object.
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
#include <dsdata/DsFileListTrigger.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>

#include "UpdateMdvVlevelTypes.hh"
#include "Params.hh"

using namespace std;


// Global variables

UpdateMdvVlevelTypes *UpdateMdvVlevelTypes::_instance =
     (UpdateMdvVlevelTypes *)NULL;



/*********************************************************************
 * Constructor
 */

UpdateMdvVlevelTypes::UpdateMdvVlevelTypes(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "UpdateMdvVlevelTypes::UpdateMdvVlevelTypes()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (UpdateMdvVlevelTypes *)NULL);
  
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

UpdateMdvVlevelTypes::~UpdateMdvVlevelTypes()
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

UpdateMdvVlevelTypes *UpdateMdvVlevelTypes::Inst(int argc, char **argv)
{
  if (_instance == (UpdateMdvVlevelTypes *)NULL)
    new UpdateMdvVlevelTypes(argc, argv);
  
  return(_instance);
}

UpdateMdvVlevelTypes *UpdateMdvVlevelTypes::Inst()
{
  assert(_instance != (UpdateMdvVlevelTypes *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool UpdateMdvVlevelTypes::init()
{
  static const string method_name = "UpdateMdvVlevelTypes::init()";
  
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
  
  case Params::FILE_LIST :
  {
    DsFileListTrigger *trigger = new DsFileListTrigger();
    if (trigger->init(_args->getInputFileList()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing FILE_LIST trigger." << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  } /* endswitch - _params->trigger_mode */
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void UpdateMdvVlevelTypes::run()
{
  static const string method_name = "UpdateMdvVlevelTypes::run()";
  
  TriggerInfo trigger_info;

  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_info))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: " 
	   << DateTime(trigger_info.getIssueTime()) << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _processData() - Process the data for the given time.
 */

bool UpdateMdvVlevelTypes::_processData(TriggerInfo &trigger_info)
{
  static const string method_name = "UpdateMdvVlevelTypes::_processData()";
  
  if (_params->debug)
    cerr << endl
	 << "*** Processing data for time: " 
	 << DateTime(trigger_info.getIssueTime()) << endl;
  
  // Read in the input file

  DsMdvx input_mdv;
  
  if (!_readMdvFile(input_mdv, trigger_info))
    return false;
  
  // Extract each of the fields in the MDV file and write them to the
  // output GRIB file.

  Mdvx::master_header_t master_hdr = input_mdv.getMasterHeader();
  
  for (int field_num = 0; field_num < master_hdr.n_fields; ++field_num)
  {
    MdvxField *field = input_mdv.getField(field_num);
    if (field == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error extracting field number " << field_num
	   << " from MDV file." << endl;
      cerr << "Skipping field....." << endl;
      
      continue;
    }
    
  } /* endfor - field_num */
  
  return true;
}


/*********************************************************************
 * _readMdvFile() - Read the MDV file for the given time.
 */

bool UpdateMdvVlevelTypes::_readMdvFile(DsMdvx &input_mdv,
			    TriggerInfo &trigger_info) const
{
  static const string method_name = "UpdateMdvVlevelTypes::_readMdvFile()";
  
  // Set up the read request

  if(_params->trigger_mode == Params::FILE_LIST) 
  {
    input_mdv.setReadPath(trigger_info.getFilePath());
  }
  else 
  {
    input_mdv.setReadTime(Mdvx::READ_CLOSEST,
			  _params->input_url,
			  0, trigger_info.getIssueTime());
  }
    

  if (_params->debug)
    input_mdv.printReadRequest(cerr);
  
  // Read the MDV file

  if (input_mdv.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input MDV file:" << endl;
    cerr << "   URL: " << _params->input_url << endl;
    cerr << "   Request time: " << DateTime::str(trigger_info.getIssueTime()) << endl;
    cerr << "   msg: " << input_mdv.getErrStr() << endl;
    
    return false;
  }
  
  // Update the vlevel types

  Mdvx::master_header_t master_hdr = input_mdv.getMasterHeader();
  master_hdr.vlevel_type = _params->new_vlevel_type;
  input_mdv.setMasterHeader(master_hdr);
  
  for (int field_num = 0; field_num < master_hdr.n_fields; ++field_num)
  {
    MdvxField *field = input_mdv.getField(field_num);
    
    Mdvx::field_header_t field_hdr = field->getFieldHeader();
    field_hdr.vlevel_type = _params->new_vlevel_type;
    field->setFieldHeader(field_hdr);

    Mdvx::vlevel_header_t vlevel_hdr = field->getVlevelHeader();
    for (int i = 0; i < field_hdr.nz; ++i)
      vlevel_hdr.type[i] = _params->new_vlevel_type;
    field->setVlevelHeader(vlevel_hdr);
    
  } /* endfor - field_num */
  
  // Write the output file

  if (input_mdv.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file to URL: " << _params->output_url << endl;
    
    return false;
  }
  
  return true;
}
