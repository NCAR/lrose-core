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
 * setup_xgraph.c
 *
 * Sets up output for xgraph - stdout
 *
 * Mike Dixon RAP NCAR August 1992
 *
 *********************************************************************/

#include "verify_grid.h"

void setup_xgraph(void)

{
  
  char line[BUFSIZ];
  FILE *prologue_file;
  
  /*
   * open xgraph prologue file
   */
  
  if ((prologue_file =
       fopen(Glob->xgraph_prologue_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:setup_xgraph.\n", Glob->prog_name);
    fprintf(stderr, "Opening xgraph prologue file.\n");
    perror(Glob->xgraph_prologue_path);
    tidy_and_exit(-1);
  }
  
  /*
   * copy contents of prologue file to stdout and
   * close prologue file
   */
  
  while (!feof(prologue_file)){
    if (fgets(line, BUFSIZ, prologue_file) != NULL)
      fputs(line, stdout);
  }

  fclose(prologue_file);

  /*
   * get ready for data set
   */

  fprintf(stdout, "\n\n\"\"\n");

}
