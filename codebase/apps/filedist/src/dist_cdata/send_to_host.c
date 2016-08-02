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
 * send_to_host.c
 *
 * Send file to a given host
 *
 * Returns 0 on success, -1 on failure.
 *
 * Mike Dixon, RAP, NCAR, POBOX 3000, Boulder, CO, 80307-3000, USA
 *
 * Sept 1998
 */

#include "dist_cdata.h"

int send_to_host(char *host_name, int *num_children_p,
		 char *date_dir, char *file_name,
		 char *tmp_name, char *index_path,
		 char *rsh, char *rcp)

{
  
  char sys_cmd[CMD_MAX];
/*   char *tmp_log_path; */
 
  int ret_val;
  int rcp_ret_val;
  int mv_ret_val;
  int c_pid, statusp;

  time_t expire_time;

  if(*num_children_p >= MAX_CHILDREN) {
      
    /*
     * Wait for children to die before forking too many processes
     */
    
    wait(&statusp);
    (*num_children_p)--;
      
  }
    
  if((c_pid = fork()) != 0) {
    
    /*
     * Is the parent - return to continue on to the next host
     */
    
    (*num_children_p)++;
    return (0);
      
  }
  
  /*
   * This is the child process - send this data file to one host only
   */
  
  STRncopy(gd.cur_host, host_name, HOST_MAX);
  
  /*
   * write tmp log file
   */
  
/*   tmp_log_path = write_tmp_log(); */
  
  /*
   * Start the death timer - this child will die then (if not sooner)
   */
  
  /* Allow 10 extra seconds to elapse before killing this process */
  
  set_timer(gd.timeout + 10);
  expire_time = time(NULL) + gd.timeout + 1;
    
  /*
   * make sure the date subdirectory and tmp exist on the remote host
   */
  
  sprintf(sys_cmd, "%s %s mkdir %s%s%s %s%s%s >& /dev/null",
	  rsh, host_name,
	  gd.dest_dir, PATH_DELIM, date_dir,
	  gd.dest_dir, PATH_DELIM, "tmp");
    
  ret_val  = safe_sys(sys_cmd,(int)(expire_time - time(0)), gd.debug);
  
  /*
   * Remove any old files of the
   * same name on the remote machine
   */
  
  sprintf(sys_cmd, "%s %s /bin/rm -f %s%s%s%s%s*",
	  rsh, host_name,
	  gd.dest_dir, PATH_DELIM, date_dir, PATH_DELIM, file_name);
    
  ret_val  = safe_sys(sys_cmd, (int)(expire_time - time(0)), gd.debug);
  
  /*
   * copy the tmp file to remote host
   */
  
  sprintf(sys_cmd,"%s %s%s%s%s%s %s:%s%s%s%s%s",
	  rcp, gd.source_dir, PATH_DELIM, "tmp", PATH_DELIM, tmp_name,
	  host_name,
	  gd.dest_dir, PATH_DELIM, "tmp", PATH_DELIM, tmp_name);
    
  rcp_ret_val  = safe_sys(sys_cmd,(int)(expire_time - time(0)), gd.debug);
  
  sleep(1);
  
  /*
   * mv to final file name
   */
  
  if(gd.compress_flag) {
    STRconcat(file_name, ".Z", FILE_MAX);
  } else if(gd.gzip_flag) {
    STRconcat(file_name, ".gz", FILE_MAX);
  }

  sprintf(sys_cmd,"%s %s mv %s%s%s%s%s %s%s%s%s%s",
	  rsh, host_name,
	  gd.dest_dir, PATH_DELIM, "tmp", PATH_DELIM, tmp_name,
	  gd.dest_dir, PATH_DELIM, date_dir, PATH_DELIM, file_name);
    
  mv_ret_val  = safe_sys(sys_cmd,(int)(expire_time - time(0)), gd.debug);

  /*
   * uncompress as appropriate
   */

  if (gd.compress_flag && gd.uncompress_flag) {

    sprintf(sys_cmd,"%s %s uncompress %s%s%s%s%s",
	    rsh, host_name,
	    gd.dest_dir, PATH_DELIM, date_dir, PATH_DELIM, file_name);
    
    ret_val  = safe_sys(sys_cmd,(int)(expire_time - time(0)), gd.debug);

  } else if (gd.gzip_flag && gd.uncompress_flag) {

    sprintf(sys_cmd,"%s %s gunzip %s%s%s%s%s",
	    rsh, host_name,
	    gd.dest_dir, PATH_DELIM, date_dir, PATH_DELIM, file_name);
    
    ret_val  = safe_sys(sys_cmd,(int)(expire_time - time(0)), gd.debug);

  }
    
  /*
   * send index file as appropriate
   */
  
  if (gd.cdata_index && index_path != NULL &&
      rcp_ret_val == 0 && mv_ret_val ==0 ) {
    send_index_file(index_path, host_name);
  }
	
  /*
   * run per-file command on remote
   */
  
  if(strlen(gd.per_file_cmd) > MIN_CMD_LEN) {
    sprintf(sys_cmd,"%s %s %s", rsh,
	    host_name,gd.per_file_cmd);
    ret_val  = safe_sys(sys_cmd,(int)(expire_time - time(0)), gd.debug);
  }
    
  /*
   * remove tmp log file and die
   */
  
/*   if (tmp_log_path != NULL) { */
/*     if (unlink(tmp_log_path)) { */
/*       fprintf(stderr, "WARNING - dist_cdata\n"); */
/*       fprintf(stderr, "Cannot unlink tmp log file\n"); */
/*       perror(tmp_log_path); */
/*     } else { */
/*       if (gd.debug) { */
/* 	fprintf(stderr, "Removing log file %s\n", tmp_log_path); */
/*       } */
/*     } */
/*   } */
   
  _exit(0); /* Children die after each transfer */

  return (0);

}
