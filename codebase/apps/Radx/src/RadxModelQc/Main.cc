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
 */
#include "RadxModelQc.hh"
#include <toolsa/LogStream.hh>
#include <csignal>
#include <new>
#include <iostream>

static void tidy_and_exit (int sig);
static void out_of_store(void);
static RadxModelQc *Prog = NULL;

//--------------------------------------------------------------------
int main(int argc, char **argv)

{
  // create program object
  Prog = new RadxModelQc(argc, argv, tidy_and_exit, out_of_store);
  if (!Prog->OK)
  {
    LOG(FATAL) << "Could not create RadxModelQc object.";
    return(1);
  }

  // run it
  int iret = Prog->Run();
  if (iret != 0)
  {
    LOG(ERROR) << "running RadxModelQc";
  }
  
  // clean up
  tidy_and_exit(iret);
  return (iret);
  
}

//--------------------------------------------------------------------
// tidy up on exit
static void tidy_and_exit (int sig)
{
  if (Prog) {
    delete Prog;
    Prog = NULL;
  }
  exit(sig);
}

//--------------------------------------------------------------------
// Handle out-of-memory conditions
static void out_of_store()

{
  LOG(FATAL) << "Operator new failed - out of store";
  exit(-1);
}
