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

#include "rctable_generate.h"

void setup_scan(scan_table_t *scan_table)

{

  int i;
  char *resource_str;
  char *elev_target_string;
  char *start_pt, *end_pt;
  scan_table_elev_t *elev;

  /*
   * use scan table?
   */

  resource_str = uGetParamString(Glob->prog_name,
                                 "use_azimuth_table", USE_AZIMUTH_TABLE);
  
  if (uset_true_false_param(Glob->prog_name,
                            "setup_scan",
                            Glob->params_path_name,
                            resource_str,
                            &Glob->use_azimuth_table,
                            "use_azimuth_table"))
    exit(-1);

  scan_table->use_azimuth_table = Glob->use_azimuth_table;
  
  /*
   * get decision on whether to limit the vertical extent or not
   *
   * If extend_below is set, the bottom beam is used to fill the
   * region below the sampled volume.
   *
   */

  resource_str = uGetParamString(Glob->prog_name,
                                 "extend_below", EXTEND_BELOW);
  
  if (uset_true_false_param(Glob->prog_name,
                            "setup_scan",
                            Glob->params_path_name,
                            resource_str,
                            &scan_table->extend_below,
                            "extend_below"))
    exit(-1);

  /*
   * basic params 
   */
  
  scan_table->ngates =
    uGetParamLong(Glob->prog_name, "ngates", NGATES);
  
  scan_table->gate_spacing = uGetParamDouble(Glob->prog_name,
					"gate_spacing", GATE_SPACING);
  
  scan_table->start_range = uGetParamDouble(Glob->prog_name,
				       "start_range", START_RANGE);
  
  scan_table->beam_width = uGetParamDouble(Glob->prog_name,
				      "beam_width", BEAM_WIDTH);
  
  if (!Glob->use_azimuth_table) {
    
    scan_table->nazimuths =
      uGetParamLong(Glob->prog_name, "nazimuths", NAZIMUTHS);
    
    scan_table->delta_azimuth = uGetParamDouble(Glob->prog_name,
						"delta_azimuth",
						DELTA_AZIMUTH);
    
    scan_table->start_azimuth = uGetParamDouble(Glob->prog_name,
					      "start_azimuth",
						START_AZIMUTH);
    
    /*
     * if not using scan table, read in azimuth details and
     * elevation angles
     */
  
    scan_table->nelevations =
      uGetParamLong(Glob->prog_name, "nelevations", NELEVATIONS);
    
    /*
     * decode elevation string and load up elevations
     */
    
    elev_target_string = uGetParamString(Glob->prog_name,
					 "elev_target", ELEV_TARGET);
    
    scan_table->elevs = (scan_table_elev_t *) umalloc
      (scan_table->nelevations * sizeof(scan_table_elev_t));
  
    scan_table->elev_angles = (double *)
      umalloc((ui32) (scan_table->nelevations * sizeof(double)));

    for (i = 0; i < scan_table->nelevations; i++) {
      
      if (i == 0)
	start_pt = strtok(elev_target_string, "\'\" ");
      else
	start_pt = strtok((char *) NULL, "\'\" ");

      if (start_pt == NULL) {
	fprintf(stderr, "ERROR - %s:setup_scan.\n", Glob->prog_name);
	fprintf(stderr, "Decoding elevation angles\n");
	fprintf(stderr, "Edit params file '%s'\n", Glob->params_path_name);
	exit(-1);
      }
      
      errno = 0;
      scan_table->elev_angles[i] = strtod(start_pt, &end_pt);
      
      if (errno != 0) {
	fprintf(stderr, "ERROR - %s:setup_scan.\n", Glob->prog_name);
	fprintf(stderr, "Decoding string '%s'\n", elev_target_string);
	fprintf(stderr, "Edit params file '%s'\n", Glob->params_path_name);
	perror(start_pt);
	exit(-1);
      }

    } /* i */

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
    
    if (RfReadScanTable(scan_table,
			Glob->azimuth_table_file_path,
			"setup_scan") != R_SUCCESS) {
      exit(-1);
    }

    /*
     * compute the azimuth limits
     */
    
    RfComputeScanTableAzLimits(scan_table);

    /*
     * load up extended elevs array
     */

    RfLoadScanTableExtElevs(scan_table);

  } /* if (!Glob->use_azimuth_table) */

  /*
   * compute missing data index - one past the last index in the 
   * radar data array
   */
  
  scan_table->missing_data_index =
    scan_table->nbeams_vol * scan_table->ngates;

  /*
   * set up extra elevation arrays
   */
  
  scan_table->ext_elev_angles = (double *) umalloc
    ((scan_table->nelevations + 2) * sizeof(double));
  
  scan_table->elev_limits = (double *) umalloc
    ((scan_table->nelevations + 1) * sizeof(double));
  
  /*
   * compute elev limits
   */
  
  RfComputeScanTableElevLimits(scan_table);

  /*
   * compute extrended elev array
   */
  
  RfComputeScanTableExtElev(scan_table);
    
  if (Glob->debug && Glob->use_azimuth_table) {
    RfPrintScanTable(stderr, "  ", scan_table);
  }

}








