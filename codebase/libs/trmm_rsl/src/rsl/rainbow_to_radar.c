/*
    NASA/TRMM, Code 912
    This is the TRMM Office Radar Software Library.
    Copyright (C) 2004
            Bart Kelley
	    George Mason University
	    Fairfax, Virginia

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <trmm_rsl/rsl.h> 
#include <unistd.h>
#include "rainbow.h"

struct dms {
    int deg;
    int minute;
    int sec;
};

struct dms deg_to_dms(float deg)
{
    /* Convert value in degrees to degrees-minutes-seconds. */

    int sign, ideg, iminute, sec;
    float fminute;  /* minute with fractional part */
    struct dms dms;

    sign = (deg < 0.) ? -1 : 1;
    deg = sign * deg;  /* absolute value */
    ideg = deg;        /* truncated degree */
    iminute = fminute = (deg - ideg) * 60.;
    sec = (fminute - iminute) * 60. + .5;
    dms.deg = sign * ideg;
    dms.minute = sign * iminute;
    dms.sec = sign * sec;
    return dms;
}

static float (*f)(Range x);
static Range (*invf)(float x);

/**********************************************************/
/*                                                        */
/*                  RSL_rainbow_to_radar                  */
/*                                                        */
/**********************************************************/

Radar *RSL_rainbow_to_radar(char *infile)
{
    /* This function reads the Rainbow format scan data file and returns a
     * radar structure.
     */

    Radar *radar;
    FILE *fp;
    int c;
    int nvolumes;
    Rainbow_hdr rainbow_hdr;
    struct dms latdms, londms;

    /* These next lines allow program to read from a regular file, a
     * compressed file, or standard input.  I lifted them from RSL_uf_to_radar
     * by John Merritt.
     */

    if (infile == NULL) {
  	int save_fd;
  	save_fd = dup(0);
  	fp = fdopen(save_fd, "r");
    }
    else if ((fp = fopen(infile, "r")) == NULL) {
  	perror(infile);
  	return NULL;
    }
    fp = uncompress_pipe(fp); /* Transparently gunzip. */

    /* Read first character and verify file format. */

    if ((c = fgetc(fp)) != SOH) {
	fprintf(stderr,"%s is not a valid Rainbow format file.\n",infile);
  	return NULL;
    }

    /* Read Rainbow file header and check for correct product. */

    read_rainbow_header(&rainbow_hdr, fp);

    if (rainbow_hdr.filetype != SCAN_DATA ) {
	fprintf(stderr,"ERROR: File is not a scan data file.\n");
	fprintf(stderr,"File type number (header label H3) is %d\n",
		rainbow_hdr.filetype);
	fprintf(stderr,"See Rainbow File Format Document for details on "
		"header labels.\n");
	return NULL;
    }
    if (rainbow_hdr.product != VOLUME_SCAN) {
	fprintf(stderr,"WARNING: Product is not volume scan as expected.\n");
	fprintf(stderr,"Header label N is %d, expected %d\n",
		rainbow_hdr.product, VOLUME_SCAN);
	fprintf(stderr,"See Rainbow File Format Document for details on "
		"header labels.\n");
    }
    if (rainbow_hdr.compressed) {
	fprintf(stderr,"RSL_rainbow_to_radar: Label F3 indicates data "
		"compression.\n");
	fprintf(stderr,"This routine can not handle compressed data.\n");
	fprintf(stderr,"See Rainbow File Format Document for details on "
		"header labels.\n");
	return NULL;
    }

    /* Make radar structure and assign header information.  */

    nvolumes = 1; /* There is only one raw data type in a volume scan file. */
    radar = RSL_new_radar(MAX_RADAR_VOLUMES);
    if (radar == NULL) {
	perror("RSL_new_radar returned NUL to RSL_rainbow_to_radar");
	return NULL;
    }
    radar->h.nvolumes = nvolumes;
    strcpy(radar->h.radar_name, rainbow_hdr.radarname);
    strcpy(radar->h.radar_type, "rainbow");
    radar->h.month = rainbow_hdr.month;
    radar->h.day = rainbow_hdr.day;
    radar->h.year = rainbow_hdr.year;
    radar->h.hour = rainbow_hdr.hour;
    radar->h.minute = rainbow_hdr.minute;
    radar->h.sec = (float) rainbow_hdr.sec;
    latdms = deg_to_dms(rainbow_hdr.lat);
    londms = deg_to_dms(rainbow_hdr.lon);
    radar->h.latd = latdms.deg;
    radar->h.latm = latdms.minute;
    radar->h.lats = latdms.sec;
    radar->h.lond = londms.deg;
    radar->h.lonm = londms.minute;
    radar->h.lons = londms.sec;

    /* Read the data portion of the file into the radar structure. */

    if (rainbow_data_to_radar(radar, rainbow_hdr, fp) < 1)
	radar = NULL;

    return radar;
}

/**********************************************************/
/*                                                        */
/*                 rainbow_data_to_radar                  */
/*                                                        */
/**********************************************************/

