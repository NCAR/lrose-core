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
 * /file <iwrf_time_series.h>
 *
 * Defines for handling time series in radar data.
 *
 * CSU-CHILL/NCAR FORMATS (IWRF)
 *
 * Mike Dixon, RAL, NCAR, Boulder, CO
 * Dec 2008
 *
 *********************************************************************/

#ifndef _IWRF_TIME_SERIES_H_
#define _IWRF_TIME_SERIES_H_

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
 * \li fl64: 8-byte floationg point
 */

#include <dataport/port_types.h>
#include <math.h>

#define IWRF_MAX_CHAN 4  /**< Max number of channels<br>Normal dual-pol operation uses 2 channels */
#define IWRF_MAX_RADAR_NAME 32  /**< Max length of radar name */
#define IWRF_MAX_SITE_NAME  32  /**< Max length of site name */
#define IWRF_MAX_SEGNAME_LENGTH 32  /**< Max length of scan segment name */
#define IWRF_MAX_PRJNAME_LENGTH 32  /**< Max length of project name */
#define IWRF_MAX_FIXED_ANGLES 512  /**< Length of array of fixed angles */
#define IWRF_N_TXSAMP 512  /**< Length of transmit sample array */
#define IWRF_MAX_PHASE_SEQ_LEN 256  /**< Max length of phase coding sequence array */

/* missing value for meta-data
 * used where value is not known */

#define IWRF_MISSING_INT -9999 /**< Missing val for integer values */
#define IWRF_MISSING_FLOAT NAN /**< Missing val for float values */

/* packet IDs */

#define IWRF_SYNC_ID 0xaaaa0001 /**< ID for sync packet */
#define IWRF_RADAR_INFO_ID 0xaaaa0002 /**< ID for radar_info packet */
#define IWRF_SCAN_SEGMENT_ID 0xaaaa0003 /**< ID for scan segment packet */
#define IWRF_ANTENNA_CORRECTION_ID 0xaaaa0004 /**< ID for antenna correction packet */
#define IWRF_TS_PROCESSING_ID 0xaaaa0005 /**< ID for time-series processing packet */
#define IWRF_XMIT_POWER_ID 0xaaaa0006 /**< ID for measured transmit power packet */
#define IWRF_XMIT_SAMPLE_ID 0xaaaa0007 /**< ID for transmit sample packet */
#define IWRF_CALIBRATION_ID 0xaaaa0008 /**< ID for calibration packet */
#define IWRF_EVENT_NOTICE_ID 0xaaaa0009 /**< ID for event notice packet */
#define IWRF_PHASECODE_ID 0xaaaa000a /**< ID for phase code packet */
#define IWRF_XMIT_INFO_ID 0xaaaa000b /**< ID for transmit infopacket */
#define IWRF_PULSE_HEADER_ID 0xaaaa000c /**< ID for pulse header packet */
#define IWRF_RVP8_PULSE_HEADER_ID 0xaaaa000d /**< ID for RVP8 pulse header packet */
#define IWRF_RVP8_OPS_INFO_ID 0xaaaa000e /**< ID for RVP8 operations info packet */

#define IWRF_SYNC_VAL_00 0xdadadada /**< Value for odd words in sync packet */
#define IWRF_SYNC_VAL_01 0x17171717 /**< Value for even words in sync packet */

/************************************************************************/
/************************************************************************/
/************************************************************************/
/* ENUMERATED TYPES */
/************************************************************************/
/************************************************************************/
/************************************************************************/

/************************************************************************/
/**
 * \enum iwrf_xmit_rcv_mode
 *
 * Transmit and receive modes.
 *
 ************************************************************************/

