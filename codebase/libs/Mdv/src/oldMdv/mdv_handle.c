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
/*************************************************************************
 *
 * mdv_handle.c
 *
 * Routines for handle utilities
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * May 1997
 *
 **************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <dataport/swap.h>
#include <rapformats/dobson.h>
#include <toolsa/umisc.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>

#include <Mdv/mdv/mdv_handle.h>
#include <Mdv/mdv/mdv_print.h>
#include <Mdv/mdv/mdv_read.h>
#include <Mdv/mdv/mdv_user.h>
#include <Mdv/mdv/mdv_utils.h>

#include "mdv_private.h"

static int crop_latlon_planes(MDV_handle_t *mdv,
			      int field_num,
			      double min_lat, double max_lat,
			      double min_lon, double max_lon);

static void *crop_unencoded_plane(int min_x, int min_y,
				  int max_x, int max_y,
				  int nx, int ny,
				  int elem_size,
				  void *data,
				  int *plane_size_p);

static void free_chunk_data(MDV_handle_t *mdv);


/*************************************************************************
 *
 * MDV_init_handle
 *
 * initializes the memory associated with handle
 *
 * Returns 0 on success, -1 on failure.
 *
 **************************************************************************/

int MDV_init_handle(MDV_handle_t *mdv)

{
  
  memset(mdv, 0, sizeof(*mdv));
  DsRadarElev_init(&mdv->radarElevs);
  return (0);

}

/*************************************************************************
 *
 * MDV_free_handle
 *
 * Frees the memory associated with handle
 *
 **************************************************************************/

void MDV_free_handle(MDV_handle_t *mdv)

{

  DsRadarElev_free(&mdv->radarElevs);

  if (mdv->field_plane != NULL) {
    MDV_handle_free_field_planes(mdv);
    ufree2((void **) mdv->field_plane);
  }

  if (mdv->field_plane_len != NULL) {
    ufree2((void **) mdv->field_plane_len);
  }

  free_chunk_data(mdv);
  ufree(mdv->chunk_data);
  ufree(mdv->chunk_hdrs);

  ufree(mdv->vlv_hdrs);
  ufree(mdv->fld_hdrs);

  memset(mdv, 0, sizeof(*mdv));

}

/*************************************************************************
 *
 * MDV_alloc_handle_arrays()
 *
 * allocates memory for handle arrays
 *
 **************************************************************************/

void MDV_alloc_handle_arrays(MDV_handle_t *mdv,
			     int n_fields,
			     int n_levels,
			     int n_chunks)
     
{

  /*
   * field headers and vlevel headers
   */

  if (n_fields != mdv->n_fields_alloc) {

    if (mdv->fld_hdrs != NULL) {
      ufree(mdv->fld_hdrs);
    }

    mdv->fld_hdrs = (MDV_field_header_t *) umalloc
      (n_fields * sizeof(MDV_field_header_t));

    if (mdv->vlv_hdrs != NULL) {
      ufree(mdv->vlv_hdrs);
    }

    mdv->vlv_hdrs = (MDV_vlevel_header_t *) umalloc
      (n_fields * sizeof(MDV_vlevel_header_t));

  }

  /*
   * chunks
   */
  
  free_chunk_data(mdv);

  if (n_chunks != mdv->n_chunks_alloc) {

    if (mdv->chunk_hdrs != NULL) {
      ufree(mdv->chunk_hdrs);
    }
    
    if (mdv->chunk_data != NULL) {
      ufree(mdv->chunk_data);
    }
    
    mdv->chunk_hdrs = (MDV_chunk_header_t *) umalloc
      (n_chunks * sizeof(MDV_chunk_header_t));

    mdv->chunk_data = (void **) umalloc
      (n_chunks * sizeof(void *));

  }

  /*
   * field plane pointers
   */
  
  MDV_handle_free_field_planes(mdv);
  
  if (n_fields != mdv->n_fields_alloc ||
      n_levels != mdv->n_levels_alloc) {

    if (mdv->field_plane != NULL) {
      ufree2((void **) mdv->field_plane);
    }

    if (mdv->field_plane_len != NULL) {
      ufree2((void **) mdv->field_plane_len);
    }

    mdv->field_plane = (void ***) ucalloc2
      (n_fields, n_levels, sizeof(void *));
				 
    mdv->field_plane_len = (int **) ucalloc2
      (n_fields, n_levels, sizeof(int));
				 
  }

  mdv->n_fields_alloc = n_fields;
  mdv->n_levels_alloc = n_levels;
  mdv->n_chunks_alloc = n_chunks;
  
}

/*************************************************************************
 *
 * MDV_realloc_handle_arrays()
 *
 * reallocates memory for handle arrays saving current information
 *
 **************************************************************************/

