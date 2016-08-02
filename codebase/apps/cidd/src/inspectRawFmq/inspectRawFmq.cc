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
#include <toolsa/DateTime.hh>

#include <rapformats/bdry.h>

//
// The application class and version
//
#include "inspectRawFmq.hh"
const string inspectRawFmq::version = "0.1_alpha";

//
// Putting the parameter structure inside as a member of
// the application class does not pass the muster with Purify
//
#include "inspectRawFmq_tdrp.h"
inspectRawFmq_tdrp_struct params;

#include <Spdb/DsSpdb.hh>

//////////////////////////////////////////////////////////////////////
// Application Constructor 
// 
inspectRawFmq::inspectRawFmq()
{
}

//////////////////////////////////////////////////////////////////////
// Application Destructor 
// 
inspectRawFmq::~inspectRawFmq()
{
}

//////////////////////////////////////////////////////////////////////
// Application Initializer 
// 
int
inspectRawFmq::init(int argc, char **argv )
{
   string paramPath;
   bool   printParams, checkParams;
   int status = 0;

   //
   // Set the program name
   //
   program.setPath(argv[0]);
   msgLog.setApplication( getProgramName() );

   ucopyright( (char*)PROGRAM_NAME );
    
   //
   // Process the command line arguments
   //
   if ( processArgs( argc, argv, 
                     paramPath, &checkParams, &printParams ) != 0 )
      POSTMSG( FATAL, "Problems processing arguments");

   //
   // Read the application parameters
   //
   if ( readParams( paramPath, checkParams, printParams ) != 0 )
      POSTMSG( FATAL, "Problems reading parameters");

   //
   // Process the parameters
   //
   if ( processParams( paramPath ) != 0 )
      POSTMSG( FATAL, "Problems processing parameters");

   return status;
}

//////////////////////////////////////////////////////////////////////
// Application Usage Info 
// 
void
inspectRawFmq::usage()
{
   cerr << "Usage: " << PROGRAM_NAME   << " [options as below]\n"
        << "       [ -check_params ]        check parameter settings\n"
        << "       [ -debug ]               print verbose debug messages\n"
        << "       [ --, -h, -help, -man ]  produce this list\n"
        << "       [ -params params_file ]  set parameter file name\n"
        << "       [ -print_params ]        produce parameter listing\n"
        << "       [ -v, -version ]         display version number\n"
        << endl;
   dieGracefully( -1 );
}

//////////////////////////////////////////////////////////////////////
// Process Arguments 
// 
int 
inspectRawFmq::processArgs( int argc, char **argv,
                           string &paramPath,
                           bool *checkParams, bool *printParams )
{
   int          i;
   int          paramArg;

   //
   // Parameter initializations
   //
   paramArg = 0;
   *printParams = *checkParams = false;


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
      // request for debug messages
      //
      if ( !strcmp(argv[i], "debug" )) {
         msgLog.enableMsg( DEBUG, true );
      }  
                    
							   
      if ( !strcmp(argv[i], "-v" ) ||
           !strcmp(argv[i], "-version" )) {
         POSTMSG( "version %s", version.c_str() );
         dieGracefully( 0 );
      }

      //
      // requests for parameter information
      //
      else if ( !strcmp( argv[i], "-print_params" )) {
         *printParams = true;
      }
      else if ( !strcmp( argv[i], "-check_params" )) {
         *checkParams = true;
      }

      //
      // parameter file setting
      //
      else if ( !strcmp( argv[i], "-params") ) {
         if ( i < argc - 1 ) {
            paramArg = i+1;
         }
         else {
            POSTMSG( ERROR, "Missing parameter file name." );
            usage();
         }
      }
   }

   if ( paramArg ) {
      //
      // Check for the files' existence
      //
      if ( Path::exists( argv[paramArg] )) {
         paramPath = argv[paramArg];
      }
      else {
         POSTMSG( ERROR, "Cannot find parameter file: %s",
                         argv[paramArg] );
         return( -1 );
      }
   }

   return( 0 );
}

