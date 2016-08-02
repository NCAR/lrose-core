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
 *  MDV_WRITE.C  Subroutines writing MDV data to files.
 *  F. Hage.  Dec 1993. RAP, R. Ames 6/96.
 *
 *  Separated from mdv_user.c.  N. Rehak, Aug. 1996.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <toolsa/os_config.h>

#include <dataport/bigend.h>
#include <toolsa/file_io.h>
#include <toolsa/mem.h>
#include <toolsa/membuf.h>
#include <toolsa/umisc.h>
#include <toolsa/ldata_info.h>

#include <Mdv/mdv/mdv_print.h>
#include <Mdv/mdv/mdv_write.h>
#include <Mdv/mdv/mdv_user.h>
#include <Mdv/mdv/mdv_utils.h>
#include <Mdv/mdv/mdv_field_handle.h>

#include "mdv_private.h"

/******************************************************************************
 * MDV_handle_write_to_dir
 *
 * Similar to MDV_write_to_dir(), but with control over the output
 * encoding, compression and scaling.
 *
 * Write out the handle structs and data to a given dirctory. The
 * file path is computed from the centroid time. Output files all
 * have the mdv extension.
 *
 * Inputs: mdv - handle containing the mdv structs and data
 *         output_dir - output directory
 *         output_encoding_type - format for the output field data.
 *         write_ldata_info - option to write latest data info file.
 *
 * Note on chunk swapping: if the chunk type has a swapping routine
 * in the mdv library, it will be swapped. If not, it will not
 * be swapped.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 *
 */

int MDV_handle_write_to_dir(MDV_handle_t *mdv,
			    char *output_dir,
			    int output_encoding,
			    int output_compression,
			    int output_scaling,
			    double output_scale,
			    double output_bias,
			    int write_ldata_info)

{

  static char *routine_name = "MDV_handle_write_to_dir";

  char file_name[MAX_PATH_LEN];
  char file_path[MAX_PATH_LEN];
  char subdir_path[MAX_PATH_LEN];
  
  struct stat dir_stat;

  /*
   * compute paths
   */

  date_time_t time_cent;
  time_cent.unix_time = mdv->master_hdr.time_centroid;
  uconvert_from_utime(&time_cent);
  
  sprintf(subdir_path, "%s%s%.4d%.2d%.2d",
          output_dir, PATH_DELIM,
          (int) time_cent.year,
          (int) time_cent.month,
          (int) time_cent.day);
  
  sprintf(file_name, "%.2d%.2d%.2d.mdv",
          (int) time_cent.hour,
          (int) time_cent.min,
          (int) time_cent.sec);
  
  sprintf(file_path, "%s%s%s",
          subdir_path, PATH_DELIM, file_name);
  
  /*
   * create subdirectory, if needed
   */
  
  if (0 != stat(subdir_path, &dir_stat)) {
    if (ta_makedir_recurse(subdir_path)) {
      fprintf(stderr, "ERROR - %s\n", routine_name);
      fprintf(stderr, "Trying to make output dir\n");
      perror(subdir_path);
      return (MDV_FAILURE);
    }
  }

  /*
   * write file
   */
  
  if (MDV_handle_write_all(mdv, file_path, output_encoding,
			   output_compression, output_scaling,
			   output_scale, output_bias) != MDV_SUCCESS) {
    return (MDV_FAILURE);
  }

  /*
   * write latest data info if requested
   */

  if (write_ldata_info) {
    
    LDATA_handle_t ldata;
    LDATA_init_handle(&ldata, "unknown", FALSE);
        
    if (LDATA_info_write(&ldata,
                         output_dir,
                         time_cent.unix_time,
                         "mdv",
                         NULL, NULL, 0, NULL)) {
      LDATA_free_handle(&ldata);
      return (MDV_FAILURE);
    }
    
    LDATA_free_handle(&ldata);

  } /* if (write_current_index) */

  return (MDV_SUCCESS);


}

/******************************************************************************
 * MDV_write_to_dir
 *
 * Write out the handle structs and data to a given dirctory. The file
 * path is computed from the centroid time. Output files all have the
 * mdv extension.
 *
 * Inputs: mdv - handle containing the mdv structs and data
 *         output_dir - output directory
 *         output_encoding_type - format for the output field data.
 *         write_ldata_info - option to write latest data info file.
 *
 * Note on chunk swapping: if the chunk type has a swapping routine
 * in the mdv library, it will be swapped. If not, it will not
 * be swapped.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.  */

int MDV_write_to_dir(MDV_handle_t *mdv,
		     char *output_dir,
		     int output_encoding_type,
		     int write_ldata_info)

{

  int output_encoding;
  int output_compression;

  output_encoding = output_encoding_type;
  output_compression = MDV_COMPRESSION_NONE;

  if (output_encoding_type == MDV_PLANE_RLE8) {
    output_encoding = MDV_INT8;
    output_compression = MDV_COMPRESSION_RLE;
  }

  return (MDV_handle_write_to_dir(mdv, output_dir,
				  output_encoding,
				  output_compression,
				  MDV_SCALING_ROUNDED, 0.0, 0.0,
				  write_ldata_info));

}

