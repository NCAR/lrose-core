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
 * CDATA_UTIL.C: Routines that retrieve data from 
 * Cartesian Data Servers
 *
 * Frank Hage July 1991 NCAR, Research Applications Program
 * Modified my Mike Dixon. 
 * Version 2 support added April 1999 - Frank Hage,
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <memory.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>

#include <cidd/cdata_util.h>
#include <toolsa/sockutil.h>
#include <toolsa/servmap.h>
#include <toolsa/pjg.h>
#include <toolsa/smu.h>
#include <toolsa/str.h>

#include <dataport/bigend.h>
#include <dataport/swap.h>

#define TRUE 1
#define FALSE 0

static void convert_command(cdata_comm_t *com,
			    cd_command_t *comm);

static int convert_info(cd_grid_info_t *g_info,
			cdata_info_t *info);

static void convert_reply(cd_reply_t *reply,
			  cdata_reply_t *rep);


static void convert_command_v2(cdata_ieee_comm_t *com,
			    cd_command_t *comm);

static int convert_info_v2(cd_grid_info_t *g_info,
			cdata_ieee_info_t *info);

static void convert_reply_v2(cd_reply_t *reply,
			  cdata_ieee_reply_t *rep);

static int get_data(cd_command_t *command,
		    cd_reply_t *reply,
		    cd_grid_info_t *grid_info,
		    double **plane_heights,
		    ui08 **data,
		    char *host_name,
		    int port,
		    int return_plane_heights,
		    int messages,
		    int debug,
		    char *calling_routine);

static int get_data_v2(cd_command_t *command,
                    cd_reply_t *reply,
                    cd_grid_info_t *grid_info,
                    double **plane_heights_p,
                    ui08 **data,
                    char *url,
                    int num_way_points,
                    double *lat_wp, /* Pointers to arrays of lat,lons */
                    double *lon_wp,
                    int return_plane_heights,
                    int messages,
                    int debug,
                    char *calling_routine);

static void load_string(char *source, char **target);

static int query_server_mapper(cdata_index_t *index);

extern ui08 *RLDecode8(ui08 *coded_data,
                       unsigned int *nbytes_full);

/*****************************************************************
 * CDATA_INIT()
 *
 * initialize cidd server index
 *
 *   prog_name : program name - used for error messages
 *
 *   servmap_host1, servmap_host2 : server mapper hosts
 *
 *   server_type, server_subtype, server_instance : server details
 *
 *   default_host, default_port : defaults for server location to be
 *   used if server mapper fails
 *
 *   messages_flag - if set, warnings and failure messages will be
 *     printed
 *
 *   debug_flag - if set, debug messages will be printed
 */

void cdata_init(char *prog_name,
		char *servmap_host1,
		char *servmap_host2,
		char *server_type,
		char *server_subtype,
		char *server_instance,
		char *default_host,
		int default_port,
		int messages_flag,
		int debug_flag,
		cdata_index_t *index)
     
{
  
  memset(index, 0, sizeof(cdata_index_t));
  
  load_string(prog_name, &index->prog_name);
  load_string(servmap_host1, &index->servmap_host1);
  load_string(servmap_host2, &index->servmap_host2);
  load_string(server_type, &index->server_type);
  load_string(server_subtype, &index->server_subtype);
  load_string(server_instance, &index->server_instance);
  load_string(default_host, &index->default_host);
  
  index->default_port = default_port;

  index->messages_flag = messages_flag;
  index->debug_flag = debug_flag;
  
  index->last_request_successful = FALSE;
  
}

/*************************************************************************
 * CDATA_READ()
 *
 * Reads cidd data using a cdata_index_t struct
 *
 * Returns 0 on success, -1 on failure
 *
 * The index struct points to the plane heights and data arrays.
 * The data in these arrays will be over-written on the next call,
 * therefore they should be copied as required.
 */

int cdata_read(cdata_index_t *index)
     
{

  SERVMAP_info_t *sinfo;
  
  /*
   * if the previuos request was not successful, get the
   * server info from the server mapper
   */
  
  if (!index->last_request_successful)
    query_server_mapper(index);
  
  while (index->current_server_num < index->n_servers) {

    sinfo = index->server_info + index->current_server_num;

    if (get_data(&index->command,
		 &index->reply,
		 &index->grid_info,
		 &index->plane_heights,
		 &index->data,
		 sinfo->host,
		 sinfo->port,
		 TRUE,
		 index->messages_flag,
		 index->debug_flag,
		 "cdata_read") == 0) {

      index->last_request_successful = TRUE;

      return (0);
      
    }

    index->current_server_num++;
	
  } /* while */

  /*
   * failure
   */

  index->last_request_successful = FALSE;
  return (-1);

}


/*****************************************************************
 * CDATA2_INIT()
 *
 *   prog_name : program name - used for error messages
 *
 *  uRL:
 *
 *   messages_flag - if set, warnings and failure messages will be
 *     printed
 *
 *   debug_flag - if set, debug messages will be printed
 */

void cdata2_init(char *prog_name,
		char *url,
		int messages_flag,
		int debug_flag,
		cdata2_index_t *index)
     
{
  
  memset(index, 0, sizeof(cdata2_index_t));
  
  load_string(prog_name, &index->prog_name);
  load_string(url, &index->url);
  
  index->messages_flag = messages_flag;
  index->debug_flag = debug_flag;
  
  index->last_request_successful = FALSE;
}

#define CDATA_URL_BUF_SIZE 1024
/*************************************************************************
 * CDATA2_READ()
 *
 * Reads cidd data using a cdata2_index_t struct
 *
 * Returns 0 on success, -1 on failure
 *
 * The index struct points to the plane heights and data arrays.
 * The data in these arrays will be over-written on the next call,
 * therefore they should be copied as required.
 */

