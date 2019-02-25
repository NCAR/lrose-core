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
/******************************************************************/
/**
 *
 * /file <iwrf_data.h>
 *
 * Defines for handling moments and time series radar data.
 *
 * CSU-CHILL/NCAR
 * IWRF - INTEGRATED WEATHER RADAR FACILILTY
 *
 * Mike Dixon, RAL, NCAR, Boulder, CO
 * Dec 2008
 *
 *********************************************************************/

#ifndef _IWRF_DATA_H_
#define _IWRF_DATA_H_

#ifdef WIN32                       
#pragma pack(push,4)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************/
/**
 * \mainpage
 *
 * This document describes the time series format for the IWRF project.
 * <br>
 * The format uses the following platform-independent data types:
 * 
 * \li si08: signed   1-byte integer
 * \li ui08: unsigned 1-byte integer
 * \li si16: signed   2-byte integer
 * \li ui16: unsigned 2-byte integer
 * \li si32: signed   4-byte integer
 * \li ui32: unsigned 4-byte integer
 * \li si64: signed   8-byte integer
 * \li ui64: unsigned 8-byte integer
 * \li si64: signed 8-byte integer
 * \li fl32: 4-byte floating point
 * \li fl64: 8-byte floating point
 */

#include <dataport/port_types.h>
#include <math.h>

#define IWRF_MAX_CHAN 4  /**< Max number of channels<br>Normal dual-pol operation uses 2 channels */
#define IWRF_MAX_RADAR_NAME 32  /**< Max length of radar name */
#define IWRF_MAX_SITE_NAME  32  /**< Max length of site name */
#define IWRF_MAX_SEGMENT_NAME 32  /**< Max length of scan segment name */
#define IWRF_MAX_PROJECT_NAME 32  /**< Max length of project name */
#define IWRF_MAX_FIXED_ANGLES 512  /**< Length of array of fixed angles */
#define IWRF_N_TXSAMP 512  /**< Length of transmit sample array */
#define IWRF_MAX_PHASE_SEQ_LEN 256  /**< Max length of phase coding sequence array */
#define IWRF_VERSION_NAME_LEN 64  /**< Version name length */
#define IWRF_UI_MAX_OPERATOR_NAME 32 /**< Radar Operator name */
#define IWRF_UI_MAX_TASKS_PER_TASKLIST 30 /**< Max # tasks in a tasklist */
#define IWRF_UI_MAX_ERROR_MSG 128 /**< Max error msg returned to UI */

#define IWRF_MAX_MOMENTS_FIELD_NAME 32  /**< Max length of moments field name */
#define IWRF_MAX_MOMENTS_FIELD_NAME_LONG 128  /**< Max length of moments field long name */
#define IWRF_MAX_MOMENTS_FIELD_NAME_STANDARD 128 /**< Max length of moments field standard name */
#define IWRF_MAX_MOMENTS_FIELD_UNITS 16 /**< Max length of moments field units */

/* missing value for meta-data
 * used where value is not known */

#define IWRF_MISSING_INT -9999 /**< Missing val for integer values */
#define IWRF_MISSING_FLOAT -9999.0F /**< Missing val for float values */ 
#define IWRF_MISSING_DOUBLE -9999.0 /**< Missing val for double values */ 
#ifdef NOTNOW
#define IWRF_MISSING_FLOAT NAN /**< Missing val for float values */
#endif

/* packet IDs */

#define IWRF_SYNC_ID 0x77770001 /**< ID for sync packet */
#define IWRF_RADAR_INFO_ID 0x77770002 /**< ID for radar_info packet */
#define IWRF_SCAN_SEGMENT_ID 0x77770003 /**< ID for scan segment packet */
#define IWRF_ANTENNA_CORRECTION_ID 0x77770004 /**< ID for antenna correction packet */
#define IWRF_TS_PROCESSING_ID 0x77770005 /**< ID for time-series processing packet */
#define IWRF_XMIT_POWER_ID 0x77770006 /**< ID for measured transmit power packet */
#define IWRF_XMIT_SAMPLE_ID 0x77770007 /**< ID for transmit RF sample packet */
#define IWRF_CALIBRATION_ID 0x77770008 /**< ID for calibration packet */
#define IWRF_EVENT_NOTICE_ID 0x77770009 /**< ID for event notice packet */
#define IWRF_PHASECODE_ID 0x7777000a /**< ID for phase code packet */
#define IWRF_XMIT_INFO_ID 0x7777000b /**< ID for transmit infopacket */
#define IWRF_PULSE_HEADER_ID 0x7777000c /**< ID for pulse header packet */
#define IWRF_VERSION_ID 0x7777000d /**< ID for version packet */
#define IWRF_UI_OPERATIONS_ID 0x7777000e /**< ID for UI operation request packet */
#define IWRF_ANT_CONTROL_CONSTANTS_ID 0x7777000f /**< ID for antenna control constant packet */
#define IWRF_XMIT_SAMPLE_V2_ID 0x77770010 /**< ID for transmit sample version 2 packet */
#define IWRF_BURST_HEADER_ID 0x77770011 /**< ID for burst IQ data */
#define IWRF_STATUS_XML_ID 0x77770012 /**< ID for status in XML format */
#define IWRF_ANTENNA_ANGLES_ID 0x77770013 /**< ID for antenna angles */
#define IWRF_RX_POWER_ID 0x77770014 /**< ID for measured received power packet */

#define IWRF_RVP8_OPS_INFO_ID 0x77770070 /**< ID for RVP8 operations info packet */
#define IWRF_RVP8_PULSE_HEADER_ID 0x77770071 /**< ID for RVP8 pulse header packet */

#define IWRF_MOMENTS_FIELD_HEADER_ID 0x77770101 /**< ID for moments field header */
#define IWRF_MOMENTS_RAY_HEADER_ID 0x77770102 /**< ID for moments field header */
#define IWRF_MOMENTS_FIELD_INDEX_ID 0x77770103 /**< ID for moments field index structs */

#define IWRF_PLATFORM_GEOREF_ID 0x77770111 /**< ID for moving platform georeference */
#define IWRF_GEOREF_CORRECTION_ID 0x77770112 /**< ID for moving platform georeference */

#define IWRF_SYNC_VAL_00 0x2a2a2a2a /**< Value for first word in sync packet */
#define IWRF_SYNC_VAL_01 0x7e7e7e7e /**< Value for second word in sync packet */

/* event flags */

typedef si32 iwrf_event_flags_t;
#define IWRF_END_OF_SWEEP (1 << 0)
#define IWRF_END_OF_VOLUME (1 << 1)
#define IWRF_START_OF_SWEEP (1 << 2)
#define IWRF_START_OF_VOLUME (1 << 3)

/* txrx state flags
 *
 * These indicate the state of the transmitter for the current pulse
 *
 * e.g. If IWRF_SHORT_PRT is set, then the current prt mode is short
 */

typedef si32 iwrf_txrx_state_t;
#define IWRF_TXRX_LONG_PRT (1 << 0)
#define IWRF_TXRX_SHORT_PRT (1 << 1)
#define IWRF_TXRX_HPOL_BYPASS (1 << 2)

/************************************************************************/
/************************************************************************/
/************************************************************************/
/* ENUMERATED TYPES */
/************************************************************************/
/************************************************************************/
/************************************************************************/

typedef enum {
  IWRF_DEBUG_OFF = 0,
  IWRF_DEBUG_NORM,
  IWRF_DEBUG_VERBOSE,
  IWRF_DEBUG_EXTRA
} IwrfDebug_t;

/************************************************************************/
/**
 * \enum iwrf_xmit_rcv_mode
 *
 * Transmit and receive modes.
 *
 ************************************************************************/

typedef enum iwrf_xmit_rcv_mode {
  
  IWRF_XMIT_RCV_MODE_NOT_SET = 0,

  /** single polarization - H  */

  IWRF_SINGLE_POL = 1,

  /** Dual pol, alternating transmission,
   * copolar receiver only (CP2 SBand) */

  IWRF_ALT_HV_CO_ONLY = 2,

  /** Dual pol, alternating transmission, co-polar and cross-polar
   * receivers (SPOL with Mitch Switch and receiver in 
   * switching mode, CHILL) */

  IWRF_ALT_HV_CO_CROSS = 3,
  
  /** Dual pol, alternating transmission, fixed H and V receivers (SPOL
   * with Mitch Switch and receive7rs in fixed mode) */
  
  IWRF_ALT_HV_FIXED_HV = 4,
  
  /** Dual pol, simultaneous transmission, fixed H and V receivers (NEXRAD
   * upgrade, SPOL with T and receivers in fixed mode) */

  IWRF_SIM_HV_FIXED_HV = 5,
  
  /** Dual pol, simultaneous transmission, switching H and V receivers
   * (SPOL with T and receivers in switching mode) */
  
  IWRF_SIM_HV_SWITCHED_HV = 6,

  /** Dual pol, H transmission, fixed H and V receivers (CP2 X band) */

  IWRF_H_ONLY_FIXED_HV = 7,

  /** Dual pol, V transmission, fixed H and V receivers */
  
  IWRF_V_ONLY_FIXED_HV = 8,

  /** Dual pol, alternating transmission, pulsing HHVV sequence,
   * fixed receiver chain (HCR) */

  IWRF_ALT_HHVV_FIXED_HV = 9,
  
  /** single polarization - V  */

  IWRF_SINGLE_POL_V = 10,

  /** not used */
  
  IWRF_XMIT_RCV_MODE_LAST

} iwrf_xmit_rcv_mode_t;

