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
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * UpdateMdvOrigin.cc: update_mdv_times program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <iostream>
#include <signal.h>

#include <toolsa/os_config.h>
#include <didss/DsInputPath.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Params.hh"
#include "UpdateMdvOrigin.hh"
using namespace std;

// Global variables

UpdateMdvOrigin *UpdateMdvOrigin::_instance = (UpdateMdvOrigin *)NULL;

/*********************************************************************
 * Constructor
 */

UpdateMdvOrigin::UpdateMdvOrigin(int argc, char **argv)
{
  static const string method_name = "UpdateMdvOrigin::UpdateMdvTImes()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (UpdateMdvOrigin *)NULL);
  
  // Set the singleton instance pointer

  _instance = this;

  // Initialize the okay flag.

  okay = true;
  
  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  if (!_args->okay)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr  << "Problem with command line arguments." << endl;
    
    okay = false;
    
    return;
  }
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = (char *)"unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file <" << params_path << endl;
    
    okay = false;
    
    return;
  }

  // Make sure start and end times were specified

  if (_args->getStartTime() <= 0 ||
      _args->getEndTime() <= 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Must specify -starttime and -endtime on command line" << endl;
      
    okay = false;
      
    return;
  }


  // Create the data trigger

  if (_params->debug)
    cerr << "---> Creating time list trigger for URL " << _params->input_url << endl;

  
  DsTimeListTrigger *data_trigger = new DsTimeListTrigger();
  if (data_trigger->init(_params->input_url,
			 _args->getStartTime().utime(),
			 _args->getEndTime().utime()))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error initializing time list trigger:" << endl;
    cerr << "   URL: " << _params->input_url << endl;
    cerr << "   Start time: " << _args->getStartTime() << endl;
    cerr << "   End time: " << _args->getEndTime() << endl;
    
    okay = false;
    
    return;
  }
  
  _dataTrigger = data_trigger;
  
}


/*********************************************************************
 * Destructor
 */

UpdateMdvOrigin::~UpdateMdvOrigin()
{
  delete _dataTrigger;
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

UpdateMdvOrigin *UpdateMdvOrigin::Inst(int argc, char **argv)
{
  if (_instance == (UpdateMdvOrigin *)NULL)
    new UpdateMdvOrigin(argc, argv);
  
  return(_instance);
}

UpdateMdvOrigin *UpdateMdvOrigin::Inst()
{
  assert(_instance != (UpdateMdvOrigin *)NULL);
  
  return(_instance);
}


    
/*********************************************************************
* run()
*/

void UpdateMdvOrigin::run()
{
  static const string method_name = "UpdateMdvOrigin::run()";
  
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
 * _processData()
 */

bool UpdateMdvOrigin::_processData(const DateTime &data_time)
{
  static const string method_name = "UpdateMdvOrigin::_processData()";
  
  if (_params->debug)
    cerr << "*** Processing data for time: " << data_time << endl;
  
  // Read in the file

  DsMdvx mdvx;
  
  mdvx.setReadTime(Mdvx::READ_CLOSEST,
		   _params->input_url,
		   0, data_time.utime());
  
  if (mdvx.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading MDV file:" << endl;
    cerr << "   URL: " << _params->input_url << endl;
    cerr << "   Request time: " << data_time << endl;
    
    return false;
  }

  // Update the origin - and sensor lat/lon, if requested.

  if (_params->MoveSensorToo)
  {
    Mdvx::master_header_t master_hdr = mdvx.getMasterHeader();
    
    master_hdr.sensor_lat = _params->NewLatitude;
    master_hdr.sensor_lon = _params->NewLongitude;

    mdvx.setMasterHeader(master_hdr);
  }


  for (size_t i = 0; i < mdvx.getNFields(); ++i)
  {
    MdvxField *field = mdvx.getField(i);
    Mdvx::field_header_t field_hdr = field->getFieldHeader();
    
    if (_params->UpdateGridMins)
    {
      field_hdr.grid_minx = _params->NewGridMinx;
      field_hdr.grid_miny = _params->NewGridMiny;
    }
    
    field_hdr.proj_origin_lat = _params->NewLatitude;
    field_hdr.proj_origin_lon = _params->NewLongitude;

    field->setFieldHeader(field_hdr);
  } /* endfor - i */
    
  // Write out the new file

  if (mdvx.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file to URL:" << _params->output_url << endl;
    
    return false;
  }

  if (_params->debug) {
    cerr << "Wrote file: " << mdvx.getPathInUse() << endl;
  }
  
    
  return true;
}
