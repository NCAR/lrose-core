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

#ifndef _RSM_H_
#define _RSM_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include <arpa/inet.h>

#define RSM_DEFAULT_GRP "224.0.0.3"
#define RSM_DEFAULT_PORT 20000
#define RSM_MAX_TEXT 64
#define RSM_MAX_STATUS 64
#define RSM_MAX_VER 16
#define MAX_DIGIO_NAME 8
/**
\brief RSM communications object
*/
typedef struct rsm {
	int send_socket; /**< Socket used to send RSM communications */
	int recv_socket; /**< Socket used to receive RSM communications */
	struct sockaddr_in send_addr; /**< Address to send data */
	struct sockaddr_in recv_addr; /**< Address to receive data */
} rsm_t;

/**
\brief RSM message type
*/
typedef enum rsm_msgtype {
	RSM_MSG_QUERY = 'Q', /**< Sent to a target to retrieve information */
	RSM_MSG_STATUS = 'S', /**< Response to a query or a general status update */
	RSM_MSG_WARNING = 'W', /**< Status update, indicating a warning condition */
	RSM_MSG_ERROR = 'E', /**< Status update, indicating an error */
	RSM_MSG_PRESET = 'P', /**< Sent to a target to set some parameter */
	RSM_MSG_KEEPALIVE = 'K', /**< Periodic "keepalive" messages sent by targets */
} rsm_msgtype_t;

/**
\brief RSM message header

This is sent at the head of every RSM message. It is always 16 bytes in
length. The Module ID is used to identify the source of the status message.
*/
typedef struct rsm_msghdr {
	char length[4]; /**< ASCII representation of length */
	char type; /**< From rsm_msgtype_t */
	char module_id[11]; /**< Sending module identifier */
	uint64_t timestamp; /* 64-bit time stamp from Unix Epoch */
	char pad[8];
} rsm_msghdr_t;

#define RSM_GENERIC "GEN"

/* Acquisition status */
#define RSM_ACQ "ACQ"

typedef enum rsm_acq_id {
	RSM_ACQ_ID_STATUS,
	RSM_ACQ_ID_CONN_NOTICE,
	RSM_ACQ_ID_MISC_STATS,
	RSM_ACQ_ID_TXSAMPCTL,
	RSM_ACQ_ID_CMD,
} rsm_acq_id_t;

typedef enum rsm_acq_status {
	RSM_ACQ_IDLE = 0,
	RSM_ACQ_NOSIGNAL,
	RSM_ACQ_ACQUIRING,
	RSM_ACQ_INIT,
	RSM_ACQ_ERROR,
	RSM_ACQ_LAST_STATE
} rsm_acq_status_t;

typedef enum rsm_acq_cmd {
	RSM_ACQ_CMD_RECONFIG,
	RSM_ACQ_CMD_RESET,
	RSM_ACQ_CMD_TXSAMPLE,
} rsm_acq_cmd_t;

typedef struct rsm_acq {
	unsigned char id; /**< Set from rsm_acq_id_t */
	union {
		struct {
			unsigned char state; /**< Set from rsm_acq_status_t */
			char msg[RSM_MAX_STATUS]; /**< Status message */
		} state;
		struct {
			unsigned char flags1;
			unsigned char numclients;
			unsigned char acq_len_lsb;
			unsigned char acq_len_msb;
		} misc_stats;
		struct {
			unsigned char type; /**< 0 = connect, 1 = disconnect */
			char hostname[RSM_MAX_STATUS]; /**< Name of host that connected */
		} conn_notice;
		struct {
			unsigned char port;
			unsigned char length_lsb;
			unsigned char length_msb;
			unsigned char offset_lsb;
			unsigned char offset_msb;
		} tx_sample_control;
		struct {
			unsigned char cmd;
		} control;
	} data;
} rsm_acq_t;

#define RSM_ACQ_FLAGS_FIFO1_OVF 0x01
#define RSM_ACQ_FLAGS_FIFO2_OVF 0x02
#define RSM_ACQ_FLAGS_ADC1_OR  0x04
#define RSM_ACQ_FLAGS_ADC3_OR  0x08
#define RSM_ACQ_WANT_OVERSAMPLED 0x10

/* Acquisition server version */
#define RSM_ACQ_VER "ACQVER"

typedef struct rsm_acq_version {
	char ver[RSM_MAX_VER]; /**< Version string */
} rsm_acq_version_t;

/* Acquisition server antenna angles */
#define RSM_ACQ_ANG "ACQANG"

typedef struct rsm_acq_angles {
	unsigned char azimuth_lsb; /**< LSB of azimuth */
	unsigned char azimuth_msb; /**< MSB of azimuth */
	unsigned char elevation_lsb; /**< LSB of elevation */
	unsigned char elevation_msb; /**< MSB of elevation */
	unsigned char nstime_1; /**< LSB of time-stamp */
	unsigned char nstime_2; /**< Byte 2 of time-stamp */
	unsigned char nstime_3; /**< Byte 3 of time-stamp */
	unsigned char nstime_4; /**< Byte 4 of time-stamp */
	unsigned char nstime_5; /**< Byte 5 of time-stamp */
	unsigned char nstime_6; /**< MSB of time-stamp */
} rsm_acq_angles_t;

/* Acquisition server transmitter burst pulse */
#define RSM_ACQ_TXSMP "ACQTXSMP"

typedef enum rsm_acq_smptype {
	RSM_ACQ_SMPTYPE_VCH = 0,
	RSM_ACQ_SMPTYPE_HCH,
	RSM_ACQ_LAST_SMPTYPE
} rsm_smptype_t;

typedef struct rsm_acq_txsample {
	unsigned char seq; /**< Sequence identifier */
	unsigned char sample_type; /**< Type of sample in this packet */
	unsigned short samplerate; /**< Sample rate (in tens of kHz) at which this data is sampled */
	unsigned short offset; /**< Sample offset at which this packet begins */
	unsigned short nsamples; /**< Total number of samples acquired */
	unsigned short trigger_offset; /**< Offset from trigger pulse of these samples */
	signed short samples[512]; /**< Samples */
} rsm_acq_txsample_t;


/**************************** DTcontrol antenna/system controller begin *******************/
/* DTcontrol system Controller - status  - uses generic text */
#define RSM_SYSTEM_CONTROLLER "SCON"
/** System Controller - status  - uses generic text */
#define RSM_SCON_DETAIL "SCON"
#define RSM_MAX_SEGNAME_LENGTH 16