/************************************************************************/
/**
 * \enum iwrf_xmit_phase_mode
 *
 * Transmitter waveform.
 *
 ************************************************************************/

typedef enum iwrf_xmit_phase_mode {

  IWRF_XMIT_PHASE_MODE_NOT_SET = 0,
  IWRF_XMIT_PHASE_MODE_FIXED = 1,  /**< Klystron */
  IWRF_XMIT_PHASE_MODE_RANDOM = 2, /**< Magnetron */
  IWRF_XMIT_PHASE_MODE_SZ864 = 3,  /**< Sachinanda-Zrnic 8/64 phase code */
  IWRF_XMIT_PHASE_MODE_LAST /**< not used */

} iwrf_xmit_phase_mode_t;

/************************************************************************/
/**
 * \enum iwrf_prf_mode
 *
 * PRF mode - fixed or dual-prt.
 *
 ************************************************************************/

typedef enum iwrf_prf_mode {

  IWRF_PRF_MODE_NOT_SET = 0,
  IWRF_PRF_MODE_FIXED = 1, /**< fixed pulsing mode */
  IWRF_PRF_MODE_STAGGERED_2_3 = 2, /**< staggered PRT, 2/3 ratio */
  IWRF_PRF_MODE_STAGGERED_3_4 = 3, /**< staggered PRT, 3/4 ratio */
  IWRF_PRF_MODE_STAGGERED_4_5 = 4, /**< staggered PRT, 4/5 ratio */
  IWRF_PRF_MODE_MULTI_PRT = 5,     /**< staggered mode, more than 2 PRT's */
  IWRF_PRF_MODE_BLOCK_MODE = 6,    /**< multiple prts, blocked mode */
  IWRF_PRF_MODE_LAST /**< not used */

} iwrf_prf_mode_t;

/************************************************************************/
/**
 * \enum iwrf_pulse_type
 *
 ************************************************************************/

typedef enum iwrf_pulse_type {

  IWRF_PULSE_TYPE_NOT_SET = 0,
  IWRF_PULSE_TYPE_RECT = 1, /**< rectangular pulse */
  IWRF_PULSE_TYPE_GAUSSIAN = 2, /**< gaussian-weighted pulse */
  IWRF_PULSE_TYPE_CUSTOM = 3, /**< custom- downloaded to waveform generator */
  IWRF_PULSE_TYPE_LAST /**< not used */
  
} iwrf_pulse_type_t;

/************************************************************************/
/**
 * \enum iwrf_pol_mode - polarization
 *
 ************************************************************************/

typedef enum iwrf_pol_mode {
  
  IWRF_POL_MODE_NOT_SET = 0,
  IWRF_POL_MODE_H = 1, /**< H pulse */
  IWRF_POL_MODE_V = 2, /**< V pulse */
  IWRF_POL_MODE_HV_ALT = 3, /**< Fast alternating */
  IWRF_POL_MODE_HV_SIM = 4, /**< Simultaneous Slant-45 */
  IWRF_POL_MODE_HHVV_ALT = 5, /**< Fast alternating, double pulsing */
  IWRF_POL_MODE_LAST /**< not used */
  
} iwrf_pol_mode_t;

/************************************************************************/
/**
 * \enum iwrf_scan_mode
 *
 * Antenna scanning mode - these are legacy NCAR codes, with the CHILL
 * FIXED, MANPPI and MANRHI appended
 *
 ************************************************************************/

typedef enum iwrf_scan_mode {

  IWRF_SCAN_MODE_NOT_SET = 0,
  IWRF_SCAN_MODE_SECTOR = 1, /**< sector scan mode */
  IWRF_SCAN_MODE_COPLANE = 2, /**< co-plane dual doppler mode */
  IWRF_SCAN_MODE_RHI = 3, /**< range height vertical scanning mode */
  IWRF_SCAN_MODE_VERTICAL_POINTING = 4, /**< vertical pointing for calibration */
  IWRF_SCAN_MODE_IDLE = 7, /**< between scans */
  IWRF_SCAN_MODE_AZ_SUR_360 = 8, /**< 360-degree azimuth mode - surveillance */
  IWRF_SCAN_MODE_EL_SUR_360 = 9, /**< 360-degree elevation mode - eg Eldora */
  IWRF_SCAN_MODE_SUNSCAN = 11, /**< scanning the sun for calibrations */
  IWRF_SCAN_MODE_POINTING = 12, /**< fixed pointing */
  IWRF_SCAN_MODE_FOLLOW_VEHICLE = 13, /**< follow target vehicle */
  IWRF_SCAN_MODE_EL_SURV = 14, /**< elevation surveillance (ELDORA) */
  IWRF_SCAN_MODE_MANPPI = 15, /**< Manual PPI mode (elevation does
			       * not step automatically) */
  IWRF_SCAN_MODE_MANRHI = 16, /**< Manual RHI mode (azimuth does
			       * not step automatically) */
  IWRF_SCAN_MODE_SUNSCAN_RHI = 17, /**< sunscan in RHI mode */
  IWRF_SCAN_MODE_LAST /**< not used */

} iwrf_scan_mode_t;

/************************************************************************/
/**
 * \enum iwrf_follow_mode
 *
 * Is the radar scan following an object?
 *
 ************************************************************************/

typedef enum iwrf_follow_mode {

  IWRF_FOLLOW_MODE_NOT_SET = 0,
  IWRF_FOLLOW_MODE_NONE = 1, /**< Radar is not tracking any object */
  IWRF_FOLLOW_MODE_SUN = 2, /**< Radar is tracking the sun */
  IWRF_FOLLOW_MODE_VEHICLE = 3, /**< Radar is tracking a vehicle */
  IWRF_FOLLOW_MODE_AIRCRAFT = 4, /**< Radar is tracking an aircraft */
  IWRF_FOLLOW_MODE_TARGET = 5, /**< Radar is tracking a target - e.g. sphere */
  IWRF_FOLLOW_MODE_MANUAL = 6, /**< Radar is under manual tracking mode */
  IWRF_FOLLOW_MODE_LAST /**< not used */

} iwrf_follow_mode_t;

/************************************************************************/
/**
 * \enum iwrf_radar_platform
 *
 * The type of platform on which the radar is mounted.
 *
 ************************************************************************/

typedef enum iwrf_radar_platform {

  IWRF_RADAR_PLATFORM_NOT_SET = 0,
  IWRF_RADAR_PLATFORM_FIXED = 1, /**< Radar is in a fixed location */
  IWRF_RADAR_PLATFORM_VEHICLE = 2, /**< Radar is mounted on a land vehicle */
  IWRF_RADAR_PLATFORM_SHIP = 3, /**< Radar is mounted on a ship */
  IWRF_RADAR_PLATFORM_AIRCRAFT = 4, /**< Radar is mounted on an aircraft */
  IWRF_RADAR_PLATFORM_LAST /**< not used */

} iwrf_radar_platform_t;

/************************************************************************/
/**
 * \enum iwrf_cal_type
 *
 * Calibration type.
 *
 ************************************************************************/

typedef enum iwrf_cal_type {
  
  IWRF_CAL_TYPE_NOT_SET = 0,
  IWRF_CAL_TYPE_NONE = 1, /**< No calibration is currently taking place */
  IWRF_CAL_TYPE_CW_CAL = 2, /**< CW calibration */
  IWRF_CAL_TYPE_SOLAR_CAL_FIXED = 3, /**< Fixed sun-pointing */
  IWRF_CAL_TYPE_SOLAR_CAL_SCAN = 4, /**< Scanning across face of the sun */
  IWRF_CAL_TYPE_NOISE_SOURCE_H = 5, /**< Noise source connected to H chan */
  IWRF_CAL_TYPE_NOISE_SOURCE_V = 6, /**< Noise source connected to V chan */
  IWRF_CAL_TYPE_NOISE_SOURCE_HV = 7, /**< Noise source connected to both chans */
  IWRF_CAL_TYPE_BLUESKY = 8,    /**< Antenna is pointing at blue-sky */
  IWRF_CAL_TYPE_SAVEPARAMS = 9, /**< Not a real test mode,
				 *   indicates to compute nodes to
				 *   save calibration params */
  IWRF_CAL_TYPE_LAST /**< not used */

} iwrf_cal_type_t;

/************************************************************************/
/**
 * \enum iwrf_event_cause
 * Cause of particular events
 *
 ************************************************************************/

