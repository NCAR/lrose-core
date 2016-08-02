/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1997
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
#include <stdlib.h>
#include <unistd.h>
#include <trmm_rsl/rsl.h>
#include "africa.h"

static void ymd(int jday, int yy, int *mm, int *dd);

static int daytab[2][13] = {
  {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
  {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};

static void ymd(int jday, int year, int *mm, int *dd)
{
  /*  Input: jday, yyyy */
  /* Output: mm, dd */
  int leap;
  int i;

  leap = (year%4 == 0 && year%100 != 0) || year%400 == 0;
  for (i=0; daytab[leap][i]<jday; i++) continue;
  *mm = i;
  i--;
  *dd = jday - daytab[leap][i];
}

extern int radar_verbose_flag;

Radar *RSL_africa_to_radar(char *infile)  
{

  FILE *fp;
  Africa_ray *ray;
  Africa_sweep *sweep;
  int n;
  int hour, min=0, sec=0, month, day, year, jday;
  float elev, azim, s_azim, e_azim;
  int iray, ielev, isite;

  Radar *radar;
  Volume *v;
  Sweep  *s;
  Ray    *r;
  int     i,ibin;
  int save_fd;

  if (infile == NULL) {
	save_fd = dup(0);
	fp = fdopen(save_fd, "r");
  } else {
	fp = fopen(infile, "r");
  }
  fp = uncompress_pipe(fp);
  n = 0;
  
  radar = RSL_new_radar(MAX_RADAR_VOLUMES);
  radar->v[DZ_INDEX] = RSL_new_volume(20);
  v = radar->v[DZ_INDEX];

  for(i=0; (sweep = africa_read_sweep(fp)); i++) {

	/* Load the sweep into the radar volume */
	v->sweep[i] = RSL_new_sweep((int)sweep->nrays);
	s = v->sweep[i];
	if (radar_verbose_flag) printf("NUMBER OF RAYS: %d\n", sweep->nrays);
	for (n=0; n < sweep->nrays; n++) {
	  ray = sweep->ray[n];
	  if (ray == NULL) continue;
	  year = ray->yearjday/512;
	  jday = ray->yearjday & 0x1ff;
	  year += 1900;
	  if (year < 1970) year += 100; /* >=2000 */
	  ymd(jday, year, &month, &day);
	  hour = ray->hour;
	  min  = ray->minute_sec/60;
	  sec  = ray->minute_sec - min*60;
	  
	  elev = africa_bcd_convert (ray->bcd_elevation);
	  azim = africa_bcd_convert (ray->bcd_azimuth);
	  s_azim = africa_bcd_convert (ray->bcd_start_azim);
	  e_azim = africa_bcd_convert (ray->bcd_end_azim);
	  
	  /* Values that I CANNOT trust.
	   *    e_azim
	   *
	   * DATA ORGANIZATION:
	   *   Usually, 10 ray groups for each field type.  Field type is in
	   *            ray->xmit_power_site (Doc wrong?)
	   *   iray == 0 when at the end of the sweep.  (Don't need lookahead)
	   */
	  
	  ielev = ray->raycount/512;
	  iray  = ray->raycount - ielev*512;
	  isite = ray->xmit_power_site & 0x1f;
	  printf("Record %d, time = %.2d:%.2d:%.2d %.2d/%.2d/%.2d  --> elev,azim = %f, %f.  Start: %f, %f, iray/ielev %d %d, site=%d\n", n,
			 hour, min, sec, month, day, year-1900, elev, azim, s_azim, e_azim, iray, ielev, isite);
	  

	  if (isite != 22) continue;
	  /* ONLY LOAD from isite==22, for now. */
	  r = RSL_new_ray(224);
	  s->ray[n]  = r;
	  r->h.month = month;
	  r->h.day   = day;
	  r->h.year  = year;
	  r->h.hour  = hour;
	  r->h.minute  = min;
	  r->h.sec   = sec;
	  r->h.azimuth = azim;
	  r->h.ray_num = iray;
	  r->h.elev    = elev;
	  r->h.elev_num = ielev;
	  r->h.beam_width = 1.0;
	  r->h.gate_size = 1000;

	  /* What are the others? */

	  r->h.invf = DZ_INVF;
	  r->h.f    = DZ_F;
	  s->h.horz_half_bw = 0.5;
	  s->h.vert_half_bw = 0.5;
	  s->h.beam_width = r->h.beam_width;
		
	  for (ibin=0; ibin<r->h.nbins; ibin++) {
		r->range[ibin] = r->h.invf(ray->bin[ibin]/8.0/100.0 + 0.5);
	  }
	}
  }
  fclose(fp);

  sprintf(radar->h.radar_type, "south_africa");
  sprintf(radar->h.name, "SAFRICA");
  sprintf(radar->h.radar_name, "SAFRICA");
  sprintf(radar->h.city, "I don't know");
  sprintf(radar->h.state, "??");
  sprintf(radar->h.country, "South Africa");

  radar_load_date_time(radar);
  return radar;
}
