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
/*******************************************************************************
 * load_runs_truth_grid.c
 *
 * loads the truth grid based on the runs for the storm at the forecast time
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * January 1992
 *
 *******************************************************************************/

#include "verify_tracks.h"

void load_runs_truth_grid(vt_storm_t *storm, ui08 **grid)
{

  si32 i, ix, irun;
  si32 lx, ly;
  si32 *x_lookup, *y_lookup;

  storm_file_run_t *run;

  x_lookup = storm->x_lookup;
  y_lookup = storm->y_lookup;

  /*
   * loop through runs
   */

  run = storm->runs;
  
  for (irun = 0; irun < storm->gprops.n_runs; irun++) {

    ly = y_lookup[run->iy];

    if (ly >= 0) {

      ix = run->ix;

      for (i = 0; i < run->n; i++) {

	lx = x_lookup[ix];
      
	if (lx >= 0)
	  grid[ly][lx] = TRUE;

	ix++;

      } /* i */

    } /* if (ly >= 0) */
    
    run++;

  } /* irun */

}
