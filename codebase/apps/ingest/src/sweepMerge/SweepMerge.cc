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
/////////////////////////////////////////////////////////////////
//  sweepMerge top-level application class
//
//  $Id: SweepMerge.cc,v 1.4 2016/03/07 01:23:11 dixon Exp $
//
///////////////////////////////////////////////////////////////
#include <string>
#include <sys/stat.h>
#include <toolsa/MsgLog.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>

#include "SweepMerge.hh"

SweepMerge::SweepMerge() 
{
   paramsPath    = NULL;
}

SweepMerge::~SweepMerge()
{
}

int SweepMerge::init( int argc, char **argv )
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
   // Process the parameters
   //
   if ( processParams() != SUCCESS )
      return( FAILURE );

   return( SUCCESS );
}

void SweepMerge::usage( int status )
{
   //
   // New-style command lines
   //
   cerr << "Usage: " << PROGRAM_NAME   << " [options as below]\n"
           "       [ -debug ]                        produce verbose debug "
                                                    "messages\n"
           "       [ --, -h, -help, -man ]           produce this list\n"
           "       [ -info ]                         produce verbose info "
                                                    "messages\n"
           "       [ -dir <directory path> ]         directory to process\n" 
        << endl;

   params.usage( cerr );
   
   dieGracefully( status );
}

int SweepMerge::processArgs( int argc, char **argv )
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
         usage( SUCCESS );
      }

      //
      // request for verbose debug messaging
      //
      if ( !strcmp(argv[i], "-debug" )) {
         TDRP_add_override( &tdrpOverride, "debug = TRUE;" );
      }
 
      //
      // request for verbose info messaging
      //
      if ( !strcmp(argv[i], "-info" )) {
         TDRP_add_override( &tdrpOverride, "info = TRUE;" );
      }

      //
      // set input directory
      //
      if ( !strcmp(argv[i], "-dir") ) {
         if( i+1 >= argc ) {
            cerr << "Error:  no directory specified" << endl;
            usage( FAILURE );
         }
         string tmpStr( "input_dir = " );
         tmpStr += argv[i+1];
         TDRP_add_override( &tdrpOverride, tmpStr.c_str() );
      }

   }

   return( SUCCESS );
}

int SweepMerge::readParams( int argc, char **argv )
{
   int status = 0;

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
         POSTMSG( ERROR, "Check syntax of parameter file: %s", paramsPath );
      return( FAILURE );
   }

   return( SUCCESS );
}

int SweepMerge::processParams()
{
   //
   // Initialize the messaging
   //
   POSTMSG( DEBUG, "Initializing message log." );
   if ( initLog() != SUCCESS )
      return( FAILURE );

   //
   // Initialize the data management
   //
   POSTMSG( DEBUG, "Initializing the data manager." );
   if ( dataMgr.init( params ) != SUCCESS ) {
      return( FAILURE );
   }

   return( SUCCESS );
}

int SweepMerge::initLog()
{
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

   return( SUCCESS );
}

int SweepMerge::run()
{
   int status = dataMgr.run();
   
   return( status );
}