int cdata2_read(cdata2_index_t *index,char *field_name)
     
{
    char url_buf[CDATA_URL_BUF_SIZE];

    /* assemble the whole url */
    STRncopy(url_buf, index->url,CDATA_URL_BUF_SIZE);
    strncat(url_buf, field_name, CDATA_URL_BUF_SIZE - 1);

    if (get_data_v2(&index->command,
		 &index->reply,
		 &index->grid_info,
		 &index->plane_heights,
		 &index->data,
                 url_buf,
                 0,
                 NULL, NULL,
                 TRUE,
		 index->messages_flag,
		 index->debug_flag,
		 "cdata_read") == 0) {

      index->last_request_successful = TRUE;

      return (0);
    }

  /*
   * failure
   */

  index->last_request_successful = FALSE;
  return (-1);

}

/**************************************************************************
 * CDATA_GET()
 *
 * Get a Plane of data. Returns ptr to data if successful, NULL otherwise.
 *
 * User must copy data for use; Routine uses internal static buffer, which
 * is overwritten each call.
 */

ui08 *cdata_get(cd_command_t *command,
		  cd_reply_t *reply,
		  cd_grid_info_t *grid_info,
		  char *host_name,
		  int port)
     
{

  static ui08 *data;
  
  if (get_data(command,
	       reply,
	       grid_info,
	       (double **) NULL,
	       &data,
	       host_name,
	       port,
	       FALSE,
	       FALSE,
	       FALSE,
	       "cdata_get")) {

    return ((ui08 *) NULL);

  } else {

    return (data);

  }

}

/**************************************************************************
 * CDATA_GET_WITH_HEIGHTS()
 *
 * Get a plane of data with heights.
 *
 * Returns 0 if successful, -1 otherwise.
 *
 * The data and plane heights will be overwritten on subsequent calls
 * to this routine - the client routine should copy the data if this
 * is a problem.
 */

int cdata_get_with_heights(cd_command_t *command,
			   cd_reply_t *reply,
			   cd_grid_info_t *grid_info,
			   double **plane_heights,
			   ui08 **data,
			   char *host_name,
			   int port)
     
{

  if (get_data(command,
	       reply,
	       grid_info,
	       plane_heights,
	       data,
	       host_name,
	       port,
	       TRUE,
	       FALSE,
	       FALSE,
	       "get_cidd_with_heights")) {

    return (-1);

  } else {

    return (0);

  }

}


/**************************************************************************
 * CDATA2_GET()
 *
 * Get a Plane of data. Returns ptr to data if successful, NULL otherwise.
 *
 * User must copy data for use; Routine uses internal static buffer, which
 * is overwritten each call.
 */

ui08 *cdata2_get(cd_command_t *command,
		  cd_reply_t *reply,
		  cd_grid_info_t *grid_info,
		  char *url)
{

  static ui08 *data;
  
  if (get_data_v2(command,
	       reply,
	       grid_info,
	       (double **) NULL,
	       &data,
	       url,
	       0,NULL,NULL,
	       FALSE,
	       FALSE,
	       FALSE,
	       "cdata2_get")) {

    return ((ui08 *) NULL);

  } else {
    return (data);

  }
}

/**************************************************************************
 * CDATA2_GET_WITH_HEIGHTS() Version 2 Protocol
 *
 * Get a plane of data with heights.
 * Returns 0 if successful, -1 otherwise.
 * The data and plane heights will be overwritten on subsequent calls
 * to this routine - the client routine should copy the data if this
 * is a problem.
 */

int cdata2_get_with_heights(cd_command_t *command,
			   cd_reply_t *reply,
			   cd_grid_info_t *grid_info,
			   double **plane_heights,
			   ui08 **data,
			   char *url)
     
{

  if (get_data_v2(command,
	       reply,
	       grid_info,
	       plane_heights,
	       data,
	       url,
	       0,NULL,NULL,
	       TRUE,
	       FALSE,
	       FALSE,
	       "cdata2_get_with_heights")) {

    return (-1);

  } else {

    return (0);

  }

}

/******************************************************************************
 * CONVERT_COMMAND: Convert the native floating point format command to the
 * integer based network command structure.  This structure still needs to be
 * byte-swapped before being sent to another process.  This is not done here
 * so that fields in the integer structure can still be updated before sending.
 */

static void convert_command(cdata_comm_t *com,
			    cd_command_t *comm)
     
{

  com->primary_com = comm->primary_com;
  com->second_com = comm->second_com;
  com->divisor = (long) CDATA_DIVISOR;
  com->lat_origin = comm->lat_origin * CDATA_DIVISOR;
  com->lon_origin = comm->lon_origin * CDATA_DIVISOR;
  com->ht_origin = comm->ht_origin * CDATA_DIVISOR;
  com->time_min = comm->time_min;
  com->time_cent = comm->time_cent;
  com->time_max = comm->time_max;
  com->min_x = comm->min_x * CDATA_DIVISOR;
  com->max_x = comm->max_x * CDATA_DIVISOR;
  com->min_y = comm->min_y * CDATA_DIVISOR;
  com->max_y = comm->max_y * CDATA_DIVISOR;
  com->min_z = comm->min_z * CDATA_DIVISOR;
  com->max_z = comm->max_z * CDATA_DIVISOR;
  com->data_field = comm->data_field;
  com->data_type = comm->data_type;
  com->add_data_len = 0;

}

/******************************************************************************
 * CONVERT_INFO: Convert the integer format reply to native floating point
 * format.  Byte-swapping should be done before calling this routine.
 */

