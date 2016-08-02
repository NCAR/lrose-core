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
 * CIDD_DATA.C : Routines that manipulate and retrieve radar data
 *
 * For the Cartesian Radar Display (CIDD)
 * Frank Hage   July 1991 NCAR, Research Applications Program
 *
 */

#if defined(IRIX5) || defined(SUNOS5) || defined(SUNOS5_INTEL)
#define FASYNC          0x1000  /* interrupt-driven I/O for sockets */
#endif
#include <fcntl.h>
#include <memory.h>
#include <errno.h>

#define    RD_DATA    1
#include "cidd.h"

#include <toolsa/utim.h>
#include <sys/resource.h>

extern int errno;
 
static unsigned char *Incoming_data_pointer = NULL;
/*  waiting for: 0 =  reply, 1 = info, 2 = heights/elevs  3 = data */
static int Read_in_prog = 0;

/* Uncomment out the following or define it on the compile line */
/* To get Cidd to wait for Prod_sel to indicate it has new data */
/* #define IMPLEMENT_DATA_CHECK 1 */

/**********************************************************************
 * GATHER_HWIN_DATA: Collect all the data necessary for horizontal 
 *    display windows 
 *
 */

gather_hwin_data(field,start_time,end_time) 
    int    field;
    long    start_time,end_time;
{
    int    i;
    static int    primary_data_time_set = 0;
    time_t    now;
    met_record_t *mr;       /* pointer to record for convienence */
    char label[64];

    static int wait_for_prod_sel = -1;

    if(wait_for_prod_sel < 0) {
	wait_for_prod_sel = XRSgetLong(gd.cidd_db, "cidd.wait_for_prod_sel",1);
    }

    now = time(0);
     
    /* Check to make sure we are not currently waiting on an I/O request */
    if(gd.io_info.outstanding_request > 0) {
        if(now > gd.io_info.expire_time) {
            close(gd.io_info.fd);
            gd.io_info.outstanding_request = 0;
	    gd.io_info.expire_time = now + gd.data_timeout_secs;

	    if(Incoming_data_pointer != NULL) {
		ufree(Incoming_data_pointer);
		Incoming_data_pointer = NULL;
	    }

            if(gd.debug1) fprintf(stderr,"Timeout for data access - giving up\n");
	    PMU_auto_register("Data Timeout");
            rd_h_msg("Data Request Timed Out!... Retrying... ",-1);
            return FAILURE;
        }
        return INCOMPLETE;
    }

    mr = gd.mrec[field];    /* get pointer to data record */
    if(mr->h_data_valid == 0) {
        if(gd.debug1) fprintf(stderr, "Requesting False Color Field %d data time %d %d\n", field,start_time,end_time);
        gd.data_status_changed = 0;
	primary_data_time_set = 0;
        if(request_horiz_data_plane(mr,start_time,end_time,field) < 0) {
	   return FAILURE;
	} else {
           return INCOMPLETE;
	}
    } else {
	if(! primary_data_time_set) { /* Avoid setting this each pass through */
	    CSPR_calc_and_set_time();
	    primary_data_time_set = 1;
	}
    }

  /* Request any needed data for gridded layers */
  for(i=0; i < NUM_GRID_LAYERS; i++) {
    if(gd.extras.overlay_field_on[i]) {
        mr = gd.mrec[gd.extras.overlay_field[i]];
        if(mr->h_data_valid == 0) {
            if(gd.debug1) fprintf(stderr, "Requesting Overlay Field %d data\n", gd.extras.overlay_field[i]);
            gd.data_status_changed = 0;
            request_horiz_data_plane(mr,start_time,end_time,mr->sub_field);
            return INCOMPLETE;
        }
    }
  }

  /* request any needed data for contours */
  for(i=0; i < NUM_CONT_LAYERS; i++) {
    if(gd.extras.cont[i].active) {
        mr = gd.mrec[gd.extras.cont[i].field];
        if(mr->h_data_valid == 0) {
            if(gd.debug1) fprintf(stderr,
	      "Requesting Contour Layer %d, field %d data\n",
	      i, gd.extras.cont[i].field);

            gd.data_status_changed = 0;
            request_horiz_data_plane(mr,start_time,end_time,mr->sub_field);
            return INCOMPLETE;
        }
    }
  }

    for(i=0; i < gd.extras.num_wind_sets; i++) {
        switch(gd.extras.wind_mode) {
            default:
            case WIND_MODE_ON:  /* gather data as usual */
            break;

            case WIND_MODE_LAST: /* Gather data for last frame only */
                if(gd.movie.cur_frame != gd.movie.end_frame) continue;
            break;

            case WIND_MODE_STILL: /* Gather data for the last frame only
                                   * if the movie loop is off
                                   */
                if(gd.movie.movie_on || gd.movie.cur_frame != gd.movie.end_frame) continue;
            break;
        }

        if(gd.extras.wind[i].active ) {
            mr = gd.extras.wind[i].wind_u;
            if(mr->h_data_valid == 0) {
                if(gd.debug1) fprintf(stderr, "Requesting Wind %d data - U\n", i);
                gd.data_status_changed = 0;
                request_horiz_data_plane(mr,start_time,end_time,mr->sub_field);
                return INCOMPLETE;
            }
    
            mr = gd.extras.wind[i].wind_v;
            if(mr->h_data_valid == 0) {
                if(gd.debug1) fprintf(stderr, "Requesting Wind %d data - V\n", i);
                gd.data_status_changed = 0;
                request_horiz_data_plane(mr,start_time,end_time,mr->sub_field);
                return INCOMPLETE;
            }
    
            mr = gd.extras.wind[i].wind_w;
            if(mr != NULL) {
                if(mr->h_data_valid == 0) {
                    if(gd.debug1) fprintf(stderr, "Requesting Wind %d  data - W\n", i);
                    gd.data_status_changed = 0;
                    request_horiz_data_plane(mr,start_time,end_time,mr->sub_field);
                    return INCOMPLETE;
                }
            }
        }
    }

  if(wait_for_prod_sel) {
    /* check if not in limited mode or The products are turned on */
    if((gd.limited_mode != 1)  || (gd.prod.products_on != 0)) {
      /*
       * Make sure we have the prod_sel data.
       */

      if (!CSPR_data_current()) {
        if(now > gd.io_info.expire_time) { 
          if (gd.debug1) fprintf(stderr, " SPR data not current - timeout\n");
	  return SUCCESS;
        }
        sprintf(label,"Waiting For Product Data - %d seconds",
	  gd.io_info.expire_time - now);
        rd_h_msg(label,-1);

        if (gd.debug1) fprintf(stderr, "gather_hwin_data: SPR data not current\n");
        return INCOMPLETE;
      }
    }
  }
    
    return SUCCESS;
}

