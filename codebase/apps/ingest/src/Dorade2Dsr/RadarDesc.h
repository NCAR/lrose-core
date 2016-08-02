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
 *	$Id: RadarDesc.h,v 1.3 2016/03/07 01:23:00 dixon Exp $
 *
 *	Module:		 RadarDesc.h
 *	Original Author: Richard E. K. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date: 2016/03/07 01:23:00 $
 *
 * revision history
 * ----------------
 * Revision 1.5  1992/07/28  17:32:30  thor
 * Removed fixed_angle.
 *
 * Revision 1.4  1992/04/20  17:18:31  thor
 * Latest Eldora/Asterea revisions included.
 *
 * Revision 1.3  1991/10/16  15:33:53  thor
 * Changed variable name.
 *
 * Revision 1.2  1991/10/15  17:56:24  thor
 * Fixed to meet latest version of tape spec.
 *
 * Revision 1.1  1991/08/30  18:39:36  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCRadarDesch
#define INCRadarDesch

#ifdef OK_RPC

#if defined(UNIX) && defined(sun)
#include <rpc/rpc.h>
#else
# if defined(WRS)
#   include "rpc/rpc.h"
# endif
#endif /* UNIX */


#endif /* OK_RPC */

struct radar_d {
    char  radar_des[4];		/* Identifier for a radar descriptor */
				/* block (ascii characters "RADD"). */
    si32  radar_des_length;	/* Length of a radar descriptor block */
				/* in bytes. */
    char  radar_name[8];	/* Eight character radar name. */
    fl32 radar_const;		/* Radar/lidar constant in ?? */
    fl32 peak_power;		/* Typical peak power of the sensor in kw. */
				/* Pulse energy is really the
				 * peak_power * pulse_width */
    fl32 noise_power;		/* Typical noise power of the sensor in dBm. */
    fl32 receiver_gain;	/* Gain of the receiver in db. */
    fl32 antenna_gain;		/* Gain of the antenna in db. */
    fl32 system_gain;		/* System Gain in db.
				 * (Ant G - WG loss) */
    fl32 horz_beam_width;	/* Horizontal beam width in degrees. */
				/* beam divergence in milliradians
				 * is equivalent to beamwidth */
    fl32 vert_beam_width;	/* Vertical beam width in degrees. */
    si16 radar_type;		/* Radar Type (0)Ground, 1)Airborne */
				/* Fore, 2)Airborne Aft, 3)airborne */
				/* Tail, 4)Airborne Lower Fuselage, */
				/* 5)Shipborne. */
    si16 scan_mode;		/* Scan Mode (0)Calibration, 1)PPI */
				/* (constant Elevation) 2)Coplane, */
				/* 3)RHI (Constant Azimuth), 4) */
				/* Vertical Pointing, 5)Target */
				/* (Stationary), 6)Manual, 7)Idle (Out */
				/* of Control). */
    fl32 req_rotat_vel;	/* Requested rotational velocity of */
				/* the antenna in degrees/sec. */
    fl32 scan_mode_pram0;	/* Scan mode specific parameter #0 */
				/* (Has different meaning for */
				/* different scan modes). */
    fl32 scan_mode_pram1;	/* Scan mode specific parameter #1. */
    si16 num_parameter_des;	/* Total number of parameter */
				/* descriptor blocks for this radar. */
    si16 total_num_des;	/* Total number of additional */
				/* descriptor block for this radar. */
    si16 data_compress;	/* Data compression. 0 = none, 1 = HRD */
				/* scheme. */
    si16 data_reduction;	/* Data Reduction algorithm: 1 = none, */
				/* 2 = between 2 angles, 3 = Between */
				/* concentric circles, 4 = Above/below */
				/* certain altitudes.*/
    fl32 data_red_parm0;	/* 1 = smallest positive angle in */
				/* degrees, 2 = inner circle diameter, */
				/* km, 4 = minimum altitude, km. */
    fl32 data_red_parm1;	/* 1 = largest positve angle, degress, */
				/* 2 = outer cicle diameter, km, 4 = */
				/* maximum altitude, km. */
    fl32 radar_longitude;	/* Longitude of radar in degrees. */
    fl32 radar_latitude;	/* Latitude of radar in degrees. */
    fl32 radar_altitude;	/*  Altitude of radar above msl in km. */
    fl32 eff_unamb_vel;	/* Effective unambiguous velocity, m/s. */
    fl32 eff_unamb_range;	/* Effective unambiguous range, km. */
    si16 num_freq_trans;	/* Number of frequencies transmitted. */
    si16 num_ipps_trans;	/* Number of different inter-pulse */
				/* periods transmitted. */
    fl32 freq1;		/* Frequency 1. */
    fl32 freq2;		/* Frequency 2. */
    fl32 freq3;		/* Frequency 3. */
    fl32 freq4;		/* Frequency 4. */
    fl32 freq5;		/* Frequency 5. */
    fl32 interpulse_per1;	/* Interpulse period 1. */
    fl32 interpulse_per2;	/* Interpulse period 2. */
    fl32 interpulse_per3;	/* Interpulse period 3. */
    fl32 interpulse_per4;	/* Interpulse period 4. */
    fl32 interpulse_per5;	/* Interpulse period 5. */

