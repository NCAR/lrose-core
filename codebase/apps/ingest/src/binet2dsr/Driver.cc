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
//  Driver for application
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  September 2001
//
//  $Id: Driver.cc,v 1.6 2018/01/26 20:15:07 jcraig Exp $
//
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <cassert>

#include <didss/DsInputPath.hh>
#include <dsserver/DsLocator.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>

#include "Driver.hh"
#include "DataMgr.hh"
using namespace std;

//
// Revision History
//
// v1.0  Initial implementation
//
const string Driver::version = "1.0";


Driver::Driver()
{
   paramPath     = NULL;
   inputFileList = NULL;
}

Driver::~Driver()
{
}

int Driver::init( int argc, char **argv )
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
   PMU_auto_register( "starting up application" );

   //
   // Process the parameters
   //
   if ( processParams() != 0 )
      return( -1 );

   return( 0 );
}

void Driver::usage()
{
   //
   // New-style command lines
   //
   cerr << "Usage: " << PROGRAM_NAME   << " [options as below]\n"
           "       [ -check_params ]        check parameter settings\n"
           "       [ -debug ]               produce verbose debug messages\n"  
           "       [ --, -h, -help, -man ]  produce this list\n"
           "       [ -f file_paths]         list of input file paths\n"
           "                                overrides radar_input_url param\n"
           "       [ -params params_file ]  set parameter file name\n"
           "       [ -print_params ]        produce parameter listing\n"
           "       [ -summary [n]]          print summary each n records\n"
           "                                (default=90)\n"
           "       [ -v ]                   display version number\n"
        << endl;
   dieGracefully( -1 );
}

int Driver::processArgs( int argc, char **argv )
{
   char paramVal[256];
   TDRP_init_override( &tdrpOverride );

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
      else if ( !strcmp(argv[i], "-version" )) {
         msgLog.postMsg( "version %s", version.c_str() );
         dieGracefully( 0 );
      }

      //
      // request for debug messaging
      //
      else if ( !strcmp(argv[i], "-debug" )) {
         sprintf( paramVal, "debug = true;" );
         TDRP_add_override( &tdrpOverride, paramVal );
      }

      //
      // request to print summary
      //
      else if ( !strcmp(argv[i], "-summary" )) {
         sprintf( paramVal, "print_summary = TRUE;" );
         TDRP_add_override( &tdrpOverride, paramVal );

         if (i < argc - 1) {
            sprintf( paramVal, "summary_interval = %s;", argv[i+1] );
            TDRP_add_override( &tdrpOverride, paramVal );
         }
      }

      //
      // file list specification
      //
      else if ( !strcmp(argv[i], "-f" )) {
         assert( inputFileList == NULL );
         inputFileList = new vector< string >;

         //
         // search for next arg which starts with '-'
         //
         int j;
         for( j = i+1; j < argc; j++ ) {
            if (argv[j][0] == '-')
               break;
            else
               inputFileList->push_back( argv[j] );
         }

         if ( inputFileList->size() == 0 ) {
            POSTMSG( ERROR, "Missing file list specification." );
            usage();
         }
      }
   }

   return( 0 );
}

int Driver::readParams( int argc, char **argv )
{
   int status = 0;

   //
   // Read the parameter file
   //
   if ( params.loadFromArgs( argc, argv,
                             tdrpOverride.list, &paramPath ) != 0 ) {
      status = -1;
   }
   TDRP_free_override( &tdrpOverride );

   //
   // Make sure the read worked
   //
   if ( status == -1 ) {
      msgLog.postMsg( ERROR, "Unable to load parameters." );
      if ( paramPath ) {
         msgLog.postMsg( ERROR, "Check syntax of parameter file: %s", 
                                 paramPath );
      }
      return( status );
   }
      
   return( 0 );
}

int
Driver::processParams()
{
   int status = 0;

   //
   // Set debug level messaging
   //
   if ( params.debug ) {
      msgLog.enableMsg( DEBUG, true );
   } 
   if ( DEBUG_ENABLED  &&  paramPath )
      msgLog.postMsg( DEBUG, "Loaded Parameter file: %s", paramPath );

   //
   // Set up the input file trigger to be handed off to the dataMgr
   //
   if ( inputFileList ) {
      //
      // Trigger from a file list
      //
      POSTMSG( DEBUG, "Initializing file list trigger with %d file(s).",
                      inputFileList->size() );

      fileTrigger = new DsInputPath( PROGRAM_NAME, DEBUG_ENABLED,
                                     *inputFileList );
   }
   else {
      //
      // Trigger watches the url directory
      // Use a DsURL here so that we may be able to trigger remotely
      // in the future.  This might be overkill, but it's already here
      // so I'll leave it for now.
      //
      DsURL inputUrl( params.radar_input_url );
      if ( (DsLocator.resolveFile( inputUrl )) != 0 ) {
         POSTMSG( ERROR, "Invalid specification for 'radar_input_url'" );
         return( -1 );
      }
      char* filePath = (char*)inputUrl.getFile().c_str();

      POSTMSG( DEBUG, "Initializing trigger watch to directory path '%s'.",
                      filePath );
      fileTrigger = new DsInputPath( PROGRAM_NAME, DEBUG_ENABLED,
                                     filePath,
                                     params.max_valid_age_min*60, 
                                     PMU_auto_register,
                                     params.latest_data_info_avail );
   }

   //
   // Initialize the data management
   //
   POSTMSG( DEBUG, "Initializing the data manager." );
   if ( dataMgr.init( params, fileTrigger ) != 0 )
      return( -1 );

   return( status );
}

int
Driver::run()
{
   if ( dataMgr.processData() != 0 ) {
      //
      // Something went wrong in processing the data
      //
      POSTMSG( ERROR, "Unexpected return from processing data" );
      return( -1 );
   }
   return 0;
}
