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
 * spdb_merge.cc: Program to Merge SPDB files 
 * F. Hage. 3/99
 *
 *********************************************************************/

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <ctime>

#include <toolsa/os_config.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>
#include <Spdb/DsSpdb.hh>
using namespace std;

// Work with sets that span this interval - secs
#define INTERVAL_TIME 3600

/////////////////////////////////////////////////////////////////////


static int parse_args(int argc, char **argv);
static void tidy_and_exit(int sig);

// Set by parse_args() - Used by main();
string input1_db_url;
string input2_db_url;
string output_db_url;

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
  
  DsSpdb input1_spdb;
  DsSpdb input2_spdb;
  DsSpdb output_spdb;
  output_spdb.setAppName("spdb_merge");

  time_t start_time, end_time;
  time_t begin_time, span_time;

  int  nchunks1,nchunks2;
  int pr_id;
  int i;

  string pr_label;

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
  // Initialize input spdb objects
  //
  output_spdb.addUrl(output_db_url);  // Set the output
  output_spdb.setPutMode(Spdb::putModeOver);  // Write over - avoid duplicates

  // Process database

  begin_time = start_time;
  while( begin_time < end_time )
  {
     span_time = begin_time + INTERVAL_TIME;
     if( span_time > end_time ) span_time = end_time;

     input1_spdb.clearPutChunks();     // Clean out input1 buffer
     input2_spdb.clearPutChunks();     // Clean out input2 buffer

     output_spdb.clearPutChunks();     // Start each span with a clean output buffer

     // READ
     if( input1_spdb.getInterval(input1_db_url, begin_time, span_time)) {
	fprintf(stderr,
		"Problems getting Interval from source 1 between %s and %s\n",
		asctime(gmtime(&begin_time)),
		asctime(gmtime(&span_time)));
     }

     nchunks1 = input1_spdb.getNChunks();
     if(nchunks1 > 0) printf("Read %d chunks from source 1\n",nchunks1);

     // Get the Chunk vector ref
     const vector< Spdb::chunk_t > &chunks = input1_spdb.getChunks();

     pr_id = input1_spdb.getProdId();
     pr_label = input1_spdb.getProdLabel();

     // PUT INTO BUFFER
     for( i = 0; i < nchunks1; i++ ) {
	 output_spdb.addPutChunk(chunks[i].data_type,
	                      chunks[i].valid_time,
	                      chunks[i].expire_time,
	                      chunks[i].len,
	                      chunks[i].data,
	                      chunks[i].data_type2);
     }

     //READ
     if( input2_spdb.getInterval(input2_db_url, begin_time, span_time)) {
	fprintf(stderr,
		"Problems getting Interval from source 2 between %s and %s\n",
		asctime(gmtime(&begin_time)),
		asctime(gmtime(&span_time)));
     }

     nchunks2 = input2_spdb.getNChunks();
     if(nchunks2 > 0) printf("Read %d chunks from source 2\n",nchunks2);

     const vector< Spdb::chunk_t > &chunks2 = input2_spdb.getChunks();
     pr_id = input2_spdb.getProdId();
     pr_label = input2_spdb.getProdLabel();

     // ADD TO BUFFER
     for( i = 0; i < nchunks2; i++ ) {
	 output_spdb.addPutChunk(chunks2[i].data_type,
	                      chunks2[i].valid_time,
	                      chunks2[i].expire_time,
	                      chunks2[i].len,
	                      chunks2[i].data,
	                      chunks2[i].data_type2);
     }

     // WRITE
     if( output_spdb.put(output_db_url, pr_id,pr_label)) {
	fprintf(stderr,
		"Problems Putting Interval between %s and %s\n",
		asctime(gmtime(&begin_time)),
		asctime(gmtime(&span_time)));
     }

     // Move to the next time span
     begin_time = span_time + 1;
     
  }// End of while( begin_time < end_time )
    
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

  if (argc == 6) {
      input1_db_url = argv[1];
      input2_db_url = argv[2];
      output_db_url = argv[3];

      start_time_string = argv[4];
      end_time_string = argv[5];

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
	  "Usage:", argv[0], "input1_db_url input2_db_url out_db_url start_time end_time\n" 
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

   // close output file

  exit(sig);
}