void MDV_realloc_handle_arrays(MDV_handle_t *mdv,
			       int n_fields,
			       int n_levels,
			       int n_chunks)
{
  /*
   * Make sure we keep any extra information
   */

  if (n_fields < mdv->n_fields_alloc)
    n_fields = mdv->n_fields_alloc;
    
  if (n_levels < mdv->n_levels_alloc)
    n_levels = mdv->n_levels_alloc;
    
  if (n_chunks < mdv->n_chunks_alloc)
    n_chunks = mdv->n_chunks_alloc;
  

  /*
   * field headers and vlevel headers
   */

  if (n_fields > mdv->n_fields_alloc)
  {
    if (mdv->fld_hdrs == (MDV_field_header_t *)NULL)
    {
      mdv->fld_hdrs =
	(MDV_field_header_t *)umalloc(n_fields * sizeof(MDV_field_header_t));
    }
    else
    {
      mdv->fld_hdrs =
	(MDV_field_header_t *)urealloc(mdv->fld_hdrs,
				       n_fields * sizeof(MDV_field_header_t));
    }

    if (mdv->vlv_hdrs == (MDV_vlevel_header_t *)NULL)
    {
      mdv->vlv_hdrs =
	(MDV_vlevel_header_t *)umalloc(n_fields * sizeof(MDV_vlevel_header_t));
    }
    else
    {
      mdv->vlv_hdrs =
	(MDV_vlevel_header_t *)urealloc(mdv->vlv_hdrs,
					n_fields * sizeof(MDV_vlevel_header_t));
    }
  }

  /*
   * chunks
   */
  
  if (n_chunks > mdv->n_chunks_alloc)
  {
    if (mdv->chunk_hdrs == (MDV_chunk_header_t *)NULL)
    {
      mdv->chunk_hdrs =
	(MDV_chunk_header_t *)umalloc(n_chunks * sizeof(MDV_chunk_header_t));
    }
    else
    {
      mdv->chunk_hdrs =
	(MDV_chunk_header_t *)urealloc(mdv->chunk_hdrs,
				       n_chunks * sizeof(MDV_chunk_header_t));
    }
    
    if (mdv->chunk_data == (void **)NULL)
    {
      mdv->chunk_data = (void **)umalloc(n_chunks * sizeof(void *));
    }
    else
    {
      mdv->chunk_data = (void **)urealloc(mdv->chunk_data,
					  n_chunks * sizeof(void *));
    }
  }

  /*
   * field plane pointers
   */
  
  if (n_fields > mdv->n_fields_alloc ||
      n_levels > mdv->n_levels_alloc)
  {
    int ifield;
    int iplane;
    
    if (mdv->field_plane == (void ***)NULL)
    {
      mdv->field_plane =
	(void ***)ucalloc2(n_fields, n_levels, sizeof(void *));
    }
    else
    {
      mdv->field_plane =
	(void ***)urealloc2((void **)mdv->field_plane,
			    n_fields, n_levels, sizeof(void *));
    }
    
    if (mdv->field_plane_len == NULL)
    {
      mdv->field_plane_len =
	(int **)ucalloc2(n_fields, n_levels, sizeof(int));
    }
    else
    {
      mdv->field_plane_len =
	(int **)urealloc2((void **)mdv->field_plane_len,
			  n_fields, n_levels, sizeof(int));
    }
    
    for (ifield = 0; ifield < n_fields; ifield++)
    {
      for (iplane = 0; iplane < n_levels; iplane++)
      {
	if (ifield >= mdv->n_fields_alloc ||
	    iplane >= mdv->n_levels_alloc)
	{
	  mdv->field_plane[ifield][iplane] = (void *)NULL;
	  mdv->field_plane_len[ifield][iplane] = 0;
	}
      }
    }
    
  }

  mdv->n_fields_alloc = n_fields;
  mdv->n_levels_alloc = n_levels;
  mdv->n_chunks_alloc = n_chunks;
  
}

/*************************************************************************
 *
 * MDV_field_name_to_pos
 *
 * Returns field position to match the name.
 * Returns -1 on error (name not in file).
 *
 * NOTE: must use MDV_read_all() or MDV_load_all() before using this function.
 *
 **************************************************************************/

int MDV_field_name_to_pos(const MDV_handle_t *mdv,
			  const char *field_name)
     
{

  int i;

  if (!mdv->read_all_done) {
    fprintf(stderr, "ERROR - MDV_field_name_to_pos\n");
    fprintf(stderr, "MDV_read_all() or MDV_load_all() must be called first\n");
    return (-1);
  }

  for (i = 0; i < mdv->master_hdr.n_fields; i++) {
    if (!strcmp(field_name, mdv->fld_hdrs[i].field_name)) {
      return (i);
    }
  }

  return (-1);
  
}

/*************************************************************************
 *
 * MDV_long_field_name_to_pos
 *
 * Returns field position to match the long field name.
 * Returns -1 on error (name not in file).
 *
 * NOTE: must use MDV_read_all() or MDV_load_all() before using this function.
 *
 **************************************************************************/

int MDV_long_field_name_to_pos(const MDV_handle_t *mdv,
			       const char *field_name)
     
{

  int i;

  if (!mdv->read_all_done) {
    fprintf(stderr, "ERROR - MDV_field_name_to_pos\n");
    fprintf(stderr, "MDV_read_all() or MDV_load_all() must be called first\n");
    return (-1);
  }

  for (i = 0; i < mdv->master_hdr.n_fields; i++) {
    if (!strcmp(field_name, mdv->fld_hdrs[i].field_name_long)) {
      return (i);
    }
  }

  return (-1);
  
}

/*************************************************************************
 *
 * MDV_plane_ht_to_num
 *
 * Sets plane num to match the height for a given field.
 * Also sets the actual ht for the plane.
 *
 * Returns 0 on success, -1 on error.
 *
 * NOTE: must use MDV_read_all() or MDV_load_all() before using this function.
 *
 **************************************************************************/

int MDV_plane_ht_to_num(MDV_handle_t *mdv,
			int field_num,
			double requested_ht,
			int *plane_num_p,
			double *actual_ht_p)
     
{

  int plane_num;
  double actual_ht = 0;
  MDV_field_header_t *fhdr;
  MDV_vlevel_header_t *vlv;

  if (!mdv->read_all_done) {
    fprintf(stderr, "ERROR - MDV_field_ht_to_num\n");
    fprintf(stderr, "MDV_read_all() or MDV_load_all() must be called first\n");
    return (-1);
  }
  
  if (field_num >= mdv->master_hdr.n_fields) {
    fprintf(stderr, "ERROR - MDV_field_ht_to_num\n");
    fprintf(stderr, "Field number %d exceeds max of %d\n",
	    field_num, mdv->master_hdr.n_fields - 1);
    fprintf(stderr, "Remember fields numbers are 0-based\n");
    return (-1);
  }
  
  fhdr = mdv->fld_hdrs + field_num;
  
  if (!mdv->master_hdr.vlevel_included) {

    /*
     * planes are at constant spacing
     */

    plane_num = (int)
      ((requested_ht - fhdr->grid_minz) / fhdr->grid_dz + 0.5);

    plane_num = MAX(0, plane_num);
    plane_num = MIN(fhdr->nz - 1, plane_num);

    actual_ht = fhdr->grid_minz + plane_num * fhdr->grid_dz;
    
  } else {

    double min_diff = 1.0e99;
    double diff;
    int i;
    plane_num = 0;
    
    /*
     * search vlevel header
     */

    vlv = mdv->vlv_hdrs + field_num;

    for (i = 0; i < fhdr->nz; i++) {
      diff = fabs(vlv->vlevel_params[i] - requested_ht);
      if (diff < min_diff) {
	plane_num = i;
	actual_ht = vlv->vlevel_params[i];
	min_diff = diff;
      }
    } /* i */
    
  }

  *plane_num_p = plane_num;
  *actual_ht_p = actual_ht;
  return (0);
  
}

