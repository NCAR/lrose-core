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
 * operate.c
 *
 * Subroutines for DobsonServer
 * 
 * Frank Hage, RAP, NCAR, Boulder, CO, USA
 *
 * Updated by Mike Dixon, Dec 1991
 */

#include "DobsonServer.h"
#include <dataport/bigend.h>

static void copy_field_params_to_info(vol_file_handle_t *v_handle,
				      cdata_info_t *info,
				      si32 field);

static void copy_grid_params_to_info(vol_file_handle_t *index,
				     cdata_comm_t *com,
				     cdata_info_t *info);

static void copy_vol_params_to_info(vol_file_handle_t *index,
				    cdata_info_t *info);

static void initialize_reply(cdata_reply_t *reply,
			     cdata_comm_t *com);

static void process_request(cdata_comm_t *com,
			    int sockfd,
			    vol_file_handle_t *v_handle);

static void send_reply(cdata_comm_t *com,
		       cdata_reply_t *reply,
		       cdata_info_t *info,
		       ui08 *ptr,
		       int sockfd,
		       vol_file_handle_t *v_handle);

#define DEG_TO_RAD 0.01745329251994372

#define LISTEN_MSECS 1000 /* the time in secs between checking
			   * for registration */

void operate(void)

{
  
  int forever = TRUE;
  int protofd, sockfd;
  vol_file_handle_t v_handle;
  cdata_comm_t com;
  
  /*
   * initialize volume file handle
   */
  
  RfInitVolFileHandle(&v_handle,
		      Glob->prog_name,
		      (char *) NULL,
		      (FILE *) NULL);

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
      
      process_request(&com, sockfd, &v_handle);
      
    }
    
    close(sockfd);
    
  } /* while (forever) */
  
}

/*****************************************************************
 * PROCESS_REQUEST: Take an incoming request and satisfy it 
 */

static void process_request(cdata_comm_t *com,
			    int sockfd,
			    vol_file_handle_t *v_handle)
     
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

  /*
   * read in the data volume
   */
  
  PMU_auto_register("Reading in data volume");
  
  if (read_volume(v_handle, com)) {
    
    /*
     * read failed
     */
    
    reply.status |= NO_INFO | NO_DATA;
    
  } else if (com->data_field >= v_handle->vol_params->nfields) {

    /*
     * field number is out of range
     */

    if (Glob->params.debug) {
      fprintf(stderr, "Field num %ld exceeds max of %ld\n",
	      (long) com->data_field,
	      (long) v_handle->vol_params->nfields);
    }
    
    reply.status |= NO_INFO | NO_DATA;

  } else {

    if (com->primary_com & GET_NEW) {
      reply.status |= NEW_DATA;
    }
    
    copy_vol_params_to_info(v_handle, &info);
    
    copy_grid_params_to_info(v_handle, com, &info);
    
    copy_field_params_to_info(v_handle, &info,
			      com->data_field);
    
    /*
     * get the data if requested
     */
    
    if(com->primary_com & GET_DATA) {
      data_ptr = (ui08 *) NULL;
      if(Glob->params.debug) {
	fprintf(stderr, "getting data\n");
      }
      data_ptr = get_grid_data(com, &reply, &info, v_handle);
    }
    
  } /* if (read_volume(v_handle, com)) */

  PMU_auto_register("Sending reply");

  send_reply(com, &reply, &info, data_ptr, sockfd, v_handle);
  
  if(data_ptr != NULL) {
    ufree((char *) data_ptr);

  if (Glob->params.free_volume)
    RfFreeVolPlanes(v_handle, "process_request");
  
  }
  
}

/*****************************************************************
 * INITIALIZE_REPLY: Set up default values in to the reply
 */

static void initialize_reply(cdata_reply_t *reply,
			     cdata_comm_t *com)
     
{
  
  reply->divisor = CDATA_DIVISOR;
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
		       vol_file_handle_t *v_handle)
     
