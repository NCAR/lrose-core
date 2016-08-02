#include <trmm_rsl/rsl.h>

#ifdef HAVE_LIBETOR

/*----------------------------------------------------------------------**
**
** EDGE_to_Radar.c
**
**----------------------------------------------------------------------**
**
** DESCRIPTION
**
** Converts an EDGE Volume structure to an RSL Radar structure
**
** USAGE:
**
** Radar *RSL_EDGE_to_radar(EDGE_filename);	-* Usage Section with comments *-
** char *EDGE_filename;			-* name of EDGE volume file *-
**
** PROCESSING:
**
**	This program creates an RSL radar data structure copies an Edge 
**	volume data structure into it and returns the pointer to the Radar 
**	structure.
**
** COPYRIGHT NOTICE
**
**	Copyright (c) 1997 by Enterprise Electronics Corporation
**	All Rights Reserved
** 
** This program is  copyright  by  Enterprise  Electronics  Corpora-
** tion,    Enterprise,  Alabama,  USA  36330 (334) 347-3478.  It is
** licensed for  use  on  a  specific  CPU   and   is  not    to  be
** transferred  or otherwise divulged.   Copies  or modifications of
** this program must carry this copyright notice.
** 
**
**
** HEADER INFOMATION
**
**	Software Suite 		- EDGE
**	Package			-
**	Reference number	- SP1/PGM/
**	Revision number		- $Revision: 1.4 $
**	Release State		- $State: Exp $
**	Author, designer	- Don Burrows
** Modification Date		- $Date: 2009/11/13 00:22:51 $
** Modified by			- $Author: oien $
** $Source: /cvs/libs/trmm_rsl/src/rsl/edge_to_radar.c,v $
**
** MODIFICATION RECORD
**
** $Log: edge_to_radar.c,v $
** Revision 1.4  2009/11/13 00:22:51  oien
** Niles : Checking in version 1.4 which deals with 2 byte data thanks to Bart Kelley kelley@radar.gsfc.nasa.gov
**
** Revision 1.3  1999/11/23 00:36:00  merritt
** auto configure scripts added
**
** Revision 1.2  1999/04/02 16:14:45  merritt
** ready for v1.23
**
** Revision 1.1  1999/03/31 22:35:16  merritt
** round 1 for edge incorporation.   Still seg faults for any_to_gif
**
**
**----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Feature Test Switches                                                */
/*----------------------------------------------------------------------*/
#define _POSIX_SOURCE	1

/*----------------------------------------------------------------------*/
/* System Headers            { full list in stdinc.h }                  */
/*----------------------------------------------------------------------*/
#include	<stdio.h>	/* stdio library			*/
#include	<stddef.h>	/* Some popular symbols			*/
#include	<stdlib.h>	/* Some standard funct.			*/
#include	<unistd.h>	/* POSIX symbols definitions		*/
#include        <time.h>
#include 	<math.h>
#include    <string.h>

/*----------------------------------------------------------------------*/
/* Application Headers                                                  */
/*----------------------------------------------------------------------*/
#include "vol.h"
#include "antenna.h"

/*----------------------------------------------------------------------*/
/* Macros                                                               */
/*----------------------------------------------------------------------*/
#define NEEDED_VOLS 8

/*----------------------------------------------------------------------*/
/* External (Import) Variables                                          */
/*----------------------------------------------------------------------*/
extern int radar_verbose_flag;

/*----------------------------------------------------------------------*/
/* External Functions                                                   */
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Structures and Unions                                       	        */
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Global (Export) Variables                                            */
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Local (Static) Variables                                             */
/*----------------------------------------------------------------------*/
static struct vol_struct *EDGE_vol=NULL;
static int num_sweeps,num_rays,num_bins,gate_width;
static float azimuth,elevation;
static float prf,wavelength,nyq_vel,meansr;
static struct tm *sweeptime;
static float lat,lon;
static int bytes_bin;
static float beam_width;
static float (*f)(Range x);
static Range (*invf)(float x);

