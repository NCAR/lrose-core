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
/*

	CHILL data processor data type defines
	
*/

/**
\file chill_types.h
\author Jim George
\brief CHILL data processor data type defines.

This file includes all the CHILL inter-process communication
structures and constants. Also included are the data structures used
in archived data.
*/ 

/**
\page sdb Standard Data Buffers
\author Jim George
\section sdb_endianness Endianness of SDB structure elements 
Since most of the CHILL hardware runs on Intel-compatible (x86) machines,
all the binary data is stored in little-endian format. An important
exception to this is when data is sent to the Java VCHILL display (see \ref vchill),
which makes use of big-endian data. Structures must be converted to network byte
ordering before being sent to VCHILL.

When reading SDB formatted streams and
files on big-endian hardware (eg: SPARC or PowerPC), ensure that the bytes
within words are reversed.

\section sdb_reading Reading SDB structs from a stream:
 - First read the ID and length
 - Read the length paramter to determine how many more bytes to read
 - Read the extra bytes, if enough memory is allocated, else bail
 - Check to see if the type ID matches one of the SDB structures
   that have trailing data, such as MOM_ID_SDB
 - Read in the trailing data  

\section sdb_fmt General format of streams/files
The standard data buffer format is evolving, and streaming data may not
be backwards compatible. For this reason, check the SDB version header
(see sdb_version_hdr) before reading the stream. Note that this only
affects the streaming SDB, not archived data. Changes that can break
backward compatibility include adding new fields to gate_mom_data, which
is not used in archive files. For details of the version history, see
\ref sdb_fmt_hist

In general, packets can arrive in any order, with no strictly defined
sequence. Hence, no value should be attached to the arrival of a packet,
with an important exception: HSK_ID_EVENT_NOTICE.
Arrival of this packet signals some time-bound event such as a scan
start or end.

Typically, most of the housekeeping (HSK_ID_*) packets are sent out at
the start of a volume. Programs can cache this information for later
reference. Archivers can also write out the last-seen values when they
start writing a new file. This ensures that all output files will
contain the required housekeeping data.

Data packets typically arrive in the following order:
 -# Version
 -# Radar Info
 -# Processor Info
 -# Scan segment
 -# ...
 -# Ray header + ray data
 -# Power updates approx. every 2 seconds
 -# ...
 -# End notice 
*/

/**
\mainpage CSU-CHILL signal processor and display system
This is the documentation for the CSU-CHILL signal processor
and display system.

- \subpage acq_svr
- \subpage moment_svr
- \subpage archiver
- \subpage sdb
- \subpage data_arch_fmt
- \subpage txctrl_api
- \subpage vchilld
- \subpage vchill

*/

#ifndef _CHILL_TYPES_H_
#define _CHILL_TYPES_H_

#define MAX_DOP_FILTERS	14

#ifdef __cplusplus

#include <complex>

using std::complex;
typedef complex<float>		float_complex;
typedef complex<double>		double_complex;
typedef complex<long double>	long_double_complex;

#else

#include <complex.h>

typedef float complex float_complex;
typedef double complex double_complex;
typedef long double complex long_double_complex;

#endif

#ifndef SGN
#define SGN(x) ((x<0)?-1:1)
#endif

#ifndef M_SQRT3
#define M_SQRT3 1.73205080756887729353
#endif

#ifndef DBS
#define DBS(x) (10.0*log10(x))
#endif

#ifndef IDBS
#define IDBS(x) (pow(10.0, (x)/10.0))
#endif

#ifndef RADIAN_TO_DEG
#define RADIAN_TO_DEG(x) (57.2957795130823208768*(x))
#endif

#ifndef DEG_TO_RADIAN
#define DEG_TO_RADIAN(x) (0.01745329251994329577*(x))
#endif

#define N_TXSAMP 512

/* TODO: fix this */
#define MAX_GATE	2044
#define MAX_GATE_OV	((MAX_GATE+HDR_SIZE)*5-HDR_SIZE)

/***************************
 *    Port numbers used    *
 ***************************/
/* Note: these are only for default values. Servers & clients should
   use configurable port numbers */
#define ACQ_SVR_PORT	2112
#define VCHILL_PORT	2510
#define TXCTL_PORT	2110
#define SDB_PORT	2111
#define RT_VCHILL_PORT	2511
#define VT_PORT         2113
#define TRACK_PORT	2114

/*******************************************
 *    Timeseries stream data structures    *
 ******************************************/

#define TS_MAGIC        0xABCD0000
#define HS_MAGIC        0xABCD0001
#define HS2_MAGIC	0xABCD0002
/**\brief Generic packet header, which only defines the magic word (ID)
and the payload_length fields. Payload length is defined as the number
of quad-words (8 bytes) of data that will follow the packet header.
    
This is customized to fit the needs of various packet types as needed
*/
typedef struct generic_packet_header
{
	unsigned int filler1;           /**< Unused */
	unsigned int magic_word;	/**< can be *_MAGIC, see table above */
	
	unsigned int filler2;           /**< Unused */
	unsigned int filler3;           /**< Unused */

	unsigned int filler4;           /**< Unused */
	unsigned short payload_length;  /**< Length of payload */
	unsigned short filler5;         /**< Unused */
	
	unsigned int filler6;           /**< Unused */
	unsigned int filler7;           /**< Unused */
} generic_packet_header_t;

/** \brief Timeseries data packet header.
Found at the head of every timeseries data packet.
*/
typedef struct timeseries_packet_header
{
	unsigned short  antenna_elevation;      /**< Antenna elevation, 0 -> 0, 360 -> 65536 */
	unsigned short  antenna_azimuth;        /**< Antenna azimuth, 0 -> 0, 360 -> 65536 */
	unsigned int    magic_word;		/**< ID: TS_MAGIC*/

	unsigned int    timestamp;              /**< 25 ns resolution timestamp, reset to zero every second */
	unsigned int    hdr_word3;              /**< bits 16-0: GPS time-of-day (seconds since midnight)\n
	                                             bits 18-17: Indicate polarization state (V/H transmit)\n
	                                             bit 19: End of integration cycle\n
	                                             bits 22-20: Not used\N
	                                             bits 31-23: Pulse counter (phase sequence indicator) */

	unsigned int    hdr_word6;              /***/
	unsigned short  gatecount;              /**< Number of gates in this pulse. Structure of a gate is
	                                             given in timeseries_sample_t */
	unsigned short  hdr_word5;              /**< Indicates the first gate # in this pulse. Used by ts archiver. */

	unsigned int    hdr_word8;              /**<  */
	unsigned int    hdr_word7;              /**< UNIX time (seconds since midnight, Jan 1 1970) of 
	                                             12:00 AM today. Add this to the timestamp to get the
	                                             true UNIX time. Currently this is set by the acq_svr,
	                                             and not derived from GPS. */
} timeseries_packet_header_t;

/*
    Convenience structure, with the timeseries itself tacked on to the end
*/
typedef struct timeseries_packet
{
	struct timeseries_packet_header hdr;
	unsigned long long timeseries[1];
} timeseries_packet_t;