//////////////////////////////////////////////////////////////////////
// Read Parameters 
// 
int
inspectRawFmq::readParams( const string &paramPath, 
                          bool checkParams, bool printParams )
{
   int             status = 0;
   char           *readPath;
   TDRPtable      *table;
   tdrp_override_t override;

   //
   // Degenerate case
   //
   if ( paramPath.empty() )
      readPath = NULL;
   else
      readPath = (char*)paramPath.c_str();

   //
   // Read the parameter file
   //
   TDRP_init_override( &override );

   table = inspectRawFmq_tdrp_init( &params );

   if ( TDRP_read( readPath, table,
                   &params, override.list ) == FALSE ) {
      status = -1;
   }
   TDRP_free_override( &override );

   //
   // Make sure the read worked
   //
   if ( status == -1 ) {
      POSTMSG( ERROR, "Unable to load parameters.\n"
                      "Check the syntax of file: %s",
                      paramPath.c_str() );
      return( status );
   }

   //
   // Check or print parameters?
   //
   if ( checkParams ) {
      TDRP_check_is_set( table, &params );
      dieGracefully( 0 );
   }
   if ( printParams ) {
      TDRP_print_params( table, &params, (char*)PROGRAM_NAME, TRUE );
      dieGracefully( 0 );
   }
   return( 0 );
}

//////////////////////////////////////////////////////////////////////
// Process Parameters 
// 
int
inspectRawFmq::processParams( const string &paramPath )
{
   if ( !paramPath.empty() ) {
      POSTMSG( DEBUG, "Loading Parameter file: %s", paramPath.c_str() );
   }

   //
   // Initialize message logging
   //
   POSTMSG( DEBUG, "Initializing message log." );
   if ( params.debug == TRUE ) {
      msgLog.enableMsg( DEBUG, true );
   }
   if ( msgLog.setOutputDir( params.log_dir ) != 0 ) {
      POSTMSG( WARNING, "Cannot write log messages to output directory '%s'",
                        params.log_dir );
   }
     
   // Initialize the Draw queue
   //
   POSTMSG(DEBUG, "Initializing the Draw Input queue." );
   if ( drawqueue.initReadBlocking( params.draw_fmq_url,
				    PROGRAM_NAME, DEBUG_ENABLED, Fmq::START ) != 0 ) {
      POSTMSG( ERROR, "Cannot initialize Draw queue.\n"
                      "Make sure draw_fmq_url (%s) is correct.",
                      params.draw_fmq_url );
      return( -1 );
   }

   //
   // Set the trigger and execution parameters
   //
   POSTMSG( DEBUG, "Initializing execution parameters." );

   return( 0 );
}

//////////////////////////////////////////////////////////////////////
// APPLICATION MAIN ENTRY POINT - DO THE WORK 
// 
void
inspectRawFmq::run()
{
   int status;
 
   //
   // Read messages off the Queue and print them.
   //
   POSTMSG( DEBUG, "Watching DrawQueue %s for output",params.draw_fmq_url);
   while( 1 ) {
      //

      const Human_Drawn_Data_t& h_prod = drawqueue.nextProduct( status );

      if ( status == 0 ) { // success
         //
         //
	fprintf(stderr, "Got a Message - Product_ID_Label: %s\n",
		h_prod.id_label.c_str());


     
	fprintf(stderr,"Product ID_NO: %d, Valid: %d seconds\n",h_prod.id_no,h_prod.valid_seconds);
	time_t dataTime = (time_t)h_prod.data_time; time_t issueTime = (time_t)h_prod.issueTime;
	fprintf(stderr,"Product issued: %s",ctime(&issueTime));
	fprintf(stderr,"Product data time: %s",ctime(&dataTime));
	fprintf(stderr,"Product label text: %s\n",h_prod.prod_label.c_str());
	fprintf(stderr,"Product sender: %s\n",h_prod.sender.c_str());
	fprintf(stderr," %d points\n",h_prod.num_points);

	for(int i=0;i < h_prod.num_points; i++) {
	  fprintf(stderr,"LAT, LON: %g,%g\n",h_prod.lat_points[i],h_prod.lon_points[i]);
	  if ((h_prod.lat_points[i] < -90.0) || (h_prod.lat_points[i] > 90.0) ||
	      (h_prod.lon_points[i] < -180.0) || (h_prod.lon_points[i] > 360.0)){
	    fprintf(stderr, "lat/lons look odd - were all apps involved compiled 32 bit?\n");
	  }
	}
      } else {  // no valid draw product - in this mode, end of the line
	break;
      }

   } // while(1)

   exit(0);

}
