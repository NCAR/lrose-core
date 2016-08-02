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
/*************************************************************************
 *
 * RfWriteDobson.c
 *
 * part of the rfutil library - radar file access
 *
 * Dobson file access routines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * December 1994
 *
 **************************************************************************/

#include <titan/file_io.h>
#include <titan/mdv.h>
#include <titan/radar.h>
#include <toolsa/ldata_info.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_SEQ 256

/*************************************************************************
 *
 * RfWriteDobson.c
 *
 * writes dobson volume, with index file as appropriate
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfWriteDobson"
#define TMP_DOB_FILE_NAME "tmp_dobson_file"

int RfWriteDobson(vol_file_handle_t *v_handle,
		  int write_current_index,
		  int debug,
		  const char *output_dir,
		  const char *output_file_ext,
		  const char *prog_name,
		  const char *calling_routine)
     
{

  char calling_sequence[MAX_SEQ];
  char tmp_file_path[MAX_PATH_LEN];
  char file_name[MAX_PATH_LEN];
  char file_path[MAX_PATH_LEN];
  char dir_path[MAX_PATH_LEN];
  
  vol_params_t *vparams;
  
  struct stat dir_stat;

  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * compute file names
   */

  vparams = v_handle->vol_params;
  
  sprintf(dir_path, "%s%s%.4d%.2d%.2d",
	  output_dir, PATH_DELIM,
	  (int) vparams->mid_time.year,
	  (int) vparams->mid_time.month,
	  (int) vparams->mid_time.day);
  
  sprintf(file_name, "%.2d%.2d%.2d.%s",
	  (int) vparams->mid_time.hour,
	  (int) vparams->mid_time.min,
	  (int) vparams->mid_time.sec,
	  output_file_ext);
    
  sprintf(file_path, "%s%s%s",
	  dir_path, PATH_DELIM, file_name);
      
  sprintf(tmp_file_path, "%s%s%s.%ld",
	  output_dir, PATH_DELIM, TMP_DOB_FILE_NAME,
	  (long) time(NULL));

  /*
   * create directory, if needed
   */

  if (0 != stat(dir_path, &dir_stat)) {
    if (mkdir(dir_path, 0755)) {
      fprintf(stderr, "ERROR - %s\n", calling_sequence);
      fprintf(stderr, "Trying to make output dir\n");
      perror(dir_path);
      return (R_FAILURE);
    }
  }

  /*
   * write tmp file
   */
  
  v_handle->vol_file_path = tmp_file_path;
  
  if (RfWriteVolume(v_handle, calling_sequence) != R_SUCCESS) {
    return(R_FAILURE);
  }

  /*
   * move to correct location
   */

  rename(tmp_file_path, file_path);

  if (write_current_index) {

    LDATA_handle_t ldata;
    date_time_t dtime;

    LDATA_init_handle(&ldata, prog_name, debug);
	
    Rfrtime2dtime(&vparams->mid_time, &dtime);

    if (LDATA_info_write(&ldata,
			 output_dir,
			 dtime.unix_time,
			 output_file_ext,
			 NULL, NULL, 0, NULL)) {
      LDATA_free_handle(&ldata);
      return (R_FAILURE);
    }
    
    LDATA_free_handle(&ldata);

  } /* if (write_current_index) */

  return (R_SUCCESS);

}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * RfWriteDobsonRemote.c
 *
 * writes dobson volume, with index file as appropriate,
 * to remote machine
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "RfWriteDobsonRemote"
#define TMP_DOB_FILE_NAME "tmp_dobson_file"

int RfWriteDobsonRemote(vol_file_handle_t *v_handle,
			int write_current_index,
			int debug,
			const char *output_host,
			const char *output_dir,
			const char *output_file_ext,
			const char *local_tmp_dir,
			const char *prog_name,
			const char *calling_routine)
     