/**********************************************************************
 * GATHER_VWIN_DATA: Collect all the data necessary for vertical 
 *    display windows 
 *
 */

gather_vwin_data(field,start_time,end_time) 
    int    field;
    long    start_time,end_time;
{
    int    i;
    met_record_t *mr;       /* pointer to record for convienence */
    time_t now;

    now = time(0);
    /* Check to make sure we are not currently waiting on an I/O request */
    if(gd.io_info.outstanding_request) {
        if(now > gd.io_info.expire_time) {
           close(gd.io_info.fd);
           gd.io_info.outstanding_request = 0;
	   gd.io_info.expire_time = now + gd.data_timeout_secs;
	   if(Incoming_data_pointer != NULL) { 
	       ufree(Incoming_data_pointer);
	       Incoming_data_pointer = NULL;

	    }
           if(gd.debug1) fprintf(stderr,"Timeout for data access - Giving up\n");
	   PMU_auto_register("Data Timeout");
           rd_h_msg("Data Timeout!",-1);
           fflush(stderr);
           return FAILURE;
        }
        return INCOMPLETE;
    }
     
    mr = gd.mrec[field];    /* get pointer to data record */
    if(mr->v_data_valid == 0) {
        if(gd.debug1) fprintf(stderr, "Requesting False Color Field %d data\n", field);
        gd.data_status_changed = 0;
        if(request_vert_data_plane(mr,start_time,end_time,field) < 0) {
            return FAILURE;
	} else {
            return INCOMPLETE;
	}
    }

    for(i=0; i < NUM_CONT_LAYERS ; i++) {
      if(gd.extras.cont[i].active) {
        mr = gd.mrec[gd.extras.cont[i].field];
        if(mr->v_data_valid == 0) {
            if(gd.debug1) fprintf(stderr,
			   "Requesting VERT Contour Field %d data\n",
			   gd.extras.cont[i].field);
             
            gd.data_status_changed = 0;
            request_vert_data_plane(mr,start_time,end_time,field);
            return INCOMPLETE;
        }
      }
    }

    for(i=0; i < gd.extras.num_wind_sets; i++) {
        if(gd.extras.wind[i].active) {
            mr = gd.extras.wind[i].wind_u;
            if(mr->v_data_valid == 0) {
                if(gd.debug1) fprintf(stderr, "Requesting Wind %d - %s data - U\n", i,mr->field_name);
                gd.data_status_changed = 0;
                request_vert_data_plane(mr,start_time,end_time,field);
                return INCOMPLETE;
            }
    
            mr = gd.extras.wind[i].wind_v;
            if(mr->v_data_valid == 0) {
                if(gd.debug1) fprintf(stderr, "Requesting Wind %d - %s  data - V\n", i,mr->field_name);
                gd.data_status_changed = 0;
                request_vert_data_plane(mr,start_time,end_time,field);
                return INCOMPLETE;
            }
    
            mr = gd.extras.wind[i].wind_w;
            if(mr != NULL) {
                if(mr->v_data_valid == 0) {

                    if(gd.debug1) fprintf(stderr, "Requesting Wind %d - %s data - W\n", i,mr->field_name);

                    gd.data_status_changed = 0;
                    request_vert_data_plane(mr,start_time,end_time,field);
                    return INCOMPLETE;
                }
            }
        }
    }

    return SUCCESS;
}

/**********************************************************************
 * REQUEST_HORIZ_DATA_PLANE: Querry the server for a data plane
 *
 */

