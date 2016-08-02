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
/*********************************************************************
 * operate.c
 *
 * Subroutines for MDV_server
 * 
 * Frank Hage, RAP, NCAR, Boulder, CO, USA
 *
 * Updated by Mike Dixon, Dec 1991
 */

#include "MDV_server.hh"
#include <dataport/bigend.h>
#include <toolsa/str.h>
#include <toolsa/pjg.h>
#include <rapmath/math_macros.h>
using namespace std;

static void copy_field_params_to_info(MdvRead &mdv,
				      cdata_comm_t *com,
				      cdata_info_t *info,
				      si32 field);

static void copy_grid_params_to_info(MdvRead &mdv,
				     cdata_comm_t *com,
				     cdata_info_t *info);

static void copy_vol_params_to_info(MdvRead &mdv,
				    cdata_comm_t *com,
				    cdata_info_t *info);

static void initialize_reply(cdata_reply_t *reply,
			     cdata_comm_t *com);

static void process_request(cdata_comm_t *com,
			    int sockfd,
			    MdvRead &mdv);

static void send_reply(cdata_comm_t *com,
		       cdata_reply_t *reply,
		       cdata_info_t *info,
		       ui08 *ptr,
		       int sockfd,
		       MdvRead &mdv);

#define LISTEN_MSECS 1000 /* the time in secs between checking
			   * for registration */

void operate(void)

{
  
  int forever = TRUE;
  int protofd, sockfd;
  MdvRead mdv;
  cdata_comm_t com;
  
  /*
   * initialize socket connections 
   */

  if((protofd = SKU_open_server(Glob->params.port)) < 0) {
    fprintf(stderr,"ERROR - %s:operate\n", Glob->prog_name);
    fprintf(stderr,"Couldn't open socket for listening\n");
    perror("");
    tidy_and_exit(-1);
  }
  
  while (forever) {
    
    /*
     * get next client - will timeout after set period
     */
    
    while ((sockfd =
	    SKU_get_client_timed(protofd, LISTEN_MSECS)) < 0) {
      
      /*
       * register process
       */
      
      PMU_auto_register("Timed out in operate while loop");
      
      /*
       * register server with the server mapper
       */
      
      SMU_auto_register();
      
    }
    
    PMU_force_register("Reading request from socket");
    
    /*
     * Read in comms
     */
    
    if(SKU_read((int) sockfd,
		(char *) &com,
		(si32) sizeof(com), -1) != sizeof(com)) {
      
      fprintf(stderr,"Problems reading Message\n");
      
    } else {
      
      BE_to_array_32((ui32 *) &com,
		     (ui32) sizeof(cdata_comm_t));
      
      process_request(&com, sockfd, mdv);
      
    }
    
    close(sockfd);
    
  } /* while (forever) */
  
}

/*****************************************************************
 * PROCESS_REQUEST: Take an incoming request and satisfy it 
 */

static void process_request(cdata_comm_t *com,
			    int sockfd,
			    MdvRead &mdv)
     
{
  
  ui08 *data_ptr = NULL; /* pointer to data */
  
  cdata_info_t info;
  cdata_reply_t reply;

  Glob->time_last_request = time((time_t *) NULL);
  Glob->n_data_requests++;

  if(Glob->params.debug) {
    fprintf(stderr, "Received request\n");
  }
  
  PMU_auto_register("Processing request");
  
  /*
   * if plane_heights are requested, request info as well
   */
  
  if (com->primary_com & GET_PLANE_HEIGHTS) {
    com->primary_com |= GET_INFO;
  }
  
  /*
   * Set default values in reply
   */
  
  initialize_reply(&reply, com); 

  /*
   * In the case of a request for new data when no realtime file
   * is available, change the request to GET_MOST_RECENT
   */

  if (com->primary_com & GET_NEW && !Glob->params.use_realtime_file) {
    com->primary_com &= ~GET_NEW;
    com->primary_com |= GET_MOST_RECENT;
  }

  if(Glob->params.debug) {
    fprintf(stderr, "Request Time_cent = %s\n",
	    utimstr(com->time_cent));
  }
  
  /*
   * read in the data volume
   */
  
  PMU_auto_register("Reading in data volume");
  
  if (read_mdv(mdv, com)) {
    
    /*
     * read failed
     */
    
    reply.status |= NO_INFO | NO_DATA_AVAILABLE;
    
  } else if (com->data_field >= mdv.getMasterHeader().n_fields) {

    /*
     * field number is out of range
     */
    
    if (Glob->params.debug) {
      fprintf(stderr, "Field num %d exceeds max of %d\n",
	      (int) com->data_field,
	      (int) mdv.getMasterHeader().n_fields);
    }
    
    reply.status |= NO_INFO | NO_DATA_AVAILABLE;

  } else {

    if (com->primary_com & GET_NEW) {
      reply.status |= NEW_DATA;
    }
    
    copy_vol_params_to_info(mdv, com, &info);
    
    copy_grid_params_to_info(mdv, com, &info);
    
    copy_field_params_to_info(mdv, com, &info,
			      com->data_field);
    
    /*
     * get the data if requested
     */
    
    if(com->primary_com & GET_DATA) {
      data_ptr = (ui08 *) NULL;
      if(Glob->params.debug) {
	fprintf(stderr, "getting data\n");
      }
      data_ptr = get_grid_data(com, &reply, &info, mdv);
    }
    
  } /* if (read_mdv(mdv, com)) */

  PMU_auto_register("Sending reply");

  send_reply(com, &reply, &info, data_ptr, sockfd, mdv);
  
  if(data_ptr != NULL) {
    ufree((char *) data_ptr);
    if (Glob->params.free_volume) {
      mdv.clear();
    }
  }
  
}