				/* 1995 extension #1 */
    si32  extension_num;
    char  config_name[8];	/* used to identify this set of
				 * unique radar characteristics */
    si32  config_num;		/* facilitates a quick lookup of radar
				 * characteristics for each ray */
    /*
     * extend the radar descriptor to include unique lidar parameters
     */
    fl32 aperature_size;       /* Diameter of the lidar aperature in cm. */
    fl32 field_of_view;        /* Field of view of the receiver. mra; */
    fl32 aperature_eff;        /* Aperature efficiency in %. */
    fl32 freq[11];		/* make space for a total of 16 freqs */
    fl32 interpulse_per[11];	/* and ipps */
    /*
     * other extensions to the radar descriptor
     */
    fl32 pulsewidth;           /* Typical pulse width in microseconds. */
				/* pulse width is inverse of the
				 * band width */
    fl32 primary_cop_baseln;	/* coplane baselines */
    fl32 secondary_cop_baseln;
    fl32 pc_xmtr_bandwidth;	/* pulse compression transmitter bandwidth */
    si32  pc_waveform_type;	/* pulse compression waveform type */
    char  site_name[20];

}; /* End of Structure */


typedef struct radar_d radar_d;
typedef struct radar_d RADARDESC;

#ifdef OK_RPC

bool_t xdr_radar_d(XDR *, RADARDESC *);

#endif /* OK_RPC */


struct radar_d_v01 {
    char  radar_des[4];		/* Identifier for a radar descriptor */
				/* block (ascii characters "RADD"). */
    si32 radar_des_length;	/* Length of a radar descriptor block */
				/* in bytes. */
    char  radar_name[8];	/* Eight character radar name. */
    fl32 radar_const;		/* Radar constant in ?? */
    fl32 peak_power;		/* Typical peak power of the radar in kw. */
    fl32 noise_power;		/* Typical noise power of the radar in dBm. */
    fl32 receiver_gain;	/* Gain of the receiver in db. */
    fl32 antenna_gain;		/* Gain of the antenna in db. */
    fl32 system_gain;		/* Radar System Gain in db. */
    fl32 horz_beam_width;	/* Horizontal beam width in degrees. */
    fl32 vert_beam_width;	/* Vertical beam width in degrees. */
    si16  radar_type;		/* Radar Type (0)Ground, 1)Airborne */
				/* Fore, 2)Airborne Aft, 3)airborne */
				/* Tail, 4)Airborne Lower Fuselage, */
				/* 5)Shipborne. */
    si16  scan_mode;		/* Scan Mode (0)Calibration, 1)PPI */
				/* (constant Elevation) 2)Coplane, */
				/* 3)RHI (Constant Azimuth), 4) */
				/* Vertical Pointing, 5)Target */
				/* (Stationary), 6)Manual, 7)Idle (Out */
				/* of Control). */
    fl32 req_rotat_vel;	/* Requested rotational velocity of */
				/* the antenna in degrees/sec. */
    fl32 scan_mode_pram0;	/* Scan mode specific parameter #0 */
				/* (Has different meaning for */
				/* different scan modes). */
    fl32 scan_mode_pram1;	/* Scan mode specific parameter #1. */
    si16  num_parameter_des;	/* Total number of parameter */
				/* descriptor blocks for this radar. */
    si16  total_num_des;	/* Total number of additional */
				/* descriptor block for this radar. */
    si16 data_compress;	/* Data compression. 0 = none, 1 = HRD */
				/* scheme. */
    si16 data_reduction;	/* Data Reduction algorithm: 1 = none, */
				/* 2 = between 2 angles, 3 = Between */
				/* concentric circles, 4 = Above/below */
				/* certain altitudes.*/
    fl32 data_red_parm0;	/* 1 = smallest positive angle in */
				/* degrees, 2 = inner circle diameter, */
				/* km, 4 = minimum altitude, km. */
    fl32 data_red_parm1;	/* 1 = largest positve angle, degress, */
				/* 2 = outer cicle diameter, km, 4 = */
				/* maximum altitude, km. */
    fl32 radar_longitude;	/* Longitude of radar in degrees. */
    fl32 radar_latitude;	/* Latitude of radar in degrees. */
    fl32 radar_altitude;	/*  Altitude of radar above msl in km. */
    fl32 eff_unamb_vel;	/* Effective unambiguous velocity, m/s. */
    fl32 eff_unamb_range;	/* Effective unambiguous range, km. */
    si16 num_freq_trans;	/* Number of frequencies transmitted. */
    si16 num_ipps_trans;	/* Number of different inter-pulse */
				/* periods transmitted. */
    fl32 freq1;		/* Frequency 1. */
    fl32 freq2;		/* Frequency 2. */
    fl32 freq3;		/* Frequency 3. */
    fl32 freq4;		/* Frequency 4. */
    fl32 freq5;		/* Frequency 5. */
    fl32 interpulse_per1;	/* Interpulse period 1. */
    fl32 interpulse_per2;	/* Interpulse period 2. */
    fl32 interpulse_per3;	/* Interpulse period 3. */
    fl32 interpulse_per4;	/* Interpulse period 4. */
    fl32 interpulse_per5;	/* Interpulse period 5. */
}; /* End of Structure */


