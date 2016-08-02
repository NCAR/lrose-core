/* 	$Id: dd_defines.h,v 1.2 2010/12/22 23:58:08 dixon Exp $	 */

#ifndef INCdd_definesH
#define INCdd_definesH



# define DD_DEFINES

# ifndef K64
# define K64 65536
# endif
# define ANGSCALE 0.005493164
# define BIN_ANG_TO_RADIANS .00009587379924
# define BA2F(x) ((float)(x)*ANGSCALE)
# define BA2RAD(x) ((float)(x)*BIN_ANG_TO_RADIANS)
# define CART_ANGLE(x) ((double)90.-(x))
# define RADIANS(x) ((x)*0.017453292)
# define DEGREES(x) ((x)*57.29577951)
# define PIOVR2 1.570796327
# ifndef PI
# define PI 3.141592654
# endif

# define TWOPI 6.283185307
# define RPM_TO_DPS(x) ((float)(x)*6.)
# ifndef US10
# define US10(x) ((float)(x)*.1)
# endif
# define SQ(x) ((x)*(x))

# define INC_NDX(i,que_size) ( ((i)+1) % (que_size) )
# define DEC_NDX(i,que_size) ( ((i)+(que_size)-1) % (que_size) )

# define DD_SCALE(x,scale,offset) ((x)*(scale)+(offset)+.5)

/* this is slower, but correct for negative numbers */
# define DD_SCALE2(x,scale,offset) ( (x) >= 0 ? ((x)*(scale)+(offset)+.5) : \
	((x)*(scale)+(offset)-.5) )

# define DD_UNSCALE(x,rcp_scale,offset) (((x)-(offset))*(rcp_scale))

# ifndef SPEED_OF_LIGHT
# define SPEED_OF_LIGHT 2.99792458e8 /* meters/sec. */
# endif

# define SECONDS_PER_DAY 86400
# define SECONDS_TO_DAYS 1.157407407e-5
# define SECONDS_TO_HOURS 2.777777778e-4
# define SECONDS_TO_MINUTES .01666667
# define DAY_ZERO 0
# define ETERNITY 0x7fffffff
# define D_SECS(h,m,s,ms) ((double)((h)*3600+(m)*60+(s))+(ms)*.001)

# define MALLOC_S(a_struct) ((a_struct *)malloc(sizeof(a_struct)))
# define BITS_TO_BYTES(x) ((((x)-1) >> 3) +1)
# define BYTES_TO_MB(x) ((x)*1.e-6)
# define LONGS_TO_BYTES(x) ((x)<<2)
# define SHORTS_TO_BYTES(x) ((x)<<1)
# define BYTES_TO_LONGS(x) ((((x)-1)>>2)+1)
# define BYTES_TO_SHORTS(x) ((((x)-1)>>1)+1)
# define SHORTS_TO_LONGS(x) ((((x)-1)>>1)+1)
# define SIZEOF_16_BITS 2	/* bytes */
# define SIZEOF_32_BITS 4	/* bytes */
# define   SIZEOF_FLOAT 4	/* bytes */

#ifndef NULL
# define     NULL 0
#endif
# define  A_MATCH 0
# define       NO 0
# define      YES 1
# define    RESET 2
# define  POINTER 3
# define iabs(x) (((x) < 0 ? -(x) : (x)))
# define IABS(x) (((x) < 0 ? -(x) : (x)))
# define RESET_IN |
# define SET_IN &
# define AND &
# define OR |
# define BACKWARD -1
# define FORWARD 1
# define BEFORE -1
# define AFTER 1

/* ray quality */
# define  UNACCEPTABLE 0x0001

# define BLOCK_SIZE K64
# define GS_TOLERANCE .001	/* meters */

# define     DD_FLUSH_ALL -1
# define  DD_FLUSH_VOLUME -2
# define     END_OF_SWEEP -8
# define END_OF_TIME_SPAN -9

# define  PHYSICAL_TAPE 0
# define          FB_IO 1
# define      BINARY_IO 2

/* hrd code word bit definitions */
# define REFLECTIVITY_BIT 0x8000 /* bit 15 */
# define VELOCITY_BIT     0x4000 /* bit 14 */
# define WIDTH_BIT        0x2000 /* bit 13 */
# define TA_DATA_BIT      0x1000 /* bit 12 */
# define LF_DATA_BIT      0x0800 /* bit 11 */
# define TIME_SERIES_BIT  0x0400 /* bit 10 */

typedef  char *CHAR_PTR;
typedef   int *INT_PTR;
typedef float *FLOAT_PTR;
typedef  long *LONG_PTR;

# define  DELETE_FLAG -32768
# define   EMPTY_FLAG DELETE_FLAG

/* data types */
# define  VELOCITY 1
# define     OTHER 0

