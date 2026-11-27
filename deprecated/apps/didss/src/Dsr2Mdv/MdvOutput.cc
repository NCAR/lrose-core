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
// MdvOutput.cc
//
// Handles output to MDV files
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1998
//
///////////////////////////////////////////////////////////////

#include "MdvOutput.hh"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <Mdv/mdv/mdv_write.h>
#include <Mdv/mdv/mdv_utils.h>
#include <Mdv/mdv/mdv_user.h>
#include <dsserver/DsLdataInfo.hh>
using namespace std;

//////////////
// Constructor

MdvOutput::MdvOutput(char *prog_name,
		     Dsr2Mdv_tdrp_struct *params,
		     Lookup *lookup)
  
{

  _progName = STRdup(prog_name);
  _params = params;
  _lookup = lookup;
  MDV_init_handle(&_handle);
  DsRadarElev_init(&_elevs);
  OK = TRUE;

}

/////////////
// destructor

MdvOutput::~MdvOutput()

{
  
  STRfree(_progName);
  MDV_free_handle(&_handle);

}

//////////////////
// setVolHdrs()
//
// Initialize the volume headers.
//
// Returns 0 on success, -1 on failure.
//

void MdvOutput::setVolHdrs(DsRadarMsg &radarMsg,
			   MDV_radar_grid_t *grid,
			   int n_fields,
			   int *field_pos,
			   int missing_data_val,
			   radar_scan_table_t *scan_table)
  
