/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996  John H. Merritt of Applied Research Corporation,
                        Landover, Maryland, a NASA/GSFC on-site contractor.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef _lassen_h_
#define _lassen_h_

typedef struct {
  short degree;  /* Geographic position: lat or lon */
  short minute;  /* + above equator, - below,       */
  short second;     /* - Western hemi, + Eastern hemi. */
  short spare;
} Lassen_radar_position;

typedef struct {
  char           radar_name[8]; /* Radar name. */
  char           site_name[8];  /* Site name.  */
  unsigned short antenna_height;/* In meters above sea level. */
  unsigned short spare;
  Lassen_radar_position latitude; /* Latitude or radar  */
  Lassen_radar_position longitude; /* Longitude of radar.*/
} Lassen_radar_info;

/* This is for Lassen files version 1.3 (coded as 13) and
 * files version 1.4 (coded 14)
 */

/* The maximum number of offsets is 10.  The LASSEN file will
 * have written data (space) for all 10 fields even when there is no field.
 */
#define NUMOFFSETS 10
#define OFF_UZ 0
#define OFF_CZ 1
#define OFF_VEL 2
#define OFF_WID 3
#define OFF_ZDR 4
#define OFF_PHI 5
#define OFF_RHO 6
#define OFF_LDR 7
#define OFF_KDP 8
#define OFF_TIME 9

/* This is absolute and cannot be changed.  The LASSEN file is 
 * written with this number specified.  It is used during the 
 * XDR decode process.
 */
#define LASSEN_MAX_SWEEPS 30

/* Sweep types (modes) are:
 *   POINT = 0
 *   PPI   = 1
 *   RHI   = 2
 *   SECTOR= 3
 *
 *   STOP  = 7  - Flag the end of the scan mode.
 */

/*
 * The Lassen_ray:
 * 
 * The Lassen_sweep contains 360 pointers to Lassen_rays.
 * Each Lassen_ray is dynamically allocated and the pointer inserted
 * into the Lassen_sweep.  The 'offsets' array contains indexes to
 * the field type.  When an index value is 0, then there is no data
 * available for that field type.  The order hard-coded and matches
 * the order given by OFF_* variables in the #define above.  Checking
 * for a non-zero index value seems to be the most reliable method
 * to determine the existance of a field type.  Using 'flags' is
 * problematic when there is no field of that type (ie. you get a 
 * false positive).
 *
 * Use the 'offset' array to jump to the field type.
 *
 */
typedef struct {
  unsigned short vangle;    /* Azimuth angle.           */
  unsigned short fanglet;   /* Target fixed angle.  */
  unsigned short fanglea;   /* Actual fixed angle.  */
  unsigned short a_start;   /* Azimuth angle start.  */
  unsigned short a_stop;    /* Azimuth angle stop.  */
  unsigned short status;    /* Hardware status word. */
  unsigned char  max_height;/* Maximum height, km.  */
  unsigned char  volume;    /* Volume serial number. */
  unsigned char  sweep;     /* Sweep number 1..LASSEN_MAX_SWEEPS       */
  unsigned char  sweep_type;/* Sweep type.              */
  unsigned short gatewid;   /* Gate width, meters.  */
  unsigned short rangeg1;   /* Range to gate 1, meters. */
  unsigned short numgates;  /* Number of gates.         */
  unsigned short prf;       /* Primary prf, hz.      */
  unsigned short prflow;    /* Secondary prf, hz.  */
  unsigned short n_pulses;  /* Sample size in pulses. */
  unsigned char  p_width;   /* Pulse width, .05 us units.*/
  unsigned char  cfilter;   /* Clutter filter code.   */
  unsigned char  spare[2];
    
  /*
   * Like I said before, don't trust these values.  Trust
   * the 'offset' instead -- offset in the ray.
   */
  
  /* flags is 'short' -- I hope it never becomes 'long' */
  struct {
	unsigned int packed     : 1; /* Is the data packed? */
	unsigned int good_data  : 1; /* Is the data good? */
	unsigned int uz         : 1; /* UZ  present. */
	unsigned int cz         : 1; /* CZ  present. */
	unsigned int vel        : 1; /* VEL present. */
	unsigned int wid        : 1; /* WID present. */
	unsigned int zdr        : 1; /* ZDR present. */
	unsigned int phi        : 1; /* PHI present. */
	unsigned int rho        : 1; /* RHO present. */
	unsigned int ldr        : 1; /* LDR present. */
	unsigned int kdp        : 1; /* KDP present. */
	unsigned int time       : 1; /* TIME series present. */
	unsigned int spares     : 4;
  } flags;
  
  
  /*
   * Here is where you get to the data for a field type.
   * If offset[OFF_CZ] is non-zero, for instance, then
   * CZ exists and is located that many bytes into
   * the data array.  The data array is in the ray pointer
   * which is in the Lassen_sweep.
   */
  unsigned short offset[NUMOFFSETS];

  unsigned char year;   /* year-1900   */
  unsigned char month;  /* month  1-12 */
  unsigned char day;    /* day    1-31 */
  unsigned char hour;   /* hour   0-23 */
  unsigned char minute; /* minute 0-59 */
  unsigned char second; /* second 0-59 */
  unsigned char spare2[2];
} Lassen_ray;