static int convert_info(cd_grid_info_t *g_info,
			cdata_info_t *info)
     
{

  if (info->divisor == 0) {
    return (-1);
  }

  g_info->order = info->order;
  g_info->data_field = info->data_field;
  g_info->projection = info->projection;
  g_info->lat_origin = info->lat_origin / (double)info->divisor;
  g_info->lon_origin = info->lon_origin / (double)info->divisor;
  g_info->source_x = info->source_x / (double)info->divisor;
  g_info->source_y = info->source_y / (double)info->divisor;
  g_info->source_z = info->source_z / (double)info->divisor;
  g_info->ht_origin = info->ht_origin / (double)info->divisor;
  g_info->nx = info->nx;
  g_info->ny = info->ny;
  g_info->nz = info->nz;
  if (info->projection == PJG_LATLON &&
      info->highres_divisor == CDATA_HIGHRES_DIVISOR) {
    g_info->dx = info->dx / (double) info->highres_divisor;
    g_info->dy = info->dy / (double) info->highres_divisor;
    g_info->min_x = info->min_x / (double) info->highres_divisor;
    g_info->max_x = info->max_x / (double) info->highres_divisor;
    g_info->min_y = info->min_y / (double) info->highres_divisor;
    g_info->max_y = info->max_y / (double) info->highres_divisor;
  } else {
    g_info->dx = info->dx / (double)info->divisor;
    g_info->dy = info->dy / (double)info->divisor;
    g_info->min_x = info->min_x / (double)info->divisor;
    g_info->max_x = info->max_x / (double)info->divisor;
    g_info->min_y = info->min_y / (double)info->divisor;
    g_info->max_y = info->max_y / (double)info->divisor;
  }
  g_info->dz = info->dz / (double)info->divisor;
  g_info->min_z = info->min_z / (double)info->divisor;
  g_info->max_z = info->max_z / (double)info->divisor;
  g_info->north_angle = info->north_angle / (double)info->divisor;
  g_info->gate_spacing = info->gate_spacing / (double)info->divisor;
  g_info->wavelength = info->wavelength;
  g_info->frequency = info->frequency;
  g_info->min_range = info->min_range / (double)info->divisor;
  g_info->max_range = info->max_range / (double)info->divisor;
  g_info->num_gates = info->num_gates;
  g_info->min_gates = info->min_gates;
  g_info->max_gates = info->max_gates;
  g_info->num_tilts = info->num_tilts;
  g_info->min_elev = info->min_elev / (double)info->divisor;
  g_info->max_elev = info->max_elev / (double)info->divisor;
  g_info->radar_const = info->radar_const / (double)info->divisor;
  g_info->nyquist_vel = 0;
  g_info->delta_azmith = info->delta_azmith / (double)info->divisor;
  g_info->start_azmith = info->start_azmith / (double)info->divisor;
  g_info->beam_width = info->beam_width / (double)info->divisor;
  g_info->pulse_width = info->pulse_width;
  g_info->data_length = info->data_length;
  g_info->noise_thresh = info->noise_thresh / (double)info->divisor;
  g_info->nfields = info->nfields;
  
  STRncopy(g_info->units_label_x, info->units_label_x, LAB_LEN);
  STRncopy(g_info->units_label_y, info->units_label_y, LAB_LEN);
  STRncopy(g_info->units_label_z, info->units_label_z, LAB_LEN);
  STRncopy(g_info->field_units, info->field_units, LAB_LEN);
  STRncopy(g_info->field_name, info->field_name, LAB_LEN);
  STRncopy(g_info->source_name, info->source_name, LAB_LEN);

  return (0);

}

/******************************************************************************
 * CONVERT_REPLY: Convert the integer format reply to native floating point
 * format.  Byte-swapping should be done before calling this routine.
 */

static void convert_reply(cd_reply_t *reply,
			  cdata_reply_t *rep)
     
{

  reply->status = rep->status;
  reply->orient = rep->orient;
  reply->nx = rep->nx;
  reply->ny = rep->ny;
  reply->nz = rep->nz;
  reply->dx = rep->dx / (double)rep->divisor;
  reply->dy = rep->dy / (double)rep->divisor;
  reply->dz = rep->dz / (double)rep->divisor;
  reply->x1 = rep->x1;
  reply->x2 = rep->x2;
  reply->y1 = rep->y1;
  reply->y2 = rep->y2;
  reply->z1 = rep->z1;
  reply->z2 = rep->z2;
  reply->scale = rep->scale / (double)rep->divisor;
  reply->bias = rep->bias / (double)rep->divisor;
  reply->time_begin = rep->time_begin;
  reply->time_end = rep->time_end;
  reply->time_cent = rep->time_cent;
  reply->bad_data_val = rep->bad_data_val;
  reply->data_type = rep->data_type;
  reply->data_field = rep->data_field;
  reply->n_points = rep->n_points;
  reply->data_length = rep->data_length;

}

/******************************************************************************
 * CONVERT_COMMAND_V2: Convert the native floating point format command to the
 * V2 Protocol network command structure.  This structure still needs to be
 * byte-swapped before being sent to another process.  This is not done here
 * so that fields in the integer structure can still be updated before sending.
 */

static void convert_command_v2(cdata_ieee_comm_t *com,
			    cd_command_t *comm)
     
