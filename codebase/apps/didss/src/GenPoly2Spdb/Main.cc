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

/**
 * @file Main.cc
 *
 * GenPoly2Spdb reads ascii data and converts to GenPoly, writing to SPDB
 */

#include "GenPoly2Spdb.hh"
#include <toolsa/port.h>
#include <toolsa/LogStream.hh>
#include <signal.h>
#include <new>

static void tidy_and_exit (int sig);
static void out_of_store();
static GenPoly2Spdb *_prog = NULL;

//---------------------------------------------------------------------
int main(int argc, char **argv)
{
  // create program object
  _prog = new GenPoly2Spdb(argc, argv);
  if (!_prog->isOK)
  {
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

//---------------------------------------------------------------------
static void tidy_and_exit (int sig)
{
  if (_prog != NULL)
  {
    delete(_prog);
  }
  exit(sig);
}

//---------------------------------------------------------------------
static void out_of_store()
{
  LOG(FATAL) << "Operator new failed - out of store";
  exit(-1);
}



