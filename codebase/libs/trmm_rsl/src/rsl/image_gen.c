/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996, 1997
            John H. Merritt
            Space Applications Corporation
            Vienna, Virginia

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
 *---------------------------------------------------------------------
 * Image generation functions:
 *
 *   void RSL_load_color_table(char *infile, char buffer[256], int *ncolors);
 *   void RSL_load_red_table(char *infile);
 *   void RSL_load_green_table(char *infile);
 *   void RSL_load_blue_table(char *infile);
 *   void RSL_load_refl_color_table();
 *   void RSL_load_vel_color_table();
 *   void RSL_load_sw_color_table();
 *   void RSL_load_height_color_table();
 *   void RSL_load_rainfall_color_table();
 *   void RSL_bscan_ray(FILE *fp, Ray *r);
 *   void RSL_bscan_sweep(Sweep *s, char *outfile);
 *   void RSL_bscan_volume(Volume *v, char *basename);
 *   unsigned char *RSL_sweep_to_cart(Sweep *s, int xdim, int ydim, float range);
 *   unsigned char *RSL_rhi_sweep_to_cart(Sweep *s, int xdim, int ydim, float range, int vert_scale);
 *   void RSL_write_gif(char *outfile, unsigned char *image, int xdim, int ydim, char c_table[256][3]);
 *   void RSL_write_pict(char *outfile, unsigned char *image, int xdim, int ydim, char c_table[256][3]);
 *   void RSL_write_ppm(char *outfile, unsigned char *image, int xdim, int ydim, char c_table[256][3]);
 *   void RSL_write_pgm(char *outfile, unsigned char *image, int xdim, int ydim);
 *   void RSL_sweep_to_gif(Sweep *s, char *outfile, int xdim, int ydim, float range);
 *   void RSL_rhi_sweep_to_gif(Sweep *s, char *outfile, int xdim, int ydim, float range, int vert_scale);
 *   void RSL_sweep_to_pict(Sweep *s, char *outfile, int xdim, int ydim, float range);
 *   void RSL_sweep_to_ppm(Sweep *s, char *outfile, int xdim, int ydim, float range);
 *   void RSL_sweep_to_pgm(Sweep *s, char *outfile, int xdim, int ydim, float range);
 *   void RSL_volume_to_gif(Volume *v, char *basename, int xdim, int ydim, float range);
 *   void RSL_volume_to_pict(Volume *v, char *basename, int xdim, int ydim, float range);
 *   void RSL_volume_to_ppm(Volume *v, char *basename, int xdim, int ydim, float range);
 *   void RSL_volume_to_pgm(Volume *v, char *basename, int xdim, int ydim, float range);
 *   void RSL_rebin_velocity_ray(Ray *r);
 *   void RSL_rebin_velocity_sweep(Sweep *s);
 *   void RSL_rebin_velocity_volume(Volume *v);
 *   void RSL_rebin_zdr_ray(Ray *r);
 *   void RSL_rebin_zdr_sweep(Sweep *s);
 *   void RSL_rebin_zdr_volume(Volume *v);
 */
#include <stdio.h>
#include <stdlib.h>
#include <trmm_rsl/rsl.h>
extern FILE *popen(const char *, const char *);
extern int pclose(FILE *stream);
extern int radar_verbose_flag;

static char color_table[256][3];
static int ncolors = 0;


/**********************************************************************/
/*                                                                    */
/*                  RSL_load_color_table                              */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
void RSL_load_color_table(char *infile, char buffer[256], int *num_colors)
{
  FILE *fp;

  fp = fopen(infile, "r");
  if (fp == NULL) {
	perror(infile);
	exit(-1);
  }
  *num_colors = fread(buffer, sizeof(char), 256, fp);

  (void)fclose(fp);
}

/**********************************************************************/
/*                                                                    */
/*                      RSL_set_color_table                           */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      SM&A Corporation                                              */
/*      Jan 21, 1999                                                  */
/**********************************************************************/
void RSL_set_color_table(int icolor, char buffer[256], int ncolors)
{
  int i;
  /* In RSL: Red table=RSL_RED_TABLE,
   *       green table=RSL_GREEN_TABLE,
   *        blue table=RSL_BLUE_TABLE.
   */
  for (i=0; i<ncolors; i++) color_table[i][icolor] = buffer[i];
}

/**********************************************************************/
/*                                                                    */
/*                      RSL_get_color_table                           */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      SM&A Corporation                                              */
/*      Jan 21, 1999                                                  */
/**********************************************************************/
void RSL_get_color_table(int icolor, char buffer[256], int *ncolors)
{
  int i;
  /* In RSL: Red table=RSL_RED_TABLE,
   *       green table=RSL_GREEN_TABLE,
   *        blue table=RSL_BLUE_TABLE.
   */
  *ncolors = 256;
  for (i=0; i<*ncolors; i++) buffer[i] = color_table[i][icolor];
}

