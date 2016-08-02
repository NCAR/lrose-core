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

  /**********************************************************************
   * lass.h
   *
   * data structures for the lassen labs radar data
   *
   * First record is lassen_summary and occurs only once for a volume
   * The summary contains the magic word time of data ,volume summary 
   * and radar info.
   * The volume Summary contains the 
   * volume number,number of sweeps,gate width,prf etc.
   * The radar info contains radar info like name lat and long etc.
   * The sweep record contains the azimuth,elevation etc. followed 
   * by the field data.The field data is stored consecutively for
   * each gate.
   * New sweep is indicated when the sweep index changes.
   *
   * Sanjiv RAP, NCAR, Boulder, CO, USA
   *
   * December 1994
   *
   **********************************************************************/

#ifndef _lass_h
#define _lass_h

#include <dataport/port_types.h>
#include <sys/types.h>

#define LASS_TRUE 1
#define LASS_FALSE 0

#define MAGICDATA "SUNRISE"
#define MAXDATASIZE 80


  struct lass_time_structure {
    si32 year;
    si32 month;
    si32 day;
    si32 hour;
    si32 minute;
    si32 second;
  };

  struct lass_radar_info {	/* 32 bytes */
    char radar_name[8];		/* 8 char name of radar */
    char site_name[8];		/* 8 char name of site */
    si32 antenna_height;	/* meters above sea level */
    si32 latdegree;
    si32 latminute;
    si32 latsecond;
    si32 londegree;
    si32 lonminute;
    si32 lonsecond;
  };

  /* version number */
#define LASS_RAW_VERSION 13
#define LASS_NO_HEAD 99

  /* for vol->filled */
#define LASS_VOL_EMPTY -1
#define LASS_VOL_FILLING 0
#define LASS_VOL_FULL 1

  /* constants */
#define LASS_MAX_ELV 30
#define LASS_MAX_BINS 1024

  /* offset numbers */
#define LASS_NUMOFFSETS 5
#define LASS_OFF_UZ 0
#define LASS_OFF_CZ 1
#define LASS_OFF_VEL 2
#define LASS_OFF_WID 3
#define LASS_OFF_ZDR 4
#define LASS_NUMSPARES (10-LASS_NUMOFFSETS)

  /* for sweep_type variable */
#define LASS_SWEEP_POINT 0 
#define LASS_SWEEP_PPI 1 
#define LASS_SWEEP_RHI 2 
#define LASS_SWEEP_SEC 3 

  /* locations */