/*----------------------------------------------------------------------*/
/* Signal Catching Functions                                            */
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Local Function                                                       */
/*----------------------------------------------------------------------*/

Ray *Fill_Ray_Header(int num_bins,int isweep,int iray)
{
	Ray *RSL_ray;

	RSL_ray = RSL_new_ray(num_bins);
	RSL_ray->h.sec = sweeptime->tm_sec;
	RSL_ray->h.minute = sweeptime->tm_min;
	RSL_ray->h.hour = sweeptime->tm_hour;
       	RSL_ray->h.day = sweeptime->tm_mday;
       	RSL_ray->h.month = sweeptime->tm_mon+1;
       	RSL_ray->h.year = sweeptime->tm_year + 1900;
	RSL_ray->h.unam_rng = 149851.274/prf;
	RSL_ray->h.azimuth = azimuth;
	RSL_ray->h.ray_num = iray;
	RSL_ray->h.elev = elevation;
	RSL_ray->h.elev_num = isweep;
	RSL_ray->h.range_bin1 = gate_width;
	RSL_ray->h.gate_size = gate_width;
	RSL_ray->h.vel_res = nyq_vel/128.0;
	RSL_ray->h.sweep_rate = meansr;
	RSL_ray->h.prf = prf;
	RSL_ray->h.azim_rate = 
		(float)EDGE_vol->sweep[isweep].rad.antenna_speed*0.55;
	RSL_ray->h.lat = lat;
	RSL_ray->h.lon = lon;
	RSL_ray->h.alt = EDGE_vol->sweep[isweep].rad.antenna_height;
	RSL_ray->h.rvc = 0;
	RSL_ray->h.pulse_count = EDGE_vol->sweep[isweep].rad.pulses;
	RSL_ray->h.pulse_count = EDGE_vol->sweep[isweep].rad.pulse_width*1.2+0.8;
	RSL_ray->h.beam_width = beam_width;
	RSL_ray->h.frequency = 299.702547/wavelength;
	RSL_ray->h.wavelength = wavelength;
	RSL_ray->h.nyq_vel = nyq_vel;
	RSL_ray->h.f = f;
	RSL_ray->h.invf = invf;
	RSL_ray->h.nbins = num_bins;
	return RSL_ray;
}

/*----------------------------------------------------------------------*/
/* Main Function                                                        */
/*----------------------------------------------------------------------*/

