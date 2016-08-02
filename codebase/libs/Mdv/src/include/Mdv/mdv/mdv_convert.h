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
 * MDV_CONVERT.H : Prototypes and defines for routines for converting MDV
 *                 data between the supported formats.
 *
 * N. Rehak, Feb 1997. NCAR, RAP.
 */

#ifndef MDV_CONVERT_H
#define MDV_CONVERT_H

#include <dataport/port_types.h>
#include <Mdv/mdv/mdv_file.h>

/******************************************************************************
 * MDV_CONVERT_VOLUME: Allocate space for a data volume (data for all levels)
 * and convert the given volume of data into the specified format. Caller is 
 * responsible for freeing up the returned buffer when done.
 *
 * Inputs: input_volume - pointer to the input volume of data.
 *         input_volume_size - number of bytes in the input volume.
 *         nx - grid size in the x direction.
 *         ny - grid size in the y direction.
 *         nz - grid size in the z direction.
 *         input_volume_format - the format of the input data.
 *         return_volume_format - the desired format for the returned data.
 *
 * Outputs: return_volume_size - number of bytes in returned volume.
 *
 * Returns: returns a pointer to the converted data volume information, or
 *          NULL if there is an error.  The space for the returned data volume
 *          is allocated by this routine and must be freed by the calling
 *          routine.
 */

ui08 *MDV_convert_volume(ui08 *input_volume,
			 int input_volume_size,
			 int nx,
			 int ny,
			 int nz,
			 int input_volume_format,
			 int return_volume_format,
			 int *return_volume_size);

/******************************************************************************
 * MDV_CONVERT_VOLUME_SCALED: Allocate space for a data volume (data for
 * all levels) and convert the given volume of data into the specified format,
 * calculating scale and bias to scale the data into the new format.  The
 * given field header is updated to reflect the updated volume.  Caller is 
 * responsible for freeing up the returned buffer when done.
 *
 * Inputs: input_volume - pointer to the input volume of data.
 *         return_volume_format - the desired format for the returned data.
 *         field_hdr - field header for the data.  Should match the input
 *                     volume on entry.
 *
 * Outputs: field_hdr -   Will be updated to match the output volume on exit.
 *
 * Returns: returns a pointer to the converted data volume information, or
 *          NULL if there is an error.  The space for the returned data volume
 *          is allocated by this routine and must be freed by the calling
 *          routine.
 *
 * Note: This routine will currently only convert MDV_FLOAT32 volume data
 *       to MDV_INT8 format.
 */

ui08 *MDV_convert_volume_scaled(ui08 *input_volume,
				int return_volume_format,
				MDV_field_header_t *field_hdr);


#endif

#ifdef __cplusplus
}
#endif
