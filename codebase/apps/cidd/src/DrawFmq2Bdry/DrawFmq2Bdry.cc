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
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>

#include <rapformats/bdry.h>

//
// The application class and version
//
#include "DrawFmq2Bdry.hh"
const string DrawFmq2Bdry::version = "0.1_alpha";

//
// Putting the parameter structure inside as a member of
// the application class does not pass the muster with Purify
//
#include "DrawFmq2Bdry_tdrp.h"
DrawFmq2Bdry_tdrp_struct params;

#include <Spdb/DsSpdb.hh>

//////////////////////////////////////////////////////////////////////
// Application Constructor 
// 
DrawFmq2Bdry::DrawFmq2Bdry()
{
}

//////////////////////////////////////////////////////////////////////
// Application Destructor 
// 
DrawFmq2Bdry::~DrawFmq2Bdry()
{
}

//////////////////////////////////////////////////////////////////////
// Application Initializer 
// 
int
DrawFmq2Bdry::init(int argc, char **argv )
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
   // Register with procmap now that we have the instance name
   //
   if ( params.instance == NULL )
      params.instance = (char *)"generic";
   PMU_auto_init( (char*)PROGRAM_NAME, params.instance,
                  PROCMAP_REGISTER_INTERVAL );

   PMU_auto_register( "Starting up application" );

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
DrawFmq2Bdry::usage()
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
DrawFmq2Bdry::processArgs( int argc, char **argv,
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
DrawFmq2Bdry::readParams( const string &paramPath, 
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

   table = DrawFmq2Bdry_tdrp_init( &params );

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
DrawFmq2Bdry::processParams( const string &paramPath )
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
                            PROGRAM_NAME, DEBUG_ENABLED ) != 0 ) {
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
DrawFmq2Bdry::run()
{
   int status;
   int buffer_len;
   int num_points_allocated;

   BDRY_product_t bdry;
   BDRY_polyline_t poly;  // Only one line is possible 
   BDRY_point_t *pt = NULL;
   BDRY_spdb_product_t *spdb_bdry = NULL;

   // Set up Boundry structures;
   bdry.type = BDRY_TYPE_BDRY_TRUTH;
   bdry.subtype =  BDRY_SUBTYPE_ALL;
   bdry.line_type = BDRY_LINE_TYPE_TRUTH;
   bdry.num_polylines = 1; // Alway 1 
   bdry.motion_direction = 0.0;
   bdry.motion_speed = 0.0;
   bdry.line_quality_value = 1.0;
   bdry.line_quality_thresh = 1.0;
   bdry.type_string = "BDT";
   bdry.subtype_string = "NONE";
   bdry.line_type_string = "Human Defined Boundry";

   bdry.polylines = &poly;

   // Allocate 128 points to start
   pt = (BDRY_point_t *) calloc(128,sizeof(BDRY_point_t));
   if(pt == NULL) POSTMSG( FATAL, "Couldn't allocate space for Boundry points\n");
   num_points_allocated = 128;
   poly.points = pt;

   // bool found_match = false;


   // Set up SPDB output
   DsSpdb HIB_spdb;
   HIB_spdb.setPutMode(Spdb::putModeOver); // Draw products with same times will be added

   //
   // Read messages off the Queue. If the proper one arrives
   // Reformat it into a BDRY_spdb_product_t and
   // post it to the database.
   //
   POSTMSG( DEBUG, "Watching DrawQueue %s for output",params.draw_fmq_url);
   while( 1 ) {
      //
      // Register with procmap
      //
      PMU_auto_register( "Looping around the trigger condition" );

      const Human_Drawn_Data_t& h_prod = drawqueue.nextProduct( status );

      if ( status == 0 ) { // success
         //
         //
         POSTMSG(DEBUG, "Got a Message - Product_ID_Label: %s\n",
		 h_prod.id_label.c_str());

         HIB_spdb.clearUrls();
		 int index = -1;

		 // Look for ID Label Matches
		 for (int i = 0; i < params.HIB_n; i++ ) {
			 if(strncmp(h_prod.id_label.c_str(),params._HIB[i].ID_label,
						DrawQueue::ID_label_Len) == 0) {

				 HIB_spdb.addUrl(params._HIB[i].dest_url);
				 index = i;
			 }
		 }


	 if(index >= 0) {
	   POSTMSG( DEBUG,"Product ID_NO: %d, Valid: %d seconds\n",h_prod.id_no,h_prod.valid_seconds);
	   POSTMSG( DEBUG,"Product issued: %s",ctime(&h_prod.issueTime));
	   POSTMSG( DEBUG,"Product data time: %s",ctime(&h_prod.data_time));
	   POSTMSG( DEBUG,"Product label text: %s\n",h_prod.prod_label.c_str());
	   POSTMSG( DEBUG,"Product sender: %s\n",h_prod.sender.c_str());
	   POSTMSG( DEBUG," %d points\n",h_prod.num_points);

	   if(DEBUG_ENABLED) {
	     for(int i=0; i < h_prod.num_points; i++) {
	        POSTMSG( DEBUG,"LAT, LON: %g,%g\n",h_prod.lat_points[i],h_prod.lon_points[i]);
	     }
	   }

	   // Allocate more space for more points if needed
	   if(h_prod.num_points > num_points_allocated) {
	     pt = (BDRY_point_t *) realloc(pt,(sizeof(BDRY_point_t) * h_prod.num_points));
	     if(pt == NULL) 
	       POSTMSG(FATAL," Problems reallocating %d points\n",h_prod.num_points);
	     num_points_allocated = h_prod.num_points;
	     poly.points = pt;
	   }

	   // Not sure which one should be set - Use 'em all.
	   bdry.sequence_num = h_prod.id_no;
	   bdry.bdry_id = h_prod.id_no;
	   bdry.group_id = h_prod.id_no;

	   bdry.generate_time = h_prod.issueTime;
	   bdry.data_time = h_prod.data_time;
	   bdry.forecast_time = h_prod.data_time;
	   bdry.expire_time = h_prod.data_time + h_prod.valid_seconds;

	   bdry.polylines->num_secs = 0;
	   bdry.polylines->object_label =(char *)  h_prod.prod_label.c_str();
           bdry.desc = (char *)  h_prod.prod_label.c_str();

	   int count = 0;
	   int found_break = 0;
	   for( int i=0; i < h_prod.num_points && !found_break; i++) { 
	    
	     if ((h_prod.lat_points[i] < -90.0) || (h_prod.lat_points[i] > 90.0) ||
		 (h_prod.lon_points[i] < -180.0) || (h_prod.lon_points[i] > 360.0)){
	       fprintf(stderr, "lat/lons look odd - were all apps involved compiled 32 bit?\n");
	     }


		   // Trim out any pen up  points
		   if(h_prod.lat_points[i] > -90.0 ) {
	         bdry.polylines->points[count].lat = h_prod.lat_points[i];
	         bdry.polylines->points[count].lon = h_prod.lon_points[i];
	         bdry.polylines->points[count].u_comp = 0.0;
	         bdry.polylines->points[count].v_comp = 0.0;
	         bdry.polylines->points[count].value = 0.0;
			 count ++;
			}  else {
				found_break = 1;
			}
	   }

	   bdry.polylines->num_pts = count;

	   spdb_bdry = BDRY_product_to_spdb(&bdry,&buffer_len);

	   // Make big endian
	   BDRY_spdb_product_to_BE(spdb_bdry);

	   // Add to our data bases
	   if(HIB_spdb.put(SPDB_BDRY_ID,
			 SPDB_BDRY_LABEL,
			 bdry.bdry_id,
			 bdry.data_time,
			 bdry.expire_time,
			 buffer_len,
			 (void *) spdb_bdry,
			 bdry.bdry_id)) {

	            POSTMSG( DEBUG," Problems putting %d points to %s\n",
			h_prod.num_points,
			params._HIB[index].dest_url);

	   }

      }   // no valid match


     } else {  // no valid draw product
        sleep( 1 );
	 }

   } // while(1)

}
