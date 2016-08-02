/* For SIGMET version 1 and version 2 files.
 *
 * John H. Merritt
 * Applied Research Corp.
 * NASA GSFC Code 910.1
 *
 *
 * The structures exactly match the SIGMET documentation.
 */

#ifndef NSIG2_H
#define NSIG2_H

/*Structure identifier, byte 0 of structure_header, III-18 */
#define NSIG_INGEST_SUM 3
#define NSIG_PROD       7

/* Data type, value for byte 22 in ingest data header, III-29 */
#define NSIG_DTB_EXH       0
#define NSIG_DTB_UCR       1
#define NSIG_DTB_CR        2
#define NSIG_DTB_VEL       3 
#define NSIG_DTB_WID       4
#define NSIG_DTB_ZDR       5
#define NSIG_DTB_UCR2      8
#define NSIG_DTB_CR2       9
#define NSIG_DTB_VEL2     10
#define NSIG_DTB_WID2     11
#define NSIG_DTB_ZDR2     12
#define NSIG_DTB_KDP      14
#define NSIG_DTB_KDP2     15
#define NSIG_DTB_PHIDP    16
#define NSIG_DTB_VELC     17 
#define NSIG_DTB_SQI      18
#define NSIG_DTB_RHOHV    19
#define NSIG_DTB_RHOHV2   20
#define NSIG_DTB_VELC2    22
#define NSIG_DTB_SQI2     23
#define NSIG_DTB_PHIDP2   24
#define NSIG_DTB_HCLASS   55
#define NSIG_DTB_HCLASS2  56

/* Product type code ,value for byte 12 in product configuration 
 * struct, III-35
 */
#define NSIG_PROD_PPI        1             /*  PPI */
#define NSIG_PROD_RHI        2             /*  RHI */
#define NSIG_PROD_CAPPI      3             /*  CAPPI */
#define NSIG_PROD_CROSS      4             /*  Cross section */
#define NSIG_PROD_TOPS       5             /*  Echo tops */
#define NSIG_PROD_RAIN1      7             /*  Precipitation 1 hour */
#define NSIG_PROD_TRACK      6             /*  Storm track */
#define NSIG_PROD_RAINN      8             /*  Precipitation n hour */
#define NSIG_PROD_VVP        9             /*  Velocity Volume processing */
#define NSIG_PROD_VIL        10            /*  Vertically Integrated Liquid */
#define NSIG_PROD_SHEAR      11            /*  Wind shear */
#define NSIG_PROD_WARN       12            /*  Warning (overlay) */
#define NSIG_PROD_RTPPI      13            /*  Real time PPI */
#define NSIG_PROD_RTRHI      14            /*  Real time RHI */
#define NSIG_PROD_RAW        15            /*  Raw data set (no display)*/
#define NSIG_PROD_MAX        16            /*  Maximum with side panels */
#define NSIG_PROD_USER       17            /*  Earth projection user product */
#define NSIG_PROD_USERV      18            /*  Section projection user product */
#define NSIG_PROD_OTHER      19            /*  Other user product (no display) */
#define NSIG_PROD_STATUS     20            /*  Status product (no display) */
#define NSIG_PROD_SLINE      21            /*  Shear Line Product */
#define NSIG_PROD_WIND       22            /*  Horizontal wind field */

#define NSIG_SCAN_PPI 1
#define NSIG_SCAN_RHI 2
#define NSIG_SCAN_MAN 3
#define NSIG_SCAN_CON 4
#define NSIG_SCAN_FIL 5

#define NSIG_BLOCK   6144
#define NSIG_MAX_BIN 1536

/* Two byte binary angle is unsigned short */
/* Using these typedefs forces non-word alignment.  This is because
 * we don't want any space between members of a structure.
 * -- A coding trick --
 */
typedef unsigned char bang[2];
typedef unsigned char twob[2];
typedef unsigned char fourb[4];

/* Ray header 3.4.2, page III-29 */
/* No change for NSIG_VER2 */
typedef struct {
  bang beg_azm;  /* Azimuth   at beginning of ray (binary angle). */
  bang beg_elev; /* Elevation at beginning of ray (binary angle). */
  bang end_azm;  /* Azimuth   at end of ray (binary angle). */
  bang end_elev; /* Elevation at end of ray (binary angle). */
  twob num_bins; /* Actual number of bins in the ray. */
  twob sec;      /* Time in seconds from start of sweep (unsigned). */
} NSIG_Ray_header;
/*============================================================*/
/*============================================================*/

/* Extended Header version 0, section 3.4.3, page III-29 */
/* No change for NSIG_VER2 */
typedef struct {
  fourb   msec;
  twob  cal_sig;
  twob  spare[7];
} NSIG_Ext_header_ver0;
/*============================================================*/
/*============================================================*/

