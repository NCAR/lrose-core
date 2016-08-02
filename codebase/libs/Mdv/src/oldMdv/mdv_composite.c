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
 *  MDV_COMPOSITE.C  Subroutines for compositing MDV data.
 *
 *  N. Rehak, Nov. 1998
 */

#include <stdio.h>
#include <stdlib.h>

#include <toolsa/os_config.h>

#include <toolsa/mem.h>
#include <Mdv/mdv/mdv_composite.h>
#include <Mdv/mdv/mdv_handle.h>
#include <Mdv/mdv/mdv_macros.h>
#include <Mdv/mdv/mdv_print.h>


/*
 * Forward declarations to static routines
 */

static int composite_max(MDV_handle_t *mdv_handle);

  
/******************************************************************************
 * MDV_composite_data(): Composites the given MDV data using the given
 *                       composition technique.
 *
 * Inputs: mdv_handle - handle for the MDV data to be composited.
 *         composite_type - composition technique to use
 *                            (e.g. MDV_COMPOSITE_MAX).
 *
 * Outputs: mdv_handle - data is updated to contain composite rather
 *                         than original data.
 *
 * Returns: MDV_SUCCESS on success, MDV_FAILURE on failure.
 */

int MDV_composite_data(MDV_handle_t *mdv_handle,
		       int composite_type)
{
  static char *routine_name = "MDV_composite_data";
  
  switch (composite_type)
  {
  case MDV_COMPOSITE_NONE :
    /*
     * Do nothing.
     */
    break;
    
  case MDV_COMPOSITE_MAX :
    return(composite_max(mdv_handle));
    break;
  
  default:
    fprintf(stderr, "ERROR: mdv_composite::%s\n", routine_name);
    fprintf(stderr, "Invalid composite type %d requested\n", composite_type);
    return(MDV_FAILURE);
  } /* endswitch - composite_type */
  
  return(MDV_SUCCESS);
}


/********************************************************
 * STATIC ROUTINES
 ********************************************************/

/******************************************************************************
 * composite_max(): Composites the given MDV data using the maximum data value
 *                  for each data column.
 *
 * Inputs: mdv_handle - handle for the MDV data to be composited.
 *
 * Outputs: mdv_handle - data is updated to contain composite rather
 *                         than original data.
 *
 * Returns: MDV_SUCCESS on success, MDV_FAILURE on failure.
 */

static int composite_max(MDV_handle_t *mdv_handle)
{
  static char *routine_name = "composite_max";
  
  int field;
  
  /*
   * Check for errors.
   */

  for (field = 0; field < mdv_handle->master_hdr.n_fields; field++)
  {
    if (mdv_handle->fld_hdrs[field].encoding_type != MDV_INT8)
    {
      fprintf(stderr, "ERROR: mdv_composite::%s\n", routine_name);
      fprintf(stderr, "Compositing encoding type %s not yet implemented\n",
	      MDV_encode2string(mdv_handle->fld_hdrs[field].encoding_type));
      
      return(MDV_FAILURE);
    }
  }
  
  /*
   * Composite each field separately.
   */

  for (field = 0; field < mdv_handle->master_hdr.n_fields; field++)
  {
    MDV_field_header_t *field_hdr = &mdv_handle->fld_hdrs[field];
    
    int plane_size = field_hdr->nx * field_hdr->ny;
    
    ui08 *composite_data = (ui08 *)umalloc(plane_size * sizeof(ui08));
    
    int i;
    int plane;
    
    /*
     * Composite the data.
     */

    for (i = 0; i < plane_size; i++)
    {
      composite_data[i] = (ui08)field_hdr->missing_data_value;
      
      for (plane = 0; plane < field_hdr->nz; plane++)
      {
	ui08 plane_data = *((ui08 *)mdv_handle->field_plane[field][plane] + i);
	
	if (plane_data != (ui08)field_hdr->missing_data_value &&
	    plane_data != (ui08)field_hdr->bad_data_value)
	{
	  if (composite_data[i] == (ui08)field_hdr->missing_data_value)
	    composite_data[i] = plane_data;
	  else if (plane_data < composite_data[i])
	    plane_data = composite_data[i];
	}
      } /* endfor - plane */
    } /* endfor - i */
    
    /*
     * Update the field information in the handle.
     */

    for (plane = 0; plane < field_hdr->nz; plane++)
    {
      ufree(mdv_handle->field_plane[field][plane]);
      mdv_handle->field_plane[field][plane] = (void *)NULL;
    }
    mdv_handle->field_plane[field][0] = (void *)composite_data;
    
    field_hdr->nz = 1;
    
  } /* endfor - field */
  
  /*
   * Update the master header fields.
   */

  mdv_handle->master_hdr.data_dimension = 2;
  
  return(MDV_SUCCESS);
}