typedef enum iwrf_xmit_rcv_mode {
  
  /** single polarization */
  IWRF_SINGLE_POL = 0,

  /** Dual pol, alternating transmission,
   * copolar receiver only (CP2 SBand) */

  IWRF_ALT_HV_CO_ONLY = 1,

  /** Dual pol, alternating transmission, co-polar and cross-polar
   * receivers (SPOL with Mitch Switch and receiver in 
   * switching mode, CHILL) */

  IWRF_ALT_HV_CO_CROSS = 2,
  
  /** Dual pol, alternating transmission, fixed H and V receivers (SPOL
   * with Mitch Switch and receive7rs in fixed mode) */
  
  IWRF_ALT_HV_FIXED_HV = 3,
  
  /** Dual pol, simultaneous transmission, fixed H and V receivers (NEXRAD
   * upgrade, SPOL with T and receivers in fixed mode) */

  IWRF_SIM_HV_FIXED_HV = 4,
  
  /** Dual pol, simultaneous transmission, switching H and V receivers
   * (SPOL with T and receivers in switching mode) */
  
  IWRF_SIM_HV_SWITCHED_HV = 5,

  /** Dual pol, H transmission, fixed H and V receivers (CP2 X band) */

  IWRF_H_ONLY_FIXED_HV = 6,

  /** Dual pol, V transmission, fixed H and V receivers */
  
  IWRF_V_ONLY_FIXED_HV = 7,

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

  IWRF_XMIT_PHASE_MODE_FIXED = 0,  /**< Klystron */
  IWRF_XMIT_PHASE_MODE_RANDOM = 1, /**< Magnetron */
  IWRF_XMIT_PHASE_MODE_SZ864 = 2,  /**< Sachinanda-Zrnic 8/64 phase code */
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

  IWRF_PRF_MODE_FIXED = 0, /**< fixed pulsing mode */
  IWRF_PRF_MODE_STAGGERED_2_3, /**< staggered PRT, 2/3 ratio */
  IWRF_PRF_MODE_STAGGERED_3_4, /**< staggered PRT, 3/4 ratio */
  IWRF_PRF_MODE_STAGGERED_4_5, /**< staggered PRT, 4/5 ratio */
  IWRF_PRF_MODE_LAST /**< not used */

} iwrf_prf_mode_t;

/************************************************************************/
/**
 * \enum iwrf_pulse_type
 *
 ************************************************************************/

typedef enum iwrf_pulse_type {

  IWRF_PULSE_TYPE_RECT = 0, /**< rectangular pulse */
  IWRF_PULSE_TYPE_GAUSSIAN = 1, /**< gaussian-weighted pulse */
  IWRF_PULSE_TYPE_LAST /**< not used */

} iwrf_pulse_type_t;

/************************************************************************/
/**
 * \enum iwrf_pulse_polarization
 *
 ************************************************************************/

typedef enum iwrf_pulse_polarization {

  IWRF_PULSE_POL_H = 0, /**< H pulse */
  IWRF_PULSE_POL_V = 1, /**< V pulse */
  IWRF_PULSE_POL_SLANT45 = 2, /**< Slant-45 */
  IWRF_PULSE_POL_LAST /**< not used */

} iwrf_pulse_polarization_t;

/************************************************************************/
/**
 * \enum iwrf_scan_mode
 *
 * Antenna scanning mode - these are legacy NCAR codes, with the CHILL
 * FIXED, MANPPI and MANRHI appended
 *
 ************************************************************************/