/* the following descriptor was used for some of the early
 * pre TOGA-COARE test tapes
 */

struct radar_d_v00 {
    char  radar_des[4];		/* Identifier for a radar descriptor */
				/* block (ascii characters "RADD"). */
    si32 radar_des_length;	/* Length of a radar descriptor block */
				/* in bytes. */
    char  radar_name[8];	/* Eight character radar name. */
    fl32 radar_const;		/* Radar constant in ?? */
    fl32 peak_power;		/* Typical peak power of the radar in kw. */
    fl32 noise_power;		/* Typical noise power of the radar in dBm. */
    fl32 receiver_gain;	/* Gain of the receiver in db. */
    fl32 antenna_gain;		/* Gain of the antenna in db. */
    fl32 system_gain;		/* Radar System Gain in db. */
    fl32 horz_beam_width;	/* Horizontal beam width in degrees. */
    fl32 vert_beam_width;	/* Vertical beam width in degrees. */
    si16  radar_type;		/* Radar Type (0)Ground, 1)Airborne */
				/* Fore, 2)Airborne Aft, 3)airborne */
				/* Tail, 4)Airborne Lower Fuselage, */
				/* 5)Shipborne. */
    si16  scan_mode;		/* Scan Mode (0)Calibration, 1)PPI */
				/* (constant Elevation) 2)Coplane, */
				/* 3)RHI (Constant Azimuth), 4) */
				/* Vertical Pointing, 5)Target */
				/* (Stationary), 6)Manual, 7)Idle (Out */
				/* of Control). */
    fl32 req_rotat_vel;	/* Requested rotational velocity of */
				/* the antenna in degrees/sec. */
    fl32 scan_mode_pram0;	/* Scan mode specific parameter #0 */
				/* (Has different meaning for */
				/* different scan modes). */
    fl32 scan_mode_pram1;	/* Scan mode specific parameter #1. */
    fl32 fixed_angle;
    si16  num_parameter_des;	/* Total number of parameter */
				/* descriptor blocks for this radar. */
    si16  total_num_des;	/* Total number of additional */
				/* descriptor block for this radar. */
    si16 data_compress;	/* Data compression. 0 = none, 1 = HRD */
				/* scheme. */
    si16 data_reduction;	/* Data Reduction algorithm: 1 = none, */
				/* 2 = between 2 angles, 3 = Between */
				/* concentric circles, 4 = Above/below */
				/* certain altitudes.*/
    fl32 data_red_parm0;	/* 1 = smallest positive angle in */
				/* degrees, 2 = inner circle diameter, */
				/* km, 4 = minimum altitude, km. */
    fl32 data_red_parm1;	/* 1 = largest positve angle, degress, */
				/* 2 = outer cicle diameter, km, 4 = */
				/* maximum altitude, km. */
    fl32 radar_longitude;	/* Longitude of radar in degrees. */
    fl32 radar_latitude;	/* Latitude of radar in degrees. */
    fl32 radar_altitude;	/*  Altitude of radar above msl in m. */
    fl32 eff_unamb_vel;	/* Effective unambiguous velocity, m/s. */
    fl32 eff_unamb_range;	/* Effective unambiguous range, km. */
    si16 num_freq_trans;	/* Number of frequencies transmitted. */
    si16 num_ipps_trans;	/* Number of different inter-pulse */
				/* periods transmitted. */
    fl32 freq1;		/* Frequency 1. */
    fl32 freq2;		/* Frequency 2. */
    fl32 freq3;		/* Frequency 3. */
    fl32 freq4;		/* Frequency 4. */
    fl32 freq5;		/* Frequency 5. */
    fl32 interpulse_per1;	/* Interpulse period 1. */
    fl32 interpulse_per2;	/* Interpulse period 2. */
    fl32 interpulse_per3;	/* Interpulse period 3. */
    fl32 interpulse_per4;	/* Interpulse period 4. */
    fl32 interpulse_per5;	/* Interpulse period 5. */
}; /* End of Structure */

#endif /* INCRadarDesch */
