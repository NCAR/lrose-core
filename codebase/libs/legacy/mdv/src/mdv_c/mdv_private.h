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


#ifndef MDV_PRIVATE_H_INCLUDED
#define MDV_PRIVATE_H_INCLUDED

#include <mdv/mdv_handle.h>
#include <rapformats/dobson.h>

/******************************************************************************
 *  MDV_PRIVATE.H  Prototypes for private functions used by the MDV library.
 *  N. Rehak.  Jul 1996. RAP.
 */

/*****************************************************************
 * MDV_RECALLOC: Allocs or reallocs depending on which one is 
 * necessary.   
 * Returns pointer with appropriate space. 
 * --Rachel Ames 3/96, RAP/NCAR
 */

void * MDV_recalloc(void *ptr_to_mem, 
                    int number_in_mem, 
                    int size_of_mem);

/******************************************************************************
 * DOBSON_VOL_PARAMS_FROM_BE:  Convert the fields in Dobson volume params chunk
 * data from big endian format to native format.
 */
 
void dobson_vol_params_from_BE(vol_params_t *vol_params);

/******************************************************************************
 * DOBSON_VOL_PARAMS_TO_BE:  Convert the fields in Dobson volume params chunk
 * data from native format to big endian format.
 */
 
void dobson_vol_params_to_BE(vol_params_t *vol_params);

/******************************************************************************
 * DOBSON_ELEVATIONS_FROM_BE:  Convert the fields in Dobson elevations chunk
 * data from big endian format to native format.
 */
 
void dobson_elevations_from_BE(void *elevations, long size);

/******************************************************************************
 * DOBSON_ELEVATIONS_TO_BE:  Convert the fields in Dobson elevations chunk
 * data from native format to big endian format.
 */
 
void dobson_elevations_to_BE(void *elevations, long size);


#endif

#ifdef __cplusplus
}
#endif
