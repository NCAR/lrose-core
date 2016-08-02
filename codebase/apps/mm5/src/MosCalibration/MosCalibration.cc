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
////////////////////////////////////////////////////////////
//
//  mosCalibration top-level application class
//
//  $Id: MosCalibration.cc,v 1.11 2016/03/07 01:33:50 dixon Exp $
//
///////////////////////////////////////////////////////////
#include <string>
#include <cassert>
#include <sys/stat.h>
#include <toolsa/MsgLog.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>

#include "DataMgr.hh"
#include "MosCalibration.hh"
using namespace std;

//
// Constants
//
const int MosCalibration::DAYS_TO_SEC = 24 * 60 * 60;

MosCalibration::MosCalibration()
{
   paramsPath   = NULL;
   startTime    = DateTime::NEVER;
   endTime      = time( NULL );
}

int MosCalibration::init( int argc, char **argv )
{
   //
   // Some general stuff
   //
   program.setPath( argv[0] );
   msgLog.setApplication( getProgramName() );

   if ( processArgs( argc, argv ) != SUCCESS )
      return( FAILURE );                 

   ucopyright( (char*)PROGRAM_NAME );

   if ( readParams( argc, argv ) != SUCCESS )
      return( FAILURE );                      

   //
   // Register with procmap now that we have the instance name
   //
   PMU_auto_init( (char*)PROGRAM_NAME, params.instance, 
                  PROCMAP_REGISTER_INTERVAL );
   PMU_auto_register( "starting up MosCalibration" );

   //
   // Register with procmap now that we have the instance name
   //
   PMU_auto_init( (char*)PROGRAM_NAME, params.instance, 
                  PROCMAP_REGISTER_INTERVAL );
   PMU_auto_register( "starting up MosCalibration" );


   //
   // Process the parameters
   //
   if ( processParams() != SUCCESS )
      return( FAILURE );

   return( SUCCESS );
}

void MosCalibration::usage()
{
   //
   // New-style command lines
   //
   cerr << "Usage: " << PROGRAM_NAME   << " [options as below]\n"
        << "       [ -debug ]                        produce verbose debug messages\n"
        << "       [ -end \"yyyy/mm/dd hh:mm:ss\"]   end time\n"
        << "       [ --, -h, -help, -man ]           produce this list\n"
        << "       [ -start \"yyyy/mm/dd hh:mm:ss\"] start time\n"
        << endl;
   cerr << "Note: If start time is not specified on the command line, it\n"
        << "will be calculated by subtracting the value of the look_back\n"
        << "parameter in the parameter file from the end time.  If the end\n"
        << "time is not set, the current time will be used for the end time\n"
        << endl;
   dieGracefully( -1 );
}

int MosCalibration::processArgs( int argc, char **argv )
{

   TDRP_init_override( &tdrpOverride );

   //
   // Process each argument
   //
   for( int i = 1; i < argc; i++ ) {
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
         TDRP_add_override( &tdrpOverride, "debug = TRUE;" );
      }

      //
      // start time specification
      //
      if ( !strcmp(argv[i], "-start" )) {
         assert( ++i <= argc );
         startTime = DateTime::parseDateTime( argv[i] );
         if ( startTime == DateTime::NEVER ) {
            POSTMSG( ERROR, "Bad date/time syntax in -start "
                            "specification." );
            usage();
         }
      }

      //
      // end time specification
      //
      if ( !strcmp(argv[i], "-end" )) {
         assert( ++i < argc );
         endTime = DateTime::parseDateTime( argv[i] );
         if ( endTime == DateTime::NEVER ) {
            POSTMSG( ERROR, "Bad date/time syntax in -end "
                            "specification." );
            usage();
         }
      }

   }

   return( SUCCESS );
}

int MosCalibration::readParams( int argc, char **argv )
{
   int             status = 0;

   //
   // Read the parameter file
   //
   if( params.loadFromArgs( argc, argv, tdrpOverride.list,
                            &paramsPath ) != 0 ) {
      status = -1;
   }
   TDRP_free_override( &tdrpOverride );

   //
   // Make sure the read worked
   //
   if ( status == -1 ) {
      POSTMSG( ERROR, "Unable to load parameters." );
      if ( paramsPath )
         POSTMSG( ERROR, "Check syntax of parameter file" );
      return( FAILURE );
   }

   return( SUCCESS );
}

int
MosCalibration::processParams()
{
   //
   // Set the start time if it has not been set on the command line
   //
   if( startTime == DateTime::NEVER ) {
      startTime = endTime - params.look_back * DAYS_TO_SEC;
   }

   //
   // Enable debug level messaging
   //
   if ( params.debug )
      msgLog.enableMsg( DEBUG, true );

   //
   // Enable info level messaging
   //
   if ( params.info )
      msgLog.enableMsg( INFO, true );

   //
   // Direct message logging to a file
   //
   if ( msgLog.setOutputDir( params.log_dir ) != 0 ) {
      POSTMSG( WARNING, "Cannot write log messages to output directory '%s'",
                        params.log_dir );
   }

   //
   // Tell the user what we are doing
   //
   DateTime whenStart( startTime );
   DateTime whenEnd( endTime );
   
   POSTMSG( INFO, "Loading Parameter file: %s", paramsPath );
   POSTMSG( INFO, "  Start time = %s", whenStart.dtime() );
   POSTMSG( INFO, "  End time = %s", whenEnd.dtime() );

   return( SUCCESS );
}

int
MosCalibration::run()
{
   PMU_auto_register( "Initializing" );
   
   int status = dataMgr.init( params, startTime, endTime );

   if( status != SUCCESS ) {
      POSTMSG( ERROR, "Initialization failed" );
      return( FAILURE );
   }
   
   PMU_auto_register( "Processing data" );
   
   POSTMSG( INFO, "run: before processData" );
   status = dataMgr.processData();
   POSTMSG( INFO, "run: after processData" );

   if( status != SUCCESS ) {
      POSTMSG( ERROR, "Cannot process the data" );
      return( FAILURE );
   }

   
   return( SUCCESS );
}
