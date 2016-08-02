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

#include <titan/file_io.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * Rf_file_uncompress()
 *
 * Uncompresses the file if it is compressed
 *
 * Returns 1 if uncompression done, 0 if not, -1 if error
 */

int Rf_file_uncompress(char *file_path)
     
{

  char *ext;
  char compressed_path[MAX_PATH_LEN];
  char call_str[BUFSIZ];
  struct stat file_stat;

  /*
   * if file name indicates that it is compressed,
   * uncompress it
   */

  ext = file_path + strlen(file_path) - 2;

  if (!strncmp(ext, ".Z", 2)) {

    /*
     * alter file path to remove extension
     */

    *ext = '\0';

    sprintf(call_str, "uncompress %s", file_path);
    errno = 0;
    int iret = system (call_str);
	
    if (errno) {

      fprintf(stderr, "WARNING - could not uncompress file, uncompress iret: %d\n", iret);
      perror(call_str);
      return (-1);

    } else {

      return (1);
      
    } /* if (errno) */

  }

  /*
   * if file name indicates that it is gzipped,
   * gunzip it
   */

  ext = file_path + strlen(file_path) - 3;

  if (!strncmp(ext, ".gz", 3)) {

    /*
     * alter file path to remove extension
     */

    *ext = '\0';
    
    sprintf(call_str, "gunzip %s", file_path);
    errno = 0;
    int iret = system (call_str);
	
    if (errno) {

      fprintf(stderr, "WARNING - could not gunzip file, gunzip iret: %d\n", iret);
      perror(call_str);
      return (-1);
      
    } else {
      
      return (1);
      
    } /* if (errno) */
    
  }

  /*
   * check if the compressed file exists.
   * If so, uncompress it.
   */

  sprintf(compressed_path, "%s.Z", file_path);

  if (!stat(compressed_path, &file_stat)) {
    
    /*
     * uncompress file
     */
    
    sprintf(call_str, "uncompress %s", compressed_path);
    errno = 0;
    int iret = system (call_str);
    
    if (errno) {
      
      fprintf(stderr, "WARNING - could not uncompress file, uncompress iret: %d\n", iret);
      perror(call_str);
      return (-1);
      
    } else {
      
      return (1);
      
    }

  }

  /*
   * check if the gzipped file exists.
   * If so, gunzip it.
   */

  sprintf(compressed_path, "%s.gz", file_path);
  
  if (!stat(compressed_path, &file_stat)) {
    
    /*
     * gunzip file
     */
    
    sprintf(call_str, "gunzip %s", compressed_path);
    errno = 0;
    int iret = system (call_str);
    
    if (errno) {
      
      fprintf(stderr, "WARNING - could not gunzip file, iret: %d\n", iret);
      perror(call_str);
      return (-1);
      
    } else {
      
      return (1);
      
    }
    
  }

  return (0);

}

/*********************************************************
 * Rf_fopen_uncompress()
 *
 * Uncompresses the file if necessary, then opens it
 *
 * Return is identical to fopen()
 */

FILE *Rf_fopen_uncompress(char *filename, const char *type)
     
{
  
  if (Rf_file_uncompress(filename) == -1) {
    return ((FILE *) NULL);
  }

  return (fopen(filename, type));

}

/*********************************************************
 * Rf_stat_uncompress()
 *
 * stats a file in uncompressed or compressed state.
 *
 * Return is identical to stat()
 */

int Rf_stat_uncompress(const char *path, struct stat *buf)
     
{

  char compressed_path[MAX_PATH_LEN];
  int iret;

  /*
   * if standard stat works, return results
   */
  
  iret = stat(path, buf);

  if (iret == 0) {
    return (iret);
  }
  
  /*
   * check if the compressed file exists.
   * If so, stat it
   */
  
  sprintf(compressed_path, "%s.Z", path);
  iret = stat(compressed_path, buf);

  if (iret == 0) {
    return (iret);
  }
  
  sprintf(compressed_path, "%s.gz", path);
  iret = stat(compressed_path, buf);

  if (iret == 0) {
    return (iret);
  }
  
  /*
   * otherwise return stat of original path
   */

  return (stat(path, buf));

}