{

  com->primary_com = comm->primary_com;
  com->second_com = comm->second_com;
  com->lat_origin = comm->lat_origin;
  com->lon_origin = comm->lon_origin;
  com->ht_origin = comm->ht_origin;
  com->time_min = comm->time_min;
  com->time_cent = comm->time_cent;
  com->time_max = comm->time_max;
  com->min_x = comm->min_x;
  com->max_x = comm->max_x;
  com->min_y = comm->min_y;
  com->max_y = comm->max_y;
  com->min_z = comm->min_z;
  com->max_z = comm->max_z;
  com->data_type = comm->data_type;
  com->add_data_len = 0;
}

/******************************************************************************
 * CONVERT_INFO_V2: Convert the protocol format reply to native floating point
 * format.  Byte-swapping should be done before calling this routine.
 */

static int convert_info_v2(cd_grid_info_t *g_info,
			cdata_ieee_info_t *info)
     
{

  g_info->order = info->order;
  g_info->data_field = info->data_field;
  g_info->projection = info->projection;
  g_info->lat_origin = info->lat_origin;
  g_info->lon_origin = info->lon_origin;
  g_info->source_x = info->source_x;
  g_info->source_y = info->source_y;
  g_info->source_z = info->source_z;
  g_info->ht_origin = info->ht_origin;
  g_info->nx = info->nx;
  g_info->ny = info->ny;
  g_info->nz = info->nz;
  g_info->dx = info->dx;
  g_info->dy = info->dy;
  g_info->min_x = info->min_x;
  g_info->max_x = info->max_x;
  g_info->min_y = info->min_y;
  g_info->max_y = info->max_y;
  g_info->dz = info->dz;
  g_info->min_z = info->min_z;
  g_info->max_z = info->max_z;
  g_info->north_angle = info->north_angle;
  g_info->gate_spacing = info->gate_spacing;
  g_info->wavelength = info->wavelength;
  g_info->frequency = info->frequency;
  g_info->min_range = info->min_range;
  g_info->max_range = info->max_range;
  g_info->num_gates = info->num_gates;
  g_info->min_gates = info->min_gates;
  g_info->max_gates = info->max_gates;
  g_info->num_tilts = info->num_tilts;
  g_info->min_elev = info->min_elev;
  g_info->max_elev = info->max_elev;
  g_info->radar_const = info->radar_const;
  g_info->nyquist_vel = info->nyquist_vel;
  g_info->delta_azmith = info->delta_azmith;
  g_info->start_azmith = info->start_azmith;
  g_info->beam_width = info->beam_width;
  g_info->pulse_width = info->pulse_width;
  g_info->data_length = info->data_length;
  g_info->noise_thresh = info->noise_thresh;
  g_info->nfields = info->nfields;
  
  STRncopy(g_info->units_label_x, info->units_label_x, LAB_LEN);
  STRncopy(g_info->units_label_y, info->units_label_y, LAB_LEN);
  STRncopy(g_info->units_label_z,info->units_label_z, LAB_LEN);
  STRncopy(g_info->field_units, info->field_units, LAB_LEN);
  STRncopy(g_info->field_name, info->field_name, LAB_LEN);
  STRncopy(g_info->source_name, info->source_name, LAB_LEN);

  return (0);
}

/******************************************************************************
 * CONVERT_REPLY_V2: Convert the protocol format reply to native floating point
 * format.  Byte-swapping should be done before calling this routine.
 */

static void convert_reply_v2(cd_reply_t *reply,
			  cdata_ieee_reply_t *rep)
     
{

  reply->status = rep->status;
  reply->orient = rep->orient;
  reply->nx = rep->nx;
  reply->ny = rep->ny;
  reply->nz = rep->nz;
  reply->dx = rep->dx;
  reply->dy = rep->dy;
  reply->dz = rep->dz;
  reply->x1 = rep->x1;
  reply->x2 = rep->x2;
  reply->y1 = rep->y1;
  reply->y2 = rep->y2;
  reply->z1 = rep->z1;
  reply->z2 = rep->z2;
  reply->scale = rep->scale;
  reply->bias = rep->bias;
  reply->time_begin = rep->time_begin;
  reply->time_end = rep->time_end;
  reply->time_cent = rep->time_cent;
  reply->bad_data_val = rep->bad_data_val;
  reply->data_type = rep->data_type;
  reply->data_field = rep->data_field;
  reply->n_points = rep->n_points;
  reply->data_length = rep->data_length;
}

/**************************************************************************
 * GET_DATA
 *
 * Get a plane of data.
 *
 * Optionally returns the plane heights in an array of doubles.
 *
 * Returns 0 if successful, -1 otherwise.
 */

static int get_data(cd_command_t *command,
		    cd_reply_t *reply,
		    cd_grid_info_t *grid_info,
		    double **plane_heights_p,
		    ui08 **data,
		    char *host_name,
		    int port,
		    int return_plane_heights,
		    int messages,
		    int debug,
		    char *calling_routine)
     
