/* *************************************************
 * *                                               *
 * *  SIGMET Standard Type Definition Header File  *
 * *                                               *
 * *************************************************
 * File: include/sigtypes.h
 *
 * COPYRIGHT (c) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006 BY 
 *             VAISALA INC., WESTFORD MASSACHUSETTS, U.S.A.  
 * 
 * THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
 * ONLY  IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
 * INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE  OR  ANY OTHER
 * COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
 * OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
 * TRANSFERED. 
 */ 
#include "signoarch.h" /* Always include architecture-independent defs */

#ifndef SIGMET_SIGTYPES_H
#define SIGMET_SIGTYPES_H 1

/* ============== Check for incorrect machine flags ==============
 */
#ifdef HPUX
#define _HPUX 1
#else
#define _HPUX 0
#endif

#ifdef IRIX
#define _IRIX 1
#else
#define _IRIX 0
#endif

#ifdef LINUX
#define _LINUX 1
#else
#define _LINUX 0
#endif

#ifdef MSDOS
#define _MSDOS 1
#else
#define _MSDOS 0
#endif

// #if (_HPUX + _IRIX + _LINUX + _MSDOS) != 1
// error 'Inconsistent Machine FLags'
// #endif

#undef _HPUX
#undef _IRIX
#undef _LINUX
#undef _MSDOS

/* =========== Various Physical and Numerical Constants ===========
 */

/* Speed of light in Km/Sec
 */
#define FSPEED_OF_LIGHT (299792458.0)  

/* PI, and some variants
 */
#define FPI   (3.14159265358979)
#define FPI_P FPI
#define SQRT2PI (2.506628274)

/* KM per Nautical Mile, Statute Mile, and Kilofoot
 */
#ifndef KM_PER_NM
#define KM_PER_NM   (1.852)
#endif
#define KM_PER_MILE (1.609344)
#define KM_PER_KFT  (0.3048)

/* meters per second per knot, mi/hr, and km/hr
 */
#define MPS_PER_KNOT (KM_PER_NM   * 1000.0/3600.0)
#define MPS_PER_MPH  (KM_PER_MILE * 1000.0/3600.0)
#define MPS_PER_KMHR (1000.0/3600.0)

/* cm / inch
 */
#define CM_PER_INCH (2.54)

/* Average earth radius in Km, and effective radar radius in Km.
 */
#define FEARTH_MEAN_RADIUS    (6371.0)
#define FEARTH_RADAR_RADIUS   (8495.0)

/* Directory and pathname sizes
 */
#define DIRECTORY_SIZE (81)  /* Size allocated to hold directories */
#define FILENAME_SIZE  (80)  /* Size allocated to hold filenames */
#define PATHNAME_SIZE  (160) /* Size allocated to hold pathnames */

/* Signal and Message definitions.  The message catalog number, plus
 * other internal modification bits, reside in the low 16-bits of a
 * 32-bit message.  The Facility number is in the upper 16-bits.
 */
#define SS_NORMAL    (0x00000001) /* Normal Successful Return */

#define MSGMOD_BIT   (11)	  /* Bit offset of mod bits in message value */
#define MSGVAL_MASK  (0xFFFF &  ((1 << MSGMOD_BIT) - 1))
#define MSGMOD_MASK  (0xFFFF & ~((1 << MSGMOD_BIT) - 1))
#define MSGMOD_COUNT ((1 << (16 - MSGMOD_BIT)) - 1)

#define SIGBASE( SIGNAL ) ((SIGNAL) & (0xFFFF0000 | MSGVAL_MASK))

/* Basic typedefs for all platforms
 */
#ifdef HPUX
#define SIGNED
#endif
#ifdef IRIX
#define SIGNED signed
#define TWOD_QUALIFIERS_BROKEN 1
#endif
#ifdef LINUX
#define SIGNED signed
#define TWOD_QUALIFIERS_BROKEN 1
#endif
#ifdef MSDOS
#define SIGNED signed
#define TWOD_QUALIFIERS_BROKEN 1
#endif

/* Use this if the system call has a (char *) argument which
 * should be (const char *).
 */
