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
 * props_compute.c
 *
 * Computes storm properties.
 *
 * Returns 1 if storm added, 0 if not.
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * January 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_ident.h"
#include <physics/vil.h>

/*
 * file scope globals
 */

static int Range_limited;
static int Top_missing;
static int Hail_present;
static int Second_trip;

static si32 Nz_valid;
static si32 Top_layer, Base_layer;
static si32 N_dbz_intvls;
static si32 N_layers;

static double Min_valid_z;
static double Test_elev;
static double Test_range;
static double Dbz_scale, Dbz_bias;
static double Vel_scale, Vel_bias;
static double Z_p_inverse_coeff, Z_p_inverse_exponent;
static double Z_m_inverse_coeff, Z_m_inverse_exponent;
static double Vorticity_hemisphere_factor;
static double Min_vorticity_dist;
  
static si32 High_dbz_byte;
static layer_stats_t *Layer;
static dbz_hist_entry_t *Dbz_hist;
static storm_file_global_props_t Gprops;
static sum_stats_t Sum;

/*
 * file scope prototypes
 */

static int compute_basic_props(Clump_order *clump,
			       vol_file_handle_t *v_handle,
			       storm_ident_grid_t *grid_params,
			       double clump_vol,
			       double dvol_at_centroid,
			       double darea_at_centroid);

static void compute_secondary_props(Clump_order *clump,
				    vol_file_handle_t *v_handle,
				    storm_ident_grid_t *grid_params);

static si32 store_runs(Clump_order *clump,
		       storm_file_handle_t *s_handle,
		       storm_ident_grid_t *grid_params);

/*
 * main
 */

si32
props_compute(si32 storm_num,
	      Clump_order *clump,
	      vol_file_handle_t *v_handle,
	      storm_file_handle_t *s_handle,
	      storm_ident_grid_t *grid_params,
	      double clump_vol,
	      double dvol_at_centroid,
	      double darea_at_centroid,
	      double darea_ellipse)
      
{
  
  si32 iz, dbz_intvl;
  storm_file_global_props_t *gprops;
  
  PMU_auto_register("Computing properties");
    
  /*
   * initialize
   */
    
  Base_layer = Nz_valid;
  Top_layer = 0;
  Gprops.dbz_max = 0.0;
  Range_limited = FALSE;
  Top_missing = FALSE;
  Hail_present = FALSE;
  Second_trip = FALSE;
  
  memset ((void *)  Layer,
	  (int) 0, (size_t) (Nz_valid * sizeof(layer_stats_t)));
  memset ((void *)  Dbz_hist,
	  (int) 0, (size_t) (Glob->n_dbz_hist_intervals *
			     sizeof(dbz_hist_entry_t)));
  memset ((void *)  &Sum,
	  (int) 0, (size_t) (sizeof(sum_stats_t)));
    
  /*
   * first pass through the clumps, computing the relevant things
   * from which to compute the storm properties.
   * Also, count the number of data runs
   * for this storm
   */
    
  if (compute_basic_props(clump, v_handle, grid_params,
			  clump_vol, dvol_at_centroid,
			  darea_at_centroid)) {
    return (0L);
  }
  
  /*
   * perform the areal computations for precip and projected
   * areas, including dbz histogram for area
   */

  area_comps(clump, grid_params,
	     &Gprops, darea_at_centroid, darea_ellipse);
  
  /*
   * compute other props during second pass through intervals
   */
  
  compute_secondary_props(clump, v_handle, grid_params);

  /*
   * tilt angle computations
   */

  tilt_compute(Glob->nz, N_layers, Base_layer, Top_layer,
	       Min_valid_z, Layer,
	       &Gprops.tilt_dirn, &Gprops.tilt_angle);

  /*
   * compute dbz gradient with height
   */

  dbz_gradient_compute(Glob->nz, N_layers, Base_layer, Top_layer,
		       Min_valid_z, Layer,
		       &Gprops.dbz_max_gradient,
		       &Gprops.dbz_mean_gradient);

  /*
   * decide whether this is a second trip echo
   */
   
  if (Glob->params.check_second_trip) {
    
    Second_trip = check_second_trip(Gprops.top,
				    Gprops.base,
				    Gprops.proj_area_centroid_x,
				    Gprops.proj_area_centroid_y,
				    Gprops.proj_area_major_radius,
				    Gprops.proj_area_minor_radius,
				    Gprops.proj_area_orientation);
    
  }
      
  /*
   * write valid_storms verification file
   */
  
  if (Glob->params.create_verification_files) {
    update_valid_storms_grid(clump, grid_params);
  }

  /*
   * load up global storm properties
   */

  gprops = s_handle->gprops + storm_num;
  *gprops = Gprops;
  
  load_gprops(gprops,
	      storm_num, N_layers, Base_layer, N_dbz_intvls,
	      Range_limited, Top_missing,
	      Hail_present, Second_trip);
  
  /*
   * load layer props structure, for each layer
   */
  
  for (iz = Base_layer; iz <= Top_layer; iz++) {
    load_lprops(Layer + iz, s_handle->layer + iz - Base_layer);
  } /* iz */
  
  /*
   * load dbz histogram struct, for each histogram interval
   */
    
  for (dbz_intvl = 0; dbz_intvl < N_dbz_intvls; dbz_intvl++) {
    load_dbz_hist(Dbz_hist + dbz_intvl,
		  s_handle->hist + dbz_intvl);
  }
  
  /*
   * store runs
   */

  if (Glob->params.store_storm_runs) {
    gprops->n_runs =
      store_runs(clump, s_handle, grid_params);
  } else {
    gprops->n_runs = 0;
  }

  if (Glob->params.store_proj_runs) {
    gprops->n_proj_runs =
      store_proj_runs(s_handle, grid_params);
  } else {
    gprops->n_proj_runs = 0;
  }

  /*
   * write the layer props and dbz histogram to file
   */
  
  if (RfWriteStormProps(s_handle, storm_num,
			"props_compute"))
    tidy_and_exit(-1);
  
  return (1L);

}