/** \brief Structure of each time-series gate.
*/
typedef struct timeseries_sample
{
	signed short q2; /**<  Q: Channel 2 - H or copol */
	signed short i2; /**<  I: Channel 2 - H or copol */
	signed short q1; /**<  Q: Channel 1 - V or crosspol */
	signed short i1; /**<  I: Channel 1 - V or crosspol */
} timeseries_sample_t;

/** \brief Housekeeping encapsulation header
This header encapsulates the housekeeping data embedded within a 
timeseries file or stream. The payload for this packet is one of
the SDB housekeeping packets (such as radar_info_t).
*/
typedef struct housekeeping_packet_header
{
	unsigned int    event_code;   /**< Not used */
	unsigned int    magic_word;   /**< ID: HS2_MAGIC */

	unsigned int    txmit_power_V;  /**< Not used */
	unsigned int    txmit_power_H;  /**< Not used */

	unsigned int    phidp_rotation; /**< Not used */
	unsigned short  data_len; /**< payload length in BYTES, not DWORDS */
	unsigned short  vol_termination_code; /**< Not used */

	unsigned int    par_rec_rng_offset_mm;  /**< Not used */
	unsigned int    hdr_word8; /**< Not used */
} housekeeping_packet_header_t;

/*
    Convenience structure, with the old housekeeping stuff tacked on the end
*/
/*
typedef struct housekeeping_packet
{
	struct housekeeping_packet_header hdr;
	struct HSK_SERVER_PACKET hspk;  
} housekeeping_packet_t;
*/

/**
\brief Time series request mode
*/
typedef enum ts_req_mode {
	CREQ_MODE_TIMESERIES = 0, /**< Mode for non-oversampled data */
	CREQ_MODE_HOUSEKEEPING, /**< Housekeeping data only mode */
	CREQ_MODE_OVERSAMPLED /**< Oversampled data mode */
} ts_req_mode_t;

#define CREQ_MAGIC        0xDCBA0000
#define CREQ_CMD_SET_MODE 0x00000001

/**\brief Acquisition server request packet */
typedef struct client_request_packet
{
	unsigned int    magic_word; /**< ID: DCBA0000 */
	unsigned int    command; /**< Command sent to the acq server -- see CREQ_CMD_* in acq_svr_defs.h */
	unsigned int    data[2]; /**< Data associated with command */
} client_request_packet_t;

/**************************************
 *    Housekeeping data structures    *
 **************************************/

/*
This is the largest likely houskeeping pkt size (bytes)
It's only used for memory allocation, so it can be
much larger than required.
*/
#define MAX_HSK_SIZE			3000

#define HSK_ID_RADAR_INFO 0x5AA50001
#define HSK_ID_SCAN_SEG 0x5AA50002
#define HSK_ID_PROCESSOR_INFO 0x5AA50003
#define HSK_ID_PWR_UPDATE 0x5AA50004
#define HSK_ID_EVENT_NOTICE 0x5AA50005
#define HSK_ID_CAL_TERMS 0x5AA50006
#define HSK_ID_VERSION 0x5AA50007
#define HSK_ID_XMIT_INFO 0x5AA50008
#define HSK_ID_TRACK_INFO 0x5AA50009
#define HSK_ID_ANT_OFFSET 0x5AA5000A
#define HSK_ID_XMIT_SAMPLE 0x5AA5000B
#define HSK_ID_PHASE_CODE 0x5AA5000C
#define HSK_ID_FILTER_INFO 0x5AA5000D

#define DAT_ID_CHILL_AIQ 0x5AA50016
#define DAT_ID_PAWNEE_AIQ 0x5AA50017

#define MAX_SEGNAME_LENGTH 16
#define MAX_PRJNAME_LENGTH 16

/* Scan flags */
#define SF_LAST_SEG_INVOL		(1)
#define SF_SECTOR			(1 << 1)
#define SF_INIT_DIRECTION_CW		(1 << 2)
#define SF_INIT_DIRECTION_CCW		(1 << 3)
#define SF_TIME_SERIES_RECORD_ENABLE    (1 << 4)
#define SF_RECORD_ENABLE_1		(1 << 5)
#define SF_RECORD_ENABLE_2		(1 << 6)
#define SF_RECORD_ENABLE_3		(1 << 7)
#define SF_SELECT_XMIT_2		(1 << 8)

#define MAX_RADAR_NAME 32

/* Possible gate spacing values */
#define GATE_SPACING_DEFAULT 150.0
#define GATE_SPACING_OVERSMP 30.0

/* Threshold at which to decide if oversampled mode is used */
#define OVERSMP_THRESHOLD 31.0

/**
\brief Generic SDB structure.

This is used only for memory allocation and file/socket I/O operations.
You can typecast to other blocks, based on the ID 

An SDB structure always starts with the same two 32-bit ints.
The ID indicates the type of packet. IDs are defined in the chill_types.h
include file, each different type of header has a different ID. Although
not strictly enforced, the ID may be split into a low-word and high-word,
where the high-word indicates the category of packet, and the low-word
indicates the actual packet type itself. 

The length parameter stores the length of the entire structure (including
the ID and length fields). This *must* be filled in before storing any
SDB structure to disk or sending it over the network.

*/
typedef struct generic_hsk
{
	int id;
	int length;

	int data[MAX_HSK_SIZE - 2 * sizeof(int)];
} generic_hsk_t;

/*
    Another convenience strucure, for the 2006+ housekeeping packets
*/
typedef struct housekeeping_packet2
{
  struct housekeeping_packet_header hdr;
  generic_hsk_t hsk_data;
} housekeeping_packet2_t;

/** \brief Contains general radar info and fixed calibration terms
*/
typedef struct radar_info
{
	int id; /**< HSK_ID_RADAR_INFO */
	int length; /**< Structure length */
 
	char radar_name[MAX_RADAR_NAME]; /**< UTF-8 encoded radar name */
	float latitude_d; /**< Current latitude, in degrees */
	float longitude_d; /**< Current longitude, in degrees */
	float altitude_m; /**< Current altitude, in meters */
	float beamwidth_d; /**< Antenna beamwidth, in degrees */
	float wavelength_cm; /**< Radar wavelength, in centimeters */
	
	/* gains are moved to the cal_terms structure */
	/* float gain_v_rx_1_db; */ /* channel 1 gain when xfer switch is off */
	/* float gain_h_rx_2_db; */ /* channel 2 gain when xfer switch is off */
	/* float gain_v_rx_2_db; */ /* channel 1 gain when xfer switch is on */
	/* float gain_h_rx_1_db; */ /* channel 2 gain when xfer switch is on */
	float unused1; /**< Reserved field */
	float unused2; /**< Reserved field */
	float unused3; /**< Reserved field */
	float unused4; /**< Reserved field */
	
	float gain_ant_h_db; /**< Antenna gain, H pol, from ref point through antenna */
	float gain_ant_v_db; /**< Antenna gain, V pol, from ref point through antenna */
	float zdr_cal_base_db; /**< Operator-settable ZDR cal base, in dB */
	float phidp_rot_d; /**< Operator-settable phidp rotation, in degrees */
	float base_radar_constant_db; /**< Used to calculate dBZ, using
				dBz = base_radar_const_db - pk_txmit_power - 2*ant_gain -
					reciever_gain + dbu + 20*log(range/100km)
				where dbu = 10*log10(i^2 + q^2)    */
	float unused5; /**< Not used, must be set to 0.0 */
	float power_measurement_loss_h_db;  /**< Loss from ref. point power to power meter sensor, H channel */
	float power_measurement_loss_v_db;  /**< Loss from ref. point power to power meter sensor, V channel */
	float zdr_cal_base_vhs_db; /**< Operator-settable ZDR cal base for VHS mode */
	float test_power_h_db;	/**< Power into directional coupler when test set commanded to 0 dBm, H channel */
	float test_power_v_db;	/**< Power into directional coupler when test set commanded to 0 dBm, V channel */
	float dc_loss_h_db;	/**< Directional coupler forward loss, V channel */
	float dc_loss_v_db;	/**< Directional coupler forward loss, V channel */
} radar_info_t;

