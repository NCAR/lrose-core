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
 * read_goes.c
 *
 * Reads GOES image, remaps
 *
 * RAP, NCAR, Boulder CO
 *
 * Feb 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "mcidas2mdv.h"
#include <cidd/cdata_util.h>
#include <toolsa/file_io.h>
#include <toolsa/pjg.h>
#include <mdv/mdv_grid.h>
#include <ctype.h>

#define RAD_TO_DEG 57.29577951308092

#define BETWEEN(x,lower,upper) (((x)-(lower))*((x)-(upper)) <= 0)

static long *get_lookup_goes(long int *nav_cod,
			     long int linelen,
			     long int prefixlen,
			     long int nbytes,
			     long int xres,
			     long int yres,
			     long int minline,
			     long int maxline,
			     long int minpixel,
			     long int maxpixel);

static long *get_lookup_gvar(long int *nav_cod,
			     long int linelen,
			     long int prefixlen,
			     long int nbytes,
			     long int xres,
			     long int yres,
			     long int minline,
			     long int maxline,
			     long int minpixel,
			     long int maxpixel);

/*********************************************************************
 * read_goes()
 *
 * module for reading and remapping goes data
 *
 * Original mcidas file code provided by Chris Burghart, ATD, NCAR
 *
 * Returns 0 on success, -1 on failure.
 *
 *********************************************************************/

int read_goes(char *file_path)

{
  
  static first_call = TRUE;
  static ui08 *plane;

  char *field_name;
  char source[8];
  char platform[8];
  char sat_type[8];
  ui08 *image;
  ui08 *p;

  int need_swap;

  long ix, iy;
  long header[64], nav_cod[128 * MAXNAVCHUNKS];
  long band;
  long xres, yres, nbytes;
  long nlines_image, npixels_image;
  long linelen, prefixlen;
  long minline, maxline, minpixel, maxpixel;
  long *lookup, *l, lindex;

  date_time_t image_time;

  mcidas2mdv_output_grid *grid = &Glob->params.output_grid;

  /*
   * allocate space for the data plane
   */
  
  if (first_call) {
    plane =
      (u_char *) umalloc (grid->nx * grid->ny);
    first_call = FALSE;
  }
  memset(plane, 0, grid->nx * grid->ny);
  
  /*
   * file info
   */

  if (MCIDAS_file_info (file_path, &image_time, sat_type, &need_swap)) {
    fprintf(stderr, "ERROR - %s:get_grid_data\n", Glob->prog_name);
    return (-1);
  }

  fprintf(stdout, "\n");
  fprintf(stdout, "Processing McIdas file '%s'\n", file_path);
  fprintf(stdout, "  File time : %s\n", utimestr(&image_time));
  fprintf(stdout, "  Sat type : %s\n", sat_type);
  if (need_swap) {
    fprintf(stdout, "  Swapping bytes\n");
  }
  fflush(stdout);
  
  if (!strcmp(sat_type, "GOES")) {

    /*
     * read in image
     */

    if (MCIDAS_read_goes(file_path, need_swap,
			 header, nav_cod, &image, source,
			 &xres, &yres, &nbytes,
			 &npixels_image, &nlines_image,
			 &linelen, &prefixlen)) {
      fprintf (stderr, "ERROR - %s:get_grid_data\n",
	       Glob->prog_name);
      perror (file_path);
      return (-1);
    }

  } else if (!strcmp(sat_type, "GVAR")) {

    /*
     * read in image
     */

    if (MCIDAS_read_gvar(file_path, need_swap,
			 header, nav_cod, &image, source,
			 &xres, &yres, &nbytes,
			 &npixels_image, &nlines_image,
			 &linelen, &prefixlen)) {
      fprintf (stderr, "ERROR - %s:get_grid_data\n",
	       Glob->prog_name);
      perror (file_path);
      return (-1);
    }

  } else {
    
    fprintf (stderr, "ERROR - unknown file type, file '%s'\n",
	     file_path);
    return (-1);
    
  }
    
  /*
   * set platform name and band number
   */
  
  memset (platform, 0, 8);
  strncpy(platform, (char *) nav_cod, 4);
  
  band = header[11];
  
  /*
   * Pixel and line limits
   */
  
  minline = header[5];
  maxline = minline + (nlines_image - 1) * yres;
  
  minpixel = header[6];
  maxpixel = minpixel + (npixels_image - 1) * xres;
  
  fprintf(stdout, "  minline, minpixel = %ld, %ld\n",  minline, minpixel);
  fprintf(stdout, "  maxline, maxpixel = %ld, %ld\n",  maxline, maxpixel);
  fflush(stdout);

  /*
   * get the lookup table
   */
  
  if (!strcmp(sat_type, "GOES")) {

    lookup = get_lookup_goes(nav_cod, linelen, prefixlen, nbytes,
			     xres, yres,
			     minline, maxline, minpixel, maxpixel);

  } else {

    lookup = get_lookup_gvar(nav_cod, linelen, prefixlen, nbytes,
			     xres, yres,
			     minline, maxline, minpixel, maxpixel);

  }
    
  fflush(stdout);

  if (lookup == (long *) NULL) {
    ufree((char *) image);
    return (-1);
  }

  if (band == 1) {
    field_name = "Visible";
  } else if (band == 4) {
    field_name = "IR";
  } else {
    field_name = "Unknown";
  }
  
  /*
   * load the MDV headers
   */

  /* The way MDV files are written has changed - Niles.
  LoadMdvFieldHdr(image_time.unix_time);
  LoadMdvMasterHdr(image_time.unix_time, file_path, field_name, sat_type);
  */

  /*
   * load the image
   */
  
  l = lookup;
  p = plane;
  for (iy = 0; iy < grid->ny; iy++) {
    for (ix = 0; ix < grid->nx; ix++, p++, l++) {
      lindex = *l;
      if (lindex >= 0) {
	*p = image[lindex];
      }
    } /* ix */
  } /* iy */
  
  /*
   * free up image
   */
  
  ufree((char *) image);
  
  /*
   * write the mdv file
   */

  /* The way this is done has changed - Niles.
  WriteMdv(&image_time, plane);
  */

  mcidas_mdv(image_time.unix_time, file_path,
	     field_name, sat_type, plane);

  return (0);
  
}

