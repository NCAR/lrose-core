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
/**************************************************************************
 * titan_hail.h
 *
 * header file for hail metrics in Titan
 *
 * Terri L. Betancourt
 *
 * RAP NCAR Boulder CO USA
 *
 * October 2001
 *
 **************************************************************************/

#ifndef titan_hail_h
#define titan_hail_h

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
 * NOTE: This struct becomes part of a union in titan's storm properties struct
 *       storm_file_global_props_t.  The union only allows us 16 bytes,
 *       so we MUST MAKE SURE that we fit within that space!
 *       Although AbshaevCategory only needs one byte of storage
 *       (categories 0-4), sticking with 32bit values makes the byte
 *       swapping more straight forward.
 **************************************************************************/
typedef struct {

  si32 FOKRcategory;            /*  category 0-4 */
  fl32 waldvogelProbability;    /* 0 <= probability <= 1.0 */
  fl32 hailMassAloft;           /* ktons */
  fl32 vihm;                    /* kg/m2 (from maxz) */

} titan_hail_t;

/**************************************************************************
 * Results of NEXRAD Hail Detection Algorithm (HDA)
 * Reference:
 *   Arthur Witt, Michael D. Eilts, Gregory J. Stumph, J. T. Johnson,
 *   E DeWayne Mitchell and Kevin W Thomas:
 *   An Enhanced Hail Detection Algorithm for the WSR-88D.
 *   Weather and Forecasting, Volume 13, June 1998.
 **************************************************************************/

typedef struct {

  fl32 poh; /* Waldvogel Probability (%) */
  fl32 shi; /* Severe Hail Index (J.m-1.s-1) */
  fl32 posh; /* probability of severe hail (%) */
  fl32 mehs; /* Maximum Expected Hail Size (mm) */
  
} nexrad_hda_t;


#ifdef __cplusplus
}
#endif

#endif
