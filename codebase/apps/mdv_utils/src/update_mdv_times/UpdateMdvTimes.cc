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
//   $Date: 2016/03/04 02:22:16 $
//   $Id: UpdateMdvTimes.cc,v 1.14 2016/03/04 02:22:16 dixon Exp $
//   $Revision: 1.14 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * UpdateMdvTimes.cc: update_mdv_times program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <signal.h>

#include <toolsa/os_config.h>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Params.hh"
#include "UpdateMdvTimes.hh"

using namespace std;

// Global variables

UpdateMdvTimes *UpdateMdvTimes::_instance = NULL;

/*********************************************************************
 * Constructor
 */

UpdateMdvTimes::UpdateMdvTimes(int argc, char **argv)
{
  static const string method_name = "UpdateMdvTimes::UpdateMdvTimes()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (UpdateMdvTimes *)NULL);
  
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
    cerr << "Problem with TDRP parameters in file <"
	 << params_path << ">" << endl;
    
    okay = false;
    
    return;
  }

  // Make sure start and end times were specified in multiple mode
  
  if (_params->mode == Params::MULTIPLE) {
    if (_args->getStartTime() <= 0 ||
	_args->getEndTime() <= 0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "MULTIPLE mode" << endl;
	cerr << " Must specify -starttime and -endtime on command line" << endl;
	okay = false;
	return;
      }
  }
  
  // Make sure path and mod time were specified in single mode
  
  if (_params->mode == Params::SINGLE) {
    if (_args->getSinglePath().size() == 0 ||
	_args->getModTime() <= 0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "SINGLE mode" << endl;
	cerr << " Must specify -path and -modtime on command line" << endl;
	okay = false;
	return;
      }
  }
  
  // Make sure that opposing parameters aren't set

  if (_params->update_forecast_time && _params->set_forecast_delta)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error in parameter file" << endl;
    cerr << "Cannot set both update_forecast_time and set_forecast_delta to true" << endl;
    cerr << "Update parameter file and rerun" << endl;
    
    okay = false;
    return;
  }
  
}


/*********************************************************************
 * Destructor
 */

