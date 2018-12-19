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
// CaseStats.cc
//
// CaseStats object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1997
//
///////////////////////////////////////////////////////////////

#include "CaseStats.hh"
#include <toolsa/str.h>
#include <ctime>
using namespace std;

// Constructor

CaseStats::CaseStats(int argc, char **argv)

{

  isOK = TRUE;
  _cases = NULL;
  _caseProps = NULL;

  // set programe name
  
  _progName = "CaseStats";
  ucopyright((char *) _progName.c_str());

  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  if (!isOK) {
    return;
  }

  // case file object
  
  _cases = new Cases(_progName.c_str(),
		     (_params.debug >= Params::DEBUG_VERBOSE),
		     _params.case_file_path);
  
  if (!_cases->isOK) {
    isOK = FALSE;
    return;
  }

  // create CaseProps object
  
  _caseProps = new CaseProps(_progName.c_str(), (_params.debug >= Params::DEBUG_VERBOSE),
			     _params.props_files_dir,
			     _params.global_props_n,
			     _params._global_props,
			     _params.tseries_props_n,
			     _params._tseries_props,
			     _params.tseries_dtimes_n,
			     _params._tseries_dtimes,
			     _params.conditions_n,
			     _params._conditions,
			     _params.set_missing_val_in_interp,
			     _params.allow_one_ended_interp,
			     _params.max_time_error_for_one_ended_interp);

  // initialize process registration
  
  PMU_auto_init(_progName.c_str(), _params.instance, PROCMAP_REGISTER_INTERVAL);
  
}

//////////////
// destructor

CaseStats::~CaseStats()

{

  // unregister process

  PMU_auto_unregister();

  // call destructors

  if (_caseProps) {
    delete _caseProps;
  }
  if (_cases) {
    delete _cases;
  }

}

//////////////////////////////////////////////////
// run

int CaseStats::run()
{

  // print header

  _printHeader(stdout);

  // add cases to the CaseProps object

  if (_addCases()) {
    return (-1);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _caseProps->printDebug(stderr);
  }

  // if requested, write out interpolated properties

  if (_params.write_interp_files) {
    _caseProps->writeInterp();
  }

  // compute stats

  _caseProps->computeStats(_params.stat_type);

  // perform rerandomization if required
  
  if (_params.use_rerandomization) {
    _caseProps->doRerand(_params.stat_type,
			 _params.n_rerand,
			 _params.n_random_list,
			 _params.max_split);
  }

  // print results

  _caseProps->printStats(stdout);

  return (0);
}

//////////////////////////////////////////////////
// Private routines
//////////////////////////////////////////////////

//////////////////////////////////////////////////
// print header
//

void CaseStats::_printHeader(FILE *out)

{

  char hstring[256];
  time_t now = time(NULL);
 
  // main header

  fprintf(out, "\n\n");
  sprintf(hstring, "Program %s, run at %s", _progName.c_str(), ctime(&now));
  fprintf(out, "%s", hstring);

  // underline header

  int len = strlen(hstring) - 1;
  for (int i = 0; i < len; i++) {
    fprintf(out, "=");
  }
  fprintf(out, "\n\n");

  // Statistic type

  fprintf(out, "\n");

  switch (_params.stat_type) {

  case Params::ARITH_MEAN:
    fprintf(out, "STAT TYPE: ARITHMETIC MEANS\n");
    break;
    
  case Params::GEOM_MEAN:
    fprintf(out, "STAT TYPE: GEOMETRIC MEANS\n");
    break;
    
  case Params::FIRST_QUARTILE:
    fprintf(out, "STAT TYPE: FIRST_QUARTILE\n");
    break;
    
  case Params::SECOND_QUARTILE:
    fprintf(out, "STAT TYPE: SECOND_QUARTILE\n");
    break;
    
  case Params::THIRD_QUARTILE:
    fprintf(out, "STAT TYPE: THIRD_QUARTILE\n");
    break;
    
  } // switch

  fprintf(out, "\n");

  // use missing data or zeros for interp

  if (_params.set_missing_val_in_interp) {
    fprintf(out, "Using MISSING_DATA_VAL when no storm exists at interp time\n\n");
  } else {
    fprintf(out, "Using ZERO when no storm exists at interp time\n\n");
  }

  // rerandomization

  if (_params.use_rerandomization) {

    fprintf(out, "Performing rerandomization:\n\n");
    fprintf(out, "    n_rerand:      %5d\n", (int) _params.n_rerand);
    fprintf(out, "    n_random_list: %5d\n", (int) _params.n_random_list);
    fprintf(out, "    max_split:     %5d\n", (int) _params.max_split);
    fprintf(out, "\n");

  }

  // conditional properties

  fprintf(out, "Conditional properties:\n\n");
  fprintf(out, "%30s %10s %10s\n",
	  "Property name", "Min-val", "Max-val");
  
  for (int i = 0; i < _params.conditions_n; i++) {
    fprintf(out, "%30s %10.3f %10.3f\n",
	    _params._conditions[i].prop_name,
	    _params._conditions[i].min_val,
	    _params._conditions[i].max_val);
  }
  fprintf(out, "\n");

  fflush(out);

}

//////////////////////////////////////////////////
// Add cases

int CaseStats::_addCases()
{

  int iret = 0;

  // for each case, have CaseProps get the appropriate data
  // from the props files and store in the arrays

  SeedCaseTracks::CaseTrack this_case;
  
  while (_cases->next(&this_case) == 0) {

    if (_params.debug) {
      fprintf(stderr, "Adding Case %d\n", this_case.num);
    }

    if (_caseProps->addCase(&this_case, stdout)) {
      
      fprintf(stderr, "ERROR - CaseStats::Run\n");
      fprintf(stderr, "Cannot load up case %d\n",
	      this_case.num);
      iret = -1;
      
    }

  } // while

  return (iret);

}


