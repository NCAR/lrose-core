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
 * kavltg.h - Header file for the kavltg module of the rapformats library.
 *
 * Nancy Rehak
 * Janurary 1997
 *                   
 *************************************************************************/

#include <stdio.h>

#include <dataport/port_types.h>
#include <rapformats/ltg.h>


#ifdef __cplusplus
 extern "C" {
#endif

#ifndef KAVLTG_H
#define KAVLTG_H

/*
 * Constants found in the Kavouras lightning files.
 */

#define KAVLTG_GROUND_STROKE     0
#define KAVLTG_CLOUD_STROKE      1


/*
 * Define the structure contained in the Kavouras lightning files.
 */

typedef struct
{
  si32 time;            /* UNIX time of strike */
  si32 latitude;        /* degrees * 1000 */
  si32 longitude;       /* degrees * 1000 */
  si16 amplitude;       /* kiloamps */
  si16 type;            /* KAVLTG_GROUND_STROKE or KAVLTG_CLOUD_STROKE */
} KAVLTG_strike_t;

/************************************************************************
 * Function prototypes.
 ************************************************************************/

/************************************************************************
 * KAVLTG_kav_to_spdb() - Convert the Kavouras lightning data to the
 *                        used in the SPDB database.
 *
 * Returns a pointer to static memory.  This pointer should NOT be freed.
 */

LTG_strike_t *KAVLTG_kav_to_spdb(KAVLTG_strike_t *strike_kav);

/************************************************************************
 * KAVLTG_print_strike(): Print the strike information to the indicated
 *                        stream in ASCII format.
 */

void KAVLTG_print_strike(FILE *stream, KAVLTG_strike_t *strike);

/************************************************************************
 * KAVLTG_read_strike(): Read a Kavouras lightning strike from an input
 *                       file.  Bytes are swapped as necessary.
 *
 * Returns a pointer to a static structure (or NULL on error or EOF).
 * Do NOT free this pointer.
 */

KAVLTG_strike_t *KAVLTG_read_strike(FILE *input_file);

/************************************************************************
 * KAVLTG_strike_from_BE() - Convert the Kavouras strike information from
 *                           big-endian format to native format.
 */

void KAVLTG_strike_from_BE(KAVLTG_strike_t *strike);

/************************************************************************
 * KAVLTG_type2spdb() - Convert the strike type found in a Kavouras
 *                      data file into the type value used in an
 *                      SPDB database.
 */

int KAVLTG_type2spdb(int kav_type);

/************************************************************************
 * KAVLTG_strike_to_BE() - Convert the Kavouras strike information from
 *                         native format to big-endian format.
 */

void KAVLTG_strike_to_BE(KAVLTG_strike_t *strike);


# endif     /* KAVLTG_H */

#ifdef __cplusplus
}
#endif