typedef enum iwrf_scan_mode {

  IWRF_SCAN_MODE_UNKNOWN = -1, /**< don't know mode */
  IWRF_SCAN_MODE_SECTOR = 1, /**< sector scan mode */
  IWRF_SCAN_MODE_COPLANE = 2, /**< co-plane dual doppler mode */
  IWRF_SCAN_MODE_RHI = 3, /**< range height vertical scanning mode */
  IWRF_SCAN_MODE_VERTICAL_POINTING = 4, /**< vertical pointing for calibration */
  IWRF_SCAN_MODE_IDLE = 7, /**< between scans */
  IWRF_SCAN_MODE_AZ_SUR_360 = 8, /**< 360-degree azimuth mode - surveillance */
  IWRF_SCAN_MODE_EL_SUR_360 = 9, /**< 360-degree elevation mode - eg Eldora */
  IWRF_SCAN_MODE_SUNSCAN = 11, /**< scanning the sun for calibrations */
  IWRF_SCAN_MODE_POINTING = 12, /**< fixed pointing */
  IWRF_SCAN_MODE_FOLLOW_VEHICLE = 13, /**< follow vehicle */
  IWRF_SCAN_MODE_EL_SURV = 14, /**< elevation surveillance (eldora) */
  IWRF_SCAN_MODE_MANPPI = 15, /**< Manual PPI mode (elevation does
			       * not step automatically) */
  IWRF_SCAN_MODE_MANRHI = 16, /**< Manual RHI mode (azimuth does
			       * not step automatically) */
  IWRF_SCAN_SUNSCAN_RHI = 17, /**< sunscan in RHI mode */
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

  IWRF_FOLLOW_MODE_NONE = 0, /**< Radar is not tracking any object */
  IWRF_FOLLOW_MODE_SUN = 1, /**< Radar is tracking the sun */
  IWRF_FOLLOW_MODE_VEHICLE = 2, /**< Radar is tracking a vehicle */
  IWRF_FOLLOW_MODE_AIRCRAFT = 3, /**< Radar is tracking an aircraft */
  IWRF_FOLLOW_MODE_TARGET = 4, /**< Radar is tracking a target - e.g. sphere */
  IWRF_FOLLOW_MODE_MANUAL = 5, /**< Radar is under manual tracking mode */
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

  IWRF_RADAR_PLATFORM_FIXED = 0, /**< Radar is in a fixed location */
  IWRF_RADAR_PLATFORM_VEHICLE = 1, /**< Radar is mounted on a land vehicle */
  IWRF_RADAR_PLATFORM_SHIP = 2, /**< Radar is mounted on a ship */
  IWRF_RADAR_PLATFORM_AIRCRAFT = 3, /**< Radar is mounted on an aircraft */
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
  
  IWRF_CAL_TYPE_NONE = 0, /**< No calibration is currently taking place */
  IWRF_CAL_TYPE_CW_CAL, /**< CW calibration */
  IWRF_CAL_TYPE_SOLAR_CAL_FIXED, /**< Fixed sun-pointing */
  IWRF_CAL_TYPE_SOLAR_CAL_SCAN, /**< Scanning across face of the sun */
  IWRF_CAL_TYPE_NOISE_SOURCE_H, /**< Noise source connected to H chan */
  IWRF_CAL_TYPE_NOISE_SOURCE_V, /**< Noise source connected to V chan */
  IWRF_CAL_TYPE_NOISE_SOURCE_HV, /**< Noise source connected to both chans */
  IWRF_CAL_TYPE_BLUESKY,    /**< Antenna is pointing at blue-sky */
  IWRF_CAL_TYPE_SAVEPARAMS, /**< Not a real test mode,
                             *   indicates to compute nodes to
                             *   save calibration params */
  IWRF_CAL_TYPE_LAST /**< not used */

} iwrf_cal_type_t;

/************************************************************************/
/**
 * \enum iwrf_event_notice_cause
 * Cause of particular events
 *
 ************************************************************************/

typedef enum iwrf_event_notice_cause {

  IWRF_ENOTICE_CAUSE_DONE = 0, /**< Scan completed normally */
  IWRF_ENOTICE_CAUSE_TIMEOUT = 1, /**< Scan has timed out */
  IWRF_ENOTICE_CAUSE_TIMER = 2, /**< Timer caused this scan to abort */
  IWRF_ENOTICE_CAUSE_ABORT = 3, /**< Operator issued an abort */
  IWRF_ENOTICE_CAUSE_SCAN_ABORT = 4, /**< Scan Controller detected error */
  IWRF_ENOTICE_CAUSE_RESTART = 5, /**< communication fault was recovered,
                                  *   restarting scan */
  IWRF_ENOTICE_CAUSE_LAST /**< not used */

} iwrf_event_notice_cause_t;

/************************************************************************/
/**
 * \enum iwrf_iq_encoding
 *
 * How the IQ data is encoded and packed.
 *
 ************************************************************************/

