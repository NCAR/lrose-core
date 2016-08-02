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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <memory.h>
#include <sys/stat.h>

#include <toolsa/os_config.h>
#include <toolsa/mem.h>
#include <toolsa/file_io.h>
#include <toolsa/umisc.h>
#include <Mdv/mdv/mdv_dataset.h>
#include <Mdv/mdv/mdv_print.h>
#include <Mdv/mdv/mdv_read.h>
#include <Mdv/mdv/mdv_utils.h>
#include <Mdv/mdv/mdv_user.h>
#include <Mdv/mdv/mdv_write.h>

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

void MDV_init_dataset( MDV_dataset_t *dsp)
{
    dsp->master_hdr    = NULL;
    dsp->fld_hdrs      = NULL;
    dsp->vlv_hdrs      = NULL;
    dsp->chunk_hdrs    = NULL;
    dsp->field_plane   = NULL;
    dsp->chunk_data    = NULL;
    dsp->datafile_buf  = NULL;
    dsp->nfields_alloc = 0;
    dsp->nchunks_alloc = 0;
    return;
}


/******************************************************************************
 * MDV_FREE_DATASET: Frees space for an MDV_dataset.  If the datafile_buf
 * pointer is NULL, the dataset is assumed to have been constructed by the
 * calling routine and each header/data pointer is freed separately and the
 * pointers are set to NULL.  If the datafile_buf pointer is set, the dataset
 * is assumed to have been read directly from disk and the datafile_buf
 * pointer is freed and all of the other pointers are set to NULL.
 *
 * Inputs: dataset - pointer to dataset to be freed.  Not-used pointers should
 *                   be set to NULL otherwise serious side-effects could occur.
 *
 * Outputs: dataset - pointers are freed and set to NULL.  The MDV_dataset_t
 *                    pointer is NOT freed by this routine.
 */
 
void MDV_free_dataset( MDV_dataset_t *dataset)
{
  int ifield;
  int ilevel;
  int ichunk;
  
  int nfields;
  int nchunks;
  int nlevels;
  
  nfields = dataset->master_hdr->n_fields;
  nchunks = dataset->master_hdr->n_chunks;
  if (dataset->master_hdr->vlevel_included)
    nlevels = nfields;
  else
    nlevels = 0;
  
  if (dataset->datafile_buf != NULL)
  {
    
    /*
     * If the data was read in from a file, all of the data is in
     * a contiguous area and we just need to free this and all of
     * the pointers in the dataset structure.
     */

    dataset->master_hdr = NULL;
    
    ufree(dataset->fld_hdrs);
    dataset->fld_hdrs = NULL;

    if (dataset->vlv_hdrs != NULL) {
       ufree(dataset->vlv_hdrs);
       dataset->vlv_hdrs = NULL;
    }

    if (dataset->chunk_hdrs != NULL) {
       ufree(dataset->chunk_hdrs);
       dataset->chunk_hdrs = NULL;
    }

    for (ifield = 0; ifield < nfields; ifield++)
      ufree(dataset->field_plane[ifield]);
    ufree(dataset->field_plane);
    dataset->field_plane = NULL;

    if (dataset->chunk_data != NULL) {
      ufree(dataset->chunk_data);
      dataset->chunk_data = NULL;
    }
 
    dataset->master_hdr = NULL;
  
    dataset->nfields_alloc = 0;
    dataset->nchunks_alloc = 0;
    
    ufree(dataset->datafile_buf);
    dataset->datafile_buf = NULL;
    
  }
  else
  {
    /*
     * Otherwise, the dataset was constructed and all of the pieces
     * need to be freed.
     */

    ufree(dataset->master_hdr);
    dataset->master_hdr = NULL;
    
    for (ifield = 0; ifield < nfields; ifield++)
      ufree(dataset->fld_hdrs[ifield]);
    ufree(dataset->fld_hdrs);
    dataset->fld_hdrs = NULL;

    if (nlevels > 0)
    {
      for (ilevel = 0; ilevel < nlevels; ilevel++)
	ufree(dataset->vlv_hdrs[ilevel]);
      ufree(dataset->vlv_hdrs);
      dataset->vlv_hdrs = NULL;
    }
    
    for (ichunk = 0; ichunk < nchunks; ichunk++)
    {
      ufree(dataset->chunk_hdrs[ichunk]);
      ufree(dataset->chunk_data[ichunk]);
    }
    ufree(dataset->chunk_hdrs);
    ufree(dataset->chunk_data);
    dataset->chunk_hdrs = NULL;
    dataset->chunk_data = NULL;
    
    /*
     * The data is stored in volumes for each field, so free
     * each volume first.
     */

    for (ifield = 0; ifield < nfields; ifield++)
    {
      ufree(dataset->field_plane[ifield][0]);
      ufree(dataset->field_plane[ifield]);
    }
    ufree(dataset->field_plane);
    dataset->field_plane = NULL;

    dataset->nfields_alloc = 0;
    dataset->nchunks_alloc = 0;
    
  }
  
}



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
 
