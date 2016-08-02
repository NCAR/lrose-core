/*
    NASA/TRMM, Code 613
    This is the TRMM Office Radar Software Library.
    Copyright (C) 2008
            Bart Kelley
	    SSAI
	    Lanham, Maryland

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


/* 
 * This file contains routines for processing Message Type 31, the digital
 * radar message type introduced in WSR-88D Level II Build 10.
 */

#include <trmm_rsl/rsl.h> 
#include <string.h>
#include "wsr88d.h"

/* Data descriptions in the following data structures are from the "Interface
 * Control Document for the RDA/RPG", Build 10.0 Draft, WSR-88D Radar
 * Operations Center.
 */

typedef struct {
    short rpg[6]; /* Unused.  Not really part of message header, but is
		   * inserted by RPG Communications Manager for each message.
		   */
    unsigned short msg_size;  /* for this segment, in halfwords */
    unsigned char  channel;   /* RDA Redundant Channel */
    unsigned char  msg_type;  /* Message Type */
    unsigned short id_seq;    /* I.d. Seq = 0 to 7FFF, then roll over to 0 */
    unsigned short msg_date;  /* modified Julian date from 1/1/70 */
    unsigned int   msg_time;  /* packet generation time in ms past midnite */
    unsigned short num_segs;
    unsigned short seg_num;
} Wsr88d_msg_hdr;

typedef struct {
    char radar_id[4];
    unsigned int   ray_time; /* Data collection time in milliseconds past midnight GMT */
    unsigned short ray_date; /* Julian date - 2440586.5 (1/01/1970) */
    unsigned short azm_num ; /* Radial number within elevation scan */
    float azm;      /* Azimuth angle in degrees (0 to 359.956055) */
    unsigned char compression_code; /* 0 = uncompressed, 1 = BZIP2, 2 = zlib */
    unsigned char spare; /* for word alignment */
    unsigned short radial_len; /* radial length in bytes, including data header block */
    unsigned char azm_res;
    unsigned char radial_status;
    unsigned char elev_num;
    unsigned char sector_cut_num;
    float elev;  /* Elevation angle in degrees (-7.0 to 70.0) */
    unsigned char radial_spot_blanking;
    unsigned char azm_indexing_mode;
    unsigned short data_block_count;
    /* Data Block Pointers */
    unsigned int dbptr_vol_const;
    unsigned int dbptr_elev_const;
    unsigned int dbptr_radial_const;
    unsigned int dbptr_ref;
    unsigned int dbptr_vel;
    unsigned int dbptr_sw;
    unsigned int dbptr_zdr;
    unsigned int dbptr_phi;
    unsigned int dbptr_rho;
} Ray_header_m31;  /* Called Data Header Block in RDA/RPG document. */

typedef struct {
    char dataname[4];
    unsigned int  reserved;
    unsigned short ngates;
    short range_first_gate;
    short range_samp_interval;
    short thresh_not_overlayed;
    short snr_thresh;
    unsigned char controlflag;
    unsigned char datasize_bits;
    float scale;
    float offset;
} Data_moment_hdr;

#define MAX_RADIAL_LENGTH 14288

typedef struct {
    Ray_header_m31 ray_hdr;
    float unamb_rng;
    float nyq_vel;
    unsigned char data[MAX_RADIAL_LENGTH];
} Wsr88d_ray_m31;

#define MAXRAYS_M31 800
#define MAXSWEEPS 20

enum {START_OF_ELEV, INTERMED_RADIAL, END_OF_ELEV, BEGIN_VOS, END_VOS};


void wsr88d_swap_m31_hdr(Wsr88d_msg_hdr *msghdr)
{
    swap_2_bytes(&msghdr->msg_size);
    swap_2_bytes(&msghdr->id_seq);
    swap_2_bytes(&msghdr->msg_date);
    swap_4_bytes(&msghdr->msg_time);
    swap_2_bytes(&msghdr->num_segs);
    swap_2_bytes(&msghdr->seg_num);
}


