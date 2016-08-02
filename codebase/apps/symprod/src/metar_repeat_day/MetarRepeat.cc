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
///////////////////////////////////////////////////////////////////////////
//  metar_repeat_day top-level application class
//
//  $Id: MetarRepeat.cc,v 1.5 2016/03/06 23:31:57 dixon Exp $
//
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/MsgLog.hh>

#include "MetarRepeat.hh"
#include "DataMgr.hh"

metar_repeat_day_tdrp_struct params;

MetarRepeat::MetarRepeat()
{
   msgLog        = new MsgLog();
   dataMgr       = new DataMgr();
   filePrefix    = NULL;
}

MetarRepeat::~MetarRepeat() 
{
   if( dataMgr )
      delete dataMgr;
   if( msgLog )
      delete msgLog;
   if( filePrefix )
      STRfree( filePrefix );
}

int
MetarRepeat::init( int argc, char **argv )
{
   bool            printParams, checkParams;
   tdrp_override_t override;

   //
   // Some general stuff
   //
   program.setPath( argv[0] );
   msgLog->setApplication( getProgramName() );

   TDRP_init_override( &override );   

   if ( processArgs( argc, argv, &checkParams, 
                     &printParams, override ) != SUCCESS )
      return( FAILURE );              

   ucopyright( (char*)PROGRAM_NAME );
   if ( readParams( override, checkParams, printParams ) != SUCCESS )
      return( FAILURE ); 

   TDRP_free_override( &override );                     

   //
   // Register with procmap now that we have the instance name
   //
   if ( params.instance == NULL )
     params.instance = (char *) "generic";
   PMU_auto_init( (char*)PROGRAM_NAME, params.instance, 
                  PROCMAP_REGISTER_INTERVAL );
   PMU_auto_register( "starting up metar_repeat_day" );

   //
   // Process the parameters
   //
   if ( processParams() != SUCCESS )
      return( FAILURE );

   return( SUCCESS );
}

void
MetarRepeat::usage()
{
   //
   // New-style command lines
   //
   cerr << "Usage: " << PROGRAM_NAME   << " [options as below]\n"
        << "       [ -check_params ]        check parameter settings\n"
        << "       [ -debug ]               produce debug messages\n"
        << "       [ --, -h, -help, -man ]  produce this list\n"
        << "       [ -params params_file ]  set parameter file name\n"
        << "       [ -print_params ]        produce parameter listing\n"
        << endl;
   dieGracefully( -1 );
}

int
MetarRepeat::processArgs( int argc, char **argv,
                         bool *checkParams, bool *printParams,
                         tdrp_override_t& override )
{
   int          i;
   int          paramArg;
   char         tmpStr[BUFSIZ];

   //
   // Parameter initializations
   //
   paramArg = 0;
   *printParams = *checkParams = false;

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
      // requests for parameter information
      //
      else if ( !strcmp( argv[i], "-print_params" )) {
         *printParams = true;
      }
      else if ( !strcmp( argv[i], "-check_params" )) {
         *checkParams = true;
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
   }

   if ( paramArg ) {
      //
      // Test for the existence of the parameter file
      //
      if ( !Path::exists( argv[paramArg] ) )
         return( FAILURE );
      else
         paramPath = argv[paramArg];
   }

   return( SUCCESS );
}

int
MetarRepeat::readParams( tdrp_override_t& override,
                        bool checkParams, bool printParams )
{
   int             status = SUCCESS;
   char           *readPath;
   TDRPtable      *table;

   //
   // Degenerate case
   //
   if ( paramPath.isValid() )
      readPath = (char*)paramPath.getPath().c_str();
   else
      readPath = NULL;

   //
   // Read the parameter file
   //
   table = metar_repeat_day_tdrp_init( &params );
   if ( TDRP_read( readPath, table, 
                   &params, override.list ) == FALSE ) {
      status = FAILURE;
   }

   //
   // Make sure the read worked
   //
   if ( status == FAILURE ) {
      POSTMSG( ERROR, "Unable to load parameters" );
      if ( paramPath.isValid() )
         POSTMSG( ERROR, "Check syntax of parameter file: %s in %s", 
                         paramPath.getFile().c_str(), 
                         paramPath.getDirectory().c_str() );
      return( status );
   }

   //
   // Check or print parameters?
   //
   if ( checkParams ) {
      TDRP_check_is_set( table, &params );                   
      dieGracefully( 0 );
   }
   if ( printParams ) {
      TDRP_print_params( table, &params, (char*)PROGRAM_NAME, TRUE );
      dieGracefully( 0 );
   }

   return( SUCCESS );
}

int
MetarRepeat::processParams()
{
   //
   // User-specified parameter file -- do validity checks
   //
   if ( paramPath.isValid() ) {
        POSTMSG( INFO, "Loading Parameter file: %s", 
		 paramPath.getFile().c_str() );
   }

   //
   // Initialize the messaging
   //
   POSTMSG( INFO, "Initializing message log." );
   initLog(); 

   //
   // Set the file prefix
   //
   filePrefix    = STRdup(params.file_prefix);

   //
   // Initialize the data manager
   //
   if( dataMgr->init( params ) )
      return ( FAILURE );

   return ( SUCCESS );
}

void
MetarRepeat::initLog()
{
   //
   // Enable debug level messaging
   //
   if ( params.debug == TRUE )
      msgLog->enableMsg( DEBUG, true );
   else 
      msgLog->enableMsg( DEBUG, false );

   //
   // Enable info level messaging
   //
   if ( params.info == TRUE )
      msgLog->enableMsg( INFO, true );
   else
      msgLog->enableMsg( INFO, false );

   //
   // Direct message logging to a file
   //
   if ( msgLog->setOutputDir( params.log_dir ) != 0 ) {
      POSTMSG( WARNING, "Cannot write log messages to output directory '%s'",
                        params.log_dir );
   }

}
 
int
MetarRepeat::run()
{
    time_t now;
    time_t nextCheckTime = 0;
    bool   forever = true;

    //
    // Realtime mode
    //
    while( forever ) {
      now = time(0);

      if( now >= nextCheckTime ) {
	nextCheckTime = now + params.check_seconds;

	PMU_auto_register( "Processing files" );
        if( dataMgr->processFiles() != SUCCESS ) {
	   return( FAILURE );
	}

	PMU_auto_register("Waiting");
        if( params.debug ) {
	   POSTMSG( DEBUG, "Next Scan of data base scheduled for %s",
		    asctime( gmtime(&nextCheckTime) ));
	}
	 
      } else {
	PMU_auto_register("Waiting for next processing time");
      }

      PMU_auto_register("Waiting again");
      sleep(1);
		  
    }

    return(SUCCESS);
}


