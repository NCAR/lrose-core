#ifndef RSL_HH
#define RSL_HH

#include <climits>
#include <Radx/Radx.hh>

// TODO: convert all the Radx::si32, Radx::fl32, etc. to Radx::Int32, etc.

// how to handle this ?
// Let's just make it all TWO_BYTE_PRECISION
// If #define USE_TWO_BYTE_PRECISION when building and installing RSL, then
//typedef unsigned short Range;  // 2 bytes
//typedef Radx::si16 Range;  // 2 bytes
typedef Radx::fl32 Range;  // 4 bytes
// else,
// typedef unsigned char Range;

// TODO: convert this to a missing or bad value from Radx.hh
//#define BADVAL  (Radx::si16)0x2000  // 2 bytes


//
// copy of trmm_rsl data structures

// ----------------
// Ray

typedef struct {
  Radx::si32   month; /* Date for this ray; month (1-12). */
  Radx::si32   day;   /* Date for this ray; day (1-31).   */
  Radx::si32   year;  /* Date for this ray; year (eg. 1993). */
  Radx::si32   hour;  /* Time for this ray; hour (0-23). */
  Radx::si32   minute;/* Time for this ray; minute (0-59).*/
  Radx::fl32 sec;   /* Time for this ray; second + fraction of second. */
  Radx::fl32 unam_rng;  /* Unambiguous range. (KM). */
  Radx::fl32 azimuth;   /* Azimuth angle. (degrees). Must be positive
                    * 0=North, 90=east, -90/270=west.
                    * This angle is the mean azimuth for the whole ray.
                    * Eg. for NSIG the beginning and end azimuths are
                    * averaged.
                    */
  Radx::si32   ray_num;   /* Ray no. within elevation scan. */
  Radx::fl32 elev;       /* Elevation angle. (degrees). */
  Radx::si32   elev_num;   /* Elevation no. within volume scan. */
  
  Radx::si32   range_bin1; /* Range to first gate.(meters) */
  Radx::si32   gate_size;  /* Data gate size (meters)*/
  
  Radx::fl32  vel_res;    /* Doppler velocity resolution */
  Radx::fl32 sweep_rate;   /* Sweep rate. Full sweeps/min. */

  Radx::fl32 scale;
  Radx::fl32 bias;
  
  Radx::si32 prf;          /* Pulse repitition frequency, in Hz. */
  Radx::si32 prf2;         /* Second PRF, for Sigmet dual PRF. */
  Radx::fl32 azim_rate;  /* Sweep rate in degrees/second.*/
  Radx::fl32 fix_angle;  /* Elevation angle for the sweep. (degrees). */
  Radx::fl32 pitch;      /* Pitch angle. */
  Radx::fl32 roll;       /* Roll  angle. */
  Radx::fl32 heading;    /* Heading. */
  Radx::fl32 pitch_rate; /* (angle/sec) */
  Radx::fl32 roll_rate;  /* (angle/sec) */
  Radx::fl32 heading_rate; /* (angle/sec) */
  Radx::fl32 lat;          /* Latitude (degrees) */
  Radx::fl32 lon;          /* Longitude (degrees) */
  Radx::si32   alt;          /* Altitude (m) */
  Radx::fl32 rvc;          /* Radial velocity correction (m/sec) */
  Radx::fl32 vel_east;     /* Platform velocity to the east (negative for west)   (m/sec) */
  Radx::fl32 vel_north;    /* Platform velocity to the north (negative for south) (m/sec) */
  Radx::fl32 vel_up;       /* Platform velocity toward up (negative for down)     (m/sec) */
  Radx::fl32 pulse_count;  /* Pulses used in a single dwell time. */
  Radx::fl32 pulse_width; /* Pulse width (micro-sec). */
  Radx::fl32 beam_width;  /* Beamwidth in degrees. */
  Radx::fl32 frequency;   /* Bandwidth GHz. */
  Radx::fl32 wavelength;  /* Wavelength. Meters. */
  Radx::fl32 nyq_vel;    /* Nyquist velocity. m/s */
  //Radx::fl32 (*f)(Range x);       /* Data conversion function. f(x). */
  //Range (*invf)(Radx::fl32 x);    /* Data conversion function. invf(x). */
  Radx::si32   nbins;               /* Number of array elements for 'Range'. */
  bool binDataAllocated;
} Ray_header;

