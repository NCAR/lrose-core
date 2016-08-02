#include <stdio.h>
#include "rapic_routines.h"
#include <string.h>

#include <netinet/in.h>
void rapic_decode(unsigned char *inbuf, int inbytes, unsigned char *outbuf, int *outbytes,
				  float *azim, float *elev, int *delta_time)
{
  /* Decode RLE rapic buffer.
   *
   * Output to 'outbuf' and indicate the size w/ 'nout'.
   */

  /* There is some risidual parsing to do:
   *
 * 	@		<data>			*
 * 	<data>		AAA.A,EEE.E,TTT=LEN16D1D1D1NLD1D1D1NLNL
 * 	
 * 	AAA.A		Azimuth in degrees
 * 	EEE.E		Elevation in degress
 * 	TTT		Delta time in seconds since the start of this scan
 * 	LEN16		2 byte long length of radial
 * 	
 * 	D1		Run length coded Data.
 * 	NL		Null The Next Byte is count of NULL data.
 * 	NLNL		End of this Radial
 * 
 *         eg.		There will be no white space in the actual radial data.
 * 				
 * 			@066.1,010.6,004=002082B2817F8C84830048D72D0038
 * 	                                 999C0036202D35FD2C00238A99008AFE920000
 * 			Azimuth = 66.1
 * 			Elev    = 10.6
 * 			Dt	= 4 sec since the start
 * 			0020	= Bytes to follow
 * 			Data    = 82,B2,81,7F,8C,84,83
 * 			0048	= 48H null bytes
 * 			Data    = D7,2D
 * 			0038    = 38H null bytes
 * 			Data	= 99,9C
 * 			0036	= 36H Null bytes
 * 			........................
 * 			0000	= End of Data.
 * 	In  versions before 10.1 			
 * 	@		<data>			
 * 	<data>		AAALEN16D1D1D1NLD1D1D1NLNL
 * 	
 * 	AAA		Azimuth in degrees
 * 	LEN16		2 byte long length of radial
 * 	
 * 	D1		Run length coded Data.
 * 	NL		Null The Next Byte is count of NULL data.
 * 	NLNL		End of this Radial
 * 
 *         eg.		There will be no white space in the actual radial data.
 * 				
 * 			@066002082B2817F8C84830048D72D0038
 * 	                    999C0036202D35FD2C00238A99008AFE920000
 * 			Azimuth = 66
 * 			0020	= Bytes to follow
 * 			Data    = 82,B2,81,7F,8C,84,83
 * 			0048	= 48H null bytes
 * 			Data    = D7,2D
 * 			0038    = 38H null bytes
 * 			Data	= 99,9C
 * 			0036	= 36H Null bytes
 * 			........................
 * 			0000	= End of Data.
 *
 */


  /* The parser won't give us a line w/o '@' at the begining nor '\0\0'
   * at the end.  So we can be sure of that.
   */

  int i;
  char prefix[16];
  unsigned short nnulls;
  short i16;
  
  /* Find the '=' and start RLE decode from there. */
  *outbytes = 0;
  memset(prefix, '\0', sizeof(prefix));
  memcpy(prefix, &inbuf[1], 15);
  
  sscanf(prefix, "%f,%f,%d", azim, elev, delta_time);
  /*  fprintf(stderr, "AZIM=%f, ELEV=%f, TTT=%d\n", *azim, *elev, *delta_time); */

  /* Now, decode RLE. Don't care about 17,18 (they are the RLE buf size) */
  memcpy(&i16, &inbuf[17], 2);
  i16 = ntohs(i16);
  /*  fprintf(stderr, "Expecting %d bins\n", (int)i16); */
  i = 19;
  while (i<inbytes-2) {
	/*	fprintf(stderr, "i=%d byte=%2.2x(next %2.2x%2.2x) outbytes=%d\n", i, (unsigned char)inbuf[i], (unsigned char)inbuf[i+1], (unsigned char)inbuf[i+2], *outbytes); */
	if (inbuf[i] == '\0') { /* Next byte is a count of NULL's */
	  i++;
	  nnulls = (int)inbuf[i];
	  /* fprintf(stderr, "NULL .. number of nulls=%4.4x\n", nnulls); */
	  memset(&outbuf[*outbytes], '\0', (int)nnulls);
	  *outbytes += nnulls;
	} else {
	  /* fprintf(stderr, "Data\n"); */
	  /* Data. */
	  outbuf[*outbytes] = inbuf[i];
	  *outbytes += 1;
	}
	i++;
  }

  /* fprintf(stderr, "Decoded RLE: len=%d buf=<", *outbytes); */
  /* binprint(outbuf, *outbytes); */
  /* fprintf(stderr, ">\n"); */
}

	

