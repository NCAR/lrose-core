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
//  Driver for MdvMatch application class
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  January 2000
//
//  $Id: Driver.cc,v 1.5 2016/03/04 02:22:11 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>


#include "Driver.hh"
#include "DataMgr.hh"
using namespace std;

//
// static definitions
//
const string Driver::version = "1.0";


Driver::Driver()
{
   paramPath = NULL;
   startTime = DateTime::NEVER;
   endTime   = DateTime::NEVER;
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
        << "       [ -check_params ]        check parameter settings\n"
        << "       [ -debug ]               produce debug messages\n"
           "       [ -end \"yyyy/mm/dd/hh:mm:ss\"] \n"
           "                                end time - ARCHIVE mode only\n"
           "       [ -f file_paths]         list of input file paths - "
                                            "ARCHIVE mode only\n"
           "       [ --, -h, -help, -man ]  produce this list\n"
           "       [ -params params_file ]  set parameter file name\n"
           "       [ -print_params ]        produce parameter listing\n"
           "       [ -start \"yyyy/mm/dd/hh:mm:ss\"] \n"
           "                                start time -  ARCHIVE mode only\n"
           "       [ -v ]                   display version number\n\n"
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
      // start time for archive mode analysis
      //
      else if ( !strcmp(argv[i], "-start" )) {
         assert( ++i <= argc );
         startTime = DateTime::parseDateTime( argv[i] );
         TDRP_add_override( &tdrpOverride, "mode = ARCHIVE;" );
      }

      //
      // end time for archive mode analysis
      //
      else if ( !strcmp(argv[i], "-end" )) {
         assert( ++i < argc );
         endTime = DateTime::parseDateTime( argv[i] );
         TDRP_add_override( &tdrpOverride, "mode = ARCHIVE;" );
      }

      //
      // file list for archive mode analysis
      //
      else if ( !strcmp(argv[i], "-f" )) {
         assert( ++i < argc );

         //
         // search for next arg which starts with '-'
         //
         int j;
         for( j = i; j < argc; j++ ) {
            if (argv[j][0] == '-')
               break;
            else
               inputFileList.push_back( argv[j] );
         }
         TDRP_add_override( &tdrpOverride, "mode = FILELIST;" );
      }
   }

   //
   // Do some validation of the command line arguments
   //
   if ( startTime != DateTime::NEVER  &&  endTime != DateTime::NEVER  &&
        inputFileList.size() > 0 ) {
      POSTMSG( ERROR, "-start and -end arguments "
                      "cannot be specified in conjunction with -f option." );
      return( -1 );
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
   // Initialize the data management
   //
   POSTMSG( DEBUG, "Initializing the data manager." );
   if ( dataMgr.init( params ) != 0 )
      return( -1 );

   //
   // Based on the operational mode, set up the inputPath mechanism
   //
   switch ( params.mode ) {
      case Params::REALTIME:
           //
           // For now, we don't support realtime mode
           //
           POSTMSG( "Sorry, REALTIME mode is not currently supported." );
           return( -1 );
           status = trigger.setRealtime( params.input_url_A,
                                         params.max_valid_age_min * 60,
                                         PMU_auto_register );
           break;

      case Params::ARCHIVE:
           if ( startTime == DateTime::NEVER ) {
              startTime = 0;
           }
           if ( endTime == DateTime::NEVER ) {
              endTime = INT_MAX;
           }
           status = trigger.setArchive( params.input_url_A,
                                        startTime, endTime );
           break;
      case Params::FILELIST:
           status = trigger.setFilelist( inputFileList );
           break;
   }

   //
   // Set up the mdvx input specifications
   //
//   mdvxInputA.setDebug( params.debug );
   mdvxInputA.addReadField( params.field_name_A );
   mdvxInputA.setReadEncodingType( Mdvx::ENCODING_FLOAT32 );
   mdvxInputA.setReadCompressionType( Mdvx::COMPRESSION_NONE );
   mdvxInputA.setReadComposite();

//   mdvxInputB.setDebug( params.debug );
   mdvxInputB.addReadField( params.field_name_B );
   mdvxInputB.setReadEncodingType( Mdvx::ENCODING_FLOAT32 );
   mdvxInputB.setReadCompressionType( Mdvx::COMPRESSION_NONE );
   mdvxInputB.setReadComposite();

   return( status );
}

int
Driver::run()
{
   time_t      dataTime;
   time_t      timeMargin = params.margin_min * 60;
   time_t      timeOffset = params.offset_min * 60;
   DateTime    when;

   MdvxField  *fieldA;
   MdvxField  *fieldB;

   //
   // Process each input file
   //
   trigger.reset();
   while( !trigger.endOfData() ) {

      //
      // Read in the trigger (input A) dataset
      //
      POSTMSG( DEBUG, "Reading dataset A" );
      if ( trigger.readVolumeNext( mdvxInputA ) != 0 ) {
          //
          // If we can't read dataset A, there likely a problem
          // so bail out
          //
          POSTMSG( ERROR, "Cannot read input dataset A...\n %s",
                          trigger.getErrStr().c_str() );
          return( -1 );
      }
      
      //
      // Based on the dataTime from the trigger plus specified offset
      // see if there is a dataset available from input B within
      // the specified time margin
      //
      dataTime = trigger.getDataTime() + timeOffset;
      mdvxInputB.setTimeListModeValid( params.input_url_B, 
                                       dataTime - timeMargin,
                                       dataTime + timeMargin );

      if ( mdvxInputB.compileTimeList() ) {
          //
          // Something when wrong scanning the inputB location, bail out
          //
          POSTMSG( ERROR, "Cannot read timelist for dataset B...\n %s",
                          mdvxInputB.getErrStr().c_str() );
          return( -1 );
      }

      if ( mdvxInputB.getNTimesInList() == 0 ) {
          //
          // No valid dataset time from inputB, continue on
          //
          when = dataTime;
          POSTMSG( DEBUG, "No matching dataset B available for %s "
                          "(margin=%d min)", 
                          when.dtime(), params.margin_min );
          continue;
      }

      //
      // Read in the dependent (input B) dataset
      //
      POSTMSG( DEBUG, "Reading dataset B" );
      mdvxInputB.setReadTime( Mdvx::READ_CLOSEST, params.input_url_B,
                              timeMargin, dataTime );
      if ( mdvxInputB.readVolume() != 0 ) {
          //
          // Something went wrong trying to read datasetB, bail out
          //
          POSTMSG( ERROR, "Cannot read input dataset B...\n %s",
                          mdvxInputB.getErrStr().c_str() );
          return( -1 );
      }

      //
      // If we've made it this far, do the probability matching
      //
      POSTMSG( DEBUG, "Matching input A '%s'\n"
                      "    with input B '%s'",
                      mdvxInputA.getPathInUse().c_str(),
                      mdvxInputB.getPathInUse().c_str() );

      fieldA = mdvxInputA.getFieldByName( params.field_name_A );
      fieldB = mdvxInputB.getFieldByName( params.field_name_B );

      if ( dataMgr.probabilityMatch( *fieldA, *fieldB ) != 0 ) {
         return( -1 );
      }
   }

   return( 0 );
}
