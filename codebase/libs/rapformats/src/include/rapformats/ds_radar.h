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

/*******************************************************************
 * ds_radar.h
 *
 * Radar data types for DIDSS.
 ******************************************************************/

#ifndef DsRadar_h
#define DsRadar_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <dataport/port_types.h>

#ifndef DS_FILE_LABEL_LEN
#define DS_FILE_LABEL_LEN 40
#endif

#ifndef DS_LABEL_LEN
#define DS_LABEL_LEN 40
#endif

#define DS_FIELD_NAME_LEN 32
#define DS_FIELD_UNITS_LEN 16

/* radar platform type */

#define DS_RADAR_GROUND_TYPE 0
#define DS_RADAR_AIRBORNE_FORE_TYPE 1
#define DS_RADAR_AIRBORNE_AFT_TYPE 2
#define DS_RADAR_AIRBORNE_TAIL_TYPE 3
#define DS_RADAR_AIRBORNE_LOWER_TYPE 4
#define DS_RADAR_SHIPBORNE_TYPE 5
#define DS_RADAR_VEHICLE_TYPE 6
#define DS_RADAR_AIRBORNE_UPPER_TYPE 7

/* scan type */

#define DS_RADAR_UNKNOWN_MODE -1
#define DS_RADAR_CALIBRATION_MODE 0
#define DS_RADAR_SECTOR_MODE 1
#define DS_RADAR_COPLANE_MODE 2
#define DS_RADAR_RHI_MODE 3
#define DS_RADAR_VERTICAL_POINTING_MODE 4
#define DS_RADAR_TARGET_MODE 5
#define DS_RADAR_MANUAL_MODE 6
#define DS_RADAR_IDLE_MODE 7
#define DS_RADAR_SURVEILLANCE_MODE 8
#define DS_RADAR_AIRBORNE_MODE 9
#define DS_RADAR_HORIZONTAL_MODE 10
#define DS_RADAR_SUNSCAN_MODE 11 /* scan the sun in PPI mode */
#define DS_RADAR_POINTING_MODE 12
#define DS_RADAR_FOLLOW_VEHICLE_MODE 13
#define DS_RADAR_EL_SURV_MODE 14 /* Elevation surveillaince, e.g. ELDORA */
#define DS_RADAR_MANPPI_MODE 15 /* manual ppi */
#define DS_RADAR_MANRHI_MODE 16 /* manual rhi */
#define DS_RADAR_SUNSCAN_RHI_MODE 17 /* scan the sun in RHI mode */

/* follow mode duplicated from IWRF */

#define DS_RADAR_FOLLOW_MODE_UNKNOWN 0
#define DS_RADAR_FOLLOW_MODE_NONE 1 /**< Radar is not tracking any object */
#define DS_RADAR_FOLLOW_MODE_SUN 2 /**< Radar is tracking the sun */
#define DS_RADAR_FOLLOW_MODE_VEHICLE 3 /**< Radar is tracking a vehicle */
#define DS_RADAR_FOLLOW_MODE_AIRCRAFT 4 /**< Radar is tracking an aircraft */
#define DS_RADAR_FOLLOW_MODE_TARGET 5 /**< Radar is tracking a target - e.g. sphere */
#define DS_RADAR_FOLLOW_MODE_MANUAL 6 /**< Radar is under manual tracking mode */

/* polarization type */

#define DS_POLARIZATION_HORIZ_TYPE 0
#define DS_POLARIZATION_VERT_TYPE 1
#define DS_POLARIZATION_RIGHT_CIRC_TYPE 2
#define DS_POLARIZATION_ELLIPTICAL_TYPE 3
#define DS_POLARIZATION_LEFT_CIRC_TYPE 4
#define DS_POLARIZATION_DUAL_TYPE 5
#define DS_POLARIZATION_DUAL_HV_ALT 6
#define DS_POLARIZATION_DUAL_HV_SIM 7
#define DS_POLARIZATION_DUAL_H_XMIT 8
#define DS_POLARIZATION_DUAL_V_XMIT 9