/*
 * The Lassen_sweep:
 *
 * This structure contains information for an entire sweep for a possible
 * 360 rays.  These will be pointers to Lassen_ray.
 * The Lassen_volume may have up to 30 pointers to Lassen_sweep.
 * The '30' is mandatory and not adjustable because of how the LASSEN
 * file is written.  The LASSEN file writes 30 angles, gates, and
 * 300 offset values (30*NUMOFFSETS).  LASSEN_MAX_SWEEPS = 30.
 * The number of rays in a Lassen_sweep is 'numrays'.
 *
 * The 'offset' is copied from the one in Lassen_ray.
 */
typedef struct {
  unsigned short volume;     /* Volume serial number. */
  unsigned short sweep;      /* Sweep index. */
  unsigned short sweep_type; /* Sweep type code.  */
  unsigned short max_height; /* Maximum height, km. */

  unsigned short fangle;  /* Fixed, azimuth angle.  */
  unsigned short min_var; /* 'leftmost' variable angle. */
  unsigned short max_var; /* 'rightmost' variable angle. */
  unsigned short a_start; /* Azimuth angle start.  */
  unsigned short a_stop;  /* Azimuth angle stop.  */

  unsigned short gatewid;  /* Gate width, meters.  */
  unsigned short rangeg1;  /* Range to gate 1, meters. */
  unsigned short numgates; /* Number of gates.  */
  unsigned short numrays;  /* Number of rays this sweep. */
  
  unsigned short prf;     /* Primary prf, hz.  */
  unsigned short prflow;  /* Secondary prf, hz.  */
  unsigned short n_pulses;/* Sample size in pulses. */
  unsigned short p_width; /* Pulse width, .05 us units. */
  unsigned short cfilter; /* Clutter filter code.  */
  
  unsigned short offset[NUMOFFSETS];
  
  unsigned char year;    /* Year - 1900. */
  unsigned char month;   /* Month 1-12 */
  unsigned char day;     /* Day   1-31 */
  unsigned char shour;   /* Start hour   0-23 */
  unsigned char sminute; /* Start minute 0-59 */
  unsigned char ssecond; /* Start second 0-59 */
  unsigned char ehour;   /* End hour   0-23  */
  unsigned char eminute; /* End minute 0-59  */
  unsigned char esecond; /* End second 0-59  */
  unsigned char spare1[3];
  
  unsigned short status;  /* Status word. */
  unsigned short spare2;

  Lassen_ray *ray[360]; /* The Lassen_ray pointers. */
} Lassen_sweep;