typedef enum {
	RSM_SCON_SSTATE_SYSCON_INIT = 0,        /**< system controller program initializing */
	RSM_SCON_SSTATE_IDLE,	          /**< in idle state - not scanning */
	RSM_SCON_SSTATE_PREP_WAIT,	     /**< in PREP_STATE waiting for DTAU to signal start of scan */
	RSM_SCON_SSTATE_IN_SCAN,	       /**< doing the scan */
	RSM_SCON_SSTATE_SCAN_ENDING,	   /**< DTAU has signalled end volume, next_scan will begin next */
	RSM_SCON_SSTATE_SCAN_TIMEOUT,	  /**< scan is terminating because its timeout timer lapsed */
	RSM_SCON_SSTATE_ABORTING,	      /**< scan is being aborted to start a new scan (operator request) */
	RSM_SCON_SSTATE_TIMER_ABORTING,	/**< scan is being aborted by timer initiated start of a new scan */
	RSM_SCON_SSTATE_ABORTING_ON_ERROR,      /**< DTAU indicated an error -- aborting to idle mode */
	RSM_SCON_SSTATE_RESTARTING_ON_ERROR,    /**< DTAU comunication lapse error -- restarting current scan */
	RSM_SCON_SSTATE_CONTROLLER_RECONNECT,   /**< DTAU not responding, restoring connection */
	RSM_SCON_SSTATE_LAST_STATE      
} rsm_scan_status_t;

      /* operator started scan */
#define        RSM_ST_OPERATOR_BEGIN  0
      /* timer started scan */
#define        RSM_ST_TIMER_BEGIN 1
      /* scan started following next segment link */
#define        RSM_ST_NEXTSEG_LINK 2
      /* scan was restarted due to an error */
#define        RSM_ST_RESTART 3
#define        RSM_ST_LAST_STATE 4

      /* system controller program initializing */
#define        RSM_SS_SYSCON_INIT  0 
	     /* in idle state - not scanning */
#define        RSM_SS_IDLE     1
	     /* in PREP_STATE waiting for DTAU to signal start of scan */
#define        RSM_SS_PREP_WAIT 2
	     /* doing the scan */
#define        RSM_SS_IN_SCAN 3
	     /* DTAU has signalled end volume, next_scan will begin next */
#define        RSM_SS_SCAN_ENDING 4
	     /* scan is terminating because its timeout timer lapsed */
#define        RSM_SS_SCAN_TIMEOUT 5
	     /* scan is being aborted to start a new scan (operator request) */
#define        RSM_SS_ABORTING 6
	     /* scan is being aborted by timer initiated start of a new scan */
#define        RSM_SS_TIMER_ABORTING 7
	     /* DTAU indicated an error -- aborting to idle mode */
#define        RSM_SS_ABORTING_ON_ERROR 8
	     /* DTAU comunication lapse error -- restarting current scan */
#define        RSM_SS_RESTARTING_ON_ERROR 9
	     /* DTAU not responding, restoring connection */
#define        RSM_SS_CONTROLLER_RECONNECT 10
#define        RSM_SS_LAST_STATE 11


#define RSM_SCON_SERVO_ACTIVATED 1
#define RSM_SCON_SERVO_ENABLED   2
#define RSM_SCON_SERVO_OPEN_LOOP   4
#define RSM_SCON_SERVO_AMPLIFIER_FAULT 8
#define RSM_SCON_SERVO_FATAL_FOLLOWING_ERROR 16
#define RSM_SCON_SERVO_FOLLOWING_ERROR_WARN 32
#define RSM_SCON_SERVO_IN_POSITION 64

typedef enum {
	RSM_SCON_BC_OPERATOR_BEGIN = 0,         /** operator started scan */
	RSM_SCON_BC_TIMER_BEGIN,                /** timer started scan */
	RSM_SCON_BC_NEXTSEG_LINK,               /** scan started following next segment link */
	RSM_SCON_BC_RESTART,                    /** scan was restarted due to an error */
	RSM_SCON_BC_LAST_STATE          
 } rsm_scon_begin_cause_t;

typedef struct  {
       unsigned char scan_state;               /**< current scan controller state (see RSM_ST_ defines */
       unsigned char seconds_in_state1;        /**< seconds elapsed in current state LSB */
       unsigned char seconds_in_state2;        /**< seconds elapsed in current state MSB */
       char DTmonitorOK;               /**< nonzero indicates DTmonitor thread is stuck */
       unsigned char az_servo;        /**< bit fields for servo status (ANT_SERVO_ defines) */
       unsigned char el_servo;        /**< bit field */
       unsigned char sweep_num;        /**< current sweep number */
       unsigned char  start_cause;   /**< how did scan start - (see RSM_SS_ defines) */
       char segname[RSM_MAX_SEGNAME_LENGTH];   /**< current scan segment name */
}  rsm_syscon_detail_t;

/* System Controller status message */
#define RSM_SCON_STATUS "SCONSTAT"
typedef struct {
       char message[RSM_MAX_TEXT];     /**< one line status summary */
} rsm_syscon_status_t;

/* System Controller version */
#define RSM_SCON_VERSION "SCONVER"
typedef struct rsm_syscon_version {
       char ver[RSM_MAX_VER]; /**< Version string */
} rsm_syscon_version_t;

/*************************  end DTcontrol rsm items ******************************************/

/** 2010 System Controller (syscon) - status  - uses generic text */
#define RSM_SYSCON "SYSCON"
#define RSM_MAX_SYSCON_SEGNAME_LENGTH 32

typedef enum rsm_syscon_id {	/**< describes the union element included in this packet */
	RSM_SYSCON_ID_STATE, /**< syscon state related items */
	RSM_SYSCON_ID_SCAN,  /**<  syscon scan related items */
	RSM_SYSCON_ID_VERSION, /**< syscon version num */
	RSM_SYSCON_ID_STATUS,  /**< syscon error conditions */
	RSM_SYSCON_ID_ZDRBIAS, /**< Set ZDR bias */
	RSM_SYSCON_ID_PHIDPOFFSET, /**< Set PHIDP offset */
}  rsm_syscon_id_t;

typedef enum {				/* this should track rcs_syscon_state_t in syscon/src/globals.h */
	RSM_SYSCON_NO_STATE=0,
	RSM_SYSCON_INITIALIZING,	/* connecting to txmit control and antenna controller */
	RSM_SYSCON_IDLE,		/* connected, but not scanning */
	RSM_SYSCON_BEGIN_SCAN_SENT,	/* begin scan sent to antenna control */
	RSM_SYSCON_TIMER_WAIT,		/* waiting for timer to begin deferred start scan */
	RSM_SYSCON_IN_SCAN,		/* currently scanning */
	RSM_SYSCON_BETWEEN_SCANS,	/* between scans */
	RSM_SYSCON_LAST_STATE	
} rsm_syscon_state_t;