/* prf mode duplicated from IWRF */

#define DS_RADAR_PRF_MODE_NOT_SET 0
#define DS_RADAR_PRF_MODE_FIXED 1
#define DS_RADAR_PRF_MODE_STAGGERED_2_3 2
#define DS_RADAR_PRF_MODE_STAGGERED_3_4 3
#define DS_RADAR_PRF_MODE_STAGGERED_4_5 4

/*
 * message type definition
 */

#define DS_MESSAGE_TYPE_DSRADAR 1001
#define DS_MESSAGE_TYPE_RIDDS_BEAM 1002
#define DS_MESSAGE_TYPE_LL_BEAM 1003
#define DS_MESSAGE_TYPE_EEC_ASCII 1004

/*
 * end-of-vol message type - needs to be moved - dixon
 */

#define DS_MESSAGE_TYPE_END_OF_VOLUME 1008

/*
 * part data type definitions
 */

#define DS_DATA_TYPE_RADAR_PARAMS 1
#define DS_DATA_TYPE_RADAR_FIELD_PARAMS 2
#define DS_DATA_TYPE_RADAR_BEAM_DATA 4
#define DS_DATA_TYPE_RADAR_FLAGS 8
#define DS_DATA_TYPE_RADAR_CALIB 16
#define DS_DATA_TYPE_STATUS_XML 32
#define DS_DATA_TYPE_PLATFORM_GEOREF 64

#define DS_IWRF_PLATFORM_GEOREF_ID 0x77770111
 
/*
 * The message part data types are as follows:
 *
 * RADAR_PARAMS = 1,
 * FIELD_PARAMS = 2,
 * RADAR_BEAM   = 4,
 * RADAR_FLAGS  = 8
 * RADAR_CALIB  = 16
 *
 * The actual definitions are in a typedef in the DsRadarMsg class.
 */
  
/*
 * DS_MESSAGE_TYPE_RADAR has the following components.
 * Not all components will be in each message.
 *
 *   DsRadarParams_t
 *   nfields * DsFieldParams_t
 *   DsBeamHdr_t and gate_by_gate data
 */

#define NCHAR_DS_RADAR_PARAMS (2 * DS_LABEL_LEN)

typedef struct {
  
  si32 radar_id;		/* unique number */

  si32 radar_type;              /* use radar type defs above */

  si32 nfields;                 /* number of fields */

  si32 ngates;                  /* number of range gates */

  si32 samples_per_beam;        /* number of pulse samples per
				 * data beam */

  si32 scan_type;		/* the current scan strategy */
  
  si32 scan_mode;		/* use scan type defs above */
  
  si32 nfields_current;	        /* the number of fields currently being
				 * sent - the positions of the fields
				 * are indicated by the
				 * bits set in the field_flag */
  
  si32 field_flag;		/* for each field included in the beam data,
				 * the relevant bit is set in this long.
				 * For example, suppose there are a total
				 * of 6 fields referred to in the params
				 * struct, and only fields
				 * 0, 1, 3, and 5 are currently in
				 * the data stream.
				 * Then, field_flag = 00.....0101011 */

  si32 polarization;            /* use polarization type defs above */

  si32 follow_mode;
  si32 prf_mode;
  si32 spare_ints[2];

  fl32 radar_constant;          /* radar constant */
  fl32 altitude;		/* km */
  fl32 latitude;		/* degrees */
  fl32 longitude;		/* degrees */
  fl32 gate_spacing;		/* km */
  fl32 start_range;		/* km */
  fl32 horiz_beam_width;	/* degrees */
  fl32 vert_beam_width;         /* degrees */
  fl32 pulse_width;		/* micro-seconds */
  fl32 prf;			/* pulse repitition freq (/s) */
  fl32 wavelength;		/* cm */
  fl32 xmit_peak_pwr;           /* calibrated power, watts */
  fl32 receiver_mds;            /* dBm */
  fl32 receiver_gain;           /* dB */
  fl32 antenna_gain;            /* dB */
  fl32 system_gain;             /* dB */
  fl32 unambig_vel;             /* m/s */
  fl32 unambig_range;           /* km */
   
  fl32 measXmitPowerDbmH;       /* measured H power in dBm */
  fl32 measXmitPowerDbmV;       /* measured V power in dBm */

  fl32 prt;
  fl32 prt2;
  fl32 spare_floats[4];

  char radar_name[DS_LABEL_LEN];
  char scan_type_name[DS_LABEL_LEN];
  
} DsRadarParams_t;