/*
 * Alternate name of routine to handle type in original code.
 */

int MDV_field_ht_to_num(MDV_handle_t *mdv,
			int field_num,
			double requested_ht,
			int *plane_num_p,
			double *actual_ht_p)
     
{
  return MDV_plane_ht_to_num(mdv, field_num, requested_ht,
			     plane_num_p, actual_ht_p);
}

/*************************************************************************
 *
 * MDV_add_field
 *
 * Adds the given field to the dataset.  The dataset must already have
 * the data allocated for the previous fields before the new field is
 * added.
 *
 * The volume data is copied and put into the mdv_handle structure, so
 * the original data should be freed by the user after this routine.
 *
 * Returns 0 on success, -1 on failure.
 *
 **************************************************************************/

int MDV_add_field(MDV_handle_t *mdv,
		  MDV_field_header_t *field_hdr,
		  MDV_vlevel_header_t *vlevel_hdr,
		  void *volume_data)
{
  int field_index = mdv->master_hdr.n_fields;
  int num_fields = field_index + 1;
  
  int i;
  
  /*
   * Reallocate space for the different buffers.
   */

  MDV_realloc_handle_arrays(mdv,
			    num_fields,
			    field_hdr->nz,
			    0);
  
  /*
   * Copy the header structures
   */

  if (field_hdr == (MDV_field_header_t *)NULL)
    memset(&mdv->fld_hdrs[field_index], 0, sizeof(MDV_field_header_t));
  else
    mdv->fld_hdrs[field_index] = *field_hdr;
  
  if (vlevel_hdr == (MDV_vlevel_header_t *)NULL)
    memset(&mdv->vlv_hdrs[field_index], 0, sizeof(MDV_vlevel_header_t));
  else
    mdv->vlv_hdrs[field_index] = *vlevel_hdr;
  
  /*
   * Copy the data.
   */

  for (i = 0; i < field_hdr->nz; i++)
  {
    void *plane;
    int plane_len;
    
    plane = MDV_get_plane_from_volume(field_hdr,
				      i,
				      volume_data,
				      &plane_len);
    
    mdv->field_plane[field_index][i] = plane;
    mdv->field_plane_len[field_index][i] = plane_len;
  
  } /* endfor - i */
  
  mdv->field_planes_allocated = TRUE;
  
  /*
   * Update the master header and return.
   */

  mdv->master_hdr.n_fields = num_fields;
  
  return(0);
}

     
/*************************************************************************
 *
 * MDV_remove_field
 *
 * Removes the given field from the dataset.
 *
 * Returns 0 on success, -1 on failure.
 *
 **************************************************************************/

int MDV_remove_field(MDV_handle_t *mdv, int field_num)
{
  static char *routine_name = "MDV_remove_field()\n";
  
  int i, j;
  
  /*
   * Check for errors.
   */

  if (field_num >= mdv->master_hdr.n_fields)
  {
    fprintf(stderr, "ERROR: %s\n", routine_name);
    fprintf(stderr, "Invalid field number %d requested for removal\n",
	    field_num);
    fprintf(stderr, "File only has %d fields\n",
	    mdv->master_hdr.n_fields);
    
    return(-1);
  }
  
  /*
   * Free the deleted field's data.
   */

  if (mdv->field_planes_allocated)
  {
    for (i = 0; i < mdv->n_levels_alloc; i++)
    {
      if (mdv->field_plane[field_num][i] != NULL) {
	ufree(mdv->field_plane[field_num][i]);
	mdv->field_plane[field_num][i] = NULL;
	mdv->field_plane_len[field_num][i] = 0;
      }
    }
  }
  
  /*
   * Move the header structures and the data
   */

  for (i = field_num + 1; i < mdv->master_hdr.n_fields; i++)
  {
    mdv->fld_hdrs[i-1] = mdv->fld_hdrs[i];
    mdv->vlv_hdrs[i-1] = mdv->vlv_hdrs[i];

    for (j = 0; j < mdv->n_levels_alloc; j++) {
      mdv->field_plane[i-1][j] = mdv->field_plane[i][j];
      mdv->field_plane_len[i-1][j] = mdv->field_plane_len[i][j];
    }
    
  } /* endfor - i */
  
  /*
   * Update the master header and return.
   */

  mdv->master_hdr.n_fields--;
  
  return(0);
}

     
/*************************************************************************
 *
 * MDV_remove_field_plane
 *
 * Removes the plane of the given field from the dataset.
 *
 * Returns 0 on success, -1 on failure.
 *
 **************************************************************************/

