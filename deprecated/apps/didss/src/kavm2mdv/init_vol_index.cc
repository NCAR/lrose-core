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
 * init_vol_index.cc
 *
 * Initialize the index which we use to write the dobson file
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1997
 *
 * Nancy Rehak
 *
 * Taken from init_vol_index.c by Mike Dixon
 *
 *********************************************************************/

#include <rapformats/km.h>

#include "kavm2mdv.h"
using namespace std;

#define LAT_TO_KM 111.12

void init_vol_index(vol_file_handle_t *v_handle)
{
  int iplane;
  int nx, ny;
  double minz, dz;
  double scale, bias;
  double minx, dx;
  double miny, dy;
  cart_params_t *cart;
  field_params_t *fparams;

  /*
   * initialize
   */

  RfInitVolFileHandle(v_handle,
		      Glob->prog_name,
		      (char *) NULL,
		      (FILE *) NULL);

  /*
   * allocate memory for vol params
   */

  RfAllocVolParams(v_handle, "init_vol_index");

  /*
   * set vol params
   */

  memset((void *) v_handle->vol_params,
	 0, sizeof(vol_params_t));
  
  sprintf(v_handle->vol_params->note, "%s\n",
	  "Dobson file based on radar mosaic");

  /*
   * set cart params
   */

  if (Glob->params.resample_grid)
  {
    minx = Glob->params.x_grid.min;
    dx = Glob->params.x_grid.delta;
    nx = Glob->params.x_grid.n;

    miny = Glob->params.y_grid.min;
    dy = Glob->params.y_grid.delta;
    ny = Glob->params.y_grid.n;
  }
  else
  {
    minx = KAV_MINLON;
    dx = KAV_DLON;
    nx = KAV_NLON;

    miny = KAV_MINLAT;
    dy = KAV_DLAT;
    ny = KAV_NLAT;
  }

  cart = &v_handle->vol_params->cart;

  switch (Glob->params.output_projection)
  {
  case PROJ_LATLON :
  {
    double mean_lat;
    double latitude_factor;
    double origin_lat, origin_lon;

    cart->scalex = 10000000;
    cart->scaley = 10000000;
    cart->scalez = 10000;

    mean_lat = miny + dy * (ny / 2);
    latitude_factor = cos(mean_lat * DEG_TO_RAD);
  
    cart->km_scalex =
      (int)((cart->scalex / LAT_TO_KM) * latitude_factor);
    cart->km_scaley = (int)(cart->scaley / LAT_TO_KM);
    cart->km_scalez = 10000;

    origin_lat = (miny + dy / 2.0);
    origin_lon = (minx + dx / 2.0);
    cart->latitude = (si32) (origin_lat * 1000000.0 + 0.5);
    cart->longitude = (si32) (origin_lon * 1000000.0 + 0.5);
    cart->rotation = 0;

    cart->nx = nx;
    cart->ny = ny;
    cart->nz = 1;

    cart->dx = (si32) (dx * cart->scalex + 0.5);
    cart->dy = (si32) (dy * cart->scaley + 0.5);
    dz = 1.0;
    cart->dz = (si32) (dz * cart->scalez + 0.5);

    cart->minx = (si32) ((minx + dx * 0.5) * cart->scalex + 0.5);
    cart->miny = (si32) ((miny + dy * 0.5) * cart->scaley + 0.5);
    minz = 0.5;
    cart->minz = (si32) (minz * cart->scalez + 0.5);

    cart->dz_constant = TRUE;

    strncpy(cart->unitsx, "Deg Lon", R_LABEL_LEN);
    strncpy(cart->unitsy, "Deg Lat", R_LABEL_LEN);
    strncpy(cart->unitsz, "km", R_LABEL_LEN);
  }
  break;

  case PROJ_FLAT :
  {
    cart->scalex = 10000;
    cart->scaley = 10000;
    cart->scalez = 10000;

    cart->km_scalex = cart->scalex;
    cart->km_scaley = cart->scaley;
    cart->km_scalez = cart->scalez;

    cart->latitude = (si32) (Glob->params.flat_origin.lat * 1000000.0 + 0.5);
    cart->longitude = (si32) (Glob->params.flat_origin.lon * 1000000.0 + 0.5);
    cart->rotation = 0;

    cart->nx = nx;
    cart->ny = ny;
    cart->nz = 1;

    cart->dx = (si32) (dx * cart->scalex + 0.5);
    cart->dy = (si32) (dy * cart->scaley + 0.5);
    dz = 1.0;
    cart->dz = (si32) (dz * cart->scalez + 0.5);

    cart->minx = (si32) ((minx + dx * 0.5) * cart->scalex + 0.5);
    cart->miny = (si32) ((miny + dy * 0.5) * cart->scaley + 0.5);
    minz = 0.5;
    cart->minz = (si32) (minz * cart->scalez + 0.5);

    cart->dz_constant = TRUE;

    strncpy(cart->unitsx, "km", R_LABEL_LEN);
    strncpy(cart->unitsy, "km", R_LABEL_LEN);
    strncpy(cart->unitsz, "km", R_LABEL_LEN);
  }
  break;

  } /* endswitch - Glob->params.output_projection */

  /*
   * set number of fields
   */

  v_handle->vol_params->nfields = 1;

  /*
   * allocate the arrays in the index
   */

  RfAllocVolArrays(v_handle, "init_vol_index");

  /*
   * set field params
   */

  fparams = v_handle->field_params[0];
  memset((void*) fparams, 0, sizeof(field_params_t));

  scale = 0.5;
  bias = -30.0;
  
  fparams->encoded = TRUE;
  fparams->factor = 10000;
  fparams->scale = (si32) (scale * fparams->factor + 0.5);
  fparams->bias = (si32) (bias * fparams->factor + 0.5);
  strncpy(fparams->transform, "dB", R_LABEL_LEN);
  strncpy(fparams->name, "Reflectivity", R_LABEL_LEN);
  strncpy(fparams->units, "dBZ", R_LABEL_LEN);

  /*
   * initialize lookup table for translating vip levels to dbz
   */

  KM_vip2dbz_init(scale, bias,
		  Glob->params.vip2dbz.val, Glob->params.vip2dbz.len,
		  Glob->params.output_type,
		  Glob->params.dbz_threshold);
  
  /*
   * set plane heights
   */

  for (iplane = 0; iplane < cart->nz; iplane++)
  {
    v_handle->plane_heights[iplane][PLANE_BASE_INDEX] = (si32)
      ((minz + ((double) iplane - 0.5) * dz) * cart->scalez + 0.5);
    
    v_handle->plane_heights[iplane][PLANE_MIDDLE_INDEX] = (si32)
      ((minz + ((double) iplane) * dz) * cart->scalez + 0.5);
    
    v_handle->plane_heights[iplane][PLANE_TOP_INDEX] = (si32)
      ((minz + ((double) iplane + 0.5) * dz) * cart->scalez + 0.5);
    
  } /* iplane */
  
}

