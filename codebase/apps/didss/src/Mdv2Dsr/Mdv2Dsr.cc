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
////////////////////////////////////////////////////////////////////////////
//  Mdv2Dsr top-level application class
//
//  $Id: Mdv2Dsr.cc,v 1.12 2016/03/06 23:53:41 dixon Exp $
//
//////////////////////////////////////////////////////////////////////////

#include <toolsa/MsgLog.hh>
#include <toolsa/ucopyright.h>

#include "Mdv2Dsr.hh"
#include "DataMgr.hh"
using namespace std;

Mdv2Dsr::Mdv2Dsr()
{
   paramsPath      = NULL;
   params          = NULL;
   msgLog          = new MsgLog();
   dataMgr         = new DataMgr();
}

Mdv2Dsr::~Mdv2Dsr() 
{
   if( params )
      delete params;
   if( msgLog )
      delete msgLog;
   if( dataMgr )
      delete dataMgr;
}

int
Mdv2Dsr::init( int argc, char **argv )
{
   tdrp_override_t   override;
   vector< string > fileList;
   
   //
   // Some general stuff
   //
   program.setPath( argv[0] );
   msgLog->setApplication( getProgramName() );

   TDRP_init_override( &override );   

   //
   // Process the arguments
   //
   if ( processArgs( argc, argv, override, fileList ) != SUCCESS )
      return( FAILURE );              

   //
   // Print ucopright message
   //
   ucopyright( (char*)PROGRAM_NAME ); 

   //
   // Read the parameter file
   //
   params = new Params();
   paramsPath = (char *) "unknown";
   if( params->loadFromArgs(argc, argv,
			    override.list,
			    &paramsPath)) {
      POSTMSG( ERROR, "Problem with TDRP parameters" );
      return( FAILURE );
   }

   TDRP_free_override( &override );                     

   //
   // Register with procmap now that we have the instance name
   //
   if ( params->instance == NULL )
     params->instance = (char *) "generic";
   PMU_auto_init( (char*)PROGRAM_NAME, params->instance, 
                  PROCMAP_REGISTER_INTERVAL );

   //
   // Process the parameters
   //
   if ( processParams( fileList ) != SUCCESS )
      return( FAILURE );

   return( SUCCESS );
}

void
Mdv2Dsr::usage()
{
   //
   // New-style command lines
   //
   cout << "Usage: " << PROGRAM_NAME   << " [options as below]\n"
        << "       [ --, -h, -help, -man ]  produce this list\n"
        << "       [ -debug ] produce debug messages\n"
        << "       [ -f ? ? ?] set file paths for analysis\n"
        << "       [ -indir ?] set input directory\n"
        << "       [ -start \"yyyy mm dd hh mm ss\" ] set start time\n"
        << "       [ -end \"yyyy mm dd hh mm ss\" ] set end time\n"
        << "       [ -summary [n] ] print summary each n records\n"
        << "       [ -verbose ] produce verbose info messages\n"
        << endl;

   TDRP_usage( stdout );
   
   dieGracefully( -1 );
}

int
Mdv2Dsr::processArgs( int argc, char** argv,
                      tdrp_override_t& override,
                      vector< string > &fileList )
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

      if ( !strcmp(argv[i], "-verbose" )) {
         sprintf(tmpStr, "info = TRUE;");
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
      else if ( !strcmp( argv[i], "-indir") ) {
         if ( i < argc - 1 ) {
	   sprintf( tmpStr, "input_dir = \"%s\";", argv[++i] );
	   TDRP_add_override( &override, tmpStr );
         }
         else {
	   POSTMSG( ERROR, "Missing parameter file name");
	   usage();
         }
      }
      else if ( !strcmp( argv[i], "-start") ) {
         if ( i < argc - 1 ) {
	   sprintf( tmpStr, "start_time = \"%s\";", argv[++i] );
	   TDRP_add_override( &override, tmpStr );
         }
         else {
	   POSTMSG( ERROR, "Missing parameter file name");
	   usage();
         }
      }
      else if ( !strcmp( argv[i], "-end") ) {
         if ( i < argc - 1 ) {
	   sprintf( tmpStr, "end_time = \"%s\";", argv[++i] );
	   TDRP_add_override( &override, tmpStr );
         }
         else {
	   POSTMSG( ERROR, "Missing parameter file name");
	   usage();
         }
      }
      else if ( !strcmp( argv[i], "-f" ) ) {

	 if( i < argc - 1 ) {
            int j;
	    for( j = i+1; j < argc; j++ ) {
	      fileList.push_back(argv[j]);
	       if( argv[j][0] == '-')
		  break;
	    }
	 }
      }
      else if( !strcmp( argv[i], "-summary" ) ) {
	 sprintf( tmpStr, "print_summary = TRUE;" );
	 TDRP_add_override( &override, tmpStr );
	 
         if( i < argc - 1 && argv[i+1][0] != '-' ) {
	    sprintf( tmpStr, "summary_interval = %s;", argv[i+1] );
	    TDRP_add_override( &override, tmpStr );
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
Mdv2Dsr::processParams( vector< string > &fileList )
{
  
   //
   // Initialize the messaging
   //
   POSTMSG( INFO, "Initializing message log." );
   initLog();

   //
   // Initialize the data manager
   //   Decide whether to pass a list of files from
   //   the argument list or whether to get it from
   //   the parameter file
   //
   if( fileList.size() > 0 ) {

      if( dataMgr->init( *params, msgLog, fileList ) )
	 return ( FAILURE );
   }
   else {
     vector< string > file_list;
     for (int i = 0; i < params->input_files_n; ++i)
       file_list.push_back(params->_input_files[i]);
     
      if( dataMgr->init( *params, msgLog, 
                         file_list ) != SUCCESS )
	 return( FAILURE );
   }

   return ( SUCCESS );
}

void
Mdv2Dsr::initLog()
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

   //
   // Direct message logging to a file
   //
   if ( msgLog->setOutputDir( params->log_dir ) != 0 ) {
      POSTMSG( WARNING, "Cannot write log messages to output directory '%s'",
                        params->log_dir );
   }

}
 
int
Mdv2Dsr::run()
{

  int iret = 0;
  if ( params->simulate_repeat_forever) {

    while (true) {
      PMU_auto_register( "Getting data" );
      if( dataMgr->processData() != SUCCESS ) {
        iret = -1;
      }
      dataMgr->resetInputDataQueue();
    }

  } else {

    PMU_auto_register( "Getting data" );
    if( dataMgr->processData() != SUCCESS ) {
      iret = -1;
    }

  }

  return iret;

}


