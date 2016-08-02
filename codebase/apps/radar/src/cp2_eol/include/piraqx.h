/********************************************************/
/* definitions for accessing packet data and parameters */
/********************************************************/
#pragma once
#include <windows.h>
#include "dd_types.h"

#define	MAGIC		0x12345678

typedef struct  {float x,y;}   complex;

#define K2      0.93
#define C       2.99792458E8
#define M_PI    3.141592654 
#define TWOPI   6.283185307

#define	MAXPACKET	1200
#define	MAXGATES		2000  
#define MAXHITS 1024
#define PRODS_ELEMENTS  16      /* number of elements in prods array */
#define	DATA_TIMEOUT	1

/****************************************************/
/* set up the infra structure for software FIFO's  */
/****************************************************/
#pragma pack(4) // set 4-byte alignment of structures for compatibility w/destination computers
typedef struct {
    uint4 magic;             /* must be 'MAGIC' value above */
    uint4 type;             /* e.g. DATA_SIMPLEPP, defined in piraq.h */
    uint4 sequence_num;     /* increments every beam */
    uint4 totalsize;      /* total amount of data only (don't count the size of this header) */
    uint4 pagesize;       /* amount of data in each page INCLUDING the size of the header */
    uint4 pagenum;		/* packet number : 0 - pages-1 */
    uint4 pages;     /* how many 'pages' (packets) are in this group */
} UDPHEADER;


/****************************************************/
/* set up the infrastructure for intraprocess communication */
/****************************************************/

typedef struct {
	int		type;
	int		count;
	int		flag;		/* done, new, old, handshake, whatever, ..... */
	int		arg[5];
	}	COMMAND;
		
typedef struct {
	UDPHEADER		udphdr;
	char				buf[MAXPACKET];
	} UDPPACKET;

#define MAXNUM 1200