{
  
  static long n_heights_allocated = 0;
  static long n_points_allocated = 0;
  static ui08 *data_array;
  static double *plane_heights;

  ui08 *data_ptr;
  
  int sockfd;
  unsigned int nbytes_full;
  
  ui32 *lheights;
  long n_heights, iheight;
  long nbytes_plane_heights;
  long field;
  
  cdata_comm_t com;
  cdata_reply_t rep;
  cdata_info_t info;

  *data = NULL;
  if ( return_plane_heights )
     *plane_heights_p = NULL;
   
  /*
   * return now if host and port not set
   */

  if (host_name == NULL || port == 0) {

    if (messages || debug) {
      fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
      fprintf(stderr, "Host and port not set.\n");
    }

    return(-1);

  }

  /*
   * set sockutil headers to new in case they were set to old by
   * the server mapper
   */

  SKU_set_headers_to_new();
  
  /*
   * convert the command from floating point
   */
  
  convert_command(&com, command);
  
  field = command->data_field;
  com.data_type = CDATA_CHAR;		/* Only type supported so far */
  com.primary_com |= GET_INFO;	/* This routine always gets grid info */
  com.primary_com |= GET_PLANE_HEIGHTS;	/* This routine always gets height info */
  
  /*
   * open the socket from the client to the server
   */
  
  if ((sockfd = SKU_open_client(host_name, port)) < 0) {
    
    if (messages || debug) {
      fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
      fprintf(stderr, "Cannot open connection to server.\n");
      fprintf(stderr, "Hostame, port : %s, %d\n", host_name, port);
    }
    
    return (-1);

  }
    
  /*
   * write the command to the server
   */
  
  BE_from_array_32(&com, sizeof (com));

  if(SKU_write(sockfd,
	       (char *) &com,
	       (long) sizeof(com), -1) != sizeof(com)) {
    fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
    fprintf(stderr,"Data request failed\n");
    fprintf(stderr, "Field %ld, host %s, port %d\n",
	    field, host_name, port);
    close(sockfd);

    if (messages || debug) {
      fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
      fprintf(stderr, "Cannot write command to server.\n");
      fprintf(stderr, "Host, port : %s, %d\n", host_name, port);
    }
    
    return (-1);
  }
  
  /*
   * read the reply from the server
   */
  
  if(SKU_read((int) sockfd,
	      (char *) &rep,
	      (long) sizeof(rep), -1) != sizeof(rep)) {
    fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
    fprintf(stderr,"Read Reply failed\n");
    fprintf(stderr, "Field %ld, host %s, port %d\n",
	    field, host_name, port);
    close(sockfd);

    if (messages || debug) {
      fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
      fprintf(stderr, "Cannot read reply from server.\n");
      fprintf(stderr, "Host, port : %s, %d\n", host_name, port);
    }
    
    return (-1);
  } 
  
  BE_to_array_32(&rep, sizeof (rep));
  
  /*
   * convert the reply to floating point format
   */
  
  convert_reply(reply, &rep);
  
  if(rep.status & INFO_FOLLOWS) {
    
    if(SKU_read((int) sockfd,
		(char *) &info,
		(long) sizeof(info), -1) != sizeof(info)) {
      
      close(sockfd);
      
      if (messages || debug) {
	fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
	fprintf(stderr, "Cannot read grid info from server.\n");
	fprintf(stderr, "Host, port : %s, %d\n", host_name, port);
      }
    
      return(-1);
    }
    
    BE_to_array_32(&info, (NUM_INFO_LONGS * sizeof(ui32)));
  
    /*
     * convert info to floating point
     */
  
    if (convert_info(grid_info, &info)) {
      if (messages || debug) {
	fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
	fprintf(stderr, "Cannot convert grid info.\n");
	fprintf(stderr, "Host, port : %s, %d\n", host_name, port);
      }
      return(-1);
    }
  
  }
  
  /*
   * if available, read plane heights
   */
  
  if(rep.status & PLANE_HEIGHTS_FOLLOW) {
    
    n_heights = info.nz * 3;

    nbytes_plane_heights = n_heights * sizeof(ui32);
    
    lheights = (ui32 *) malloc (nbytes_plane_heights);

    if(SKU_read((int) sockfd,
		(char *) lheights,
		nbytes_plane_heights, -1) != nbytes_plane_heights) {
      fprintf(stderr, "WARNING - %s:get_data\n", calling_routine);
      fprintf(stderr, "Couldn't read plane heights\n");
      close(sockfd);

      if (messages || debug) {
	fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
	fprintf(stderr, "Cannot read plane heights from server.\n");
	fprintf(stderr, "Host, port : %s, %d\n", host_name, port);
      }
    
      return(-1);
    }

    if (return_plane_heights) {

      /*
       * convert plane heights byte order
       */
      
      BE_to_array_32(lheights, nbytes_plane_heights);
      
      /*
       * allocate space for double array of plane heights
       */

      if (n_heights > n_heights_allocated) {
      
	if (n_heights_allocated == 0) {
	  plane_heights = (double *) malloc (n_heights * sizeof(double));
	} else {
	  plane_heights = (double *) realloc
	    (plane_heights, (n_heights * sizeof(double)));
	}
	
	n_heights_allocated = n_heights;
	
      } /* if (n_heights > n_heights_allocated) */
      
      /*
       * load up double array
       */
      
      for (iheight = 0; iheight < n_heights; iheight++) {
	plane_heights[iheight] =
	  (double) lheights[iheight] / (double) info.divisor;
      } /* iheight */
      
      *plane_heights_p = plane_heights;
      
    } /* if (return_plane_heights) */
    
    free((char *) lheights);

  } /* if(rep.status & PLANE_HEIGHTS_FOLLOW) */
  
  /*
   * if available, read plane data
   */
  
  if((rep.status & DATA_FOLLOWS) && (rep.n_points > 0)) {
    
    /*
     * allocated or reallocate plane data as required
     */
    
    if (rep.n_points > n_points_allocated) {
      
      if (n_points_allocated == 0) {
	data_array = (ui08 *)
	  malloc(rep.n_points * sizeof(ui08));
      } else {
	data_array = (ui08 *)
	  realloc(data_array, (rep.n_points * sizeof(ui08)));
      }
      
      n_points_allocated = rep.n_points;
      
    } /* if (rep.n_points > n_points_allocated) */
    
    memset (data_array, 0, (rep.n_points * sizeof(ui08)));
    
    if(SKU_read(sockfd,
		(char *) data_array,
		rep.n_points, -1) != rep.n_points) {

      close(sockfd);

      if (messages || debug) {
	fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
	fprintf(stderr, "Cannot read data from server.\n");
	fprintf(stderr, "Host, port : %s, %d\n", host_name, port);
      }
    
      return(-1);
    }

    /*
     * uncompress the data if required
     */

    data_ptr = RLDecode8((ui08 *) data_array,
			 &nbytes_full);

    /*
     * if compressed, free up compressed array and reset n_points
     */
    
    if (data_ptr != NULL) {

      free ((char *) data_array);
      data_array = data_ptr;
      reply->n_points = nbytes_full;
      n_points_allocated = nbytes_full;

    }
    
  } /* if((rep.status & DATA_FOLLOWS) ... */

  /*
   * return error if data asked for but not received, unless
   * new data was requested and there is none
   */
  
  if ((command->primary_com & GET_DATA) && 
      (rep.status & NO_DATA_AVAILABLE) &&
      !(rep.status & NO_NEW_DATA)) {

    close(sockfd);

    if (messages || debug) {
      fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
      fprintf(stderr, "Data requested but not received.\n");
      fprintf(stderr, "Host, port : %s, %d\n", host_name, port);
    }
    
    return(-1);
  }
  
  *data = data_array;

  close(sockfd);

  return (0);
  
}