void wsr88d_swap_m31_ray(Ray_header_m31 *wsr88d_ray)
{
    int *fullword;

    swap_4_bytes(&wsr88d_ray->ray_time);
    swap_2_bytes(&wsr88d_ray->ray_date);
    swap_2_bytes(&wsr88d_ray->azm_num);
    swap_4_bytes(&wsr88d_ray->azm);
    swap_2_bytes(&wsr88d_ray->radial_len);
    swap_4_bytes(&wsr88d_ray->elev);
    swap_2_bytes(&wsr88d_ray->data_block_count);
    fullword = (int *) &wsr88d_ray->dbptr_vol_const;
    for (; fullword <= (int *) &wsr88d_ray->dbptr_rho; fullword++)
	swap_4_bytes(fullword);
}


void wsr88d_swap_data_hdr(Data_moment_hdr *this_field)
{
    short *halfword;
    halfword = (short *) &this_field->ngates;
    for (; halfword < (short *) &this_field->controlflag; halfword++)
	swap_2_bytes(halfword);
    swap_4_bytes(&this_field->scale);
    swap_4_bytes(&this_field->offset);
}


void testprt(Ray_header_m31 ray_hdr)
{
    /* For testing: Print some values from data block header. */
    printf("\nray_time: %d\n",ray_hdr.ray_time);
    printf("ray_date: %d\n",ray_hdr.ray_date);
    printf("azm_num: %d\n",ray_hdr.azm_num);
    printf("azm: %f\n",ray_hdr.azm);
    printf("radial_len: %d\n",ray_hdr.radial_len);
    printf("elev: %f\n",ray_hdr.elev);
    printf("data block count: %d\n",ray_hdr.data_block_count);
    printf("dbptr_vol_const: %d\n",ray_hdr.dbptr_vol_const);
    printf("dbptr_elev_const: %d\n",ray_hdr.dbptr_elev_const);
    printf("dbptr_radial_const: %d\n",ray_hdr.dbptr_radial_const);
    printf("dbptr_ref: %d\n",ray_hdr.dbptr_ref);
    printf("dbptr_vel: %d\n",ray_hdr.dbptr_vel);
    printf("dbptr_sw: %d\n",ray_hdr.dbptr_sw);
}


float wsr88d_get_angle(short bitfield)
{
    short mask = 1;
    int i;
    float angle = 0.;
    float value[13] = {0.043945, 0.08789, 0.17578, 0.35156, .70313, 1.40625,
	2.8125, 5.625, 11.25, 22.5, 45., 90., 180.};

    /* find which bits are set and sum corresponding values to get angle. */

    bitfield = bitfield >> 3;  /* 3 least significant bits aren't used. */
    for (i = 0; i < 13; i++) {
	if (bitfield & mask) angle += value[i];
	bitfield = bitfield >> 1;
    }
    return angle;
}


float wsr88d_get_azim_rate(short bitfield)
{
    short mask = 1;
    int i;
    float rate = 0.;
    float value[12] = {0.0109863, 0.021972656, 0.043945, 0.08789, 0.17578,
	0.35156, .70313, 1.40625, 2.8125, 5.625, 11.25, 22.5};

    /* Find which bits are set and sum corresponding values to get rate. */

    bitfield = bitfield >> 3;  /* 3 least significant bits aren't used. */
    for (i = 0; i < 12; i++) {
	if (bitfield & mask) rate += value[i];
	bitfield = bitfield >> 1;
    }
    if (bitfield >> 15) rate = -rate;
    return rate;
}

#define WSR88D_MAX_SWEEPS 20

typedef struct {
    int vcp;
    int num_cuts;
    float vel_res;
    float fixed_angle[WSR88D_MAX_SWEEPS];
    float azim_rate[WSR88D_MAX_SWEEPS];
    int waveform[WSR88D_MAX_SWEEPS];
    int super_res_ctrl[WSR88D_MAX_SWEEPS];
    int surveil_prf_num[WSR88D_MAX_SWEEPS];
    int doppler_prf_num[WSR88D_MAX_SWEEPS];
} VCP_data;

static VCP_data vcp_data;