/* Extended Header version 1, section 3.4.3, page III-29 */
typedef struct {
  fourb msec;       /* Time in milliseconds from the sweep starting time. */
  twob  cal_sig;    /* Calibration Signal level. */
  bang  azm;        /* Azimuth (binary angle) */
  bang  elev;       /* Elevation (binary angle) */
  bang  train_ord;  /* Train order (binary angle) */
  bang  elev_ord;   /* Elevation order (binary angle) */
  bang  pitch;      /* Pitch   (binary angle) */
  bang  roll;       /* Roll    (binary angle) */
  bang  heading;    /* Heading (binary angle) */
  bang  azm_rate;   /* Azimuth Rate (binary angle/sec) */
  bang  elev_rate;  /* Elevation Rate (binary angle/sec) */
  bang  pitch_rate; /* Pitch Rate (binary angle/sec) */
  bang  roll_rate;  /* Roll Rate (binary angle/sec) */
#ifdef NSIG_VER2

#else
  bang  heading_rate; /* Heading Rate (binary angle/sec) */
#endif
  fourb   lat; /* Latitude (binary angle) */
  fourb   lon; /* Longitude (binary angle) */
  twob  alt;   /* Altitude (meters) */
  twob  vel_e; /* Velocity East (cm/sec) */
  twob  vel_n; /* Velocity North (cm/sec) */
  twob  vel_u; /* Velocity Up (cm/sec) */
#ifdef NSIG_VER2

#else
  fourb   time_update; /* Time since last update (milliseconds) */
#endif
  twob  nav_sys_flag;  /* Navigation system OK flag */
  twob  rad_vel_cor;   /* Radial velocity correction (velocity units) */
} NSIG_Ext_header_ver1;
/*============================================================*/
/*============================================================*/




/*-----------------------------------------------------------------*/
/* Note:
 *   All structure names are prefixed with NSIG_ and have the
 *   first letter of the remainder capitalized.
 */
/*-----------------------------------------------------------------*/

/* Structure header 3.2.35, page III-18 */
typedef struct {
  twob  id;
#ifdef NSIG_VER2
  twob  version;
  fourb num_bytes;
#else
  fourb num_bytes;
  twob  version;
#endif
  twob  spare;
  twob  flags;
} NSIG_Structure_header;
/*============================================================*/
/*============================================================*/

/* Time sturcture 3.2.36, page III-18 */
typedef struct {
#ifdef NSIG_VER2
  fourb sec;  
  twob  msec;  /* Fractions of seconds in milliseconds. */
  twob  year;
  twob  month;
  twob  day;
#else
  twob  year;
  twob  month;
  twob  day;
  fourb sec;
#endif
} NSIG_Ymds_time;
/*============================================================*/
/*============================================================*/

/* ingest data header 3.4.1, page III-28 */
typedef struct {
   NSIG_Structure_header struct_head;
   NSIG_Ymds_time        time;
#ifdef NSIG_VER2
#else
   twob  data_type;
#endif
   twob  sweep_num;
   twob  num_rays_swp;
   twob  ind_ray_one;
   twob  num_rays_exp;
   twob  num_rays_act;
   bang  fix_ang;
   twob  bits_bin;
#ifdef NSIG_VER2
  twob data_type;  /* Data code (See Task_DSP_Info.IDATA) */
  char  spare[36];
#else
  char  spare[38];
#endif
} NSIG_Ingest_data_header;
/*============================================================*/
/*============================================================*/

/* No change for NSIG_VER2 */
typedef struct {
  twob    rec_num;
  twob    sweep_num;
  twob    ray_loc;
  twob    ray_num;
  twob    flags;
  twob    spare;
} NSIG_Raw_prod_bhdr;
/*============================================================*/
/*============================================================*/
#ifdef NSIG_VER2
/* Define the color scale conversion */
typedef struct {
#define COLOR_SCALE_OVERRIDE (0x0200)
#define COLOR_SCALE_VARIABLE (0x0100)
#define COLOR_LABEL_MASK     (0x00ff)
  fourb iflags;
  fourb istart;
  fourb istep;
  twob  icolcnt;
  twob  ipalette_num;
  twob  ilevel_seams[16];
} NSIG_Color_scale_def;
#endif
/*============================================================*/
/*============================================================*/

