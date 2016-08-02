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
#include <stdlib.h>
#include <time.h>
#include <iostream>

#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <Spdb/DsSpdb.hh>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>

#include <rapformats/ltg.h>

// Work with sets that span this interval - secs
#define INTERVAL_TIME 3600

/////////////////////////////////////////////////////////////////////


static int parse_args(int argc, char **argv);
static void tidy_and_exit(int sig);
static void print_strike(LTG_strike_t &s);
static void print_ext_strike(LTG_extended_t &strike);

// Set by parse_args() - Used by main();
char *input_db_url;
char *start_time_string;
char *end_time_string;
char *data_url;
int add_label;

double lat_min,lon_min,lat_max,lon_max; // Bounding box

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
  LTG_strike_t *strike;

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

  // Sanity check
  if(start_time == 0 || end_time == 0 || (start_time >= end_time)) {
       fprintf( stderr, "Start time or end_time is bad\n");
       fprintf( stderr, "Use the format: hour:min_month/day/year\n");
       tidy_and_exit( -1 );
  }

  int has_err = 0;
  if(lat_min < -90 || lat_min > 90) has_err = 1;
  if(lat_max < -90 || lat_max > 90) has_err = 1;
  if(lon_min < -180 || lon_min > 180) has_err = 1;
  if(lon_max < -180 || lon_max > 180) has_err = 1;
  if(lat_max < lat_min || lon_max < lon_min ) has_err = 1;
  if(has_err) {
       fprintf( stderr, "Lat-Lon bounding box problems\n");
       fprintf( stderr, "Expecting Lon_min Lon_max Lat_min Lat_max\n");
       fprintf( stderr, "Found Lon: %.4f to %.4f  Lat: %.4f to %.4f\n",lon_min,lon_max,lat_min,lat_max);
       tidy_and_exit( -1 );
  }

  DsSpdb spdb;
                                                                
  // Process database

  begin_time = start_time;
  while( begin_time < end_time )
  {
     span_time = begin_time + INTERVAL_TIME -1;
     if( span_time > end_time ) span_time = end_time;

     // DEBUG:  fprintf(stderr, "Processsing time %s to %s from %s\n",
     //              utimstr(begin_time), utimstr(span_time),data_url);

	 
    if(spdb.getInterval(data_url,begin_time,span_time,0) < 0) {
        fprintf( stderr, "%s: ERROR: could not read data base:\n %s\n",
               prog_name,
                spdb.getErrorStr().c_str());
    }

   const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
   nchunks = spdb.getNChunks();
   int p_id = spdb.getProdId();

   //DEBUG:  fprintf(stderr, "Read %d chunks\n",nchunks);

     for( i = 0; i < nchunks; i++ ) {
	 int data_len = chunks[i].len;
	 unsigned int cookie;
	 memcpy(&cookie, chunks[i].data, sizeof(cookie));
	 if(cookie == LTG_EXTENDED_COOKIE) {
	    int num_strikes = data_len / sizeof(LTG_extended_t);
	    LTG_extended_t *ltg_data = (LTG_extended_t *)chunks[i].data;
	    for(int j=0; j < num_strikes; j++) {
		LTG_extended_from_BE(&(ltg_data[j]));
                print_ext_strike(ltg_data[j]);
	    }
	 } else {
	    int num_strikes = data_len / sizeof(LTG_strike_t);
	    LTG_strike_t *ltg_data = (LTG_strike_t *)chunks[i].data;
	    for(int j=0; j < num_strikes; j++) {
		LTG_from_BE(&(ltg_data[j]));
                print_strike(ltg_data[j]);
	    }
	 }

     }
 
     begin_time = span_time + 1; // The next second after the end of the last interval.
     
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

  add_label = 0;
   
  // load up usage string

  if (argc == 8) {
      data_url = argv[1];
      start_time_string = argv[2];
      end_time_string = argv[3];
          lon_min = atof(argv[4]);
          lon_max = atof(argv[5]);
          lat_min = atof(argv[6]);
          lat_max = atof(argv[7]);


  } else if (argc == 9) { // -l added
      data_url = argv[2];
      start_time_string = argv[3];
      end_time_string = argv[4];
          lon_min = atof(argv[5]);
          lon_max = atof(argv[6]);
          lat_min = atof(argv[7]);
          lat_max = atof(argv[8]);

          if(strncmp(argv[1],"-l",2) != 0) error_flag = 1;
          add_label = 1;
  } else {
     error_flag = 1;
  }

  if(error_flag) {
     sprintf(usage, "%s %s %s %s",
          "Error: Wrong number of arguments\n",
          "Usage:", argv[0], " [-l] Ltg_URL start_time end_time Lon_min Lon_max Lat_min Lat_max \n" 
          "Times use the format: hour:min_month/day/year\n"
          "-l - Add a Legend\n");
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
          "Usage:", argv[0], " [-l] start_time end_time Gauge_name Data_url\n" 
          "Times use the format: hour:min_month/day/year\n"
          "-l - Add a Legend\n");
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
 * PRINT_STRIKE: Output an ascii line
 */

void print_strike(LTG_strike_t &strike)
{
  struct tm *tm;
  double fract_hour;
  char t_label[64];

  if(add_label) {
    printf("# Yr Mo D  Hr Mi S     Julian     Lat       Lon      Amplitude   Strike_Type\n");
    add_label = 0;  // Make it a one shot.
  }

  // Check bounding box
  if(strike.latitude == LTG_MISSING_FLOAT ||
	 strike.longitude == LTG_MISSING_FLOAT ||
	 strike.longitude < lon_min ||
	 strike.longitude > lon_max ||
	 strike.latitude < lat_min ||
	 strike.latitude > lat_max ) return;


  tm = gmtime((time_t *) &strike.time);

  strftime(t_label,64,"%Y %m %d %H %M %S",tm);
  fprintf(stdout, "%s",t_label);

  double jday = tm->tm_yday + tm->tm_hour/24.0 + tm->tm_min/1440.0 + tm->tm_sec/86400.0;
  fprintf(stdout, " %10.5f", jday);

  fprintf(stdout, " %10.5f", strike.latitude);
  fprintf(stdout, " %10.5f", strike.longitude);

  if (strike.amplitude != LTG_MISSING_INT) {
    fprintf(stdout, " %6d", strike.amplitude);
  } else {
    fprintf(stdout, " %6d", -99999);
  }
  if (strike.type != LTG_MISSING_INT) {
    fprintf(stdout, " %20s", LTG_type2string(strike.type));
  } else {
    fprintf(stdout, " none");
  }
  fprintf(stdout, "\n");
}

/************************************************************************
 * PRINT_EXT_STRIKE: Output an ascii line
 */

void print_ext_strike(LTG_extended_t &strike)
{
  struct tm *tm;
  double fract_hour;
  char t_label[64];
  if(add_label) {
    printf("# Yr Mo D  Hr Mi S    Julian     Lat        Lon      Amplitude    Strike_Type     secs\n");
    add_label = 0;  // Make it a one shot.
  }

  // Check bounding box
  if(strike.latitude == LTG_MISSING_FLOAT ||
	 strike.longitude == LTG_MISSING_FLOAT ||
	 strike.longitude < lon_min ||
	 strike.longitude > lon_max ||
	 strike.latitude < lat_min ||
	 strike.latitude > lat_max ) return;

  tm = gmtime((time_t *) &strike.time);

  strftime(t_label,64,"%Y %m %d %H %M %S",tm);
  fprintf(stdout, "%s",t_label);

  double jday = tm->tm_yday + tm->tm_hour/24.0 + tm->tm_min/1440.0 + tm->tm_sec/86400.0;
  fprintf(stdout, " %10.5f", jday);

  fprintf(stdout, " %10.5f", strike.latitude);
  fprintf(stdout, " %10.5f", strike.longitude);

  if (strike.amplitude != LTG_MISSING_FLOAT) {
    fprintf(stdout, " %6.0f", strike.amplitude);
  } else {
    fprintf(stdout, " %6.0f", -99999.9);
  }
  if (strike.type != LTG_MISSING_INT) {
    fprintf(stdout, " %20s", LTG_type2string(strike.type));
  } else {
    fprintf(stdout, " None");
  }
  if (strike.nanosecs != LTG_MISSING_INT) {
    fprintf(stdout, " %5.2f",(double) strike.nanosecs/ 1000000000.0);
  } else {
    fprintf(stdout, " %10d", -99999);
  }
  fprintf(stdout, "\n");
}
