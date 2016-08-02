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
 * dobson.c
 *
 * Handle the dobson files
 *
 * RAP, NCAR, Boulder CO
 *
 * Nov 1996
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "dva_cart.h"
#include <titan/radar.h>

static vol_file_handle_t Vhandle;
static int Npoints_cappi;

/***************
 * init_dobson()
 *
 * Initialize dobson handle
 */

void init_dobson(dva_grid_t *grid,
		 dva_rdas_cal_t *cal,
		 int n_elev,
		 double *elevations)

{

  int i, iz;
  double scalez, minz, dz;
  cart_params_t *cart;
  field_params_t *fparams;
  radar_params_t *rparams;

  Npoints_cappi = grid->ny * grid->nx;

  RfInitVolFileHandle(&Vhandle,
		      Glob->prog_name,
		      NULL,
		      (FILE *) NULL);

  if (RfAllocVolParams(&Vhandle,
		       "init_dobson")) {
    tidy_and_exit(-1);
  }
    
  /*
   * set radar params
   */

  rparams = &Vhandle.vol_params->radar;

  rparams->altitude = (Glob->params.radar_altitude * 1000 + 0.5);
  rparams->latitude = (Glob->params.radar_latitude * 1000000 + 0.5);
  rparams->longitude = (Glob->params.radar_longitude * 1000000 + 0.5);
  rparams->nelevations = n_elev;
  rparams->nazimuths = 360;
  rparams->ngates = cal->ngates;
  rparams->gate_spacing = (cal->gate_spacing * 1000000 + 0.5);
  rparams->start_range = (cal->start_range * 1000000 + 0.5);
  rparams->delta_azimuth = 1000000;
  rparams->start_azimuth = 0;
  rparams->beam_width = (Glob->params.beam_width * 1000000 + 0.5);
  rparams->samples_per_beam = Glob->params.samples_per_beam;
  rparams->pulse_width = (Glob->params.pulse_width * 1000 + 0.5);
  rparams->prf = (Glob->params.prf * 1000 + 0.5);
  rparams->wavelength = (Glob->params.wavelength * 10000 + 0.5);
  rparams->nmissing = 0;

  STRncopy(rparams->name, Glob->params.radar_name, R_LABEL_LEN);

  /*
   * set cart parameters
   */
  
  cart = &Vhandle.vol_params->cart;

  cart->latitude = rparams->latitude;
  cart->longitude = rparams->longitude;
  cart->rotation = 0;

  cart->nx = grid->nx;
  cart->ny = grid->ny;
  cart->nz = grid->nz;

  cart->scalex = 1000;
  cart->scaley = 1000;
  cart->scalez = 1000;

  cart->km_scalex = 1000;
  cart->km_scaley = 1000;
  cart->km_scalez = 1000;

  cart->dx = (grid->dx * cart->scalex + 0.5);
  cart->dy = (grid->dy * cart->scaley + 0.5);
  cart->dz = (grid->dz * cart->scalez + 0.5);

  cart->minx =
    -((((grid->nx - 1) / 2.0) * grid->dx) * cart->scalex + 0.5);

  cart->miny =
    -((((grid->ny - 1) / 2.0) * grid->dy) * cart->scaley + 0.5);

  cart->minz =
    (grid->minz * cart->scalez + 0.5);

  cart->radarx = 0;
  cart->radary = 0;
  cart->radarz = (Glob->params.radar_altitude * cart->scalez + 0.5);

  cart->dz_constant = TRUE;

  strcpy(cart->unitsx, "km");
  strcpy(cart->unitsy, "km");
  strcpy(cart->unitsz, "km");

  /*
   * number of fields
   */

  Vhandle.vol_params->nfields = 1;
  
  /*
   * set the note
   */
  
  sprintf(Vhandle.vol_params->note,
	  "Cartesian transformation using DISPLACE VARIATE AVERAGING\n");
  
  /*
   * allocate the arrays for the vol file handle
   */

  if (RfAllocVolArrays(&Vhandle,
		       "init_dobson")) {
    tidy_and_exit(-1);
  }

  /*
   * set radar elevations
   */

  for (i = 0; i < rparams->nelevations; i++) {
    Vhandle.radar_elevations[i] =
      (elevations[i] * 1000000 + 0.5);
  }

  /*
   * set plane heights
   */

  scalez = (double) cart->scalez;
  minz = (double) cart->minz / scalez;     
  dz = (double) cart->dz / scalez;
  
  for (iz = 0; iz < cart->nz; iz++) {
    
    Vhandle.plane_heights[iz][PLANE_BASE_INDEX] = (si32)
      ((minz + ((double) iz - 0.5) * dz) * scalez + 0.5);
    
    Vhandle.plane_heights[iz][PLANE_MIDDLE_INDEX] = (si32)
      ((minz + ((double) iz) * dz) * scalez + 0.5);
    
    Vhandle.plane_heights[iz][PLANE_TOP_INDEX] = (si32)
      ((minz + ((double) iz + 0.5) * dz) * scalez + 0.5);
    
  } /* iz */

  /*
   * set the field params
   */

  fparams = Vhandle.field_params[0];
  fparams->encoded = TRUE;
  fparams->factor = 10000;
  fparams->scale = 5000;
  fparams->bias = -300000;
  fparams->missing_val = 0;
  fparams->noise = 0;
  strcpy(fparams->name, "Refl");
  strcpy(fparams->units, "dBZ");
  strcpy(fparams->transform, "dBZ");

  /*
   * allocate field data
   */
  
  for (iz = 0; iz < cart->nz; iz++) {
    Vhandle.field_plane[0][iz] = (ui08 *) umalloc(Npoints_cappi);
  }

}

/***************
 * load_dobson()
 *
 * Load a cappi plane into the dobson handle
 */


void load_dobson(int iz, ui08 *cappi)

{

  memcpy(Vhandle.field_plane[0][iz], cappi, Npoints_cappi);

}

/****************
 * write_dobson()
 *
 * Write the dobson file.
 *
 * returns 0 on success, -1 on failure.
 */

int write_dobson(int radar_id, si32 start_time, si32 end_time)
  
{

  si32 mid_time;

  /*
   * set radar id
   */

  Vhandle.vol_params->radar.radar_id = radar_id;

  /*
   * set times
   */
  
  mid_time = (si32) (((double) start_time + (double) end_time) / 2.0);

  Rfutime2rtime(start_time, &Vhandle.vol_params->start_time);
  Rfutime2rtime(mid_time, &Vhandle.vol_params->mid_time);
  Rfutime2rtime(end_time, &Vhandle.vol_params->end_time);

  /*
   * write the file
   */
  
  if (RfWriteDobson(&Vhandle, TRUE,
		    Glob->params.debug,
		    Glob->params.output_dir,
		    "mdv", Glob->prog_name,
		    "dobson_write")) {
    return(-1);
  } else {
    return(0);
  }

}