/* Product configuration structure 3.5.1.1, page III-35 */
typedef struct {
   NSIG_Structure_header st_head;
   twob                  prod_code;
#ifdef NSIG_VER2
  twob  isch;                    /*  Scheduling */
#define PSC_HOLD_P    0           /*   Do not run at all. */
#define PSC_NEXT_P    1           /*   Run once on next available data */
#define PSC_ALL_P     2           /*   Run as data becomes available */
#define PSC_AGAIN_P   3           /*   Run again on data last used */

  fourb isch_skip;                /*  # seconds between runs */

#else

#endif
   NSIG_Ymds_time        prod_time;
   NSIG_Ymds_time        file_time;
   NSIG_Ymds_time        schd_time;
   twob   schd_code;    /* Not used in Ver 2. */
   fourb    sec_skip;   /* Not used in Ver 2. */
   char   user_name[12];
#ifdef NSIG_VER2

#else
   char   file_name[12];
#endif
   char   task_name[12];
#ifdef NSIG_VER2

#else
   char   spare_name[12];
#endif
   twob   flag;
#ifdef NSIG_VER2
  fourb ixscale, iyscale, izscale; /*  Scale in cm/pixel  */
#else

#endif
   fourb    x_size;
   fourb    y_size;
   fourb    z_size;
   fourb    x_loc;
   fourb    y_loc;
   fourb    z_loc;
   fourb    max_rng;
#ifdef NSIG_VER2
  fourb irange_last_v20;       /* Range of last bin in cm (raw only) */
  char ipad128x2[2];
  twob idata_out;             /* Data type of data generated by product gen */

  /* This section for version 2.1+ products: */
  char ipad132x12[12];
  twob idata_in;                 /*  Data type used by the generator */
  char ipad146x2[2];
  twob iradial_smooth;           /*  Range in km*100 over which radial  */
                                  /*  smoothing should be done.  0:none. */
  twob iruns;                    /*  # of times this pcf has been run   */
  fourb izr_const;                /*  Z-R or Z-W constant and exponent   */
  fourb izr_exp;                  /*   in 1/1000 of integers             */
  twob ix_smooth;                /*  X-Y Smoothing parameters for 2D */
  twob iy_smooth;                /*   products.  km*100,  0:none */

  /* ---- Product Specific Parameters ---- */
  /*The following area conveys information needed for each specific product.*/

  char psi[80];  /* Do we need these??  -John 8/14/96 */

  char ipad244x28[28];
  NSIG_Color_scale_def colors;

#else

   twob   bits_item;
   twob   data_type;
   fourb    data_start;
   fourb    data_step;
   twob   num_col;
  /* The following depends on version 2.0, 2.1 etc check III-34,35 */
  /* Currently, though, this is not used. */
  char spare[178];
#endif
} NSIG_Product_config;
/*============================================================*/
/*============================================================*/


/* product end 3.5.1.2 ,page III-36 */
typedef struct {
#ifdef NSIG_VER2
  char sprod_sitename[16];  /* Name of product generator site, space padded */
  char sprod_version[8];    /* Product IRIS version, null terminated */
  char sing_version[8];     /* Ingest IRIS version, null terminated */
  NSIG_Ymds_time data_time; /* Oldest data in this file */
  char ipad44x46[42];
#else
  char  part_name[80];     /* Path name of file on disk. */
  NSIG_Ymds_time data_time; /* Date/time structure. */
#endif
  char  site_name[16];     /* Site name.  Eg. mit, tog, kwa (upper-case?). */
  twob  ahead_gms;         /* # minutes ahead of GMT. */
  fourb lat;               /* Latitude (binary angle format). */
  fourb lon;               /* Longitude (binary angle format). */
  twob  grnd_sea_ht;       /* Signed ground height (meters). */
  twob  rad_grnd_ht;       /* Radar height above ground (meters). */
#ifdef NSIG_VER2

#else
  twob  sig_proc;          /* Type of signal processor used. */
#endif
  fourb prf;               /* PRF (hz). */
  fourb pulse_wd;          /* sample width in microsec/100*/
#ifdef NSIG_VER2
  twob  sig_proc;          /* Type of signal processor used. */
#else

#endif
  twob  trg_rate;          /* Trigger rate scheme. */
  twob  num_samp;          /* number of samples (per ray). */
  char  clutter_file[12];  /* Clutter filter file name. */
  twob  num_filter;        /* Number of filter used for the first range bin. */
  fourb wavelen;           /* Wavelength in 1/100 of centimeters. */
  fourb trunc_ht;          /* Truncation height in cm. */
  fourb rng_f_bin;         /* Range of the first bin in cm. */
  fourb rng_l_bin;         /* Range of the last  bin in cm. */
  fourb num_bin;           /* Number of output bins. */
  twob  flag;              /* Flag word. */
#define PH_OVERLAY_P   (0x0001)     /*  Has an overlay \ For picture */
#define PH_RINGS_P     (0x0002)     /*  Has range rings/ products only */

  twob  file_up;           /* Number of updates to the file. */
  char  label[16][4];      /* Array of labels for color parameter legend. */
#ifdef NSIG_VER2
  twob ilog_filter_first;             /* Log filter used on first bin */
  char ipad238x10[10];
#else
  char  label_unit[12];    /* Text holding units of the labels. (Ver 2 only) */
#endif
  twob  prod_seq;          /* Product sequence number. */
  twob  color_num[16];     /* Color numbers for the up to 16 steps. */
  char  color_reject;      /* Color used for rejected data. */
#ifdef NSIG_VER2
 char ipad283x2[3];

  /* The number of results elements at the end of the file.
   * Used for warning, shearline, and track only */
  twob iresults_count;

#define PROD_END_PAD 20
  char ipad_end[PROD_END_PAD];
#else
  char  color_unscan;      /* Color used for unscanned area. */
  char  color_over;        /* Color used for overlays. */
  char  spare;
  fourb prod_max_rng;      /* Max range of the first product used as input. */
  char  spare2[18];
#endif
} NSIG_Product_end;
/*============================================================*/
/*============================================================*/

