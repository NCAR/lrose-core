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
/**
 *
 * @file ApRemoval.cc
 *
 * @class ApRemoval
 *
 * ApRemoval is the top level application class.
 *  
 * @date 9/18/2002
 *
 */

using namespace std;

#include <toolsa/MsgLog.hh>
#include <toolsa/umisc.h>

#include "ApRemoval.hh"
#include "DataMgr.hh"


/** 
 * Constructor
 */

ApRemoval::ApRemoval()
{
   paramsPath      = NULL;
   params          = NULL;
}


/** 
 * Destructor
 */

ApRemoval::~ApRemoval() 
{
   delete params;
}


/** 
 * init()
 */

int
ApRemoval::init( int argc, char **argv )
{
   tdrp_override_t override;

   //
   // Some general stuff
   //
   program.setPath( argv[0] );
   msgLog.setApplication( getProgramName() );

   TDRP_init_override( &override );   

   //
   // Process the arguments
   //
   if ( processArgs( argc, argv, override ) )
      return( -1 );              

   //
   // Print ucopyright message
   //
   ucopyright( (char*)PROGRAM_NAME ); 

   //
   // Read the parameter file
   //
   params     = new Params();
   paramsPath = (char *)"unknown";
   if( params->loadFromArgs(argc, argv,
			    override.list,
			    &paramsPath)) {
      POSTMSG( ERROR, "Problem with TDRP parameters" );
      return( -1 );
   }

   TDRP_free_override( &override );                     

   //
   // Register with procmap now that we have the instance name
   //
   if ( params->instance == NULL )
     params->instance = (char *)"generic";
   PMU_auto_init( (char*)PROGRAM_NAME, params->instance, 
                  PROCMAP_REGISTER_INTERVAL );
   PMU_auto_register( "starting up ApRemove" );

   //
   // Process the parameters
   //
   if( processParams() )
      return( -1 );
   
   return( 0 );
}


/** 
 * usage()
 */

void
ApRemoval::usage()
{
   //
   // New-style command lines
   //
   cerr << "Usage: " << PROGRAM_NAME   << " [options as below]\n"
        << "       [ -debug ]               produce debug messages\n"
        << "       [ --, -h, -help, -man ]  produce this list\n"
        << endl;

   TDRP_usage( stdout );
   
   dieGracefully( -1 );
}


/** 
 * processArgs()
 */

int
ApRemoval::processArgs( int argc, char **argv,
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
         return( -1 );
      else
         paramsPath = argv[paramArg];
   }

   return( 0 );
}


/** 
 * processParams()
 */

int
ApRemoval::processParams()
{
   //
   // Initialize the messaging
   //
   POSTMSG( INFO, "Initializing message log." );
   initLog();

   //
   // Initialize the data manager
   //
   if( dataMgr.init( *params, msgLog ) )
      return ( -1 );

   return ( 0 );
}


/** 
 * initLog()
 */

void
ApRemoval::initLog()
{
   //
   // Enable debug level messaging
   //
   if ( params->debug == TRUE )
      msgLog.enableMsg( DEBUG, true );
   else 
      msgLog.enableMsg( DEBUG, false );

   //
   // Enable info level messaging
   //
   if ( params->info == TRUE )
      msgLog.enableMsg( INFO, true );
   else
      msgLog.enableMsg( INFO, false );

   //
   // Direct message logging to a file
   //
   if ( msgLog.setOutputDir( params->log_dir ) != 0 ) {
      POSTMSG( WARNING, "Cannot write log messages to output directory '%s'",
                        params->log_dir );
   }

}
 

/** 
 * run()
 */

int
ApRemoval::run()
{
    PMU_auto_register( "Getting data" );
    if( dataMgr.processData() ) {
       return( -1 );
    }
    return(0);
}


