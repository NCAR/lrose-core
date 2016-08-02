// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// GridCont.cc
//
// Contingency grid object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
///////////////////////////////////////////////////////////////

#include "GridCont.h"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/utim.h>
using namespace std;

//////////////
// Constructor

GridCont::GridCont(char *prog_name, Params *params) : Cont(prog_name, params)

{

  // init intermediate grid if needed

  /*
  if (!(_params->relaxed_contingency || !_params->use_specified_grid) &&
      _params->output_intermediate_grids) {
    _useInterGrids = TRUE;
  } else {
    _useInterGrids = FALSE;
  }
  */

  if (_params->output_intermediate_grids) {
    _useInterGrids = TRUE;
  } else {
    _useInterGrids = FALSE;
  }

  if (_useInterGrids) {
    _initInterGrid();
  }

}

/////////////
// destructor

GridCont::~GridCont()

{

  // free MDV dataset
  /* No - causes core dump. Niles.
  if (_useInterGrids) {
    MDV_free_dataset(&_interDataset);
  }
  */

}

////////////////////////////
// update()
//
// Update contingency table
//
// Returns 0 on success, -1 on failure

int GridCont::update(char *forecast_file_path)

{
  
  // load up computation grids

  if (_loadGrids(forecast_file_path)) {
    return (-1);
  }

  // get the cont file based on the forecast file
  
  if (_params->output_scan_cont) {
    if (_openContFile(_forecastGrid->timeCent)) {
      return (-1);
    }
  }

  // if required, print out start of line for scan-by-scan
  // contingency data

  if (_params->output_scan_cont) {

    UTIMstruct time_struct;
    UTIMunix_to_date(_forecastGrid->timeCent, &time_struct);

    fprintf(_scanContFile, "%ld %ld %ld %ld %ld %ld %g %g ",
	    time_struct.year, time_struct.month, time_struct.day,
	    time_struct.hour, time_struct.min, time_struct.sec,
	    (double) _params->forecast_lead_time / 60.0,
	    _params->forecast_level_lower);

    fflush(_scanContFile);

  }

  // compute contingency
  
  if (_params->relaxed_contingency) {
    _computeRelaxed();
  } else {
    _computeStrict();
  }

  if (_params->output_scan_cont) {
    fprintf(_scanContFile, "\n");
  }

  /* This call now takes place later. Niles.
  if (_useInterGrids) {
    _outputInterGrids();
  }
  */

  return (0);

}

//////////
// print()
//

void GridCont::print(FILE *out)

{

  double w, x, y, z;
  double pod, pod_denom;
  double pod_no, pod_no_denom;
  double far, far_denom;
  double csi, csi_denom;
  double hss, hss_denom;
  double fcst_bias;

  x = _cont.n_success;
  y = _cont.n_failure;
  z = _cont.n_false_alarm;
  w = _cont.n_non_event;

  pod_denom = x + y;
  pod_no_denom = w + z;
  far_denom = x + z;
  csi_denom = x + y + z;
  hss_denom =  (y * y) + (z * z) + (2.0 * x * w) + (y + z) * (x + w);

  if (pod_denom > 0)
    pod = x / pod_denom;
  else
    pod = 0.0;

  if (pod_no_denom > 0)
    pod_no = w / pod_no_denom;
  else 
    pod_no = 0.0;

  if (far_denom > 0)
    far = z / far_denom;
  else
    far = 0.0;

  if (csi_denom > 0)
    csi = x / csi_denom;
  else
    csi = 0.0;

  if (hss_denom > 0)
    hss = (2.0 * (x * w - y * z)) / hss_denom;
  else
    hss = 0.0;

  if (_cont.n_truth > 0)
    fcst_bias = _cont.n_forecast / _cont.n_truth;
  else
    fcst_bias = 0.0;

  fprintf(out, "n_forecast    : %g\n", _cont.n_forecast);
  fprintf(out, "n_truth       : %g\n", _cont.n_truth);
  fprintf(out, "n_success     : %g\n", _cont.n_success);
  fprintf(out, "n_failure     : %g\n", _cont.n_failure);
  fprintf(out, "n_false_alarm : %g\n", _cont.n_false_alarm);
  fprintf(out, "n_non_event   : %g\n", _cont.n_non_event);
  
  fprintf(out, "\n");

  fprintf(out, "POD           : %g\n", pod);
  fprintf(out, "POD_NO        : %g\n", pod_no);
  fprintf(out, "FAR           : %g\n", far);
  fprintf(out, "CSI           : %g\n", csi);
  fprintf(out, "HSS           : %g\n", hss);
  fprintf(out, "FCST_BIAS     : %g\n", fcst_bias);
  
}