typedef enum {			/* this should track iwrf_data.h -> IWRF_SCAN_MODE */
 RSM_SCAN_MODE_NOT_SET = 0,
 RSM_SCAN_MODE_SECTOR = 1, /**< sector scan mode */
 RSM_SCAN_MODE_COPLANE = 2, /**< co-plane dual doppler mode */
 RSM_SCAN_MODE_RHI = 3, /**< range height vertical scanning mode */
 RSM_SCAN_MODE_VERTICAL_POINTING = 4, /**< vertical pointing for calibration */
 RSM_SCAN_MODE_IDLE = 7, /**< between scans */
 RSM_SCAN_MODE_AZ_SUR_360 = 8, /**< 360-degree azimuth mode - surveillance */
 RSM_SCAN_MODE_EL_SUR_360 = 9, /**< 360-degree elevation mode - eg Eldora */
 RSM_SCAN_MODE_SUNSCAN = 11, /**< scanning the sun for calibrations */
 RSM_SCAN_MODE_POINTING = 12, /**< fixed pointing */
 RSM_SCAN_MODE_MANPPI = 15, /**< Manual PPI mode (elevation does
			       * not step automatically) */
 RSM_SCAN_MODE_MANRHI = 16, /**< Manual RHI mode (azimuth does
			       * not step automatically) */
 RSM_SCAN_MODE_LAST /**< not used */
} rsm_scan_mode_t;

typedef enum {			/**< status codes set by syscon while talking to txctrl module */
	RSM_TXCTRL_ERR_NO_ERROR=0,
	RSM_TXCTRL_ERR_CONNECT=1,
	RSM_TXCTRL_ERR_SET_POL=2,
	RSM_TXCTRL_ERR_SET_INT_CYCLE=3,
	RSM_TXCTRL_ERR_SET_PRT1=4,
	RSM_TXCTRL_ERR_SET_PRT2=5,
	RSM_TXCTRL_ERR_SET_DUAL_PRT=6,
	RSM_TXCTRL_ERR_SET_TEST_PULSE=7,
	RSM_TXCTRL_ERR_SET_PHASE_SEQUENCE=8,
	RSM_TXCTRL_ERR_SET_WAVEFORM=9,
	RSM_TXCTRL_ERR_SET_COMMIT=10,
	RSM_TXCTRL_ERR_TX_ENABLE=11,
	RSM_TXCTRL_ERR_WAVEFORM_NOT_FOUND=12,
	RSM_TXCTRL_ERR_PHASE_SEQUENCE_NOT_FOUND=13,
	RSM_TXCTRL_ERR_PHASE_MODE_NOT_SUPPORTED=14,
	RSM_TXCTRL_ERR_PRETRIG_H=15,
	RSM_TXCTRL_ERR_PRETRIG_V=16,
	RSM_TXCTRL_ERR_LAST,
} rsm_txctrl_err_t;

typedef enum {
	RSM_EVENT_CAUSE_NOT_SET = 0,
	RSM_EVENT_CAUSE_DONE = 1, /**< Scan completed normally */
	RSM_EVENT_CAUSE_TIMEOUT = 2, /**< Scan has timed out */
	RSM_EVENT_CAUSE_TIMER = 3, /**< Timer caused this scan to abort */
	RSM_EVENT_CAUSE_ABORT = 4, /**< Operator issued an abort */
	RSM_EVENT_CAUSE_SCAN_ABORT = 5, /**< Scan Controller detected error */
	RSM_EVENT_CAUSE_RESTART = 6, /**< communication fault was recovered,
		                         *   restarting scan */
	RSM_EVENT_CAUSE_SCAN_STATE_TIMEOUT = 7, /**< Scan Controller state machine timeout */
	RSM_EVENT_CAUSE_LAST /**< not used */
} rsm_event_cause_t;

#define RSM_SYSCON_ERR_STATE_TIMEOUT 1

#define RSM_MISC_STATUS_ANTCON_NOT_CONNECTED_MASK 1
#define RSM_MISC_STATUS_TXCTRL_NOT_AVAILABLE_MASK 2
#define RSM_MISC_STATUS_VOLUME_STARTED 4
#define RSM_MISC_STATUS_SCAN_DEFERRED 8
#define RSM_MISC_STATUS_ACQD_NOT_CONNECTED_MASK 16

typedef struct rsm_syscon {
    unsigned char id;		/**< see rsm_syscon_id_t above */
    union {
		struct rsm_syscon_state {
			unsigned char rcs_state;	/**< current scan controller state (see rsm_syscon_state_t) */
			unsigned char seconds_in_state1; /**< seconds elapsed in current scan state LSB */
			unsigned char seconds_in_state2; /**< seconds elapsed in current scan state MSB */
			unsigned char sweep_num;	/**< current sweep number */
		} state;
		struct rsm_syscon_scan {
			char segname[RSM_MAX_SEGNAME_LENGTH];	/**< current scan segment name */
			unsigned char cause;	   	/**< how did scan start - see rsm_event_cause_t above -> tracks iwrf_event_cause_t */
			unsigned char scan_mode;	/**< iwrf_scan_mode */
			unsigned char last_vol_duration1;  /**< duration of last scan in seconds (LSB) */
			unsigned char last_vol_duration2;  /**< duration of last scan in seconds (MSB) */
		} scan;
		struct rsm_syscon_status {
			char misc_status;		/**<  see RSM_MISC_STATUS above, this is LSB from syscon globals.  (rcs->misc_status word)*/
			char syscon_error_code;		/**< see RSM_SYSCON_ERR defs above */
			char txctrl_error_code;		/**< see rsm_txctrl_err_t, above */
		} status;
		struct rsm_syscon_bias {
			unsigned char lsb; /**< least significant byte of bias */
			unsigned char msb; /**< most significant byte of bias */
		} bias; /**< Bias/offset term, a 16-bit signed int,
				  (100ths of a dB for ZDR, 10ths of a degree for PHIDP) */
		char version[RSM_MAX_VER];	/** syscon version number */
    } data;
} rsm_syscon_t;

/*********************************************************************************/

