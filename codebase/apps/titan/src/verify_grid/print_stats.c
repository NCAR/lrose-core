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
/*****************************************************************************
 * print_stats.c
 *
 * printout of statistics
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * January 1992
 *
 *****************************************************************************/

#include "verify_grid.h"

void print_stats(statistics_t *stats,
		 FILE *fout)

{

  si32 i;
  double var;
  double lower, upper;

  stats->mean = stats->sumx / stats->n_total;

  var = ((stats->n_total / (stats->n_total - 1.0)) *
	 ((stats->sum2x / stats->n_total) -
	  pow(stats->sumx / stats->n_total, 2.0)));

  if (var < 0.0)
    stats->sd = 0.0;
  else
    stats->sd = sqrt(var);

  for (i = 0; i < stats->hist_n_intervals; i++)
    stats->percent_per_interval[i] =
      100.0 * (double) stats->n_per_interval[i] / (double) stats->n_total;
		   
  fprintf(fout, "mean : %g\n", stats->mean);
  fprintf(fout, "sd   : %g\n", stats->sd);

  fprintf(fout, "\n");

  fprintf(fout, "%10s %10s %10s %10s\n\n",
	  "lower", "upper", "n", "%");

  for (i = 0; i < stats->hist_n_intervals; i++) {
    
    lower = stats->hist_low_limit + i * stats->hist_interval_size;
    upper = lower + stats->hist_interval_size;
    
    fprintf(fout, "%10g %10g %10g %10g\n",
	    lower, upper,
	    stats->n_per_interval[i],
	    stats->percent_per_interval[i]);
    
  } /* i */

}
