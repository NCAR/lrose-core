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
/***********************************************************************
 * beam_file.c
 *
 * Routines to handle the beam files.
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * November 1996
 *
 ************************************************************************/

#include "dva_ingest.h"
#include <toolsa/ldata_info.h>

static FILE *Beam_file = NULL;
static int File_num = 0;
static char File_path[MAX_PATH_LEN];

/***************************
 * writes a beam to the file
 *
 * Returns 0 on success, -1 on failure.
 */

int write_beam(bprp_beam_t *beam)

{

  PMU_auto_register("In write_beam");

  if (Glob->params.debug >= DEBUG_VERBOSE) {
    fprintf(stderr, "Writing beam to file file %s\n", File_path);
  }

  if (Beam_file != NULL) {
    if (fwrite(beam, sizeof(bprp_beam_t), 1, Beam_file) != 1) {
      fprintf(stderr, "ERROR - %s:write_beam\n", Glob->prog_name);
      fprintf(stderr, "Cannot write beam to file\n");
      perror(File_path);
      return (-1);
    } else {
      return(0);
    }
  } else {
    return (0);
  }

}

/*****************
 * new_beam_file()
 *
 * Open new beam file. If file is already open, close existing file
 * and write file index.
 *
 * Returns 0 on success, -1 on failure.
 */

int new_beam_file(void)

{

  /*
   * if file is already open, close it and write index
   */

  if (Beam_file != NULL) {
    if (close_beam_file()) {
      return (-1);
    }
  }
  
  sprintf(File_path, "%s%s%s.%d", Glob->params.beam_dir, PATH_DELIM,
	  Glob->params.beam_file_name, File_num);
  
  if ((Beam_file = fopen(File_path, "w")) == NULL) {
    fprintf(stderr, "ERROR - %s:new_file\n", Glob->prog_name);
    fprintf(stderr, "Cannot open new file for beams\n");
    perror(File_path);
    return(-1);
  }

  if (Glob->params.debug) {
    fprintf(stderr, "Opening beam file %s\n", File_path);
  }

  return (0);

}

/*******************
 * close_beam_file()
 *
 * Close beam file and write index.
 *
 * Returns 0 on success, -1 on failure.
 */

int close_beam_file(void)

{

  static int first_call = TRUE;
  static LDATA_handle_t ldata;

  char ext[32];

  if (first_call) {
    LDATA_init_handle(&ldata,
		      Glob->prog_name,
		      (Glob->params.debug > DEBUG_NORM));
    first_call = FALSE;
  }
  
  /*
   * close file
   */

  fclose(Beam_file);
  Beam_file = NULL;

  if (Glob->params.debug) {
    fprintf(stderr, "Closing beam file %s\n", File_path);
  }

  /*
   * load file extension
   */
  
  sprintf(ext, "%d", File_num);

  /*
   * change File_num
   */

  File_num = (File_num + 1) % Glob->params.n_beam_files;

  /*
   * write latest data info
   */

  if (LDATA_info_write(&ldata,
		       Glob->params.beam_dir,
		       time(NULL), ext,
		       Glob->params.beam_file_name, NULL,
		       0, NULL)) {
    
    fprintf(stderr, "WARNING - %s:close_beam_file\n",
	    Glob->prog_name);
    fprintf(stderr, "Cannot write index file to dir %s\n",
	    Glob->params.beam_dir);

    return (-1);

  }

  return (0);
  
}
