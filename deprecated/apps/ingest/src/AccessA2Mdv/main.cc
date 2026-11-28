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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: main.cc,v 1.2 2016/03/07 01:22:59 dixon Exp $
 *
 */

# ifndef    lint
static char RCSid[] = "$Id: main.cc,v 1.2 2016/03/07 01:22:59 dixon Exp $";
# endif     /* not lint */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/


/////////////////////////////////////////////////////////////////////////
//
// Module:      main
//
// Author:      George McCabe
//
// Date:        Mon Sep 18 12:00:21 2000
//
// Description: This is the main for AccessA2Mdv. This program reads 
//		netcdf from the NASA Langley group.
//



// C++ include files
#include <csignal>
#include <iostream>
#include <cstdlib>

// System/RAP include files
#include <toolsa/port.h>

// Local include files
#include "AccessA2Mdv.hh"

using namespace std;

static void tidy_and_exit(int sig);
static AccessA2Mdv *mainObj = 0;

/////////////////////////////////////////////////////////////////////////
//
// Function Name:       main
// 
// Description: main routine for AccessA2Mdv
// 
// Returns:     -1 for failure and 0 for success
// 

int main(int argc, char **argv)
{

  // create main object
  mainObj =  AccessA2Mdv::instance(argc, argv);
  if (!mainObj->isOK()) {
    return(-1);
  }

  // set signal handling
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

  // run it
  int iret = mainObj->run();
  if (iret != 0) {
    cerr << mainObj->getErrStr() << endl;
  }

  // clean up and exit
  tidy_and_exit(iret);
}


///////////////////
// tidy up on exit

static void tidy_and_exit (int sig)

{

  delete mainObj;
  exit(sig);
  

}