void wsr88d_get_vcp_data(short *msgtype5)
{
    short azim_rate, fixed_angle, vel_res;
    short sres_and_survprf; /* super res ctrl and surveil prf, one byte each */
    short chconf_and_waveform;
    int i;
    
    vcp_data.vcp = (unsigned short) msgtype5[2];
    vcp_data.num_cuts = msgtype5[3];
    if (little_endian()) {
	swap_2_bytes(&vcp_data.vcp);
	swap_2_bytes(&vcp_data.num_cuts);
    }
    vel_res = msgtype5[5];
    if (little_endian()) swap_2_bytes(&vel_res);
    vel_res = vel_res >> 8;
    if (vel_res == 2) vcp_data.vel_res = 0.5;
    else if (vel_res == 4) vcp_data.vel_res = 1.0;
    else vcp_data.vel_res = 0.0;
    /* Get elevation related information for each sweep. */
    for (i=0; i < vcp_data.num_cuts; i++) {
	fixed_angle = msgtype5[11 + i*23];
	azim_rate = msgtype5[15 + i*23];
	chconf_and_waveform = msgtype5[12 + i*23];
	sres_and_survprf = msgtype5[13 + i*23];
	vcp_data.doppler_prf_num[i] = msgtype5[23 + i*23];
	if (little_endian()) {
	    swap_2_bytes(&fixed_angle);
	    swap_2_bytes(&azim_rate);
	    swap_2_bytes(&chconf_and_waveform);
	    swap_2_bytes(&sres_and_survprf);
	    swap_2_bytes(&vcp_data.doppler_prf_num[i]);
	}
	vcp_data.fixed_angle[i] = wsr88d_get_angle(fixed_angle);
	vcp_data.azim_rate[i] = wsr88d_get_azim_rate(azim_rate);
	vcp_data.waveform[i] = chconf_and_waveform & 0xff;
	vcp_data.super_res_ctrl[i] = sres_and_survprf >> 8;
	vcp_data.surveil_prf_num[i] = sres_and_survprf & 0xff;
    }
}


void get_wsr88d_unamb_and_nyq_vel(Wsr88d_ray_m31 *wsr88d_ray, float *unamb_rng,
	float *nyq_vel)
{
    int dbptr, found, ray_hdr_len;
    short nyq_vel_sh, unamb_rng_sh;

    found = 0;
    ray_hdr_len = sizeof(wsr88d_ray->ray_hdr);
    dbptr = wsr88d_ray->ray_hdr.dbptr_radial_const - ray_hdr_len;
    if (strncmp(&wsr88d_ray->data[dbptr], "RRAD", 4) == 0) found = 1;
    else {
	dbptr = wsr88d_ray->ray_hdr.dbptr_elev_const - ray_hdr_len;
	if (strncmp(&wsr88d_ray->data[dbptr], "RRAD", 4) == 0) found = 1;
	else {
	    dbptr = wsr88d_ray->ray_hdr.dbptr_vol_const - ray_hdr_len;
	    if (strncmp(&wsr88d_ray->data[dbptr], "RRAD", 4) == 0) found = 1;
	}
    }
    if (found) {
	memcpy(&unamb_rng_sh, &wsr88d_ray->data[dbptr+6], 2);
	memcpy(&nyq_vel_sh, &wsr88d_ray->data[dbptr+16], 2);
	if (little_endian()) {
	    swap_2_bytes(&unamb_rng_sh);
	    swap_2_bytes(&nyq_vel_sh);
	}
	*unamb_rng = unamb_rng_sh / 10.;
	*nyq_vel = nyq_vel_sh / 100.;
    } else {
	*unamb_rng = 0.;
	*nyq_vel = 0.;
    }
}


int read_wsr88d_ray_m31(Wsr88d_file *wf, int msg_size,
	Wsr88d_ray_m31 *wsr88d_ray)
{
    Ray_header_m31 ray_hdr;
    int n, bytes_to_read, ray_hdr_len;
    float nyq_vel, unamb_rng;

    /* Read ray data header block */
    n = fread(&wsr88d_ray->ray_hdr, sizeof(Ray_header_m31), 1, wf->fptr);
    if (n < 1) {
	fprintf(stderr,"read_wsr88d_ray_m31: Read failed.\n");
	return 0;
    }

    /* Byte swap header if needed. */
    if (little_endian()) wsr88d_swap_m31_ray(&wsr88d_ray->ray_hdr);

    ray_hdr = wsr88d_ray->ray_hdr;

    /*
    if (ray_hdr.azm_num == 1) testprt(wsr88d_ray->ray_hdr);
    */

    ray_hdr_len = sizeof(ray_hdr);

    /* Read data portion of radial. */

    bytes_to_read = msg_size - ray_hdr_len;
    n = fread(wsr88d_ray->data, bytes_to_read, 1, wf->fptr);
    if (n < 1) {
	fprintf(stderr,"read_wsr88d_ray_m31: Read failed.\n");
	return 0;
    }

    /* We retrieve unambiguous range and Nyquist velocity here so that we don't
     * have to repeatedly do it for each data moment later.
     */
    get_wsr88d_unamb_and_nyq_vel(wsr88d_ray, &unamb_rng, &nyq_vel);
    wsr88d_ray->unamb_rng = unamb_rng;
    wsr88d_ray->nyq_vel = nyq_vel;

    return 1;
}