Radar *RSL_EDGE_to_radar(char *EDGE_filename)
{
	Radar *RSL_rad;
	int i,j,k;
	Sweep *sweep_u,*sweep_z,*sweep_v,*sweep_w,*sweep_d=NULL;
	unsigned char *EDGE_ray;
	unsigned short *sray;
	struct tm *voltime;
	float start_azimuth,end_azimuth;
	char state[2];

	float uz,cz,rv,sw,zdr=0;

	if (radar_verbose_flag) printf("EDGE_to_radar(%s)\n",EDGE_filename);

/** Load the EDGE volume structure  **/
	if (load_data((char **)&EDGE_vol,EDGE_filename,VOL_FILE) == -1)
	{
		EDGE_vol = NULL;
		fprintf(stderr,"EDGE_to_radar: Could not load EDGE Volume File: %s\n",EDGE_filename);
		return NULL;
	}
/** Allocating memory for radar structure **/
	RSL_rad = RSL_new_radar(MAX_RADAR_VOLUMES);
	if (RSL_rad == NULL)
	{
		fprintf(stderr, "EDGE_to_radar: radar is NULL\n");
		free(EDGE_vol);
		return NULL;
	}
	bytes_bin = BYTES_BIN(EDGE_vol);
	prf = (float)EDGE_vol->sweep[0].rad.prf1;
	wavelength = EDGE_vol->sweep[0].rad.wavelength;
	if (radar_verbose_flag) printf("bytes_bin: %d  prf %5.0f  wavelength %4.1f\n",
		bytes_bin,prf,wavelength*100.0);
	nyq_vel = prf*wavelength/4.0;
	num_sweeps = EDGE_vol->num_sweeps;
	if (radar_verbose_flag) printf("nyq_vel %5.1f  num_sweeps %d\n",nyq_vel,num_sweeps);

	meansr = 0.0;
	for(i=0;i<num_sweeps-1;i++)
		meansr += (float)(EDGE_vol->sweep[i+1].date-EDGE_vol->sweep[i].date);
	meansr /= (float)num_sweeps-1.0;
	meansr = 60.0/meansr;

	voltime = gmtime(&EDGE_vol->date);
	if (num_sweeps > MAX_SWEEPS) num_sweeps = MAX_SWEEPS;
	sprintf(state,"NA");

/*  Now fill the Radar header  */
        RSL_rad->h.sec = voltime->tm_sec;
        RSL_rad->h.minute = voltime->tm_min;
        RSL_rad->h.hour = voltime->tm_hour;
        RSL_rad->h.day = voltime->tm_mday;
        RSL_rad->h.month = voltime->tm_mon + 1;
        RSL_rad->h.year = voltime->tm_year + 1900;
	strcpy(RSL_rad->h.radar_type,"EDGE");
	RSL_rad->h.nvolumes = NEEDED_VOLS;
	RSL_rad->h.number = 553;/* What is this number supposed to be??? */
	memmove(RSL_rad->h.name,EDGE_vol->sweep[0].rad.site_name,
		sizeof(RSL_rad->h.name));
	memmove(RSL_rad->h.radar_name,EDGE_vol->sweep[0].rad.radar_type,
		sizeof(RSL_rad->h.radar_name));
	memmove(RSL_rad->h.city,EDGE_vol->sweep[0].rad.site_name,
		sizeof(RSL_rad->h.city));
	memmove(RSL_rad->h.state,state,sizeof(RSL_rad->h.state));
	RSL_rad->h.latd = EDGE_vol->sweep[0].rad.lat_deg;
	RSL_rad->h.latm = EDGE_vol->sweep[0].rad.lat_min;
	RSL_rad->h.lats = EDGE_vol->sweep[0].rad.lat_sec;
	RSL_rad->h.lond = EDGE_vol->sweep[0].rad.long_deg;
	RSL_rad->h.lonm = EDGE_vol->sweep[0].rad.long_min;
	RSL_rad->h.lons = EDGE_vol->sweep[0].rad.long_sec;
	if (RSL_rad->h.latd < 0) 
	{
		if(RSL_rad->h.latm > 0) RSL_rad->h.latm *= -1;
		if(RSL_rad->h.lats > 0) RSL_rad->h.lats *= -1;
	}
	if (RSL_rad->h.lond < 0) 
	{
		if(RSL_rad->h.lonm > 0) RSL_rad->h.lonm *= -1;
		if(RSL_rad->h.lons > 0) RSL_rad->h.lons *= -1;
	}
	lat = (float)RSL_rad->h.latd+(float)RSL_rad->h.latm/60.0+
		(float)RSL_rad->h.lats/3600.0;
	lon = (float)RSL_rad->h.lond+(float)RSL_rad->h.lonm/60.0+
		(float)RSL_rad->h.lons/3600.0;
	RSL_rad->h.height = EDGE_vol->sweep[0].rad.antenna_height;
	RSL_rad->h.spulse = EDGE_vol->sweep[0].rad.pulse_width*1200 + 800;
	RSL_rad->h.lpulse = EDGE_vol->sweep[0].rad.pulse_width*1200 + 800;
	if (radar_verbose_flag) printf("Radar Header Filled\n");
/* 
	Done with Radar header
	Now create the necessary volumes and fill the
	volume headers
*/

	RSL_rad->v[DZ_INDEX] = RSL_new_volume(num_sweeps);
	if (radar_verbose_flag) printf("DZ volume created index is %d\n",DZ_INDEX);
	if ((RSL_rad->v[DZ_INDEX]->h.type_str = malloc(25)) != NULL)
	{
		RSL_rad->v[DZ_INDEX]->h.type_str[24] = '\0';
		strcpy(RSL_rad->v[DZ_INDEX]->h.type_str,"Uncorrected Reflectivity"); 
	}
	if (radar_verbose_flag) printf("Uncorrected Reflectivity\n");
	RSL_rad->v[DZ_INDEX]->h.nsweeps = num_sweeps;
	if (radar_verbose_flag) printf("num_sweeps %d assigned\n",num_sweeps);
	RSL_rad->v[DZ_INDEX]->h.f = DZ_F;
	if (radar_verbose_flag) printf("DZ_F assigned\n");
	RSL_rad->v[DZ_INDEX]->h.invf = DZ_INVF;
	if (radar_verbose_flag) printf("DZ volume created and header Filled\n");

	RSL_rad->v[CZ_INDEX] = RSL_new_volume(num_sweeps);
	if ((RSL_rad->v[CZ_INDEX]->h.type_str = malloc(23)) != NULL)
	{
		RSL_rad->v[CZ_INDEX]->h.type_str[22] = '\0';
		strcpy(RSL_rad->v[CZ_INDEX]->h.type_str,"Corrected Reflectivity"); 
	}
	RSL_rad->v[CZ_INDEX]->h.nsweeps = num_sweeps;
	RSL_rad->v[CZ_INDEX]->h.f = CZ_F;
	RSL_rad->v[CZ_INDEX]->h.invf = CZ_INVF;
	if (radar_verbose_flag) printf("CZ volume created and header Filled\n");

	RSL_rad->v[VR_INDEX] = RSL_new_volume(num_sweeps);
	if ((RSL_rad->v[VR_INDEX]->h.type_str = malloc(16)) != NULL)
	{
		RSL_rad->v[VR_INDEX]->h.type_str[15] = '\0';
		strcpy(RSL_rad->v[VR_INDEX]->h.type_str,"Radial Velocity"); 
	}
	RSL_rad->v[VR_INDEX]->h.nsweeps = num_sweeps;
	RSL_rad->v[VR_INDEX]->h.f = VR_F;
	RSL_rad->v[VR_INDEX]->h.invf = VR_INVF;
	if (radar_verbose_flag) printf("VR volume created and header Filled\n");

	RSL_rad->v[SW_INDEX] = RSL_new_volume(num_sweeps);
	if ((RSL_rad->v[SW_INDEX]->h.type_str = malloc(15)) != NULL)
	{
		RSL_rad->v[SW_INDEX]->h.type_str[14] = '\0';
		strcpy(RSL_rad->v[SW_INDEX]->h.type_str,"Spectrum Width"); 
	}
	RSL_rad->v[SW_INDEX]->h.nsweeps = num_sweeps;
	RSL_rad->v[SW_INDEX]->h.f = SW_F;
	RSL_rad->v[SW_INDEX]->h.invf = SW_INVF;
	if (radar_verbose_flag) printf("SW volume created and header Filled\n");

	if (bytes_bin == 5) 
	{
		RSL_rad->v[ZD_INDEX] = RSL_new_volume(num_sweeps);
		strcpy(RSL_rad->v[ZD_INDEX]->h.type_str,"Differential Reflectivity"); 
		if ((RSL_rad->v[ZD_INDEX]->h.type_str = malloc(26)) != NULL)
		{
			RSL_rad->v[ZD_INDEX]->h.type_str[25] = '\0';
			strcpy(RSL_rad->v[ZD_INDEX]->h.type_str,"Differential Reflectivity"); 
		}
		RSL_rad->v[ZD_INDEX]->h.nsweeps = num_sweeps;
		RSL_rad->v[ZD_INDEX]->h.f = ZD_F;
		RSL_rad->v[ZD_INDEX]->h.invf = ZD_INVF;
		if (radar_verbose_flag) printf("ZD volume created and header Filled\n");
	}
/*
	Volume Headers complete now fill the sweeps
*/

	for (i=0;i<num_sweeps;i++)
	{
		if (radar_verbose_flag) printf("Sweep number %d\n",i);
		num_rays = EDGE_vol->sweep[i].num_rays;
		sray = (unsigned short *)RAY_PTR(EDGE_vol,i,10);
                elevation = ((float)(BINEL2IANG100(sray[1])) +
                        (float)(BINEL2IANG100(sray[3])))/200.0;
		sweeptime = gmtime(&EDGE_vol->sweep[i].date);
/* 
	In newer versions of edge the beam width will be EDGE_vol->sweep[i].rad.beam_width 
*/
		beam_width = 1.0;

		RSL_rad->v[DZ_INDEX]->sweep[i] = RSL_new_sweep(num_rays);
		sweep_u = RSL_rad->v[DZ_INDEX]->sweep[i];
		sweep_u->h.sweep_num = i;
		sweep_u->h.elev = elevation;
		sweep_u->h.beam_width = beam_width;
		sweep_u->h.vert_half_bw = beam_width/2.0;
		sweep_u->h.horz_half_bw = beam_width/2.0;
		sweep_u->h.nrays = num_rays;
		sweep_u->h.f = DZ_F;
		sweep_u->h.invf = DZ_INVF;

		RSL_rad->v[CZ_INDEX]->sweep[i] = RSL_new_sweep(num_rays);
		sweep_z = RSL_rad->v[CZ_INDEX]->sweep[i];
		sweep_z->h.sweep_num = i;
		sweep_z->h.elev = elevation;
		sweep_z->h.beam_width = beam_width;
		sweep_z->h.vert_half_bw = beam_width/2.0;
		sweep_z->h.horz_half_bw = beam_width/2.0;
		sweep_z->h.nrays = num_rays;
		sweep_z->h.f = CZ_F;
		sweep_z->h.invf = CZ_INVF;

		RSL_rad->v[VR_INDEX]->sweep[i] = RSL_new_sweep(num_rays);
		sweep_v = RSL_rad->v[VR_INDEX]->sweep[i]; 
		sweep_v->h.sweep_num = i;
		sweep_v->h.elev = elevation;
		sweep_v->h.beam_width = beam_width;
		sweep_v->h.vert_half_bw = beam_width/2.0;
		sweep_v->h.horz_half_bw = beam_width/2.0;
		sweep_v->h.nrays = num_rays;
		sweep_v->h.f = VR_F;
		sweep_v->h.invf = VR_INVF;

		RSL_rad->v[SW_INDEX]->sweep[i] = RSL_new_sweep(num_rays);
		sweep_w = RSL_rad->v[SW_INDEX]->sweep[i];
		sweep_w->h.sweep_num = i;
		sweep_w->h.elev = elevation;
		sweep_w->h.beam_width = beam_width;
		sweep_w->h.vert_half_bw = beam_width/2.0;
		sweep_w->h.horz_half_bw = beam_width/2.0;
		sweep_w->h.nrays = num_rays;
		sweep_w->h.f = SW_F;
		sweep_w->h.invf = SW_INVF;

		if (bytes_bin == 5) 
		{
			RSL_rad->v[ZD_INDEX]->sweep[i] = RSL_new_sweep(num_rays);
			sweep_d = RSL_rad->v[ZD_INDEX]->sweep[i];
			sweep_d->h.sweep_num = i;
			sweep_d->h.elev = elevation;
			sweep_d->h.beam_width = beam_width;
			sweep_d->h.vert_half_bw = beam_width/2.0;
			sweep_d->h.horz_half_bw = beam_width/2.0;
			sweep_d->h.nrays = num_rays;
			sweep_d->h.f = ZD_F;
			sweep_d->h.invf = ZD_INVF;
		}
/*
	Sweeps are complete now to do the rays
*/
		num_bins = (int)EDGE_vol->sweep[i].rad.gates;
		for (j=0;j<num_rays;j++)
		{
			sray = (unsigned short *)RAY_PTR(EDGE_vol,i,j);
			elevation = ((float)(BINEL2IANG100(sray[1])) +
                        (float)(BINEL2IANG100(sray[3])))/200.0;
			gate_width = EDGE_vol->sweep[i].rad.gw1;
			start_azimuth =BIN2IANG(sray[0]);
			end_azimuth =BIN2IANG(sray[2]);
			azimuth = (start_azimuth + end_azimuth)/2.0;
			if (fabs(end_azimuth - start_azimuth) > 180.0) azimuth = azimuth - 180.0;
			if (azimuth < 0.0) azimuth = azimuth + 360.0;

			f = DZ_F;
			invf = DZ_INVF;
			sweep_u->ray[j] = Fill_Ray_Header(num_bins,i,j);
			f = CZ_F;
			invf = CZ_INVF;
			sweep_z->ray[j] = Fill_Ray_Header(num_bins,i,j);
			f = VR_F;
			invf = VR_INVF;
			sweep_v->ray[j] = Fill_Ray_Header(num_bins,i,j);
			f = SW_F;
			invf = SW_INVF;
			sweep_w->ray[j] = Fill_Ray_Header(num_bins,i,j);
			if (bytes_bin == 5)
			{
				f = ZD_F;
				invf = ZD_INVF;
				sweep_d->ray[j] = Fill_Ray_Header(num_bins,i,j);
			}
			EDGE_ray = RAY_PTR(EDGE_vol,i,j);

/*
	Now fill the rest of the ray 
*/
			for (k=0;k<num_bins;k++)
			{	
				uz = (float)EDGE_ray[k*bytes_bin+2];
				if (uz == 0.0) uz = NOECHO;
				else if (uz > 255.0) uz = BADVAL;
				else uz = uz/2.0-32.0;
				cz = (float)EDGE_ray[k*bytes_bin];
				if (cz == 0.0) cz = NOECHO;
				else if (cz > 255.0) cz = BADVAL;
				else cz = cz/2.0-32.0;
				rv = (float)EDGE_ray[k*bytes_bin+1];
				if (rv == 0.0) rv = NOECHO;
				else if (rv > 255.0) rv = BADVAL;
				else rv = (rv-128.0)/128.0*nyq_vel;
				sw = (float)EDGE_ray[k*bytes_bin+3];
				if (sw == 0.0) sw = NOECHO;
				else if (sw > 255.0) sw = BADVAL;
				else sw = sw/128.0*nyq_vel;
				if (bytes_bin == 5) 
				{
					zdr = (float)EDGE_ray[k*bytes_bin+4];
					if (zdr == 0.0) zdr = NOECHO;
					else if (zdr > 255.0) zdr = BADVAL;
					else zdr = (zdr-128.0)/16.0;
				}
				sweep_u->ray[j]->range[k] = DZ_INVF(uz);
				sweep_z->ray[j]->range[k] = CZ_INVF(cz);
				sweep_v->ray[j]->range[k] = VR_INVF(rv);
				sweep_w->ray[j]->range[k] = SW_INVF(sw);
				if (bytes_bin == 5) sweep_d->ray[j]->range[k] = DR_INVF(zdr);
			}
		}
	}
	if (radar_verbose_flag)
	  printf("EDGE to RSL conversion complete\n");	
	free(EDGE_vol);
	RSL_rad = RSL_prune_radar(RSL_rad);
	return RSL_rad;
}
/*-END OF MODULE--------------------------------------------------------*/
#else
Radar *RSL_EDGE_to_radar(char *infile)
{
  fprintf(stderr,
"The library libetor.a (or .so) was not found when RSL was installed,\n\
therefore, EDGE capability was disabled.  If you now have libetor, then\n\
you must reinstall RSL to enable EDGE.\n"
);
  return NULL;
}
#endif
