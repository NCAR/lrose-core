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


19 Jan 1998
   Michael Whimpey from BMRC, Australia changed code, for the
   different callibrations, between, Pre_mctex, mctex, Gunn_Pt
   periods.
22 Apr 1999
   Michael Whimpey added more callibration periods over berrimah and scsmex.
   please see code where there is m.whimpey 

*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define USE_RSL_VARS
#include <trmm_rsl/rsl.h>

#ifdef HAVE_LASSEN
#include "lassen.h"
extern int radar_verbose_flag;

/* Some parameter headaches are prevented when vol is declared global. */
Lassen_volume vol;
extern int read_entire_lassen_file(FILE *f, Lassen_volume *vol);

/**********************************************************************/
/*                                                                    */
/*                     lassen_load_sweep                              */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      May  26, 1994                                                 */
/**********************************************************************/
void lassen_load_sweep(Sweep *s, int isweep_num, int ifield, int period, Lassen_sweep *ptr)
{
  float c = RSL_SPEED_OF_LIGHT;
  float elev;
  Lassen_ray *aray;
  unsigned char *ray_data;
  int i,j,m,kk=0;
  float x[2000];
  double Vu;
  Range (*invf)(float x);
  float (*f)(Range x);

/*  calibration period flags   m.whimpey  */
#define BERRIMAH1 0
#define MCTEX_EARLY 1
#define MCTEX 2
#define BERRIMAH2 3
#define GUNN_PT 4
#define GUNN_PT1 5
#define SCSMEX 6
#define SCSMEX1 7

  if (s == NULL) return;
  f    = (float (*)(Range x))NULL;  /* This quiets the pedantic warning. */
  invf = (Range (*)(float x))NULL;
  if (ifield == ZT_INDEX) {kk = OFF_UZ;  invf = ZT_INVF; f = ZT_F;}
  if (ifield == DZ_INDEX) {kk = OFF_CZ;  invf = DZ_INVF; f = DZ_F;}
  if (ifield == VR_INDEX) {kk = OFF_VEL; invf = VR_INVF; f = VR_F;}
  if (ifield == SW_INDEX) {kk = OFF_WID; invf = SW_INVF; f = SW_F;}
  if (ifield == ZD_INDEX) {kk = OFF_ZDR; invf = ZD_INVF; f = ZD_F;}
  if (ifield == PH_INDEX) {kk = OFF_PHI; invf = PH_INVF; f = PH_F;}
  if (ifield == RH_INDEX) {kk = OFF_RHO; invf = RH_INVF; f = RH_F;}
  if (ifield == LR_INDEX) {kk = OFF_LDR; invf = LR_INVF; f = LR_F;}
  if (ifield == KD_INDEX) {kk = OFF_KDP; invf = KD_INVF; f = KD_F;}
  if (ifield == TI_INDEX) {kk = OFF_TIME; invf = TI_INVF; f = TI_F;}
  
  elev = (float)ptr->fangle*360.0/16384.0;
  Vu = c*((float)vol.prf/10.)/(4.*(float)vol.freq*100000.0);
  
  s->h.sweep_num = ptr->sweep;
  s->h.elev = elev;
  s->h.nrays = ptr->numrays;
  s->h.beam_width = 1.0;  /* What is it really? */
  s->h.horz_half_bw = .5;
  s->h.vert_half_bw = .5;
  s->h.f = f;
  s->h.invf = invf;

  for(i=0;i<(int)ptr->numrays;i++) {
	aray = ptr->ray[i];
	if (aray == NULL) continue;
	s->ray[i] = RSL_new_ray((int)aray->numgates);
	s->ray[i]->h.month    = aray->month;
	s->ray[i]->h.day      = aray->day;
	s->ray[i]->h.year     = (int)aray->year + 1900;
	if (s->ray[i]->h.year < 1980) s->ray[i]->h.year += 100; /* Year > 2000. */
	s->ray[i]->h.hour     = aray->hour;
	s->ray[i]->h.minute   = aray->minute;
	s->ray[i]->h.sec      = aray->second;
	s->ray[i]->h.azimuth  =  (float)aray->vangle*360.0/16384.0;

	s->ray[i]->h.ray_num    = i;
	s->ray[i]->h.elev       = elev;
	s->ray[i]->h.elev_num   = s->h.sweep_num;
	s->ray[i]->h.range_bin1 = aray->rangeg1;
	s->ray[i]->h.gate_size  = aray->gatewid;
	s->ray[i]->h.vel_res  = 0.5; /* What is this really? */

	s->ray[i]->h.fix_angle = s->h.elev;
	s->ray[i]->h.frequency = 	(float)vol.freq*1.0e-4;  /* GHz */
	s->ray[i]->h.wavelength = c / s->ray[i]->h.frequency * 1.0e-9;
	s->ray[i]->h.prf      = (int)aray->prf/10;
	s->ray[i]->h.nyq_vel = s->ray[i]->h.prf * s->ray[i]->h.wavelength / 4.0;
	if (s->ray[i]->h.prf != 0)
	  s->ray[i]->h.unam_rng = c / (2.0 * s->ray[i]->h.prf * 1000.0); /* km */
	else
	  s->ray[i]->h.unam_rng = 0.0;
	s->ray[i]->h.pulse_width = (float)(aray->p_width * 0.05);
	s->ray[i]->h.pulse_count = aray->n_pulses;

	s->ray[i]->h.beam_width = 1.0; /* What is it really? */
	s->ray[i]->h.f = f;
	s->ray[i]->h.invf = invf;

	ray_data = (unsigned char *)aray;

	m = aray->offset[kk];
	if(m==0) continue;

/*  conversion changes by m.whimpey   */
	for(j=0; j<s->ray[i]->h.nbins; j++) {
	  switch (kk) {
	  case OFF_UZ: /* UZ field. */
	  case OFF_CZ: /* CZ field. */
		/* Apply a 1.4 dB correction. Ken Glasson did this. */
	        /* Removed 1.4 dB correction 09/27/2006--BMRC doesn't use it. */
		if (period == BERRIMAH1)
                    x[j] = ((float)ray_data[m+j] - 56.0)/2.0; /* Removed +1.4 */
		else 
                    x[j] = ((float)ray_data[m+j] - 64.0)/2.0; /* Removed +1.4 */
		break;
	  case OFF_VEL: /* VR field */
		if (period == BERRIMAH1)
                   x[j] =  (float)(Vu*((double)ray_data[m+j]-128.0)/128.);
		else if (period == SCSMEX)
                  x[j] =  (float)(Vu*((double)(ray_data[m+j]^0x80)-128.0)/127.);
		else
                   x[j] =  (float)(Vu*((double)ray_data[m+j]-128.0)/127.);
/*		fprintf(stderr,"Velocity for ray[%d] at x[%d] = %f, nyquist = %f\n",i,j,x[j],s->ray[i]->h.nyq_vel); */
		break;
	  case OFF_WID: /* SW field */
		if (period == BERRIMAH1)
                    x[j] = (float)(Vu*(double)ray_data[m+j]/100.);
		else 
                    x[j] = (float)(Vu*(double)ray_data[m+j]/256.);
		break;
	  case OFF_ZDR: /* ZD field */
		if (period < MCTEX_EARLY) break;
		if (period <= BERRIMAH2)
		    x[j] = ((float)ray_data[m+j] - 64.0)/21.25;
		else
		    x[j] = ((float)ray_data[m+j] - 128.)*18./254.;
		break;
	  case OFF_PHI: /* PH field */
		if (period < MCTEX_EARLY) break;
		if (period <= BERRIMAH2)
		    x[j] = ((float)ray_data[m+j] - 128.0)*32.0/127.0;
		else if (period == GUNN_PT)
		    x[j] = ((float)ray_data[m+j] - 64.5)*360.0/254.0;
		else if (period > GUNN_PT)
		    x[j] = 90.0+((float)ray_data[m+j] - 128.0)*180.0/254.0;
		    /*Extra 90 degrees added by Scott Collis, CAWCR/BMRC 2008*/
		break;
	  case OFF_RHO: /* RH field */
		if (period < MCTEX_EARLY) break;
		if (period <= BERRIMAH2)
		    x[j] = sqrt((float)ray_data[m+j]/256.822 + 0.3108);
		else
		    x[j] = (((float)ray_data[m+j]-1.)*1.14/254.) + 0.01;
		break;
	  case OFF_LDR: /* LR field -- no 'period' here. */
		x[j] = ((float)ray_data[m+j] - 250.0)/6;
		break;
	  case OFF_KDP: /* KP field -- no 'period' here. */
		x[j] = ((float)ray_data[m+j]); /* This conversion is taken
										  from SIGMET.  Is this right? */
		break;
	  case OFF_TIME: /* TIME field -- no 'period' here. */
		x[j] = ((float)ray_data[m+j]); /* This conversion is taken
										  from SIGMET.  Is this right? */
		break;
	  }
	  if (ray_data[m+j] == 0) x[j] = BADVAL;
	}
	for (j=0; j<s->ray[i]->h.nbins; j++) 
	  s->ray[i]->range[j] = invf(x[j]);
  }
}