/******************************************************************************
 * MDV_handle_write_to_ds_dir
 *
 * Similar to MDV_write_to_ds_dir(), but with control over the output
 * encoding, compression and scaling.
 *
 * Write out the handle structs and data to a dirctory,
 * using the Didss data naming convention:
 *   output_dir/yyyymmdd/g_hhmmss/f_ssssssss
 *
 * Inputs: mdv - handle containing the mdv structs and data
 *         output_dir - output directory
 *         output_encoding_type - format for the output field data.
 *         write_ldata_info - option to write latest data info file.
 *
 * Note on chunk swapping: if the chunk type has a swapping routine
 * in the mdv library, it will be swapped. If not, it will not
 * be swapped.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

int MDV_handle_write_to_ds_dir(MDV_handle_t *mdv,
			       char *output_dir,
			       int output_encoding,
			       int output_compression,
			       int output_scaling,
			       double output_scale,
			       double output_bias,
			       int write_ldata_info)

{

  static char *routine_name = "MDV_handle_write_to_ds_dir";

  char file_name[MAX_PATH_LEN];
  char file_path[MAX_PATH_LEN];
  char daydir_path[MAX_PATH_LEN];
  char gendir_path[MAX_PATH_LEN];
  
  struct stat dir_stat;

  /*
   * compute paths
   */

  int leadSecs;
  date_time_t time_gen;
  date_time_t time_cent;
  time_t lead_secs = mdv->master_hdr.time_centroid - mdv->master_hdr.time_gen;
  time_gen.unix_time = mdv->master_hdr.time_gen;
  uconvert_from_utime(&time_gen);

  /* Calculate the ldata info time based on data valid time. */
  time_cent.unix_time = mdv->master_hdr.time_centroid;
  uconvert_from_utime(&time_cent);
  
  sprintf(daydir_path, "%s%s%.4d%.2d%.2d",
          output_dir, PATH_DELIM,
          (int) time_gen.year,
          (int) time_gen.month,
          (int) time_gen.day);

  sprintf(gendir_path, "%s%sg_%.2d%.2d%.2d",
          daydir_path, PATH_DELIM,
          (int) time_gen.hour,
          (int) time_gen.min,
          (int) time_gen.sec);
  
  sprintf(file_name, "f_%.8d.mdv",
          (int) lead_secs);
  
  sprintf(file_path, "%s%s%s",
          gendir_path, PATH_DELIM, file_name);
  
  /*
   * create daydirectory, if needed
   */
  
  if (0 != stat(daydir_path, &dir_stat)) {
    if (ta_makedir_recurse(daydir_path)) {
      fprintf(stderr, "ERROR - %s\n", routine_name);
      fprintf(stderr, "Trying to make output day dir\n");
      perror(daydir_path);
      return (MDV_FAILURE);
    }
  }

  /*
   * create gendirectory, if needed
   */
  
  if (0 != stat(gendir_path, &dir_stat)) {
    if (ta_makedir_recurse(gendir_path)) {
      fprintf(stderr, "ERROR - %s\n", routine_name);
      fprintf(stderr, "Trying to make output gen dir\n");
      perror(gendir_path);
      return (MDV_FAILURE);
    }
  }

  /*
   * write file
   */

  if (MDV_handle_write_all(mdv, file_path, output_encoding,
			   output_compression, output_scaling,
			   output_scale, output_bias) != MDV_SUCCESS) {
    return (MDV_FAILURE);
  }

  /*
   * write latest data info if requested
   */

  if (write_ldata_info) {
    
    LDATA_handle_t ldata;
    LDATA_init_handle(&ldata, "unknown", FALSE);
        
    leadSecs = (int) lead_secs;
    /* Todo: Read the directory for other forecasts. */
    if (LDATA_info_write(&ldata,
                         output_dir,
                         time_gen.unix_time,
                         "mdv",
                         NULL, NULL, 1, &leadSecs )) {
      LDATA_free_handle(&ldata);
      return (MDV_FAILURE);
    }
    
    LDATA_free_handle(&ldata);

  } /* if (write_current_index) */

  return (MDV_SUCCESS);

}


