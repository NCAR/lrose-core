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
 * compute_contingency_data.c
 *
 * computes the contingency data from the forecast and verify grids
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * january 1992
 *
 *****************************************************************************/

#include "verify_tracks.h"

static void compute_as_is_area(ui08 **forecast_grid,
			       ui08 **truth_grid,
			       vt_count_t *count);

static void compute_as_is_point(ui08 **forecast_grid,
				ui08 **truth_grid,
				long nbytes_grid,
				vt_count_t *count);

static void compute_best_fit(ui08 **forecast_grid,
			     ui08 **truth_grid,
			     vt_count_t *count);

static void load_mod_truth_grid(ui08 **truth_grid);

static void print_grid(char *label,
		       ui08 **grid);

static void setup_template(void);

/*
 * file scope variables
 */

static long Template_half_nx;
static long Template_nx;
static long Template_half_ny;
static long Template_ny;
static long Template_n;
static long Template_n_fraction;
static long Template_n_nonfraction;
static long *Template_start_ix;
static long *Template_active_nx;
static ui08 **Template;
static ui08 **Mod_truth;

void compute_contingency_data(ui08 **forecast_grid,
			      ui08 **truth_grid,
			      long nbytes_grid,
			      vt_count_t *count)

{

  static int first_call = TRUE;
  
  /*
   * if first call, set up template for area activity computations
   */
  
  if (first_call) {
    if (Glob->activity_criterion == AREA_ACTIVITY)
      setup_template();
    first_call = FALSE;
  } /* if (first_call) */
  
  if (Glob->force_xy_fit) {
    
    /*
     * best fit computations - grid data is moved so create best fit
     * of one set over the other
     */

    compute_best_fit(forecast_grid,
		     truth_grid,
		     count);
    
  } else {

    /*
     * normal computations, leaving grids unaltered
     */

    if (Glob->activity_criterion == POINT_ACTIVITY)
      compute_as_is_point(forecast_grid,
			  truth_grid,
			  nbytes_grid,
			  count);
    else
      compute_as_is_area(forecast_grid,
			 truth_grid,
			 count);
    
  }
  
}

/**********************************************************************
 * compute_best_fit()
 *
 * best fit computations - grid data is moved so create best xy fit
 * of one set relative to the other
 */

static void compute_best_fit(ui08 **forecast_grid,
			     ui08 **truth_grid,
			     vt_count_t *count)

{

  ui08 *truth, *forecast;

  long ix, iy;
  long n_succ = 0, n_fail = 0, n_false = 0, n_non = 0;
  long nx_compare, ny_compare;
  long startx_forecast, starty_forecast;
  long startx_truth, starty_truth;
  long xoffset, yoffset;

  double x, y;
  double forecast_meanx, forecast_meany;
  double truth_meanx, truth_meany;
  double forecast_sumx = 0.0, forecast_sumy = 0.0, forecast_n = 0.0;
  double truth_sumx = 0.0, truth_sumy = 0.0, truth_n = 0.0;
  double xdiff, ydiff;

  /*
   * compute the mean (x, y) for the data in each grid
   */

  y = Glob->miny;
  
  for (iy = 0; iy < Glob->ny; iy++) {

    x = Glob->minx;
    truth = truth_grid[iy];
    forecast = forecast_grid[iy];
    
    for (ix = 0; ix < Glob->nx; ix++) {
      
      if (*forecast) {
	forecast_sumx += x;
	forecast_sumy += y;
	forecast_n++;
      }

      if (*truth) {
	truth_sumx += x;
	truth_sumy += y;
	truth_n++;
      }

      x += Glob->dx;
      truth++;
      forecast++;

    } /* ix */

    y += Glob->dy;

  } /* iy */
  
  forecast_meanx = forecast_sumx / forecast_n;
  forecast_meany = forecast_sumy / forecast_n;

  truth_meanx = truth_sumx / truth_n;
  truth_meany = truth_sumy / truth_n;

  /*
   * compute the offsets required to match the centroids of the data
   * when performing the grid comparison
   */

  xdiff = truth_meanx - forecast_meanx;
  ydiff = truth_meany - forecast_meany;

  xoffset = (long) floor (xdiff / Glob->dx + 0.5);
  yoffset = (long) floor (ydiff / Glob->dy + 0.5);

  if (Glob->debug) {
    fprintf(stderr, "compute_contingency_data\n");
    fprintf(stderr, "xoffset, yoffset = %ld, %ld\n", xoffset, yoffset);
  }

  nx_compare = Glob->nx - abs((int) xoffset);
  ny_compare = Glob->ny - abs((int) yoffset);

  if (xoffset >= 0) {
    startx_forecast = 0;
    startx_truth = xoffset;
  } else {
    startx_forecast = -xoffset;
    startx_truth = 0;
  }
    
  if (yoffset >= 0) {
    starty_forecast = 0;
    starty_truth = yoffset;
  } else {
    starty_forecast = -yoffset;
    starty_truth = 0;
  }

  if (Glob->debug) {

    fprintf(stderr, "forecast startx, starty ' %ld, %ld\n",
	    startx_forecast, starty_forecast);

    fprintf(stderr, "truth startx, starty ' %ld, %ld\n",
	    startx_truth, starty_truth);

    fprintf(stderr, "compare nx, ny = %ld, %ld\n",
	    nx_compare, ny_compare);

  }

  for (iy = 0; iy < ny_compare; iy++) {
    
    truth = truth_grid[starty_truth + iy] + startx_truth;
    forecast = forecast_grid[starty_forecast + iy] + startx_forecast;
    
    for (ix = 0; ix < nx_compare; ix++) {

      if (*forecast && *truth) {
	
	n_succ++;
	
      } else if (!*forecast && *truth) {

	n_fail++;
      
      } else if (*forecast && !*truth) {
	
	n_false++;

      } else {

	n_non++;
	
      }
      
      truth++;
      forecast++;

    } /* ix */

  } /* iy */
  
  count->n_success = (double) n_succ;
  count->n_failure = (double) n_fail;
  count->n_false_alarm = (double) n_false;
  count->n_non_event = (double) n_non;

}