#define PIRAQX_CURRENT_REVISION 1
//struct piraqX_header_rev1
typedef struct 
{
		/* all elements start on 4-byte boundaries
         * 8-byte elements start on 8-byte boundaries
         * character arrays that are a multiple of 4
         * are welcome
         */
#define	PX_MAX_RADAR_DESC	4
    char desc[4];			/* "SVH ", "XH", "XV" */
    uint4 recordlen;        /* total length of record - must be the second field */
    uint4 channel;          /* e.g., RapidDOW range 0-5 */
    uint4 rev;		        /* format revision #-from RADAR structure */
    uint4 one;			    /* always set to the value 1 (endian flag) */
    uint4 byte_offset_to_data;
    uint4 dataformat;

    uint4 typeof_compression;	/*  */
/*
      Pulsenumber (pulse_num) is the number of transmitted pulses
since Jan 1970. It is a 64 bit number. It is assumed
that the first pulse (pulsenumber = 0) falls exactly
at the midnight Jan 1,1970 epoch. To get unix time,
multiply by the PRT. The PRT is a rational number a/b.
More specifically N/Fc where Fc is the counter clock (PIRAQ_CLOCK_FREQ),
and N is the divider number. So you can get unix time
without roundoff error by:
secs = pulsenumber * N / Fc. The
computations is done with 64 bit arithmatic. No
rollover will occur.

The 'nanosecs' field is derived without roundoff
error by: 100 * (pulsenumber * N % Fc).

Beamnumber is the number of beams since Jan 1,1970.
The first beam (beamnumber = 0) was completed exactly
at the epoch. beamnumber = pulsenumber / hits. 
*/
    

#ifdef _TMS320C6X   /* TI doesn't support long long */
    uint4 pulse_num_low;
    uint4 pulse_num_high;
#else
    uint8 pulse_num;   /*  keep this field on an 8 byte boundary */
#endif
#ifdef _TMS320C6X   /* TI doesn't support long long */
    uint4 beam_num_low;
    uint4 beam_num_high;
#else
    uint8 beam_num;	/*  keep this field on an 8 byte boundary */
#endif
    uint4 gates;
    uint4 start_gate;
    uint4 hits;
/* additional fields: simplify current integration */
    uint4 ctrlflags; /* equivalent to packetflag below?  */
    uint4 bytespergate; 
    float4 rcvr_pulsewidth;
#define PX_NUM_PRT 4
    float4 prt[PX_NUM_PRT];
    float4 meters_to_first_gate;  

    uint4 num_segments;  /* how many segments are we using */
#define PX_MAX_SEGMENTS 8
    float4 gate_spacing_meters[PX_MAX_SEGMENTS];
    uint4 gates_in_segment[PX_MAX_SEGMENTS]; /* how many gates in this segment */
    
    

#define PX_NUM_CLUTTER_REGIONS 4
    uint4 clutter_start[PX_NUM_CLUTTER_REGIONS]; /* start gate of clutter filtered region */
    uint4 clutter_end[PX_NUM_CLUTTER_REGIONS];  /* end gate of clutter filtered region */
    uint4 clutter_type[PX_NUM_CLUTTER_REGIONS]; /* type of clutter filtering applied */

#define PIRAQ_CLOCK_FREQ 10000000  /* 10 Mhz */

/* following fields are computed from pulse_num by host */
    uint4 secs;     /* Unix standard - seconds since 1/1/1970
                       = pulse_num * N / ClockFrequency */
    uint4 nanosecs;  /* within this second */
    float4 az;   /* azimuth: referenced to 9550 MHz. possibily modified to be relative to true North. */
    float4 az_off_ref;   /* azimuth offset off reference */ 
    float4 el;		/* elevation: referenced to 9550 MHz.  */ 
    float4 el_off_ref;   /* elevation offset off reference */ 

    float4 radar_longitude;
    float4 radar_latitude;
    float4 radar_altitude;
#define PX_MAX_GPS_DATUM 8
    char gps_datum[PX_MAX_GPS_DATUM]; /* e.g. "NAD27" */
    
    uint4 ts_start_gate;   /* starting time series gate , set to 0 for none */
    uint4 ts_end_gate;     /* ending time series gate , set to 0 for none */
    
    float4 ew_velocity;

    float4 ns_velocity;
    float4 vert_velocity;

    float4 fxd_angle;		/* in degrees instead of counts */
    float4 true_scan_rate;	/* degrees/second */
    uint4 scan_type;
    uint4 scan_num;
    uint4 vol_num;

    uint4 transition;
    float4 xmit_power;

    float4 yaw;
    float4 pitch;
    float4 roll;
    float4 track;
    float4 gate0mag;  /* magnetron sample amplitude */
    float4 dacv;
    uint4  packetflag; 

    /*
    // items from the depricated radar "RHDR" header
    // do not set "radar->recordlen"
    */

    uint4 year;             /* e.g. 2003 */
    uint4 julian_day;
    
#define PX_MAX_RADAR_NAME 16
    char radar_name[PX_MAX_RADAR_NAME];
#define PX_MAX_CHANNEL_NAME 16
    char channel_name[PX_MAX_CHANNEL_NAME];
#define PX_MAX_PROJECT_NAME 16
    char project_name[PX_MAX_PROJECT_NAME];
#define PX_MAX_OPERATOR_NAME 12
    char operator_name[PX_MAX_OPERATOR_NAME];
#define PX_MAX_SITE_NAME 12
    char site_name[PX_MAX_SITE_NAME];
    

    uint4 polarization;
    float4 test_pulse_pwr;
    float4 test_pulse_frq;
    float4 frequency;

    float4 noise_figure;
    float4 noise_power;
    float4 receiver_gain;
    float4 E_plane_angle;  /* offsets from normal pointing angle */
    float4 H_plane_angle;
    

    float4 data_sys_sat;
    float4 antenna_gain;
    float4 H_beam_width;
    float4 V_beam_width;

    float4 xmit_pulsewidth;
    float4 rconst;
    float4 phaseoffset;

    float4 zdr_fudge_factor;

    float4 mismatch_loss;
    float4 rcvr_const;

    float4 test_pulse_rngs_km[2];
    float4 antenna_rotation_angle;   /* S-Pol 2nd frequency antenna may be 30 degrees off vertical */
    
#define PX_SZ_COMMENT 64
    char comment[PX_SZ_COMMENT];
    float4 i_norm;  /* normalization for timeseries */
    float4 q_norm;
    float4 i_compand;  /* companding (compression) parameters */
    float4 q_compand;
    float4 transform_matrix[2][2][2];
    float4 stokes[4]; 
#if 1
	float4 vxmit_power;
    float4 vtest_pulse_pwr; //
    float4 vnoise_power;
    float4 vreceiver_gain;
    float4 vantenna_gain;
    float4 h_rconst;
    float4 v_rconst;
    float4 peak_power;            /* added by JVA -  needed for
                                     v/h_channel_radar_const */
    // additional floats for magnetron parameters: 
    float4 mag_current;         // magnetron current from ASE transmitter status 
    float4 xmtr_enclosure_temp; // 
    float4 mag_est_freq;        // runtime estimated transmitter frequency 
    float4 receiver_freq;       // runtime computed transmitter frequency 
	//	conformal w/piraqx.xls containing EOL standard piraqx definitions to here

	float4 zdr_bias;	//	added 7-25-06; required for S-band zdr calculation
	float4 noise_phase_offset;	//	added 8-1-06; used for S-band velocity, phidp calculations
    float4 i_offset;	/*	dc offsets, for each channel */
    float4 q_offset;
    float4 vi_offset; 
    float4 vq_offset;
	float4 spare[2];
#else    
    float4 i_offset;  /* dc offset, one for each channel? */
    float4 q_offset;
    float4 vi_offset; 
    float4 vq_offset;

    float4 vnoise_power;	/* 'v' second piraq channel */ 
    float4 vreceiver_gain;
    float4 vaz_off_ref;      /* SynthAngle output for 'v' channel */
    float4 vel_off_ref;
    float4 vfrequency;
//    float4 spare[20];
    float4 spare[11];
#endif
    /*
    // always append new items so the alignment of legacy variables
    // won't change
    */

} INFOHEADER;

typedef struct piraqX_header_rev1 PIRAQX;

/****************************************************/
/* now use this infrastructure to define the records in the */
/* various types of FIFO's used for interprocess communication */
/****************************************************/

typedef struct {		/* data that's in the PIRAQ1 FIFO */
	INFOHEADER		info;
	float				data[MAXGATES * 12];
	} DATABLOCK;

typedef struct {
	UDPHEADER	udp;
	COMMAND		cmd;
	DATABLOCK	data;
    } PACKET;

typedef struct {			/* this structure must match the non-data portion of the PACKET structure */
	UDPHEADER	udp;
	COMMAND		cmd;
	INFOHEADER	info;
	} PACKETHEADER;

#pragma pack(8) // return to default 8-byte alignment of structures 

#define	HEADERSIZE		sizeof(PACKETHEADER)
#define	IQSIZE			(sizeof(short) * 4 * MAXGATES) 
#define	ABPSIZE			(sizeof(float) * 12 * MAXGATES)
#define	DATASIZE(a)		(a->data.info.gates * a->data.info.bytespergate)
#define	RECORDLEN(a)		(sizeof(INFOHEADER) + (DATASIZE(a)))
#define	TOTALSIZE(a)		(sizeof(UDPHEADER) + sizeof(COMMAND) + (RECORDLEN(a)))

// CP2: 
#define BUFFER_EPSILON	0	// CP2 space between hits in buffer and within N-hit PCI packet