/******************************************************************************
 * MDV_WRITE_TO_DS_DIR: Write out the handle structs and data to a dirctory,
 *                   using the Didss data naming convention:
 *                                                           
 *                       output_dir/yyyymmdd/g_hhmmss/f_ssssssss
 * 
 *
 * Inputs: mdv - handle containing the mdv structs and data
 *         output_dir - output directory
 *         output_encoding_type - format for the output field data.
 *         write_ldata_info - option to write latest data info file.
 *
 * Note on chunk swapping: if the chunk type has a swapping routine
 * in the mdv library, it will be swapped. If not, it will not
 * be swapped.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

int MDV_write_to_ds_dir(MDV_handle_t *mdv,
                        char *output_dir,
                        int output_encoding_type,
                        int write_ldata_info)

{

  int output_encoding;
  int output_compression;

  output_encoding = output_encoding_type;
  output_compression = MDV_COMPRESSION_NONE;

  if (output_encoding_type == MDV_PLANE_RLE8) {
    output_encoding = MDV_INT8;
    output_compression = MDV_COMPRESSION_RLE;
  }

  return (MDV_handle_write_to_ds_dir(mdv, output_dir,
				     output_encoding,
				     output_compression,
				     MDV_SCALING_ROUNDED, 0.0, 0.0,
				     write_ldata_info));

}

/******************************************************************************
 * MDV_HANDLE_WRITE_ALL
 *
 * Write out the handle structs and data to the given path.
 *
 * This is an up-to-date version of MDV_write_all(), which allows
 * you to specify output encoding, compression and scaling.
 *
 * Inputs: mdv - handle containing the mdv structs and data
 *         file_path - path of output file. 
 *         output_encoding_type - format for the output field data.
 *
 * Note on chunk swapping: if the chunk type has a swapping routine
 * in the mdv library, it will be swapped. If not, it will not
 * be swapped.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

int MDV_handle_write_all(MDV_handle_t *mdv,
			 char *file_path,
			 int output_encoding,
			 int output_compression,
			 int output_scaling,
			 double output_scale,
			 double output_bias)

{
  
  static char *routine_name = "MDV_handle_write_all";

  int i;
  int plane_len, plane_offset;
  int next_offset;
  FILE *outfile;
  MDV_master_header_t blank_master_hdr;
  char tmp_path[MAX_PATH_LEN];
  ui32 plane_offsets[MDV_MAX_VLEVELS];
  ui32 plane_sizes[MDV_MAX_VLEVELS];
  MEMbuf *work_buf;
  MEMbuf *field_buf;

  /*
   * compute tmp file path
   */

  ta_tmp_path_from_final(file_path, tmp_path,
                         MAX_PATH_LEN, "TMP_MDV");

  /*
   * Open file
   */

  outfile = fopen(tmp_path, "w");
  if (outfile == NULL)
  {
    fprintf(stderr, "%s: Cannot open tmp output file.\n", routine_name);
    perror(tmp_path);
    return(MDV_FAILURE);
  }

  /*
   * Make sure the file offset values in the master header are okay.
   */

  MDV_set_master_hdr_offsets(&mdv->master_hdr);
  
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

  if (mdv->master_hdr.vlevel_included)
  {
    for (i = 0; i < mdv->master_hdr.n_fields; i++)
    {
      if (MDV_write_vlevel_header(outfile, mdv->vlv_hdrs + i,
                                  &mdv->master_hdr, i) != MDV_SUCCESS)
      {
        fprintf(stderr,
                "%s: Error writing vlevel header %d.\n",
                routine_name, i);
        fclose(outfile);
        return(MDV_FAILURE);
      }
    }
  }
  
  /*
   * Some older files do not have the vlevel types in the
   * field header. In those cases, make the field header
   * consistent with the master header.
   */
  
  for (i = 0; i < mdv->master_hdr.n_fields; i++) {
    MDV_field_header_t *fhdr = mdv->fld_hdrs + i;
    if (fhdr->native_vlevel_type == 0 &&
	mdv->master_hdr.native_vlevel_type != 0) {
      fhdr->native_vlevel_type = mdv->master_hdr.native_vlevel_type;
    }
    if (fhdr->vlevel_type == 0 &&
	mdv->master_hdr.vlevel_type != 0) {
      fhdr->vlevel_type = mdv->master_hdr.vlevel_type;
    }
  } /* i */
      
  /*
   * Write the field headers and data.  Calculate the data offsets
   * since the output offsets will be different than the offsets in
   * memory if the output encoding format is different.
   */

  next_offset = sizeof(MDV_master_header_t) +
    (mdv->master_hdr.n_fields * sizeof(MDV_field_header_t)) +
    (mdv->master_hdr.n_chunks * sizeof(MDV_chunk_header_t)) +
    sizeof(ui32);

  if (mdv->master_hdr.vlevel_included) {
    next_offset += mdv->master_hdr.n_fields * sizeof(MDV_vlevel_header_t);
  }

  for (i = 0; i < mdv->master_hdr.n_fields; i++)
  {

    ui08 *field_data;
    int j;
    int output_volume_size;
    MDV_field_header_t fhdr;

    /*
     * compute plane and vol sizes
     */
    
    fhdr = mdv->fld_hdrs[i];

    /*
     * load up field into contiguous array
     */

    field_buf = MEMbufCreate();
    work_buf = MEMbufCreate();
    plane_offset = 0;

    for (j = 0; j < fhdr.nz; j++) {
      if (mdv->field_plane_len[i][j] == 0) {
	plane_len = MDV_calc_plane_size(&fhdr, j,
					mdv->field_plane[i][j]);
      } else {
	plane_len = mdv->field_plane_len[i][j];
      }
      MEMbufAdd(work_buf, mdv->field_plane[i][j], plane_len);
      plane_offsets[j] = plane_offset;
      plane_sizes[j] = plane_len;
      plane_offset += plane_len;
    } /* j */

    if (!MDV_compressed(fhdr.compression_type)) {
      field_data = (ui08 *) MEMbufPtr(work_buf);
      fhdr.volume_size = MEMbufLen(work_buf);
    } else {
      BE_from_array_32(plane_offsets, fhdr.nz * sizeof(ui32));
      BE_from_array_32(plane_sizes, fhdr.nz * sizeof(ui32));
      MEMbufAdd(field_buf, plane_offsets, fhdr.nz * sizeof(ui32));
      MEMbufAdd(field_buf, plane_sizes, fhdr.nz * sizeof(ui32));
      /*
       * strip off the two ui32s at the start of the plane: offset & len
       */
      MEMbufAdd(field_buf,
		(ui08 *) MEMbufPtr(work_buf) + 2 * sizeof(ui32),
		MEMbufLen(work_buf) - 2 * sizeof(ui32));
      field_data = (ui08 *) MEMbufPtr(field_buf);
      fhdr.volume_size = MEMbufLen(field_buf);
    }

    fhdr.field_data_offset = next_offset;

    if (MDV_write_field_vol(outfile,
			    &fhdr,
			    i,
			    field_data,
			    output_encoding,
			    output_compression,
			    output_scaling,
			    output_scale,
			    output_bias,
			    &output_volume_size)) {
      fprintf(stderr,
              "%s: Error writing field header %d.\n",
              routine_name, i);
      MEMbufDelete(field_buf);
      MEMbufDelete(work_buf);
      fclose(outfile);
      return(MDV_FAILURE);
    }
    
    next_offset += output_volume_size + (2 * sizeof(ui32));
    MEMbufDelete(field_buf);
    MEMbufDelete(work_buf);

  }
  
  /*
   * Write the chunk headers and data.
   */

  for (i = 0; i < mdv->master_hdr.n_chunks; i++)
  {
    if (MDV_write_chunk(outfile, mdv->chunk_hdrs + i,
                        mdv->chunk_data[i],
                        &mdv->master_hdr, i,
                        next_offset,
                        TRUE) != MDV_SUCCESS)
    {
      fprintf(stderr,
              "%s: Error writing chunk header %d.\n",
              routine_name, i);
      fclose(outfile);
      return(MDV_FAILURE);
    }

    next_offset += mdv->chunk_hdrs[i].size + (2 * sizeof(ui32));
    
  }
  
  /*
   * Finally, write the master header.
   */
  
  if (MDV_write_master_header(outfile, &mdv->master_hdr)
      != MDV_SUCCESS)
  {
    fprintf(stderr,
            "%s: Error writing master header.\n",
            routine_name);
    fclose(outfile);
    return(MDV_FAILURE);
  }

  /*
   * close file
   */
  
  fclose(outfile);

  /*
   * rename the tmp file to the final file
   */

  if (rename(tmp_path, file_path)) {
    fprintf(stderr, "ERROR: %s\n", routine_name);
    fprintf(stderr, "Error renaming tmp output file %s\n", tmp_path);
    perror(file_path);
    return(MDV_FAILURE);
  }

  return(MDV_SUCCESS);

} /* end MDV_write_mdv */