#define SHOULD_BE_CONST( VALUE ) ((char *)(VALUE))

typedef   SIGNED char  SINT1;
typedef unsigned char  UINT1;
typedef   SIGNED short SINT2;
typedef unsigned short UINT2;
typedef   SIGNED long  SINT4;
typedef unsigned long  UINT4;
typedef float           FLT4;
typedef double          FLT8;

typedef unsigned short  BIN2;
typedef unsigned long   BIN4;

typedef UINT4 MESSAGE;

/* Max signed, Min signed, and Max unsigned integer bounds
 */
#define MAXSINT4 ((SINT4)0x7FFFFFFF)
#define MINSINT4 ((SINT4)0x80000000)
#define MAXUINT4 ((UINT4)0xFFFFFFFF)

#define MAXSINT2 ((SINT2)0x7FFF)
#define MINSINT2 ((SINT2)0x8000)
#define MAXUINT2 ((UINT2)0xFFFF)

#define MAXSINT1 ((SINT1)0x7F)
#define MINSINT1 ((SINT1)0x80)
#define MAXUINT1 ((UINT1)0xFF)

/* Macros for general use
 */
#define BIT( ibit ) ( 1 << (ibit) )
#define BTEST( ivalue, ibit ) (((ivalue) &  BIT(ibit)) != 0 )
#define IBSET( ivalue, ibit ) ( (ivalue) |  BIT(ibit) )
#define IBCLR( ivalue, ibit ) ( (ivalue) & ~BIT(ibit) )
#define IBINV( ivalue, ibit ) ( (ivalue) ^  BIT(ibit) )

#define NINT(fvalue) ((SINT4)floor(0.5+(double)(fvalue)))
#define SQUARE(value) ((value)*(value))

#define STR_UPPER(str,size) \
   {int _ii; for(_ii=0;_ii<size;_ii++) (str)[_ii]=toupper((str)[_ii]);}

#define STR_LOWER(str,size) \
   {int _ii; for(_ii=0;_ii<size;_ii++) (str)[_ii]=tolower((str)[_ii]);}

#define SCALARSWAP( ITEMX, ITEMY, TYPE ) \
  do { TYPE __swapVal = ITEMX ; ITEMX = ITEMY ; ITEMY = __swapVal ; } while (0)

#ifndef FALSE  
#define TRUE  (1)
#define FALSE (0)
#endif

/* Copyright Notices embedded in object and executable code.  Some of
 * the compilers have support for embedded notices, others do not, so
 * use our own construction.
 */
#ifdef HPUX
#pragma COPYRIGHT "VAISALA INC."
#endif
#ifdef IRIX
static volatile const char *vaisala_copyright_notice  =
  { "Copyright VAISALA INC., 2006.  All Rights Reserved." } ;
#endif
#ifdef LINUX
volatile static const char *vaisala_copyright_notice __attribute__ ((unused)) =
  { "Copyright VAISALA INC., 2006.  All Rights Reserved." } ;
#endif
#ifdef MSDOS
volatile static const char *vaisala_copyright_notice __attribute__ ((unused)) =
  { "Copyright VAISALA INC., 2006.  All Rights Reserved." } ;
#endif

/* Message facility numbers.  Allocate them all here to avoid name
 * collisions and numeric collisions.
 */
#define ASCOPE_FACILITY     (  11 << 16 )
#define RVP8MAIN_FACILITY   (  12 << 16 )
#define RVP8PROC_FACILITY   (  13 << 16 )
#define RCP8_FACILITY       (  14 << 16 )

#define ANT_FACILITY        ( 100 << 16 )
#define DSP_FACILITY        ( 101 << 16 )
#define CONFIG_FACILITY     ( 103 << 16 )
#define TVSUBS_FACILITY     ( 104 << 16 )
#define VTV_FACILITY        ( 105 << 16 )
#define USER_FACILITY       ( 107 << 16 )
#define LINK_FACILITY       ( 108 << 16 )
#define RIBLIB_FACILITY     ( 109 << 16 )
#define PCI_FACILITY        ( 110 << 16 )
#define RDA_FACILITY        ( 111 << 16 )
#define IPP_FACILITY        ( 112 << 16 )
#define SPL_FACILITY        ( 113 << 16 )
#define MAPSLIB_FACILITY    ( 114 << 16 )
#define HIMATH_FACILITY     ( 115 << 16 )
#define RTQLIB_FACILITY     ( 116 << 16 )