int
request_horiz_data_plane(mr,start_time,end_time,field)
    met_record_t *mr;
    long    start_time,end_time;
    int    field;
{
    time_t tm;
    cdata_comm_t    com;
    int  sockfd;
    char label[128];
    double divisor;

    SERVMAP_request_t  request;
    SERVMAP_info_t     *info;
    int                n_servers;
    int                i;
    int                socket_connected = FALSE;
    

    switch(gd.projection_mode) {
       case CARTESIAN_PROJ: divisor = CDATA_DIVISOR; break;
       /* case LAT_LON_PROJ: divisor = CDATA_LATLON_DIVISOR; break; */
       case LAT_LON_PROJ: divisor = 1000000; break;
    }


    if(gd.debug1)
        fprintf(stderr, "Get Horiz Plane - field : %d   %d\n", field,mr->h_date.unix_time);

    /* Build the command */
    com.primary_com = GET_DATA;
    com.primary_com |= GET_INFO;    /* Must get grid info always */
    com.primary_com |= GET_PLANE_HEIGHTS;    /*ASK FOR PLANE HEIGHTS */
    com.divisor = divisor;
    com.lat_origin = gd.h_win.origin_lat * divisor;
    com.lon_origin = gd.h_win.origin_lon * divisor;
    com.ht_origin = 0;

    if(mr->composite_mode == FALSE) {
        com.second_com = GET_XY_PLANE;
    } else {
        com.second_com = GET_MAX_XY_PLANE;
    }
     
    /* allow for +/-  1 time unit slop */
    com.time_min = start_time - ( 60.0 * mr->time_allowance);
    com.time_max = end_time + (60.0 *  mr->time_allowance);
    if(gd.gather_data_mode == 0) {
        com.time_cent = (com.time_min + com.time_max) / 2;
    } else {
	com.time_cent = end_time;
    }

	/* Off set the request time */
	com.time_min += (int) mr->time_offset * 60.0;
	com.time_max += (int) mr->time_offset * 60.0;
	com.time_cent += (int) mr->time_offset * 60.0;

    tm = time(0);    /* get current time */
    
    /* In realtime mode, on the last frame, keep the data as up to date as possible */
    if(gd.movie.mode == MOVIE_MR && gd.movie.cur_frame == gd.movie.num_frames -1) { 
	if(gd.movie.forecast_interval <= 0.0) {
	    if(gd.debug1) fprintf(stderr,"Asking for Most Recent data\n");
            com.primary_com |= GET_MOST_RECENT; 
	}
        if(mr->background_render == 1) {
	    if(gd.debug1) fprintf(stderr,"Asking for NEW data\n");
            com.time_cent = mr->h_date.unix_time;    /* Put existing data time in this field */
	    com.primary_com |= GET_NEW; /*  */
	}
        gd.io_info.mode = LIVE_DATA;
    } else {  /* Generic Archival Mode */
	if(gd.debug1) fprintf(stderr,"Asking for Static data\n");
        gd.io_info.mode = STATIC_DATA;
    }

    tm += (gd.movie.forecast_interval * 60);

    if (mr->data_format == PPI_DATA_FORMAT) {

      if(gd.always_get_full_domain) {
        com.min_x = gd.h_win.min_r * divisor;
        com.max_x = gd.h_win.max_r * divisor;
        com.min_y = gd.h_win.min_deg * divisor;
        com.max_y = gd.h_win.max_deg * divisor;
      } else {
        com.min_x = gd.h_win.cmin_r * divisor;
        com.max_x = gd.h_win.cmax_r * divisor;
        com.min_y = gd.h_win.cmin_deg * divisor;
        com.max_y = gd.h_win.cmax_deg * divisor;
      }
      com.min_z = (gd.h_win.cmin_ht - 20.0) * divisor;
      com.max_z = (gd.h_win.cmax_ht + 20.0) * divisor;

    } else {  /* NOT A PPI_DATA_FORMAT REQUEST */

      if(gd.always_get_full_domain) {
        com.min_x = gd.h_win.min_x * divisor;
        com.max_x = gd.h_win.max_x * divisor;
        com.min_y = gd.h_win.min_y * divisor;
        com.max_y = gd.h_win.max_y * divisor;
      } else {
        com.min_x = gd.h_win.cmin_x * divisor;
        com.max_x = gd.h_win.cmax_x * divisor;
        com.min_y = gd.h_win.cmin_y * divisor;
        com.max_y = gd.h_win.cmax_y * divisor;
      }

      if(mr->composite_mode == FALSE) {
	/* widen limit requests to get closest plane whenever possible */
        com.min_z = (gd.h_win.cmin_ht - 20.0) * divisor;
        com.max_z = (gd.h_win.cmax_ht + 20.0) * divisor;
      } else {
        com.min_z = 0;
        com.max_z = 40.0 * divisor;
      }
    }

    com.data_type = CDATA_CHAR;  /* CIDD always asks for 8 bit data */
    com.add_data_len = 0;
    

    /* GET data */
    com.data_field = mr->sub_field;
     
     /*
     * Querry the Server mapper and then send a Send data request to the first server that answers 
     */

    if (mr->use_servmap && mr->data_server_type[0] != '\0' ) {
        if (gd.debug1) { fprintf(stderr,
                    "Sending server request to server mapper: %s %s %s\n",
                    mr->data_server_type, mr->data_server_subtype,
                    mr->data_server_instance);
    
            fflush(stderr);
        }
        /*
         * load up request structure
         */

        memset((char *)&request, 0, sizeof(request));
    
        STRcopy(request.server_type, mr->data_server_type,SERVMAP_NAME_MAX);
        STRcopy(request.server_subtype, mr->data_server_subtype,SERVMAP_NAME_MAX);
        STRcopy(request.instance, mr->data_server_instance,SERVMAP_INSTANCE_MAX);
    
        if (gd.io_info.mode == LIVE_DATA) request.want_realtime = TRUE;
        else request.want_realtime = FALSE;
	 
        request.time = 0;
    
        /*
         * get the server information from the server mapper
         */

        if (SMU_requestInfo(&request, &n_servers, &info, gd.servmap_host1, gd.servmap_host2) == 1) {
            if (gd.debug1) fprintf(stderr, "%d servers returned by server mapper\n",n_servers);
            
            for (i = 0; i < n_servers && socket_connected == FALSE; i++) {
                if(info[i].start_time > com.time_max || (info[i].end_time + 3600 < com.time_min)) continue;

                if (gd.debug1) fprintf(stderr, "Sending data command to server: %s %d\n",
                            info[i].host, info[i].port);
                
                if ((sockfd = send_server_command(info[i].host, info[i].port, &com)) > 0) {
                    gd.io_info.fd = sockfd;
                    socket_connected = TRUE;
                } /* endelse - socket connection made */
            
            } /* endfor - i */
        
        } /* endif - got server information */
        else if (gd.debug1) fprintf(stderr, "No response to server mapper request\n");
        
    } /* endif - check for server mapper servers */

    /* Use the Default host and port as a last resort - or if not using servmapper */
    if(!socket_connected) {
      if (gd.debug1) {
        fprintf(stderr, "Sending data request to default server - %s %d\n", mr->data_hostname, mr->port);
        fflush(stderr);
      }
    
     
      /* send the command to the default server and host */
      if ((sockfd = send_server_command(mr->data_hostname, mr->port, &com)) > 0) {
        gd.io_info.fd = sockfd;
        socket_connected = TRUE;
	PMU_auto_register("Requesting Data (OK)");
        rd_h_msg("Requesting Data",-1);
      }
  } 


    if (!socket_connected) {
        sprintf(label, " NO DATA SERVICE: host %s, port %d \n", mr->data_hostname, mr->port);
	rd_h_msg(label,-1);
        fprintf(stderr, "Unable to connect to data server: host %s, port %d \n", mr->data_hostname, mr->port);

        if (mr->use_servmap && mr->data_server_type[0] != '\0' ) {
            fprintf(stderr, "No servers (%s, %s, %s) with valid data on the server mapper\n",
                    mr->data_server_type, mr->data_server_subtype, mr->data_server_instance);
	}
        mr->h_data_valid = 1;
        gd.io_info.mr = mr;
        return(-1);
	 
    } else {
        PMU_auto_register("Requesting Data (OK)");

        gd.io_info.outstanding_request = 1;
        gd.io_info.expire_time = tm + gd.data_timeout_secs;
        gd.io_info.hv_flag = 1;    /* a horizontal data access */
        gd.io_info.field = field;
        gd.io_info.mr = mr;
        return(0);
    }

 
}

 
/**********************************************************************
 * REQUEST_VERT_DATA_PLANE: Query the server for a data plane
 *
 */

