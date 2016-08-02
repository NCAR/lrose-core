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
//////////////////////////////////////////////////////////////
//  Driver for application
//
//  Jaimi Yee, RAP, NCAR, Boulder, CO, 80307, USA
//  July 2004
//
//  Adapted from nexrad2dsr application by Terri Betancourt 
//  RAP, NCAR, Boulder, CO, 80307, USA
//
//  $Id: Driver.cc,v 1.9 2016/03/07 01:23:10 dixon Exp $
//
/////////////////////////////////////////////////////////////
#include <string>

#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>

#include "Status.hh"
#include "Driver.hh"
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
   startTime     = DateTime::NEVER;
   endTime       = DateTime::NEVER;
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
           "       [ -debug ]               produce verbose debug messages\n"  
           "       [ -end \"yyyy/mm/dd hh:mm:ss\"] end time - "
                                            "ARCHIVE mode implied\n"

           "       [ -f file_paths]         list of input file paths\n"
           "                                overrides radar_input_url param\n"
           "       [ --, -h, -help, -man ]  produce this list\n"
           "       [ -instance name]      set the instance\n"
           "       [ -params params_file ]  set parameter file name\n"
           "       [ -start \"yyyy/mm/dd hh:mm:ss\"] start time - "
                                            "ARCHIVE mode implied\n"
           "       [ -summary [n]]          print summary each n records\n"
           "                                (default=90)\n"
           "       [ -v ]                   display version number\n"
        << endl;   

   TDRP_usage( stdout );

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
         sprintf( paramVal, "printSummary = TRUE;" );
         TDRP_add_override( &tdrpOverride, paramVal );

         if (i < argc - 1) {
            sprintf( paramVal, "summaryInterval = %s;", argv[i+1] );
            TDRP_add_override( &tdrpOverride, paramVal );
         }
      }

      //
      // instance specification
      //
      else if( !strcmp(argv[i], "-instance" )) {
         if( !( ++i <= argc ) ) {
            POSTMSG( ERROR, "Instance name must follow -instance argument." );
            exit( -1 );
         }
         sprintf( paramVal, "instance = %s;", argv[i] );
         TDRP_add_override( &tdrpOverride, paramVal );
      }

      //
      // start time specification
      //
      else if ( !strcmp(argv[i], "-start" )) {
	if (!( ++i <= argc )){
	  POSTMSG( ERROR, "Date must follow -start argument." );
	  exit(-1);
	 }
         startTime = DateTime::parseDateTime( argv[i] );
         if ( startTime == DateTime::NEVER ) {
            POSTMSG( ERROR, "Bad date/time syntax in -start specification." );
            usage();
         }
         else {
            TDRP_add_override( &tdrpOverride, "mode = ARCHIVE;" );
         }
      }

      //
      // end time specification
      //
      else if ( !strcmp(argv[i], "-end" )) {
	 if (!( ++i < argc )){
	   POSTMSG( ERROR, "Date must follow -end argument." );
	   exit(-1);
         }
         endTime = DateTime::parseDateTime( argv[i] );
         if ( endTime == DateTime::NEVER ) {
            POSTMSG( ERROR, "Bad date/time syntax in -end specification." );
            usage();
         }
         else {
            TDRP_add_override( &tdrpOverride, "mode = ARCHIVE;" );
         }
      }
      //
      // file list specification
      //
      else if ( !strcmp(argv[i], "-f" )) {

         //
         // search for next arg which starts with '-'
         //
         int j;
         for( j = i+1; j < argc; j++ ) {
            if (argv[j][0] == '-')
               break;
            else
               inputFileList.push_back( argv[j] );
         }
         TDRP_add_override( &tdrpOverride, "mode = ARCHIVE;" );

         if ( inputFileList.size() == 0 ) {
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
   PMU_auto_register( "Reading the parameter file" );

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
   // Initialize the data management
   //
   POSTMSG( DEBUG, "Initializing the data manager" );
   if ( dataMgr.init( params ) != 0 )
      return( -1 );

   return( status );
}

int
Driver::run()
{
   Status::info_t status = dataMgr.processData();

   switch ( status ) {

      case Status::ALL_OK:
           POSTMSG( ERROR, "Unexpected return from DataMgr::processData()" );
           return( -1 );
           break;

      case Status::FAILURE:
           POSTMSG( ERROR, "Unexpected failure." );
           return( -1 );
           break;

      case Status::END_OF_DATA:
      case Status::END_OF_FILE:
           POSTMSG( INFO, "End of data reached." );
           return( 0 );
           break;

      case Status::BAD_DATA:
           POSTMSG( ERROR, "Unable to process radar data." );
           return( -1 );
           break;

      case Status::BAD_INPUT_STREAM:
           POSTMSG( ERROR, "Unable to input from NEXRAD radar stream." );
           return( -1 );
           break;

      case Status::BAD_OUTPUT_STREAM:
           POSTMSG( ERROR, "Unable to output to DSR radar stream." );
           return( -1 );
           break;

      case Status::OUTPUT_WRITTEN:
           POSTMSG( DEBUG, "Output written." );
           return( 0 );
           break;
   }

   POSTMSG( ERROR, "You can't get here from there!" );
   return( -1 );
}
