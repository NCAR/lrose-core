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
//   $Date: 2016/03/06 23:53:42 $
//   $Id: KavLtg2Spdb.cc,v 1.13 2016/03/06 23:53:42 dixon Exp $
//   $Revision: 1.13 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * KavLtg2Spdb.cc: Program to convert Kavouras lightning data into
 *                 SPDB format.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 1998
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <signal.h>
#include <string>

#include <rapformats/KavLtgFile.hh>
#include <Spdb/LtgSpdbBuffer.hh>
#include <toolsa/InputDir.hh>
#include <toolsa/ldata_info.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>

#include "Params.hh"
#include "Args.hh"
#include "KavLtg2Spdb.hh"
using namespace std;


// Global variables

KavLtg2Spdb *KavLtg2Spdb::_instance = (KavLtg2Spdb *)NULL;

// Global constants

const int FOREVER = true;


/*********************************************************************
 * Constructor
 */

KavLtg2Spdb::KavLtg2Spdb(int argc, char **argv)
{
  const string method_name = "KavLtg2Spdb::Constructor";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (KavLtg2Spdb *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  if (!_args->okay)
  {
    cerr << "ERROR: "<< method_name << endl;
    cerr << "Problem with command line arguments." << endl;
    
    okay = false;
    
    return;
  }
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file <" << params_path << ">" <<
      endl;
    
    okay = false;
    
    return;
  }

  // Create the input objects

  _inputDir = new InputDir(_params->input_dir,
			   _params->input_substring,
			   _params->process_old_data);
  
  // Initialize the beginning latest data time used for speeding up restart
  // times

  _beginLatestDataTime = 0;
  
  if (_params->process_old_data)
  {
    LDATA_handle_t handle;
    
    LDATA_init_handle(&handle, _progName, _params->debug);
    
    if (LDATA_info_read(&handle, _params->output_url, -1) == 0)
      _beginLatestDataTime = handle.info.latest_time;
    
    LDATA_free_handle(&handle);
  }
  
  cout << "_beginLatestDataTime = " << DateTime(_beginLatestDataTime) << endl;

  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // Set the singleton instance pointer

  _instance = this;
}


/*********************************************************************
 * Destructor
 */

KavLtg2Spdb::~KavLtg2Spdb()
{
  // Free contained objects

  delete _params;
  delete _args;

  if (_inputDir != (InputDir *)NULL)
    delete _inputDir;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

KavLtg2Spdb *KavLtg2Spdb::Inst(int argc, char **argv)
{
  if (_instance == (KavLtg2Spdb *)NULL)
    new KavLtg2Spdb(argc, argv);
  
  return(_instance);
}

KavLtg2Spdb *KavLtg2Spdb::Inst()
{
  assert(_instance != (KavLtg2Spdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run()
 */

void KavLtg2Spdb::run()
{
  const string method_name = "KavLtg2Spdb::run()";
  
  int startup_flag = TRUE;
  
  // Create local objects

  KavLtgFile ltg_file(_params->debug);
  LtgSpdbBuffer spdb_buffer(_params->debug);
  
  // calculate oldest strike time

  time_t oldest_strike_time = 0;
  if ((_params->runmode == Params::ARCHIVE) && (_params->process_interval > 0))
  {
    oldest_strike_time = time(0) - _params->process_interval;
    if (_params->debug)
      cerr << "oldest_strike_time = " << DateTime(oldest_strike_time) << endl;    
  }
    
  while (FOREVER)
  {
    // Register with the process mapper

    PMU_auto_register("Checking for new files");
    
    // Process any new files

    char *input_filename;
    
    while ((input_filename = _inputDir->getNextFilename(TRUE)) != NULL)
    {
      char procmap_string[BUFSIZ];
      
      // Register with the process mapper

      sprintf(procmap_string, "Processing file <%s>", input_filename);
      PMU_force_register(procmap_string);
      
      if (!ltg_file.loadFile(input_filename, _params->processing_delay))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error loading Kavouras data from file <" << input_filename <<
	  ">" << endl;
	cerr << "   *** SKIPPING FILE ***" << endl;
	
	continue;
      }
      
      // Process the data.  Note that KAVLTG_read_strike() returns
      // a pointer to static memory that should NOT be freed.

      KAVLTG_strike_t *strike_kav;
      int strike_count = 0;
      time_t buffer_time = 0;
      
      while ((strike_kav = ltg_file.getNextStrike()) != NULL)
      {
	
	// check that strike time is within processing window
	if ((strike_kav->time < oldest_strike_time) && 
	    (_params->runmode == Params::ARCHIVE))
	  continue;

	// See if this strike goes in the current SPDB buffer

	if (buffer_time != strike_kav->time)
	{
	  // Send to SPDB destinations

	  spdb_buffer.writeToDatabase(_params->output_url,
				      _params->expire_secs,
				      startup_flag && _params->check_old_data);
	  
	  spdb_buffer.clear();
	  
	  buffer_time = strike_kav->time;
	  
	} /* endif - buffer_time != strike_kav->time */
	
	// Register with the process mapper again just in case this
	// is a long file
	
	PMU_auto_register(procmap_string);

	// Convert the buffer to SPDB format and add it to the buffer.
	// First check to make sure that the strike is within the grid.
	// Note that KAVLTG_kav_to_spdb() returns a pointer to a static
	// data area so this pointer must NOT be freed.

	LTG_strike_t *strike_spdb = KAVLTG_kav_to_spdb(strike_kav);

	if (strike_spdb->latitude >= _params->min_strike_lat &&
	    strike_spdb->latitude <= _params->max_strike_lat &&
	    strike_spdb->longitude >= _params->min_strike_lon &&
	    strike_spdb->longitude <= _params->max_strike_lon)
	{
	  
	  spdb_buffer.addStrike(strike_spdb);
	}
	else
	{
	  if (_params->debug)
	    cerr << "*** Ltg strike at " << strike_spdb->latitude << ", " <<
	      strike_spdb->longitude << " outside of grid" << endl;
	}
	
	// Increment the strike count

	strike_count++;
	
      } /* endwhile - strike_kav != NULL */
      
      // Send the last strikes in the file to the SPDB destinations

      spdb_buffer.writeToDatabase(_params->output_url,
				  _params->expire_secs,
				  startup_flag && _params->check_old_data);
      
      spdb_buffer.clear();
      
      if (_params->debug)
	cerr << "Found " << strike_count << " strikes in input file <" <<
	  input_filename << ">" << endl;
      
      // Close the input file

      ltg_file.close();
	
    } /* endwhile - input_filename != NULL */
    
    // We have processed all of the files existing at start-up if we
    // get to this point.

    startup_flag = FALSE;
    
    // Sleep a little bit if in REALTIME mode

    if (_params->runmode == Params::REALTIME)
      sleep(_params->sleep_seconds);
    else
      break;
    
  } /* endwhile - FOREVER */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
