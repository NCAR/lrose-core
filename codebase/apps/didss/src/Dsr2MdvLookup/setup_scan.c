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
 * setup_scan.c
 *
 * Set up the radar scan details
 *
 * RAP, NCAR, Boulder CO
 *
 * September 1995
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Dsr2MdvLookup.h"

void setup_scan(radar_scan_table_t *scan_table)

{

  int i;
  radar_scan_table_elev_t *elev;

  scan_table->use_azimuth_table = Glob->params.use_azimuth_table;
  scan_table->extend_below = Glob->params.extend_below;
  /*
   * basic params 
   */
  
  scan_table->ngates = Glob->params.ngates;
  scan_table->gate_spacing = Glob->params.gate_spacing;
  scan_table->start_range = Glob->params.start_range;
  scan_table->beam_width = Glob->params.beam_width;
  
  if (!Glob->params.use_azimuth_table) {
    
    scan_table->nazimuths = Glob->params.nazimuths;
    scan_table->start_azimuth = Glob->params.start_azimuth;
    scan_table->delta_azimuth = Glob->params.delta_azimuth;

    scan_table->nelevations = Glob->params.elev_target.len;

    scan_table->elevs = (radar_scan_table_elev_t *) umalloc
      (scan_table->nelevations * sizeof(radar_scan_table_elev_t));
  
    scan_table->elev_angles = (fl32 *)
      umalloc(scan_table->nelevations * sizeof(fl32));

    for (i = 0; i < scan_table->nelevations; i++) {
      scan_table->elev_angles[i] = Glob->params.elev_target.val[i];
    }

    scan_table->max_azimuths = scan_table->nazimuths;

    scan_table->nbeams_vol =
      scan_table->nelevations * scan_table->nazimuths;
    
    /*
     * set the start and end beam nums for each elev
     */

    elev = scan_table->elevs;
    for (i = 0; i < scan_table->nelevations; i++, elev++) {
      elev->start_beam_num = i * scan_table->nazimuths;
      elev->end_beam_num = elev->start_beam_num + scan_table->nazimuths - 1;
    }    
    
  } else {
    
    /*
     * read in scan table
     */
    
    if (RadarReadScanTable(scan_table,
			   Glob->params.azimuth_table_path,
			"setup_scan")) {
      tidy_and_exit(-1);
    }

    /*
     * compute the azimuth limits
     */
    
    RadarComputeScanTableAzLimits(scan_table);

    /*
     * load up extended elevs array
     */

    RadarLoadScanTableExtElevs(scan_table);
    
  } /* if (!Glob->params.use_azimuth_table) */

  /*
   * compute missing data index - one past the last index in the 
   * radar data array
   */
  
  scan_table->missing_data_index =
    scan_table->nbeams_vol * scan_table->ngates;

  /*
   * set up extra elevation arrays
   */
  
  scan_table->ext_elev_angles = (fl32 *) umalloc
    ((scan_table->nelevations + 2) * sizeof(fl32));
  
  scan_table->elev_limits = (fl32 *) umalloc
    ((scan_table->nelevations + 1) * sizeof(fl32));
  
  /*
   * compute elev limits
   */
  
  RadarComputeScanTableElevLimits(scan_table);

  /*
   * compute extrended elev array
   */
  
  RadarComputeScanTableExtElev(scan_table);
    
  if (Glob->params.debug && Glob->params.use_azimuth_table) {
    RadarPrintScanTable(stderr, "  ", scan_table);
  }

}








