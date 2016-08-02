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

/************************************************************************
*                    
* MDV_HANDLE.H
*
* MDV handle struct header file
*                   
* May 1997
*************************************************************************/

# ifndef MDV_HANDLE_H
# define MDV_HANDLE_H

#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_grid.h>
#include <rapformats/ds_radar.h>

/**************
 * MDV_handle_t
 *
 * The MDV_handle_handle_t is a container class for an entire
 * volume.
 *
 * It is initialized by MDV_init_handle(), and freed by
 * MDV_free_handle().
 */

typedef struct {
  
  MDV_master_header_t master_hdr; 	

  MDV_field_header_t *fld_hdrs; /* array of field headers */

  MDV_vlevel_header_t *vlv_hdrs; /* array of vlevel headers */

  MDV_chunk_header_t *chunk_hdrs; /* array of chunk headers */

  mdv_grid_t grid; /* grid for field 0 */

  void **chunk_data; /* array of pointers to chunk data */

  void ***field_plane; /* array of field plane pointers -
			* field_plane[ifield][iplane] */

  int **field_plane_len; /* array of field plane lengths -
			  * field_plane_len[ifield][iplane] */

  /*
   * memory allocation
   */

  int n_fields_alloc;		               
  int n_chunks_alloc;		               
  int n_levels_alloc;

  int field_planes_allocated; /* indicates that the field planes have
			       * been allocated and must be freed by
			       * the library routines */

  int chunk_data_allocated;   /* indicates that the chunk data has
			       * been allocated and must be freed by
			       * the library routines */

  /*
   * radar parameters
   */

  int radarAvail;             /* flag set to indicate the radar params
			       * and elevation angles are available */

  DsRadarParams_t radarParams;
  DsRadarElev_t radarElevs;

  /*
   * read_all flag - gets set after a read_all
   */

  int read_all_done;

} MDV_handle_t;

/*
 * prototypes
 */
/*************************************************************************
 *
 * MDV_init_handle
 *
 * initializes the memory associated with handle
 *
 * Returns 0 on success, -1 on failure.
 *
 **************************************************************************/

extern int MDV_init_handle(MDV_handle_t *mdv);

/*************************************************************************
 *
 * MDV_free_handle
 *
 * Frees the memory associated with handle
 *
 **************************************************************************/

extern void MDV_free_handle(MDV_handle_t *mdv);

/*************************************************************************
 *
 * MDV_handle_free_field_planes()
 *
 * frees memory for field plane data
 *
 **************************************************************************/

extern void MDV_handle_free_field_planes(MDV_handle_t *mdv);
     
/*****************************************************************************
 *
 * MDV_set_volume3d:  get a volume of mdv data and return it as a
 *                    three-dimensional array.
 *
 * The MDV_handle_t stores field data as a 2-d array.
 *
 * NOTE:  the caller is responsible for freeing the 3d dataset
 *        by calling MDV_free_dataset3d()
 *
 ****************************************************************************/

extern void *** MDV_set_volume3d( MDV_handle_t *mdv, int field_index, 
				  int return_type, int nrepeat_beams );

/*****************************************************************************
 *
 * MDV_free_volume3d: free memory associated with 3-d pointers
 *                    
 ****************************************************************************/

extern void MDV_free_volume3d(void ***three_d_array, int nrepeat_beams );

/*************************************************************************
 *
 * MDV_alloc_handle_arrays()
 *
 * allocates memory for handle arrays
 *
 **************************************************************************/

extern void MDV_alloc_handle_arrays(MDV_handle_t *mdv,
				    int n_fields,
				    int n_levels,
				    int n_chunks);

/*************************************************************************
 *
 * MDV_realloc_handle_arrays()
 *
 * reallocates memory for handle arrays saving current information
 *
 **************************************************************************/

extern void MDV_realloc_handle_arrays(MDV_handle_t *mdv,
				      int n_fields,
				      int n_levels,
				      int n_chunks);
     
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

extern int MDV_field_name_to_pos(const MDV_handle_t *mdv,
				 const char *field_name);
     
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

extern int MDV_long_field_name_to_pos(const MDV_handle_t *mdv,
				      const char *field_name);
     
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

extern int MDV_plane_ht_to_num(MDV_handle_t *mdv,
			       int field_num,
			       double requested_ht,
			       int *plane_num_p,
			       double *actual_ht_p);
     
/*
 * Alternate name of routine to handle type in original code.
 */

extern int MDV_field_ht_to_num(MDV_handle_t *mdv,
			       int field_num,
			       double requested_ht,
			       int *plane_num_p,
			       double *actual_ht_p);
     
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

extern int MDV_add_field(MDV_handle_t *mdv,
			 MDV_field_header_t *field_hdr,
			 MDV_vlevel_header_t *vlevel_hdr,
			 void *volume_data);

/*************************************************************************
 *
 * MDV_remove_field
 *
 * Removes the given field from the dataset.
 *
 * Returns 0 on success, -1 on failure.
 *
 **************************************************************************/

extern int MDV_remove_field(MDV_handle_t *mdv, int field_num);

/*************************************************************************
 *
 * MDV_remove_field_plane
 *
 * Removes the plane of the given field from the dataset.
 *
 * Returns 0 on success, -1 on failure.
 *
 **************************************************************************/

extern int MDV_remove_field_plane(MDV_handle_t *mdv,
				  int field_num, int plane_num);

/*************************************************************************
 *
 * MDV_load_info
 *
 * Loads info string into the master header.
 *
 **************************************************************************/

extern void MDV_load_info(MDV_handle_t *mdv, char *info);
     
/*************************************************************************
 *
 * MDV_append_info
 *
 * Append info string to that already in the master header.
 *
 **************************************************************************/

extern void MDV_append_info(MDV_handle_t *mdv, char *info);

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

extern int MDV_load_all(MDV_handle_t *mdv, void *buffer, int return_type);

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
		    char *buffer);

/*************************************************************************
 *
 * MDV_handle_load_radar_structs()
 *
 * Loads up radar structs if they are available as chunks
 *
 **************************************************************************/

extern void MDV_handle_load_radar_structs(MDV_handle_t *mdv);

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
		    double min_lon, double max_lon);


#endif /* MDV_HANDLE_H */

#ifdef __cplusplus
}
#endif
