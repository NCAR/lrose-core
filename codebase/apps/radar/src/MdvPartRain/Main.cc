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
//
// main for MdvPartRain
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2008
//
///////////////////////////////////////////////////////////////
//
// MdvPartRain reads dual pol data from an MDV file, computes
// derived fields and writes the fields to an output file.
//
////////////////////////////////////////////////////////////////

/**
 * @file Main.cc
 * @brief The entry point of the application
 * @mainpage MdvPartRain
 * MdvPartRain reads dual pol data from an MDV file, computes
 * derived fields and writes the fields to an output file.
 * Particles are identified using a fuzzy logic approach. For each radar variable, all 
 * of the particles receive a value between 0 and 1 from the respective
 * membership function (fuzzification). These values are then multiplied by the
 * appropriate weight and summed for each particle. The maximum of the weighted
 * sums is then found to determine the particle type.
 */

#include "MdvPartRain.hh"
#include <toolsa/str.h>
#include <toolsa/port.h>
#include <signal.h>
#include <new>
using namespace std;

// file scope

/**
 * Exit the application gracefully
 * @param[in] The exit status flag (0=ok, -1=error)
 */
static void tidy_and_exit (int sig);

/**
 * Handle out-of-memory conditions
 */
static void out_of_store();


static MdvPartRain *_prog; /**< The singleton handle for the application object */
static int _argc; /**< The number of command line arguments */
static char **_argv; /**< The list of command line arguments */


/**
 * The entry point of the application
 * @param[in] argc The number of command line arguments
 * @param[in] argv The list of command line arguments
 */
int main(int argc, char **argv)

{

  _argc = argc;
  _argv = argv;

  // create program object

  _prog = new MdvPartRain(argc, argv);
  if (!_prog->isOK) {
    return(-1);
  }

  // set signal handling
  
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

  // set new() memory failure handler function

  set_new_handler(out_of_store);

  // run it

  int iret = _prog->Run();

  // clean up

  tidy_and_exit(iret);
  return (iret);
  
}

///////////////////
// tidy up on exit

static void tidy_and_exit (int sig)

{

  delete(_prog);
  exit(sig);

}
////////////////////////////////////
// out_of_store()
//
// Handle out-of-memory conditions
//

static void out_of_store()

{

  fprintf(stderr, "FATAL ERROR - program MdvPartRain\n");
  fprintf(stderr, "  Operator new failed - out of store\n");
  exit(-1);

}
