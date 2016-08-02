/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996-1999
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
#include <strings.h>
#include <string.h>
#define USE_RSL_VARS
#include <trmm_rsl/rsl.h>
#include "dorade.h"

extern int radar_verbose_flag;

/********************************************************************/
/*                                                                  */
/*                    find_rsl_field_index                          */
/*                                                                  */
/********************************************************************/
int find_rsl_field_index(char *dorade_field_name)
{
  /*  
   * Dorade: VE, DM, SW, DBZ, ZDR, PHI, RHO, LDR,  DX,  CH,  AH,  CV,  AV
   *    RSL: VR, DM, SW, DZ,  ZD,  PH,  RH,  LR,  *DX, *CH, *AH, *CV, *AV.
   */
  if (strncasecmp(dorade_field_name, "ve", 2) == 0)  return VR_INDEX;
  if (strncasecmp(dorade_field_name, "vr", 2) == 0)  return VR_INDEX;
  if (strncasecmp(dorade_field_name, "dm", 2) == 0)  return DM_INDEX;
  if (strncasecmp(dorade_field_name, "sw", 2) == 0)  return SW_INDEX;
  if (strncasecmp(dorade_field_name, "dbz", 3) == 0) return DZ_INDEX;
  if (strncasecmp(dorade_field_name, "zdr", 3) == 0) return ZD_INDEX;
  if (strncasecmp(dorade_field_name, "phi", 3) == 0) return PH_INDEX;
  if (strncasecmp(dorade_field_name, "rho", 3) == 0) return RH_INDEX;
  if (strncasecmp(dorade_field_name, "ldr", 3) == 0) return LR_INDEX;
  if (strncasecmp(dorade_field_name, "dx", 2) == 0)  return DX_INDEX;
  if (strncasecmp(dorade_field_name, "ch", 2) == 0)  return CH_INDEX;
  if (strncasecmp(dorade_field_name, "ah", 2) == 0)  return AH_INDEX;
  if (strncasecmp(dorade_field_name, "cv", 2) == 0)  return CV_INDEX;
  if (strncasecmp(dorade_field_name, "av", 2) == 0)  return AV_INDEX;
  if (strncasecmp(dorade_field_name, "vs", 2) == 0)  return VS_INDEX;
  if (strncasecmp(dorade_field_name, "vl", 2) == 0)  return VL_INDEX;
  if (strncasecmp(dorade_field_name, "vg", 2) == 0)  return VG_INDEX;
  if (strncasecmp(dorade_field_name, "vt", 2) == 0)  return VT_INDEX;
  if (strncasecmp(dorade_field_name, "ncp", 2) == 0) return NP_INDEX;

  return -1;
}

void prt_skipped_field_msg(char *dorade_field_name)
{
  char prtname[9];
  int i, already_printed;
#define MAXFIELDS 20
  static int nskipped = 0;
  static char skipped_list[MAXFIELDS][9];

  /* Make sure name is a properly formed string. */
  strncpy(prtname, dorade_field_name, 8);
  prtname[8] = '\0';

  /* We don't want to repeat messages for the same field, so keep a list of
   * fields already printed.
   */
  already_printed = 0;
  i = 0;
  while (!already_printed && i < nskipped) {
    if (strncmp(prtname, skipped_list[i], 2) == 0) already_printed = 1;
    i++;
  }
  if (!already_printed) {
    fprintf(stderr, "Unknown DORADE parameter type <%s> -- skipping.\n", prtname);
    strcpy(skipped_list[nskipped], prtname);
    nskipped++;
  }
}

