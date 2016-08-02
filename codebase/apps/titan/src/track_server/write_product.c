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
 * write_product.c
 *
 * Writes product track data set to client
 *
 * returns 0 on success, -1 on failure
 *
 * RAP, NCAR, Boulder CO
 *
 * Oct 1994
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "track_server.h"
#include <toolsa/sockutil.h>

#define CONSTRAIN(x, low, high) if ((x) < (low)) (x) = (low); \
                                else if ((x) > (high)) (x) = (high)

/*
 * prototypes
 */

static ui08 *allocate_output_buffer(int message_len);
     
/*
 * main routine
 */

int write_product(int sockfd,
		  storm_file_handle_t *s_handle,
		  track_file_handle_t *t_handle,
		  si32 dtime)
     
{
  
  static si32 seq_no = 0;
  static si32 prev_dtime = 0;

  ui08 *output_buffer;
  ui08 *buf_ptr;
  
  int plot_ellipse;
  int plot_polygon;
  int ellipse_included;
  int polygon_included;

  int n_entries, n_scans;
  int n_sent;
  int scan_num;
  int ientry;
  int n_poly_sides;
  int polygon_product_size;
  int message_len;
  int buffer_len;
  
  double lead_time_hr;
  double min_storm_size;
  
  track_file_entry_t *file_entry;
  storm_file_params_t *sparams;
  storm_file_scan_header_t *scan;
  storm_file_global_props_t *gprops;
  track_file_params_t *tparams;
  track_file_forecast_props_t *fprops;

  titan_grid_comps_t grid_comps;
  
  sparams = &s_handle->header->params;
  tparams = &t_handle->header->params;

  /*
   * unless requested, do not send out data which is
   * older than previosly sent out
   */

  if (!Glob->params.product_mode_resend_old_data) {

    if (dtime < prev_dtime) {
      return (0);
    }

  } /* if (!Glob->params.product_mode_resend_old_data) */

  prev_dtime = dtime;

  /*
   * determine which scan is closest to the request time
   */

  n_scans = s_handle->header->n_scans;
  scan_num = n_scans - 1;

  /*
   * read in track file scan entries
   */

  if (RfReadTrackScanEntries(t_handle,
			     scan_num,
			     "write_product") != R_SUCCESS)
    return (-1);
  
  /*
   * read in storm file scan
   */

  if (RfReadStormScan(s_handle,
		      scan_num,
		      "write_product") != R_SUCCESS)
    return (-1);

  /*
   * initialize projection
   */
  
  TITAN_init_proj(&s_handle->scan->grid, &grid_comps);
  
  /*
   * initialize
   */

  lead_time_hr = Glob->params.product_mode_forecast_lead_time / 3600.0;

  scan = s_handle->scan;

  min_storm_size = sparams->min_storm_size;
  n_poly_sides = sparams->n_poly_sides;

  n_entries = t_handle->scan_index[scan_num].n_entries;

  plot_ellipse = Glob->params.product_mode_plot_ellipse;
  plot_polygon = Glob->params.product_mode_plot_polygon;

  ellipse_included = FALSE;
  polygon_included = FALSE;

  if (plot_ellipse) {
    ellipse_included = TRUE;
  }
  
  if (plot_polygon) {
    if (!ellipse_included) {
      ellipse_included = TRUE;
    }
    polygon_included = TRUE;
  }

  /*
   * initialize product module
   */
  
  tdata_prod_init(Glob->params.product_mode_zero_growth_forecast,
		  Glob->params.product_mode_send_invalid_forecasts,
		  Glob->params.product_mode_forecast_lead_time,
		  Glob->params.product_mode_plot_current,
		  Glob->params.product_mode_plot_forecast,
		  Glob->params.product_mode_plot_trend,
		  Glob->params.product_mode_speed_knots,
		  Glob->params.product_mode_speed_round,
		  Glob->params.product_mode_fixed_length_arrows);

  /*
   * allocate buffer
   */
  
  buffer_len = tdata_prod_buffer_len(plot_ellipse, plot_polygon,
				     n_poly_sides, n_entries);
  
  output_buffer = allocate_output_buffer(buffer_len);
  polygon_product_size = sizeof(si32) + n_poly_sides * sizeof(ui08);
  /*
   * loop through the entries
   */
  
  n_sent = 0;
  file_entry = t_handle->scan_entries;
  buf_ptr = output_buffer + sizeof(tdata_product_header_t);
  
  for (ientry = 0; ientry < n_entries; ientry++, file_entry++) {

    fprops = &file_entry->dval_dt;

    gprops = s_handle->gprops + file_entry->storm_num;

    /*
     * load up product entry struct
     */
    
    if (tdata_prod_load_entry((tdata_product_entry_t *) buf_ptr,
			      file_entry,
			      gprops,
			      fprops,
			      &grid_comps,
			      lead_time_hr,
			      min_storm_size)) {
      
      /*
       * entry is valid
       */

      n_sent++;
      buf_ptr += sizeof(tdata_product_entry_t);

      if (ellipse_included) {

	tdata_prod_load_ellipse((tdata_product_ellipse_t *) buf_ptr,
				gprops, fprops, &grid_comps);

	buf_ptr += sizeof(tdata_product_ellipse_t);

      }
      
      if (polygon_included) {
	
	tdata_prod_load_polygon((tdata_product_polygon_t *) buf_ptr,
				gprops, n_poly_sides);

	buf_ptr += polygon_product_size;
	
      }
      
    }
    
  } /* ientry */
  
  /*
   * set data header
   */
  
  tdata_prod_load_header((tdata_product_header_t *) output_buffer,
			 sparams,
			 &scan->grid,
			 t_handle,
			 dtime,
			 n_sent,
			 plot_ellipse,
			 plot_polygon,
			 ellipse_included,
			 polygon_included,
			 n_poly_sides);

  /*
   * write message to socket - only sending the valid entries
   */
  
  seq_no++;

  message_len = tdata_prod_buffer_len(plot_ellipse, plot_polygon,
				      n_poly_sides, n_sent);
  
  if(SKU_writeh(sockfd,
		output_buffer,
		message_len,
		Glob->params.product_mode_product_id,
		seq_no) < 0)
    return(-1);

  return (0);
  
}

/*****************************************************************
 * allocate_output_buffer()
 */

static ui08 *allocate_output_buffer(int message_len)
     
{

  static ui08 *buffer = NULL;
  static int size_allocated = 0;
  
  /*
   * allocate array for entries
   */
  
  if (size_allocated < message_len) {

    if (buffer == NULL) {
      buffer = (ui08 *) umalloc ((ui32) message_len);
    } else {
      buffer = (ui08 *)
	urealloc ((char *) buffer,(ui32) message_len);
    } /* if (buffer == NULL) */

    size_allocated = message_len;
    
  } /* if (size_allocated < message_len) */
  
  return (buffer);

}
  
