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
 * refl_motion.c
 *
 * Motion for refl forecasts
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000
 *
 * Jan 1997
 *
 ****************************************************************************/

#include "precip_map.h"

/*
 * file scope
 */

static precip_map_motion_t *Motion_grid = NULL;
static si32 Npoints;
static cart_params_t *Cart;
static vol_file_handle_t Motion_v_handle;
  
/*
 * function prototypes
 */

static void load_grid_for_storm(mdv_grid_t *mdv_params,
				double x, double y,
				double u, double v,
				double majr);

/*
 * principal function
 */
     
int load_motion_grid(storm_file_handle_t *s_handle,
		     track_file_handle_t *t_handle,
		     cart_params_t *cart,
		     si32 iscan,
		     double lead_time_hr)
     
{

  int i;

  si32 ientry;
  si32 n_entries;

  double u, v;
  double x, y;
  double majr;

  storm_file_global_props_t *gprops;
  track_file_forecast_props_t *fprops;
  track_file_entry_t *entry;
  mdv_grid_t mdv_params;
  precip_map_motion_t *mptr;

  /*
   * allocate grid if needed
   */
  
  if (Motion_grid == NULL) {
    Cart = cart;
    Npoints = cart->ny * cart->nx;
    Motion_grid = (precip_map_motion_t *)
      umalloc (Npoints * sizeof(precip_map_motion_t));
  }
  
  /*
   * convert the cart params to floating point
   */

  cart_params_to_mdv_grid(cart, &mdv_params, MDV_PROJ_FLAT);

  /*
   * initialize the motion grid
   */

  memset(Motion_grid, 0, Npoints * sizeof(precip_map_motion_t));

  /*
   * read in track file scan entries
   */
  
  if (RfReadTrackScanEntries(t_handle, iscan, "refl_motion"))
    return (-1);
  
  n_entries = t_handle->scan_index[iscan].n_entries;
  
  /*
   * loop through the entries
   */
  
  entry = t_handle->scan_entries;
  
  for (ientry = 0; ientry < n_entries; ientry++, entry++) {

    if (!entry->forecast_valid) {
      continue;
    }
    
    fprops = &entry->dval_dt;
    
    /*
     * read in storm props
     */
      
    if (RfReadStormProps(s_handle, entry->storm_num,
			 "refl_motion")) {
      return (-1);
    }
    gprops = s_handle->gprops + entry->storm_num;

    /*
     * get major radius
     */

    majr = gprops->proj_area_major_radius;

    /*
     * get posn and area
     */

    x = gprops->proj_area_centroid_x;
    y = gprops->proj_area_centroid_y;
    
    /*
     * compute rates of change of posn and area
     */
    
    if (entry->forecast_valid) {
      u = fprops->proj_area_centroid_x;
      v = fprops->proj_area_centroid_y;
    } else {
      u = 0.0;
      v = 0.0;
    }

    /*
     * adjust the x,y pos for storm motion over the lead time
     */

    x += u * lead_time_hr;
    y += v * lead_time_hr;

    /*
     * load grid for this storm with weighted u,v values
     */

    load_grid_for_storm(&mdv_params, x, y, u, v, majr);
    
  } /* ientry */

  /*
   * compute the means of the velocity vectors for each grid point
   */

  mptr = Motion_grid;
  for (i = 0; i < Npoints; i++, mptr++) {
    if (mptr->wt > 0) {
      mptr->u /= mptr->wt;
      mptr->v /= mptr->wt;
      mptr->dx = (int) ((mptr->u * lead_time_hr) / mdv_params.dx + 0.5);
      mptr->dy = (int) ((mptr->v * lead_time_hr) / mdv_params.dy + 0.5);
    }
  } /* i */

  return (0);
  
}

/*******************************************************
 * load_refl_motion_forecast()
 *
 * Load up forecast grid based on motion grid
 */

void load_refl_motion_forecast(ui08 *comp_grid,
			       ui08 *forecast_grid,
			       field_params_t *dbz_fparams)