int MDV_remove_field_plane(MDV_handle_t *mdv,
			   int field_num, int plane_num)
{
  static char *routine_name = "MDV_remove_field_plane()\n";
  
  MDV_field_header_t *field_hdr;
  
  int i;
  
  /*
   * Check for errors.
   */

  if (field_num >= mdv->master_hdr.n_fields)
  {
    fprintf(stderr, "ERROR: %s\n", routine_name);
    fprintf(stderr, "Invalid field number %d requested for removal\n",
	    field_num);
    fprintf(stderr, "File only has %d fields\n",
	    mdv->master_hdr.n_fields);
    
    return(-1);
  }
  
  field_hdr = &mdv->fld_hdrs[field_num];
  
  if (plane_num >= field_hdr->nz)
  {
    fprintf(stderr, "ERROR: %s\n", routine_name);
    fprintf(stderr, "Invalid plane number %d requested for removal\n",
	    plane_num);
    fprintf(stderr, "Field only has %d planes\n",
	    field_hdr->nz);
    
    return(-1);
  }
  
  if (!mdv->master_hdr.vlevel_included &&
      (plane_num == 0 || plane_num == field_hdr->nz - 1))
  {
    fprintf(stderr, "ERROR: %s\n", routine_name);
    fprintf(stderr, "Can only delete first or last plane if vlevel headers are not included.\n");
    fprintf(stderr, "Otherwise, we can't update the header fields properly.\n");
    
    return(-1);
  }
  
  /*
   * Free the deleted plane's data.
   */

  if (mdv->field_planes_allocated)
  {
    if (mdv->field_plane[field_num][plane_num] != NULL) {
      ufree(mdv->field_plane[field_num][plane_num]);
      mdv->field_plane[field_num][plane_num] = NULL;
      mdv->field_plane_len[field_num][plane_num] = 0;
    }
  }
  
  /*
   * Move the data for the following planes.
   */

  for (i = plane_num + 1; i < field_hdr->nz; i++) {
    mdv->field_plane[field_num][i-1] = mdv->field_plane[field_num][i];
    mdv->field_plane_len[field_num][i-1] = mdv->field_plane_len[field_num][i];
  }
  
  /*
   * Update the headers and return.
   */

  if (mdv->master_hdr.vlevel_included)
  {
    for (i = plane_num + 1; i < field_hdr->nz; i++)
    {
      mdv->vlv_hdrs[field_num].vlevel_type[i-1] =
	mdv->vlv_hdrs[field_num].vlevel_type[i];
      mdv->vlv_hdrs[field_num].vlevel_params[i-1] =
	mdv->vlv_hdrs[field_num].vlevel_params[i];
    } /* endfor - i */
  }
  else       /* constant plane heights */
  {
    if (plane_num == 0)
      field_hdr->grid_minz += field_hdr->grid_dz;
  }
  
  field_hdr->nz--;
  
  return(0);
}

     
/*************************************************************************
 *
 * MDV_load_info
 *
 * Loads info string into the master header.
 *
 **************************************************************************/

void MDV_load_info(MDV_handle_t *mdv, char *info)

{
  memset(mdv->master_hdr.data_set_info, 0, MDV_INFO_LEN);
  STRncopy(mdv->master_hdr.data_set_info, info, MDV_INFO_LEN);
}

     
/*************************************************************************
 *
 * MDV_append_info
 *
 * Append info string to that already in the master header.
 *
 **************************************************************************/

void MDV_append_info(MDV_handle_t *mdv, char *info)

{
  STRconcat(mdv->master_hdr.data_set_info, "\n", MDV_INFO_LEN);
  STRconcat(mdv->master_hdr.data_set_info, info, MDV_INFO_LEN);
}


/*************************************************************************
 *
 * MDV_handle_free_field_planes()
 *
 * frees memory for field plane data
 *
 **************************************************************************/

void MDV_handle_free_field_planes(MDV_handle_t *mdv)
     
{

  int ifield, ilevel;
  void *plane;
  
  if (mdv->field_plane != NULL) {
    
    for (ifield = 0; ifield < mdv->n_fields_alloc; ifield++) {

      if (mdv->field_plane[ifield] != NULL) {
	
	for (ilevel = 0; ilevel < mdv->n_levels_alloc; ilevel++) {
	  
	  plane = mdv->field_plane[ifield][ilevel];
	  if (plane != NULL && mdv->field_planes_allocated) {
	    ufree(plane);
	  }
	  mdv->field_plane[ifield][ilevel] = NULL;
	  mdv->field_plane_len[ifield][ilevel] = 0;
	  
	} /* ilevel */
	
      }
      
    } /* ifield */
    
  }

  mdv->field_planes_allocated = FALSE;

}

/*****************************************************************************
 *
 * MDV_set_volume3d:  get a volume of mdv data and return it as a
 *                    three-dimensional array
 *
 *
 * NOTE:  the caller is responsible for freeing the 3d dataset
 *        by calling MDV_free_volume3d()
 *
 *        The last argument allows for repeated data by extending
 *        the raw dataset and copying the requested number of beams.
 *        This is a very bogus work-around for a problem in ptrec.
 *        Most callers will just pass in a zero for the last argument.
 *
 ****************************************************************************/

void *** MDV_set_volume3d( MDV_handle_t *mdv, int field_index, 
			   int return_type, int nrepeat_beams )
{

  int i, j;
  int nx, ny, nz, element_size, offset;
  int nbytes_beam, nbytes_repeated, old_bytes_tilt, new_bytes_tilt;
  char *data_ptr = NULL, *old_data = NULL, *new_data = NULL;
  char ***three_d_array;
  
  /*
   * make sure we aren't doing this on a run-length encoded dataset
   * or a native data type (cause it could be RLencoded)
   */
  if ( return_type == MDV_PLANE_RLE8  ||
       return_type == MDV_NATIVE )
    return NULL;
  else
    element_size = MDV_data_element_size( return_type );
  
  /*
   * figure out the size of the arrays
   * which may include some repeated data
   */
   nx = mdv->fld_hdrs[field_index].nx;
   ny = mdv->fld_hdrs[field_index].ny + nrepeat_beams;
   nz = mdv->fld_hdrs[field_index].nz;
   nbytes_beam = nx * element_size;

  /*
   * reallocate the original data array and copy data around
   * if repeated beams were requested
   */
   if ( nrepeat_beams ) {
      nbytes_repeated = nrepeat_beams * nbytes_beam;
      new_data        = malloc( nx * ny * nz * element_size );
      new_bytes_tilt  = ny * nbytes_beam;
      old_bytes_tilt  = mdv->fld_hdrs[field_index].ny * nbytes_beam;

      for( i=0; i < nz; i++ ) {

         /* copy the original data */
	 old_data = mdv->field_plane[field_index][i];
         offset = i * new_bytes_tilt;
         memcpy( &(new_data[offset]), old_data, old_bytes_tilt );

         /* repeat the first beams */
         offset += old_bytes_tilt;
         memcpy( &(new_data[offset]), old_data, nbytes_repeated );
      }
      data_ptr = new_data;
   }

  /*
   * allocate the 3-d array as a 2-d array of pointers.
   */
  
  three_d_array = (char ***)
    umalloc2( nz, ny, sizeof(char *) );
  
  /*
   * load up the 2d pointer array to point to different parts of
   * the data buffer
   */
  
  for( i=0; i < nz; i++ ) {
    if ( !nrepeat_beams )
      data_ptr = mdv->field_plane[field_index][i];
    for( j=0; j < ny; j++) {
      three_d_array[i][j] = data_ptr;
      data_ptr += nbytes_beam;
    }
  }
  
  /*
   * return 3d pointer array address
   */

  return ((void ***)three_d_array);

}

