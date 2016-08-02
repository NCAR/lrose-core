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
 * MDV_SERVER_GET_CLIENT
 * F. Hage - NCAR/RAP
 */

#define MDV_SERVER_GET_CLIENT

#include "mdv_server.h"

void process_request(int id, cdata_comm_t *com, void *comm2_buf);
void  com2_struct_from_BE(void * buf);
/*****************************************************************
 * GET_CLIENT_LOOP : Start the service  and get client requests for
 *  procesing
 */

void get_client_loop(void)
{
  int    cnt;
  int    still_active;
  cdata_comm_t    com;    /* The incomming command request */
  char com2_buf[4096];   /* Buffer for command part 2 of version 2 */

  still_active = 1;
  cnt = 0;
  do { 
    /* Wait up to 500 msec for a new client */
    if ((gd.sockfd = SKU_get_client_timed(gd.protofd, 500)) > 0) {
      if (SKU_read(gd.sockfd, &com, sizeof(com), 10) != sizeof(com)) {
	if(!gd.daemonize_flag) fprintf(stderr,"Problems reading Message\n");
	cnt++;
	if(cnt > 3) {
	  close(gd.protofd);
	  sleep(1);
	  init_sockets();
	  cnt = 0;
	}
      } else {
	if (gd.reg) PMU_auto_register("Processing Client Request");
	if(!gd.daemonize_flag) {
	    fprintf(stderr,"Processing Client Request\n");
	    fflush(stderr);
	}
		 
	/* This will byte order all cdata struct versions correctly */
	BE_to_array_32((ui32 *)&com, (ui32)sizeof(com));

	if(com.add_data_len >0)  {
	  if(com.add_data_len > 4096) {
	      if(!gd.daemonize_flag) {
		 fprintf(stderr,"Command Buffer too big!: size %d\n",com.add_data_len);
	      }
	    com.add_data_len = 4096;
	  }
          if (SKU_read(gd.sockfd, com2_buf, com.add_data_len, 10) != 
			 com.add_data_len) {
	      if(!gd.daemonize_flag)
		 fprintf(stderr,"Problems Reading Command 2\n");
	  }
	}
    
	gd.clients_serviced++;
	process_request(0, &com,(void *) com2_buf);
      }
    
      close(gd.sockfd);
    } else {
      switch (gd.sockfd) {
      case -1:
	if (gd.reg) {
	  register_server(gd.top_dir,gd.num_top_dirs,gd.data_file_suffix,
			  gd.app_instance,gd.service_subtype,gd.user_info,
			  gd.port,gd.live_update,gd.last_request_time,
			  gd.servmap_host1,gd.servmap_host2,gd.clients_serviced);

	  PMU_auto_register("Waiting for Client Request");
	}
	break;

      case -2:
      case -3:
      default:
	close(gd.protofd);
	init_sockets();
	break;
      }
    }

  } while (still_active);

  return;
}
 
/*****************************************************************
 * PROCESS_REQUEST: Take an incomming request and satisfy it 
 */

void process_request(int id, cdata_comm_t *com, void *comm2_buf)