/**********************************************************************/
/*                                                                    */
/*                     RSL_lassen_to_radar                            */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      May  26, 1994                                                 */
/**********************************************************************/
Radar *RSL_lassen_to_radar(char *infile)
{
/* Lassen specific. */
  Lassen_sweep *ptr;
  Lassen_ray *aray;
  int period;  /*   m.whimpey changed early variable to period  */
  FILE *f;
  int q[MAX_RADAR_VOLUMES];
  extern int rsl_qfield[];
  extern int *rsl_qsweep; /* See RSL_read_these_sweeps in volume.c */
  extern int rsl_qsweep_max;

/* Radar specific */
  Radar *radar;
  int i, j, k;
  /* Listing of the RSL field type indexes, in the order that
   * LASSEN stores them.
   */
  int rsl_index[] = {ZT_INDEX, DZ_INDEX, VR_INDEX, SW_INDEX, ZD_INDEX,
					 PH_INDEX, RH_INDEX, LR_INDEX, KD_INDEX, TI_INDEX};
  
  char *ltype[] = {"UZ", "CZ", "Vel", "Wid", "Zdr", "Phi",
				   "Rho", "Ldr", "Kdp", "Time"};


  struct compare_date {   /*   introduced by m.whimpey   */
	int year;
	int month; 
	int day;
	int hour;
	int minute;
	int second; 
  };
/*  M.Whimpey changes made 19990422 for extra periods */
#define NUM_DATES 8
  struct compare_date  cvrt_date[NUM_DATES] =
  { {1992,  1,  1,  0,  0,  0},
	{1995, 11,  1,  0,  0,  0},
	{1995, 11, 25, 20,  5,  0},
	{1996,  1,  1,  0,  0,  0},
	{1997, 10,  1,  0,  0,  0},
	{1997, 11, 20,  3, 40,  0},
	{1998, 05, 04,  0,  0,  0},
	{1998, 05, 17, 03, 47,  0} };

  int d;	/* date counter */

  unsigned long vt;	/* vol time */
  unsigned long dt;	/* date time */
	

    /*   open Lassen file  */
  if (infile == NULL) {
	int save_fd;
	save_fd = dup(0);
	f = fdopen(save_fd, "r");
  }  else
    if((f=fopen(infile, "r"))==(FILE *)NULL) {
	  perror(infile);
	  return NULL;
    }
  f = uncompress_pipe(f); /* Transparently, use gunzip. */
#define NEW_BUFSIZ 16384
  setvbuf(f,NULL,_IOFBF,(size_t)NEW_BUFSIZ); /* Faster i/o? */
  
    if((read_entire_lassen_file(f, &vol)) == 0)
    {
        perror("RSL_lassen_to_radar ... read_entire_lassen_file");
        exit(1);
    }

	rsl_pclose(f);

	if (radar_verbose_flag) {
	  fprintf(stderr,"\n Version   = %d",vol.version);
	  fprintf(stderr,"\n Volume    = %d",vol.volume);
	  fprintf(stderr,"\n Numsweeps = %d",vol.numsweeps);
	  fprintf(stderr,"\n Time      = %2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d - %2.2d:%2.2d:%2.2d",
		 (int)vol.month, (int)vol.day, (int)vol.year,
		 (int)vol.shour, (int)vol.sminute, (int)vol.ssecond,
		 (int)vol.ehour, (int)vol.eminute, (int)vol.esecond);
	  fprintf(stderr,"\n Angle: start %d, stop %d", (int)vol.a_start, (int)vol.a_stop);
	  fprintf(stderr,"\n");
	}
  
/*  determine which period the lassen volume belongs   m.whimpey   */
	vt = (vol.year-90) * 32140800;
	vt += vol.month * 2678400;
	vt += vol.day * 86400;
	vt += vol.shour * 3600;
	vt += vol.sminute * 60;
	vt += vol.ssecond;

	for(d=0; d<NUM_DATES; d++) {
	    dt = (cvrt_date[d].year-1990) * 32140800;
	    dt += cvrt_date[d].month * 2678400;
	    dt += cvrt_date[d].day * 86400;
	    dt += cvrt_date[d].hour * 3600;
	    dt += cvrt_date[d].minute * 60;
	    dt += cvrt_date[d].second;

	    if(vt<=dt) {
		break;
	    }
	}

	if(d==0) {
	    fprintf(stderr, "%s: Error Vol date before first known!\n",
		     infile);
	    exit(3);
	}
	period = d-1;

/* Max. expected volumes. */
  radar = RSL_new_radar(MAX_RADAR_VOLUMES);

  radar->h.month      = vol.month;
  radar->h.day        = vol.day;
  radar->h.year       = vol.year + 1900;
  radar->h.hour       = vol.shour;
  radar->h.minute     = vol.sminute;
  radar->h.sec        = vol.ssecond;
  strcpy(radar->h.radar_type, "lassen");
  radar->h.nvolumes   = MAX_RADAR_VOLUMES;
  memcpy(&radar->h.radar_name, vol.radinfo.radar_name, 8);
  memcpy(&radar->h.name, vol.radinfo.site_name, 8);
  memcpy(&radar->h.city, "????", 4);
  memcpy(&radar->h.state,"AU", 2);
  radar->h.latd       = vol.radinfo.latitude.degree;
  radar->h.latm       = vol.radinfo.latitude.minute;
  radar->h.lats       = vol.radinfo.latitude.second;
  /* Is there a problem with the minutes/seconds when negative?
   * The degree/minute/sec all should have the same sign.
   */
  if (radar->h.latd < 0) {
	if (radar->h.latm > 0) radar->h.latm *= -1;
	if (radar->h.lats > 0) radar->h.lats *= -1;
  }
  radar->h.lond       = vol.radinfo.longitude.degree;
  radar->h.lonm       = vol.radinfo.longitude.minute;
  radar->h.lons       = vol.radinfo.longitude.second;
  if (radar->h.lond < 0) {
	if (radar->h.lonm > 0) radar->h.lonm *= -1;
	if (radar->h.lons > 0) radar->h.lons *= -1;
  }
  radar->h.height     = vol.radinfo.antenna_height;
  radar->h.spulse     = 0;
  radar->h.lpulse     = 0;

  
  /*   iterate for each sweep in radar volume   */

/* Determine which field types exist. The array 'q' is a boolean to
 * force one print messages, if requested. 
 *
 * Well, I tried looping through all the rays and examining aray->flags.<?>,
 * but, it turns out that these flags bounce around, ie. toggle, for
 * fields that don't really exist.  Therefore, I cannot just logically OR
 * the field flags together to determine which fields exist -- is this
 * a bug when the file was created?
 *
 * What I've seen in lass2uf is to examine the first ray of the first sweep,
 * but, I think that is unreliable too, due to the above finding.
 *
 * The solution, now, is to examine the existance of OFFSET values.
 * This seems to be consistant, throughout the volume.
 * The OFFSET is really what is used, anyway, to extract the data.
 */
  memset(q, 0, sizeof(q));
  for (i=0; i<vol.numsweeps; i++) {
	ptr = vol.index[i];
	for (j=0; j<ptr->numrays; j++) {
	  aray = ptr->ray[j];
	  for (k=0; k<NUMOFFSETS; k++) {
		if (aray->offset[k] != 0 && !q[rsl_index[k]]) {
		  /* From RSL_select_fields */
		  if (rsl_qfield[rsl_index[k]] == 1) 
			q[rsl_index[k]]=1;
		}
	  }
	}
  }
  if (radar_verbose_flag) 
	fprintf(stderr,"\n Fields are (Lassen nomenclature):");
  for (k=0; k<NUMOFFSETS; k++) {
	i = rsl_index[k]; /* Lassen index order to RSL index order translation. */
	if (q[i])  {
	  if (radar_verbose_flag) fprintf(stderr," %s", ltype[k]);
	}
  }
  if (radar_verbose_flag)   fprintf(stderr,"\n");
  
  if (radar_verbose_flag) fprintf(stderr," Fields are    (RSL nomenclature):");
  for (k=0; k<NUMOFFSETS; k++) {

	/* BTW, it doesn't matter if we allocate volumes, sweeps, or rays that
	 * have no data.  The radar library will check for NULL pointers. 
	 */
	i = rsl_index[k]; /* Lassen index order to RSL index order translation. */
	if (q[i])  {
	  radar->v[i] = RSL_new_volume(vol.numsweeps);
	  radar->v[i]->h.f    = RSL_f_list[i];
	  radar->v[i]->h.invf = RSL_invf_list[i];
	  if (radar_verbose_flag)  fprintf(stderr," %s", RSL_ftype[i]);
	  if (k >= 2 && radar_verbose_flag) fprintf(stderr," "); /* Alignment. */
	}
  }
  if (radar_verbose_flag)   fprintf(stderr,"\n");

  for(j=0;j<radar->h.nvolumes; j++) {
	for(i=0;i<(int)vol.numsweeps;i++) {
	  if (rsl_qsweep != NULL) {
		if (i > rsl_qsweep_max) break;
		if (rsl_qsweep[i] == 0) continue;
	  }
	  ptr = vol.index[i];
	  if (radar->v[j]) {
		radar->v[j]->sweep[i] = RSL_new_sweep(ptr->numrays);

		/* 'period' is a flag for different calibrations */
		lassen_load_sweep(radar->v[j]->sweep[i], i, j, period, ptr);
	  }	  
	}
  }
  radar = RSL_prune_radar(radar);
  return radar;
}
#else
Radar *RSL_lassen_to_radar(char *infile)
{
  fprintf(stderr, "LASSEN is not installed in this version of RSL.\n");
  fprintf(stderr, "Reinstall RSL w/ -DHAVE_LASSEN in the Makefile.\n");
  return NULL;
}
#endif
