/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996, 1997, 1998
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
#include <stdio.h>
#include <string.h>
#include <trmm_rsl/rsl.h> 

#ifdef HAVE_LIBIMPLODE
#include "radtec.h"

extern int radar_verbose_flag;

static void fill_ray_header(Ray_header *h, Radtec_header *rh, Radtec_ray_header *rrh)
{
  
  /* Fill the limited ray header information. */
  h->month = rh->month; /* Time for this ray; month (1-12). */
  h->day   = rh->day;   /* Time for this ray; day (1-31).   */
  h->year  = rh->year;  /* Time for this ray; year (eg. 1993). */
  h->hour  = rrh->hour;  /* Date for this ray; hour (0-23). */
  h->minute = rrh->min;/* Date for this ray; minute (0-59).*/
  h->sec   = rrh->sec; /* Date for this ray; second + fraction of second. */
  h->azimuth  = rrh->azim_angle;   /* Azimuth angle. (degrees). Must be positive
					 * 0=North, 90=east, -90/270=west.
					 * This angle is the mean azimuth for the whole ray.
					 * Eg. for NSIG the beginning and end azimuths are
					 *     averaged.
					 */
  h->ray_num = rrh->ray_num;    /* Ray no. within elevation scan. */
  h->elev    = rrh->elev_angle;       /* Elevation angle. (degrees). */
  
  h->gate_size  = rh->range_bin_size * RSL_SPEED_OF_LIGHT / 1e6;  /* Data gate size (meters)*/
  h->lat = rh->site_latitude;          /* Latitude (degrees) */
  h->lon = rh->site_longitude;          /* Longitude (degrees) */
  h->alt = rh->site_elevation;          /* Altitude (m) */
  h->nbins = rh->num_range_bins;             /* Number of array elements for 'Range'. */
  h->beam_width = rh->azim_resolution;  /* Beamwidth in degrees. */
  
#ifdef RADTEC_UNKNOWN
  h->unam_rng = 0;  /* Unambiguous range. (KM). */
  h->elev_num = 0;   /* Elevation no. within volume scan. */
  h->range_bin1 = 0; /* Range to first gate.(meters) */
  h-> vel_res = 0;   /* Doppler velocity resolution */
  h->sweep_rate = 0; /* Sweep rate. Full sweeps/min. */
  h->prf = 0;          /* Pulse repetition frequency, in Hz. */
  h->azim_rate;  /* degrees/sec */
  h->fix_angle;
  h->pitch;      /* Pitch angle. */
  h->roll;       /* Roll  angle. */
  h->heading;    /* Heading. */
  h->pitch_rate; /* (angle/sec) */
  h->roll_rate;  /* (angle/sec) */
  h->heading_rate; /* (angle/sec) */
  h->rvc;          /* Radial velocity correction (m/sec) */
  h->vel_east;     /* Platform velocity to the east  (m/sec) */
  h->vel_north;    /* Platform velocity to the north (m/sec) */
  h->vel_up;       /* Platform velocity toward up    (m/sec) */
  h->pulse_count; /* Pulses used in a single dwell time. */
  h->pulse_width; /* Pulse width (micro-sec). */
  h->frequency;   /* Carrier freq. GHz. */
  h->wavelength;  /* Wavelength. Meters. */
  h->nyq_vel;     /* Nyquist velocity. m/s */
#endif

  return;
}