/**********************
 * init_props_compute()
 *
 * initialize static variables for the props_compute() routine
 */

void init_props_compute(vol_file_handle_t *v_handle,
			storm_file_handle_t *s_handle,
			double *min_valid_z_p)

{

  static int first_call = TRUE;
  static si32 nz_alloc;
  
  double max_range, range_margin;
  double max_elev, elev_margin;
    
  radar_params_t *radar;

  if (first_call) {
    
    /*
     * allocate space for computational arrays
     */
    
    Layer =
      (layer_stats_t *) umalloc((ui32) (Glob->nz * sizeof(layer_stats_t)));
    nz_alloc = Glob->nz;
    
    Dbz_hist = (dbz_hist_entry_t *)
      umalloc((ui32) (Glob->n_dbz_hist_intervals * sizeof(dbz_hist_entry_t)));
    
    High_dbz_byte = Glob->high_dbz_byte;

    /*
     * alloc for props
     */
    
    RfAllocStormProps(s_handle, Glob->nz,
		      Glob->n_dbz_hist_intervals,
		      0L, 0L, "init_props_compute");

    first_call = FALSE;
    
  } else {

    if (Glob->nz > nz_alloc) {
      Layer =
	(layer_stats_t *) urealloc((char *) Layer,
				   (ui32) (Glob->nz * sizeof(layer_stats_t)));
      nz_alloc = Glob->nz;
    }
    
  } /* if (first_call) */
  
  /*
   * initialize the computation module for volume
   * and area
   */

  init_vol_and_area_comps();

  /*
   * min valid ht and number of valid planes
   */

  Min_valid_z = Glob->min_z + Glob->min_valid_layer * Glob->delta_z;
  Nz_valid = Glob->nz - Glob->min_valid_layer;
  
  /*
   * compute test sampling range - if any points in a storm are at
   * longer range, set flag indicating it is range limited. The test
   * range is the max range less twice the diagonal of a grid cell.
   * For this test, it is only necessary to consider x and y, since the
   * elevation angle will be low.
   */
  
  radar = &v_handle->vol_params->radar;
  
  if(radar->ngates > 0) {
      max_range = ((double) radar->start_range +
		   (double) radar->gate_spacing *
		   radar->ngates) / 1000000.0;
      range_margin =
	  2.0 * pow((Glob->delta_x * Glob->delta_x +
		     Glob->delta_y * Glob->delta_y), 0.5);
      Test_range = max_range - range_margin;
  } else {
      Test_range = 0.0;
  }
  
  /*
   * compute test elevation angle - if any points in a storm are at
   * higher elevation angle, set flag to indicate that the storm top
   * has not been sampled. The test angle is the highest elevation angle
   * in the volume scan less some margin.
   */

  if (radar->nelevations > 0) {
      elev_margin = (double) radar->beam_width / 1000000.0;
      max_elev =
	  ((double) v_handle->radar_elevations[radar->nelevations - 1] /
	   1000000.0);
      Test_elev = (max_elev - elev_margin) * DEG_TO_RAD;
  } else {
      Test_elev = 0.0;
  }
  
  Dbz_scale = (double) v_handle->field_params[Glob->params.dbz_field]->scale /
    (double) v_handle->field_params[Glob->params.dbz_field]->factor;
  
  Dbz_bias = (double) v_handle->field_params[Glob->params.dbz_field]->bias /
    (double) v_handle->field_params[Glob->params.dbz_field]->factor;
  
  if (Glob->params.vel_available) {
    
    Vel_scale =
      (((double) v_handle->field_params[Glob->params.vel_field]->scale /
	(double) v_handle->field_params[Glob->params.vel_field]->factor) *
       Glob->params.vel_to_m_per_sec_scale);
    
    Vel_bias =
      (((double) v_handle->field_params[Glob->params.vel_field]->bias /
	(double) v_handle->field_params[Glob->params.vel_field]->factor) *
       Glob->params.vel_to_m_per_sec_scale);
    
  }
  
  /*
   * inverse coefficients for the Z-R and Z-M relationships
   */
  
  Z_p_inverse_coeff = (double) 1.0 / (double) Glob->params.ZR.coeff;
  Z_p_inverse_exponent = (double) 1.0 / (double) Glob->params.ZR.expon;
  
  Z_m_inverse_coeff = (double) 1.0 / (double) Glob->params.ZM.coeff;
  Z_m_inverse_exponent = (double) 1.0 / (double) Glob->params.ZM.expon;
  
  /*
   * vorticity factor to take into account the fact that
   * positive vorticity is anti-clockwise in the northern
   * hemisphere and clockwise in the southern hemisphere.
   * Northern - latitude positive. Southern - latitude negative.
   */
  
  if (Glob->params.vel_available) {
    
    if (v_handle->vol_params->radar.latitude >= 0)
      Vorticity_hemisphere_factor = 1.0;
    else
      Vorticity_hemisphere_factor = -1.0;
    
    /*
     * min vorticity distance limits the calculations to a certain
     * distance from the cemtroid. to avoid very close points from
     * having a dis-proportional effect on the calculation
     */
    
    Min_vorticity_dist = 2.0 * sqrt(Glob->delta_x * Glob->delta_x +
				    Glob->delta_y * Glob->delta_y);
    
  }

  /*
   * initialize the area_comps module
   */

  init_area_comps(Dbz_scale, Dbz_bias, Dbz_hist,
		  Z_p_inverse_coeff, Z_p_inverse_exponent);
  
  /*
   * set return vals
   */

  *min_valid_z_p = Min_valid_z;
  
  return;

}