int
request_vert_data_plane(mr,start_time,end_time,field)
    met_record_t *mr;
    long    start_time,end_time;
    int    field;
{
    time_t    tm;
    cdata_comm_t    com;
    int     sockfd;
    char    label[128];
    
    SERVMAP_request_t  request;
    SERVMAP_info_t     *info;
    int                n_servers;
    int                i;
    int                socket_connected = FALSE;
    double divisor;

    if (gd.debug1)
        fprintf(stderr, "Get Vert Plane : Field: %d, %d\n", field, mr->v_date.unix_time);

    switch(gd.projection_mode) {
       case CARTESIAN_PROJ: divisor = CDATA_DIVISOR; break;
       /* case LAT_LON_PROJ: divisor = CDATA_LATLON_DIVISOR; break; */
       case LAT_LON_PROJ: divisor = 1000000; break;
    }

    /* Build the command */
    com.primary_com = GET_DATA;
    com.primary_com |= GET_INFO;            /* Must ask for info always */
    com.primary_com |= GET_PLANE_HEIGHTS;    /* ASK FOR PLANE HEIGHTS */
    com.second_com = GET_V_PLANE;
    com.divisor = divisor;
    com.lat_origin = gd.h_win.origin_lat * divisor;
    com.lon_origin = gd.h_win.origin_lon * divisor;
    com.ht_origin = 0;

    com.time_min = start_time - ( 60.0 * mr->time_allowance);
    com.time_max = end_time + (60.0 *  mr->time_allowance);
    if(gd.gather_data_mode == 0) {
        com.time_cent = (com.time_min + com.time_max) / 2;
    } else {
	com.time_cent = end_time;
    }

     
    tm = time(0);    /* get current time */
    tm += (gd.movie.forecast_interval * 60);
    
    /* In realtime mode, on the last frame, keep the data as up to date as possible */
    if(gd.movie.mode == MOVIE_MR && gd.movie.cur_frame == gd.movie.num_frames -1) { 
	if(gd.movie.forecast_interval <= 0.0) {
	    if(gd.debug1) fprintf(stderr,"Asking for Vertical Section - Most Recent data\n");
            com.primary_com |= GET_MOST_RECENT; 
	}
        if(mr->background_render == 1) {
	    if(gd.debug1) fprintf(stderr,"Asking for Vertical Section - NEW data\n");
	    com.primary_com |= GET_NEW; /*  */
            com.time_cent = mr->v_date.unix_time;    /* Put existing data time in this field */
	}
        gd.io_info.mode = LIVE_DATA;
    } else {  /* Generic Archival Mode */
	if(gd.debug1) fprintf(stderr,"Asking for Vertical Section - Archival data\n");
        gd.io_info.mode = STATIC_DATA;
    }

    com.min_x = gd.v_win.cmin_x * divisor;
    com.max_x = gd.v_win.cmax_x * divisor;
    com.min_y = gd.v_win.cmin_y * divisor;
    com.max_y = gd.v_win.cmax_y * divisor;
    com.min_z = gd.v_win.cmin_ht * divisor;
    com.max_z = gd.v_win.cmax_ht * divisor;

    com.data_type = CDATA_CHAR;
    com.add_data_len = 0;
    
    /* GET data */
    com.data_field = mr->sub_field;


    /*
     * Querry the server mapper
     * Send data request to the first appropriate servers 
     *
     */

    if (mr->use_servmap && mr->data_server_type[0] != '\0' ) {
    
        /*
         * load up request structure
         */

        memset((char *)&request, 0, sizeof(request));
    
        STRcopy(request.server_type, mr->data_server_type,SERVMAP_NAME_MAX);
        STRcopy(request.server_subtype, mr->data_server_subtype,SERVMAP_NAME_MAX);
        STRcopy(request.instance, mr->data_server_instance,SERVMAP_INSTANCE_MAX);
    
        if (gd.io_info.mode == LIVE_DATA) request.want_realtime = TRUE;
        else request.want_realtime = FALSE;

        request.time = tm;
        /*
         * get the server information from the server mapper
         */

        if (SMU_requestInfo(&request, &n_servers, &info, gd.servmap_host1, gd.servmap_host2)) {
            for (i = 0; i < n_servers && socket_connected == FALSE ; i++) {
                if(info[i].start_time > com.time_max || (info[i].end_time + 3600 < com.time_min)) continue;

                if ((sockfd = send_server_command(info[i].host, info[i].port, &com)) > 0) {
                    gd.io_info.fd = sockfd;
                    socket_connected = TRUE;
	            PMU_auto_register("Recieving Data (OK)");
                } /* endelse - socket connection made */
            
            } /* endfor - i */
        
        } /* endif - got server information */
    } /* endif - check for server mapper servers */
    
    /* Use the Default host and port as a last resort - or if not using servmapper */
    if(!socket_connected) {
      if ((sockfd = send_server_command(mr->data_hostname, mr->port, &com)) > 0) {
        gd.io_info.fd = sockfd;
        socket_connected = TRUE;
	PMU_auto_register("Recieving Data (OK)");
	rd_h_msg("Requesting Data",-1);
      }
    }

    if (!socket_connected) {
        sprintf(label, "NO DATA SERVICE: host %s, port %d\n", mr->data_hostname, mr->port);
        fprintf(stderr, "Unable to connect to data server: host %s, port %d \n", mr->data_hostname, mr->port);
        if (mr->use_servmap && mr->data_server_type[0] != '\0' ) {
            fprintf(stderr, "No servers (%s, %s, %s) with valid data on the server mapper\n",
                    mr->data_server_type, mr->data_server_subtype, mr->data_server_instance);
	}
        mr->v_data_valid = 1;
        gd.io_info.mr = mr;
        return(-1);
    } else { 
        PMU_auto_register("Requesting Data (OK)");
    
        gd.io_info.outstanding_request = 1;
        gd.io_info.expire_time = tm + gd.data_timeout_secs;
        gd.io_info.hv_flag = 2;    /* a vertical data access */
        gd.io_info.field = field;
        gd.io_info.mr = mr;
        return(0);
    }

}

