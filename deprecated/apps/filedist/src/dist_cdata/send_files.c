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
 * send_files.c
 *
 * Send files for a given time.
 *
 * Returns 0 on success, -1 on failure.
 *
 * Mike Dixon, RAP, NCAR, POBOX 3000, Boulder, CO, 80307-3000, USA
 *
 * Sept 1998
 */

#include "dist_cdata.h"

int send_files(time_t file_time, time_t last_time,
	       char *tmp_dir, char *rsh, char *rcp)

{
  
  char file_name[FILE_MAX];
  char file_path[MAX_PATH_LEN];
  char tmp_name[FILE_MAX];
  char tmp_path[MAX_PATH_LEN];
  char date_dir[16];
  char sys_cmd[CMD_MAX];
  char *index_path;
 
  int ihost;
  int ret_val;
  int num_children, statusp;

  date_time_t ftime;
  struct stat sbuf;

  if (gd.debug) {
    fprintf(stderr, "file_time: %s\n", utimstr(file_time));
    fprintf(stderr, "last_time: %s\n", utimstr(last_time));
  }

  /*
   * build a unique temporary file name
   */
  
  sprintf(tmp_name, "%d.%d.tmp", (int) getpid(), rand());
  sprintf(tmp_path, "%s%s%s", tmp_dir, PATH_DELIM, tmp_name);

  /*
   * Build the file name
   */

  ftime.unix_time = file_time;
  uconvert_from_utime (&ftime);

  sprintf(file_name, "%.2d%.2d%.2d.%s",
	  ftime.hour, ftime.min, ftime.sec, gd.suffix);
  
  sprintf(date_dir, "%.4d%.2d%.2d",
	  ftime.year, ftime.month, ftime.day);

  sprintf(file_path, "%s%s%s%s%s",
	  gd.source_dir, PATH_DELIM, date_dir, PATH_DELIM, file_name);
  
  if(stat(file_path, &sbuf) < 0) {
    if (gd.debug) {
      fprintf(stderr, "File %s does not exist - not sent\n", file_path);
    } 
    return (-1);
  }

  /*
   * If too old or still being written - skip it it 
   */

  if(!gd.cdata_index &&
     sbuf.st_mtime >= (time(NULL) - gd.quiescent_secs)) {
    if (gd.debug) {
      fprintf(stderr, "File %s not quiescent - not sent\n", file_path);
    } 
    return (-1);
  }
  
  if (sbuf.st_mtime < last_time) {
    if (gd.debug) {
      fprintf(stderr, "File %s before last_time - not sent\n", file_path);
    } 
    return(-1);
  }
      
  PMU_force_register("Sending New file");

  /*
   * copy the file to tmp dir
   */
  
  sprintf(sys_cmd, "/bin/cp %s %s", file_path, tmp_path);
  ret_val = safe_sys(sys_cmd, gd.timeout, gd.debug);

  /*
   * if required, compress this file before remote copy
   */
      
  if(gd.compress_flag) {
    
    sprintf(sys_cmd, "compress %s", tmp_path);
    ret_val = safe_sys(sys_cmd, gd.timeout, gd.debug);
    STRconcat(tmp_name, ".Z", FILE_MAX);
    STRconcat(tmp_path, ".Z", MAX_PATH_LEN);
    
  } else if(gd.gzip_flag) {
    
    sprintf(sys_cmd, "gzip %s", tmp_path);
    ret_val = safe_sys(sys_cmd, gd.timeout, gd.debug);
    STRconcat(tmp_name, ".gz", FILE_MAX);
    STRconcat(tmp_path, ".gz", MAX_PATH_LEN);
    
  }
  
  sprintf(gd.cur_file, "%s", tmp_path);
  
  num_children = 0;
  
  /*
   * write index file as appropriate
   */
  
  if (gd.cdata_index) {
    index_path = write_index_file(file_time);
  }
  
  /*
   * Distribute this file to each host
   */
  
  for(ihost = 0; ihost < gd.num_hosts; ihost++) {
    
    send_to_host(gd.remote_host[ihost], &num_children,
		 date_dir, file_name, tmp_name, index_path, rsh, rcp);

  } /* ihost */
    
  while(num_children > 0) {   /* Reap all remaining child processes */
    wait(&statusp);
    num_children--;
  }
      
  /*
   * remove tmp file on local host
   */
  
  unlink(tmp_path);
  PMU_force_register("Done Sending New file");

  return (0);

}