{

  char calling_sequence[MAX_SEQ];
  char file_name[MAX_PATH_LEN];
  char remote_file_path[MAX_PATH_LEN];
  char remote_dir_path[MAX_PATH_LEN];
  char tmp_file_path[MAX_PATH_LEN];
  char remote_tmp_file_path[MAX_PATH_LEN];
  char call_str[BUFSIZ];

  long now;

  vol_params_t *vparams;

  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * For hostname "local", call the local routine
   */

  if (strcmp(output_host, "local") == 0) {
    return (RfWriteDobson(v_handle,
			  write_current_index,
			  debug,
			  output_dir,
			  output_file_ext,
			  prog_name,
			  calling_sequence));
  }

  /*
   * compute file names and paths
   */

  vparams = v_handle->vol_params;
  
  sprintf(file_name, "%.2d%.2d%.2d.%s",
	  (int) vparams->mid_time.hour,
	  (int) vparams->mid_time.min,
	  (int) vparams->mid_time.sec,
	  output_file_ext);
    
  sprintf(remote_dir_path, "%s%s%.4d%.2d%.2d",
	  output_dir, PATH_DELIM,
	  (int) vparams->mid_time.year,
	  (int) vparams->mid_time.month,
	  (int) vparams->mid_time.day);
  
  sprintf(remote_file_path, "%s%s%s",
	  remote_dir_path, PATH_DELIM, file_name);
      
  now = time(NULL);
  sprintf(tmp_file_path, "%s%s%s.%ld",
	  local_tmp_dir, PATH_DELIM, TMP_DOB_FILE_NAME, now);

  sprintf(remote_tmp_file_path, "%s%s%s.%ld",
	  output_dir, PATH_DELIM, TMP_DOB_FILE_NAME, now);

  /*
   * create directory, if needed
   */

  sprintf(call_str, "rsh -n %s mkdir -p %s",
	  output_host, remote_dir_path);
  if (debug) {
    fprintf(stderr, "%s\n", call_str);
  }
  usystem_call(call_str);
  
  /*
   * write tmp file
   */
  
  v_handle->vol_file_path = tmp_file_path;
  
  if (RfWriteVolume(v_handle, calling_sequence) != R_SUCCESS) {
    return(R_FAILURE);
  }
  
  if (debug) {
    fprintf(stderr, "Writing tmp file %s\n", tmp_file_path);
  }
  
  /*
   * copy to tmp area on remote host
   */
  
  sprintf(call_str, "rcp %s %s:%s",
	  tmp_file_path,
	  output_host, remote_tmp_file_path);
  
  if (debug) {
    fprintf(stderr, "%s\n", call_str);
  }
  usystem_call(call_str);
  
  /*
   * move to correct location on remote host
   */
  
  sprintf(call_str, "rsh %s mv %s %s",
	  output_host, remote_tmp_file_path, remote_file_path);
  
  if (debug) {
    fprintf(stderr, "%s\n", call_str);
  }
  usystem_call(call_str);
  
  /*
   * delete tmp file locally
   */
  
  if (unlink(tmp_file_path)) {
    fprintf(stderr, "WARNING - %s\n", calling_sequence);
    fprintf(stderr, "Removing %s\n", tmp_file_path);
    perror(tmp_file_path);
  }
  
  if (write_current_index) {

    LDATA_handle_t ldata;
    date_time_t dtime;

    LDATA_init_handle(&ldata, prog_name, debug);
	
    Rfrtime2dtime(&vparams->mid_time, &dtime);

    if (LDATA_info_write(&ldata,
			 output_dir,
			 dtime.unix_time,
			 output_file_ext,
			 NULL, NULL, 0, NULL)) {
      LDATA_free_handle(&ldata);
      return (R_FAILURE);
    }
    
    /*
     * rcp index file to remote host
     */
    
    sprintf(call_str, "rcp %s %s:%s",
	    ldata.file_path,
	    output_host, output_dir);
    if (debug) {
      fprintf(stderr, "%s\n", call_str);
    }
    usystem_call(call_str);
    
    LDATA_free_handle(&ldata);

  } /* if (write_current_index) */

  return (R_SUCCESS);

}

#undef THIS_ROUTINE