#define LASS_VOL_ON_SUN 0xa5a5
#define LASS_VOL_ON_FORCE 0x5c5c

  /*
   * Ray Header Structure:
   *
   * This structure contains information regarding a specific ray. The
   * sweep_index normally contains 360 pointers to these headers. The
   * actual data for the ray is not contained within the structure, but
   * resides in memory after the end of the structure. The moment offsets
   * represent the physical location, in bytes, of a particular moment's
   * data, relative to the START of the ray header; i.e. if the intensity
   * moment's offset was 400, then the intensity data would start at the
   * 400th byte after the start of the ray header. The moment data may be
   * in any order, and has a length in bytes equal to numgates. An offset
   * of zero for a moment represents a lack of data for that moment.
   *
   * As noted before, the moments may be in any order, but this order must
   * be followed uniformly though each sweep. This is extremely important.
   */
  typedef si32 USHORTLONG;
  typedef si32 SHORTLONG ;
  typedef si32 UCHARLONG;
  typedef si32 UINTLONG;

  struct lass_ray_header {
    USHORTLONG vangle,		/* variable angle */
    fanglet,			/* target fixed angle */
    fanglea;			/* actual fixed angle */

    USHORTLONG a_start,		/* variable angle start */
    a_stop;			/* variable angle stop */


    UCHARLONG max_height,	/* maximum height, km */
    volume,			/* volume serial number */
    sweep,			/* sweep index 1 -> SMAX */
    sweep_type;			/* sweep type code */

    USHORTLONG gatewid,		/* gate width, meters */
    rangeg1,			/* range to gate 1, meters */
    numgates;			/* number of gates */

    USHORTLONG prf,		/* primary prf, hz */
    prflow,			/* secondary prf, hz */
    n_pulses;			/* sample size in pulses */

    UCHARLONG p_width,		/* pulse width, .05 us units */
    cfilter,			/* clutter filter code */
    status;			/* hardware status word */


    struct {			/* software flags */
      unsigned packed : 1;	/* is the data packed? */
      unsigned good_data : 1;	/* is the data good? */
      unsigned uz : 1;		/* uz required */
      unsigned cz : 1;		/* cz required */
      unsigned vel : 1;		/* vel required */
      unsigned wid : 1;		/* wid required */
      unsigned zdr : 1;		/* zdr required */
      unsigned time : 1;	/* time series required*/
#ifdef CXREF
      unsigned spares : 8;
#else
      unsigned spares : 24;
#endif
    } flags;


    /*
     * The offsets are in an array in order to make
     * products much easier to write. i.e. instead of
     * using off_uz to get the uncorrected dbZ and
     * off_wid to get the spectral width, you can
     * use offset[OFF_UZ] and offset[OFF_WID]
     * respectively. This gains the flexibility to
     * be able to use the same code with different
     * array offsets to do such things as ppi's without
     * code for each moment.
     */
    USHORTLONG offset[LASS_NUMOFFSETS],
    spares[LASS_NUMSPARES];	/* spare 16-bit int's */

    UCHARLONG year,		/* last two digits of year */
    month,			/* month 1-12 */
    day,			/* day 1-31 */
    hour,			/* hour 0-23 */
    minute,			/* minute 0-59 */
    second;			/* second 0-59 */
  };

  /*
   * Sweep Index Structure:
   *
   * This structure contains information for an entire sweep (beam) of up to
   * 360 rays. The volume_summary structure may have up to 30 pointers to
   * sweep_index's. Each sweep_index has up to 360 pointers to ray_headers. 
   * The number of rays in a given sweep is stored in numrays.
   *
   * The offsets for moment data are duplicated in this structure so that
   * a program which needs this information does not have to go down to
   * the ray level to get it. The information is copied from the ray_header
   * at the array position 0. (which will always have data if there is any
   * good data at all in the structure)
   */
  struct lass_sweep_index {
    USHORTLONG volume,		/* volume serial number */
    sweep,			/* sweep index 1 -> SMAX */
    sweep_type,			/* sweep type code */
    max_height;			/* maximum height, km */


    USHORTLONG fangle,		/* fixed angle */
    min_var,			/* 'leftmost' variable angle */
    max_var,			/* 'rightmost' variable angle */
    a_start,			/* variable angle start */
    a_stop;			/* variable angle stop */
    
    USHORTLONG gatewid,		/* gate width, meters */
    rangeg1,			/* range to gate 1, meters */
    numgates,			/* number of gates */
    numrays;			/* number of rays this sweep */
    
    USHORTLONG prf,		/* primary prf, hz */
    prflow,			/* secondary prf, hz */
    n_pulses,			/* sample size in pulses */
    p_width,			/* pulse width, .05 us units */
    cfilter;			/* clutter filter code */
    
    USHORTLONG offset[LASS_NUMOFFSETS],
    spares[LASS_NUMSPARES],	/* spare 16-bit int's */
    dummy;
    
    UCHARLONG year,		/* last two digits of year */
    month,			/* month 1-12 */
    day,			/* day 1-31 */
    shour,			/* start hour 0-23 */
    sminute,			/* start minute 0-59 */
    ssecond,			/* start second 0-59 */
    ehour,			/* end hour 0-23 */
    eminute,		        /* end minute 0-59 */
    esecond,		        /* end second 0-59 */
    spareb[3];			/* spare */
    
    USHORTLONG status,		/* status word */
    filler;
    
    struct lass_ray_header *ray[360]; /* pointers to data ray headers */
  };

  /*
   * Volume Summary Structure:
   *
   * This is the structure which contains information for an entire volume.
   * A volume may contain up to 30 sweeps. The number of sweeps present
   * in a volume is stored in numsweeps. The pointers to sweep_index's
   * are always stored from array position 0 to array position n.
   *
   * The offsets for moment data are duplicated in this structure so that
   * a program which needs this information does not have to go down to
   * the ray level to get it. This information is copied for each ray
   * from the sweep_index structure.
   */

  struct lass_volume_summary {

    USHORTLONG version;		/* raw version number */
    SHORTLONG filled;		/* <0=empty 0=filling >0=full */

    UINTLONG volume;		/* volume serial number */
    USHORTLONG sweep,		/* sweep index 1 -> SMAX */
    sweep_type,			/* sweep type code */
    max_height;			/* maximum height, km */

    USHORTLONG status;		/* status word */

    USHORTLONG min_fangle,	/* minimum fixed angle */
    max_fangle,			/* maximum fixed angle */
    min_var,			/* minimum variable angle */
    max_var,			/* maximum variable angle */
    a_start,			/* variable angle start */
    a_stop,			/* variable angle stop */
    numsweeps,			/* number of sweeps in volume */

    fangles[30];		/* fixed angles for each sweep */

    USHORTLONG gatewid,		/* gate width, meters */
    rangeg1,			/* range to gate 1, meters */

    numgates[30],		/* gates for each sweep */
    maxgates,			/* max # of gates in volume */
    dummy;
    USHORTLONG p_width,
    cfiller,
    local,
    prf,			/* primary prf, hz */
    prflow,			/* secondary prf, hz */
    freq,
    n_pulses;			/* sample size in pulses */
    /* radex will not auto-delete */

    USHORTLONG offset[30][LASS_NUMOFFSETS],
    spares[30][LASS_NUMSPARES];	/* spare 16-bit int's */
  
    UCHARLONG year,		/* last two digits of year */
    month,			/* month 1-12 */
    day,			/* day 1-31 */
    shour,			/* start hour 0-23 */
    sminute,			/* start minute 0-59 */
    ssecond,			/* start second 0-59 */
    ehour,			/* end hour 0-23 */
    eminute,			/* end minute 0-59 */
    esecond;			/* end second 0-59 */
  
    struct {			/* software status flags */
      unsigned compress : 1;
#ifdef CXREF
      unsigned spares : 15;
#else
      unsigned spares : 31;
#endif
    } volflags;
  
  };

  /*
   * number of fields
   */

