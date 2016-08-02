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
 * load_data_grid.cc
 *
 * Loads the filtered data grid
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1997
 *
 * Nancy Rehak
 *
 * Based on load_data_grid.c by Mike Dixon
 *
 *********************************************************************/

#include <toolsa/pjg_flat.h>
#include <rapformats/km.h>

#include "kavm2mdv.h"
using namespace std;

static void load_native_grid(vol_file_handle_t *v_handle,
			     char *file_buf,
			     int file_size);

static void load_resampled_grid(vol_file_handle_t *v_handle,
				char *file_buf,
				int file_size,
				double coverage_threshold,
				kavm2mdv_filter_type_t filter_type);

static void load_flat_grid(vol_file_handle_t *v_handle,
			   char *file_buf,
			   int file_size);

void load_data_grid(vol_file_handle_t *v_handle,
		    char *file_buf,
		    int file_size)
{
  if (Glob->params.resample_grid)
  {
    if (Glob->params.output_projection == PROJ_LATLON)
      load_resampled_grid(v_handle, file_buf, file_size,
			  Glob->params.coverage_threshold,
			  Glob->params.filter_type);
    else
      load_flat_grid(v_handle, file_buf, file_size);
  }
  else
  {
    load_native_grid(v_handle, file_buf, file_size);
  }

  return;
}


