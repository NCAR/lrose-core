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
 * DIST_CDATA.C  A Simple program to Distribute the Latest CDATA files to 
 * a remote host
 * Frank Hage	Dec 1995 NCAR, Research Applications Program
 */

#define MAIN

#include "dist_cdata.h"
 
/********************************************************************
 * MAIN: Process arguments, initialize and begin application
 */
 
int
main(int argc, char **argv)

{

  int forever = TRUE;
  int i;
  int ret_val;
  int num_sent;
  int nstart;
  int num_data_times;
  time_t *data_time_list; 

  FILE *s_file;

  char tmp_dir[MAX_PATH_LEN];
  
  char sys_cmd[CMD_MAX];

  char *rsh, *rcp;

  struct stat sbuf;
  time_t expire_time;
  time_t last_time = 0;
  time_t now = 0;
  time_t search_time;

  LDATA_handle_t ldata;

  process_args(argc,argv);

  if (gd.secure_shell) {
    rsh = "ssh -n";
    rcp = "scp";
  } else {
    rsh = "rsh -n";
    rcp = "rcp";
  }
  
  if (gd.cdata_index) {
    LDATA_init_handle(&ldata, "dist_cdata", gd.debug);
  }

  PMU_auto_init("dist_cdata",
		gd.app_instance, PROCMAP_REGISTER_INTERVAL);
  
  signal(SIGINT,signal_trap);
  signal(SIGTERM,signal_trap);
  signal(SIGPIPE,signal_trap);

  /*
   * make sure the temp directory exists
   */

  sprintf(tmp_dir, "%s%s%s", gd.source_dir, PATH_DELIM, "tmp");
  if(stat(tmp_dir, &sbuf) < 0) {
    if (mkdir(tmp_dir, 0777)) {
      fprintf(stderr, "Cannot make tmp directory\n");
      perror(tmp_dir);
      return (-1);
    }
  }

  /*
   * loop forever
   */

  while (forever) {

    /*
     * Find out the modification time of the time_stamp file
     */
    
    errno = 0;
    if(stat(gd.stamp_file, &sbuf) < 0) {
      if(errno == ENOENT) {  /* status file missing */
	last_time = 0;
      } else {
	perror("dist_cdata: Stat failed on time stamp file\n");
	return(-1);
      }
    } else {
      last_time = sbuf.st_mtime;
    }
    
    now = time(NULL);

    PMU_auto_register("Checking For New files");
    
    if (gd.cdata_index) {
      LDATA_info_read_blocking(&ldata, gd.source_dir, 3600,
			       1, PMU_auto_register);
      search_time = ldata.ltime.unix_time;
    } else {
      search_time = now;
    }

    /*
     * Find all data files output within 2 hours
     */
    
    num_data_times =
      find_times(&data_time_list, gd.source_dir, gd.suffix,
		 search_time-7200, search_time+7200);

    /*
     * start looking at the last MAX_FILES files - Ignore old files
     */
    
    nstart = num_data_times - gd.max_files_on_start;
    if (nstart < 0) {
      nstart = 0;
    }
    
    /* Send the appropriate files */

    num_sent = 0;

    for (i = nstart; i < num_data_times; i++) {

      if (send_files(data_time_list[i], last_time,
		     tmp_dir, rsh, rcp) == 0) {
	num_sent++;
      }

    } /* i */

    if(num_sent > 0) {

      /*
       * end command
       */

      for(i=0; i < gd.num_hosts; i++ ) {
	if(strlen(gd.end_cmd) > MIN_CMD_LEN) {
	  sprintf(sys_cmd,"%s -n %s %s &", rsh,
		  gd.remote_host[i], gd.end_cmd);
	  ret_val  = safe_sys(sys_cmd,(int)(expire_time - time(0)), gd.debug);
	}
      }

      /*
       * Update the time stamp file
       */
      
      if((s_file = fopen(gd.stamp_file,"w")) == NULL) {
	fprintf(stderr, "Couldn't open time stamp file %s for writing\n",
		gd.stamp_file);
	perror("Dist_cdata: Open failed\n");
	_exit(-1);
      }
      fprintf(s_file, "dist_cdata: %s\n", ctime(&now));
      fclose(s_file);
      
    }

    if (!gd.cdata_index) {
      PMU_auto_register("Waiting for new files");
      sleep(gd.check_secs);
    }

  } /* while (forever) */

  return (0);
}