int MDV_get_dataset( char * infile_name, MDV_dataset_t *dsp)
     
{
  
  static char *routine_name = "MDV_get_dataset";
  
  struct stat file_stat;
  FILE *fp;
  int ifld,ilev,ichunk;
  int nfields,nchunks,nplanes;
  int sum_vlev,plane_size;
  char *begin_field_data;
  si32 *plane_locs;
  si32 *plane_sizes;
  int bytes_read;
  
  /* Do some sanity checking */
  
  if (infile_name == NULL || infile_name[0] == '\0')
    {
      fprintf(stderr, "%s: Input file name not specified.\n",
	      routine_name);
      return (MDV_FAILURE);
    }
  
  if (!MDV_verify(infile_name))
    {
      fprintf(stderr, "ERROR - MDV_get_dataset\n");
      fprintf(stderr, "File %s is not MDV format\n", infile_name);
      return (MDV_FAILURE);
    }
  
  /* open file */

  if ((fp = ta_fopen_uncompress(infile_name,"r")) == NULL)
    {
      fprintf(stderr,"%s: Error opening file: %s\n",
	      routine_name, infile_name);
      return (MDV_FAILURE);
    }
  
  /* Find out how big the file is */

  if (fstat(fileno(fp), &file_stat) != 0)
    {
      fprintf(stderr,"%s: Could not stat file input file %s.\n",
	      routine_name, infile_name);
      fclose(fp);
      return (MDV_FAILURE);
    }

  /* go to beginning of file */

  fseek(fp,0,SEEK_SET);
     
  /* Allocate the space for the whole dataset */

  dsp->datafile_buf =
    (char *) MDV_recalloc(dsp->datafile_buf, 1, file_stat.st_size);
  
  /* Read all the data into the buffer and close the file */

  if ((bytes_read = ufread(dsp->datafile_buf, 1,
			   file_stat.st_size, fp))
      != file_stat.st_size)
    {
      fprintf(stderr,
	      "%s: Error reading in file information "
	      "(bytes expected = %d, bytes read = %d)\n",
	      routine_name, (int)file_stat.st_size, bytes_read);
      ufree(dsp->datafile_buf);
      fclose(fp);
      return (MDV_FAILURE);
    }
  fclose(fp);

  /* Set the master header pointer */

  dsp->master_hdr = (MDV_master_header_t *)dsp->datafile_buf;
  
  /*
   * Swap the data in the master header.
   */
  
  MDV_master_header_from_BE(dsp->master_hdr);
  
  /* set pointers to each field header and each vlevel header */

  nfields = dsp->master_hdr->n_fields;
    
  if (dsp->nfields_alloc < nfields) {
    dsp->fld_hdrs = (MDV_field_header_t **)
      MDV_recalloc(dsp->fld_hdrs,nfields, sizeof(MDV_field_header_t *));
    dsp->vlv_hdrs = (MDV_vlevel_header_t **)
      MDV_recalloc(dsp->vlv_hdrs,nfields, sizeof(MDV_vlevel_header_t *));
    dsp->nfields_alloc = nfields;
  }
  
  /* set field and vlevel header pointers and swap the data */

  for (ifld = 0; ifld < nfields; ifld ++ ) {
    
    dsp->fld_hdrs[ifld] = (MDV_field_header_t *)
      (dsp->master_hdr->field_hdr_offset +
       ifld*sizeof(MDV_field_header_t)+dsp->datafile_buf);
    
    MDV_field_header_from_BE(dsp->fld_hdrs[ifld]);
    
    if (dsp->master_hdr->vlevel_included)
      {
	dsp->vlv_hdrs[ifld] = (MDV_vlevel_header_t *)
	  (dsp->master_hdr->vlevel_hdr_offset +
	   ifld*sizeof(MDV_vlevel_header_t)+dsp->datafile_buf);
	
	MDV_vlevel_header_from_BE(dsp->vlv_hdrs[ifld]);
      }
    else
      {
	dsp->vlv_hdrs[ifld] = NULL;
      }
    
  } /* end setting field and vlevel pointers */
  
  /* set chunk headers pointer */
  nchunks = dsp->master_hdr->n_chunks;
  if (dsp->nchunks_alloc < nchunks) {
    dsp->chunk_hdrs = (MDV_chunk_header_t **)
      MDV_recalloc(dsp->chunk_hdrs,nchunks, sizeof(MDV_chunk_header_t *));
    
    dsp->chunk_data = (void **)
      MDV_recalloc(dsp->chunk_data, nchunks, sizeof(void *));
    
    dsp->nchunks_alloc = nchunks;
  }
  
  for (ichunk = 0; ichunk < nchunks; ichunk ++ ) {
    dsp->chunk_hdrs[ichunk] = (MDV_chunk_header_t *)
      (dsp->master_hdr->chunk_hdr_offset + 
       ichunk*sizeof(MDV_chunk_header_t) + 
       dsp->datafile_buf);

    MDV_chunk_header_from_BE(dsp->chunk_hdrs[ichunk]);
       
    dsp->chunk_data[ichunk] = (void *)
      (dsp->datafile_buf +
       dsp->chunk_hdrs[ichunk]->chunk_data_offset);
       
    if ((MDV_chunk_data_from_BE((void *)((char *)dsp->chunk_data[ichunk] -
					 sizeof(si32)),
				dsp->chunk_hdrs[ichunk]->size,
				dsp->chunk_hdrs[ichunk]->chunk_id))
	== MDV_FAILURE)
      {
	fprintf(stderr,
		"\n%s: Error swapping chunk %d data.\n",
		routine_name, ichunk);
      }
       
  }

  /* allocate pointers to beginning of each plane in each field */
  nplanes = dsp->master_hdr->max_nz;
       
  dsp->field_plane = (void ***)MDV_recalloc(dsp->field_plane,
					    nfields,
					    sizeof(void **));
  
  for (ifld = 0; ifld < nfields; ifld++) {

    dsp->field_plane[ifld] =
      (void **)MDV_recalloc(dsp->field_plane[ifld],
			    nplanes,
			    sizeof(void *));
      
    /*
     * Set pointers to beginning of each plane in each field and swap
     * the data, if necessary.
     */
    
    if (!MDV_compressed(dsp->fld_hdrs[ifld]->compression_type)) {
      
      /* uncompressed data -
       * plane size remains constant for each field */
      
      plane_size = (dsp->fld_hdrs[ifld]->nx)*
	(dsp->fld_hdrs[ifld]->ny)*
	MDV_data_element_size(dsp->fld_hdrs[ifld]->encoding_type);
      
      begin_field_data = dsp->datafile_buf +
	dsp->fld_hdrs[ifld]->field_data_offset;
      sum_vlev = 0;
      
      for (ilev = 0; ilev < dsp->fld_hdrs[ifld]->nz ; ilev ++)  {
	dsp->field_plane[ifld][ilev] =
	  (void *)(begin_field_data + sum_vlev);
	sum_vlev += plane_size;
      } /* end for each level loop */
      
      /* swap the data if necessary */
      if (MDV_unencoded_volume_from_BE(dsp->field_plane[ifld][0],
				       dsp->fld_hdrs[ifld]->volume_size,
				       dsp->fld_hdrs[ifld]->encoding_type)
	  != MDV_SUCCESS) {
	fprintf(stderr,
		"%s: Error swapping unencoded data volume for field %d\n",
		routine_name, ifld);
	return(MDV_FAILURE);
      }
      
    } else { /* compressed data */
      
      int array_size;
      
      begin_field_data = dsp->datafile_buf +
	dsp->fld_hdrs[ifld]->field_data_offset +
	(2 * dsp->fld_hdrs[ifld]->nz * sizeof(si32));

      sum_vlev = 0;
      
      plane_locs = (si32 *)(dsp->datafile_buf +
			    dsp->fld_hdrs[ifld]->field_data_offset);
      
      plane_sizes = (si32 *)(dsp->datafile_buf +
			     dsp->fld_hdrs[ifld]->field_data_offset +
			     (dsp->fld_hdrs[ifld]->nz * sizeof(si32)));
      
      array_size = dsp->fld_hdrs[ifld]->nz * sizeof(si32);
      
      BE_to_array_32((ui32 *)plane_locs, array_size);
      
      BE_to_array_32((ui32 *)plane_sizes, array_size);
      
      for (ilev = 0; ilev < dsp->fld_hdrs[ifld]->nz; ilev++) {
	dsp->field_plane[ifld][ilev] =
	  (void *)(begin_field_data + sum_vlev);
	
	if (MDV_plane_rle8_from_BE(dsp->field_plane[ifld][ilev])
	    != MDV_SUCCESS) {
	  fprintf(stderr,
		  "%s: Error swapping MDV_PLANE_RLE8 data\n",
		  routine_name);
	  return(MDV_FAILURE);
	}
	
	sum_vlev += plane_sizes[ilev];
      }
      
    } /* if (compression_type == MDV_COMPRESSION_NONE) */
    
  } /* ifld */
  
  return MDV_SUCCESS;

} /* end MDV_get_dataset */

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