/**********************************************************************
 * SEND_SERVER_COMMAND: Send a command to a server requesting data.
 *
 * Returns -1 if unsuccessful, the socket descriptor for the socket if
 * successful.
 *
 */

int send_server_command(hostname, port, com)

char          *hostname;
int           port;
cdata_comm_t  *com;

{
    int         tries;
    int         sockfd;
    int         flag;
    char        label[128];
    

    /*
     * try to open a socket with the server (maximum of 3 tries)
     */

    tries = 0;
    do {
        sockfd = SKU_open_client(hostname, port);
        if (sockfd == -1) {
            sprintf(label, "Unresponsive Server host: %s - port: %d\n", hostname, port);
	    rd_h_msg(label,-2);
            if(gd.debug1) fprintf(stderr, "Unresponsive Server  host: %s -port %d\n", hostname, port);
            return(-1);
        } /* endif */
        tries++;
    } while (sockfd < 0 && tries < 3);


    if (sockfd < 0)   /* was not able to connect to the server */
        return(-1);
    
    sprintf(label, "Requesting data from: %s - port %d\n", hostname, port);
    rd_h_msg(label,-1);

    /*
     * set the descriptor status flag
     */
    if (fcntl(sockfd, F_SETFL, FASYNC) < 0) fprintf(stderr, "fcntl fails\n");

    /*
     * try to send the command to the server
     */

    XDRU_fromhl((u_long) com,(u_long)  sizeof(cdata_comm_t));
    
    tries = 0;
    flag = 1;
    do { 
        if (SKU_write(sockfd, com, sizeof(cdata_comm_t), 10) != sizeof(cdata_comm_t)) {
            tries++;
            close(sockfd);
        } else {
            flag = 0;
        } 
    } while (flag && (tries < 3));

    if (flag) {
        sprintf(label, "Data request failed to %s - %d\n", hostname, port);
	rd_h_msg(label,-2);
        if(gd.debug1) fprintf(stderr, "Data request failed\n");
        return(-1);
    } /* endif */
 
    Read_in_prog = 0; /* Set state to waiting for reply */
    return(sockfd);
}

 
/******************************************************************************
 * COPY_INFO_TO_DATA_REC: Copy Useful information from a server info struct
 *        into our local data record struct
 */