#define NCHAR_DS_FIELD_PARAMS (DS_FIELD_NAME_LEN + DS_FIELD_UNITS_LEN)

typedef struct {

  si32 byte_width;                    /* width of data in bytes */
  si32 missing_data_value;            /* value used for missing data */
   
  fl32 scale;			      /* gain of the data */
  fl32 bias;			      /* offset of zero value */

  fl32 spare_floats[2];

  char name[DS_FIELD_NAME_LEN];       /* field name */
  char units[DS_FIELD_UNITS_LEN];     /* field units */
  
} DsFieldParams_t;

/*
 * struct for header for beam data packet
 */

typedef struct {
  
  si32 time;               /* secs since Jan 1 1970 */
  si32 nano_secs;          /* nano-second time */
  si32 reference_time;     /* epoch - not used yet, set to 0 */

  si32 vol_num;            /* the volume scan number */
  si32 tilt_num;	   /* the tilt number in the volume scan */

  si32 byte_width;         /* data byte width
                            * 1 = ui08, 2 = ui16, 4 = fl32 */


  si32 scan_mode;          /* use scan type defs above */
  si32 beam_is_indexed;    /* 0: not indexed, 1: indexed */
  
  si32 antenna_transition; /* 1 = antenna is in transition, 0 = not */

  si32 n_samples;          /* number of pulse (time series) samples per
                            * dwell for this data beam.
                            * Same as samples_per_beam in radar params */

  si32 spare_ints[2];

  fl32 measXmitPowerDbmH;  /* measured H power in dBm */
  fl32 measXmitPowerDbmV;  /* measured V power in dBm */

  fl32 azimuth;		   /* deg */
  fl32 elevation;	   /* deg */
  fl32 target_elev;	   /* deg - PPI mode */
  fl32 target_az;          /* deg - RHI mode */

  fl32 angular_resolution; /* deg - resolution of beam (dwell) indexing */

  fl32 spare_floats[3];
  
} DsBeamHdr_t;

/*
 * old version 1 of beam header
 */

typedef struct {
  
  si32 time;              /* secs since Jan 1 1970 */

  si32 vol_num;           /* the volume scan number */
  si32 tilt_num;	  /* the tilt number in the volume scan */

  si32 reference_time;

  si32 byte_width;        /* data byte width
			   * 1 = ui08, 2 = ui16, 4 = fl32 */

  si32 spare_ints[1];

  fl32 azimuth;		  /* deg */
  fl32 elevation;	  /* deg */
  fl32 target_elev;	  /* deg */

  fl32 spare_floats[1];
  
} DsBeamHdr_v1_t;

/*
 * flags indicating end of volume etc.
 */

typedef struct {
  
  si32 time;              /* secs since Jan 1 1970 */

  si32 vol_num;           /* the volume scan number */
  si32 tilt_num;	  /* the tilt number in the volume scan */
  si32 scan_type;	  /* the current scan strategy */
  
  si32 start_of_tilt;
  si32 end_of_tilt;

  si32 start_of_volume;
  si32 end_of_volume;

  si32 new_scan_type;

  si32 spare_ints[4];

} DsRadarFlags_t;

#define DSRADAR_ELEV_INIT 91817161

