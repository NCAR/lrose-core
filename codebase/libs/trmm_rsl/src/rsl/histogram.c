/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996, 1997
            David B. Wolff
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
#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include <unistd.h>
#include <trmm_rsl/rsl.h>

/*********************************************************************/
/*                                                                   */
/* Unless otherwise stated, all routines in this file are            */
/* were coded by David B. Wolff, Space Applications Corporation,     */
/* Landover, MD.                                                     */
/*                                                                   */
/*********************************************************************/

extern int radar_verbose_flag;

/**********************************************************/
/* This set of functions and typedefs create a histogram  */
/* from a volume, sweep or ray. The structure of the      */
/* histogram is as such:                                  */
/*                                                        */
/*   histogram->nbins  # number of bins in histogram      */
/*   histogram->low    # Low end of histogram             */
/*   histogram->hi     # High end of histogram            */
/*   histogram->ucount # Total number of bins looked at   */
/*                     # in calculating histogram         */
/*   histogram->ccount # Valid number of bins looked at   */
/*                     # in calculating histogram         */
/*   histogram->data[] # Counts of each bin (How is this  */
/*                     # allocated????                    */
/**********************************************************/
/** DBW and JHM *******************************************/
/**********************************************************/


Histogram *RSL_allocate_histogram(int low, int hi)
{
  Histogram *histogram;
  int nbins;

  nbins = hi - low + 1;
  histogram = (Histogram *)calloc(1, sizeof(Histogram));
  if (histogram) {
	histogram->nbins  = nbins;
	histogram->low    = low;
	histogram->hi     = hi;
	histogram->ucount = 0;
	histogram->ccount = 0;
    histogram->data = (int *)calloc(nbins, sizeof(int));
  }
  return histogram;
}

/*********************************************************************/
/*                                                                   */
/*                     RSL_free_histogram                               */
/*                                                                   */
/*********************************************************************/
void RSL_free_histogram(Histogram *histogram)
{
  if (histogram == NULL) return;
  if (histogram->data) free(histogram->data);
  free(histogram);
}

/*********************************************************/
/** DBW and JHM ******************************************/
/*********************************************************/
void RSL_print_histogram(Histogram *histogram, int min_range, int max_range,
					 char *filename)
{
	float    PDF[200], CDF[200];

	int      nbins;
	float    ucount, ccount;
	
	int      i, index;
	FILE     *fp;

/* Print the stats on this histogram */

    if(histogram == NULL) {
		perror("Cannot print NULL histogram");
		return;
	}

    if (radar_verbose_flag) fprintf(stderr,"print_histogram: %s\n",filename);
	if((fp = fopen(filename,"w")) == NULL ) {
		perror(filename);
		return;
    }
	fprintf(fp,"Min range = %d\n", min_range);
	fprintf(fp,"Max range = %d\n", max_range);
	fprintf(fp,"histogram->nbins  = %d\n",histogram->nbins);
	fprintf(fp,"histogram->low    = %d\n",histogram->low);
	fprintf(fp,"histogram->hi     = %d\n",histogram->hi);
	fprintf(fp,"histogram->ucount = %d\n",histogram->ucount);
	fprintf(fp,"histogram->ccount = %d\n",histogram->ccount);

/* 
   Compute the PDF and CDF from the histogram.
   PDF = histogram->data[i]/ucount (PDF)
   CDF = runsum(PDF)               (CDF)
*/
	nbins   = histogram->nbins;
	ucount  = histogram->ucount;
	ccount  = histogram->ccount;
	
	for(i=0; i<nbins; i++) {
		PDF[i] = histogram->data[i]/ucount;
        PDF[i] = histogram->data[i]/ccount;
    }
		   
    CDF[0] = PDF[0];
    for(i=0; i<nbins-1; i++) 
	    CDF[i+1] = CDF[i] + PDF[i];

    index = histogram->low;

    fprintf(fp,"\nBin Count PDF CDF\n");
    for(i=0; i<nbins; i++) {
        fprintf(fp,"%d %d %f %f\n", index, histogram->data[i], PDF[i],CDF[i]);
        index++;
    }
	fclose(fp);
}

