
/* Darwin data structures and parameters
 *
 * Dennis Flanigan, Jr.
 * Applied Research Corp.
 * NASA GSFC Code 910.1
 * 
 *
 * added tg_file_str structure 09 Jun 93    ...Mike
 * 
 * updated with new tg_ray_data structure 5/6/93   ...Mike
 * This structure is intended to supercede rp_ray
 *
 * updated for use with libtg 8/13/92
 *
 * updated 7/13/92
 *
 * 12/31/91
 *
 *
 */

#define TG_OK   0
#define TG_SYS_ERR  -1
#define TG_END_RAY  -2
#define TG_END_DATA -3
#define TG_REC_NOSEQ -4
#define TG_RAY_NOTYPE -5
#define TG_RAY_READ_ERR -6

#define TG_HDSIZE  1280
#define TG_RECSIZE 4096

#define TG_ANT_PPI 1
#define TG_ANT_RHI 2
#define TG_ANT_MAN 3
#define TG_ANT_FIL 4

/* field indices for tg_ray_data.da_inv */
#define TG_DM_IND  0     /* uncorrected reflectivity */
#define TG_DZ_IND  1     /* corrected reflectivity */
#define TG_VR_IND  2     /* radial velocity */
#define TG_SW_IND  3     /* spectral width */

/* missing data flag */
#define TG_NO_DATA 0x1000

#ifndef FALSE
#define FALSE    0
#endif

#ifndef TRUE
#define TRUE     1
#endif

/****** rp_ray is the old, outdated structure in which to 
  store (toga format-encoded) ray data. I have removed references
  to it in the toga library libtg.a .
  Use instead the tg_ray_data structure below, in which decoded
  ray data is stored....Mike  */
typedef struct
   {
   float elev;
   float azm;
   short bin[1800];  /* raw (encoded) ray data in toga format */ 
   }
rp_ray;

typedef struct
   {
   float azm;            /* azimuth angle */
   float elev;           /* elevation angle */
   /* time */            /* time of some sort (not done yet) */
   int da_inv[4];        /* data inventory */
   short num_bins[4];       /* number of bins */
   float start_km[4];          /* start range of data in km*/
   float interval_km[4];          /* interval of range bins in km */
   float data[4][1024];  /* real value data */
   }
tg_ray_data;


typedef struct
   {
   short axrat;         /* axial ratio   in signed hundredths */
   short ort_hor;       /* orientation ccw of horizontal in degrees */
   short pw_div_bits;   /* power divider bits (A) in low 7 bits  */
   short delay_bits;    /* delay bits (P) in low 7 bits */
   }
tg_tran_pol_str;