UpdateMdvTimes::~UpdateMdvTimes()
{
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

UpdateMdvTimes *UpdateMdvTimes::Inst(int argc, char **argv)
{
  if (_instance == (UpdateMdvTimes *)NULL)
    new UpdateMdvTimes(argc, argv);
  
  return(_instance);
}

UpdateMdvTimes *UpdateMdvTimes::Inst()
{
  assert(_instance != (UpdateMdvTimes *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run()
 */

int UpdateMdvTimes::run()
{
  static const string method_name = "UpdateMdvTimes::run()";
  
  int iret = 0;

  if (_params->mode == Params::SINGLE) {

    // mode is SINGLE

    if (_params->debug >= Params::DEBUG_VERBOSE)
      cerr << "Using SINGLE mode" << endl;
    
    if (_processFile(_args->getSinglePath())) {
      iret = -1;
    }
    
  } else {
    
    // mode is MULTIPLE
    
    if (_params->debug >= Params::DEBUG_VERBOSE)
      cerr << "Using MULTIPLE mode" << endl;
    
    // Create the input path object
  
    if (_params->debug)
      cerr << "---> Creating time list trigger object for URL " << _params->input_url << endl;
  
    DsTimeListTrigger trigger;
    
    if (trigger.init(_params->input_url,
		     _args->getStartTime(), _args->getEndTime()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating time list trigger:" << endl;
      cerr << "   URL: " << _params->input_url << endl;
      cerr << "   start time: " << DateTime::str(_args->getStartTime()) << endl;
      cerr << "   end time: " << DateTime::str(_args->getEndTime()) << endl;
      
      return -1;
    }
    
    DateTime trigger_time;
    
    while (trigger.nextIssueTime(trigger_time) == 0)
      if (_processFile(trigger_time))
	return -1;
  }

  return iret;

}

  
/*********************************************************************
 * process file
 */

int UpdateMdvTimes::_processFile(const string &file_path)
{
  static const string method_name = "UpdateMdvTimes::_processFile()";
  
  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "Processing data from file: " << file_path << endl;

  // Read in the file

  DsMdvx mdvx;
  mdvx.setReadPath(file_path);
  if (mdvx.readVolume()) {
    cerr << "ERROR: " << method_name << endl;
    cerr << "  Cannot read in file:" << endl;
    cerr << "      Path: " << file_path << endl;
    cerr << "  " << mdvx.getErrStr() << endl;
    return -1;
  }
  
  if (_params->debug) {
    cerr << "====>>  Read file: " << mdvx.getPathInUse() << endl;
  }

  return _processFile(mdvx);
}


int UpdateMdvTimes::_processFile(const DateTime trigger_time)
{
  static const string method_name = "UpdateMdvTimes::_processFile()";
  
  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "Processing data for time: " << trigger_time << endl;

  // Read in the file

  DsMdvx mdvx;
  mdvx.setReadTime(Mdvx::READ_CLOSEST,
		   _params->input_url,
		   0, trigger_time.utime());
  if (mdvx.readVolume()) {
    cerr << "ERROR: " << method_name << endl;
    cerr << "  Cannot read in file:" << endl;
    cerr << "      URL: " << _params->input_url << endl;
    cerr << "      time: " << trigger_time << endl;
    cerr << "  " << mdvx.getErrStr() << endl;
    return -1;
  }
  
  if (_params->debug) {
    cerr << "====>>  Read file: " << mdvx.getPathInUse() << endl;
  }

  return _processFile(mdvx);
}


int UpdateMdvTimes::_processFile(DsMdvx &mdvx)
{
  static const string method_name = "UpdateMdvTimes::_processFile()";
  
  // Update the time fields
  
  long time_offset = _params->time_offset;
  long forecast_time_offset = _params->forecast_time_offset;
  Mdvx::master_header_t mhdr = mdvx.getMasterHeader();
  if (_params->mode == Params::SINGLE) {
    time_t modTime = _args->getModTime();
    time_offset = modTime - mhdr.time_centroid;
  }
  
  if (_params->debug >= Params::DEBUG_VERBOSE)
  {
    cerr << "  Original master header times:" << endl;
    cerr << "     time_gen = " << DateTime::str(mhdr.time_gen) << endl;
    cerr << "     time_begin = " << DateTime::str(mhdr.time_begin) << endl;
    cerr << "     time_end = " << DateTime::str(mhdr.time_end) << endl;
    cerr << "     time_centroid = "
	 << DateTime::str(mhdr.time_centroid) << endl;
    cerr << "     time_expire = " << DateTime::str(mhdr.time_expire) << endl;
  }
  
  if (_params->reset_gen_time) 
      mhdr.time_gen = mhdr.time_centroid;
  mhdr.time_begin += time_offset;
  mhdr.time_end += time_offset;
  mhdr.time_centroid += time_offset;
  if(mhdr.time_expire != 0) 
      mhdr.time_expire += time_offset;
  mdvx.setMasterHeader(mhdr);
  
  if (_params->debug >= Params::DEBUG_VERBOSE)
  {
    cerr << "  Updated master header times:" << endl;
    cerr << "     time_gen = " << DateTime::str(mhdr.time_gen) << endl;
    cerr << "     time_begin = " << DateTime::str(mhdr.time_begin) << endl;
    cerr << "     time_end = " << DateTime::str(mhdr.time_end) << endl;
    cerr << "     time_centroid = "
	 << DateTime::str(mhdr.time_centroid) << endl;
    cerr << "     time_expire = " << DateTime::str(mhdr.time_expire) << endl;
  }
  
  for (int i = 0; i < mhdr.n_fields; i++)
  {
      MdvxField *fld = mdvx.getField(i);
      Mdvx::field_header_t fhdr = fld->getFieldHeader();

      if (_params->debug >= Params::DEBUG_VERBOSE)
      {
	cerr << "    Original " << fhdr.field_name << " field header times;" << endl;
	cerr << "       forecast_time = "
	     << DateTime::str(fhdr.forecast_time) << endl;
	cerr << "       forecast_delta = " << fhdr.forecast_delta << endl;
      }
      
      if(_params->update_forecast_time) 
      {
	fhdr.forecast_time += forecast_time_offset;
	fhdr.forecast_delta = fhdr.forecast_time - mhdr.time_gen;
      }
      else if (_params->set_forecast_delta)
      {
	fhdr.forecast_delta = _params->forecast_delta_value;
	fhdr.forecast_time = mhdr.time_gen + fhdr.forecast_delta;
      }
      else if (fhdr.forecast_time != 0)
      {
	fhdr.forecast_time += time_offset;
	fhdr.forecast_delta = fhdr.forecast_time - mhdr.time_gen;
      }

      fld->setFieldHeader(fhdr);

      if (_params->debug >= Params::DEBUG_VERBOSE)
      {
	cerr << "    Updated " << fhdr.field_name << " field header times;" << endl;
	cerr << "       forecast_time = "
	     << DateTime::str(fhdr.forecast_time) << endl;
	cerr << "       forecast_delta = " << fhdr.forecast_delta << endl;
      }
      
  }

  if(_params->write_as_forecast) 
      mdvx.setWriteAsForecast();  

  // Write out the updated file

  if (mdvx.writeToDir(_params->output_url)) {
    cerr << "ERROR: " << method_name << endl;
    cerr << "  Cannot write out file to dir: " << _params->output_url << endl;
    cerr << "  " << mdvx.getErrStr() << endl;
    return -1;
  }

  if (_params->debug) {
    cerr << "====>> Wrote file: " << mdvx.getPathInUse() << endl;
  }

  return 0;

}
