/* 	$Id: Correction.h,v 1.1 2007/09/22 21:08:21 dixon Exp $	 */
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