/************************************************************************
 * compute_as_is_area()
 *
 * Normal contingency table computations - area criterion
 */


static void compute_as_is_area(ui08 **forecast_grid,
			       ui08 **truth_grid,
			       vt_count_t *count)

{

  ui08 *truth, *forecast;
  long ix, iy;
  long n_succ = 0, n_fail = 0, n_false = 0, n_non = 0;

  load_mod_truth_grid(truth_grid);

  for (iy = Template_half_ny; iy < Glob->ny - Template_half_ny; iy++) {

    truth = Mod_truth[iy] + Template_half_nx;  
    forecast = forecast_grid[iy] + Template_half_nx;  
    
    for (ix = Template_half_nx; ix < Glob->nx - Template_half_nx; ix++) {

      if ((long) (*forecast && *truth) >= Template_n_fraction) {
	
	n_succ++;
	
      } else if ((long) (!*forecast && *truth) >= Template_n_nonfraction) {
	
	n_fail++;
	
      } else if ((long) (*forecast && *truth) < Template_n_fraction) {
	
	n_false++;

      } else {

	n_non++;
	
      }
      
      truth++;
      forecast++;
      
    } /* ix */
    
  } /* iy */
  
  count->n_success = (double) n_succ;
  count->n_failure = (double) n_fail;
  count->n_false_alarm = (double) n_false;
  count->n_non_event = (double) n_non;

}

/************************************************************************
 * compute_as_is_point()
 *
 * Normal contingency table computations - point criterion
 */


static void compute_as_is_point(ui08 **forecast_grid,
				ui08 **truth_grid,
				long nbytes_grid,
				vt_count_t *count)

{

  register ui08 *truth, *forecast;
  register long i;
  register long n_succ = 0, n_fail = 0, n_false = 0, n_non = 0;

  /*
   * set pointer to start of grids
   */
  
  truth = *truth_grid;
  forecast = *forecast_grid;
  
  /*
   * loop through grid points
   */
  
  for (i = 0; i < nbytes_grid; i++) {
    
    if (*forecast && *truth) {
	
      n_succ++;
	
    } else if (!*forecast && *truth) {

      n_fail++;
      
    } else if (*forecast && !*truth) {
	
      n_false++;

    } else {

      n_non++;
	
    }
      
    truth++;
    forecast++;
      
  } /* i */

  count->n_success = (double) n_succ;
  count->n_failure = (double) n_fail;
  count->n_false_alarm = (double) n_false;
  count->n_non_event = (double) n_non;

}

/************************************************************************
 * load_mod_truth_grid()
 */

static void load_mod_truth_grid(ui08 **truth_grid)

{

  ui08 *mod_truth, *truth;

  long ix, iy, jx, jy, kx, ky;
  long *start_ix;
  long *active_nx;
  
  /*
   * initialize the mod truth grid
   */
  
  memset ((void *)  *Mod_truth,
          (int) 0, (size_t) (Glob->nx * Glob->ny));
  
  /*
   * look through the truth grid for active points
   */
  
  for (iy = Template_half_ny; iy < Glob->ny - Template_half_ny; iy++) {
    
    truth = truth_grid[iy] + Template_half_nx;

    for (ix = Template_half_nx; ix < Glob->nx - Template_half_nx; ix++) {
      
      if (*truth) {
	
	/*
	 * the truth point is active, so spread the effect according
	 * to the template
	 */
	
	ky = iy - Template_half_ny;
	kx = ix - Template_half_nx;
	start_ix = Template_start_ix;
	active_nx = Template_active_nx;
	
	for (jy = 0; jy < Template_ny; jy++) {
	  
	  mod_truth = Mod_truth[ky] + kx + *start_ix;

	  for (jx = *start_ix; jx < *active_nx; jx++) {
	    *mod_truth = *mod_truth + 1;
	    mod_truth++;
	  } /* jx */
	  
	  start_ix++;
	  active_nx++;
	  ky++;
	  
	} /* jy */

      } /* if (*truth) */

      truth++;

    } /* ix */

  } /* iy */

  if (Glob->debug) {
    print_grid("Truth grid\n-------------", truth_grid);
    print_grid("Mod truth grid\n-----------------", Mod_truth);
  }
  
}