{

  DsRadarParams radarParams = radarMsg.getRadarParams();

  // clear the master header
  
  MDV_master_header_t *mhdr = &_handle.master_hdr;
  MDV_init_master_header(mhdr);
  P2mdv_lookup_params_t *lookup_params = _lookup->handle.lookup_params;
  
  // fill the master header

  mhdr->num_data_times = 1;
  mhdr->data_dimension = 3;
  mhdr->data_collection_type = MDV_DATA_MEASURED;
  if (lookup_params->geom == P2MDV_CART) {
    mhdr->native_vlevel_type = MDV_VERT_TYPE_ELEV;
    mhdr->vlevel_type = MDV_VERT_TYPE_Z;
  } else if (lookup_params->geom == P2MDV_PPI) {
    mhdr->native_vlevel_type = MDV_VERT_TYPE_ELEV;
    mhdr->vlevel_type = MDV_VERT_TYPE_ELEV;
  } else if (lookup_params->geom == P2MDV_POLAR) {
    mhdr->native_vlevel_type = MDV_VERT_TYPE_ELEV;
    mhdr->vlevel_type = MDV_VERT_TYPE_ELEV;
  }
  mhdr->vlevel_included = TRUE;
  mhdr->grid_order_direction = MDV_ORIENT_SN_WE;
  mhdr->grid_order_indices = MDV_ORDER_XYZ;
  mhdr->n_fields = n_fields;
  mhdr->max_nx = grid->nx;
  mhdr->max_ny = grid->ny;
  mhdr->max_nz = grid->nz;
  mhdr->n_chunks = 2;
  mhdr->field_grids_differ = FALSE;

  mhdr->sensor_lon = radarParams.longitude;
  mhdr->sensor_lat = radarParams.latitude;
  mhdr->sensor_alt = radarParams.altitude;

  // info and names

  STRncopy(mhdr->data_set_info, _params->radar_info, MDV_INFO_LEN);
  STRncopy(mhdr->data_set_name, radarParams.radarName.c_str(), MDV_NAME_LEN);
  sprintf(mhdr->data_set_source,
	  "DsRadar data stream for radar id %d\n"
	  "Scan type %s\n",
	  radarParams.radarId,
	  radarParams.scanTypeName.c_str());
  
  // allocate MDV arrays

  MDV_alloc_handle_arrays(&_handle, mhdr->n_fields,
			  mhdr->max_nz, mhdr->n_chunks);

  // fill in field headers and vlevel headers

  for (int ifield = 0; ifield < n_fields; ifield++) {

    const DsFieldParams *fparams = radarMsg.getFieldParams(field_pos[ifield]);
    MDV_field_header_t *fhdr = _handle.fld_hdrs + ifield;
    MDV_init_field_header(fhdr);

    fhdr->nx = grid->nx;
    fhdr->ny = grid->ny;
    fhdr->nz = grid->nz;

    if (lookup_params->geom == P2MDV_CART) {
      fhdr->proj_type = MDV_PROJ_FLAT;
    } else if (lookup_params->geom == P2MDV_PPI) {
      fhdr->proj_type = MDV_PROJ_FLAT;
    } else if (lookup_params->geom == P2MDV_POLAR) {
      fhdr->proj_type = MDV_PROJ_POLAR_RADAR;
    }
    fhdr->encoding_type = MDV_INT8;
    fhdr->data_element_nbytes = 1;
    fhdr->field_data_offset = 0;
    fhdr->volume_size = fhdr->nx * fhdr->ny * fhdr->nz * sizeof(ui08);

    fhdr->proj_origin_lat = grid->latitude;
    fhdr->proj_origin_lon = grid->longitude;

    fhdr->grid_dx = grid->dx;
    fhdr->grid_dy = grid->dy;
    fhdr->grid_dz = grid->dz;

    fhdr->grid_minx = grid->minx;
    fhdr->grid_miny = grid->miny;
    fhdr->grid_minz = grid->minz;

    fhdr->scale = fparams->scale;
    fhdr->bias = fparams->bias;

    fhdr->bad_data_value = missing_data_val;
    fhdr->missing_data_value = missing_data_val;
    fhdr->proj_rotation = grid->rotation;

    if (!strcmp(fparams->name.c_str(), "DBZ")) {
      fhdr->field_code = MDV_get_field_code_from_abbrev("DBZ");
      STRncopy(fhdr->units, "dBZ", MDV_TRANSFORM_LEN);
    } else if (!strcmp(fparams->name.c_str(), "VEL")) {
      fhdr->field_code = MDV_get_field_code_from_abbrev("R VEL");
      STRncopy(fhdr->units, "none", MDV_TRANSFORM_LEN);
    } else if (!strcmp(fparams->name.c_str(), "SNR")) {
      fhdr->field_code = MDV_get_field_code_from_abbrev("S/N");
      STRncopy(fhdr->units, "dB", MDV_TRANSFORM_LEN);
    } else if (!strcmp(fparams->name.c_str(), "SPW")) {
      fhdr->field_code = MDV_get_field_code_from_abbrev("S WIDTH");
      STRncopy(fhdr->units, "none", MDV_TRANSFORM_LEN);
    } else {
      fhdr->field_code = -1;
    }

    STRncopy(fhdr->field_name_long,
	     MDV_get_field_name(fhdr->field_code),
	     MDV_LONG_FIELD_LEN);

    STRncopy(fhdr->field_name,
	     fparams->name.c_str(), MDV_SHORT_FIELD_LEN);

    STRncopy(fhdr->units,
	     fparams->units.c_str(), MDV_UNITS_LEN);

    MDV_vlevel_header_t *vhdr = _handle.vlv_hdrs + ifield;
    MDV_init_vlevel_header(vhdr);
    for (int iz = 0; iz < fhdr->nz; iz++) {
      if (lookup_params->geom == P2MDV_CART) {
	vhdr->vlevel_type[iz] = MDV_VERT_TYPE_Z;
	vhdr->vlevel_params[iz] = fhdr->grid_minz + iz * fhdr->grid_dz;
      } else if (lookup_params->geom == P2MDV_PPI) {
	vhdr->vlevel_type[iz] = MDV_VERT_TYPE_ELEV;
	vhdr->vlevel_params[iz] = scan_table->elev_angles[iz];
      } else if (lookup_params->geom == P2MDV_POLAR) {
	vhdr->vlevel_type[iz] = MDV_VERT_TYPE_ELEV;
	vhdr->vlevel_params[iz] = scan_table->elev_angles[iz];
      }
    }
    
  } // ifield

  // radar params chunk
  
  radarParams.loadStruct(&_radarParamsStruct);

  MDV_chunk_header_t *chdr = _handle.chunk_hdrs;
  chdr->record_len1 = sizeof(MDV_chunk_header_t) - (2 * sizeof(si32));
  chdr->struct_id = MDV_CHUNK_HEAD_MAGIC_COOKIE;
  chdr->chunk_id = MDV_CHUNK_DSRADAR_PARAMS;
  chdr->chunk_data_offset = 0;
  chdr->size = sizeof(DsRadarParams_t);
  STRncopy((char *)chdr->info, "DsRadarParams_t struct",
	   MDV_CHUNK_INFO_LEN);
  chdr->record_len2 = chdr->record_len1;
  _handle.chunk_data[0] = &_radarParamsStruct;

  // radar elevations struct

  DsRadarElev_alloc(&_elevs, scan_table->nelevations);
  for (int i = 0; i < scan_table->nelevations; i++) {
    _elevs.elev_array[i] = scan_table->elev_angles[i];
  }
  DsRadarElev_load_chunk(&_elevs);

  chdr++;
  chdr->record_len1 = sizeof(MDV_chunk_header_t) - (2 * sizeof(si32));
  chdr->struct_id = MDV_CHUNK_HEAD_MAGIC_COOKIE;
  chdr->chunk_id = MDV_CHUNK_DSRADAR_ELEVATIONS;
  chdr->chunk_data_offset = 0;
  chdr->size = _elevs.chunk_len;
  STRncopy((char *)chdr->info, "DsRadar Elevation angles",
	   MDV_CHUNK_INFO_LEN);
  chdr->record_len2 = chdr->record_len1;
  _handle.chunk_data[1] = _elevs.chunk_buf;

  // set chunk_data_allocated to indicate that the library routines
  // must free the memory allocated to the chunk_data

  // _handle.chunk_data_allocated = TRUE;

}