static void load_resampled_grid(vol_file_handle_t *v_handle,
				char *file_buf,
				int file_size,
				double coverage_threshold,
				kavm2mdv_filter_type_t filter_type)
{
  static int first_call = TRUE;
  static ui08 *data_grid, *dg;
  static si32 npoints;
  static si32 *xindex;
  static double *nhits, *nh;
  static double *npossible, *np;
  static double *sum, *z_sum, *su;
  static double z_table[256];
  static double scale = (double)v_handle->field_params[0]->scale /
    (double)v_handle->field_params[0]->factor;
  static double bias = (double)v_handle->field_params[0]->bias /
    (double)v_handle->field_params[0]->factor;
      
  
  ui08 *line;
  si32 ilat, ilon;
  si32 i, ix, iy;
  si32 index, start_index;
  si32 mean_val;
  double lon, lat;
  double minx, miny;
  double dx, dy;
  double fraction_covered;
  cart_params_t *cart;

  cart = &v_handle->vol_params->cart;
  
  minx = (double) cart->minx / (double) cart->scalex;
  miny = (double) cart->miny / (double) cart->scaley;
  dx = (double) cart->dx / (double) cart->scalex;
  dy = (double) cart->dy / (double) cart->scaley;

  /*
   * allocate on first call
   */

  if (first_call)
  {
    /*
     * allocate
     */

    npoints = cart->ny * cart->nx;
    data_grid = (ui08 *) umalloc ((ui32) (npoints * sizeof(ui08)));
    sum = (double *) umalloc ((ui32) (npoints * sizeof(double)));
    z_sum = (double *) umalloc ((ui32) (npoints * sizeof(double)));
    nhits = (double *) umalloc ((ui32) (npoints * sizeof(double)));
    npossible = (double *) umalloc ((ui32) (npoints * sizeof(double)));
    xindex = (si32 *) umalloc ((ui32) (KAV_NLON * sizeof(si32)));
    
    /*
     * load up x index array
     */
    
    lon = KAV_MINLON + KAV_DLON * 0.5;
    for (ilon = 0; ilon < KAV_NLON; ilon++, lon += KAV_DLON)
    {
      ix = (si32) ((lon - minx) / dx + 0.5);
      if (ix >= 0 && ix < cart->nx)
      {
	xindex[ilon] = ix;
      }
      else
      {
	xindex[ilon] = -1;
      }
    } /* ilon */

    /*
     * Load up the Z table
     */

    for (i = 0; i < 255; i++)
    {
      z_table[i] = pow(10.0,
		       (((double)i * scale) + bias) / 10.0);
    } /* endfor - i */
    
    first_call = FALSE;
  }
  
  /*
   * initialize
   */
  
  KM_unpack_init(file_buf, file_size);
  memset((void *) data_grid, 0, (int) (npoints * sizeof(ui08)));
  memset((void *) sum, 0, (int) (npoints * sizeof(double)));
  memset((void *) z_sum, 0, (int) (npoints * sizeof(double)));
  memset((void *) nhits, 0, (int) (npoints * sizeof(double)));
  memset((void *) npossible, 0, (int) (npoints * sizeof(double)));

  /*
   * Unpack data lines, and accumulate stats.
   * Data starts at top of grid, and works down.
   */
  
  lat = KAV_MINLAT + KAV_DLAT * (KAV_NLAT - 0.5);
  
  for (ilat = 0; ilat < KAV_NLAT; ilat++, lat -= KAV_DLAT)
  {
    /*
     * compute y index
     */
    
    iy = (si32) ((lat - miny) / dy + 0.5);
    start_index = iy * cart->nx;
    
    /*
     * unpack line
     */

    line = KM_unpack_raw_line();

    if (line == NULL)
    {
      fprintf(stderr, "ERROR - null line %d\n", i);
    }
    else if (iy >= 0 && iy < cart->ny)
    {
      /*
       * accumulate the grid data stats
       */

      for (ilon = 0; ilon < KAV_NLON; ilon++)
      {
	if (xindex[ilon] >= 0)
	{
	  index = start_index + xindex[ilon];
	  if (line[ilon] > 0)
	  {
	    switch (filter_type)
	    {
	    case MAX_FILTER :
	      data_grid[index] = MAX(data_grid[index], line[ilon]);
	      break;

	    case MEAN_DBZ_FILTER :
	      sum[index] += (double) line[ilon];
	      break;
	      
	    case MEAN_Z_FILTER :
	      z_sum[index] += z_table[line[ilon]];
	      break;
	      
	    } /* endswitch - filter_type */
	    nhits[index] += 1.0;
	  }
	  npossible[index] += 1.0;
	}
      } /* ilon */
	   
    } /* if (iy >= 0 && iy < cart->ny) */
    
  } /* ilat */

  /*
   * load up data grid array
   */

  switch (filter_type)
  {
  case MAX_FILTER:

    dg = data_grid;
    nh = nhits;
    np = npossible;
    
    for (i = 0; i < npoints; i++, dg++, nh++, np++)
    {
      fraction_covered = *nh / *np;
      if (fraction_covered < coverage_threshold)
      {
	*dg = 0;
      }
    } /* i */

    break;

  case MEAN_DBZ_FILTER:

    dg = data_grid;
    su = sum;
    nh = nhits;
    np = npossible;
    
    for (i = 0; i < npoints; i++, dg++, su++, nh++, np++)
    {
      fraction_covered = *nh / *np;
      
      if (fraction_covered >= coverage_threshold)
      {
	mean_val = (si32) (*su / *nh + 0.5);
	if (mean_val < 0)
	{
	  mean_val = 0;
	}
	else if (mean_val > 255)
	{
	  mean_val = 255;
	}
	*dg = mean_val;
	
      } /* if (fraction_covered >= coverage_threshold) */
      
    } /* i */

    break;

  case MEAN_Z_FILTER:

    dg = data_grid;
    su = z_sum;
    nh = nhits;
    np = npossible;
    
    for (i = 0; i < npoints; i++, dg++, su++, nh++, np++)
    {
      fraction_covered = *nh / *np;
      
      if (fraction_covered >= coverage_threshold)
      {
	mean_val =
	  (si32) (((10.0 * log10(*su / *nh) - bias) / scale) + 0.5);
	if (mean_val < 0)
	{
	  mean_val = 0;
	}
	else if (mean_val > 255)
	{
	  mean_val = 255;
	}
	*dg = mean_val;
	
      } /* if (fraction_covered >= coverage_threshold) */
      
    } /* i */

    break;

  default:
    break;

  } /* switch */

  /*
   * set pointer to data grid
   */

  v_handle->field_plane[0][0] = data_grid;
  
  return;
}