void wsr88d_load_ray_hdr(Wsr88d_ray_m31 wsr88d_ray, Ray *ray)
{
    int month, day, year, hour, minute, sec;
    float fsec;
    Wsr88d_ray m1_ray;
    Ray_header_m31 ray_hdr;

    ray_hdr = wsr88d_ray.ray_hdr;
    m1_ray.ray_date = ray_hdr.ray_date;
    m1_ray.ray_time = ray_hdr.ray_time;

    wsr88d_get_date(&m1_ray, &month, &day, &year);
    wsr88d_get_time(&m1_ray, &hour, &minute, &sec, &fsec);
    ray->h.year = year + 1900;
    ray->h.month = month;
    ray->h.day = day;
    ray->h.hour = hour;
    ray->h.minute = minute;
    ray->h.sec = sec + fsec;
    ray->h.azimuth = ray_hdr.azm;
    ray->h.ray_num = ray_hdr.azm_num;
    ray->h.elev = ray_hdr.elev;
    ray->h.elev_num = ray_hdr.elev_num;
    ray->h.unam_rng = wsr88d_ray.unamb_rng;
    ray->h.nyq_vel = wsr88d_ray.nyq_vel;
    int elev_index;
    elev_index = ray_hdr.elev_num - 1;
    ray->h.azim_rate = vcp_data.azim_rate[elev_index];
    ray->h.fix_angle = vcp_data.fixed_angle[elev_index];
    ray->h.vel_res = vcp_data.vel_res;
    if (ray_hdr.azm_res != 1)
	ray->h.beam_width = 1.0;
    else ray->h.beam_width = 0.5;

    /* Get some values using message type 1 routines.
     * First load VCP and elevation numbers into msg 1 ray.
     */
    m1_ray.vol_cpat = vcp_data.vcp;
    m1_ray.elev_num = ray_hdr.elev_num;
    m1_ray.unam_rng = (short) (wsr88d_ray.unamb_rng * 10.);
    ray->h.frequency = wsr88d_get_frequency(&m1_ray);
    ray->h.pulse_width = wsr88d_get_pulse_width(&m1_ray);
    ray->h.pulse_count = wsr88d_get_pulse_count(&m1_ray);
    ray->h.prf = wsr88d_get_prf(&m1_ray);
    ray->h.wavelength = 0.1071;
}


