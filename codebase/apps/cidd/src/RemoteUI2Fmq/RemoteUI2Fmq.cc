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
//
// $Id: 
//

#include <string>
#include <toolsa/umisc.h>

//
// The application class and version
//
#include "RemoteUI2Fmq.hh"
using namespace std;

const string RemoteUI2Fmq::version = "0.1_alpha";
//////////////////////////////////////////////////////////////////////
// Application Constructor 
// 
RemoteUI2Fmq::RemoteUI2Fmq()
{
     debug = false;
}

//////////////////////////////////////////////////////////////////////
// Application Destructor 
// 
RemoteUI2Fmq::~RemoteUI2Fmq()
{
}

//////////////////////////////////////////////////////////////////////
// Application Initializer 
// 
int
RemoteUI2Fmq::init(int argc, char **argv )
{
   int status = 0;

   //
   // Set the program name
   //
   program.setPath(argv[0]);
   msgLog.setApplication( getProgramName() );

   //
   // Process the command line arguments
   //
   if ( processArgs( argc, argv ) != 0 )
      POSTMSG( FATAL, "Problems processing arguments");

  // Last two arguments should be input and output
  in_fname = argv[argc-2];
  output_fmq_url = argv[argc-1];

   return status;
}

//////////////////////////////////////////////////////////////////////
// Application Usage Info 
// 
void
RemoteUI2Fmq::usage()
{
   cerr << "Usage: " << PROGRAM_NAME   << " [Options as below] Input_Filename Output_FMQ_URL \n"
        << "       [ -debug ]               print verbose debug messages\n"
        << "       [ --, -h, -help, -man ]  produce this list\n"
        << "       [ -v, -version ]         display version number\n"
        << endl;
   dieGracefully( -1 );
}

//////////////////////////////////////////////////////////////////////
// Process Arguments 
// 
int 
RemoteUI2Fmq::processArgs( int argc, char **argv )
{

  //
  // request for usage information
  //
  if ( argc < 2 || argc > 4 ||
       !strcmp(argv[1], "--" ) ||
       !strcmp(argv[1], "-h" ) ||
       !strcmp(argv[1], "-help" ) ||
       !strcmp(argv[1], "-man" )) {
     usage();
     dieGracefully( 0 );
  }

  //
  // request for debug messages
  //
  if ( !strcmp(argv[1], "-debug" )) {
     debug = true;
     msgLog.enableMsg( DEBUG, true );
  }  
                    
							   
  if ( !strcmp(argv[1], "-v" ) ||
       !strcmp(argv[1], "-version" )) {
     POSTMSG( "version %s", version.c_str() );
     dieGracefully( 0 );
  }

   return( 0 );
}

//////////////////////////////////////////////////////////////////////
// APPLICATION MAIN ENTRY POINT - DO THE WORK 
// 
void
RemoteUI2Fmq::run()
{
     // Open the Queue
     if (remoteQueue.init(output_fmq_url,  "RemoteUI2Fmq", debug) != 0) {
	 cerr << "Problems initialising Output FMQ: "  << output_fmq_url  << " \n";
	 dieGracefully( 0 );
     }

     // Send the File Contents
     remoteQueue.sendFileContents(in_fname);
     dieGracefully( 0 );
}