/**********************************************************************/
/*                                                                    */
/*                      RSL_load_red_table                            */
/*                      RSL_load_green_table                          */
/*                      RSL_load_blue_table                           */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
void RSL_load_red_table(char *infile)
{
  char buffer[256];
  /* Red only. */
  RSL_load_color_table(infile, buffer, &ncolors);
  RSL_set_color_table(RSL_RED_TABLE, buffer, ncolors);
}
void RSL_load_green_table(char *infile)
{
  char buffer[256];
  /* Green only. */
  RSL_load_color_table(infile, buffer, &ncolors);
  RSL_set_color_table(RSL_GREEN_TABLE, buffer, ncolors);
}
void RSL_load_blue_table(char *infile)
{
  char buffer[256];
  /* Blue only. */
  RSL_load_color_table(infile, buffer, &ncolors);
  RSL_set_color_table(RSL_BLUE_TABLE, buffer, ncolors);
}

#include <string.h>
/**********************************************************************/
/*                                                                    */
/*                      RSL_load_refl_color_table                     */
/*                      RSL_load_vel_color_table                      */
/*                      RSL_load_sw_color_table                       */
/*                      RSL_load_zdr_color_table                      */
/*                      RSL_load_rainfall_color_table                 */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
void RSL_load_refl_color_table()
{
  char buffer[256];
  char *fnames[3] = { REFL_RED_FILE, REFL_GREEN_FILE, REFL_BLUE_FILE };

  int i, igun;

  for (igun=0; igun<3; igun++) {
	RSL_load_color_table(fnames[igun], buffer, &ncolors);
	for (i=0; i<ncolors; i++) color_table[i][igun] = buffer[i];
  }
}
void RSL_load_vel_color_table()
{
  char buffer[256];
  char *fnames[] = { VEL_RED_FILE, VEL_GREEN_FILE, VEL_BLUE_FILE };

  int i, igun;

  for (igun=0; igun<3; igun++) {
	RSL_load_color_table(fnames[igun], buffer, &ncolors);
	for (i=0; i<ncolors; i++) color_table[i][igun] = buffer[i];
  }
}
void RSL_load_sw_color_table()
{
  char buffer[256];
  char *fnames[] = { SW_RED_FILE, SW_GREEN_FILE, SW_BLUE_FILE };

  int i, igun;

  for (igun=0; igun<3; igun++) {
	RSL_load_color_table(fnames[igun], buffer, &ncolors);
	for (i=0; i<ncolors; i++) color_table[i][igun] = buffer[i];
  }
}

void RSL_load_height_color_table()
{
  char buffer[256];
  char *fnames[] = { HEIGHT_RED_FILE, HEIGHT_GREEN_FILE, HEIGHT_BLUE_FILE };

  int i, igun;

  for (igun=0; igun<3; igun++) {
	RSL_load_color_table(fnames[igun], buffer, &ncolors);
	for (i=0; i<ncolors; i++) color_table[i][igun] = buffer[i];
  }
}

void RSL_load_zdr_color_table()
{
  char buffer[256];
  char *fnames[] = { ZDR_RED_FILE, ZDR_GREEN_FILE, ZDR_BLUE_FILE };

  int i, igun;

  for (igun=0; igun<3; igun++) {
	RSL_load_color_table(fnames[igun], buffer, &ncolors);
	for (i=0; i<ncolors; i++) color_table[i][igun] = buffer[i];
  }
}

void RSL_load_rainfall_color_table()
{
  char buffer[256];
  char *fnames[] = { RAINFALL_RED_FILE, RAINFALL_GREEN_FILE, RAINFALL_BLUE_FILE };

  int i, igun;

  for (igun=0; igun<3; igun++) {
	RSL_load_color_table(fnames[igun], buffer, &ncolors);
	for (i=0; i<ncolors; i++) color_table[i][igun] = buffer[i];
  }
}
/**********************************************************************/
/*                                                                    */
/*                        Volume to BSCAN GIF                         */
/*                     (simple dump of rays)                          */
/*               RSL_bscan_ray                                        */
/*               RSL_bscan_sweep                                      */
/*               RSL_bscan_volume                                     */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      March 22, 1994                                                */
/**********************************************************************/
static unsigned char *outvect;

void RSL_bscan_ray(Ray *r, FILE *fp)
{
  int i;
  float (*f)(Range x);

  if (r == NULL) return;
  memset(outvect, 0, r->h.nbins);
  f = r->h.f;
  for (i=0; i<r->h.nbins; i++)
	if (f(r->range[i]) != BADVAL) {
	  if (f(r->range[i]) >= 0) outvect[i] = (unsigned char) f(r->range[i]);
	}
	else
	  outvect[i] = (unsigned char) (255 + f(r->range[i]));
  
  for(i=0; i<r->h.nbins; i++)
	(void)fwrite(color_table[outvect[i]], sizeof(char), 3, fp);
  
}
void RSL_bscan_sweep(Sweep *s, char *outfile)
{
  int i, j, nrecs, nbins;
  Ray *first_ray;
  FILE *fp;
  if (s == NULL) return;
  
  fp = fopen(outfile,"w");
  if (fp == NULL) {
	perror(outfile);
	return;
  }
  first_ray = RSL_get_first_ray_of_sweep(s);
  nrecs = s->h.nrays;
  nbins = first_ray->h.nbins;
  fprintf(fp, "P6\n# %s\n%d %d\n255\n",outfile, nbins, nrecs);
  outvect = NULL;
  outvect = (unsigned char *) calloc(nbins, sizeof(unsigned char));
  if (outvect == NULL) {
	perror((char *)outvect);
	return;
  }

/* Now, write the bscan. */
  for (j=0; j<s->h.nrays; j++) {
	if (s->ray[j]) RSL_bscan_ray(s->ray[j], fp);
	else {
	  memset(outvect, 0, nbins);
	  for(i=0; i<nbins; i++)
		(void)fwrite(color_table[outvect[i]], sizeof(char), 3, fp);
	}
  }
  
  free(outvect);
  fclose(fp);
}
void RSL_bscan_volume(Volume *v, char *basename)
{
  int i;
  char *outfile;
  
  RSL_load_refl_color_table();
  outfile = (char *)calloc(strlen(basename)+7, sizeof(char));
  for (i=0; i<v->h.nsweeps; i++) { 
	(void)sprintf(outfile,"bscan.%2.2d.ppm", i);

	RSL_bscan_sweep(v->sweep[i], outfile);
	if (radar_verbose_flag)
	  fprintf(stderr,"Output: %s\n", outfile);
  }
  free (outfile);
}


