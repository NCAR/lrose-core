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
// PartialProps.cc
//
// PartialProps object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1997
//
///////////////////////////////////////////////////////////////

#include "PartialProps.hh"
#include <toolsa/str.h>
using namespace std;

// Constructor

PartialProps::PartialProps(int argc, char **argv)

{

  isOK = TRUE;
  _cases = NULL;
  _stormFile = NULL;

  // set programe name
  
  _progName = "PartialProps";
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
		     (_params.debug >= Params::DEBUG_NORM),
		     (_params.debug >= Params::DEBUG_VERBOSE),
		     _params.case_file_path,
		     _params.altitude_threshold,
		     _params.output_dir);
  
  if (!_cases->isOK) {
    isOK = FALSE;
    return;
  }

  // storm file lookup
  
  _stormFile = new StormFile(_progName.c_str(),
			     (_params.debug >= Params::DEBUG_VERBOSE),
			     _params.storm_data_dir);

  if (!_stormFile->isOK) {
    isOK = FALSE;
    return;
  }

  // initialize process registration
  
  PMU_auto_init(_progName.c_str(),
		_params.instance, PROCMAP_REGISTER_INTERVAL);
  
}

// destructor

PartialProps::~PartialProps()

{

  // unregister process

  PMU_auto_unregister();

  // call destructors

  if (_stormFile) {
    delete _stormFile;
  }
  if (_cases) {
    delete _cases;
  }

}

//////////////////////////////////////////////////
// Run

int PartialProps::Run ()
{

  int iret = 0;
  SeedCaseTracks::CaseTrack this_case;
  
  while (_cases->next(&this_case) == 0) {

    fprintf(stderr, "Processing case %d\n", this_case.num);

    char *storm_file_path =
      _stormFile->get_path(this_case.ref_time);

    // process this case

    if (storm_file_path != NULL) {

      if (_cases->process(&this_case, storm_file_path)) {
	fprintf(stderr, "ERROR - %s:PartialProps::Run\n", _progName.c_str());
	fprintf(stderr, "ERROR processing case %d\n", this_case.num);
	fprintf(stderr, "  No relevant track at time %s",
		utimstr(this_case.ref_time));
	fprintf(stderr, "  Complex_num, simple_num: %d, %d\n",
		this_case.complex_track_num,
		this_case.simple_track_num);
	iret = -1;
      }

    } else {

      fprintf(stderr, "ERROR - %s:PartialProps::Run\n", _progName.c_str());
      fprintf(stderr, "Case %d, time %s, no file found.\n",
	      this_case.num, utimstr(this_case.ref_time));
      iret = -1;
      
    }

  }
  
  return (iret);

}
