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
/************************************************************************
 *                    
 * ltg.h - Header file for the ltg module of the rapformats library.
 *
 * Nancy Rehak
 * March 1999
 *                   
 *************************************************************************/

#include <stdio.h>

#include <dataport/port_types.h>

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef LTG_H
#define LTG_H

/*
 * Constants found in the lightning files.
 */

#define LTG_TYPE_UNKNOWN     -1
#define LTG_GROUND_STROKE     0
#define LTG_CLOUD_STROKE      1
#define LTG_EXTENDED_COOKIE   0xfcfcfcfc
#define LTG_MISSING_FLOAT     -9999.0f
#define LTG_MISSING_INT       -9999

/*
 * Define the structure contained in the lightning SPDB files.
 */

typedef struct {
  si32 time;            /* UNIX time of strike */
  fl32 latitude;        /* degrees */
  fl32 longitude;       /* degrees */
  si16 amplitude;       /* kiloamps, sign indicates polarity */
  si16 type;            /* LTG_GROUND_STROKE, LTG_CLOUD_STROKE or
                         *  LTG_TYPE_UNKNOWN */
} LTG_strike_t;

typedef struct {

  ui32 cookie;            /* LTG_EXTENDED_COOKIE */
  ti32 time;              /* UNIX time of strike */
  fl32 latitude;          /* degrees */
  fl32 longitude;         /* degrees */
  fl32 altitude;          /* altitude in km MSL */
  fl32 amplitude;         /* kiloamps, sign indicates polarity */
  si32 type;              /* LTG_GROUND_STROKE, LTG_CLOUD_STROKE or
                          * LTG_TYPE_UNKNOWN */
  si32 nanosecs;          /* time - nanosec portion */\
  si32 n_sensors;         /* number of sensors used in solution */
  si32 degrees_freedom;   /* in computing location */
  fl32 ellipse_angle;     /* angle of ellipse degrees T */
  fl32 semi_major_axis;   /* of ellipse, in km */
  fl32 semi_minor_axis;   /* of ellipse, in km */
  fl32 chi_sq;            /* from location optimization */
  fl32 rise_time;         /* of waveform, us */
  fl32 peak_to_zero_time; /* of waveform, in us */
  fl32 max_rate_of_rise;  /* of waveform, in kA/us */
  si32 angle_flag;        /* 1 if angle used in solution, 0 otherwise */
  si32 signal_flag;       /* 1 if angle used in solution, 0 otherwise */
  si32 timing_flag;       /* 1 if angle used in solution, 0 otherwise */
  
  fl32 residual;
  fl32 spare[3];
  
} LTG_extended_t;


/************************************************************************
 * Function prototypes.
 ************************************************************************/

/************************************************************************
 * LTG_print_strike(): Print the strike information to the indicated
 *                     stream in ASCII format.
 */

void LTG_print_strike(FILE *stream, const LTG_strike_t *strike);
void LTG_print_extended(FILE *stream, const LTG_extended_t *strike);

/************************************************************************
 * LTG_init() - Initialize a lightning struct
 */

void LTG_init(LTG_strike_t *strike);
void LTG_init_extended(LTG_extended_t *strike);

/************************************************************************
 * LTG_from_BE() - Convert the strike information from big-endian format
 *                 to native format.
 */

void LTG_from_BE(LTG_strike_t *strike);
void LTG_extended_from_BE(LTG_extended_t *strike);

/************************************************************************
 * LTG_to_BE() - Convert the strike information from native format to
 *               big-endian format.
 */

void LTG_to_BE(LTG_strike_t *strike);
void LTG_extended_to_BE(LTG_extended_t *strike);

/************************************************************************
 * LTG_type2string() - Convert the strike type to a string for printing.
 */

char *LTG_type2string(int type);

/************************************************************************
 * TWNLTG_read_strike(): Read a Taiwan lightning strike from an input
 *                       file.  Bytes are swapped as necessary.
 *
 * Returns a pointer to a static structure (or NULL on error or EOF).
 * Do NOT free this pointer.
 */

LTG_strike_t *TWNLTG_read_strike(FILE *input_file, char *prog_name);

#endif     /* LTG_H */

#ifdef __cplusplus
}
#endif
