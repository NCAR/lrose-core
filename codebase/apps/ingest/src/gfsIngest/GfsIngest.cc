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
//  gfsIngest top-level application class
//
//////////////////////////////////////////////////////////////////////////
#include <cassert>
#include <cstdio>
#include <string.h>
#include <ctime>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>

#include "GfsIngest.hh"
#include "Grib2Mdv.hh"
using namespace std;

// Global variables

gfsIngest *gfsIngest::_instance = (gfsIngest *)NULL;


/*********************************************************************
 * Constructor
 */

gfsIngest::gfsIngest(int argc, char **argv) {

  const char *routine_name = "Constructor";

  // Make sure the singleton wasn't already created.

  assert(_instance == (gfsIngest *)NULL);

  // Set the singleton instance pointer

  _instance = this;

  // Initialize the okay flag.

  okay = true;

  // Set the program name.

  path_parts_t progname_parts;

  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);

  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);

  if (!_args->okay)
  {
    fprintf(stderr,
    "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr,
    "Problem with command line arguments.\n");

    okay = false;

    return;
  }

  // Get TDRP parameters.
  _params = new Params();
  char *params_path = "unknown";

  if (_params->loadFromArgs(argc, argv, _args->override.list, &params_path))
  {
    fprintf(stderr,
    "ERROR: %s::%s\n", _className(), "init");
    fprintf(stderr, "Problem with TDRP parameters in file <%s>\n",
                                                     params_path);
    okay = false;
    return;
  }

  _grib2Mdv = new Grib2Mdv(*_params);
 
  // initialize process registration

  PMU_auto_init( _className(), _params->instance, 
		 PROCMAP_REGISTER_INTERVAL );

}

/*********************************************************************
 * Destructor
 */

gfsIngest::~gfsIngest() 
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

  if (_grib2Mdv != (Grib2Mdv *)NULL)
    delete _grib2Mdv;
}

/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

gfsIngest *gfsIngest::Inst(int argc, char **argv)
{
  if (_instance == (gfsIngest *)NULL)
    new gfsIngest(argc, argv);

  return(_instance);
}

gfsIngest *gfsIngest::Inst()
{
  assert(_instance != (gfsIngest *)NULL);
  return(_instance);
}

int
gfsIngest::run()
{

  //
  // Initialize the data manager
  //
  if( _args->_nFiles > 0 ) {
    if( _grib2Mdv->init (_args->_nFiles, _args->_fileList, _args->_printSummary ) )
      return ( RI_FAILURE );
  }
  else {
    if( _grib2Mdv->init (0, NULL, _args->_printSummary ) != RI_SUCCESS )
      return( RI_FAILURE );
  }

  if( _grib2Mdv->getData() != RI_SUCCESS ) {
    return( RI_FAILURE );
  }

  return( RI_SUCCESS );
}


