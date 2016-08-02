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
 * convert_file.c
 *
 * converts file from dobson to mdv
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "dobson2mdv.h"
#include <sys/stat.h>

void convert_file(vol_file_handle_t *v_handle, char *input_file_path)

{

  char output_file_name[MAX_PATH_LEN];
  char output_file_path[MAX_PATH_LEN];
  char dir_path[MAX_PATH_LEN];
  vol_params_t *vparams;
  struct stat dir_stat;

  v_handle->vol_file_path = input_file_path;

  fprintf(stderr, "Converting file %s\n", input_file_path);

  if (RfReadVolume(v_handle, "convert_file") != R_SUCCESS) {
    fprintf(stderr, "ERROR - %s:convert_file\n", Glob->prog_name);
    fprintf(stderr, "Cannot read in file %s\n", input_file_path);
    return;
  }

  /*
   * compute file names
   */

  vparams = v_handle->vol_params;
  
  sprintf(dir_path, "%s%s%.4d%.2d%.2d",
	  Glob->params.output_dir, PATH_DELIM,
	  (int) vparams->mid_time.year,
	  (int) vparams->mid_time.month,
	  (int) vparams->mid_time.day);
  
  sprintf(output_file_name, "%.2d%.2d%.2d.%s",
	  (int) vparams->mid_time.hour,
	  (int) vparams->mid_time.min,
	  (int) vparams->mid_time.sec,
	  Glob->params.output_file_ext);
    
  sprintf(output_file_path, "%s%s%s",
	  dir_path, PATH_DELIM, output_file_name);
      
  /*
   * create directory, if needed
   */

  if (0 != stat(dir_path, &dir_stat)) {
    if (mkdir(dir_path, 0755)) {
      fprintf(stderr, "ERROR - %s:convert_file\n", Glob->prog_name);
      fprintf(stderr, "Trying to make output dir\n");
      perror(dir_path);
      return;
    }
  }

  /*
   * write file
   */
  
  v_handle->vol_file_path = output_file_path;
  
  if (RfWriteVolumeMdv(v_handle, MDV_PLANE_RLE8, "convert_file")
      != R_SUCCESS) {
    fprintf(stderr, "ERROR - %s:convert_file\n", Glob->prog_name);
    fprintf(stderr, "Cannot write out file %s\n", output_file_path);
    return;
  }

  fprintf(stderr, "Wrote file %s\n", output_file_path);

}

