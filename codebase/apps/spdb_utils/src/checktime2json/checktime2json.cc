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
 * checktime2json.cc: Program to Print Station Report SPDB files 
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

#include <rapformats/ChecktimeReport.hh>

/////////////////////////////////////////////////////////////////////


static int parse_args(int argc, char **argv);
static void tidy_and_exit(int sig);
static void print_report(time_t time, ChecktimeReport &report);

// Set by parse_args() - Used by main();
char *input_db_url;
char *start_time_string;
char *data_url;

/*********************************************************************
 * MAIN()
 */

int main(int argc, char **argv)
{
  int gauge_id;
  char *prog_name;
  path_parts_t progname_parts;
  
  time_t start_time;

  unsigned int  nchunks;

  int i;

  // set program name
  uparse_path(argv[0], &progname_parts);
  prog_name = STRdup(progname_parts.base);


  if (parse_args(argc, argv) != 0) exit(-1);

  //
  // Get times for data base
  //
  start_time = UTIMstring_US_to_time(start_time_string );

  // Sanity check
  if(start_time == 0) {
       fprintf( stderr, " Start time is bad\n");
       tidy_and_exit( -1 );
  }

  DsSpdb spdb;
								
  // Process database
  fprintf(stdout, "[");
     
  if( spdb.getFirstBefore(data_url,start_time,300) < 0) {
	fprintf( stderr, "%s: ERROR: could not read data base:\n %s\n",
               prog_name,
		spdb.getErrorStr().c_str());
  }

   vector<Spdb::chunk_t> chunks = spdb.getChunks();
   nchunks = spdb.getNChunks();
   ChecktimeReport report;

   for( i = 0; i < nchunks; i++ ) {
        const Spdb::chunk_t &chunk = chunks[i];
        int data_len = chunk.len;
        void *data = chunk.data;

        if (!report.disassemble(data, data_len)) {
            cerr << "ERROR - Print::ChecktimeReport" << endl;
            cerr << "  Cannot disassemble chunk." << endl;
            continue;
        }

        time_t time = chunks[i].valid_time;
        int tmp = report.getNFluids();
        fprintf(stdout, "%d,", tmp);
        fprintf(stdout, "%d,", chunks[i].len);
	print_report(time, report); // Output a line.
        
        if (i < (nchunks-1)) {
	    fprintf(stdout,",");
        }
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

  if (argc == 3) {
      start_time_string = argv[1];
      data_url = argv[2];

  } else {
     error_flag = 1;
  }

  if(error_flag) {
     sprintf(usage, "%s %s %s %s",
	  "Error: Wrong number of arguments\n",
	  "Usage:", argv[0], " time Data_url\n" 
	  "Time uses the format: hour:min_month/day/year\n");
     fputs(usage,stderr);
      return -1;
  }

  if((tptr = strchr(start_time_string,'_')) == NULL) {
	  error_flag = 1;
  } else {
	  *tptr = ' '; // replace the underscore with a space
  }

  if(error_flag) {
     sprintf(usage, "%s %s %s %s",
	  "Error: Bad format on time string\n",
	  "Usage:", argv[0], " time Data_url\n" 
	  "Time uses the format: hour:min_month/day/year\n");
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

void print_report(time_t time, ChecktimeReport &report)
{
  cerr << report.getNFluids() << endl;
  vector<ChecktimeReport::checktime_fluid_t> fluids = report.getFluids();

  for (int j=0;j<fluids.size();j++) {
    fprintf(stdout, "{\"time\":%d,", time);
    fprintf(stdout, "\"fluid\":%d,", fluids[j].fluid_id);
    fprintf(stdout, "\"dilution\":%d,", fluids[j].fluid_dilution);
    fprintf(stdout, "\"failtimes\":[");

    for (int i=0;i<8;i++) {
        fprintf(stdout, "%d,", fluids[j].fract_failure_time[i]);
    }
    fprintf(stdout, "%d],", fluids[j].fract_failure_time[8]);
    fprintf(stdout, "\"status\":%d,", fluids[j].status);
    fprintf(stdout, "\"checktime\":%d", fluids[j].checktime);

    fprintf(stdout, "}");
    if (j < fluids.size()-1) {
      fprintf(stdout, ",");
    }
  }


}