void wsr88d_load_ray(Wsr88d_ray_m31 wsr88d_ray, int data_ptr,
	int isweep, int iray, Radar *radar)
{
    /* Load data into ray structure for this field or data moment. */

    Data_moment_hdr data_hdr;
    int ngates;
    int i, hdr_size;
    float value, scale, offset;
    unsigned char *data;
    Range (*invf)(float x);
    float (*f)(Range x);
    Ray *ray;
    int vol_index, waveform;

    enum waveforms {surveillance=1, doppler_w_amb_res, doppler_no_amb_res,
	batch};

    /* Get data moment header. */
    hdr_size = sizeof(data_hdr);
    memcpy(&data_hdr, &wsr88d_ray.data[data_ptr], hdr_size);
    data_ptr += hdr_size;
    if (little_endian()) wsr88d_swap_data_hdr(&data_hdr);

    vol_index = wsr88d_get_vol_index(data_hdr.dataname);
    switch (vol_index) {
	case DZ_INDEX: f = DZ_F; invf = DZ_INVF; break;
	case VR_INDEX: f = VR_F; invf = VR_INVF; break;
	case SW_INDEX: f = SW_F; invf = SW_INVF; break;
	default: f = DZ_F; invf = DZ_INVF; break;
    }
    /* If this is reflectivity, check the waveform type to make sure
     * it isn't from a Doppler split cut.
     * We only keep reflectivity if the waveform type is surveillance or
     * batch, or the elevation is above the split cut elevations.
     */
    waveform = vcp_data.waveform[isweep];
    if (vol_index != DZ_INDEX || (waveform == surveillance ||
		waveform == batch || vcp_data.fixed_angle[isweep] >= 6.0)) {
	if (radar->v[vol_index] == NULL) {
	    radar->v[vol_index] = RSL_new_volume(MAXSWEEPS);
	    radar->v[vol_index]->h.f = f;
	    radar->v[vol_index]->h.invf = invf;
	}
	if (radar->v[vol_index]->sweep[isweep] == NULL) {
	    radar->v[vol_index]->sweep[isweep] = RSL_new_sweep(MAXRAYS_M31);
	    radar->v[vol_index]->sweep[isweep]->h.f = f;
	    radar->v[vol_index]->sweep[isweep]->h.invf = invf;
	}
	ngates = data_hdr.ngates;
	ray = RSL_new_ray(ngates);

	/* Convert data to float, then use range function to store in ray.
	 * Note: data range is 2-255. 0 means signal is below threshold, and 1
	 * means range folded.
	 */

	offset = data_hdr.offset;
	scale = data_hdr.scale;
	if (data_hdr.scale == 0) scale = 1.0; 
	data = &wsr88d_ray.data[data_ptr];
	for (i = 0; i < ngates; i++) {
	    if (data[i] > 1)
		value = (data[i] - offset) / scale;
	    else value = (data[i] == 0) ? BADVAL : RFVAL;
	    ray->range[i] = invf(value);
	    ray->h.f = f;
	    ray->h.invf = invf;
	}
	wsr88d_load_ray_hdr(wsr88d_ray, ray);
	radar->v[vol_index]->sweep[isweep]->ray[iray] = ray;
	radar->v[vol_index]->sweep[isweep]->h.nrays = iray+1;
    }

    ray->h.range_bin1 = data_hdr.range_first_gate;
    ray->h.gate_size = data_hdr.range_samp_interval;
    ray->h.nbins = ngates;
}


int wsr88d_get_vol_index(char* dataname)
{
    int vol_index;

    if (strncmp(dataname, "DREF", 4) == 0) vol_index = DZ_INDEX;
    if (strncmp(dataname, "DVEL", 4) == 0) vol_index = VR_INDEX;
    if (strncmp(dataname, "DSW", 3) == 0) vol_index = SW_INDEX;
    /* TODO: Add the other data moments. */

    return vol_index;
}


void wsr88d_load_ray_into_radar(Wsr88d_ray_m31 wsr88d_ray, int isweep, int iray,
	Radar *radar)
{
    int data_ptr, hdr_size;

    hdr_size = sizeof(wsr88d_ray.ray_hdr);
    data_ptr = wsr88d_ray.ray_hdr.dbptr_ref - hdr_size;
    if (data_ptr  > 0) wsr88d_load_ray(wsr88d_ray, data_ptr, isweep,
	    iray, radar);
	{
    }
    data_ptr = wsr88d_ray.ray_hdr.dbptr_vel - hdr_size;
    if (data_ptr  > 0) wsr88d_load_ray(wsr88d_ray, data_ptr, isweep,
	    iray, radar);
	{
    }
    data_ptr = wsr88d_ray.ray_hdr.dbptr_sw - hdr_size;
    if (data_ptr  > 0) wsr88d_load_ray(wsr88d_ray, data_ptr, isweep,
	    iray, radar);
	{
    }
}


void wsr88d_load_sweep_header(Radar *radar, int isweep,
	Wsr88d_ray_m31 wsr88d_ray)
{
    int ivolume;
    Ray_header_m31 ray_hdr;
    Sweep *sweep;
    /*    int vcp; */

    ray_hdr = wsr88d_ray.ray_hdr;

    for (ivolume=0; ivolume < MAX_RADAR_VOLUMES; ivolume++) {
	if (radar->v[ivolume] != NULL && radar->v[ivolume]->sweep[isweep] != NULL) {
	    sweep = radar->v[ivolume]->sweep[isweep];
	    radar->v[ivolume]->sweep[isweep]->h.sweep_num = ray_hdr.elev_num;
	    radar->v[ivolume]->sweep[isweep]->h.elev =
		vcp_data.fixed_angle[isweep];
	    if (ray_hdr.azm_res != 1)
		sweep->h.beam_width = 1.0;
	    else sweep->h.beam_width = 0.5;
	    sweep->h.vert_half_bw = sweep->h.beam_width / 2.;
	    sweep->h.horz_half_bw = sweep->h.beam_width / 2.;
	}
    }
}


