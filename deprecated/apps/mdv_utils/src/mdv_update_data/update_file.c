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
 * update_file.c
 *
 * The data values in the given file.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <stdio.h>
#include <sys/stat.h>

#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <mdv/mdv_file.h>
#include <mdv/mdv_handle.h>
#include <mdv/mdv_macros.h>
#include <mdv/mdv_read.h>
#include <mdv/mdv_write.h>
#include <toolsa/globals.h>

#include "mdv_update_data.h"


void update_file(char *input_file_path)
{
  static char *routine_name = "update_file";
  
  static int first_call = TRUE;
  static MDV_handle_t mdv_handle;
  
  int field_num;
  int plane_num;
  int i;
  
  char *slash_pos;
  char output_file_path[MAX_PATH_LEN];
  
  fprintf(stderr, "Updating file %s\n", input_file_path);

  /*
   * Initialize the handle on the first call.
   */

  if (first_call)
  {
    if (MDV_init_handle(&mdv_handle) != 0)
    {
      fprintf(stderr, "ERROR - %s:%s\n",
	      Glob->prog_name, routine_name);
      fprintf(stderr,
	      "Error initializing MDV handle structure -- EXITING\n");
      
      tidy_and_exit(-1);
    }
    
    first_call = FALSE;
  }
  
  /*
   * Read in the input file.
   */

  if (MDV_read_all(&mdv_handle,
		   input_file_path,
		   Glob->mdv_data_type)!= 0)
  {
    fprintf(stderr, "ERROR - %s:%s\n",
	    Glob->prog_name, routine_name);
    fprintf(stderr,
	    "Error reading MDV file <%s> -- SKIPPING FILE\n",
	    input_file_path);
    
    return;
  }
  
  /*
   * Run through the data, changing data values where necessary.
   */

  for (field_num = 0; field_num < mdv_handle.master_hdr.n_fields; field_num++)
  {
    MDV_field_header_t *fld_hdr = &mdv_handle.fld_hdrs[field_num];
    
    for (plane_num = 0; plane_num < fld_hdr->nz; plane_num++)
    {
      if (Glob->params.debug_flag)
	fprintf(stderr,
		"Field %d has %d bytes per plane:\n",
		field_num, fld_hdr->nx * fld_hdr->ny);
      
      for (i = 0; i < fld_hdr->nx * fld_hdr->ny; i++)
      {
	switch(Glob->mdv_data_type)
	{
	case MDV_INT8 :
	{
	  ui08 *data_ptr =
	    (ui08 *)((char *)mdv_handle.field_plane[field_num][plane_num] +
		     sizeof(ui08) * i);
	  
	  if (*data_ptr == (ui08)Glob->params.old_data_value)
	    *data_ptr = (ui08)Glob->params.new_data_value;
	  
	  break;
	} /* endcase - MDV_INT8 */
	  
	case MDV_INT16 :
	{
	  ui16 *data_ptr =
	    (ui16 *)((char *)mdv_handle.field_plane[field_num][plane_num] +
		     sizeof(ui16) * i);
	  
	  if (*data_ptr == (ui16)Glob->params.old_data_value)
	    *data_ptr = (ui16)Glob->params.new_data_value;
	  
	  break;
	} /* endcase - MDV_INT16 */
	  
	case MDV_FLOAT32 :
	{
	  fl32 *data_ptr =
	    (fl32 *)((char *)mdv_handle.field_plane[field_num][plane_num] +
		     sizeof(fl32) * i);
	  
	  if (*data_ptr == (fl32)Glob->params.old_data_value)
	    *data_ptr = (fl32)Glob->params.new_data_value;
	  
	  break;
	} /* endcase - MDV_FLOAT32 */
	  
	} /* endswitch - Glob->mdv_data_type */
	
      } /* endfor - i */
    } /* endfor - plane_num */
  } /* endfor - field_num */
  

  /*
   * Write the output file
   */

  slash_pos = strrchr(input_file_path, '/');
  
  if (slash_pos == NULL)
    sprintf(output_file_path, "%s/%s",
	    Glob->params.output_dir, input_file_path);
  else
    sprintf(output_file_path, "%s/%s",
	    Glob->params.output_dir, (slash_pos+1));
  
  if (MDV_write_all(&mdv_handle,
		    output_file_path,
		    Glob->mdv_output_type) != MDV_SUCCESS)
  {
    fprintf(stderr, "ERROR - %s:%s\n",
	    Glob->prog_name, routine_name);
    fprintf(stderr,
	    "Error writing MDV info to file <%s> -- SKIPPING FILE\n",
	    output_file_path);
    
    return;
  }
  
  fprintf(stderr, "Wrote file %s\n", output_file_path);

  return;
}