////////////////////////////////
// _initInterGrid()
//
// Initialize intermediate grid

void GridCont::_initInterGrid()

{

  MDV_master_header_t *master_hdr;
  MDV_field_header_t  *field_hdr;
  
  int field;
  
  /*
   * Initialize the dataset.
   */

  MDV_init_dataset(&_interDataset);
  
  /*
   * Initialize the master header.
   */

  master_hdr =
    (MDV_master_header_t *)umalloc(sizeof(MDV_master_header_t));

  MDV_init_master_header(master_hdr);
  
  master_hdr->data_dimension = 2;
  master_hdr->data_collection_type = MDV_DATA_MIXED;
  master_hdr->native_vlevel_type = MDV_VERT_TYPE_MIXED;
  master_hdr->vlevel_type = MDV_VERT_TYPE_SURFACE;
  master_hdr->vlevel_included = FALSE;
  master_hdr->n_fields = 3;
  master_hdr->max_nx = _params->specified_grid.nx;
  master_hdr->max_ny = _params->specified_grid.ny;
  master_hdr->max_nz = 1;
  master_hdr->n_chunks = 0;
  
  STRcopy(master_hdr->data_set_info,
	  "Contingency table intermediate results",
	  MDV_INFO_LEN);
  STRcopy(master_hdr->data_set_name,
	  "Contingency table intermediate results",
	  MDV_NAME_LEN);
  
  MDV_set_master_hdr_offsets(master_hdr);
  
  _interDataset.master_hdr = master_hdr;
  
  /*
   * Initialize the field headers.  There will be 3 fields:  the
   * derived truth flags on the contingency grid and the derived
   * forecast flags on the contingency grid.
   */

  _interDataset.fld_hdrs =
    (MDV_field_header_t **)umalloc(3 * sizeof(MDV_field_header_t *));
  
  _interDataset.field_plane =
    (void ***)umalloc(3 * sizeof(void **));
  
  for (field = 0; field < 3; field++)
  {
    field_hdr = (MDV_field_header_t *)umalloc(sizeof(MDV_field_header_t));
    
    MDV_init_field_header(field_hdr);
    
    field_hdr->nx = _params->specified_grid.nx;
    field_hdr->ny = _params->specified_grid.ny;
    field_hdr->nz = 1;
    if (_params->projection == Params::PROJ_LATLON)
      field_hdr->proj_type = MDV_PROJ_LATLON;
    else
      field_hdr->proj_type = MDV_PROJ_FLAT;

    field_hdr->encoding_type = MDV_INT8;
    field_hdr->data_element_nbytes = 1;
    field_hdr->volume_size = field_hdr->nx * field_hdr->ny;
    
    // field_hdr->proj_origin_lat = _params->specified_grid.miny;
    // field_hdr->proj_origin_lon = _params->specified_grid.minx;

    field_hdr->proj_origin_lat = _params->specified_grid.origin_lat;
    field_hdr->proj_origin_lon = _params->specified_grid.origin_lon;

    field_hdr->grid_dx = _params->specified_grid.dx;
    field_hdr->grid_dy = _params->specified_grid.dy;
    field_hdr->grid_dz = 1.0;
    field_hdr->grid_minx = _params->specified_grid.minx;
    field_hdr->grid_miny = _params->specified_grid.miny;
    field_hdr->grid_minz = 1.0;
    field_hdr->scale = 1.0;
    field_hdr->bias = 0.0;
    field_hdr->bad_data_value = 255.0;
    field_hdr->missing_data_value = 255.0;
    field_hdr->proj_rotation = 0.0;
    
    switch (field){
    case 0 :
      field_hdr->field_data_offset = MDV_get_first_field_offset(master_hdr);
      
      STRcopy(field_hdr->field_name_long,
	      "Truth Contingency Grid",
	      MDV_LONG_FIELD_LEN);
      STRcopy(field_hdr->field_name,
	      "Truth Grid",
	      MDV_SHORT_FIELD_LEN);
      break;

    case 1:
     field_hdr->field_data_offset =
	MDV_get_first_field_offset(master_hdr) + field_hdr->volume_size;
      
      STRcopy(field_hdr->field_name_long,
	      "Forecast Contingency Grid",
	      MDV_LONG_FIELD_LEN);
      STRcopy(field_hdr->field_name,
	      "Forecast Grid",
	      MDV_SHORT_FIELD_LEN);
      break;

    case 2 :
     field_hdr->field_data_offset =
	MDV_get_first_field_offset(master_hdr) + 2*field_hdr->volume_size;
      
      STRcopy(field_hdr->field_name_long,
	      "Contingency",
	      MDV_LONG_FIELD_LEN);
      STRcopy(field_hdr->field_name,
	      "Contingency",
	      MDV_SHORT_FIELD_LEN);
     break;

    default :
      fprintf(stderr, "Error in switch statement.\n");
      exit(-1);

    }


    _interDataset.fld_hdrs[field] = field_hdr;
      
    /*
     * Each field has 1 plane
     */

    _interDataset.field_plane[field] =
      (void **)umalloc(1 * sizeof(void *));
    
    _interDataset.field_plane[field][0] = NULL;
    
  } /* endfor - field */
  
  _interDataset.nfields_alloc = 3;
  CG=NULL;
  
  return;

}  

