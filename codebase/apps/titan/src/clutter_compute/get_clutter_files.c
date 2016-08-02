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
/******************************************************************************
 * get_clutter_files.c
 *
 * gets the list of file names to be used in the clutter computations
 *
 * Mike Dixon RAP, NCAR, Boulder, Colorado, November 1990
 *
 *****************************************************************************/

#include "clutter_compute.h"
#include <dirent.h>

void get_clutter_files(si32 *nfiles,
		       char ***clutter_file_names)

{

  DIR *dirp;
  struct dirent	*dp;
  char ext[16];
  char **file_names;
  int hour, min, sec;
  int i;

  /*
   * open directory file for reading
   */

  if ((dirp = opendir (Glob->clutter_dir)) == NULL) {
    fprintf(stderr, "ERROR - clutter_compute:get_clutter_files\n");
    fprintf(stderr, "Cannot open clutter directory for listing.\n");
    perror (Glob->clutter_dir);
    exit(1);
  }

  /*
   * read through the directory to get the number of files
   */

  *nfiles = 0;

  for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp)) {

    if (dp != NULL)
      (*nfiles)++;

  }

  /*
   * rewind the directory stream
   */

  closedir(dirp);
  if ((dirp = opendir (Glob->clutter_dir)) == NULL) {
    fprintf(stderr, "ERROR - clutter_compute:get_clutter_files\n");
    fprintf(stderr, "Cannot open clutter directory for listing.\n");
    perror (Glob->clutter_dir);
    exit(1);
  }

  /*
   * read through the directory again and set up the file names
   */

  file_names = (char **)
    umalloc ((ui32) (*nfiles * sizeof(char *)));

  *clutter_file_names = file_names;

  for (i = 0; i < *nfiles; i++) {

    dp = readdir (dirp);

    /*
     * parse the file name into parts
     */

    if (dp->d_name[0] == '.') {

      /*
       * exclude dir entries and files beginning with .
       */

      i--;
      (*nfiles)--;

    } else if ((sscanf(dp->d_name, "%2d%2d%2d.%s",
		       &hour, &min, &sec, ext) != 4) ||
	       (hour < 0 || hour > 23 || min < 0 || min > 59 ||
		sec < 0 || sec > 59)) {
      
      /*
       * exclude file names not in the correct format
       */
      
      i--;
      (*nfiles)--;

    } else {

      file_names[i] = (char *)
	umalloc((ui32) (strlen(Glob->clutter_dir) +
			 strlen(PATH_DELIM) +
			 strlen(dp->d_name) + 1));

      sprintf(file_names[i], "%s%s%s",
	      Glob->clutter_dir, PATH_DELIM, dp->d_name);

      fprintf(stdout, "Using file '%s'\n", file_names[i]);

    }

  } /* i */

  /*
   * close the directory file
   */

  closedir(dirp);

}
