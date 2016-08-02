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
 * gailtg.h - Header file for the gailtg module of the rapformats library.
 *
 * Gary Blackburn
 * June 2002
 *                   
 *************************************************************************/

#include <stdio.h>

#include <dataport/port_types.h>
#include <rapformats/ltg.h>


#ifdef __cplusplus
 extern "C" {
#endif

#ifndef GAILTG_H
#define GAILTG_H

/*
 * Define the structure contained in GAI lightning files.
 */

typedef struct
{
  si32 time;            /* Unix time */
  si32 latitude;        /* degrees * 1000 */
  si32 longitude;       /* degrees * 1000 */
  si16 amplitude;       /* kiloamps */
  si16 type;            /* always unknown */
} GAILTG_strike_t;

/************************************************************************
 * Function prototypes.
 ************************************************************************/

/************************************************************************
 * GAILTG_to_spdb() - Convert the GAI lightning data to the
 *                        used in the SPDB database.
 *
 * Returns a pointer to static memory.  This pointer should NOT be freed.
 */

LTG_strike_t *GAILTG_to_spdb(GAILTG_strike_t *strike);

/************************************************************************
 * GAILTG_print_strike(): Print the strike information to the indicated
 *                        stream in ASCII format.
 */

void GAILTG_print_strike(FILE *stream, GAILTG_strike_t *strike);

/************************************************************************
 * GAILTG_strike_from_BE() - Convert GAI strike information from
 *                           big-endian format to native format.
 */

void GAILTG_strike_from_BE(GAILTG_strike_t *strike);

/************************************************************************
 * GAILTG_strike_to_BE() - Convert the GAI strike information from
 *                         native format to big-endian format.
 */

void GAILTG_strike_to_BE(GAILTG_strike_t *strike);


# endif     /* GAILTG_H */

#ifdef __cplusplus
}
#endif