////////////////////
// _computeStrict()
//

void GridCont::_computeStrict()

{

  float *dd = _forecastGrid->dataArray;
  float *tt = _truthGrid->dataArray;
  float *rr = _forecastGrid->rangeArray;

  int truth, forecast;
  int n_truth = 0;
  int n_forecast = 0;
  int n_success = 0;
  int n_failure = 0;
  int n_false_alarm = 0;
  int n_non_event = 0;

  if (_useInterGrids) {
      CG = (ui08 *)umalloc(sizeof(ui08) * _truthGrid->ny * _truthGrid->nx);
    for (int k=0;k<_truthGrid->ny * _truthGrid->nx; k++) CG[k]=255;
  }

  for (int iy = 0; iy < _truthGrid->ny; iy++) {

    for (int ix = 0; ix < _truthGrid->nx; ix++, dd++, tt++, rr++) {

      if (!_params->check_range || *rr <= _params->max_range) {
 
	if (*tt >= _params->truth_level_lower &&
	    *tt <= _params->truth_level_upper) {
	  truth = TRUE;
	  n_truth++;
	} else {
	  truth = FALSE;
	}

	if (*dd >= _params->forecast_level_lower &&
	    *dd <= _params->forecast_level_upper) {
	  forecast = TRUE;
	  n_forecast++;
	} else {
	  forecast = FALSE;
	}

	if (truth && forecast) {
	  if (_useInterGrids) CG[iy * _truthGrid->nx + ix]=3;
	  n_success++;
	} else if (truth && !forecast) {
	  n_failure++;
	  if (_useInterGrids) CG[iy * _truthGrid->nx + ix]=2;
	} else if (!truth && forecast) {
	  n_false_alarm++;
	  if (_useInterGrids) CG[iy * _truthGrid->nx + ix]=1;
	} else {
	  n_non_event++;
	  if (_useInterGrids) CG[iy * _truthGrid->nx + ix]=0;
	}  /* if (truth ... */

      } /* if (range ... */

    } /* ix */

  } /* iy */

  // add this file's stats to accumulators

  _cont.n_forecast += n_forecast;
  _cont.n_truth += n_truth;
  _cont.n_success += n_success;
  _cont.n_failure += n_failure;
  _cont.n_false_alarm += n_false_alarm;
  _cont.n_non_event += n_non_event;

  if (_params->output_scan_cont) {
    fprintf(_scanContFile, "%d %d %d %d %d %d",
	    n_truth, n_forecast, n_success, n_failure,
	    n_false_alarm, n_non_event);
  }

  if (_useInterGrids) {
    _outputInterGrids(CG);
    ufree(CG);
  }

}

////////////////////
// _computeRelaxed()
//

void GridCont::_computeRelaxed()

