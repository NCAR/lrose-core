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
 * acars.h
 *
 * structs and defines for ACARS - pilot reports
 *
 * Holin Tsai, RAP, NCAR, Boulder, CO
 *
 * Nov 1999
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef acars_h
#define acars_h

#include <dataport/port_types.h>
#include <stdio.h>

#define ACARS_FLOAT_MISSING ((float) -999.0)

#define ACARS_FLIGHTNO_LEN 16
#define ACARS_TEXT_LEN     8

#define ACARS_N_FL32 20

typedef struct {

  ti32 time; /* unix time */
  fl32 lat; /* deg */
  fl32 lon; /* deg */
  fl32 alt; /* ft */
  
  fl32 spare[13];

  fl32 temp; /* deg C */
  fl32 wind_speed; /* knots */
  fl32 wind_dirn;  /* deg T */
  fl32 accel_lateral;	/* g */
  fl32 accel_vertical;	/* g */

  ti32 eta;	/* estimated Time of arrival */
  fl32 fuel_remain;

  char flight_number[ACARS_FLIGHTNO_LEN];
  char depart_airport[ACARS_TEXT_LEN];
  char dest_airport[ACARS_TEXT_LEN];

} acars_t;

/*
 * prototypes
 */

extern void acars_init(acars_t *acars);

extern void BE_from_acars(acars_t *posn);

extern void BE_to_acars(acars_t *posn);

extern void acars_print(FILE *out,
			const char *spacer,
			const acars_t *acars);


#endif

#ifdef __cplusplus
}
#endif
