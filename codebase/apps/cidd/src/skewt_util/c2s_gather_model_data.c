/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/*************************************************************************
 * C2S_GATHER_MODEL_DATA.c : Make all requests to Servers and compute
 *   fields necessary to generate a CLASS file
 */

#define C2S_GATHER_MODEL_DATA

#include "cidd2skewt.h"

static void compute_press_levels(void);
static void compute_temper_levels(void);
static void compute_dewpt_levels(void);
static void compute_relhum_levels(void);
double e_sub_s(double deg_c);
 
/*************************************************************************
 * GATHER_MODEL_DATA : Gather a vertical column of various variables from
 * a cidd server. Compute any fields that are not directly availible from the model
 */
void gather_model_data(void)
{

    int i;
    unsigned char *d_ptr = NULL;
    double *heights = NULL;
    double *h_ptr = NULL;
     
    cd_command_t com;
    cd_reply_t reply;
    cd_grid_info_t info;

    struct tm *tm;
    char time_string[256];

    /* Record info from CIDD's key click  */
    gd.t_begin = gd.coord_expt->epoch_start;
    gd.t_mid = gd.coord_expt->time_cent;
    gd.t_end = gd.coord_expt->epoch_end;

    gd.lat = gd.coord_expt->pointer_lat;
    gd.lon = gd.coord_expt->pointer_lon;

    gd.x_km = gd.coord_expt->pointer_x;
    gd.y_km = gd.coord_expt->pointer_y;

    /* Set command info common to all requests */
    com.primary_com = GET_INFO | GET_DATA;
    com.second_com = GET_V_PLANE;
    com.time_min = gd.t_begin;
    com.time_cent = gd.t_mid;
    com.time_max = gd.t_end;
    com.data_type = CDATA_CHAR;
    com.lat_origin = gd.lat;
    com.lon_origin = gd.lon;
    com.ht_origin = 0.0;
    com.min_x = 0.0;     /* We want only one column of data centered around the lat,lon */
    com.max_x = 0.0;
    com.min_y = 0.0;
    com.max_y = 0.0;
    com.min_z = 0.0;     /* sea level to top of atm */
    com.max_z = 40.0;

    if(gd.debug) {
	printf("Gather data around %g, %g\n",com.lat_origin,com.lon_origin);
	strftime(time_string,256,"Between %R %D and ",gmtime(&gd.t_begin));
	puts(time_string);
	strftime(time_string,256,"%R %D\n",gmtime(&gd.t_end));
	puts(time_string);
	strftime(time_string,256,"Closest to %R %D\n",gmtime(&gd.t_mid));
	puts(time_string);
    }
     
    /* Gather V Wind data & plane heights  */
    if(gd.V_wind.field_no >=0) {
	com.data_field = gd.V_wind.field_no;
        if((cdata_get_with_heights(&com,&reply,&info,&heights,&d_ptr,gd.V_wind.host,gd.V_wind.port)) < 0) {
	    fprintf(stderr,"Couldn't get V_winds\n");
	    gd.num_levels = 0;
	    xv_set(gd.C2s_base_win_c2s_bw->c2s_bw,XV_LABEL, "FAILURE", NULL);
	    return;
	}
	gd.num_levels = info.nz;
	gd.data_time = reply.time_cent;
	/* Unpack data array */
	for(i=0; i < info.nz; i++ ) {
	    gd.V_wind.val[i] = d_ptr[i] * reply.scale + reply.bias;
	    gd.V_wind.val[i] = gd.V_wind.val[i] * gd.V_wind.scale + gd.V_wind.bias;
	}
	free(d_ptr);

	if(heights != NULL) {
	    /* Extract the altitude info - 3 values per plane, pick the middle */
	    for((h_ptr = heights + 1),i=0; i < info.nz; i++,h_ptr +=3 ) {
	        gd.alt_msl[i] = *h_ptr * 1000.0;
	    }
	    free(heights);
	} else {
	    for(i=0; i < info.nz; i++) {
		gd.alt_msl[i] = ((info.min_z + info.dz / 2.0) + (info.dz * i)) * 1000.0;
	    }
	}
    } else {
	for(i=0; i < MAX_VERT_LEVELS; i++ ) {
	    gd.V_wind.val[i] = 0.0;
	}
    }
     
    /* Gather U Wind data */
    if(gd.U_wind.field_no >=0) {
	com.data_field = gd.U_wind.field_no;
        if((d_ptr = cdata_get(&com,&reply,&info,gd.U_wind.host,gd.U_wind.port)) == NULL) {
	    fprintf(stderr,"Couldn't get U_winds \n");
	    gd.num_levels = 0;
	    xv_set(gd.C2s_base_win_c2s_bw->c2s_bw,XV_LABEL, "FAILURE", NULL);
	    return;
	}
	gd.num_levels = info.nz;
	/* Unpack data array */
	for(i=0; i < info.nz; i++ ) {
	    gd.U_wind.val[i] = d_ptr[i] * reply.scale + reply.bias;
	    gd.U_wind.val[i] = gd.U_wind.val[i] * gd.U_wind.scale + gd.U_wind.bias;
	}
	free(d_ptr);
    } else {
	for(i=0; i < MAX_VERT_LEVELS; i++ ) {
	    gd.U_wind.val[i] = 0.0;
	}
    }

    /* Gather Mix_r data */
    if(gd.Mix_r.field_no >=0) {
	com.data_field = gd.Mix_r.field_no;
        if((d_ptr = cdata_get(&com,&reply,&info,gd.Mix_r.host,gd.Mix_r.port)) == NULL) {
	    fprintf(stderr,"Couldn't get Mix_r \n");
	    gd.num_levels = 0;
	    xv_set(gd.C2s_base_win_c2s_bw->c2s_bw,XV_LABEL, "FAILURE", NULL);
	    return;
	}
	gd.num_levels = info.nz;
	/* Unpack data array */
	for(i=0; i < info.nz; i++ ) {
	    gd.Mix_r.val[i] = d_ptr[i] * reply.scale + reply.bias;
	    gd.Mix_r.val[i] = gd.Mix_r.val[i] * gd.Mix_r.scale + gd.Mix_r.bias;
	}
	free(d_ptr);
    } else {
	for(i=0; i < MAX_VERT_LEVELS; i++ ) {
	    gd.Mix_r.val[i] = 0.0;
	}
    }
     
    /* Gather Theta  data */
    if(gd.Theta.field_no >=0) {
	com.data_field = gd.Theta.field_no;
        if((d_ptr = cdata_get(&com,&reply,&info,gd.Theta.host,gd.Theta.port)) == NULL) {
	    fprintf(stderr,"Couldn't get Theta \n");
	    gd.num_levels = 0;
	    xv_set(gd.C2s_base_win_c2s_bw->c2s_bw,XV_LABEL, "FAILURE", NULL);
	    return;
	}
	gd.num_levels = info.nz;
	/* Unpack data array */
	for(i=0; i < info.nz; i++ ) {
	    gd.Theta.val[i] = d_ptr[i] * reply.scale + reply.bias;
	    gd.Theta.val[i] = gd.Theta.val[i] * gd.Theta.scale + gd.Theta.bias;
	}
	free(d_ptr);
    } else {
	for(i=0; i < MAX_VERT_LEVELS; i++ ) {
	    gd.Theta.val[i] = 0.0;
	}
    }

    /* Gather Pressure data */
    if(gd.Press.field_no >=0) {
	com.data_field = gd.Press.field_no;
        if((d_ptr = cdata_get(&com,&reply,&info,gd.Press.host,gd.Press.port)) == NULL) {
	    fprintf(stderr,"Couldn't get Press \n");
	    gd.num_levels = 0;
	    xv_set(gd.C2s_base_win_c2s_bw->c2s_bw,XV_LABEL, "FAILURE", NULL);
	    return;
	}
	gd.num_levels = info.nz;
	/* Unpack data array */
	for(i=0; i < info.nz; i++ ) {
	    gd.Press.val[i] = d_ptr[i] * reply.scale + reply.bias;
	    gd.Press.val[i] = gd.Press.val[i] * gd.Press.scale + gd.Press.bias;
	}
	free(d_ptr);
    } else {
	/* Pressure can be computed as a funtion of theta and altitude */
	compute_press_levels();
    }
     
    /* Gather Temper data */
    if(gd.Temper.field_no >=0) {
	com.data_field = gd.Temper.field_no;
        if((d_ptr = cdata_get(&com,&reply,&info,gd.Temper.host,gd.Temper.port)) == NULL) {
	    fprintf(stderr,"Couldn't get Temper \n");
	    gd.num_levels = 0;
	    xv_set(gd.C2s_base_win_c2s_bw->c2s_bw,XV_LABEL, "FAILURE", NULL);
	    return;
	}
	gd.num_levels = info.nz;
	/* Unpack data array */
	for(i=0; i < info.nz; i++ ) {
	    gd.Temper.val[i] = d_ptr[i] * reply.scale + reply.bias;
	    gd.Temper.val[i] = gd.Temper.val[i] * gd.Temper.scale + gd.Temper.bias;
	}
	free(d_ptr);
    } else {
	/* Temperature can be computed as a funtion of theta and pressure */
	compute_temper_levels();
    }
     
    /* Gather Dew point data */
    if(gd.Dewpt.field_no >=0) {
	com.data_field = gd.Dewpt.field_no;
        if((d_ptr = cdata_get(&com,&reply,&info,gd.Dewpt.host,gd.Dewpt.port)) == NULL) {
	    fprintf(stderr,"Couldn't get Dewpt \n");
	    gd.num_levels = 0;
	    xv_set(gd.C2s_base_win_c2s_bw->c2s_bw,XV_LABEL, "FAILURE", NULL);
	    return;
	}
	gd.num_levels = info.nz;
	/* Unpack data array */
	for(i=0; i < info.nz; i++ ) {
	    gd.Dewpt.val[i] = d_ptr[i] * reply.scale + reply.bias;
	    gd.Dewpt.val[i] = gd.Dewpt.val[i] * gd.Dewpt.scale + gd.Dewpt.bias;
	}
	free(d_ptr);
    } else {
	/* Dew Point can be computed as a funtion of Pressure and Mixing_ratio */
	compute_dewpt_levels();
    }

     
    /* Gather Relative Humidity data */
    if(gd.Relhum.field_no >=0) {
	com.data_field = gd.Relhum.field_no;
        if((d_ptr = cdata_get(&com,&reply,&info,gd.Relhum.host,gd.Relhum.port)) == NULL) {
	    fprintf(stderr,"Couldn't get Relhum \n");
	    gd.num_levels = 0;
	    xv_set(gd.C2s_base_win_c2s_bw->c2s_bw,XV_LABEL, "FAILURE", NULL);
	    return;
	}
	gd.num_levels = info.nz;
	/* Unpack data array */
	for(i=0; i < info.nz; i++ ) {
	    gd.Relhum.val[i] = d_ptr[i] * reply.scale + reply.bias;
	    gd.Relhum.val[i] = gd.Relhum.val[i] * gd.Relhum.scale + gd.Relhum.bias;
	}
	free(d_ptr);
    } else {
	/* Relative Humidity can be computed as a funtion of Temperature and Dew point */
	compute_relhum_levels();
    }

    xv_set(gd.C2s_base_win_c2s_bw->c2s_bw,XV_LABEL, "Cidd2Skewt", NULL);
    if(gd.debug) printf("Number of points: %d\n",gd.num_levels);
}