typedef struct
   {
   /* storm id : word 1*/
   short strm_year;
   short strm_mon;
   short strm_day;
   short strm_num;
   short map_num;
   
   /* time of start of scan : word 6*/
   short scan_year;
   short scan_mon;
   short scan_day;
   short scan_hour;
   short scan_min;
   short scan_sec;
   
   /* word 12 */
   short data_set;
   
   /* transmit polarization 1.1 : word 13*/
   short tp1_ar;
   short tp1_occw;
   short tp1_dibit;
   short tp1_debit;

   /* transmit polarization 1.2 : word 17*/
   short tp2_ar;
   short tp2_occw;
   short tp2_dibit;
   short tp2_debit;

   /* status bits : word 21*/
   short status;

   /* word 22 */
   short strng;         /* start range */
   short numbin;        /* number of data bins */
   short rnginc;        /* range increment between bins */
   short rngjit;        /* range jitter boollean */
   short numcbin;       /* number of range cal bins */
   short strtcal1;      /* start range of cal bins #1 */
   short strtcal2;      /* start range if cak bubs #2 */
   short stepcal;       /* step between cal bins 1 & 3 and 2 & 4 */
   short azmleft;       /* azimuth left, min azimuth */
   short azmrght;       /* azimuth right, max azimuth */
   short elev_low;      /* elevation low */
   short elev_hgh;      /* elevation  high */

   /* word 34 */
   short at_angres;     /* attempted angular res */
   short numfix_ang;    /* num fixed angles used */
   short angfix[20];    /* angles used for fixed coordinate */

   /* word 56 */
   short rlparm;        /* real time display parameters */
   short signois;       /* signal to noise threshold */
   short sigcltr;       /* signal to clutter threshold */
   short thrsh_flg;     /* threshold flags */

   /* word 60 */
   short numdsp;        /* number of doppler signal processors working */
   short numwrd;        /* number of words which are difined in ray header */

   /* word 62 */
   short scanmod;       /* scan mode */
   char  filename[16];  /* file name if scan mode is file (value 4)*/

   /* word 71 */
   short prf;           /* prf */
   short transiz;       /* number of samp per proc interval (transform size) */
   short spconf;        /* signal processor configuration */

   /* word 74 */
   short sufchar;       /* suffix character of data base directory */

   /* word 75 */
   short recsat1;       /* receiver saturation or */
   short recsat2;       /* 0 if standard rang-dependent STC was used */ 

   /*words 77 to 88 :  bias levels set to zeor if not applicable */

   /* word 77 */
   short dsp1cor_log;   /* co rec dsp 1, log rec noise level */
   short dsp1cor_iad;   /* co rec dsp 1, "I" a/d offset */
   short dsp1cor_qad;   /* co rec dsp 1, "Q" a/d offset */
   
   /* word 80 */
   short dsp1crr_log;   /* cross rec dsp 1, log rec noise level */
   short dsp1crr_iad;   /* cross rec dsp 1, "I" a/d offset */ 
   short dsp1crr_qad;   /* cross rec dsp 1, "Q" a/d offset */

   /* word 83 */
   short dsp2cor_log;   /* co rec dsp 2, log rec noise level */
   short dsp2cor_iad;   /* co rec dsp 2, "I" a/d offset */
   short dsp2cor_qad;   /* co rec dsp 2, "Q" a/d offset */
   
   /* word 86 */
   short dsp2crr_log;   /* cross rec dsp 2, log rec noise level */
   short dsp2crr_iad;   /* cross rec dsp 2, "I" a/d offset */ 
   short dsp2crr_qad;   /* cross rec dsp 2, "Q" a/d offset */

   /* word 89 */
   short wavelen;       /* wavelength in hundredths of cm */
   short pulsewd;       /* pulse width in hundredths of microsec. */
   short hortran_pow;   /* horizontal transmit power */
   short vertran_pow;   /* vertical transmit power */
   
   /* word 93 */       
   short high_zero;     /* height of zeroing in kilomiters */
   short sitelat;       /* latitude in .01 deg (if zero see words 108-111)*/
   short sitelong;      /* longitude in .01 deg  (if zero see words 108-111)*/
   short time_zone;     /* time zone of rec time, minutes ahead of GMT */

   /* word 97 */
   short zm_dsp1_mas;   /* Z slope, dsp 1, master board */
   short zm_dsp1_slv;   /* Z slope, dsp 1, slave board */
   short zm_dsp2_mas;   /* Z slope, dsp 2, master board */
   short zm_dsp2_slv;   /* Z slope, dsp 2, slave board */
   
   /* word 101 */
   short minz_dsp1_mas; /* minimum detectable Z, dsp 1, master */
   short minz_dsp1_slv; /* minimum detectable Z, dsp 1, slave */
   short minz_dsp2_mas; /* minimum detectable Z, dsp 2, master */
   short minz_dsp2_slv; /* minimum detectable Z, dsp 2, slave */
   
   /* word 105 */
   short num_pol;       /* number of polarization pairs used above 1 */

   /* word 106 */
   short exinfo_rayhd;  /* extra information in ray header */
                        /* bit 0 : IFF data available */
                        /* bit 1 : roll available */
                        /* bit 2 : pitch available */
                        /* bit 3 : heading available */

   /* word 107 */
   short len_exhd;      /* length of extended ray header ( 0 means 20 words) */

   /* word 108 */
   short lat_deg;       /* latitude degrees */
   short lat_hun_min;   /* latitude in .01 minutes */
   short lon_deg;       /* longitude degrees */
   short lon_hun_min;   /* longitude in .01 minutes */

   /* word 112 */
   short alt_atn;       /* altitude of antenna in meters above sea level*/
   short alt_grn;       /* altitude of ground at radar site in meters */
   
   /* word 114 */
   short vel_plat;      /* speed of platform from senser in .01 meters/sec */
   short vel_cor;       /* velocity value loaded into dsp for vel correction */
   short head_plat;     /* heading of platform from sensor in .1 degrees */
   short head_dsp;      /* heading loaded into dsp */
   
   /* word 118 */
   short set_plat;      /* set of platform (signed 1/10 degrees) */
   short drift_plat;    /* drift of platform (1/100 meters per sec */
   short ok_plat;       /* ok flags for words 108 to 120 */
                        /* bit 0: navigator input ok */
                        /* bit 1: navigator used for lat, long */
                        /* bit 2: navigator used for altitude  */
                        /* bit 3: navigator used for speed and heading */
                        /* bit 4: mavigator used for set and drift */
   

   /* word 121 */
   short spare121[79];
   
   /* word 200 */
   tg_tran_pol_str tp21;   /* transmit polarization 2.1 */
   tg_tran_pol_str tp22;   /* transmit polarization 2.2 */

   /* word 208 */
   tg_tran_pol_str tp31;   /* transmit polarization 3.1 */
   tg_tran_pol_str tp32;   /* transmit polarization 3.2 */

   /* word 216 */
   tg_tran_pol_str tp41;   /* transmit polarization 4.1 */
   tg_tran_pol_str tp42;   /* transmit polarization 4.2 */

   /* word 224 */
   tg_tran_pol_str tp51;   /* transmit polarization 5.1 */
   tg_tran_pol_str tp52;   /* transmit polarization 5.2 */

   /* word 232 */
   tg_tran_pol_str tp61;   /* transmit polarization 6.1 */
   tg_tran_pol_str tp62;   /* transmit polarization 6.2 */

   /* word 240 */
   tg_tran_pol_str tp71;   /* transmit polarization 7.1 */
   tg_tran_pol_str tp72;   /* transmit polarization 7.2 */
   
   /* word 248 */
   tg_tran_pol_str tp81;   /* transmit polarization 8.1 */
   tg_tran_pol_str tp82;   /* transmit polarization 8.2 */

   /* word 255 */
   short spare255[55];

   /* word 301 */
   char comments[680];
   }
tg_map_head_str;


typedef struct
   {
   short first_ray;
   short rec_num;
   short rec_bol;
   short res1;
   short data[2044];
   }
tg_data_rec_str;

typedef struct
   {
   short azm;
   short elev;
   short year;
   short mon;
   short day;
   short hour;
   short min;
   short hunsec;
   short tilt;
   short step;
   short type;
   short strt_rng;
   short srngkill;
   short erngkill;
   short spare[6];
   }
tg_ray_head_str;

/* tg_file_str contains all info relevant to one open toga data file */
typedef struct
   {
   int fd;
   int ray_num;
   int swap_bytes;
   short dec_buf[32768]; /*** Buffer and pointers for tg_read_map_bytes. */
   int buf_ind;
   int buf_end;          /****************/
   tg_data_rec_str recbuf; /*** buffer and indices for tg_read_rec_bytes. */
   int first_rec;
   int data_ind;
   int recnum;             /*************/
   tg_map_head_str map_head;
   tg_ray_head_str ray_head;
   tg_ray_data ray;
   }
tg_file_str;