/*****************************************************************
 * INITIALIZE_REPLY: Set up default values in to the reply
 */

static void initialize_reply(cdata_reply_t *reply,
			     cdata_comm_t *com)
     
{
  
  reply->divisor = (int) CDATA_DIVISOR;
  reply->status = 0;
  reply->data_length = 0;
  reply->data_field = com->data_field;
  
  /*
   * These are set by get_data routines
   */
  
  reply->orient = 0;
  reply->nx = 0;
  reply->ny = 0;
  reply->nz = 0;
  reply->dz = 0;
  reply->x1 = 0;
  reply->x2 = 0;
  reply->y1 = 0;
  reply->y2 = 0;
  reply->z1 = 0;
  reply->z2 = 0;
  reply->scale = 0;
  reply->bias = 0;
  reply->bad_data_val = 0;
  reply->data_type = CDATA_CHAR;
  reply->n_points = 0;
  
}

/*****************************************************************
 * SEND_REPLY: Send a reply message to the client
 */

/*ARGSUSED*/

static void send_reply(cdata_comm_t *com,
		       cdata_reply_t *reply,
		       cdata_info_t *info,
		       ui08 *ptr,
		       int sockfd,
		       MdvRead &mdv)
     
{
  
  si32 **int_hts;
  si32 nheights;
  si32 nbytes_plane_heights;
  si32 iz;
  int nz = info->nz;
  int half_dz;

  double ht;
  
  cdata_reply_t tmp_reply;
  cdata_info_t tmp_info;
  
  reply->data_length = 0;

  if (!(reply->status & NO_INFO)) {
    nheights = nz * 3; /* lower, mid, upper */
  }
  
  /*
   * if info was requested and is available,
   * set reply status accordingly
   */
  
  if((com->primary_com & GET_INFO) &&
     !(reply->status & NO_INFO)) {
    
    reply->status |= REQUEST_SATISFIED;
    reply->status |= INFO_FOLLOWS;
    reply->data_length += sizeof(*info);
    
  }
  
  /*
   * if plane_heights were requested and info is available,
   * set reply status accordingly
   */
  
  if((com->primary_com & GET_PLANE_HEIGHTS) &&
     !(reply->status & NO_INFO)) {
    
    reply->status |= REQUEST_SATISFIED;
    reply->status |= PLANE_HEIGHTS_FOLLOW;
    reply->data_length += (nheights * sizeof(si32));
    
  }
  
  /*
   * if data was requested and is available,
   * set reply status accordingly
   */
  
  if((com->primary_com & GET_DATA) &&
     !(reply->status & NO_DATA_AVAILABLE)) {
    
    reply->status |= REQUEST_SATISFIED;
    reply->status |= DATA_FOLLOWS;
    reply->data_length += reply->n_points;
    
  }
  
  /*
   * Copy reply to temp struct and code into network byte order
   */
  
  memcpy (&tmp_reply, reply, sizeof(cdata_reply_t));
  
  BE_from_array_32((ui32 *) &tmp_reply,
		   (ui32) (sizeof(cdata_reply_t)));
  
  /*
   * Send the reply
   */
  
  if(SKU_write((int) sockfd,
	       (char *) &tmp_reply,
	       (si32) sizeof(cdata_reply_t),
	       -1) != sizeof(cdata_reply_t)) {
    fprintf(stderr,"Problems writing reply to socket.\n");
  }
  
  if(Glob->params.debug) {
    fprintf(stderr, "sending reply\n");
  }
  
  /*
   * Send info with reply if required
   */
  
  if(reply->status & INFO_FOLLOWS) {
    
    info->data_length = reply->n_points;
    
    memcpy (&tmp_info, info, sizeof(cdata_info_t));
    
    BE_from_array_32((ui32 *) &tmp_info,
		     (ui32) (NUM_INFO_LONGS * sizeof(si32)));
    
    if(SKU_write((int) sockfd, (char *) &tmp_info,
		 (si32) sizeof(*info),
		 -1) != sizeof(cdata_info_t)) {
      fprintf(stderr,"Problems writing info to socket\n");
    }
    
    if(Glob->params.debug) {
      fprintf(stderr, "sending info\n");
    }
    
  }
  
  /*
   * Send plane heights with reply if required. Plane heights
   * is a 1-D array of length 3 * nhts. For each plane there
   * is a lower, mid and upper point.
   */
  
  if(reply->status & PLANE_HEIGHTS_FOLLOW) {
    
    int_hts = (si32 **) umalloc2 (nz, 3, sizeof(si32));
    
    /*
     * load up mid plane hts
     */

    MdvReadField &field = mdv.getField(com->data_field);
    if (mdv.getMasterHeader().vlevel_included) {
      for (iz = 0; iz < nz; iz++) {
	ht = field.getVlevelHeader().vlevel_params[iz];
	int_hts[iz][1] = (si32) floor (ht * info->divisor + 0.5);
      } 
    } else {
      for (iz = 0; iz < nz; iz++) {
	ht = field.getGrid().minz + iz * field.getGrid().dz;
	int_hts[iz][1] = (si32) floor (ht * info->divisor + 0.5);
      }
    }

    /*
     * load up lower hts
     */

    for (iz = 1; iz < nz; iz++) {
      int_hts[iz][0] = (int_hts[iz-1][1] + int_hts[iz][1]) / 2;
    }
    if (nz > 1) {
      half_dz = int_hts[1][0] - int_hts[0][1];
      int_hts[0][0] = int_hts[0][1] - half_dz;
    } else {
      int_hts[0][0] = int_hts[0][1] - info->dz / 2;
    }

    /*
     * load up upper hts
     */

    for (iz = 0; iz < nz-1; iz++) {
      int_hts[iz][2] = (int_hts[iz][1] + int_hts[iz+1][1]) / 2;
    }
    if (nz > 1) {
      half_dz = int_hts[nz-1][1] - int_hts[nz-1][0];
      int_hts[nz-1][2] = int_hts[nz-1][1] + half_dz;
    } else {
      int_hts[nz-1][2] = int_hts[nz-1][1] + info->dz / 2;
    }

    nbytes_plane_heights = nheights * sizeof(si32);
    BE_from_array_32(*int_hts, nbytes_plane_heights);
    
    if(SKU_write(sockfd, *int_hts,
		 nbytes_plane_heights, -1) != nbytes_plane_heights) {
      fprintf(stderr,"Problems writing plane heights to socket\n");
    }
    
    if(Glob->params.debug) {
      fprintf(stderr, "sending plane heights\n");
    }
    
    ufree2((void **) int_hts);
    
  }
  
  /*
   * Send data if requested
   */

  umalloc_verify();

  if(reply->status & DATA_FOLLOWS) { 
    
    if(SKU_write(sockfd, (char *) ptr,
		 reply->n_points,
		 -1) != reply->n_points) {
      fprintf(stderr,"Problems writing message\n");
    }
    
    if(Glob->params.debug) {
      fprintf(stderr, "sending data\n");
    }
    
  }
  
}