{

  int i, j, ix, jx, nx;
  ui08 *f;
  int dbz_byte, min_dbz_byte;
  double dbz_scale, dbz_bias;
  precip_map_motion_t *m;
  
  /*
   * compute min dbz byte val
   */
  
  dbz_scale = ((double) dbz_fparams->scale /
	       (double) dbz_fparams->factor);
  dbz_bias = ((double) dbz_fparams->bias /
	      (double) dbz_fparams->factor);

  min_dbz_byte = (int) ((Glob->params.min_dbz - dbz_bias) / dbz_scale + 0.5);

  /*
   * zero out forecast grid
   */
  
  memset(forecast_grid, 0, Npoints);
  
  /*
   * load up forecast grid
   */
  
  m = Motion_grid;
  f = forecast_grid;
  
  nx = Cart->nx;
  
  for (i = 0; i < Npoints; i++, m++, f++) {

    /*
     * the forecast value for a grid point is set to the
     * value in the composite grid from which the motion
     * vector originates, i.e. at the (-dx, -dy) offset 
     * position
     */

    ix = i % nx;
    jx = ix - m->dx;

    /*
     * check origin of vector is within grid
     */
    
    if (jx >= 0 && jx < nx) {
      j = i - (m->dy * nx) - m->dx;
      if (j >= 0 && j < Npoints) {
	dbz_byte = comp_grid[j];
	if (dbz_byte >= min_dbz_byte) {
	  *f = dbz_byte;
	}
      }
    }
    
  } /* i */

}

/*******************************************************
 * load_grid_for_storm()
 *
 * Load up u and v components for grid squares affected by the
 * given storm.
 */

static void load_grid_for_storm(mdv_grid_t *mdv_params,
				double x, double y,
				double u, double v,
				double majr)
     
{
  
  int ix, ix1, ix2;
  int iy, iy1, iy2;
  int nx, ny;
  double minx, miny;
  double dx, dy;
  double gx, gy;
  double diffx, diffy;
  double r, dr;
  double inner_r, outer_r;
  double ur, vr;
  double fraction, wt;
  precip_map_motion_t *mptr;

  /*
   * compute inner and outer radii
   *
   * Within the inner radius, the storm velocity applies.
   * Between the concentric circles, the velocity decreases linearly
   * with radius, to 0 at the outer radius.
   */
  
  inner_r = majr;
  outer_r = inner_r + Glob->params.smoothing_radius;
  dr = Glob->params.smoothing_radius;

  /*
   * compute the bounding box, constraining within the grid
   */

  ix1 = (int) (((x - outer_r) - mdv_params->minx) / mdv_params->dx + 0.5);
  ix1 = MAX(0, ix1);
  ix2 = (int) (((x + outer_r) - mdv_params->minx) / mdv_params->dx + 0.5);
  ix2 = MIN(mdv_params->nx - 1, ix2);
  
  iy1 = (int) (((y - outer_r) - mdv_params->miny) / mdv_params->dy + 0.5);
  iy1 = MAX(0, iy1);
  iy2 = (int) (((y + outer_r) - mdv_params->miny) / mdv_params->dy + 0.5);
  iy2 = MIN(mdv_params->ny - 1, iy2);

  /*
   * loop through the points in the bounding box
   */

  dx = mdv_params->dx;
  dy = mdv_params->dy;
  minx = mdv_params->minx;
  miny = mdv_params->miny;
  nx = mdv_params->nx;
  ny = mdv_params->ny;

  gy = miny + iy1 * dy;
  
  for (iy = iy1; iy <= iy2; iy++, gy += dy) {

    mptr = Motion_grid + (iy * nx) + ix1;
    gx = minx + ix1 * dx;

    for (ix = ix1; ix <= ix2; ix++, mptr++, gx += dx) {

      diffx = gx - x;
      diffy = gy - y;

      r = sqrt(diffx * diffx + diffy * diffy);
      
      /*
       * if the radius is outside the inner radius, decrease the
       * velocity linearly to a value of 0 at the outer radius.
       * Beyond the outer radius the velocity  is 0.
       */

      if (r < inner_r) {
	ur = u;
	vr = v;
      } else if (r < outer_r) {
	fraction = 1.0 - ((r - inner_r) / dr);
	ur = u * fraction;
	vr = v * fraction;
      } else {
	ur = 0.0;
	vr = 0.0;
      }

      /*
       * weight the velocity with inverse radius from storm
       */
      
      if (r == 0.0) {
	wt = 1000.0;
      } else {
	wt = 1.0 / r;
      }

      mptr->u += ur * wt;
      mptr->v += vr * wt;
      mptr->wt += wt;
      
    } /* ix */

  } /*  iy */
  
}

/************************************************************************
 * init_motion_v_handle()
 */

void init_motion_v_handle(vol_file_handle_t *map_v_handle)
     
