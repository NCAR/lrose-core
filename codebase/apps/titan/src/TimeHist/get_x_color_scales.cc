// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*********************************************************************
 * get_x_color_scales.c
 * 
 * reads a color table file for different variable types, and sets up
 * a color scale struct for each one
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "TimeHist.hh"
using namespace std;

#define NSCALES 3
#define DBZ_SCALE 0
#define VORTICITY_SCALE 1
#define PERCENT_SCALE 2

void get_x_color_scales(void)

{

  const char *name[NSCALES];
  char *colorscale_dir;
  char *colorscale_name[NSCALES];
  char colorscale_path_name[BUFSIZ];
  g_color_scale_t *cscale[NSCALES];
  double data_scale[NSCALES], data_bias[NSCALES];

  int iscale, ilevel;

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

  colorscale_dir = uGetParamString(Glob->prog_name,
				   "colorscale_dir", COLORSCALE_DIR);
  
  /*
   * get x color scale names from paramteres
   */

  colorscale_name[DBZ_SCALE] =
    uGetParamString(Glob->prog_name,
		    "x_dbz_color_scale", X_DBZ_COLOR_SCALE);
  
  colorscale_name[VORTICITY_SCALE] =
    uGetParamString(Glob->prog_name,
		    "x_vorticity_color_scale", X_VORTICITY_COLOR_SCALE);
  
  colorscale_name[PERCENT_SCALE] =
    uGetParamString(Glob->prog_name,
		    "x_percent_color_scale", X_PERCENT_COLOR_SCALE);

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
      fprintf(stderr, "Color scale file '%s' for %s is invalid.\n",
	      colorscale_path_name, name[iscale]);
      tidy_and_exit(1);
    }

    /*
     * load up color scale GC pointer array - this array is used
     * to point to the GC relevant for a given byte data value
     */

    
    if (GLoadGCScale(cscale[iscale],
		     Glob->rdisplay, cmap,
		     &Glob->color_index,
		     data_scale[iscale], data_bias[iscale]) != 0) {

      fprintf(stderr, "ERROR - %s:get_x_color_scales\n", Glob->prog_name);
      fprintf(stderr, "Loading colorscale '%s' for %s.\n",
	      colorscale_path_name, name[iscale]);
      tidy_and_exit(1);
    }

    /*
     * print out GC ID values for the colors
     */

    if (Glob->verbose) {

      fprintf(stderr, "Colors for field %s\n", colorscale_name[iscale]);

      for (ilevel = 0; ilevel < cscale[iscale]->nlevels; ilevel++) {

	fprintf(stderr, "Level %d, color name '%s', GC ID %ld\n",
		ilevel,
		cscale[iscale]->level[ilevel].colorname,
		(long) cscale[iscale]->level[ilevel].gc);

      } /* ilevel */

    } /* if (Glob->verbose) */

  } /* iscale */

  /*
   * set the global pointers to the color scales
   */

  Glob->x_dbz_cscale = cscale[DBZ_SCALE];
  Glob->x_vorticity_cscale = cscale[VORTICITY_SCALE];
  Glob->x_percent_cscale = cscale[PERCENT_SCALE];

  switch (Glob->timeht_mode) {

  case TIMEHT_MAXZ:
    Glob->x_timeht_cscale = Glob->x_dbz_cscale;
    break;
    
  case TIMEHT_MEANZ:
    Glob->x_timeht_cscale = Glob->x_dbz_cscale;
    break;
    
  case TIMEHT_MASS:
    Glob->x_timeht_cscale = Glob->x_percent_cscale;
    break;
    
  case TIMEHT_VORTICITY:
    Glob->x_timeht_cscale = Glob->x_vorticity_cscale;
    break;

  }

  Glob->x_rdist_cscale = Glob->x_dbz_cscale;
  
}