typedef enum iwrf_event_cause {

  IWRF_EVENT_CAUSE_NOT_SET = 0,
  IWRF_EVENT_CAUSE_DONE = 1, /**< Scan completed normally */
  IWRF_EVENT_CAUSE_TIMEOUT = 2, /**< Scan has timed out */
  IWRF_EVENT_CAUSE_TIMER = 3, /**< Timer caused this scan to abort */
  IWRF_EVENT_CAUSE_ABORT = 4, /**< Operator issued an abort */
  IWRF_EVENT_CAUSE_SCAN_ABORT = 5, /**< Scan Controller detected error */
  IWRF_EVENT_CAUSE_RESTART = 6, /**< communication fault was recovered,
                                 *   restarting scan */
  IWRF_EVENT_CAUSE_SCAN_STATE_TIMEOUT = 7, /**< Scan Controller state machine timeout */
  IWRF_EVENT_CAUSE_ANTENNA_FAULT = 8, /**< Antenna controller fault */
  IWRF_EVENT_CAUSE_LAST /**< not used */
  
} iwrf_event_cause_t;

/************************************************************************/
/**
 * \enum iwrf_iq_encoding
 *
 * How the IQ data is encoded and packed.
 *
 ************************************************************************/

typedef enum iwrf_iq_encoding {

  IWRF_IQ_ENCODING_NOT_SET = 0,

  IWRF_IQ_ENCODING_FL32 = 1, /**< 4-byte floats, volts */

  IWRF_IQ_ENCODING_SCALED_SI16 = 2, /**< voltages scaled as signed 16-bit ints,<br>
				     * volts = (si16 * scale) + offset */
  
  IWRF_IQ_ENCODING_DBM_PHASE_SI16 = 3, /**< 16-bit signed ints,
					* storing power_dbm and phase,<br>
					* power_dbm = (si16 * scale) + offset<br>
					* phase = (si16 * 360.0) / 65536.0  */
  
  IWRF_IQ_ENCODING_SIGMET_FL16 = 4,  /**< SIGMET 16-bit floats */

  IWRF_IQ_ENCODING_SCALED_SI32 = 5, /**< voltages scaled as signed 32-bit ints,<br>
				     * volts = (si32 * scale) + offset */
  
  IWRF_IQ_ENCODING_LAST /**< not used */

} iwrf_iq_encoding_t;

/************************************************************************/
/**
 * \enum iwrf_moments_encoding
 *
 * How the moments data is encoded and packed in a field.
 *
 ************************************************************************/

typedef enum iwrf_moments_encoding {

  IWRF_MOMENTS_ENCODING_NOT_SET = 0,

  IWRF_MOMENTS_ENCODING_FL64 = 1, /**< 8-byte floats */
  
  IWRF_MOMENTS_ENCODING_FL32 = 2, /**< 4-byte floats */
  
  IWRF_MOMENTS_ENCODING_SCALED_SI32 = 3, /**< val = (si32 * scale) + offset */
  
  IWRF_MOMENTS_ENCODING_SCALED_SI16 = 4, /**< val = (si16 * scale) + offset */
  
  IWRF_MOMENTS_ENCODING_SCALED_SI08 = 5, /**< val = (si08 * scale) + offset */
  
  IWRF_MOMENTS_ENCODING_LAST /**< not used */

} iwrf_moments_encoding_t;

/************************************************************************/
/**
 * \enum iwrf_sample_units
 * 
 * Used by xmit_sample_v2
 *
 ************************************************************************/

typedef enum iwrf_sample_units {

  IWRF_SAMPLE_UNITS_COUNTS = 0,
  IWRF_SAMPLE_UNITS_VOLTS = 1

} iwrf_sample_units_t;

/************************************************************************/
/************************************************************************/
/************************************************************************/
/* STRUCT TYPES */
/* PACKET DEFINITIONS */
/************************************************************************/
/************************************************************************/
/************************************************************************/

/************************************************************************/
/**
 * \struct iwrf_packet_info
 *
 * Struct included at top of all other structs.
 * packet_id is set to the id relevant to that packet type.
 *
 ************************************************************************/

typedef struct iwrf_packet_info {
    
  si32 id; /**< Id for the packet type e.g. IWRF_RADAR_INFO_ID */
  si32 len_bytes; /**< length of packet structure, in bytes,
		   ** except for burst_header, status_xml and pulse_header,
                   ** in which len_bytes
                   **    = length of structure, plus following data */
  si64 seq_num;  /**< packet sequence number */
  si32 version_num; /**< version number of this structure */
  si32 radar_id; /**< id of radar - in case of multiple radars */
  si64 time_secs_utc;  /**< UTC time in secs since Jan 1 1970 */
  si32 time_nano_secs; /**< partial secs - nanosecs */
  si32 reserved[5]; /**< future expansion */

} iwrf_packet_info_t;

/************************************************************************/
/**
 * \struct iwrf_sync
 *
 * Synchronization packet.
 * Sent regularly to resynchronize data stream if needed.
 *
 * The magik[2] array will contain the following:
 *
 *   <br>IWRF_SYNC_VAL_00  = 0x2a2a2a2a		ascii: ****
 *   <br>IWRF_SYNC_VAL_01  = 0x7e7e7e7e		ascii: ~~~~
 *
 ************************************************************************/

typedef struct iwrf_sync {
    
  iwrf_packet_info_t packet; /*< packet_id = IWRF_SYNC_ID */

  si32 magik[2]; /**< array of sync values */
  
} iwrf_sync_t;

/************************************************************************/
/**
 * \struct iwrf_version
 *
 * Version packet.
 *
 ************************************************************************/

typedef struct iwrf_version {
    
  iwrf_packet_info_t packet; /*< packet_id = IWRF_VERSION_ID */

  si32 major_version_num;
  si32 minor_version_num;

  char version_name[IWRF_VERSION_NAME_LEN];
  
} iwrf_version_t;

/************************************************************************/
/**
 * \struct iwrf_radar_info
 *
 * Fixed radar-specific information.
 *
 * NOTE: calibration information is in calibration.
 *
 ************************************************************************/

typedef struct iwrf_radar_info {
    
  iwrf_packet_info_t packet; /*< packet_id = IWRF_RADAR_INFO_ID */
  
  fl32 latitude_deg;  /**< latitude, in degrees */
  fl32 longitude_deg; /**< longitude, in degrees */
  fl32 altitude_m;    /**< altitude, in meters */
  si32 platform_type; /**< platform type - \ref iwrf_radar_platform */

  fl32 beamwidth_deg_h; /**< Antenna beamwidth, horizontal, in degrees */
  fl32 beamwidth_deg_v; /**< Antenna beamwidth, vertical, in degrees */
  fl32 wavelength_cm;   /**< Radar wavelength, in centimeters */
  
  fl32 nominal_gain_ant_db_h; /**< nominal antenna gain, H */
  fl32 nominal_gain_ant_db_v; /**< nominal antenna gain, V */

  fl32 unused[25]; /**< for future expansion */
  
  char radar_name[IWRF_MAX_RADAR_NAME]; /**< UTF-8 encoded radar name */
  char site_name[IWRF_MAX_SITE_NAME];   /**< UTF-8 encoded radar name */

} iwrf_radar_info_t;

/************************************************************************/
/**
 * \struct iwrf_scan_segment
 *
 * Scanning strategy - scan segment or volume
 *
 ************************************************************************/

typedef struct iwrf_scan_segment

{

  iwrf_packet_info_t packet; /*< packet_id = IWRF_SCAN_SEGMENT_ID */
  
  si32 scan_mode; /**< The current scan mode - \ref iwrf_scan_mode */
  si32 follow_mode; /**< The current follow mode - \ref iwrf_follow_mode */
  si32 volume_num; /**< Volume number, increments linearly */
  si32 sweep_num; /**< Sweep number within the current volume */
  si32 time_limit; /**< Timeout for this scan, in seconds, if nonzero */

  fl32 az_manual; /**< Manual azimuth position.
                   *   Used in pointing and Manual PPI/RHI modes */
  fl32 el_manual; /**< Manual elevation position.
                   *   Used in pointing and Manual PPI/RHI modes */
  fl32 az_start; /**< Azimuth start. If>360 implies don't care */
  fl32 el_start; /**< Elevation start. If>360 implies don't care */
  fl32 scan_rate; /**< Antenna scan rate, in degrees/sec */
  fl32 left_limit; /**< Left limit used in sector scan */
  fl32 right_limit; /**< Right limit used in sector scan */
  fl32 up_limit; /**< Upper limit used in sector scan */
  fl32 down_limit; /**< Lower limit used in sector scan */
  fl32 step; /**< Antenna step, used to increment az (in RHI)
              ** or el (in PPI) if max_sweeps is zero */

  fl32 current_fixed_angle; /**< Current fixed angle
                             *   (az in RHI, el in PPI) */
  si32 init_direction_cw; /**< Initial sector direction clockwise
                           *   TRUE/FALSE */
  si32 init_direction_up; /**< Initial rhi direction up - TRUE/FALSE */

  si32 n_sweeps;  /**< Indicates that a set of discrete angles
                   *   is specified for az or el */
  fl32 fixed_angles[IWRF_MAX_FIXED_ANGLES];
  
  /** Scan Optimizer parameters */

  fl32 optimizer_rmax_km;
  fl32 optimizer_htmax_km;
  fl32 optimizer_res_m;
  
  /** Sun Scan sector widths */

  fl32 sun_scan_sector_width_az; /**< sector width (deg) when follwing sun */
  fl32 sun_scan_sector_width_el; /**< sector width (deg) when follwing sun */

  si32 timeseries_archive_enable; /**< allow timeseries archiving for this scan */
  si32 waveguide_switch_position; /**< 0 -> normal, 1->alternate transmitter */
  si32 start_ppi_ccw; /**< if true, make first sweep of ppi scan be ccw */
  
  fl32 unused[455]; /**< for future expansion */
  
  char segment_name[IWRF_MAX_SEGMENT_NAME]; /**< Name of this scan segment */
  char project_name[IWRF_MAX_PROJECT_NAME]; /**< Project name */

} iwrf_scan_segment_t;