/* For date conversion routines. */
static int daytab[2][13] = {
  {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
  {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};

/*************************************************************/
/*                                                           */
/*                          julian                           */
/*                                                           */
/*************************************************************/
static int julian(int year, int mo, int day)
{
/* Converts a calendar date (month, day, year) to a Julian date. 
	 Returns:
	   Julian day.
*/
  int leap;

  leap = (year%4 == 0 && year%100 != 0) || year%400 == 0;
  return(day + daytab[leap][mo-1]);
}

/*************************************************************/
/*                                                           */
/*                           ymd                             */
/*                                                           */
/*************************************************************/
static void ymd(int jday, int year, int *mm, int *dd)
{
  /* Input: jday, yyyy */
  /* Output: mm, dd */
  /* Copied from hdf_to_radar.c, written by Mike Kolander. */

  int leap;
  int i;

  leap = (year%4 == 0 && year%100 != 0) || year%400 == 0;
  for (i=0; daytab[leap][i]<jday; i++) continue;
  *mm = i;
  i--;
  *dd = jday - daytab[leap][i];
}

/* Secretly defined in uf_to_radar.c */
Volume *copy_sweeps_into_volume(Volume *new_volume, Volume *old_volume);

/**********************************************************************/
/*                                                                    */
/*                       RSL_dorade_to_radar                          */
/*                                                                    */
/**********************************************************************/
Radar *RSL_dorade_to_radar(char *infile)
{
  Radar  *radar;
  Volume *new_volume;
  Sweep  *sweep;
  Ray    *ray;
  int iv, iray, iparam;
  int range_bin1, gate_size;

  int nbins, data_len, word_size; 
  short *ptr2bytes;
  int   *ptr4bytes;
  float scale, offset, value;
  int datum, missing_data_flag;

  /* float (*f)(Range x);
     Range (*invf)(float x); Defined but not used - Niles */

  FILE  *fp;
  Comment_block   *cb;
  Volume_desc     *vd;
  Sensor_desc    **sd;
  Sweep_record    *sr;
  Radar_desc      *rd;
  Data_ray        *dray;
  Parameter_data  *pd;
  Ray_info        *ray_info;
  Platform_info   *platform_info;

  int nsweep;
  int i;
  char buf[1024];

  int degree, minute;
  float second;

  int year, month, day, jday, jday_vol;

  radar = NULL;
  if (infile == NULL) {
    int save_fd;
    save_fd = dup(0);
    fp = fdopen(save_fd, "r");
  }  else
    if((fp=fopen(infile, "r"))==(FILE *)NULL) {
      perror(infile);
      return radar;
    }

  fp = uncompress_pipe(fp); /* Transparently, use gunzip. */

  cb = dorade_read_comment_block(fp);

  /**********************************************************************/

  vd = dorade_read_volume_desc(fp);   /* R E A D */
  if (radar_verbose_flag)   dorade_print_volume_desc(vd);  /* P R I N T */

  /* R E A D */
  sd = (Sensor_desc **) calloc(vd->nsensors, sizeof(Sensor_desc *));
  for (i=0; i<vd->nsensors; i++) {
    sd[i] = dorade_read_sensor(fp);
  }

  /* P R I N T */
  if (radar_verbose_flag) {
    for (i=0; i<vd->nsensors; i++) {
      fprintf(stderr, "============ S E N S O R   # %d =====================\n", i);
      dorade_print_sensor(sd[i]);
    }
  }
  /* R E A D   sweeps. */
  if (vd->nsensors > 1) {
    fprintf(stderr, "RSL_dorade_to_radar: Unable to process for more than 1 sensor.\n");
    fprintf(stderr, "RSL_dorade_to_radar: Number of sensors is %d\n", vd->nsensors);
    return NULL;
  }

  /* Use sensor 0 for vitals. */
  rd = sd[0]->radar_desc;
  range_bin1 = sd[0]->cell_range_vector->range_cell[0];
  gate_size =  sd[0]->cell_range_vector->range_cell[1] - range_bin1;

  radar = RSL_new_radar(MAX_RADAR_VOLUMES);
  radar->h.month = vd->month;
  radar->h.day   = vd->day;
  radar->h.year  = vd->year;
  radar->h.hour  = vd->hour;
  radar->h.minute = vd->minute;
  radar->h.sec    = vd->second;
  sprintf(radar->h.radar_type, "dorade");
  radar->h.number = 0;
  strncpy(radar->h.name, vd->flight_num, sizeof(radar->h.name));
  strncpy(radar->h.radar_name, rd->radar_name, sizeof(radar->h.radar_name));
  strncpy(radar->h.project, vd->project_name, sizeof(radar->h.project));
  sprintf(radar->h.city, "Unknown");
  strncpy(radar->h.state, "UKN", 3);
  sprintf(radar->h.country, "Unknown");
  /* Convert lat to d:m:s */
  degree = (int)rd->latitude;
  minute = (int)((rd->latitude - degree) * 60);
  second = (rd->latitude - degree - minute/60.0) * 3600.0;
  radar->h.latd = degree;
  radar->h.latm = minute;
  radar->h.lats = second;
  /* Convert lat to d:m:s */
  degree = (int)rd->longitude;
  minute = (int)((rd->longitude - degree) * 60);
  second = (rd->longitude - degree - minute/60.0) * 3600.0;
  radar->h.lond = degree;
  radar->h.lonm = minute;
  radar->h.lons = second;
  radar->h.height = rd->altitude * 1000.0;
  radar->h.spulse = 0; /* FIXME */
  radar->h.lpulse = 0; /* FIXME */

  year = vd->year;
  month = vd->month;
  day = vd->day;
  jday_vol = julian(year, month, day);

  /* TODO:
     Get any threshold values from parameter descriptions.
     Note that the indicater for no threshold parameter is supposed to be the
     string 'NONE', but has sometimes been found to be a string of all spaces
     instead.  The corresponding missing-data-flag may also vary, the nominal
     value being -999 but has also been -32768.

     Pseudo code:
     nparam = rd->nparam_desc; [<--real code]
     create string array for threshold parameter names, size nparams.
     for each param:
       strcpy thresh param name from param desc to threshold param array.
       if string is all blanks, replace with 'NONE'.
     endfor
   */

  /* Begin volume code. */
  /* We don't know how many sweeps per volume exist, until we read
   * the file.  So allocate a large number of pointers, hope we don't
   * exceed it, and adjust the pointer array at the end.  This is 
   * efficient because we'll be manipulating pointers to the sweeps and
   * not the sweeps themselves.
   */

  if (radar_verbose_flag)
    fprintf(stderr, "Number of parameters: %d\n", rd->nparam_desc);

  /* All the parameters are together, however, their order within
   * the ray is not guarenteed.  For instance, VE could appear after
   * DM.  For this we'll keep a list of parameter names and perform
   * a (linear) search.  The result will be an index into the RSL
   * volume array (radar->v[i]).  It is likely that the order will be
   * consistant within a file, therefore, we'll keep track of the index of
   * our previous parameter type and begin the search from there; the next
   * index should be a match.
   *
   * The dorade parameter names and the rsl mapping is:
   *
   * Dorade: VE, DM, SW, DBZ, ZDR, PHI, RHO, LDR,  DX,  CH,  AH,  CV,  AV
   *    RSL: VR, DM, SW, DZ,  ZD,  PH,  RH,  LR,  *DX, *CH, *AH, *CV, *AV.
   * 
   *    * means this is a new RSL name.
   */

#define DORADE_MAX_SWEEP 20
  nsweep = 0;
  while((sr = dorade_read_sweep(fp, sd))) {
    for(iray = 0; iray < sr->nrays; iray++) {
      dray = sr->data_ray[iray];

      /* Now, loop through the parameters and fill the rsl structures. */
      for (iparam = 0; iparam < dray->nparam; iparam++) {
        pd = dray->parameter_data[iparam];
        iv = find_rsl_field_index(pd->name);
        if (iv < 0) {
          prt_skipped_field_msg(pd->name);
          continue;
        }
        if (radar->v[iv] == NULL) {
          radar->v[iv] = RSL_new_volume(DORADE_MAX_SWEEP); /* Expandable */
        } else if (nsweep >= radar->v[iv]->h.nsweeps) {
          /* Must expand the number of sweeps. */
          /* Expand by another DORADE_MAX_SWEEP. */
          if (radar_verbose_flag) {
            fprintf(stderr, "nsweeps (%d) exceeds radar->v[%d]->h.nsweeps (%d)."
              "\n", nsweep, iv, radar->v[iv]->h.nsweeps);
            fprintf(stderr, "Increasing it to %d sweeps\n",
              radar->v[iv]->h.nsweeps+DORADE_MAX_SWEEP);
          }
          new_volume = RSL_new_volume(radar->v[iv]->h.nsweeps+DORADE_MAX_SWEEP);
          /* Look in uf_to_radar.c for 'copy_sweeps_into_volume' */
          new_volume = copy_sweeps_into_volume(new_volume, radar->v[iv]);
          radar->v[iv] = new_volume;
        }
	/* Assign f and invf
	switch (iv) . . .
	 * Or just use RSL_f_list[iv] and RSL_invf_list[iv] as in sweep.h below.
	 */

        /* Allocate the ray and load the parameter data. */
        if ((sweep = radar->v[iv]->sweep[nsweep]) == NULL) {
          sweep = radar->v[iv]->sweep[nsweep] = RSL_new_sweep(sr->s_info->nrays);
          sweep->h.sweep_num    = sr->s_info->sweep_num;
          sweep->h.elev         = sr->s_info->fixed_angle;
          sweep->h.beam_width   = rd->horizontal_beam_width;
          sweep->h.vert_half_bw = rd->vertical_beam_width / 2.0;
	  /*
          sweep->h.vert_half_bw = radar->v[iv]->sweep[nsweep]->h.beam_width / 2.0;
	  */
          sweep->h.horz_half_bw = rd->horizontal_beam_width / 2.0;
          sweep->h.f = RSL_f_list[iv];
          sweep->h.invf = RSL_invf_list[iv];
        }
        

	data_len = dray->data_len[iparam];
	word_size = dray->word_size[iparam];
	if (word_size != 2 && word_size != 4) {
	  fprintf(stderr,"RSL_dorade_to_radar: dray->word_size[%d] = %d\n",
		  iparam, dray->word_size[iparam]);
	  fprintf(stderr,"Expected value is 2 or 4.\n");
	  return NULL;
	}
	nbins = data_len / word_size;

        if ((ray = sweep->ray[iray]) == NULL) {
          if (radar_verbose_flag)
            fprintf(stderr, "Allocating %d bins for ray %d\n",
	      dray->data_len[iparam], iray);
          ray = sweep->ray[iray] = RSL_new_ray(nbins);
        }

	/* TODO: Load ray header. */
	/* Get info from RYIB */
	/*
	ymd(jday, year, &month, &day);
	Instead of calling ymd every ray, just compute jday for date given
	in VOLD (vd) and compare it with jday from ray for each ray.
	Only need to recompute month and day if it changes.
	  If new jday != prev jday, then recompute month day:
	    If new jday < prev jday, change of year: increment year
	    Run ymd.
          Endif
	*/
	ray_info = dray->ray_info;
	jday = ray_info->jday;
	if (jday != jday_vol) {
	  if (jday > 0 && jday < 367) {
	    /* TODO Recompute year month day */
	    if (jday < jday_vol) year++;
	    ymd(jday, year, &month, &day);
	    jday_vol = jday;
	  }
	  else { /* Invalid jday */
	  }
	}

	ray->h.year     = year;
	ray->h.month    = month;
	ray->h.day      = day;
	ray->h.hour     = ray_info->hour;
	ray->h.minute   = ray_info->minute;
	ray->h.sec      = ray_info->second + ray_info->msec / 1000.;
	ray->h.azimuth  = ray_info->azimuth;
	ray->h.ray_num  = iray + 1;
	ray->h.elev     = ray_info->elevation;
	ray->h.elev_num = ray_info->sweep_num;
	ray->h.unam_rng = rd->unambiguous_range;
	ray->h.nyq_vel = rd->unambiguous_velocity;
	ray->h.azim_rate = ray_info->scan_rate;
	/* TODO
	ray->h.fix_angle = ;
	*/
	ray->h.range_bin1 = range_bin1;
	ray->h.gate_size = gate_size;
	platform_info = dray->platform_info;
	ray->h.pitch = platform_info->pitch;
	ray->h.roll = platform_info->roll;
	ray->h.heading = platform_info->heading;
	ray->h.pitch_rate = platform_info->pitch_rate;
	ray->h.heading_rate = platform_info->heading_rate;
	ray->h.lat = platform_info->latitude;
	ray->h.lon = platform_info->longitude;
	ray->h.alt = platform_info->altitude;
	ray->h.vel_east = platform_info->ew_speed;
	ray->h.vel_north = platform_info->ns_speed;
	ray->h.vel_up = platform_info->v_speed;
        /* Does DORADE have doppler velocity res?
	      ray->h.vel_res =
	*/
	ray->h.beam_width = rd->horizontal_beam_width;
	/* TODO
	ray->h.frequency =
	ray->h.wavelength = 
	ray->h.pulse_width = 
	ray->h.pulse_count = 
	ray->h.prf = 
	*/
	ray->h.nbins = nbins;
	ray->h.f = RSL_f_list[iv];
	ray->h.invf = RSL_invf_list[iv];

        /* Copy the ray data into the RSL ray. */

        /* .... fill here .... */

	/* Assign pointer to data.
	 * Get datum using word size and proper cast.
	 * Convert and store in rsl ray->range.
	 * Increment pointer to data based on word size.
	 */

        ptr2bytes = (short *) pd->data;
        ptr4bytes = (int *)   pd->data;
	scale  = sd[0]->p_desc[iparam]->scale_factor;
	offset = sd[0]->p_desc[iparam]->offset_factor;
	missing_data_flag = sd[0]->p_desc[iparam]->missing_data_flag;

	for (i=0; i<nbins; i++) {
	  if (word_size == 2) datum = *ptr2bytes++;
	  else datum = *ptr4bytes++;
	  /*
	    TODO: If there is a threshold parameter for this parameter then
	    apply threshold value.  I think threshold works like this: If there's a
	    threshold parameter, as with VT (threshold parm = NCP), then use
	    threshold value from that parameter unless it is the missing data value.
	  */
	  if (datum != missing_data_flag) {
	    value = ((float)datum - offset) / scale;
	  }
	  else value = BADVAL;

	  ray->range[i] = ray->h.invf(value);
	}

        if (iray == 0) {
          radar->v[iv]->h.nsweeps = nsweep + 1;
          radar->v[iv]->h.f = RSL_f_list[iv];
          radar->v[iv]->h.invf = RSL_invf_list[iv];
        }
      }
    }
    nsweep++;
    if (radar_verbose_flag) fprintf(stderr, "______NEW SWEEP__<%d>____\n", nsweep);
    /* Save for loading into volume structure. */
    dorade_free_sweep(sr);
  }

  /* The following avoids a broken pipe message, since a VOLD at the end
   * is not read yet.
   */
  while(fread(buf, sizeof(buf), 1, fp)) continue;  /* Read til EOF */

  rsl_pclose(fp);

  return radar;
}
