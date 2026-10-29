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

/*********************************************************************
 * convertBdry.cc: Program to convert boundaries in spdb from 
 *                 km/hr to m/s.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1999
 *
 * Jaimi Yee
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <os_config.h>
#include <dataport/port_types.h>
#include <rapformats/bdry.h>
#include <symprod/spdb_client.h>
#include <symprod/spdb_products.h>
#include <tdrp/tdrp.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>
#include <toolsa/file_io.h>

#include "convertBdry_tdrp.h"

#define INTERVAL_TIME 5
#define MAX_TYPE_LEN 40
#define KMPERHR_TO_MPERSEC 0.27777778
#define SPDB_DATA_TYPE 0

// Prototypes for static functions

static int parse_args(int argc, char **argv,
		      int *check_params_p,
                      int *print_params_p,
		      char **params_file_path_p,
		      tdrp_override_t *override);

static void tidy_and_exit(int sig);


// Global variables

convertBdry_tdrp_struct Params;
FILE *OutputFp;


/*********************************************************************
 * main()
 */

int main(int argc, char **argv)
{
  // basic declarations

  char *prog_name;
  path_parts_t progname_parts;
  char *params_path_name;
  
  int check_params, print_params;
  tdrp_override_t override;
  TDRPtable *table;

  time_t first_time, end_time;
  time_t start_time, last_time;

  int   nchunks;
  spdb_chunk_ref_t *headers;
  char *chunk_buffer;

  BDRY_spdb_product_t *bdry_spdb_product;
  int productLen;

  int i, j, k, l;

  // initialize output file pointer
  OutputFp = NULL;
  
  // set program name

  uparse_path(argv[0], &progname_parts);
  prog_name = STRdup(progname_parts.base);

  // display ucopyright message

  ucopyright(prog_name);

  // parse the command line arguments

  if (parse_args(argc, argv, &check_params, &print_params,
		 &params_path_name, &override) != 0)
    exit(-1);

  // load up parameters

  table = convertBdry_tdrp_init(&Params);
  
  if (params_path_name)
  {
    if (TDRP_read(params_path_name,
		  table, &Params,
		  override.list) == FALSE)
    {
      fprintf(stderr, "ERROR - %s: Cannot read params file '%s'\n",
	      prog_name, params_path_name);
      exit(-1);
    }
  }
  else
  {
    TDRP_set_defaults(table, &Params);
  }
  
  TDRP_free_override(&override);
  
  if (check_params)
  {
    TDRP_check_is_set(table, &Params);
    tidy_and_exit(0);
  }
  if (print_params)
  {
    TDRP_print_params(table, &Params, prog_name, TRUE);
    tidy_and_exit(0);
  }

  //
  // Get start and end times for data base
  //
  first_time = UTIMstring_US_to_time( Params.start_time );
  last_time = UTIMstring_US_to_time( Params.end_time );
  if( Params.debug ) {
     fprintf( stderr, "%s: DEBUG: start time = %s, end time = %s\n",
	      prog_name, Params.start_time, Params.end_time );
  }
  
  //
  // Process boundary files
  //
  start_time = first_time;
  while( start_time < last_time )
  {
     end_time = start_time + INTERVAL_TIME;
     if( end_time > last_time )
	end_time = last_time;
     
     if( SPDB_get_interval( Params.spdb_source, SPDB_BDRY_ID, 
                            SPDB_DATA_TYPE, start_time, end_time,
                            (ui32 *) &nchunks, &headers, 
                            (void **) &chunk_buffer ) ) {
	fprintf( stderr, "%s: ERROR: could not read data base",
                 prog_name );
	tidy_and_exit( -1 );
     }
     
     if( Params.debug ) {
 	fprintf( stderr, "%s: DEBUG: got %d products for %s to %s\n",
 		 prog_name, nchunks, UTIMstr(start_time), UTIMstr(end_time) );
     }

     for( i = 0; i < nchunks; i++ ) {
	
        bdry_spdb_product = (BDRY_spdb_product_t *)
	   (chunk_buffer + headers[i].offset );
	
        BDRY_spdb_product_from_BE( bdry_spdb_product );

        productLen = sizeof( BDRY_spdb_product_t );


	//
	// Convert u and v components from m/s to km/hr
	//
        for( j = 0; j < bdry_spdb_product->num_polylines; j++ ) {
	   for( k = 0; k < bdry_spdb_product->polylines[j].num_pts; k++ ) {

              if( bdry_spdb_product->polylines[j].points[k].u_comp !=
                  BDRY_VALUE_UNKNOWN )
		 bdry_spdb_product->polylines[j].points[k].u_comp *= 
		    KMPERHR_TO_MPERSEC;
	      
              if( bdry_spdb_product->polylines[j].points[k].v_comp !=
                  BDRY_VALUE_UNKNOWN )
		 bdry_spdb_product->polylines[j].points[k].v_comp *= 
		    KMPERHR_TO_MPERSEC;

	   }

           productLen += sizeof( BDRY_spdb_polyline_t ) +
	      bdry_spdb_product->polylines[j].num_pts * 
	      sizeof( BDRY_spdb_point_t );
	   
	}

        bdry_spdb_product->motion_speed *= KMPERHR_TO_MPERSEC;
	
        BDRY_spdb_product_to_BE( bdry_spdb_product );
	
     }
     
     if( nchunks > 0 ) {
	
	if( Params.debug ) {
	   fprintf( stderr, "%s: DEBUG: Writing %d products to database\n",
		    prog_name, nchunks );
	}
     
	for( l = 0; l < Params.spdb_destinations.len; l++ ) {
	   SPDB_put_add( Params.spdb_destinations.val[l],
			 SPDB_BDRY_ID, SPDB_BDRY_LABEL,
			 (ui32) nchunks, headers,
			 (void *) chunk_buffer, 
			 productLen );
	}
     }

     start_time = end_time + 1;
  }
    
  tidy_and_exit(0);
}


