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
 * flt_path.h - Header file for the flt_path module of the rapformats library.
 *
 * Nancy Rehak
 * December 1997
 *                   
 *************************************************************************/

#include <dataport/port_types.h>
#include <stdio.h>

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef FLT_PATH_H
#define FLT_PATH_H

/*
 * Define the structures used for storing the flight path data in the SPDB
 * files.
 */

typedef struct
{
  fl32 x;
  fl32 y;
} FLTPATH_location_t;


typedef struct
{
  FLTPATH_location_t  loc;
  fl32                time;
  fl32                cost;
} FLTPATH_leg_t;

typedef struct
{
  si32          num_pts;
  FLTPATH_leg_t pts[1];
} FLTPATH_path_t;

/************************************************************************
 * Function prototypes.
 ************************************************************************/

/************************************************************************
 * FLTPATH_print_path(): Print the SPDB path information to the indicated
 *                       stream in ASCII format.
 */

void FLTPATH_print_path(FILE *stream, FLTPATH_path_t *path);

/************************************************************************
 * FLTPATH_path_from_BE() - Convert the flight path information from
 *                          big-endian format to native format.
 */

void FLTPATH_path_from_BE(FLTPATH_path_t *path);

/************************************************************************
 * FLTPATH_path_to_BE() - Convert the flight path information from
 *                        native format to big-endian format.
 */

void FLTPATH_path_to_BE(FLTPATH_path_t *path);



#endif

#ifdef __cplusplus
}
#endif