/*****************************************************************************
 *
 * MDV_free_volume3d: free memory associated with 3-d pointers
 *                    
 ****************************************************************************/

void MDV_free_volume3d(void ***three_d_array, int nrepeat_beams )
{
  if ( nrepeat_beams ) {
    /*
     * free up the extended data array
     */
     ufree( three_d_array[0][0] );
  }
  ufree2((void **) three_d_array);
}

/*************************************************************************
 *
 * MDV_load_all
 *
 * Loads all fields and planes in volume from the given MDV buffer.
 * The buffer is assumed to be in big-endian format.
 * Handle members and pointers are loaded up with the data.
 *
 * Must call MDV_init_handle() first.
 *
 * This routine may be called repeatedly - memory allocation is
 * handled within the routine.
 *
 * When done, call MDV_free_handle() to free memory. 
 *
 * Returns 0 on success, -1 on failure.
 *
 **************************************************************************/

int MDV_load_all(MDV_handle_t *mdv, void *buffer, int return_type)
{
  static char *routine_name = "MDV_load_all";
  
  char *buf_ptr;
  int ichunk, ifield, ilevel;
  void *plane;
  MDV_field_header_t *fld;

  /*
   * Load the master header.
   */
  
  buf_ptr = (char *)buffer;
  memcpy(&mdv->master_hdr, buf_ptr, sizeof(MDV_master_header_t));
  buf_ptr += sizeof(MDV_master_header_t);
  
  MDV_master_header_from_BE(&mdv->master_hdr);

  /*
   * alloc arrays
   */

  MDV_alloc_handle_arrays(mdv,
			  mdv->master_hdr.n_fields,
			  mdv->master_hdr.max_nz,
			  mdv->master_hdr.n_chunks);
  
  /*
   * Load field headers.
   */

  memcpy(mdv->fld_hdrs, buf_ptr,
	 mdv->master_hdr.n_fields * sizeof(MDV_field_header_t));
  buf_ptr += mdv->master_hdr.n_fields * sizeof(MDV_field_header_t);
  
  for (ifield = 0; ifield < mdv->master_hdr.n_fields; ifield++)
    MDV_field_header_from_BE(&mdv->fld_hdrs[ifield]);
  
  /*
   * Load vlevel headers.
   */
  
  if (mdv->master_hdr.vlevel_included)
  {
    memcpy(mdv->vlv_hdrs, buf_ptr,
	   mdv->master_hdr.n_fields * sizeof(MDV_vlevel_header_t));
    
    buf_ptr += mdv->master_hdr.n_fields * sizeof(MDV_vlevel_header_t);

    for (ifield = 0; ifield < mdv->master_hdr.n_fields; ifield++)
      MDV_vlevel_header_from_BE(&mdv->vlv_hdrs[ifield]);
  
  } /* endif - vlevel_included */

  /*
   * Load chunk headers.
   */

  memcpy(mdv->chunk_hdrs, buf_ptr,
	 mdv->master_hdr.n_chunks * sizeof(MDV_chunk_header_t));
  buf_ptr += mdv->master_hdr.n_chunks * sizeof(MDV_chunk_header_t);
  
  for (ichunk = 0; ichunk < mdv->master_hdr.n_chunks; ichunk++)
    MDV_chunk_header_from_BE(&mdv->chunk_hdrs[ichunk]);
  
  /*
   * Load up the grid information.
   */

  if (mdv->master_hdr.n_fields > 0)
    MDV_load_grid_from_hdrs(&mdv->master_hdr, mdv->fld_hdrs,
			    &mdv->grid);
  
  /*
   * Load the data planes.
   */

  for (ifield = 0; ifield < mdv->master_hdr.n_fields; ifield++)
  {
    int volume_size_in_buffer;
    int converted_return_type;
    
    fld = mdv->fld_hdrs + ifield;

    if (return_type == MDV_NATIVE)
      converted_return_type = fld->encoding_type;
    else
      converted_return_type = return_type;
    
    volume_size_in_buffer = fld->volume_size;
    fld->volume_size = 0;
    
    /*
     * Skip the leading FORTRAN record length.
     */

    buf_ptr += sizeof(si32);
    
    for (ilevel = 0; ilevel < fld->nz; ilevel++)
    {
      int plane_size;
      
      plane = MDV_load_plane(buf_ptr, fld, converted_return_type,
			     ilevel, &plane_size);
    
      if (plane == NULL)
      {
	fprintf(stderr,
		"ERROR: %s - reading field %d, level %d from buffer\n",
		routine_name, ifield, ilevel);
	return(-1);
      }
      
      if (mdv->field_plane[ifield][ilevel] &&
	  mdv->field_planes_allocated) {
	ufree(mdv->field_plane[ifield][ilevel]);
	mdv->field_plane[ifield][ilevel] = NULL;
      }

      mdv->field_plane[ifield][ilevel] = plane;
      mdv->field_plane_len[ifield][ilevel] = plane_size;
      fld->volume_size += plane_size;
      
    } /* ilevel */
	
    /*
     * Skip the trailing FORTRAN record length.
     */

    buf_ptr += sizeof(si32);
    
    fld->encoding_type = converted_return_type;

    buf_ptr += volume_size_in_buffer;
    
  } /* ifield */

  if (mdv->master_hdr.n_fields > 0)
    mdv->field_planes_allocated = TRUE;

  /*
   * Load the chunk data.
   */

  for (ichunk = 0; ichunk < mdv->master_hdr.n_chunks; ichunk++)
  {
    int chunk_buffer_size;
    
    if (mdv->chunk_data_allocated && mdv->chunk_data[ichunk])
    {
      ufree(mdv->chunk_data[ichunk]);
      mdv->chunk_data[ichunk] = NULL;
    }

    chunk_buffer_size = mdv->chunk_hdrs[ichunk].size + (2 * sizeof(si32));
    
    mdv->chunk_data[ichunk] = umalloc(chunk_buffer_size);
    memcpy(mdv->chunk_data[ichunk], buf_ptr, chunk_buffer_size);

    MDV_chunk_data_from_BE(mdv->chunk_data[ichunk],
			   mdv->chunk_hdrs[ichunk].size,
			   mdv->chunk_hdrs[ichunk].chunk_id);
    
    buf_ptr += chunk_buffer_size;
    
  } /* ichunk */

  if (mdv->master_hdr.n_chunks > 0)
    mdv->chunk_data_allocated = TRUE;

  /*
   * load radar data structs if applicable
   */

  MDV_handle_load_radar_structs(mdv);

  mdv->read_all_done = TRUE;
  return (0);
}