/** 2010 Antenna Controller - status */
/* these items are set by syscon based on info received from the PPMAC antenna controller */
#define RSM_ANTCON "ANTCON"
/* antenna states  -  this must track values defined in deltatau.h - ant_state_t */
typedef enum RSM_ANT_STATE
{	RSM_ANT_NO_STATE=0,
	RSM_ANT_FAULTED,
	RSM_ANT_IDLE,
	RSM_ANT_SCAN_DONE,
	RSM_ANT_HOLD_INIT,
	RSM_ANT_HOLD,
	RSM_ANT_PRE_POSITION,
	RSM_ANT_SECTOR_INIT,
	RSM_ANT_IN_SECTOR,
	RSM_ANT_PPI_AZ_POSITIONING,
	RSM_ANT_PPI_EL_POSITIONING,
	RSM_ANT_PPI_ACCEL,
	RSM_ANT_PPI_IN_SWEEP,
	RSM_ANT_PROG4_DO_SECTOR,
	RSM_ANT_PROG4_DO_SURV,
	RSM_ANT_PROG4_DO_RHI,
	RSM_ANT_DISCONNECTED,
	RSM_ANT_PROG4_FAIL,
	RSM_ANT_LAST_STATE,
} rsm_ant_state_t;

/* define scan events reported by motion control program - must track scan_event_t (deltatau.h) */
typedef enum rsm_scan_event {
	RSM_ANT_EVENT_NOT_SET=0,
	RSM_ANT_EVENT_AZ_IN_POSITION=1,
	RSM_ANT_EVENT_EL_IN_POSITION=2,
	RSM_ANT_EVENT_ANTENNA_IN_POSITION=3,
	RSM_ANT_EVENT_BEGIN_SWEEP=4,
	RSM_ANT_EVENT_END_SWEEP=5,
	RSM_ANT_EVENT_ERROR_SIGNAL=6,
	RSM_ANT_EVENT_ELEVATION_RANGE_ERROR=7,
	RSM_ANT_EVENT_START_COMMAND=8,
	RSM_ANT_EVENT_LAST=9, 
} rsm_scan_event_t;

typedef enum rsm_antcon_id {	/**< describes the union element in the rsm_antcon packet */
	RSM_ANTCON_ID_AZELONLY, /**< az and el positions only */
	RSM_ANTCON_ID_STATE,    /**< +  data.state */
	RSM_ANTCON_ID_SERVO,    /**< +  data.servo */
	RSM_ANTCON_ID_MOTION,   /**< +  data.motion */
	RSM_ANTCON_ID_ERROR,    /**< + error counts */
	RSM_ANTCON_ID_MOTION_UPDATE_RATE,  /**< rate at which motion structs are sent */
	RSM_ANTCON_ID_VERSION, /**< + version string */
}  rsm_ant_id_t;

/**< antcon error_code bits */
#define	RSM_ANTCON_ERR_ANTENNA_FAULTED 1
#define	RSM_ANTCON_ERR_ANTENNA_SEQ_ERROR 2

/* define Motor Status Bits for ant_report_t az/el status - these must match those defined in deltatau.h */
#define RSM_ACON_MS_FE_FATAL 2
#define RSM_ACON_MS_MINUS_LIMIT 4
#define RSM_ACON_MS_PLUS_LIMIT 8
#define RSM_ACON_MS_DES_VEL_ZERO 16
#define RSM_ACON_MS_CLOSED_LOOP 32
#define RSM_ACON_MS_AMP_ENA 64

typedef struct rsm_antcon {
    unsigned char id;		/**< see rsm_ant_it_t */
    unsigned char current_az1;	/** current az LSB, 100ths of a degree */
    unsigned char current_az2;	/** current az MSB, 100ths of a degree */
    unsigned char current_el1;	/** current el LSB, 100ths of a degree */
    unsigned char current_el2;	/** current el MSB, 100ths of a degree */
    union {
		struct rsm_antcon_state {
			unsigned char ant_state;	/**< antenna controller state  -- see deltatau.h */
			unsigned char sweep_num;	/**< current sweep number */
			unsigned char scan_mode; 	/** current iwrf scan_mode */	
			unsigned char fixed_angle1;	/** current fixed_angle LSB, 100ths of a degree */
			unsigned char fixed_angle2;	/** current fixed_angle MSB, 100ths of a degree */
		} state;
		struct rsm_antcon_servo {
			unsigned char az_servo;		/**< bit fields for servo status (RSM_ACON_MS_ defines) */
			unsigned char el_servo;		/**< bit fields for servo status (RSM_ACON_MS_ defines) */
			unsigned char last_event;	/** last antenna event detected by rticplc */
		} servo;
		struct rsm_antcon_motion {
			unsigned char az_rate1;		/** current az rate LSB 100ths deg/sec */
			unsigned char az_rate2;		/** current az rate MSB 100ths deg/sec */
			unsigned char el_rate1;		/** current el rate LSB 100ths deg/sec */
			unsigned char el_rate2;		/** current el rate MSB 100ths deg/sec */
			unsigned char az_following_err1;	/** az fe LSB 100ths of a deg */
			unsigned char az_following_err2;	/** az fe MSB 100ths of a deg */
			unsigned char el_following_err1;	/** el fe LSB 100ths of a deg */
			unsigned char el_following_err2;	/** el fe MSB 100ths of a deg */
		} motion;
		unsigned char motion_update_rate;		/** number of rsm_antcon_motion structures to send per second */
		struct rsm_antcon_error {
			unsigned char error_code;		/** bit mask of error conditions, see RSM_ANCTON_ERR above */
			unsigned char unexpected_event_cnt1;  /**< total count of unexpected events on ant controller LSB */
			unsigned char unexpected_event_cnt2;  /**< total count of unexpected events on ant controller MSB */
			unsigned char state_sync_errcnt1;  /**< total times syscon fell behind in tracking ant_state */
			unsigned char state_sync_errcnt2;  /**< total times syscon fell behind in tracking ant_state */
		} error;
		char version[RSM_MAX_VER];	/** antcon version number */
    } data;
} rsm_antcon_t;


/* Archiver status */
#define RSM_ARCHIVER "ARCH"

typedef enum rsm_archiver_id {
	RSM_ARCHIVER_ID_STATUS = 0,
	RSM_ARCHIVER_ID_FIELDS,
	RSM_ARCHIVER_ID_STATISTICS,
} rsm_archiver_id_t;

typedef enum rsm_archiver_status {
	RSM_ARCHIVER_OFFLINE = 0,
	RSM_ARCHIVER_IDLE,
	RSM_ARCHIVER_WRITING,
	RSM_ARCHIVER_ERROR,
	RSM_ARCHIVER_DISKFULL
} rsm_archiver_status_t;