typedef enum iwrf_iq_encoding {

  IWRF_IQ_FLOAT32 = 0,         /**< 4-byte floats, volts */

  IWRF_IQ_COUNTS_INT16 = 1,    /**< raw counts,<br>
				* volts = (int16 * scale) + offset */

  IWRF_IQ_DBM_PHASE_INT16 = 2, /**< scaled 2-byte ints,
				* storing power_dbm and phase,<br>
				* power_dbm = (int16 * scale) + offset<br>
				* phase = (int16 * 360.0) / 65536.0  */

  IWRF_IQ_SIGMET_FLOAT16 = 3,  /**< SIGMET 16-bit floats */

  IWRF_IQ_LAST /**< not used */

} iwrf_iq_encoding_t;

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
 * \struct iwrf_sync
 *
 * Synchronization packet.
 * Sent regularly to resynchronize data stream if needed.
 *
 * The magik[6] array will contain the following:
 *
 *   <br>IWRF_SYNC_VAL_00  = 0xdadadada
 *   <br>IWRF_SYNC_VAL_01  = 0x17171717
 *   <br>IWRF_SYNC_VAL_00  = 0xdadadada
 *   <br>IWRF_SYNC_VAL_01  = 0x17171717
 *   <br>IWRF_SYNC_VAL_00  = 0xdadadada
 *   <br>IWRF_SYNC_VAL_01  = 0x17171717
 *
 ************************************************************************/

typedef struct iwrf_sync {
    
  si32 packet_id; /**< IWRF_SYNC_ID */
  si32 len_bytes; /**< length of this structure, in bytes */
  si32 version_num; /**< version number of this structure */
  si32 radar_id; /**< id of radar - in case of multipler radars */
  si64 time_secs_utc;  /**< UTC time in secs since Jan 1 1970 */
  si32 time_nano_secs; /**< partial secs - nanosecs */
  si32 reserved[3]; /**< future expansion */

  si32 magik[6]; /**< array of sync values */
  
} iwrf_sync_t;

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
    
  si32 packet_id; /**< IWRF_RADAR_INFO_ID */
  si32 len_bytes; /**< length of this structure, in bytes */
  si32 version_num; /**< version number of this structure */
  si32 radar_id; /**< id of radar - in case of multipler radars */
  si64 time_secs_utc;  /**< UTC time in secs since Jan 1 1970 */
  si32 time_nano_secs; /**< partial secs - nanosecs */
  si32 reserved[3]; /**< future expansion */
  
  fl32 latitude_deg;  /**< latitude, in degrees */
  fl32 longitude_deg; /**< longitude, in degrees */
  fl32 altitude_m;    /**< altitude, in meters */
  si32 platform_type; /**< platform type - \ref iwrf_radar_platform */

  fl32 beamwidth_deg_h; /**< Antenna beamwidth, horizontal, in degrees */
  fl32 beamwidth_deg_v; /**< Antenna beamwidth, vertical, in degrees */
  fl32 wavelength_cm;   /**< Radar wavelength, in centimeters */
  
  fl32 nominal_gain_ant_db_h; /**< nominal antenna gain, H */
  fl32 nominal_gain_ant_db_v; /**< nominal antenna gain, V */

  fl32 unused[29]; /**< for future expansion */
  
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

  si32 packet_id; /**< IWRF_SCAN_SEGMENT_ID */
  si32 len_bytes; /**< length of this structure, in bytes */
  si32 version_num; /**< version number of this structure */
  si32 radar_id; /**< id of radar - in case of multipler radars */
  si64 time_secs_utc;  /**< UTC time in secs since Jan 1 1970 */
  si32 time_nano_secs; /**< partial secs - nanosecs */
  si32 reserved[3]; /**< future expansion */
  
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
  
  fl32 unused[464]; /**< for future expansion */

  char segname[IWRF_MAX_SEGNAME_LENGTH]; /**< Name of this scan segment */
  char project[IWRF_MAX_PRJNAME_LENGTH]; /**< Project name */

} iwrf_scan_segment_t;

/************************************************************************/
/**
 * \struct iwrf_antenna_correction.
 *
 * For correcting antenna angles, as required.
 * These angles must be *added* to the az and el to obtain
 * the corrected version as follows:
 *   corrected_angle = raw_angle + correction
 *
 ************************************************************************/

