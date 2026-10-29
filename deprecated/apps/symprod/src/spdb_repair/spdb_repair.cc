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
 * spdb_repair.cc: Program to Repair SPDB files 
 * Please read the README
 * F. Hage. 3/99
 * Based on a template provided by Jaimi Yee
 *
 *********************************************************************/

#include <stdio.h>
#include <stream.h>
#include <stdlib.h>
#include <time.h>

#include <os_config.h>
#include <dataport/port_types.h>
#include <symprod/spdb_client.h>
#include <symprod/spdb_products.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>

#include <rapformats/station_reports.h>
#include <rapformats/trec_gauge.h>
#include <rapformats/zrpf.h>

////////////////////////////////////////////////////////////////////
// This section needs to change for each type of repair.
//
// Make sure to include the definition of the chunk data 
#include <rapformats/station_reports.h>

// Work with sets that span this interval - secs
#define INTERVAL_TIME 3600

/////////////////////////////////////////////////////////////////////


static int parse_args(int argc, char **argv);
static void tidy_and_exit(int sig);

// Set by parse_args() - Used by main();
char *input_db_dir;
char *output_db_dir;
char *start_time_string;
char *end_time_string;

/*********************************************************************
 * MAIN()
 */

int main(int argc, char **argv)
{
  // basic declarations

  char *prog_name;
  path_parts_t progname_parts;
  
  spdb_handle_t input_handle;
  spdb_handle_t output_handle;

  time_t start_time, end_time;
  time_t begin_time, span_time;

  int  nchunks;
  spdb_chunk_ref_t *headers;
  char *chunk_buffer;

  int i;

  // set program name
  uparse_path(argv[0], &progname_parts);
  prog_name = STRdup(progname_parts.base);


  if (parse_args(argc, argv) != 0) exit(-1);

  //
  // Get start and end times for data base
  //
  start_time = UTIMstring_US_to_time(start_time_string );
  end_time = UTIMstring_US_to_time(end_time_string );

  printf("Start time: %s",asctime(gmtime(&start_time)));
  printf("End time: %s",asctime(gmtime(&end_time)));

  // Sanity check
  if(start_time == 0 || end_time == 0 || (start_time >= end_time)) {
       fprintf( stderr, " Start time or end_time is bad\n");
       tidy_and_exit( -1 );
  }

  //
  // Initialize input spdb handle
  //

  if( SPDB_init( &input_handle, NULL, 0, input_db_dir ) ) {
     fprintf( stderr, "%s: ERROR: could not create input spdb handle",
              prog_name );
     tidy_and_exit( -1 );
  }

  // read some data to set vals in handle

  if( SPDB_fetch_closest( &input_handle, 0, start_time, 1000000000,
			  &nchunks, &headers, (void **) &chunk_buffer ) ) {
    fprintf( stderr, "%s: ERROR: could not read data base",
	     prog_name );
    tidy_and_exit( -1 );
  }
  
  // init output data base to be same type as input

  fprintf(stderr,"\n\n ********* PROD_ID: %d, %s ******** \n",input_handle.prod_id,input_handle.prod_label);

  if( SPDB_init( &output_handle, input_handle.prod_label,
		 input_handle.prod_id, output_db_dir ) ) {
    fprintf( stderr, "%s: ERROR: could not create output spdb handle",
	     prog_name );
    tidy_and_exit( -1 );
  }

  // Process database

  begin_time = start_time;
  while( begin_time < end_time )
  {
     span_time = begin_time + INTERVAL_TIME;
     if( span_time > end_time )
	span_time = end_time;

     fprintf(stdout, "Processsing time %s to %s\n",
	     utimstr(begin_time), utimstr(span_time));
     
     if( SPDB_fetch_interval( &input_handle, 0, begin_time, span_time,
                              &nchunks, &headers, (void **) &chunk_buffer ) ) {
	fprintf( stderr, "%s: ERROR: could not read data base",
                 prog_name );
	tidy_and_exit( -1 );
     }

     if(nchunks > 0) printf("Read %d chunks\n",nchunks);

     for( i = 0; i < nchunks; i++ ) {
	
       // determine the data type from the chunk data
       // no need to swap because ascii data only

       si32 data_type;
       void *chunk_ptr = (chunk_buffer + headers[i].offset);
       
       if (input_handle.prod_id == SPDB_STATION_REPORT_ID) {

	 station_report_t *report = (station_report_t *) chunk_ptr;
	 data_type = SPDB_4chars_to_int32(report->station_label);

       } else if (input_handle.prod_id == SPDB_TREC_GAUGE_ID) {

	 trec_gauge_hdr_t *gauge = (trec_gauge_hdr_t *) chunk_ptr;
	 data_type = SPDB_4chars_to_int32(gauge->name);

       } else if (input_handle.prod_id == SPDB_ZRPF_ID) {

	 zrpf_hdr_t *zrpf = (zrpf_hdr_t *) chunk_ptr;
	 data_type = SPDB_4chars_to_int32(zrpf->gauge_name);

       }

       // Add this to the new database - with the new data_type
       SPDB_store_add(&output_handle,
		      data_type,
		      headers[i].valid_time,
		      headers[i].expire_time,
		      chunk_ptr,
		      headers[i].len);

     }
 
     begin_time = span_time + 1;
     
  }
    
  SPDB_close(&input_handle);
  SPDB_close(&output_handle);

  tidy_and_exit(0);
}


/*******************************************************************
 * parse_args() - Parse the command line arguments.
 *
 * Returns 0 if successful and command line is proper, returns -1
 * if the program should exit.
 */

static int parse_args(int argc, char **argv)
{
  int error_flag = 0;
  char usage[BUFSIZ];
  char *tptr;
  
  // load up usage string

  if (argc == 5) {
      input_db_dir = argv[1];
      output_db_dir = argv[2];
      start_time_string = argv[3];
      end_time_string = argv[4];

      if((tptr = strchr(start_time_string,'_')) == NULL) {
	  error_flag = 1;
      } else {
	  *tptr = ' '; // replace the underscore with a space
      }

      if((tptr = strchr(end_time_string,'_')) == NULL) {
	  error_flag = 1;
      } else {
	  *tptr = ' '; // replace the underscore with a space
      }
  } else {
     error_flag = 1;
  }

  if(error_flag) {
     sprintf(usage, "%s %s %s",
	  "Usage:", argv[0], "input_db_dir out_db_dir start_time end_time\n" 
	  "Times use the format: hour:min_month/day/year\n"
	  "\n");
      fputs(usage,stderr);
      return -1;
  }

  return(0);
}

/************************************************************************
 * tidy_and_exit()
 */

static void tidy_and_exit(int sig)
{
   // free memory associated with SPDB calls

   SPDB_free_get();

   // close output file

  exit(sig);
}
