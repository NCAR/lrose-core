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
 * pirep.c
 *
 * Aircraft Position data
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * May 1999
 *
 *********************************************************************/

#include <stdio.h>
#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <rapformats/pirep.h>

/********************************************************************
 *
 * pirep_init()
 *
 * Initialize pirep struct
 *
 *********************************************************************/

void pirep_init(pirep_t *pirep)

{
  memset(pirep, 0, sizeof(pirep_t));
  pirep->lat = PIREP_FLOAT_MISSING;
  pirep->lon = PIREP_FLOAT_MISSING;
  pirep->alt = PIREP_FLOAT_MISSING;
  pirep->temp = PIREP_FLOAT_MISSING;
  pirep->visibility = PIREP_FLOAT_MISSING;
  pirep->wind_speed = PIREP_FLOAT_MISSING;
  pirep->wind_dirn = PIREP_FLOAT_MISSING;

  pirep->turb_fl_base = PIREP_INT_MISSING;
  pirep->turb_fl_top = PIREP_INT_MISSING;
  pirep->icing_fl_base = PIREP_INT_MISSING;
  pirep->icing_fl_top = PIREP_INT_MISSING;
  pirep->sky_fl_base = PIREP_INT_MISSING;
  pirep->sky_fl_top = PIREP_INT_MISSING;

  pirep->turb_freq = PIREP_INT_MISSING;
  pirep->turb_index = PIREP_INT_MISSING;
  pirep->icing_index = PIREP_INT_MISSING;
  pirep->sky_index = PIREP_INT_MISSING;

}


/********************************************************************
 *
 * BE_from_pirep()
 *
 *********************************************************************/

void BE_from_pirep(pirep_t *pirep)

{

  BE_from_array_32(&pirep->time, sizeof(ti32));

  BE_from_array_32(&pirep->lat, sizeof(fl32));
  BE_from_array_32(&pirep->lon, sizeof(fl32));
  BE_from_array_32(&pirep->alt, sizeof(fl32));

  BE_from_array_32(&pirep->temp, sizeof(fl32));
  BE_from_array_32(&pirep->visibility, sizeof(fl32));
  BE_from_array_32(&pirep->wind_speed, sizeof(fl32));
  BE_from_array_32(&pirep->wind_dirn, sizeof(fl32));

  BE_from_array_16(&pirep->turb_fl_base, sizeof(si16));
  BE_from_array_16(&pirep->turb_fl_top, sizeof(si16));
  BE_from_array_16(&pirep->icing_fl_base, sizeof(si16));
  BE_from_array_16(&pirep->icing_fl_top, sizeof(si16));
  BE_from_array_16(&pirep->sky_fl_base, sizeof(si16));
  BE_from_array_16(&pirep->sky_fl_top, sizeof(si16));

}

/********************************************************************
 *
 * BE_to_pirep()
 *
 *********************************************************************/

void BE_to_pirep(pirep_t *pirep)

{
  BE_to_array_32(&pirep->time, sizeof(ti32));

  BE_to_array_32(&pirep->lat, sizeof(fl32));
  BE_to_array_32(&pirep->lon, sizeof(fl32));
  BE_to_array_32(&pirep->alt, sizeof(fl32));

  BE_to_array_32(&pirep->temp, sizeof(fl32));
  BE_to_array_32(&pirep->visibility, sizeof(fl32));
  BE_to_array_32(&pirep->wind_speed, sizeof(fl32));
  BE_to_array_32(&pirep->wind_dirn, sizeof(fl32));

  BE_to_array_16(&pirep->turb_fl_base, sizeof(si16));
  BE_to_array_16(&pirep->turb_fl_top, sizeof(si16));
  BE_to_array_16(&pirep->icing_fl_base, sizeof(si16));
  BE_to_array_16(&pirep->icing_fl_top, sizeof(si16));
  BE_to_array_16(&pirep->sky_fl_base, sizeof(si16));
  BE_to_array_16(&pirep->sky_fl_top, sizeof(si16));

}


/****************************
 * pirep_print
 *
 * Prints out struct
 */

void pirep_print(FILE *out,
		 const char *spacer,
		 const pirep_t *pirep)