#define COM2_BUF_LEN 1024
/**************************************************************************
 * GET_DATA_V2
 *
 * Get a plane of data.
 *
 * Optionally returns the plane heights in an array of doubles.
 *
 * Returns 0 if successful, -1 otherwise.
 */

static int get_data_v2(cd_command_t *command,
		    cd_reply_t *reply,
		    cd_grid_info_t *grid_info,
		    double **plane_heights_p,
		    ui08 **data,
		    char *url,
		    int num_way_points,
		    double *lat_wp, /* Pointers to arrays of lat,lons */
		    double *lon_wp, 
                    int return_plane_heights,
	            int messages,
		    int debug,
		    char *calling_routine)
     
{
  
  static long n_heights_allocated = 0;
  static long n_points_allocated = 0;
  static ui08 *data_array;
  static double *plane_heights;

  char com2_buf[COM2_BUF_LEN];
  char url_buf[COM2_BUF_LEN];
  char      *host_ptr,*port_ptr; /* Pointer for picking apart the url */
  char *ptr;
  ui08 *data_ptr;
  
  int i;
  int sockfd;
  int port;
  unsigned int nbytes_full;
  
  fl32 *fheights;
  long n_heights, iheight;
  long nbytes_plane_heights;
  
  cdata_ieee_comm_t com;
  cdata_ieee_comm2_t *com2;
  cdata_ieee_reply_t rep;
  cdata_ieee_info_t info;
  way_point_t *wp;

  *data = NULL;
  if ( return_plane_heights )
     *plane_heights_p = NULL;
   
  /*
   * return now if url not set
   */

  if (url == NULL ) {
    if (messages || debug) {
      fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
      fprintf(stderr, "URL not set.\n");
    }
    return(-1);
  }

  /* Copy the URL into a temporary area for host, port extraction */
  STRncopy(url_buf,url,COM2_BUF_LEN);
  host_ptr = strstr(url_buf,"://"); /* Find the first part*/ 
  if(host_ptr != NULL) {
       host_ptr += 3; /* Skip over the host part now */
       port_ptr = strchr(host_ptr,':');
  } else {
    if (messages || debug) {
      fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
      fprintf(stderr, "URL Error - No host section found.\n");
    }
    return(-1);
  }

  if(port_ptr != NULL) { 
    *port_ptr = '\0';
    port_ptr++;
    /* Null terminate port string */
    ptr = strchr(host_ptr,'/');
    if(ptr != NULL) *ptr = '\0';

    port = atoi(port_ptr);

  } else { /* No explicit port specified */
    port = CDATA_SERVER_PORT; /* Use the Default */

    /* Null terminate the host name string */
    port_ptr = strchr(host_ptr,'/');
    if(port_ptr != NULL) *port_ptr = '\0';
  }

  memset(com2_buf,0,COM2_BUF_LEN);
  com2 = (cdata_ieee_comm2_t *) com2_buf;

  /* Make sure enough space is made for null terminator */
  com2->url_len = strlen(url) +1;
  /* Force following way points to be word aligned - Per Spec */
  com2->url_len += 4 - ((com2->url_len) % 4);                 
                                               
  STRncopy(com2->url,url,com2->url_len);  
  com2->num_way_points = num_way_points;

  wp = (way_point_t *) com2->url + com2->url_len;
  for(i=0 ; i < num_way_points; i++) {
      wp->lon = lon_wp[i];
      wp->lat = lat_wp[i];
      wp++;
  }

  SKU_set_headers_to_new();
  
  /*
   * convert the command to v2 protocol struct 
   */
  
  convert_command_v2(&com, command);
  
  com.data_type = CDATA_CHAR;		/* Only type supported so far */
  com.primary_com |= GET_INFO;	/* This routine always gets grid info */
  com.primary_com |= GET_PLANE_HEIGHTS;	/* This routine always gets height info */
  com.primary_com |= CMD_USE_IEEE_FLOAT;  /* Use Version 2 Protocols */

  com.add_data_len = sizeof(cdata_ieee_comm2_t)  +
                        com2->url_len +
                        (com2->num_way_points * sizeof(way_point_t));

  /*
   * open the socket from the client to the server
   */
  
  if ((sockfd = SKU_open_client(host_ptr, port)) < 0) {
    
    if (messages || debug) {
      fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
      fprintf(stderr, "Cannot open connection to server.\n");
      fprintf(stderr, "URL:%s, port: %d\n", url, port);
    }
    
    return (-1);

  }
    
  /*
   * write the commands to the server
   */
  
  BE_from_array_32(&com, sizeof (com));

  if(SKU_write(sockfd, (char *) &com,
	       (long) sizeof(com), -1) != sizeof(com)) {
    close(sockfd);

    if (messages || debug) {
      fprintf(stderr, "ERROR - %s:get_data_v2\n", calling_routine);
      fprintf(stderr, "Cannot write command to server.\n");
      fprintf(stderr, "Url : %s\n", url);
    }
    
    return (-1);
  }

  /* Version 2 Protocol Struct */
  com2_struct_to_BE((void *) &com2);

  if(SKU_write(sockfd, (char *) com2,
	       (long) sizeof(*com2), -1) != sizeof(*com2)) {
    close(sockfd);

    if (messages || debug) {
      fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
      fprintf(stderr, "Cannot write command2 to server.\n");
      fprintf(stderr, "Url : %s\n", url);
    }
    
    return (-1);
  }
  
  /*
   * read the reply from the server
   */
  
  if(SKU_read((int) sockfd,
	      (char *) &rep,
	      (long) sizeof(rep), -1) != sizeof(rep)) {
    if (messages || debug) {
      fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
      fprintf(stderr, "Cannot read reply from server.\n");
      fprintf(stderr, "URL %s, port %d\n", url, port);
    }
    
    return (-1);
  } 
  
  BE_to_array_32(&rep, sizeof (rep));
  
  /*
   * convert the reply to floating point format
   */
  
  convert_reply_v2(reply, &rep);
  
  if(rep.status & INFO_FOLLOWS) {
    
    if(SKU_read((int) sockfd,
		(char *) &info,
		(long) sizeof(info), -1) != sizeof(info)) {
      
      close(sockfd);
      
      if (messages || debug) {
	fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
	fprintf(stderr, "Cannot read grid info from server.\n");
        fprintf(stderr, "URL %s, port %d\n", url, port);
      }
    
      return(-1);
    }
    
    BE_to_array_32(&info, (NUM_INFO_LONGS * sizeof(ui32)));
  
    /*
     * convert info to floating point
     */
  
    if (convert_info_v2(grid_info, &info)) {
      if (messages || debug) {
	fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
	fprintf(stderr, "Cannot convert grid info.\n");
        fprintf(stderr, "URL %s, port %d\n", url, port);
      }
      return(-1);
    }
  
  }
  
  /*
   * if available, read plane heights
   */
  
  if(rep.status & PLANE_HEIGHTS_FOLLOW) {
    
    n_heights = info.nz * 3;

    nbytes_plane_heights = n_heights * sizeof(fl32);
    
    fheights = (fl32 *) malloc (nbytes_plane_heights);

    if(SKU_read((int) sockfd,
		(char *) fheights,
		nbytes_plane_heights, -1) != nbytes_plane_heights) {
      close(sockfd);

      if (messages || debug) {
	fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
	fprintf(stderr, "Cannot read plane heights from server.\n");
        fprintf(stderr, "URL %s, port %d\n", url, port);
      }
    
      return(-1);
    }

    if (return_plane_heights) {

      /*
       * convert plane heights byte order
       */
      
      BE_to_array_32(fheights, nbytes_plane_heights);
      
      /*
       * allocate space for double array of plane heights
       */

      if (n_heights > n_heights_allocated) {
      
	if (n_heights_allocated == 0) {
	  plane_heights = (double *) malloc(n_heights * sizeof(double));
	} else {
	  plane_heights = (double *) realloc
	    (plane_heights, (n_heights * sizeof(double)));
	}
	
	n_heights_allocated = n_heights;
	
      } /* if (n_heights > n_heights_allocated) */
      
      /*
       * load up double array
       */
      
      for (iheight = 0; iheight < n_heights; iheight++) {
	plane_heights[iheight] = fheights[iheight];
      } /* iheight */
      
      *plane_heights_p = plane_heights;
      
    } /* if (return_plane_heights) */
    
    free((char *) fheights);

  } /* if(rep.status & PLANE_HEIGHTS_FOLLOW) */
  
  /*
   * if available, read plane data
   */
  
  if((rep.status & DATA_FOLLOWS) && (rep.n_points > 0)) {
    
    /*
     * allocated or reallocate plane data as required
     */
    
    if (rep.n_points > n_points_allocated) {
      
      if (n_points_allocated == 0) {
	data_array = (ui08 *) malloc(rep.n_points * sizeof(ui08));
      } else {
	data_array = (ui08 *)
	  realloc(data_array, (rep.n_points * sizeof(ui08)));
      }
      
      n_points_allocated = rep.n_points;
      
    } /* if (rep.n_points > n_points_allocated) */
    
    memset (data_array, 0, (rep.n_points * sizeof(ui08)));
    
    if(SKU_read(sockfd,
		(char *) data_array,
		rep.n_points, -1) != rep.n_points) {

      close(sockfd);

      if (messages || debug) {
	fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
	fprintf(stderr, "Cannot read data from server.\n");
        fprintf(stderr, "URL %s, port %d\n", url, port);
      }
    
      return(-1);
    }

    /*
     * uncompress the data if required
     */

    data_ptr = RLDecode8((ui08 *) data_array,
			 &nbytes_full);

    /*
     * if compressed, free up compressed array and reset n_points
     */
    
    if (data_ptr != NULL) {

      free ((char *) data_array);
      data_array = data_ptr;
      reply->n_points = nbytes_full;
      n_points_allocated = nbytes_full;

    }
    
  } /* if((rep.status & DATA_FOLLOWS) ... */

  /*
   * return error if data asked for but not received, unless
   * new data was requested and there is none
   */
  
  if ((command->primary_com & GET_DATA) && 
      (rep.status & NO_DATA_AVAILABLE) &&
      !(rep.status & NO_NEW_DATA)) {

    close(sockfd);

    if (messages || debug) {
      fprintf(stderr, "ERROR - %s:get_data\n", calling_routine);
      fprintf(stderr, "Data requested but not received.\n");
      fprintf(stderr, "URL %s, port %d\n", url, port);
    }
    
    return(-1);
  }
  
  *data = data_array;

  close(sockfd);

  return (0);
  
}