/* Ingest Summary Header 3.3.1 , page III-19 */
typedef struct {
  char   file_name[80];         /* Name of file on disk. */
  twob   num_file;              /* Number of associated data files extant. */
#ifdef NSIG_VER2
    twob isweeps_done ;	    /* # of sweeps that have been completed */
#endif
  fourb  sum_size;              /* Total size of all files in bytes. */
  NSIG_Ymds_time start_time;    
#ifdef NSIG_VER2
  char ipad100x12[12];
#else
  char   drive_name[16];        /* Name of tape drive written to. */
#endif
  twob   size_ray_headers;      /* Number of bytes in the ray headers. */
  twob   size_ext_ray_headers;  /* Number of bytes in extended ray headers. */
#ifdef NSIG_VER2
 twob ib_task;                     /* # bytes in task config table */
  char ipad_118x6[6];
  char siris_version[8];             /* Null terminated */
  char ipad_132x18[18];
#else
  twob   num_task_conf_tab;     /* Number of task configuration table. */
  twob   size_device_status_tab; /* Number of bytes in device status table. */
  twob   gparam_size;         /* Number of bytes in each gparam. */
  char   spare[28];
#endif
  char   site_name[16];       /* Name of site from setup program. */
  twob   time_zone;           /* Time zione of recorded time, +min of GMT */
  fourb  lat_rad;             /* Latitude of radar. */
  fourb  lon_rad;             /* Longitude of radar. */
  twob   grd_height;          /* Height of ground at site (meters) */
  twob   ant_height;          /* Height of radar above ground (meters) */
  twob   azm_res;             /* Resolution of delta azimuth in sweep. */
  twob   ray_ind;             /* Index of first rays from above set of rays.
							   *  Or the angle of the first ray.
							   */
  twob   num_rays;            /* Number of rays in a sweep. */
  fourb  ant_alt;             /* Altitude of radar above sea level in cm */
  fourb  vel[3];              /* [0]=east, [1]=north, [2]=up */
  fourb  ant_offset[3];       /* [0]=starboard, [1]=bow, [2]=up */
#ifdef NSIG_VER2
  char spare2[264];
#else
  char spare2[266];
#endif
} NSIG_Ingest_summary;
/*============================================================*/
/*============================================================*/

/* rvp5_gparam structure 3.3.4.1, page III-26 */
typedef struct {
  twob  revision;		/* Revision                 */
  twob  num_bins;		/* Number of range bins     */
  twob  cur_trig_p;		/* Current trigger period   */
  twob  cur_tag1;		/* Current TAG00 - TAG15    */
  twob  cur_tag2;		/* Current TAG16 - TAG31    */
  twob  l_chan_noise;   /* Log channel noise level  */
  twob  i_chan_noise;   /* I Channel noise level    */
  twob  q_chan_noise;   /* Q Channel noise level    */
  twob  lat_proc_status;/* Latched processor status */
  twob  imm_proc_status;/* Immdiate processor status */
  twob  diag_reg_a;     /* Diagnostic register A    */
  twob  diag_reg_b;     /* Diagnostic register B    */
  twob  num_pulses;     /* Number of pulses per ray */
  twob  trig_c_low;     /* Trigger count (low 16 bits)  */
  twob  trig_c_high;    /* Trigger count (high 8 bits)  */
  twob  num_acq_bins;   /* # of properly acquired bins  */
  twob  num_pro_bins;   /* # of properly processed bins */
  twob  rng_off;        /* 25-meter range offset        */
  twob  noise_rng;      /* Noise range in KM            */
  twob  noise_trg;      /* Noise trigger period         */
  twob  pulse_w_0;      /* Pulse width 0 min trig period */
  twob  pulse_w_1;      /* Pulse width 1 min trig period */
  twob  pulse_w_2;      /* Pulse width 2 min trig period */
  twob  pulse_w_3;      /* Pulse width 3 min trig period */
  twob  pulse_w_pat;    /* Pulse width bit patterns      */
  twob  cur_wave_pw;    /* Current waveform/pulsewidth   */
  twob  cur_trig_gen;   /* Current trigger gen period    */
  twob  des_trig_gen;   /* Desired trigger gen period    */
  twob  prt_start;      /* PRT at start of last ray      */
  twob  prt_end;        /* PRT at end   of last ray      */
  twob  proc_thr_flag;  /* Processing/threshold flags    */
  twob  log_con_slope;  /* LOG conversion slope          */
  twob  log_noise_thr;  /* LOG noise threshold           */
  twob  clu_cor_thr;    /* Clutter correction threshold  */
  twob  sqi_thr;        /* SQI threshold                 */
  twob  log_thr_w;      /* LOG threshold for width       */
  twob  cal_ref;        /* Calibration reflectivity      */
  twob  q_i_cur_samp;   /* Q and I current sample        */
  twob  l_cur_samp;     /* Log current sample            */
  twob  rng_avr_cho;    /* Range averaging choice        */
  twob  spare1[3];
  twob  i_sqr_low;
  twob  i_sqr_high;
  twob  q_sqr_low;
  twob  q_sqr_high;
  twob  noise_mean;
  twob  noise_std;
  twob  spare2[15];
} NSIG_Rpv5_gparam;
/*============================================================*/
/*============================================================*/