/*
 * Lassen_volume:
 *
 * A volume can contain a maximum of LASSEN_MAX_SWEEPS sweeps.
 * The value for LASSEN_MAX_SWEEPS *MUST* be 30.  This cannot change,
 * otherwise, you'll not be able to read the file.
 * The number of sweeps is in the member 'numsweeps'.
 *
 */

typedef struct {
  unsigned short version; /* Raw version number.  */
  short          filled;  /* <0=empty 0=filling >0=full. */
  
  unsigned int   volume;     /* Volume serial number.  */
  unsigned short sweep;      /* Sweep index 1 -> max. */
  unsigned short sweep_type; /* Sweep type code.   */
  unsigned short max_height; /* Maximum height, km.  */
  unsigned short status;     /* Status word. */
  
  unsigned short min_fangle; /* Minimum fixed angle.  */
  unsigned short max_fangle; /* Maximum fixed angle.  */
  unsigned short min_var;    /* Minimum variable angle. */
  unsigned short max_var;    /* Maximum variable angle. */
  unsigned short a_start;    /* Variable angle start. */
  unsigned short a_stop;     /* Variable angle stop.  */
  unsigned short numsweeps;  /* Number of sweeps in volume. */
  unsigned short fangles[LASSEN_MAX_SWEEPS]; /* Fixed angles for each sweep. */
  
  unsigned short gatewid; /* Gate width, meters.  */
  unsigned short rangeg1; /* Range to gate 1, meters. */
  unsigned short numgates[LASSEN_MAX_SWEEPS]; /* Gates for each sweep. */
  unsigned short maxgates; /* Max # of gates in volume. */
  
  unsigned short prf;      /* Primary prf, hz. */
  unsigned short prflow;   /* Secondary prf, hz. */
  unsigned short n_pulses; /* Sample size in pulses. */
  unsigned short p_width;  /* Pulse width, .05 us units. */
  unsigned short cfilter;  /* Clutter filter code. */
  unsigned short local;    /* Used as volume lock: nonzero. */
  
  unsigned int freq;  /* Mhz * 10   */
  
  unsigned short offset[LASSEN_MAX_SWEEPS][NUMOFFSETS];
  
  unsigned char year;    /* Year - 1900      */
  unsigned char month;   /* Month 1-12        */
  unsigned char day;     /* Day   1-31        */
  unsigned char shour;   /* Start hour   0-23 */
  unsigned char sminute; /* Start minute 0-59 */
  unsigned char ssecond; /* Start second 0-59 */
  unsigned char ehour;   /* End hour   0-23   */
  unsigned char eminute; /* End minute 0-59   */
  unsigned char esecond; /* End second 0-59   */
  unsigned char spare[3];
  
  struct {   /* Software status flags. Length of volflags is 'short' */
 unsigned int compress : 1;
 unsigned int spares : 15;
  } volflags;
  
  Lassen_radar_info  radinfo; /* Radar information. */
  
  Lassen_sweep *index[LASSEN_MAX_SWEEPS]; /* The Lassen_sweep pointers. */
} Lassen_volume;

typedef struct {
  unsigned char year;   /* year - 1900 */
  unsigned char month;  /* 1-12  */
  unsigned char day;    /* 1-31  */
  unsigned char hour;   /* 0-23  */
  unsigned char minute; /* 0-59  */
  unsigned char second; /* 0-59  */
  unsigned char dummy[2];
} Lassen_time;

typedef struct {
  char magic[8];     /* Magic number.  This must be 'SUNRISE'. */
  Lassen_time mdate; /* Last modification. */
  Lassen_time cdate; /* Creation date.     */
  int  type;         /* See #defines above.*/
  char mwho[16];     /* Last person to modify.   */
  char cwho[16];     /* Person who created file. */
  int  protection;   /* Is file protected? */
  int  checksum;     /* Data bcc.   */
  char description[40]; /* File description. */
  int  id;
  int  spare[12];
} Lassen_head;

#define ANGLE_CONVERT(X)  ((unsigned short) ((((unsigned int)X+22)*360) >> 14))
#endif