int rainbow_data_to_radar(Radar *radar, Rainbow_hdr rainbow_hdr, FILE *fp)
{
    /* Read Rainbow data into the radar structure.  Data in the file is stored
     * as bytes, where each byte contains the value for one range bin.  The
     * data is ordered as follows: The first byte corresponds to the first bin
     * of the first ray, followed by the remaining bins of that ray, followed
     * by the remaining rays of the first sweep.  This sequence of bins, rays,
     * and sweeps continues for the remainder of sweeps in the volume scan.
     */

    int iray, isweep, nread, nsweeps, nrays, nbins, vol_index;
    unsigned char *rainbow_ray;
    Volume *v;
    Sweep *sweep;
    Ray *ray;
    int i;
    float azim_rate, beam_width, dz, elev_angle, prf, unam_rng;

    beam_width = 1.0; /* for now */

    switch (rainbow_hdr.datatype) {
        /* TODO: Add f and invf for each field */
	case 0:
	    vol_index = VR_INDEX;
	    f = VR_F;
	    invf = VR_INVF;
	    break;
	case 1:
	    vol_index = DZ_INDEX;
	    f = DZ_F;
	    invf = DZ_INVF;
	    break;
	case 2:
	    vol_index = SW_INDEX;
	    break;
	case 3:
	    vol_index = ZT_INDEX;
	    break;
	case 5:
	    vol_index = DR_INDEX;
	    break;
    }
    /* TODO: remove the following if-statement once the code for other data
     * types has been implemented.  Currently only handles reflectivity (DZ).
     */
    if (vol_index != DZ_INDEX) {
	fprintf(stderr, "RSL_rainbow_to_radar: currently only handles "
		"field type DZ\n");
	fprintf(stderr,"Rainbow data type value (label F9) is %d\n",
		rainbow_hdr.datatype);
	fprintf(stderr,"Corresponding vol_INDEX number is %d\n", vol_index);
	return 0;
    }

    nsweeps = rainbow_hdr.nsweeps;
    nrays = (rainbow_hdr.az_stop - rainbow_hdr.az_start + 1) /
	rainbow_hdr.az_step + .5;
    nbins = rainbow_hdr.nbins;
    if (nrays != 360) {
	fprintf(stderr,"WARNING: number of rays computed is not the number "
		"expected.\n");
	/*	fprintf(stderr,"Computed = nrays: azstart = %d, az_stop = %d, "
		"az_step = %f\n"); */
	fprintf(stderr,"Expected 360\n");
    }
    radar->v[vol_index] = RSL_new_volume(nsweeps);
    v = radar->v[vol_index];
    v->h.nsweeps = nsweeps;
    v->h.f = f;
    v->h.invf = invf;
    if (vol_index == DZ_INDEX) v->h.type_str = strdup("Reflectivity");
    rainbow_ray = (unsigned char *) malloc(nbins);
    
    /* Load sweeps. */

    for (isweep = 0; isweep < nsweeps; isweep++) {
	sweep = RSL_new_sweep(nrays);
	prf = rainbow_hdr.elev_params[isweep]->prf_high;
	unam_rng = RSL_SPEED_OF_LIGHT / (2. * prf * 1000.);
	azim_rate = rainbow_hdr.elev_params[isweep]->az_rate;
	elev_angle = rainbow_hdr.elev_params[isweep]->elev_angle;

	/* Load rays. */

	for (iray = 0; iray < nrays; iray++) {
	    nread = fread(rainbow_ray, 1, nbins, fp);
	    if (nread != nbins) {
		fprintf(stderr, "ERROR: Could not read enough bytes to fill "
			"ray.\n");
		fprintf(stderr, "Sweep = %d, ray = %d, number read = %d\n",
			isweep, iray, nread);
		return 0;
	    }
	    ray = RSL_new_ray(nbins);

	    /* TODO: Add code for other fields. */

	    if (vol_index == DZ_INDEX) {
		for (i=0; i < ray->h.nbins; i++) {
		    dz = -31.5 + (rainbow_ray[i] - 1) * 0.5;
		    if (dz < -31.5)
			ray->range[i] = invf(BADVAL);
		    else
			ray->range[i] = invf(dz);
		}
	    }
	    /* Load ray header. Note: the rainbow data file has only one time
	     * stamp, which is the data acquisition time at the start of the
	     * volume scan. For now, that is the time assigned to all rays.
	     */
	    ray->h.month = rainbow_hdr.month;
	    ray->h.day = rainbow_hdr.day;
	    ray->h.year = rainbow_hdr.year;
	    ray->h.hour = rainbow_hdr.hour;
	    ray->h.minute = rainbow_hdr.minute;
	    ray->h.sec = rainbow_hdr.sec;
	    ray->h.f = f;
	    ray->h.invf = invf;
	    ray->h.unam_rng = unam_rng;
	    ray->h.prf = prf;
	    ray->h.azim_rate = azim_rate;
	    ray->h.elev = elev_angle;
	    ray->h.elev_num = isweep + 1;
	    ray->h.fix_angle = elev_angle;
	    ray->h.azimuth = rainbow_hdr.az_start + iray * rainbow_hdr.az_step;
	    ray->h.ray_num = iray + 1;
	    ray->h.range_bin1 = rainbow_hdr.range_start;
	    ray->h.gate_size = rainbow_hdr.range_step * 1000.;
	    ray->h.beam_width = beam_width;
	    sweep->ray[iray] = ray;
	} /* iray */
	sweep->h.sweep_num = isweep + 1;
	sweep->h.beam_width = beam_width;
	sweep->h.vert_half_bw = sweep->h.beam_width * 0.5;
	sweep->h.horz_half_bw = sweep->h.beam_width * 0.5;
	sweep->h.nrays = nrays;
	sweep->h.f = f;
	sweep->h.invf = invf;
	v->sweep[isweep] = sweep;
    } /* isweep */
    return 1;
}
