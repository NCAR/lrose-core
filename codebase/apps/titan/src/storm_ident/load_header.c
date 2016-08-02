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
 * load_header.c
 *
 * Loads up the storm file header data
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * March 1991
 *
 ****************************************************************************/

#include "storm_ident.h"

void load_header(storm_file_header_t *file_header)

{
  
  /*
   * initialize file header structure
   */

  memset ((void *)  file_header,
          (int) 0, (size_t) sizeof(storm_file_header_t));

  /*
   * fill in parts of file header
   */

  file_header->params.low_dbz_threshold =
    Glob->params.low_dbz_threshold;

  file_header->params.high_dbz_threshold =
    Glob->params.high_dbz_threshold;

  file_header->params.hail_dbz_threshold =
    Glob->params.hail_dbz_threshold;

  file_header->params.dbz_hist_interval =
    Glob->params.dbz_hist_interval;

  file_header->params.base_threshold =
    Glob->params.base_threshold;

  file_header->params.top_threshold =
    Glob->params.top_threshold;

  file_header->params.min_storm_size =
    Glob->params.min_storm_size;
 
  file_header->params.check_morphology = Glob->params.check_morphology;

  file_header->params.morphology_erosion_threshold =
    Glob->params.morphology_erosion_threshold;

  file_header->params.morphology_refl_divisor =
    Glob->params.morphology_refl_divisor;

  file_header->params.check_tops = Glob->params.check_tops;

  file_header->params.min_radar_tops =
    Glob->params.min_radar_tops;

  file_header->params.tops_edge_margin =
    Glob->params.tops_edge_margin;

  file_header->params.max_storm_size =
    Glob->params.max_storm_size;
  
  file_header->params.z_p_coeff =
    Glob->params.ZR.coeff;
  
  file_header->params.z_p_exponent =
    Glob->params.ZR.expon;
  
  file_header->params.z_m_coeff =
    Glob->params.ZM.coeff;
  
  file_header->params.z_m_exponent =
    Glob->params.ZM.expon;

  file_header->params.sectrip_vert_aspect =
    Glob->params.sectrip_vert_aspect;

  file_header->params.sectrip_horiz_aspect =
    Glob->params.sectrip_horiz_aspect;

  file_header->params.sectrip_orientation_error =
    Glob->params.sectrip_orientation_error;

  file_header->params.n_poly_sides = N_POLY_SIDES;

  file_header->params.poly_start_az = 0.0;

  file_header->params.poly_delta_az = 360.0 / (double) N_POLY_SIDES;

  file_header->n_scans = 0;

  file_header->params.vel_available = Glob->params.vel_available;

}