#define TSARCHLIB_FACILITY  ( 150 << 16 )

#define ING_FACILITY        ( 202 << 16 )
#define FIO_FACILITY        ( 203 << 16 )
#define IRIS_FACILITY       ( 204 << 16 )
#define WAT_FACILITY        ( 205 << 16 )
#define RADAR_FACILITY      ( 206 << 16 )
#define RIBBLD_FACILITY     ( 207 << 16 )
#define PRO_FACILITY        ( 208 << 16 )
#define OUT_FACILITY        ( 209 << 16 )
#define ARCHIVE_FACILITY    ( 210 << 16 )
#define REI_FACILITY        ( 211 << 16 )
#define INPUT_FACILITY      ( 212 << 16 )
#define SERV_FACILITY       ( 213 << 16 )
#define NET_FACILITY        ( 214 << 16 )
#define IRIS_SHR_FACILITY   ( 215 << 16 )
#define NOR_FACILITY        ( 217 << 16 )

#define VCPC_FACILITY       ( 230 << 16 )

/* Storage of time as Year, Month, day, and # seconds into the day.
 */
#define YMDS_FLG_DST  (0x0400)   /* Time is in Daylight Savings Time */
#define YMDS_FLG_UTC  (0x0800)   /* Time is UTC (else local) */
#define YMDS_FLG_LDST (0x1000)   /* Local time is in Daylight Savings Time */
struct ymds_time
{
  UINT4 isec;
  UINT2 imills;		/* Fractions of seconds in milliseconds, in low 10 bits */
  UINT2 iyear, imon, iday;
};

/* Extract the milliseconds from the ymds structure
 */
#define YMDS_MASK_FLAGS   (0xfc00)
#define YMDS_MASK_MS      (0x03ff)
#define DST_FROM_MILLS( _MILLS )   (0 != ( YMDS_FLG_DST & (_MILLS) ))
#define UTC_FROM_MILLS( _MILLS )   (0 != ( YMDS_FLG_UTC & (_MILLS) ))
#define LDST_FROM_MILLS( _MILLS )  (0 != ( YMDS_FLG_LDST & (_MILLS) ))
#define FLAGS_FROM_MILLS( _MILLS ) ( YMDS_MASK_FLAGS & (_MILLS) )
#define MS_FROM_MILLS( _MILLS )    ( YMDS_MASK_MS & (_MILLS) )
#define MILLS_FROM_MS_FLAGS( _MS, _FLAGS ) ((YMDS_MASK_MS&(_MS))|(YMDS_MASK_FLAGS&(_FLAGS)))

/* Structures used for passing time between client and server.
 * Storage of time as Year, Month, day, seconds, and nanoseconds.
 */
#define SYMDS_FLG_DST  (0x00100000)   /* Time is in Daylight Savings Time */
#define SYMDS_FLG_UTC  (0x00200000)   /* Time is UTC (else local) */
#define SYMDS_FLG_LDST (0x00400000)   /* Local time is in Daylight Savings Time */
struct serv_ymds_time
{
  UINT4 iyear, imon, iday;
  UINT4 isec;
  UINT4 inanos;            /* Fractions in nanoseconds in low 20 bits */
};

/* Extract the nanoseconds from the serv_ymds structure
 */
#define SYMDS_MASK_FLAGS   (0xFFF00000)
#define SYMDS_MASK_NS      (0x000FFFFF)
#define DST_FROM_NANOS( _NANOS )  (0 != ( SYMDS_FLG_DST & (_NANOS) ))
#define UTC_FROM_NANOS( _NANOS )  (0 != ( SYMDS_FLG_UTC & (_NANOS) ))
#define LDST_FROM_NANOS( _NANOS ) (0 != ( SYMDS_FLG_LDST& (_NANOS) ))
#define DST_FROM_NANOS( _NANOS )  (0 != ( SYMDS_FLG_DST & (_NANOS) ))
#define FLAGS_FROM_NANOS( _NANOS )( SYMDS_MASK_FLAGS & (_NANOS) )
#define NS_FROM_NANOS( _NANOS )   ( SYMDS_MASK_NS & (_NANOS) )
#define NANOS_FROM_NS_FLAGS( _NS, _FLAGS ) \
  ((SYMDS_MASK_NS&(_NS))|(SYMDS_MASK_FLAGS&(_FLAGS)))