/************************************************************************/
/**
 * \struct iwrf_antenna_correction.
 *
 * For correcting antenna angles, as required.
 * These angles must be *added* to the az and el to obtain
 * the corrected version as follows:
 *   corrected_angle = raw_angle + correction
 * In the CHILL processor, these corrections have been added to the 
 * angles provided TimeSeries server.
 *
 ************************************************************************/

typedef struct iwrf_antenna_correction {

  iwrf_packet_info_t packet; /*< packet_id = IWRF_ANTENNA_CORRECTION_ID */
  
  fl32 az_correction; /**< Azimuth offset in 16-bit angle units */
  fl32 el_correction; /**< Elevation offset in 16-bit angle units */
  
  fl32 unused[16]; /**< for future expansion */
  
} iwrf_antenna_correction_t;
  
/************************************************************************/
/**
 * \struct iwrf_ts_processing
 *
 * Time-series-specific processing.
 *
 * This packet contains the transmit modes, digitization in range and
 * hints about how to compute moments from the time series.
 *
 ************************************************************************/

typedef struct iwrf_ts_processing {

  iwrf_packet_info_t packet; /*< packet_id = IWRF_TS_PROCESSING_ID */
  
  si32 xmit_rcv_mode;   /**< current transmit/receive mode -
                         * \ref iwrf_xmit_rcv_mode */
  si32 xmit_phase_mode; /**< current transmit phase mode -
                         * \ref iwrf_xmit_phase_mode */
  si32 prf_mode;        /**< current PRF mode - \ref iwrf_prf_mode */
  si32 pulse_type;      /**< current pulse type - \ref iwrf_pulse_type */

  fl32 prt_usec;  /**< PRT in microseconds */
  fl32 prt2_usec; /**< PRT in microseconds used in dual PRT/PRF modes */

  si32 cal_type; /**< calibration type - \ref iwrf_cal_type */
  
  fl32 burst_range_offset_m; /**< Range offset to the gate at which the 
                              * transmitter fires, (in meters)
                              * If positive, this means that the digitizer
                              * starts sampling before the transmitter
                              * fires */

  fl32 pulse_width_us;   /**< pulse width in microsecs */

  fl32 start_range_m;  /**< Range to start of first gate (m)
                        * This is the negative of burst_burst_range_offset_m */

  fl32 gate_spacing_m; /**< Gate spacing (in meters) */

  si32 integration_cycle_pulses; /**< Requested number of hits for a ray */
  si32 clutter_filter_number; /**< Clutter filter number to be used */
  si32 range_gate_averaging; /**< Number of range gates to average */
  si32 max_gate;      /**< Number of gates for digitizer to acquire */

  fl32 test_power_dbm;  /**< Power at signal generator output in dBm,
                         * when test set is commanded to output 0dBm */
  fl32 test_pulse_range_km;  /**< Range at which test pulse is located */
  fl32 test_pulse_length_usec;  /**< Length of test pulse */

  si32 pol_mode;  /**< current polarization mode - \ref iwrf_pol_mode */ 

  si32 xmit_flag[2]; /**< true to enable transmitters,
                      * for each transmitter  (H is [0], V is [1])
                      * bit mask values: 1 = wavelength 1 enabled,
                      *                  0 = disabled */
  si32 beams_are_indexed; /**< 1 = indexed,  0 = not indexed */
  si32 specify_dwell_width; /**< 1 =  use indexed_beam_width_deg,
                             *   0 = use integration_cycle_pulses */
  fl32 indexed_beam_width_deg; /**< width if beam in indexed beam mode */
  fl32 indexed_beam_spacing_deg; /**< angular spacing of center of beams */

  si32 num_prts;  /**< number of prt's used, allowed values 1 through 4 */
  fl32 prt3_usec;  /**< optional third PRT in microseconds used in SMPRF modes  */
  fl32 prt4_usec;  /**< optional fourth PRT in microseconds used in SMPRF modes */
  
  /* in multiple prt, block mode, the sequence would be:
     do "integration_cycle_pulses" pulses at prt: prt_usec
     then do "block_mode_prt2_pulses" pulses at prt: prt2_usec
     then do "block_mode_prt3_pulses" pulses at prt: prt3_usec
     then do "block_mode_prt4_pulses" pulses at prt: prt4_usec */

  si32 block_mode_prt2_pulses;
  si32 block_mode_prt3_pulses;
  si32 block_mode_prt4_pulses;
  
  ui32 pol_sync_mode; /**< synch required with UI to set polarization mode */
  fl32 unused[18]; /**< for future expansion */
  
} iwrf_ts_processing_t;

/*
 * For example:
 *  
 *      beam       beam      beam     beam
 *     center     center    center   center
 *       I         I         I         I         indexed_beam_spacing (1.0)
 *  WWWWWWWWWWWWW       WWWWWWWWWWWWW            indexed_beam_width_deg (1.3)
 *            WWWWWWWWWWWWW      WWWWWWWWWWWWW
 *             
 *  Each  WWWWWWWWWWWWW represents an overlapping beam
 * 
 * If 'specify_dwell_width' is 1, indexed_beam_width_deg takes
 * precedence over integration_cycle_pulses.  Each beam will always be
 * indexed_beam_width_deg wide, centered on the beams centers,
 * regardless of the actual antenna speed.
 *
 * If 'specify_dwell_width' is 0, each beam consists of
 * integration_cycle_pulses, independent of the actual beam width
 * (which might vary, depending on antenna speed)
 */

/************************************************************************/
/**
 * \struct iwrf_xmit_power
 *
 * Measured transmitter power
 *
 ************************************************************************/

typedef struct iwrf_xmit_power {

  iwrf_packet_info_t packet; /*< packet_id = IWRF_XMIT_POWER_ID */
  
  fl32 power_dbm_h; /**< Peak power in dBm assuming a rect pulse - H.
		     * We use a simple conversion from average power readings to
		     * peak power.\n
		     * \code peak_power = average_power * (pulse_len/PRT) \endcode */
  
  fl32 power_dbm_v; /**< Peak power in dBm assuming a rect pulse - V.
		     * We use a simple conversion from average power readings to
		     * peak power.\n
		     * \code peak_power = average_power * (pulse_len/PRT) \endcode */
  
  si32 unused[16]; /**< for future expansion */

} iwrf_xmit_power_t;

/************************************************************************/
/**
 * \struct iwrf_rx_power
 *
 * Measured received power
 *
 ************************************************************************/

typedef struct iwrf_rx_power {

  iwrf_packet_info_t packet; /*< packet_id = IWRF_RX_POWER_ID */
  
  fl32 max_power_dbm_hc; /**< Peak power at any gate, in dBm, in HC channel. */
  fl32 max_power_dbm_vc; /**< Peak power at any gate, in dBm, in VC channel. */
  fl32 max_power_dbm_hx; /**< Peak power at any gate, in dBm, in HX channel. */
  fl32 max_power_dbm_vx; /**< Peak power at any gate, in dBm, in VX channel. */
  
  si32 unused[14]; /**< for future expansion */

} iwrf_rx_power_t;

/************************************************************************/
/**
 * \struct iwrf_xmit_sample
 *
 * Measured transmit sample - gives the shape of the burst
 * This is raw samples, not IQ data.
 *
 ************************************************************************/

typedef struct iwrf_xmit_sample {
  
  iwrf_packet_info_t packet; /*< packet_id = IWRF_XMIT_SAMPLE_ID */
  
  fl32 power_dbm_h; /**< H peak power in dBm assuming a rect pulse */
  fl32 power_dbm_v; /**< V peak power in dBm assuming a rect pulse */
  si32 offset; /**< Offset from trigger pulse of these samples */
  si32 n_samples; /**< Number of valid samples */

  fl32 sampling_freq; /**< sampling frequency for transmit pulse - Hz */

  fl32 scale_h;   /**< volts = (sample * scale) + offset - H */
  fl32 offset_h;  /**< volts = (sample * scale) + offset - H */

  fl32 scale_v;   /**< volts = (sample * scale) + offset - V */
  fl32 offset_v;  /**< volts = (sample * scale) + offset - V */

  si32 samples_h[IWRF_N_TXSAMP]; /**< Sample of H xmit, in counts,
                                  * at sampling frequency  */
  si32 samples_v[IWRF_N_TXSAMP]; /**< Sample of V xmit, in counts,
                                  * at sampling frequency */

  si32 unused[1001]; /**< for future expansion */

} iwrf_xmit_sample_t;

