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
 * sampling.c
 *
 * Samples a dobson file to simulate radar data
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder CO
 *
 * September 1995
 *********************************************************************/

#include <errno.h>
#include <time.h>
#include <rapmath/umath.h>
#include "test2gate.h"

#define PSEUDO_RADIUS 8533.0

static vol_file_handle_t V_Handle;
static time_t start_time;
static double *Cosphi, *Sinphi;
static double **Beam_ht, **Gnd_range;
static double Minx, Miny, Minz, Dx, Dy, Dz;
static si32 Nx, Ny, Nz;
static ui08 *Noise;
static int Ngates;
static double Vel_scale, Vel_bias;

static void setup_noise_array(void);

void init_sampling(void)

{

  cart_params_t *cart;
  field_params_t *vel_params;
  
  /*
   * initialize volume file handle
   */

  RfInitVolFileHandle(&V_Handle,
		      Glob->prog_name,
		      Glob->params.radar_sample_file_path,
		      (FILE *) NULL);

  /*
   * read in radar volume
   */

  if (RfReadVolume(&V_Handle, "init_sampling") != R_SUCCESS) {
    tidy_and_exit(-1);
  }

  /*
   * get start time
   */

  start_time = time(NULL);

  /*
   * set statics
   */

  cart = &V_Handle.vol_params->cart;

  Nx = cart->nx;
  Ny = cart->ny;
  Nz = cart->nz;

  Dx = (double) cart->dx / (double) cart->km_scalex;
  Dy = (double) cart->dy / (double) cart->km_scaley;
  Dz = (double) cart->dz / (double) cart->km_scalez;

  Minx = (double) cart->minx / (double) cart->km_scalex;
  Miny = (double) cart->miny / (double) cart->km_scaley;
  Minz = (double) cart->minz / (double) cart->km_scalez;

  setup_noise_array();

  if (Glob->params.output_vel_field) {
    vel_params = V_Handle.field_params[Glob->params.sample_vel_field];
    Vel_scale = (double) vel_params->scale / (double) vel_params->factor;
    Vel_bias = (double) vel_params->bias / (double) vel_params->factor;
  }

}

vol_file_handle_t *get_sampling_vol_index(void)

{
  return (&V_Handle);
}

void calc_sampling_geom(scan_table_t *scan_table)

{

  int ielev, igate;
  int nelevs = scan_table->nelevations;
  int ngates = scan_table->ngates;
  
  double radar_altitude;
  double twice_radius;
  double slant_range;

  twice_radius = 2.0 * PSEUDO_RADIUS;
  radar_altitude = Glob->params.radar_params.altitude / 1000.0;
  
  /*
   * allocate memory
   */

  Sinphi = (double *) umalloc((ui32) (nelevs * sizeof(double)));
  Cosphi = (double *) umalloc((ui32) (nelevs * sizeof(double)));

  Gnd_range = (double **) ucalloc2
    ((ui32) nelevs, (ui32) ngates, sizeof(double));

  Beam_ht = (double **) ucalloc2
    ((ui32) nelevs, ngates, sizeof(double));

  for (ielev = 0; ielev < nelevs; ielev++) {

    Sinphi[ielev] = sin(scan_table->elev_angles[ielev] * DEG_TO_RAD);
    Cosphi[ielev] = cos(scan_table->elev_angles[ielev] * DEG_TO_RAD);

    slant_range = scan_table->start_range;

    for (igate = 0; igate < scan_table->ngates; igate++) {

      Gnd_range[ielev][igate] = slant_range * Cosphi[ielev];

      Beam_ht[ielev][igate] =
	(radar_altitude + slant_range * Sinphi[ielev] +
	 slant_range * slant_range / twice_radius);

      slant_range += scan_table->gate_spacing;

    } /* igate */
    
  } /* ielev */
  
}

#ifndef RAND_MAX
#define RAND_MAX 2147483000
#endif

void sample_dbz_and_vel(ui08 *byte_ptr,
			scan_table_t *scan_table,
			int curr_elev, int curr_az)