/**********************************************************************/
/**********************************************************************/
/*                                                                    */
/*                    Volume to cartesean GIF                         */
/*                     (polar to cartesean)                           */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
/**********************************************************************/
#include <math.h>


/**********************************************************************/
/*                                                                    */
/*                    RSL_sweep_to_cart                               */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
unsigned char *RSL_sweep_to_cart(Sweep *s, int xdim, int ydim, float range)
{
 /* Range specifies the maximum range to load that points into the image. */

  int x, y;
  float azim, r;
  float val;
  int the_index;
  Ray *ray;
  float beam_width;
  

  static unsigned char *cart_image = NULL;

  if (s == NULL) return NULL;
  if (xdim != ydim || ydim < 0 || xdim < 0) {
	fprintf(stderr, "(xdim=%d) != (ydim=%d) or either negative.\n", xdim, ydim);
	return NULL;
  }
  cart_image = (unsigned char *) calloc(xdim*ydim, sizeof(unsigned char));

  beam_width = s->h.beam_width/2.0 * 1.2;
  if (beam_width == 0) beam_width = 1.2;  /* Sane image generation. */

  for (y=-ydim/2; y<ydim/2; y++)
	for (x=-xdim/2; x<xdim/2; x++){/* Find azimuth and range, then search Volume. */
	  if (x !=0 ) 
		azim = (float)atan((double)y/(double)x)*180.0/3.14159;
	  else
		if (y < 0) azim = -90.0;
		else azim = 90.0;
	  if (y<0 && x<0) /* Quadrant 3 (math notation). */
		azim -= 180;
	  else if (y>=0 && x<0) /* Quad: 2 */
		azim += 180;
	  
	  /* Radar is clockwise increasing. */
	  azim = -azim;
	  
	  azim -= 90.0;
	  if (azim < 0) azim += 360.0;

	  r = (float)sqrt((double)x*x + (double)y*y);
	  if (ydim < xdim) r *= range/(.5*ydim);
	  else r *= range/(.5*xdim);
	  if (r > range) val = BADVAL;
	  else {
		ray = RSL_get_closest_ray_from_sweep(s, azim, beam_width);
		val = RSL_get_value_from_ray(ray, r);
	  }
	  the_index =  (y+ydim/2)*ydim + (xdim-1)-(x+xdim/2);
	  if (val == BADVAL || val == NOTFOUND_V || val == NOTFOUND_H)
		cart_image[the_index] = (unsigned char) 0;
	  else if (val >= 0)
	    cart_image[the_index] = (unsigned char) val;
	  else 
		cart_image[the_index] = (unsigned char) (256+val);

	}
  return cart_image;
}

/**********************************************************************/
/*                                                                    */
/*                    RSL_rhi_sweep_to_cart                           */
/*                                                                    */
/*  A modified version of RSL_sweep_to_cart()                         */
/*  -> Handles rhi (vertical) sweeps                                  */
/*      Kolander                                                      */
/*      Space Applications Corporation                                */
/*      July 27, 1995                                                 */
/**********************************************************************/
unsigned char *RSL_rhi_sweep_to_cart(Sweep *s, int xdim, int ydim, 
									 float range, int vert_scale)
{
  int x, y, xx, yy, i, j;
  float angle, r;
  float val;
  int the_index;

  static unsigned char *rhi_cart_image = NULL;
  static unsigned char *buffer = NULL;

  if (s == NULL) return NULL;
  if (buffer == NULL)
     buffer = (unsigned char *)calloc(xdim*ydim, sizeof(unsigned char));
  if (rhi_cart_image == NULL)
     rhi_cart_image = (unsigned char *)calloc(xdim*ydim, sizeof(unsigned char));
  memset(buffer, (unsigned char)0, xdim*ydim);
  memset(rhi_cart_image, (unsigned char)0, xdim*ydim);
  
  for (y=0; y<xdim; y++)
	for (x=0; x<ydim; x++){
	  if (x != 0) 
		angle = (float)atan((double)y/(double)x)*180.0/3.14159;
	  else
		angle = 90.0;

	  angle = 90.0 - angle;
	  r = (float)sqrt((double)x*x + (double)y*y);
	  if (r > range) 
	     val = BADVAL;
	  else 
	     val = RSL_get_value_from_sweep(s, angle, r);

	  /* A vertical sweep extends from 0 deg to perhaps 25 deg in
		 elevation. For proper orientation of a displayed rhi 
		 vertical sweep:
		 1. reflect image about the line y=x .
		 2. translate image downward so that radar site is located
		    at the bottom left corner of the image. The sweep will
			then extend upward from the bottom of the image.
	  */
	  xx = y;    /* reflection about line y=x */
	  yy = x;    /* reflection about line y=x */
	  the_index =  (ydim - 1 - yy)*xdim + xx;
	  if (val == BADVAL || val == NOTFOUND_V || val == NOTFOUND_H)
		buffer[the_index] = (unsigned char) 0;
	  else if (val >= 0)
		buffer[the_index] = (unsigned char) val;
	  else 
		buffer[the_index] = (unsigned char) (256+val);
	}

  /* To see details in the sweep, it is customary to scale the image
	 so that the scale of the vertical axis is 5 or 10 times the scale
	 of the horizontal axis. 
	 Copy each row of pixel values from buffer[] a constant multiple 
	 number of times into rhi_cart_image[], in order to vertically
	 expand the image.
  */
  for (j=0; j<ydim/vert_scale; j++)
	 for (i=1; i<=vert_scale; i++)
		memcpy(&rhi_cart_image[(ydim - i - vert_scale*j)*xdim], 
			   &buffer[(ydim - 1 - j)*xdim], xdim);
  
  return rhi_cart_image;
}

