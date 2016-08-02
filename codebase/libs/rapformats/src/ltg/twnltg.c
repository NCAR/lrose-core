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
 *   $Id: twnltg.c,v 1.6 2016/03/03 18:45:40 dixon Exp $
 *   $Revision: 1.6 $
 *   $State: Exp $
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * taiwanltg.c: Routines to manipulate Taiwan lightning data.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1999
 *
 * Holin Tsai
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <toolsa/os_config.h>

#include <dataport/bigend.h>
#include <dataport/port_types.h>

#include <toolsa/mem.h>

#include <rapformats/ltg.h>

/************************************************************************
 * TWNLTG_print_strike_spdb(): Print the SPDB strike information to the
 *                             indicated stream in ASCII format.
 */

void TWNLTG_print_strike_spdb(FILE *stream, LTG_strike_t *strike)
{
  time_t print_time = strike->time;
  
  fprintf(stream, "\nSPDB strike info:\n");
  fprintf(stream, "   time = %s", ctime(&print_time));
  fprintf(stream, "   latitude = %f deg\n", (double)strike->latitude);
  fprintf(stream, "   longitude = %f deg\n", (double)strike->longitude);
  fprintf(stream, "\n");
  
  return;
}


/************************************************************************
 * TWNLTG_spdb_from_BE() - Convert the SPDB strike information from big-
 *                         endian format to native format.
 */

void TWNLTG_spdb_from_BE(LTG_strike_t *strike)
{
  strike->time =      BE_to_si32(strike->time);
  BE_to_array_32(&strike->latitude, 4);
  BE_to_array_32(&strike->longitude, 4);
  
  return;
}


/************************************************************************
 * TWNLTG_spdb_to_BE() - Convert the SPDB strike information from native
 *                       format to big-endian format.
 */

void TWNLTG_spdb_to_BE(LTG_strike_t *strike)
{
  strike->time =      BE_from_si32(strike->time);
  BE_from_array_32(&strike->latitude, 4);
  BE_from_array_32(&strike->longitude, 4);
  
  return;
}