/************************************************************************/
/**
 * \struct iwrf_xmit_sample_v2
 *
 * Measured transmit sample at RF - alternative version
 *
 * The tranmit samples follow this structure.
 * The samples are stored as fl32s.
 * They are packed as:
 *   nsamples for channel 0
 *   nsamples for channel 1
 *   .....
 *   nsamples for channel nchannels-1
 *
 ************************************************************************/

typedef struct iwrf_xmit_sample_v2 {
  
  iwrf_packet_info_t packet; /*< packet_id = IWRF_XMIT_SAMPLE_ID */
  
  si32 n_samples; /**< Number of samples */
  si32 n_channels; /**< Number of channels */
  
  si32 unused[6]; /**< for future expansion */

} iwrf_xmit_sample_v2_t;

/************************************************************************/
/**
 * \struct iwrf_burst_header
 *
 * Burst sample IQ data and characteristics.
 *
 * This struct is followed by n_samples IQ pairs,
 * i.e. n_samples * 2 values. The values are either
 * 16-bit integers or 32-bit floats, depending on the
 * 'iq_encoding' parameter. See IWRF_IQ_ENCODING.
 *
 * This struct should be sent BEFORE the pulse data, so that the
 * burst information is up-to-date when the pulse arrives.
 *
 ************************************************************************/

typedef struct iwrf_burst_header {
  
  iwrf_packet_info_t packet; /*< packet_id = IWRF_BURST_HEADER_ID */
  
  si64 pulse_seq_num;  /**< pulse sequence number - increments */

  si32 n_samples; /**< Number of samples */
  si32 channel_id; /**< Channel number for this sample */

  si32 iq_encoding; /**< IWRF_IQ_ENCODING  - how the IQ data is packed */

  fl32 scale; /**< for use with IWRF_IQ_ENCODING types
               * to convert to/from floats */
  fl32 offset; /**< for use with IWRF_IQ_ENCODING types
                * to convert to/from floats */

  fl32 power_dbm; /**< mean power in dBm (assuming a rect pulse) */
  fl32 phase_deg; /**< burst phase - degrees */
  fl32 freq_hz; /**< burst frequency - hz */
  fl32 sampling_freq_hz; /**< sampling frequency for burst - hz */

  fl32 power_max_dbm; /**< power in dBm */
  fl32 power_p90_dbm; /**< 90th percentile of power in dBm */
  fl32 pulse_width_sec; /**< estimated pulse width in sec */

  si32 unused[4]; /**< for future expansion */

} iwrf_burst_header_t;

/************************************************************************/
/**
 * \struct iwrf_status_xml
 *
 * Status as ASCII in XML format.
 *
 * This struct is followed by an ASCII buffer, length xml_len.
 * The ASCII XML buffer should be null-terminated.
 * The contents are free-form. Any suitable status information
 * may be included in the XML buffer.
 *
 * packet.len_bytes = sizeof(iwrf_packet_info_t) + xml_len.
 *
 ************************************************************************/

typedef struct iwrf_status_xml {
  
  iwrf_packet_info_t packet; /*< packet_id = IWRF_STATUS_XML */
  
  si32 xml_len; /**< Length of XML buffer in bytes
                 * This should include the trailing NULL */
  si32 unused[17]; /**< for future expansion */

} iwrf_status_xml_t;

/************************************************************************/
/**
 * \struct iwrf_antenna_angles
 *
 * Short packet with antenna angles.
 * Not normally part of the data stream.
 * Used for miscellaneous purposes, such as monitoring.
 *
 ************************************************************************/

typedef struct iwrf_antenna_angles {
  
  iwrf_packet_info_t packet; /*< packet_id = IWRF_ANTENNA_ANGLES_ID *
                              * len_bytes = length of this structure,
                              * plus following data, in bytes */
  
  si32 scan_mode; /**< The current scan mode - \ref iwrf_scan_mode */
  si32 follow_mode; /**< The current follow mode - \ref iwrf_follow_mode */
  si32 volume_num;  /**< scan vol  num - if avail, otherwise set to -1 */ 
  si32 sweep_num;   /**< scan tilt num - if avail, otherwise set to -1 */ 

  fl32 fixed_el;  /**< target elevation for ppis */
  fl32 fixed_az;  /**< target azimuth for rhis */
  fl32 elevation;    /**< antenna elevation */
  fl32 azimuth;      /**< antenna azimuth */
  
  si32 antenna_transition; /**< antenna is in transition
                            * 1 = TRUE, 0 = FALSE */
  
  si32 status; /**< general status flag - optional use */

  si32 event_flags; /**< flags from end of volume, start of tilt etc.
                     * See iwrf_event_flags_t */

  si32 unused[7];

} iwrf_antenna_angles_t;

/************************************************************************/
/**
 * \struct iwrf_calibration
 *
 * Calibration information
 *
 * Polarization terminology is receiver-centric:
 *
 *  \li hc - horizontal rx co-polar    = receive H, transmit H
 *  \li hx - horizontal rx cross-polar = receive H, transmit V
 *  \li vc - vertical   rx co-polar    = receive V, transmit V
 *  \li vx - vertical   rx cross-polar = receive V, transmit H
 *
 ************************************************************************/