typedef struct {
  
  si32 n_elev;
  fl32 *elev_array;
  ui08 *chunk_buf;
  int chunk_len;
  int init_flag;

} DsRadarElev_t;

/************************************************************************/
/**
 * /struct ds_iwrf_packet_info
 *
 * Duplicated from IWRF structs in libs/radar
 * will be merged later. (dixon)
 *
 ************************************************************************/

typedef struct {
    
  si32 id; /**< Id for the packet type e.g. IWRF_RADAR_INFO_ID */
  si32 len_bytes; /**< length of packet structure, in bytes,
		   ** except for burst_header, status_xml and pulse_header,
                   ** in which len_bytes
                   **    = length of structure, plus following data */
  si64 seq_num;  /**< packet sequence number */
  si32 version_num; /**< version number of this structure */
  si32 radar_id; /**< id of radar - in case of multipler radars */
  si64 time_secs_utc;  /**< UTC time in secs since Jan 1 1970 */
  si32 time_nano_secs; /**< partial secs - nanosecs */
  si32 reserved[5]; /**< future expansion */

} ds_iwrf_packet_info_t;

/************************************************************************/
/**
 * \struct ds_iwrf_platform_georef
 *
 * Georeference information for moving platforms
 *
 * Duplicated from IWRF structs in libs/radar
 * will be merged later. (dixon)
 *
 ************************************************************************/

typedef struct {
    
  ds_iwrf_packet_info_t packet; /*< packet_id = IWRF_PLATFORM_GEOREF_ID */
  
  si32 unit_num;            /** number of the unit providing the data
                             *  0 indicates primary, 1 indicates secondary
                             *  set to 0 if only 1 unit is in operation
                             *  set to 0 or 1 if 2 units are in operation */

  si32 unit_id;             /** optional - used for serial number etc. of
                             *  the GPS/INS unit */

  fl32 altitude_msl_km;     /**< Antenna Altitude above mean sea
                             * level (MSL) in km */
  fl32 altitude_agl_km;     /**< Antenna Altitude above ground level
                             * (AGL) in km */
  fl32 ew_velocity_mps;     /**< Antenna east-west ground speed
                             * (towards East is positive) in m/sec */
  fl32 ns_velocity_mps;     /**< Antenna north-south ground speed
                             * (towards North is positive) in m/sec */
  fl32 vert_velocity_mps;   /**< Antenna vertical velocity (Up is
                             * positive) in m/sec */
  fl32 heading_deg;         /**< Antenna heading (angle between
                             * rotodome rotational axis and true
                             * North, clockwise (looking down)
                             * positive) in degrees */
  fl32 roll_deg;            /**< Roll angle of aircraft tail section
                             * (Horizontal zero, Positive left wing up)
                             * in degrees */
  fl32 pitch_deg;           /**< Pitch angle of rotodome (Horizontal
                             * is zero positive front up) in degrees*/
  fl32 drift_angle_deg;     /**< Antenna drift Angle. (angle between
                             * platform true velocity and heading,
                             * positive is drift more clockwise
                             * looking down) in degrees */
  fl32 rotation_angle_deg;  /**< Angle of the radar beam with
                             * respect to the airframe (zero is
                             * along vertical stabilizer, positive
                             * is clockwise) in deg */
  fl32 tilt_angle_deg;      /**< Angle of radar beam and line normal
                             * to longitudinal axis of aircraft,
                             * positive is towards nose of
                             * aircraft) in degrees */
  fl32 ew_horiz_wind_mps;   /**< east - west wind velocity at the
                             * platform (towards East is positive)
                             * in m/sec */
  fl32 ns_horiz_wind_mps;   /**< North - South wind velocity at the
                             * platform (towards North is 
                             * positive) in m/sec */
  fl32 vert_wind_mps;       /**< Vertical wind velocity at the
                             * platform (up is positive) in m/sec */
  fl32 heading_rate_dps;    /**< Heading change rate in degrees/second. */
  fl32 pitch_rate_dps;      /**< Pitch change rate in degrees/second. */
  
  fl32 drive_angle_1_deg;   /**< angle of motor drive 1 (degrees) */
  fl32 drive_angle_2_deg;   /**< angle of motor drive 2 (degrees) */
  
  fl64 longitude;           /**< Antenna longitude (Eastern
                             * Hemisphere is positive, West
                             * negative) in degrees */
  fl64 latitude;            /**< Antenna Latitude (Northern
                             * Hemisphere is positive, South
                             * Negative) in degrees */

  fl32 track_deg;           /** track over the ground */
  fl32 roll_rate_dps;       /**< roll angle rate in degrees/second. */

  fl32 unused[24]; /**< for future expansion */
  
} ds_iwrf_platform_georef_t;