#define LASS_NFIELDS 2
#define LASS_MAXGATES 700

  /*
   * factor by which scale and bias values have been multiplied
   * before storage
   */

#define LASS_SCALE_AND_BIAS_MULT 1

  /*
   * scan modes
   */

#define LASS_SURVEILLANCE_MODE LASS_SWEEP_PPI 
#define LASS_RHI_MODE LASS_SWEEP_RHI 
#define LASS_SECTOR_MODE LASS_SWEEP_SEC 

  /*
   * record types
   */

#define LASS_SUMMARY_REC 1
#define LASS_VOLUME_REC 2
#define LASS_BEAM_REC 3 
#define MAGIC_DATASIZE 8

  /*
   * lass parameters for summary rec
   */

  typedef struct {

    char magic_data[MAGIC_DATASIZE+1];
    struct lass_time_structure sTime;
    struct lass_time_structure fTime;
    struct lass_volume_summary volSummary;
    struct lass_radar_info radinfo;
    double myAzymuth;
    double myElevation;
    double targetAngle;
    si32 sweep;
    si32 rangeTo1stGate;
    struct lass_ray_header rays;
    double biasFactor[LASS_NFIELDS+1];
    double scaleFactor[LASS_NFIELDS+1];
    ui08 fieldValues[LASS_NFIELDS*LASS_MAXGATES+1];

  } lass_params_t;

  /*
   * lass parameters for beam data 
   */

  typedef struct {
    struct lass_ray_header rays;
  } lass_beam_params_t;


  /* field 0 is dbz and field 1 will be vel */

  typedef struct {

    /* these fields will always be in a specific order */

    double biasFactor[LASS_NFIELDS+1];
    double scaleFactor[LASS_NFIELDS+1];
    ui08 fieldValues[LASS_NFIELDS*LASS_MAXGATES+1];

  } lass_field_params_t;

#endif
#ifdef __cplusplus
}
#endif