#include <signal.h>
/**********************************************************************/
/*                                                                    */
/*                    RSL_write_gif                                   */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
void RSL_write_gif(char *outfile, unsigned char *image, int xdim, int ydim, char c_table[256][3])
{
  int i;
  int nbytes;
  char pipecmd[300];
  FILE *fpipe;

  if (image == NULL) {
	fprintf(stderr, "No image for file %s\n", outfile);
    return;
  }
  fpipe = NULL;
  nbytes = xdim*ydim;
  (void)sprintf(pipecmd, "ppmtogif > %s 2>/dev/null", outfile);
  fpipe = popen(pipecmd, "w");  /* Global FILE * */
  if (fpipe == NULL) {
	perror("RSL_write_gif1");
	return;
  }
  if (fprintf(fpipe, "P6\n# %s\n%d %d\n255\n",outfile, xdim, ydim) < 0) {
	perror("RSL_write_gif2");
	pclose(fpipe);
	return;
  }
  
  for (i=0; i<nbytes; i++)
	if (fwrite(c_table[image[i]], sizeof(char), 3, fpipe) != 3) {
	  perror("RSL_write_gif3");
	  pclose(fpipe);
	  return;
	}

  if(pclose(fpipe) != 0) perror("RSL_write_gif4");
}
/**********************************************************************/
/*                                                                    */
/*                    RSL_write_pict                                  */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
void RSL_write_pict(char *outfile, unsigned char *image, int xdim, int ydim, char c_table[256][3])
{
  int i;
  int nbytes;
  char pipecmd[100];
  FILE *fpipe;

  if (image == NULL) {
	fprintf(stderr, "No image for file %s\n", outfile);
    return;
  }
  nbytes = xdim*ydim;
  (void)sprintf(pipecmd, "ppmtopict > %s 2>/dev/null", outfile);
  fpipe = popen(pipecmd, "w");  /* Global FILE * */
  fprintf(fpipe, "P6\n# %s\n%d %d\n255\n",outfile, xdim, ydim);
  for (i=0; i<nbytes; i++) (void)fwrite(c_table[image[i]], sizeof(char), 3, fpipe);
  (void)pclose(fpipe);
}
/**********************************************************************/
/*                                                                    */
/*                    RSL_write_ppm                                   */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
void RSL_write_ppm(char *outfile, unsigned char *image, int xdim, int ydim, char c_table[256][3])
{
  int i;
  int nbytes;
  FILE *fpipe;

  if (image == NULL) {
	fprintf(stderr, "No image for file %s\n", outfile);
    return;
  }
  nbytes = xdim*ydim;
  fpipe = fopen(outfile, "w");  /* Global FILE * */
  fprintf(fpipe, "P6\n# %s\n%d %d\n255\n",outfile, xdim, ydim);
  for (i=0; i<nbytes; i++) (void)fwrite(c_table[image[i]], sizeof(char), 3, fpipe);
  (void)fclose(fpipe);
}
/**********************************************************************/
/*                                                                    */
/*                    RSL_write_pgm                                   */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
void RSL_write_pgm(char *outfile, unsigned char *image, int xdim, int ydim)
{
  int nbytes;
  char pipecmd[100];
  FILE *fpipe;

  if (image == NULL) {
	fprintf(stderr, "No image for file %s\n", outfile);
    return;
  }
  nbytes = xdim*ydim;
  (void)sprintf(pipecmd, "gzip > %s.gz 2>/dev/null", outfile);
  fpipe = popen(pipecmd, "w");  /* Global FILE * */
  fprintf(fpipe, "P5\n# %s\n%d %d\n255\n",outfile, xdim, ydim);
  (void)fwrite(image, sizeof(char), nbytes, fpipe);
  (void)pclose(fpipe);

/*  The following is commented and is for non compressed. */
#ifdef COMPILE
  nbytes = xdim*ydim;
  fpipe = fopen(outfile, "w");  /* Global FILE * */
  fprintf(fpipe, "P5\n# %s\n%d %d\n255\n",outfile, xdim, ydim);
  (void)fwrite(image, sizeof(char), nbytes, fpipe);
  (void)fclose(fpipe);
#endif
}