typedef struct iwrf_calibration {
  
  iwrf_packet_info_t packet; /*< packet_id = IWRF_CALIBRATION_ID */
  
  fl32 wavelength_cm;  /**< radar wavelength in cm */
  fl32 beamwidth_deg_h; /**< Antenna beamwidth, horizontal, in degrees */
  fl32 beamwidth_deg_v; /**< Antenna beamwidth, vertical, in degrees */
  fl32 gain_ant_db_h; /**< computed antenna gain, H pol,
                       *   from ref point through antenna */
  fl32 gain_ant_db_v; /**< computed antenna gain, V pol,
                       *   from ref point through antenna */

  fl32 pulse_width_us;   /**< pulse width in microsecs */
  fl32 xmit_power_dbm_h;  /**< peak transmit H power in dBm */
  fl32 xmit_power_dbm_v;  /**< peak transmit V power in dBm */
  
  /** 2-way waveguide loss from feedhorn to measurement plane - H.
   * Set to 0 is the loss is incorporated into the antenna gain */
  
  fl32 two_way_waveguide_loss_db_h;

  /** 2-way waveguide loss from feedhorn to measurement plane - V.
   * Set to 0 is the loss is incorporated into the antenna gain */
  
  fl32 two_way_waveguide_loss_db_v;
  
  /** 2-way Radome loss (dB) - H.
   * Set to 0 if the loss is incorporated
   *into the antenna gain */

  fl32 two_way_radome_loss_db_h;

  /** 2-way Radome loss (dB) - V.
   * Set to 0 if the loss is incorporated
   *into the antenna gain */
  
  fl32 two_way_radome_loss_db_v;
  
  fl32 receiver_mismatch_loss_db; /**< receiver mistmatch loss (dB) */

  fl32 radar_constant_h; /**< radar constant, H */
  fl32 radar_constant_v; /**< radar constant, V */
  
  /* SNR is computed relative to the following noise values */
  
  fl32 noise_dbm_hc; /**< calibrated noise power, dBm, H co-polar */
  fl32 noise_dbm_hx; /**< calibrated noise power, dBm, H cross-polar */
  fl32 noise_dbm_vc; /**< calibrated noise power, dBm, V co-polar */
  fl32 noise_dbm_vx; /**< calibrated noise power, dBm, V cross-polar */
  
  fl32 receiver_gain_db_hc; /**< receiver gain from waveguide to digitizer,
			     * dB, H co-polar */
  fl32 receiver_gain_db_hx; /**< receiver gain from waveguide to digitizer,
			     * dB, H cross-polar */
  fl32 receiver_gain_db_vc; /**< receiver gain from waveguide to digitizer,
			     * dB, V co-polar */
  fl32 receiver_gain_db_vx; /**< receiver gain from waveguide to digitizer,
			     * dB, V cross-polar */
  
  fl32 base_dbz_1km_hc; /**< base reflectivity at 1km, for SNR = 0, H co-polar.
			 * To compute dBZ:
			 * \code dBZ = baseDbz1km + SNR + 20log10(rangeKm) + (atmosAtten * rangeKm) \endcode
			 * BaseDbz1km can be computed as follows:
			 * \code baseDbz1km = noiseDbm - receiverGainDb - radarConst \endcode
			 * However, sometimes these values are not available, and the
			 * baseDbz1km value must be used as is.
			 * This is the case for RVP8 time series data*/

  fl32 base_dbz_1km_hx; /**< base reflectivity at 1km, for SNR = 0, H cross-polar.
			 * To compute dBZ:
			 * \code dBZ = baseDbz1km + SNR + 20log10(rangeKm) + (atmosAtten * rangeKm) \endcode
			 * BaseDbz1km can be computed as follows:
			 * \code baseDbz1km = noiseDbm - receiverGainDb - radarConst \endcode
			 * However, sometimes these values are not available, and the
			 * baseDbz1km value must be used as is.
			 * This is the case for RVP8 time series data*/

  fl32 base_dbz_1km_vc; /**< base reflectivity at 1km, for SNR = 0, V co-polar.
			 * To compute dBZ:
			 * \code dBZ = baseDbz1km + SNR + 20log10(rangeKm) + (atmosAtten * rangeKm) \endcode
			 * BaseDbz1km can be computed as follows:
			 * \code baseDbz1km = noiseDbm - receiverGainDb - radarConst \endcode
			 * However, sometimes these values are not available, and the
			 * baseDbz1km value must be used as is.
			 * This is the case for RVP8 time series data*/

  fl32 base_dbz_1km_vx; /**< base reflectivity at 1km, for SNR = 0, V cross-polar.
			 * To compute dBZ:
			 * \code dBZ = baseDbz1km + SNR + 20log10(rangeKm) + (atmosAtten * rangeKm) \endcode
			 * BaseDbz1km can be computed as follows:
			 * \code baseDbz1km = noiseDbm - receiverGainDb - radarConst \endcode
			 * However, sometimes these values are not available, and the
			 * baseDbz1km value must be used as is.
			 * This is the case for RVP8 time series data*/

  fl32 sun_power_dbm_hc; /**< sun power, dBm, H co-polar */
  fl32 sun_power_dbm_hx; /**< sun power, dBm, H cross-polar */
  fl32 sun_power_dbm_vc; /**< sun power, dBm, V co-polar */
  fl32 sun_power_dbm_vx; /**< sun power, dBm, V cross-polar */

  fl32 noise_source_power_dbm_h; /**< noise source power, dBm, H */
  fl32 noise_source_power_dbm_v; /**< noise source power, dBm, V */

  fl32 power_meas_loss_db_h; /**< Power measurement loss from ref point
			      * to power meter sensor - H.
			      * This will generally be positive, to indicate a loss.
			      * If there is an amplifier in the calibration circuit, use a negative
			      * number to indicate a gain */

  fl32 power_meas_loss_db_v; /**< Power measurement loss from ref point
			      * to power meter sensor - V.
			      * This will generally be positive, to indicate a loss.
			      * If there is an amplifier in the calibration circuit, use a negative
			      * number to indicate a gain */

  fl32 coupler_forward_loss_db_h; /**< Directional coupler loss, H. */
  fl32 coupler_forward_loss_db_v; /**< Directional coupler loss, V. */
  
  fl32 test_power_dbm_h;  /**< Power into directional coupler
                           * when test set commanded to 0 dBm, H */
  fl32 test_power_dbm_v;  /**< Power into directional coupler
                           * when test set commanded to 0 dBm, V */

  fl32 zdr_correction_db;  /**< ZDR correction, dB.
			    * Corrected ZDR = measured ZDR + correction */
  
  fl32 ldr_correction_db_h; /**< LDR correction, dB, H.
			     * Corrected LDR = measured LDR + correction */
  fl32 ldr_correction_db_v; /**< LDR correction, dB, V.
			     * Corrected LDR = measured LDR + correction */

  fl32 phidp_rot_deg; /**< system phipd - degrees */
  
  fl32 receiver_slope_hc; /**< receiver curve slope, H co-polar */
  fl32 receiver_slope_hx; /**< receiver curve slope, H cross-polar */
  fl32 receiver_slope_vc; /**< receiver curve slope, V co-polar */
  fl32 receiver_slope_vx; /**< receiver curve slope, V cross-polar */
  
  fl32 i0_dbm_hc; /**< calibrated I0 noise, dBm, H co-polar */
  fl32 i0_dbm_hx; /**< calibrated I0 noise, dBm, H cross-polar */
  fl32 i0_dbm_vc; /**< calibrated I0 noise, dBm, V co-polar */
  fl32 i0_dbm_vx; /**< calibrated I0 noise, dBm, V cross-polar */
  
  fl32 dynamic_range_db_hc; /**< dynamic range, dB, H co-polar */
  fl32 dynamic_range_db_hx; /**< dynamic range, dB, H cross-polar */
  fl32 dynamic_range_db_vc; /**< dynamic range, dB, V co-polar */
  fl32 dynamic_range_db_vx; /**< dynamic range, dB, V cross-polar */
  
  fl32 k_squared_water; /**< dielectric constant for water at this wavelength */ 

  fl32 dbz_correction;  /**< DBZ correction, dB.
                         * Corrected DBZ = measured DBZ + correction */

  si32 unused[49]; /**< for future expansion */ 

  char radar_name[IWRF_MAX_RADAR_NAME]; /**< name of instrument */

} iwrf_calibration_t;

/************************************************************************/
/**
 * \struct iwrf_event_notice
 *
 * Signals events such as end of volume, errors, restart etc.
 *
 ************************************************************************/

typedef struct iwrf_event_notice {

  iwrf_packet_info_t packet; /*< packet_id = IWRF_EVENT_NOTICE_ID */
  
  si32 start_of_sweep; /**< TRUE / FALSE */
  si32 end_of_sweep; /**< TRUE / FALSE */

  si32 start_of_volume; /**< TRUE / FALSE */
  si32 end_of_volume; /**< TRUE / FALSE */
  
  si32 scan_mode; /**< The current scan mode - \ref iwrf_scan_mode */
  si32 follow_mode; /**< The current follow mode - \ref iwrf_follow_mode */
  si32 volume_num; /**< Volume number, increments linearly */
  si32 sweep_num; /**< Sweep number within the current volume */
  
  si32 cause; /**< Additional information about the event -
	       * \ref iwrf_event_cause */

  fl32 current_fixed_angle; /**< Current fixed angle
                             *   (az in RHI, el in PPI) */

  si32 antenna_transition; /**< antenna is in transition
                            * 1 = true, 0 = false */

  si32 unused[7]; /**< not used - for later expansion */

} iwrf_event_notice_t;

/************************************************************************/
/**
 * \struct iwrf_phase_sample
 *
 * Pair of phase values for two channels.
 *
 ************************************************************************/

typedef struct iwrf_phase_sample {
  fl32 phase_deg_h; /**< V channel phase, in degrees */
  fl32 phase_deg_v; /**< H channel phase, in degrees */
} iwrf_phase_sample_t;

/************************************************************************/
/**
 * \struct iwrf_phasecode
 *
 * Phase code definition, sent when phase-coded volumes are started
 *
 ************************************************************************/

typedef struct iwrf_phasecode {

  iwrf_packet_info_t packet; /*< packet_id = IWRF_PHASECODE_ID */
  
  si32 seq_length; /**< Number of pulses in sequence */
  si32 spare;
  iwrf_phase_sample_t phase[IWRF_MAX_PHASE_SEQ_LEN]; /**< Phase sequence */

  fl32 unused[496];

} iwrf_phasecode_t;

/************************************************************************/
/**
 * \struct iwrf_xmit_info
 *
 * Transmitter mode and status
 *
 ************************************************************************/

typedef struct iwrf_xmit_info {
  
  iwrf_packet_info_t packet; /*< packet_id = IWRF_XMIT_INFO_ID */
  
  si32 xmit_0_enabled; /**< transmitter 0 firing  */
  si32 xmit_1_enabled; /**< transmitter 1 firing  */

  si32 xmit_rcv_mode;   /**< current transmit/receive mode - \ref iwrf_xmit_rcv_mode */
  si32 pol_mode;  /**< current polarization mode - \ref iwrf_pol_mode */ 
  si32 xmit_phase_mode; /**< current transmit phase mode - \ref iwrf_xmit_phase_mode */
  si32 prf_mode;        /**< current PRF mode - \ref iwrf_prf_mode */
  si32 pulse_type;      /**< current pulse type - \ref iwrf_pulse_type */

  fl32 prt_usec; /**< PRT in microseconds */
  fl32 prt2_usec; /**< Second PRT in microseconds for Dual-PRT mode */

  fl32 unused[9]; /**< for future expansion */
  
} iwrf_xmit_info_t;

/************************************************************************/
/**
 * \struct iwrf_pulse_header
 *
 * Pulse header.
 * This header precedes the IQ data.
 *
 * A pulse data packet is made up as follows:
 * 
 *  \li iwrf_pulse_header
 *  \li iq-data channel 0 (H or co-polar)
 *  \li iq-data channel 1 (V or x-polar) (if available)
 *  \li iq-data channel 2 (if available)
 *  \li iq-data channel 3 (if available)
 *
 * len_bytes will be set to the entire packet length.
 *   len_bytes = length of structure, plus following data
 *
 ************************************************************************/

