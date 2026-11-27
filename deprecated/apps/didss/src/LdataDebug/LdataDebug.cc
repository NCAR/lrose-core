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
// LdataDebug.cc
//
// LdataDebug object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2002
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

#include <dsserver/DsLdataInfo.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/str.h>

#include "LdataDebug.hh"
using namespace std;

// Constructor

LdataDebug::LdataDebug(int argc, char **argv)
{
  isOK = true;

  // set programe name

  _progName = "LdataDebug";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName))
  {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *)"unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath))
  {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;
}


// destructor

LdataDebug::~LdataDebug()
{
  // unregister process

  PMU_auto_unregister();
}


//////////////////////////////////////////////////
// Run

bool LdataDebug::Run ()
{
  // register with procmap
  
  PMU_auto_register("Run");

  // Set up LdataInfo

  DsLdataInfo ldata(_params.input_url);

  // wait for data
  
  while (true)
  {
    ldata.readBlocking(-1, 1000, PMU_auto_register);

    time_t current_time = time(0);
    cerr << "*** Ldata file updated:  " << DateTime::str(current_time) << endl;
    ldata.print(cerr);
  }

  return true;
}