typedef struct iwrf_antenna_correction {

  si32 packet_id; /**< \ref IWRF_ANTENNA_CORRECTION_ID */
  si32 len_bytes; /**< length of this structure, in bytes */
  si32 version_num; /**< version number of this structure */
  si32 radar_id; /**< id of radar - in case of multipler radars */
  si64 time_secs_utc;  /**< UTC time in secs since Jan 1 1970 */
  si32 time_nano_secs; /**< partial secs - nanosecs */
  si32 reserved[3]; /**< future expansion */
  
  fl32 az_correction; /**< Azimuth offset in 16-bit angle units */
  fl32 el_correction; /**< Elevation offset in 16-bit angle units */

  fl32 unused[20]; /**< for future expansion */

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

  si32 packet_id; /**< \ref IWRF_TS_PROCESSING_ID */
  si32 len_bytes; /**< length of this structure, in bytes */
  si32 version_num; /**< version number of this structure */
  si32 radar_id; /**< id of radar - in case of multipler radars */
  si64 time_secs_utc;  /**< UTC time in secs since Jan 1 1970 */
  si32 time_nano_secs; /**< partial secs - nanosecs */
  si32 reserved[3]; /**< future expansion */
  
  si32 xmit_rcv_mode;   /**< current transmit/receive mode - \ref iwrf_xmit_rcv_mode */
  si32 xmit_phase_mode; /**< current transmit phase mode - \ref iwrf_xmit_phase_mode */
  si32 prf_mode;        /**< current PRF mode - \ref iwrf_prf_mode */
  si32 pulse_type;      /**< current pulse type - \ref iwrf_pulse_type */

  fl32 prt_usec;  /**< PRT in microseconds */
  fl32 prt2_usec; /**< PRT in microseconds used in dual PRT/PRF modes */

  si32 cal_type; /**< calibration type - \ref iwrf_cal_type */
  
  fl32 range_offset_m; /**< Range offset to first gate, in meters */
  fl32 range_start_km; /**< Range to start processing */
  fl32 range_stop_km;  /**< Range to stop processing */
  fl32 gate_spacing_m; /**< Gate spacing (in meters) */

  si32 integration_cycle_pulses; /**< Requested number of hits for a ray */
  si32 clutter_filter_number; /**< Clutter filter number to be used */
  si32 range_gate_averaging; /**< Number of range gates to average */
  si32 max_gate;      /**< Number of gates for digitizer to acquire */

  fl32 test_power_dbm;  /**< Power at signal generator output in dBm,
                         * when test set is commanded to output 0dBm */
  fl32 test_pulse_range_km;  /**< Range at which test pulse is located */
  fl32 test_pulse_length_usec;  /**< Length of test pulse */
  
  fl32 unused[36]; /**< for future expansion */

} iwrf_ts_processing_t;

/************************************************************************/
/**
 * \struct iwrf_xmit_power
 *
 * Measured transmitter power
 *
 ************************************************************************/

typedef struct iwrf_xmit_power {

  si32 packet_id; /**< \ref IWRF_XMIT_POWER_ID */
  si32 len_bytes; /**< length of this structure, in bytes */
  si32 version_num; /**< version number of this structure */
  si32 radar_id; /**< id of radar - in case of multipler radars */
  si64 time_secs_utc;  /**< UTC time in secs since Jan 1 1970 */
  si32 time_nano_secs; /**< partial secs - nanosecs */
  si32 reserved[3]; /**< future expansion */
  
  fl32 power_dbm_h; /**< Peak power in dBm assuming a rect pulse - H.
		     * We use a simple conversion from average power readings to
		     * peak power.\n
		     * \code peak_power = average_power * (pulse_len/PRT) \endcode */
  
  fl32 power_dbm_v; /**< Peak power in dBm assuming a rect pulse - V.
		     * We use a simple conversion from average power readings to
		     * peak power.\n
		     * \code peak_power = average_power * (pulse_len/PRT) \endcode */
  
  si32 unused[20]; /**< for future expansion */

} iwrf_xmit_power_t;

/************************************************************************/
/**
 * \struct iwrf_xmit_sample
 *
 * Measured transmit sample
 *
 ************************************************************************/