/***************************************************************************
 * print_grid()
 */

static void print_grid(char *label,
		       ui08 **grid)

{

  int info_found;
  long count;
  long ix, iy;

  /*
   * print header
   */

  fprintf(stderr, "\n%s\n", label);

  fprintf(stderr, "     ");
  count = 10;
  for (ix = 0; ix < Glob->nx; ix++) {
    if (count < 10) {
      fprintf(stderr, "   ");
    } else {
      fprintf(stderr, " | ");
      count = 0;
    }
    count++;
  } /* ix */

  fprintf(stderr, "\n");

  /*
   * print grid
   */

  for (iy = Glob->ny - 1; iy >= 0; iy--) {

    info_found = FALSE;

    for (ix = 0; ix < Glob->nx; ix++)
      if (grid[iy][ix])
	info_found = TRUE;
    
    if (info_found) {
      
      fprintf(stderr, "%4ld ", iy);
      
      for (ix = 0; ix < Glob->nx; ix++)
	fprintf(stderr, "%3d", grid[iy][ix]);
      
      fprintf(stderr, "\n");
      
    } /* if (info_found) */
    
  } /* iy */

}

/****************************************************************
 * setup_template()
 *
 * set up the template for area activity computations
 */

static void setup_template(void)

{

  long ix, iy;
  long start_ix, end_ix;
  double x, y, r;

  /*
   * compute template size
   */

  Template_half_nx = (long) (Glob->activity_radius / Glob->dx + 0.5);
  Template_nx = 1 + 2 * Template_half_nx;
  Template_half_ny = (long) (Glob->activity_radius / Glob->dy + 0.5);
  Template_ny = 1 + 2 * Template_half_ny;

  /*
   * allocate memeory
   */

  Template = (ui08 **) ucalloc2
    ((ui32) Template_ny, (ui32) Template_nx, sizeof(ui08));

  Template_start_ix = (long *) umalloc
    ((ui32) (Glob->ny * sizeof(long)));

  Template_active_nx = (long *) umalloc
    ((ui32) (Glob->ny * sizeof(long)));
  
  Mod_truth = (ui08 **) ucalloc2
    ((ui32) Glob->ny, (ui32) Glob->nx, sizeof(ui08));
  
  /*
   * set template points for which the distance from the center of
   * the template is within the activity_radius from the center of
   * the template
   */

  Template_n = 0;

  for (iy = 0; iy < Template_nx; iy++) {

    y = (double) (iy - Template_half_ny) * Glob->dy;
    
    for (ix = 0; ix < Template_nx; ix++) {

      x = (double) (ix - Template_half_nx) * Glob->dx;
      
      r = sqrt(x * x + y * y);

      if (r <= Glob->activity_radius) {
	Template[iy][ix] = TRUE;
	Template_n++;
      }

    } /* ix */

  } /* iy */

  
  for (iy = 0; iy < Template_nx; iy++) {

    start_ix = Template_nx;
    end_ix = 0;
    
    for (ix = 0; ix < Template_nx; ix++) {
      
      if (Template[iy][ix]) {
     	start_ix = MIN(start_ix, ix);
	end_ix = MAX(end_ix, ix);
      }

    } /* ix */

    Template_start_ix[iy] = start_ix;
    Template_active_nx[iy] = end_ix - start_ix + 1;
    
  } /* iy */

  Template_n_fraction = (long)
    ((double) Template_n * Glob->activity_fraction + 0.5);

  Template_n_nonfraction = Template_n - Template_n_fraction;

  /*
   * check that the number of active template points does not
   * exceed the capacity of a ui08
   */

  if (Template_n > 255) {
    fprintf(stderr,
	    "ERROR - %s:compute_contingency_data:setup_template\n",
	    Glob->prog_name);
    fprintf(stderr, "Template exceeds 255 active points\n");
    fprintf(stderr, "Decrease the active radius or");
    fprintf(stderr, "the grid resolution.\n");
    exit (-1);
  } /* if (Template_n > 255) */
  
}