/**
\brief Scan Optimizer parameters
I am a poor orphaned struct. Please describe me...
*/
typedef struct scan_optimizer
{
	float rmax_km;
	float htmax_km;
	float res_m;
} scan_optimizer_t;

/**
\brief Modes in which the antenna can scan
*/
typedef enum scan_types
{
	SCAN_TYPE_PPI = 0, /**< PPI (Plan Position Indicator) Mode */
	SCAN_TYPE_RHI, /**< RHI (Range Height Indicator) Mode */
	SCAN_TYPE_FIXED, /**< Fixed pointing angle mode */
	SCAN_TYPE_MANPPI, /**< Manual PPI mode (elevation does not step automatically) */
	SCAN_TYPE_MANRHI, /**< Manual RHI mode (azimuth does not step automatically) */
	SCAN_TYPE_IDLE, /**<  IDLE mode, radar is not scanning */
	SCAN_TYPE_LAST
} scan_types_t;

/**
\brief Indicates the object the antenna is currently tracking
*/
typedef enum follow_mode
{
	FOLLOW_MODE_NONE = 0, /**< Radar is not tracking any object */
	FOLLOW_MODE_SUN, /**< Radar is tracking the sun */
	FOLLOW_MODE_VEHICLE, /**< Radar is tracking a vehicle */
	FOLLOW_MODE_LAST
} follow_mode_t;

/**
\brief Defines a single scan segment or volume
A scan segment describes a portion (or a complete) volume
*/

typedef struct scan_seg
{
	int id; 	/**< = HSK_ID_SCAN_SEG */
	int length;	/**< Number of bytes in this structure */
	float az_manual; /**< Manual azimuth position. Used in pointing and Manual PPI/RHI modes */
	float el_manual; /**< Manual elevation position. Used in pointing and Manual PPI/RHI modes */
	float az_start; /**< Azimuth start. If>360 implies don't care */
	float el_start; /**< Elevation start. If>360 implies don't care */
	float scan_rate; /**< Antenna scan rate, in degrees/sec */
	char segname[MAX_SEGNAME_LENGTH]; /**< Name of this scan segment */
	scan_optimizer_t opt; /**< Scan optimizer parameters */
	follow_mode_t follow_mode; /**< Indicates the object being followed (tracked) by the antenna */
	scan_types_t scan_type; /**< Antenna scanning mode */
	unsigned int scan_flags;   /**< Scan segment flags. See SF_... above  */
	unsigned int volume_num; /**< Volume number, increments linearly */
	unsigned int sweep_num; /**< Sweep number within the current volume */
	unsigned int time_limit; /**< Timeout for this scan, in seconds, if nonzero */
	unsigned int webtilt;    /**< if nonzero, archive image of specified sweep */
	float left_limit; /**< Left limit used in sector scan */
	float right_limit; /**< Right limit used in sector scan */
	float up_limit; /**< Upper limit used in sector scan */
	float down_limit; /**< Lower limit used in sector scan */
	float step; /**< Antenna step, used to increment az (in RHI) or el (in PPI) if max_sweeps is zero */
	unsigned int max_sweeps;  /**< Indicates that a set of discrete angles is specified for az or el */
	unsigned int filter_break_sweep;    /**< Sweep at which the clutter filter switch from filter 1 to 2 */
	unsigned int clutter_filter1;    /**< Clutter filter for sweeps 1 to filter_break_sweep */
	unsigned int clutter_filter2;    /**< Clutter filter for sweeps >= filter_break_sweep */
	char project[MAX_PRJNAME_LENGTH];   /**< Project name - used to organize scans + data */
	float current_fixed_angle; /**< Current fixed angle (az in RHI, el in PPI) */
} scan_seg_t;

/**
\brief Transmitter Polarization mode
*/
typedef enum pol_mode
{	
	POL_MODE_V = 0, /**< V-only mode */
	POL_MODE_H, /**< H-only mode */
	POL_MODE_VH, /**< Alternating mode, with polarization switch */
	POL_MODE_VHS, /**< Simulataneous (hybrid) mode */
	POL_MODE_VHA, /**< Alternating mode 2, no polarization switch */
	POL_MODE_VVHH, /**< Alternating mode, each pol repeated, i.e. VVHHVVHH... */
	POL_MODE_LAST
} pol_mode_t;

#ifdef _OLD_PROC_MODE
/**
\brief Processing mode used by moment calculator
*/
typedef enum proc_mode
{	
	PROC_MODE_NORMAL = 0,/**< Pulse-pair, int. cycle set by tx control */
	PROC_MODE_INDEXEDBEAM, /**< Pulse-pair, int cycle is variable depending on antenna speed */
	PROC_MODE_LONGINT, /**< Pulse-pair, but int. cycle does not depend on tx control */
	PROC_MODE_LAST
} proc_mode_t;
#else
/**
\brief Processing mode used by moment calculator

Indexed beam uses a the beam width parameter and the current antenna position
to determine the integration cycle

Long Integration mode decouples the signal processor's beam indexing from the
transmitter controller's pulse numbering scheme, which allows for
integration times much longer than the 256-hit limit imposed by the
transmitter controller.

Dual PRT mode makes use of the second prt stored in prt2_usec to enable
staggered PRT
*/
typedef unsigned int proc_mode_t;

#define PROC_MODE_INDEXEDBEAM_MSK	(1 << 0)
#define PROC_MODE_LONGINT_MSK		(1 << 1)
#define PROC_MODE_DUALPRT_MSK		(1 << 2)
#define PROC_MODE_PHASECODE_MSK		(1 << 3)

#endif

/**
\brief Pulse type used by transmitter
*/
typedef enum pulse_type
{
	PULSE_TYPE_RECT_1US = 0, /**< 1 microsecond rectangular pulse */
	PULSE_TYPE_RECT_200NS, /**< 200 nanosecond rectangular pulse */
	PULSE_TYPE_GAUSSIAN_1US, /**< 1 microsecond gaussian-weighted pulse */
	PULSE_TYPE_LAST
} pulse_type_t;