int MDV_write_dataset(FILE *outfile, MDV_dataset_t *dataset,
                      int output_encoding_type,
                      int swap_chunk_data)
{
  static char *routine_name = "MDV_write_dataset";
  
  int i;
  long next_offset;
  MDV_master_header_t blank_master_hdr;
  
  /*
   * Make sure the output file pointer is valid.
   */

  if (outfile == NULL)
  {
    fprintf(stderr,
            "%s: Invalid output file pointer.\n",
            routine_name);
    return(MDV_FAILURE);
  }

  /*
   * Make sure the file offset values in the master header are okay.
   */

  MDV_set_master_hdr_offsets(dataset->master_hdr);
  
  /*
   * Write a blank master header, we will write the correct
   * master header at the end of the routine.  This is done to flag
   * the file as corrupted if the program is interrupted before the
   * write is completed.
   */
  
  if (fseek(outfile, 0, SEEK_SET) != 0)
  {
    fprintf(stderr,
            "%s: Error seeking to beginning of output file\n",
            routine_name);
    fclose(outfile);
    return(MDV_FAILURE);
  }
  
  memset(&blank_master_hdr, 0, sizeof(MDV_master_header_t));
  
  if (ufwrite(&blank_master_hdr, sizeof(MDV_master_header_t),
              1, outfile) != 1)
  {
    fprintf(stderr,
            "%s: Error writing blank master header to output file.\n",
            routine_name);
    fclose(outfile);
    return(MDV_FAILURE);
  }
  
  /*
   * Write the vlevel headers.
   */

  if (dataset->master_hdr->vlevel_included)
  {
    for (i = 0; i < dataset->master_hdr->n_fields; i++)
    {
      if (MDV_write_vlevel_header(outfile, dataset->vlv_hdrs[i],
                                  dataset->master_hdr, i) != MDV_SUCCESS)
      {
        fprintf(stderr,
                "%s: Error writing vlevel header %d.\n",
                routine_name, i);
        return(MDV_FAILURE);
      }
    }
  }
  
  /*
   * Write the field headers and data.  Calculate the data offsets
   * since the output offsets will be different than the offsets in
   * memory if the output encoding format is different.
   */

  if (dataset->master_hdr->vlevel_included)
    next_offset = sizeof(MDV_master_header_t) +
      (dataset->master_hdr->n_fields * sizeof(MDV_field_header_t)) +
        (dataset->master_hdr->n_fields * sizeof(MDV_vlevel_header_t)) +
          (dataset->master_hdr->n_chunks * sizeof(MDV_chunk_header_t)) +
            sizeof(si32);
  else
    next_offset = sizeof(MDV_master_header_t) +
      (dataset->master_hdr->n_fields * sizeof(MDV_field_header_t)) +
        (dataset->master_hdr->n_chunks * sizeof(MDV_chunk_header_t)) +
          sizeof(si32);

  for (i = 0; i < dataset->master_hdr->n_fields; i++)
  {
    int output_volume_size;
    
    if ((output_volume_size = MDV_write_field(outfile, dataset->fld_hdrs[i],
                                              dataset->field_plane[i][0], i,
                                              next_offset,
                                              output_encoding_type))
        < 0)
    {
      fprintf(stderr,
              "%s: Error writing field header %d.\n",
              routine_name, i);
      return(MDV_FAILURE);
    }

    next_offset += output_volume_size + (2 * sizeof(si32));
  }
  
  /*
   * Write the chunk headers and data.  Recalculate the file offsets because
   * these will have changed if the data was compressed as it was
   * written.
   */

  MDV_set_chunk_hdr_offsets(dataset);
  
  for (i = 0; i < dataset->master_hdr->n_chunks; i++)
  {
    if (MDV_write_chunk(outfile, dataset->chunk_hdrs[i],
                        dataset->chunk_data[i],
                        dataset->master_hdr, i,
                        next_offset,
                        swap_chunk_data) != MDV_SUCCESS)
    {
      fprintf(stderr,
              "%s: Error writing chunk header %d.\n",
              routine_name, i);
      return(MDV_FAILURE);
    }

    next_offset += dataset->chunk_hdrs[i]->size + (2 * sizeof(si32));
    
  }
  
  /*
   * Finally, write the master header.
   */

  if (MDV_write_master_header(outfile, dataset->master_hdr)
      != MDV_SUCCESS)
  {
    fprintf(stderr,
            "%s: Error writing master header.\n",
            routine_name);
    return(MDV_FAILURE);
  }
  
  return(MDV_SUCCESS);

} /* end MDV_write_dataset */


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

