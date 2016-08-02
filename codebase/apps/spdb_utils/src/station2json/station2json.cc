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
 * station2json.cc: Program to Print Station Report SPDB files 
 * 
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <Spdb/DsSpdb.hh>
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

  unsigned int  nchunks;
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
  gauge_id = Spdb::hash4CharsToInt32(gauge_name);

  // Sanity check
  if(start_time == 0 || end_time == 0 || (start_time >= end_time)) {
       fprintf( stderr, " Start time or end_time is bad\n");
       tidy_and_exit( -1 );
  }

  DsSpdb spdb;
								
  // Process database

  begin_time = start_time;
  fprintf(stdout, "[");
  while( begin_time < end_time )
  {
     span_time = begin_time + INTERVAL_TIME;
     if( span_time > end_time ) span_time = end_time;

     //fprintf(stderr, "Processsing time %s to %s\n",
     //	       utimstr(begin_time), utimstr(span_time));

     
    if( spdb.getInterval(data_url,begin_time,span_time,gauge_id) < 0) {
	fprintf( stderr, "%s: ERROR: could not read data base:\n %s\n",
               prog_name,
		spdb.getErrorStr().c_str());
    }

   const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
   nchunks = spdb.getNChunks();

     //if(nchunks > 0) fprintf(stderr, "Read %d chunks\n",nchunks);

     for( i = 0; i < nchunks; i++ ) {

	 report = (station_report_t *) chunks[i].data;
	 station_report_from_be(report);
	 print_report(report); // Output a line.
         if (i < (nchunks-1) || span_time < end_time) {
	    fprintf(stdout,",");
         }
     }
 
     begin_time = span_time + 1;
     
  }  
  fprintf(stdout, "]");
    
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

  } else {
     error_flag = 1;
  }

  if(error_flag) {
     sprintf(usage, "%s %s %s %s",
	  "Error: Wrong number of arguments\n",
	  "Usage:", argv[0], " start_time end_time Gauge_name Data_url\n" 
	  "Times use the format: hour:min_month/day/year\n");
     fputs(usage,stderr);
      return -1;
  }

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

  if(error_flag) {
     sprintf(usage, "%s %s %s %s",
	  "Error: Badly format on time strings\n",
	  "Usage:", argv[0], " start_time end_time Gauge_name Data_url\n" 
	  "Times use the format: hour:min_month/day/year\n");
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
 * PRINT_REPORT: Output a json line
 */

void print_report(station_report_t *report)
{
  char t_label[64];

  fprintf(stdout, "{\"time\":%d,",  report->time);
  fprintf(stdout, "\"id\":\"%s\",", report->station_label);

  fprintf(stdout, "\"temp\":%.3f,",  (report->temp != STATION_NAN) ? report->temp : STATION_NAN);
  fprintf(stdout, "\"dew\":%.3f,",  (report->dew_point != STATION_NAN) ? report->dew_point : STATION_NAN);
  fprintf(stdout, "\"rh\":%.3f,",  (report->relhum != STATION_NAN) ? report->relhum : STATION_NAN);
  fprintf(stdout, "\"wspd\":%.3f,",  (report->windspd != STATION_NAN) ? report->windspd : STATION_NAN);
  fprintf(stdout, "\"wdir\":%.3f,",  (report->winddir != STATION_NAN) ? report->winddir : STATION_NAN);
  fprintf(stdout, "\"wgst\":%.3f,",  (report->windgust != STATION_NAN) ? report->windgust : STATION_NAN);
  fprintf(stdout, "\"pres\":%.3f,",  (report->pres != STATION_NAN) ? report->pres : STATION_NAN);
  fprintf(stdout, "\"acm1\":%.3f,",  (report->liquid_accum != STATION_NAN) ? report->liquid_accum : STATION_NAN);
  fprintf(stdout, "\"rate\":%.3f,",  (report->precip_rate != STATION_NAN) ? report->precip_rate : STATION_NAN);
  fprintf(stdout, "\"vis\":%.3f,",  (report->visibility != STATION_NAN) ? report->visibility : STATION_NAN);
  fprintf(stdout, "\"rvr\":%.3f,",  (report->rvr != STATION_NAN) ? report->rvr : STATION_NAN);
  fprintf(stdout, "\"cei\":%.3f,",  (report->ceiling != STATION_NAN) ? report->ceiling : STATION_NAN);
  fprintf(stdout, "\"acm2\":%.3f,",  (report->shared.station.liquid_accum2 != STATION_NAN) ? report->shared.station.liquid_accum2 : STATION_NAN);
  fprintf(stdout, "\"sp1\":%.3f,",  (report->shared.station.Spare1 != STATION_NAN) ? report->shared.station.Spare1 : STATION_NAN);
  fprintf(stdout, "\"sp2\":%.3f,",  (report->shared.station.Spare2 != STATION_NAN) ? report->shared.station.Spare2 : STATION_NAN);
  fprintf(stdout, "\"wx\":%d}",  (report->weather_type != WX_NAN) ? report->weather_type : WX_NAN);
}