{
  int i;
  ui08      *data_ptr;            /* pointer to data */
  time_t    time_cent;
  time_t    time_begin;
  time_t    time_end;
  char      *ptr,*ptr2,*ptr3;;
  cdata_ieee_comm_t comi;
  cdata_ieee_comm2_t *com2 = NULL;

  gd.last_request_time = time(0);

  if (com->time_cent < com->time_min ||
      com->time_cent > com->time_max)
    time_cent = (com->time_min + com->time_max) / 2;
  else
    time_cent = com->time_cent;

  if(com->primary_com & CMD_USE_IEEE_FLOAT || com->add_data_len > 0) {
      gd.protocol_version = 2;

      // Make a copy of the command struct for internal use
      memcpy(&comi,com,sizeof(cdata_ieee_comm_t));

      if(comm2_buf != NULL) {
	  com2_struct_from_BE(comm2_buf);
	  com2 = (cdata_ieee_comm2_t *) comm2_buf;

          /* Extract the directory part out of the URL */
	  ptr2 = NULL;
          ptr = strstr(com2->url,"://"); /* Find the first part*/
	  if(ptr != NULL) {
	      ptr += 3; /* Skip over the host:port part now */
	      ptr2 = strchr(ptr,'/');
	  }
	  if(ptr2 != NULL) { /* Seems to contain a valid URL */
	    // Look for the field name
	    if((ptr3 = strchr(ptr2,'&')) != NULL) {
		*ptr3  = '\0'; // replace with null
		ptr3++;
	        gd.req_field_name = ptr3;

		// Replace the Caret: ^ with spaces for field names
		while((ptr3 = strchr(gd.req_field_name,'^')) != NULL) {
		   *ptr3 = ' ';
		}
                gd.request_field_index =  -1;
	    } 
	    for(i=0; i < gd.num_top_dirs; i++) {
		sprintf(gd.top_dir_url[i],"%s%s",
		       gd.top_dir[i], ptr2);
	    }
	  } else {
	    for(i=0; i < gd.num_top_dirs; i++) {
		strcpy(gd.top_dir_url[i],gd.top_dir[i]);
	    }
	  }


          if (!gd.daemonize_flag) {
              printf("URL: %s\n",com2->url);
              printf("Field name: %s\n",gd.req_field_name);
          }
      }
  } else {
      gd.protocol_version = 0;
      for(i=0; i < gd.num_top_dirs; i++) {
	strcpy(gd.top_dir_url[i],gd.top_dir[i]);
      }
      gd.request_field_index = com->data_field;

      // Convert to float for internal use
      memcpy(&comi,com,sizeof(cdata_ieee_comm_t));
      comi.lat_origin = (double)com->lat_origin / com->divisor;
      comi.lon_origin = (double)com->lon_origin / com->divisor;
      comi.ht_origin = (double)com->ht_origin / com->divisor;

      comi.min_x = (double)com->min_x / com->divisor;
      comi.max_x = (double)com->max_x / com->divisor;
      comi.min_y = (double)com->min_y / com->divisor;
      comi.max_y = (double)com->max_y / com->divisor;
      comi.min_z = (double)com->min_z / com->divisor;
      comi.max_z = (double)com->max_z / com->divisor;
  }

  /* Choose and open a data file, find the proper field and copy its grid info. 
   * This must be done
   * so we can get and use the projection and the projection origin
   * in the data file.
   */
  gd.current_ieee_reply.status = choose_and_open_file(&comi,com2);
  gd.last_time_cent = time_cent;


  if (!gd.daemonize_flag) {
    if (gd.current_ieee_reply.status & NO_DATA_AVAILABLE) {
      printf("No data file found for requested data.\n");
    } else {
        printf("Requested MIN_X,MAX_X: %.2f,%.2f  MIN_Y, MAX_Y: %.2f,%.2f  MIN_Z, MAX_Z: %.2f, %.2f\n",
	     comi.min_x, comi.max_x,
	     comi.min_y, comi.max_y,
	     comi.min_z, comi.max_z);
        printf("Requested origin_lat, origin_lon: %.4f, %.4f\n",
	     comi.lat_origin, comi.lon_origin);

    }
  }

  // Bail out
  if (gd.current_ieee_reply.status & NO_DATA_AVAILABLE) {
      gd.current_ieee_reply.n_points = 0;
      gd.current_ieee_reply.data_length = 0;
      send_reply(id, &comi,  &gd.current_ieee_reply,
		 &gd.current_ieee_info, data_ptr);
      return;
  }

  /* Convert request coordinates to local domain if we were able
   * to find a matching data file */
    
   if (gd.cur_f_head->fld_hdr->proj_type == MDV_PROJ_LATLON) {
      gd.x_offset = 0.0;
      gd.y_offset = 0.0;
   } else {
       latlong_to_xy(gd.origin_lat, gd.origin_lon,
	    comi.lat_origin, comi.lon_origin,
	    &gd.x_offset,&gd.y_offset);
   }
    
   /* Put the request in the grid's coord system */
   comi.min_x -= gd.x_offset;
   comi.max_x -= gd.x_offset;
   comi.min_y += gd.y_offset;
   comi.max_y += gd.y_offset;

   if (!gd.daemonize_flag)
      printf("Remmpped: MIN_X,MAX_X %.2f,%.2f  MIN_Y, MAX_Y: %.2f,%.2f  MIN_Z, MAX_Z : %.2f, %.2f\n",
         comi.min_x, comi.max_x,
         comi.min_y, comi.max_y,
	 comi.min_z, comi.max_z);

  /* copy the header information to the info structure now that we
   * have calculated the offset information */
  copy_header_to_info(&gd.current_ieee_info);
  
  /* Set values in reply based on info in file  */
  initialize_reply(&gd.current_ieee_reply);

  /* Get these quanities from the file header */
  time_begin = gd.cur_m_head.time_begin;
  time_end = gd.cur_m_head.time_end;
  data_ptr = NULL;

  if (comi.primary_com & GET_DATA &&
      !(gd.current_ieee_reply.status & NO_DATA_AVAILABLE)) {
    /*
     * Retrieve data.  When we receive the GET_NEW command, the
     * client is polling for new data and the com->time_cent field
     * contains the latest data received by the client.  If this
     * is the centroid time for our latest data, there is no new
     * data for this client so send a NO_NEW_DATA reply.
     */
    if ((comi.primary_com & GET_NEW) &&
        (comi.time_cent == ((time_begin + time_end) / 2))) {
      gd.current_ieee_reply.status |= NO_NEW_DATA;
    } else {
      data_ptr = get_grid_data(&comi,com2);
    }
  }

  send_reply(id, &comi,  &gd.current_ieee_reply,
		 &gd.current_ieee_info, data_ptr);
     
  if (data_ptr != NULL)
    free(data_ptr);
}
