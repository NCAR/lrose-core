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
 * ltg.c: Routines to manipulate RAP lightning data.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <toolsa/os_config.h>
#include <dataport/bigend.h>
#include <dataport/port_types.h>
#include <rapformats/ltg.h>
#include <toolsa/mem.h>
#include <toolsa/umisc.h>


/************************************************************************
 * LTG_print_strike(): Print the strike information to the indicated
 *                     stream in ASCII format.
 */

void LTG_print_strike(FILE *stream, const LTG_strike_t *strike)
{
  fprintf(stream, "\nStrike info:\n");
  if (strike->time != LTG_MISSING_INT) {
    fprintf(stream, "   time = %s\n", utimstr(strike->time));
  }
  if (strike->latitude != LTG_MISSING_FLOAT) {
    fprintf(stream, "   latitude = %g deg\n", strike->latitude);
  }
  if (strike->longitude != LTG_MISSING_FLOAT) {
    fprintf(stream, "   longitude = %g deg\n", strike->longitude);
  }
  if (strike->amplitude != LTG_MISSING_INT) {
    fprintf(stream, "   amplitude = %d kA\n", strike->amplitude);
  }
  if (strike->type != LTG_MISSING_INT) {
    fprintf(stream, "   type = %s\n", LTG_type2string(strike->type));
  }
  fprintf(stream, "\n");
}

void LTG_print_extended(FILE *stream, const LTG_extended_t *strike)
{
  fprintf(stream, "\nStrike info - extended version:\n");
  if (strike->time != LTG_MISSING_INT) {
    fprintf(stream, "   time = %s\n", utimstr(strike->time));
  }
  if (strike->latitude != LTG_MISSING_FLOAT) {
    fprintf(stream, "   latitude = %g deg\n", strike->latitude);
  }
  if (strike->longitude != LTG_MISSING_FLOAT) {
    fprintf(stream, "   longitude = %g deg\n", strike->longitude);
  }
  if (strike->altitude != LTG_MISSING_FLOAT) {
    fprintf(stream, "   altitude = %g deg\n", strike->altitude);
  }
  if (strike->amplitude != LTG_MISSING_FLOAT) {
    fprintf(stream, "   amplitude = %g kA\n", strike->amplitude);
  }
  if (strike->type != LTG_MISSING_INT) {
    fprintf(stream, "   type = %s\n", LTG_type2string(strike->type));
  }
  if (strike->nanosecs != LTG_MISSING_INT) {
    fprintf(stream, "   nanosecs = %d\n", strike->nanosecs);
  }
  if (strike->n_sensors != LTG_MISSING_INT) {
    fprintf(stream, "   n_sensors = %d\n", strike->n_sensors);
  }
  if (strike->degrees_freedom != LTG_MISSING_INT) {
    fprintf(stream, "   degrees_of_freedom = %d\n", strike->degrees_freedom);
  }
  if (strike->ellipse_angle != LTG_MISSING_FLOAT) {
    fprintf(stream, "   ellipse_angle = %g\n", strike->ellipse_angle);
  }
  if (strike->semi_major_axis != LTG_MISSING_FLOAT) {
    fprintf(stream, "   semi_major_axis = %g\n", strike->semi_major_axis);
  }
  if (strike->semi_minor_axis != LTG_MISSING_FLOAT) {
    fprintf(stream, "   semi_minor_axis = %g\n", strike->semi_minor_axis);
  }
  if (strike->chi_sq != LTG_MISSING_FLOAT) {
    fprintf(stream, "   chi_sq = %g\n", strike->chi_sq);
  }
  if (strike->rise_time != LTG_MISSING_FLOAT) {
    fprintf(stream, "   rise_time = %g\n", strike->rise_time);
  }
  if (strike->peak_to_zero_time != LTG_MISSING_FLOAT) {
    fprintf(stream, "   peak_to_zero_time = %g\n", strike->peak_to_zero_time);
  }
  if (strike->max_rate_of_rise != LTG_MISSING_FLOAT) {
    fprintf(stream, "   max_rate_of_rise = %g\n", strike->max_rate_of_rise);
  }
  if (strike->residual != LTG_MISSING_FLOAT) {
    fprintf(stream, "   residual = %g\n", strike->residual);
  }
  if (strike->angle_flag != LTG_MISSING_INT) {
    fprintf(stream, "   angle_flag = %s\n",
            strike->angle_flag? "TRUE" : "FALSE");
  }
  if (strike->signal_flag != LTG_MISSING_INT) {
    fprintf(stream, "   signal_flag = %s\n",\
            strike->signal_flag? "TRUE" : "FALSE");
  }
  if (strike->timing_flag != LTG_MISSING_INT) {
    fprintf(stream, "   timing_flag = %s\n",
            strike->timing_flag? "TRUE" : "FALSE");
  }
  fprintf(stream, "\n");
}

