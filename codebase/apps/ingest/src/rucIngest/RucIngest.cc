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
///////////////////////////////////////////////////////////////////////////
//  rucIngest top-level application class
//
//  $Id: RucIngest.cc,v 1.18 2016/03/07 01:23:11 dixon Exp $
//
//////////////////////////////////////////////////////////////////////////

// C++ include files
#include <cassert>
#include <cstdio>
#include <cstring>
#include <ctime>

// System/RAP include files
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>

// Local include files
#include "RucIngest.hh"
#include "DataMgr.hh"
#include "Args.hh"
using namespace std;

// the singleton itself
RucIngest *RucIngest::_instance = 0;

// define any constants
const string RucIngest::_className = "RucIngest";

RucIngest::RucIngest(int argc, char **argv) :
  _isOK(true),
  _progName(""),
  _errStr(""),
  _args(0),
  _params(0)
{
  const string methodName = _className + string("::Constructor");
  //  _errStr = string("ERROR: ") + methodName;

  // Make sure the singleton wasn't already created.
  assert(_instance == 0);

  // Set the singleton instance pointer
  _instance = this;

  // set programe name
  Path pathParts(argv[0]);
  _progName = pathParts.getBase();

  // display ucopyright message and RCSid
  ucopyright(const_cast<char*>(_progName.c_str()));

  // get command line args
  _args = new Args(argc, const_cast<char**>(argv), _progName);
  if (!_args->isOK) {
    _errStr += "\tProblem with command line arguments.\n";
    _isOK = false;
  }

  // get TDRP params
  _params = new Params();
  char *paramsPath = "unknown";
  if (_params->loadFromArgs(argc, argv, _args->override.list, 
			    &paramsPath)) {
    _errStr += "\tProblem with TDRP parameters.\n";
    _isOK = false;
    return;
  }


  //
  // Create and initialize the data manager
  //
   _dataMgr = new DataMgr();
   
   cerr << "initialize data manager." << endl;

   switch (_params->mode)
   {
   case Params::REALTIME :
   case Params::REALTIME_DIR :
   {
     cerr << "Initalizing DataMgr for realtime operation." << endl;
     if( !_dataMgr->init( *_params )  ) {
       _isOK = false;
       return;
     }
     break;
   }

   case Params::FILELIST :
   {
     cerr << "Initalizing DataMgr for filelist operation." << endl;
     if( !_dataMgr->init( *_params,  _args->getInputFileList() ) ) {
       _isOK = false;
       return;
     }
   }
   } /* endswitch - _params->mode */

  // init process mapper registration
  PMU_auto_init((char *) _progName.c_str(),
		_params->instance,
		PROCMAP_REGISTER_INTERVAL);
  cerr << "started procmap with progname: " << _progName.c_str() << " and instance: " << _params->instance << endl;
  PMU_force_register( "STARTING UP." );
  return;
}

RucIngest::~RucIngest() 
{
   //
   // Unregister with process mapper
   //
   PMU_auto_unregister();

   delete _params;
   delete _args;
   delete _dataMgr;
}

RucIngest* 
RucIngest::instance(int argc, char **argv)
{
  if ( _instance == 0 ) {
    _instance = new RucIngest(argc, argv);
  }
  
  return(_instance);
}

RucIngest* 
RucIngest::instance(void)
{
  assert(_instance != 0 );
  return(_instance);
}

bool
RucIngest::run()
{
  if( !_dataMgr->getData() ) {
    return false;
  }

  return true;
}