/**********************************************************************
 * get_lookup_goes() - get lookup table for image-to-grid conversion
 * for GOES satellites
 */

static long *get_lookup_goes(long int *nav_cod,
			     long int linelen,
			     long int prefixlen,
			     long int nbytes,
			     long int xres,
			     long int yres,
			     long int minline,
			     long int maxline,
			     long int minpixel,
			     long int maxpixel)

{

  static int first_call = TRUE;
  static long *lookup = NULL;

  long image_x, image_y;
  long ix, iy;
  long line, pixel;
  long index;
  long status;
  long pos;
  long count = 0;

  double x, y, dx, dy;
  double lat_point, lon_point;
 
  double flat, flon, fline, fpixel, fdummy = 0.0;
  mdv_grid_comps_t mgrid;

  /*
   * Initialize the navigation stuff
   */

  status = goes_nvxini_c (1L, (nx_nav_goes *) nav_cod);

  if (status < 0)
    return ((long *) NULL);
  
  /*
   * allocate memory for the lookup table
   */

  if (first_call) {
    lookup = (long *) umalloc
      ((u_int) (Glob->params.output_grid.nx *
		Glob->params.output_grid.ny * sizeof(long)));
    first_call = FALSE;
  }

  /*
   * load the lookup table
   */

  MDV_init_flat(Glob->params.output_grid.origin_lat,
		Glob->params.output_grid.origin_lon,
		0.0, &mgrid);

  index = 0;
  dx = Glob->params.output_grid.dx;
  dy = Glob->params.output_grid.dy;

  y = Glob->params.output_grid.miny;

  for (iy = 0; iy < Glob->params.output_grid.ny; iy++, y += dy) {

    x = Glob->params.output_grid.minx;

    for (ix = 0; ix < Glob->params.output_grid.nx; ix++, index++, x += dy) {

      MDV_xy2latlon(&mgrid, x, y, &lat_point, &lon_point);

      /*
       * FIX - goes_nvxeas expects pos longitudes
       */

      flon = (double) fabs(lon_point);
      flat = (double) lat_point;
     
      status = goes_nvxeas_c (flat, flon, fdummy,
			      &fline, &fpixel, &fdummy);
      
      line = (long) (fline + 0.5);
      pixel = (long) (fpixel + 0.5);

      if (iy == 0) {
	if (ix == 0) {
	  fprintf(stdout, "  SW corner, line, pixel: %ld, %ld\n", line, pixel);
	} else if (ix == Glob->params.output_grid.nx - 1) {
	  fprintf(stdout, "  SE corner, line, pixel: %ld, %ld\n", line, pixel);
	}
      } else if (iy == Glob->params.output_grid.ny - 1) {
	if (ix == 0) {
	  fprintf(stdout, "  NW corner, line, pixel: %ld, %ld\n", line, pixel);
	} else if (ix == Glob->params.output_grid.nx - 1) {
	  fprintf(stdout, "  NE corner, line, pixel: %ld, %ld\n", line, pixel);
	}
      }

      if (BETWEEN(line, minline, maxline) &&
	  BETWEEN(pixel, minpixel, maxpixel)) {
	
	/*
	 * Translate from satellite coordinates to image coordinates
	 */
	
	image_x = (pixel - minpixel) / xres;
	image_y = (line - minline) / yres;
	
	/*
	 * Find the offset into the image
	 */
	
	pos = image_y * linelen + prefixlen + image_x * nbytes;

	lookup[index] = pos;
	
      } else {
	
	lookup[index] = -1;
	
      } /* if (BETWEEN ... */
      
      if (Glob->params.debug >= DEBUG_EXTRA) {
	count++;
	if (count == 1000) {
	  printf ("x, y = %g, %g\n", x, y);
	  printf("flat, flon, line, pixel = %g, %g, %ld, %ld\n",
		 flat, flon, line, pixel);
	  printf ("image_x, image_y, pos, index = %ld, %ld, %ld, %ld\n",
		  image_x, image_y, pos, index);
	  count = 0;
	}

      } /* if (Glob->params.debug) */

    } /* ix */

  } /* iy */

  return (lookup);


}