int MDV_write_dataset_remote(MDV_dataset_t *dataset,
                             int output_encoding_type,
                             int swap_chunk_data,
                             char *output_host,
                             char *output_dir,
                             char *output_filename,
                             char *local_tmp_dir)
{
  static char *routine_name = "MDV_write_dataset_remote";
  
  char full_output_filename[MAX_PATH_LEN];
  
  /*
   * For the local host, just call the local routine.
   */

  if (strcmp(output_host, "local") == 0)
  {
    FILE *mdv_file;

    /*
     * Make sure the output directory exists.
     */

    if (makedir(output_dir) != 0)
    {
      fprintf(stderr,
              "%s: Error creating output directory %s\n",
              routine_name, output_dir);
      
      return(MDV_FAILURE);
    }
    
    /*
     * Determine the output file path.
     */

    sprintf(full_output_filename, "%s/%s",
            output_dir, output_filename);
    
    /*
     * Open the output file.
     */

    if ((mdv_file = fopen(full_output_filename, "w")) == NULL)
    {
      fprintf(stderr,
              "%s: Error opening file for output.\n",
              routine_name);
      perror(output_filename);
      
      return(MDV_FAILURE);
    }
    
    /*
     * Write the dataset information.
     */

    if (MDV_write_dataset(mdv_file, dataset, output_encoding_type,
                          swap_chunk_data) != MDV_SUCCESS)
    {
      fprintf(stderr,
              "%s: Error writing dataset to file <%s>\n",
              routine_name, output_filename);
      
      fclose(mdv_file);
      return(MDV_FAILURE);
    }
    
    /*
     * Close the output file.
     */

    fclose(mdv_file);
    
  } /* endif - local file */
  else
  {
    time_t now;
    char tmp_file_path[MAX_PATH_LEN];
    char remote_tmp_file_path[MAX_PATH_LEN];
    char remote_file_path[MAX_PATH_LEN];
    FILE *tmp_file;
    char call_str[BUFSIZ];
    
    /*
     * Compute file names and paths.
     */

    now = time(NULL);
    sprintf(tmp_file_path, "%s/mdv_temp.%ld",
            local_tmp_dir, now);
    sprintf(remote_tmp_file_path, "%s/mdv_temp.%ld",
            output_dir, now);
    
    sprintf(remote_file_path, "%s/%s",
            output_dir, output_filename);
    
    /*
     * Create the temp directories, if necessary.
     */

    if (makedir(tmp_file_path) != 0)
    {
      fprintf(stderr,
              "%s: Error creating temporary directory <%s> on local machine\n",
              routine_name, tmp_file_path);
      
      return(MDV_FAILURE);
    }
    
    sprintf(call_str, "rsh -n %s mkdir -p %s",
            output_host, output_dir);
    
    usystem_call(call_str);
    
    /*
     * Open the temporary file.
     */

    if ((tmp_file = fopen(tmp_file_path, "w")) == NULL)
    {
      fprintf(stderr,
              "%s: Error opening temporary file.\n",
              routine_name);
      perror(tmp_file_path);
      
      return(MDV_FAILURE);
    }
    
    /*
     * Write the temporary file.
     */

    if (MDV_write_dataset(tmp_file, dataset, output_encoding_type,
                          swap_chunk_data) != MDV_SUCCESS)
    {
      fprintf(stderr,
              "%s: Error writing dataset to file <%s>\n",
              routine_name, tmp_file_path);
      
      fclose(tmp_file);
      return(MDV_FAILURE);
    }
    
    /*
     * Close the temporary file.
     */

    fclose(tmp_file);
    
    /*
     * Copy the temp file into the temp area on the remote host.
     */

    sprintf(call_str, "rcp %s %s:%s",
            tmp_file_path, output_host, remote_tmp_file_path);
    
    usystem_call(call_str);
    
    /*
     * Move the file to the correct location on the remote host.
     */

    sprintf(call_str, "rsh %s mv %s %s",
            output_host, remote_tmp_file_path, remote_file_path);
    
    usystem_call(call_str);
    
    /*
     * Delete the temp file locally.  Spit out a message but continue
     * processing if there is an error.
     */

    if (unlink(tmp_file_path) != 0)
    {
      fprintf(stderr,
              "%s: Error removing local temp file\n",
              routine_name);
      perror(tmp_file_path);
    }
    
  } /* endelse - file on remote system */
  
  return(MDV_SUCCESS);
    
} /* end MDV_write_dataset_remote */

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
 