{

  fprintf(out, "%sPIREP\n", spacer);
  fprintf(out, "%s  callsign: %s\n", spacer, pirep->callsign);
  fprintf(out, "%s  time: %s\n", spacer, utimstr(pirep->time));
  fprintf(out, "%s  lat: %g\n", spacer, pirep->lat);
  fprintf(out, "%s  lon: %g\n", spacer, pirep->lon);
  fprintf(out, "%s  alt (ft): %g\n", spacer, pirep->alt);

  if (strlen(pirep->text) > 0) {
    fprintf(out, "%s  text: %s\n", spacer, pirep->text);
  }
  
  if (pirep->turb_freq != PIREP_INT_MISSING) {
    fprintf(out, "%s  turb_freq: %d\n", spacer, pirep->turb_freq);
  }

  switch (pirep->turb_index) {
  case PIREP_TURB_NONE:
    fprintf(out, "%s  turb: none\n", spacer);
    break;
  case PIREP_TURB_TRACE:
    fprintf(out, "%s  turb: trace\n", spacer);
    break;
  case PIREP_TURB_LIGHT:
    fprintf(out, "%s  turb: light\n", spacer);
    break;
  case PIREP_TURB_LIGHT_MODERATE:
    fprintf(out, "%s  turb: light-moderate\n", spacer);
    break;
  case PIREP_TURB_MODERATE:
    fprintf(out, "%s  turb: moderate\n", spacer);
    break;
  case PIREP_TURB_MODERATE_SEVERE:
    fprintf(out, "%s  turb: moderate-severe\n", spacer);
    break;
  case PIREP_TURB_SEVERE:
    fprintf(out, "%s  turb: severe\n", spacer);
    break;
  case PIREP_TURB_SEVERE_EXTREME:
    fprintf(out, "%s  turb: severe-extreme\n", spacer);
    break;
  case PIREP_TURB_EXTREME:
    fprintf(out, "%s  turb: extreme\n", spacer);
    break;
  default: {}
  }
    
  if (pirep->turb_fl_base != PIREP_INT_MISSING) {
    fprintf(out, "%s  turb_fl_base: %d\n", spacer, pirep->turb_fl_base);
  }
  if (pirep->turb_fl_top != PIREP_INT_MISSING) {
    fprintf(out, "%s  turb_fl_top: %d\n", spacer, pirep->turb_fl_top);
  }

  switch (pirep->icing_index) {
  case PIREP_ICING_NONE:
    fprintf(out, "%s  icing: none\n", spacer);
    break;
  case PIREP_ICING_TRACE:
    fprintf(out, "%s  icing: trace\n", spacer);
    break;
  case PIREP_ICING_LIGHT:
    fprintf(out, "%s  icing: light\n", spacer);
    break;
  case PIREP_ICING_MODERATE:
    fprintf(out, "%s  icing: moderate\n", spacer);
    break;
  case PIREP_ICING_SEVERE:
    fprintf(out, "%s  icing: severe\n", spacer);
    break;
  default: {}
  }
    
  if (pirep->icing_fl_base != PIREP_INT_MISSING) {
    fprintf(out, "%s  icing_fl_base: %d\n", spacer, pirep->icing_fl_base);
  }
  if (pirep->icing_fl_top != PIREP_INT_MISSING) {
    fprintf(out, "%s  icing_fl_top: %d\n", spacer, pirep->icing_fl_top);
  }

  switch (pirep->sky_index) {
  case PIREP_SKY_CLEAR:
    fprintf(out, "%s  sky: clear\n", spacer);
    break;
  case PIREP_SKY_SCATTERED:
    fprintf(out, "%s  sky: scattered\n", spacer);
    break;
  case PIREP_SKY_BROKEN:
    fprintf(out, "%s  sky: broken\n", spacer);
    break;
  case PIREP_SKY_OVERCAST:
    fprintf(out, "%s  sky: overcast\n", spacer);
    break;
  case PIREP_SKY_OBSCURED:
    fprintf(out, "%s  sky: obscured\n", spacer);
    break;
  default: {}
  }
    
  if (pirep->sky_fl_base != PIREP_INT_MISSING) {
    fprintf(out, "%s  sky_fl_base: %d\n", spacer, pirep->sky_fl_base);
  }
  if (pirep->sky_fl_top != PIREP_INT_MISSING) {
    fprintf(out, "%s  sky_fl_top: %d\n", spacer, pirep->sky_fl_top);
  }

  if (pirep->temp > PIREP_FLOAT_MISSING) {
    fprintf(out, "%s  temp (C): %g\n", spacer, pirep->temp);
  }
  if (pirep->visibility > PIREP_FLOAT_MISSING) {
    fprintf(out, "%s  visibility (sm): %g\n", spacer, pirep->visibility);
  }
  if (pirep->wind_speed > PIREP_FLOAT_MISSING) {
    fprintf(out, "%s  wind_speed (kts): %g\n", spacer, pirep->wind_speed);
  }
  if (pirep->wind_dirn > PIREP_FLOAT_MISSING) {
    fprintf(out, "%s  wind_dirn (deg T): %g\n", spacer, pirep->wind_dirn);
  }

}