/************************************************************************
 * LTG_init() - Initialize a lightning struct
 */

void LTG_init(LTG_strike_t *strike)
{
  MEM_zero(*strike);
  strike->time = LTG_MISSING_INT;
  strike->latitude = LTG_MISSING_FLOAT;
  strike->longitude = LTG_MISSING_FLOAT;
  strike->amplitude = LTG_MISSING_INT;
  strike->type = LTG_MISSING_INT;
}

void LTG_init_extended(LTG_extended_t *strike)
{
  MEM_zero(*strike);
  strike->cookie = LTG_EXTENDED_COOKIE;
  strike->time = LTG_MISSING_INT;
  strike->latitude = LTG_MISSING_FLOAT;
  strike->longitude = LTG_MISSING_FLOAT;
  strike->altitude = LTG_MISSING_FLOAT;
  strike->amplitude = LTG_MISSING_FLOAT;
  strike->type = LTG_MISSING_INT;
  strike->nanosecs = LTG_MISSING_INT;
  strike->n_sensors = LTG_MISSING_INT;
  strike->degrees_freedom = LTG_MISSING_INT;
  strike->ellipse_angle = LTG_MISSING_FLOAT;
  strike->semi_major_axis = LTG_MISSING_FLOAT;
  strike->semi_minor_axis = LTG_MISSING_FLOAT;
  strike->chi_sq = LTG_MISSING_FLOAT;
  strike->rise_time = LTG_MISSING_FLOAT;
  strike->peak_to_zero_time = LTG_MISSING_FLOAT;
  strike->max_rate_of_rise = LTG_MISSING_FLOAT;
  strike->residual = LTG_MISSING_FLOAT;
  strike->angle_flag = LTG_MISSING_INT;
  strike->signal_flag = LTG_MISSING_INT;
  strike->timing_flag = LTG_MISSING_INT;
}

/************************************************************************
 * LTG_from_BE() - Convert the strike information from big-endian format
 *                 to native format.
 */

void LTG_from_BE(LTG_strike_t *strike)
{
  strike->time =      BE_to_si32(strike->time);
  BE_from_array_32(&strike->latitude, 4);
  BE_from_array_32(&strike->longitude, 4);
  strike->amplitude = BE_to_si16(strike->amplitude);
  strike->type =      BE_to_si16(strike->type);
}

void LTG_extended_from_BE(LTG_extended_t *strike)
{
  BE_from_array_32(strike, sizeof(LTG_extended_t));
}

/************************************************************************
 * LTG_to_BE() - Convert the strike information from native format to
 *               big-endian format.
 */

void LTG_to_BE(LTG_strike_t *strike)
{
  strike->time =      BE_from_si32(strike->time);
  BE_from_array_32(&strike->latitude, 4);
  BE_from_array_32(&strike->longitude, 4);
  strike->amplitude = BE_from_si16(strike->amplitude);
  strike->type =      BE_from_si16(strike->type);
}

void LTG_extended_to_BE(LTG_extended_t *strike)
{
  strike->cookie = LTG_EXTENDED_COOKIE;
  BE_from_array_32(strike, sizeof(LTG_extended_t));
}

/************************************************************************
 * LTG_type2string() - Convert the strike type to a string for printing.
 */

char *LTG_type2string(int type)
{
  switch(type)
  {
  case LTG_GROUND_STROKE :
    return "LTG_GROUND_STROKE";
    
  case LTG_CLOUD_STROKE :
    return "LTG_CLOUD_STROKE";
    
  case LTG_TYPE_UNKNOWN :
    return "LTG_TYPE_UNKNOWN";
  }
  
  return "*** Invalid lightning type ***";
}
