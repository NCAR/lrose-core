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
/*
-------------------------------------------------------------------------------
SstNc2MdvMain.cc - main program for SstNc2Mdv

Steve Carson, RAP, NCAR, Boulder, CO, USA, 80307-3000
March 2006
-------------------------------------------------------------------------------
CODING CONVENTIONS

- line length limited to 80 characters

- indentation: 3 spaces per level

- curly braces:
  line up with the code they enclose;
  line up vertically;
  if( expression_is_true )
     {
     some_code();
     }

- ONE SPACE of indentation for first level of code between { and }
  of a function; thereafter THREE SPACES per level.
  This is a compromise to make the emacs color-coding of C++ work
  correctly. Normally I do not indent the first level at all since
  indenting the entire function body wastes "horizontal real estate", IMHO

- naming conventions:
   NAME TYPE                  DESCRIPTION                        EXAMPLE
   function args              start with "a"                  : aArrNRows
   function arg pointers      have a "p"                      : *apNcInputFile
   function arg references    have an "r"                     : &arPrtArgs
   local (stack) variables    lowercase C-style               : sum_wtd_data
   function (method) names    initial caps                    : LoadVariables
-------------------------------------------------------------------------------
DESIGN NOTES

-------------------------------------------------------------------------------
*/

using namespace std;

#define SstNc2MdvMain_c

//
// LIBRARY INCLUDES
//

#include <toolsa/port.h>
#include <toolsa/umisc.h>

//
// LOCAL INCLUDES
//

#include "SstNc2MdvGlobals.hh"
#include "SstNc2MdvApp.hh"

//
// File-scope variables
//

SstNc2MdvApp
   *sstnc2mdv_app;

//
// FORWARD REFERENCES
//

MsgLog&
   GetMsgLog();

MsgLog&
   GetMsgLog() { return sstnc2mdv_app->GetMsgLog(); }

void
   TidyAndExit( int sig );

/*
-------------------------------------------------------------------------------
main
-------------------------------------------------------------------------------
*/

int
main( int argc, char **argv )

{ /* begin main */

 int status;

 //
 // Trap signals for a clean exit
 //

 PORTsignal( SIGHUP,  TidyAndExit );
 PORTsignal( SIGINT,  TidyAndExit );                    
 PORTsignal( SIGTERM, TidyAndExit );
 PORTsignal( SIGQUIT, TidyAndExit );      
 PORTsignal( SIGKILL, TidyAndExit );      
 PORTsignal( SIGPIPE, (PORTsigfunc) SIG_IGN );
   
 //
 // Instantiate and initialize the top-level application class - SstNc2MdvApp
 //

 sstnc2mdv_app = new SstNc2MdvApp;

 status  = sstnc2mdv_app->Init( argc, argv );

 //
 // Print diagnostic message
 //

 if ( status == SUCCESS )
    {
    POSTMSG( INFO, "SstNc2Mdv initialized." );
    }
 else if ( status == FAILURE )
    {
    POSTMSG( ERROR, "SstNc2Mdv initialization failed." );
 
    TidyAndExit( status );
    }
 else
    {
    TidyAndExit( status );
    }

 //
 // Run the application
 //

 status = sstnc2mdv_app->Run();

 if( status == SUCCESS )
    {
    POSTMSG( INFO, "SstNc2Mdv finished successfully." );
   }
 else if( status == FAILURE )
    {
    POSTMSG( ERROR, "SstNc2Mdv finished with error" );
    }

 TidyAndExit( status );

} // end main

/*
-------------------------------------------------------------------------------
TidyAndExit

SstNc2Mdv exit point

Steve Carson, RAL, NCAR, Boulder, CO, USA, 80307-3000
March 2006
-------------------------------------------------------------------------------
*/

#include <toolsa/pmu.h>

void TidyAndExit( int sig )
{

 fprintf(stderr, "\nExiting application with signal or status = %d\n", sig);

 //
 // Remove the top-level application class
 //

 delete sstnc2mdv_app;

 //
 // Unregister with process mapper
 //

 PMU_auto_unregister();
 exit( sig );
}