Radar *RSL_radtec_to_radar(char *infile)
{
  Radar  *radar;
  Volume *volume;
  Sweep  *sweep;
  Ray    *ray;
  float (*f)(Range x);       /* Data conversion function. f(x). */
  Range (*invf)(float x);    /* Data conversion function. invf(x). */

  Radtec_file *rfile;
  float tmp;
  int nsweeps, nrays, nbins;
  int isweep, iray, ibin;
  float sum_elev;

  rfile = radtec_read_file(infile);

  if (radar_verbose_flag) {
	radtec_print_header(&rfile->h);
  }

  /* Have the entire radtec file; headers and data.
   * Initialize the RSL radar and load it up.
   *
   * Handles both RHI and PPI scan types.
   */

  /* Only one field type per file. */
  radar = RSL_new_radar(MAX_RADAR_VOLUMES);

  if (rfile->h.scan_type == 1) { /* PPI */
	nrays   = (int)(360.0 / rfile->h.azim_resolution + 0.5);
	nsweeps = (int)(rfile->h.num_rays / nrays + 0.5);
  } else { /* RHI */
	nrays   = 1;
	nsweeps = (int)(rfile->h.num_rays / nrays + 0.5);
	/* Each sweep will contain one ray for RHI scans. */
  }
  nbins = rfile->h.num_range_bins;

  if (radar_verbose_flag) {
	fprintf(stderr,"Expecting %d sweeps.\n", nsweeps);
	fprintf(stderr,"Expecting %d rays.\n", nrays);
	fprintf(stderr,"Expecting %d bins.\n", nbins);
  }

  if (rfile->h.scan_mode == 0) { /* Log video == Reflectivity. */
	f    = DZ_F;
	invf = DZ_INVF;
	volume = radar->v[DZ_INDEX] = RSL_new_volume(nsweeps);
	volume->h.type_str = strdup("Reflectivity");
  } else { /* Doppler */
	f    = VR_F;
	invf = VR_INVF;
	volume = radar->v[VR_INDEX] = RSL_new_volume(nsweeps);
	volume->h.type_str = strdup("Velocity");
  }
  volume->h.f = f;
  volume->h.invf = invf;

  /* Load the radar header information. */
  radar->h.month = rfile->h.month;
  radar->h.day   = rfile->h.day;
  radar->h.year  = rfile->h.year;
  radar->h.hour  = rfile->h.hour;
  radar->h.minute = rfile->h.min;
  radar->h.sec    = rfile->h.sec; /* Second plus fractional part. */
  sprintf(radar->h.radar_type,"radtec");

  radar->h.number = rfile->h.version; /* arbitrary number of this radar site */
  sprintf(radar->h.name,"radtec");      /* Nexrad site name */
  sprintf(radar->h.radar_name,"RADTEC"); /* Radar name. */
  sprintf(radar->h.city,"Unknown"); /* nearest city to  radar site */
  sprintf(radar->h.state,"??");     /* state of radar site */
  sprintf(radar->h.country,"???");

   /** Latitude deg, min, sec **/
  radar->h.latd = (int)rfile->h.site_latitude;
  tmp = (rfile->h.site_latitude - radar->h.latd) * 60.0;
  radar->h.latm = (int)tmp;
  radar->h.lats = (int)((tmp - radar->h.latm) * 60.0);
   /** Longitude deg, min, sec **/
  radar->h.lond = (int)rfile->h.site_longitude;
  tmp = (rfile->h.site_longitude - radar->h.lond) * 60.0;
  radar->h.lonm = (int)tmp;
  radar->h.lons = (int)((tmp - radar->h.lonm) * 60.0);
  radar->h.height = rfile->h.site_elevation; /* height of site in meters above sea level*/
  radar->h.spulse = 0; /* length of short pulse (ns)*/
  radar->h.lpulse = 0; /* length of long pulse (ns) */

  /* Loop through the RSL number of sweeps and attach the data. */
  for (isweep = 0; isweep < nsweeps; isweep++) {
	sweep = volume->sweep[isweep] = RSL_new_sweep(nrays);
	sweep->h.f = f;
	sweep->h.invf = invf;
	sum_elev = 0;
	for (iray = 0; iray < nrays; iray++) {
	  ray = sweep->ray[iray] = RSL_new_ray(nbins);
	  ray->h.f = f;
	  ray->h.invf = invf;
	  fill_ray_header(&ray->h, &rfile->h, rfile->ray[iray].h);
	  RSL_fix_time(ray);
	  sum_elev += ray->h.elev;
	  /* Fill the data. */
	  for (ibin=0; ibin < nbins; ibin++) {
		/*		printf ("ray[%d].dbz[%d] = %f\n", iray, ibin, rfile->ray[iray].dbz[ibin]); */
		ray->range[ibin] = invf(rfile->ray[iray].dbz[ibin]);
	  }
	}
	sweep->h.elev = sum_elev / nrays;
    sweep->h.beam_width = rfile->h.azim_resolution;  /* This is in the ray header too. */
    sweep->h.vert_half_bw = sweep->h.beam_width/2;  /* Vertical beam width divided by 2 */
    sweep->h.horz_half_bw = sweep->h.beam_width/2;  /* Horizontal beam width divided by 2 */
  }

  radtec_free_file(rfile);
  rfile = NULL;

  return radar;
}
#else
Radar *RSL_radtec_to_radar(char *infile)
{
  fprintf(stderr, "RADTEC is not installed in this version of RSL.\n");
  fprintf(stderr, "The library libimplode.a (or .so) was not found during\n");
  fprintf(stderr, "the RSL configuration.\n");
  return NULL;
}
#endif
