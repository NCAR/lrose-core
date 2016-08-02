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
 *  $Id: Main.cc,v 1.3 2016/03/04 02:22:11 dixon Exp $
 *
 */

# ifndef    lint
static char RCSid[] = "$Id: Main.cc,v 1.3 2016/03/04 02:22:11 dixon Exp $";
# endif     /* not lint */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/


/////////////////////////////////////////////////////////////////////////
//
// Module:	MdvFixTimes
//
// Author:	G. M. Cunning
//
// Date:	Fri Apr 22 16:06:22 2005
//
// Description: modifies master and field header times
//



// C++ include files
#include <csignal>
#include <cstdlib>


// System/RAP include files
#include <toolsa/port.h>

// Local include files
#include "MdvFixTimes.hh"

using namespace std;

static void tidy_and_exit(int sig);
static MdvFixTimes *mainObj;

/////////////////////////////////////////////////////////////////////////
//
// Function Name: 	main
// 
// Description:	main routine for MdvFixTimes
// 
// Returns:	-1 for failure and iret if program runs
// 
// Globals:	
// 
// Notes:
//

int 
main(int argc, char **argv)
{

  // create main object
  mainObj = MdvFixTimes::instance(argc, argv);
  if(!mainObj->isOK()) {
    tidy_and_exit(-1);
  }

  mainObj->initialize();
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

