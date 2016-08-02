/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/*********************************************************************
 * PROCESS_ARGS
 * 
 * Process command line args
 *
 * Set option flags and print usage info if necessary.
 *
 * Returns the total length of the arg list
 *
 * Frank Hage Dec 1995 NCAR, Research Applications Program
 */

#include "dist_cdata.h"
 
static void print_usage(FILE *out);

#define ARG_OPTION_STRING   "cguvISd:D:s:f:h:e:r:i:t:q:C:H:m:"

int process_args(int argc, char *argv[])

{
  int err_flag =0;
  int i,c;
  int arg_count = 1;  /* first arg is exec name */
  char stamp_name[STAMP_MAX];
  extern char *optarg;  /* option argument string */


  /*
   * set defaults
   */
  
  gd.debug = 0;
  gd.compress_flag = 0;
  gd.gzip_flag = 0;
  gd.timeout = TIMEOUT_SECS; 
  gd.max_files_on_start = MAX_FILES_ON_START;
  gd.num_hosts = 0;
  gd.cdata_index = 0;
  gd.secure_shell= 0;
  gd.check_secs = CHECK_SECS;
  gd.quiescent_secs = QUIESCENT_SECS;

  STRncopy(gd.app_instance, "Generic", PROCMAP_INSTANCE_MAX);
  STRncopy(gd.suffix, "mdv", SUFFIX_MAX);
  STRncopy(stamp_name, "t_stamp", STAMP_MAX);
  STRncopy(gd.per_file_cmd, "", CMD_MAX);
  STRncopy(gd.end_cmd, "", CMD_MAX);
  STRncopy(gd.source_dir, ".", MAX_PATH_LEN);
  STRncopy(gd.dest_dir, "null", MAX_PATH_LEN);
  
  while ((c = getopt(argc, argv, ARG_OPTION_STRING)) != EOF) {

    switch(c) {
      
    case 'c':  /* Turn on file compression prior to remote copy */
      gd.compress_flag = 1;
      arg_count += 1;
      break;
   
    case 'g':  /* Turn on file gzipping prior to remote copy */
      gd.gzip_flag = 1;
      arg_count += 1;
      break;
      
    case 'u':  /* Turn on file uncompress or gunzip after copy */
      gd.uncompress_flag = 1;
      arg_count += 1;
      break;
      
    case 'd':  /* directory to examine for data */ 
      STRncopy(gd.source_dir, optarg, MAX_PATH_LEN);
      arg_count += 2;
      break;
   
    case 'D':  /* remote destination directory */
      STRncopy(gd.dest_dir, optarg, MAX_PATH_LEN);
      arg_count += 2;
      break;
   
    case 'e':  /* command to run at end of file transfers */ 
      STRncopy(gd.end_cmd, optarg, CMD_MAX);
      arg_count += 2;
      break;
   
    case 'i':  /* Application Instance */ 
      STRncopy(gd.app_instance, optarg, PROCMAP_INSTANCE_MAX);
      arg_count += 2;
      break;
   
    case 'I':  /* Send cdata index file */
      gd.cdata_index = 1;
      arg_count += 1;
      break;
   
    case 'f':  /* Time stamp file */ 
      STRncopy(stamp_name, optarg, STAMP_MAX);
      arg_count += 2;
      break;
   
    case 'h':  /* remote host to send data */ 
      if (gd.num_hosts < MAX_NHOSTS - 1) {
	STRncopy(gd.remote_host[gd.num_hosts++], optarg, HOST_MAX);
        arg_count += 2;
      } else {
	err_flag++;
      }
      break;
   
    case 'r':  /* command to run at end of each file transfer */ 
      STRncopy(gd.per_file_cmd, optarg, CMD_MAX);
      arg_count += 2;
      break;
   
    case 's':  /* suffix of data files */ 
      STRncopy(gd.suffix, optarg, SUFFIX_MAX);
      arg_count += 2;
      break;
   
    case 'S':  /* use secure shell for transfers */ 
      gd.secure_shell = 1;
      arg_count += 1;
      break;
   
    case 't':  /* timeouts for transfers */
      gd.timeout = atoi(optarg);
      arg_count += 2;
      if(gd.timeout < 1 || gd.timeout > 3600) {
       fprintf(stderr,"Error - Strange timeout value: %d \n",gd.timeout);
       fprintf(stderr,"Valid Range: 1-3600 seconds inclusive\n");
       _exit(-1);
      }
       
      break;
      
    case 'm':  /* max files on startup */
      gd.max_files_on_start = atoi(optarg);
      arg_count += 2;
      if(gd.max_files_on_start < 1 || gd.max_files_on_start > 100) {
	gd.max_files_on_start = MAX_FILES_ON_START;
      }
      break;
      
    case 'q':  /* quiescent time */
      gd.quiescent_secs = atoi(optarg);
      arg_count += 2;
      if(gd.quiescent_secs < 1 || gd.quiescent_secs > 1200) {
	gd.quiescent_secs = QUIESCENT_SECS;
      }
      break;
      
    case 'C':  /* check time */
      gd.check_secs = atoi(optarg);
      arg_count += 2;
      if(gd.check_secs < 1 || gd.check_secs > 1200) {
	gd.check_secs = CHECK_SECS;
      }
      break;
      
    case 'v':  /* Turn on verbose output */
      gd.debug = 1;
      arg_count += 1;
      break;
   
    case 'H':  /* dummy grab - see loop below */
      break;
   
    case '?':  /* error in options */
    default:
      err_flag++;
      break;
    }
  
  };

  for(i=arg_count +1; i < argc && gd.num_hosts < MAX_NHOSTS - 1; i++) {
      STRncopy(gd.remote_host[gd.num_hosts], argv[i], HOST_MAX);
      gd.num_hosts++;
  }
 
  for(i=0; i < gd.num_hosts; i++) {
    if(gd.debug)
      fprintf(stderr,
	      "Destination: %s\n",gd.remote_host[i]);
  }
 
  if(gd.num_hosts <= 0 ) err_flag++;

  if(!strcmp(gd.dest_dir, "null")) {
    strcpy(gd.dest_dir, gd.source_dir);
  }

  sprintf(gd.stamp_file, "%s%s%s",
	  gd.source_dir, PATH_DELIM, stamp_name);
 
  if(err_flag) {
    print_usage(stderr);
    _exit(-1);
  }

  return (0);

} 