typedef struct {
  Ray_header h;
  Range *range; /* range[0..nbins-1] */
                /* For wsr88d file:
                 * 0..460 for reflectivity, 0..920 for velocity and 
                 * spectrum width. You must allocate this space.
                 */
} Ray; 

// ----------------
// Sweep

typedef struct {
  Radx::si32 sweep_num;      /* Integer sweep number. */
  Radx::fl32 elev;         /* Elevation angle (mean) for the sweep. */
  Radx::fl32 beam_width;   /* This is in the ray header too. */
  Radx::fl32 vert_half_bw; /* Vertical beam width divided by 2 */
  Radx::fl32 horz_half_bw; /* Horizontal beam width divided by 2 */
  Radx::si32 nrays;
  //Radx::fl32 (*f)(Range x); /* Data conversion function. f(x). */
  //Range (*invf)(Radx::fl32 x); /* Data conversion function. invf(x). */ 
} Sweep_header; 


typedef struct {
  Sweep_header h;
  Ray **ray; /* ray[0..nrays-1]. */
} Sweep; 

// --------------
// Volume

typedef struct {
  char *type_str;  /* One of:'Reflectivity', 'Velocity' or 'Spectrum width' */
  Radx::si32 nsweeps;
  Radx::fl32 calibr_const;        /* Calibration constant. */
  //Radx::fl32 (*f)(Range x);       /* Data conversion function. f(x). */
  //Range (*invf)(Radx::fl32 x);    /* Data conversion function. invf(x). */
} Volume_header;


typedef struct {
  Volume_header h; /* Specific info for each elev. */
                   /* Includes resolution: km/bin. */
  Sweep **sweep;   /* sweep[0..nsweeps-1]. */
} Volume; 

// ------------------
// Radar

typedef struct { 
  Radx::si32 month, day, year; 
  Radx::si32 hour, minute; 
  Radx::fl32 sec; /* Second plus fractional part. */
  char radar_type[50]; /* Type of radar. Use for QC-ing the data.
                        * Supported types are:
                        * "wsr88d", "lassen", "uf",
                        * "nsig", "nsig2", "mcgill",
                        * "kwajalein", "rsl", "toga".
                        * Set by appropriate ingest routine.
                        */ 
  Radx::si32 nvolumes;
  Radx::si32 number;        /* arbitrary number of this radar site */
  char name[8];      /* Nexrad site name */
  char radar_name[8]; /* Radar name. */
  char project[24];   /* Project assocated with data. */
  char city[15];     /* nearest city to radaar site */
  char state[2];     /* state of radar site */
  Radx::si32 latd;   /* degrees of latitude of site */
  Radx::si32 latm;   /* minutes of latitude of site */
  Radx::si32 lats;   /* seconds of latitude of site */
  Radx::si32 lond;   /* degrees of longitude of site */
  Radx::si32 lonm;   /* minutes of longitude of site */
  Radx::si32 lons;   /* seconds of longitude of site */
  Radx::si32 height; /* height of site in meters above sea level*/
  Radx::si32 spulse; /* length of short pulse (ns)*/
  Radx::si32 lpulse; /* length of long pulse (ns) */
  Radx::si32 vcp;    /* Volume Coverage Pattern (for WSR-88D only) */
} Radar_header;