#define RSM_MAX_ARCH_FLDLEN 200
#define RSM_MAX_ARCH_FILENAMELEN 200

typedef struct rsm_archiver {
	unsigned char id; /**< Set from rsm_archiver_id_t */
	union {
		struct {
			char msg[RSM_MAX_STATUS]; /**< Status message */
			unsigned char state; /**< Set from rsm_archiver_status_t */
		} state;
		char flds[RSM_MAX_ARCH_FLDLEN]; /**< Archived fields */
		struct {
			unsigned char megabytes_written_1; /**< Byte 1, number of MBs written */
			unsigned char megabytes_written_2; /**< Byte 2, number of MBs written */
			unsigned char megabytes_written_3; /**< Byte 3, number of MBs written */
			unsigned char megabytes_written_4; /**< Byte 4, number of MBs written */
			unsigned char files_written_lsb; /**< LSB of number of files written */
			unsigned char files_written_msb; /**< MSB of number of files written */
			char last_file_written[RSM_MAX_ARCH_FILENAMELEN]; /**< File name of the
				last file successfully written to disk */
			unsigned char disk_space_used_gb_lsb; /**< LSB of the disk space used, in gigabytes */
			unsigned char disk_space_used_gb_msb; /**< MSB of the disk space used, in gigabytes */
			unsigned char disk_space_total_gb_lsb; /**< LSB of the total disk space, in gigabytes */
			unsigned char disk_space_total_gb_msb; /**< MSB of the total disk space, in gigabytes */
		} statistics;
	} data;
} rsm_archiver_t;

/* Archiver status */
#define RSM_ARCHIVER_VERSION "ARCH_VER"

typedef struct rsm_archiver_version {
	char ver[RSM_MAX_STATUS];
} rsm_archiver_version_t;

/* Instrument server status */
#define RSM_INS "INS"

typedef enum rsm_ins_svr_status {
	RSM_INS_IDLE = 0,
	RSM_INS_POWERMEAS,
	RSM_INS_SETSTALO,
	RSM_INS_SETTEST,
	RSM_INS_INITIALIZING,
	RSM_INS_FCMEAS,
	RSM_INS_SETFC,
} rsm_ins_svr_status_t;

typedef struct rsm_ins {
	unsigned char state; /**< Set from rsm_ins_svr_status_t */
	char uptime_min;
	char uptime_hrs;
	char uptime_days;
} rsm_ins_t;

/* Instrument server version */
#define RSM_INS_VER "INSVER"

typedef struct rsm_ins_version {
	char ver[RSM_MAX_VER]; /**< Version string */
} rsm_ins_version_t;

/* Instrument status - Power meters*/
#define RSM_INS_PM "INS_PM"

typedef enum rsm_ins_pm_status {
	RSM_INS_PM_OK = 0,
	RSM_INS_PM_FAULT
} rsm_ins_pm_status_t;

typedef struct rsm_ins_pm {
	unsigned char state; /**< Set from rsm_ins_pm_status_t */
	char htxp1; /**< H Transmitter Power in 1/100 dBm, byte 1 (LSB) */
	char htxp2; /**< H Transmitter Power in 1/100 dBm, byte 2 (MSB) */
	char vtxp1; /**< V Transmitter Power in 1/100 dBm, byte 1 (LSB) */
	char vtxp2; /**< V Transmitter Power in 1/100 dBm, byte 2 (MSB) */
  /*----------------------ADDED----------------------------------------------*/
	char atxp1;
        char atxp2;
        char btxp1;
        char btxp2;
  /*-----------------------------------------------------------------------*/
} rsm_ins_pm_t;

/* Instrument query/status - Power meter settings */
#define RSM_INS_PMSET "INS_PMSET"

typedef struct rsm_ins_pmset {
	char auto_avg; /**< 0 - Manual averaging, 1 - Auto averaging */
	char avg; /**< Number indicating the averaging mode */
	unsigned char freq1; /**< Frequency in MHz, LSB */
	unsigned char freq2; /**< Frequency in MHz, MSB */
} rsm_ins_pmset_t;

/* Instrument query/status - Power meter type/version */
#define RSM_INS_PMVER "INS_PMVER"

/* TODO: Verify this! */
#define RSM_MAX_VER_STR 56

typedef struct rsm_ins_pmver {
	char v_version[RSM_MAX_VER_STR]; /**< Version ID of V channel PM */
	char h_version[RSM_MAX_VER_STR]; /**< Version ID of H channel PM */
 /*-----------------------ADDED-----------------------------------*/
	char a_version[RSM_MAX_VER_STR]; /**< Version ID of A channel PM */
	char b_version[RSM_MAX_VER_STR]; /**< Version ID of B channel PM */
  /*-------------------------------------------------------------------*/
} rsm_ins_pmver_t;

/* Instrument status - Test Set */
#define RSM_INS_TS "INS_TS"

typedef enum rsm_ins_ts_status {
	RSM_INS_TS_OK = 0,
	RSM_INS_TS_FAULT
} rsm_ins_ts_status_t;

typedef enum rsm_ins_ts_mode {
	RSM_INS_TS_MODE_CW = 0,
	RSM_INS_TS_MODE_PULSED
} rsm_ins_ts_mode_t;

typedef struct rsm_ins_testset {
	unsigned char state; /**< Set from rsm_ins_ts_status_t */

	unsigned char testf1; /**< Frequency in Hz, byte 1 (LSB) */
	unsigned char testf2; /**< Frequency in Hz, byte 2 */
	unsigned char testf3; /**< Frequency in Hz, byte 3 */
	unsigned char testf4; /**< Frequency in Hz, byte 4 (MSB) */

	char testp1; /**< Power in 1/100 dBm, byte 1 (LSB) */
	char testp2; /**< Power in 1/100 dBm, byte 2 (MSB) */

	char mode; /**< Set from rsm_ins_ts_mode_t */
	char output_mode; /**< 0 if output is off, 1 if it's on */
} rsm_ins_testset_t;

/* Instrument status - Test Set */
#define RSM_INS_TSVER "INS_TSVER"

typedef struct rsm_ins_tsver {
	char version[RSM_MAX_VER_STR]; /**< Version ID of Instrument */
} rsm_ins_tsver_t;

/* Instrument status - Power meters*/
#define RSM_INS_FC "INS_FC"

typedef enum rsm_ins_fc_status {
	RSM_INS_FC_OK = 0,
	RSM_INS_FC_FAULT
} rsm_ins_fc_status_t;

