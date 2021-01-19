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
// StormModel.cc
//
// StormModel object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 1998
//
///////////////////////////////////////////////////////////////

#include "StormModel.hh"
#include "Generate.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/ucopyright.h>
#include <rapmath/math_macros.h>
using namespace std;

// Constructor

StormModel::StormModel(int argc, char **argv)

{

  OK = TRUE;

  // set programe name

  _progName = STRdup("StormModel");
  ucopyright((char *) _progName.c_str());

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    OK = false;
    return;
  }

  if (!OK) {
    return;
  }

  PMU_auto_init((char *) _progName.c_str(),
                _params.instance,
                PROCMAP_REGISTER_INTERVAL);
  
}

// destructor

StormModel::~StormModel()

{

  // unregister process
  
  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int StormModel::Run ()
{

  Generate *gen = new Generate(_progName.c_str(), _params);
  
  gen->printHeader(stdout);
  
  if (!gen->OK) {
    return (-1);    
  }

  for (int i = 0; i < _params.nstorms_gen; i++) {
    while (gen->Another(i, stdout));
  } // i

  fprintf(stderr, "Sum Area: %g\n", gen->sumArea());

  delete gen;

  return 0;

}