/* Constants used in computations */
#define G    9.81
#define K    0.288
#define R    287.0
#define A    5.0065
#define B    19.839
#define T3   273.15
#define E3   6.1078
#define ETA  622.0
/*************************************************************************
 * COMPUTE_PRESS_LEVELS: Generate Pressure data based on standard conversion or
 *   as a function of Theta
 */
static void compute_press_levels(void)
{
    int i;
    double del_z;
    double theta_ave;
    double gk_r;
    double pnot_k;
    double temp;

    if(gd.Theta.field_no < 0) { /* use standard conversion */
        for(i=0; i < gd.num_levels; i++) {
	    gd.Press.val[i] = exp(log(1.0 - gd.alt_msl[i]/44307.692)/0.19)*1013.2;
        }
    } else {  /* Compute as a function of theta */

	gd.Press.val[0] = gd.base_press;
	gk_r =  G * K / R;
	pnot_k = pow(gd.base_press,K);

	for(i=1; i < gd.num_levels; i++) {
	    del_z = gd.alt_msl[i] - gd.alt_msl[i -1];
	    theta_ave = (gd.Theta.val[i] + gd.Theta.val[i -1]) / 2.0;
	    temp = pow(gd.Press.val[i-1],K) - (gk_r * pnot_k * (del_z / theta_ave));
	    gd.Press.val[i] = pow(temp,(1/K));
	}
    }
}

