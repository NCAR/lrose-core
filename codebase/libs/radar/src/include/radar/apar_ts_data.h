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
 * /file <apar_ts_data.h>
 *
 * Defines for handling apar time series radar data.
 *
 * Mike Dixon, EOL, NCAR, Boulder, CO
 * July 2019
 *
 *********************************************************************/

#ifndef _APAR_TS_DATA_H_
#define _APAR_TS_DATA_H_

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
 * This document describes the time series format for the APAR_TS project.
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

#define APAR_TS_MAX_CHAN 4  /**< Max number of channels in data */
#define APAR_TS_MAX_PRT  4  /**< Max number of PRTs to be used */
#define APAR_TS_MAX_RADAR_NAME 32  /**< Max length of radar name */
#define APAR_TS_MAX_SITE_NAME  32  /**< Max length of site name */
#define APAR_TS_MAX_SEGMENT_NAME 32  /**< Max length of scan segment name */
#define APAR_TS_MAX_PROJECT_NAME 32  /**< Max length of project name */
#define APAR_TS_MAX_FIXED_ANGLES 520 /**< Length of array of fixed angles */
#define APAR_TS_MAX_PHASE_SEQ_LEN 1024  /**< Max length of phase coding sequence array */
#define APAR_TS_VERSION_NAME_LEN 56  /**< Version name length */

/* missing value for meta-data
 * used where value is not known */

#define APAR_TS_MISSING_INT -9999 /**< Missing val for integer values */
#define APAR_TS_MISSING_FLOAT -9999.0F /**< Missing val for float values */ 
#define APAR_TS_MISSING_DOUBLE -9999.0 /**< Missing val for double values */ 
#ifdef NOTNOW
#define APAR_TS_MISSING_FLOAT NAN /**< Missing val for float values */
#endif

/* packet IDs */

#define APAR_TS_SYNC_ID 0x55550001 /**< ID for sync packet */
#define APAR_TS_RADAR_INFO_ID 0x55550002 /**< ID for radar_info packet */
#define APAR_TS_SCAN_SEGMENT_ID 0x55550003 /**< ID for scan segment packet */
#define APAR_TS_PROCESSING_ID 0x55550004 /**< ID for time-series processing packet */
#define APAR_TS_CALIBRATION_ID 0x55550005 /**< ID for calibration packet */
#define APAR_TS_EVENT_NOTICE_ID 0x55550006 /**< ID for event notice packet */

#define APAR_TS_PULSE_HEADER_ID 0x55550007 /**< ID for pulse header packet */
#define APAR_TS_VERSION_ID 0x55550008 /**< ID for version packet */
#define APAR_TS_STATUS_XML_ID 0x55550009 /**< ID for status in XML format */
  
#define APAR_TS_PLATFORM_GEOREF_ID 0x5555000a /**< ID for moving platform georeference */
#define APAR_TS_GEOREF_CORRECTION_ID 0x5555000b /**< ID for moving platform georeference */

#define APAR_TS_SYNC_VAL_00 0x2a2a2a2a /**< Value for first word in sync packet */
#define APAR_TS_SYNC_VAL_01 0x7e7e7e7e /**< Value for second word in sync packet */

/* event flags */

typedef si32 apar_ts_event_flags_t;
#define APAR_TS_END_OF_SWEEP (1 << 0)
#define APAR_TS_END_OF_VOLUME (1 << 1)
#define APAR_TS_START_OF_SWEEP (1 << 2)
#define APAR_TS_START_OF_VOLUME (1 << 3)

/************************************************************************/
/************************************************************************/
/************************************************************************/
/* ENUMERATED TYPES */
/************************************************************************/
/************************************************************************/
/************************************************************************/

typedef enum {
  APAR_TS_DEBUG_OFF = 0,
  APAR_TS_DEBUG_NORM,
  APAR_TS_DEBUG_VERBOSE,
  APAR_TS_DEBUG_EXTRA
} AparTsDebug_t;

/************************************************************************/
/**
 * \enum apar_ts_prf_mode
 *
 * PRF mode - fixed or dual-prt.
 *
 ************************************************************************/

