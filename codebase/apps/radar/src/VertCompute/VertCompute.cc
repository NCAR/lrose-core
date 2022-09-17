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
// VertCompute.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2006
//
///////////////////////////////////////////////////////////////
//
// VertCompute analyses time series data from vertical scans
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <toolsa/DateTime.hh>
#include <toolsa/uusleep.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/mem.h>

#include "VertCompute.hh"
#include "TsDataMgr.hh"
#include "DsrDataMgr.hh"
#include "RadxDataMgr.hh"

using namespace std;

// Constructor

VertCompute::VertCompute(int argc, char **argv)
  
{

  isOK = true;

  // set programe name
  
  _progName = "VertCompute";
  
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
    return;
  }

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
  return;
  
}

// destructor

VertCompute::~VertCompute()

{

}

//////////////////////////////////////////////////
// Run

int VertCompute::Run ()
{

  // create stats manager object, based on type of input data
  
  StatsMgr *statsMgr = NULL;
  if (_params.input_mode == Params::RADX_MOMENTS_INPUT) {
    statsMgr = new RadxDataMgr(_progName, _args, _params);
  } else if (_params.input_mode == Params::DSR_MOMENTS_INPUT) {
    statsMgr = new DsrDataMgr(_progName, _args, _params);
  } else {
    statsMgr = new TsDataMgr(_progName, _args, _params);
  }
  if (statsMgr == NULL) {
    return -1;
  }

  // read in the data and run
  
  if (statsMgr->run()) {
    return -1;
  }
  
  statsMgr->computeGlobalStats();
  if (_params.write_stats_to_text_file) {
    statsMgr->writeGlobalStats();
  }
  if (_params.write_zdr_point_values_to_text_file) {
    statsMgr->writeZdrPoints();
  }

  delete statsMgr;
  return 0;

}

