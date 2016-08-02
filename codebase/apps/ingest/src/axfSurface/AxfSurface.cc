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
////////////////////////////////////////////////////////////////////////////////
//
//  axfSurface top-level application class
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  August 1999
//
//  $Id: AxfSurface.cc,v 1.5 2016/03/07 01:23:07 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <sys/stat.h>
#include <toolsa/utim.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/MsgLog.hh>

#include "DataMgr.hh"
#include "AxfSurface.hh"
using namespace std;

//
// static definitions
//
const string AxfSurface::version = "1.0";

int 
AxfSurface::init( int argc, char **argv )
{
   //
   // Some general stuff
   //
   program.setPath( argv[0] );
   msgLog.setApplication( getProgramName() );

   if ( processArgs( argc, argv ) != 0 )
      return( -1 );                 

   ucopyright( (char*)PROGRAM_NAME );
   if ( readParams( argc, argv ) != 0 )
      return( -1 );                      

   //
   // Register with procmap now that we have the instance name
   //
   PMU_auto_init( (char*)PROGRAM_NAME, params.instance, 
                  PROCMAP_REGISTER_INTERVAL );
   PMU_auto_register( "starting up axfSurface" );

   //
   // Process the parameters
   //
   if ( processParams() != 0 )
      return( -1 );

   return( 0 );
}

void 
AxfSurface::usage()
{
   //
   // New-style command lines
   //
   cerr << "Usage: " << PROGRAM_NAME   << " [options as below]\n"
        << "       [ -check_params ]        check parameter settings\n"
        << "       [ -debug ]               produce verbose debug messages\n"
        << "       [ --, -h, -help, -man ]  produce this list\n"
        << "       [ -params params_file ]  set parameter file name\n"
        << "       [ -print_params ]        produce parameter listing\n"
        << "       [ -v ]                   display version number\n"
        << endl;
   dieGracefully( -1 );
}

int 
AxfSurface::processArgs( int argc, char **argv )
{
   //
   // Process each argument
   //
   for( int i=1; i < argc; i++ ) {
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
      // request for version number
      //
      if ( !strcmp(argv[i], "-v" ) ||
           !strcmp(argv[i], "-version" )) {
         POSTMSG( "version %s", version.c_str() );
         dieGracefully( 0 );
      }

      //
      // request for verbose debug messaging
      //
      if ( !strcmp(argv[i], "-debug" )) {
         msgLog.enableMsg( DEBUG, true );
      }
   }

   return( 0 );
}

int 
AxfSurface::readParams( int argc, char **argv )
{
   int             status = 0;
   char           *readPath = NULL;
   tdrp_override_t override;

   //
   // Read the parameter file
   //
   TDRP_init_override( &override );
   if ( params.loadFromArgs( argc, argv, 
                             override.list, &readPath ) != 0 ) {
      status = -1;
   }
   if ( readPath ) {
      paramPath = readPath;
   }
   TDRP_free_override( &override );

   //
   // Make sure the read worked
   //
   if ( status == -1 ) {
      POSTMSG( ERROR, "Unable to load parameters." );
      if ( paramPath.isValid() )
         POSTMSG( ERROR, "Check syntax of parameter file: %s in %s", 
                         paramPath.getFile().c_str(), 
                         paramPath.getDirectory().c_str() );
      return( status );
   }

   return( 0 );
}

int
AxfSurface::processParams()
{
   //
   // User-specified parameter file -- do validity checks
   //
   if ( paramPath.isValid() ) {
      POSTMSG( DEBUG, "Loading Parameter file: %s", 
                      paramPath.getFile().c_str() );
      if ( checkParamValues() != 0 )
         return( -1 );
   }

   //
   // Initialize the messaging
   //
   POSTMSG( DEBUG, "Initializing message log." );
   if ( params.debug ) {
      msgLog.enableMsg( DEBUG, true );
   }
   if ( params.info ) {
      msgLog.enableMsg( INFO, true );
   }
   else {
      msgLog.enableMsg( INFO, false );
   }

   if ( msgLog.setOutputDir( params.log_dir ) != 0 ) {
      POSTMSG( WARNING, "Cannot write log messages to output directory '%s'",
                        params.log_dir );
   }

   //
   // Initialize the data management
   //
   POSTMSG( DEBUG, "Initializing the data manager." );
   if ( dataMgr.init( params ) != 0 )
      return( -1 );

   return 0;
}

int
AxfSurface::checkParamValues()
{
   return( 0 );
}

int
AxfSurface::run()
{
    time_t now;
    time_t nextCheckTime = 0; 

   //
   // Archive mode
   //
   if ( params.mode ==  Params::ARCHIVE ) {
      return( dataMgr.ingest() );
   }

   //
   // Realtime mode
   //
   while( true ) {

      //
      // Is it time to check the directory again?
      //
      now = time( 0 );
      if( now >= nextCheckTime ) {

         nextCheckTime = now + params.check_seconds;
         PMU_auto_register( "Checking for next available data" );

         if ( dataMgr.ingest() != 0 ) {
            POSTMSG( ERROR, "Exiting process." );
            return( -1 );
         }
      }

      PMU_auto_register( "Waiting for next processing time" );
      sleep( 1 );
   }

   return( 0 );
}