typedef enum apar_ts_prf_mode {

  APAR_TS_PRF_MODE_NOT_SET = 0,
  APAR_TS_PRF_MODE_FIXED = 1, /**< fixed pulsing mode */
  APAR_TS_PRF_MODE_STAGGERED_2_3 = 2, /**< staggered PRT, 2/3 ratio */
  APAR_TS_PRF_MODE_STAGGERED_3_4 = 3, /**< staggered PRT, 3/4 ratio */
  APAR_TS_PRF_MODE_STAGGERED_4_5 = 4, /**< staggered PRT, 4/5 ratio */
  APAR_TS_PRF_MODE_MULTI_PRT = 5,     /**< staggered mode, more than 2 PRT's */
  APAR_TS_PRF_MODE_LAST /**< not used */

} apar_ts_prf_mode_t;

/************************************************************************/
/**
 * \enum apar_ts_pulse_type
 *
 ************************************************************************/

typedef enum apar_ts_pulse_shape {

  APAR_TS_PULSE_SHAPE_NOT_SET = 0,
  APAR_TS_PULSE_SHAPE_RECT = 1, /**< rectangular pulse */
  APAR_TS_PULSE_SHAPE_GAUSSIAN = 2, /**< gaussian-weighted pulse */
  APAR_TS_PULSE_SHAPE_CUSTOM = 3, /**< custom- downloaded to waveform generator */
  APAR_TS_PULSE_SHAPE_LAST /**< not used */
  
} apar_ts_pulse_shape_t;

/************************************************************************/
/**
 * \enum apar_ts_pol_mode - polarization
 *
 ************************************************************************/

typedef enum apar_ts_pol_mode {
  
  APAR_TS_POL_MODE_NOT_SET = 0,
  APAR_TS_POL_MODE_H = 1, /**< H pulse */
  APAR_TS_POL_MODE_V = 2, /**< V pulse */
  APAR_TS_POL_MODE_MIXED = 3, /**< Mixture of H and V */
  APAR_TS_POL_MODE_LAST /**< not used */
  
} apar_ts_pol_mode_t;

/************************************************************************/
/**
 * \enum apar_ts_scan_mode
 *
 * Antenna scanning mode - these are legacy NCAR codes, with the CHILL
 * FIXED, MANPPI and MANRHI appended
 *
 ************************************************************************/

typedef enum apar_ts_scan_mode {

  APAR_TS_SCAN_MODE_NOT_SET = 0,
  APAR_TS_SCAN_MODE_PPI = 1, /**< PPI sector */
  APAR_TS_SCAN_MODE_RHI = 2, /**< RHI vertical slice */
  APAR_TS_SCAN_MODE_COPLANE = 3, /**< co-plane dual doppler mode */
  APAR_TS_SCAN_MODE_VPOINT = 4, /**< vertical pointing for calibration */
  APAR_TS_SCAN_MODE_SUNSCAN = 5, /**< scanning the sun for calibrations */
  APAR_TS_SCAN_MODE_POINTING = 6, /**< fixed pointing */
  APAR_TS_SCAN_MODE_IDLE = 7, /**< between scans */
  APAR_TS_SCAN_MODE_LAST /**< not used */

} apar_ts_scan_mode_t;

/************************************************************************/
/**
 * \enum apar_ts_radar_platform
 *
 * The type of platform on which the radar is mounted.
 *
 ************************************************************************/

typedef enum apar_ts_radar_platform {

  APAR_TS_RADAR_PLATFORM_NOT_SET = 0,
  APAR_TS_RADAR_PLATFORM_FIXED = 1, /**< Radar is in a fixed location */
  APAR_TS_RADAR_PLATFORM_VEHICLE = 2, /**< Radar is mounted on a land vehicle */
  APAR_TS_RADAR_PLATFORM_SHIP = 3, /**< Radar is mounted on a ship */
  APAR_TS_RADAR_PLATFORM_AIRCRAFT = 4, /**< Radar is mounted on an aircraft */
  APAR_TS_RADAR_PLATFORM_LAST /**< not used */

} apar_ts_radar_platform_t;

