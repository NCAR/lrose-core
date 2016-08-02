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
 * get_ps_color_scales.c
 * 
 * reads a color table file for different variable types, and sets up
 * a color scale struct for each one
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1990   
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "time_hist.h"

#define NSCALES 3
#define DBZ_SCALE 0
#define VORTICITY_SCALE 1
#define PERCENT_SCALE 2

void get_ps_color_scales(void)

{

  char *name[NSCALES];
  char *colorscale_dir;
  char *colorscale_name[NSCALES];
  char colorscale_path_name[BUFSIZ];
  g_color_scale_t *cscale[NSCALES];
  double data_scale[NSCALES], data_bias[NSCALES];

  int iscale;

  Colormap cmap;

  name[DBZ_SCALE] = "dbz";
  name[VORTICITY_SCALE] = "vorticity";
  name[PERCENT_SCALE] = "percent";

  data_scale[DBZ_SCALE] = 1.0;
  data_bias[DBZ_SCALE] = -30.0;

  data_scale[VORTICITY_SCALE] = 20.0;
  data_bias[VORTICITY_SCALE] = -10.0;

  data_scale[PERCENT_SCALE] = 2.55;
  data_bias[PERCENT_SCALE] = 0.0;

  /*
   * get color map 
   */

  cmap = DefaultColormap(Glob->rdisplay, Glob->rscreen);

  /*
   * get colorscale directory
   */

  colorscale_dir = xGetResString(Glob->rdisplay, Glob->prog_name,
				 "colorscale_dir", COLORSCALE_DIR);

  /*
   * get x color scale names from paramteres
   */

  colorscale_name[DBZ_SCALE] =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "ps_dbz_color_scale", PS_DBZ_COLOR_SCALE);

  colorscale_name[VORTICITY_SCALE] =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "ps_vorticity_color_scale", PS_VORTICITY_COLOR_SCALE);

  colorscale_name[PERCENT_SCALE] =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "ps_percent_color_scale", PS_PERCENT_COLOR_SCALE);

  /*
   * loop through the color scales
   */

  for (iscale = 0; iscale < NSCALES; iscale++) {

    /*
     * create file name
     */
    
    sprintf(colorscale_path_name, "%s%s%s",
	    colorscale_dir, PATH_DELIM,
	    colorscale_name[iscale]);

    /*
     * read color scale from file
     */

    if (GReadColorScale(colorscale_path_name, &cscale[iscale]) != 0) {

      fprintf(stderr, "ERROR - %s:get_x_color_scales\n", Glob->prog_name);
      fprintf(stderr, "Postscript color scale file '%s' for %s is invalid.\n",
	      colorscale_path_name, name[iscale]);
      tidy_and_exit(1);
    }

    /*
     * load up color scale psgc pointer array - this array is used
     * to point to the psgc relevant for a given byte data value
     */

    if (GLoadPsgcScale(cscale[iscale], Glob->rdisplay, cmap,
		       data_scale[iscale],
		       data_bias[iscale]) != 0) {

      fprintf(stderr, "ERROR - %s:get_ps_color_scales\n", Glob->prog_name);
      fprintf(stderr, "Loading psgc scale '%s' for %s.\n",
	      colorscale_path_name, name[iscale]);
      tidy_and_exit(1);
    }

  } /* iscale */

  /*
   * set the global pointers to the color scales
   */

  Glob->ps_dbz_cscale = cscale[DBZ_SCALE];
  Glob->ps_vorticity_cscale = cscale[VORTICITY_SCALE];
  Glob->ps_percent_cscale = cscale[PERCENT_SCALE];

  switch (Glob->timeht_mode) {

  case TIMEHT_MAXZ:
    Glob->ps_timeht_cscale = Glob->ps_dbz_cscale;
    break;
    
  case TIMEHT_MEANZ:
    Glob->ps_timeht_cscale = Glob->ps_dbz_cscale;
    break;
    
  case TIMEHT_MASS:
    Glob->ps_timeht_cscale = Glob->ps_percent_cscale;
    break;
    
  case TIMEHT_VORTICITY:
    Glob->ps_timeht_cscale = Glob->ps_vorticity_cscale;
    break;

  }

  Glob->ps_rdist_cscale = Glob->ps_dbz_cscale;
  
}