/*
 * function prototypes
 */

/***********************
 * BE_to_DsRadarParams()
 *
 * Convert BE to DsRadarParams_t
 */

extern void BE_to_DsRadarParams(DsRadarParams_t *params);
     
/*************************
 * BE_from_DsRadarParams()
 *
 * Convert DsRadarParams_t to BE
 */

extern void BE_from_DsRadarParams(DsRadarParams_t *params);

/***********************
 * BE_to_DsFieldParams()
 *
 * Convert BE to DsFieldParams_t
 */

extern void BE_to_DsFieldParams(DsFieldParams_t *field);
     
/*************************
 * BE_from_DsFieldParams()
 *
 * Convert DsFieldParams_t to BE
 */

extern void BE_from_DsFieldParams(DsFieldParams_t *field);

/*******************
 * BE_to_DsBeamHdr()
 *
 * Convert BE to DsBeamHdr_t
 */

extern void BE_to_DsBeamHdr(DsBeamHdr_t *beam);

/***************************
 * BE_from_DsBeamHdr()
 *
 * Convert DsBeamHdr_t to BE
 */

extern void BE_from_DsBeamHdr(DsBeamHdr_t *beam);

/*******************
 * BE_to_DsRadarFlags()
 *
 * Convert BE to DsRadarFlags_t
 */

extern void BE_to_DsRadarFlags(DsRadarFlags_t *flags);

/***************************
 * BE_from_DsRadarFlags()
 *
 * Convert DsRadarFlags_t to BE
 */

extern void BE_from_DsRadarFlags(DsRadarFlags_t *flags);

/********************
 * DsRadarElev_init()
 *
 * Initialize radar elevation struct
 */

extern void DsRadarElev_init(DsRadarElev_t *elev);

/*********************
 * DsRadarElev_alloc()
 *
 * Alloc arrays for radar elevation struct
 */

extern void DsRadarElev_alloc(DsRadarElev_t *elev, int nelev);

/********************
 * DsRadarElev_free()
 *
 * Free arrays for radar elevation struct
 */

extern void DsRadarElev_free(DsRadarElev_t *elev);

/****************************
 * DsRadarElev_load_chunk()
 *
 * Load up chunk data
 */

extern void DsRadarElev_load_chunk(DsRadarElev_t *elev);

/****************************
 * DsRadarElev_unload_chunk()
 *
 * Unload chunk data into struct
 */

extern void DsRadarElev_unload_chunk(DsRadarElev_t *elev,
				     ui08 *chunk, int chunk_len);

/*****************
 * print routines
 */

extern void DsRadarParams_print(FILE *out, char *spacer,
				DsRadarParams_t *rparams);

extern void DsFieldParams_print(FILE *out, char *spacer,
				DsFieldParams_t *fparams);

extern void DsBeamHdr_print(FILE *out, char *spacer,
			    DsBeamHdr_t *bhdr);

extern void DsRadarFlags_print(FILE *out, char *spacer,
			       DsRadarFlags_t *flags);

extern void DsRadarElev_print(FILE *out, char *spacer,
			      DsRadarElev_t *elev);

#ifdef __cplusplus
}
#endif

#endif