typedef struct iwrf_xmit_sample {
  
  si32 packet_id; /**< \ref IWRF_XMIT_SAMPLE_ID */
  si32 len_bytes; /**< length of this structure, in bytes */
  si32 version_num; /**< version number of this structure */
  si32 radar_id; /**< id of radar - in case of multipler radars */
  si64 time_secs_utc;  /**< UTC time in secs since Jan 1 1970 */
  si32 time_nano_secs; /**< partial secs - nanosecs */
  si32 reserved[3]; /**< future expansion */
  
  fl32 power_dbm_h; /**< H peak power in dBm assuming a rect pulse */
  fl32 power_dbm_v; /**< V peak power in dBm assuming a rect pulse */
  si32 offset; /**< Offset from trigger pulse of these samples */
  si32 samples; /**< Number of valid samples */

  fl32 sampling_freq; /**< sampling frequency for transmit pulse - Hz */

  fl32 scale_h;   /**< volts = (sample * scale) + offset - H */
  fl32 offset_h;  /**< volts = (sample * scale) + offset - H */

  fl32 scale_v;   /**< volts = (sample * scale) + offset - V */
  fl32 offset_v;  /**< volts = (sample * scale) + offset - V */

  si32 unused[512]; /**< for future expansion */

  si32 samples_h[IWRF_N_TXSAMP]; /**< Sample of H xmit, at sampling frequency  */
  si32 samples_v[IWRF_N_TXSAMP]; /**< Sample of V xmit, at sampling frequency */

  si32 unused2[493]; /**< for future expansion */

} iwrf_xmit_sample_t;

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
  
  si32 packet_id; /**< \ref IWRF_CALIBRATION_ID */
  si32 len_bytes; /**< length of this structure, in bytes */
  si32 version_num; /**< version number of this structure */
  si32 radar_id; /**< id of radar - in case of multipler radars */
  si64 time_secs_utc;  /**< UTC time in secs since Jan 1 1970 */
  si32 time_nano_secs; /**< partial secs - nanosecs */
  si32 reserved[3]; /**< future expansion */
  
  fl32 wavelength_cm;  /**< radar wavelength in cm */
  fl32 beamwidth_deg_h; /**< Antenna beamwidth, horizontal, in degrees */
  fl32 beamwidth_deg_v; /**< Antenna beamwidth, vertical, in degrees */
  fl32 gain_ant_db_h; /**< computed antenna gain, H pol,
                       *   from ref point through antenna */
  fl32 gain_ant_db_v; /**< computed antenna gain, V pol,
                       *   from ref point through antenna */

  fl32 pulse_width_us;   /**< pulse width in Us */
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

  fl32 zdr_bias_db;  /**< ZDR correction, dB.
		      * Corrected ZDR = measured ZDR + bias */

  fl32 ldr_bias_db_h; /**< LDR correction, dB, H.
		       * Corrected LDR = measured LDR + bias */
  fl32 ldr_bias_db_v; /**< LDR correction, dB, V.
		       * Corrected LDR = measured LDR + bias */

  fl32 phidp_rot_deg; /**< system phipd - degrees */
  
  si32 unused[75]; /**< for future expansion */ 

} iwrf_calibration_t;

/************************************************************************/
/**
 * \struct iwrf_event_notice
 *
 * Signals events such as end of volume, errors, restart etc.
 *
 ************************************************************************/