{

  // allocate tmp grids for forecast and truth hits

  ui08 **truth = (ui08 **) ucalloc2
    (_truthGrid->ny, _truthGrid->nx, sizeof(ui08));
  
  ui08 **forecast = (ui08 **) ucalloc2
    (_truthGrid->ny, _truthGrid->nx, sizeof(ui08));

  // 2-d arrays have underlying 1-d array which is
  // useful for iterators

  ui08 *pt = *truth;  
  ui08 *pd = *forecast;

  float *dd = _forecastGrid->dataArray;
  float *tt = _truthGrid->dataArray;
  float *rr = _forecastGrid->rangeArray;

   if (_useInterGrids) {
     CG = (ui08 *)umalloc(sizeof(ui08) * _truthGrid->ny * _truthGrid->nx);
     for (int k=0;k<_truthGrid->ny * _truthGrid->nx; k++) CG[k]=255;
   }

  // load up truth and forecast arrays

  for (int iy = 0; iy < _truthGrid->ny; iy++) {

    for (int ix = 0; ix < _truthGrid->nx;
	 ix++, dd++, tt++, rr++, pt++, pd++) {

      if (!_params->check_range || *rr <= _params->max_range) {
 
	if (*tt >= _params->truth_level_lower &&
	    *tt <= _params->truth_level_upper) {
	  *pt = TRUE;
	}

	if (*dd >= _params->forecast_level_lower &&
	    *dd <= _params->forecast_level_upper) {
	  *pd = TRUE;
	}

      } /* if (range ... */

    } /* ix */

  } /* iy */

  int margin = _params->relaxed_margin;

  int startx = margin;
  int endx = _truthGrid->nx - margin - 1;

  int starty = margin;
  int endy = _truthGrid->ny - margin - 1;
  int max_truth = (1 + 2 * margin) * (1 + 2 * margin);
  int truth_thresh;
  if (_params->truth_must_fill_relaxed) {
    truth_thresh = max_truth;
  } else {
    truth_thresh = 1;
  }

  for (int iy = starty; iy <= endy; iy++) {

    ui08 *p_forecast = forecast[iy] + startx;
    
    for (int ix = startx; ix <= endx; ix++, p_forecast++) {
      
      int n_truth = 0;
      
      for (int jy = iy - margin; jy <= iy + margin; jy++) {
	for (int jx = ix - margin; jx <= ix + margin; jx++) {
	  n_truth += truth[jy][jx];
	}
      }

      // Additions by Niles start.

      int n_forecast = 0;
      bool Forecast;

      if (_params->relax_forecast){
	for (int jy = iy - margin; jy <= iy + margin; jy++) {
	  for (int jx = ix - margin; jx <= ix + margin; jx++) {
	    n_forecast += forecast[jy][jx];
	  }
	}
	Forecast = (n_forecast >= truth_thresh);

      } else { // Just work off the single point.
	Forecast = (*p_forecast > 0);
      }

      // Additions by Niles end.

      /* THIS CODE COMMENTED OUT
      if (n_truth > 0) {
	_cont.n_truth++;
      }

      if (*p_forecast > 0) {
	_cont.n_forecast++;
      }
      END OF COMMENTED OUT SECTION. */

      if (n_truth >= truth_thresh) {
	_cont.n_truth++;
      }

      if (Forecast) {
	_cont.n_forecast++;
      }



      /* THIS CODE COMMENTED OUT BY NILES....
      if (n_truth > 0 && *p_forecast) {
	_cont.n_success++;
	if (_useInterGrids) CG[iy * _truthGrid->nx + ix]=3;
	// } else if (n_truth == truth_thresh && !*p_forecast) {

      } else if (n_truth >= truth_thresh && !*p_forecast) {
	_cont.n_failure++;
	if (_useInterGrids) CG[iy * _truthGrid->nx + ix]=2;

      } else if (n_truth  == 0 && *p_forecast) {
	_cont.n_false_alarm++;
	if (_useInterGrids) CG[iy * _truthGrid->nx + ix]=1;

      } else {
	_cont.n_non_event++;
	if (_useInterGrids) CG[iy * _truthGrid->nx + ix]=0;
      } END OF COMMENTED OUT SECTION...  */ /* if (truth ... */




      if ((n_truth >= truth_thresh) && (Forecast)) {
	_cont.n_success++;
	if (_useInterGrids) CG[iy * _truthGrid->nx + ix]=3;

      } else if (n_truth >= truth_thresh && !(Forecast)) {
	_cont.n_failure++;
	if (_useInterGrids) CG[iy * _truthGrid->nx + ix]=2;

      } else if (!(n_truth  >= truth_thresh) && (Forecast)) {
	_cont.n_false_alarm++;
	if (_useInterGrids) CG[iy * _truthGrid->nx + ix]=1;

      } else {
	_cont.n_non_event++;
	if (_useInterGrids) CG[iy * _truthGrid->nx + ix]=0;
      }  /* if (truth ... */
      
    } /* ix */
  
  } /* iy */

  // free up local arrays

  ufree2((void **) truth);
  ufree2((void **) forecast);

  if (_useInterGrids) {
    _outputInterGrids(CG);
    ufree(CG);
  }

}