/**
\brief Test type, used when performing calibration cycles
*/
typedef enum test_type
{
	TEST_TYPE_NONE = 0, /**< No test is currently taking place */
	TEST_TYPE_CW_CAL, /**< CW calibration is taking place */
	TEST_TYPE_SOLAR_CAL_FIXED, /**< Fixed sun-pointing is taking place */
	TEST_TYPE_SOLAR_CAL_SCAN, /**< Scanning across the face of the sun */
	TEST_TYPE_NOISE_SOURCE_H, /**< Noise source is connected to the H channel */
	TEST_TYPE_NOISE_SOURCE_V, /**< Noise source is connected to the V channel */
	TEST_TYPE_BLUESKY, /**< Antenna is pointing at blue-sky */
	TEST_TYPE_SAVEPARAMS, /**< Not a real test mode, indicates to compute nodes to save cal params */
	TEST_TYPE_LAST
} test_type_t;

/**
\brief Processor control/status information
*/
typedef struct processor_info
{
	int id; /**< HSK_ID_PROCESSOR_INFO */
	int length; /**< Length of structure in bytes */
	
	pol_mode_t polarization_mode; /**< Transmitter polarization mode */
	proc_mode_t processing_mode; /**< Signal Processing mode */
	pulse_type_t pulse_type; /**< Transmitter pulse type */
	test_type_t test_type; /**< Radar calibration/test type */

	unsigned int integration_cycle_pulses; /**< Number of cycles integrated to give one ray */
	unsigned int clutter_filter_number; /**< Clutter filter used by the processor */
	unsigned int range_gate_averaging; /**< Number of range gates to average */
	float indexed_beam_width_d;   /**< Beamwidth in degrees, over which to integrate in indexed beam mode*/
	
	float gate_spacing_m; /**< Gate spacing (in meters), does not include effect of range avergaing */
	float prt_usec; /**< PRT in microseconds */
	float range_start_km; /**< Range to start processing */
	float range_stop_km; /**< Range to stop processing */

	unsigned int max_gate; /**< Number of gates for digitizer to acquire */
	float test_power_dbm;	/**< Power at signal generator output in dBm, when test set is commanded to output 0dBm */
	//float test_setup_loss_h_db;
	//float test_setup_loss_v_db;
	float unused1; /**< Reserved */
	float unused2; /**< Reserved */
	float test_pulse_range_km;  /**< Range at which test pulse is located */
	float test_pulse_length_usec;  /**< Length of test pulse */
	
	float prt2_usec; /**< PRT in microseconds used in dual PRT/PRF modes */
	float range_offset_m; /**< Range offset to first gate, in meters */
} processor_info_t;

/**
\brief Sent each time the transmitter monitoring power meters are read 
*/
typedef struct power_update
{
	int id; /**< HSK_ID_PWR_UPDATE */
	int length; /**< Structure length */

	float h_power_dbm; /**< H peak power in dBm assuming a rectangular pulse */
	float v_power_dbm; /**< V peak power in dBm assuming a rectangular pulse */
} power_update_t;

/**
\brief Sent out by acq server when measuring transmitter sample channel
*/
typedef struct xmit_sample
{
	int id; /**< HSK_ID_XMIT_SAMPLE */
	int length; /**< Structure length */

	float h_power_dbm; /**< H peak power in dBm assuming a rectangular pulse */
	float v_power_dbm; /**< V peak power in dBm assuming a rectangular pulse */
	int offset; /**< Offset from trigger pulse of these samples */
	int samples; /**< Number of valid samples */
	signed short h_samples[N_TXSAMP]; /**< Sample of H transmitter, at 40 MHz rate */
	signed short v_samples[N_TXSAMP]; /**< Sample of V transmitter, at 40 MHz rate */
} xmit_sample_t;

typedef unsigned int event_notice_flags_t;
#define EN_END_SWEEP (1 << 0)
#define EN_END_VOLUME (1 << 1)
#define EN_START_SWEEP (1 << 2)

/**
\brief Indicates the reason this scan terminated
*/
typedef enum event_notice_cause
{
	END_NOTICE_CAUSE_DONE = 0, /**< Scan completed normally */
	END_NOTICE_CAUSE_TIMEOUT, /**< Scan has timed out */
	END_NOTICE_CAUSE_TIMER, /**< Timer caused this scan to abort */
	END_NOTICE_CAUSE_ABORT, /**< Operator issued an abort */
	END_NOTICE_CAUSE_ERROR_ABORT, /**< Scan Controller detected error */
	END_NOTICE_CAUSE_RESTART, /**< communication fault with DTau was recovered, restarting scan */
	END_NOTICE_CAUSE_LAST
} event_notice_cause_t;

/**
\brief Indicates a radar event 
*/
typedef struct event_notice
{
	int id; /**< HSK_ID_EVENT_NOTICE */
	int length; /**< Structure length */
	event_notice_flags_t flags; /**< Flags indicating type of event */
	event_notice_cause_t cause; /**< Additional information about the event */
} event_notice_t;

/**
\brief These are computed cal terms from receiver measurements
*/
typedef struct cal_terms
{
	int id; /**< HSK_ID_CAL_TERMS */
	int length; /**< Structure length */
	
	/* Noise floor measurements (during blue-sky mode) */
	float noise_v_rx_1; /**< average noise level, V channel, transfer switch off */
	float noise_h_rx_2; /**< average noise level, H channel, transfer switch off */
	float noise_v_rx_2; /**< average noise level, H channel, transfer switch on */
	float noise_h_rx_1; /**< average noise level, V channel, transfer switch on */

	/* Computed calibration terms */
	float zcon_h_db;	/**< dbz_h = dbu_h + zcon_h_db + 20.0 *log10 (range_km/100) */
	float zcon_v_db;

	float zdr_bias_db;  /**< ZDR bias  */
	float ldr_bias_h_db; /**< LDR bias for H transmit  */
	float ldr_bias_v_db; /**< LDR bias for V transmit  */
	
	float noise_source_h_db; /**< Noise source power, H channel, in dBU */
	float noise_source_v_db; /**< Noise source power, V channel, in dBU */
	
	float gain_v_rx_1_db; /**< channel 1 gain when xfer switch is off */
	float gain_h_rx_2_db; /**< channel 2 gain when xfer switch is off */
	float gain_v_rx_2_db; /**< channel 1 gain when xfer switch is on */
	float gain_h_rx_1_db; /**< channel 2 gain when xfer switch is on */
	
	float sun_pwr_v_rx_1_db; /**< solar calibration results for channel 1 when xfer sw is off */
	float sun_pwr_h_rx_2_db; /**< solar calibration results for channel 2 when xfer sw is off */
	float sun_pwr_v_rx_2_db; /**< solar calibration results for channel 1 when xfer sw is on */
	float sun_pwr_h_rx_1_db; /**< solar calibration results for channel 2 when xfer sw is on */
} cal_terms_t;

/**
\brief Pair of phase values for two channels
*/
typedef struct phase_sample {
	float phase_v_d; /**< V channel phase, in degrees */
	float phase_h_d; /**< H channel phase, in degrees */
} phase_sample_t;

#define MAX_PHASE_SEQ_LEN 256

/**
\brief Phase code definition, sent when phase-coded volumes are started
*/
typedef struct phasecode
{
	int id; /**< HSK_ID_PHASE_CODE */
	int length; /**< Structure length */
	
	int seq_length; /**< Number of pulses in sequence */
	phase_sample_t phase[MAX_PHASE_SEQ_LEN]; /**< Phase sequence */
} phasecode_t;

