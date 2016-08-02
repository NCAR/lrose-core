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
//  LtgSpdb2GenPt top-level application class
//
////////////////////////////////////////////////////////////////////////////////
#include <sys/stat.h>
#include <toolsa/utim.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>

#include "LtgSpdb2GenPt.hh"
using namespace std;


LtgSpdb2GenPt::LtgSpdb2GenPt()
{
   paramPath = NULL;
}

LtgSpdb2GenPt::~LtgSpdb2GenPt()
{
 
}

int LtgSpdb2GenPt::init( int argc, char **argv )
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


   //
   // Initialize the data management 
   //
   POSTMSG( DEBUG, "Initializing the data manager." );
   if ( dataMgr.init( params ) != 0 )
      return( -1 );

   return( 0 );
}

void LtgSpdb2GenPt::usage()
{
   cerr << "Usage: " << PROGRAM_NAME   << " [options as below]\n"
           "       [ -check_params ]        check parameter settings\n"
           "       [ -debug ]               produce verbose debug messages\n"
           "       [ --, -h, -help, -man ]  produce this list\n"
           "       [ -params params_file ]  set parameter file name\n"
           "       [ -print_params ]        produce parameter listing\n"
        << endl;
   dieGracefully( -1 );
}

int LtgSpdb2GenPt::processArgs( int argc, char **argv )
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
      // request for verbose debug messaging
      //
      else if ( !strcmp(argv[i], "-debug" )) {
         sprintf( paramVal, "debug = true;" );
         TDRP_add_override( &tdrpOverride, paramVal );
      }
   }

   return( 0 );
}

int LtgSpdb2GenPt::readParams( int argc, char **argv )
{
   int             status = 0;

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
   }

   return( 0 );
}

int
LtgSpdb2GenPt::processParams()
{
   int status = 0;

   if (params.info || params.debug)
     msgLog.enableMsg( INFO, true);
   
   //
   // Set debug level messaging
   //
   if ( params.debug ) {
      msgLog.enableMsg( DEBUG, true );
   
   }
   if ( DEBUG_ENABLED  &&  paramPath )
      msgLog.postMsg( DEBUG, "Loaded Parameter file: %s", paramPath );

   return( status );
}


int
LtgSpdb2GenPt::run()
{
   
   int status = dataMgr.processData(); 

   POSTMSG(INFO, "Exiting with status %d", status);

   return( 0 );
}













