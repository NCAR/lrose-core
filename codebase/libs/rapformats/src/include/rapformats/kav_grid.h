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

/*
 * kav_grid.h
 *
 * Parameters for kavouras grid, and packing scheme
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000
 *
 * June 1994
 */

#ifndef kav_grid_h
#define kav_grid_h

#define N_KAV_COLORS 7

/*
 * default grid parameters
 */

#define KAV_MINLON -129.0
#define KAV_MINLAT 14.23
#define KAV_NLON 2560
#define KAV_NLAT 2880
#define KAV_DLON 0.025791324
#define KAV_DLAT 0.017193469

/*
 * compression codes
 */

#define KAV_CMPR_COUNT1 13	/* max count for one-byte format */
#define KAV_CMPR_COUNT2 0xFF	/* max count for two-byte format */
#define KAV_CMPR_COUNT3 0xFFFF	/* max count for three-byte format */
#define KAV_CMPR_TWO_BYTE 14	/* code indicating two-byte format */
#define KAV_CMPR_THREE_BYTE 13	/* code indicating three-byte format */
#define KAV_CMPR_EOL    0xFE	/* code indicating end of line */
#define KAV_CMPR_EOF    0xFF	/* code indicating end of grid */

/*
 * kavouras header
 */

#ifndef NEW_DBS_HEADER
#define MAX_SITES 128
#else
#define MAX_SITES 200
#endif

#define KAV_DATA_OFFSET 2048 /* offset to data in buffer */

#endif

#ifdef __cplusplus
}
#endif