typedef struct rsm_ins_fc {
	unsigned char state; /**< Set from rsm_ins_fc_status_t */
	unsigned char freq6; /**< MSB of frequency */
	unsigned char freq5;
	unsigned char freq4;
	unsigned char freq3;
	unsigned char freq2;
	unsigned char freq1; /**< LSB of frequency */
} rsm_ins_fc_t;

/* Instrument query/status - Power meter settings */
#define RSM_INS_FCSET "INS_FCSET"

typedef enum rsm_ins_fc_inputmode {
	RSM_INS_FC_INPUTMODE_AUTO = 0,
	RSM_INS_FC_INPUTMODE_MANUAL,
	RSM_INS_FC_INPUTMODE_LOWZ,
	RSM_INS_FC_INPUTMODE_HIGHZ,
	RSM_INS_FC_INPUTMODE_LAST
} rsm_ins_fc_inputmode_t;

typedef enum rsm_ins_fc_fmrate {
	RSM_INS_FC_FMRATE_NORMAL = 0,
	RSM_INS_FC_FMRATE_LOW,
	RSM_INS_FC_FMRATE_TRACK,
	RSM_INS_FC_FMRATE_LAST
} rsm_ins_fc_fmrate_t;

typedef struct rsm_ins_fcset {
	unsigned inputmode : 2; /**< From rsm_ins_fc_inputmode_t */
	unsigned resolution : 3; /**< Frequency resolution in powers of 10 */
	unsigned hires : 1; /**< Set to 1 to enable high resolution mode */
	unsigned fmrate : 2; /**< FM rate tracking, from rsm_ins_fc_fmrate_t */
	unsigned char freq_lsb; /**< LSB of center frequency, in manual mode */
	unsigned char freq_msb; /**< MSB of center frequency, in manual mode */
} rsm_ins_fcset_t;

/* Instrument query/status - Power meter type/version */
#define RSM_INS_FCVER "INS_FCVER"

typedef struct rsm_ins_fcver {
	char version[RSM_MAX_VER_STR]; /**< Version ID of Instrument */
	char serial[RSM_MAX_VER_STR]; /**< Serial number of Instrument */
} rsm_ins_fcver_t;



/* Instrument status - STALO */
#define RSM_INS_STALO "INS_STALO"

typedef enum rsm_ins_stalo_mode {
	RSM_INS_STALO_OK = 0,
	RSM_INS_STALO_FAULT
} rsm_ins_stalo_mode_t;

typedef struct rsm_ins_stalo {
	unsigned char state; /**< Set from rsm_ins_stalo_status_t */

	unsigned char stalof1; /**< Frequency in Hz, byte 1 (LSB) */
	unsigned char stalof2; /**< Frequency in Hz, byte 2 */
	unsigned char stalof3; /**< Frequency in Hz, byte 3 */
	unsigned char stalof4; /**< Frequency in Hz, byte 4 (MSB) */
} rsm_ins_stalo_t;

/* Instrument status - STALO */
#define RSM_INS_STALOVER "INS_STVER"

typedef rsm_ins_pmver_t rsm_ins_stalover_t;

/* Moment Server status */
#define RSM_MSERV "DSP"

/* Indicates the type of moment server message */
typedef enum rsm_mserv_id {
	RSM_MSERV_ID_STATUS, /**< Status message */
	RSM_MSERV_ID_SCAL, /**< Solar Calibration message */
	RSM_MSERV_ID_PCAL, /**< Power Calibration message */
	RSM_MSERV_ID_NCAL, /**< Noise Calibration message */
	RSM_MSERV_ID_CONNECT, /**< SDB connection message */
	RSM_MSERV_ID_Z_THRESHOLD, /**< Z threshold message */
	RSM_MSERV_ID_ZDR_THRESHOLD, /**< ZDR threshold message */
	RSM_MSERV_ID_LDR_THRESHOLD, /**< LDR threshold message */
	RSM_MSERV_ID_W_THRESHOLD, /**< W threshold message */
} rsm_mserv_id_t;

typedef enum rsm_mserv_conn {
	RSM_MSERV_CONN_SDB_CONNECT, /**< SDB connection */
	RSM_MSERV_CONN_SDB_DISCONNECT, /**< SDB disconnection */
	RSM_MSERV_CONN_VCHILL_CONNECT, /**< VCHILL connection */
	RSM_MSERV_CONN_VCHILL_DISCONNECT, /**< VCHILL disconnection */
} rsm_mserv_conn_t;

typedef struct rsm_mserv {
	unsigned char msg_id;
	union {
		char msg[RSM_MAX_STATUS];
		char threshold;
		struct {
			char v_co_db_low;
			char v_co_db_high;
			char h_co_db_low;
			char h_co_db_high;
			char v_cx_db_low;
			char v_cx_db_high;
			char h_cx_db_low;
			char h_cx_db_high;
			
			unsigned char cal_time_1; /**< LSB of UNIX time, last cal */
			unsigned char cal_time_2; /**< Byte 2 of UNIX time, last cal */
			unsigned char cal_time_3; /**< Byte 3 of UNIX time, last cal */
			unsigned char cal_time_4; /**< MSB of UNIX time, last cal */
		} cal;
		struct {
			char connect; /**< One of rsm_mserv_conn_t */
			char hostname[RSM_MAX_STATUS - 1];
		} conn;
	} data;
} rsm_mserv_t;

/* Moment server version */
#define RSM_MSERV_VER "DSPVER"

typedef struct rsm_mserv_version {
	char ver[RSM_MAX_VER]; /**< Version string */
} rsm_mserv_version_t;

/* Transmitter status */
#define RSM_XMIT "XMIT"

typedef enum rsm_xmit_status {
	RSM_XMT_STATUS_IDLE = 0,
	RSM_XMT_STATUS_UPDATING,
	RSM_XMT_STATUS_LAST
} rsm_xmit_status_t;

typedef enum rsm_xmit_polmode {
	RSM_XMT_POLMODE_H = 0,
	RSM_XMT_POLMODE_V,
	RSM_XMT_POLMODE_VH,
	RSM_XMT_POLMODE_VHS,
	RSM_XMT_POLMODE_LAST
} rsm_xmit_polmode_t;