typedef struct {
  Radar_header h;
  Volume **v;   /* Array 0..nvolumes-1 of poRadx::si32ers to Volumes.
                      * 0 = DZ_INDEX = reflectivity.
                      * 1 = VR_INDEX = velocity.
                      * 2 = SW_INDEX = spectrum_width.
                      * 3 = CZ_INDEX = corrected reflectivity.
                      * 4 = ZT_INDEX = total reflectivity.
                      * 5 = DR_INDEX = differential refl.
                      * 6 = LR_INDEX = another differential refl.
                      * 7 = ZD_INDEX = another refl form.
                      * 8 = DM_INDEX = recieved power.
                      * 9 = RH_INDEX = Rho coefficient.
                      *10 = PH_INDEX = Phi (MCTEX parameter).
                      *11 = XZ_INDEX = X-band reflectivity.
                      *12 = CR_INDEX = Corrected DR.
                      *13 = MZ_INDEX = DZ mask for 1C-51 HDF.
                      *14 = MR_INDEX = DR mask for 1C-51 HDF.
                      *15 = ZE_INDEX = Edited reflectivity.
                      *16 = VE_INDEX = Edited velocity.
                      *17 = KD_INDEX = KDP (unknown)  for MCTEX data.
                      *18 = TI_INDEX = TIME (unknown)  for MCTEX data.
                */
} Radar;

/*
 * DZ     Reflectivity (dBZ), may contain some   DZ_INDEX
 *        signal-processor level QC and/or      
 *        filters. This field would contain 
 *        Darwin's CZ, or WSR88D's standard 
 *        reflectivity. In other words, unless
 *        the field is described otherwise, it
 *        should always go here. In essence, this
 *        is the "cleanest" reflectivity field
 *        for a radar.
 *
 * VR     Radial Velocity (m/s)                  VR_INDEX
 *
 * SW     Spectral Width (m2/s2)                 SW_INDEX
 *
 * CZ     QC Reflectivity (dBZ), contains
 *        post-processed QC'd data               CZ_INDEX
 *
 * ZT     Total Reflectivity (dBZ)               ZT_INDEX
 *        May be uncommon, but important
 *        This is UZ in UF files.
 *
 * DR     Differential reflectivity              DR_INDEX
 *        DR and LR are for dual-polarization
 *        radars only. Unitless or in dB.
 *
 * LR     Another form of differential ref       LR_INDEX
 *        called LDR, not sure of units
 *
 * ZD     ZDR: Reflectivity Depolarization Ratio ZD_INDEX
 *        ZDR = 10log(ZH/ZV)  (dB)
 *
 * DM     Received power measured by the radar.  DM_INDEX
 *        Units are dBm.
 *
 * RH     Rho : Correlation coefficient (MCTEX)  RH_INDEX
 *
 * PH     Phi (MCTEX parameter)                  PH_INDEX
 *
 * XZ     X-band reflectivity                    XZ_INDEX
 *
 * CD     Corrected ZD reflectivity (differential) CD_INDEX
 *        contains QC'ed data
 *
 * MZ     DZ mask volume for HDF 1C-51 product.  MZ_INDEX
 *
 * MD     ZD mask volume for HDF 1C-51 product.  MD_INDEX
 *
 * ZE     Edited Reflectivity.                   ZE_INDEX
 *
 * VE     Edited Velocity.                       VE_INDEX
 *
 * KD     KDP (unknown)  for MCTEX data.         KD_INDEX
 *
 * TI     TIME (unknown)  for MCTEX data.        TI_INDEX
 */

class Rsl
{
public: 

  Rsl();
  ~Rsl();

  static Radar *new_radar(Radx::si32 nvolumes);
  static Volume *new_volume(Radx::si32 max_sweeps);
  static Sweep *new_sweep(Radx::si32 max_rays);
  static Ray *new_ray(Radx::si32 max_bins);

  static void free_radar(Radar *radar);
  static void free_volume(Volume *volume);
  static void free_sweep(Sweep *sweep);
  static void free_ray(Ray *ray);

  static Volume *copy_volume(Volume *v);
  static Sweep  *copy_sweep(Sweep *sweep);
  static Ray    *copy_ray(Ray *ray);

  static Radx::fl32 DZ_F(Range x);
  static Radx::fl32 VR_F(Range x);

  static Range DZ_INVF(Radx::fl32 x);
  static Range VR_INVF(Radx::fl32 x);

  static void print_volume(Volume *volume);
  static void print_sweep(Sweep *sweep);
  static void print_ray(Ray *ray);
  static void print_ray_header(Ray_header header);
  static void print_sweep_header(Sweep_header header);

  static void verifyEqualDimensions(Volume *currDbzVol, Volume *currVelVol);

};

#endif 