void MDV_set_chunk_hdr_offsets(MDV_dataset_t *dataset)
{

  int i;
  
  int last_field_index = dataset->master_hdr->n_fields-1;
  
  long next_offset =
    dataset->fld_hdrs[last_field_index]->field_data_offset +
      dataset->fld_hdrs[last_field_index]->volume_size +
	(2 * sizeof(si32));
  
  for (i = 0; i < dataset->master_hdr->n_chunks; i++)
  {
    dataset->chunk_hdrs[i]->chunk_data_offset = next_offset;
    
    next_offset += dataset->chunk_hdrs[i]->size + (2 * sizeof(si32));
  } /* endfor - i */
      
  return;

}

/*****************************************************************
 * MDV_PRINT_DATASET_DATA: print out all of the data for the
 *                         dataset
 * --Nancy Rehak 8/96
 */

void MDV_print_dataset_data(MDV_dataset_t *dataset,
			    FILE *outfile)
{
  int ifield;
  int iplane;
  
  for (ifield = 0; ifield < dataset->master_hdr->n_fields; ifield++)
  {
    for (iplane = 0; iplane < dataset->fld_hdrs[ifield]->nz; iplane++)
    {
      MDV_print_field_plane_full(dataset->fld_hdrs[ifield], 
				 dataset->field_plane[ifield][iplane],
				 ifield, iplane,
				 outfile);
    } /* endfor - iplane */
  } /* endfor - ifield */
    
  return;
}