/*************************************************************************
 * MDV_LOAD_BUFFER: Write out the handle structs and data to the given buffer.
 *
 * Inputs: mdv - handle containing the mdv structs and data
 *         buffer - pointer to previously allocated buffer to load.
 *
 * Note on chunk swapping: if the chunk type has a swapping routine
 * in the mdv library, it will be swapped. If not, it will not
 * be swapped.
 *
 * Returns 0 on success, -1 on failure.
 *
 **************************************************************************/

int MDV_load_buffer(MDV_handle_t *mdv,
		    char *buffer)
{
  int i;
  int data_offset;
  
  char *buf_ptr = buffer;
  
  int n_fields = mdv->master_hdr.n_fields;
  int n_chunks = mdv->master_hdr.n_chunks;
  
  /*
   * Make sure the file offset values in the master header are okay.
   */

  MDV_set_master_hdr_offsets(&mdv->master_hdr);
  
  /*
   * Load and swap the master header.
   */

  memcpy(buf_ptr, &mdv->master_hdr, sizeof(MDV_master_header_t));
  MDV_master_header_to_BE((MDV_master_header_t *)buf_ptr);
  
  buf_ptr += sizeof(MDV_master_header_t);

  /*
   * Load and swap the field headers.  Make sure the field header
   * offsets are correct along the way.
   */

  data_offset = sizeof(MDV_master_header_t) +
    n_fields * sizeof(MDV_field_header_t) +
    n_chunks * sizeof(MDV_chunk_header_t) +
    sizeof(si32);     /* leading FORTRAN record length */
  
  if (mdv->master_hdr.vlevel_included)
    data_offset += n_fields * sizeof(MDV_vlevel_header_t);
  
  for (i = 0; i < n_fields; i++)
  {
    MDV_field_header_t *field_hdr = &mdv->fld_hdrs[i];
    
    /* Update field header offsets */

    field_hdr->field_data_offset = data_offset;
    
    /* Load the field header */
    
    memcpy(buf_ptr, field_hdr, sizeof(MDV_field_header_t));
    
    /* Byte swap */

    MDV_field_header_to_BE((MDV_field_header_t *)buf_ptr);
    
    /* Update pointers and offsets for next header */
	   
    buf_ptr += sizeof(MDV_field_header_t);
    
    data_offset += field_hdr->volume_size +
      (2 * sizeof(si32));                /* FORTRAN record lengths */

  } /* endfor - i */
  
  /*
   * Load and swap the vlevel headers.
   */

  if (mdv->master_hdr.vlevel_included)
  {
    for (i = 0; i < n_fields; i++)
    {
      memcpy(buf_ptr, &mdv->vlv_hdrs[i], sizeof(MDV_vlevel_header_t));
      MDV_vlevel_header_to_BE((MDV_vlevel_header_t *)buf_ptr);
      buf_ptr += sizeof(MDV_vlevel_header_t);
    }
    
  }
  
  /*
   * Load and swap the chunk headers, updating offsets as we go along.
   */

  data_offset = sizeof(MDV_master_header_t) +
    n_fields * sizeof(MDV_field_header_t) +
    n_chunks * sizeof(MDV_chunk_header_t);
  
  if (mdv->master_hdr.vlevel_included)
    data_offset += n_fields * sizeof(MDV_vlevel_header_t);
  
  for (i = 0; i < n_fields; i++)
    data_offset += mdv->fld_hdrs[i].volume_size +
      (2 * sizeof(si32));                 /* FORTRAN record lengths */
  
  for (i = 0; i < n_chunks; i++)
  {
    MDV_chunk_header_t *chunk_hdr = &mdv->chunk_hdrs[i];
    
    /* Update data offset */

    chunk_hdr->chunk_data_offset = data_offset;
    
    /* Load header */

    memcpy(buf_ptr, &mdv->chunk_hdrs[i], sizeof(MDV_chunk_header_t));

    /* Byte swap */

    MDV_chunk_header_to_BE((MDV_chunk_header_t *)buf_ptr);

    /* Update pointers and offsets for next header */

    buf_ptr += sizeof(MDV_chunk_header_t);

    data_offset += chunk_hdr->size;
    
  } /* endfor - i */
  
  /*
   * Load field data.
   */

  for (i = 0; i < n_fields; i++)
  {
    int j;
    MDV_field_header_t *field_hdr = &mdv->fld_hdrs[i];
    
    /*
     * Load the leading record length.
     */

    si32 record_len = field_hdr->volume_size;
    
    record_len = BE_from_si32(record_len);
    
    memcpy(buf_ptr, &record_len, sizeof(si32));
    buf_ptr += sizeof(si32);
    
    for (j = 0; j < field_hdr->nz; j++)
    {
      int plane_size = MDV_calc_plane_size(field_hdr, j,
					   mdv->field_plane[i][j]);
      
      memcpy(buf_ptr, mdv->field_plane[i][j], plane_size);
      MDV_plane_to_BE(field_hdr, buf_ptr);
      
      buf_ptr += plane_size;
      
    } /* endfor - j */
    
    /*
     * Load trailing FORTRAN record length.
     */

    memcpy(buf_ptr, &record_len, sizeof(si32));
    buf_ptr += sizeof(si32);
    
  } /* endfor - i */
  
  /*
   * Load the chunk data.
   */

  for (i = 0; i < n_chunks; i++)
  {
    int chunk_buffer_size = mdv->chunk_hdrs[i].size + (2 * sizeof(si32));
    
    memcpy(buf_ptr, mdv->chunk_data[i], chunk_buffer_size);
    MDV_chunk_data_to_BE(buf_ptr, mdv->chunk_hdrs[i].size,
			 mdv->chunk_hdrs[i].chunk_id);
    buf_ptr += chunk_buffer_size;
  } /* endfor - i */
  
  return(MDV_SUCCESS);

} /* end MDV_load_buffer */


