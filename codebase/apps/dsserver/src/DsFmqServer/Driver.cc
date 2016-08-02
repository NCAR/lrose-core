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
//  Driver for DsFmqServer application class
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  March 1999
//
//  $Id: Driver.cc,v 1.18 2016/03/04 02:29:41 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>
#include <Fmq/DsFmq.hh>

#include "Driver.hh"
#include "DsFmqServer.hh"

using namespace std;

//
// static definitions
//
const string Driver::version = "2.0";


Driver::Driver()
{
   paramPath = NULL;
   server    = NULL;
   progName = "DsFmqServer";
   TDRP_init_override( &tdrpOverride );
}

Driver::~Driver()
{
   TDRP_free_override( &tdrpOverride );
   if ( server ) {
      delete server;
   }
}

int Driver::init( int argc, char **argv )
{

   //
   // process args
   //

   if ( processArgs( argc, argv ) != 0 )
      return( -1 );                 

   ucopyright( progName.c_str() );
   if ( readParams( argc, argv ) != 0 )
      return( -1 );                      

   // read params
   if ( params.debug  &&  paramPath ) {
     cerr << "Loaded Parameter file: " << paramPath << endl;
   }

   //
   // Instantiate the fmq server
   //
   
   if (params.debug) {
     cerr << "Instantiating the fmq server." << endl;
   }
   server = new DsFmqServer( progName,
			     params );
   
   if ( !server->isOkay() ) {
     cerr << "ERROR - " << progName << endl;
     cerr << "  Cannot initialize fmq server" << endl;
     cerr << "  " << server->getErrString() << endl;
     return( -1 );
   }
   
   return 0;

}

void Driver::usage(ostream &out)
{
   //
   // New-style command lines
   //
   out << "Usage: " << progName << " [options as below]\n"
       << "       [ --, -h, -help, -man ]  produce this list\n"
       << "       [ -cmax ? ]              set max number of clients\n"
       << "       [ -debug ]               produce debug messages\n"
       << "       [ -instance ? ]          set instance (default primary)\n"
       << "         Default is port number.\n"
       << "       [ -noThreads ]           turn off threads for debugging\n"
       << "       [ -port ? ] set port number.\n"
       << "       [ -qmax ? ]              set max quiescent period (secs)\n"
       << "                                Default - server never exits\n"
       << "       [ -version ]             display version number\n"
       << "       [ -verbose ]             produce verbose debug messages\n"
       << endl;
   
   Params::usage(out);

}

int Driver::processArgs( int argc, char **argv )
{
   char paramVal[256];
   int iret = 0;
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
         usage(cout);
	 iret = -1;
      }

      //
      // request for version number
      //
      else if ( !strcmp(argv[i], "-version" )) {
	cout << "version " << version << endl;
	iret = -1;
      }
      
      //
      // request for verbose debug messaging
      //
      else if ( !strcmp(argv[i], "-debug" )) {
	 sprintf( paramVal, "debug = TRUE;");
         TDRP_add_override( &tdrpOverride, paramVal );
      }

      //
      // request for verbose messaging
      //
      else if ( !strcmp(argv[i], "-verbose" )) {
	 sprintf( paramVal, "debug = TRUE;");
         TDRP_add_override( &tdrpOverride, paramVal );
	 sprintf( paramVal, "verbose = TRUE;");
         TDRP_add_override( &tdrpOverride, paramVal );
      }

      // set threading off for debugging

      else if (!strcmp(argv[i], "-noThreads")) {
	 sprintf( paramVal, "no_threads = TRUE;");
         TDRP_add_override( &tdrpOverride, paramVal );
      }

      //
      // override instance
      //
      else if (!strcmp(argv[i], "-instance")) {
      	if (i < argc - 1) {
	  sprintf( paramVal, "instance = \"%s\";", argv[++i]);
	  TDRP_add_override( &tdrpOverride, paramVal );
         } 
         else {
	   usage(cerr);
	   iret = -1;
         }
      }
      
      //
      // override port number
      //
      else if ( !strcmp(argv[i], "-port" )) {
         if ( i < argc - 1 ) {
            sprintf( paramVal, "port = %s;", argv[++i] );
            TDRP_add_override( &tdrpOverride, paramVal );
         } 
         else {
            usage(cerr);
	    iret = -1;
         }
      }

      //
      // override max number of clients
      //
      else if ( !strcmp(argv[i], "-cmax" )) {
         if ( i < argc - 1 ) {
            sprintf( paramVal, "max_clients = %s;", argv[++i] );
            TDRP_add_override( &tdrpOverride, paramVal );
         }
         else {
            usage(cerr);
	    iret = -1;
         }
      }

      //
      // override quiescence (seconds) timeout
      //
      else if ( !strcmp(argv[i], "-qmax" )) {
         if ( i < argc - 1 ) {
            sprintf( paramVal, "qmax = %s;", argv[++i] );
            TDRP_add_override( &tdrpOverride, paramVal );
         }
         else {
            usage(cerr);
	    iret = -1;
         }
      }

   }

   return iret;
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

   //
   // Make sure the read worked
   //
   if ( status == -1 ) {
     cerr << "ERROR: " << progName << endl;
     cerr << "  Unable to load parameters." << endl;
     if ( paramPath ) {
       cerr << "  Check syntax of parameter file: " << paramPath << endl;
     }
     return( status );
   }

   return( 0 );
}

int
Driver::run()
{

  while( true ) {
    server->waitForClients();
  }
  
  return( 0 );
}