/**********************************************************************/
/*                                                                    */
/*                    RSL_sweep_to_gif                                */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
void RSL_sweep_to_gif(Sweep *s, char *outfile, int xdim, int ydim, float range)
{
  /* Currently range is not used. */
  unsigned char *cart_image;

  /* Assumes the color table is already loaded. */
  if (s == NULL) return;
  if (ncolors == 0) {
	fprintf(stderr, "No colors in color table.  Load color table first.\n");
	return;
  }
  cart_image = RSL_sweep_to_cart(s, xdim, ydim, range);
  RSL_write_gif(outfile, cart_image, xdim, ydim, color_table);
  free(cart_image);
}

/**********************************************************************/
/*                                                                    */
/*                    RSL_rhi_sweep_to_gif                            */
/*                                                                    */
/* Slightly modified version of RSL_sweep_to_gif()                    */
/*                                                                    */
/*  By: Kolander                                                      */
/*      Space Applications Corporation                                */
/*      July 27, 1995                                                 */
/**********************************************************************/
void RSL_rhi_sweep_to_gif(Sweep *s, char *outfile, int xdim, int ydim, 
						  float range, int vert_scale)
{
  unsigned char *cart_image;

  /* Assumes the color table is already loaded. */
  if (s == NULL) return;
  if (ncolors == 0) {
	fprintf(stderr, "No colors in color table.  Load color table first.\n");
	return;
  }
  cart_image = RSL_rhi_sweep_to_cart(s, xdim, ydim, range, vert_scale);
  RSL_write_gif(outfile, cart_image, xdim, ydim, color_table);
  free(cart_image);
}

/**********************************************************************/
/*                                                                    */
/*                    RSL_sweep_to_pict                               */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
void RSL_sweep_to_pict(Sweep *s, char *outfile, int xdim, int ydim, float range)
{
  /* Currently range is not used. */
  unsigned char *cart_image;

  /* Assumes the color table is already loaded. */
  if (s == NULL) return;
  if (ncolors == 0) {
	fprintf(stderr, "No colors in color table.  Load color table first.\n");
	return;
  }
  cart_image = RSL_sweep_to_cart(s, xdim, ydim, range);
  RSL_write_pict(outfile, cart_image, xdim, ydim, color_table);
  free(cart_image);
}
/**********************************************************************/
/*                                                                    */
/*                    RSL_sweep_to_ppm                                */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
void RSL_sweep_to_ppm(Sweep *s, char *outfile, int xdim, int ydim, float range)
{
  /* Currently range is not used. */
  unsigned char *cart_image;

  /* Assumes the color table is already loaded. */
  if (s == NULL) return;
  if (ncolors == 0) {
	fprintf(stderr, "No colors in color table.  Load color table first.\n");
	return;
  }
  cart_image = RSL_sweep_to_cart(s, xdim, ydim, range);
  RSL_write_ppm(outfile, cart_image, xdim, ydim, color_table);
  free(cart_image);
}
/**********************************************************************/
/*                                                                    */
/*                    RSL_sweep_to_pgm                                */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
void RSL_sweep_to_pgm(Sweep *s, char *outfile, int xdim, int ydim, float range)
{
  /* Currently range is not used. */
  unsigned char *cart_image;

  if (s == NULL) return;
  cart_image = RSL_sweep_to_cart(s, xdim, ydim, range);
  RSL_write_pgm(outfile, cart_image, xdim, ydim);
  free(cart_image);
}

/**********************************************************************/
/*                                                                    */
/*                    RSL_volume_to_gif                               */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
void RSL_volume_to_gif(Volume *v, char *basename, int xdim, int ydim, float range)
{
/*
 * Make a xdim by ydim cartesean image.  Center of radar is xdim/2,ydim/2.
 */
  int i;
  char outfile[100];
  unsigned char *cart_image;

  if (v == NULL) return;
  if (ncolors == 0) {
	fprintf(stderr, "No colors in color table.  Load color table first.\n");
	return;
  }
/*
 * Sweep 0 has Reflectivity only.
 * Sweep 1 has Velocity and Spectrum width, No Reflectivity.
 * Sweep > 1 any.
 *
 */
  for (i=0; i<v->h.nsweeps; i++) {
	(void)sprintf(outfile,"%s.%2.2d.gif", basename, i); /* File name: sweep.[0-10] */
	if (v->sweep[i] == NULL) continue;
	cart_image = RSL_sweep_to_cart(v->sweep[i], xdim, ydim, range);
	if (radar_verbose_flag)
	  fprintf(stderr,"==> Sweep %d of %d\n",i, v->h.nsweeps);
	if (cart_image != NULL) {
	  RSL_write_gif(outfile, cart_image, xdim, ydim, color_table);
	  printf("%s\n", outfile);
	  free (cart_image);
	} else {
	  if (radar_verbose_flag)
		fprintf(stderr,"No image.  cart_image for sweep %d is NULL.\n", i);
	}
  }
}