typedef struct iwrf_pulse_header {
  
  iwrf_packet_info_t packet; /*< packet_id = IWRF_PULSE_HEADER_ID *
			  * len_bytes = length of this structure,
			  * plus following data, in bytes */
  
  si64 pulse_seq_num;  /**< pulse sequence number - increments */

  si32 scan_mode; /**< The current scan mode - \ref iwrf_scan_mode */
  si32 follow_mode; /**< The current follow mode - \ref iwrf_follow_mode */
  si32 volume_num;  /**< scan vol  num - if avail, otherwise set to -1 */ 
  si32 sweep_num;   /**< scan tilt num - if avail, otherwise set to -1 */ 

  fl32 fixed_el;  /**< target elevation for ppis */
  fl32 fixed_az;  /**< target azimuth for rhis */
  fl32 elevation;    /**< antenna elevation */
  fl32 azimuth;      /**< antenna azimuth */
  
  fl32 prt;   /**< time since previous pulse, secs */
  fl32 prt_next;     /**< time to next pulse, secs, if available */
  
  fl32 pulse_width_us;  /**< microsecs */

  si32 n_gates;  /**< number of gates of IQ data */

  si32 n_channels;  /**< number of channels in the IQ data */
  si32 iq_encoding; /**< IWRF_IQ_ENCODING */
  si32 hv_flag;     /**< 1 if H, 0 if V for alternating mode
                     **< 3 if simultaneous H/V */

  si32 antenna_transition; /**< antenna is in transition
                            * 1 = TRUE, 0 = FALSE */
  
  si32 phase_cohered; /**< received phases are cohered to burst phase
                       * 1 = TRUE, 0 = FALSE.
		       * Applies to
                       * \ref IWRF_XMIT_PHASE_MODE_RANDOM and
                       * \ref IWRF_XMIT_PHASE_MODE_SZ864 */
  
  si32 status; /**< general status flag - optional use */

  si32 n_data; /**< number of data values in the IQ array
                * \code nData = nChannels * (nGates + nGatesBurst) * 2 \endcode */
  
  si32 iq_offset[IWRF_MAX_CHAN];  /**< Index of gate 0 for this chan, * in IQ[] array.
                                   * This should always be a multiple of 2, since it
                                   * refers to the I in and I/Q pair.A
                                   * e.g. if n_gates_burst is 1, then iq_offset should be 2. */
  fl32 burst_mag[IWRF_MAX_CHAN];  /**< Burst pulse magnitude
                                   * (amplitude, not power) */
  fl32 burst_arg[IWRF_MAX_CHAN];  /**< Burst phase phase (deg) */
  fl32 burst_arg_diff[IWRF_MAX_CHAN]; /**< Burst phase change (deg)
                                       * \code diff = (ArgPrevPulse - ArgThisPulse) \endcode */

  fl32 scale;   /**< for use with IWRF_IQ_ENCODING types
                 * to convert to/from floats */
  fl32 offset;  /**< for use with IWRF_IQ_ENCODING types
                 * to convert to/from floats */

  si32 n_gates_burst; /**< number of gates at start of data array
                       * holding burst IQ information. For example
                       * the RVP8 stores burst data in the first
		       * IWRF_RVP8_NGATES_BURST gates.
                       * For other systems this should normally be 0 */

  fl32 start_range_m; /**< Range to start of first gate, (in meters) */
  fl32 gate_spacing_m; /**< Gate spacing (in meters) */

  si32 event_flags; /**< flags from end of volume, start of tilt etc.
                     * See iwrf_event_flags_t */

  si32 txrx_state;  /**< flags for tx/rx status, short/long PRT etc.
                     * see iwrf_txrx_state_flags_t */

  si32 unused[6];

} iwrf_pulse_header_t;

/************************************************************************/
/************************************************************************/
/* RVP8-specific support for time series data */
/************************************************************************/
/************************************************************************/

#define IWRF_RVP8_GATE_MASK_LEN  512 /**< Length of range mask in RVP8 ops info */
#define IWRF_RVP8_NGATES_BURST 2 /**< number of gates used for storing RVP8 burst pulse */

/************************************************************************/
/**
 * \struct  iwrf_rvp8_ops_info
 *
 * Operations info for RVP8-specific support.
 * This is only needed if (a) the processor is an RVP8 and (b) you need
 * to wrote time-series data in RVP8 tsarchive format.
 *
 ************************************************************************/

typedef struct iwrf_rvp8_ops_info {
  
  iwrf_packet_info_t packet; /*< packet_id = IWRF_RVP8_OPS_INFO_ID */
  
  si32 i_version; /**< Version of this public structure
                   * \li 0: Initial public element layout
                   * \li 1: Added fDBzCalib, iSampleSize, iMeanAngleSync, iFlags, iPlaybackVersion
                   * \li Set to 0 if no RVP8 processor
		   */

  ui32 i_major_mode;   /**< RVP8 major mode, one of PMODE_xxx */  
  ui32 i_polarization; /**< Polarization selection, one of POL_xxx */
  ui32 i_phase_mode_seq;  /**< Tx phase modulation, one of PHSEQ_xxx */

  ui16 i_task_sweep;    /**< Sweep number within a task */
  ui16 i_task_aux_num;   /**< Auxiliary sequence number */
  si32 i_task_scan_type; /**< Scan geometry, one of SCAN_xxx */
  si32 unused1[3];

  char s_task_name[32];  /**< Name of the task (NULL terminated) */

  char s_site_name[32];  /**< Data are from this site (NULL terminated) */

  ui32 i_aq_mode;     /**< Acquisition mode (filled in by local API) */
  ui32 i_unfold_mode; /**< Dual-PRF unfolding 0:None, 1:2/3, 2:3/4, 3:4/5 */
  
  ui32 i_pwidth_code;  /**< Pulse width selection (0 to RVP8NPWIDS-1) */
  fl32 f_pwidth_usec;  /**< Actual width in microseconds */

  fl32 f_dbz_calib;       /**< Calibration reflectivity (dBZ at 1 km) */
  si32 i_sample_size;     /**< Number of pulses per ray */
  ui32 i_mean_angle_sync; /**< Mean angle sync ray spacing */

  ui32 i_flags; /**< Various bit flags */

  si32 i_playback_version; /**< Allows playback tagging, zeroed initially */

  fl32 f_sy_clk_mhz;     /**< IFD system (XTAL) clock frequency (MHz) */
  fl32 f_wavelength_cm;  /**< Radar wavelength (cm) */
  fl32 f_saturation_dbm; /**< Power (dBm) for MAG(I,Q) of 1.0 */

  fl32 f_range_mask_res;   /**< Spacing between range bins (meters) */
  ui16 i_range_mask[IWRF_RVP8_GATE_MASK_LEN]; /**< Bit mask of bins that have
					       * actually been selected at
					       * the above spacing. */

  fl32 f_noise_dbm[IWRF_MAX_CHAN]; /**< Noise level in dBm, and standard */
  fl32 f_noise_stdv_db[IWRF_MAX_CHAN]; /**< deviation of measurement, dB. */
  fl32 f_noise_range_km;      /**< Range (km) and PRF (Hz) at which the */
  fl32 f_noise_prf_hz;        /**<   last noise sample was taken. */

  ui16 i_gparm_latch_sts[2];  /**< Copies of latched and immediate status */
  ui16 i_gparm_immed_sts[6];  /**<   words from the GPARM structure. */
  ui16 i_gparm_diag_bits[4];  /**< Copies of GPARM diagnostic bits */

  char s_version_string[12];  /**< IRIS/RDA version that created the data. */

  si32 unused2[185];

} iwrf_rvp8_ops_info_t;

/************************************************************************/
/**
 * \struct  iwrf_rvp8_pulse_header
 *
 * Time-series pulse header for RVP8-specific support.
 * This is only needed if (a) the processor is an RVP8 and (b) you need
 * to write time-series data in RVP8 tsarchive format.
 *
 * If relevant, this packet will immediately precede a normal pulse packet.
 *   
 ************************************************************************/

typedef struct iwrf_rvp8_pulse_header {

  iwrf_packet_info_t packet; /*< packet_id = IWRF_RVP8_PULSE_HEADER_ID */
  
  si32 i_version; /**< version number */

  ui08 i_flags; /**< Control flags - set to 255 if no RVP8 processor */
  ui08 i_aq_mode; /**< Sequence numbers for acquisition mode */
  ui08 i_polar_bits; /**< State of polarization control bits */
  ui08 i_viq_per_bin; /**< (I,Q) vectors/bin, i.e., # of Rx channels */
  ui08 i_tg_bank; /**< Trigger bank number (one of TBANK_xxx) */
  ui08 unused1[3]; /**< not used - for later expansion */

  ui16 i_tx_phase;  /**< Phase angle of transmit data */
  ui16 i_az;  /**< Antenna azimuth * 65535 / 360 */
  ui16 i_el;  /**< Antenna elevation * 65535 / 360 */
  si16 i_num_vecs;  /**< Actual (I,Q) vectors (burst+data) for each Rx, */
  si16 i_max_vecs;  /**<   and maximum number (which sets data stride). */
  ui16 i_tg_wave;   /**< Trigger waveform sequence number within a bank */

  ui32 i_btime_api; /**< Local time (ms) when pulse arrived in API */
  ui32 i_sys_time;  /**< IFD system clock time
                     * (see rvp8PulseInfo.fSyClkMHz) */
  ui32 i_prev_prt;  /**< SysTime ticks to previous/next pulse */
  ui32 i_next_prt;  /**< SysTime ticks to previous/next pulse */

  ui32 i_seq_num;   /**< pulse sequence number */

  ui32 uiq_perm[2]; /**< User specified bits (Permanent) */
  ui32 uiq_once[2]; /**< User specified bits (One-Shot) */

  si32 i_data_off[IWRF_MAX_CHAN]; /**< Offset of start of this pulse in fIQ[] */
  fl32 f_burst_mag[IWRF_MAX_CHAN];/**< Burst pulse magnitude (amplitude, not power) */
  ui16 i_burst_arg[IWRF_MAX_CHAN];/**< Burst phase changes (PrevPulse - ThisPulse) */
  ui16 i_wrap_iq[IWRF_MAX_CHAN];  /**< Data wraparound count for validity check */

  si32 unused2[23]; /**< not used - for later expansion */

} iwrf_rvp8_pulse_header_t;