#define NANOS_FROM_MILLS( _MILLS2 ) \
( (1000*MS_FROM_MILLS(_MILLS2)) | (FLAGS_FROM_MILLS(_MILLS2)<<10) ) 

#define MILLS_FROM_NANOS( _NANOS2 ) \
( (NS_FROM_NANOS(_NANOS2)/1000) | (FLAGS_FROM_NANOS(_NANOS2)>>10) )

#define TZNAME_SIZE (8)
struct full_time_spec
{
  struct ymds_time Ymds;
  char  sLocalTZName[TZNAME_SIZE];
  char  sRecordTZName[TZNAME_SIZE];
  long  tv_sec;       /* Seconds from timeval structure */
  long  tv_usec;      /* Microseconds from timeval structure */
  SINT4 iRelDay;      /* Sigmet defined relative day number */
  SINT2 iMinutesWest; /* Offset from recorded standard time to UTC */
  SINT2 iLocalWest;   /* Offset from local standard time to UTC */
  SINT2 iJulianDay;   /* Julian day origin 0, like tm_yday */
  char  ipad_end[2];
};

/* Latitude and Longitude of a point on the earth
 */
struct latlon8
{
  FLT8  fLat;  /* Angle in radians +/- PI/2 */
  FLT8  fLon;  /* Angle in radians +/- PI   */
#define LATLON_FLG_INVALID (0x0001)
  UINT4 iFlags;
  char  ipad_end[4];
};

/* Polar coordinate relative position
 */
struct D2Polar8
{
  FLT8  fRange;  /* Range in km */
  FLT8  fAz;     /* Azimuth in radians range 0 to 2PI   */
};

/* Triangle on either a plane or sphere.
 */
struct Triangle8
{
  FLT8  fAngle1;  /* Angle of vertex in radians */
  FLT8  fAngle2;  
  FLT8  fAngle3;  
  FLT8  fSide1;   /* Size opposite angle 1 */
  FLT8  fSide2; 
  FLT8  fSide3; 
};

/* Various XY points
 */
struct XYPoint4
{
  FLT4 fX, fY ;
};
struct XYPoint8
{
  FLT8 fX, fY ;
};

/* ------------------------------
 * FIFO holding a string of bytes (actually, unsigned shorts, so that
 * extra bits can be passed with each byte if needed).  The macro
 * expands into whatever size structure is needed.  The buffer size
 * may be one slot longer to insure that the size of the overall
 * structure is a multiple of four.
 */
#define BFIFO(NSLOTS) struct {		\
  UINT2 nSlots, writeptr, readptr ;	\
  UINT2 buf[ (NSLOTS) | 1 ] ;		\
}
#define BFIFO_HSIZE 6		/* Size of header portion */

/* ------------------------------
 * Least Squares and Polynomial Fitting Structures
 */
#define MAX_POLY_ORDER (11)
struct polynomial {
  SINT4 iorder ;
  FLT4 fcoefs[MAX_POLY_ORDER+1] ;
} ;

struct lsquare8 {
  SINT4 icount ;		      /* Number of points */
  SINT4 iorder ;		      /* Max order of polynomial fit */
  FLT8  fWeight ;		      /* Total weight of all points */
  FLT8  xnsum[1+(2*MAX_POLY_ORDER)] ; /* Sums of X^n */
  FLT8  yxnsum[1+MAX_POLY_ORDER]  ;   /* Sums of Y*X^n */
  FLT8  yysum ;                       /* Sum of Y^2 */
} ;

