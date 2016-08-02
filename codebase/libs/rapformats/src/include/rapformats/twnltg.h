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
 * twnltg.h - Header file for the twnltg module of the rapformats library.
 *
 * Holin Tsai
 * March 1999
 *                   
 *************************************************************************/

#include <rapformats/ltg.h>

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef TWNLTG_H
#define TWNLTG_H

/*
 * Constants found in the Taiwan lightning files.
 */


/*
 * Define the structure contained in the Taiwan lightning files.
 */

/*
 * Define the structure used for storing the lightning data in the SPDB
 * files.
 */

typedef struct
{
  si32 time;            // UNIX time of strike
  fl32 latitude;        // degrees
  fl32 longitude;       // degrees
  si32 nothing;         // alignment
} TWNLTG_strike_t;



/************************************************************************
 * Function prototypes.
 ************************************************************************/

/************************************************************************
 * TWNLTG_print_strike_spdb(): Print the SPDB strike information to the
 *                             indicated stream in ASCII format.
 */

void TWNLTG_print_strike_spdb(FILE *stream, KAVLTG_strike_spdb_t *strike);

/************************************************************************
 * TWNLTG_read_strike(): Read a Taiwan lightning strike from an input
 *                       file. 
 *
 * Returns a pointer to a static structure (or NULL on error or EOF).
 * Do NOT free this pointer.
 */

KAVLTG_strike_spdb_t *TWNLTG_read_strike(FILE *input_file, char *prog_name);

/************************************************************************
 * TWNLTG_spdb_from_BE() - Convert the SPDB strike information from big-
 *                         endian format to native format.
 */

void TWNLTG_spdb_from_BE(KAVLTG_strike_spdb_t *strike);

/************************************************************************
 * TWNLTG_spdb_to_BE() - Convert the SPDB strike information from native
 *                       format to big-endian format.
 */

void TWNLTG_spdb_to_BE(KAVLTG_strike_spdb_t *strike);

# endif     /* TWNLTG_H */

#ifdef __cplusplus
}
#endif
