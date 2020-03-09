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
///////////////////////////////////////////////////////////////
// VerifyGrid.cc
//
// VerifyGrid object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 1998
//
///////////////////////////////////////////////////////////////

#include "VerifyGrid.h"
#include "GridCont.h"
#include "RegionCont.h"
#include "Stats.h"
#include "MapCont.h"

#include "Regression.h"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <rapmath/math_macros.h>
using namespace std;

// Constructor

VerifyGrid::VerifyGrid(int argc, char **argv)

{

  OK = TRUE;
  Done = FALSE;

  // set programe name

  _progName = STRdup("VerifyGrid");
  ucopyright(_progName);

  // get command line args

  _args = new Args(argc, argv, _progName);
  if (!_args->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }
  if (_args->Done) {
    Done = TRUE;
    return;
  }

  // get TDRP params

  _params = new Params;
  char *paramsPath = (char *) "unknown";
  if (_params->loadFromArgs(argc, argv, _args->override.list,
                            &paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    OK = FALSE;
    return;
  }

  if (!OK) {
    return;
  }

  // input file object

  if (_params->mode == Params::ARCHIVE) {

    if (_args->nFiles > 0) {

      _input = new DsInputPath(_progName,
			       _params->debug >= Params::DEBUG_VERBOSE,
			       _args->nFiles,
			       _args->filePaths);
      
    } else if (_args->startTime != 0 && _args->endTime != 0) {
      
      _input = new DsInputPath(_progName,
			       _params->debug >= Params::DEBUG_VERBOSE,
			       _params->forecast_dir,
			       _args->startTime,
			       _args->endTime);

    } else {

      fprintf(stderr, "ERROR: %s\n", _progName);
      fprintf(stderr,
	      "In ARCHIVE mode you must either specify a file list using\n"
	      "-f or the start and end times using -start and -end\n");
      OK = FALSE;
      
    }

  } else {

    _input = new DsInputPath(_progName,
			     _params->debug >= Params::DEBUG_VERBOSE,
			     _params->forecast_dir,
			     _params->max_realtime_valid_age,
			     PMU_auto_register);
    
  }

  PMU_auto_init(_progName, _params->instance, PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

VerifyGrid::~VerifyGrid()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  delete(_input);
  delete _params;
  delete _args;
  STRfree(_progName);

}

//////////////////////////////////////////////////
// Run

int VerifyGrid::Run ()
{

  PMU_auto_register("VerifyGrid::Run");

  _printHeader(stdout);

  Comps *comps = NULL;

  if (_params->method == Params::GRID_CONT_METHOD) {

    comps = new GridCont(_progName, _params);

  } else if (_params->method == Params::REGION_CONT_METHOD) {

    comps = new RegionCont(_progName, _params);

  } else if (_params->method == Params::STATS_METHOD) {

    comps = new Stats(_progName, _params);

  } else if (_params->method == Params::REGRESSION_METHOD) {

    comps = new Regression(_progName, _params);
  
  } else if (_params->method == Params::MAP_CONT_METHOD) { // Option added by Niles.

    comps = new MapCont(_progName, _params);
  
  }

  if (!comps->OK) {
    return (-1);    
  }

  if (_params->mode == Params::ARCHIVE) {
    _input->reset();
  }
  char *inputFilePath;
  while ((inputFilePath = _input->next()) != NULL) {
    if (comps->update(inputFilePath)) {
      if (_params->debug >= Params::DEBUG_VERBOSE) {
	fprintf(stderr, "WARNING: Comps::update failed for %s\n",
		inputFilePath);
      }
    }
    fflush(stdout);
  }
  fprintf(stdout, "\n");
  fprintf(stdout, "Summary statistics\n");
  fprintf(stdout, "==================\n");
  fprintf(stdout, "\n");
  comps->print(stdout);
  fflush(stdout);

  delete(comps);

  return (0);

}

//////////////////
// _printHeader()
//

void VerifyGrid::_printHeader(FILE *out)

{

  date_time_t file_time;

  ulocaltime(&file_time);

  fprintf(out, "#Program %s\n", _progName);

  fprintf(out, "#File create time : %s\n", utimestr(&file_time));

  if (_params->mode == Params::ARCHIVE) {
    fprintf(out, "#File name(s) :\n");
    _input->reset();
    char *inputFilePath;
    while ((inputFilePath = _input->next()) != NULL) {
      fprintf(out, "#    %s\n", inputFilePath);
    }
  }

  fprintf (out, "#Truth_data_dir : %s\n", _params->truth_data_dir);

  fprintf (out, "#forecast_lead_time : %d\n", _params->forecast_lead_time);

  fprintf(out, "#\n");

}