/* Two-dimensional polynomials defined by XYPOLY are scalar functions
 * of (x,y).  The order of XYPOLY is the highest sum of the individual
 * powers of "x" and "y" in any given term.  An Nth order XYPOLY
 * consists of an (N-1)th order XYPOLY followed by (N+1) additional
 * terms.  For example, an order three XYPOLY would have the following
 * 10 coefficients corresponding to the indicated powers of "x" and
 * "y":
 *
 *    00   10  01  20  11  02  30  21  12  03
 *   ----  ------  ----------  --------------
 *   zero   one       two           three
 */
#define MAX_XYPOLY_ORDER (4)
#define MAX_XYPOLY_TERMS ( ((MAX_XYPOLY_ORDER+1) * (MAX_XYPOLY_ORDER+2)) / 2 )
#define MAX_XYPOLY_UDIAG ( ((MAX_XYPOLY_TERMS  ) * (MAX_XYPOLY_TERMS+1)) / 2 )

struct xypoly8 {		      /* Polynomial in (x,y) */
  SINT4 iOrder, iPad4x4 ;	      /*   Highest x+y power */
  FLT8  fTerms[ MAX_XYPOLY_TERMS ] ;  /*   Coefficients, or mixed data terms */
} ;
struct xypoly_lsq8 {		      /* Least squares statistics */
  SINT4 iCount ;		      /*   Number of points */
  SINT4 iOrder ;		      /*   Max order of polynomial fit */
  FLT8  fWSums[ MAX_XYPOLY_TERMS ] ;  /*   Weighted data sums */
  FLT8  fUDiag[ MAX_XYPOLY_UDIAG ] ;  /*   Upper diagonal matrix terms */
  FLT8  fZZSum ;		      /*   Sum of Z^2 */
} ;
struct xypoly_area8 {		      /* 2D area modelling descriptor */
  struct xypoly8 xypoly ;	      /*   The model function for the region */
  FLT4 fMinX, fMaxX, fMinY, fMaxY ;   /*   X and Y limits of the region */
  FLT4  fZOffset ;		      /*   Offset to add for final value */
  FLT4  fError ;		      /*   Mean error in the fit */
  UINT4 iFlags ;		      /*   Flags affecting the model */
  char  ipad_end[4];
#define XYPOLYAREAFLAGS_SQMODEL 0x01  /*     Model Z^2 rather than Z */
} ;

/* Flags for units conversion/display
 */
#define UNITS_METRIC    (0) /* Use: km, km,  m/s  */
#define UNITS_NAUTICAL  (1) /* Use: nm, kft, kts  */
#define UNITS_METRIC_HR (2) /* Use: km, km, km/hr */
#define UNITS_ENGLISH   (3) /* Use: mi, kft, mi/hr */

/* Type for VTV image pointer, needs somewhat global definition
 */
typedef unsigned long VTV;

/* Do not change this without checking the event flags defined in event_flags.h
 */
#define MAX_OUT_DEVICES       (24)       /*Max # of output devices in IRIS*/
#define MAX_INPUT_DEVICES     (16)       /*Max # of input devices in IRIS*/

/* Maximum number of different political overlays loaded into memory.  Note that
 * if the overlays are large, the storage space will be used up, and less can
 * be loaded.
 */
#define MAX_OVERLAYS    (20)

/* Structure holding information about data types.  This is somewhat
 * awkward to deal with backwards compatibility.
 */
struct dsp_data_mask
{
  UINT4 iWord0;
  UINT4 iXhdrType;
  UINT4 iWord1;
  UINT4 iWord2;
  UINT4 iWord3;
  UINT4 iWord4;
};

/* ------------------------------
 * Color structures
 */
struct color_def	 /* Define the color map for a single entry */
{
  UINT1 ired;
  UINT1 igreen;
  UINT1 iblue;
  UINT1 pad3x1[1] ;
};

struct linear_list_header
{
  SINT4 iused, isize;		/* # slots used, and total # slots */
};

/* Structure used to hold 42-bit numbers.  The low ten bits (1K) are
 * split off from the upper 32.
 */
struct kilo_count {
  SINT4 ikilo ;			/* Upper 32-bits (thousands) */
  SINT4 iones ;			/* Lower 10-bits (ones) */
} ;

