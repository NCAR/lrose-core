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
/*********************************************************************
 *
 * acars.c
 *
 * Aircraft Position data
 *
 * Holin Tsai
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Dec 1999
 *
 *********************************************************************/

#include <stdio.h>
#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <rapformats/acars.h>

/********************************************************************
 *
 * acars_init()
 *
 * Initialize acars struct
 *
 *********************************************************************/

void acars_init(acars_t *acars)

{
  memset(acars, 0, sizeof(acars_t));
  acars->lat = ACARS_FLOAT_MISSING;
  acars->lon = ACARS_FLOAT_MISSING;
  acars->alt = ACARS_FLOAT_MISSING;
  acars->temp = ACARS_FLOAT_MISSING;
  acars->wind_speed = ACARS_FLOAT_MISSING;
  acars->wind_dirn = ACARS_FLOAT_MISSING;
  acars->accel_lateral = ACARS_FLOAT_MISSING;
  acars->accel_vertical = ACARS_FLOAT_MISSING;
  acars->fuel_remain = ACARS_FLOAT_MISSING;
}


/********************************************************************
 *
 * BE_from_acars()
 *
 * Gets BE format from ac posn struct
 *
 *********************************************************************/

void BE_from_acars(acars_t *posn)

{
  BE_from_array_32(posn, ACARS_N_FL32 * sizeof(fl32));
}

/********************************************************************
 *
 * BE_to_acars()
 *
 * Converts BE format to acars struct
 *
 *********************************************************************/

void BE_to_acars(acars_t *posn)

{
  BE_to_array_32(posn, ACARS_N_FL32 * sizeof(fl32));
}


/****************************
 * acars_print
 *
 * Prints out struct
 */

void acars_print(FILE *out,
		 const char *spacer,
		 const acars_t *acars)

{

  fprintf(out, "%sACARS\n", spacer);
  fprintf(out, "%s  flight#: %s\n", spacer, acars->flight_number);
  fprintf(out, "%s  depart_airport: %s\n", spacer, acars->depart_airport);
  fprintf(out, "%s  dest_airport: %s\n", spacer, acars->dest_airport);
  fprintf(out, "%s  time: %s\n", spacer, utimstr(acars->time));
  fprintf(out, "%s  lat: %g\n", spacer, acars->lat);
  fprintf(out, "%s  lon: %g\n", spacer, acars->lon);
  fprintf(out, "%s  alt (ft): %g\n", spacer, acars->alt);

  if (acars->temp > ACARS_FLOAT_MISSING) {
    fprintf(out, "%s  temp (C): %g\n", spacer, acars->temp);
  }
  if (acars->wind_speed > ACARS_FLOAT_MISSING) {
    fprintf(out, "%s  wind_speed (kts): %g\n", spacer, acars->wind_speed);
  }
  if (acars->wind_dirn > ACARS_FLOAT_MISSING) {
    fprintf(out, "%s  wind_dirn (deg T): %g\n", spacer, acars->wind_dirn);
  }
  if (acars->accel_lateral > ACARS_FLOAT_MISSING) {
    fprintf(out, "%s  accel_lateral (g): %g\n", spacer, acars->accel_lateral);
  }
  if (acars->accel_vertical > ACARS_FLOAT_MISSING) {
    fprintf(out, "%s  accel_vertical (g): %g\n",
            spacer, acars->accel_vertical);
  }

  if (acars->eta > 0) {
    fprintf(out, "%s  eta: %s\n", spacer, utimstr(acars->eta));
  }
  if (acars->fuel_remain > 0) {
    fprintf(out, "%s  fuel_remain : %g\n", spacer, acars->fuel_remain);
  }
}

