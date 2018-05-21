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
//  Grib2toNc top-level application class
//
// Handles params, arguments and instantiation.
// Grib2Nc class does the actual conversion.
//
// Jason Craig  -  Jan 2015
//////////////////////////////////////////////////////////////////////////
#include <cassert>
#include <cstdio>
#include <string.h>
#include <ctime>

#include "Grib2toNc.hh"
#include "Grib2Nc.hh"

#ifndef NOT_RAL
#include <toolsa/pmu.h>
#endif
#include <toolsa/utim.h> // For unix_to_date

using namespace std;

// Global instance variable
Grib2toNc *Grib2toNc::_instance = (Grib2toNc *)NULL;

//
// Constructor
Grib2toNc::Grib2toNc(int argc, char **argv) {

  // Make sure the singleton wasn't already created.
  assert(_instance == (Grib2toNc *)NULL);

  // Set the singleton instance pointer
  _instance = this;

  okay = true;

  // Set the base program name.
  strcpy(_progName, "Grib2toNc");

  // Display UCAR ucopyright message.
  _ucopyright(_progName);

  // Get the command line arguments via the Args class
  _args = new Args(argc, argv, _progName);
  if (!_args->okay)
  {
    fprintf(stderr, "ERROR: Problem reading command line arguments.\n");
    okay = false;
    return;
  }

  // Get TDRP parameters.
  _params = new Params();
  char *params_path = new char[300];
  strcpy(params_path, "unknown");

  if (_params->loadFromArgs(argc, argv, _args->override.list, &params_path))
  {
    fprintf(stderr, "ERROR: Problem reading TDRP parameters in file <%s>\n", params_path);
    okay = false;
    return;
  }

  _grib2Nc = new Grib2Nc(*_params);
 
#ifndef NOT_RAL
  // initialize process registration
  PMU_auto_init(_progName, _params->instance,
                _params->procmap_register_interval_secs);
  #endif
}

//
// Destructor
Grib2toNc::~Grib2toNc() 
{
#ifndef NOT_RAL
   //
   // Unregister with process mapper
   //
   PMU_auto_unregister();
#endif

  if (_params != (Params *)NULL)
    delete _params;
  if (_args != (Args *)NULL)
    delete _args;

  if (_grib2Nc != (Grib2Nc *)NULL)
    delete _grib2Nc;
}

//
// Inst() - Retrieve the singleton instance of this class.
Grib2toNc *Grib2toNc::Inst(int argc, char **argv)
{
  if (_instance == (Grib2toNc *)NULL)
    new Grib2toNc(argc, argv);

  return(_instance);
}

Grib2toNc *Grib2toNc::Inst()
{
  assert(_instance != (Grib2toNc *)NULL);
  return(_instance);
}

int Grib2toNc::run()
{

  //
  // Initialize and run the Grib2Nc object
  //
  if( _args->_nFiles > 0 ) {
    if( _grib2Nc->init (_args->_nFiles, _args->_fileList, _args->_outputFile,
			_args->_printVarList, _args->_printSummary, _args->_printSections ) != RI_SUCCESS )
      return ( RI_FAILURE );
  }
  else {
    if( _grib2Nc->init (0, NULL, _args->_outputFile, _args->_printVarList,
			 _args->_printSummary, _args->_printSections ) != RI_SUCCESS )
      return( RI_FAILURE );
  }

  if( _grib2Nc->getData() != RI_SUCCESS ) {
    return( RI_FAILURE );
  }

  return( RI_SUCCESS );
}

void Grib2toNc::_ucopyright(const char *prog_name)
{
  char whenChar[20];
  time_t now = time(NULL);
  UTIMstruct timeStruct;
  UTIMunix_to_date(now, &timeStruct);  
  sprintf(whenChar, "%4d/%02d/%02d %02d:%02d:%02d",
	  timeStruct.year, timeStruct.month, timeStruct.day,
	  timeStruct.hour, timeStruct.min, timeStruct.sec);

  fprintf(stderr, "\n");
  fprintf(stderr, "** Program '%s'\n", prog_name);
  fprintf(stderr, "** Copyright (c) 1992-%.4d UCAR\n",  timeStruct.year);
  fprintf(stderr,
	  "** University Corporation for Atmospheric Research - UCAR.\n");
  fprintf(stderr, "** National Center for Atmospheric Research - NCAR.\n");
  fprintf(stderr, "** Research Applications Program - RAP.\n");
  fprintf(stderr, "** P.O.Box 3000, Boulder, Colorado, 80307, USA.\n");
  fprintf(stderr, "** Run-time %s.\n", whenChar);
  fprintf(stderr, "\n");
  fflush(stderr);

}
