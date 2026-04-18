////////////////////////////////////////////////////////////////////////////////
//
//  Driver for HailKE application class
//
//  Terri L. Betancourt, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//  October 2001
//
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <assert.h>

/*------------------------------------------------------------------
 * Temporarily use old DsInputPath until we get ahold of libs/dsdata
 *------------------------------------------------------------------
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsOneTimeTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
--------------------------------------------------------------------*/
#include <didss/DsInputPath.hh>

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
// Makitov:  Initial version using Russian algorithm via Viktor Makitov of WMI
//           libs/dsdata not available, using older DsInputPath instead
//
const string Driver::version = "Makitov";


Driver::Driver()
{
/*------------------------------------------------------------------
 * Temporarily use old DsInputPath until we get ahold of libs/dsdata
 *------------------------------------------------------------------
   dsTrigger     = NULL;
--------------------------------------------------------------------*/
   dsInputPath   = NULL;

   paramPath     = NULL;
   inputFileList = NULL;
   startTime     = DateTime::NEVER;
   endTime       = DateTime::NEVER;
   oneTime       = DateTime::NEVER;
}

Driver::~Driver()
{
   delete inputFileList;

/*------------------------------------------------------------------
 * Temporarily use old DsInputPath until we get ahold of libs/dsdata
 *------------------------------------------------------------------
   delete dsTrigger;
--------------------------------------------------------------------*/
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
           "       [ -end \"yyyy/mm/dd hh:mm:ss\"] end time - "
                                            "TIME_LIST mode implied\n"
           "       [ -f file_paths]         list of input file paths - "
                                            "FILE_LIST mode implied\n"
           "       [ --, -h, -help, -man ]  produce this list\n"
           "       [ -once \"yyyy/mm/dd hh:mm:ss\"] issue time - "
                                            "ONE_TIME mode implied\n"
           "       [ -params params_file ]  set parameter file name\n"
           "       [ -print_params ]        produce parameter listing\n"
           "       [ -start \"yyyy/mm/dd hh:mm:ss\"] start time - "
                                            "TIME_LIST mode implied\n"
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
      // start time specification
      //
      else if ( !strcmp(argv[i], "-start" )) {
         assert( ++i <= argc );
         startTime = DateTime::parseDateTime( argv[i] );
         if ( startTime == DateTime::NEVER ) {
            POSTMSG( ERROR, "Bad date/time syntax in -start specification." );
            usage();
         }
         else {
            TDRP_add_override( &tdrpOverride, "trigger_mode = TIME_LIST;" );
         }
      }

      //
      // end time specification
      //
      else if ( !strcmp(argv[i], "-end" )) {
         assert( ++i < argc );
         endTime = DateTime::parseDateTime( argv[i] );
         if ( endTime == DateTime::NEVER ) {
            POSTMSG( ERROR, "Bad date/time syntax in -end specification." );
            usage();
         }
         else {
            TDRP_add_override( &tdrpOverride, "trigger_mode = TIME_LIST;" );
         }
      }

      //
      // one time specification
      //
      else if ( !strcmp(argv[i], "-once" )) {
/*------------------------------------------------------------------
 * Temporarily use old DsInputPath until we get ahold of libs/dsdata
 *------------------------------------------------------------------
         if ( i < argc - 1 ) {
            oneTime = DateTime::parseDateTime( argv[i+1] );
            TDRP_add_override( &tdrpOverride, "trigger_mode = ONE_TIME;" );
         }
         else {
            POSTMSG( ERROR, "Missing issue time specification." );
            usage();
         }
--------------------------------------------------------------------*/
         POSTMSG( ERROR, "This feature is temporarily unsupported." );
         dieGracefully( 0 );
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
         TDRP_add_override( &tdrpOverride, "trigger_mode = FILE_LIST;" );

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
   // Initialize the data management
   //
   POSTMSG( DEBUG, "Initializing the data manager." );
   if ( dataMgr.init( params ) != 0 )
      return( -1 );

   //
   // Based on the operational mode, set up the trigger mechanism
   //
   switch ( params.trigger_mode )
   {
   case Params::LATEST_DATA:
   {
     int maxValidAge = -1;
     string radarUrl = params.radar_url;
     POSTMSG( DEBUG, "Initializing LATEST_DATA trigger to %s.",
	      params.radar_url );

/*------------------------------------------------------------------
 * Temporarily use old DsInputPath until we get ahold of libs/dsdata
 *------------------------------------------------------------------
     DsLdataTrigger *trigger = new DsLdataTrigger();
     status = trigger->init(radarUrl,
			    maxValidAge,
			    PMU_auto_register);
     if ( status != 0 )
       POSTMSG( ERROR, "Cannot initialize input trigger mechanism.\n%s",
		trigger->getErrString().c_str() );

     dsTrigger = trigger;
--------------------------------------------------------------------*/
     dsInputPath = new DsInputPath ( (char*)PROGRAM_NAME, DEBUG, 
                                     params.radar_url, 
                                     maxValidAge,
                                     PMU_auto_register );

     break;
   }

   case Params::TIME_LIST:
   {
     if ( startTime == DateTime::NEVER )
       startTime = 0;

     if ( endTime == DateTime::NEVER )
       endTime = INT_MAX;

     POSTMSG( DEBUG, "Initializing TIME_LIST trigger from %s  to   %s.", 
	      DateTime::str( startTime ).c_str(),
	      DateTime::str( endTime ).c_str() );

/*------------------------------------------------------------------
 * Temporarily use old DsInputPath until we get ahold of libs/dsdata
 *------------------------------------------------------------------
     DsTimeListTrigger *trigger = new DsTimeListTrigger();
     status = trigger->init(params.radar_url,
			    startTime, endTime);
     if ( status != 0 )
       POSTMSG( ERROR, "Cannot initialize input trigger mechanism.\n%s",
		trigger->getErrString().c_str() );

     dsTrigger = trigger;
--------------------------------------------------------------------*/

     dsInputPath = new DsInputPath ( (char*)PROGRAM_NAME, DEBUG, 
                                     params.radar_url, 
                                     startTime, endTime );
     break;
   }
   
   case Params::FILE_LIST:
   {
     assert( inputFileList != NULL );
     POSTMSG( DEBUG, "Initializing FILE_LIST trigger with %d file(s).",  
	      inputFileList->size() );

/*--------------------------------------
     DsFileListTrigger *trigger = new DsFileListTrigger();
     status = trigger->init( *inputFileList );
     if ( status != 0 )
       POSTMSG( ERROR, "Cannot initialize input trigger mechanism.\n%s",
		trigger->getErrString().c_str() );

     dsTrigger = trigger;
------------------------------------------*/
     dsInputPath = new DsInputPath ( (char*)PROGRAM_NAME, DEBUG, 
                                     *inputFileList );
     break;
   }
   
   case Params::ONE_TIME:
   {
/*------------------------------------------------------------------
 * Temporarily use old DsInputPath until we get ahold of libs/dsdata
 *------------------------------------------------------------------
     POSTMSG( DEBUG, "Initializing ONE_TIME trigger to %s.",
	      DateTime::str( oneTime ).c_str() );

     DsOneTimeTrigger *trigger = new DsOneTimeTrigger();
     status = trigger->init( oneTime );
     if ( status != 0 )
       POSTMSG( ERROR, "Cannot initialize input trigger mechanism.\n%s",
		trigger->getErrString().c_str() );

     dsTrigger = trigger;
--------------------------------------------------------------------*/
     POSTMSG( ERROR, "This trigger mode is temporarily unsupported." );
     dieGracefully( 0 );
     break;
   }
   
   } /* endswitch - params.trigger_mode */

   return( status );
}

int
Driver::run()
{
   int        status = 0;
   DateTime   issueTime;

   //
   // As long as there's more data...
   //
/*------------------------------------------------------------------
 * Temporarily use old DsInputPath until we get ahold of libs/dsdata
 *------------------------------------------------------------------
   while( !dsTrigger->endOfData() ) {

      //
      // Fetch the next trigger
      // If something goes wrong, just skip this loop iteration
      //
      if ( dsTrigger->nextIssueTime( issueTime ) != 0 ) {
         POSTMSG( WARNING, "Failed getting next trigger\n%s",
                  dsTrigger->getErrString().c_str() );
         continue;
      }

      //
      // Fetch the input datasets
      // If something goes wrong on input, bail out
      //
      POSTMSG( DEBUG, "\nFetching input data for %s",
                      issueTime.dtime() );

      if ( dataMgr.loadInputData( issueTime ) != 0 ) {
         status = -1;
         break;
      }
--------------------------------------------------------------------*/
   char *inputPath;
   while( (inputPath = dsInputPath->next() ) != NULL ) {
      if ( dataMgr.loadInputPath( inputPath ) != 0 ) {
         status = -1;
         break;
      }
///////////////////////////////////////////////////////////////////////////////

      //
      // Convert reflectivity to hail kinetic energy
      // If something goes wrong on input, bail out
      //
      if ( dataMgr.convert2kineticEnergy() != 0 ) {
         status = -1;
         break;
      }

      //
      // Write out the results
      //
      if ( dataMgr.writeOutput() != 0 ) {
         status = -1;
         break;
      }
   }

   return( status );
}