/******************************************************************************
 * MDV_WRITE_ALL: Write out the handle structs and data to the given path.
 *
 * Inputs: mdv - handle containing the mdv structs and data
 *         file_path - path of output file. 
 *         output_encoding_type - format for the output field data.
 *
 * Note on chunk swapping: if the chunk type has a swapping routine
 * in the mdv library, it will be swapped. If not, it will not
 * be swapped.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

int MDV_write_all(MDV_handle_t *mdv,
                  char *file_path,
                  int output_encoding_type)

{

  int output_encoding;
  int output_compression;

  output_encoding = output_encoding_type;
  output_compression = MDV_COMPRESSION_NONE;

  if (output_encoding_type == MDV_PLANE_RLE8) {
    output_encoding = MDV_INT8;
    output_compression = MDV_COMPRESSION_RLE;
  }

  return (MDV_handle_write_all(mdv,
			       file_path,
			       output_encoding,
			       output_compression,
			       MDV_SCALING_ROUNDED,
			       0.0, 0.0));

}

/******************************************************************************
 * MDV_WRITE_MASTER_HEADER: Write an MDV master header to the file in the
 * appropriate place.
 *
 * Inputs: outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *         m_hdr - pointer to master header information.
 *
 * Outputs: outfile - updated on disk to contain the information in the
 *                    master header, byte swapped if necessary.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

int MDV_write_master_header(FILE *outfile, MDV_master_header_t *m_hdr)
{
  static char *routine_name = "MDV_write_master_header";
  
  MDV_master_header_t m_hdr_be = *m_hdr;
  
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
   * Move to the beginning of the file.
   */

  if (fseek(outfile, 0, SEEK_SET) != 0) 
  {
    fprintf(stderr,
            "%s: Error moving to beginning of output file.\n",
            routine_name);
    return(MDV_FAILURE);
  }
  
  /*
   * Make sure the header is in big-endian format.
   */

  MDV_master_header_to_BE(&m_hdr_be);
  
  /*
   * Write the header to the output file.
   */

  if (ufwrite(&m_hdr_be, sizeof(MDV_master_header_t), 1, outfile) != 1)
  {
    fprintf(stderr,
            "%s: Error writing master header to output file.\n",
            routine_name);
    return(MDV_FAILURE);
  }
  
  return(MDV_SUCCESS);
}


