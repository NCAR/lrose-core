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

/******************************************************************************
 *  MDV_DATASET.C
 *
 *  Routines for using the MDV_dataset_t.
 *
 *  F. Hage.  Dec 1993. RAP, R. Ames 6/96.
 *
 *  Divided into mdv_user.c, mdv_read.c and mdv_write.c.
 *  N. Rehak, Aug. 1996.
 *
 *  Moved all routines into this file.
 *  Mike Dixon, August 1999.
 * 
 */

#ifndef MDV_DATASET_H
#define MDV_DATASET_H

#include <toolsa/os_config.h>
#include <Mdv/mdv/mdv_file.h>

/******************************************************************************
 * MDV_INIT_DATASET: Initializes an MDV_dataset_t struct.  No memory is freed
 * by this routine.  If the dataset contains memory which needs to be freed,
 * call MDV_free_dataset.
 *
 * Inputs: dsp - pointer to dataset to be initialized.
 *
 * Outputs: dsp - pointers are set to NULL and values are initialized to 0.
 *                No memory is freed by this routine.
 */

void MDV_init_dataset( MDV_dataset_t *dsp);

/******************************************************************************
 * MDV_FREE_DATASET: Frees space for an MDV_dataset.  If the datafile_buf
 * pointer is NULL, the dataset is assumed to have been constructed by the
 * calling routine and each header/data pointer is freed separately and the
 * pointers are set to NULL.  If the datafile_buf pointer is set, the dataset
 * is assumed to have been read directly from disk and the datafile_buf
 * pointer is freed and all of the other pointers are set to NULL.
 *
 * Inputs: dataset - pointer to dataset to be freed.
 *
 * Outputs: dataset - pointers are freed and set to NULL.  The MDV_dataset_t
 *                    pointer is NOT freed by this routine.
 */
 
void MDV_free_dataset( MDV_dataset_t *dataset);
 
/******************************************************************************
 * MDV_GET_DATASET: Allocate space for an entire dataset. Read the headers
 * and set the pointers to beginning of field and chunk data.
 * Caller is responsible for freeing up the buffer when done. 
 * Caller is responsible for making sure the MDV_dataset_t is correct and 
 * proper for the open file.
 *
 * Inputs: infile_name - name of the input file.
 *         dsp - pointer to the dataset used for storing the information in
 *               memory.  If the dataset is orignally empty, it must be
 *               initialized before calling this routine.
 *
 * Outputs: dsp - updated to contain the dataset information from the input
 *                file, byte swapped if necessary.  Memory is allocated or
 *                reallocated as needed.  Note that chunk data will only be
 *                byte swapped if it is of a type known to the library.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */
 
extern int MDV_get_dataset( char * infile_name, MDV_dataset_t *dsp);
     
/******************************************************************************
 * MDV_WRITE_DATASET: Write out the dataset to the given file.
 *
 * Inputs: outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *         dataset - pointer to the dataset information.
 *         output_encoding_type - format for the output field data.
 *         swap_chunk_data - flag indicating if the chunk data should be
 *                           swapped by the library.  The library can only
 *                           swap chunk data of a known type.  If TRUE, the
 *                           library will try to swap the chunk data.  If
 *                           FALSE, the chunk data will be written to disk
 *                           as is.
 *
 * Outputs: outfile - updated on disk to contain the dataset information,
 *                    byte swapped if necessary.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

extern int MDV_write_dataset(FILE *outfile, MDV_dataset_t *dataset,
			     int output_encoding_type,
			     int swap_chunk_data);

/******************************************************************************
 * MDV_WRITE_DATASET_REMOTE: Write an MDV dataset to a possibly remote
 * file system.
 *
 * Inputs: dataset - pointer to dataset information to be written.
 *         output_encoding_type - encoding type to be used for output
 *                                data.
 *         swap_chunk_data - flag indicating if this routine should
 *                           attempt to swap the chunk data on output.
 *         output_host - output host name.  Use "local" if the output
 *                       host is to be the local machine.
 *         output_dir - output directory path.
 *         output_filename - output file name.
 *         local_tmp_dir - local directory to be used for creating the
 *                         output file before sending it to the remote
 *                         machine.
 *
 * Outputs: the data is written the indicated format in the indicate
 *          location.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

extern int MDV_write_dataset_remote(MDV_dataset_t *dataset,
				    int output_encoding_type,
				    int swap_chunk_data,
				    char *output_host,
				    char *output_dir,
				    char *output_filename,
				    char *local_tmp_dir);

/******************************************************************************
 * MDV_SET_CHUNK_HDR_OFFSETS: Set the chunk data offsets in all of the chunk
 * headers.  This is done based on the offset for the last data field, so
 * this value must be set before this routine is called.
 *
 * Inputs: dataset - pointer to the dataset whose chunk header offset values
 *                   are to be updated.
 *
 * Outputs: dataset - all offset values in the chunk headers are set
 *                    appropriately.  Othere dataset information is used
 *                    to determine these offsets and must be accurate
 *                    before this routine is called.
 */
 
void MDV_set_chunk_hdr_offsets(MDV_dataset_t *dataset);

/*****************************************************************
 * MDV_PRINT_DATASET_DATA: print out all of the data for the
 *                         dataset
 */

void MDV_print_dataset_data(MDV_dataset_t *dataset, FILE *outfile);

/*****************************************************************
 * MDV_PRINT_DATASET: print out the important fields in the dataset
 * --Nancy Rehak 7/96
 */

extern void MDV_print_dataset(MDV_dataset_t *dataset, FILE *outfile);

/*****************************************************************
 * MDV_PRINT_DATASET_FULL: print out all of the dataset
 * info
 * --Nancy Rehak 4/96
 */

extern void MDV_print_dataset_full(MDV_dataset_t *dataset, FILE *outfile);

#endif /* MDV_DATASET_H */

#ifdef __cplusplus
}
#endif