/*************************************************************************
 * COMPUTE_TEMPER_LEVELS: Compute Temperature as a funtion of theta and pressure
 */
static void compute_temper_levels(void)
{
    int i;
    for(i=0; i < gd.num_levels; i++) {
	gd.Temper.val[i] = gd.Theta.val[i] * pow((gd.Press.val[i]/gd.base_press),K) - T3;
    }
}

/*************************************************************************
 * COMPUTE_DEWPT_LEVELS:  Compute Dew Point as a funtion of Pressure and Mixing_ratio
 */
static void compute_dewpt_levels(void)
{
    int i;
    double e;
    double temp;

    for(i=0; i < gd.num_levels; i++) {
	e = gd.Press.val[i] * gd.Mix_r.val[i] / (ETA + gd.Mix_r.val[i]);
	temp = sqrt(B * B + (2 * A * log(E3 / e)));
	gd.Dewpt.val[i] = (T3 * A / (A - B + temp)) - T3;
    }
}

/*************************************************************************
 * E_SUB_S:  Return saturation vapor pressure (Pa) over liquid using
 *       polynomial fit of goff-gratch (1946) formulation. (walko, 1991)
 *  C by F. Hage from G. Thompson's FORTRAN. - 1995.
 */
#define C0 610.5851
#define C1 44.40316
#define C2 1.430341
#define C3 .2641412e-1
#define C4 .2995057e-3
#define C5 .2031998e-5
#define C6 .6936113e-8
#define C7 .2564861e-11
#define C8 -.3704404e-13

double e_sub_s(double deg_c)
{
    double t;
    double e_sub_s;

    t = (deg_c < -80.0)? -80.0: deg_c;
    e_sub_s = C0 + t*(C1 + t*(C2 + t*(C3 + t*(C4 + t*(C5 + t*(C6 + t*(C7 + t*C8)))))));
    return e_sub_s;
}

/*************************************************************************
 * COMPUTE_RELHUM_LEVELS:  Compute Relative Humidity as a function of
 *  Temperature and Dewpoint. - RH - e/e_s
 */
static void compute_relhum_levels(void)
{
    int i;
    double temp;

    for(i=0; i < gd.num_levels; i++) {
	temp = e_sub_s(gd.Dewpt.val[i]) / e_sub_s(gd.Temper.val[i]);
	gd.Relhum.val[i] = temp * 100.0;
    }
}