/********************************************************************
 * load_string()
 *
 * Allocates mem for a string and copies it in
 */

static void load_string(char *source, char **target)
     
{
  
  if (*target == (char *) NULL) {
    
    *target = (char *) malloc(strlen(source) + 1);
    
  } else {
    
    *target = (char *) realloc(*target, (strlen(source) + 1));
    
  }
  
  strcpy(*target, source);
  
}

/*************************************************************************
 * query_server_mapper()
 *
 * get list of available servers
 *
 * returns 0 on success, -1 on failure, in which case the default
 * host and port are set in server info
 *
 *************************************************************************/

static int query_server_mapper(cdata_index_t *index)
     
{
  
  long i;
  SERVMAP_request_t request;
  SERVMAP_info_t *info;
  
  /*
   * initial allocation
   */

  if (index->server_info == NULL) {
    
    index->server_info = (SERVMAP_info_t *) malloc (sizeof(SERVMAP_info_t));
    
  }

  /*
   * load up request struct
   */
  
  memset (&request, 0, sizeof(SERVMAP_request_t));
  
  STRncopy(request.server_type, index->server_type,
	  SERVMAP_NAME_MAX);
  STRncopy(request.server_subtype, index->server_subtype,
	  SERVMAP_NAME_MAX);
  STRncopy(request.instance, index->server_instance,
	  SERVMAP_INSTANCE_MAX);
  
  request.want_realtime = index->want_realtime;
  
  request.time = index->command.time_cent;
  
  if (SMU_requestInfo(&request, (int *) &index->n_servers, &info,
		      index->servmap_host1,
		      index->servmap_host2) != 1) {
    SKU_set_headers_to_new();
    index->n_servers = 0;
  }

  /*
   * set sockutil headers to new in case they were set to old by
   *  the server mapper
   */
  
  SKU_set_headers_to_new();
  
  if (index->n_servers > 0) {

    /*
     * success, realloc space for info, and copy info into it
     */
    
    index->server_info = (SERVMAP_info_t *) realloc
      (index->server_info,
       (index->n_servers * sizeof(SERVMAP_info_t)));
    
    memcpy (index->server_info,
	    info,
	    (index->n_servers * sizeof(SERVMAP_info_t)));

    index->current_server_num = 0;

    /*
     * set type, subtype and instance in case not set by server mapper
     */

    for (i = 0; i < index->n_servers; i++) {

      STRncopy(index->server_info[i].server_type, index->server_type,
	      SERVMAP_NAME_MAX);
      STRncopy(index->server_info[i].server_subtype, index->server_subtype,
	      SERVMAP_NAME_MAX);
      STRncopy(index->server_info[i].instance, index->server_instance,
	      SERVMAP_INSTANCE_MAX);

    }
    
    return (0);
    
  } else {

    /*
     * failure, set default values
     */
    
    STRncopy(index->server_info[0].server_type, index->server_type,
	    SERVMAP_NAME_MAX);
    STRncopy(index->server_info[0].server_subtype, index->server_subtype,
	    SERVMAP_NAME_MAX);
    STRncopy(index->server_info[0].instance, index->server_instance,
	    SERVMAP_INSTANCE_MAX);
    
    STRncopy(index->server_info[0].host, index->default_host,
	    SERVMAP_HOST_MAX);
    
    index->server_info[0].port = index->default_port;

    index->n_servers = 1;

    index->current_server_num = 0;

    return (-1);

  }
  
}