#define MAX_CLUTTER_FILTER_NAME 64

/**
\brief Indicates the reason this scan terminated
*/
typedef enum clutter_filter_type
{
	CLUTTER_FILTER_TYPE_IIR = 0,
	CLUTTER_FILTER_TYPE_FFT,
	CLUTTER_FILTER_TYPE_LAST
} clutter_filter_type_t;

/**
\brief Indicates the reason this scan terminated
*/
typedef enum iir_filter_type
{
	IIR_FILT_0_POLE = 0,
	IIR_FILT_3_POLE,
	IIR_FILT_4_POLE,
	IIR_FILT_5_POLE,
	MAX_IIR_FILT_TYPE
} iir_filter_type_t;

typedef struct iir_filter_info {
	iir_filter_type_t type;
	float gain;
	float b11, a11, a21;
	float b12, a12, a22;
	float b13, a13, a23;
} iir_filter_info_t;

/**
\brief Clutter filter information, sent in addition to proc_info
*/
typedef struct clutter_filter_info
{
	int id; /**< HSK_ID_FILTER_INFO */
	int length; /**< Structure length */

	clutter_filter_type_t type;
	char name[MAX_CLUTTER_FILTER_NAME];
	union {
		iir_filter_info_t iir;
	} info;
} clutter_filter_info_t;

/**
\page sdb_fmt_hist Standard Data Buffer stream format history
- 1.0 - Initial version
- 1.1 - Includes sweep table information #sweep_info_hdr_t
- 1.2 - Uses event notice instead of sweep header
- 1.3 - Includes the lag0_hv field separately, added KDP, Zc, PHIDPf fields
- 1.4 - Ray header includes a field indicating the size of data in each gate
- 1.5 - The ray header and archive ray header include az/el start/end
*/

#define SDB_FORMAT_VERSION	0x00010005

/**
\brief SDB stream version indicator
*/
typedef struct sdb_version_hdr {
	int id; /**< HSK_ID_VERSION */
	int length; /**< Structure length */
	
	unsigned int version; /**< Stream format version. First word=major, second word=minor */
	unsigned int creator_version; /**< Creator version. First word=major, second word=minor */
} sdb_version_hdr_t;

#define XMIT_H_ENABLE	0x01
#define XMIT_V_ENABLE	0x02

/**
\brief Transmitter information header
*/
typedef struct xmit_info {
	int id; /**< HSK_ID_XMIT_INFO */
	int length; /**< Structure length */
	
	unsigned int xmit_enables; /**< Indicates which transmitters are firing (see XMIT_?_ENABLE) */
	pol_mode_t polarization_mode; /**< Transmitter polarization mode */
	pulse_type_t pulse_type; /**< Transmitter pulse waveform */
	float prt_usec; /**< PRT in microseconds */
	float prt2_usec; /**< Second PRT in microseconds for Dual-PRT mode */
} xmit_info_t;

/**
\brief Antenna angle correction information
These angles must be *added* to the az and el to obtain
the corrected version as follows:
angle = (raw_angle + correction) & 0xFFFF;
*/
typedef struct antenna_correction {
	int id; /**< HSK_ID_ANT_OFFSET */
	int length; /**< Structure length */
	
	int azimuth; /**< Azimuth offset in 16-bit angle units */
	int elevation; /**< Elevation offset in 16-bit angle units */
} antenna_correction_t;

#define MAX_TRACK_ID	32
#define MAX_TRACK_INFO	32

/**
\brief SDB Vehicle Track information
This structure describes vehicle tracks. Position reports are recorded here,
along with a timestamp, a track ID and any additional information reported
from the vehicle (such as telemetry)
*/
typedef struct sdb_track_info {
	int id; /**< HSK_ID_TRACK_INFO */
	int length; /**< Structure length */
	
	unsigned long long time; /**< Seconds since UNIX Epoch for this report */
	
	float latitude_d; /**< Vehicle GPS Latitude +ve = E, -ve = W */
	float longitude_d; /**< Vehicle GPS Longitude +ve = N, -ve = S */
	float altitude_m; /**< Vehicle altitude MSL */
	float heading_d; /**< Vehicle heading (if available) */
	char track_id[MAX_TRACK_ID]; /**< Name of this vehicle */
	char track_info[MAX_TRACK_INFO]; /**< Additional track information */
	unsigned int source_id; /**< Unique Numeric ID assigned to each track source by moment server */
} sdb_track_info_t;

/***********************************
 *    Processed data structures    *
 ***********************************/

#define MOM_ID_ALL_DATA		0x5AA60001
#define MOM_ID_SDB		MOM_ID_ALL_DATA

/**
\brief SDB socket ray header. Not used in files (see arch_ray_header)
*/
typedef struct ray_header
{
	int id; /**< MOM_ID_ALL_DATA */
	int length; /**< Structure length */

	float azimuth; /**< Mean azimuth for this ray */
	float elevation; /**< Mean elevation for this ray */

	float azimuth_width; /**< Width of the ray, in azimuth (valid only for PPI) */
	float elevation_width; /**< Width of the ray, in elevation (valid only for RHI) */	
	
	unsigned short gates; /**< Number of gates in this ray */
	unsigned short beam_index; /**< Beam index number */
	unsigned int ray_number; /**< Ray number (continuously increasing) */
	unsigned int time; /**< UNIX time (seconds since Midnight 1 Jan 1970) */
	unsigned int ns_time; /**< Nanosecond timestamp counter */
	unsigned int num_pulses; /**< Number of pulses averaged for ray */
	unsigned int bytes_per_gate; /**< Number of bytes per range gate */

	float azimuth_start; /**< The beginning azimuth of this integration volume */
	float azimuth_end; /**< The end azimuth of this integration volume */
	float elevation_start; /**< The beginning elevation of this integration volume */
	float elevation_end; /**< The end elevation of this integration volume */
} ray_header_t;

/**
\brief Moment data for each gate.
All standard moments, polarimetric moments and the complex covariances are recorded here
Note: for parts of the moment server to work correctly, sizeof(gate_mom_data_t) must be
an integer multiple of sizeof(float).
*/
typedef struct gate_mom_data {
	float Z; /**< Reflectivity in dbZ */
	float V; /**< Velocity, in m/s */
	float W; /**< Spectral width (variance) in m/s */
	float NCP; /**< Normalized Coherent Power (aka SQI) */
	float ZDR; /**< Differential Reflectivity in dB */
	float PHIDP; /**< Differential Phase in degrees */
	float RHOHV; /**< Crosspolar correlation */
	float LDR_H; /**< Linear Depolarization, H transmitting in dB */
	float LDR_V; /**< Linear Depolarization, V transmitting in dB */
	float KDP; /**< Specific Differential Propogation in degrees/km */
	float Zc; /**< Corrected Reflectivity in dbZ */
	float ZDRc; /**< Corrected Differential Reflectivity in dB */
	float PHIDPf; /**< Smoothed Differential Phase in degrees */
	float_complex avg_v; /**< Average V I/Q (in arbitrary digitizer units (ADU) */
	float_complex avg_h; /**< Average H I/Q (in arbitrary digitizer units (ADU) */
	float lag0_h; /**< lag-0 copolar correlation, H channel */
	float lag0_v; /**< lag-0 copolar correlation, V channel */
	float lag0_hx; /**< lag-0 crosspolar correlation, H channel */
	float lag0_vx; /**< lag-0 crosspolar correlation, V channel */
	float_complex lag1_h; /**< lag-1 complex correlation, H channel */
	float_complex lag1_v; /**< lag-1 complex correlation, V channel */
	float_complex lag2_h; /**< lag-2 complex correlation, H channel */
	float_complex lag2_v; /**< lag-2 complex correlation, V channel */
	float_complex lag0_hv; /**< lag-0 complex cross correlation between H & V channels */
	float RHOHV_HCX; /**< Co-to-cross polar correlation */
	float RHOHV_VCX; /**< Co-to-cross polar correlation */
} gate_mom_data_t;

