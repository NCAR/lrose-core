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
/* 	$Id: Correction.h,v 1.3 2016/03/07 01:23:00 dixon Exp $	 */
struct correction_d {
    char correction_des[4];	/* Correction descriptor identifier: ASCII */
				/* characters "CFAC" stand for Volume */
				/* Descriptor. */
    si32  correction_des_length; /*Correction  descriptor length in bytes. */
    fl32 azimuth_corr;          /* Correction added to azimuth[deg]*/
    fl32 elevation_corr;        /* Correction added to elevation[deg]*/
    fl32 range_delay_corr;      /* Correction used for range delay[m]*/
    fl32 longitude_corr;        /* Correction added to radar longitude*/
    fl32 latitude_corr;         /* Correction added to radar latitude*/
    fl32 pressure_alt_corr;     /* Correction added to pressure altitude*/
                                 /* (msl)[km]*/
    fl32 radar_alt_corr;        /* Correction added to radar altitude above */
                                 /* ground level(agl) [km]*/
    fl32 ew_gndspd_corr;       /* Correction added to radar platform*/
                                   /*ground speed(E-W)[m/s]*/
    fl32 ns_gndspd_corr;          /* Correction added to radar platform*/
                                   /* ground speed(N-S)[m/s]*/
    fl32 vert_vel_corr;          /* Correction added to radar platform */
                                   /*vertical velocity[m/s]*/
    fl32 heading_corr;            /* Correction added to radar platform */
                                   /* heading [deg])*/
    fl32 roll_corr;               /* Correction added to radar platform*/
                                   /* roll[deg]*/
    fl32 pitch_corr;              /* Correction added to radar platform*/
                                   /* pitch[deg]*/
    fl32 drift_corr;              /* Correction added to radar platform*/
                                   /*drift[deg]*/
    fl32 rot_angle_corr;          /* Corrrection add to radar rotation angle*/
                                   /*[deg]*/
    fl32 tilt_corr;              /* Correction added to radar tilt angle*/

}; /* End of Structure */


typedef struct correction_d correction_d;
typedef struct correction_d CORRECTION;