typedef struct {
  NSIG_Structure_header struct_head;
  NSIG_Rpv5_gparam rpv5;
} NSIG_Gparam;
/*============================================================*/
/*============================================================*/

/* One_device_structure.  Sect: 3.3.3.1 */
#ifdef NSIG_VER2
typedef struct {
  fourb status;
#define DEV_NULL_P   (0)         /*Not applicable*/
#define DEV_OK_P     (1)         /*OK*/
#define DEV_ERROR_P  (2)         /*Error has occured*/
/* The following are only valid for network devices */
#define DEV_REMOTE_P (5)         /*Remote computer unavailable*/
#define DEV_IRIS_P   (6)         /*Remote IRIS unavailable*/

/* This number indicates which process is using the device. */
  fourb process;
#define PROC_NONE_P    (0)       /*Noone is using it*/
#define PROC_RTDISP_P  (1)       /*Real time display*/
#define PROC_INGEST_P  (2)       /*Ingest*/
#define PROC_INGFIO_P  (3)       /*Ingest file output*/
#define PROC_REINGEST_P (4)       /*Reingest !!!tom */
#define PROC_OUTFMT_P  (5)       /*Output Formatter*/
#define PROC_PRODUCT_P (6)       /*Product generator*/
#define PROC_NETWORK_P (7)       /*Network  !!!tom */
#define PROC_QUICK_P   (8)       /*Quick look menu(part of out)*/
#define PROC_TAPE_P    (9)       /*Tape process*/
#define PROC_NORDRAD_P (10)       /*NORDRAD process*/

  /* Node name or user name */
  char  nuser_name[16];
  /* Number of characters in the name */
  char  nchar;
  /* Process mode, see process_status structure */
  fourb imode;
#define MODE_NULL    (0)
#define MODE_STOPPED (1)
#define MODE_IDLE    (2)
#define MODE_RUNNING (3)
#define MODE_EXIT    (4)
#define MODE_QUICK   (5)
#define MODE_INIT    (6)
#define MODE_HOLD    (7)

/* Used only for antenna device */
#define MODE_ANT_NULL  (0)
#define MODE_ANT_IRIS  (1)
#define MODE_ANT_LOCAL (2)
#define MODE_ANT_MAIN  (3)
#define MODE_ANT_COMP  (4)
#define MODE_ANT_SHUT  (5)
  char spare[8];
} NSIG_One_device;
#else
typedef struct {
  twob status;
  twob process;
  char user_name[15];
  char nchar;
  char spare[10];
} NSIG_One_device;
/*============================================================*/
/*============================================================*/
#endif

typedef struct {
#ifdef NSIG_VER2
/* THIS IS WRONG.... but, does it matter for RSL ???? */
  NSIG_Structure_header struct_head;
  NSIG_One_device       dsp_stat[4];
  NSIG_One_device       ant_stat[4];
  NSIG_One_device       outdev_stat[12];
  char spare[120];
#else
  NSIG_Structure_header struct_head;
  NSIG_One_device       dsp_stat[4];
  NSIG_One_device       ant_stat[4];
  NSIG_One_device       outdev_stat[12];
  char spare[120];
#endif
} NSIG_Device_status;
/*============================================================*/
/*============================================================*/
  
/* No change for NSIG_VER2 */
typedef struct {
  fourb startt; /* Start time (seconds within a day) */
  fourb stopt;  /* Stop  time (seconds within a day) */
  fourb skipt;  /* Desired skip time (seconds) */
  fourb time_last; /* Time last run (seconds w/in a day) */
  fourb time_used; /* Time used on last run (seconds) */
  fourb day_last;  /* Relative day of last run. */
  twob  iflag;     /* bit 0=ASAP, bit 1= Mandatory,
					* bit 2=Late skip, bit 3= Time used has been measured,
					* bit 4=Stop after running.
					*/
  char spare[94];
} NSIG_Task_sched_info;
/*============================================================*/
/*============================================================*/