/**********************************************************************/
/*                                                                    */
/*                    RSL_volume_to_pict                              */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
void RSL_volume_to_pict(Volume *v, char *basename, int xdim, int ydim, float range)
{
/*
 * Make a xdim by ydim cartesean image.  Center of radar is xdim/2,ydim/2.
 */
  int i;
  char outfile[100];
  unsigned char *cart_image;

  if (v == NULL) return;
  if (ncolors == 0) {
	fprintf(stderr, "No colors in color table.  Load color table first.\n");
	return;
  }
/*
 * Sweep 0 has Reflectivity only.
 * Sweep 1 has Velocity and Spectrum width, No Reflectivity.
 * Sweep > 1 any.
 *
 */
  for (i=0; i<v->h.nsweeps; i++) {
	cart_image = RSL_sweep_to_cart(v->sweep[i], xdim, ydim, range);
	if (radar_verbose_flag)
	  fprintf(stderr,"==> Sweep %d of %d\n",i, v->h.nsweeps);
	(void)sprintf(outfile,"%s.%2.2d.pict", basename, i); /* File name: sweep.[0-10] */
	if (cart_image != NULL) {
	  RSL_write_pict(outfile, cart_image, xdim, ydim, color_table);
	  if (radar_verbose_flag)
		fprintf(stderr,"Wrote: %s\n", outfile);
	  free (cart_image);
	} else {
	  if (radar_verbose_flag)
		fprintf(stderr,"No image.  cart_image for sweep %d is NULL.\n", i);
	}
  }
}

/**********************************************************************/
/*                                                                    */
/*                    RSL_volume_to_ppm                               */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
void RSL_volume_to_ppm(Volume *v, char *basename, int xdim, int ydim, float range)
{
/*
 * Make a xdim by ydim cartesean image.  Center of radar is xdim/2,ydim/2.
 */
  int i;
  char outfile[100];
  unsigned char *cart_image;

  if (v == NULL) return;
  if (ncolors == 0) {
	fprintf(stderr, "No colors in color table.  Load color table first.\n");
	return;
  }
/*
 * Sweep 0 has Reflectivity only.
 * Sweep 1 has Velocity and Spectrum width, No Reflectivity.
 * Sweep > 1 any.
 *
 */
  for (i=0; i<v->h.nsweeps; i++) {
	cart_image = RSL_sweep_to_cart(v->sweep[i], xdim, ydim, range);
	if (radar_verbose_flag)
	  fprintf(stderr,"==> Sweep %d of %d\n",i, v->h.nsweeps);
	(void)sprintf(outfile,"%s.%2.2d.ppm", basename, i); /* File name: sweep.[0-10] */
	if (cart_image != NULL) {
	  RSL_write_ppm(outfile, cart_image, xdim, ydim, color_table);
	  if (radar_verbose_flag)
		fprintf(stderr,"Wrote: %s\n", outfile);
	  free (cart_image);
	} else {
	  if (radar_verbose_flag)
		fprintf(stderr,"No image.  cart_image for sweep %d is NULL.\n", i);
	}
  }
}
/**********************************************************************/
/*                                                                    */
/*                    RSL_volume_to_pgm                               */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
void RSL_volume_to_pgm(Volume *v, char *basename, int xdim, int ydim, float range)
{
/*
 * Make a xdim by ydim cartesean image.  Center of radar is xdim/2,ydim/2.
 */
  int i;
  char outfile[100];
  unsigned char *cart_image;

/*
 * Sweep 0 has Reflectivity only.
 * Sweep 1 has Velocity and Spectrum width, No Reflectivity.
 * Sweep > 1 any.
 *
 */
  if (v == NULL) return;
  for (i=0; i<v->h.nsweeps; i++) {
	cart_image = RSL_sweep_to_cart(v->sweep[i], xdim, ydim, range);
	if (radar_verbose_flag)
	  fprintf(stderr,"==> Sweep %d of %d\n",i, v->h.nsweeps);
	(void)sprintf(outfile,"%s.%2.2d.pgm", basename, i); /* File name: sweep.[0-10] */
	if (cart_image != NULL) {
	  RSL_write_pgm(outfile, cart_image, xdim, ydim);
	  if (radar_verbose_flag)
		fprintf(stderr,"Wrote: %s\n", outfile);
	  free (cart_image);
	} else {
	  if (radar_verbose_flag)
		fprintf(stderr,"No image.  cart_image for sweep %d is NULL.\n", i);
	}
  }
}