typedef struct rsm_xmit {
	unsigned char state; /**< Set from rsm_xmit_status_t */
	unsigned unused : 5; /**< Unused bits */
	unsigned xmit_enabled : 1; /**< Transmitters enabled */
	unsigned dprt : 1; /**< Dual PRT mde enabled */
	unsigned cal_relay_pos : 1; /**< Calibration Relay position */
	
	char polmode; /**< 3-letter code for polarization mode */
	unsigned char prt_low;  /**< Low byte of PRT */
	unsigned char prt_high; /**< High byte of PRT */
	unsigned char prt2_low;  /**< Low byte of PRT2 */
	unsigned char prt2_high; /**< High byte of PRT2 */
	unsigned char intcycle; /**< Integration cycles */
	unsigned char ptv_low;  /**< Low byte of V pretrigger, in ns */
	unsigned char ptv_high; /**< High byte of V pretrigger, in ns */
	unsigned char pth_low;  /**< Low byte of H pretrigger, in ns */
	unsigned char pth_high; /**< High byte of H pretrigger, in ns */
	unsigned char tploc_low; /**< LSB of test pulse location, in us */
	unsigned char tploc_high; /**< MSB of test pulse location, in us */
	unsigned char tpwidth_low; /**< LSB of test pulse width, in ns */
	unsigned char tpwidth_high; /**< LSB of test pulse width, in ns */
} rsm_xmit_t;

/** Transmitter server version */
#define RSM_XMIT_VER "XMITVER"

typedef struct rsm_xmit_version {
	char ver[RSM_MAX_VER]; /**< Version string */
} rsm_xmit_version_t;

/** Transmitter status */
#define RSM_DUAL_XMIT "DXMIT"

typedef enum rsm_dual_xmit_sel {
	RSM_DUAL_XMIT_SEL_1 = 0,
	RSM_DUAL_XMIT_SEL_2,
	RSM_DUAL_XMIT_LAST_SEL
} rsm_dual_xmit_sel_t;

/** Switch points to transmitter 1 */
#define RSM_DUAL_XMIT_SWITCH_POS_H (1 << 1)
/** Switch points to transmitter 2 */
#define RSM_DUAL_XMIT_SWITCH_POS_V (1 << 0)

typedef struct rsm_dual_xmit {
	unsigned char sel; /**< Which transmitter to select, from dual_xmit_sel_t */
	unsigned char interlock; /**< If set to 1, disable transmitter interlocks */
	unsigned char enable_xmit_1; /**< Enable transmitter 1 */
	unsigned char enable_xmit_2; /**< Enable transmitter 2 */
	unsigned char switch_1_in_pos; /**< Switch for transmitter 1 set, from RSM_DUAL_XMIT_SWITCH_POS_* */
	unsigned char switch_2_in_pos; /**< Switch for transmitter 2 set, from RSM_DUAL_XMIT_SWITCH_POS_* */
} rsm_dual_xmit_t;

/** Timeseries archiver */
#define RSM_TSARCH "TSARCH"

/* Indicates type of timeseries archiver message */
typedef enum rsm_tsarch_id {
	RSM_TSARCH_ID_MODE, /**< TS archive mode */
	RSM_TSARCH_ID_FILENAME, /**< Updating archive file name */
	RSM_TSARCH_ID_STATUS, /**< Status */
	RSM_TSARCH_ID_MAXGATES, /**< Max gates to archive */
	RSM_TSARCH_ID_VERSION, /**< Server version */
	RSM_TSARCH_LAST_ID
} rsm_tsarch_id_t;

typedef enum rsm_tsarch_mode {
	RSM_TSARCH_MODE_OFF,
	RSM_TSARCH_MODE_AUTO,
	RSM_TSARCH_MODE_ON,
	RSM_TSARCH_LAST_MODE
} rsm_tsarch_mode_t;

typedef enum rsm_tsarch_state {
	RSM_TSARCH_STATE_IDLE, /**< Disabled */
	RSM_TSARCH_STATE_WAITING, /**< Enabled, waiting for start of scan */
	RSM_TSARCH_STATE_RECORDING, /**< Enabled, writing data */
	RSM_TSARCH_LAST_STATE
} rsm_tsarch_state_t;

#define RSM_MAX_TSARCH_FILENAMELEN 200

typedef struct rsm_tsarch {
	unsigned char id;
	union {
		unsigned char mode; /**< Operating mode, one of RSM_TSARCH_MODE_* */
		unsigned char filename[RSM_MAX_TSARCH_FILENAMELEN]; /**< Current file being archived */
		struct {
			unsigned char state; /**< One of the RSM_TSARCH_STATE_* */
			unsigned char throughput; /**< Disk I/O rate, in 100s of kb/sec */
			unsigned char buffer_level; /**< Value between 0 and 10 indicating memory buffer depth */
		} status; /**< Status, if msg_id is RSM_TSARCH_ID_STATUS */
		struct {
			unsigned char low;
			unsigned char high;
		} maxgates; /**< Maximum number of gates, if msg_id is RSM_TSARCH_ID_MAXGATES */
		char version[RSM_MAX_VER_STR];
	} data;
} rsm_tsarch_t;

/*
 * S-Pol monitoring and control 
 */
#define RSM_SPOL_MON_CTL "SPOL_MON_CTL"
typedef enum rsm_status {
	RSM_STATUS_OK = 0, 		/**< value is OK */
      RSM_STATUS_WARN = 1, 		/**< warn user value is marginal */
	RSM_STATUS_CRITICAL = 2, 	/**< value is critical */
	RSM_STATUS_UNKNOWN = 3, 	/**< value has unknown state */
} rsm_status_t;

typedef struct scaled_float_val {
 char msb;	/**< most significant byte */
 char lsb;
 unsigned char scale;   /** float_val = ((msb << 8) + lsb) / scale */
 unsigned char status;  /**< rsm_status_t */
} scaled_float_val_t;

typedef struct rsm_spol_named_dig_io {
char name[MAX_DIGIO_NAME];    
char msb;    /**< most significant byte */
char lsb;    /**< least significant byte */
} rsm_spol_named_dig_io_t;


typedef struct rsm_spol_temp {
 scaled_float_val_t klystron_temp;
 scaled_float_val_t shelter_ac_output_temp;
 scaled_float_val_t shelter_ambient_temp;
 scaled_float_val_t temperature_One;
 scaled_float_val_t temperature_Two;
 scaled_float_val_t temperature_Three;
 scaled_float_val_t temperature_Four;
 scaled_float_val_t temperature_Zero;
} rsm_spol_temp_t;

