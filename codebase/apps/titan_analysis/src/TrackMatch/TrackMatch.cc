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
// TrackMatch.cc
//
// TrackMatch object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1998
//
///////////////////////////////////////////////////////////////

#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <rapmath/math_macros.h>
#include "TrackMatch.hh"
#include "Cases.hh"
#include "PropsFile.hh"
#include "PropsList.hh"
using namespace std;

// Constructor

TrackMatch::TrackMatch(int argc, char **argv)

{

  OK = TRUE;
  Done = FALSE;

  // set programe name

  _progName = STRdup("TrackMatch");
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

  Params *params = new Params(_args->paramsFilePath,
			      &_args->override,
			      _progName,
			      _args->checkParams,
			      _args->printParams,
			      _args->printShort);
  
  if (!params->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = FALSE;
    return;
  }
  if (params->Done) {
    Done = TRUE;
    return;
  }
  _params = &params->p;

  if (!OK) {
    return;
  }

  // props file object

  if (_args->nFiles > 0) {

    _props = new DsInputPath(_progName,
			     _params->debug >= DEBUG_VERBOSE,
			     _args->nFiles,
			     _args->filePaths);
      
  } else {
    
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr,
	    "You must specify a file list using -f\n");
    OK = FALSE;
      
  }

  PMU_auto_init(_progName, _params->instance, PROCMAP_REGISTER_INTERVAL);

  return;

}

//////////////
// destructor

TrackMatch::~TrackMatch()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  delete(_props);
  delete _params;
  delete _args;
  STRfree(_progName);

}

//////////////////////////////////////////////////
// PrintComments()
//
// Print comments at start of file
//

void TrackMatch::_printComments ()
{

  date_time_t file_time;
  ulocaltime(&file_time);
  
  fprintf(stdout, "#  Run time: %s\n", utimestr(&file_time));
  fprintf(stdout, "#  case_num: %d\n", (int) _params->case_num);
  fprintf(stdout, "#  case_file_path: %s\n", _params->case_file_path);
  fprintf(stdout, "#  n_candidates: %d\n", (int) _params->n_candidates);

  switch (_params->match_property) {
  case VOLUME:
    fprintf(stdout, "#  match_property: %s\n", "VOLUME");
    break;
  case AREA:
    fprintf(stdout, "#  match_property: %s\n", "AREA");
    break;
  case MASS:
    fprintf(stdout, "#  match_property: %s\n", "MASS");
    break;
  case PRECIP_FLUX:
    fprintf(stdout, "#  match_property: %s\n", "PRECIP_FLUX");
    break;
  } // switch

  fprintf(stdout, "#  time_margin: %g\n", _params->time_margin);
  fprintf(stdout, "#  range_margin: %g\n", _params->range_margin);
  fprintf(stdout, "\n");

  fprintf(stdout, "#  File name(s):\n");
  _props->reset();
  char *propsFilePath;
  while ((propsFilePath = _props->next()) != NULL) {
    fprintf(stdout, "#    %s\n", propsFilePath);
  }

}

//////////////////////////////////////////////////
// Run

int TrackMatch::Run ()
{

  PMU_auto_register("TrackMatch::Run");

  // print comments to start of output file

  _printComments();

  // create the list object

  PropsList *list = new PropsList(_progName, _params);

  // create the Case file object

  Cases *cases = new Cases(_progName,
			   (_params->debug >= DEBUG_VERBOSE? 1:0),
			   _params->case_file_path);

  if (!cases->OK) {
    return (-1);
  }

  // find the relevant case

  int case_found = FALSE;
  case_track_t this_case;
  cases->reset();
  while (cases->next(&this_case) == 0) {
    if (this_case.num == _params->case_num) {
      case_found = TRUE;
      break;
    }
  }
  if (!case_found) {
    fprintf(stderr, "ERROR - %s:TrackMatch::Run\n", _progName);
    fprintf(stderr, "Cannot find case %d in file '%s'\n",
	    (int) _params->case_num, _params->case_file_path);
    return (-1);
  }

  fprintf(stdout, "\n");
  case_track_print(stdout, (char *) "", &this_case);
  fprintf(stdout, "\n");
  fflush(stdout);

  // loop through the props files to find the props for the case

  int case_props_found = FALSE;
  initial_props_t case_props;
  char *propsFilePath;
  _props->reset();
  while ((propsFilePath = _props->next()) != NULL) {
    PropsFile *pfile = new PropsFile(_progName, _params,
				     &this_case, propsFilePath,
				     list);
    if (pfile->OK) {
      if (pfile->search_for_case(&this_case, &case_props) == 0) {
	case_props_found = TRUE;
	delete (pfile);
	break;
      } else {
	delete (pfile);
      }
    }
  } // while

  if (!case_props_found) {
    fprintf(stderr, "ERROR - %s:TrackMatch::Run\n", _progName);
    fprintf(stderr, "Cannot find case %d in props files\n",
	    (int) _params->case_num);
    return (-1);
  }

  // loop through the props files again, finding the best matches
  // to the case file

  _props->reset();
  while ((propsFilePath = _props->next()) != NULL) {

    if (_params->debug) {
      fprintf(stderr, "Processing props file %s\n", propsFilePath);
    }
    
    PropsFile *pfile = new PropsFile(_progName, _params,
				     &this_case, propsFilePath,
				     list);
    
    if (pfile->OK) {
      if (pfile->update_list(&case_props)) {
	fprintf(stderr, "ERROR - %s:TrackMatch::Run\n", _progName);
	fprintf(stderr, "Processing props file '%s'\n", propsFilePath);
      }
      delete (pfile);
    }

  } // while

  // print the list

  fprintf(stdout, "\n");
  fprintf(stdout, "Case properties:\n");
  fprintf(stdout, "\n");
  list->print(stdout, &case_props);

  fprintf(stdout, "\n");
  fprintf(stdout, "Track match array:\n");
  fprintf(stdout, "\n");
  list->print(stdout);

  // clean up

  delete(list);
  delete(cases);
  
  return (0);

}

