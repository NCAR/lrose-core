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
/************************************************************************
 * print_header.c
 *
 * Prints the header line
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * August 1993
 *
 ************************************************************************/

#include "verify_grid.h"

void print_header(si32 n_files,
		  char **file_paths,
		  FILE *fout)

{

  si32 i;

  date_time_t file_time;

  ulocaltime(&file_time);

  fprintf(fout, "#File create time : %s\n", utimestr(&file_time));

  fprintf(fout, "#File name(s) :\n");
  
  for (i = 0; i < n_files; i++)
    fprintf(fout, "#    %s\n", file_paths[i]);

  fprintf (fout, "#Truth_data_dir : %s\n", Glob->params.truth_data_dir);

  fprintf (fout, "#Time_lag : %ld\n", Glob->params.time_lag);
  fprintf (fout, "#Time_margin : %ld\n", Glob->params.time_margin);
  fprintf (fout, "#Min_regression_val : %g\n", Glob->params.min_regression_val);

  if (Glob->params.mode == REGRESSION) {
    fprintf (fout, "#Grid params : nx, ny : %ld, %ld\n",
	     Glob->params.grid.nx, Glob->params.grid.ny);
    fprintf (fout, "#              minx, miny : %g, %g\n",
	     Glob->params.grid.minx, Glob->params.grid.miny);
    fprintf (fout, "#              dx, dy : %g, %g\n",
	     Glob->params.grid.dx, Glob->params.grid.dy);
  }

  fprintf (fout, "#labels: %s,%s,%s,%s\n",
	   "Truth_time", "Detect_time",
	   Glob->params.truth_label, Glob->params.detect_label);
  
}