/*---------------------------------------------------------*/
/*                                                         */
/*                    binprint                             */
/*                                                         */
/*---------------------------------------------------------*/
void binprint(char *s, int n)
{
int i;

for (i=0; i<n; i++)
	 fprintf(stderr,"%c", s[i]);
	 
}

/* Shut the damn linker up for librsl.so on Linux. This reference is
 * in the HDF library; don't need it, don't care. 
 */
void i_len(void)
{
}


#include <time.h>
void rapic_fix_time (Ray *ray)
{
  struct tm the_time;
  float fsec;
  /* Fixes possible overflow values in month, day, year, hh, mm, ss */
  /* Normally, ss should be the overflow.  This code ensures end of 
   * month, year and century are handled correctly by using the Unix
   * time functions.
   */
  if (ray == NULL) return;
  memset(&the_time, 0, sizeof(struct tm));
  the_time.tm_sec = ray->h.sec;
  fsec = ray->h.sec - the_time.tm_sec;
  the_time.tm_min  = ray->h.minute;
  the_time.tm_hour = ray->h.hour;
  the_time.tm_mon  = ray->h.month - 1;
  the_time.tm_year = ray->h.year - 1900;
  the_time.tm_mday = ray->h.day;
  the_time.tm_isdst = -1;
  (void) mktime(&the_time);
  /* The time is fixed. */
  ray->h.sec    = the_time.tm_sec;
  ray->h.sec   += fsec;
  ray->h.minute = the_time.tm_min;
  ray->h.hour   = the_time.tm_hour;
  ray->h.month  = the_time.tm_mon + 1;
  ray->h.year   = the_time.tm_year + 1900;
  ray->h.day    = the_time.tm_mday;
  return;
}

void rapic_load_ray_header(Rapic_sweep_header rh, int iray, int isweep, float elev, float azim, Ray_header *h)
{
  sscanf(rh.yyyymoddhhmmss,"%4d%2d%2d%2d%2d%2f",
		 &h->year,&h->month,&h->day,
		 &h->hour, &h->minute, &h->sec);

  h->unam_rng = rh.end_range/1000.0;  /* Unambiguous range. (KM). */
  h->azimuth  = azim;   /* Azimuth angle. (degrees). Must be positive
					* 0=North, 90=east, -90/270=west.
                    * This angle is the mean azimuth for the whole ray.
					* Eg. for NSIG the beginning and end azimuths are
					*     averaged.
					*/
  h->ray_num  = iray;     /* Ray no. within elevation scan. */
  h->elev     = rh.elev;  /* Elevation angle. (degrees). */
  h->elev_num = isweep;   /* Elevation no. within volume scan. */
  
  h->range_bin1 = rh.start_range;       /* Range to first gate.(meters) */
  h->gate_size  = rh.range_resolution;  /* Data gate size (meters)*/
  
  h->vel_res    = rh.range_resolution;   /* Doppler velocity resolution */
  h->sweep_rate = rh.anglerate/6.0;      /* Sweep rate. Full sweeps/min. */
  
  h->prf       = rh.prf;          /* Pulse repetition frequency, in Hz. */
  h->azim_rate = rh.anglerate;    /* degrees/sec */
  h->fix_angle = elev;
  h->pitch     = 0;       /* Pitch angle. */
  h->roll      = 0;       /* Roll  angle. */
  h->heading   = 0;       /* Heading. */
  h->pitch_rate   = 0;            /* (angle/sec) */
  h->roll_rate    = 0;            /* (angle/sec) */
  h->heading_rate = 0;            /* (angle/sec) */
  h->lat          = rh.lat;       /* Latitude (degrees) */
  h->lon          = rh.lon;       /* Longitude (degrees) */
  h->alt          = rh.height;    /* Altitude (m) */
  h->rvc          = 0;            /* Radial velocity correction (m/sec) */
  h->vel_east     = 0;            /* Platform velocity to the east  (m/sec) */
  h->vel_north    = 0;            /* Platform velocity to the north (m/sec) */
  h->vel_up       = 0;            /* Platform velocity toward up    (m/sec) */
  h->pulse_count  = 0;            /* Pulses used in a single dwell time. */
  h->pulse_width  = rh.pulselen;  /* Pulse width (micro-sec). */
  h->beam_width   = rh.angle_resolution; /* Beamwidth in degrees. */
  h->frequency    = rh.freq/1000.0; /* Carrier freq. GHz. */
  h->wavelength   = 0;            /* Wavelength. Meters. */
  h->nyq_vel      = rh.nyquist;   /* Nyquist velocity. m/s */

  return;
}