/******************************************************************************
 * MDV_WRITE_FIELD_HEADER: Write an MDV field header from the given 
 * structure into the proper place in the file.
 *
 * Inputs: outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *         f_hdr - pointer to field header information.
 *         field_num - the field number for this field.  This is used to
 *                     determine where in the file the header will be
 *                     written.
 *
 * Outputs: outfile - updated on disk to contain the information in the
 *                    field header, byte swapped if necessary.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

int MDV_write_field_header(FILE *outfile, MDV_field_header_t *f_hdr,
                           int field_num)
{
  static char *routine_name = "MDV_write_field_header";
  
  long hdr_offset;
  MDV_field_header_t f_hdr_be = *f_hdr;
  
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
   * Move to the appropriate position in the output file.
   */

  hdr_offset = sizeof(MDV_master_header_t) + 
    (field_num * sizeof(MDV_field_header_t));

  if (fseek(outfile, hdr_offset, SEEK_SET) != 0) 
  {
    fprintf(stderr,
            "%s: Error moving to field header %d position in output file.\n",
            routine_name, field_num);
    return(MDV_FAILURE);
  }

  /*
   * Make sure the header is in big-endian format.
   */

  MDV_field_header_to_BE(&f_hdr_be);
  
  /*
   * Write the header to the output file.
   */

  if (ufwrite(&f_hdr_be, sizeof(MDV_field_header_t), 1, outfile) != 1)
  {
    fprintf(stderr,
            "%s: Error writing field header %d to output file.\n",
            routine_name, field_num);
    return(MDV_FAILURE);
  }

  return(MDV_SUCCESS);

}


/******************************************************************************
 * MDV_WRITE_VLEVEL_HEADER: Write an MDV vlevel header from the given
 * structure into the proper place in the file.
 *
 * Inputs: outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *         v_hdr - pointer to vlevel header information.
 *         m_hdr - pointer to associated master header information.  This
 *                 information is used to position the vlevel header in
 *                 the output file.
 *         field_num - the number of the field associated with the vlevel
 *                     information.  This value is also used to position
 *                     the vlevel header in the output file.
 *
 * Outputs: outfile - updated on disk to contain the information in the
 *                    vlevel header, byte swapped if necessary.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */
 
int MDV_write_vlevel_header(FILE *outfile, MDV_vlevel_header_t *v_hdr,
                            MDV_master_header_t *m_hdr, int field_num)
{
  static char *routine_name = "MDV_write_vlevel_header";
  
  long hdr_offset;
  MDV_vlevel_header_t v_hdr_be = *v_hdr;
  
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
   * Move to the appropriate position in the output file.
   */

  hdr_offset = m_hdr->vlevel_hdr_offset + 
    (field_num * sizeof(MDV_vlevel_header_t));
 
  if (fseek(outfile, hdr_offset, SEEK_SET) != 0) 
  {
    fprintf(stderr,
            "%s: Error moving to vlevel header %d position in output file.\n",
            routine_name, field_num);
    return(MDV_FAILURE);
  }
  
  /*
   * Make sure the header is in big-endian format.
   */

  MDV_vlevel_header_to_BE(&v_hdr_be);
  
  /*
   * Write the header to the output file.
   */

  if (ufwrite(&v_hdr_be, sizeof(MDV_vlevel_header_t), 1, outfile) != 1)
  {
    fprintf(stderr,
            "%s: Error writing vlevel header %d to output file.\n",
            routine_name, field_num);
    return(MDV_FAILURE);
  }

  return(MDV_SUCCESS);
}


/******************************************************************************
 * MDV_WRITE_FIELD_VLEVEL_HEADER: Write an MDV field header and an MDV vlevel
 * header into a file.  Note, these are not written in the file sequentially.
 * The master header field and vlevel offsets MUST be correct!
 *
 * Inputs: outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *         fv_hdr - pointer to the field/vlevel header structure to be
 *                  written to disk.
 *         m_hdr - pointer to the associated master header information.
 *                 This information is used for positioning the field and
 *                 vlevel headers in the file.
 *         field_num - the number of the associated field.  This information
 *                     is also used for positioning the headers in the file.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */
 
