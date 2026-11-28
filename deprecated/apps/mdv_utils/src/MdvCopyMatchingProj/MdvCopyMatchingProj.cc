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
 * @file MdvCopyMatchingProj.cc
 *
 * @class MdvCopyMatchingProj
 *
 * MdvCopyMatchingProj program object.
 *  
 * @date 6/29/2012
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

#include <os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "MdvCopyMatchingProj.hh"
#include "Params.hh"

using namespace std;

// Global variables

const double MdvCopyMatchingProj::TOLERANCE = 0.001;

MdvCopyMatchingProj *MdvCopyMatchingProj::_instance =
     (MdvCopyMatchingProj *)NULL;


/*********************************************************************
 * Constructors
 */

MdvCopyMatchingProj::MdvCopyMatchingProj(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvCopyMatchingProj::MdvCopyMatchingProj()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvCopyMatchingProj *)NULL);
  
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

MdvCopyMatchingProj::~MdvCopyMatchingProj()
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

MdvCopyMatchingProj *MdvCopyMatchingProj::Inst(int argc, char **argv)
{
  if (_instance == (MdvCopyMatchingProj *)NULL)
    new MdvCopyMatchingProj(argc, argv);
  
  return(_instance);
}

MdvCopyMatchingProj *MdvCopyMatchingProj::Inst()
{
  assert(_instance != (MdvCopyMatchingProj *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool MdvCopyMatchingProj::init()
{
  static const string method_name = "MdvCopyMatchingProj::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
 
  // Initialize the desired projection

  if (!_initProj())
    return false;
  
  // Initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void MdvCopyMatchingProj::run()
{
  static const string method_name = "MdvCopyMatchingProj::run()";
  
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
    
    _processData(trigger_info.getIssueTime());

  } /* endwhile - !_dataTrigger->endOfData() */

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _initProj()
 */

bool MdvCopyMatchingProj::_initProj()
{
  static const string method_name = "MdvCopyMatchingProj;:_initProj()";
  
  switch (_params->proj.proj_type)
  {
  case Params::PROJ_POLAR_RADAR :
    _proj.initPolarRadar(_params->proj.origin_lat, _params->proj.origin_lon,
			 _params->proj.nx, _params->proj.ny, 1,
			 _params->proj.dx, _params->proj.dy, 1.0,
			 _params->proj.minx, _params->proj.miny, 0.0);
    break;
  } /* endswitch - _params->trigger_mode */

  return true;
}

    
/*********************************************************************
 * _initTrigger()
 */

bool MdvCopyMatchingProj::_initTrigger()
{
  static const string method_name = "MdvCopyMatchingProj;:_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_url,
		      _params->max_valid_secs,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger" << endl;
      cerr << "   URL: " << _params->input_url << endl;
      cerr << "   max valid secs: "
	   << _params->max_valid_secs << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::TIME_LIST :
  {
    DateTime start_time = _args->getStartTime();
    DateTime end_time = _args->getEndTime();
    
    if (start_time == DateTime::NEVER ||
	end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "In TIME_LIST mode, start and end times must be included on the command line." << endl;
      cerr << "Update command line and try again." << endl;
      
      return false;
    }
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->input_url,
		      start_time.utime(), end_time.utime()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger" << endl;
      cerr << "   URL: " << _params->input_url << endl;
      cerr << "   start time: " << start_time << endl;
      cerr << "   end time: " << end_time << endl;
      
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

bool MdvCopyMatchingProj::_processData(const DateTime &trigger_time)
{
  static const string method_name = "MdvCopyMatchingProj::_processData()";
  
  PMU_auto_register("Processing data...");

  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  // Read in the file

  DsMdvx mdvx;
  
  if (_params->only_write_specified_field)
    mdvx.addReadField(_params->field_name);
  
  mdvx.setReadTime(Mdvx::READ_CLOSEST,
		   _params->input_url,
		   0, trigger_time.utime());
  
  if (mdvx.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading MDV file:" << endl;
    cerr << "    URL = " << _params->input_url << endl;
    cerr << "    data time = " << trigger_time << endl;
    cerr << mdvx.getErrStr() << endl;
    
    return false;
  }
  
  if (_params->verbose)
    cerr << "   File read in" << endl;
  
  // Pull the desired field from the file

  MdvxField *field;
  
  if ((field = mdvx.getField(_params->field_name)) == 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Mdv file doesn't contain " << _params->field_name
	 << " field" << endl;
    cerr << "Skipping file" << endl;
    
    return true;
  }
  
  if (_params->verbose)
    cerr << "   " << _params->field_name << " field found" << endl;
  
  // Check the projection

  MdvxPjg proj(mdvx.getMasterHeader(), field->getFieldHeader());
  
  if (proj.getProjType() != _proj.getProjType())
  {
    cerr << "WARNING: Projection doesn't match -- proj_type" << endl;
    return true;
  }
  

  if (proj.getNx() != _proj.getNx() ||
      proj.getNy() != _proj.getNy())
  {
    cerr << "WARNING: Projection doesn't match -- nx/ny" << endl;
    return true;
  }

  if (fabs(proj.getDx() - _proj.getDx()) > TOLERANCE ||
      fabs(proj.getDy() - _proj.getDy()) > TOLERANCE)
  {
    cerr << "WARNING: Projection doesn't match -- dx/dy" << endl;
    return true;
  }

  if (fabs(proj.getMinx() - _proj.getMinx()) > TOLERANCE ||
      fabs(proj.getMiny() - _proj.getMiny()) > TOLERANCE)
  {
    cerr << "WARNING: Projection doesn't match -- minx/miny" << endl;
    return true;
  }
  
  if (proj.getProjType() == Mdvx::PROJ_POLAR_RADAR &&
      (fabs(proj.getOriginLat() - _proj.getOriginLat()) > TOLERANCE ||
       fabs(proj.getOriginLon() - _proj.getOriginLon()) > TOLERANCE))
  {
    cerr << "WARNING: Projection doesn't match -- proj origin" << endl;
    return true;
  }
  
  if (_params->verbose)
    cerr << "   Projection matches" << endl;
  
  // If we get here, we need to copy the file so write it to the output URL

  if (mdvx.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing file to URL: " << _params->output_url << endl;
    cerr << mdvx.getErrStr() << endl;
    
    return false;
  }
  
  if (_params->verbose)
    cerr << "   File written to: " << _params->output_url << endl;
  
  return true;
}
