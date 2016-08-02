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
/**********************************************************************
 * pirep.h
 *
 * structs and defines for PIREPS - pilot reports
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO
 *
 * May 1999
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef pirep_h
#define pirep_h

#include <stdio.h>
#include <dataport/port_types.h>

#define PIREP_TURB_NONE             0
#define PIREP_TURB_TRACE            1
#define PIREP_TURB_LIGHT            2
#define PIREP_TURB_LIGHT_MODERATE   3
#define PIREP_TURB_MODERATE         4
#define PIREP_TURB_MODERATE_SEVERE  5
#define PIREP_TURB_SEVERE           6
#define PIREP_TURB_SEVERE_EXTREME   7
#define PIREP_TURB_EXTREME          8

#define PIREP_ICING_NONE     0
#define PIREP_ICING_TRACE    1
#define PIREP_ICING_LIGHT    2
#define PIREP_ICING_MODERATE 3
#define PIREP_ICING_SEVERE   4

#define PIREP_SKY_CLEAR     0
#define PIREP_SKY_SCATTERED 1
#define PIREP_SKY_BROKEN    2
#define PIREP_SKY_OVERCAST  3
#define PIREP_SKY_OBSCURED  4

#define PIREP_INT_MISSING -9
#define PIREP_FLOAT_MISSING ((float) -999.0)

#define PIREP_CALLSIGN_LEN 16
#define PIREP_TEXT_LEN     64

typedef struct {

  ti32 time; /* unix time */
  fl32 lat; /* deg */
  fl32 lon; /* deg */
  fl32 alt; /* ft */
  
  si32 spare1[7];

  fl32 temp; /* deg C */
  fl32 visibility; /* miles */
  fl32 wind_speed; /* knots */
  fl32 wind_dirn;  /* deg T */

  fl32 spare2[3];

  si16 spare3[2];
  si16 turb_fl_base;  /* flight level = ft / 100 */
  si16 turb_fl_top;   /* flight level = ft / 100 */
  si16 icing_fl_base; /* flight level = ft / 100 */
  si16 icing_fl_top;  /* flight level = ft / 100 */
  si16 sky_fl_base;   /* flight level = ft / 100 */
  si16 sky_fl_top;    /* flight level = ft / 100 */

  si08 spare4[4];
  si08 turb_freq;
  si08 turb_index;  /* 0 through 8 */
  si08 icing_index;
  si08 sky_index;

  char callsign[PIREP_CALLSIGN_LEN];
  char text[PIREP_TEXT_LEN];

} pirep_t;

/*
 * prototypes
 */

extern void BE_from_pirep(pirep_t *pirep);

extern void BE_to_pirep(pirep_t *pirep);

extern void pirep_init(pirep_t *pirep);

extern void pirep_print(FILE *out,
			const char *spacer,
			const pirep_t *pirep);


#endif

#ifdef __cplusplus
}
#endif