{

  static int first_call = TRUE;
  
  field_params_t *fparams;
  
  if (first_call) {
    RfInitVolFileHandle(&Motion_v_handle,
			Glob->prog_name,
			(char *) NULL,
			(FILE *) NULL);
    RfAllocVolParams(&Motion_v_handle, "init_motion_v_handle");
    first_call = FALSE;
  }
  
  *(Motion_v_handle.vol_params) = *(map_v_handle->vol_params);
  
  Motion_v_handle.vol_params->nfields = 2;
  Motion_v_handle.vol_params->cart.nz = 1;
  Motion_v_handle.vol_params->cart.scalez = 10000;
  Motion_v_handle.vol_params->cart.km_scalez = 10000;
  Motion_v_handle.vol_params->cart.dz = 10000;
  
  if (RfAllocVolArrays(&Motion_v_handle, "init_motion_v_handle")) {
    tidy_and_exit(-1);
  }
  
  memcpy((void *) Motion_v_handle.radar_elevations,
	 (void *) map_v_handle->radar_elevations,
	 (int) (Motion_v_handle.vol_params->radar.nelevations * sizeof(si32)));
  
  Motion_v_handle.plane_heights[0][PLANE_BASE_INDEX] = 0;
  Motion_v_handle.plane_heights[0][PLANE_MIDDLE_INDEX] = 50000;
  Motion_v_handle.plane_heights[0][PLANE_TOP_INDEX] = 100000;

  /*
   * init field params for motion fields (u - field 0, v - field 1)
   */

  fparams = Motion_v_handle.field_params[0];
  
  memset((void *) fparams,
	 (int) 0, sizeof(field_params_t));
  fparams->encoded = TRUE;
  fparams->factor = 1000;
  fparams->scale = 500;
  fparams->bias = -63000;
  strcpy(fparams->name, "U storm motion");
  strcpy(fparams->transform, "none");
  strcpy(fparams->units, "m/s");

  fparams = Motion_v_handle.field_params[1];
  
  memset((void *) fparams,
	 (int) 0, sizeof(field_params_t));
  fparams->encoded = TRUE;
  fparams->factor = 1000;
  fparams->scale = 500;
  fparams->bias = -63000;
  strcpy(fparams->name, "V storm motion");
  strcpy(fparams->transform, "none");
  strcpy(fparams->units, "m/s");

}

/************************************************************************
 * write_motion_grid_file()
 *
 * Write the file with motion fields
 */

void  write_motion_grid_file(void)

{

  static int first_call = TRUE;
  static ui08 *u_grid, *v_grid;
  int i;
  int spd;
  ui08 *u, *v;
  double uscale, ubias;
  double vscale, vbias;
  precip_map_motion_t *mptr;

  /*
   * on first call, allocate memory
   */

  if (first_call) {

    u_grid = (ui08 *) umalloc (Npoints * sizeof(ui08));
    v_grid = (ui08 *) umalloc (Npoints * sizeof(ui08));
    first_call = FALSE;

  }

  /*
   * load u and v fields with scaled velocity data
   */
  
  Motion_v_handle.field_plane[0][0] = u_grid;
  Motion_v_handle.field_plane[1][0] = v_grid;

  uscale = ((double) Motion_v_handle.field_params[0]->scale /
	    (double) Motion_v_handle.field_params[0]->factor);
  ubias = ((double) Motion_v_handle.field_params[0]->bias /
	   (double) Motion_v_handle.field_params[0]->factor);
  vscale = ((double) Motion_v_handle.field_params[1]->scale /
	    (double) Motion_v_handle.field_params[1]->factor);
  vbias = ((double) Motion_v_handle.field_params[1]->bias /
	   (double) Motion_v_handle.field_params[1]->factor);

  /*
   * write to grid, converting to m/s - divide by 3.6
   */ 

  mptr = Motion_grid;
  u = u_grid;
  v = v_grid;
  for (i = 0; i < Npoints; i++, mptr++, u++, v++) {
    
    spd = ((mptr->u / 3.6) - ubias) / uscale;
    spd = MAX(0, spd);
    spd = MIN(255, spd);
    *u = spd;

    spd = ((mptr->v / 3.6) - vbias) / vscale;
    spd = MAX(0, spd);
    spd = MIN(255, spd);
    *v = spd;

  }
      
  /*
   * write the map file
   */
  
  sprintf(Motion_v_handle.vol_params->note,
	  "%s\n",
	  "Velocity vectors for reflectivity forecast");
  
  RfWriteDobson(&Motion_v_handle, FALSE, Glob->params.debug,
		Glob->params.motion_grid_dir,
		Glob->params.output_file_ext,
		Glob->prog_name,
		"refl_motion");
      
}


  
