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
// Archiver.cc
//
// Archiver object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 2001
//
///////////////////////////////////////////////////////////////

#include <string.h>
#include <malloc.h>

#include <toolsa/udatetime.h>

#include "ArchiveProcessor.hh"
#include "Args.hh"
#include "Archiver.hh"
using namespace std;

// Constructor

Archiver::Archiver(int argc, char **argv)
{
  OK = true;

  // set programe name

  _progName = strdup("Archiver");

  // get command line args

  Args args(argc, argv, _progName);

  // get TDRP params
  
  _params = new Params();
  _paramsPath = (char *) "unknown";
  if (_params->loadFromArgs(argc, argv,
			    args.override.list,
			    &_paramsPath)) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = false;
    return;
  }

  // Determine the start and end times for the archive

  _startTime = args.startTime;
  _endTime = args.endTime;

  return;
}

// destructor

Archiver::~Archiver()
{
  // free up

  delete(_params);
}


//////////////////////////////////////////////////
// Run

bool Archiver::Run()
{
  // Clean up the data repository.  This is done to each directory
  // recursively inside of the process() method.

  ArchiveProcessor processor(_progName, _params->debug);
  processor.processDir(_params->data_dir, _params->archive_dir, _params,
		       _startTime, _endTime);

  return true;
}

//////////////////////////////////////////////////