/************************************************************************/
/************************************************************************/
/* Moments data support */
/*
 * The moments data stream uses many of the same meta-data structures
 * as the time series data.
 *
 * Moments data is represented as follows:
 *
 * (a) For each field in the moments, there will be included in the data
 *     stream a field_header_t struct, which specifies the field name,
 *     units etc, as well as other information on how the field is stored.
 *     These headers will appear before a ray.
 *
 * (b) Each ray (or beam) will have a ray_header_t, which amongst other
 *     things will specify the number of fields in the ray.
 *     This will be followed by an array of field_index_t structs, which
 *     will specify where in the ray data the specific field appears.
 *     This will be followed by the data itself.
 *
 *     So a ray packet will be made up of:
 *
 *       iwrf_moments_ray_header_t
 *       n_fields * iwrf_moments_field_index_t
 *       data bytes
 *
 */
/************************************************************************/
/************************************************************************/

/************************************************************************/
/**
 * \struct iwrf_moments_field_header
 *
 * Field header for moments data.
 * There must be one of these headers for each field to be included
 * later in the data stream.
 *
 ************************************************************************/

typedef struct iwrf_moments_field_header {
  
  iwrf_packet_info_t packet; /*< packet_id = IWRF_MOMENTS_FIELD_HEADER_ID *
                              * len_bytes = length of this structure */

  /* names and units */

  char name[IWRF_MAX_MOMENTS_FIELD_NAME];
  char long_name[IWRF_MAX_MOMENTS_FIELD_NAME_LONG];
  char standard_name[IWRF_MAX_MOMENTS_FIELD_NAME_STANDARD];
  char units[IWRF_MAX_MOMENTS_FIELD_UNITS];
  char threshold_field_name[IWRF_MAX_MOMENTS_FIELD_NAME];

  /* data encoding */

  si32 encoding; /**< IWRF_MOMENTS_ENCODING */
  si32 byte_width; /**< n bytes per value */

  fl32 scale;   /**< for si08, si16, si32 */
  fl32 offset;  /**<  for si08, si16, si32 */
  
  /* Sampling ratio:
   * Some fields may be computed using a different number of samples
   * from others. This ratio is the number of samples used
   * to compute this field divided by the number of samples
   * stored on the RadxRay object.
   * By default it is 1.0, which will apply to most cases. */

  fl32 sampling_ratio;

  /* thresholding on another field */

  fl32 threshold_value;
  
  si32 n_samples;   /**< number of pulse samples used to compute field */
  fl32 nyquist_mps; /**< nyquist velocity (m/s) if applicable */

  si32 unused[22]; /**< keep size at 512 bytes */

} iwrf_moments_field_header_t;

/************************************************************************/
/**
 * \struct iwrf_moments_ray_header
 *
 * Ray header for moments data.
 *
 * A ray data packet is made up as follows:
 * 
 *  \li iwrf_moments_ray_header
 *  \li n_fields * iwrf_moments_field_index_t
 *  \li field_data
 *
 * len_bytes will be set to the entire packet length.
 *   len_bytes = length of this structure, plus following structures and data
 *
 ************************************************************************/

typedef struct iwrf_moments_ray_header {
  
  iwrf_packet_info_t packet; /*< packet_id = IWRF_MOMENTS_RAY_HEADER_ID *
                              * len_bytes = length of this structure */

  // data
  
  si32 volume_num;  /**< scan vol  num - if avail, otherwise set to -1 */ 
  si32 sweep_num;   /**< scan tilt num - if avail, otherwise set to -1 */ 

  si32 scan_mode;   /**< The current scan mode - \ref iwrf_scan_mode */
  si32 follow_mode; /**< The current follow mode - \ref iwrf_follow_mode */

  si32 prf_mode; /**< The current PRF mode - iwrf_prf_mode_t */
  si32 polarization_mode; /**< The current pol mode - iwrf_pol_mode_t */

  fl32 elevation; /**< antenna elevation */
  fl32 azimuth;   /**< antenna azimuth */
  fl32 fixed_angle;/**< target angle - elevation for ppis, ez for rhis */
  
  fl32 target_scan_rate;  // deg/s
  fl32 true_scan_rate;    // deg/s
  
  si32 is_indexed;   /**< ray is indexed */
  fl32 angle_res;    /**< angular resolution if indexed (deg) */
  
  si32 antenna_transition; /**< antenna is in transition
                            * 1 = TRUE, 0 = FALSE */

  fl32 prt;       /**< time since previous pulse, secs */
  fl32 prt_ratio; /**<  PRT ratio, for staggered or dual PRT, < 1
                   *    generally 2/3, 3/4 or 4/5 */

  fl32 pulse_width_us;  /**< microsecs */
  
  si32 n_samples; /**< number of pulses in dwell */
  si32 n_fields;  /**< number of fields in this ray */
  
  si32 n_gates;   /**< number of gates in each field */
  fl32 start_range_m;  /**< Range to start of first gate, (in meters) */
  fl32 gate_spacing_m; /**< Gate spacing (in meters) */
  
  fl32 nyquist_mps;      /**< nyquist velocity (m/s) */
  fl32 unambig_range_km; /**< unambiguous range (km) */

  // transmit power is generally uncorrected for gains etc

  fl32 meas_xmit_power_dbm_h; /**< measured transmit power in H channel */
  fl32 meas_xmit_power_dbm_v; /**< measured transmit power in V channel */

  si32 event_flags; /**< flags from end of volume, start of tilt etc.
                     * See iwrf_event_flags_t */
  
  si32 unused[23]; /* keep size at 256 bytes */

} iwrf_moments_ray_header_t;

/************************************************************************/
/**
 * \struct iwrf_moments_index_header
 *
 * Index struct to show where each field is stored.
 * An array of these, n_fields long, follows the ray_header,
 * and precedes the data.
 *
 ************************************************************************/

typedef struct iwrf_moments_field_index {

  char field_name[IWRF_MAX_MOMENTS_FIELD_NAME];
  si32 id; /**< Id for the packet type: IWRF_MOMENTS_FIELD_INDEX_ID */
  si32 offset; /**< in bytes, from start of packet */
  si32 len; /**<  in bytes */
  si32 spare[5]; /**< keep len at 64 bytes */

} iwrf_moments_field_index_t;

/************************************************************************/
/**
 * \struct platform_georef
 *
 * Georeference information for moving platforms
 *
 ************************************************************************/

typedef struct iwrf_platform_georef {
    
  iwrf_packet_info_t packet; /*< packet_id = IWRF_PLATFORM_GEOREF_ID */
  
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
  fl32 tilt_deg;            /**< Angle of radar beam and line normal
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
  
} iwrf_platform_georef_t;

/************************************************************************/
/**
 * \struct georef_correction
 *
 * Corrections to georeference information
 *
 ************************************************************************/
  
typedef struct iwrf_georef_correction {
    
  iwrf_packet_info_t packet; /*< packet_id = IWRF_GEOREF_CORRECTION_ID */
  
  fl32 azimuth_corr_deg;      /**< Correction added to azimuth[deg] */
  fl32 elevation_corr_deg;    /**< Correction added to elevation[deg] */
  fl32 range_delay_corr_mps;  /**< Correction used for range delay[m] */
  fl32 longitude_corr_deg;    /**< Correction added to radar longitude */
  fl32 latitude_corr_deg;     /**< Correction added to radar latitude */
  fl32 pressure_alt_corr_km ; /**< Correction added to pressure altitude
                               * (msl)[km] */
  fl32 radar_alt_corr_km;     /**< Correction added to radar altitude above
                               * ground level(agl) [km] */
  fl32 ew_gndspd_corr_mps;    /**< Correction added to radar platform
                               * ground speed(E-W)[m/s] */
  fl32 ns_gndspd_corr_mps;    /**< Correction added to radar platform
                               * ground speed(N-S)[m/s] */
  fl32 vert_vel_corr_mps;     /**< Correction added to radar platform
                               * vertical velocity[m/s] */
  fl32 heading_corr_deg;      /**< Correction added to radar platform
                               * heading [deg]) */
  fl32 roll_corr_deg;         /**< Correction added to radar platform
                               * roll[deg] */
  fl32 pitch_corr_deg;        /**< Correction added to radar platform
                               * pitch[deg] */
  fl32 drift_corr_deg;        /**< Correction added to radar platform
                               * drift[deg] */
  fl32 rot_angle_corr_deg;    /**< Corrrection add to radar rotation angle
                               *[deg] */
  fl32 tilt_corr_deg;         /**< Correction added to radar tilt angle */
  
  fl32 unused[34];            /**< for future expansion */
  
} iwrf_georef_correction_t;

#ifdef __cplusplus
}
#endif

#ifdef WIN32
#pragma pack(pop)
#endif

#endif

