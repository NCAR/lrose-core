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

typedef struct {
  unsigned short
    yearjday,       /* Year * 512 + Julian day. */
	hour,           /* Hour. */
	minute_sec,     /* Minute * 60 + seconds. */
	bcd_start_azim, /* BCD code for start of azimuth. */
	bcd_end_azim,   /* BCD code for end of azimuth. */
	raycount,       /* Raycount + elstep * 512. Set to 0 for last ray of ppi */
	bcd_azimuth,    /* BCD code for azimuth. */
	bcd_elevation,  /* BCD code for elevation. */
	mds,            /* Minumum detectable signal (MDS). */
    mus,            /* Minumum usable signal (MUS). */
	rvpc_high,
    rvpc_low,       /* High and low level, in RVPC units. */
	phi,            /* PHI - RVPC high level in dbm * 32. */
	plo,            /* PLO - RVPC high level in dbm * 32. */
	xmit_power_site,     /* Transimitter Power * 32 + Site number. */
	skip_width_azim_avg, /* Skip + Bin width * 256 +
						  * Azimuth Averaging Factor * 4096.
						  */
	bin[224],            /* RVPC COUNT for range bin[1..224] */
	notused[256-241+1];  /* Not used. */
} Africa_buffer;

typedef Africa_buffer Africa_ray;

typedef struct {
  int nrays;
  Africa_ray **ray; /* 0..nrays-1 of Africa_rays */
} Africa_sweep;


typedef struct {
  int nsweeps;
  Africa_sweep **sweep; /*0..nsweeps-1 of Africa_sweeps */
} Africa_volume;

/* Prototype routine definitions */

int africa_read_buffer(FILE *fp, Africa_buffer *buffer);
float africa_bcd_convert(unsigned short bcd);
Africa_sweep * africa_new_sweep(int nray);
Africa_ray *africa_new_ray(void);
void africa_free_ray(Africa_ray *r);
void africa_free_sweep(Africa_sweep *s);
Africa_sweep *africa_read_sweep(FILE *fp);