/************************************************************************/
/**
 * \enum apar_ts_iq_encoding
 *
 * How the IQ data is encoded and packed.
 *
 ************************************************************************/

typedef enum apar_ts_iq_encoding {

  APAR_TS_IQ_ENCODING_NOT_SET = 0,

  APAR_TS_IQ_ENCODING_FL32 = 1, /**< 4-byte floats, volts */

  APAR_TS_IQ_ENCODING_SCALED_SI16 = 2, /**< voltages scaled as signed 16-bit ints,<br>
				     * volts = (si16 * scale) + offset */
  
  APAR_TS_IQ_ENCODING_DBM_PHASE_SI16 = 3, /**< 16-bit signed ints,
					* storing power_dbm and phase,<br>
					* power_dbm = (si16 * scale) + offset<br>
					* phase = (si16 * 360.0) / 65536.0  */
  
  APAR_TS_IQ_ENCODING_SCALED_SI32 = 5, /**< voltages scaled as signed 32-bit ints,<br>
                                        * volts = (si32 * scale) + offset */
  
  APAR_TS_IQ_ENCODING_LAST /**< not used */

} apar_ts_iq_encoding_t;

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
 * \struct apar_ts_packet_info
 *
 * Struct included at top of all other structs.
 * packet_id is set to the id relevant to that packet type.
 *
 ************************************************************************/

typedef struct apar_ts_packet_info {
    
  si32 id; /**< Id for the packet type e.g. APAR_TS_RADAR_INFO_ID */
  si32 len_bytes; /**< length of packet structure, in bytes,
		   ** except for status_xml and pulse_header,
                   ** in which len_bytes
                   **    = length of structure, plus following data */
  si64 seq_num;  /**< packet sequence number */
  si32 version_num; /**< version number of this structure */
  si32 radar_id; /**< id of radar - in case of multiple radars */
  si64 time_secs_utc;  /**< UTC time in secs since Jan 1 1970 */
  si32 time_nano_secs; /**< partial secs - nanosecs */
  si32 reserved[7]; /**< future expansion */

} apar_ts_packet_info_t;

/************************************************************************/
/**
 * \struct apar_ts_sync
 *
 * Synchronization packet.
 * Sent regularly to resynchronize data stream if needed.
 *
 * The magik[2] array will contain the following:
 *
 *   <br>APAR_TS_SYNC_VAL_00  = 0x2a2a2a2a		ascii: ****
 *   <br>APAR_TS_SYNC_VAL_01  = 0x7e7e7e7e		ascii: ~~~~
 *
 ************************************************************************/

typedef struct apar_ts_sync {
    
  apar_ts_packet_info_t packet; /*< packet_id = APAR_TS_SYNC_ID */

  si32 magik[16]; /**< array of sync values */
  
} apar_ts_sync_t;

/************************************************************************/
/**
 * \struct apar_ts_version
 *
 * Version packet.
 *
 ************************************************************************/

typedef struct apar_ts_version {
    
  apar_ts_packet_info_t packet; /*< packet_id = APAR_TS_VERSION_ID */

  si32 major_version_num;
  si32 minor_version_num;

  char version_name[APAR_TS_VERSION_NAME_LEN];
  
} apar_ts_version_t;

/************************************************************************/
/**
 * \struct apar_ts_radar_info
 *
 * Fixed radar-specific information.
 *
 * NOTE: calibration information is in calibration.
 *
 ************************************************************************/

