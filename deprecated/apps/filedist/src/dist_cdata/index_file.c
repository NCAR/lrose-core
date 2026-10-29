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
 * index_file.c
 *
 * Index file routines
 * 
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80303, USA
 *
 * November 1996
 */

#include "dist_cdata.h"
 
/**********************
 * write_index_file()
 *
 * Writes a tmp index file for remote machines
 *
 * Returns index file path, NULL on failure
 *
 */

char *write_index_file(long data_time)

{
  
  static first_call = TRUE;
  static char index_dir[MAX_PATH_LEN];
  static LDATA_handle_t ldata;

  struct stat dstat;

  if (first_call) {

    /*
     * init LDATA handle
     */
    
    LDATA_init_handle(&ldata, "dist_cdata", gd.debug);

    /*
     * set up index dir
     */
    
    sprintf(index_dir, "%s%sremote_index",
	    gd.source_dir, PATH_DELIM);
    
    /*
     * make index subdirectory if it does not exist
     */
  
    if (stat(index_dir, &dstat)) {
      if (mkdir(index_dir, 0775)) {
	fprintf(stderr, "WARNING - dist_cdata\n");
	fprintf(stderr, "Cannot create tmp index dir\n");
	perror(index_dir);
	return (NULL);
      }
    }

    first_call = FALSE;

  }

  /*
   * write latest data info file
   */

  if (LDATA_info_write(&ldata, index_dir, data_time,
		       gd.suffix,
		       NULL, NULL, 0, NULL)) {
    
    fprintf(stderr, "WARNING - dist_cdata\n");
    fprintf(stderr, "Cannot write index file\n");
    return (NULL);

  } else {

    return (ldata.file_path);

  }
  
}

/******************************************************
 * send_index_file()
 *
 * Sends index file off to the remote host.
 *
 * Returns 0 on success, -1 on failure
 *
 */

void send_index_file(char *index_path, char *remote_host)

{

  char sys_cmd[CMD_MAX];
  int ret_val;
  char *rsh, *rcp;

  if (gd.secure_shell) {
    rsh = "ssh -n";
    rcp = "scp";
  } else {
    rsh = "rsh -n";
    rcp = "rcp";
  }
  
  /*
   * send current index file to tmp path
   */
  
  sprintf(sys_cmd, "%s %s %s:%s%s%s", rcp, index_path,
	  remote_host, gd.dest_dir, PATH_DELIM, "tmp_info_file");
  ret_val  = safe_sys(sys_cmd, gd.timeout, gd.debug);
  
  /*
   * mv tmp file to final name
   */
  
  sprintf(sys_cmd,"%s %s /bin/mv %s%s%s %s%s_%s", rsh,
	  remote_host,
	  gd.dest_dir, PATH_DELIM, "tmp_info_file",
	  gd.dest_dir, PATH_DELIM, LDATA_INFO_FILE_NAME);
  ret_val  = safe_sys(sys_cmd, gd.timeout, gd.debug);

}

