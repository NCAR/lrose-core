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
 * write_tmp_log.c
 * 
 * Writes a tmp log file, which is then removed after
 * a successful transfer.
 *
 * Returns pointer to log file, NULL if not successful.
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80303, USA
 *
 * November 1996
 */

#include "dist_cdata.h"
 
char *write_tmp_log(void)


{
  static char log_file_path[MAX_PATH_LEN];
  char *cur_file_name;
  FILE *log;

  /*
   * get short file name
   */
  
  if ((cur_file_name = strrchr(gd.cur_file, '/')) == NULL) {
    cur_file_name = gd.cur_file;
  } else {
    cur_file_name++;
  }
      
  /*
   * compute log file name
   */
  
  sprintf(log_file_path, "%s%sdist_cdata.%d.log",
	  gd.source_dir, PATH_DELIM,
	  (int) getpid());

  if ((log = fopen(log_file_path, "w")) == NULL) {
    fprintf(stderr, "WARNING - dist_cdata\n");
    fprintf(stderr, "Cannot open tmp log file\n");
    perror(log_file_path);
    return (NULL);
  }

  fprintf(log, "Sending %s to %s\n",
	  gd.cur_file, gd.cur_host);

  fclose(log);

  if (gd.debug) {
    fprintf(stderr, "Wrote log file %s\n", log_file_path);
  }

  return (log_file_path);
}