typedef struct rsm_spol_analog {
 scaled_float_val_t xmit_v1;
 scaled_float_val_t xmit_v2;
 scaled_float_val_t value3;
 scaled_float_val_t value4;
 scaled_float_val_t check_voltage;
 scaled_float_val_t check_voltage_AC;
 scaled_float_val_t check_voltage_DC;
 scaled_float_val_t check_voltage_High;
 scaled_float_val_t check_voltage_Low;
 scaled_float_val_t check_voltage_Trans;
 scaled_float_val_t check_power_one;
 scaled_float_val_t check_power_two;

} rsm_spol_analog_t;

/* digital values read for S-Pol */
typedef struct rsm_spol_dig_read {
 char waveguide_pressure_low;	/**< waveguide pressure below threshold */
 char shelter_power_fail;
 char antenna_not_scanning;
 char smoke_alarm;
}rsm_spol_dig_read_t;

typedef enum rsm_dig_id {
  RSM_SPOL_DIG_ID_XMIT_POWER_ENABLE,         /**< writing 0 disables transmitter */
  RSM_SPOL_DIG_ID_XMIT_RESET_FAULT,          /**< writing 1 resets transmitter fault */
  RSM_SPOL_DIG_ID_SERVO_RESET_FAULT,	      /**< writing 1 resets antenna servo fault */
} rsm_dig_id_t;


/* digital values read/written for S-Pol */
typedef struct rsm_spol_dig_io {
  char id; 	/**< from rsm_dig_it_t */ 
  char pad;
  char msb;	/**< most significant byte */
  char lsb;	/**< least significant byte */
} rsm_spol_dig_io_t;

/* transmitter status for S-Pol - see NWS EHB 6-511 
 * the NEXRAD transmitter has 8 bytes of digital fault status, which are read
 * by setting the 3 select lines to the values 0-7
 */
typedef enum rsm_spol_xmit_bits {
    RSM_SPOL_XMIT_AUTO_FAULT_CLEAR=0,
    RSM_SPOL_XMIT_HARD_FAULT=1,
    RSM_SPOL_XMIT_HV_ON=2,

}rsm_spol_xmit_bits_t;

#define NUM_SPOL_XMIT_FAULT_BYTES 8
typedef struct rsm_spol_transmitter_status {
  char faultData[NUM_SPOL_XMIT_FAULT_BYTES]; /**< digital fault data - select = 0*/
  char status;  /**< bit 0 - transmitter clearing fault */
  	        /**< bit 1 - transmitter can't clear fault */
  	        /**< bit 2 - HV On/Off */
  char spare0;
  char spare1;
  char spare2;
  char spare3;

} rsm_spol_transmitter_status_t;


typedef enum rsm_spol_mon_id {
RSM_SPOL_MON_ID_TEMP,
RSM_SPOL_MON_ID_ANALOG,
RSM_SPOL_MON_ID_DIG_READ,
RSM_SPOL_MON_ID_DIG_IO,
RSM_SPOL_MON_ID_DIG_NAME,
RSM_SPOL_MON_ID_XMIT_STATUS
} rsm_spol_mon_id_t;

typedef struct rsm_spol_mon {
unsigned char id;  /**< rsf_spol_mon_id_t */
union {
rsm_spol_temp_t  temp; 
rsm_spol_analog_t analogs; 
rsm_spol_dig_read_t dig_read; 
rsm_spol_dig_io_t dig_io; 
rsm_spol_named_dig_io_t dig_name;
rsm_spol_transmitter_status_t xmit_status;

} data;

} rsm_spol_mon_t;


/**
\brief RSM message payload

The payload can be one of several types, the type is selected using the module
identifier
*/
typedef union rsm_payload {
	char generic_text[RSM_MAX_TEXT]; /**< Generic ASCII text message (GEN) */
	rsm_acq_t acq; /**< Acquisition status (ACQ) */
	rsm_acq_version_t acqver; /**< Acquisition server version (ACQVER) */
	rsm_acq_angles_t angles; /**< Antenna angles (ACQANG) */
	rsm_acq_txsample_t txsample; /**< Transmitter sample (ACQTXSMP) */
	rsm_archiver_t arch; /**< Archiver status (ARCH) */
	rsm_archiver_version_t archver; /**< ARCH_VER */
	rsm_ins_t ins; /**< Instrument server (INS) */
	rsm_ins_pm_t pm; /**< Power meter (INS_PM) */
	rsm_ins_pmset_t pmset; /**< Power meter settings (INS_PM_SET) */
	rsm_ins_pmver_t pmver; /**< Power meter version (INS_PM_VER) */
	rsm_ins_testset_t ts; /**< Test set (INS_TS) */
	rsm_ins_tsver_t tsver;
	rsm_ins_stalo_t stalo; /**< Test set (INS_STALO) */
	rsm_ins_stalover_t stalover;
	rsm_ins_version_t insver;
	rsm_ins_fc_t fc; /**< Frequency counter (INS_FC) */
	rsm_ins_fcset_t fcset; /**< Frequency counter settings (INS_FCSET) */
	rsm_ins_fcver_t fcver; /**< Frequency counter version (INS_FCVER) */

	rsm_spol_mon_t spol_mon; /**< S-Pol monitoring and control */
	
	rsm_xmit_t xmit; /**< Transmitter (XMIT) */
	rsm_xmit_version_t xmit_ver;  /**< Transmitter version (XMIT_VER) */
	rsm_mserv_t mserv; /**< Moment Server (DSP) */
	rsm_mserv_version_t mservver; /**< Moment Server version (DSPVER) */
        rsm_syscon_detail_t scdet; /**< DTcontrol System Controller Details */
        rsm_syscon_status_t scstat; /**< DTcontrol System Controller Status */
        rsm_syscon_version_t scver; /**< DTcontrol System Controller Version */
	rsm_syscon_t syscon; /**< 2010 System Controller info */
	rsm_antcon_t antcon; /**< 2030 Antenna Controller info  */
	rsm_dual_xmit_t dxmit; /**< Dual transmitter control (DXMIT) */
	rsm_tsarch_t tsarch; /**< Time series archiver (TSARCH) */
} rsm_payload_t;

typedef struct rsm_pkt {
	rsm_msghdr_t header;
	rsm_payload_t payload;
} rsm_pkt_t;

int rsm_init(rsm_t *rsm, char *multicast_addr, int port);
void rsm_close(const rsm_t *rsm);
int rsm_recvdata(rsm_t *rsm, char **msgid, rsm_pkt_t *pkt);
int rsm_senddata(rsm_t *rsm, unsigned int length, rsm_msgtype_t type, const char *msgid, rsm_pkt_t *pkt);
int rsm_waitfordata(rsm_t *rsm, unsigned int timeout);

#ifdef __cplusplus
}
#endif 

#endif /* _RSM_H_ */

