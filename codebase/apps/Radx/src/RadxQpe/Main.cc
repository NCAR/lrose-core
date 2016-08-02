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
 * @brief main for RadxQpe
 *
 * Dave Albo, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
 *
 * April 2014
 *
 * RadxQpe fills a RadxVol with precip information using PID and beam
 * blockage inputs, and writes that out.
 */

#include "RadxQpeMgr.hh"

static void tidy_and_exit (int sig);
static void out_of_store(void);
static RadxQpeMgr *_prog = NULL;
static int _argc;
static char **_argv;

//----------------------------------------------------------------------
int main(int argc, char **argv)
{
  _argc = argc;
  _argv = argv;

  // create program object

  _prog = new RadxQpeMgr(argc, argv, tidy_and_exit);
  if (!_prog->_isOK)
  {
    return(-1);
  }

  // set new() memory failure handler function

  set_new_handler(out_of_store);

  // run it

  int iret = _prog->Run();

  // clean up

  tidy_and_exit(iret);
  return (iret);
}

//----------------------------------------------------------------------
// tidy up on exit

static void tidy_and_exit (int sig)

{
  if (_prog) {
    delete _prog;
    _prog = NULL;
  }
  exit(sig);

}
//----------------------------------------------------------------------
// Handle out-of-memory conditions

static void out_of_store()
{
  fprintf(stderr, "FATAL ERROR - program RadxQpe\n");
  fprintf(stderr, "  Operator new failed - out of store\n");
  exit(-1);
}