/********************************
 *    HPIB server structures    *
 ********************************/
#define MIN_TP_FREQ		2000000
#define MAX_TP_FREQ		12400000
#define MIN_TP_POWER		-100
#define MAX_TP_POWER		13

#define HPIB_ID_TESTSET		0x5AA70001
#define HPIB_ID_TXPOW_REQ	0x5AA70002
#define HPIB_ID_TXPOW		0x5AA70003
#define HPIB_ID_RESET		0x5AA70004
#define HPIB_ID_IDLE		0x5AA70005

/**
\brief HPIB server command structure
Sends a command to the HPIB control server to set test frequency, pulse mode and power 
*/
typedef struct hpib_command {
	int id; /* HPIB_ID_TESTSET */
	int length; /**< Structure length */
	
	int is_pulsed; /**< Nonzero if the test signal is pulsed, otherwise zero */
	float test_freq_khz; /**< Test signal frequency in kHz */
	float test_pow_dbm; /**< Test signal power in dBm */
} hpib_command_t;

/**
\brief HPIB server request packet
HPIB_ID_TXPOW_REQ: read and cache the power meters 
HPIB_ID_RESET: reset the bus and initialize the instruments to a known state
HPIB_ID_IDLE: Release control of the bus
*/
typedef struct hpib_txpow_req {
	int id; /**< HPIB_ID_TXPOW_REQ, HPIB_ID_RESET, HPIB_ID_IDLE */
	int length; /**< Structure length */
} hpib_txpow_req_t;

/**
\brief HPIB server data packet
Data packet emitted by the HPIB server when it receives an HPIB_ID_TXPOW_REQ packet 
*/
typedef struct hpib_data {
	int id; /**< HPIB_ID_TXPOW */
	int length; /**< Structure length */
	
	int is_pulsed;
	float test_freq_khz; /* in kHz */
	float test_pow_dbm; /* in dB as requested at sig gen */
	
	float tx_pow_h_dbm; /* in dB avg from meter */
	float tx_pow_v_dbm; /* in dB avg from meter */
} hpib_data_t;

/*****************************************
*    Headers for moment data archives    *
******************************************/
/**
\page file_fmt_hist Archive File Format History
- 1.0 - Initial version
- 1.1 - Added support for az/el start/end
*/

#define ARCH_FORMAT_VERSION	0x00010001

/* Version to test for if reading az/el start/end */
#define ARCH_FORMAT_MINVER_AZEL_STARTEND	0x00010001

#define ARCH_ID_CONTROL		0x5AA80001
#define ARCH_ID_FIELD_SCALE	0x5AA80002
#define ARCH_ID_RAY_HDR		0x5AA80003
#define ARCH_ID_FILE_HDR	0x5AA80004
#define ARCH_ID_SWEEP_BLOCK	0x5AA80005

/**
\brief Determines the type of compression used when archiving data
This is currently not used (anything besides COMP_TYPE_NONE is ignored)
*/
typedef enum compression {
	COMP_TYPE_NONE = 0, /**< No compression */
	COMP_TYPE_GZIP, /**< Use gzip compression */
	COMP_TYPE_BZIP2, /**< Use bzip2 compression */
	LAST_COMP_TYPE
} compression_t;

/**
\brief Global data quality setting.
Used to switch between 16-bit and 8-bit moments
*/
typedef enum quality {
	QUALITY_TYPE_LOW = 0, /**< Use 8-bit moment data */
	QUALITY_TYPE_HIGH, /**< Use 16-bit moment data */
	LAST_QUALITY_TYPE
} quality_t;

/**
\brief Archiver control packet 
The archiver listens for mode changes with this packet
*/
typedef struct arch_ctl
{
	int id; /**< ARCH_ID_CONTROL */
	int length; /**< Packet length */
	
	unsigned long long field_mask; /**< Bit mask indicating what fields to record */
	compression_t comp; /**< How to compress the moment data */
	quality_t quality; /**< What quality to archive data in */
	float max_hgt; /**< Maximum height up to which data is recorded */
	float ncp_threshold; /**< NCP Threshold, below which all fields are set to "not available"
				when recording. Set to zero or a negative number to disable */ 
} arch_ctl_t;

/**
\brief Generic housekeeping packet, a union of all the above.
This may be used instead of typecasting to avoid the gcc warnings
about type-punned pointers in -Wall mode 
*/
typedef union housekeeping {
	struct {
		int id; /**< Can be one of the SDB header IDs */
		int length; /**< SDB header length */
		
		int rest_of_packet; /**< used to obtain a pointer to the rest of the header */
	} gen; /**< Generic packet */
	radar_info_t radar_info; /**< Radar info packet */
	scan_seg_t scan_seg; /**< Scan segment packet */
	processor_info_t proc_info; /**< Processor info packet */
	power_update_t power_update; /**< Power update packet */
	event_notice_t event_notice; /**< Event notice packet */
	cal_terms_t cal_terms; /**< Cal terms packet */
	xmit_info_t xmit_info; /**< Transmitter info packet */
	ray_header_t ray; /**< Ray header packet */
#ifndef __cplusplus
	/* In C++, we're using the complex template class for fields in gate_mom_data
	   and members with constructors aren't allowed in unions in C++ */
	struct {
		ray_header_t ray_hdr; /**< Ray header */
		gate_mom_data_t gates[1]; /**< Ray data */
	} ray_data; /**< Ray header + data */
#endif
	sdb_track_info_t trk_info; /**< Vehicle Track information */
	sdb_version_hdr_t ver; /**< SDB Version structure */ 
	xmit_sample_t xmit_sample; /**< Transmitter sample */
	clutter_filter_info_t clutter_filter_info; /**< Clutter filter info */
	arch_ctl_t arch_ctl; /**< Archiver control packet */
} housekeeping_t;

/*
    Yet Another convenience strucure, for 2006+ housekeeping with the new union of hsk structures
*/
typedef struct housekeeping_packet_u
{
  struct housekeeping_packet_header hdr;
  housekeeping_t hsk_u;
} housekeeping_packet_u_t;

/**\brief Available encoding formats used by moments */
typedef enum format {
	MOMENT_FMT_8BIT = 0, /**< 8-bit moment data */
	MOMENT_FMT_32BIT, /**< 32-bit moment data */
	MOMENT_FMT_FLOATINGPT, /**< Single-precision floating point moment data */
	MOMENT_FMT_16BIT, /**< 16-bit moment data */
	LAST_MOMENT_FMT
} format_t;