copy_info_to_data_rec(info,rec)
    cdata_info_t    *info;
    met_record_t    *rec;
{
    int    i;
    double    div;
    double    hires_div;


    XDRU_tohl(info, NUM_INFO_LONGS * sizeof(long));


    if(info->divisor == 0 || info->ny > MAX_ROWS || info->nx > MAX_COLS || info->nz > MAX_SECTS) return FAILURE;
    if(info->divisor != 0) div = info->divisor;  /*  */

    if(info->highres_divisor == CDATA_HIGHRES_DIVISOR) {
	hires_div = info->highres_divisor;
    } else {
	hires_div = div;
    }

    rec->cols = info->nx;
    rec->rows = info->ny;
    rec->sects = info->nz;

    rec->dx = (double) info->dx / hires_div;
    rec->dy = (double) info->dy / hires_div;
    rec->dz = (double) info->dz / div;

    rec->min_x = (double) info->min_x / hires_div;
    rec->max_x = (double) info->max_x / hires_div;

    rec->min_y = (double) info->min_y / hires_div;
    rec->max_y = (double) info->max_y / hires_div;

    rec->vdx = (double) info->dx / div;
    rec->vdy = (double) info->dy / div;
    rec->vdz = (double) info->dz / div;

    rec->vmin_x = (double) info->min_x / div;
    rec->vmax_x = (double) info->max_x / div;

    rec->vmin_y = (double) info->min_y / div;
    rec->vmax_y = (double) info->max_y / div;

    if(rec->dz != 0.0) {
        for(i=0; i < rec->sects; i++) {
            rec->vert[i].min = info->min_z / div + ((double)i * rec->dz);
            rec->vert[i].max = info->min_z/ div + ((double)(i+1) * rec->dz);
            rec->vert[i].cent = (rec->vert[i].min + rec->vert[i].max) / 2.0;
        }
    }
    rec->origin_lat = (double) info->lat_origin / div;
    rec->origin_lon = (double) info->lon_origin / div;

    rec->order = info->order;

    STRcopy(rec->units_label_cols,info->units_label_x,LABEL_LENGTH);
    if (gd.mrec[gd.h_win.field]->data_format == PPI_DATA_FORMAT)
        STRcopy(rec->units_label_rows,info->units_label_x,LABEL_LENGTH);
    else
        STRcopy(rec->units_label_rows,info->units_label_y,LABEL_LENGTH);
    STRcopy(rec->units_label_sects,info->units_label_z,LABEL_LENGTH);
    /* STRcopy(rec->field_units,info->field_units,LABEL_LENGTH);  /*  */
    STRcopy(rec->field_label,info->field_name,NAME_LENGTH);    /*  */
    STRcopy(rec->source_label,info->source_name,NAME_LENGTH); /*  */
     
    if(gd.debug1) {
    	printf("Info.divisor %d\n",info->divisor);
    	printf("Info.nx %d\n",info->nx);
    	printf("Info.ny %d\n",info->ny);
    	printf("Info.nz %d\n",info->nz);
    	printf("Info.min_x %d\n",info->min_x);
    	printf("Info.min_y %d\n",info->min_y);
    	printf("Info.max_x %d\n",info->max_x);
    	printf("Info.max_y %d\n",info->max_y);
    	printf("Info.dx %d\n",info->dx);
    	printf("Info.dy %d\n",info->dy);
    	printf("Info.dz %d\n",info->dz);
    	printf("Nx, Ny: %d, %d \n",rec->h_nx, rec->h_ny);
    } 

    return SUCCESS;
}

/**********************************************************************
 * CHECK_FOR_IO: This routine handles the receipt of incoming data
 *
 */

