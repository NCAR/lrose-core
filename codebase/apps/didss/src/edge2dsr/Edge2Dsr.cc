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
//////////////////////////////////////////////////////////////////////////////
// $Id: Edge2Dsr.cc,v 1.10 2016/03/06 23:53:42 dixon Exp $
//
// Top level application class
/////////////////////////////////////////////////////////////////////////////
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/MsgLog.hh>

#include "Edge2Dsr.hh"
#include "DataMgr.hh"
using namespace std;

Edge2Dsr::Edge2Dsr()
{
   params  = NULL;
   msgLog  = new MsgLog();
   dataMgr = new DataMgr();
   
}

Edge2Dsr::~Edge2Dsr() 
{
   if( params ) 
      delete params;
   if( msgLog )
      delete msgLog;
   if( dataMgr )
      delete dataMgr;
}

int
Edge2Dsr::init( int argc, char **argv )
{
   tdrp_override_t override;

   //
   // Some general stuff
   //
   program.setPath( argv[0] );

   TDRP_init_override( &override );   

   //
   // Process the arguments
   //
   if ( processArgs( argc, argv, override ) != SUCCESS )
      return( FAILURE );              

   //
   // Print ucopright message
   //
   ucopyright( (char*)PROGRAM_NAME ); 

   //
   // Read the parameter file
   //
   params = new Params();
   paramsPath = "unknown";
   if( params->loadFromArgs(argc, argv,
			    override.list,
			    &paramsPath)) {
      POSTMSG( ERROR, "Problem with TDRP parameters");
      return( FAILURE );
   }

   TDRP_free_override( &override ); 

   //
   // Register with procmap now that we have the instance name
   //
   if ( params->instance == NULL )
      params->instance = "generic";
   PMU_auto_init( (char*)PROGRAM_NAME, params->instance, 
                  PROCMAP_REGISTER_INTERVAL );
   PMU_auto_register( "starting up edge2dsr" );

   //
   // Process the parameters
   //
   if ( processParams() != SUCCESS )
      return( FAILURE );

   return( SUCCESS );
}

void
Edge2Dsr::usage()
{
   //
   // New-style command lines
   //
   cerr << "Usage: " << PROGRAM_NAME   << " [options as below]\n"
        << "       [ -debug ]               produce debug messages\n"
        << "       [ -summary [n]]          print summary each n records"
        << " (n default 90)\n"
        << "       [ -archive [file path]]  write raw radar data to"
        << " archive file\n"
        << "       [ --, -h, -help, -man ]  produce this list\n"
        << endl;

   TDRP_usage( stdout );
   
   dieGracefully( -1 );
}

int
Edge2Dsr::processArgs( int argc, char **argv,
                        tdrp_override_t& override )
{
   int          i;
   int          paramArg;
   char         tmpStr[BUFSIZ];

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
         sprintf(tmpStr, "debug = TRUE;");
         TDRP_add_override(&override, tmpStr);
      }

      //
      // request to print summary
      //
      if ( !strcmp(argv[i], "-summary" )) {
      
	 sprintf(tmpStr, "print_summary = TRUE;");
         TDRP_add_override(&override, tmpStr);
      
	 if (i < argc - 1) {
	    sprintf(tmpStr, "summary_interval = %s;", argv[i+1]);
	    TDRP_add_override(&override, tmpStr);
	 }
      }

      //
      // Write raw data to archive file
      //
      if ( !strcmp(argv[i], "-archive" )) {
         
         sprintf(tmpStr, "archive_file = TRUE;");
         TDRP_add_override(&override, tmpStr);

         if (i < argc - 1) {
             sprintf(tmpStr, "archive_file_path = %s;", argv[i+1]);
             TDRP_add_override(&override, tmpStr);
         }
         else {
             POSTMSG( ERROR, "Missing archive file path" );
             usage();
         }
      }

      //
      // parameter file setting
      //
      else if ( !strcmp( argv[i], "-params") ) {
         if ( i < argc - 1 ) {
            paramArg = i+1;
         }
         else {
            POSTMSG( ERROR, "Missing parameter file name" );
            usage();
         }
      }
   }

   if ( paramArg ) {
      //
      // Test for the existence of the parameter file
      //
     if ( !Path::exists( argv[paramArg] ) ) {
         POSTMSG( ERROR, "Parameter file %s not valid", argv[paramArg] );
         return( FAILURE );
     }
     else
        paramsPath = argv[paramArg];
   }

   return( SUCCESS );
}

int
Edge2Dsr::processParams()
{
   //
   // Initialize the messaging
   //
   POSTMSG( INFO, "Initializing message log." );
   initLog();

   //
   // Initialize the data manager object
   //
   if( dataMgr->init( *params, msgLog ) )
      return ( FAILURE );

   return ( SUCCESS );
}

void
Edge2Dsr::initLog()
{
   //
   // Enable debug level messaging
   //
   if ( params->debug == TRUE )
      msgLog->enableMsg( DEBUG, true );
   else 
      msgLog->enableMsg( DEBUG, false );

   //
   // Enable info level messaging
   //
   if ( params->info == TRUE )
      msgLog->enableMsg( INFO, true );
   else
      msgLog->enableMsg( INFO, false );

}
 
int
Edge2Dsr::run()
{
    bool forever = true;

    while( forever ) {
       if( dataMgr->processData() != SUCCESS ) {
	  POSTMSG( WARNING, "Trouble reading data" );
       }
       PMU_auto_register( "Processing data" );
    }

    return( SUCCESS );
}


