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
#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 * MDV_composite.h : Prototypes and defines for routines for compositing MDV
 *                   data.
 *
 * N. Rehak, Nov 1998. NCAR, RAP.
 */

#ifndef MDV_composite_H
#define MDV_composite_H

#include <Mdv/mdv/mdv_handle.h>

/******************************************************************************
 * MDV_composite_data(): Composites the given MDV data using the given
 *                       composition technique.
 *
 * Inputs: mdv_handle - handle for the MDV data to be composited.
 *         composite_type - composition technique to use
 *                            (e.g. MDV_COMPOSITE_MAX).
 *
 * Outputs: mdv_handle - data is updated to contain composite rather
 *                         than original data.
 *
 * Returns: MDV_SUCCESS on success, MDV_FAILURE on failure.
 */

int MDV_composite_data(MDV_handle_t *mdv_handle,
		       int composite_type);


#endif

#ifdef __cplusplus
}
#endif