void
check_for_io()
{
    int    i;
    int     set_size;
    int     bytes_read;
    long    *lptr = NULL;
    long    *vpts = NULL;
    time_t now;
    unsigned int size;
    UTIMstruct    temp_utime;
    char    label[1024];
    fd_set  readfds;
 
    static cdata_reply_t   reply;
    static cdata_info_t    info;

    static unsigned char    *ptr = NULL;
    static unsigned char    *tmp_ptr = NULL;
    static int target = 0;
    static int total_wanted = 0;
    static int total_read = 0;
    static struct timeval timeout = { 0, 0 };

    if(gd.debug) umalloc_verify();

    set_size = gd.io_info.fd + 1;
    FD_ZERO(&readfds); 
    FD_SET(gd.io_info.fd,&readfds);
    if ((i = select(set_size, &readfds, (fd_set *)NULL, (fd_set *)NULL, &timeout)) <= 0) {
        if(gd.debug1 > 1) fprintf(stderr,"Read not ready/ Select failed: Status %d\n",i);
        sprintf(label,"Waiting for server to reply... : %d secs before timeout",(gd.io_info.expire_time - time(0)));
        rd_h_msg(label,-1);
        return;
    }

    if(!FD_ISSET(gd.io_info.fd,&readfds)) return;
    switch(Read_in_prog) {
      case 0: break;  
      case 1:
	goto read_info;
      break;

      case 2:
	goto read_heights;
      break;

      case 3:
	goto read_data;
      break;
    }

    if(gd.debug) umalloc_verify();
    
    bytes_read = SKU_read_timed(gd.io_info.fd, &reply, sizeof(reply),10,100);
    if (bytes_read != sizeof(reply)) {  /* Bail out on this server */
        fprintf(stderr, "Read Reply failed: %d bytes in reply (should be %d)\n", bytes_read, sizeof(reply));
        goto done_with_server;
    }  
    if(gd.debug1) printf("Read reply from server: %d bytes\n",bytes_read);
    Read_in_prog = 1;  /* now ready for info */

    XDRU_tohl(&reply, sizeof(cdata_reply_t));

    if(reply.nx > MAX_COLS || (reply.x2 - reply.x1 +1) > MAX_COLS) {
       fprintf(stderr, "FATAL ERROR: Too many X Points for display!\n");
       exit(-2);
    }

    if(reply.ny > MAX_ROWS || (reply.y2 - reply.y1 +1) > MAX_ROWS) {
        fprintf(stderr,"FATAL ERROR: Too many Y Points for display!\n");
        exit(-2);
    }

    if(gd.debug) umalloc_verify();
    
    if(gd.debug1) printf("Arriving %s data from has date:  %d, len %d\n",
                gd.io_info.mr->field_name,reply.time_cent,reply.n_points);

    switch(gd.io_info.hv_flag) {
        case 1: /* horiz */
            UTIMunix_to_date(reply.time_cent,&gd.io_info.mr->h_date);
            break;
        case 2: /* vert */
            UTIMunix_to_date(reply.time_cent,& gd.io_info.mr->v_date);
            break;
    }

    if(gd.debug) umalloc_verify();
    
    if(reply.status & DATA_FOLLOWS) {
        gd.io_info.mr->missing_val = reply.bad_data_val;
        /* Fill in info about current data grid */
        switch(gd.io_info.hv_flag) {
        case 1: /* horizontal */
            gd.io_info.mr->x1 = reply.x1;
            gd.io_info.mr->x2 = reply.x2;
            gd.io_info.mr->y1 = reply.y1;
            gd.io_info.mr->y2 = reply.y2;
            gd.io_info.mr->plane = reply.z2;

            gd.io_info.mr->h_nx = reply.nx;
            gd.io_info.mr->h_ny = reply.ny;
            break;

        case 2: /* vertical */
            gd.io_info.mr->vx1 = reply.x1;
            gd.io_info.mr->vx2 = reply.x2;
            gd.io_info.mr->vy1 = reply.y1;
            gd.io_info.mr->vy2 = reply.y2;
            gd.io_info.mr->z1 = reply.z1;
            gd.io_info.mr->z2 = reply.z2;

            gd.io_info.mr->v_nx = reply.nx;
            gd.io_info.mr->v_ny = reply.ny;
            break;
        }
    }

read_info:

    if(gd.debug) umalloc_verify();
    
    if(reply.status & INFO_FOLLOWS) {
        bytes_read = SKU_read_timed(gd.io_info.fd, &info,sizeof(info), 10,500);
        if(bytes_read != sizeof(info) ) {  /* Bail out on this server */
            fprintf(stderr,"Warning: Couldn't read Data Grid Info. Wanted %d, got %d\n",sizeof(info),bytes_read);
            goto done_with_server;
        } else {
            if(gd.debug1) printf("Read grid info from server: %d bytes\n",bytes_read);
            switch(gd.io_info.hv_flag) {
            case 1:    /* horizontal */
                copy_info_to_data_rec(&info,gd.io_info.mr);
                break;
            
            case 2:    /* vertical */
                copy_info_to_data_rec(&info, gd.io_info.mr);
                break;
            }  /* endswitch */
        
        }
    }
    Read_in_prog = 2;  /* now ready for heights/elev */

read_heights:

    if(gd.debug) umalloc_verify();
    
    if(reply.status & PLANE_HEIGHTS_FOLLOW) {
        vpts = (long *) ucalloc(gd.io_info.mr->sects * 3,sizeof(long));
        bytes_read = SKU_read_timed(gd.io_info.fd, vpts, (gd.io_info.mr->sects * 3 * sizeof(long)),5,500);
        if(bytes_read != (gd.io_info.mr->sects * 3 * sizeof(long))) {  /* Bail out on this server */

            fprintf(stderr,"Warning: Error reading Plane Heights Info, field %s, Expected %d, got %d\n",
                gd.io_info.mr->field_name,(gd.io_info.mr->sects * 3 * sizeof(long)),bytes_read);

            ufree(vpts);
	    vpts = NULL;
            goto done_with_server;
        }
        XDRU_tohl(vpts, gd.io_info.mr->sects * 3 * sizeof(long));

        lptr = vpts;
        for(i=0; i < gd.io_info.mr->sects && i < MAX_SECTS; i++) {
            gd.io_info.mr->vert[i].min = *lptr++ / (double) reply.divisor;
            gd.io_info.mr->vert[i].cent = *lptr++ / (double) reply.divisor;
            gd.io_info.mr->vert[i].max = *lptr++ / (double) reply.divisor;
            if(gd.debug1 > 1) printf("Plane %d : %g,%g,%g\n", i, gd.io_info.mr->vert[i].min,
                           gd.io_info.mr->vert[i].cent,
                           gd.io_info.mr->vert[i].max);
        }

        ufree(vpts);
	vpts = NULL;
        if(gd.debug1) printf("Read %d plane heights from server: %d bytes\n",gd.io_info.mr->sects,bytes_read);
    } 

    Incoming_data_pointer = NULL;
    Read_in_prog = 3;  /* now ready for data */
	total_read = 0;

    if(gd.debug) umalloc_verify();
    
    if(reply.status & DATA_FOLLOWS) {

	/* See if the data's scaling has changed */
        switch(gd.io_info.hv_flag) {
        case 1: /* horizontal */ 
            gd.io_info.mr->h_scale = (double) reply.scale / reply.divisor;
            gd.io_info.mr->h_bias = (double) reply.bias / reply.divisor;

            if((gd.io_info.mr->h_scale != gd.io_info.mr->h_last_scale) || 
               ( gd.io_info.mr->h_bias != gd.io_info.mr->h_last_bias))
            {
		/* REmap the data values onto the colorscale */
                setup_color_mapping(gd.io_info.mr, &(gd.io_info.mr->h_vcm), gd.io_info.mr->h_scale,
                                    gd.io_info.mr->h_bias);
                gd.io_info.mr->h_last_scale = gd.io_info.mr->h_scale;
                gd.io_info.mr->h_last_bias = gd.io_info.mr->h_bias;
            }
            break;
 
        case 2: /* vertical */ 
            gd.io_info.mr->v_scale = (double) reply.scale / reply.divisor;
            gd.io_info.mr->v_bias = (double) reply.bias / reply.divisor;

            if((gd.io_info.mr->v_scale != gd.io_info.mr->v_last_scale) || 
               (gd.io_info.mr->v_bias != gd.io_info.mr->v_last_bias))
            {
                 /* Remap the data values onto the colorscale */
                setup_color_mapping(gd.io_info.mr, &(gd.io_info.mr->v_vcm),
                                    gd.io_info.mr->v_scale,
                                    gd.io_info.mr->v_bias);
                gd.io_info.mr->v_last_scale = gd.io_info.mr->v_scale;
                gd.io_info.mr->v_last_bias = gd.io_info.mr->v_bias;
            }
            break;
        }

	if(gd.debug) umalloc_verify();
	
	/* Allocate some new space for data */
        if(reply.n_points) Incoming_data_pointer = (unsigned char *) ucalloc(1,reply.n_points);
	if(gd.debug) umalloc_verify();
        if(gd.debug1 > 1) fprintf(stderr,"ALLOC: %d bytes @ %x for data \n",
	    reply.n_points, Incoming_data_pointer);

	if(gd.debug) umalloc_verify();
        target = reply.n_points;
	total_wanted = target;
        tmp_ptr = Incoming_data_pointer;

read_data:
        now = time(0);

	if(gd.debug) umalloc_verify();
	
    do {
        bytes_read = SKU_read_timed(gd.io_info.fd, tmp_ptr, (target < 2048? target: 2048), 1,50);

        target -= bytes_read;
        tmp_ptr += bytes_read;
        total_read += bytes_read;

        if(gd.debug1 > 1) fprintf(stderr,"Read %d data bytes, %d left of %d\n",
	    bytes_read,target,total_wanted);
	  
     } while (target > 0 && time(0) == now); /* keep trying for a second */

	if(gd.debug) umalloc_verify();
	
        if(total_wanted > 0) {
	    sprintf(label,"Reading Incoming data... : %d%% complete",
		(int)(100 * total_read / total_wanted));
	    rd_h_msg(label,-1);
	}
        if(target > 0 ) return;

         Read_in_prog = 0;

        if((ptr = RLDecode8(Incoming_data_pointer,&size)) == (u_char*) NULL) {
            ptr = Incoming_data_pointer; /* data is not compressed, set pointer to raw data */
        } else {
            reply.n_points = size;
            if(gd.debug1 > 1) 
		fprintf(stderr,"FREE: Compressed array Mem: %x New size: %d\n",Incoming_data_pointer ,size );
            ufree(Incoming_data_pointer); /* free up the area for compressed data */
	    Incoming_data_pointer = NULL;
        }
 
        switch(gd.io_info.hv_flag) {
        case 1: /* horizontal */
            if(gd.data_status_changed == 0) {
                gd.io_info.mr->h_data_valid = 1;
                if(gd.io_info.mr->h_data != NULL)  ufree(gd.io_info.mr->h_data);

                if(gd.debug1 > 1)
                    fprintf(stderr,"FREE: &: %x   Mem: %x\n",
		    &(gd.io_info.mr->h_data), ( gd.io_info.mr->h_data));
                 
                gd.io_info.mr->h_data = (unsigned char *) ptr;
                if(gd.debug1 > 1) fprintf(stderr,"ASSIGN:  h_data (%x) to %x\n",
		    &(gd.io_info.mr->h_data), ptr);

                gd.h_win.redraw[gd.io_info.field] = 1;
                if(gd.io_info.field == gd.h_win.field) {
                    gd.movie.frame[gd.movie.cur_index].redraw_horiz = 1;
                }
            }
        break;

        case 2: /* vertical */
            if(gd.data_status_changed == 0) {
                gd.io_info.mr->v_data_valid = 1;
                if(gd.io_info.mr->v_data != NULL) ufree(gd.io_info.mr->v_data);

                gd.io_info.mr->v_data = (unsigned char *) ptr;
                if(gd.debug1 > 1) {
			printf("Vert Plane: \n");
			{
			double val;
			tmp_ptr = gd.io_info.mr->v_data;
			for(i=0; i < gd.io_info.mr->v_nx * gd.io_info.mr->v_ny; i++) {
			     if((i % gd.io_info.mr->v_nx) == 0) printf("\n");
			     val = *tmp_ptr++ * gd.io_info.mr->v_scale + gd.io_info.mr->v_bias;
			     printf("%.1f ",val);
			}
			printf("\n");
			}
		    }
                    gd.v_win.redraw[gd.io_info.field] = 1;
                    if(gd.io_info.field == gd.v_win.field) {
                        gd.movie.frame[gd.movie.cur_index].redraw_vert = 1;
                    }
                }
                break;
            }

        } else {
          if ((gd.io_info.mode == STATIC_DATA  || ((reply.status & NO_NEW_DATA) == 0))) {
 
                if(gd.debug1) fprintf(stderr, "NO DATA returned - field %s\n", gd.io_info.mr->field_name);
                sprintf(label, "NO %s DATA Retrieved\n", gd.io_info.mr->field_name);
		rd_h_msg(label,-1);

                switch(gd.io_info.hv_flag) {
                case 1: /* horizontal */
                    gd.io_info.mr->h_data_valid = 1; /*  */
                    if(gd.io_info.mr->h_data != NULL) ufree(gd.io_info.mr->h_data);
                    gd.io_info.mr->h_data = NULL;
                    gd.io_info.mr->h_date.unix_time = 0;
                    gd.h_win.redraw[gd.io_info.field] = 1;
                    if(gd.io_info.field == gd.h_win.field) {
                        gd.movie.frame[gd.movie.cur_index].redraw_horiz = 1;
                    }
                    break;
    
                case 2: /* vertical */
                    gd.io_info.mr->v_data_valid = 1; /*  */
                    if(gd.io_info.mr->v_data != NULL) ufree(gd.io_info.mr->v_data);
                    gd.io_info.mr->v_data = NULL;
                    gd.io_info.mr->v_date.unix_time = 0;
                    gd.v_win.redraw[gd.io_info.field] = 1;
                    if(gd.io_info.field == gd.v_win.field) {
                        gd.movie.frame[gd.movie.cur_index].redraw_vert = 1;
                    }
                    break;
                }
            } else {

                if(gd.debug1) fprintf(stderr, "NO NEW DATA returned - field %s\n", gd.io_info.mr->field_name);
                switch(gd.io_info.hv_flag) {
                case 1: /* horizontal */
                    gd.io_info.mr->h_data_valid = 1;
                    break;
    
                case 2: /* vertical */
                    gd.io_info.mr->v_data_valid = 1;
                    break;
                }
            }
            rd_h_msg(gd.frame_label,1);
        }

done_with_server:

     close(gd.io_info.fd);
     gd.io_info.outstanding_request = 0;

    if(gd.debug) umalloc_verify();
    
    return ;
}

#ifndef LINT
static char RCS_id[] = "$Id: cidd_data.c,v 1.63 2016/03/07 18:28:26 dixon Exp $";
#endif /* not LINT */
