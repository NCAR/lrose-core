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
/*************************************************************************
 *
 *                             mpar.h           
 *
 *
 *      Mon May  2 14:41:58 1997                                
 *
 *      Description: mpar.h header file                          
 *      Author: blackburn                
 *
 *      See Also:  tdwr.h header file
 *      Modification History:          
 *
 *************************************************************************/


#include <dataport/port_types.h>

#define MAX_NUM_BEAMS    60

#define BEGINNING_OF_VOL_SCAN 16384     /*bit 14 set denotes new volume*/
#define END_OF_VOL_SCAN 32768		/*bit 15 set denotes end of volume*/
#define START_OF_NEW_ELEVATION 4194304  /* bit 22 set denotes new elevation */
#define SCAN_STRATEGY 255		/* bits 0 - 7 denotes scan strategy */
#define SECTOR_SCAN 256			/* bit 8 denotes sector scan */
#define LOW_PRF 1			/* bit 0 set denotes a low prf scan */
#define DUMMY_RECORD 4			/* bit 2 set indicates a dummy record */
#define SCAN_RESTART 1			/* bit 0 set indicates a elevation
					* scan restart - record will contain
					* no base data */
#define VOL_RESTART 2			/* bit 1 set indicates a volume
					* scan restart - record will contain
					* no base data */
#define INCOMP_ELEV 4			/* bit 2 set indicates a incomplete
					* elevation - record will contain
					* no base data */
#define INCOMP_VOL 8			/* bit 3 set indicates a incomplete
					* volume - record will contain
					* no base data */
#define SECTOR_BLANK 32768

#define MONITOR		1
#define HAZARDOUS	2
#define	CLEAR_AIR	3
#define CLUTTER_COL	4
#define CLUTTER_EDIT	5
#define PPI 		6
#define RHI 		7

#define MAX_REC_LEN	100000

#define MPAR_VEL_SCALE 25
#define MPAR_VEL_BIAS -8000
#define RANGE_TO_FIRST_GATE 926

#define SNR_SCALE 50
#define SNR_BIAS 0
#define DBZ_SCALE 50
#define DBZ_BIAS -3000
#define VEL_SCALE 31			/* original 16 bit data has a scale */
#define VEL_BIAS -4000			/* 25 with the bias of -80 */
#define SW_SCALE 25
#define SW_BIAS 0

#define PULSE_WIDTH	1100 
#define FREQUENCY	2785 /* mpar-> average between F1 and F1; 2.775, 2.795*/

/*
 * definitions for tdwr input data
#define NORMAL_PRF_BASE_DATA 0x2B00
#define LOW_PRF_BASE_DATA 0x2B01
#define LLWASII_DATA 0x2C02
#define LLWASII_MAPPING 0x4206
#define LLWASIII_DATA 0x2C01
 */

/* MPAR Message ID values */
#define MB_MOMENTS_DATA 0xD02
#define COURSE_GRAIN_MOMENTS_DATA 0xD03
#define MB_CLEAR_AIR_MAP 0x1A04
#define COURSE_GRAIN_CLEAR_AIR_MAP 0x1A05
#define SEND_LAST_TILT  0xffff


typedef struct
{
	ui32  fr_seq;             /* sequence # */
	ui16  fr_msg_length;	/* total msg length (bytes) (w/o hdr) */
	ui16  fr_frames_per_msg;  /* total frames for message */
	ui16  fr_frame_num_in_msg;    /* frame # in message */
	ui16  fr_frame_offset;  /*offset from msg start (bytes) */
	ui16  fr_frame_length;  /* length of packet - hdr (bytes) */
	ui16  fr_options;	  /* optional flag bits */
} Packet_hdr;

typedef struct
{
	ui16 message_id;
	ui16 message_length;
	ui16 volume_count;
	ui16 volume_flag;	/* the first 8 bits denote the
					* scan strategy. Bit 14 -start new
					* volume, bit 15 - volume end
					*/
		
	ui16	power_trans;	/* peak transmitter power */
	ui16  playback_flag;	/* flags for: start of playback,
					* start of live and a dummy record 						* indicater */
        ui32 scan_info_flag;		/* flags to specify low prf, 
					* gust front, MB surface, low
					* elevation, wind shift, precip 
					* and resolution, MB aloft, 
					* sector, velocity dealiasing,
					* spike removal, obscuration 
					* flagging, 8 bits reserved.
    					* flags for clutter map number,
					* start of elevation, end of elev
					* and contains the scan number 
					*/

	fl32		current_elevation;
	/*fl32 		angular_scan_rate*/
	ui16	beam_width;
	ui16	gate_spacing;
	ui16	pri;		/* Pulse Repetion Interval */
	ui16	dwell_flag;	/* pulses per dwell, solar 
					* indicator, and dwell ID
					*/
	ui16 final_range_sample;	/* specifies the last range 
						* sample that contains a valid 
						* radar return
						*/
	ui16   rng_samples_per_dwell;	
	fl32 		azimuth;
	fl32 		total_noise_power;  /* total noise power for each dwell
					     * compensated for the effects
					     * of solar flux
					     */
	ui32		timestamp;
	ui32		beam_number;

	/*ui16  base_data_type;*/  /* indicates unconditioned, edited
					      * and fully conditioned data types
					      */
	/*ui16 vol_elev_status_flag;*/  /* indicates incomplete and 
					* restarted volumes and elevations
					*/
	ui16	interger_azimuth;
	ui16	empty_short;
} MPAR_data_header;