typedef struct apar_ts_radar_info {
    
  apar_ts_packet_info_t packet; /*< packet_id = APAR_TS_RADAR_INFO_ID */
  
  fl64 latitude_deg;  /**< latitude, in degrees */
  fl64 longitude_deg; /**< longitude, in degrees */
  fl32 altitude_m;    /**< altitude, in meters */
  si32 platform_type; /**< platform type - \ref apar_ts_radar_platform */

  fl32 beamwidth_deg_h; /**< Antenna beamwidth, horizontal, in degrees */
  fl32 beamwidth_deg_v; /**< Antenna beamwidth, vertical, in degrees */
  fl32 wavelength_cm;   /**< Radar wavelength, in centimeters */
  
  fl32 nominal_gain_ant_db_h; /**< nominal antenna gain, H */
  fl32 nominal_gain_ant_db_v; /**< nominal antenna gain, V */

  si32 unused[21]; /**< for future expansion */
  
  char radar_name[APAR_TS_MAX_RADAR_NAME]; /**< UTF-8 encoded radar name */
  char site_name[APAR_TS_MAX_SITE_NAME];   /**< UTF-8 encoded radar name */

} apar_ts_radar_info_t;

/************************************************************************/
/**
 * \struct apar_ts_scan_segment
 *
 * Scanning strategy - scan segment or volume
 *
 ************************************************************************/

typedef struct apar_ts_scan_segment

{

  apar_ts_packet_info_t packet; /*< packet_id = APAR_TS_SCAN_SEGMENT_ID */
  
  si32 scan_mode; /**< The current scan mode - \ref apar_ts_scan_mode */
  si32 volume_num; /**< Volume number, increments linearly */
  si32 sweep_num; /**< Sweep number within the current volume */

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
  si32 n_sweeps;  /**< Indicates that a set of discrete angles
                   *   is specified for az or el */
  fl32 fixed_angles[APAR_TS_MAX_FIXED_ANGLES];
  
  /** Sun Scan sector widths */

  fl32 sun_scan_sector_width_az; /**< sector width (deg) when follwing sun */
  fl32 sun_scan_sector_width_el; /**< sector width (deg) when follwing sun */

  si32 unused[456]; /**< for future expansion */
  
  char segment_name[APAR_TS_MAX_SEGMENT_NAME]; /**< Name of this scan segment */
  char project_name[APAR_TS_MAX_PROJECT_NAME]; /**< Project name */

} apar_ts_scan_segment_t;

/************************************************************************/
/**
 * \struct apar_ts_processing
 *
 * Time-series-specific processing.
 *
 * This packet contains the transmit modes, digitization in range and
 * hints about how to compute moments from the time series.
 *
 ************************************************************************/

typedef struct apar_ts_processing {

  apar_ts_packet_info_t packet; /*< packet_id = APAR_TS_PROCESSING_ID */
  
  si32 pol_mode;        /**< current polarization mode - 
                         *   \ref apar_ts_pol_mode */

  si32 prf_mode;        /**< current PRF mode -
                         *   \ref apar_ts_prf_mode */

  si32 pulse_shape;      /**< current pulse type -
                         *   \ref apar_ts_pulse_shape */

  fl32 pulse_width_us;   /**< pulse width in microsecs */
  
  fl32 start_range_m;  /**< Range to start of first gate (m) */
  fl32 gate_spacing_m; /**< Gate spacing (m) */

  fl32 test_pulse_range_km;     /**< Range at which test pulse is located */
  fl32 test_pulse_length_us;    /**< Length of test pulse */
  
  si32 num_prts;       /**< number of prt's used
                        * allowed values 1 through APAR_TS_MAX_PRT */
  fl32 prt_us[APAR_TS_MAX_PRT]; /**< PRT in microseconds */

  si32 unused[35];     /**< for future expansion */
  
} apar_ts_processing_t;

/************************************************************************/
/**
 * \struct apar_ts_status_xml
 *
 * Status as ASCII in XML format.
 *
 * This struct is followed by an ASCII buffer, length xml_len.
 * The ASCII XML buffer should be null-terminated.
 * The contents are free-form. Any suitable status information
 * may be included in the XML buffer.
 *
 * packet.len_bytes = sizeof(apar_ts_packet_info_t) + xml_len.
 *
 ************************************************************************/

typedef struct apar_ts_status_xml {
  
  apar_ts_packet_info_t packet; /*< packet_id = APAR_TS_STATUS_XML */
  
  si32 xml_len; /**< Length of XML buffer in bytes
                 * This should include the trailing NULL */
  si32 unused[15]; /**< for future expansion */

} apar_ts_status_xml_t;

