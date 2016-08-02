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
//  Ascii2Mdv top-level application class
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  June 1999
//
//  $Id: Ascii2Mdv.cc,v 1.6 2016/03/04 02:22:14 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <sys/stat.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>

#include "DataMgr.hh"
#include "Ascii2Mdv.hh"
using namespace std;

//
// static definitions
//
const string Ascii2Mdv::version = "1.0";

Ascii2Mdv::Ascii2Mdv()
{
   msgLog  = new MsgLog;
   dataMgr = new DataMgr;

   TDRP_init_override( &override );
}

Ascii2Mdv::~Ascii2Mdv()
{
   delete msgLog;
   delete dataMgr;

   TDRP_free_override( &override );
}

int Ascii2Mdv::init( int argc, char **argv )
{
   //
   // Some general stuff
   //
   program.setPath( argv[0] );
   msgLog->setApplication( getProgramName() );

   if ( processArgs( argc, argv ) != 0 )
      return( -1 );                 

   ucopyright( (char*)PROGRAM_NAME );
   if ( readParams( argc, argv ) != 0 )
      return( -1 );                      

   //
   // Register with procmap now that we have the instance name
   //
   if ( params.instance == NULL )
     params.instance = (char *) "generic";
   PMU_auto_init( (char*)PROGRAM_NAME, params.instance, 
                  PROCMAP_REGISTER_INTERVAL );
   PMU_auto_register( "starting up Ascii2Mdv" );

   //
   // Process the parameters
   //
   if ( processParams() != 0 )
      return( -1 );

   return( 0 );
}

void Ascii2Mdv::usage()
{
   //
   // New-style command lines
   //
   cerr << endl 
        << "Usage: " << PROGRAM_NAME   << " [options as below]\n"
           "       [ -check_params ]        check parameter settings\n"
           "       [ -debug ]               produce verbose debug messages\n"
           "       [ --, -h, -help, -man ]  produce this list\n"
           "       [ -params params_file ]  set parameter file name\n"
           "       [ -print_params ]        produce parameter listing\n"
           "       [ -v ]                   display version number\n\n"
           "NOTE:  This application does not operate in REALTIME mode.\n"
           "       and only handles one mdv field per mdv file.\n"
        << endl;
   dieGracefully( -1 );
}

int Ascii2Mdv::processArgs( int argc, char **argv )
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
         msgLog->enableMsg( DEBUG, true );
      }
   }

   return( 0 );
}

int Ascii2Mdv::readParams( int argc, char **argv )
{
   int             status = 0;
   char           *readPath = NULL;

   //
   // Read the parameter file
   //
   if ( params.loadFromArgs( argc, argv, 
                             override.list, &readPath ) != 0 ) {
      status = -1;
   }
   if ( readPath ) {
      paramPath = readPath;
   }

   //
   // Make sure the read worked
   //
   if ( status == -1 ) {
      POSTMSG( ERROR, "Unable to load parameters." );
      if ( paramPath.isValid() )
         POSTMSG( ERROR, "Check syntax of parameter file '%s'", 
                         paramPath.getPath().c_str() );
      return( status );
   }

   return( 0 );
}

int
Ascii2Mdv::processParams()
{
   //
   // Make sure we got a parameter file
   //
   if ( !paramPath.isValid() ) {
      POSTMSG( ERROR, "Parameter file is required." );
      return( -1 );
   }

   //
   // Initialize the messaging
   //
   if ( params.debug )
      msgLog->enableMsg( DEBUG, true );
   if ( params.info )
      msgLog->enableMsg( INFO, true );

   //
   // Initialize the data management
   //
   POSTMSG( DEBUG, "Initializing the data manager." );
   if ( dataMgr->init( params ) != 0 )
      return( -1 );

   return 0;
}

int
Ascii2Mdv::run()
{
   int      status = 0;
   char    *inputPath;

   //
   // Process each input file
   //
   for( int i=0; i < params.input_ascii_files_n ; i++ ) {

      //
      // Reformat the input data
      //
      inputPath = params._input_ascii_files[i];
      if ( dataMgr->format2mdv( inputPath ) != 0 ) {
         //
         // It didn't work -- see if we should continue on or quit
         //
         if ( params.continue_on_error ) {
            POSTMSG( WARNING, "Input file '%s' could not be converted to mdv.",
                               inputPath );
            continue;
         }
         else {
            POSTMSG( ERROR, "Input file '%s' could not be converted to mdv.",
                             inputPath );
            break;
         }
      }
   }

   return( status );
}