/* ------------------------------
 * Some constants that can't reside within more specific include files
 * due to awkwardness when including them.
 */
#define MAXTRIGPERIODS (64)	/* Longest definable trigger sequence, and */
#define CUSTOMTRIGS    ( 4)	/*   max number of custom sequences. */

/* *************************************
 * *                                   *
 * *  Data Parameter Type Definitions  *
 * *                                   *
 * *************************************
 */
/* These are the bit numbers that are used in may places to specify a
 * choice of data type and to tell what kind of data to process.  Note
 * that the "Extended Header" type is included here, though it is not
 * generated by the DSP.  Historically, types 0-31 were directly
 * produced by the DSP, but that distinction is no longer maintained.
 */
#define DB_XHDR          (0)	/* Extended Headers  */
#define DB_DBT           (1)	/* Total power (1 byte) */
#define DB_DBZ           (2)	/* Clutter Corrected reflectivity (1 byte) */
#define DB_VEL           (3)	/* Velocity (1 byte) */
#define DB_WIDTH         (4)	/* Width (1 byte) */
#define DB_ZDR           (5)	/* Differential reflectivity (1 byte) */
#define DB_ORAIN         (6)	/* Old Rainfall rate (stored as dBZ), not used */
#define DB_DBZC          (7)	/* Fully corrected reflectivity (1 byte) */
#define DB_DBT2          (8)	/* Uncorrected reflectivity (2 byte) */
#define DB_DBZ2          (9)	/* Corrected reflectivity (2 byte) */
#define DB_VEL2         (10)	/* Velocity (2 byte) */
#define DB_WIDTH2       (11)	/* Width (2 byte) */
#define DB_ZDR2         (12)	/* Differential reflectivity (2 byte) */
#define DB_RAINRATE2    (13)	/* Rainfall rate (2 byte) */
#define DB_KDP          (14)	/* Kdp (specific differential phase)(1 byte) */
#define DB_KDP2         (15)	/* Kdp (specific differential phase)(2 byte) */
#define DB_PHIDP        (16)	/* PHIdp (differential phase)(1 byte) */
#define DB_VELC         (17)	/* Corrected Velocity (1 byte) */
#define DB_SQI          (18)	/* SQI (1 byte) */
#define DB_RHOHV        (19)	/* RhoHV(0) (1 byte) */
#define DB_RHOHV2       (20)	/* RhoHV(0) (2 byte) */
#define DB_DBZC2        (21)	/* Fully corrected reflectivity (2 byte) */
#define DB_VELC2        (22)	/* Corrected Velocity (1 byte) */
#define DB_SQI2         (23)	/* SQI (2 byte) */
#define DB_PHIDP2       (24)	/* PHIdp (differential phase)(2 byte) */
#define DB_LDRH         (25)	/* LDR H to V (1 byte) */
#define DB_LDRH2        (26)	/* LDR H to V (2 byte) */
#define DB_LDRV         (27)	/* LDR V to H (1 byte) */
#define DB_LDRV2        (28)	/* LDR V to H (2 byte) */
#define DB_FLAGS        (29)	/* Individual flag bits for each bin */
#define DB_FLAGS2       (30)	/*   (See bit definitions below) */