typedef struct iwrf_event_notice {

  si32 packet_id; /**< \ref IWRF_EVENT_NOTICE_ID */
  si32 len_bytes; /**< length of this structure, in bytes */
  si32 version_num; /**< version number of this structure */
  si32 radar_id; /**< id of radar - in case of multipler radars */
  si64 time_secs_utc;  /**< UTC time in secs since Jan 1 1970 */
  si32 time_nano_secs; /**< partial secs - nanosecs */
  si32 reserved[3]; /**< future expansion */
  
  si32 start_of_sweep; /**< TRUE / FALSE */
  si32 end_of_sweep; /**< TRUE / FALSE */

  si32 start_of_volume; /**< TRUE / FALSE */
  si32 end_of_volume; /**< TRUE / FALSE */
  
  si32 scan_mode; /**< The current scan mode - \ref iwrf_scan_mode */
  si32 follow_mode; /**< The current follow mode - \ref iwrf_follow_mode */
  si32 volume_num; /**< Volume number, increments linearly */
  si32 sweep_num; /**< Sweep number within the current volume */
  
  si32 cause; /**< Additional information about the event -
	       * \ref iwrf_event_notice_cause */

  si32 unused[13]; /**< not used - for later expansion */

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

  si32 packet_id; /**< \ref IWRF_PHASECODE_ID */
  si32 len_bytes; /**< length of this structure, in bytes */
  si32 version_num; /**< version number of this structure */
  si32 radar_id; /**< id of radar - in case of multipler radars */
  si64 time_secs_utc;  /**< UTC time in secs since Jan 1 1970 */
  si32 time_nano_secs; /**< partial secs - nanosecs */
  si32 reserved[3]; /**< future expansion */
  
  si32 seq_length; /**< Number of pulses in sequence */
  iwrf_phase_sample_t phase[IWRF_MAX_PHASE_SEQ_LEN]; /**< Phase sequence */

  fl32 unused[501];

} iwrf_phasecode_t;

/************************************************************************/
/**
 * \struct iwrf_xmit_info
 *
 * Transmitter mode and status
 *
 ************************************************************************/