#ifdef NSIG_VER2
typedef struct {
  twob low_prf; /* Hertz */
  twob low_prf_frac; /* Fraction part, scaled by 2**-16 */
  twob low_prf_sample_size;
  twob low_prf_range_averaging; /* In bins */
  twob thresh_refl_unfolding; /* Threshold for reflectivity unfolding in 1/100 dB */
  twob thresh_vel_unfolding; /* Threshold for velocity unfolding in 1/100 dB */
  twob thresh_sw_unfolding; /* Threshold for width unfolding in 1/100 dB */
  char spare[18];
} NSIG_Task_dsp_mode_batch;
#endif
/*============================================================*/
/*============================================================*/

#ifdef NSIG_VER2
typedef struct {
  fourb mask_word_0;
  fourb ext_hdr_type;
  fourb mask_word_1;
  fourb mask_word_2;
  fourb mask_word_3;
  fourb mask_word_4;
} NSIG_Dsp_data_mask;
#endif
/*============================================================*/
/*============================================================*/


/* Task dsp info 3.3.2.2, page III-22 */
typedef struct {
#ifdef NSIG_VER2
  twob   dsp_num;
  fourb   dsp_type;
  NSIG_Dsp_data_mask  data_mask_cur;
  NSIG_Dsp_data_mask  data_mask_orig;
  NSIG_Task_dsp_mode_batch task_dsp_mode;
  char spare[52];
#else
  twob   dsp_num;
  twob   dsp_type;
  fourb  data_mask;
  fourb  aux_data_def[32];
#endif
  fourb  prf;
  fourb  pwid;
  twob   prf_mode;
  twob   prf_delay;
  twob   agc_code;
  twob   samp_size;
  twob   gain_con_flag;
  char   filter_name[12];
#ifdef NSIG_VER2
  char idop_filter_first;	/* Doppler based filter used on first bin */
  char ilog_filter_first;	/* Z based filter used on first bin */
#else
  twob   f_num;
#endif
  twob   atten_gain;
#ifdef NSIG_VER2
  twob  igas_atten;		/* 100000 * db/km */
  twob  clutter_map_flag;
  twob  xmt_phase_seq;
  fourb ray_hdr_mask;
  twob  time_series_playback;
  twob  spare2;
  char  custom_ray_hdr_name[16];
#define TASK_DSP_INFO_PAD 120
  char ipad_end[TASK_DSP_INFO_PAD];
#else
  char   spare[150];
#endif
} NSIG_Task_dsp_info;
/*============================================================*/
/*============================================================*/

/* task cal info struct: 3.3.2.3, page III-22, rec 2 offset 944 */
/* No change for NSIG_VER2 */
typedef struct {
  twob slope;		  /* 00: Reflectivity slope (4096*dB/ A/D Count) */
  twob noise;		  /* 02: Noise threshold (1/16 dB above noise)   */
  twob clutr_corr;	  /* 04: Clutter correction threshold (1/16 dB)  */
  twob sqi;			  /* 06: (0-1)*256                               */
  twob power;		  /* 08: (1/16 dBZ)                              */
  char spare1[8];	  /* 10: <spare>  8                              */
  twob cal_ref;		  /* 18: Calibration reflectivity                */
  twob z_flag_unc;	  /* 20: Threshold flags for Unc. reflectivity   */
  twob z_flag_cor;	  /* 22: Threshold flags for Cor. reflectivity   */
  twob v_flag;		  /* 24: Threshold flags for velocity            */
  twob w_flag;		  /* 26: Threshold flags for width               */
  char spare2[8];	  /* 28: <spare> 8                               */
  twob speckle;		  /* 36: Speckle remover flag. See III-22        */
  twob slope_2;		  /* 38: Refl. slope for second processor        */
  twob cal_ref_2;	  /* 40: Calibration reflectivity for 2nd proc   */
  twob zdr_bias;	  /* 42: ZDR bias in signed 1/16 dB              */
  char spare3[276];	  /* 44: <spare> 276                             */
} NSIG_Task_calib_info;
/*============================================================*/
/*============================================================*/


/* Task_range_info Structure 3.3.2.4,  page III-23                    */
typedef struct { 
  fourb   rng_first;		/* 00: Range to first bin [cm]          */
  fourb   rng_last;			/* 04: Range to last bin [cm]           */
#ifdef NSIG_VER2
  fourb ibin_last;		/* Range of last (input) bin in cm */
#else
#endif
  twob    num_bins;			/* 08: Number of input bins             */
  twob    num_rngbins;		/* 10: Number of output range bins      */
  twob    var_bin_spacing;	/* 12: Variable range bin spacing (0,1) */
  fourb   binstep_in;		/* 14: Step between input bins          */
  fourb   binstep_out;		/* 18: Step between output bins         */
  twob    bin_avg_flag;		/* 22: Range bin averaging flag  */
                            /* 0:No Avg,  1:Avg Pairs, ... */
#ifdef NSIG_VER2
  char    spare[132];       /* 24: <spare> 132                      */
#else
  char    spare[136];       /* 24: <spare> 136                      */
#endif
} NSIG_Task_range_info;
/*============================================================*/
/*============================================================*/


