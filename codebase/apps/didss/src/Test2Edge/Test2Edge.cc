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
// $Id: Test2Edge.cc,v 1.5 2016/03/06 23:53:41 dixon Exp $
//
// Top level application class
////////////////////////////////////////////////////////////
#include <toolsa/MsgLog.hh>
#include <toolsa/umisc.h>

#include "Test2Edge.hh"
#include "DataMgr.hh"
using namespace std;

Test2Edge::Test2Edge()
{
   params  = NULL;
   msgLog  = new MsgLog();
   dataMgr = new DataMgr();
   
}

Test2Edge::~Test2Edge() 
{
   if( params ) 
      delete params;
   if( msgLog )
      delete msgLog;
   if( dataMgr )
      delete dataMgr;
}

int
Test2Edge::init( int argc, char **argv )
{
   tdrp_override_t override;

   //
   // Some general stuff
   //
   program.setPath( argv[0] );

   TDRP_init_override( &override );   

   //
   // Process the arguments
   //
   if ( processArgs( argc, argv, override ) != SUCCESS )
      return( FAILURE );              

   //
   // Print ucopright message
   //
   ucopyright( (char*)PROGRAM_NAME ); 

   //
   // Read the parameter file
   //
   params = new Params();
   paramsPath = "unknown";
   if( params->loadFromArgs(argc, argv,
			    override.list,
			    &paramsPath)) {
      POSTMSG( ERROR, "Problem with TDRP parameters");
      return( FAILURE );
   }

   TDRP_free_override( &override ); 

   //
   // Process the parameters
   //
   if ( processParams() != SUCCESS )
      return( FAILURE );

   return( SUCCESS );
}

void
Test2Edge::usage()
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

int
Test2Edge::processArgs( int argc, char **argv,
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
         return( FAILURE );
      else
         paramsPath = argv[paramArg];
   }

   return( SUCCESS );
}

int
Test2Edge::processParams()
{
   //
   // Initialize the messaging
   //
   POSTMSG( INFO, "Initializing message log." );
   initLog();

   //
   // Initialize the data manager object
   //
   if( dataMgr->init( *params ) )
      return ( FAILURE );

   return ( SUCCESS );
}

void
Test2Edge::initLog()
{
   //
   // Enable debug level messaging
   //
   if ( params->debug == TRUE )
      msgLog->enableMsg( DEBUG, true );
   else 
      msgLog->enableMsg( DEBUG, false );

   //
   // Enable info level messaging
   //
   if ( params->info == TRUE )
      msgLog->enableMsg( INFO, true );
   else
      msgLog->enableMsg( INFO, false );

}
 
int
Test2Edge::run()
{
    bool   forever = true;

    //
    // Realtime mode
    //
    while( forever ) {
       time_t now = time(0);

       if( dataMgr->writeData( now ) != SUCCESS ) {
	  return( FAILURE );
       }
    }

    return( SUCCESS );
}