int MDV_write_field_vlevel_header(FILE *outfile,
                                  MDV_field_vlevel_header_t *fv_hdr,
                                  MDV_master_header_t *m_hdr, int field_num)
{
  static char *routine_name = "MDV_write_field_vlevel_header";
  
  /*
   * The field header MUST be included.
   */

  if (fv_hdr->fld_hdr == NULL)
  {
    fprintf(stderr,
            "%s: Invalid field header pointer for field %d.\n",
            routine_name, field_num);
    return(MDV_FAILURE);
  }
  
  /*
   * Write the field header.
   */

  if (MDV_write_field_header(outfile, fv_hdr->fld_hdr, field_num)
      != MDV_SUCCESS)
    return(MDV_FAILURE);
  
  /*
   * Write the vlevel header, if included.
   */

  if (fv_hdr->vlv_hdr != NULL)
  {
    if (MDV_write_vlevel_header(outfile, fv_hdr->vlv_hdr,
                                m_hdr, field_num) != MDV_SUCCESS)
      return(MDV_FAILURE);
  }
 
  return(MDV_SUCCESS);
}
 

/******************************************************************************
 * MDV_WRITE_CHUNK: Write an MDV chunk header and data from the given
 * structures into the proper place in the file.  The data offset for
 * the chunk data must be specified by the caller in case the position
 * on disk is different from the position in memory (for example, if
 * the preceding field data was written in a compressed format but was
 * stored in memory in an uncompressed format.)
 *
 * Inputs: outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *         c_hdr - pointer to chunk header information.
 *         c_data - pointer to chunk data.  (This points to the beginning
 *                  of the actual data, not to any preceding FORTRAN
 *                  record length value unless the record length value
 *                  is to be included as part of the chunk data.  This
 *                  routine will put the FORTRAN record length fields
 *                  around the chunk data for you.)
 *         m_hdr - pointer to the associated master header information.
 *                 This information is used for positioning the chunk
 *                 header in the output file.
 *         chunk_num - the number of the chunk.  This information is also
 *                     used for positioning the chunk header in the file.
 *         chunk_data_offset - the offset into the output file where the
 *                             chunk data should be written.  This value
 *                             will be different from the value in the
 *                             chunk header if the field data in the file
 *                             is in a different format than the field data
 *                             in memory.
 *         swap_chunk_data - flag indicating if this routine should try to
 *                           swap the chunk data.  If TRUE, the data will
 *                           be swapped, if necessary, so that the data on
 *                           disk is written in big-endian format.  If
 *                           FALSE, the data will be written to disk as is.
 *                           Note that this routine can only swap the chunk
 *                           data if it is in a known format.
 *
 * Outputs: outfile - updated on disk to contain the information in the
 *                    chunk header, byte swapped if necessary, and the
 *                    chunk data.  The chunk data offset value on disk
 *                    will be the value passed into the routine rather
 *                    than the value in c_hdr.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */
 
int MDV_write_chunk(FILE *outfile, MDV_chunk_header_t *c_hdr,
                    void *c_data,
                    MDV_master_header_t *m_hdr, int chunk_num,
                    int chunk_data_offset,
                    int swap_chunk_data)
{
  static char *routine_name = "MDV_write_chunk";
  
  MDV_chunk_header_t output_c_hdr;
  
  /*
   * Fill in the chunk header information.  Update the offset value
   * to match that for the output rather than that for the dataset
   * in memory.
   */

  output_c_hdr = *c_hdr;
  output_c_hdr.chunk_data_offset = chunk_data_offset;
  
  /*
   * Write the header information.  The header is positioned in the
   * output file based on the information in the master header.
   */

  if (MDV_write_chunk_header(outfile, &output_c_hdr,
                             m_hdr, chunk_num) != MDV_SUCCESS)
  {
    fprintf(stderr,
            "%s: Error writing chunk header %d, chunk_id = %d.\n",
            routine_name, chunk_num, output_c_hdr.chunk_id);
    return(MDV_FAILURE);
  }

  /*
   * Write the chunk data.
   */

  if (MDV_write_chunk_data(outfile,
                           output_c_hdr.chunk_data_offset,
                           c_data,
                           output_c_hdr.size,
                           output_c_hdr.chunk_id,
                           swap_chunk_data) != MDV_SUCCESS)
  {
    fprintf(stderr,
            "%s: Error writing data for chunk %d\n",
            routine_name, chunk_num);
    return(MDV_FAILURE);
  }

  return(MDV_SUCCESS);
}
 

/******************************************************************************
 * MDV_WRITE_CHUNK_HEADER: Write an MDV chunk header from the given
 * structure into the proper place in the file.
 *
 * Inputs: outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *         c_hdr - pointer to chunk header information.
 *         m_hdr - pointer to the associated master header information.
 *                 This information is used for positioning the chunk
 *                 header in the file.
 *         chunk_num - the chunk number.  This information is also used for
 *                     positioning the header in the file.
 *
 * Outputs: outfile - updated on disk to contain the information in the
 *                    chunk header, byte swapped if necessary.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */
 