/*******************************************************************
 * parse_args() - Parse the command line arguments.
 *
 * Returns 0 if successful and command line is proper, returns -1
 * if the program should exit.
 */

static int parse_args(int argc, char **argv,
		      int *check_params_p,
                      int *print_params_p,
		      char **params_file_path_p,
		      tdrp_override_t *override)
{
  int error_flag = 0;
  int warning_flag = 0;
  int i;

  char usage[BUFSIZ];
  
  // load up usage string

  sprintf(usage, "%s%s%s",
	  "Usage:\n\n", argv[0], " [options] as below:\n\n"
	  "       [ --, -help, -man] produce this list.\n"
	  "       [ -check_params] check parameter usage\n"
	  "       [ -params name] parameters file name (required)\n"
	  "       [ -print_params] print parameter usage\n"
	  "\n");

  // initialize

  *check_params_p = FALSE;
  *print_params_p = FALSE;
  TDRP_init_override(override);
  *params_file_path_p = (char *)NULL;
  
  // search for command options
  
  for (i =  1; i < argc; i++)
  {
    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "-man"))
    {
      printf("%s", usage);
      return(-1);
    }
    else if ( STRequal_exact(argv[i], "-check_params") )
    {
      *check_params_p = TRUE;
    }
    else if ( STRequal_exact(argv[i], "-print_params") )
    {
       *print_params_p = TRUE;
    }
    else if (i < argc - 1)
    {
      if (STRequal_exact(argv[i], "-params"))
      {
	*params_file_path_p = argv[i+1];

	i++;
      }
    } /* if (i < argc - 1) */
    
  } /* i */

  // print usage if there was an error

  if (error_flag || warning_flag)
  {
    fprintf(stderr, "%s\n", usage);
    fprintf(stderr, "Check the parameters file '%s'.\n\n",
	    *params_file_path_p);
  }

  if (error_flag)
    return(-1);

  return(0);
}

/************************************************************************
 * tidy_and_exit()
 */

static void tidy_and_exit(int sig)
{
   //
   // Free memory associated with SPDB calls
   //
   SPDB_free_get();

   //
   // Exit with code sig
   //
   exit(sig);

}
