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
 * bdry_extrap.h - Header file for the extrapolation portion of the
 *                 BDRY module.
 *
 * The boundary module contains routines for extrapolating boundaries
 * in the BDRY format using the method used in colide.
 *
 * Nancy Rehak
 * March 1998
 *                   
 *************************************************************************/

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef bdry_extrap_h
#define bdry_extrap_h


#include <sys/time.h>

#include <rapformats/bdry.h>


/************************************************************************
 * BDRY_extrapolate(): Extrapolates the given boundary using simple
 *                     extrapolation of each point in the boundary using
 *                     the boundary motion direction and vector.
 *
 * Note: This routines updates the detections in place with the
 *       resulting extrapolation locations.
 */

void BDRY_extrapolate(BDRY_product_t *detections,
		      int num_detections,
		      int extrap_secs);

/************************************************************************
 * BDRY_extrap_pt_motion(): Extrapolates the given boundary using by
 *                          extrapolating each point in the boundary
 *                          based on the motion vector at that point
 *                          in the boundary.
 *
 * Note: This routines updates the detections in place with the
 *       resulting extrapolation locations.
 */

void BDRY_extrap_pt_motion(BDRY_product_t *detections,
		           int num_detections,
		           int extrap_secs);
#endif

#ifdef __cplusplus
}
#endif

