/* 	$Id: viraq.h,v 1.1 2008/01/24 22:22:32 rehak Exp $	 */

# ifndef INC_VIRAQ_HH
# define INC_VIRAQ_HH

# ifndef DD_BYTE_ARRAYS
# define DD_BYTE_ARRAYS

/* structure for four byte words */
typedef struct
   {
   unsigned char  zero;
   unsigned char  one;
   unsigned char  two;
   unsigned char  three;
   }
fourB;


typedef struct
   {
   char  one;
   char  two;
   char  three;
   char  four;
   }
fourb;

typedef struct
   {
   char  one;
   char  two;
   char  three;
   char  four;
   char  five;
   char  six;
   char  seven;
   char  eight;
   }
eightB;
/* stucture for two byte words */
typedef struct
   {
   char one;
   char two;
   }
twob;

# endif /* DD_BYTE_ARRAYS */

/* definition of several different data formats */
#define DATA_SIMPLEPP    0 /* simple pulse pair ABP */
#define DATA_POLYPP      1 /* poly pulse pair ABPAB */
#define DATA_POL1        3 /* dual polarization pulse pair ABP,ABP */
#define DATA_POL2        4 /* more complex dual polarization ??????? */
#define DATA_POL3        5 /* almost full dual polarization with log integers */
#define DATA_SIMPLEPP16  6 /* simple pulse pair ABP (16-bit ints not floats) */
#define DATA_POL12       8 /* simple pulse pair ABP (16-bit ints not floats) */
#define DATA_POL_PLUS    9 /* full pol plus */
#define DATA_MAX_POL    10 /* same as full plus plus more gates */
#define DATA_HVSIMUL    11 /* simultaneous transmission of H and V */
#define DATA_SHRTPUL    12 /* same as MAX_POL with gate averaging */
#define DATA_DUALPP     15 /* DOW dual prt pulse pair ABP,ABP */
#define DATA_POL_PLUS_CMP 29	/* full pol plus */
#define DATA_MAX_POL_CMP  30	/* same as full plus plus more gates */
#define DATA_HVSIMUL_CMP  31	/* simultaneous transmission of H and V */
#define DATA_SHRTPUL_CMP  32	/* same as MAX_POL with gate averaging */

typedef twob    LeShort;
typedef fourB   LeLong;
typedef fourB   LeFloat;
typedef eightB  LeLongLong;


/* header for each dwell describing parameters which might change dwell by dwell */
/* this structure will appear on tape before each abp set */
typedef struct  {
		char            desc[4];
		unsigned short  recordlen;
		short           gates,hits;
		float           rcvr_pulsewidth,prt,delay; /* delay to first gate */
		char            clutterfilter,timeseries;
		short           tsgate;
		unsigned int    time;      /* seconds since 1970 */
		short           subsec;    /* fractional seconds (.1 mS) */
		float           az,el;
		float           radar_longitude; 
		float           radar_latitude;
		float           radar_altitude;
		float           ew_velocity;
		float           ns_velocity;
		float           vert_velocity;
		char            dataformat;     /* 0 = abp, 1 = abpab (poly), 2 = abpab (dual prt) */
		float           prt2;
		float           fxd_angle;
		unsigned char   scan_type;
		unsigned char   scan_num;
		unsigned char   vol_num;
		unsigned int    ray_count;
		char            transition;
		float           hxmit_power;    /* on the fly hor power */
		float           vxmit_power;    /* on the fly ver power */
		float           yaw;            /* platform heading in degrees */
		float           pitch;          /* platform pitch in degrees */
		float           roll;           /* platform roll in degrees */
                                float           gate0mag;       /* gate zero magnitude in rel dB */
                                float           dacv;           /* dac voltage value or afc freq */
                                long long    pulsenum;       /* pulsenum of first pulse in dwell starting 1970 */
                                char            spare[72]; 
		} HEADERV;

/* this structure gets recorded once per volume (when a parameter changes) */
typedef struct  {
		char    desc[4];
		unsigned short   recordlen;
		short   rev;
		short   year;           /* this is also in the dwell as sec from 1970 */
		char    radar_name[8];
		char    polarization;   /* H or V */
		float   test_pulse_pwr; /* power of test pulse (refered to antenna flange) */
		float   test_pulse_frq; /* test pulse frequency */
		float   frequency;      /* transmit frequency */
		float   peak_power;     /* typical xmit power (at antenna flange) read from config.rdr file */
		float   noise_figure;
		float   noise_power;    /* for subtracting from data */
		float   receiver_gain;  /* hor chan gain from antenna flange to VIRAQ input */
		float   data_sys_sat;   /* VIRAQ input power required for full scale */
		float   antenna_gain;
		float   horz_beam_width;
		float   vert_beam_width;
		float   xmit_pulsewidth; /* transmitted pulse width */
		float   rconst;         /* radar constant */
		float   phaseoffset;    /* offset for phi dp */
		float   vreceiver_gain; /* ver chan gain from antenna flange to VIRAQ */
		float   vtest_pulse_pwr; /* ver test pulse power refered to antenna flange */
		float   vantenna_gain;  
		float   vnoise_power;   /* for subtracting from data */
		float   zdr_fudge_factor; /* what else? */
                                float   mismatch_loss;
		float   misc[3];        /* 3 more misc floats */
		char    text[960];
		} RADARV;