/**
\brief Display type hints
Type hints used by display programs to decide colour scales and display modes
The generic fields are meant for use by any additional fields that may be computed
in the future.
*/
typedef enum type_id_hint {
	TYPE_ID_Z, /**< Reflectivity */
	TYPE_ID_V, /**< Velocity */
	TYPE_ID_W, /**< Spectral Width */
	TYPE_ID_NCP, /**< Normalized Coherent Power */
	TYPE_ID_ZDR, /**< Differential Reflectivity */
	TYPE_ID_LDR, /**< Linear Depolarization */
	TYPE_ID_PHIDP, /**< Differential Phase Shift */
	TYPE_ID_RHOHV, /**< lag-0 correlation between H&V */
	TYPE_ID_KDP, /**< Specific Differential Propogation Phase */
	TYPE_ID_GENERIC_POWER, /**< Generic field representing power */
	TYPE_ID_GENERIC_VELOCITY,/**< Generic field representing velocity */
	TYPE_ID_GENERIC_ANGLE,/**< Generic field representing angle */
	TYPE_ID_GENERIC_UNSCALED,/**< Generic field for unscaled data */
	TYPE_ID_GENERIC_RATIO,/**< Generic field representing a ratio */
	LAST_TYPE_ID
} type_hint_t;

#define FLDSCALE_NAME_LEN	32
#define FLDSCALE_UNITS_LEN	32
#define FLDSCALE_DESCR_LEN	128

/**
\brief Field Scaling parameters
These are used to obtain information about the various fields recorded
in an archive file, there can be field scale structures for fields that
are not actually in the archive file (ie, don't assume that if a field_scale_t
for a field is present in the file that this field is available 
*/
typedef struct field_scale
{
	int id; /**< ARCH_ID_FIELD_SCALE */
	int length; /**< Structure length */
	
	format_t format; /**< Format used for this field */
	float min_val; /**< The minimum represented value for this field */
	float max_val; /**< The maximum represented value for this field */	
	int bit_mask_pos; /**< The bit mask position used by this field in ray header bitmask */
	type_hint_t type_hint; /**< Hint to display programs for the data type */
	int fld_factor; /**< Field scale factor. See \ref arch_scaling for more information */
	int dat_factor; /**< Data scale factor. See \ref arch_scaling for more information */
	int dat_bias; /**< Data bias factor. See \ref arch_scaling for more information */
	char name[FLDSCALE_NAME_LEN]; /**< Short name for this field, UTF-8 encoded */
	char units[FLDSCALE_UNITS_LEN]; /**< Units for this field, UTF-8 encoded */			
	char descr[FLDSCALE_DESCR_LEN]; /**< Description for this field, UTF-8 encoded */			
} field_scale_t;

/**
\brief Archive Ray header
This header is stored with every ray in the file. It replaces the MOM_ID_SDB field when
data is written to a file, and includes a bit-mask in addition to the fields found in
the ray_header_t structure.
*/
typedef struct arch_ray_header {
	int id; /**< ARCH_ID_RAY_HDR */
	int length; /**< Structure length */

	float azimuth; /**< Average azimuth */
	float elevation; /**< Average elevation */
	float azimuth_width; /**< Beamwidth in azimuth. This is only valid in PPI mode */
	float elevation_width;	/**< Beamwidth in elevation. This is only valid in RHI mode */
	
	unsigned short gates; /**< Number of gates following the header */
	unsigned short beam_index; /**< The beam index number. Only valid in indexed beam mode. */
	unsigned int ns_time; /**< Nanosecond timestamp for this ray */
	unsigned long long time; /**< The UNIX time word for this ray.
		This is seconds since UNIX epoch (12:00 AM, 1 Jan 1970) */
	unsigned long long bit_mask; /**< Bit mask of fields present in this ray */
	unsigned int ray_number; /**< Ray number. This is computed from the beginning of the volume */
	unsigned int num_pulses; /**< Number of pulses averaged to give this ray */

	float azimuth_start; /**< The beginning azimuth of this integration volume */
	float azimuth_end; /**< The end azimuth of this integration volume */
	float elevation_start; /**< The beginning elevation of this integration volume */
	float elevation_end; /**< The end elevation of this integration volume */
} arch_ray_header_t;

#define ARCH_FILE_CREATOR_ID_LEN	32

/**
\brief Archive File header
This block is stored at the start of every file. It identifies the creator and version information
*/
typedef struct arch_file_hdr {
	int id; /**< ARCH_ID_FILE_HDR */
	int length; /**< Structure length */
	
	unsigned int version; /**< File format version. First word=major, second word=minor */
	unsigned int creator_version; /**< Creator version. First word=major, second word=minor */
	char creator_id[ARCH_FILE_CREATOR_ID_LEN]; /**< Creator name, UTF-8 encoded */
	unsigned long long sweep_table_offset; /**< Offset from start of file where sweep table is located */
} arch_file_hdr_t;

/* Requires this to prevent 64-bit arch's from padding num_sweeps out to 64 bits -- aargh! why wasn't i more careful? */
#ifdef __x86_64 /* ARM compilers complain about these pragmas */
#pragma pack(push,4)
#endif
/**
\brief Sweep information block

This block contains a list of offsets into the file where different sweeps are found
The list can be of variable length, use \p num_sweeps to determine the length of the
\p sweep_offsets list. 
*/
typedef struct sweep_info_hdr {
	int id; /**< ARCH_ID_SWEEP_BLOCK */
	int length; /**< Structure length */
	
	unsigned int num_sweeps; /**< Number of sweeps in the list below */
	unsigned long long sweep_offsets[1]; /**< List of offsets of various sweeps */
} sweep_info_hdr_t;
#ifdef __x86_64 /* ARM compilers complain about these pragmas */
#pragma pack(pop)
#endif

/**************************************************************
*    New generic structure, a union of all possible           *
*    housekeeping structures visible in the archive stream    *
**************************************************************/

/**
\brief Generic archive stream housekeeping packet, a union of all 
hsk style structures found in the archive stream.
This may be used instead of typecasting to avoid the gcc warnings
about type-punned pointers in -Wall mode 
*/
typedef union arch_housekeeping {
	struct {
		int id; /**< Can be one of the SDB header IDs */
		int length; /**< SDB header length */
		
		int rest_of_packet; /**< used to obtain a pointer to the rest of the header */
	} gen; /**< Generic packet */
	radar_info_t radar_info; /**< Radar info packet */
	scan_seg_t scan_seg; /**< Scan segment packet */
	processor_info_t proc_info; /**< Processor info packet */
	power_update_t power_update; /**< Power update packet */
	event_notice_t event_notice; /**< End notice packet */
	cal_terms_t cal_terms; /**< Cal terms packet */
	xmit_info_t xmit_info; /**< Transmitter info packet */
	arch_ray_header_t arch_ray; /**< Archive Ray header packet */
	arch_file_hdr_t arch_file; /**< Archive File header packet */
	field_scale_t arch_field_scale; /**< Archive Field Scale packet */
#ifndef __cplusplus
	/* In C++, we're using the complex template class for fields in gate_mom_data
	   and members with constructors aren't allowed in unions in C++ */
	struct {
		ray_header_t ray_hdr; /**< Ray header */
		gate_mom_data_t gates[1]; /**< Ray data */
	} ray_data; /**< Ray header + data */
#endif
	sdb_track_info_t trk_info; /**< Vehicle Track information */
	sdb_version_hdr_t ver; /**< SDB Version structure */
	sweep_info_hdr_t sweep_info_hdr; /**< Sweep start locations */ 
	xmit_sample_t xmit_sample; /**< Transmitter sample */
	clutter_filter_info_t clutter_filter_info; /**< Clutter filter info */
} arch_housekeeping_t;