typedef struct iwrf_xmit_info {
  
  si32 packet_id; /**< \ref IWRF_XMIT_INFO_ID */
  si32 len_bytes; /**< length of this structure, in bytes */
  si32 version_num; /**< version number of this structure */
  si32 radar_id; /**< id of radar - in case of multipler radars */
  si64 time_secs_utc;  /**< UTC time in secs since Jan 1 1970 */
  si32 time_nano_secs; /**< partial secs - nanosecs */
  si32 reserved[3]; /**< future expansion */
  
  si32 xmit_0_enabled; /**< transmitter 0 firing  */
  si32 xmit_1_enabled; /**< transmitter 1 firing  */

  si32 xmit_rcv_mode;   /**< current transmit/receive mode - \ref iwrf_xmit_rcv_mode */
  si32 xmit_phase_mode; /**< current transmit phase mode - \ref iwrf_xmit_phase_mode */
  si32 prf_mode;        /**< current PRF mode - \ref iwrf_prf_mode */
  si32 pulse_type;      /**< current pulse type - \ref iwrf_pulse_type */

  fl32 prt_usec; /**< PRT in microseconds */
  fl32 prt2_usec; /**< Second PRT in microseconds for Dual-PRT mode */

  fl32 unused[14]; /**< for future expansion */
  
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
 *  \li iq-data channel 0
 *  \li iq-data channel 1 (if available)
 *  \li iq-data channel 2 (if available)
 *  \li iq-data channel 3 (if available)
 *
 * len_bytes will be set to the entire packet length.
 *
 ************************************************************************/

typedef struct iwrf_pulse_header {
  
  si32 packet_id; /**< \ref IWRF_PULSE_HEADER_ID */
  si32 len_bytes; /**< length of this structure, plus following data, in bytes */
  si32 version_num; /**< version number of this structure */
  si32 radar_id; /**< id of radar - in case of multipler radars */
  si64 time_secs_utc;  /**< UTC time in secs since Jan 1 1970 */
  si32 time_nano_secs; /**< partial secs - nanosecs */
  si32 struct_len_bytes; /**< length of this structure, in bytes */
  si32 reserved[2]; /**< future expansion */
  
  si64 pulse_seq_num;  /**< pulse sequence number - increments */

  si32 scan_mode; /**< The current scan mode - \ref iwrf_scan_mode */
  si32 follow_mode; /**< The current follow mode - \ref iwrf_follow_mode */
  si32 tilt_num;   /**< scan tilt num - if avail, otherwise set to -1 */ 
  si32 vol_num;    /**< scan vol  num - if avail, otherwise set to -1 */ 

  fl32 target_el;  /**< target elevation for ppis */
  fl32 target_az;  /**< target azimuth for rhis */
  fl32 elevation;    /**< antenna elevation */
  fl32 azimuth;      /**< antenna azimuth */
  
  fl32 prt;   /**< time since previous pulse, secs */
  fl32 prt_next;     /**< time to next pulse, secs, if available */
  
  fl32 pulse_width;   /**< nanosecs */

  si32 n_gates;  /**< number of gates of IQ data */

  si32 n_channels;  /**< number of channels in the IQ data */
  si32 iq_encoding; /**< IWRF_IQ_ENCODING */
  si32 hv_flag;     /**< H, V, SLANT45 - \ref iwrf_pulse_polarization */

  si32 antenna_transition; /**< antenna is in transition
                            * 1 = TRUE, 0 = FALSE */
  
  si32 phase_cohered; /**< received phases are cohered to burst phase
                       * 1 = TRUE, 0 = FALSE.
		       * Applies to
                       * \ref IWRF_XMIT_PHASE_MODE_RANDOM and
                       * \ref IWRF_XMIT_PHASE_MODE_SZ864 */
  
  si32 status; /**< general status flag - optional use */

  si32 n_data; /**< number of data values in the IQ array
                * \code nData = nChannels + (nGates + nGatesBurst) * 2 \endcode */
  
  si32 iq_offset[IWRF_MAX_CHAN];  /**< Index of gate 0 for this chan,
                                  * in IQ[] array */
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
		       * RVP8_NGATES_BURST gates.
                       * For other systems this should normally be 0 */

  si32 unused[14];

} iwrf_pulse_header_t;

/************************************************************************/
/************************************************************************/
/************************************************************************/
/* RVP8-specific support */
/************************************************************************/
/************************************************************************/
/************************************************************************/

#define IWRF_RVP8_GATE_MASK_LEN  512 /**< Length of range mask in RVP8 ops info */
#define IWRF_RVP8_NGATES_BURST 2 /**< number of gates used for storing RVP8 burst pulse */

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

  si32 packet_id; /**< \ref IWRF_RVP8_PULSE_HEADER_ID */
  si32 len_bytes; /**< length of this structure */
  si32 version_num; /**< version number of this structure */
  si32 radar_id; /**< id of radar - in case of multipler radars */
  si64 time_secs_utc;  /**< UTC time in secs since Jan 1 1970 */
  si32 time_nano_secs; /**< partial secs - nanosecs */
  si32 reserved[3]; /**< future expansion */
  
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

  ui32 uiq_perm[2]; /**< User specified bits (Permanent) */
  ui32 uiq_once[2]; /**< User specified bits (One-Shot) */

  si32 i_data_off[IWRF_MAX_CHAN]; /**< Offset of start of this pulse in fIQ[] */
  fl32 f_burst_mag[IWRF_MAX_CHAN];/**< Burst pulse magnitude (amplitude, not power) */
  ui16 i_burst_arg[IWRF_MAX_CHAN];/**< Burst phase changes (PrevPulse - ThisPulse) */
  ui16 i_wrap_iq[IWRF_MAX_CHAN];  /**< Data wraparound count for validity check */

  si32 unused2[29]; /**< not used - for later expansion */

} iwrf_rvp8_pulse_header_t;

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
  
  si32 packet_id; /**< \ref IWRF_RVP8_OPS_INFO_ID */
  si32 len_bytes; /**< length of this structure, in bytes */
  si32 version_num; /**< version number of this structure */
  si32 radar_id; /**< id of radar - in case of multipler radars */
  si64 time_secs_utc;  /**< UTC time in secs since Jan 1 1970 */
  si32 time_nano_secs; /**< partial secs - nanosecs */
  si32 reserved[3]; /**< future expansion */
  
  si32 i_version; /**< Version of this public structure
                   * \li 0: Initial public element layout
                   * \li 1: Added fDBzCalib, iSampleSize, iMeanAngleSync, iFlags, iPlaybackVersion
                   * \li Set to 9999 if no RVP8 processor
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

  si32 unused2[189];

} iwrf_rvp8_ops_info_t;
  
#ifdef __cplusplus
}
#endif

#endif