#define DB_HEIGHT       (32)	/* Height (1/10 km) (1 byte) */
#define DB_VIL2         (33)	/* Linear liquid (.001mm) (2 byte) */
#define DB_NULL         (34)	/* Data type is not applicable  */
#define DB_SHEAR        (35)	/* Wind Shear (1 byte) */
#define DB_DIVERGE2     (36)	/* Divergence (.001 10**-4) (2-byte) */
#define DB_FLIQUID2     (37)	/* Floated liquid (2 byte) */
#define DB_USER         (38)	/* User type, unspecified data (1 byte) */
#define DB_OTHER        (39)	/* Unspecified data, no color legend */
#define DB_DEFORM2      (40)	/* Deformation (.001 10**-4) (2-byte) */
#define DB_VVEL2        (41)	/* Vertical velocity (.01 m/s) (2-byte) */
#define DB_HVEL2        (42)	/* Horizontal velocity (.01 m/s) (2-byte) */
#define DB_HDIR2        (43)	/* Horizontal wind direction (.1 degree) (2-byte) */
#define DB_AXDIL2       (44)	/* Axis of Dillitation (.1 degree) (2-byte) */
#define DB_TIME2        (45)	/* Time of data (seconds) (2-byte) */
#define DB_RHOH         (46)	/* Rho H to V (1 byte) */
#define DB_RHOH2        (47)	/* Rho H to V (2 byte) */
#define DB_RHOV         (48)	/* Rho V to H (1 byte) */
#define DB_RHOV2        (49)	/* Rho V to H (2 byte) */
#define DB_PHIH         (50)	/* Phi H to V (1 byte) */
#define DB_PHIH2        (51)	/* Phi H to V (2 byte) */
#define DB_PHIV         (52)	/* Phi V to H (1 byte) */
#define DB_PHIV2        (53)	/* Phi V to H (2 byte) */
#define DB_USER2        (54)	/* User type, unspecified data (2 byte) */

/* Remember to update these counts whenever new data types are added
 * to the above list.
 */
#define      NUM_DEFINED_DATA (55) /* Total number of defined data types */
#define  DSP_NUM_DEFINED_DATA (16) /* Number of data types direct from the DSP */
#define TASK_NUM_DEFINED_DATA (19) /* Number of data types used in IRIS/TCF menu */

#define  SETUP_DTYPE_COUNT (15)                  /* Number of data types in setup */
#define SERVER_DTYPE_COUNT (SETUP_DTYPE_COUNT+3) /* Number of data types in POM */

/* Special data values that can be used for many of the above data
 * types to indicate exceptional conditions.
 */
#define DV_NOSCAN1         (      0xFF) /* Area-not-scanned code for */
#define DV_NOSCAN2         (    0xFFFF) /*   the 1-byte, 2-byte, and */
#define DV_NOSCAN4         (0xFFFFFFFF) /*   4-byte data types. */
#define DV_THRESH1         (         0) /* Thresholded */
#define DV_THRESH2         (         0) /* Thresholded */
#define DV_HEIGHT1_UNKNOWN (      0xFE) /* Height not known */

/* Bit assignments for the DB_FLAGS data type
 */
#define DBFLG_OBSCRREFL  0x0001	/* These bits indicate whether reflectivity, */
#define DBFLG_OBSCRVEL   0x0002	/*   velocity, and width data are obscured at */
#define DBFLG_OBSCRWID   0x0004	/*   this bin, i.e., value can't be determined */
#define DBFLG_POINTCLUT  0x0008	/* Point clutter detected at this bin */

/* What kind of scan geometry is being used.
 */
#define TASK_SCAN_UNKNOWN  0		/* Undetermined scan type */
#define      SCAN_UNKNOWN  0		/* Undetermined scan type */
#define TASK_SCAN_PPI      1		/* PPI (Sector) */
#define      SCAN_PPI      1		/* PPI (Sector) */
#define TASK_SCAN_RHI      2		/* RHI (Sector) */
#define      SCAN_RHI      2		/* RHI (Sector) */
#define TASK_SCAN_MANUAL   3		/* Manual */
#define      SCAN_MANUAL   3		/* Manual */
#define TASK_SCAN_CONT     4		/* Continuous AZ PPI's */
#define      SCAN_CONT     4		/* Continuous AZ PPI's */
#define TASK_SCAN_FILE     5		/* File scan */
#define      SCAN_FILE     5		/* File scan */
#define TASK_SCAN_EXEC     6		/* Exec an arbitrary program instead */

/* IRIS data convert helper structure
 */
#define CONVERT_FLAGS_DEFAULT 0   /* Use default units */
struct data_convert
{
  FLT4 fvelun_velocity; /* Nyquist velocity, including unfolding */
  FLT4 fvelun_width;    /* Nyquist velocity, not including unfolding */
  FLT4 flambda;         /* Wavelength in cm */
  UINT4 iColorFlags;    /* Flags controlling type of color scale */
};

#endif  /* #ifndef SIGMET_SIGTYPES_H */
