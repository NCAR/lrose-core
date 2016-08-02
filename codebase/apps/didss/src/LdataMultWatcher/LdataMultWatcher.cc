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
// LdataMultWatcher.cc
//
// LdataMultWatcher object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2006
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <csignal>
#include <sys/wait.h>
#include <cstring>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <didss/RapDataDir.hh>
#include <dsserver/DmapAccess.hh>
#include "LdataMultWatcher.hh"
using namespace std;

// Constructor

LdataMultWatcher::LdataMultWatcher(int argc, char **argv)

{

  isOK = true;
  
  // set programe name

  _progName = "LdataMultWatcher";
  ucopyright((char *) _progName.c_str());
  
  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = const_cast<char*>(string("unknown").c_str());
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		_params.procmap_register_interval);

  // set up data set array

  for (int ii = 0; ii < _params.data_sets_n; ii++) {
    DataSet *set = new DataSet(_params, ii);
    if (set->isOK) {
      _dataSets.push_back(set);
    } else {
      delete set;
      isOK = false;
    }
    
  }

  return;

}

// destructor

LdataMultWatcher::~LdataMultWatcher()

{

  for (int ii = 0; ii < (int) _dataSets.size(); ii++) {
    delete _dataSets[ii];
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int LdataMultWatcher::Run ()
{

  time_t timeLastPrint = 0;

  while (true) {
    
    // register with procmap
    
    PMU_auto_register("Looking for new data ...");
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      time_t now = time(NULL);
      cerr << "LdataMultWatcher::Run - searching for latest data at "
	   << utimstr(now) << endl;
    }

    // loop through data sets
    
    int newDataFound = false;
    for (int ii = 0; ii < (int) _dataSets.size(); ii++) {
      if (_dataSets[ii]->check()) {
        newDataFound = true;
      }
    } // ii

    if (newDataFound) {
      if (_params.debug) {
        time_t now = time(NULL);
        cerr << "LdataMultWatcher - found data at " << utimstr(now) << endl;
        timeLastPrint = 0;
      }
    } else {
      if (_params.debug) {
        time_t now = time(NULL);
        if ((now - timeLastPrint > 120) || (now % 60 == 0)) {
          cerr << "LdataMultWatcher - waiting for latest data at "
               << utimstr(now) << " Zzzzz ....." << endl;
          timeLastPrint = now;
        }
      }
      umsleep(1000);
    }

  } // while

  return 0;

}
//////////////////////////
// kill remaining children
//

void LdataMultWatcher::killRemainingChildren()

{
  
  for (int ii = 0; ii < (int) _dataSets.size(); ii++) {
    _dataSets[ii]->killRemainingChildren();
  }

}

    