int MDV_write_chunk_header(FILE *outfile, MDV_chunk_header_t *c_hdr,
                           MDV_master_header_t *m_hdr, int chunk_num)
{
  static char *routine_name = "MDV_write_chunk_header";
  
  long hdr_offset;
  MDV_chunk_header_t c_hdr_be = *c_hdr;
  
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
   * Move to the appropriate position in the output file.
   */

  hdr_offset = m_hdr->chunk_hdr_offset + 
    (chunk_num * sizeof(MDV_chunk_header_t));
 
  if (fseek(outfile, hdr_offset, SEEK_SET) != 0) 
  {
    fprintf(stderr,
            "%s: Error moving to chunk header %d position in output file.\n",
            routine_name, chunk_num);
    return (MDV_FAILURE);
  }
  
  /*
   * Convert the header to big-endian format.
   */

  MDV_chunk_header_to_BE(&c_hdr_be);
  
  /*
   * Write the header to the output file.
   */

  if (ufwrite(&c_hdr_be, sizeof(MDV_chunk_header_t), 1, outfile) != 1)
  {
    fprintf(stderr,
            "%s: Error writing chunk header %d to output file.\n",
            routine_name, chunk_num);
    return(MDV_FAILURE);
  }

  return(MDV_SUCCESS);
}


/******************************************************************************
 * MDV_write_field_vol
 *
 * For an open file, write the volume data for the given field.
 *
 * The field header must match the volume data. It must be in
 * host byte order. The volume_size and field_data_offset must
 * be correct in the header.
 *
 * You must specify output_encoding, output_compression and output_scaling
 * types. See <Mdv/mdv/mdv_file.h>
 *
 * A copy of the field header is updated to reflect the output types
 * and sizes and the data offset and is written to the file.
 *
 * Returns: 0 on success, -1 on failure.
 *
 * Side effect: if output_volume_len is not NULL the length of the 
 *              field buffer is copied to it
 */

int MDV_write_field_vol(FILE *outfile,
			MDV_field_header_t *f_hdr,
			int field_num,
			void *field_data,
			int output_encoding,
			int output_compression,
			int output_scaling,
			double output_scale,
			double output_bias,
			int *output_volume_size)
     
{

  MDV_field_handle_t *fhand;

  /*
   * create a field_handle, write in the field
   */
  
  fhand = MDV_fhand_create_from_parts(f_hdr, field_data);

  /*
   * convert the data as appropriate
   */
  
  if (MDV_fhand_convert(fhand, 
			output_encoding, output_compression,
			output_scaling, output_scale, output_bias)) {
    fprintf(stderr, "ERROR - MDV_write_field_vol\n");
    fprintf(stderr, "  Cannot convert field '%s'\n", f_hdr->field_name);
    MDV_fhand_delete(fhand);
    return (-1);
  }

  /*
   * write field data
   */
  
  if (MDV_fhand_write_vol(fhand, outfile)) {
    MDV_fhand_delete(fhand);
    return (-1);
  }

  /*
   * set the output len
   */

  if (output_volume_size != NULL) {
    *output_volume_size = MDV_fhand_get_hdr(fhand)->volume_size;
  }

  /*
   * write the field header
   */

  if (MDV_write_field_header(outfile, MDV_fhand_get_hdr(fhand),
			     field_num) != MDV_SUCCESS) {
    MDV_fhand_delete(fhand);
    return (-1);
  }

  /*
   * free up
   */

  MDV_fhand_delete(fhand);

  return(0);

}

/******************************************************************************
 * MDV_WRITE_FIELD: Write the MDV field header and data from the given 
 * structure into the proper place in the file.  The position of the
 * field header is calculated from the field number and the position of
 * the field data is passed in since this position may be different from
 * the position of the data in the dataset in memory if the data is being
 * output in a format different from that in memory.
 *
 * Inputs: outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *         f_hdr - pointer to the field header information.
 *         f_data - pointer to the field data.  The format of this data
 *                  must match the description given in the field header.
 *         field_num - the field number for this field information.  This
 *                     is used for positioning the field header in the
 *                     output file and for error messages.
 *         field_data_offset - offset in the output file where the field
 *                             data should begin.  This may differ from
 *                             the field data offset in the header if the
 *                             data is being written in a different format
 *                             than it is being stored in memory.
 *         output_encoding_type - format for the output field data.
 *
 * Outputs: outfile - updated on disk to contine the information in the
 *                    field header and the field data.  The field header
 *                    information on disk contains the field data offset
 *                    and encoding type of the data as it is written to
 *                    disk rather than that  f_hdr.
 *
 * Returns: the number of bytes written to the file for the field data, or
 *          -1 on error.
 */