///////////////////////////////////////////////
// writeCompleteVol()
//
// Write out completed volume in MDV format.
//

int MdvOutput::writeCompleteVol(time_t start_time,
				time_t end_time,
                                time_t reference_time,
				ui08 **vol_data,
				int npoints_plane)

{

  if (_params->debug) {
    cerr << "Start MdvOutput::writeCompleteVol()" << endl;
  }

  // check the volume duration

  time_t vol_duration = abs(end_time - start_time);
  if (vol_duration > _params->max_vol_duration) {
    fprintf(stderr, "ERROR - %s:MdvOutput::writeCompleteVol\n", _progName);
    fprintf(stderr, "Vol duration: %d secs\n", (int) vol_duration);
    fprintf(stderr, "This exceeds max allowable of %d secs\n",
	    (int) _params->max_vol_duration);
    return (-1);
  }

  MDV_master_header_t *mhdr = &_handle.master_hdr;

  mhdr->time_gen = time(NULL);

  // in some data sets, for example TASS, the order of the beams
  // is not necessarily in time sequence. Therefore, if the start
  // and end times are out of order, switch them 

  if (start_time <= end_time) {
    mhdr->time_begin = start_time;
    mhdr->time_end = end_time;
  } else {
    mhdr->time_begin = end_time;
    mhdr->time_end = start_time;
  }

  time_t centroid;
  if (_params->auto_mid_time) {
    centroid = start_time + (end_time - start_time) / 2;
  } else {
    centroid = end_time - _params->age_at_end_of_volume;
  }
  mhdr->time_centroid = centroid;
  mhdr->time_expire = mhdr->time_end;

  // set the reference time - use the user time slot

  mhdr->user_time = reference_time;
  
  // set field plane pointers into data arrays

  for (int ifield = 0; ifield < mhdr->n_fields; ifield++) {
    MDV_field_header_t *fhdr = _handle.fld_hdrs + ifield;
    for (int iz = 0; iz < fhdr->nz; iz++) {
      _handle.field_plane[ifield][iz] =
	vol_data[ifield] + iz * npoints_plane;
    } // iz
  } // ifields

  if (_params->debug >= DEBUG_NORM) {
    fprintf(stderr, "Writing MDV file, time %s, to dir %s\n",
	    utimstr(mhdr->time_centroid), _params->output_mdv_dir);
  }

  // write to directory
  
  int iret = MDV_write_to_dir(&_handle, _params->output_mdv_dir,
			      MDV_PLANE_RLE8, FALSE);

  //
  // Write out an index file
  //
  DsLdataInfo  ldata(_params->output_mdv_dir);
  ldata.setDataFileExt("mdv");
  if (ldata.write(centroid)) {
    fprintf(stderr, "MdvOutput::writeCompleteVol\n");
    fprintf(stderr, "DsLdata write failed\n");
    iret = -1;
  }
  
  // reset the field planes to NULL so they area not freed
  // by MDV routines
  
  for (int ifield = 0; ifield < mhdr->n_fields; ifield++) {
    MDV_field_header_t *fhdr = _handle.fld_hdrs + ifield;
    for (int iz = 0; iz < fhdr->nz; iz++) {
      _handle.field_plane[ifield][iz] = NULL;
    } // iz
  } // ifields

  if (_params->debug) {
    cerr << "End MdvOutput::writeCompleteVol()" << endl;
  }

  if (iret == MDV_SUCCESS) {
    return (0);
  } else {
    return (-1);
  }

}

