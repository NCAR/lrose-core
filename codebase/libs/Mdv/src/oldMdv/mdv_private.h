/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1992 - 2010 */
/* ** University Corporation for Atmospheric Research(UCAR) */
/* ** National Center for Atmospheric Research(NCAR) */
/* ** Research Applications Laboratory(RAL) */
/* ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
/* ** 2010/10/7 23:12:31 */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef MDV_PRIVATE_H_INCLUDED
#define MDV_PRIVATE_H_INCLUDED

#include <Mdv/mdv/mdv_handle.h>
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