/*************************************************************************
 *
 * MDV_handle_load_radar_structs()
 *
 * Loads up radar structs if they are available as chunks
 *
 **************************************************************************/

void MDV_handle_load_radar_structs(MDV_handle_t *mdv)
{

  MDV_chunk_header_t *chdr = mdv->chunk_hdrs;
  vol_params_t *vol_params;
  radar_params_t *rparams;
  DsRadarParams_t *DsRparams;
  si32 *int_elev;
  int i, nelev;
  
  mdv->radarAvail = FALSE;

  if (mdv->master_hdr.n_chunks < 2) {
    return;
  }

  if (chdr[0].chunk_id == MDV_CHUNK_DOBSON_VOL_PARAMS &&
      chdr[1].chunk_id == MDV_CHUNK_DOBSON_ELEVATIONS) {

    /*
     * dobson format radar paramaters
     */

    vol_params = (vol_params_t *) mdv->chunk_data[0];
    rparams = &vol_params->radar;
    DsRparams = &mdv->radarParams;
    memset(DsRparams, 0, sizeof(DsRadarParams_t));

    DsRparams->radar_id = rparams->radar_id;
    DsRparams->nfields = mdv->master_hdr.n_fields;
    DsRparams->ngates = rparams->ngates;
    DsRparams->samples_per_beam = rparams->samples_per_beam;

    DsRparams->altitude = (double) rparams->altitude / 1000.0;
    DsRparams->latitude = (double) rparams->latitude / 1000000.0;
    DsRparams->longitude = (double) rparams->longitude / 1000000.0;
    DsRparams->gate_spacing = (double) rparams->gate_spacing / 1000000.0;
    DsRparams->start_range = (double) rparams->start_range / 1000000.0;
    DsRparams->horiz_beam_width = (double) rparams->beam_width / 1000000.0;
    DsRparams->vert_beam_width = (double) rparams->beam_width / 1000000.0;
    DsRparams->pulse_width = (double) rparams->pulse_width / 1000.0;
    DsRparams->prf = (double) rparams->prf / 1000.0;
    DsRparams->wavelength = (double) rparams->wavelength / 10000.0;

    STRncopy(DsRparams->radar_name, rparams->name, DS_LABEL_LEN);

    nelev = chdr[1].size / sizeof(si32);
    int_elev = (si32 *) mdv->chunk_data[1];
    DsRadarElev_alloc(&mdv->radarElevs, nelev);
    for (i = 0; i < nelev; i++) {
      mdv->radarElevs.elev_array[i] =
	(fl32) ((double) int_elev[i] / 1000000.0);
    }

    mdv->radarAvail = TRUE;

  } else if (chdr[0].chunk_id == MDV_CHUNK_DSRADAR_PARAMS &&
	     chdr[1].chunk_id == MDV_CHUNK_DSRADAR_ELEVATIONS) {

    /*
     * DsRadar format parameters
     */

    memcpy(&mdv->radarParams, mdv->chunk_data[0],
	   sizeof(DsRadarParams_t));
    
    DsRadarElev_unload_chunk(&mdv->radarElevs, mdv->chunk_data[1],
			     chdr[1].size);
    
    mdv->radarAvail = TRUE;

  }

  return;

}


/*************************************************************************
 * MDV_CROP_PLANES: Crop all of the planes in the MDV data to the given
 *                  boundaries.
 *
 * Inputs: mdv - handle containing the mdv structs and data
 *         min_lat - minimum latitude for crop
 *         max_lat - maximum latitude for crop
 *         min_lon - minimum longitude for crop
 *         max_lon - maximum longitude for crop
 *
 * Returns 0 on success, -1 on failure.  Note that if an error is encountered
 * cropping one field, this routine will still attempt to crop the following
 * fields.  -1 is returned if there is an error cropping ANY field.
 *
 **************************************************************************/

int MDV_crop_planes(MDV_handle_t *mdv,
		    double min_lat, double max_lat,
		    double min_lon, double max_lon)
{
  static char *routine_name = "MDV_crop_planes";
  
  int ifield;
  int return_status = 0;
  
  /*
   * Crop each of the fields.
   */

  for (ifield = 0; ifield < mdv->master_hdr.n_fields; ifield++)
  {
    MDV_field_header_t *field_hdr = &mdv->fld_hdrs[ifield];
    
    /*
     * Data calculations depend on the projection type for the data.
     */
    
    switch (field_hdr->proj_type)
    {
    case MDV_PROJ_LATLON :
    case MDV_PROJ_FLAT :
      if (crop_latlon_planes(mdv, ifield,
			     min_lat, max_lat, min_lon, max_lon) != 0)
	return_status = -1;
      break;
      
    case MDV_PROJ_ARTCC :
    case MDV_PROJ_STEREOGRAPHIC :
    case MDV_PROJ_LAMBERT_CONF :
    case MDV_PROJ_MERCATOR :
    case MDV_PROJ_POLAR_STEREO :
    case MDV_PROJ_POLAR_ST_ELLIP :
    case MDV_PROJ_CYL_EQUIDIST :
    case MDV_PROJ_POLAR_RADAR :
    case MDV_PROJ_RADIAL :
      fprintf(stderr, "ERROR: mdv_handle::%s\n", routine_name);
      fprintf(stderr,
	      "Cropping of planes for %s projection not yet implemented\n",
	      MDV_proj2string(field_hdr->proj_type));
      
      return_status = -1;
      break;
      
    case MDV_PROJ_NATIVE :
    case MDV_PROJ_UNKNOWN :
    default:
      fprintf(stderr, "ERROR: mdv_handle::%s\n", routine_name);
      fprintf(stderr, "Cannot crop planes for %s projection\n",
	      MDV_proj2string(field_hdr->proj_type));
      
      return_status = -1;
      break;
      
    } /* endswitch - field_hdr->proj_type */
    
  } /* endfor - ifield */
  
  return(return_status);
}


/*************************************************************************
 * STATIC FUNCTIONS
 *************************************************************************/

