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
 * RfStorm.v3.c
 *
 * Storm file access routines - version 3
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Jan 1996
 *
 **************************************************************************/

#include "storm_file_v3_to_v5.h"
#include <dataport/bigend.h>
#include <time.h>
#include <sys/stat.h>

#define MAX_SEQ 256
#define N_ALLOC 5

/*************************************************************************
 *
 * Rfv3AllocStormHeader()
 *
 * allocates space for the storm_v3_file_header_t structure
 *
 *************************************************************************/

/*ARGSUSED*/

int Rfv3AllocStormHeader(storm_v3_file_index_t *s_handle,
			 char *calling_routine)
     
{

  /*
   * allocate space for s_handle->file_header
   */

  if (!s_handle->header_allocated) {

    s_handle->header = (storm_v3_file_header_t *)
      ucalloc((ui32) 1, (ui32) sizeof(storm_v3_file_header_t));

    s_handle->header_allocated = TRUE;

  } /* if (!s_handle->header_allocated) */

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * Rfv3AllocStormProps()
 *
 * allocates space for the layer props and dbz hist
 *
 *************************************************************************/

/*ARGSUSED*/

int Rfv3AllocStormProps(storm_v3_file_index_t *s_handle,
			si32 n_layers, si32 n_dbz_intervals,
			si32 n_runs, char *calling_routine)
     
{

  /*
   * ensure mem for at least 1 layer and 1 interval
   */

  /*  if (n_layers == 0)
      n_layers = 1;
      
      if (n_dbz_intervals == 0)
      n_dbz_intervals = 1; */

  /*
   * allocate or reallocate for layer props
   */

  if (!s_handle->props_allocated) {
    
    s_handle->max_layers = n_layers;

    s_handle->layer = (storm_v3_file_layer_props_t *)
      ucalloc((ui32) n_layers, (ui32) sizeof(storm_v3_file_layer_props_t));

  } else if (n_layers > s_handle->max_layers) {

    s_handle->max_layers = n_layers;

    s_handle->layer = (storm_v3_file_layer_props_t *)
      urealloc((char *) s_handle->layer,
	       (ui32) n_layers * sizeof(storm_v3_file_layer_props_t));

  }

  /*
   * allocate or reallocate array for dbz histogram entries
   */

  if (!s_handle->props_allocated) {

    s_handle->max_dbz_intervals = n_dbz_intervals;

    s_handle->hist = (storm_v3_file_dbz_hist_t *)
      ucalloc((ui32) n_dbz_intervals, (ui32) sizeof(storm_v3_file_dbz_hist_t));

  } else if (n_dbz_intervals > s_handle->max_dbz_intervals) {

    s_handle->max_dbz_intervals = n_dbz_intervals;

    s_handle->hist = (storm_v3_file_dbz_hist_t *)
      urealloc((char *) s_handle->hist,
	       (ui32) n_dbz_intervals * sizeof(storm_v3_file_dbz_hist_t));

  }

  /*
   * allocate or reallocate for runs
   */

  if (!s_handle->props_allocated) {
    
    s_handle->max_runs = n_runs;

    s_handle->runs = (storm_v3_file_run_t *)
      ucalloc((ui32) n_runs, (ui32) sizeof(storm_v3_file_run_t));

  } else if (n_runs > s_handle->max_runs) {

    s_handle->max_runs = n_runs;

    s_handle->runs = (storm_v3_file_run_t *)
      urealloc((char *) s_handle->runs,
	       (ui32) n_runs * sizeof(storm_v3_file_run_t));

  }

  s_handle->props_allocated = TRUE;

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * Rfv3AllocStormScan()
 *
 * allocates scan struct and gprops array
 *
 **************************************************************************/

/*ARGSUSED*/

int Rfv3AllocStormScan(storm_v3_file_index_t *s_handle,
		       si32 nstorms, char *calling_routine)
     
{

  /*
   * ensure mem for at least 1 storm
   */

  if (nstorms == 0)
    nstorms = 1;

  /*
   * allocate space for scan struct
   */
  
  if (!s_handle->scan_allocated) {

    s_handle->scan = (storm_v3_file_scan_header_t *)
      ucalloc((ui32) 1, (ui32) sizeof(storm_v3_file_scan_header_t));

    s_handle->max_storms = nstorms;

    s_handle->gprops = (storm_v3_file_global_props_t *)
      ucalloc((ui32) nstorms, (ui32) sizeof(storm_v3_file_global_props_t));

    s_handle->scan_allocated = TRUE;

  } else if (nstorms > s_handle->max_storms) {

    s_handle->max_storms = nstorms;

    s_handle->gprops = (storm_v3_file_global_props_t *)
      urealloc((char *) s_handle->gprops,
	       (ui32) nstorms * sizeof(storm_v3_file_global_props_t));

  }

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * Rfv3AllocStormScanOffsets()
 *
 * allocates space for the scan offset array.
 *
 *************************************************************************/

/*ARGSUSED*/

int Rfv3AllocStormScanOffsets(storm_v3_file_index_t *s_handle,
			      si32 n_scans_needed,
			      char *calling_routine)
     
{

  si32 n_realloc;

  if (s_handle->n_scans_allocated < n_scans_needed) {

    /*
     * allocate the required space plus a buffer so that 
     * we do not do too many reallocs
     */
      
    if (s_handle->n_scans_allocated == 0) {
      
      s_handle->n_scans_allocated = n_scans_needed + N_ALLOC;

      s_handle->scan_offsets = (si32 *) umalloc
	((ui32) (s_handle->n_scans_allocated * sizeof(si32)));

    } else {

      n_realloc = n_scans_needed + N_ALLOC;
      
      s_handle->scan_offsets = (si32 *) urealloc
	((char *) s_handle->scan_offsets,
	 (ui32) (n_realloc * sizeof(si32)));
      
      s_handle->n_scans_allocated = n_realloc;
      
    } /* if (s_handle->n_scans_allocated == 0) */

    if (s_handle->scan_offsets == NULL) {
      fprintf(stderr, "ERROR - %s:Rfv3AllocStormScanOffsets\n",
	      calling_routine);
      return (R_FAILURE);
    }

  } /* if (s_handle->n_scans_allocated < n_scans_needed) */

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * Rfv3CloseStormFiles()
 *
 * Closes the storm header and data files
 *
 **************************************************************************/

int Rfv3CloseStormFiles(storm_v3_file_index_t *s_handle,
			char *calling_routine)
     
{

  /*
   * close the header file
   */

  if (s_handle->header_file != NULL) {
    if (fclose(s_handle->header_file)) {
      fprintf(stderr, "WARNING - %s:Rfv3CloseStormFiles\n", calling_routine);
      perror(s_handle->header_file_path);
    }
    s_handle->header_file = (FILE *) NULL;
  }

  /*
   * close the data file
   */
  
  if (s_handle->data_file != NULL) {
    if (fclose(s_handle->data_file)) {
      fprintf(stderr, "WARNING - %s:Rfv3CloseStormFiles\n", calling_routine);
      perror(s_handle->data_file_path);
    }
    s_handle->data_file = (FILE *) NULL;
  }

  /*
   * free up resources
   */

  if (s_handle->header_file_path != NULL) {
    ufree ((char *) s_handle->header_file_path);
    s_handle->header_file_path = (char *) NULL;
  }

  if (s_handle->data_file_path != NULL) {
    ufree ((char *) s_handle->data_file_path);
    s_handle->data_file_path = (char *) NULL;
  }

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * Rfv3DecodeStormHist()
 *
 * Decodes storm hist props into floating point values
 *
 **************************************************************************/

void Rfv3DecodeStormHist(storm_v3_file_params_t *params,
			 storm_v3_file_dbz_hist_t *hist,
			 storm_v3_float_dbz_hist_t *fl_hist)
     
{

  double mult;

  mult = (double) params->mult;
  
  fl_hist->percent_volume = (double) hist->percent_volume / mult;
  fl_hist->percent_precip_area = (double) hist->percent_precip_area / mult;
    
  return;

}

/*************************************************************************
 *
 * Rfv3DecodeStormLayer()
 *
 * Decodes storm layer props into floating point values
 *
 **************************************************************************/

void Rfv3DecodeStormLayer(storm_v3_file_params_t *params,
			  storm_v3_file_global_props_t *gprops,
			  storm_v3_file_layer_props_t *layer,
			  storm_v3_float_layer_props_t *fl_layer)
     
{

  int rev;
  double mult;

  rev = params->minor_rev;
  mult = (double) params->mult;

  fl_layer->dbz_max = (double) layer->dbz_max / mult;
  fl_layer->dbz_mean = (double) layer->dbz_mean / mult;
  fl_layer->vorticity =
    (double) layer->vorticity / (double) params->vort_mult;
    
  if ((rev >= STORM_V3_MINOR_REV_2) &&
      (params->grid_type == PJG_LATLON)) {

    fl_layer->vol_centroid_x =
      (double) layer->vol_centroid_x / DEG2LONG;
    fl_layer->vol_centroid_y =
      (double) layer->vol_centroid_y / DEG2LONG;
    fl_layer->refl_centroid_x =
      (double) layer->refl_centroid_x / DEG2LONG;
    fl_layer->refl_centroid_y =
      (double) layer->refl_centroid_y / DEG2LONG;
      
  } else {
      
    fl_layer->vol_centroid_x = (double) layer->vol_centroid_x / mult;
    fl_layer->vol_centroid_y = (double) layer->vol_centroid_y / mult;
    fl_layer->refl_centroid_x = (double) layer->refl_centroid_x / mult;
    fl_layer->refl_centroid_y = (double) layer->refl_centroid_y / mult;
    
  }
    
  if (rev >= STORM_V3_MINOR_REV_2) {
    
    fl_layer->area =
      (double) layer->area / (double) gprops->area_mult;
    fl_layer->mass =
      (double) layer->mass / (double) gprops->mass_mult;
    
  } else {
    
    fl_layer->area = (double) layer->area / mult;
    fl_layer->mass = (double) layer->mass / mult;

  }

  return;

}

/*************************************************************************
 *
 * Rfv3DecodeStormParams()
 *
 * Decodes storm params into floating point values
 *
 **************************************************************************/

void Rfv3DecodeStormParams(storm_v3_file_params_t *params,
			   storm_v3_float_params_t *fl_params)
     
{

  int rev;
  double mult;

  rev = params->minor_rev;
  mult = (double) params->mult;

  fl_params->low_dbz_threshold =
    (double) params->low_dbz_threshold / mult;
  fl_params->high_dbz_threshold =
    (double) params->high_dbz_threshold / mult;
  fl_params->dbz_hist_interval =
    (double) params->dbz_hist_interval / mult;
  fl_params->hail_dbz_threshold =
    (double) params->hail_dbz_threshold / mult;
  fl_params->base_threshold = (double) params->base_threshold / mult;
  fl_params->top_threshold = (double) params->top_threshold / mult;
  fl_params->merge_ht_threshold =
    (double) params->merge_ht_threshold / mult;
  fl_params->z_p_coeff = (double) params->z_p_coeff / mult;
  fl_params->z_p_exponent = (double) params->z_p_exponent / mult;
  fl_params->z_m_coeff = (double) params->z_m_coeff / mult;
  fl_params->z_m_exponent = (double) params->z_m_exponent / mult;
  fl_params->sectrip_vert_aspect =
    (double) params->sectrip_vert_aspect / mult;
  fl_params->sectrip_horiz_aspect =
    (double) params->sectrip_horiz_aspect / mult;

  if (rev >= STORM_V3_MINOR_REV_2) {

    fl_params->min_storm_size = (double) params->min_storm_size;
    fl_params->max_storm_size = (double) params->max_storm_size;
    fl_params->sectrip_orientation_error =
      (double) params->sectrip_orientation_error / DEG2LONG;
    fl_params->poly_start_az =
      (double) params->poly_start_az / DEG2LONG;
    fl_params->poly_delta_az =
      (double) params->poly_delta_az / DEG2LONG;

  } else {

    fl_params->min_storm_size = (double) params->min_storm_size / mult;
    fl_params->max_storm_size = (double) params->max_storm_size / mult;
    fl_params->sectrip_orientation_error =
      (double) params->sectrip_orientation_error / mult;
    fl_params->poly_start_az = (double) params->poly_start_az / mult;
    fl_params->poly_delta_az = (double) params->poly_delta_az / mult;

  }

  return;
  
}

/*************************************************************************
 *
 * Rfv3DecodeStormProps()
 *
 * Decodes storm global props into floating point values
 *
 **************************************************************************/

void Rfv3DecodeStormProps(storm_v3_file_params_t *params,
			  storm_v3_file_global_props_t *gprops,
			  storm_v3_float_global_props_t *fl_gprops,
			  int mode)
{

  int rev;
  int i;
  double mult, vort_mult;

  rev = params->minor_rev;
  mult = (double) params->mult;
  vort_mult = (double) params->vort_mult;

  if (mode & STORM_V3_POSITION) {

    fl_gprops->vol_centroid_z = (double) gprops->vol_centroid_z / mult;
    fl_gprops->refl_centroid_z = (double) gprops->refl_centroid_z / mult;
    
    if ((rev >= STORM_V3_MINOR_REV_2) &&
	(params->grid_type == PJG_LATLON)) {

      fl_gprops->vol_centroid_x =
	(double) gprops->vol_centroid_x / DEG2LONG;
      fl_gprops->vol_centroid_y =
	(double) gprops->vol_centroid_y / DEG2LONG;
      fl_gprops->refl_centroid_x =
	(double) gprops->refl_centroid_x / DEG2LONG;
      fl_gprops->refl_centroid_y =
	(double) gprops->refl_centroid_y / DEG2LONG;

    } else {
      
      fl_gprops->vol_centroid_x = (double) gprops->vol_centroid_x / mult;
      fl_gprops->vol_centroid_y = (double) gprops->vol_centroid_y / mult;
      fl_gprops->refl_centroid_x = (double) gprops->refl_centroid_x / mult;
      fl_gprops->refl_centroid_y = (double) gprops->refl_centroid_y / mult;
      
    }
      
    if (rev >= STORM_V3_MINOR_REV_2) {

      fl_gprops->tilt_angle =
	(double) gprops->tilt_angle / DEG2LONG;
      fl_gprops->tilt_dirn =
	(double) gprops->tilt_dirn / DEG2LONG;

    } else {

      fl_gprops->tilt_angle = (double) gprops->tilt_angle / mult;
      fl_gprops->tilt_dirn = (double) gprops->tilt_dirn / mult;
      
    }
    
  } /* STORM_V3_POSITION */

  if (mode & STORM_V3_SIZE) {

    fl_gprops->top = (double) gprops->top / mult;
    fl_gprops->base = (double) gprops->base / mult;
    fl_gprops->dbz_max = (double) gprops->dbz_max / mult;
    fl_gprops->dbz_mean = (double) gprops->dbz_mean / mult;
    fl_gprops->ht_of_dbz_max = (double) gprops->ht_of_dbz_max / mult;
    fl_gprops->vorticity = (double) gprops->vorticity / vort_mult;
    
    if (rev >= STORM_V3_MINOR_REV_2) {

      fl_gprops->volume =
	(double) gprops->volume / (double) gprops->vol_mult;
      fl_gprops->area_mean =
	(double) gprops->area_mean / (double) gprops->area_mult;
      fl_gprops->precip_flux =
	(double) gprops->precip_flux / (double) gprops->flux_mult;
      fl_gprops->mass =
	(double) gprops->mass / (double) gprops->mass_mult;
    
    } else {
      
      fl_gprops->volume = (double) gprops->volume / mult;
      fl_gprops->area_mean = (double) gprops->area_mean / mult;
      fl_gprops->precip_flux = (double) gprops->precip_flux / mult;
      fl_gprops->mass = (double) gprops->mass / mult;
    
    }

  } /* STORM_V3_SIZE */
  
  if (mode & STORM_V3_PRECIP_AREA) {
    
    if (rev >= STORM_V3_MINOR_REV_2) {

      fl_gprops->precip_area =
	(double) gprops->precip_area / (double) gprops->area_mult;
      fl_gprops->precip_area_orientation =
	(double) gprops->precip_area_orientation / DEG2LONG;

    } else {

      fl_gprops->precip_area =
	(double) gprops->precip_area / mult;
      fl_gprops->precip_area_orientation =
	(double) gprops->precip_area_orientation / mult;

    }
    
    if ((rev >= STORM_V3_MINOR_REV_2) &&
	(params->grid_type == PJG_LATLON)) {
      
      fl_gprops->precip_area_centroid_x =
	(double) gprops->precip_area_centroid_x / DEG2LONG;
      fl_gprops->precip_area_centroid_y =
	(double) gprops->precip_area_centroid_y / DEG2LONG;
      fl_gprops->precip_area_minor_sd =
	(double) gprops->precip_area_minor_sd / DEG2LONG;
      fl_gprops->precip_area_major_sd =
	(double) gprops->precip_area_major_sd / DEG2LONG;
      fl_gprops->precip_area_minor_radius =
	(double) gprops->precip_area_minor_radius / DEG2LONG;
      fl_gprops->precip_area_major_radius =
	(double) gprops->precip_area_major_radius / DEG2LONG;
      
    } else {
      
      fl_gprops->precip_area_centroid_x =
	(double) gprops->precip_area_centroid_x / mult;
      fl_gprops->precip_area_centroid_y =
	(double) gprops->precip_area_centroid_y / mult;
      fl_gprops->precip_area_minor_sd =
	(double) gprops->precip_area_minor_sd / mult;
      fl_gprops->precip_area_major_sd =
	(double) gprops->precip_area_major_sd / mult;
      fl_gprops->precip_area_minor_radius =
	(double) gprops->precip_area_minor_radius / mult;
      fl_gprops->precip_area_major_radius =
	(double) gprops->precip_area_major_radius / mult;
      
    }
    
  } /* STORM_V3_PRECIP_AREA */
  
  if (mode & STORM_V3_PROJ_AREA) {
    
    if (rev >= STORM_V3_MINOR_REV_2) {

      fl_gprops->proj_area =
	(double) gprops->proj_area / (double) gprops->area_mult;
      fl_gprops->proj_area_orientation =
	(double) gprops->proj_area_orientation / DEG2LONG;

    } else {

      fl_gprops->proj_area =
	(double) gprops->proj_area / mult;
      fl_gprops->proj_area_orientation =
	(double) gprops->proj_area_orientation / mult;

    }
    
    if ((rev >= STORM_V3_MINOR_REV_2) &&
	(params->grid_type == PJG_LATLON)) {
      
      fl_gprops->proj_area_centroid_x =
	(double) gprops->proj_area_centroid_x / DEG2LONG;
      fl_gprops->proj_area_centroid_y =
	(double) gprops->proj_area_centroid_y / DEG2LONG;
      fl_gprops->proj_area_minor_sd =
	(double) gprops->proj_area_minor_sd / DEG2LONG;
      fl_gprops->proj_area_major_sd =
	(double) gprops->proj_area_major_sd / DEG2LONG;
      fl_gprops->proj_area_minor_radius =
	(double) gprops->proj_area_minor_radius / DEG2LONG;
      fl_gprops->proj_area_major_radius =
	(double) gprops->proj_area_major_radius / DEG2LONG;
      
    } else {
      
      fl_gprops->proj_area_centroid_x =
	(double) gprops->proj_area_centroid_x / mult;
      fl_gprops->proj_area_centroid_y =
	(double) gprops->proj_area_centroid_y / mult;
      fl_gprops->proj_area_minor_sd =
	(double) gprops->proj_area_minor_sd / mult;
      fl_gprops->proj_area_major_sd =
	(double) gprops->proj_area_major_sd / mult;
      fl_gprops->proj_area_minor_radius =
	(double) gprops->proj_area_minor_radius / mult;
      fl_gprops->proj_area_major_radius =
	(double) gprops->proj_area_major_radius / mult;
      
    }
    
  } /* STORM_V3_PROJ_AREA */
  
  if (mode & STORM_V3_PROJ_POLYGON) {

    if ((rev >= STORM_V3_MINOR_REV_2) &&
	(params->grid_type == PJG_LATLON)) {

      for (i = 0; i < params->n_poly_sides; i++) {
	fl_gprops->proj_area_polygon[i] =
	  (double) gprops->proj_area_polygon[i] / DEG2LONG;
      } /* i */

    } else {

      for (i = 0; i < params->n_poly_sides; i++) {
	fl_gprops->proj_area_polygon[i] =
	  (double) gprops->proj_area_polygon[i] / mult;
      } /* i */

    }

  } /* STORM_V3_PROJ_POLYGON */
      
  return;

}

/*************************************************************************
 *
 * Rfv3DecodeStormScan()
 *
 * Decodes storm scan header into floating point values
 *
 **************************************************************************/

void Rfv3DecodeStormScan(storm_v3_file_params_t *params,
			 storm_v3_file_scan_header_t *scan,
			 storm_v3_float_scan_header_t *fl_scan)
     
{

  int rev;
  double mult;

  rev = params->minor_rev;
  mult = (double) params->mult;

  if (rev >= STORM_V3_MINOR_REV_2) {

    fl_scan->datum_longitude =
      (double) scan->datum_longitude / DEG2LONG;
    fl_scan->datum_latitude =
      (double) scan->datum_latitude / DEG2LONG;

  } else {

    fl_scan->datum_longitude =
      (double) scan->datum_longitude / mult;
    fl_scan->datum_latitude =
      (double) scan->datum_latitude / mult;

  }

  fl_scan->min_z = (double) scan->min_z / mult;
  fl_scan->delta_z = (double) scan->delta_z / mult;

  RfDecodeCartParams(&scan->cart, &fl_scan->cart);

  return;
}

/*************************************************************************
 *
 * Rfv3FlushStormFiles()
 *
 * Flushes the storm header and data files
 *
 **************************************************************************/

int Rfv3FlushStormFiles(storm_v3_file_index_t *s_handle,
			char *calling_routine)
     
{
 
  /*
   * flush the header file
   */
  
  if (fflush(s_handle->header_file)) {
    fprintf(stderr, "WARNING - %s:Rfv3FlushStormFiles\n", calling_routine);
    fprintf(stderr, "Flushing storm header file.\n");
    perror(s_handle->header_file_path);
  }

  /*
   * flush the data file
   */
  
  if (fflush(s_handle->data_file)) {
    fprintf(stderr, "WARNING - %s:Rfv3FlushStormFiles\n", calling_routine);
    fprintf(stderr, "Flushing storm data file.\n");
    perror(s_handle->data_file_path);
  }

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * Rfv3FreeStormHeader()
 *
 * frees the storm_v3_file_header_t structure and offset array
 *
 **************************************************************************/

/*ARGSUSED*/

int Rfv3FreeStormHeader(storm_v3_file_index_t *s_handle,
			char *calling_routine)
     
{

  if (s_handle->header_allocated) {
    
    ufree((char *) s_handle->header);
    s_handle->header_allocated = FALSE;

  } /* if (s_handle->header_allocated) */

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * Rfv3FreeStormProps()
 *
 * frees the storm property data arrays
 *
 **************************************************************************/

/*ARGSUSED*/

int Rfv3FreeStormProps(storm_v3_file_index_t *s_handle,
		       char *calling_routine)
     
{
  
  if (s_handle->props_allocated) {
      
    ufree ((char *) s_handle->layer);
    ufree ((char *) s_handle->hist);
    ufree ((char *) s_handle->runs);
    s_handle->max_layers = 0;
    s_handle->max_dbz_intervals = 0;
    s_handle->max_runs = 0;
    s_handle->props_allocated = FALSE;

  } /* if (s_handle->props_allocated) */

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * Rfv3FreeStormScan()
 *
 * frees the scan structures
 *
 **************************************************************************/

/*ARGSUSED*/

int Rfv3FreeStormScan(storm_v3_file_index_t *s_handle,
		      char *calling_routine)
     
{

  if (s_handle->scan_allocated) {

    ufree ((char *) s_handle->scan);
    ufree ((char *) s_handle->gprops);
    s_handle->max_storms = 0;
    s_handle->scan_allocated = FALSE;

  } /* if (s_handle->scan_allocated) */

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * Rfv3FreeStormScanOffsets()
 *
 * frees the scan offset array
 *
 **************************************************************************/

/*ARGSUSED*/

int Rfv3FreeStormScanOffsets(storm_v3_file_index_t *s_handle,
			     char *calling_routine)
     
{

  if (s_handle->n_scans_allocated) {
    
    ufree((char *) s_handle->scan_offsets);
    s_handle->n_scans_allocated = 0;

  }

  return (R_SUCCESS);

}

/*************************************************************************
 *
 * Rfv3OpenStormFiles()
 *
 * Opens the storm header and data files
 *
 * The storm header file path must have been set
 *
 **************************************************************************/

#define THIS_ROUTINE "Rfv3OpenStormFiles"

int Rfv3OpenStormFiles(storm_v3_file_index_t *s_handle,
		       char *mode,
		       char *header_file_path,
		       char *data_file_ext,
		       char *calling_routine)
     
{
 
  char file_label[R_FILE_LABEL_LEN];
  char calling_sequence[MAX_SEQ];
  char tmp_path[MAX_PATH_LEN];
  char data_file_path[MAX_PATH_LEN];
  char *chptr;

  /*
   * set up calling sequence
   */
  
  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * close files
   */

  Rfv3CloseStormFiles(s_handle, calling_sequence);
  
  /*
   * open the header file
   */
  
  if ((s_handle->header_file =
       Rf_fopen_uncompress(header_file_path, mode)) == NULL) {
    fprintf(stderr, "ERROR - %s:Rfv3OpenStormFiles\n", calling_routine);
    fprintf(stderr, "Cannot open storm header file '%s'\n",
	    header_file_path);
    return (R_FAILURE);
  }

  /*
   * compute the data file name
   */
   
  if (*mode == 'r') {

    /*
     * read the header if the file is opened for reading
     */
   
    if (Rfv3ReadStormHeader(s_handle, calling_sequence))
      return (R_FAILURE);

    /*
     * compute the file path from the header file path and
     * the data file name
     */

    strncpy(tmp_path, header_file_path, MAX_PATH_LEN);

    /*
     * if dir path has slash, get pointer to that and end the sting
     * immediately after
     */
    
    if ((chptr = strrchr(tmp_path, '/')) != NULL) {
      *(chptr + 1) = '\0';
      sprintf(data_file_path, "%s%s", tmp_path,
	      s_handle->header->data_file_name);
    } else {
      strcpy(data_file_path, s_handle->header->data_file_name);
    }

  } else {

    /*
     * file opened for writing, use ext to compute file name
     */

    if (data_file_ext == NULL) {
      fprintf(stderr, "ERROR - %s\n", calling_sequence);
      fprintf(stderr,
	      "Must provide data file extension for file creation\n");
      return (R_FAILURE);
    }

    strncpy(tmp_path, header_file_path, MAX_PATH_LEN);
    
    if ((chptr = strrchr(tmp_path, '.')) == NULL) {
      fprintf(stderr, "ERROR - %s\n", calling_sequence);
      fprintf(stderr, "Header file must have extension : %s\n",
	      header_file_path);
      return (R_FAILURE);
    }

    *(chptr + 1) = '\0';
    sprintf(data_file_path, "%s%s", tmp_path, data_file_ext);

  } /* if (*mode == 'r') */
    
  /*
   * open the data file
   */
  
  if ((s_handle->data_file =
       Rf_fopen_uncompress(data_file_path, mode)) == NULL) {
    fprintf(stderr, "ERROR - %s:Rfv3OpenStormFiles\n", calling_routine);
    fprintf(stderr, "Cannot open storm data file '%s'\n",
	    data_file_path);
    return (R_FAILURE);
  }

  /*
   * In write mode, write file labels
   */
   
  if (*mode == 'w') {

    /*
     * header file
     */

    memset ((void *) file_label,
	    (int) 0, (size_t)  R_FILE_LABEL_LEN);
    strcpy(file_label, s_handle->header_file_label);

    if (ufwrite(file_label, (int) sizeof(char),
		(int) R_FILE_LABEL_LEN,
		s_handle->header_file) != R_FILE_LABEL_LEN) {
    
      fprintf(stderr, "ERROR - %s:%s\n",
	      s_handle->prog_name, calling_sequence);
      fprintf(stderr, "Writing storm header file label.\n");
      perror(s_handle->header_file_path);
      return (R_FAILURE);
      
    }

    /*
     * data file
     */

    memset ((void *) file_label,
	    (int) 0, (size_t)  R_FILE_LABEL_LEN);
    strcpy(file_label, s_handle->data_file_label);

    if (ufwrite(file_label, (int) sizeof(char),
		(int) R_FILE_LABEL_LEN,
		s_handle->data_file) != R_FILE_LABEL_LEN) {
    
      fprintf(stderr, "ERROR - %s:%s\n",
	      s_handle->prog_name, calling_sequence);
      fprintf(stderr, "Writing storm data file label.\n");
      perror(s_handle->data_file_path);
      return (R_FAILURE);
      
    }

  } else {

    /*
     * read mode
     */


    if (ufread(file_label, (int) sizeof(char),
	       (int) R_FILE_LABEL_LEN,
	       s_handle->data_file) != R_FILE_LABEL_LEN) {
      fprintf(stderr, "ERROR - %s:%s:Rfv3ReadStormHeader\n",
	      s_handle->prog_name, calling_routine);
      fprintf(stderr, "Reading data file label.\n");
      perror(s_handle->data_file_path);
      return (R_FAILURE);
    }
    
    if (s_handle->data_file_label == NULL) {
      
      s_handle->data_file_label = (char *) ucalloc
	((ui32) R_FILE_LABEL_LEN, (ui32) sizeof(char));
      memset ((void *)  s_handle->data_file_label,
	      (int) 0, (size_t)  R_FILE_LABEL_LEN);
      strcpy(s_handle->data_file_label, file_label);
      
    } /* if (s_handle->header_file_label != NULL) */
  
  } /* if (*mode == 'w') */

  /*
   * set header file path in index
   */
  
  if (s_handle->header_file_path != NULL)
    ufree ((char *) s_handle->header_file_path);

  s_handle->header_file_path = (char *) umalloc
    ((ui32) (strlen(header_file_path)+ 1));

  strcpy(s_handle->header_file_path, header_file_path);

  /*
   * set data file path in index
   */
  
  if (s_handle->data_file_path != NULL)
    ufree ((char *) s_handle->data_file_path);

  s_handle->data_file_path = (char *) umalloc
    ((ui32) (strlen(data_file_path)+ 1));

  strcpy(s_handle->data_file_path, data_file_path);

  return (R_SUCCESS);

}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * Rfv3ReadStormHeader()
 *
 * reads in the storm_v3_file_header_t structure from a storm
 * properties file.
 *
 * Note - space for the header is allocated if header_allocated is FALSE.
 * If not, previous allocation is assumed.
 *
 * If the file label is passed in as NULL, the storm file label
 * from the file is stored. If the label is non-null, the two are
 * compared and an error is returned if they are different.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "Rfv3ReadStormheader"

int Rfv3ReadStormHeader(storm_v3_file_index_t *s_handle,
			char *calling_routine)
     
{
  
  char header_file_label[R_FILE_LABEL_LEN];
  char calling_sequence[MAX_SEQ];
  si32 nbytes_char;
  si32 n_scans;
  
  /*
   * set up calling sequence
   */
  
  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * allocate space for s_handle->file_header
   */
  
  if (!s_handle->header_allocated)
    if (Rfv3AllocStormHeader(s_handle, calling_sequence))
      return (R_FAILURE);
  
  /*
   * rewind file
   */
  
  fseek(s_handle->header_file, 0L, SEEK_SET);
  
  /*
   * read in header file label
   */
  
  if (ufread(header_file_label, (int) sizeof(char),
	     (int) R_FILE_LABEL_LEN,
	     s_handle->header_file) != R_FILE_LABEL_LEN) {
    fprintf(stderr, "ERROR - %s:%s:Rfv3ReadStormHeader\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading header file label.\n");
    perror(s_handle->header_file_path);
    return (R_FAILURE);
  }
  
  /*
   * if the label passed in is non-null, check that the label matches
   * that expected. If the label is null, store the file label there.
   */
  
  if (s_handle->header_file_label != NULL) {
    
    if (strcmp(header_file_label, s_handle->header_file_label)) {
      fprintf(stderr, "ERROR - %s:%s:Rfv3ReadStormHeader\n",
	      s_handle->prog_name, calling_routine);
      fprintf(stderr,
	      "File does not contain correct type storm properties file.\n");
      fprintf(stderr, "File label is '%s'\n", header_file_label);
      fprintf(stderr, "File label should be '%s'.\n",
	      s_handle->header_file_label);
      
      return (R_FAILURE);
      
    }
    
  } else {
    
    s_handle->header_file_label = (char *) ucalloc
      ((ui32) R_FILE_LABEL_LEN, (ui32) sizeof(char));
    
    memset ((void *)  s_handle->header_file_label,
            (int) 0, (size_t)  R_FILE_LABEL_LEN);
    strcpy(s_handle->header_file_label, header_file_label);
    
  } /* if (s_handle->header_file_label != NULL) */
  
  /*
   * read in header
   */
  
  if (ufread((char *) s_handle->header,
	     sizeof(storm_v3_file_header_t),
	     1, s_handle->header_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:Rfv3ReadStormHeader\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading storm file header structure.\n");
    perror(s_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * decode the structure into host byte order - the file
   * is stored in network byte order
   */
  
  nbytes_char = s_handle->header->nbytes_char;
  
  BE_to_array_32((ui32 *) &nbytes_char,
		 (ui32) sizeof(si32));
  
  BE_to_array_32((ui32 *) s_handle->header,
		 (ui32) (sizeof(storm_v3_file_header_t) - nbytes_char));
  
  /*
   * allocate space for scan offsets array
   */
  
  n_scans = s_handle->header->n_scans;
  
  if (Rfv3AllocStormScanOffsets(s_handle, n_scans, calling_sequence))
    return (R_FAILURE);
  
  /*
   * read in scan offset
   */
  
  if (ufread((char *) s_handle->scan_offsets,
	     sizeof(si32),
	     (int) n_scans,
	     s_handle->header_file) != n_scans) {
    
    fprintf(stderr, "ERROR - %s:%s:Rfv3ReadStormHeader\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading storm file scan offsets.\n");
    perror(s_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * decode the offset array from network byte order into host byte order
   */
  
  BE_to_array_32((ui32 *) s_handle->scan_offsets,
		 (si32) (n_scans * sizeof(si32)));
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * Rfv3ReadStormProps()
 *
 * reads in the storm property data for a given storm in a scan, given the
 * scan and index structures as inputs.
 *
 * Space for the arrays of structures is allocated if props_allocated is
 * FALSE. A check is kept on the max number of layers and
 * dbz intervals for which space is allocated.
 * If this is exceeded during a call in which space
 * is not to be allocated, the allocation is increased.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "Rfv3ReadStormProps"

int Rfv3ReadStormProps(storm_v3_file_index_t *s_handle,
		       si32 storm_num, char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  si32 n_layers, n_dbz_intervals, n_runs;
  si32 file_mark;
  
  /*
   * set up calling sequence
   */
  
  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * store the current file location
   */
  
  file_mark = ftell(s_handle->data_file);
  
  /*
   * store storm number
   */
  
  s_handle->storm_num = storm_num;
  
  /*
   * allocate or realloc mem
   */
  
  n_layers = s_handle->gprops[storm_num].n_layers;
  n_dbz_intervals = s_handle->gprops[storm_num].n_dbz_intervals;
  n_runs = s_handle->gprops[storm_num].n_runs;
  
  if (Rfv3AllocStormProps(s_handle, n_layers,
			  n_dbz_intervals, n_runs,
			  calling_sequence))
    return (R_FAILURE);
  
  /*
   * return early if nstorms is zero
   */
  
  if (s_handle->scan->nstorms == 0)
    return (R_SUCCESS);
  
  /*
   * move to layer data position in file
   */
  
  fseek(s_handle->data_file,
	s_handle->gprops[storm_num].layer_props_offset, SEEK_SET);
  
  /*
   * read in layer props
   */
  
  if (ufread((char *) s_handle->layer,
	     sizeof(storm_v3_file_layer_props_t),
	     (int) n_layers,
	     s_handle->data_file) != n_layers) {
    
    fprintf(stderr, "ERROR - %s:%s:Rfv3ReadStormProps\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading layer props - %d layers.\n", n_layers);
    fprintf(stderr, "Storm number %d, scan number %d\n",
	    storm_num, s_handle->scan->scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  /*
   * decode layer props from network byte order into host byte order
   */
  
  BE_to_array_32((ui32 *) s_handle->layer,
		 (ui32) (n_layers * sizeof(storm_v3_file_layer_props_t)));
  
  
  /*
   * move to hist data position in file
   */
  
  fseek(s_handle->data_file,
	s_handle->gprops[storm_num].dbz_hist_offset, SEEK_SET);
  
  /*
   * read in histogram data
   */
  
  if (ufread((char *) s_handle->hist,
	     sizeof(storm_v3_file_dbz_hist_t),
	     (int) n_dbz_intervals,
	     s_handle->data_file) != n_dbz_intervals) {
    
    fprintf(stderr, "ERROR - %s:%s:Rfv3ReadStormProps\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading dbz histogram - %d intervals.\n",
	    n_dbz_intervals);
    fprintf(stderr, "Storm number %d, scan number %d\n",
	    storm_num, s_handle->scan->scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  /*
   * decode histogram data from network byte order into host byte order
   */
  
  BE_to_array_32((ui32 *) s_handle->hist,
		 (ui32) (n_dbz_intervals * sizeof(storm_v3_file_dbz_hist_t)));
  
  /*
   * move to run data position in file
   */
  
  fseek(s_handle->data_file,
	s_handle->gprops[storm_num].runs_offset, SEEK_SET);
  
  /*
   * read in runs
   */
  
  if (ufread((char *) s_handle->runs,
	     sizeof(storm_v3_file_run_t),
	     (int) n_runs,
	     s_handle->data_file) != n_runs) {
    
    fprintf(stderr, "ERROR - %s:%s:Rfv3ReadStormProps\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading runs - %ld runs.\n", (long) n_runs);
    fprintf(stderr, "Storm number %ld, scan number %ld\n",
	    (long) storm_num, (long) s_handle->scan->scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  /*
   * decode runs from network byte order into host byte order
   */
  
  BE_to_array_16((ui16 *) s_handle->runs,
		 (ui32) (n_runs * sizeof(storm_v3_file_run_t)));
  
  /*
   * go back to original file location
   */
  
  fseek(s_handle->data_file, file_mark, SEEK_SET);
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * Rfv3ReadStormScan()
 *
 * reads in the scan info for a particular scan in a storm properties
 * file.
 *
 * Space for the scan structure and the storm global props structure is
 * allocated
 * if acsn_allocated is FALSE. A check is kept on the max number of storms for
 * which space is allocated. If this is exceeded during a call in which space
 * is not to be allocated, the allocation is increased.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "Rfv3ReadStormScan"

int Rfv3ReadStormScan(storm_v3_file_index_t *s_handle, si32 scan_num,
		      char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  si32 nbytes_char;
  si32 nstorms;
  si32 file_mark;
  storm_v3_file_scan_header_t scan;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * store the current file location
   */
  
  file_mark = ftell(s_handle->data_file);
  
  /*
   * move to scan position in file
   */
  
  if (s_handle->scan_offsets) {
    fseek(s_handle->data_file,
	  s_handle->scan_offsets[scan_num], SEEK_SET);
  }
  
  /*
   * read in scan struct
   */
  
  if (ufread((char *) &scan,
	     sizeof(storm_v3_file_scan_header_t),
	     1, s_handle->data_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:Rfv3ReadStormScan\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading scan number %d\n", scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  /*
   * decode the scan struct from network byte order into host byte order
   */
  
  nbytes_char = scan.nbytes_char;
  
  BE_to_array_32((ui32 *) &nbytes_char,
		 (ui32) sizeof(si32));
  
  BE_to_array_32((ui32 *) &scan,
		 (ui32) (sizeof(storm_v3_file_scan_header_t) - nbytes_char));
  
  nstorms = scan.nstorms;
  
  /*
   * allocate or reallocate
   */
  
  if (Rfv3AllocStormScan(s_handle, nstorms, calling_sequence))
    return (R_FAILURE);
  
  /*
   * copy scan header into storm file handle
   */
  
  memcpy ((void *) s_handle->scan,
          (void *) &scan,
          (size_t) sizeof(storm_v3_file_scan_header_t));
  
  /*
   * return early if nstorms is zero
   */
  
  if (nstorms == 0)
    return (R_SUCCESS);
  
  /*
   * move to gprops position in file
   */
  
  fseek(s_handle->data_file,
	s_handle->scan->gprops_offset, SEEK_SET);
  
  /*
   * read in global props
   */
  
  if (ufread((char *) s_handle->gprops,
	     sizeof(storm_v3_file_global_props_t),
	     (int) nstorms, s_handle->data_file) != nstorms) {
    
    fprintf(stderr, "ERROR - %s:%s:Rfv3ReadStormScan\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Reading global props for scan number %d\n", scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  /*
   * decode global props from network byte order into host byte order
   */
  
  BE_to_array_32((ui32 *) s_handle->gprops,
		 (ui32) (nstorms * sizeof(storm_v3_file_global_props_t)));
  
  /*
   * go back to original file location
   */
  
  fseek(s_handle->data_file, file_mark, SEEK_SET);
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * Rfv3SeekEndStormData()
 *
 * seeks to the end of the storm data in data file
 *
 **************************************************************************/

int Rfv3SeekEndStormData(storm_v3_file_index_t *s_handle,
			 char *calling_routine)
     
{

  if (fseek(s_handle->data_file,
	    (si32) R_FILE_LABEL_LEN,
	    SEEK_END)) {
    
    fprintf(stderr, "ERROR - %s:%s:Rfv3SeekEndStormData\n",
	    s_handle->prog_name, calling_routine);
    
    fprintf(stderr, "Failed on seek.\n");
    perror(s_handle->data_file_path);

    return (R_FAILURE);

  } else {

    return (R_SUCCESS);

  }

}

/*************************************************************************
 *
 * Rfv3SeekStartStormData()
 *
 * seeks to the start of the storm data in data file
 *
 **************************************************************************/

int Rfv3SeekStartStormData(storm_v3_file_index_t *s_handle,
			   char *calling_routine)
     
{

  if (fseek(s_handle->data_file,
	    (si32) R_FILE_LABEL_LEN,
	    SEEK_SET) != 0) {
    
    fprintf(stderr, "ERROR - %s:%s:Rfv3SeekStartStormData\n",
	    s_handle->prog_name, calling_routine);
    
    fprintf(stderr, "Failed on seek.\n");
    perror(s_handle->data_file_path);

    return (R_FAILURE);

  } else {

    return (R_SUCCESS);

  }

}

/*************************************************************************
 *
 * Rfv3StormGridType()
 *
 * Returns the grid type in PJG language
 *
 **************************************************************************/

int Rfv3StormGridType(storm_v3_file_params_t *params)
     
{
  
  if (params->minor_rev >= STORM_V3_MINOR_REV_2) {
    return (params->grid_type);
  } else {
    if (params->grid_type == OLD_STORM_V3_GRIDTYP_FLAT) {
      return (PJG_FLAT);
    } else {
      return (PJG_LATLON);
    }
  }
  
}

/*************************************************************************
 *
 * Rfv3WriteStormHeader()
 *
 * writes the storm_v3_file_header_t structure to a storm
 * properties file.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "Rfv3WriteStormHeader"

int Rfv3WriteStormHeader(storm_v3_file_index_t *s_handle,
			 char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  char file_label[R_FILE_LABEL_LEN];
  char *cptr;
  storm_v3_file_header_t header;
  si32 *scan_offsets;
  si32 n_scans;
  struct stat data_stat;
  date_time_t dtime;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * get data file size
   */

  fflush(s_handle->data_file);
  stat (s_handle->data_file_path, &data_stat);
  s_handle->header->data_file_size = data_stat.st_size;
  
  /*
   * copy file label
   */
  
  memset ((void *) file_label,
          (int) 0, (size_t)  R_FILE_LABEL_LEN);
  strcpy(file_label, s_handle->header_file_label);

  dtime.unix_time = time(NULL);
  uconvert_from_utime(&dtime);
  Rfdtime2rtime(&dtime, (radtim_t *) &s_handle->header->file_time);

  /*
   * copy in the file names, checking whether the path has a
   * delimiter or not, and only copying after the delimiter
   */
  
  if ((cptr = strrchr(s_handle->header_file_path, '/')) != NULL)
    strncpy(s_handle->header->header_file_name, (cptr + 1), R_LABEL_LEN);
  else
    strncpy(s_handle->header->header_file_name,
	    s_handle->header_file_path, R_LABEL_LEN);
      
  
  if ((cptr = strrchr(s_handle->data_file_path, '/')) != NULL)
    strncpy(s_handle->header->data_file_name, (cptr + 1), R_LABEL_LEN);
  else
    strncpy(s_handle->header->data_file_name,
	    s_handle->data_file_path, R_LABEL_LEN);
      
  
  /*
   * make local copies of the global file header and scan offsets
   */
  
  memcpy ((void *) &header,
          (void *) s_handle->header,
          (size_t) sizeof(storm_v3_file_header_t));
  
  n_scans = s_handle->header->n_scans;
  
  scan_offsets = (si32 *) umalloc
    ((ui32) (n_scans * sizeof(si32)));
  
  memcpy ((void *) scan_offsets,
          (void *) s_handle->scan_offsets,
          (size_t) (n_scans * sizeof(si32)));
  
  /*
   * encode the header and scan offset array into network byte order
   */
  
  ustr_clear_to_end(header.header_file_name, R_LABEL_LEN);
  ustr_clear_to_end(header.data_file_name, R_LABEL_LEN);
  header.nbytes_char = N_STORM_V3_HEADER_LABELS * R_LABEL_LEN;
  BE_from_array_32((ui32 *) &header,
		   (ui32) (sizeof(storm_v3_file_header_t) -
			   header.nbytes_char));
  BE_from_array_32((ui32 *) scan_offsets,
		   (si32) (n_scans * sizeof(si32)));
  
  /*
   * write label to file
   */
  
  fseek(s_handle->header_file, (si32) 0, SEEK_SET);
  ustr_clear_to_end(file_label, R_FILE_LABEL_LEN);

  if (ufwrite(file_label, (int) sizeof(char),
	      (int) R_FILE_LABEL_LEN,
	      s_handle->header_file) != R_FILE_LABEL_LEN) {
    
    fprintf(stderr, "ERROR - %s:%s:Rfv3WriteStormHeader\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing storm header file label.\n");
    perror(s_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * write header to file
   */
  
  if (ufwrite((char *) &header,
	      sizeof(storm_v3_file_header_t),
	      1, s_handle->header_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:Rfv3WriteStormHeader\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing storm file header structure.\n");
    perror(s_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * write scan offsets to file
   */
  
  if (ufwrite((char *) scan_offsets,
	      sizeof(si32),
	      (int) n_scans,
	      s_handle->header_file) != n_scans) {
    
    fprintf(stderr, "ERROR - %s:%s:Rfv3WriteStormHeader\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing storm file scan offsets.\n");
    perror(s_handle->header_file_path);
    return (R_FAILURE);
    
  }
  
  /*
   * flush the file buffer
   */
  
  if (Rfv3FlushStormFiles(s_handle, calling_sequence)) {
    return (R_FAILURE);
  }

  /*
   * free resources
   */

  ufree ((char *) scan_offsets);
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE

/*************************************************************************
 *
 * Rfv3WriteStormProps()
 *
 * writes the storm layer property and histogram data for a storm, at
 * the current file location.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

int Rfv3WriteStormProps(storm_v3_file_index_t *s_handle, si32 storm_num,
			char *calling_routine)
     
{
  
  si32 offset;
  si32 n_layers, n_dbz_intervals, n_runs;
  storm_v3_file_layer_props_t *layer;
  storm_v3_file_dbz_hist_t *hist;
  storm_v3_file_run_t *runs;
  
  n_layers = s_handle->gprops[storm_num].n_layers;
  n_dbz_intervals = s_handle->gprops[storm_num].n_dbz_intervals;
  n_runs = s_handle->gprops[storm_num].n_runs;
  
  /*
   * set layer props offset
   */
  
  offset = ftell(s_handle->data_file);
  s_handle->gprops[storm_num].layer_props_offset = offset;
  
  /*
   * if this is the first storm, store the first_offset value
   * in the scan header
   */
  
  if (storm_num == 0)
    s_handle->scan->first_offset = offset;
  
  /*
   * copy layer props to local variable
   */
  
  layer = (storm_v3_file_layer_props_t *)
    umalloc ((ui32) (n_layers * sizeof(storm_v3_file_layer_props_t)));
  
  memcpy ((void *)  layer,
          (void *)  s_handle->layer,
          (size_t)  (n_layers * sizeof(storm_v3_file_layer_props_t)));
  
  /*
   * code layer props into network byte order from host byte order
   */
  
  BE_from_array_32((ui32 *) layer,
		   (ui32) (n_layers * sizeof(storm_v3_file_layer_props_t)));
  
  /*
   * write layer props
   */
  
  if (ufwrite((char *) s_handle->layer,
	      sizeof(storm_v3_file_layer_props_t),
	      (int) n_layers, s_handle->data_file) != n_layers) {
    
    fprintf(stderr, "ERROR - %s:%s:Rfv3WriteStormProps\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing layer props - %d layers.\n", n_layers);
    fprintf(stderr, "Storm number %d, scan number %d\n",
	    storm_num, s_handle->scan->scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  /*
   * set dbz hist offset
   */
  
  offset = ftell(s_handle->data_file);
  s_handle->gprops[storm_num].dbz_hist_offset = offset;
  
  /*
   * copy histogram data to local variable
   */
  
  hist = (storm_v3_file_dbz_hist_t *)
    umalloc((ui32) n_dbz_intervals * sizeof(storm_v3_file_dbz_hist_t));
  
  memcpy ((void *)  hist,
          (void *)  s_handle->hist,
          (size_t)  (n_dbz_intervals * sizeof(storm_v3_file_dbz_hist_t)));
  
  /*
   * encode histogram data to network byte order from host byte order
   */
  
  BE_from_array_32((ui32 *) s_handle->hist,
		   (ui32) (n_dbz_intervals *
			   sizeof(storm_v3_file_dbz_hist_t)));
  
  /*
   * write in histogram data
   */
  
  if (ufwrite((char *) s_handle->hist,
	      sizeof(storm_v3_file_dbz_hist_t),
	      (int) n_dbz_intervals,
	      s_handle->data_file) != n_dbz_intervals) {
    
    fprintf(stderr, "ERROR - %s:%s:Rfv3WriteStormProps\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing dbz histogram - %d intervals.\n",
	    n_dbz_intervals);
    fprintf(stderr, "Storm number %d, scan number %d\n",
	    storm_num, s_handle->scan->scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  /*
   * set run offset
   */
  
  offset = ftell(s_handle->data_file);
  s_handle->gprops[storm_num].runs_offset = offset;
  
  /*
   * copy runs to local variable
   */
  
  runs = (storm_v3_file_run_t *)
    umalloc ((ui32) (n_runs * sizeof(storm_v3_file_run_t)));
  
  memcpy ((void *)  runs,
          (void *)  s_handle->runs,
          (size_t)  (n_runs * sizeof(storm_v3_file_run_t)));
  
  /*
   * code run props into network byte order from host byte order
   */
  
  BE_from_array_16((ui16 *) runs,
		   (ui32) (n_runs * sizeof(storm_v3_file_run_t)));
  
  /*
   * write runs
   */
  
  if (ufwrite((char *) s_handle->runs,
	      sizeof(storm_v3_file_run_t),
	      (int) n_runs, s_handle->data_file) != n_runs) {
    
    fprintf(stderr, "ERROR - %s:%s:Rfv3WriteStormProps\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing runs - %ld runs.\n", (long) n_runs);
    fprintf(stderr, "Storm number %ld, scan number %ld\n",
	    (long) storm_num, (long) s_handle->scan->scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  ufree((char *) layer);
  ufree((char *) hist);
  ufree((char *) runs);
  
  return (R_SUCCESS);
  
}


/*************************************************************************
 *
 * Rfv3WriteStormScan()
 *
 * writes scan header and global properties for a particular scan in a storm
 * properties file. Performs the writes from the current file location.
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

#define THIS_ROUTINE "Rfv3WriteStormScan"

int Rfv3WriteStormScan(storm_v3_file_index_t *s_handle, si32 scan_num,
		       char *calling_routine)
     
{
  
  char calling_sequence[MAX_SEQ];
  si32 nstorms;
  si32 offset;
  storm_v3_file_scan_header_t scan;
  storm_v3_file_global_props_t *gprops;
  
  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);
  
  /*
   * if nstorms is greater than zero, write global props to file
   */
  
  nstorms = s_handle->scan->nstorms;
  
  if (nstorms > 0) {
    
    /*
     * get gprops position in file
     */
    
    s_handle->scan->gprops_offset = ftell(s_handle->data_file);
    
    /*
     * allocate space for global props copy
     */
    
    gprops = (storm_v3_file_global_props_t *)
      umalloc((ui32) (nstorms * sizeof(storm_v3_file_global_props_t)));
    
    /*
     * make local copy of gprops and encode into network byte order
     */
    
    memcpy ((void *)  gprops,
            (void *)  s_handle->gprops,
            (size_t)  nstorms * sizeof(storm_v3_file_global_props_t));
    
    BE_to_array_32((ui32 *) gprops,
		   (ui32) (nstorms * sizeof(storm_v3_file_global_props_t)));
    
    /*
     * write in global props
     */
    
    if (ufwrite((char *) gprops,
		sizeof(storm_v3_file_global_props_t),
		(int) nstorms, s_handle->data_file) != nstorms) {
      
      fprintf(stderr, "ERROR - %s:%s:Rfv3WriteStormScan\n",
	      s_handle->prog_name, calling_routine);
      fprintf(stderr, "Writing global props for scan number %d\n", scan_num);
      perror(s_handle->data_file_path);
      return(R_FAILURE);
      
    }
    
    /*
     * free up global props copy
     */
    
    ufree((char *) gprops);
    
  } /* if (nstorms > 0) */
  
  /*
   * get scan position in file
   */
  
  if (Rfv3AllocStormScanOffsets(s_handle, (si32) (scan_num + 1),
				calling_sequence))
    return (R_FAILURE);

  offset = ftell(s_handle->data_file);
  s_handle->scan_offsets[scan_num] = offset;
  
  /*
   * set last scan offset
   */
  
  s_handle->scan->last_offset = offset + sizeof(storm_v3_file_scan_header_t) - 1;
  
  /*
   * copy scan header to local variable, and encode. Note that the 
   * character data at the end of the struct is not encoded
   */
  
  memcpy ((void *)  &scan,
          (void *)  s_handle->scan,
          (size_t)  sizeof(storm_v3_file_scan_header_t));
  
  scan.cart.nbytes_char =
    N_CART_PARAMS_LABELS * R_LABEL_LEN;
  
  scan.nbytes_char = scan.cart.nbytes_char;
  
  ustr_clear_to_end(scan.cart.unitsx, R_LABEL_LEN);
  ustr_clear_to_end(scan.cart.unitsy, R_LABEL_LEN);
  ustr_clear_to_end(scan.cart.unitsz, R_LABEL_LEN);

  BE_from_array_32((ui32 *) &scan,
		   (ui32) (sizeof(storm_v3_file_scan_header_t) -
			   scan.nbytes_char));
  
  /*
   * write in scan struct
   */
  
  if (ufwrite((char *) s_handle->scan,
	      sizeof(storm_v3_file_scan_header_t), 1,
	      s_handle->data_file) != 1) {
    
    fprintf(stderr, "ERROR - %s:%s:Rfv3WriteStormScan\n",
	    s_handle->prog_name, calling_routine);
    fprintf(stderr, "Writing scan number %d\n", scan_num);
    perror(s_handle->data_file_path);
    return(R_FAILURE);
    
  }
  
  return (R_SUCCESS);
  
}

#undef THIS_ROUTINE
