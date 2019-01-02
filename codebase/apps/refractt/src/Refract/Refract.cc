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
//   $Date: 2018/06/03 19:37:09 $
//   $Id: Refract.cc,v 1.25 2018/06/03 19:37:09 dixon Exp $
//   $Revision: 1.25 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Refract: Refract program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "Refract.hh"
#include "Params.hh"
#include <Refract/ParmApp.hh>
#include <Refract/RefractInput.hh>
#include <dsdata/DsTrigger.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <assert.h>

// Global variables

Refract *Refract::_instance = (Refract *)NULL;

/*********************************************************************
 * Constructor
 */

Refract::Refract(int argc, char **argv)
{
  // Make sure the singleton wasn't already created.

  assert(_instance == (Refract *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopyright message.

//  ucopyright(_progName);

  // Get TDRP parameters.
  if (!parmAppInit(_params, argc, argv))
  {
    exit(0);
  }

  if (RefParms::isPrintMode())
  {
    _refparms.print(stdout, PRINT_VERBOSE);
  }
  else
  {
    if (_refparms.load(RefParms::parmPath().c_str(), NULL,
		       !RefParms::isParmPrint(), false))
    {
      LOG(ERROR) << "cant load " <<  RefParms::parmPath();
      okay = false;
    }
    else
    {
      _refparms.setOk();
    }
    if (RefParms::isPrintAndLoadMode())
    {
      _refparms.print(stdout, PRINT_VERBOSE);
    }  
  }
  parmAppFinish(_params, _refparms);
}


/*********************************************************************
 * Destructor
 */

Refract::~Refract()
{
  LOG(FORCE) << "N extraction completed!";

  // Unregister process

  PMU_auto_unregister();

  // Free contained objects
  delete _dataTrigger;
  delete _inputHandler;

  // Free included strings

  STRfree(_progName);

  // Remove temporary files

  system("rm -f targets.tmp.*");
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Refract *Refract::Inst(int argc, char **argv)
{
  if (_instance == (Refract *)NULL)
    new Refract(argc, argv);
  
  return(_instance);
}

Refract *Refract::Inst()
{
  assert(_instance != (Refract *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool Refract::init()
{
  // Initialize the data trigger
  printf("triger mode %d\n",_refparms.trigger_mode);
  if (!_refparms.initTrigger(&_dataTrigger))
  {
    return false;
  }

  _inputHandler = _refparms.initInput();
  
  // Read in the reference file
  if (!_calib.initialize(_params.ref_file_name_day,
			 _params.ref_file_name_night,
			 _params._hms_night,
			 _params._hms_day, 
			 _params.day_night_transition_delta_minutes*60))
  {
    LOG(ERROR) << "reading calibration files";
    return false;
  }
    
  // Initialize the processor

  if (!_processor.init(&_calib, _refparms, _params))
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _refparms.instance, PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void Refract::run()
{
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      LOG(ERROR) << "Error getting next trigger time";
      continue;
    }
    
    if (!_processData(trigger_time))
    {
      LOG(ERROR) << "Error processing data for time: " << trigger_time;
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

bool Refract::_processData(const DateTime &trigger_time)
{
  PMU_auto_register("Processing data");
  
  LOG(DEBUG) << "**** Processing data for time: " << trigger_time;
  
  // Get the next scan from the input handler
  DsMdvx data_file;
  
  if (!_readInputFile(data_file, trigger_time))
  {
    return false;
  }

  if (_processor.processScan(*_inputHandler, trigger_time.utime(), data_file))
  {
    // Generate output file
    LOG(DEBUG) << "---> Writing data for scan at time "
	       << DateTime::str(data_file.getMasterHeader().time_centroid);
  
    if (data_file.writeToDir(_refparms.output_url) != 0)
    {
      LOG(ERROR) << "writing output file to URL: " << _refparms.output_url;
      LOG(ERROR) << data_file.getErrStr();
      return false;
    }
    LOG(DEBUG) << "Wrote output file: " << data_file.getPathInUse();
    return true;
  }
  else
  {
    return false;
  }
}


/*********************************************************************
 * _readInputFile()
 */

bool Refract::_readInputFile(DsMdvx &mdvx, const DateTime &data_time)
{
  if (!_inputHandler->getScan(data_time, 0, _refparms.input_url, mdvx))
  {
    LOG(ERROR) << "Could not read in data";
    return false;
  }
  return true;
}