/*****************************************************************
 * COPY_VOL_PARAMS_TO_INFO: Copy volume parameters to the
 * info structure 
 */

static void copy_vol_params_to_info(MdvRead &mdv,
				    cdata_comm_t *com,
				    cdata_info_t *info)

{
  
  si32 nelev;
  double start_range, gate_spacing;
  DsRadarParams_t *rparams = &(mdv.getRadar().getRadarParams());
  
  info->divisor = (int) CDATA_DIVISOR;
  if (mdv.getField(com->data_field).getFieldHeader().proj_type ==
      MDV_PROJ_LATLON) {
    info->highres_divisor = CDATA_HIGHRES_DIVISOR;
  } else {
    info->highres_divisor = 1;
  }
  info->order = DATA_ORDER;
  
  info->wavelength /* um */ = (int) (rparams->wavelength * 10000.0 + 0.5);
  
  info->frequency /* Khz */ = (int) floor
    ((3.0e7 / rparams->wavelength) + 0.5);
  
  gate_spacing = rparams->gate_spacing;
  start_range = rparams->start_range;
  
  info->gate_spacing /* km */ =
    (int) floor (gate_spacing * info->divisor + 0.5);
  info->min_range /* km */ =
    (int) floor (start_range * info->divisor + 0.5);
  
  info->max_range /* km */ = (int) floor
    ((start_range + gate_spacing * rparams->ngates) *
     info->divisor + 0.5);
  
  info->num_gates = rparams->ngates;
  info->min_gates = rparams->ngates;
  info->max_gates = rparams->ngates;

  DsRadarElev_t &radarElevs = mdv.getRadar().getRadarElevs();
  nelev = radarElevs.n_elev;
  info->num_tilts = nelev;
  
  if (nelev > 0) {
    info->min_elev = (int) floor
      (radarElevs.elev_array[0] * info->divisor + 0.5);
    info->max_elev = (int) floor
      (radarElevs.elev_array[nelev-1] * info->divisor + 0.5);
  } else {
    info->min_elev = 0;
    info->max_elev = 0;
  }
  
  info->radar_const = (int) floor(rparams->radar_constant *
			    info->divisor + 0.5);
  
  info->delta_azmith = info->divisor;
  info->start_azmith = 0;
  info->beam_width = (int) floor
    (rparams->horiz_beam_width * info->divisor + 0.5);
  info->pulse_width = (int) (rparams->pulse_width * info->divisor + 0.5);
  info->noise_thresh = (int) NOISE_THRESH;
  info->nfields = mdv.getMasterHeader().n_fields;

  mdv_grid_t &grid = mdv.getField(com->data_field).getGrid();
  STRncopy(info->units_label_x, grid.unitsx, LAB_LEN);
  STRncopy(info->units_label_y, grid.unitsy, LAB_LEN);
  STRncopy(info->units_label_z, grid.unitsz, LAB_LEN);
  
  STRncopy(info->source_name, rparams->radar_name, LAB_LEN);
  
}

