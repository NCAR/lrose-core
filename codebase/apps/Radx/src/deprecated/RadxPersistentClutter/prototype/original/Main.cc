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
#include "RadxPersistentClutterFirstPass.hh"
#include "RadxPersistentClutterSecondPass.hh"
#include <csignal>
#include <new>
#include <iostream>
#include <toolsa/LogMsg.hh>

static void tidy_and_exit (int sig);
static void out_of_store(void);
static RadxPersistentClutterFirstPass *Prog = NULL;
static RadxPersistentClutterSecondPass *Prog2 = NULL;

//----------------------------------------------------------------------
int main(int argc, char **argv)

{

  // create program object

  Prog = new RadxPersistentClutterFirstPass(argc, argv, tidy_and_exit,
					    out_of_store);
  if (!Prog->OK)
  {
    LOG(LogMsg::FATAL,
	"Could not create RadxPersistentClutterFirstPass object.");
    return(1);
  }

  int iret = 0;
  if (!Prog->run())
  {
    LOG(LogMsg::ERROR, "running RadxPersistentClutter First pass");
    iret = 1;
  }
  else
  {
    Prog2 = new RadxPersistentClutterSecondPass(*Prog);
    delete Prog;
    Prog = NULL;
    if (!Prog2->OK)
    {
      LOG(LogMsg::FATAL,
	  "Could not create RadxPersistentClutterSecondPass object.");
      return(1);
    }
    if (Prog2->run())
    {
      iret = 0;
    }
    else
    {
      LOG(LogMsg::ERROR, "running RadxPersistentClutter Second pass");
      iret = 1;
    }
  }
  // clean up
  tidy_and_exit(iret);
  return (iret);
  
}

//----------------------------------------------------------------------
// tidy up on exit
static void tidy_and_exit (int sig)
{
  if (Prog)
  {
    delete Prog;
    Prog = NULL;
  }
  if (Prog2)
  {
    delete Prog2;
    Prog2 = NULL;
  }
  exit(sig);
}

//----------------------------------------------------------------------
// Handle out-of-memory conditions
static void out_of_store(void)
{
  LOG(LogMsg::FATAL, "Operator new failed - out of store");
  exit(-1);
}
