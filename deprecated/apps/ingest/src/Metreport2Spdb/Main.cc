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
// main for MetReport2Spdb
//
// Gary Cunning, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
///////////////////////////////////////////////////////////////
//
// Metreport2Spdb reads 'MET REPORT' and 'SPECIAL' bulletins
// from text files and stores them in SPDB format.
//
// The SPDB format is the ASCII chunk
//
////////////////////////////////////////////////////////////////

// C++ include files
#include <csignal>
#include <cstdlib>

// System/RAP include files
#include <toolsa/port.h>

// Local include files
#include "Metreport2Spdb.hh"

// file scope

static void tidy_and_exit (int sig);
static Metreport2Spdb *mainObj;

/////////////////////////////////////////////////////////////////////////
//
// main
// 

int 
main(int argc, char **argv)
{

  // create main object
  mainObj = Metreport2Spdb::instance(argc, argv);
  if(!mainObj->isOK()) {
    tidy_and_exit(-1);
  }

  // set signal handling
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

  // run it
  int iret = mainObj->run();

  // clean up and exit
  tidy_and_exit(iret);

}


///////////////////
// tidy up on exit

static void 
tidy_and_exit(int sig)
{
  delete mainObj;
  exit( sig );
}

