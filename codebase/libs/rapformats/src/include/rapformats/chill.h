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
#ifdef __cplusplus
extern "C" {
#endif
  /************************************************************************
   * chill.h
   *
   * chill radar tape format structs
   *
   * From Dave Brunkow, CSU-Chill, July 1992
   *
   *************************************************************************/

#include <dataport/port_types.h>

  /*
   * conversions from counts to degrees
   */

#define CHILL_DEG_CONV 0.087890625
#define CHILL_FIXED_CONV 0.010986328

  /*
   * scan modes
   */

#define CHILL_SECTOR_MODE 0
#define CHILL_SURVEILLANCE_MODE 0
#define CHILL_RHI_MODE 1

  /*
   * numbers of shorts and bytes in chldat1
   */

#define CHLDAT1_NSHORTS0 2L
#define CHLDAT1_NSHORTS1 28L
#define CHLDAT1_NSHORTS2 8L
#define CHLDAT1_NSHORTS_LIMITED 17L
#define CHLDAT1_OFFSET_LIMITED 15L
#define CHLDAT1_PRT_POS 18L
#define CHLDAT1_SCANMODE_POS 21L
#define CHLDAT1_GATE_SPACE_POS 23L
#define CHLDAT1_BYPASS_POS 29L
#define CHLDAT1_NYQVEL_POS 35L
#define CHLDAT1_SEGNAME_POS 38L

  /*
   * scale and bias values
   */

#define CHILL_PHASE_BIAS -180.0
#define CHILL_PHASE_SCALE (360.0 / 256.0)
#define CHILL_ZDR_BIAS -3.01
#define CHILL_ZDR_SCALE (3.01 / 64.0)
#define CHILL_DBZ_BIAS -30.0
#define CHILL_DBZ_SCALE 0.5
#define CHILL_PARAMS_FACTOR 10000.0

  /*
   * housekeeping header
   * 
   * shorts are in VAX format on tape - least significant byte first
   */

  typedef struct {		/* vax format data is used for local use */
  
    char btype[2];
    si16 raylen;		/* number of 16 bit words in record (ray) */
    si16 offset1;		/* number of 16 bit hsk words which follow */
    si16 az,el;			/* pointing angle: 0-4095 */
    ui16 raynum;		/* ray number within volume */
    si16 hour,min,sec,tenths;	/* time of day (local time) */
    si16 antstat;		/* antenna status bits */
    si16 year,month,day;	/* date */
    si16 volnum;		/* volume number */
    si16 sweepnum;		/* sweep number */
    si16 azprogpos,elprogpos;	/* fixed angle */
  
    /* hsk data may be truncated at this point, or it may continue (1/26/89) */
  
    si16 azcwlim;		/* az right limit */
    si16 azccwlim;		/* az left limit */
    si16 prt;			/* pulse repetition time */
    si16 swprate;		/* scan rate deg/seg*100 */
    si16 hits;			/* # pulses per integration cycle */
    si16 scanmode;		/* scan mode code */
    si16 pulse_len;		/* in microsec*1024 */
    si16 gate_space;		/* in microsec*1024 */
    si16 txbin;			/*  bin offset to tranmit pulse */
  
    /* note: use txbin from IP or NC header if available */
  
    si16 txpwr;			/* peak transmit power dbm*256 */
    si16 maxtop;		/* max height of data recording */
    si16 el_down;		/* down limit */
    si16 el_up;			/* up limit */
    char step_opt;		/* 1-> el step optimizer on */
    char bypass;		/* 1 -> polarization switch bypassed */
    si16 opt_rmax;		/* max range *100 for step optimizer */
    si16 opt_htmax;		/* max ht *100 for step optimizer */
    si16 opt_res;		/* resolution term *1000 for opt */
    si16 filt_end;		/* number of clutter filtered gates */
    si16 filt_num;		/* filter bank select number */
    si16 nyqvel;		/* nyquist velocity m/sec*256 */
    si16 qual;			/* word reserved for quality check */
    si16 spare;			/* spare word */
    char segname[8];		/* scan segment name */
    char sp20prog[8];		/* sp20 program name */
    char polseq[8];		/* polarization sequence */
  
  } chldat1_t; 

  /* structure used in 1986 dbm only data */

#define CHLDAT2_NSHORTS 7

  typedef struct {
  
    char dtype[2];
    ui16 offset2;
    si16 thresh;
    si16 mapnum;
    ui16 maps[4];
    ui08 vid[1024];
  
  } chldat_2;


  /* structures for aircraft tracking */

  typedef struct {
  
    float x[32];
    float y[32];
    int entcnt;			/* number of points entered in buffer */
    int remcnt;			/* number of points removed from buffer */
    int latest;			/* index of most recent point in buffer */
    int oldest;			/* index of oldest data in buffer */
    int dspl;			/* index of last point displayed from buffer */
    int ident;			/* tranponder id of this aircraft */
    float alt;			/* latest altitude reported for this aircraft */
  
  } airplnstr_t;

#define AIRPLARCH_NSHORTS 4

  typedef struct {
  
    char name[8];		/* aircraft name */
    si16 x,y;			/* location in km*128 w.r.t. chill */
    si16 alt;			/* altitude in 100's of feet */
    si16 code;			/* txpond code */
  
  } airplarch_t; 

  /* structures used in 1987 and beyond ...  */

  /*
   * generic struct for overlaying on the field structs to get at
   * the gates and numhed data
   */

  typedef struct {
  
    char dtype[2];		/* "XX"  - field code */
    ui16 lrecl;			/* number of words this field */
    si16 gates;			/* number of gates recorded */
    si16 numhed;		/* number of header words */

  } chlgen_t;

  /* integrated power (IP) and normalized coherent power (NCP) fields */

#define IP_NSHORTS1 8
#define IP_NSHORTS2 1

  typedef struct {
  
    char dtype[2];		/* "IP" (0x4950) or "NC" field code */
    ui16 lrecl;			/* number of words this field */
    si16 gates;			/* number of gates recorded */
    si16 numhed;		/* number of header words */
    si16 format;		/* numeric representation code */
    si16 thresh;		/* threshold used for comp */
    si16 irb;			/* initial range bin */
    si16 token;			/* sp20 token */
    si16 maxgates;		/* max number gates out of sp20 */
    si16 txbin;			/* gate offset to tx pulse */
    char rngavg;		/* range average, 1=no avg, 2= 2 bin avg */
    char sp20_mode;		/* sp20 mode:  0-> doppler, no interleave
				 * 1-> zdr (VHHH mode)
				 * 2-> 
				 * 3->
				 * 4-> VH polarization mode  */
    si16 resol;			/* tx_time * 256 + rec resolution */

    /* both in units of 100's nanoseconds */
  
  } chlip_t;

  /* zdr fields */

#define DR_NSHORTS 7

  typedef struct {
  
    char dtype[2];		/* "DR" (0x4452) field code (zdr) */
    ui16 lrecl;			/* number of words this field */
    si16 gates;			/* number of gates recorded */
    si16 numhed;		/* number of header words */
    si16 format;		/* numeric representation code */
    si16 irb;			/* initial range bin */
    si16 offset;		/* offset added to zdr before rec */
    si16 scale;			/* zdr scale factor is 3.01/scale */

    /* if the scale word is missing, the value 64 should be used */
  
  } chldr_t;

  /* complex correlation fields */

#define COR_NSHORTS 5

  typedef struct {
  
    char dtype[2];		/* "r1" or "r2" field code */
    ui16 lrecl;			/* number of words this field */
    si16 gates;			/* number of gates recorded */
    si16 numhed;		/* number of header words */
    si16 format;		/* numeric format */
    si16 irb;			/* initial range bin */
  
  } chlcor_t;

  /* calculated mean velocity fields */

#define VEL_NSHORTS 6

  typedef struct {
  
    char dtype[2];		/* "VE" field code */
    ui16 lrecl;			/* number of words this field */
    si16 gates;			/* number of gates recorded */
    si16 numhed;		/* number of header words */
    si16 format;		/* numeric representation code */
    si16 irb;			/* initial range bin */
    si16 prt;			/* prt in microseconds */
  
  } chlvel_t;

  /* calculated spectral width estimators */

#define VAR_NSHORTS 5

  typedef struct {
  
    char dtype[2];		/* "W1 or W2" field code */
    ui16 lrecl;			/* number of words this field */
    si16 gates;			/* number of gates recorded */
    si16 numhed;		/* number of header words */
    si16 format;		/* numeric representation code */
    si16 irb;			/* initial range bin */
  
  } chlvar_t;

  /* aircraft track info from transponders */ 

#define AP_NSHORTS 1

  typedef struct {
  
    char dtype[2];		/* "AP" field code (a/c track) */
    ui16 lrecl;			/* number of words this field */
    airplarch_t plarch[3];	/* archive array */
  
  } chlap_t;

  /* time series data */

#define TS_NSHORTS 6

  typedef struct {
  
    char dtype[2];		/* "TS" field code (time series)*/
    ui16 lrecl;			/* number of words this field */
    si16 irb;			/* range to first bin (.25 usec) */
    si16 gsp;			/* time between samples (.25 usec) */
    si16 gates;			/* number of gates recorded per ray */
    si16 totsamp;		/* total number of samples per block */
    si16 nbytes;		/* sample size in bytes 4 or 8 */
  
  } chlts_t;

  /*
   * structure for reflectivity calibration terms
   */

  typedef struct {
    si32 year;
    si32 month;
    si32 day;
    double slope;		/* db's per receiver count */
    double noisepwr;		/* rec. noise power */
    double intnoise;		/* rec. counts for blue sky noise */
    double pktxpwr;		/* peak txmit power in dbm */
    double antgain;		/* antenna gain (db) */
    double radcon;		/* radar constant (db) */
    double zdrloss;		/* fast switch insertion loss */
    double zdrcal;		/* correction for zdr values (db) */
    int dsplithr;		/* remaining items for CHILL display */
    double dsplzthr,dsplzscl;
    double dsplnthr;
  } ch_calib_t;

#ifdef __cplusplus
}
#endif
