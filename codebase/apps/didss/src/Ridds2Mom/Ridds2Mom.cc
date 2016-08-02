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
////////////////////////////////////////////////////////////////////////////
//  Ridds2Mom top-level application class
//
//  $Id: Ridds2Mom.cc,v 1.3 2016/03/06 23:53:41 dixon Exp $
//
//////////////////////////////////////////////////////////////////////////
#include <toolsa/MsgLog.hh>

#include "Ridds2Mom.hh"
#include "DataMgr.hh"
using namespace std;

Ridds2Mom::Ridds2Mom()
{
   paramsPath = NULL;
}

Ridds2Mom::~Ridds2Mom() 
{
}

int
Ridds2Mom::init( int argc, char **argv )
{
   tdrp_override_t   override;
 
   //
   // Process the arguments
   //
   TDRP_init_override( &override );   
   if ( processArgs( argc, argv, override ) != DSR_SUCCESS )
      return( DSR_FAILURE );              

   //
   // Initialize the message logging
   //
   program.setPath( argv[0] );
   msgLog.setApplication( getProgramName() );

   initLog();
   
   //
   // Print ucopright message
   //
   ucopyright( (char*) PROGRAM_NAME ); 

   //
   // Read the parameter file
   //
   paramsPath = "unknown";
   if( params.loadFromArgs( argc, argv, override.list, &paramsPath ) ) {
      POSTMSG( ERROR, "Problem with TDRP parameters" );
      return( DSR_FAILURE );
   }

   TDRP_free_override( &override );                     

   //
   // Register with procmap now that we have the instance name
   //
   PMU_auto_init( (char*) PROGRAM_NAME, params.instance, 
                  PROCMAP_REGISTER_INTERVAL );
   PMU_auto_register( "starting up Ridds2Mom" );

   //
   // Initialize the data manager
   //
   if( dataMgr.init( params, msgLog ) != DSR_SUCCESS ) 
      return( DSR_FAILURE );

   return( DSR_SUCCESS );
}

void
Ridds2Mom::usage()
{
   //
   // New-style command lines
   //
   cerr << "Usage: " << PROGRAM_NAME << "  [options as below]\n"
        << "       [--, -h, -help, -man] produce this list.\n"
	<< "       [-debug] print debug messages\n"
	<< "       [-verbose] print verbose debug messages\n"
	<< "       [-filelist] list files only, overrides most other options\n"
	<< "       [-header [n]] print header each n records (n default 360)\n"
	<< "       [-mdebug ?] set malloc debug level\n"
	<< "       [-summary [n]] print summary each n records"
        << " (n default 90)\n"
        << endl;

   TDRP_usage( stdout );
   
   dieGracefully( 0 );
}

int
Ridds2Mom::processArgs( int argc, char** argv, tdrp_override_t& override )
{
   int  i;
   int  paramArg;
   char tmpStr[BUFSIZ];

   //
   // Parameter initializations
   //
   paramArg = 0;

   //
   // Process each argument
   //
   for( i=1; i < argc; i++ ) {
      //
      // request for usage information
      //
      if ( !strcmp(argv[i], "--" ) ||
           !strcmp(argv[i], "-h" ) ||
           !strcmp(argv[i], "-help" ) ||
           !strcmp(argv[i], "-man" )) {
         usage();
      }

      //
      // request for verbose debug messaging
      //
      if ( !strcmp(argv[i], "-debug" )) {
         sprintf( tmpStr, "debug = TRUE;" );
         TDRP_add_override( &override, tmpStr );
      }

      //
      // parameter file setting
      //
      else if ( !strcmp( argv[i], "-params") ) {
         if ( i < argc - 1 ) {
            paramArg = i+1;
         }
         else {
            POSTMSG( ERROR, "Missing parameter file name");
            usage();
         }
      }

      else if( !strcmp( argv[i], "-summary" ) ) {
	 sprintf( tmpStr, "print_summary = TRUE;" );
	 TDRP_add_override( &override, tmpStr );
	 
         if( i < argc - 1 && argv[i+1][0] != '-' ) {
	    sprintf( tmpStr, "summary_interval = %s;", argv[i+1] );
	    TDRP_add_override( &override, tmpStr );
	 }
      }
   }

   if ( paramArg ) {
      //
      // Test for the existence of the parameter file
      //
      if ( !Path::exists( argv[paramArg] ) )
         return( DSR_FAILURE );
      else
         paramsPath = argv[paramArg];
   }

   return( DSR_SUCCESS );
}


void
Ridds2Mom::initLog()
{
   //
   // Enable debug level messaging
   //
   if ( params.debug == TRUE )
      msgLog.enableMsg( DEBUG, true );
   else 
      msgLog.enableMsg( DEBUG, false );

   //
   // Enable info level messaging
   //
   if ( params.info == TRUE )
      msgLog.enableMsg( INFO, true );
   else
      msgLog.enableMsg( INFO, false );

   //
   // Direct message logging to a file
   //
   if ( msgLog.setOutputDir( params.log_dir ) != 0 ) {
      POSTMSG( WARNING, "Cannot write log messages to output directory '%s'",
                        params.log_dir );
   }

}
 
int
Ridds2Mom::run()
{
    PMU_auto_register( "Processing data" );
    dataMgr.processData();
    return(DSR_SUCCESS);
}