/*****************************************************************
 * MDV_PRINT_DATASET: print out the important fields in the dataset
 * --Nancy Rehak 7/96
 */

void MDV_print_dataset(MDV_dataset_t *dataset, FILE *outfile)
{
  int i;
  
  MDV_print_master_header(dataset->master_hdr, outfile);

  for (i = 0; i < dataset->master_hdr->n_fields; i++)
    MDV_print_field_header(dataset->fld_hdrs[i], outfile);
    
  if (dataset->master_hdr->vlevel_included)
    for (i = 0; i < dataset->master_hdr->n_fields; i++)
      MDV_print_vlevel_header(dataset->vlv_hdrs[i],
			      dataset->master_hdr->max_nz,
			      (char *)dataset->fld_hdrs[i]->field_name,
			      outfile);
    
  for (i = 0; i < dataset->master_hdr->n_chunks; i++)
    MDV_print_chunk_header(dataset->chunk_hdrs[i], outfile);
  
  return;
}


/*****************************************************************
 * MDV_PRINT_DATASET_FULL: print out all of the dataset
 * info
 * --Nancy Rehak 4/96
 */

void MDV_print_dataset_full(MDV_dataset_t *dataset, FILE *outfile)
{
  int i;
  
  MDV_print_master_header_full(dataset->master_hdr, outfile);

  fprintf(outfile,
	  "nfields_alloc = %d, nchunks_alloc = %d\n",
	  dataset->nfields_alloc, dataset->nchunks_alloc);
    
  for (i = 0; i < dataset->master_hdr->n_fields; i++)
    MDV_print_field_header_full(dataset->fld_hdrs[i], outfile);
    
  if (dataset->master_hdr->vlevel_included)
    for (i = 0; i < dataset->master_hdr->n_fields; i++)
      MDV_print_vlevel_header_full(dataset->vlv_hdrs[i],
				   dataset->master_hdr->max_nz,
				   (char *)dataset->fld_hdrs[i]->field_name,
				   outfile);
    
  for (i = 0; i < dataset->master_hdr->n_chunks; i++)
  {
    MDV_print_chunk_header_full(dataset->chunk_hdrs[i], outfile);
    MDV_print_chunk_data_full(dataset->chunk_data[i],
			      dataset->chunk_hdrs[i]->chunk_id,
			      dataset->chunk_hdrs[i]->size,
			      outfile);
  }
  
  return;
}


