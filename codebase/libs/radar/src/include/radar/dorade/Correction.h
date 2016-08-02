/* 	$Id: Correction.h,v 1.1 2008/01/24 22:22:32 rehak Exp $	 */
struct correction_d {
    char correction_des[4];	/* Correction descriptor identifier: ASCII */
				/* characters "CFAC" stand for Volume */
				/* Descriptor. */
    long  correction_des_length; /*Correction  descriptor length in bytes. */
    float azimuth_corr;          /* Correction added to azimuth[deg]*/
    float elevation_corr;        /* Correction added to elevation[deg]*/
    float range_delay_corr;      /* Correction used for range delay[m]*/
    float longitude_corr;        /* Correction added to radar longitude*/
    float latitude_corr;         /* Correction added to radar latitude*/
    float pressure_alt_corr;     /* Correction added to pressure altitude*/
                                 /* (msl)[km]*/
    float radar_alt_corr;        /* Correction added to radar altitude above */
                                 /* ground level(agl) [km]*/
    float ew_gndspd_corr;       /* Correction added to radar platform*/
                                   /*ground speed(E-W)[m/s]*/
    float ns_gndspd_corr;          /* Correction added to radar platform*/
                                   /* ground speed(N-S)[m/s]*/
    float vert_vel_corr;          /* Correction added to radar platform */
                                   /*vertical velocity[m/s]*/
    float heading_corr;            /* Correction added to radar platform */
                                   /* heading [deg])*/
    float roll_corr;               /* Correction added to radar platform*/
                                   /* roll[deg]*/
    float pitch_corr;              /* Correction added to radar platform*/
                                   /* pitch[deg]*/
    float drift_corr;              /* Correction added to radar platform*/
                                   /*drift[deg]*/
    float rot_angle_corr;          /* Corrrection add to radar rotation angle*/
                                   /*[deg]*/
    float tilt_corr;              /* Correction added to radar tilt angle*/

}; /* End of Structure */


typedef struct correction_d correction_d;
typedef struct correction_d CORRECTION;