{

  ui08 *dbz, *vel;
  ui08 **dbz_field, **vel_field;
  ui08 this_dbz;
  ui08 relv_byte;

  int igate;
  int ix, iy, iz;
  int index_xy;
  int output_vel_field;
  int irelv;
  static int count = 0;

  double grange, xsample, ysample, zsample;
  double move_x, move_y;
  double x0, y0;
  double dhr;
  double az;
  double relv;
  double dist;

  time_t now;
  scan_table_elev_t *elev;
  test2gate_sampling_origin *origin;

  /*
   * compute the sampling origin
   */

  now = time(NULL);
  dhr = (now - start_time) / 3600.0;

  origin = &Glob->params.sampling_origin;
  dist = origin->speed * dhr;
  dist = fmod(dist, origin->max_dist);
  move_x = dist * sin(origin->dirn * DEG_TO_RAD);
  move_y = dist * cos(origin->dirn * DEG_TO_RAD);

  x0 = origin->start_x + move_x;
  y0 = origin->start_y + move_y;

  elev = scan_table->elevs + curr_elev;
  az = elev->azs[curr_az].angle;

  /*
   * relative velocity
   */
  
  if (Glob->params.override_vel) {
    relv = (origin->speed * cos((az - origin->dirn) * DEG_TO_RAD)) / (-3.6);
    irelv = (int) ((relv - Vel_bias) / Vel_scale + 0.5);
    irelv = MIN(irelv, 254);
    irelv = MAX(irelv, 0);
    relv_byte = irelv;
  }

  if (Glob->params.debug) {
    if (count == 0) {
      fprintf(stderr, "speed, dirn: %g, %g\n", origin->speed, origin->dirn);
      fprintf(stderr, "vel bias, scale: %g, %g\n", Vel_bias,  Vel_scale);
      fprintf(stderr, "elev, az, originx, originy, relv, irelv: "
	      "%g, %g, %.1f, %.1f, %f, %d\n",
	      scan_table->elev_angles[curr_elev], az, x0, y0, relv, irelv);
    } else {
      count++;
      if (count == 360) {
	count = 0;
      }
    }
  }

  dbz = byte_ptr;
  vel = byte_ptr + Ngates;
  dbz_field = V_Handle.field_plane[Glob->params.sample_dbz_field];
  vel_field = V_Handle.field_plane[Glob->params.sample_vel_field];
  output_vel_field = Glob->params.output_vel_field;

  for (igate = 0; igate < Ngates; igate++, dbz++, vel++) {

    grange = Gnd_range[curr_elev][igate];
    xsample = x0 + grange * sin(az * DEG_TO_RAD);
    ysample = y0 + grange * cos(az * DEG_TO_RAD);
    zsample = Beam_ht[curr_elev][igate];

    ix = (int) ((xsample - Minx) / Dx + 0.5);
    iy = (int) ((ysample - Miny) / Dy + 0.5);
    iz = (int) ((zsample - Minz) / Dz + 0.5);

    if (ix >= 0 && ix < Nx && iy >= 0 && iy < Ny && iz >= 0 && iz < Nz) {
      index_xy = iy * Nx + ix;
      this_dbz = dbz_field[iz][index_xy];
      if (this_dbz >= Noise[igate]) {
	*dbz = this_dbz;
      } else {
	*dbz = Noise[igate];
      }
      if (output_vel_field) {
	if (this_dbz >= Noise[igate]) {
	  if (Glob->params.override_vel) {
	    *vel = relv_byte;
	  } else {
	    *vel = vel_field[iz][index_xy];
	  }
	} else {
	  *vel = (int) (((double) rand() / (double) RAND_MAX) * 250);
	}
      }
    }
    
  } /* igate */

}

static void setup_noise_array(void)

{

  int igate;
  int dbz_level;
  double range, noise_dbz;
  double dbz_scale, dbz_bias;
  vol_file_handle_t *v_handle;
  field_params_t *dbz_params;

  Ngates = Glob->params.radar_params.num_gates;

  Noise = (ui08 *) umalloc(Ngates * sizeof(ui08));
  
  v_handle = get_sampling_vol_index();
  dbz_params = v_handle->field_params[Glob->params.sample_dbz_field];
  dbz_bias = (double) dbz_params->bias / (double) dbz_params->factor;
  dbz_scale = (double) dbz_params->scale / (double) dbz_params->factor;
  
  for (igate = 0; igate < Ngates; igate++) {
    
    range = (Glob->params.radar_params.start_range +
	     igate * Glob->params.radar_params.gate_spacing) / 1000.0;
    
    noise_dbz = (Glob->params.noise_dbz_at_100km +
		 20.0 * (log10(range) - log10(100.0)));
    
    dbz_level = (int) ((noise_dbz - dbz_bias) / dbz_scale + 0.5);
    
    if (dbz_level < 0)
      dbz_level = 0;
    if (dbz_level > 255)
      dbz_level = 255;

    Noise[igate] = dbz_level;
    
  } /* igate */
  
}