////////////////////
// _outputInterGrids
//

void GridCont::_outputInterGrids(ui08 *CG)

{

  MDV_master_header_t *master_hdr = _interDataset.master_hdr;

  MDV_field_header_t *TFhdr =  _interDataset.fld_hdrs[0];
  MDV_field_header_t *FFhdr =  _interDataset.fld_hdrs[1];
  MDV_field_header_t *CFhdr =  _interDataset.fld_hdrs[2];

  // Finish filling out the geometry of the contingency field.

  master_hdr->max_nx = _truthGrid->nx;
  master_hdr->max_ny = _truthGrid->ny;
  CFhdr->nx = _truthGrid->nx;
  CFhdr->ny = _truthGrid->ny;
  
  CFhdr->proj_origin_lat = _truthGrid->originLat;
  CFhdr->proj_origin_lon = _truthGrid->originLon;
  CFhdr->grid_dx = _truthGrid->dx;
  CFhdr->grid_dy = _truthGrid->dy;
  CFhdr->grid_minx = _truthGrid->minx;
  CFhdr->grid_miny = _truthGrid->miny;       


  UTIMstruct time_struct;
  char output_directory[MAX_PATH_LEN];
  char output_filename[MAX_PATH_LEN];
  
  // Fill in master header information.

  master_hdr->time_gen = time(NULL);
  master_hdr->time_begin = _truthGrid->timeStart;
  master_hdr->time_centroid = _truthGrid->timeCent;
  master_hdr->time_end = _truthGrid->timeEnd;
  master_hdr->time_expire = master_hdr->time_end;

  // And some of the field headers.
 
  TFhdr->scale = _truthGrid->Scale;
  TFhdr->bias = _truthGrid->Bias;
  TFhdr->bad_data_value = _truthGrid->Bad;
  TFhdr->missing_data_value = _truthGrid->Missing;
 
  FFhdr->scale = _forecastGrid->Scale;
  FFhdr->bias = _forecastGrid->Bias;
  FFhdr->bad_data_value = _forecastGrid->Bad;
  FFhdr->missing_data_value = _forecastGrid->Missing;

  // Set the data pointers.

  _interDataset.field_plane[0][0] = _truthGrid->byteArray;
  _interDataset.field_plane[1][0] = _forecastGrid->byteArray;

  _interDataset.field_plane[2][0] = CG;

    
  // Create the output file.
  
  UTIMunix_to_date(master_hdr->time_centroid, &time_struct);
    
  sprintf(output_directory, "%s/%04ld%02ld%02ld",
	  _params->intermediate_dir,
	  time_struct.year, time_struct.month, time_struct.day);

  sprintf(output_filename, "%02ld%02ld%02ld.mdv",
	  time_struct.hour, time_struct.min, time_struct.sec);
    
  if (MDV_write_dataset_remote(&_interDataset, MDV_PLANE_RLE8, FALSE,
			       (char *) "local",
			       output_directory,
			       output_filename,
			       (char *) "./tmp") != MDV_SUCCESS) {
    fprintf(stderr, "ERROR - %s:_outputInterGrids\n", _progName);
    fprintf(stderr, "Error writing intermediate grids to MDV file <%s/%s>\n",
	    output_directory, output_filename);
  }
    


}
  