/***********************************************************************/
/*                                                                     */
/*                  RSL_rebin_velocity_ray                             */
/*                  RSL_rebin_velocity_sweep                           */
/*                  RSL_rebin_velocity_volume                          */
/*                                                                     */
/*  By: John Merritt                                                   */
/*      Space Applications Corporation                                 */
/*      April 30, 1994                                                 */
/*                                                                     */
/***********************************************************************/
void RSL_rebin_velocity_ray(Ray *r)
{
  /* Rebin the velocity data to the range -nyquist, +nyquist.
   * 14 bins are created centered at 0.  It sets the proper color look up
   * indexes.  This function modifies Ray r.
   */
  int i;
  float nyquist;
  float val /*  val1 */;

  int ncbins = 15; /* Number of color bins */
  float (*f)(Range x);
  Range (*invf)(float x);

  if (r == NULL) return;

  nyquist = r->h.nyq_vel;
  if (nyquist == 0.0) {
	fprintf(stderr, "RSL_rebin_velocity_ray: nyquist == 0.0\n");
	fprintf(stderr, "RSL_rebin_velocity_ray: Unable to rebin.\n");
	return;
  }
  f = r->h.f;
  invf = r->h.invf;
  for (i=0; i<r->h.nbins; i++) {
	val = f(r->range[i]);
	if (val == RFVAL) {
		val = 16;
	} else if (val != BADVAL) {
/*
	  Okay, we want to shift the data to positive values
	  then we re-scale them by the number of color bins/nyquist
*/
	  val = (int)(val/nyquist*(ncbins/2) + 1.0 + ncbins/2);

	} else {
		val = 0;
	}

	r->range[i] = invf(val);
  }
}


void RSL_rebin_velocity_sweep(Sweep *s)
{
  /* Rebin the velocity data to the range -nyquist, +nyquist.
   * 14 bins are created centered at 0. It sets the proper color look up
   * indexes.  This function modifies Sweep s.  Use this function prior
   * RSL_sweep_to_cart.  The binning is done in RSL_rebin_velocity_ray.
   */

  int i;
  
  if (s == NULL) return;

  for (i=0; i<s->h.nrays; i++)
	RSL_rebin_velocity_ray(s->ray[i]);

}

void RSL_rebin_velocity_volume(Volume *v)
{
  /* Rebin the velocity data to the range -nyquist, +nyquist.
   * 14 bins are created centered at 0. It sets the proper color look up
   * indexes.  This function modifies Volume v.  Use this function prior
   * RSL_sweep_to_cart.  The binning is done in RSL_rebin_velocity_ray.
   */

  int i;
  
  if (v == NULL) return;

  for (i=0; i<v->h.nsweeps; i++)
	RSL_rebin_velocity_sweep(v->sweep[i]);

}


/**********************************************************************/
/*                                                                    */
/*                    RSL_carpi_to_cart                               */
/*                                                                    */
/*      Space Applications Corporation                                */
/*      17 Mar 1997                                                   */
/**********************************************************************/
unsigned char *RSL_carpi_to_cart(Carpi *carpi, int xdim, int ydim, float range)
{
	/* Converts carpi data from the native RSL 1_ or 2_byte format to 
		 unsigned char format for image generation. Also reverses the
		 row order of the data so that displayed images come out right side up.
  */
	int i, j, cart_index;
  unsigned char *cart_image = NULL;
	float val;
	
  if (carpi == NULL) return NULL;
  cart_image = (unsigned char *) calloc(xdim*ydim, sizeof(unsigned char));
	
	for (j=0; j<ydim; j++)
	  for (i=0; i<xdim; i++)
		{
			cart_index = (ydim-1-j)*xdim + i;  /* Reverse the row order. */
			val = carpi->f(carpi->data[j][i]);
			if (val == BADVAL || val == NOTFOUND_V || val == NOTFOUND_H) 
			  cart_image[cart_index] = (unsigned char) 0;
			else if (val >= 0) 
			  cart_image[cart_index] = (unsigned char) val;
			else 
			  cart_image[cart_index] = (unsigned char) (256+val); /*2's complement.*/
		}
	return(cart_image);
}

/**********************************************************************/
/*                                                                    */
/*                    RSL_carpi_to_gif                                */
/*                                                                    */
/*      Space Applications Corporation                                */
/*      17 Mar 1997                                                   */
/**********************************************************************/
void RSL_carpi_to_gif(Carpi *carpi, char *outfile, int xdim, int ydim, float range)
{
  /* Currently range is not used. */
  unsigned char *cart_image;

  /* Assumes the color table is already loaded. */
  if (carpi == NULL) return;
  if (ncolors == 0) {
		fprintf(stderr, "No colors in color table.  Load color table first.\n");
		return;
  }
  cart_image = RSL_carpi_to_cart(carpi, xdim, ydim, range);
  RSL_write_gif(outfile, cart_image, xdim, ydim, color_table);
  free(cart_image);
}

/**********************************************************************/
/*                                                                    */
/*                    RSL_carpi_to_pict                               */
/*                                                                    */
/*      Space Applications Corporation                                */
/*      17 Mar 1997                                                   */
/**********************************************************************/
void RSL_carpi_to_pict(Carpi *carpi, char *outfile, int xdim, int ydim, float range)
{
  /* Currently range is not used. */
  unsigned char *cart_image;

  /* Assumes the color table is already loaded. */
  if (carpi == NULL) return;
  if (ncolors == 0) {
		fprintf(stderr, "No colors in color table.  Load color table first.\n");
		return;
  }
  cart_image = RSL_carpi_to_cart(carpi, xdim, ydim, range);
  RSL_write_pict(outfile, cart_image, xdim, ydim, color_table);
  free(cart_image);
}

