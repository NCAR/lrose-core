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
// Janitor.cc
//
// Janitor object
//
// Niles Oien, copied almost whole from
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 1998
//
///////////////////////////////////////////////////////////////
//
// Janitor uses the C++ functionality of TDRP
//
///////////////////////////////////////////////////////////////

/**
 * @file Janitor.cc
 *
 * Utility application for managing disk space by cleaning up data directories.
 *
 * @author Mike Dixon
 * @see something 
 */

#include <cstring>
#include <string>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <didss/RapDataDir.hh>

#include "Directory.hh"
#include "DiskFullDeleteList.hh"
#include "Janitor.hh"
using namespace std;

// Constructor

Janitor::Janitor(int argc, char **argv) 

{

  OK = TRUE;

  // set programe name

  _progName = string("Janitor");

  _paramsPath = (char *) "unknown";

  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with command line args" << endl;
    OK = FALSE;
    return;
  }
  
  // get TDRP params
  
  if (_params.loadFromArgs(argc, argv,
                           _args.override.list,
                           &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with TDRP parameters" << endl;
    OK = FALSE;
    return;
  }

  // initialize procmap registration

  PMU_auto_init(_progName.c_str(), _params.instance, PROCMAP_REGISTER_INTERVAL);
  
  return;

}

// destructor

Janitor::~Janitor()

{
  // unregister wilh procmap

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Janitor::Run()
{

  // compute top dir

  string topDir;
  if (strlen(_params.top_dir) == 0) {
    topDir = RapDataDir.location();
  } else {
    topDir = _params.top_dir;
  }

  if (_params.debug) {
    cerr << "top_dir: " << topDir << endl;
  }
  
  int iret = 0;

  while (true) {
    
    if (_traverse(topDir)) {
      iret = -1;
    }
    
    if (_params.once_only) {
      return 0;
    }
    
    for (int k = 0; k < _params.SleepBetweenPasses; k++){
      PMU_auto_register("Sleeping between passes");
      umsleep(1000);
    }

  } // while

  return iret;

}

//////////////////////////////////////////////////
// traverse the directory tree

int Janitor::_traverse(const string &top_dir)
{

  PMU_auto_register("Start of traverse");

  if (_params.debug) {
    cerr << endl;
    cerr << "============================================" << endl;
    cerr << "Start of traverse ......." << endl;
  }
    
  // Create the delete list.  This list will be used to delete files
  // if the disk is getting full after this cleanup pass.

  DiskFullDeleteList delete_list(_progName, _params);

  // Clean up the data repository. This is done to each directory
  // recursively inside of the process() method.
  
  Directory dir(_progName,
		&_params,
		top_dir,
		top_dir,
		&delete_list);
  
  int iret = dir.process();

  // Now delete files if the disk is getting full

  delete_list.deleteFiles();
  
  PMU_auto_register("End of traverse");
    
  if (_params.debug) {
    cerr << endl;
    cerr << "End of traverse, sleeping ......." << endl;
    cerr << "============================================" << endl;
  }
    
  return iret;

}

//////////////////////////////////////////////////




