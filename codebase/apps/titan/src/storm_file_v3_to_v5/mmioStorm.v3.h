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
/*************************************************************************
 *
 * mmioStorm.v3.h
 *
 * Memory-mapped IO version of RfStorm.c routines. These routines
 * are intended for read-only purposes, and will only work on a machine
 * which does not require byte-swapping relative to XDR.
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Jan 1996
 *
 **************************************************************************/

#include <titan/radar.h>
#include "dix_storm.v3.h"

extern void mmiov3CloseStormData(storm_v3_file_index_t *s_handle,
				 char *calling_routine);

extern int mmiov3CloseStormFiles(storm_v3_file_index_t *s_handle,
				 char *calling_routine);

extern void mmiov3CloseStormHeader(storm_v3_file_index_t *s_handle,
				   char *calling_routine);

extern int mmiov3OpenStormFiles(storm_v3_file_index_t *s_handle,
				char *mode,
				char *header_file_path,
				char *data_file_ext,
				char *calling_routine);

extern int mmiov3ReadStormHeader(storm_v3_file_index_t *s_handle,
				 char *calling_routine);

extern int mmiov3ReadStormProps(storm_v3_file_index_t *s_handle,
				si32 storm_num,
				char *calling_routine);

extern int mmiov3ReadStormScan(storm_v3_file_index_t *s_handle,
			       si32 scan_num,
			       char *calling_routine);