/**********************************************************************/
/*                                                                    */
/*                    RSL_carpi_to_ppm                                */
/*                                                                    */
/*      Space Applications Corporation                                */
/*      17 Mar 1997                                                   */
/**********************************************************************/
void RSL_carpi_to_ppm(Carpi *carpi, char *outfile, int xdim, int ydim, float range)
{
  /* Currently range is not used. */
  unsigned char *cart_image;

  /* Assumes the color table is already loaded. */
  if (carpi == NULL) return;
  if (ncolors == 0) {
		fprintf(stderr, "No colors in color table.  Load color table first.\n");
		return;
  }
  cart_image = RSL_carpi_to_cart(carpi, xdim, ydim, range);
  RSL_write_ppm(outfile, cart_image, xdim, ydim, color_table);
  free(cart_image);
}

/**********************************************************************/
/*                                                                    */
/*                    RSL_carpi_to_pgm                                */
/*                                                                    */
/*      Space Applications Corporation                                */
/*      17 Mar 1997                                                   */
/**********************************************************************/
void RSL_carpi_to_pgm(Carpi *carpi, char *outfile, int xdim, int ydim, float range)
{
  /* Currently range is not used. */
  unsigned char *cart_image;

  /* Assumes the color table is already loaded. */
  if (carpi == NULL) return;
  if (ncolors == 0) {
		fprintf(stderr, "No colors in color table.  Load color table first.\n");
		return;
  }
  cart_image = RSL_carpi_to_cart(carpi, xdim, ydim, range);
  RSL_write_pgm(outfile, cart_image, xdim, ydim);
  free(cart_image);
}

/**********************************************************************/
/*                    RSL_rebin_volume                                */
/* More generic version of RSL_rebin_velocity. Allows for centering   */
/* +/- field like zdr or vel around zero. Width is the 1/2 width of   */
/* what is to be centered; e.g., +/- 5, width=5.                      */
/* Space Applications Corporation                                     */
/* July 13, 1997                                                      */
/**********************************************************************/
void RSL_rebin_ray(Ray *r, int width)
{
  int i;
  float nyquist, val; 
  int ncbins = 15; /* Number of color bins */
  float (*f)(Range x);
  Range (*invf)(float x);

  if (r == NULL) return;

  nyquist = width;
  if (nyquist == 0.0) {
	fprintf(stderr, "RSL_rebin_ray: nyquist == 0.0\n");
	fprintf(stderr, "RSL_rebin_ray: Unable to rebin.\n");
	return;
  }
  f = r->h.f;
  invf = r->h.invf;
  for (i=0; i<r->h.nbins; i++) {
	val = f(r->range[i]);
	if (val == width+1) {
		val = ncbins + 1;
	} else if (val != BADVAL) {
/*
	  Okay, we want to shift the data to positive values
	  then we re-scale them by the number of color bins/nyquist
*/
	  val = (int)(val/nyquist*(ncbins/2) + 1.0 + ncbins/2); 
	} else {
		val = 0;
	}

	r->range[i] = invf(val);
  }
}

void RSL_rebin_sweep(Sweep *s, int width)
{
  int i;  
  if (s == NULL) return;
  for (i=0; i<s->h.nrays; i++)
	RSL_rebin_ray(s->ray[i], width);
}

void RSL_rebin_volume(Volume *v, int width)
{
  int i;
  if (v == NULL) return;
  for (i=0; i<v->h.nsweeps; i++)
	RSL_rebin_sweep(v->sweep[i], width);
}

/**********************************************************************/
/*                                                                    */
/*                 RSL_rebin_zdr_volume, _sweep, _ray                 */
/*                                                                    */
/*      Space Applications Corporation                                */
/**********************************************************************/
/*
	 Maps valid zdr values from the real set [-6.0, 10.0] onto the set of
	 integers {0, 1, 2, ..., 35}, which forms the set of indices into
	 the zdr color table. Used in conjunction with gif image generation
	 of zdr sweeps.

	      Color coding:
				-------------
	 -6.0 <= zdr < -4.0  --> black
	 -4.0 <= zdr < -2.0  --> gray
	 -2.0 <= zdr <  0.0  --> blue
    0.0 <= zdr <  2.0  --> green
		2.0 <= zdr <  4.0  --> orange
		4.0 <= zdr <  6.0  --> red
		6.0 <= zdr <  8.0  --> pink
		8.0 <= zdr < 10.0  --> white

		Space Applications Corporation
		July 13, 1997
*/

void RSL_rebin_zdr_ray(Ray *r)
{
  int i;
  float val; 
  float (*f)(Range x);
  Range (*invf)(float x);

  if (r == NULL) return;
  f = r->h.f;
  invf = r->h.invf;
  for (i=0; i<r->h.nbins; i++)
	{
		val = f(r->range[i]);
		if ((val >= -6.0) && (val < 8.4)) val = (floor) ((val + 6.0) * 2.5);
		else if (val < 10.0) val = 35.0;  /* Make all these white. */
		else val = 0;  /* invalid zdr value */
		r->range[i] = invf(val);
  }
}

void RSL_rebin_zdr_sweep(Sweep *s)
{
  int i;  
  if (s == NULL) return;
  for (i=0; i<s->h.nrays; i++) RSL_rebin_zdr_ray(s->ray[i]);
}

void RSL_rebin_zdr_volume(Volume *v)
{
  int i;
  if (v == NULL) return;
  for (i=0; i<v->h.nsweeps; i++) RSL_rebin_zdr_sweep(v->sweep[i]);
}