/**********************************************************************
 * get_lookup_gvar() - get lookup table for image-to-grid conversion
 * for GVAR satellites
 */

static long *get_lookup_gvar(long int *nav_cod,
			     long int linelen,
			     long int prefixlen,
			     long int nbytes,
			     long int xres,
			     long int yres,
			     long int minline,
			     long int maxline,
			     long int minpixel,
			     long int maxpixel)

{

  static int first_call = TRUE;
  static long *lookup = NULL;

  long image_x, image_y;
  long ix, iy;
  long line, pixel;
  long index;
  long status;
  long pos;
  long count = 0;
  long one = 1L;

  double x, y, dx, dy;
  double lat_point, lon_point;
 
  float flat, flon, fline, fpixel, fdummy = 0.0;
  mdv_grid_comps_t mgrid;

  /*
   * Initialize the navigation stuff (FORTRAN routines in libmcidas.a)
   */

  /*
#ifdef USE_GNU_F2C
  status = gvar_nvxini__ (&one, nav_cod);
#else
  status = gvar_nvxini_ (&one, nav_cod);
#endif
*/
  status = gvar_nvxini__ (&one, nav_cod);

  if (status < 0)
    return ((long *) NULL);
  
  /*
   * allocate memory for the lookup table
   */

  if (first_call) {
    lookup = (long *) umalloc
      ((u_int) (Glob->params.output_grid.nx *
		Glob->params.output_grid.ny * sizeof(long)));
    first_call = FALSE;
  }

  /*
   * load the lookup table
   */

  MDV_init_flat(Glob->params.output_grid.origin_lat,
		Glob->params.output_grid.origin_lon,
		0.0, &mgrid);

  index = 0;
  dx = Glob->params.output_grid.dx;
  dy = Glob->params.output_grid.dy;

  y = Glob->params.output_grid.miny;

  for (iy = 0; iy < Glob->params.output_grid.ny; iy++, y += dy) {

    x = Glob->params.output_grid.minx;

    for (ix = 0; ix < Glob->params.output_grid.nx; ix++, index++, x += dx) {

      MDV_xy2latlon(&mgrid, x, y, &lat_point, &lon_point);

      /*
       * FIX - gvar_nvxeas expects pos longitudes - 
       * from FORTRAN routines in libmcidas.a
       */

      flon = (float) fabs(lon_point);
      flat = (float) lat_point;
     
      status = gvar_nvxeas__(&flat, &flon, &fdummy,
			     &fline, &fpixel, &fdummy);
      
      line = (long) (fline + 0.5);
      pixel = (long) (fpixel + 0.5);

      if (iy == 0) {
	if (ix == 0) {
	  fprintf(stdout, "  SW corner, line, pixel: %ld, %ld\n", line, pixel);
	} else if (ix == Glob->params.output_grid.nx - 1) {
	  fprintf(stdout, "  SE corner, line, pixel: %ld, %ld\n", line, pixel);
	}
      } else if (iy == Glob->params.output_grid.ny - 1) {
	if (ix == 0) {
	  fprintf(stdout, "  NW corner, line, pixel: %ld, %ld\n", line, pixel);
	} else if (ix == Glob->params.output_grid.nx - 1) {
	  fprintf(stdout, "  NE corner, line, pixel: %ld, %ld\n", line, pixel);
	}
      }

      if (BETWEEN(line, minline, maxline) &&
	  BETWEEN(pixel, minpixel, maxpixel)) {
	
	/*
	 * Translate from satellite coordinates to image coordinates
	 */
	
	image_x = (pixel - minpixel) / xres;
	image_y = (line - minline) / yres;
	
	/*
	 * Find the offset into the image
	 */
	
	pos = image_y * linelen + prefixlen + image_x * nbytes;

	lookup[index] = pos;
	
      } else {
	
	lookup[index] = -1;
	
      } /* if (BETWEEN ... */
      
      if (Glob->params.debug >= DEBUG_EXTRA) {

	count++;

	if (count == 1000) {
	  printf ("x, y, = %g, %g\n", x, y);
	  printf("flat, flon, line, pixel = %g, %g, %ld, %ld\n",
		 flat, flon, line, pixel);
	  printf ("image_x, image_y, pos, index = %ld, %ld, %ld, %ld\n",
		  image_x, image_y, pos, index);
	  count = 0;
	}

      } /* if (Glob->params.debug) */

    } /* ix */

  } /* iy */

  return (lookup);

}

