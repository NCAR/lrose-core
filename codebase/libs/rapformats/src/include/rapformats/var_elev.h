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

Header: var_elev.h

Author: Dave ALbo

Date:	Thu Jun 11 17:37:47 1998

Description:	Header file for variable elevation chunk data

*************************************************************************/

# ifndef    VAR_ELEV_H
# define    VAR_ELEV_H

#ifdef __cplusplus
 extern "C" {
#endif

/* System include files / Local include files */
#include <stdio.h>

/* Constant definitions / Macro definitions / Type definitions */

#define VAR_ELEV_BAD_ELEV -99.99


/* External global variables / Non-static global variables / Static globals */


/* External functions / Internal global functions / Internal static functions */
/*
 * Print the var_elev stuff passed in as an MDV chunk
 */
extern void VAR_ELEV_print(FILE *outfile, void *data, int size);

/*
 * convert input var_elev data from BE into local format.
 */
extern void VAR_ELEV_variable_elev_from_BE(void *data, int size);

/*
 * convert input var_elev data from local into BE.
 */
extern void VAR_ELEV_variable_elev_to_BE(void *data, int size);

/*
 * Return var_elev record size in bytes associated with input nazimuth
 */
extern int VAR_ELEV_record_size(int nazimuth);

/*
 * Build and return the var_elev data associated with the inputs
 * Return the length of the data in len
 *
 * The user must free the space by calling VAR_ELEV_destroy
 */
extern void *VAR_ELEV_build(float *elevations, int nazimuth, int *len);

/*
 * Free chunk data allocated by call to VAR_ELEV_build
 */
extern void VAR_ELEV_destroy(void **var_elev);

/*
 * Return the number of elevations in the input var_elev data
 */
extern int VAR_ELEV_num_elevations(void *var_elev);

/*
 * Return the elevation associated with the input azimuth index
 * as found in the var_elev data passed in.
 */
extern float VAR_ELEV_get_elevation(void *var_elev, int index);

#ifdef __cplusplus
}
#endif

# endif     /* VAR_ELEV_H */