/*****************************************************************
 * COM2_STRUCT_TO_BE() make sure struct elements are in correct
 * byte order for this host
 */

void  com2_struct_to_BE(cdata_ieee_comm2_t *com2)
{
    int i;
    char *ptr;
    way_point_t *wp;

    if(BE_is_big_endian()) return;

    ptr = (char *) com2->url;
    ptr += com2->url_len;
    wp = (way_point_t *) ptr;

    for(i=0; i < com2->num_way_points; i++) {
        SWAP_array_32((ui32 *) wp,sizeof(way_point_t));
        wp++;
    }
    SWAP_array_32((ui32 *) com2,(ui32)COM2_SI32_BYTES); /* swap the first part */

}

/*****************************************************************
 * COM2_STRUCT_FROM_BE() make sure struct elements are in correct
 * byte order for this host
 */

void  com2_struct_from_BE(void * buf)
{
    int i;
    char *ptr;
    way_point_t *wp;
    cdata_ieee_comm2_t *com2;

    if(BE_is_big_endian()) return;

    /* swap the first part */
    SWAP_array_32((ui32 *) buf,(ui32)COM2_SI32_BYTES);

    com2 = (cdata_ieee_comm2_t *) buf;

    ptr = (char *) com2->url;
    ptr += com2->url_len;

    wp = (way_point_t *) ptr;
    for(i=0; i < com2->num_way_points; i++) {
        SWAP_array_32((ui32 *) wp,sizeof(way_point_t));
        wp++;
    }
}