///////////////////////////////////////////////
// writeIntermediateVol()
//
// Write out intermediate volume in MDV format.
//

int MdvOutput::writeIntermediateVol(time_t latest_time,
				    int vol_duration,
				    ui08 **vol_data,
				    int npoints_plane)

{
  
  MDV_master_header_t *mhdr = &_handle.master_hdr;

  mhdr->time_gen = time(NULL);
  mhdr->time_begin = latest_time - vol_duration;
  mhdr->time_end = latest_time;
  mhdr->time_centroid = latest_time;
  mhdr->time_expire = mhdr->time_end + vol_duration;
  
  // set field plane pointers into data arrays

  for (int ifield = 0; ifield < mhdr->n_fields; ifield++) {
    MDV_field_header_t *fhdr = _handle.fld_hdrs + ifield;
    for (int iz = 0; iz < fhdr->nz; iz++) {
      _handle.field_plane[ifield][iz] =
	vol_data[ifield] + iz * npoints_plane;
    } // iz
  } // ifields

  // write to latest data path
  
  char outputPath[MAX_PATH_LEN];
  sprintf(outputPath, "%s%s%s", _params->output_mdv_dir,
	  PATH_DELIM, "latest.mdv");
  
  if (_params->debug >= DEBUG_NORM) {
    fprintf(stderr, "Writing MDV file, time %s, to path %s\n",
	    utimstr(mhdr->time_centroid), outputPath);
  }
  
  int iret = MDV_handle_write_all(&_handle,
				  outputPath,
				  MDV_INT8,
				  MDV_COMPRESSION_ZLIB,
				  MDV_SCALING_ROUNDED, 1.0, 0.0);

  // reset the field planes to NULL so they area not freed
  // by MDV routines
  
  for (int ifield = 0; ifield < mhdr->n_fields; ifield++) {
    MDV_field_header_t *fhdr = _handle.fld_hdrs + ifield;
    for (int iz = 0; iz < fhdr->nz; iz++) {
      _handle.field_plane[ifield][iz] = NULL;
    } // iz
  } // ifields

  if (iret == MDV_SUCCESS) {
    return (0);
  } else {
    return (-1);
  }

}


