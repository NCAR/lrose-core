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

/****************************************************************************
 * analyze_sub_tree.c
 *
 * Analyse the forecast performance for a sub-tree
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000
 *
 * July 1995
 *
 ****************************************************************************/

#include "forecast_monitor.h"
#include "sys/types.h"
#include "sys/stat.h"

void analyze_sub_tree(storm_file_handle_t *s_handle,
		      track_file_handle_t *t_handle,
		      vol_file_handle_t *v_handle,
		      si32 verify_scan_num,
		      si32 generate_scan_num,
		      si32 n_scans,
		      double actual_lead_time,
		      date_time_t *scan_times,
		      si32 tag,
		      fm_simple_track_t *stracks,
		      tree_vertex_t *vertices,
		      si32 n_vertices)

{

  si32 ivertex;
  tree_vertex_t *vertex;
  fm_simple_track_t *strack;
  double pod, far, csi;

  /*
   * clear stats grids
   */

  clear_grids();

  /*
   * loop through vertices, looking for forecast and
   * verification entries
   */
  
  vertex = vertices;
  strack = stracks;

  for (ivertex = 0; ivertex < n_vertices; ivertex++, vertex++, strack++) {

    if (vertex->tag == tag) {

      if (strack->have_generate) {
	load_forecast_grid(s_handle, t_handle, strack, actual_lead_time);
      } /* if (strack->have_generate) */

      if (strack->have_verify) {
	load_verify_grid(strack);
      } /* if (strack->have_verify) */
      
    } /* if (vertex->tag == tag) */

  } /* ivertex */

  if (compute_grid_stats(&pod, &far, &csi)) {

    /*
     * load up output for the verify entries
     */

    vertex = vertices;
    strack = stracks;
    for (ivertex = 0; ivertex < n_vertices;
	 ivertex++, vertex++, strack++) {
      if (vertex->tag == tag) {
	if (strack->have_verify) {
	  load_output_entry(s_handle, strack, pod, far, csi);
	} /* if (strack->have_verify) */
      } /* if (vertex->tag == tag) */
    } /* ivertex */

    /*
     * print stats if needed
     */

    if (Glob->params.print_stats) {

      fprintf(stdout, "\n");
      fprintf(stdout, "    Forecast time: %s\n",
	      utimestr(scan_times + generate_scan_num));
      fprintf(stdout, "    Verify   time: %s\n",
	      utimestr(scan_times + verify_scan_num));
      fprintf(stdout, "    TRACK SET: ");
  
      vertex = vertices;
      strack = stracks;
      for (ivertex = 0; ivertex < n_vertices;
	   ivertex++, vertex++, strack++) {
	if (vertex->tag == tag) {
	  if (strack->have_generate) {
	    fprintf(stdout, "F:%ld/%ld ",
		    (long) strack->params.complex_track_num,
		    (long) strack->params.simple_track_num);
	  } /* if (strack->have_generate) */
	  if (strack->have_verify) {
	    fprintf(stdout, "V:%ld/%ld ",
		    (long) strack->params.complex_track_num,
		    (long) strack->params.simple_track_num);
	  } /* if (strack->have_verify) */
	} /* if (vertex->tag == tag) */
      } /* ivertex */

      fprintf(stdout, "\n");

      print_grids();
      print_cont();

    } /* if (Glob->params.print_stats) */

  } /* if (compute_grid_stats()) */

}

