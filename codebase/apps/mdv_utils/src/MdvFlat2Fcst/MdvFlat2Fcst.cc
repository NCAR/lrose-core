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
//   $Id: MdvFlat2Fcst.cc,v 1.6 2016/03/04 02:22:11 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvFlat2Fcst: MdvFlat2Fcst program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <signal.h>
#include <math.h>
#include <string>
#include <string.h>

#include <toolsa/os_config.h>
#include <dsdata/DsMultipleTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "MdvFlat2Fcst.hh"
#include "Params.hh"

using namespace std;

// Global variables

MdvFlat2Fcst *MdvFlat2Fcst::_instance =
     (MdvFlat2Fcst *)NULL;


/*********************************************************************
 * Constructor
 */

MdvFlat2Fcst::MdvFlat2Fcst(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvFlat2Fcst::MdvFlat2Fcst()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvFlat2Fcst *)NULL);
  
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

MdvFlat2Fcst::~MdvFlat2Fcst()
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

MdvFlat2Fcst *MdvFlat2Fcst::Inst(int argc, char **argv)
{
  if (_instance == (MdvFlat2Fcst *)NULL)
    new MdvFlat2Fcst(argc, argv);
  
  return(_instance);
}

MdvFlat2Fcst *MdvFlat2Fcst::Inst()
{
  assert(_instance != (MdvFlat2Fcst *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvFlat2Fcst::init()
{
  static const string method_name = "MdvFlat2Fcst::init()";
  
  // Initialize the input processors

  for (int i = 0; i < _params->input_fcsts_n; ++i)
  {
    if (_params->debug)
      cerr << "Processing input fcst for url: "
	   << _params->_input_fcsts[i].url << endl;
    
    InputFcst fcst(_params->_input_fcsts[i].url,
		   _params->_input_fcsts[i].fcst_lead_secs,
		   _params->_input_fcsts[i].fcst_stored_by_gen_time,
		   _params->debug);
    
    char *token_string = strdup(_params->_input_fcsts[i].field_names);
    
    char *token = strtok(token_string, " ");
    while (1)
    {
      if (token == 0) break;
      for (unsigned int j = 0; j < strlen(token); ++j)
	if (token[j] == '^') token[j] = ' ';
      if (_params->debug)
	cerr << "   Adding fcst field: " << token << endl;
      fcst.addField(token);
      token = strtok(NULL, " ");
    }
    
    free(token_string);
    
    if (fcst.getNumFields() != _params->fcst_field_names_n)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error in parameter file" << endl;
      cerr << "input_fcst for URL " << _params->_input_fcsts[i].url
	   << " has " << fcst.getNumFields() << " fields specified" << endl;
      cerr << "However, fcst_field_names has "
	   << _params->fcst_field_names_n << " fields" << endl;
      cerr << "Fix parameter file and start again" << endl;
      
      return false;
    }
    
    _inputFcsts.push_back(fcst);
    
  } /* endfor - i */

  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::REALTIME :
  {
    if (_params->debug)
      cerr << "Initializing REALTIME trigger" << endl;
    
    DsMultipleTrigger *trigger = new DsMultipleTrigger();
    
    if (!trigger->initRealtime(-1,
			       PMU_auto_register))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing REALTIME trigger" << endl;
      cerr << trigger->getErrString() << endl;
      
      return false;
    }

    for (int i = 0; i < _params->input_fcsts_n; ++i)
      trigger->add(_params->_input_fcsts[i].url,
		   DsMultTrigElem::OPTIONAL);
    
    trigger->setTriggerType(DsMultipleTrigger::TRIGGER_ANY_ONE);
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::ARCHIVE :
  {
    if (_params->debug)
      cerr << "Initializing ARCHIVE trigger" << endl;
    
    time_t start_time =
      DateTime::parseDateTime(_params->time_list.start_time);
    time_t end_time
      = DateTime::parseDateTime(_params->time_list.end_time);
    
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for ARCHIVE trigger: " <<
	_params->time_list.start_time << endl;
      
      return false;
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for ARCHIVE trigger: " <<
	_params->time_list.end_time << endl;
      
      return false;
    }
    
    DsMultipleTrigger *trigger = new DsMultipleTrigger();

    if (!trigger->initArchive(start_time, end_time))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing ARCHIVE trigger" << endl;
      cerr << "    Start time: " << _params->time_list.start_time <<
	endl;
      cerr << "    End time: " << _params->time_list.end_time << endl;
      cerr << trigger->getErrString() << endl;
      
      return false;
    }
    
    for (int i = 0; i < _params->input_fcsts_n; ++i)
      trigger->add(_params->_input_fcsts[i].url,
		   DsMultTrigElem::OPTIONAL);
    
//    trigger->setTriggerType(DsMultipleTrigger::TRIGGER_ANY_ONE);
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */
  
  // initialize process registration

  if (_params->trigger_mode == Params::REALTIME)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void MdvFlat2Fcst::run()
{
  static const string method_name = "MdvFlat2Fcst::run()";
  
  while (!_dataTrigger->endOfData())
  {
    TriggerInfo trigger;
  
    if (_dataTrigger->next(trigger) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: "
	   << trigger.getIssueTime() << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _processData() - Process the data for the given time.
 *
 * Returns TRUE on success, FALSE on failure.
 */

bool MdvFlat2Fcst::_processData(const TriggerInfo &trigger)
{
  static const string method_name = "MdvFlat2Fcst::_processData()";
  
  if (_params->debug)
  {
    cerr << "\n*** Processing data for trigger: " << endl;
    trigger.print(cerr);
  }
  
  // Process the triggered data

  vector< InputFcst >::iterator fcst;
  
  for (fcst = _inputFcsts.begin(); fcst != _inputFcsts.end(); ++fcst)
  {
    // Make sure this is the URL that triggered

    if (trigger.getFilePath() != fcst->getUrl())
      continue;
    
    // Read the forecast from the input URL.  This method also updates
    // the times in the file to be consistent with the forecast.

    DsMdvx fcst_mdvx;
    
    if (!fcst->readData(trigger.getIssueTime(), fcst_mdvx))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading data" << endl;
      continue;
    }
    
    // Update the master header to reflect that this is a forecast file

    Mdvx::master_header_t master_hdr = fcst_mdvx.getMasterHeader();
    master_hdr.data_collection_type = Mdvx::DATA_FORECAST;
    fcst_mdvx.setMasterHeader(master_hdr);
    
    // Update the field names in the file

    _updateFieldNames(fcst_mdvx);
    
    // Write the forecast to the output URL.

    fcst_mdvx.setWriteAsForecast();
    
    if (fcst_mdvx.writeToDir(_params->output_url) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing forecast to output URL: "
	   << _params->output_url << endl;
      cerr << fcst_mdvx.getErrStr() << endl;
      
      continue;
    }
    cerr << "    Wrote data to " << _params->output_url << endl;

  } /* endfor - fcst */
  
  return true;
}


/*********************************************************************
 * _updateFieldNames() - Update the field names in the output file.
 */

void MdvFlat2Fcst::_updateFieldNames(Mdvx &fcst_mdvx)
{
  for (size_t i = 0; i < fcst_mdvx.getNFields(); ++i)
  {
    MdvxField *field = fcst_mdvx.getField(i);
    
    Mdvx::field_header_t field_hdr = field->getFieldHeader();
    
    STRcopy(field_hdr.field_name_long, _params->_fcst_field_names[i],
	    MDV_LONG_FIELD_LEN);
    STRcopy(field_hdr.field_name, _params->_fcst_field_names[i],
	    MDV_SHORT_FIELD_LEN);
    
    field->setFieldHeader(field_hdr);
  } /* endfor - i */
}
