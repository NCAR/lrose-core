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
//  EraNc2Mdv top-level application class
//
// Handles params, arguments and instantiation.
// ReadNcFiles does the actual conversion.
//
// Jason Craig
//////////////////////////////////////////////////////////////////////////
#include <cassert>
#include <cstdio>
#include <string.h>
#include <ctime>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>

#include "EraNc2Mdv.hh"
#include "ReadNcFiles.hh"
using namespace std;

// Global instance variable
EraNc2Mdv *EraNc2Mdv::_instance = (EraNc2Mdv *)NULL;

//
// Constructor
EraNc2Mdv::EraNc2Mdv(int argc, char **argv) {

  // Make sure the singleton wasn't already created.
  assert(_instance == (EraNc2Mdv *)NULL);

  // Set the singleton instance pointer
  _instance = this;

  okay = true;

  // Set the base program name.
  path_parts_t progname_parts;

  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);

  if(progname_parts.dir != NULL)
    free(progname_parts.dir);
  if(progname_parts.name != NULL)
    free(progname_parts.name);
  if(progname_parts.base != NULL)
    free(progname_parts.base);
  if(progname_parts.ext != NULL)
    free(progname_parts.ext);

  // Display UCAR ucopyright message.
  ucopyright(_progName);

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
  //  char *params_path = new char[300];
  //  strcpy(params_path, "unknown");

  //  if (_params->loadFromArgs(argc, argv, _args->override.list, &params_path))
  if (_params->loadFromArgs(argc, argv, _args->override.list, NULL))  
  {
    //    fprintf(stderr, "ERROR: Problem reading TDRP parameters in file <%s>\n", params_path);
    fprintf(stderr, "ERROR: Problem reading TDRP parameters\n");    
    okay = false;
    return;
  }

  _grib2Mdv = new ReadNcFiles(*_params);
 
  // initialize process registration
  PMU_auto_init(_progName, _params->instance,
                _params->procmap_register_interval_secs);
  
}

//
// Destructor
EraNc2Mdv::~EraNc2Mdv() 
{
   //
   // Unregister with process mapper
   //
   PMU_auto_unregister();

  if (_params != (Params *)NULL)
    delete _params;
  if (_args != (Args *)NULL)
    delete _args;

  // Free included strings
  STRfree(_progName);

  if (_grib2Mdv != (ReadNcFiles *)NULL)
    delete _grib2Mdv;
}

//
// Inst() - Retrieve the singleton instance of this class.
EraNc2Mdv *EraNc2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (EraNc2Mdv *)NULL)
    new EraNc2Mdv(argc, argv);

  return(_instance);
}

EraNc2Mdv *EraNc2Mdv::Inst()
{
  assert(_instance != (EraNc2Mdv *)NULL);
  return(_instance);
}

int EraNc2Mdv::run()
{

  //
  // Initialize and run the ReadNcFiles object
  //
  if( _args->_nFiles > 0 ) {
    if( _grib2Mdv->init (_args->_nFiles, _args->_fileList, _args->_printVarList, 
			 _args->_printSummary, _args->_printSections ) )
      return ( RI_FAILURE );
  }
  else {
    if( _grib2Mdv->init (0, NULL, _args->_printVarList,
			 _args->_printSummary, _args->_printSections ) != RI_SUCCESS )
      return( RI_FAILURE );
  }

  if( _grib2Mdv->getData() != RI_SUCCESS ) {
    return( RI_FAILURE );
  }

  return( RI_SUCCESS );
}
