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
 * @brief Converts satellite NetCDF file of known format to Mdv.This is not
 *        a general netCDF file converter. Assumptions are made about the 
 *        data points, and user expected know field names and data types.
 */
#include <toolsa/str.h>
#include <toolsa/port.h>
#include <toolsa/pmu.h>
#include <signal.h>
#include <new>
#include <iostream>
using namespace std;

#define _MSG3SAT@MDV_MAIN_
#include "MSG3Sat2Mdv.hh"

static MSG3Sat2Mdv *msg3Sat2Mdv;

static void tidy_and_exit (int sig);

static void out_of_store();

int main(int argc, char **argv)

{  
  // create program object

  msg3Sat2Mdv = new MSG3Sat2Mdv(argc, argv);
  
  if (!msg3Sat2Mdv->isOK) 
  {
    return(1);
  }

  // set signal handling
  
  PORTsignal(SIGINT, tidy_and_exit);
  
  PORTsignal(SIGHUP, tidy_and_exit);
  
  PORTsignal(SIGTERM, tidy_and_exit);
  
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

  // set new() memory failure handler function

  set_new_handler(out_of_store);

  // run it

  int iret = msg3Sat2Mdv->Run();

  // clean up

  tidy_and_exit(iret);

  return (iret);
}

// tidy up on exit

static void tidy_and_exit (int sig)

{
  delete(msg3Sat2Mdv);
  
  exit(sig);
}

////////////////////////////////////
// out_of_store()
//
// Handle out-of-memory conditions
//

static void out_of_store()

{
  cerr << "FATAL ERROR - program msg3Sat2Mdv " << endl;
  cerr << "  Operator new failed - out of store" << endl;
  exit(1);
}
