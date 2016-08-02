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

#include <rapformats/GenPoly.hh>

//
// The application class and version
//
#include "DrawFmq2Polygon.hh"
const string DrawFmq2Polygon::version = "0.1_alpha";

//
// Putting the parameter structure inside as a member of
// the application class does not pass the muster with Purify
//
#include "DrawFmq2Polygon_tdrp.h"
DrawFmq2Polygon_tdrp_struct params;

#include <Spdb/DsSpdb.hh>

//////////////////////////////////////////////////////////////////////
// Application Constructor 
// 
DrawFmq2Polygon::DrawFmq2Polygon()
{
}

//////////////////////////////////////////////////////////////////////
// Application Destructor 
// 
DrawFmq2Polygon::~DrawFmq2Polygon()
{
}

//////////////////////////////////////////////////////////////////////
// Application Initializer 
// 
int
DrawFmq2Polygon::init(int argc, char **argv )
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
DrawFmq2Polygon::usage()
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
DrawFmq2Polygon::processArgs( int argc, char **argv,
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
DrawFmq2Polygon::readParams( const string &paramPath, 
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

   table = DrawFmq2Polygon_tdrp_init( &params );

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
DrawFmq2Polygon::processParams( const string &paramPath )
{
   if ( !paramPath.empty() ) {
      POSTMSG( DEBUG, "Loading Parameter file: %s", paramPath.c_str() );
   }

   //
   // Initialize message logging
   //
   if ( params.debug == TRUE ) {
      msgLog.enableMsg( DEBUG, true );
   }
	
   //
   // Initialize message logging
   //
   POSTMSG( DEBUG, "Initializing message log." );


   if ( msgLog.setOutputDir( params.log_dir ) != 0 ) {
      POSTMSG( WARNING, "Cannot write log messages to output directory '%s'",
                        params.log_dir );
   }
     
   // Initialize the Draw queue
   //
   POSTMSG(DEBUG, "Initializing the Draw Input queue at %s.", params.draw_fmq_url  );
   if ( drawqueue.initReadBlocking( params.draw_fmq_url,
                            PROGRAM_NAME, DEBUG_ENABLED ) != 0 ) {
      POSTMSG( ERROR, "Cannot initialize Draw queue.\n"
                      "Make sure draw_fmq_url (%s) is correct.",
                      params.draw_fmq_url );
      return( -1 );
   }

  

   return( 0 );
}

//////////////////////////////////////////////////////////////////////
// 
// APPLICATION MAIN ENTRY POINT - DO THE WORK 
// Store vertices of polygon in GenPoly format.
//
void
DrawFmq2Polygon::run()
{

  POSTMSG( DEBUG, "Setting up spdb output");
   //
   // Set up SPDB output
   //
   DsSpdb HIP_spdb;
   
   //
   // Unique draw products with same times will be added
   //
   HIP_spdb.setPutMode(Spdb::putModeOver); 

   //
   // Read messages off the Queue. If the proper one arrives
   // Reformat it into a  GenPoly object and post it to the database.
   //
   POSTMSG( DEBUG, "Watching DrawQueue %s for output", params.draw_fmq_url);

   while( 1 ) 
     {
       //
       // Register with procmap
       //
       PMU_auto_register( "Looping around the trigger condition" );
     
       int status = -1;
       //
       // get Draw Queue product
       //
       const Human_Drawn_Data_t& h_prod = drawqueue.nextProduct( status );
       
       if ( status == 0 ) 
	 { 
	   //
	   // success
	   //
	   POSTMSG(DEBUG, "Got a Message - Product_ID_Label: %s\n",
		   h_prod.id_label.c_str());
	   
	   //
	   // Determine list of urls to which product will be sent
	   // by checking ID label.
	   //
 	   HIP_spdb.clearUrls();
	   
	   int url_index = -1;
	   
	   for (int i = 0; i < params.HIP_n; i++ ) 
	     {
	       if(strncmp(h_prod.id_label.c_str(),params._HIP[i].ID_label,
			  DrawQueue::ID_label_Len) == 0) 
		 {  
		   HIP_spdb.addUrl(params._HIP[i].dest_url);
		   
		   url_index = i;
		 }
	     }
              
	   if(url_index >= 0) 
	     {
	       POSTMSG( DEBUG, "Product ID_NO: %d, Valid: %d seconds\n",h_prod.id_no,h_prod.valid_seconds);

	       POSTMSG( DEBUG, "Product issued: %s", ctime(&h_prod.issueTime));

	       POSTMSG( DEBUG, "Product data time: %s", ctime(&h_prod.data_time));

	       POSTMSG( DEBUG, "Product label text: %s\n", h_prod.prod_label.c_str());

	       POSTMSG( DEBUG, "Product sender: %s\n", h_prod.sender.c_str());

	       POSTMSG( DEBUG, "%d points\n", h_prod.num_points);

	       //
	       // parse label string for interest value. Interest value is 
	       // found after '&'.
	       //
	       char buf[100];

	       float interest;

	       sprintf(buf, "%s", h_prod.prod_label.c_str());

	       char *interest_ptr = strchr(buf, '&');

	       if (interest_ptr == NULL)
		 interest = 0;
	       else
		 {
		   interest_ptr = interest_ptr+1;
		   
		   interest = atof(interest_ptr);
		 }

	       POSTMSG( DEBUG, "%f polygon interest value\n", interest);

	       //
	       // Create polygon 
	       //
	       GenPoly  genPoly;

	       genPoly.setName("Human Defined Polygon");

	       genPoly.setId(h_prod.id_no);

	       genPoly.setTime( h_prod.data_time); 

	       genPoly.setNLevels(1);

	       genPoly.setClosedFlag(1);

	       genPoly.setFieldInfo("interest:no_units");

	       genPoly.addVal(interest);

	       for(int i=0; i < h_prod.num_points; i++) 
		 {
           if ((h_prod.lat_points[i] < -90.0) || (h_prod.lat_points[i] > 90.0) ||
     			 (h_prod.lon_points[i] < -180.0) || (h_prod.lon_points[i] > 360.0)){
				 fprintf(stderr, "lat/lons look odd - were all apps involved compiled 32 bit?\n");
		   }

		   //
		   // Add vertex to GenPoly
		   //
		   GenPoly::vertex_t vertex;
		   
			 // Take only the first polygon. 
		     if( h_prod.lat_points[i] > -90.1 ) {
			     vertex.lat = h_prod.lat_points[i];
		   
			     vertex.lon = h_prod.lon_points[i];
			   
			     genPoly.addVertex(vertex);
		   
			     if(DEBUG_ENABLED) 
		       {
			       POSTMSG( DEBUG,"LAT, LON: %g,%g\n",h_prod.lat_points[i],h_prod.lon_points[i]);
			     }
				 } else {
						 i= h_prod.num_points; // Break out of the loop
			     if(DEBUG_ENABLED) 
		       {
			       POSTMSG( DEBUG,"Pen Up detected\n");
			     }
				 }
			 } 
	       
	       //
	       // assemble GenPoly object
	       // 
	       genPoly.assemble();
	       
	       
	       //
	       // Add to our data bases
	       //
	       if( HIP_spdb.put(SPDB_GENERIC_POLYLINE_ID,
			       SPDB_GENERIC_POLYLINE_LABEL,
			       h_prod.id_no,
			       h_prod.data_time,
			       h_prod.data_time + 1200,
			       genPoly.getBufLen(),
				(void *) genPoly.getBufPtr() ))
		 {
		   POSTMSG( DEBUG," Problems putting GenPoly to url list\n"); 
		 }
	       else
		 POSTMSG( DEBUG,"Spdb data successfully written.");
	       
	     } // if (url_index ==0)
	 }
       else
	 {
	   //
	   // status == 1 (ie. no valid draw product)
	   //
	   sleep( 1 );
	 }
     }// while(1)
}