Radar *load_wsr88d_m31_into_radar(Wsr88d_file *wf)
{
    Wsr88d_msg_hdr msghdr;
    Wsr88d_ray_m31 wsr88d_ray;
    short non31_seg_remainder[1202]; /* Remainder after message header */
    int end_of_vos = 0, isweep = 0, iray = 0;
    int msg_hdr_size, msg_size, n;
    Radar *radar = NULL;

/* Message type 31 is a variable length message, whereas all other message
 * types are made up of 2432 byte segments.  To handle this, we read the
 * message header and check the message type.  If it is not 31, then we read
 * the remainder of the constant size segment.  If message type is 31, we read
 * the remainder of the message by the size given in message header.
 * When reading the message header, we must include 12 bytes inserted
 * by RPG, which we ignore, followed by the 8 halfwords (short integers) which
 * make up the actual message header.
 * For more information, see the "Interface Control Document for the RDA/RPG"
 * at the WSR-88D Radar Operations Center web site.
 */

    n = fread(&msghdr, sizeof(Wsr88d_msg_hdr), 1, wf->fptr);

    /* printf("msgtype = %d\n", msghdr.msg_type); */
    msg_hdr_size = sizeof(Wsr88d_msg_hdr) - sizeof(msghdr.rpg);


    radar = RSL_new_radar(MAX_RADAR_VOLUMES);
    
    while (! end_of_vos) {
	if (msghdr.msg_type == 31) {
	    if (little_endian()) wsr88d_swap_m31_hdr(&msghdr);

	    /* Get size in bytes of message following message header.
	     * The size given in message header is in halfwords, so double it.
	     */
	    msg_size = (int) msghdr.msg_size * 2 - msg_hdr_size;

	    n = read_wsr88d_ray_m31(wf, msg_size, &wsr88d_ray);
	    if (n <= 0) return NULL;

	    /* Load this ray into radar structure ray. */
	    wsr88d_load_ray_into_radar(wsr88d_ray, isweep, iray, radar);
	    iray++;
	}
	else { /* msg_type not 31 */
	    n = fread(&non31_seg_remainder, sizeof(non31_seg_remainder), 1,
		    wf->fptr);
	    if (n < 1) {
		fprintf(stderr,"Warning: load_wsr88d_m31_into_radar: ");
		if (feof(wf->fptr) != 0)
		    fprintf(stderr, "Unexpected end of file.\n");
		else fprintf(stderr,"Read failed.\n");
		fprintf(stderr,"Current sweep number: %d\n"
			"Last ray read: %d\n", isweep+1, iray);
		wsr88d_load_sweep_header(radar, isweep, wsr88d_ray);
		return radar;
	    }
	    if (msghdr.msg_type == 5) {
		wsr88d_get_vcp_data(non31_seg_remainder);
		radar->h.vcp = vcp_data.vcp;
		/* printf("VCP = %d\n", vcp_data.vcp); */
	    }
	}

	/* Check for end of sweep */
	if (wsr88d_ray.ray_hdr.radial_status == END_OF_ELEV) {
	    wsr88d_load_sweep_header(radar, isweep, wsr88d_ray);
	    isweep++;
	    iray = 0;
	}

	if (wsr88d_ray.ray_hdr.radial_status != END_VOS) {
	    n = fread(&msghdr, sizeof(Wsr88d_msg_hdr), 1, wf->fptr);
	    if (n < 1) {
		fprintf(stderr,"Warning: load_wsr88d_m31_into_radar: ");
		if (feof(wf->fptr) != 0) fprintf(stderr,
			"Unexpected end of file.\n");
		else fprintf(stderr,"Failed reading msghdr.\n");
		fprintf(stderr,"Current sweep number: %d\n"
			"Last ray read: %d\n", isweep+1, iray);
		wsr88d_load_sweep_header(radar, isweep, wsr88d_ray);
		return radar;
	    }
	}
	else {
	    end_of_vos = 1;
	    wsr88d_load_sweep_header(radar, isweep, wsr88d_ray);
	}
	if (feof(wf->fptr) != 0) end_of_vos = 1;
    }

    return radar;
}