extern float rapic_nyquist;

float RAPIC_DZ_F(unsigned char x) {
  if (x == 0) return NOECHO;
  return (((float)x-64)/2.0);  /* rapic -> float */
}

float RAPIC_VR_F(unsigned char x) {
  if (x == 0) return BADVAL;
  return (((float)((int)x-128))/128.0*rapic_nyquist);  /* rapic -> float */
}

float RAPIC_SW_F(unsigned char x) {
  if (x == 0) return NOECHO;
  return (((float)x)/256.0*rapic_nyquist);  /* rapic -> float */
}

float RAPIC_ZT_F(unsigned char x) {
  return RAPIC_DZ_F(x);
}

float RAPIC_ZD_F(unsigned char x) {
  if (x == 0) return NOECHO;
  return (((float)x-128)/16.0);  /* rapic -> float */
}

/* USE RSL INDEXING! */
static  float (*RAPIC_f_list[])(unsigned char x) = {RAPIC_DZ_F,
													RAPIC_VR_F,
													RAPIC_SW_F,
													NULL,
													RAPIC_ZT_F,
													NULL,
													NULL,
													RAPIC_ZD_F};


void rapic_load_ray_data(unsigned char *buf, int bufsize, int ifield, Ray *ray)
{
  /* ifield is the RSL numbering scheme for field types.  The conversion
   * is done in rapic.y.  In other words, we've already converted rapic
   * index to RSL indexes.
   */
  int i;
  for (i=0; i<bufsize; i++) {
	ray->range[i] = ray->h.invf(RAPIC_f_list[ifield](buf[i]));
	/* 	fprintf(stderr,"i=%d ifield=%d, buf[%d]=%2.2x, ray->range[%d]=%4.4x value=%f\n", i,ifield,i,buf[i],i,ray->range[i], RAPIC_f_list[ifield](buf[i]) ); */
  }
  ray->h.nbins = bufsize;
}

Radar *fill_header(Radar *radar)
{
  /* Learn as much as possible from the Ray headers.  Place this
   * information into radar->h.xxxxxx
   */
  Ray *ray;
  Volume *volume;
  int i;
  float tmp;
  
  volume = NULL;
  if (radar == NULL) return NULL;
  for (i=0; i<radar->h.nvolumes && !(volume = radar->v[i]); i++)
	;
  if (volume == NULL) return NULL;

  ray = RSL_get_first_ray_of_volume(volume);
  if (ray  == NULL) return NULL;

  radar->h.month = ray->h.month;
  radar->h.day   = ray->h.day;
  radar->h.year  = ray->h.year;
  radar->h.hour  = ray->h.hour;
  radar->h.minute= ray->h.minute;
  radar->h.sec   = ray->h.sec; /* Second plus fractional part. */
  sprintf(radar->h.radar_type, "rapic"); /* Type of radar. */
		  /* nvolumes   is already filled in YACC. */
		  /* number     is already filled in YACC. */
		  /* name       is already filled in YACC. */
		  /* radar_name is already filled in YACC. */
  /*  radar->h.city[15];     */ /* Not available from RAPIC. */
  /*  radar->h.state[3];     */ /* Not available from RAPIC. */
  /*  radar->h.country[15];  */ /* Not available from RAPIC. */

   /** Latitude deg, min, sec **/
  radar->h.latd = (int)ray->h.lat;
  tmp = (ray->h.lat - radar->h.latd) * 60.0;
  radar->h.latm = (int)tmp;
  radar->h.lats = (int)((tmp - radar->h.latm) * 60.0);
   /** Longitude deg, min, sec **/
  radar->h.lond = (int)ray->h.lon;
  tmp = (ray->h.lon - radar->h.lond) * 60.0;
  radar->h.lonm = (int)tmp;
  radar->h.lons = (int)((tmp - radar->h.lonm) * 60.0);

  radar->h.height = ray->h.alt; /* height of site in meters above sea level*/
  radar->h.spulse = 0; /* length of short pulse (ns)*/
  radar->h.lpulse = 0; /* length of long pulse (ns) */

  return radar;
}