/***********************
 * compute_basic_props()
 *
 * First pass through data
 *
 * Returns 0 if successful, -1 if not
 */

static int compute_basic_props(Clump_order *clump,
			       vol_file_handle_t *v_handle,
			       storm_ident_grid_t *grid_params,
			       double clump_vol,
			       double dvol_at_centroid,
			       double darea_at_centroid)

{

  ui08 *dbz_ptr;
  ui08 *vel_ptr = NULL;

  si32 index;
  si32 ix, iy, iz, jz, intv, dbz_intvl;
  si32 dbz_max_layer;
  si32 max_dbz_intvl;
  
  double cent_x, cent_y;
  double range;
  double dn, dnperlayer;
  double refl, dbz, r_dbz, vel;
  double mass_factor;
  
  Interval *intvl;

  /*
   * initialize
   */

  max_dbz_intvl = 0;

  for (intv = 0; intv < clump->size; intv++) {
    
    intvl = clump->ptr[intv];

    iz = intvl->plane;
    jz = iz + Glob->min_valid_layer;
    iy = intvl->row_in_plane;
    Layer[iz].n += intvl->len;
	
    index = ((iy + grid_params->start_iy) * Glob->nx +
	     intvl->begin + grid_params->start_ix);
    
    dbz_ptr = v_handle->field_plane[Glob->params.dbz_field][jz] + index;
    if (Glob->params.vel_available) {
      vel_ptr = v_handle->field_plane[Glob->params.vel_field][jz] + index;
    }
      
    for (ix = intvl->begin; ix <= intvl->end;
	 ix++, dbz_ptr++, vel_ptr++) {
      
      /*
       * if dbz value exceeds max threshold, don't
       * process this clump
       */
	
      if ((si32) *dbz_ptr > High_dbz_byte) {
	return (-1);
      }
	
      /*
       * get the dbz, vel and mass_factor
       */
	
      dbz = (double) *dbz_ptr * Dbz_scale + Dbz_bias;
	
      if (dbz > Glob->params.hail_dbz_threshold) {
	r_dbz = Glob->params.hail_dbz_threshold;
	Hail_present = TRUE;
      } else {
	r_dbz = dbz;
      }
	
      refl = pow((double) 10.0, r_dbz / (double) 10.0);
	
      index = iy * Glob->nx + ix;
	
      if (Glob->params.vel_available) {
	vel = *vel_ptr * Vel_scale + Vel_bias;
      }
      
      mass_factor = pow(refl, Z_m_inverse_exponent);
	
      /*
       * get the dbz histogram index number
       */
      
      dbz_intvl = Glob->dbz_interval[*dbz_ptr];
      
      /*
       * increment quantities
       */
      
      /*
       * number of grid points per layer
       */
      
      Layer[iz].sum_x += ix;
      Layer[iz].sum_y += iy;
      
      /*
       * reflectivity
       */
      
      Layer[iz].sum_refl_x += (double) ix * refl;
      Layer[iz].sum_refl_y += (double) iy * refl;
      Layer[iz].sum_refl += refl;
      
      if (dbz > Layer[iz].dbz_max)
	Layer[iz].dbz_max = dbz;
      
      /*
       * dbz histograms
       */
      
      if (dbz_intvl >= 0) {
	
	Dbz_hist[dbz_intvl].n_vol++;
	
      } /* if (dbz_intvl >= 0) */
      
      /*
       * mass
       */
      
      Layer[iz].sum_mass += mass_factor;
      
      /*
       * velocity
       */
      
      if (Glob->params.vel_available) {
	Layer[iz].sum_vel += vel;
	Layer[iz].sum_vel2 += vel * vel;
      }
      
    } /* ix */
    
  } /* intv  - first pass */
  
  /*
   * sum up variables over layers
   */
  
  for (iz = 0; iz < Nz_valid; iz++) {
    
    if (Layer[iz].n > 0) {
      
      Sum.n += Layer[iz].n;
      Sum.x += Layer[iz].sum_x;
      Sum.y += Layer[iz].sum_y;
      Sum.z += iz * Layer[iz].n;
      
      Sum.refl += Layer[iz].sum_refl;
      Sum.refl_x += Layer[iz].sum_refl_x;
      Sum.refl_y += Layer[iz].sum_refl_y;
      Sum.refl_z += (double) iz * Layer[iz].sum_refl;
	
      if (Layer[iz].dbz_max >= Gprops.dbz_max) {
	Gprops.dbz_max = Layer[iz].dbz_max;
	dbz_max_layer = iz;
      }
	
      Sum.mass += Layer[iz].sum_mass;
	
      if (iz > Top_layer)
	Top_layer = iz;
	
      if (iz < Base_layer)
	Base_layer = iz;
	
      if (Glob->params.vel_available) {
	Sum.vel += Layer[iz].sum_vel;
	Sum.vel2 += Layer[iz].sum_vel2;
      }
      
    }
      
  } /* iz */
    
  N_layers = Top_layer - Base_layer + 1;
    
  /*
   * compute the properties
   */
    
  dn = (double) Sum.n;
  
  Gprops.vol_centroid_x = grid_params->min_x + 
    ((double) Sum.x / dn) * grid_params->dx; /* km */
  Gprops.vol_centroid_y = grid_params->min_y + 
    ((double) Sum.y / dn) * grid_params->dy; /* km */
  Gprops.vol_centroid_z = Min_valid_z + 
    ((double) Sum.z / dn) * Glob->delta_z; /* km */
  
  Gprops.refl_centroid_x = grid_params->min_x +
    (Sum.refl_x / Sum.refl) * grid_params->dx; /* km */
    
  Gprops.refl_centroid_y = grid_params->min_y +
    (Sum.refl_y / Sum.refl) * grid_params->dy; /* km */
  
  Gprops.refl_centroid_z = Min_valid_z +
    (Sum.refl_z / Sum.refl) * Glob->delta_z; /* km */
  
  Gprops.volume = clump_vol; /* km3 */
  
  Gprops.area_mean =
    (dn / (double) N_layers) * darea_at_centroid; /* km2 */
  
  Gprops.top = Min_valid_z +
    ((double) Top_layer + 0.5) * Glob->delta_z; /* km */
    
  Gprops.base = Min_valid_z +
    ((double) Base_layer - 0.5) * Glob->delta_z; /* km */
    
  Gprops.ht_of_dbz_max = Min_valid_z +
    (double) dbz_max_layer * Glob->delta_z; /* km */
    
  Gprops.dbz_mean = log10(Sum.refl / dn) * 10.0; /* dbz */
  
  Gprops.rad_vel_mean = (Sum.vel / dn); /* m/s */
  
  Gprops.rad_vel_sd = usdev(Sum.vel, Sum.vel2, dn);

  Gprops.mass = (Sum.mass * dvol_at_centroid
		    * pow(Z_m_inverse_coeff,
			  Z_m_inverse_exponent)); /* ktons */
    
  /*
   * layer properties
   */

  for (iz = 0; iz < Nz_valid; iz++) {
    
    Layer[iz].area = (double) Layer[iz].n * darea_at_centroid; /* km2 */
    
    if (Layer[iz].n > 0) {
      
      dnperlayer = (double) Layer[iz].n;
	
      Layer[iz].vol_centroid_x = grid_params->min_x +
	((double) Layer[iz].sum_x / dnperlayer) * grid_params->dx; /* km */
      Layer[iz].vol_centroid_y = grid_params->min_y +
	((double) Layer[iz].sum_y / dnperlayer) * grid_params->dy; /* km */
	
      Layer[iz].refl_centroid_x = grid_params->min_x +
	(Layer[iz].sum_refl_x / Layer[iz].sum_refl) * grid_params->dx; /* km */
      Layer[iz].refl_centroid_y = grid_params->min_y +
	(Layer[iz].sum_refl_y / Layer[iz].sum_refl) * grid_params->dy; /* km */
	
      Layer[iz].dbz_mean = log10(Layer[iz].sum_refl / dnperlayer) * 10.0;
      
      Layer[iz].mass =
	(Layer[iz].sum_mass * dvol_at_centroid
	 * pow(Z_m_inverse_coeff, Z_m_inverse_exponent)); /* ktons */
      
      if (Glob->params.vel_available) {
	Layer[iz].vel_mean = Layer[iz].sum_vel / dnperlayer;
	Layer[iz].vel_sd = usdev(Layer[iz].sum_vel,
				 Layer[iz].sum_vel2, dnperlayer);
      }

    } /* if (Layer[iz].n ....... */
    
  } /* iz */

  /*
   * vil
   */

  vil_init();
  for (iz = 0; iz < Nz_valid; iz++) {
    if (Layer[iz].n > 0) {
      vil_add(Layer[iz].dbz_max, Glob->delta_z);
    } /* if (Layer[iz].n ....... */
  } /* iz */
  Gprops.vil_from_maxz = vil_compute();
  
  /*
   * dbz histograms
   */
  
  for (dbz_intvl = 0; dbz_intvl < Glob->n_dbz_hist_intervals; dbz_intvl++) {
    
    if (Dbz_hist[dbz_intvl].n_vol > 0) {
      Dbz_hist[dbz_intvl].percent_vol = 100.0 *
	(double) Dbz_hist[dbz_intvl].n_vol / dn;
      max_dbz_intvl = dbz_intvl;
    } else {
      Dbz_hist[dbz_intvl].percent_vol = 0.0;
    }
      
  } /* dbz_intvl */
    
  N_dbz_intvls = max_dbz_intvl + 1;
    
  /*
   * compute the azimuth and range from the radar to the layer centroids
   */
    
  for (iz = Base_layer; iz <= Top_layer; iz++) {
      
    if (Glob->radar_x == Layer[iz].vol_centroid_x &&
	Glob->radar_y == Layer[iz].vol_centroid_y)
      Layer[iz].vol_centroid_x += FUDGE_X;
      
    cent_x = Layer[iz].vol_centroid_x - Glob->radar_x;
    cent_y = Layer[iz].vol_centroid_y - Glob->radar_y;
      
    range = sqrt(cent_x * cent_x + cent_y * cent_y);
    Layer[iz].vol_centroid_range = range;
    if (range == 0.0)
      Layer[iz].vol_centroid_az = 0.0;
    else
      Layer[iz].vol_centroid_az = atan2(cent_y, cent_x);
      
  }

  return (0);

}

