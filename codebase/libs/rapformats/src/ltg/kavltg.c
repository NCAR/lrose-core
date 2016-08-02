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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* SCCS info
 *   %W% %D% %T%
 *   %F% %E% %U%
 *
 * RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 18:45:40 $
 *   $Id: kavltg.c,v 1.4 2016/03/03 18:45:40 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * kavltg.c: Routines to manipulate Kavouras lightning data.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1997
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

#include <toolsa/mem.h>

#include <rapformats/kavltg.h>
#include <rapformats/ltg.h>


/************************************************************************
 * KAVLTG_read_strike(): Read a Kavouras lightning strike from an input
 *                       file.  Bytes are swapped as necessary.
 *
 * Returns a pointer to a static structure (or NULL on error or EOF).
 * Do NOT free this pointer.
 */

KAVLTG_strike_t *KAVLTG_read_strike(FILE *input_file)
{
  static KAVLTG_strike_t strike_info;
  
  int bytes_read = fread((char *)&strike_info, 1,
			 sizeof(KAVLTG_strike_t), input_file);
  
  if (bytes_read == 0 || strike_info.time == 0)
    return(NULL);
  
  if (bytes_read == sizeof(KAVLTG_strike_t))
  {
    /*
     * Kavouras lightning files are in big-endian format.  Convert it
     * to native format before returning.
     */

    KAVLTG_strike_from_BE(&strike_info);
    
    return(&strike_info);
  }
  
  fprintf(stderr,
	  "Error reading data from file:  expected %ld bytes, got %d bytes.\n",
	  (long)sizeof(KAVLTG_strike_t), bytes_read);
  return(NULL);
}


/************************************************************************
 * KAVLTG_print_strike(): Print the strike information to the indicated
 *                        stream in ASCII format.
 */

void KAVLTG_print_strike(FILE *stream, KAVLTG_strike_t *strike)
{
  time_t print_time = strike->time;
  
  fprintf(stream, "\nStrike info:\n");
  fprintf(stream, "   time = %s", ctime(&print_time));
  fprintf(stream, "   latitude = %f deg\n", (double)strike->latitude/1000.0);
  fprintf(stream, "   longitude = %f deg\n", (double)strike->longitude/1000.0);
  fprintf(stream, "   amplitude = %d\n", strike->amplitude);
  fprintf(stream, "   type = %d\n", strike->type);
  fprintf(stream, "\n");
  
  return;
}


/************************************************************************
 * KAVLTG_kav_to_spdb() - Convert the Kavouras lightning data to the
 *                        used in the SPDB database.
 *
 * Returns a pointer to static memory.  This pointer should NOT be freed.
 */

LTG_strike_t *KAVLTG_kav_to_spdb(KAVLTG_strike_t *strike_kav)
{
  static LTG_strike_t strike_spdb;
  
  strike_spdb.time = strike_kav->time;
  strike_spdb.latitude = (double)strike_kav->latitude / 1000.0;
  strike_spdb.longitude = (double)strike_kav->longitude / 1000.0;
  strike_spdb.amplitude = strike_kav->amplitude;
  strike_spdb.type = KAVLTG_type2spdb(strike_kav->type);
  
  return(&strike_spdb);
}


/************************************************************************
 * KAVLTG_strike_from_BE() - Convert the Kavouras strike information from
 *                           big-endian format to native format.
 */

void KAVLTG_strike_from_BE(KAVLTG_strike_t *strike)
{
  strike->time =      BE_to_si32(strike->time);
  strike->latitude =  BE_to_si32(strike->latitude);
  strike->longitude = BE_to_si32(strike->longitude);
  strike->amplitude = BE_to_si16(strike->amplitude);
  strike->type =      BE_to_si16(strike->type);
  
  return;
}


/************************************************************************
 * KAVLTG_type2spdb() - Convert the strike type found in a Kavouras
 *                      data file into the type value used in an
 *                      SPDB database.
 */

int KAVLTG_type2spdb(int kav_type)
{
  switch(kav_type)
  {
  case KAVLTG_GROUND_STROKE :
    return LTG_GROUND_STROKE;
    
  case KAVLTG_CLOUD_STROKE :
    return LTG_CLOUD_STROKE;
  }
  
  return LTG_TYPE_UNKNOWN;
}


/************************************************************************
 * KAVLTG_strike_to_BE() - Convert the Kavouras strike information from
 *                         native format to big-endian format.
 */

void KAVLTG_strike_to_BE(KAVLTG_strike_t *strike)
{
  strike->time =      BE_from_si32(strike->time);
  strike->latitude =  BE_from_si32(strike->latitude);
  strike->longitude = BE_from_si32(strike->longitude);
  strike->amplitude = BE_from_si16(strike->amplitude);
  strike->type =      BE_from_si16(strike->type);
  
  return;
}