#define ILLEGAL_VAL_8BIT	0
#define ILLEGAL_VAL_16BIT	0
#define ILLEGAL_VAL_32BIT	0
#define ILLEGAL_VAL_FLOATINGPT	NAN

/* Minimum and maximum values for the common fields */
/* Some of these (like V) are dynamically calculated, these are just initial values */
#define Z_MIN		-32
#define	Z_MAX		96
#define ZDR_MIN		-3.0
#define	ZDR_MAX		9.0
#define	V_MIN		-27
#define	V_MAX		27
#define W_MIN		0.0
#define W_MAX		15
#define NCP_MIN		0.0
#define NCP_MAX		1.0
#define LDR_MIN		-48.0
#define LDR_MAX		0.0
#define PHIDP_MIN	-180.0
#define PHIDP_MAX	180.0
#define RHOHV_MIN	0.6
#define RHOHV_MAX	1.1
#define LAG0_MIN	0.0
#define LAG0_MAX	2147483647.0
#define LAG12_MIN	-2147483648.0
#define LAG12_MAX	2147483647.0
#define AVG_MIN		-32768
#define AVG_MAX		32767
#define KDP_MIN		-3.0
#define KDP_MAX		9.0
#define RHOCX_MIN	0.0
#define RHOCX_MAX	1.024

/* Default positions for the common fields.
   Note: these are only used to fill in the VCHILL/archive field
   structures, applications should use the values from these
   structures */
#define Z_FLD_NUM 	0
#define V_FLD_NUM 	1
#define W_FLD_NUM 	2
#define NCP_FLD_NUM 	3

#define ZDR_FLD_NUM 	4
#define LDRH_FLD_NUM 	5
#define LDRV_FLD_NUM 	6
#define PHIDP_FLD_NUM 	7

#define RHOHV_FLD_NUM 	8
#define KDP_FLD_NUM 	9
#define LAG0H_FLD_NUM	10
#define LAG0HX_FLD_NUM	11

#define LAG1IH_FLD_NUM	12
#define LAG1QH_FLD_NUM	13
#define LAG2IH_FLD_NUM	14
#define LAG2QH_FLD_NUM	15

#define LAG0V_FLD_NUM	16
#define LAG0VX_FLD_NUM	17
#define LAG1IV_FLD_NUM	18
#define LAG1QV_FLD_NUM	19

#define LAG2IV_FLD_NUM	20
#define LAG2QV_FLD_NUM	21

#define LAG0HVI_FLD_NUM	22
#define LAG0HVQ_FLD_NUM	23

#define AVGI_V_FLD_NUM	24
#define AVGQ_V_FLD_NUM	25
#define AVGI_H_FLD_NUM	26
#define AVGQ_H_FLD_NUM	27

#define RHOHV_HCX_FLD_NUM	28
#define RHOHV_VCX_FLD_NUM	29

#define ZC_FLD_NUM	30
#define ZDRC_FLD_NUM	31

#define LAST_FLD_NUM	31

#define STD_FLD_START	Z_FLD_NUM
#define STD_FLD_STOP	KDP_FLD_NUM

#define COV_FLD_START	LAG0H_FLD_NUM
#define COV_FLD_STOP	AVGQ_H_FLD_NUM

/* Default masks for the fields (based on the field positions) */
#define Z_FLD_MSK	(1LL << ((long long)Z_FLD_NUM))
#define V_FLD_MSK	(1LL << ((long long)V_FLD_NUM))
#define W_FLD_MSK	(1LL << ((long long)W_FLD_NUM))
#define NCP_FLD_MSK	(1LL << ((long long)NCP_FLD_NUM))
#define ZDR_FLD_MSK	(1LL << ((long long)ZDR_FLD_NUM))
#define LDRH_FLD_MSK	(1LL << ((long long)LDRH_FLD_NUM))
#define LDRV_FLD_MSK	(1LL << ((long long)LDRV_FLD_NUM))
#define PHIDP_FLD_MSK	(1LL << ((long long)PHIDP_FLD_NUM))
#define RHOHV_FLD_MSK	(1LL << ((long long)RHOHV_FLD_NUM))
#define KDP_FLD_MSK	(1LL << ((long long)KDP_FLD_NUM))

#define LAG0H_FLD_MSK	(1LL << ((long long)LAG0H_FLD_NUM))
#define LAG0HX_FLD_MSK	(1LL << ((long long)LAG0HX_FLD_NUM))
#define LAG1IH_FLD_MSK	(1LL << ((long long)LAG1IH_FLD_NUM))
#define LAG1QH_FLD_MSK	(1LL << ((long long)LAG1QH_FLD_NUM))
#define LAG2IH_FLD_MSK	(1LL << ((long long)LAG2IH_FLD_NUM))
#define LAG2QH_FLD_MSK	(1LL << ((long long)LAG2QH_FLD_NUM))

#define LAG0V_FLD_MSK	(1LL << ((long long)LAG0V_FLD_NUM))
#define LAG0VX_FLD_MSK	(1LL << ((long long)LAG0VX_FLD_NUM))
#define LAG1IV_FLD_MSK	(1LL << ((long long)LAG1IV_FLD_NUM))
#define LAG1QV_FLD_MSK	(1LL << ((long long)LAG1QV_FLD_NUM))
#define LAG2IV_FLD_MSK	(1LL << ((long long)LAG2IV_FLD_NUM))
#define LAG2QV_FLD_MSK	(1LL << ((long long)LAG2QV_FLD_NUM))

#define LAG0HVI_FLD_MSK	(1LL << ((long long)LAG0HVI_FLD_NUM))
#define LAG0HVQ_FLD_MSK	(1LL << ((long long)LAG0HVQ_FLD_NUM))

#define AVGI_V_FLD_MSK	(1LL << ((long long)AVGI_V_FLD_NUM))
#define AVGQ_V_FLD_MSK	(1LL << ((long long)AVGQ_V_FLD_NUM))
#define AVGI_H_FLD_MSK	(1LL << ((long long)AVGI_H_FLD_NUM))
#define AVGQ_H_FLD_MSK	(1LL << ((long long)AVGQ_H_FLD_NUM))

#define RHOHV_HCX_FLD_MSK	(1LL << ((long long)RHOHV_HCX_FLD_NUM))
#define RHOHV_VCX_FLD_MSK	(1LL << ((long long)RHOHV_VCX_FLD_NUM))

#define ZC_FLD_MSK	(1LL << ((long long)ZC_FLD_NUM))
#define ZDRC_FLD_MSK	(1LL << ((long long)ZDRC_FLD_NUM))

#endif /* _CHILL_TYPES_H_ */
