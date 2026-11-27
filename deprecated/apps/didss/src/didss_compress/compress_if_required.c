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

#include "didss_compress.h"

/*****************************************************************************
 * compress_if_required() - compress files as applicable
 *
 * Returns 1 if compression performed, 0 otherwise
 */

int compress_if_required(char *file_path)
     
{
  
  char *last2, *last3, *last2_full_file, *last3_full_file;
  char call_str[BUFSIZ], full_file[BUFSIZ];
  struct stat file_stat;

  last2 = file_path + strlen(file_path) - 2;
  strcpy(full_file, file_path);
  
  if (!strcmp(last2, ".Z")) {

    last2_full_file = full_file + strlen(full_file) - 2;
    *last2_full_file = '\0';

    if (!Glob->gzip) {
      /*
       * Case: Want a compressed file and have one.
       * If the uncompressed file exists, compress it.
       */
      if (!stat(full_file, &file_stat)) {
        sprintf(call_str, "compress -f %s", full_file);

        if (Glob->verbose) {
          fprintf(stderr, "File %s not compressed\n", full_file);
          fprintf(stderr, "Running '%s'\n", call_str);
        } /* if (Glob->verbose) */
  
	if (Glob->procmap_instance != NULL)
	  PMU_force_register("Compressing file");

        errno = 0;
        system (call_str);
	
        if (errno) {
          if (Glob->verbose) {
            fprintf(stderr,
	            "WARNING - %s:compress_if_required\n",
	            Glob->prog_name);
            perror("Could not compress file");
          } /* if (Glob->verbose) */
          return (0);
    
        } else {
          return (1);
        } /* if (errno) */

      } else {
        return (0);
      } /* if (!stat(full_file, &file_stat)) */

    } else {
      /*
       * Case: Have a compressed file and want a gzip file.
       * Uncompress the file only if the uncompressed file 
       * doesn't already exist.
       */
      if (stat(full_file, &file_stat)) {

        sprintf(call_str, "uncompress %s", file_path);
        if (Glob->verbose) {
	  fprintf(stderr,
		  "File compressed with compress instead of gzip\n");
	  fprintf(stderr, "Running '%s'\n", call_str);
        }

	if (Glob->procmap_instance != NULL)
	  PMU_force_register("Uncompressing file");

	errno = 0;
        system (call_str);

	if (errno) {
	  if (Glob->verbose) {
            fprintf(stderr,
	            "WARNING - %s:compress_if_required\n",
	            Glob->prog_name);
            perror(call_str);
	  }
	  return (0);
	} /* if (errno) */

      } else {
	if (Glob->verbose) {
	  fprintf(stderr, "Uncompressed file already exists.\n");
	  fprintf(stderr, "Not uncompressing the .Z file.\n");
	}
      } /* if (stat(full_file, &file_stat)) */

      *last2 = '\0';
    } /* if (!Glob->gzip) */

  } /* if (!strcmp(last2, ".Z")) */
    
  last3 = file_path + strlen(file_path) - 3;

  if (!strcmp(last3, ".gz")) {

    last3_full_file = full_file + strlen(full_file) - 3;
    *last3_full_file = '\0';

    if (Glob->gzip) {
      /*
       * Case: Want a gzipped file and have one.
       * If the ungzipped file exists, gzip it.
       */
      if (!stat(full_file, &file_stat)) {
        sprintf(call_str, "gzip -f %s", full_file);

        if (Glob->verbose) {
          fprintf(stderr, "File %s not gzipped\n", full_file);
          fprintf(stderr, "Running '%s'\n", call_str);
        } /* if (Glob->verbose) */
  
	if (Glob->procmap_instance != NULL)
	  PMU_force_register("Gzipping file");

        errno = 0;
        system (call_str);
	
        if (errno) {
          if (Glob->verbose) {
            fprintf(stderr,
	            "WARNING - %s:compress_if_required\n",
	            Glob->prog_name);
            perror("Could not gzip file");
          } /* if (Glob->verbose) */
          return (0);
    
        } else {
          return (1);
        } /* if (errno) */

      } else {
        return (0);
      } /* if (!stat(full_file, &file_stat)) */

    } else {
      /*
       * Case: Have a gzipped file and want a compressed file.
       * Ungzip the file only if the ungzipped file 
       * doesn't already exist.
       */
      if (stat(full_file, &file_stat)) {

        sprintf(call_str, "gunzip %s", file_path);
        if (Glob->verbose) {
	  fprintf(stderr, "File compressed with gzip instead of compress\n");
	  fprintf(stderr, "Running '%s'\n", call_str);
        }

	if (Glob->procmap_instance != NULL)
	  PMU_force_register("Gunzipping file");

	errno = 0;
        system (call_str);

	if (errno) {
	  if (Glob->verbose) {
            fprintf(stderr,
	            "WARNING - %s:compress_if_required\n",
	            Glob->prog_name);
            perror(call_str);
	  }
	  return (0);
	} /* if (errno) */

      } else {
	if (Glob->verbose) {
	  fprintf(stderr, "Ungzipped file already exists.\n");
	  fprintf(stderr, "Not unzipping the .gz file.\n");
	}
      } /* if (stat(full_file, &file_stat)) */

      *last3 = '\0';
    } /* if (Glob->gzip) */
    
  } /* if (!strcmp(last3, ".gz")) */
    
  /*
   * file is not compressed
   */

  if (Glob->gzip) {
    sprintf(call_str, "gzip -f %s", file_path);
  } else {
    sprintf(call_str, "compress -f %s", file_path);
  }
  
  if (Glob->verbose) {
    fprintf(stderr, "File %s not compressed\n", file_path);
    fprintf(stderr, "Running '%s'\n", call_str);
  }
  
  if (Glob->procmap_instance != NULL)
    PMU_force_register("Compressing file");

  errno = 0;
  system (call_str);
	
  if (errno) {
  
    if (Glob->verbose) {
      fprintf(stderr,
	      "WARNING - %s:compress_if_required\n",
	      Glob->prog_name);
      perror("Could not compress file");
    }

    return (0);
    
  } else {
    
    return (1);
    
  } /* if (errno) */
	
}

