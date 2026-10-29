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
 * station2ascii.cc: Program to Print Station Report SPDB files 
 * 
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
//#include <spdb/Spdb.hh>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>

#include <rapformats/station_reports.h>
#include <rapformats/station_reports.h>

// Work with sets that span this interval - secs
#define INTERVAL_TIME 3600

/////////////////////////////////////////////////////////////////////


static int parse_args(int argc, char **argv);
static void tidy_and_exit(int sig);
static void print_report(station_report_t *report);

// Set by parse_args() - Used by main();
char *input_db_url;
char *start_time_string;
char *end_time_string;
char *gauge_name;
char *data_url;

/*********************************************************************
 * MAIN()
 */

int main(int argc, char **argv)
{
  int gauge_id;
  char *prog_name;
  path_parts_t progname_parts;
  
  time_t start_time, end_time;
  time_t begin_time, span_time;

//  spdb_handle_t input_handle;

  unsigned int  nchunks;
  spdb_chunk_ref_t *headers;
  station_report_t *report;

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
  gauge_id = SPDB_4chars_to_int32(gauge_name);

  fprintf(stderr,"Start time: %s",asctime(gmtime(&start_time)));
  fprintf(stderr,"End time: %s",asctime(gmtime(&end_time)));
  fprintf(stderr,"Gauge_ID : %s %d\n",gauge_name ,gauge_id);

  // Sanity check
  if(start_time == 0 || end_time == 0 || (start_time >= end_time)) {
       fprintf( stderr, " Start time or end_time is bad\n");
       tidy_and_exit( -1 );
  }

  //
  // Initialize input Spdb 
  //
//if( SPDB_init( &input_handle, NULL, 0, data_url ) ) {
//   fprintf( stderr, "%s: ERROR: could not create input spdb handle",
//            prog_name );
//   tidy_and_exit( -1 );
//}
    
  //Spdb spdb;
								
  // Process database

  begin_time = start_time;
  while( begin_time < end_time )
  {
     span_time = begin_time + INTERVAL_TIME;
     if( span_time > end_time )
	span_time = end_time;

     //fprintf(stderr, "Processsing time %s to %s\n",
     //	       utimstr(begin_time), utimstr(span_time));

//   if( SPDB_fetch_interval( &input_handle, gauge_id, start_time, end_time,
//                        &nchunks, &headers, (void **) &chunk_buffer ) ) {
//     fprintf( stderr, "%s: ERROR: could not read data base",
//           prog_name );
//     tidy_and_exit( -1 );
//   }
     
//    if( spdb.getInterval(data_url,start_time,end_time,gauge_id) < 0) {
//	fprintf( stderr, "%s: ERROR: could not read data base:\n %s\n",
//               prog_name,
//		spdb.getErrorStr().c_str());
//    }

//   nchunks = spdb.getNChunks();
//   headers = spdb.getChunkRefs();
//   chunk_buffer = (char *)  spdb.getChunkData();

	 void *ptr; // For Strict C++ compilers

     if(SPDB_get_interval(data_url,SPDB_STATION_REPORT_ID,
			  gauge_id,
			  begin_time,span_time,
			  &nchunks,
			  &headers,
			  &ptr)) {
	 fprintf( stderr, "%s: ERROR: could not read data base",prog_name );
	 tidy_and_exit( -1 );
     }
	 report = (station_report_t *) ptr;

     //if(nchunks > 0) fprintf(stderr, "Read %d chunks\n",nchunks);


     for( i = 0; i < nchunks; i++ ) {
	 station_report_from_be(&report[i]);

	 print_report(&report[i]); // Output a line.
     }
 
     begin_time = span_time + 1;
     
  }
    
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
      start_time_string = argv[1];
      end_time_string = argv[2];
      gauge_name = argv[3];
      data_url = argv[4];

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
	  "Usage:", argv[0], "start_time end_time Gauge_name Data_source(port@host)\n" 
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
  exit(sig);
}

/************************************************************************
 * PRINT_REPORT: Output an ascii line
 */

void print_report(station_report_t *report)
{
  struct tm *tm;
  double fract_hour;
  char t_label[64];

  tm = gmtime((time_t *) &report->time);

  fract_hour = tm->tm_hour + tm->tm_min/60.0 + tm->tm_sec/3600.0;

  strftime(t_label,64,"%Y,%m,%d",tm);
  fprintf(stdout, "%s,",t_label);

  fprintf(stdout, "%.4f,%2d,",  fract_hour,tm->tm_min);

  if(report->temp != STATION_NAN) {
      fprintf(stdout, "%.3f,",  report->temp);
  } else {
      fprintf(stdout, "-99.99,");
  }

  if(report->dew_point != STATION_NAN) {
      fprintf(stdout, "%.3f,",  report->dew_point);
  } else {
      fprintf(stdout, "-99.99,");
  }

  if(report->relhum != STATION_NAN) {
      fprintf(stdout, "%.3f,",  report->relhum);
  } else {
      fprintf(stdout, "-99.99,");
  }

  if(report->windspd != STATION_NAN) {
      fprintf(stdout, "%.3f,",report->windspd);
  } else {
      fprintf(stdout, "-99.99,");
  }

  if(report->winddir != STATION_NAN) {
      fprintf(stdout, "%.3f,", report->winddir);
  } else {
      fprintf(stdout, "-99.99,");
  }
     if(report->windgust != STATION_NAN) {
      fprintf(stdout, "%.3f,", report->windgust);
  } else {
      fprintf(stdout, "-99.99,");
  }

  if(report->pres != STATION_NAN) {
      fprintf(stdout, "%.3f,",  report->pres);
  } else {
      fprintf(stdout, "-99.99,");
  }

  if(report->liquid_accum != STATION_NAN) {
      fprintf(stdout, "%.3f,",
           report->liquid_accum);
  } else {
      fprintf(stdout, "-99.99,");
  }

  if(report->precip_rate != STATION_NAN) {
      fprintf(stdout, "%.3f\n", report->precip_rate);
  } else {
      fprintf(stdout, "-99.99\n");
  }   

}