static void print_usage(FILE *out)

{

  fprintf(out, "\n");
  fprintf(out, "dist_cdata\n");
  fprintf(out, "\n");
  fprintf(out, "  This program distributes new data files to remote hosts\n");
  fprintf(out, "\n");

  fprintf(out,
	  "Usage:\n\n"
	  "  dist_cdata [optional args below] [-H host host host ... (must be last)]\n"
	  "     [-C how often to check (secs)]\n"
	  "     [-c (compress the data before sending)]\n"
	  "     [-g (gzip the data before sending)]\n"
	  "     [-D dest_data_dir]\n"
	  "     [-d data_directory]\n"
	  "     [-e cmd_to_run_after_all_files_transfered]\n"
	  "     [-f time_stamp_file]\n"
	  "     [-h host (single host list only)]\n"
	  "     [-I (send cdata index file)]\n"
	  "     [-i instance]\n"
	  "     [-m max_files] max number of files sent at startup\n"
	  "     [-q file quiescent time (secs)]\n"
	  "     [-r cmd_to_run_after_each_file]\n"
	  "     [-s data_file_suffix]\n"
	  "     [-S use secure shell operations]\n"
	  "     [-t timeout_secs]\n"
	  "     [-u uncompress or gunzip after transfer.]\n"
	  "         Used with -c or -g only.\n"
	  "     [-v (turns on verbose optput)]\n");

  fprintf(out, "\n");
  fprintf(out,
	  "  Defaults are: -d . -t %d -f t_stamp -s mdv -q %d -C %d -m %d\n",
	  TIMEOUT_SECS, QUIESCENT_SECS, CHECK_SECS, MAX_FILES_ON_START);

  fprintf(out, "\n");

  fprintf(out,
	  "  Max of %d hosts.\n", MAX_NHOSTS);

  fprintf(out, "\n");

  fprintf(out,
	  "  No commands run by default.\n");

  fprintf(out, "\n");

  fprintf(out,
	  "  The destination data_directory must exist under the\n"
	  "    home dir of each remote host,\n"
	  "  OR the whole path must be listed and must exist on each\n"
	  "    remote host.\n");

  fprintf(out, "\n");

  fprintf(out,
	  "  Commands are run on each remote host - must be in users path.\n");

  fprintf(out, "\n");

}


