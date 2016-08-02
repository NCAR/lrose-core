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
 * gailtg.c: Routines to manipulate GAI lightning data.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2002
 *
 * Gary Blackburn
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <toolsa/os_config.h>

#include <dataport/bigend.h>
#include <dataport/port_types.h>

#include <toolsa/mem.h>

#include <rapformats/gailtg.h>
#include <rapformats/ltg.h>


/************************************************************************
 * GAILTG_print_strike(): Print the strike information to the indicated
 *                        stream in ASCII format.
 */

void GAILTG_print_strike(FILE *stream, GAILTG_strike_t *strike)
{
  time_t print_time = strike->time;
  
  fprintf(stream, "\nStrike info:\n");
  fprintf(stream, "   time = %s", ctime(&print_time));
  fprintf(stream, "   latitude = %f deg\n", (double)strike->latitude/10000.0);
  fprintf(stream, "   longitude = %f deg\n", (double)strike->longitude/10000.0);
  fprintf(stream, "   amplitude = %d\n", strike->amplitude);
  fprintf(stream, "   type = %d\n", strike->type);
  fprintf(stream, "\n");
  
  return;
}

/************************************************************************
 * GAILTG_to_spdb() - Convert the GAI lightning data to the format
 *                        used in the SPDB database.
 *
 * Returns a pointer to static memory.  This pointer should NOT be freed.
 */

LTG_strike_t *GAILTG_to_spdb(GAILTG_strike_t *strike)
{
  static LTG_strike_t strike_spdb;
 
  strike_spdb.time = strike->time;
  strike_spdb.latitude = (double)strike->latitude / 10000.0;
  strike_spdb.longitude = (double)strike->longitude / 10000.0;
  strike_spdb.amplitude = strike->amplitude;
  strike_spdb.type = LTG_TYPE_UNKNOWN;
 
  return(&strike_spdb);
}

/************************************************************************
 * GAILTG_strike_from_BE() - Convert the Kavouras strike information from
 *                           big-endian format to native format.
 */

void GAILTG_strike_from_BE(GAILTG_strike_t *strike)
{
  strike->time =      BE_to_si32(strike->time);
  strike->latitude =  BE_to_si32(strike->latitude);
  strike->longitude = BE_to_si32(strike->longitude);
  strike->amplitude = BE_to_si16(strike->amplitude);
  strike->type =      BE_to_si16(strike->type);
 
  return;
}


/************************************************************************
 * GAILTG_strike_to_BE() - Convert the GAI strike information from
 *                         native format to big-endian format.
 */

void GAILTG_strike_to_BE(GAILTG_strike_t *strike)
{
  strike->time =      BE_from_si32(strike->time);
  strike->latitude =  BE_from_si32(strike->latitude);
  strike->longitude = BE_from_si32(strike->longitude);
  strike->amplitude = BE_from_si16(strike->amplitude);
  
  return;
}