/************************************************************************/
/**
 * \struct apar_ts_calibration
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

typedef struct apar_ts_calibration {
  
  apar_ts_packet_info_t packet; /*< packet_id = APAR_TS_CALIBRATION_ID */
  
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
			 * baseDbz1km value must be used as is. */

  fl32 base_dbz_1km_hx; /**< base reflectivity at 1km, for SNR = 0, H cross-polar.
			 * To compute dBZ:
			 * \code dBZ = baseDbz1km + SNR + 20log10(rangeKm) + (atmosAtten * rangeKm) \endcode
			 * BaseDbz1km can be computed as follows:
			 * \code baseDbz1km = noiseDbm - receiverGainDb - radarConst \endcode
			 * However, sometimes these values are not available, and the
			 * baseDbz1km value must be used as is */

  fl32 base_dbz_1km_vc; /**< base reflectivity at 1km, for SNR = 0, V co-polar.
			 * To compute dBZ:
			 * \code dBZ = baseDbz1km + SNR + 20log10(rangeKm) + (atmosAtten * rangeKm) \endcode
			 * BaseDbz1km can be computed as follows:
			 * \code baseDbz1km = noiseDbm - receiverGainDb - radarConst \endcode
			 * However, sometimes these values are not available, and the
			 * baseDbz1km value must be used as is. */

  fl32 base_dbz_1km_vx; /**< base reflectivity at 1km, for SNR = 0, V cross-polar.
			 * To compute dBZ:
			 * \code dBZ = baseDbz1km + SNR + 20log10(rangeKm) + (atmosAtten * rangeKm) \endcode
			 * BaseDbz1km can be computed as follows:
			 * \code baseDbz1km = noiseDbm - receiverGainDb - radarConst \endcode
			 * However, sometimes these values are not available, and the
			 * baseDbz1km value must be used as is. */

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

  si32 unused[47]; /**< for future expansion */ 

  char radar_name[APAR_TS_MAX_RADAR_NAME]; /**< name of instrument */

} apar_ts_calibration_t;

/************************************************************************/
/**
 * \struct apar_ts_event_notice
 *
 * Signals events such as end of volume, errors, restart etc.
 *
 ************************************************************************/

typedef struct apar_ts_event_notice {

  apar_ts_packet_info_t packet; /*< packet_id = APAR_TS_EVENT_NOTICE_ID */
  
  si32 start_of_sweep; /**< TRUE / FALSE */
  si32 end_of_sweep; /**< TRUE / FALSE */

  si32 start_of_volume; /**< TRUE / FALSE */
  si32 end_of_volume; /**< TRUE / FALSE */
  
  si32 scan_mode; /**< The current scan mode - \ref apar_ts_scan_mode */
  si32 volume_num; /**< Volume number, increments linearly */
  si32 sweep_num; /**< Sweep number within the current volume */
  
  fl32 current_fixed_angle; /**< Current fixed angle
                             *   (az in RHI, el in PPI) */

  si32 unused[40]; /**< not used - for later expansion */

} apar_ts_event_notice_t;

/************************************************************************/
/**
 * \struct apar_ts_pulse_header
 *
 * Pulse header.
 * This header precedes the IQ data.
 *
 * A pulse data packet is made up as follows:
 * 
 *  \li apar_ts_pulse_header
 *  \li iq-data channel 0 (H or co-polar)
 *  \li iq-data channel 1 (V or x-polar) (if available)
 *  \li iq-data channel 2 (if available)
 *  \li iq-data channel 3 (if available)
 *
 * len_bytes will be set to the entire packet length.
 *   len_bytes = length of structure, plus following data
 *
 ************************************************************************/