Histogram *RSL_read_histogram(char *infile)
{

  FILE *fp;
  int n;
  int low, hi;
  int nbins;
  Histogram *histogram;

  if ((fp = fopen(infile, "r")) == NULL) {
	perror(infile);
	return NULL;
  }

/*  
	1. nbins, ucount, count
    2. data[0..nbins-1]
*/
  n = 0;
  n += fread(&nbins, sizeof(nbins), 1, fp);
  n += fread(&low, sizeof(int), 1, fp);
  n += fread(&hi, sizeof(int), 1, fp);

  if ((histogram = RSL_allocate_histogram(low, hi)) == NULL) {
	perror("read_histogram");
	return NULL;
  }
  if(histogram->low != low || histogram->hi != hi) {
  	perror("Incompatible histograms");
  	return NULL;
  }
  n += fread(&histogram->ucount, sizeof(int), 1, fp);
  n += fread(&histogram->ccount, sizeof(int), 1, fp);
  n += fread(histogram->data, sizeof(int), nbins, fp);
  fclose(fp);
  return histogram;
}

int RSL_write_histogram(Histogram *histogram, char *outfile)
{

  FILE *fp;
  int n;

  if ((fp = fopen(outfile, "w")) == NULL) {
	perror(outfile);
	return -1;
  }

/*  
	1. nbins, ucount, count
    2. data[0..nbins-1]
*/
  n = 0;
  n += fwrite(&histogram->nbins, sizeof(int), 1, fp);
  n += fwrite(&histogram->low, sizeof(int), 1, fp);
  n += fwrite(&histogram->hi, sizeof(int), 1, fp);
  n += fwrite(&histogram->ucount, sizeof(int), 1, fp);
  n += fwrite(&histogram->ccount, sizeof(int), 1, fp);
  n += fwrite(histogram->data, sizeof(int), histogram->nbins, fp);
  fclose(fp);
  return n;
}
/*********************************************************/
/** DBW and JHM ******************************************/
/*********************************************************/

Histogram *RSL_get_histogram_from_ray(Ray *ray, Histogram *histogram,
	int low, int hi, int min_range, int max_range)
{
	int   i, index;
	float dbz, ray_resolution, range;
	float (*f)(Range x);
	
	if (histogram == NULL ) {
		if (radar_verbose_flag) fprintf(stderr,"Allocating histogram at ray level\n");
		histogram = RSL_allocate_histogram(low, hi);
	}

	if(ray != NULL) {
		ray_resolution = ray->h.gate_size/1000.0;
		f = ray->h.f;
		for(i=0; i<ray->h.nbins; i++) {
			histogram->ucount++;
			range = i*ray_resolution + ray->h.range_bin1/1000.;
			if(range < min_range) continue;
			if(range > max_range) break;
			dbz = f(ray->range[i]);
			if(dbz >= histogram->low && dbz <= histogram->hi) {
				index = dbz - histogram->low;
				histogram->ccount++;
				histogram->data[index]++;
			}
		}
	}
	return histogram;
}

Histogram *RSL_get_histogram_from_sweep(Sweep *sweep, Histogram *histogram, 
	int low, int hi, int min_range, int max_range)
{
	int i;
	if (histogram == NULL ) {
		if (radar_verbose_flag) fprintf(stderr,"Allocating histogram at sweep level\n");
		histogram = RSL_allocate_histogram(low, hi);
	}
	if(sweep != NULL) {
		for(i=0; i<sweep->h.nrays; i++) {
			histogram = RSL_get_histogram_from_ray(sweep->ray[i], histogram, 
							low, hi, min_range, max_range);
		}
	}

	return histogram;
}

Histogram *RSL_get_histogram_from_volume(Volume *volume, Histogram *histogram,
	int low, int hi, int min_range, int max_range)
{
	int i;
	if (histogram == NULL ) {
		if (radar_verbose_flag) fprintf(stderr,"Allocating histogram at volume level\n");
		histogram = RSL_allocate_histogram(low, hi);
	}
	if(volume == NULL) return NULL;
	for(i=0; i<volume->h.nsweeps; i++) {
		histogram = RSL_get_histogram_from_sweep(volume->sweep[i], histogram, 
						low, hi, min_range,max_range);
	}
	return histogram;
}
