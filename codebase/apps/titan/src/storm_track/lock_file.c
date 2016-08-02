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
/***************************************************************************
 * lock_file.c
 *
 * Handles the lock file.
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * June 1998
 *
 ****************************************************************************/

#include "storm_track.h"
#include <toolsa/file_io.h>

static char Lock_file_path[MAX_PATH_LEN];
static FILE *Lock_fd = NULL;

/*******************************************************************
 * create_lock_file()
 *
 * Create a lock file in the storm_data_dir. If we cannot create this
 * file there is another instance of the program running, so we cannot
 * proceed.
 */

int create_lock_file(char *storm_data_dir)

{

  /*
   * compute the lock file path
   */
  
  sprintf(Lock_file_path, "%s%s%s.%s",
	  storm_data_dir, PATH_DELIM, Glob->prog_name, "lock");
  
  Lock_fd = ta_create_lock_file(Lock_file_path);
  
  if (Lock_fd == NULL) {
    fprintf(stderr, "ERROR - cannot create lock file '%s'\n",
	    Lock_file_path);
    fprintf(stderr, "%s already running on this directory\n",
	    Glob->prog_name);
    return (-1);
  }

  return (0);

}

/*******************************************************************
 * remove_lock_file()
 *
 * Remove the lock file.
 */

void remove_lock_file(void)

{
  if (Lock_fd != NULL) {
    ta_remove_lock_file(Lock_file_path, Lock_fd);
  }
}