/***************************
 * compute_secondary_props()
 *
 * second pass through the clumps, computing vorticity and checking
 * range limits as required.
 */
  
static void compute_secondary_props(Clump_order *clump,
				    vol_file_handle_t *v_handle,
				    storm_ident_grid_t *grid_params)

{

  ui08 *vel_ptr = NULL;
  si32 index;
  si32 ix, iy, iz, jz, intv;
  
  double grid_x, grid_y, grid_z;
  double rel_x, rel_y, rel_z;
  double grid_az, delta_az;
  double gnd_range;
  double elev;
  double vel;
  double vorticity_dist;
  
  Interval *intvl;

  /*
   * initialize
   */

  for (intv = 0; intv < clump->size; intv++) {
    
    intvl = clump->ptr[intv];
    
    iz = intvl->plane;
    jz = iz + Glob->min_valid_layer;
    iy = intvl->row_in_plane;
    
    grid_y = (double) iy * grid_params->dy + grid_params->min_y;
    grid_z = (double) jz * Glob->delta_z + Glob->min_z;
    rel_y = grid_y - Glob->radar_y;
    rel_z = grid_z - Glob->radar_z;

    grid_x = (double) intvl->begin * grid_params->dx + grid_params->min_x;

    if (Glob->params.vel_available) {
      index = ((iy + grid_params->start_iy) * Glob->nx +
	       intvl->begin + grid_params->start_ix);
      vel_ptr = v_handle->field_plane[Glob->params.vel_field][jz] + index;
    }
    
    for (ix = intvl->begin; ix <= intvl->end;
	 ix++, grid_x += grid_params->dx, vel_ptr++) {
      
      /*
       * compute grid x and y
       */
      
      if (Glob->params.check_range_limits) {
      
	/*
	 * Test the range of the run end_points, and flag the storm if the
	 * range exceeds the test range - only perform this if not
	 * already flagged. Only (x, y) is considered relevant -
	 * z will have negligible effect on range computations at
	 * long range.
	 *
	 * Also, test the elevation angle of the run end_points, and flag
	 * the storm if the angle exceeds the test angle - only
	 * perform this if not already flagged.
	 * For these computations, the curvature of the earth and
	 * beam may be ignored, since range is short, and elevation
	 * angle high.
	 */
      
	if (ix == intvl->begin || ix == intvl->end) {
	
	  rel_x = grid_x - Glob->radar_x;
	
	  gnd_range = pow((rel_x * rel_x + rel_y * rel_y), 0.5);
	  
	  if (Test_range > 0.0 && !Range_limited) {
	    
	    if (gnd_range > Test_range)
	      Range_limited = TRUE;
	    else if (ix == 0)
	      Range_limited = TRUE;
	    else if (ix == Glob->nx - 1)
	      Range_limited = TRUE;
	    else if (iy == 0)
	      Range_limited = TRUE;
	    else if (iy == Glob->ny - 1)
	      Range_limited = TRUE;
	    
	    if (Glob->params.debug >= DEBUG_VERBOSE && Range_limited) {
	      fprintf(stderr,
		      "***** Range limited, x, y, range = "
		      "%g, %g, %g\n",
		      grid_x, grid_y, gnd_range);
	    }
	  
	  } /* if (!Range_limited) */

	  if (Test_elev > 0.0 && !Top_missing) {
	  
	    if (rel_z == 0.0 && gnd_range == 0.0)
	      elev = 0.0;
	    else
	      elev = atan2(rel_z, gnd_range);
	  
	    if (elev > Test_elev) {
	    
	      Top_missing = TRUE;
	    
	      if (Glob->params.debug >= DEBUG_VERBOSE) {
		fprintf(stderr,
			"** Top missing,x,y,z, elev = "
			"%g,%g,%g,%g\n",
			grid_x, grid_y, grid_z,
			elev * RAD_TO_DEG);
	      }
	    
	    } /* if (elev > Test_elev) */
	  
	  } /* if (!Top_missing) */
	
	} /* if (ix == intvl->begin ... */
	
      } /* if (Glob->params.check_range_limits) */
	
      if (Glob->params.vel_available) {

	/*
	 * compute grid azimuth from radar,
	 * and delta azimuth from vol_centroid azimuth
	 */
	
	if (grid_x == Glob->radar_x && grid_y == Glob->radar_y) {
	  grid_az = 0.0;
	} else {
	  grid_az = atan2(grid_y - Glob->radar_y, grid_x - Glob->radar_x);
	}

	delta_az = Layer[iz].vol_centroid_az - grid_az;

	if (delta_az > M_PI) {
	  delta_az -= TWO_PI;
	}
	
	if (delta_az < M_PI) {
	  delta_az += TWO_PI;
	}
	
	vorticity_dist = Layer[iz].vol_centroid_range * sin(delta_az);
	
	if (fabs(vorticity_dist) >= Min_vorticity_dist) {
	  
	  vel = *vel_ptr * Vel_scale + Vel_bias;
	  Layer[iz].n_vorticity++;
	  Layer[iz].sum_vorticity +=
	    (vel - Layer[iz].vel_mean) / (vorticity_dist * 1000.0);

	} /* if (fabs .... */
	
      } /* if (Glob->vel-available) */
      
    } /* ix */
    
  } /* intv */
  
  if (Glob->params.vel_available) {
    
    /*
     * compute the vorticity at each layer and for the entire volume
     */
    
    for (iz = Base_layer; iz <= Top_layer; iz++) {
      
      Sum.vorticity += Layer[iz].sum_vorticity;
      Sum.n_vorticity += Layer[iz].n_vorticity;
      
      if (Layer[iz].n_vorticity > 0) {
	Layer[iz].vorticity = ((Layer[iz].sum_vorticity /
				(double) Layer[iz].n_vorticity) *
			       VORTICITY_MODEL_FACTOR *
			       Vorticity_hemisphere_factor);
      } else {
	Layer[iz].vorticity = 0.0;
      }
      
    }
    
    if (Sum.n_vorticity > 0) {
      Gprops.vorticity =
	((Sum.vorticity / (double) Sum.n_vorticity) *
	 VORTICITY_MODEL_FACTOR * Vorticity_hemisphere_factor);
    } else {
      Gprops.vorticity = 0.0;
    }
    
  } else {
    
    for (iz = Base_layer; iz <= Top_layer; iz++)
      Layer[iz].vorticity = 0.0;
    
    Gprops.vorticity = 0.0;
    
  } /* if (Glob->vel_available) */

  return;

}
      
/**************
 * store_runs()
 */

static si32 store_runs(Clump_order *clump,
		       storm_file_handle_t *s_handle,
		       storm_ident_grid_t *grid_params)

{

  int intv;
  int start_ix = grid_params->start_ix;
  int start_iy = grid_params->start_iy;
  Interval *intvl;
  storm_file_run_t *run;
  
  RfAllocStormProps(s_handle, Glob->nz,
		    Glob->n_dbz_hist_intervals,
		    clump->size, 0L,
		    "props_compute");
  
  run = s_handle->runs;
  
  for (intv = 0; intv < clump->size; intv++, run++) {
    
    intvl = clump->ptr[intv];
    
    run->ix = intvl->begin + start_ix;
    run->iy = intvl->row_in_plane + start_iy;
    run->iz = intvl->plane + Glob->min_valid_layer;
    run->n = intvl->len;
	
  } /* intv */

  return (clump->size);

}