static void load_flat_grid(vol_file_handle_t *v_handle,
			   char *file_buf,
			   int file_size)
{
  static int first_call = TRUE;
  static ui08 *data_grid;
  static ui08 *kav_grid;
  static int *index_grid;
  static si32 npoints;

  ui08 *line;
  si32 i, ix, iy;
  double minx, miny;
  double dx, dy;

  cart_params_t *cart;

  cart = &v_handle->vol_params->cart;
  
  minx = (double) cart->minx / (double) cart->scalex;
  miny = (double) cart->miny / (double) cart->scaley;
  dx = (double) cart->dx / (double) cart->scalex;
  dy = (double) cart->dy / (double) cart->scaley;

  /*
   * allocate on first call
   */

  if (first_call)
  {
    /*
     * allocate
     */

    npoints = cart->ny * cart->nx;
    data_grid = (ui08 *) umalloc ((ui32) (npoints * sizeof(ui08)));
    kav_grid = (ui08 *) umalloc ((ui32) (KAV_NLAT * KAV_NLON * sizeof(ui08)));
    index_grid = (int *) umalloc ((ui32) (npoints * sizeof(int)));
    
    /*
     * load up index array
     */
    
    PJGflat_init(Glob->params.flat_origin.lat,
		 Glob->params.flat_origin.lon,
		 0.0);

    i = 0;
    for (iy = 0; iy < cart->ny; iy++)
    {
      for (ix = 0; ix < cart->nx; ix++)
      {
	double grid_centroid_x, grid_centroid_y;
	double grid_centroid_lat, grid_centroid_lon;
	int kav_x, kav_y;

	grid_centroid_x = minx + ix * dx;
	grid_centroid_y = miny + iy * dy;

	PJGflat_xy2latlon(grid_centroid_x, grid_centroid_y,
			  &grid_centroid_lat, &grid_centroid_lon);

	kav_x = (int)((grid_centroid_lon - KAV_MINLON) / KAV_DLON);
	kav_y = (int)((grid_centroid_lat - KAV_MINLAT) / KAV_DLAT);

	if (kav_x < 0 || kav_x >= KAV_NLON ||
	    kav_y < 0 || kav_y >= KAV_NLAT)
	  index_grid[i] = -1;
	else
	  index_grid[i] = (kav_y * KAV_NLON) + kav_x;

	i++;
      } /* endfor - ix */
    } /* endfor - iy */

    first_call = FALSE;
  }
  
  /*
   * initialize
   */
  
  KM_unpack_init(file_buf, file_size);
  memset((void *) data_grid, 0, (int) (npoints * sizeof(ui08)));
  memset((void *) kav_grid, 0, (int) (KAV_NLAT * KAV_NLON * sizeof(ui08)));

  /*
   * Unpack data lines, and accumulate Kavouras grid.
   * Data starts at top of grid, and works down.
   */
  
  for (iy = KAV_NLAT - 1; iy >= 0; iy--)
  {
    line = KM_unpack_raw_line();

    memcpy(kav_grid + iy * KAV_NLON, line, KAV_NLON);
  } /* endfor - iy */

  /*
   * load up data grid array
   */

  for (i = 0; i < npoints; i++)
  {
    if (index_grid[i] >= 0)
      data_grid[i] = kav_grid[index_grid[i]];
    else
      data_grid[i] = 0;
  } /* endfor - i */

  /*
   * set pointer to data grid
   */

  v_handle->field_plane[0][0] = data_grid;
  
  return;
}


static void load_native_grid(vol_file_handle_t *v_handle,
			     char *file_buf,
			     int file_size)
{
  static int first_call = TRUE;
  static ui08 *data_grid, *dp;
  ui08 *line;
  int delta_dp;
  si32 i, npoints;
  cart_params_t *cart;

  cart = &v_handle->vol_params->cart;
  npoints = cart->ny * cart->nx;

  /*
   * allocate on first call
   */

  if (first_call)
  {
    data_grid = (ui08 *) umalloc
      ((ui32) (npoints * sizeof(ui08)));
    
    first_call = FALSE;
  }
  
  /*
   * initialize
   */
  
  KM_unpack_init(file_buf, file_size);
  memset((void *) data_grid, 0, (int) (npoints * sizeof(ui08)));

  if (Glob->params.output_data_origin == BOTLEFT)
  {
    dp = data_grid + (cart->ny - 1) * cart->nx;
    delta_dp = -cart->nx;
  }
  else
  {
    dp = data_grid;
    delta_dp = cart->nx;
  }

  for (i = 0; i < cart->ny; i++)
  {
    line = KM_unpack_raw_line();
    
    if (line == NULL) {
      fprintf(stderr, "ERROR - null line %d\n", i);
    }
    else
    {
      memcpy((void *) dp, (void *) line,
	     (int) (cart->nx * sizeof(ui08)));
    }
    
    dp += delta_dp;
    
  } /* i */

  /*
   * set pointer to data grid
   */

  v_handle->field_plane[0][0] = data_grid;
  
  return;
  
}