typedef struct apar_ts_pulse_header {
  
  apar_ts_packet_info_t packet; /*< packet_id = APAR_TS_PULSE_HEADER_ID *
                                 * len_bytes = length of this structure,
                                 * plus following data, in bytes */
  
  si64 pulse_seq_num;  /**< pulse sequence number since starting receiver */
  si64 dwell_seq_num;  /**< dwell sequence number - increments every dwell */

  si32 beam_num_in_dwell;  /**< beam sequence number since start of dwell */
  si32 visit_num_in_beam;  /**< visit sequence number since start of beam */

  si32 scan_mode; /**< The current scan mode - \ref apar_ts_scan_mode */
  si32 volume_num;  /**< scan vol  num - if avail, otherwise set to -1 */ 
  si32 sweep_num;   /**< scan tilt num - if avail, otherwise set to -1 */ 

  fl32 elevation;   /**< antenna elevation */
  fl32 azimuth;     /**< antenna azimuth */
  fl32 fixed_angle; /**< target az/el in sweep */
  
  fl32 prt;         /**< time since previous pulse, secs */
  fl32 prt_next;    /**< time to next pulse, secs, if available */
  
  fl32 pulse_width_us;  /**< microsecs */

  si32 n_gates;  /**< number of gates of IQ data */
  fl32 start_range_m; /**< Range to start of first gate, (in meters) */
  fl32 gate_spacing_m; /**< Gate spacing (in meters) */

  si32 hv_flag;     /**< 1 if H, 0 if V for alternating mode
                     **< 3 if simultaneous H/V */

  si32 phase_cohered; /**< received phases are cohered to transmit phase
                       * 1 = TRUE, 0 = FALSE.
		       * Applies to phase coded operation */
  
  si32 iq_encoding; /**< APAR_TS_IQ_ENCODING */
  si32 n_channels;  /**< number of channels in the IQ data */
  si32 n_data; /**< number of data values in the IQ array
                * \code nData = nChannels * nGates * 2 \endcode */
  
  fl32 scale;   /**< for use with APAR_TS_IQ_ENCODING types
                 * to convert to/from floats */
  fl32 offset;  /**< for use with APAR_TS_IQ_ENCODING types
                 * to convert to/from floats */

  si32 chan_is_copol[APAR_TS_MAX_CHAN]; /**< channel copol flag
                                         * -1 = not set,
                                         *  1 = copol, 0 = crosspol */

  si32 status; /**< general status flag - optional use */

  si32 event_flags; /**< flags from end of volume, start of tilt etc.
                     * See apar_ts_event_flags_t */

  si32 unused[80];

} apar_ts_pulse_header_t;

/************************************************************************/
/**
 * \struct platform_georef
 *
 * Georeference information for moving platforms
 *
 ************************************************************************/

typedef struct apar_ts_platform_georef {
    
  apar_ts_packet_info_t packet; /*< packet_id = APAR_TS_PLATFORM_GEOREF_ID */
  
  fl64 longitude;           /**< Antenna longitude (Eastern
                             * Hemisphere is positive, West
                             * negative) in degrees */
  fl64 latitude;            /**< Antenna Latitude (Northern
                             * Hemisphere is positive, South
                             * Negative) in degrees */

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
  fl32 track_deg;           /** track over the ground */
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
  
  fl32 roll_rate_dps;       /**< roll angle rate in degrees/second. */

  si32 unused[24]; /**< for future expansion */
  
} apar_ts_platform_georef_t;

/************************************************************************/
/**
 * \struct georef_correction
 *
 * Corrections to georeference information
 *
 ************************************************************************/
  
typedef struct apar_ts_georef_correction {
    
  apar_ts_packet_info_t packet; /*< packet_id = APAR_TS_GEOREF_CORRECTION_ID */
  
  fl32 longitude_corr_deg;    /**< Correction added to radar longitude */
  fl32 latitude_corr_deg;     /**< Correction added to radar latitude */

  fl32 azimuth_corr_deg;      /**< Correction added to azimuth[deg] */
  fl32 elevation_corr_deg;    /**< Correction added to elevation[deg] */

  fl32 range_delay_corr_mps;  /**< Correction used for range delay[m] */
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
  
  si32 unused[32];            /**< for future expansion */
  
} apar_ts_georef_correction_t;

#ifdef __cplusplus
}
#endif

#ifdef WIN32
#pragma pack(pop)
#endif

#endif