/* Task scan info structure 3.3.2.5, page III-23 */
typedef struct {
  twob  ant_scan_mode; /* 1:PPI, 2:RHI, 3:manual, 4:file */
  twob  ang_res;       /* Desired angular resolution in 1/100 degree. */
#ifdef NSIG_VER2
 bang iscan_speed ;
#else
  twob  spare1;
#endif
  twob  num_swp;       /* Number of sweeps to perform. */
  bang  beg_ang;       /* Starting elevation(RHI)/azimuth(PPI) */
  bang  end_ang;       /* Ending   elevation(RHI)/azimuth(PPI) */
  bang  list[40];      /* List of azimuths(RSI)/elevations(PPI) */
#ifdef NSIG_VER2
  /*
union serv_task_scan_info_u
{
  struct serv_task_rhi_scan_info rhi;
  struct serv_task_ppi_scan_info ppi;
  struct serv_task_file_scan_info fil;
  struct serv_task_manual_scan_info man;
} ;
*/
  char  spare2[116];
#else
  char  spare3[112];
#endif
} NSIG_Task_scan_info;
/*============================================================*/
/*============================================================*/

typedef struct {
  fourb wavelength;       /* Wavelength in 1/100 of cm */
  char  serial_num[16];   /* T/R Serial Number */
  fourb xmit_pwr;         /* Transmit Power in watts. */
  twob  flag;             /* bit 0: Digital signal simulator in use.
						   * bit 4: Keep bit.
						   */
#ifdef NSIG_VER2
  twob ipolar;			/* Type of polarization, see dsp_lib.h */
  fourb itrunc;			/* Truncation height in cm */
  char  ipad32x18[18];		/* Reserved for polarization description */

#else
  char  spare1[24];       
#endif
  twob  display_parm1;    /* Real time display parameter #1 */
  twob  display_parm2;    /* Real time display parameter #2 */
  /* The following 3 members are not used in Ver 2. */
  twob  product_flag;     /* Real time product flag         */
  char  spare2[2];
  fourb truncation_height;/* Truncation height (cm) */
  twob  nbytes_comments;  /* Number of bytes of comments entered. */
  char  spare3[256];
} NSIG_Task_misc_info;

typedef struct {
  twob major; /* Task major number */
  twob minor; /* Task minor number */
  char name[12];  /* Name of task configuration file. */
  char desc[80];  /* Task description. */
#ifdef NSIG_VER2
  fourb ihybrid_count;		/* Number of tasks in this hybrid set */
#else

#endif
  twob state; /* Task state: 0=no task, 1=task being modified,
			   *             2=inactive, 3=scheduled, 4=running
			   */
#ifdef NSIG_VER2
 char spare[218];
#else
  char spare[222];
#endif
} NSIG_Task_end_data;
/*============================================================*/
/*============================================================*/


typedef struct {
  NSIG_Structure_header struct_head;
  NSIG_Task_sched_info  sched_info;
  NSIG_Task_dsp_info    dsp_info;
  NSIG_Task_calib_info  calib_info;
  NSIG_Task_range_info  range_info;
  NSIG_Task_scan_info   scan_info;
  NSIG_Task_misc_info   misc_info;
  NSIG_Task_end_data    end_data;
#ifdef NSIG_VER2
char comments[720];
#else

#endif
} NSIG_Task_config;
/*============================================================*/
/*============================================================*/


typedef struct {
  NSIG_Structure_header struct_head;
  NSIG_Product_config   prod_config;
  NSIG_Product_end      prod_end;
  char                  spare[5504];
} NSIG_Record1;
/*============================================================*/
/*============================================================*/

typedef struct {
  NSIG_Structure_header struct_head; 
  NSIG_Ingest_summary   ingest_head;
  NSIG_Task_config      task_config;
  NSIG_Device_status    device_stat;
  NSIG_Gparam           dsp1;
  NSIG_Gparam           dsp2;
  char                  spare[1260];
} NSIG_Record2;
/*============================================================*/
/*============================================================*/


/* This is the organization of the 2'nd to n'th file on the tape.
 * This structure is incomplete in that only one data record is
 * listed.  Record 1, Record 2, a data record.  As each data record
 * is ingested, the data replaces the data record part and keeps
 * the information in Record 1 and 2 unchanged.
 */
typedef unsigned char NSIG_Data_record[NSIG_BLOCK];

