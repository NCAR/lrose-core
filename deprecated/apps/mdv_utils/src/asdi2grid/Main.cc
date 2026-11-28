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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
 *
 * RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2017/09/09 21:22:28 $
 *   $Revision: 1.3 $
 *
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/******************************************************************************

 Main.cc - main program for asdi2grid

******************************************************************************/

/*
-----------------------
-- RAP LIBRARY INCLUDES
-----------------------
*/

#include <toolsa/port.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>

/*
-----------------
-- LOCAL INCLUDES
-----------------
*/

#include "asdi2grid.hh"
#include "Args.hh"


void dieGracefully( int sig );

asdi2grid *prog;

int main( int argc, char **argv )
{

  int status;

  //
  // Trap signals for a clean exit
  //
  
  PORTsignal( SIGHUP,  dieGracefully );
  PORTsignal( SIGINT,  dieGracefully );                    
  PORTsignal( SIGTERM, dieGracefully );
  PORTsignal( SIGQUIT, dieGracefully );      
  PORTsignal( SIGKILL, dieGracefully );      
  PORTsignal( SIGPIPE, (PORTsigfunc) SIG_IGN );

  Path program;
  program.setPath( argv[0] );

  // Get the CLI arguments and load params
  Args args;
  Params *P = new  Params(); 
  args.parse(argc, argv, program.getFile(), P);
    
  //                                                        
  // Instantiate and initialize the top-level application class
  //                              
  prog = new asdi2grid(P, program.getFile().c_str());
  
  //
  // Startup the application -- the end.
  //
  status = prog->run(args.inputFileList, args.startTime, args.endTime);
  dieGracefully( status );
}

void dieGracefully( int sig )
{
   fprintf(stdout,"Exiting application with signal %d\n", sig );

   //
   // Remove the top-level application class                                   
   // 
   delete prog;

   //
   // Unregister with process mapper
   //
   PMU_auto_unregister();
   exit( sig );
}
