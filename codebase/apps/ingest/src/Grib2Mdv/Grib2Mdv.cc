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

//  Grib2Mdv top-level application class
//  $Id: Grib2Mdv.cc,v 1.11 2017/06/09 16:27:58 prestop Exp $

//  This application was created to unify a number of grib converters
//  already in existence.  Author:  Carl Drews, circa 2004.
//  There are a lot of variations on the grib format; this app
//  was designed not to be exhaustive, but to handle the cases
//  that we are currently ingesting.  If you have a variation
//  of grib that is not handled here, consider adding it as
//  another sub-class of GribMgr.


// C++ include files
#include <cassert>
#include <cstdio>
#include <cstring>
#include <ctime>

// System/RAP include files
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>

// Local include files
#include "Grib2Mdv.hh"
#include "DataMgr.hh"
#include "Args.hh"
using namespace std;

// the singleton itself
Grib2Mdv *Grib2Mdv::_instance = 0;

// define any constants
const string Grib2Mdv::_className = "Grib2Mdv";

Grib2Mdv::Grib2Mdv(int argc, char **argv) :
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

  // display kopyright message and RCSid
  ucopyright(const_cast<char*>(_progName.c_str()));

  // get command line args
  _args = new Args(argc, const_cast<char**>(argv), _progName);
  if (!_args->isOK) {
    _errStr += "\tProblem with command line arguments.\n";
    _isOK = false;
  }

  // get TDRP params
  _params = new Params();
  char *paramsPath;// = "unknown";
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
   
   switch (_params->mode)
   {
   case Params::REALTIME :
   {
     const time_t tptr = time(NULL);
     if (_params->debug) {
       cout << "Initializing DataMgr for realtime operation at "
            << ctime(&tptr) << endl;
     }
     if( !_dataMgr->init( *_params )  ) {
       _isOK = false;
       return;
     }
     break;
   }

   case Params::REALTIME_DIR :
   {
     const time_t tptr = time(NULL);
     if (_params->debug) {
       cout << "Initializing DataMgr for realtime directory operation at "
            << ctime(&tptr) << endl;
     }
     if( !_dataMgr->init( *_params )  ) {
       _isOK = false;
       return;
     }
     break;
   }

   case Params::FILELIST :
   {
     const time_t tptr = time(NULL);
     if (_params->debug) {
       cout << "Initializing DataMgr for filelist operation at "
            << ctime(&tptr) << endl;
     }
     if( !_dataMgr->init( *_params,  _args->getInputFileList() ) ) {
       _isOK = false;
       return;
     }
     break;
   }

   case Params::NEWFILES :
   {
     const time_t tptr = time(NULL);
     if (_params->debug) {
       cout << "Initializing DataMgr for newfiles operation at "
            << ctime(&tptr) << endl;
     }
     if( !_dataMgr->init( *_params )  ) {
       _isOK = false;
       return;
     }
     break;
   }

   } /* endswitch - _params->mode */

  // init process mapper registration
  PMU_auto_init((char *) _progName.c_str(),
		_params->instance,
		PROCMAP_REGISTER_INTERVAL);

  if (_params->debug) {
    cout << "registering with procmap" << endl;
    cerr << "  progname: " << _progName << endl;
    cerr << "  instance: " << _params->instance << endl;
  }

  PMU_force_register( "STARTING UP." );
  return;
}

Grib2Mdv::~Grib2Mdv() 
{
   //
   // Unregister with process mapper
   //
   PMU_auto_unregister();

   delete _params;
   delete _args;
   delete _dataMgr;
}

Grib2Mdv* 
Grib2Mdv::instance(int argc, char **argv)
{
  if ( _instance == 0 ) {
    _instance = new Grib2Mdv(argc, argv);
  }
  
  return(_instance);
}

Grib2Mdv* 
Grib2Mdv::instance(void)
{
  assert(_instance != 0 );
  return(_instance);
}

bool
Grib2Mdv::run()
{
  if( !_dataMgr->getData() ) {
    return false;
  }

  return true;
}


