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
/*********************************************************************
 * create_file.c
 *
 * Cuts the number of vertical levels down in the mdv file and writes
 * a new mdv file
 *
 * RAP, NCAR, Boulder CO
 *
 * February 1999
 *
 * Jaimi Yee
 *
 *********************************************************************/

#include <stdio.h>
#include <sys/stat.h>

#include <mdv/mdv_file.h>
#include <mdv/mdv_handle.h>
#include <mdv/mdv_read.h>
#include <mdv/mdv_write.h>
#include <toolsa/file_io.h>
#include <toolsa/str.h>
#include <toolsa/mem.h>

#include "mdv_update_levels.h"


void create_file(char *input_file_name, char *subdir)
{
  static char *routine_name = "update_file";
  
  static int first_call = TRUE;
  static MDV_handle_t mdv_handle;

  FILE *infile;
  void *plane;
  MDV_field_header_t *fld;
  int ifield, ichunk, ilevel;
  
  char input_file_path[MAX_PATH_LEN];
  char output_dir_path[MAX_PATH_LEN];
  char output_file_path[MAX_PATH_LEN];
  char output_file_name[MAX_PATH_LEN];
  char *ext_position;

  struct stat dir_stat;

  /*
   * Create the input file path
   */
  sprintf( input_file_path, "%s/%s/%s", 
           Glob->params.input_dir, subdir, input_file_name );

  /*
   * Initialize the handle on the first call.
   */

  if (first_call)
  {
    if (MDV_init_handle(&mdv_handle) != 0)
    {
      fprintf(stderr, "ERROR - %s:%s\n", Glob->prog_name, routine_name);
      fprintf(stderr, "Error initializing MDV handle structure\n");
      tidy_and_exit(-1);
    }
    
    first_call = FALSE;
  }


  /*
   * check that file is an MDV file
   */

  if (!MDV_verify(input_file_path)) {
    fprintf(stderr, "ERROR - %s:%s\n", Glob->prog_name, routine_name);
    fprintf(stderr, "File %s is not MDV format\n", input_file_path);
    tidy_and_exit(-1);
  }
  
  /*
   * Open the input file.
   */

  if ((infile = ta_fopen_uncompress(input_file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:%s\n", Glob->prog_name, routine_name);
    fprintf(stderr, "Cannot open input file\n");
    perror(input_file_path);
    tidy_and_exit(-1);
  }
  
  /*
   * Read the master header.
   */
  
  if (MDV_load_master_header(infile, &mdv_handle.master_hdr) != MDV_SUCCESS) {
    fprintf(stderr, "ERROR - %s:%s\n", Glob->prog_name, routine_name);
    fprintf(stderr, "reading master header from input file <%s>\n", 
            input_file_path);
    fclose(infile);
    tidy_and_exit(-1);
  }

  /*
   * Modify number of vertical levels in master header
   */
  mdv_handle.master_hdr.max_nz = Glob->params.num_output_levels;

  /*
   * alloc arrays
   */

  MDV_alloc_handle_arrays(&mdv_handle,
			  mdv_handle.master_hdr.n_fields,
			  mdv_handle.master_hdr.max_nz,
			  mdv_handle.master_hdr.n_chunks);
  
  /*
   * Read field headers.
   */

  for (ifield = 0; ifield < mdv_handle.master_hdr.n_fields; ifield++) {
    if (MDV_load_field_header(infile, mdv_handle.fld_hdrs + ifield,
			      ifield) != MDV_SUCCESS) {
      fprintf(stderr, "ERROR - %s:%s\n", Glob->prog_name, routine_name);
      fprintf(stderr, "reading field %d header from input file <%s>\n", 
              ifield, input_file_path);
      fclose(infile);
      tidy_and_exit(-1);
    }
  }

  /*
   * Update field headers
   */
  for (ifield = 0; ifield < mdv_handle.master_hdr.n_fields; ifield++) {
     mdv_handle.fld_hdrs[ifield].nz = Glob->params.num_output_levels;
  }

  /*
   * Do not include vlevel header
   */
  mdv_handle.master_hdr.vlevel_included = 0;
  mdv_handle.master_hdr.vlevel_hdr_offset = 0;

  /*
   * load up the grid
   */

  if (mdv_handle.master_hdr.n_fields > 0) {
    MDV_load_grid_from_hdrs(&mdv_handle.master_hdr, mdv_handle.fld_hdrs,
			    &mdv_handle.grid);
  }
  
  /*
   * Read each chunk header and the associated data.
   */

  for (ichunk = 0; ichunk < mdv_handle.master_hdr.n_chunks; ichunk++) {

    if (MDV_load_chunk_header(infile, mdv_handle.chunk_hdrs + ichunk,
			      &mdv_handle.master_hdr,
			      ichunk) != MDV_SUCCESS) {
      fprintf(stderr, "ERROR - %s:%s\n", Glob->prog_name, routine_name);
      fprintf(stderr, "loading chunk %d header from input file <%s>\n", 
              ichunk, input_file_path);
      fclose(infile);
      tidy_and_exit(-1);
    }

    if (mdv_handle.chunk_data_allocated && mdv_handle.chunk_data[ichunk]) {
      ufree(mdv_handle.chunk_data[ichunk]);
      mdv_handle.chunk_data[ichunk] = NULL;
    }
    if ((mdv_handle.chunk_data[ichunk] =
	 MDV_get_chunk_data(infile, mdv_handle.chunk_hdrs + ichunk)) == NULL)	{
      fprintf(stderr, "ERROR - %s:%s\n", Glob->prog_name, routine_name);
      fprintf(stderr, "reading chunk %d data from input file <%s>\n", 
              ichunk, input_file_path);
      fclose(infile);
      tidy_and_exit(-1);
    }
	
  } /* ichunk */

  if (mdv_handle.master_hdr.n_chunks > 0) {
    mdv_handle.chunk_data_allocated = TRUE;
  }
  
  /*
   * load radar data structs if applicable
   */

  MDV_handle_load_radar_structs(&mdv_handle);

  /*
   * read in planes
   */

  for (ifield = 0; ifield < mdv_handle.master_hdr.n_fields; ifield++) {

    fld = mdv_handle.fld_hdrs + ifield;

    fld->volume_size = 0;
    
    for (ilevel = 0; ilevel < Glob->params.num_output_levels; ilevel++) {
      int plane_size;
      
      plane = MDV_get_plane_size(infile, fld, Glob->mdv_data_type,
				 ilevel, &plane_size);
    
      if (plane == NULL) {
        fprintf(stderr, "ERROR - %s:%s\n", Glob->prog_name, routine_name);
	fprintf(stderr, "reading field %d, level %d from input file <%s>\n", 
                ifield, ilevel, input_file_path);
	fclose(infile);
	tidy_and_exit(-1);
      }
      
      if (mdv_handle.field_plane[ifield][ilevel] && 
          mdv_handle.field_planes_allocated) {
	ufree(mdv_handle.field_plane[ifield][ilevel]);
      }
      mdv_handle.field_plane[ifield][ilevel] = plane;
      fld->volume_size += plane_size;
      
    } /* ilevel */
	
    if (Glob->mdv_data_type != MDV_NATIVE)
      fld->encoding_type = Glob->mdv_data_type;

  } /* ifield */

  if (mdv_handle.master_hdr.n_fields > 0) {
    mdv_handle.field_planes_allocated = TRUE;
  }

  fclose(infile);

  mdv_handle.read_all_done = TRUE;
  
  /*
   * Create the subdirectory if necessary
   */

  sprintf(output_dir_path, "%s/%s",
          Glob->params.output_dir, subdir);
   
  if (stat(output_dir_path, &dir_stat)) 
  {
     if (mkdir(output_dir_path, 0755)) 
     {
         fprintf(stderr, "ERROR - %s::%s\n", Glob->prog_name, routine_name);
         fprintf(stderr, "Trying to make output dir\n");
         perror(output_dir_path);
     }
  }

  /*
   * If the input file was compressed, take the .gz off the
   * output file name, because it will not be compressed
   */

  if( (ext_position = strstr( input_file_name, ".gz")) != (char *)NULL ) {
     strncpy( output_file_name, input_file_name, 
              (int) (ext_position - input_file_name) );
     output_file_name[(int) (ext_position - input_file_name)] = '\0';
     sprintf( output_file_path, "%s/%s/%s",
	      Glob->params.output_dir, subdir, output_file_name );
  } else {
     sprintf( output_file_path, "%s/%s/%s",
	      Glob->params.output_dir, subdir, input_file_name );
  }
  
  /*
   * Write the file
   */ 

  if (MDV_write_all(&mdv_handle,
		    output_file_path,
		    Glob->mdv_output_type) != MDV_SUCCESS)
  {
    fprintf(stderr, "ERROR - %s:%s\n", Glob->prog_name, routine_name);
    fprintf(stderr, "Error writing MDV info to file <%s>",
	    output_file_path);
    fprintf(stderr, " -- SKIPPING FILE\n");
    return;
  }
  
  if( Glob->params.debug )
     fprintf(stderr, "Wrote file %s\n\n", output_file_path);

  return;
}

