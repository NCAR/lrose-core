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

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <ctime>

#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>
#include <sys/time.h>
#include <time.h>

#include <physics/thermo.h>

#include <Spdb/Spdb.hh>
#include <Spdb/DsSpdb.hh>

using namespace std;

#include <rapformats/station_reports.h>
#include <rapformats/ComboPt.hh>
#include <rapformats/trec_gauge.h>
#include <rapformats/zrpf.h>
#include <rapformats/Sndg.hh>
#include <Spdb/sounding.h>

////////////////////////////////////////////////////////////////////
// This section needs to change for each type of repair.
//
// Make sure to include the definition of the chunk data 
#include <rapformats/station_reports.h>
using namespace std;

// Work with sets that span this interval - secs
#define INTERVAL_TIME 86400
//#define INTERVAL_TIME 3600

/////////////////////////////////////////////////////////////////////


static int parse_args(int argc, char **argv);
static void tidy_and_exit(int sig);

// Set by parse_args() - Used by main();
char *input_db_url;
char *output_db_url;
char *start_time_string;
char *end_time_string;

/*********************************************************************
 * MAIN()
 */

int main(int argc, char **argv)
{
  // basic declarations
  struct timeval tv;
  struct timezone tz;
  unsigned int tot_in_bytes = 0;
  unsigned int tot_out_bytes = 0;
  unsigned int tot_in_records = 0;
  unsigned int tot_out_records = 0;
  unsigned int tot_puts = 0;
  unsigned int tot_reads = 0;
  double in_mb_sec = 0.0;
  double out_mb_sec = 0.0;
  double in_recs_sec = 0.0;
  double out_recs_sec = 0.0;
  double elapsed_in_sec = 0.0;
  double elapsed_out_sec = 0.0;
  double start_t;
  double end_t;


  char *prog_name;
  path_parts_t progname_parts;
  
  DsSpdb input_spdb;
  DsSpdb output_spdb;
  output_spdb.setAppName("spdb_repair");

  time_t start_time, end_time;
  time_t begin_time, span_time;

  int  nchunks,nwritten;
  char *chunk_buffer;

  int i,index;

  // set program name
  uparse_path(argv[0], &progname_parts);
  prog_name = STRdup(progname_parts.base);


  if (parse_args(argc, argv) != 0) exit(-1);

  // Init the output
  output_spdb.setPutMode(Spdb::putModeAdd);
  output_spdb.clearUrls();
  output_spdb.addUrl(output_db_url);

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

  // Process database

  begin_time = start_time;
  while( begin_time < end_time )
  {
     span_time = begin_time + INTERVAL_TIME -1;
     if( span_time > end_time ) span_time = end_time;

     fprintf(stdout, "Processsing time %s to %s\n",
         utimstr(begin_time), utimstr(span_time));
     
	 gettimeofday(&tv,&tz);
	 start_t = (double) tv.tv_sec + tv.tv_usec / 1000000.0;
	 // Get some data.
     input_spdb.getInterval(input_db_url,begin_time, span_time,0,0,false);
	 gettimeofday(&tv,&tz);
	 end_t = (double) tv.tv_sec + tv.tv_usec / 1000000.0;
	 elapsed_in_sec += end_t - start_t;
	 tot_reads++;

     const vector<Spdb::chunk_t> &chunks =  input_spdb.getChunks();

     nchunks = input_spdb.getNChunks();
	 tot_in_records += nchunks;
     int p_id = input_spdb.getProdId();
	 nwritten = 0;

     if(nchunks > 0) printf("Read %d chunks",nchunks);
	 output_spdb.clearPutChunks();

	 // Process Each Chunk.
     for( i = 0; i < nchunks; i++ ) {
    
       // determine the data type from the chunk data

       si32 data_type;
       void *chunk_ptr = chunks[i].data;
	   tot_in_bytes += chunks[i].len;
       
	   // Struct pointer defs go here because they can't go inside case-break blocks.
	   ComboPt pt;
	   GenPt gpt; 
       station_report_t *report;
       zrpf_hdr_t *zrpf;
	   Sndg::header_t *snd_hdr;

       switch(p_id) {
         case SPDB_STATION_REPORT_ID:

           report = (station_report_t *) chunk_ptr;

           //  Swap to local byte order
           station_report_from_be(report);

           data_type = input_spdb.hash4CharsToInt32(report->station_label);

           // Fix Bad Wind Direction for AME* Gauges.
           if(strncmp(report->station_label,"WOR",3) == 0) {
			   report->winddir += 180.0;
               if(report->winddir >= 360.0) report->winddir -= 360.0;
		   }

		   // ADD dewpoint
		   if(report->temp != STATION_NAN && report->relhum && report->dew_point == STATION_NAN) {
			 report->dew_point = PHYrhdp(report->temp,report->relhum);
		   }

           // Swap Back to network byte order
           station_report_to_be(report);

		 break;

		 case SPDB_SNDG_PLUS_ID:
		   snd_hdr = (Sndg::header_t *) chunk_ptr;
		   strncpy(snd_hdr->sourceFmt,"\0\0\0\0",4);
		   strncpy(snd_hdr->siteName,input_spdb.dehashInt32To4Chars(chunks[i].data_type).c_str(),4);
		   data_type = chunks[i].data_type;
		 break;

		 case SPDB_COMBO_POINT_ID: // Cull on missing elevation
		   pt.disassemble(chunks[i].data,chunks[i].len);
		   gpt = pt.get2DPoint();
		   index = gpt.getFieldNum("Elevation");
		   data_type = chunks[i].data_type;
		   if(gpt.get1DVal(index) < -5000.0) continue;
		 break;

		 case SPDB_SNDG_ID:
           data_type = input_spdb.hash4CharsToInt32("AMA");
		 break;

         case SPDB_ZRPF_ID:
           zrpf = (zrpf_hdr_t *) chunk_ptr;
           // no need to swap because ascii data only
           data_type = input_spdb.hash4CharsToInt32(zrpf->gauge_name);
	     break;
       }

	   nwritten++;
	   tot_out_bytes += chunks[i].len;
	   tot_out_records++;
       // Add this to the new database - with the new data_type
	   output_spdb.addPutChunk(p_id,
		   chunks[i].valid_time,
		   chunks[i].expire_time,
		   chunks[i].len,
		   chunk_ptr,
		   data_type);
     }

	 gettimeofday(&tv,&tz);
	 start_t = (double) tv.tv_sec + tv.tv_usec / 1000000.0;
	 output_spdb.put(output_db_url, p_id, input_spdb.getProdLabel());
	 gettimeofday(&tv,&tz);
	 end_t = (double) tv.tv_sec + tv.tv_usec / 1000000.0;
	 elapsed_out_sec += end_t - start_t;
	 tot_puts++;

     printf(" Wrote %d\n",nwritten);
     begin_time = span_time + 1;
     
  }


  in_mb_sec = (tot_in_bytes / 1000000.0) / elapsed_in_sec;
  in_recs_sec = tot_in_records / elapsed_in_sec;
  out_mb_sec = (tot_out_bytes / 1000000.0) / elapsed_out_sec;
  out_recs_sec = tot_out_records / elapsed_out_sec;
  printf("IO Throughput: Average record size: %d\n",(int) tot_in_bytes/tot_in_records);
 
  printf("INPUT:   %u total Get calls, %u bytes, %.2f elapsed seconds, %.3f records/sec,  %.3f mb/sec\n",
        tot_reads,tot_in_bytes,elapsed_in_sec,in_recs_sec,in_mb_sec);
  printf("OUTPUT: %u total Put calls, %u bytes, %.2f elapsed seconds, %.3f records/sec,  %.3f mb/sec\n",
        tot_puts,tot_out_bytes,elapsed_out_sec,out_recs_sec,out_mb_sec);
            
    
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
      input_db_url = argv[1];
      output_db_url = argv[2];
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
      "Usage:", argv[0], "input_db_url out_db_url start_time end_time\n" 
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
   // close output file

  exit(sig);
}