int MDV_write_field(FILE *outfile,
		    MDV_field_header_t *f_hdr,
                    void *f_data,
                    int field_num,
                    int field_data_offset,
                    int output_encoding_type)
{

  return (MDV_write_field_data(f_hdr,
			       field_num,
			       field_data_offset,
			       f_data,
			       output_encoding_type,
			       outfile));

}

/******************************************************************************
 * MDV_WRITE_FIELD_DATA
 *
 *  Identical in function to MDV_write_field().
 * 
 *  Write a whole volume of field data.
 *  Write the relevant field header.
 *
 * Inputs: fhdr - pointer to the field header information.
 *         field_num - the number of the field being written.
 *         field_data_offset - the offset into the output file where the
 *                             field data should be written.  This value
 *                             may be different from the value in the field
 *                             header if the data is being written in a format
 *                             different from its format in memory.
 *         fld_data_ptr - pointer to the beginning of the field data.
 *         output_encoding_type - format to use for the output data.
 *         outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *
 * Outputs: outfile - updated on disk to contain the field information in
 *                    the indicated format, byte swapped if necessary.
 *
 * Returns: the output size for the field data written or -1 on error.
 */
 
int MDV_write_field_data(MDV_field_header_t *fhdr,
			 int field_num,
                         int field_data_offset,
                         void *fld_data_ptr,
			 int output_encoding_type,
                         FILE *outfile) 
{

  MDV_field_header_t fhdr_copy = *fhdr;
  int encoding_type = output_encoding_type;
  int compression_type = MDV_COMPRESSION_NONE;
  int output_volume_size = 0;

  if (encoding_type == MDV_PLANE_RLE8) {
    encoding_type = MDV_INT8;
    compression_type = MDV_COMPRESSION_RLE;
  }
  
  fhdr_copy.field_data_offset = field_data_offset;

  if (MDV_write_field_vol(outfile, &fhdr_copy,
			  field_num, fld_data_ptr,
			  encoding_type, compression_type,
			  MDV_SCALING_ROUNDED, 0.0, 0.0,
			  &output_volume_size)) {
    return (0);
  } else {
    return (output_volume_size);
  }

}

/******************************************************************************
 * MDV_WRITE_CHUNK_DATA: Write out the chunk data to the given file.
 *
 * Inputs: outfile - pointer to the output file.  Assumed to be currently
 *                   open for write.
 *         offset - offset into the output file where the chunk data should
 *                  be written.  This offset should point to the position
 *                  of the chunk data itself.  FORTRAN record length values
 *                  will written before and after this data on disk.
 *         c_data - pointer to the chunk data to be written.
 *         size - size in bytes of the chunk data.  This value does not
 *                include the surrounding FORTRAN record length fields.
 *         chunk_id - chunk identifier.  If this is a chunk type known to
 *                    the library, then the chunk information may be byte
 *                    swapped when written, if necessary.
 *         swap_flag - flag indicating whether this routine should try to
 *                     byte swap the chunk data before writing.  If TRUE,
 *                     the data will be byte swapped, if necessary, if the
 *                     chunk type is known by the library.  If FALSE, the
 *                     chunk data is written as is to the file.
 *
 * Outputs: outfile - updated on disk to contain the chunk information,
 *                    byte swapped if necessary and requested.
 *
 * Returns: MDV_SUCCESS or MDV_FAILURE.
 */

int MDV_write_chunk_data(FILE *outfile, long offset,
                         void *c_data, long size,
                         long chunk_id, int swap_flag)
{
  static char *routine_name = "MDV_write_chunk_data";

  int buflen;
  ui08 *c_data_be;
  ui32 ssize = size;

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
   * Move to the appropriate position in the output file.
   */
  
  if (fseek(outfile, offset - sizeof(ui32), SEEK_SET) != 0) 
  {
    fprintf(stderr,
            "%s: Error moving to chunk data (id = %ld) "
            "position in output file.\n",
            routine_name, chunk_id);
    return (MDV_FAILURE);
  }
  
  /*
   * allocate buffer for chunk plus fortran record lengths
   */

  buflen = size + 2 * sizeof(ui32);
  c_data_be = (ui08 *) umalloc(buflen);

  /*
   * copy in the record lenths at each end of buffer
   */

  memcpy(c_data_be, &ssize, sizeof(ui32));
  memcpy(c_data_be + sizeof(ui32) + size, &ssize, sizeof(ui32));

  /*
   * copy in the chunk data
   */

  memcpy(c_data_be + sizeof(ui32), c_data, size);

  /*
   * convert to BE as required
   */
  
  if (swap_flag)
  {
    MDV_chunk_data_to_BE(c_data_be, size, chunk_id);
  }
  
  /*
   * Write the chunk
   */
  
  if (ufwrite(c_data_be, buflen, 1, outfile) != 1)
  {
    fprintf(stderr,
            "%s: Error writing chunk data for chunk id %ld.\n",
            routine_name, chunk_id);
    ufree(c_data_be);
    return(MDV_FAILURE);
  }

  ufree(c_data_be);
  
  return(MDV_SUCCESS);

} /* end MDV_write_chunk_data */