/*************************************************************************
 * CROP_LATLON_PLANES: Crop all of the planes in the given field data to
 *                     the given boundaries assuming that the field data
 *                     is given in an MDV_PROJ_LATLON format.
 *
 * Inputs: mdv - handle containing the mdv structs and data
 *         field_num - field number to crop
 *         min_lat - minimum latitude for crop
 *         max_lat - maximum latitude for crop
 *         min_lon - minimum longitude for crop
 *         max_lon - maximum longitude for crop
 *
 * Returns 0 on success, -1 on failure.
 *
 **************************************************************************/

static int crop_latlon_planes(MDV_handle_t *mdv,
			      int field_num,
			      double min_lat, double max_lat,
			      double min_lon, double max_lon)
{
  static char *routine_name = "crop_latlon_planes";
  
  MDV_field_header_t *field_hdr = &mdv->fld_hdrs[field_num];
  int plane_size = 0;
  
  /*
   * The cropping method depends on the encoding format for
   * the data.
   */

  switch (field_hdr->encoding_type)
  {
  case MDV_INT8 :
  case MDV_INT16 :
  case MDV_FLOAT32 :
  {
    int min_x, max_x;
    int min_y, max_y;
    int i;
    
    /*
     * Convert the lat/lon values to x,y indices.
     */

    MDV_latlon2index_xy(&mdv->master_hdr,
			field_hdr,
			min_lat, min_lon,
			&min_x, &min_y);
    
    MDV_latlon2index_xy(&mdv->master_hdr,
			field_hdr,
			max_lat, max_lon,
			&max_x, &max_y);
    
    /*
     * Crop the planes.
     */

    for (i = 0; i < field_hdr->nz; i++)
    {
      if (mdv->field_plane[field_num][i] != NULL)
      {
	void *new_plane =
	  crop_unencoded_plane(min_x, min_y,
			       max_x, max_y,
			       field_hdr->nx, field_hdr->ny,
			       field_hdr->data_element_nbytes,
			       mdv->field_plane[field_num][i],
			       &plane_size);
	ufree(mdv->field_plane[field_num][i]);
	mdv->field_plane[field_num][i] = new_plane;
	mdv->field_plane_len[field_num][i] = plane_size;
      }
    } /* endfor - i */

    /*
     * Update the header fields.  Note that we have to calculate the
     * minx and miny values so the grid location doesn't shift.
     */

    field_hdr->nx = max_x - min_x + 1;
    field_hdr->ny = max_y - min_y + 1;
    
    field_hdr->grid_minx += min_x * field_hdr->grid_dx;
    field_hdr->grid_miny += min_y * field_hdr->grid_dy;
    
    field_hdr->volume_size = field_hdr->nx * field_hdr->ny *
      field_hdr->nz * field_hdr->data_element_nbytes;
    
    break;
  }
  
  case MDV_PLANE_RLE8 :
    fprintf(stderr, "ERROR: mdv_handle::%s\n", routine_name);
    fprintf(stderr, "Cropping for %s data not yet implemented\n",
	    MDV_encode2string(field_hdr->encoding_type));
    
    return(-1);
    
  case MDV_NATIVE :
    fprintf(stderr, "ERROR: mdv_handle::%s\n", routine_name);
    fprintf(stderr, "Cannot crop data with NATIVE encoding type\n");
    
    return(-1);
    
  default:
    fprintf(stderr, "ERROR: mdv_handle::%s\n", routine_name);
    fprintf(stderr, "Unrecognized encoding type %d in field %d header\n",
	    field_hdr->encoding_type, field_num);
    
    return(-1);
    
  } /* endswitch - field_hdr->encoding_type */
  
  return(0);
}
     

/*************************************************************************
 * CROP_UNENCODED_PLANE: Crop the data in an unencoded data plane.  An
 *                       unencoded data plane is one that is stored
 *                       without using run-length encoding.
 *
 * Inputs: min_x     minimum x index to use
 *         min_y     minimum y index to use
 *         max_x     maximum x index to use
 *         max_y     maximum y index to use
 *         nx        number of x elements in the plane
 *         ny        number of y elements in the plane
 *         elem_size number of bytes in each element
 *         data      plane data pointer
 *
 * Returns a pointer to the new plane data on success, NULL on failure.
 * The memory for the new data plane is allocated by this routine and
 * must be freed by the calling routine.
 **************************************************************************/

static void *crop_unencoded_plane(int min_x, int min_y,
				  int max_x, int max_y,
				  int nx, int ny,
				  int elem_size,
				  void *data,
				  int *plane_size_p)
{
  static char *routine_name = "crop_unencoded_plane";
  
  int new_nx = max_x - min_x + 1;
  int new_ny = max_y - min_y + 1;
  int row_size = new_nx * elem_size;
  int plane_size = row_size * new_ny;
  
  void *new_data = umalloc(plane_size);
  char *new_data_ptr = (char *)new_data;
  
  int y;
  
  /*
   * Check for errors.
   */

  if (new_nx == 0 || new_ny == 0)
  {
    fprintf(stderr, "ERROR: mdv_handle::%s\n", routine_name);
    fprintf(stderr, "No data left after lat/lon crop\n");
    
    return((void *)NULL);
  }
  
  /*
   * Crop the data.
   */

  for (y = min_y; y <= max_y; y++)
  {
    char *data_ptr = (char *)data + ((y * nx + min_x) * elem_size);
    memcpy(new_data_ptr, data_ptr, row_size);
    
    new_data_ptr += row_size;
  } /* endfor - y */

  *plane_size_p = plane_size;
  return(new_data);
}


/*************************************************************************
 *
 * free_chunk_data()
 *
 * frees memory for chunk data
 *
 **************************************************************************/

static void free_chunk_data(MDV_handle_t *mdv)
     
{

  int i;
  
  if (mdv->chunk_data != NULL) {
    
    for (i = 0; i < mdv->n_chunks_alloc; i++) {
      if (mdv->chunk_data_allocated && mdv->chunk_data[i] != NULL) {
	ufree(mdv->chunk_data[i]);
      }
      mdv->chunk_data[i] = NULL;
    }

  }

  mdv->chunk_data_allocated = FALSE;

}