{
  
  si32 *tmp_plane_heights, *tmp_ht, ht;
  si32 nheights;
  si32 nbytes_plane_heights;
  si32 iheight, iplane;
  
  double scalez;
  
  cdata_reply_t tmp_reply;
  cdata_info_t tmp_info;
  
  reply->data_length = 0;

  if (!(reply->status & NO_INFO)) {
    nheights = info->nz * N_PLANE_HEIGHT_VALUES;
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
    reply->data_length +=
      (nheights * sizeof(si32));
    
  }
  
  /*
   * if data was requested and is available,
   * set reply status accordingly
   */
  
  if((com->primary_com & GET_DATA) &&
     !(reply->status & NO_DATA)) {
    
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
   * Send plane heights with reply if required
   */
  
  if(reply->status & PLANE_HEIGHTS_FOLLOW) {
    
    tmp_plane_heights = (si32 *) umalloc
      ((ui32) nheights * sizeof(si32));
    
    tmp_ht = tmp_plane_heights;
    scalez = (double) v_handle->vol_params->cart.scalez;
    
    for (iplane = 0; iplane < info->nz; iplane++) {
      for (iheight = 0; iheight < N_PLANE_HEIGHT_VALUES; iheight++) {
	ht = v_handle->plane_heights[iplane][iheight];
	*tmp_ht = (si32) floor
	  (((double) ht / scalez) * (double) info->divisor + 0.5);
	tmp_ht++;
      } /* iheight */
    } /* iplane */

    nbytes_plane_heights = nheights * sizeof(si32);
    
    BE_from_array_32((ui32 *) tmp_plane_heights,
		     (ui32) nbytes_plane_heights);
    
    if(SKU_write((int) sockfd,
		 (char *) tmp_plane_heights,
		 nbytes_plane_heights,
		 -1) != nbytes_plane_heights) {
      fprintf(stderr,"Problems writing plane heights to socket\n");
    }
    
    if(Glob->params.debug) {
      fprintf(stderr, "sending plane heights\n");
    }
    
    ufree((char *) tmp_plane_heights);
    
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

static void copy_vol_params_to_info(vol_file_handle_t *index,
				    cdata_info_t *info)

{
  
  si32 nelev;
  double start_range, gate_spacing;
  vol_params_t *vparams;
  
  vparams = index->vol_params;
  
  info->divisor = CDATA_DIVISOR;
  info->order = DATA_ORDER;
  
  info->wavelength /* um */ = vparams->radar.wavelength;
  
  info->frequency /* Khz */ = floor
    ((3.0e11 / vparams->radar.wavelength) + 0.5);
  
  gate_spacing = (double) vparams->radar.gate_spacing / 1000000.0;
  start_range = (double) vparams->radar.start_range / 1000000.0;
  
  info->gate_spacing /* km */ = floor (gate_spacing * CDATA_DIVISOR + 0.5);
  
  info->min_range /* km */ = floor (start_range * CDATA_DIVISOR + 0.5);
  
  info->max_range /* km */ = floor
    ((start_range + gate_spacing * vparams->radar.ngates) *
     CDATA_DIVISOR + 0.5);
  
  info->num_gates = vparams->radar.ngates;
  info->min_gates = vparams->radar.ngates;
  info->max_gates = vparams->radar.ngates;
  
  info->num_tilts = vparams->radar.nelevations;

  nelev = vparams->radar.nelevations;
  if (nelev > 0) {
      info->min_elev = floor
	  (((double)index->radar_elevations[0] / DEG_FACTOR)
	   * CDATA_DIVISOR + 0.5);
      info->max_elev = floor
	  (((double)index->radar_elevations[nelev - 1]
	    / DEG_FACTOR) * CDATA_DIVISOR + 0.5);
  } else {
      info->min_elev = 0;
      info->max_elev = 0;
  }
  
  info->radar_const = floor(((double) RADAR_CONST) *
			    CDATA_DIVISOR + 0.5);
  
  info->nyquist_vel = floor
    ((((double) vparams->radar.prf / 1000.0) *
      (double) vparams->radar.wavelength / 1000000.0) *
     CDATA_DIVISOR / 4.0 + 0.5);
  
  info->delta_azmith = floor
    (((double)vparams->radar.delta_azimuth / DEG_FACTOR) *
     CDATA_DIVISOR + 0.5);
  
  info->start_azmith = floor
    (((double)vparams->radar.start_azimuth / DEG_FACTOR) *
     CDATA_DIVISOR + 0.5);
  
  info->beam_width = floor
    (((double) vparams->radar.beam_width / DEG_FACTOR) *
     CDATA_DIVISOR + 0.5);
  
  info->pulse_width = vparams->radar.pulse_width;
  
  info->noise_thresh = NOISE_THRESH;
  
  info->nfields = vparams->nfields;
  
  strncpy(info->units_label_x, vparams->cart.unitsx, LAB_LEN);
  strncpy(info->units_label_y, vparams->cart.unitsy, LAB_LEN);
  strncpy(info->units_label_z, vparams->cart.unitsz, LAB_LEN);
  
  strncpy(info->source_name, vparams->radar.name, LAB_LEN);
  
}

/*****************************************************************
 * COPY_GRID_PARAMS_TO_INFO: Copy volume parameters to the
 * info structure 
 */

static void copy_grid_params_to_info(vol_file_handle_t *v_handle,
				     cdata_comm_t *com,
				     cdata_info_t *info)

{
  
  si32 nplanes;
  double minx, miny, minz;
  double maxx, maxy, maxz;
  double dx, dy, dz;
  double scalez;
  double com_divisor;
  double com_lat, com_lon;
  double data_lat, data_lon;
  double range, theta;
  vol_params_t *vparams;
  
  vparams = v_handle->vol_params;
  
  info->projection = Glob->params.projection;

  if (Glob->params.projection == PJG_LATLON) {

    Glob->data_to_com_delta_x = 0.0;
    Glob->data_to_com_delta_y = 0.0;

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
      data_lat = (double)vparams->cart.latitude / DEG_FACTOR;
      data_lon = (double)vparams->cart.longitude / DEG_FACTOR;
    }
    
    uLatLon2RTheta(data_lat, data_lon,
		   com_lat, com_lon,
		   &range, &theta);
    
    Glob->data_to_com_delta_x = range * sin (theta * DEG_TO_RAD);
    Glob->data_to_com_delta_y = range * cos (theta * DEG_TO_RAD);

  }

  if (Glob->params.lat_lon_override) {
    info->lat_origin = floor
     (Glob->params.lat * CDATA_DIVISOR + 0.5);
    info->lon_origin = floor
     (Glob->params.lon * CDATA_DIVISOR + 0.5);
  } else {
    info->lat_origin = floor
      (((double)vparams->cart.latitude / DEG_FACTOR) * CDATA_DIVISOR + 0.5);
    info->lon_origin = floor
      (((double)vparams->cart.longitude / DEG_FACTOR) * CDATA_DIVISOR + 0.5);
  }
  info->ht_origin = 0.0;
  
  info->source_x = floor
    (((double) vparams->cart.radarx / vparams->cart.scalex) *
     CDATA_DIVISOR + 0.5);
  
  info->source_y = floor
    (((double) vparams->cart.radary / vparams->cart.scaley) *
     CDATA_DIVISOR + 0.5);
  
  info->source_z = floor
    ((vparams->radar.altitude / 1000.0) * CDATA_DIVISOR + 0.5);
  
  info->nx = vparams->cart.nx;
  info->ny = vparams->cart.ny;
  info->nz = vparams->cart.nz;
  
  dx = ((double) vparams->cart.dx / (double) vparams->cart.scalex);
  minx = (((double) vparams->cart.minx / (double) vparams->cart.scalex)
	  - dx/2.0 - Glob->data_to_com_delta_x);
  maxx = minx + ((double) vparams->cart.nx * dx);
  
  if (Glob->params.projection == PJG_LATLON) {
    info->dx = floor (dx * CDATA_LATLON_DIVISOR + 0.5);
    info->min_x = floor (minx * CDATA_LATLON_DIVISOR + 0.5);
    info->max_x = floor (maxx * CDATA_LATLON_DIVISOR + 0.5);
  } else {
    info->dx = floor (dx * CDATA_DIVISOR + 0.5);
    info->min_x = floor (minx * CDATA_DIVISOR + 0.5);
    info->max_x = floor (maxx * CDATA_DIVISOR + 0.5);
  }
  
  dy = ((double) vparams->cart.dy / (double) vparams->cart.scaley);
  miny = (((double) vparams->cart.miny / (double) vparams->cart.scaley)
	  - dy/2.0 - Glob->data_to_com_delta_y);
  maxy = miny + ((double) vparams->cart.ny * dy);
  
  if (Glob->params.projection == PJG_LATLON) {
    info->dy = floor (dy * CDATA_LATLON_DIVISOR + 0.5);
    info->min_y = floor (miny * CDATA_LATLON_DIVISOR + 0.5);
    info->max_y = floor (maxy * CDATA_LATLON_DIVISOR + 0.5);
  } else {
    info->dy = floor (dy * CDATA_DIVISOR + 0.5);
    info->min_y = floor (miny * CDATA_DIVISOR + 0.5);
    info->max_y = floor (maxy * CDATA_DIVISOR + 0.5);
  }
  
  dz = ((double) vparams->cart.dz / (double) vparams->cart.scalez);
  scalez = (double) v_handle->vol_params->cart.scalez;
  nplanes = v_handle->vol_params->cart.nz;
  minz = (double)
    v_handle->plane_heights[0][PLANE_BASE_INDEX] / scalez;
  maxz = (double)
    v_handle->plane_heights[nplanes - 1][PLANE_TOP_INDEX] / scalez;

  info->dz = floor (dz * CDATA_DIVISOR + 0.5);
  info->min_z = floor (minz * CDATA_DIVISOR + 0.5);
  info->max_z = floor (maxz * CDATA_DIVISOR + 0.5);
  
  info->north_angle = floor
    (((double)vparams->cart.rotation / DEG_FACTOR) * CDATA_DIVISOR + 0.5);
  
}

/*****************************************************************
 * COPY_FIELD_PARAMS_TO_INFO: Copy field parameters to the
 * info structure 
 */

static void copy_field_params_to_info(vol_file_handle_t *v_handle,
				      cdata_info_t *info,
				      si32 field)

{
  
  field_params_t **fparams;

  if (field < v_handle->vol_params->nfields) {

    fparams = v_handle->field_params;
    info->data_field = field;
    strncpy(info->field_name, fparams[field]->name, LAB_LEN);
    strncpy(info->field_units, fparams[field]->units, LAB_LEN);

  }
  
}

