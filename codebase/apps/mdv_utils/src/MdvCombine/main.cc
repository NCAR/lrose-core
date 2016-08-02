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
 *  $Id: main.cc,v 1.6 2016/03/04 02:22:10 dixon Exp $
 *
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/


/////////////////////////////////////////////////////////////////////////
//
// Module:	MdvCombine
//
// Author:	G M Cunning
//
// Date:	Tue Jan 16 11:31:55 2001
//
// Description: This program collects MDV data from several input directories and copies data to a single output directory
//



// C++ include files
#include <csignal>
#include <cstdlib>

// System/RAP include files
#include <toolsa/port.h>

// Local include files
#include "MdvCombine.hh"
using namespace std;

static void tidy_and_exit(int sig);
static MdvCombine *mainObj = 0;

/////////////////////////////////////////////////////////////////////////
//
// Function Name: 	main
// 
// Description:	main routine for MdvCombine
// 
// Returns:	-1 for failure and iret id program runs
// 
// Globals:	
// 
// Notes:
//

int main(int argc, char **argv)
{

  // create main object
  mainObj = MdvCombine::instance(argc, argv);
  if (!mainObj->isOk()) {
      tidy_and_exit(-1);
  }

  // set signal handling
  PORTsignal( SIGINT, tidy_and_exit);
  PORTsignal( SIGHUP, tidy_and_exit );
  PORTsignal( SIGTERM, tidy_and_exit );
  PORTsignal( SIGPIPE, (PORTsigfunc)SIG_IGN );

  // run it
  bool iret = mainObj->execute();

  // clean up and exit
  if(iret) 
    tidy_and_exit(0);
  else
    tidy_and_exit(-1);


}


///////////////////
// tidy up on exit

static void tidy_and_exit(int sig)

{
  delete mainObj;
  exit( sig );
}