/*****************************************************************
 * COPY_GRID_PARAMS_TO_INFO: Copy volume parameters to the
 * info structure 
 */

static void copy_grid_params_to_info(MdvRead &mdv,
				     cdata_comm_t *com,
				     cdata_info_t *info)
     
{
  
  si32 nz;
  double minx, miny, minz;
  double maxx, maxy, maxz;
  double dx, dy, dz;
  double com_divisor;
  double com_lat, com_lon;
  double data_lat, data_lon;
  double range, theta;
  
  if (mdv.getField(com->data_field).getFieldHeader().proj_type ==
      MDV_PROJ_LATLON) {
    info->projection = PJG_LATLON;
  } else {
    info->projection = PJG_FLAT;
  }

  mdv_grid_t &grid = mdv.getField(com->data_field).getGrid();

  if (mdv.getField(com->data_field).getFieldHeader().proj_type ==
      MDV_PROJ_LATLON) {

    if (Glob->params.lat_lon_override) {
      
      Glob->data_to_com_delta_x =
	grid.proj_origin_lon - Glob->params.lon;

      Glob->data_to_com_delta_y =
	grid.proj_origin_lat - Glob->params.lat;
      
    } else {
      
      Glob->data_to_com_delta_x = 0.0;
      Glob->data_to_com_delta_y = 0.0;
      
    }
    

  } else {

    /*
     * get the (x, y) difference between the server grid and the 
     * reference point passed in
     */
    
    com_divisor = (double) com->divisor;
    com_lat = (double) com->lat_origin / com_divisor;
    com_lon = (double) com->lon_origin / com_divisor;
    
    if (Glob->params.lat_lon_override) {
      data_lat = Glob->params.lat;
      data_lon = Glob->params.lon;
    } else {
      data_lat = grid.proj_origin_lat;
      data_lon = grid.proj_origin_lon;
    }
    
    PJGLatLon2RTheta(data_lat, data_lon,
		     com_lat, com_lon,
		     &range, &theta);
    
    Glob->data_to_com_delta_x = range * sin (theta * DEG_TO_RAD);
    Glob->data_to_com_delta_y = range * cos (theta * DEG_TO_RAD);
    
  }

  if (Glob->params.lat_lon_override) {
    info->lat_origin = (int) floor
      (Glob->params.lat * info->divisor + 0.5);
    info->lon_origin = (int) floor
      (Glob->params.lon * info->divisor + 0.5);
  } else {
    info->lat_origin = (int) floor
      (grid.proj_origin_lat * info->divisor + 0.5);
    info->lon_origin = (int) floor
      (grid.proj_origin_lon * info->divisor + 0.5);
  }
  info->ht_origin = 0;
  
  info->source_x = (int) floor (grid.sensor_x * info->divisor + 0.5);
  info->source_y = (int) floor (grid.sensor_y * info->divisor + 0.5);
  info->source_z = (int) floor (grid.sensor_z * info->divisor + 0.5);
  
  info->nx = grid.nx;
  info->ny = grid.ny;
  info->nz = grid.nz;
  
  dx = grid.dx;
  minx = grid.minx - dx / 2.0 - Glob->data_to_com_delta_x;
  maxx = minx + grid.nx * dx;
  
  if (mdv.getField(com->data_field).getFieldHeader().proj_type ==
      MDV_PROJ_LATLON) {
    info->dx = (int) floor (dx * info->highres_divisor + 0.5);
    info->min_x = (int) floor (minx * info->highres_divisor + 0.5);
    info->max_x = (int) floor (maxx * info->highres_divisor + 0.5);
  } else {
    info->dx = (int) floor (dx * info->divisor + 0.5);
    info->min_x = (int) floor (minx * info->divisor + 0.5);
    info->max_x = (int) floor (maxx * info->divisor + 0.5);
  }
  
  dy = grid.dy;
  miny = grid.miny - dy / 2.0 - Glob->data_to_com_delta_y;
  maxy = miny + grid.ny * dy;
  
  if (mdv.getField(com->data_field).getFieldHeader().proj_type ==
      MDV_PROJ_LATLON) {
    info->dy = (int) floor (dy * info->highres_divisor + 0.5);
    info->min_y = (int) floor (miny * info->highres_divisor + 0.5);
    info->max_y = (int) floor (maxy * info->highres_divisor + 0.5);
  } else {
    info->dy = (int) floor (dy * info->divisor + 0.5);
    info->min_y = (int) floor (miny * info->divisor + 0.5);
    info->max_y = (int) floor (maxy * info->divisor + 0.5);
  }

  dz = grid.dz;
  nz = grid.nz;
  if (mdv.getMasterHeader().vlevel_included) {
    MdvReadField &field = mdv.getField(com->data_field);
    minz = field.getVlevelHeader().vlevel_params[0] - dz / 2.0;
    maxz = field.getVlevelHeader().vlevel_params[nz-1] + dz / 2.0;
  } else {
    minz = grid.minz - dz / 2.0;
    maxz = minz + dz * (nz + 1);
  }

  info->dz = (int) floor (dz * info->divisor + 0.5);
  info->min_z = (int) floor (minz * info->divisor + 0.5);
  info->max_z = (int) floor (maxz * info->divisor + 0.5);
  
  info->north_angle = (int) floor
    (grid.proj_params.flat.rotation * info->divisor + 0.5);
  
}

/*****************************************************************
 * COPY_FIELD_PARAMS_TO_INFO: Copy field parameters to the
 * info structure 
 */

static void copy_field_params_to_info(MdvRead &mdv,
				      cdata_comm_t *com,
				      cdata_info_t *info,
				      si32 ifield)

{
  
  MDV_field_header_t *fhdr;
  
  if (ifield < mdv.getMasterHeader().n_fields) {

    fhdr = &mdv.getField(com->data_field).getFieldHeader();
    info->data_field = ifield;
    STRncopy(info->field_name, fhdr->field_name, LAB_LEN);
    STRncopy(info->field_units, fhdr->units, LAB_LEN);
    STRncopy(info->source_name, mdv.getMasterHeader().data_set_name, LAB_LEN);

  }
  
}