typedef struct {
  NSIG_Record1     rec1;
  NSIG_Record2     rec2;
  NSIG_Data_record data;
} NSIG_Product_file;
/*============================================================*/
/*============================================================*/


/* This is the first physical file on the TAPE. It is the only
 * file on the tape that has this organization.  All other files
 * are PRODUCT FILES (NSIG_Product_file).  Normally, this is ignored
 * when reading disk files; there you're only reading PRODUCT FILES.
 */
typedef struct {
  NSIG_Structure_header struct_head;
  char                  tape_id_name[16];
  char                  site_name[16];
  NSIG_Ymds_time        ymds;
  twob                  drive_num;
  twob                  tape_type;
  char                  spare[262];
} NSIG_Tape_header_file;
/*============================================================*/
/*============================================================*/

/* FUNCTION PROTOTYPES */
FILE *nsig_open(char *file_name);
void swap_nsig_record1(NSIG_Record1 *rec1);
void swap_nsig_record2(NSIG_Record2 *rec2);
void swap_nsig_raw_prod_bhdr(NSIG_Raw_prod_bhdr *rp);
void swap_nsig_ingest_data_header(NSIG_Ingest_data_header *ih);

/* Sweep reading structure */
typedef struct {
  NSIG_Ray_header h;
  unsigned char *range;  /* 0..h.num_bins-1 */
} NSIG_Ray;
/*============================================================*/
/*============================================================*/

typedef struct {
  NSIG_Raw_prod_bhdr bhdr;
  NSIG_Ingest_data_header idh;
  NSIG_Ray **ray;
  int nparams;  /* For freeing. */
} NSIG_Sweep;
/*============================================================*/
/*============================================================*/

/* Each routine in nsig.c is renamed when compiling Ver2 code.
 * The rename is simple: change nsig_ to nsig2_
 */
#ifdef NSIG_VER2
  #define nsig_open        nsig2_open
  #define nsig_read_record nsig2_read_record
  #define nsig_close       nsig2_close
  #define nsig_endianess   nsig2_endianess
  #define NSIG_I2          NSIG2_I2 
  #define NSIG_I4          NSIG2_I4 
  #define nsig_free_ray             nsig2_free_ray
  #define nsig_free_sweep           nsig2_free_sweep
  #define nsig_read_chunk           nsig2_read_chunk
  #define nsig_read_ext_header_ver0 nsig2_read_ext_header_ver0
  #define nsig_read_ext_header_ver1 nsig2_read_ext_header_ver1
  #define nsig_read_ray             nsig2_read_ray
  #define nsig_read_sweep           nsig2_read_sweep
  #define nsig_from_bang            nsig2_from_bang
  #define nsig_from_fourb_ang       nsig2_from_fourb_ang
  #define swap_nsig_structure_header swap_nsig2_structure_header
  #define swap_nsig_ymds_time        swap_nsig2_ymds_time
  #define swap_nsig_color_scale_def  swap_nsig2_color_scale_def
  #define swap_nsig_product_config   swap_nsig2_product_config
  #define swap_nsig_product_end      swap_nsig2_product_end
  #define swap_nsig_ingest_summary   swap_nsig2_ingest_summary
  #define swap_nsig_task_sched_info  swap_nsig2_task_sched_info 
  #define swap_nsig_task_dsp_info    swap_nsig2_task_dsp_info   
  #define swap_nsig_task_calib_info  swap_nsig2_task_calib_info 
  #define swap_nsig_task_range_info  swap_nsig2_task_range_info 
  #define swap_nsig_task_scan_info   swap_nsig2_task_scan_info  
  #define swap_nsig_task_misc_info   swap_nsig2_task_misc_info  
  #define swap_nsig_task_end_data    swap_nsig2_task_end_data   
  #define swap_nsig_task_config      swap_nsig2_task_config
  #define swap_nsig_one_device       swap_nsig2_one_device
  #define swap_nsig_device_status    swap_nsig2_device_status
  #define swap_nsig_gparam           swap_nsig2_gparam
  #define swap_nsig_record1          swap_nsig2_record1
  #define swap_nsig_record2          swap_nsig2_record2
  #define swap_nsig_raw_prod_bhdr      swap_nsig2_raw_prod_bhdr
  #define swap_nsig_ingest_data_header swap_nsig2_ingest_data_header
  #define get_extended_header_info     get2_extended_header_info
#endif

void nsig_free_ray(NSIG_Ray *r);
void nsig_free_sweep(NSIG_Sweep **s);
NSIG_Sweep **nsig_read_sweep(FILE *fp, NSIG_Product_file *prod_file);
int nsig_read_record(FILE *fp, char *nsig_rec);
int nsig_endianess(NSIG_Record1 *rec1);
short NSIG_I2 (twob x);
int NSIG_I4 (fourb x);
void nsig_close(FILE *fp);

float nsig_from_fourb_ang(fourb ang);
float nsig_from_bang(bang in);

#endif