/* format codes */
# define	  FOF_FMT 3
# define       SIGMET_FMT 5
# define	  HRD_FMT 6
# define       DORADE_FMT 7
# define	  CFT_FMT 8
# define	 TOGA_FMT 9
# define	   UF_FMT 10
# define       ELDORA_FMT 11
# define      WSR_88D_FMT 12
# define       LASSEN_FMT 14
# define	ARMAR_FMT 15
# define     NSSL_MRD_FMT 16
# define       APIRAQ_FMT 17
# define         TRMM_FMT 18
# define       MCGILL_FMT 19
# define         TDWR_FMT 20
# define    RADAR_CDF_FMT 21
# define        ETL2M_FMT 22
# define         CIMM_FMT 23
# define       NETCDF_FMT 24
# define       PIRAQX_FMT 25


/* Dorade radar types */
# define	   GROUND 0
# define	 AIR_FORE 1
# define	  AIR_AFT 2
# define	 AIR_TAIL 3
# define	   AIR_LF 4
# define	     SHIP 5
# define	 AIR_NOSE 6          /* ?? */
# define	SATELLITE 7
# define     LIDAR_MOVING 8
# define      LIDAR_FIXED 9

# define  FORE_TILT_ANGLE 17.5
# define   AFT_TILT_ANGLE (360.-17.5)

/* Field id numbers */
# define        SW_ID_NUM 1
# define        VR_ID_NUM 2
# define       NCP_ID_NUM 3
# define       DBZ_ID_NUM 4
# define        DZ_ID_NUM 5
# define        VE_ID_NUM 6
# define        VG_ID_NUM 7
# define        VU_ID_NUM 8
# define        VD_ID_NUM 9
# define       DBM_ID_NUM 10

/* Polarizations */
# define       HORIZONTAL 0
# define         VERTICAL 1
# define   CIRCULAR_RIGHT 2
# define       ELLIPTICAL 3

/* Scan modes */
# define 	      CAL 0        
# define 	      PPI 1
# define 	      COP 2
# define 	      RHI 3
# define 	      VER 4
# define 	      TAR 5
# define 	      MAN 6
# define 	      IDL 7
# define 	      SUR 8
# define 	      AIR 9
# define 	      HOR 10

/* Data reduction algorithms */
# define   NO_DATA_REDUCTION 0
# define          TWO_ANGLES 1
# define         TWO_CIRCLES 2
# define       TWO_ALTITUDES 3

/* Data reduction parameter 0 */
# define  DRP0_LEAST_POS_ANGLE 1
# define DRP0_INNER_CIR_DIA_KM 2
# define       DRP0_MIN_ALT_KM 3

/* Data reduction parameter 1 */
# define   DRP1_MOST_POS_ANGLE 1
# define DRP1_OUTER_CIR_DIA_KM 2
# define       DRP1_MAX_ALT_KM 3

/* Parameter type */
# define      DD_8_BITS 1
# define     DD_16_BITS 2
# define     DD_24_BITS 3
# define   DD_32_BIT_FP 4
# define   DD_16_BIT_FP 5

/* Filtering */
# define NO_FILTERING 0
# define FILTERING_ON 1

/* Compression */
# define         NO_COMPRESSION 0
# define        HRD_COMPRESSION 1
# define  RDP_8_BIT_COMPRESSION 8

# define     END_OF_COMPRESSION 1
# define                  SIGN8 0x80
# define                  MASK7 0x7f
# define                 SIGN16 0x8000
# define                 MASK15 0x7fff

/* Ray status */
# define         NORMAL 0
# define  IN_TRANSITION 1
# define            BAD 2
# define   QUESTIONABLE 3

# define DD_NAME_DELIMITER "."

# define   MAX_BIN_FORMATS 16
# define      MAX_DGI_SIZE 500000
# define        MAX_DOUBLE 1.e22
# define         MAX_FLOAT 1.e22
# define          MAX_FREQ 5
# define          MAX_IPPS 5
# define          MAX_LIMS 64
# define         MAX_MODES 10
# define         MAX_PARMS 64
# define      MAX_REC_SIZE 65500 /* funky number thanks to Mass Store folks */
# define     MAX_PREV_RAYS 5
# define       MAX_SENSORS 16
# define     MAX_PREV_SWPS 5
# define          MAX_READ 5*K64
# define          BAD_TIME -MAX_FLOAT

# define SIZE_COMMENTS 32768

# define TS_OK	0
# define TS_EOF	1
# define TS_ERROR 2
# define TS_EOT 3

# define DD_RAY_BOUNDARIES "RYIB-SWIB-VOLD"
# define DD_ID_LIST "-NULL-ASIB-CELV-CFAC-COMM-CSPD-FRAD-FRIB-PARM-RADD-RDAT-RYIB-SSWB-SWIB-VOLD-WAVE-"

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

# endif

/* c----------------------------------------------------------------------- */
#endif /* INCdd_definesH */





