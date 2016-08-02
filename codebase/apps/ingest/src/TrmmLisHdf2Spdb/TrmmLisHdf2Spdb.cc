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
 * @file TrmmLisHdf2Spdb.cc
 *
 * @class TrmmLisHdf2Spdb
 *
 * TrmmLisHdf2Spdb program object.
 *  
 * @date 4/9/2009
 *
 */

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <math.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Params.hh"
#include "TrmmLisHdf2Spdb.hh"

using namespace std;

// Global variables

TrmmLisHdf2Spdb *TrmmLisHdf2Spdb::_instance =
     (TrmmLisHdf2Spdb *)NULL;


/*********************************************************************
 * Constructors
 */

TrmmLisHdf2Spdb::TrmmLisHdf2Spdb(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "TrmmLisHdf2Spdb::TrmmLisHdf2Spdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (TrmmLisHdf2Spdb *)NULL);
  
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
  char *params_path = new char(strlen("unknown") + 1);
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

TrmmLisHdf2Spdb::~TrmmLisHdf2Spdb()
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
 * Inst()
 */

TrmmLisHdf2Spdb *TrmmLisHdf2Spdb::Inst(int argc, char **argv)
{
  if (_instance == (TrmmLisHdf2Spdb *)NULL)
    new TrmmLisHdf2Spdb(argc, argv);
  
  return(_instance);
}

TrmmLisHdf2Spdb *TrmmLisHdf2Spdb::Inst()
{
  assert(_instance != (TrmmLisHdf2Spdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool TrmmLisHdf2Spdb::init()
{
  static const string method_name = "TrmmLisHdf2Spdb::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // initialize process registration

  if (_params->trigger_mode != Params::FILE_LIST)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void TrmmLisHdf2Spdb::run()
{
  static const string method_name = "TrmmLisHdf2Spdb::run()";
  
  while (!_dataTrigger->endOfData())
  {
    TriggerInfo trigger_info;
    
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger information" << endl;
      cerr << "Trying again...." << endl;
      
      continue;
    }    
    _processFile(trigger_info.getFilePath());

  } /* endwhile - !_dataTrigger->endOfData() */

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _initTrigger()
 */

bool TrmmLisHdf2Spdb::_initTrigger()
{
  static const string method_name = "TrmmLisHdf2Spdb;:_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::FILE_LIST :
  {
    DsFileListTrigger *trigger = new DsFileListTrigger();
    if (trigger->init(_args->getFileList()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing FILE_LIST trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::INPUT_DIR :
  {
    DsInputDirTrigger *trigger = new DsInputDirTrigger();
    if (trigger->init(_params->input_dir,
		      _params->input_substring,
		      false, PMU_auto_register, false,
		      _params->exclude_substring) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INPUT_DIR trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::INPUT_DIR_RECURSE :
  {
    DsInputDirTrigger *trigger = new DsInputDirTrigger();
    if (trigger->init(_params->input_dir,
		      _params->input_substring,
		      false, PMU_auto_register, true,
		      _params->exclude_substring) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INPUT_DIR_RECURSE trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::LATEST_DATA :
  {
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_dir,
		      _params->max_valid_secs,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

return true;
}

    
/*********************************************************************
 * _processFile()
 */

bool TrmmLisHdf2Spdb::_processFile(const string &input_file_name)
{
  static const string method_name = "TrmmLisHdf2Spdb::_processFile()";
  
  PMU_auto_register("Processing file...");

  if (_params->debug || _params->verbose)
    cerr << endl << endl << "*** Processing file: " << input_file_name << endl;
  
  // Create and initialize the HDF file object

  if (_params->verbose)
    cerr << "Creating HDF file object" << endl;
  
  HdfFile hdf_file(input_file_name, _params->debug, _params->verbose);
  
  if (_params->verbose)
    cerr << "Initializing HDF file" << endl;
  
  if (!hdf_file.init())
    return false;

  // Process the flashes

  if (_params->output_flashes)
  {
    if (!_processFlashes(hdf_file))
      return false;
  }
  
  // Process the groups

  if (_params->output_groups)
  {
    if (!_processGroups(hdf_file))
      return false;
  }
  
  return true;
}


/*********************************************************************
 * _processFlashes()
 */

bool TrmmLisHdf2Spdb::_processFlashes(HdfFile &hdf_file) const
{
  static const string method_name = "TrmmLisHdf2Spdb::_processFlashes()";
  
  // Retrieve the flashes

  vector< LTG_strike_t > flashes;
  
  if (!hdf_file.getFlashes(flashes))
    return false;
  
  // If there are no flashes, return since there is nothing more to do.
  // Note that this is not an error condition.

  if (flashes.size() == 0)
  {
    if (_params->verbose)
      cerr << "No flashes found" << endl;
      
    return true;
  }
  
  if (_params->verbose)
  {
    cerr << "Flashes:" << endl;
    
    for (size_t i = 0; i < flashes.size(); ++i)
      LTG_print_strike(stderr, &flashes[i]);
  }
  
  // Write the flashes to the SPDB database

  if (!_writeFlashes(flashes))
    return false;
  
  return true;
}


/*********************************************************************
 * _processGroups()
 */

bool TrmmLisHdf2Spdb::_processGroups(HdfFile &hdf_file) const
{
  static const string method_name = "TrmmLisHdf2Spdb::_processGroups()";
  
  // Retrieve the groups

  vector< LtgGroup > groups;
  
  if (!hdf_file.getGroups(groups))
    return false;
  
  // If there are no groups, return since there is nothing more to do.
  // Note that this is not an error condition.

  if (groups.size() == 0)
  {
    if (_params->verbose)
      cerr << "No groups found" << endl;
      
    return true;
  }
    
  if (_params->verbose)
  {
    cerr << "Groups:" << endl;
    
    vector< LtgGroup >::const_iterator group;
    for (group = groups.begin(); group != groups.end(); ++group)
      group->print(stderr);
  }
  
  // Write the groups to the SPDB database

  if (!_writeGroups(groups))
    return false;
  
  return true;
}


/*********************************************************************
 * _writeFlashes()
 */

bool TrmmLisHdf2Spdb::_writeFlashes(const vector< LTG_strike_t > &flashes) const
{
  static const string method_name = "TrmmLisHdf2Spdb::_writeFlashes()";
  
  // Set up the output database

  DsSpdb spdb;
  
  spdb.setPutMode(Spdb::putModeAdd);
  
  // Add each of the flashes to the put buffer

  vector< LTG_strike_t >::const_iterator flash;
  
  for (flash = flashes.begin(); flash != flashes.end(); ++flash)
  {
    LTG_strike_t output_flash = *flash;
    LTG_to_BE(&output_flash);
    
    spdb.addPutChunk(0,
		     flash->time,
		     flash->time + _params->flashes_output_info.expire_secs,
		     sizeof(output_flash),
		     &output_flash);
  } /* endfor - flash */
  
  // Write the put buffer to the database

  if (spdb.put(_params->flashes_output_info.url,
	       SPDB_LTG_ID, SPDB_LTG_LABEL) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing flashes to output URL: "
	 << _params->flashes_output_info.url << endl;
    cerr << spdb.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _writeGroups()
 */

bool TrmmLisHdf2Spdb::_writeGroups(const vector< LtgGroup > &groups) const
{
  static const string method_name = "TrmmLisHdf2Spdb::_writeGroups()";
  
  // Set up the output database

  DsSpdb spdb;
  
  spdb.setPutMode(Spdb::putModeAdd);
  
  // Add each of the groups to the put buffer

  vector< LtgGroup >::const_iterator group;
  
  for (group = groups.begin(); group != groups.end(); ++group)
  {
    group->assemble();
    
    spdb.addPutChunk(0,
		     group->getTime(),
		     group->getTime() + _params->groups_output_info.expire_secs,
		     group->getBufLen(),
		     group->getBufPtr());
  } /* endfor - group */
  
  // Write the put buffer to the database

  if (spdb.put(_params->groups_output_info.url,
	       SPDB_LTG_GROUP_ID, SPDB_LTG_GROUP_LABEL) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing groups to output URL: "
	 << _params->groups_output_info.url << endl;
    cerr << spdb.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}