/* this is what the top of either the radar or dwell struct looks like */
/* it is used for recording on disk and tape */


typedef struct  {
		char            desc[4];
		LeShort           recordlen;
		LeShort           gates,hits;
		LeFloat           rcvr_pulsewidth,prt,delay; /* delay to first gate */
		char            clutterfilter,timeseries;
		LeShort           tsgate;
		LeLong          time;      /* seconds since 1970 */
		LeShort           subsec;    /* fractional seconds (.1 mS) */
		LeFloat           az,el;
		LeFloat           radar_longitude; 
		LeFloat           radar_lattitude;
		LeFloat           radar_altitude;
		LeFloat           ew_velocity;
		LeFloat           ns_velocity;
		LeFloat           vert_velocity;
		char              dataformat; /* 0 = abp, 1 = abpab (poly),
					       * 2 = abpab (dual prt) */
		LeFloat           prt2;
		LeFloat           fxd_angle;
		unsigned char     scan_type; 
		unsigned char     scan_num; /* bumped by one for each new scan */
		unsigned char     vol_num; /* bumped by one for each new vol */
		LeLong            ray_count;
		char              transition;
		LeFloat           hxmit_power;    /* on the fly hor power */
		LeFloat           vxmit_power;    /* on the fly ver power */
		LeFloat           yaw;            /* platform heading in degrees */
		LeFloat           pitch;          /* platform pitch in degrees */
		LeFloat           roll;           /* platform roll in degrees */
                                LeFloat           gate0mag;       /* gate zero magnitude in rel dB */
                                LeFloat           dacv;           /* dac voltage value or afc freq */
                                LeLongLong   pulsenum;   /* pulsenum of first pulse in dwell starting 1970 */
                                char            spare[72];
		} LeHEADERV;


/* this structure gets recorded once per volume (when a parameter changes) */
typedef struct  {
		char    desc[4];
		LeShort   recordlen;
		LeShort   rev;
		LeShort   year;           /* this is also in the dwell as sec from 1970 */
		char    radar_name[8];
		char    polarization;   /* H or V */
		LeFloat   test_pulse_pwr; /* power of test pulse (refered to antenna flange) */
		LeFloat   test_pulse_frq; /* test pulse frequency */
		LeFloat   frequency;      /* transmit frequency */
		LeFloat   peak_power;     /* typical xmit power (at antenna flange) */
		LeFloat   noise_figure;
		LeFloat   noise_power;    /* for subtracting from data */
		LeFloat   receiver_gain;  /* gain from antenna flange to PIRAQ input */
		LeFloat   data_sys_sat;   /* PIRAQ input power required for full scale */
		LeFloat   antenna_gain;
		LeFloat   horz_beam_width;
		LeFloat   vert_beam_width;
		LeFloat   xmit_pulsewidth; /* transmitted pulse width */
		LeFloat   rconst;         /* radar constant */
		LeFloat   phaseoffset;    /* offset for phi dp */
		LeFloat   vreceiver_gain; /* ver chan gain from antenna flange to VIRAQ */
		LeFloat   vtest_pulse_pwr; /* ver test pulse power refered to antenna flange */
		LeFloat   vantenna_gain;  
		LeFloat   vnoise_power;   /* for subtracting from data */
		LeFloat   zdr_fudge_factor; /* what else? */
                                LeFloat   mismatch_loss;
		LeFloat   misc[3];        /* 3 more misc floats */
		char    text[960];
		} LeRADARV;

typedef struct 
{
    LeHEADERV h;
    LeRADARV r;
} LeCombinedHskp;

typedef struct 
{
    HEADERV h;
    RADARV r;
} CombinedHskp;

#define VIRAQ_MAX_GATES 2000

/* c------------------------------------------------------------------------ */

typedef   int   int4;
typedef float float4;


/* c------------------------------------------------------------------------ */


# endif /* INC_VIRAQ_HH */
