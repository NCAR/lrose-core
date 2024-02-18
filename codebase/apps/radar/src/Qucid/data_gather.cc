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
/**********************************************************************
 * DATA_GATHER.CC 
 *  The gather* routines check each data set that must be retrieved
 *  in order to render and image. When a data set is marked as invalid,
 *  a single request for data is made. Then control is returned
 *  as a IO thread takes over reading the reply. Once all data
 *  is marked as valid (all requests have been fulfilled) then
 *  these routines indicate that the data are complete and rendering
 *  is ready to take place.
 */

#define DATA_GATHER

#include "cidd.h"

void cancel_pending_request();
/**********************************************************************
 * GATHER_HWIN_DATA: Collect all the data necessary for horizontal 
 *    display windows 
 */

int gather_hwin_data( int page, time_t start_time, time_t end_time)
{
    int    i;
    time_t    now;
    met_record_t *mr;       /* pointer to record for convienence */

    now = time(0);
    /* Check to make sure we are not currently waiting on an I/O request */
    if(gd.io_info.outstanding_request > 0) {
        if(now > gd.io_info.expire_time) {
	    cancel_pending_request();
            return CIDD_FAILURE;
	}
        return INCOMPLETE;
    }

    // MAIN GRID
    mr = gd.mrec[page];    /* get pointer to data record */
    if(mr->h_data_valid == 0) {
        if(gd.debug1) fprintf(stderr, "Requesting False Color Field %d data time %s %s\n", page,utimstr(start_time),utimstr(end_time));
        gd.data_status_changed = 0;
        if(mdvx_request_horiz_data_plane(mr,start_time,end_time,page) < 0) {
	   return CIDD_FAILURE;
	} else {
           return INCOMPLETE;
	}
    }

    // TERRAIN GRID
    if(gd.layers.earth.terrain_active && mr->ds_fhdr.nz > 1) {
        mr = gd.layers.earth.terr;    /* get pointer to data record */
        if(mr->h_data_valid == 0) {
            if(gd.debug1) fprintf(stderr,
	       "Requesting Terrain data time %ld %ld\n",start_time,end_time);
            gd.data_status_changed = 0;
            if(mdvx_request_horiz_data_plane(mr,start_time,end_time,page) < 0) {
	       return CIDD_FAILURE;
	    } else {
               return INCOMPLETE;
	    }
	}
    } 

    // LANDUSE GRID
    if(gd.layers.earth.landuse_active) {
        mr = gd.layers.earth.land_use;    /* get pointer to data record */
        if(mr->h_data_valid == 0) {
            if(gd.debug1) fprintf(stderr,
	       "Requesting Landuse data time %ld %ld\n",start_time,end_time);
            gd.data_status_changed = 0;
            if(mdvx_request_horiz_data_plane(mr,start_time,end_time,page) < 0) {
	       return CIDD_FAILURE;
	    } else {
               return INCOMPLETE;
	    }
	}
    } 

  /* LAYERED GRIDS -  Request any needed data for gridded layers */
  for(i=0; i < NUM_GRID_LAYERS; i++) {
    if(gd.layers.overlay_field_on[i]) {
        mr = gd.mrec[gd.layers.overlay_field[i]];
        if(mr->h_data_valid == 0) {
            if(gd.debug1) fprintf(stderr, "Requesting Overlay Field %d data\n", gd.layers.overlay_field[i]);
            gd.data_status_changed = 0;
            mdvx_request_horiz_data_plane(mr,start_time,end_time,page);
            return INCOMPLETE;
        }
    }
  }

  /* CONTOURS: request any needed data for contours */
  for(i=0; i < NUM_CONT_LAYERS; i++) {
    if(gd.layers.cont[i].active) {
        mr = gd.mrec[gd.layers.cont[i].field];
        if(mr->h_data_valid == 0) {
            if(gd.debug1) fprintf(stderr,
	      "Requesting Contour Layer %d, field %d data\n",
	      i, gd.layers.cont[i].field);

            gd.data_status_changed = 0;
            mdvx_request_horiz_data_plane(mr,start_time,end_time,page);
            return INCOMPLETE;
        }
    }
  }

    // WINDS
    for(i=0; i < gd.layers.num_wind_sets; i++) {
        switch(gd.layers.wind_mode) {
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

        if(gd.layers.wind_vectors  && gd.layers.wind[i].active ) {
            mr = gd.layers.wind[i].wind_u;
            if(mr->h_data_valid == 0) {
                if(gd.debug1) fprintf(stderr, "Requesting Wind %d data - U\n", i);
                gd.data_status_changed = 0;
                mdvx_request_horiz_data_plane(mr,start_time,end_time,page);
                return INCOMPLETE;
            }
    
            mr = gd.layers.wind[i].wind_v;
            if(mr->h_data_valid == 0) {
                if(gd.debug1) fprintf(stderr, "Requesting Wind %d data - V\n", i);
                gd.data_status_changed = 0;
                mdvx_request_horiz_data_plane(mr,start_time,end_time,page);
                return INCOMPLETE;
            }
    
            mr = gd.layers.wind[i].wind_w;
            if(mr != NULL) {
                if(mr->h_data_valid == 0) {
                    if(gd.debug1) fprintf(stderr, "Requesting Wind %d  data - W\n", i);
                    gd.data_status_changed = 0;
                    mdvx_request_horiz_data_plane(mr,start_time,end_time,page);
                    return INCOMPLETE;
                }
            }
        }
    }

  // Native SYMPROD DATA Gather
  if(gd.prod.products_on) {
    if(gd.syprod_P->short_requests) { 
        return gd.prod_mgr->getData(start_time, end_time);
    } else {
        return gd.prod_mgr->getData(gd.epoch_start, gd.epoch_end);
    }
  } else {
      return CIDD_SUCCESS;
  }
    
}

/**********************************************************************
 * GATHER_VWIN_DATA: Collect all the data necessary for vertical 
 *    display windows 
 *
 */

int gather_vwin_data( int page, time_t    start_time, time_t end_time)
{
    int    i;
    met_record_t *mr;       /* pointer to record for convienence */
    time_t now;

    now = time(0);
    /* Check to make sure we are not currently waiting on an I/O request */
    if(gd.io_info.outstanding_request) {
        if(now > gd.io_info.expire_time) {
	   cancel_pending_request();
           return CIDD_FAILURE;
        }
        return INCOMPLETE;
    }
     
    // MAIN GRID
    mr = gd.mrec[page];    /* get pointer to data record */
    if(mr->v_data_valid == 0) {
	if(mr->h_data_valid == 0) {  // Need Header info obtained from horiz request
            if(mdvx_request_horiz_data_plane(mr,start_time,end_time,page) < 0) {
                return CIDD_FAILURE;
	    } else {
                return INCOMPLETE;
	    }
	}
	    
        if(gd.debug1) fprintf(stderr, "Requesting False Color Field %d data\n", page);
        gd.data_status_changed = 0;
        if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
            return CIDD_FAILURE;
	} else {
            return INCOMPLETE;
	}
    }

    // TERRAIN GRID
    if(gd.layers.earth.terrain_active) {
        mr = gd.layers.earth.terr;    /* get pointer to data record */
        if(mr->v_data_valid == 0) {
            if(gd.debug1) fprintf(stderr,
	       "Requesting Terrain cross section data time %ld %ld\n",start_time,end_time);
            gd.data_status_changed = 0;
            if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
	       return CIDD_FAILURE;
	    } else {
               return INCOMPLETE;
	    }
	}
    } 

    // Land use is not valid in cross section

    // CONTOURS
    for(i=0; i < NUM_CONT_LAYERS ; i++) {
      if(gd.layers.cont[i].active) {
        mr = gd.mrec[gd.layers.cont[i].field];
        if(mr->v_data_valid == 0) {
            if(gd.debug1) fprintf(stderr,
			   "Requesting VERT Contour Field %d data\n",
			   gd.layers.cont[i].field);
             
            gd.data_status_changed = 0;
            if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
                return CIDD_FAILURE;
            } else {
                return INCOMPLETE;
	    }
        }
      }
    }

    // ROUTE WINDs, TURBULENCE, ICING
    if(gd.layers.route_wind.u_wind != NULL) {
        mr = gd.layers.route_wind.u_wind;
        if(mr->v_data_valid == 0) {
            if(gd.debug1) fprintf(stderr, "Requesting  %s Route data \n",mr->legend_name);
            gd.data_status_changed = 0;
            if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
                return CIDD_FAILURE;
            } else {
                return INCOMPLETE;
	    }
        }
    }

    if(gd.layers.route_wind.v_wind != NULL) {
        mr = gd.layers.route_wind.v_wind;
        if(mr->v_data_valid == 0) {
            if(gd.debug1) fprintf(stderr, "Requesting  %s Route data \n",mr->legend_name);
            gd.data_status_changed = 0;
            if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
                return CIDD_FAILURE;
            } else {
                return INCOMPLETE;
	    }
        }
    }

    if(gd.layers.route_wind.turb != NULL) {
        mr = gd.layers.route_wind.turb;
        if(mr->v_data_valid == 0) {
            if(gd.debug1) fprintf(stderr, "Requesting - %s Route data \n",mr->legend_name);
            gd.data_status_changed = 0;
            if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
                return CIDD_FAILURE;
            } else {
                return INCOMPLETE;
	    }
        }
    }

    if(gd.layers.route_wind.icing != NULL) {
        mr = gd.layers.route_wind.icing;
        if(mr->v_data_valid == 0) {
            if(gd.debug1) fprintf(stderr, "Requesting Route Icing  - %s data\n",mr->legend_name);
            gd.data_status_changed = 0;
            if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
                return CIDD_FAILURE;
            } else {
                return INCOMPLETE;
	    }
        }
    }

    // WINDS
    for(i=0; i < gd.layers.num_wind_sets; i++) {
        if(gd.layers.wind_vectors  && gd.layers.wind[i].active) {
            mr = gd.layers.wind[i].wind_u;
            if(mr->v_data_valid == 0) {
                if(gd.debug1) fprintf(stderr, "Requesting Wind %d - %s data - U\n", i,mr->legend_name);
                gd.data_status_changed = 0;
                if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
                    return CIDD_FAILURE;
                } else {
                    return INCOMPLETE;
	        }
            }
    
            mr = gd.layers.wind[i].wind_v;
            if(mr->v_data_valid == 0) {
                if(gd.debug1) fprintf(stderr, "Requesting Wind %d - %s  data - V\n", i,mr->legend_name);
                gd.data_status_changed = 0;
                if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
                    return CIDD_FAILURE;
                } else {
                    return INCOMPLETE;
	        }
            }
    
            mr = gd.layers.wind[i].wind_w;
            if(mr != NULL) {
                if(mr->v_data_valid == 0) {

                    if(gd.debug1) fprintf(stderr, "Requesting Wind %d - %s data - W\n", i,mr->legend_name);

                    gd.data_status_changed = 0;
                    if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
                        return CIDD_FAILURE;
                    } else {
                        return INCOMPLETE;
	            }
                }
            }
        }
    }


    return CIDD_SUCCESS;
}

/**********************************************************************
 * CANCEL_PENDING_REQUEST: 
 *
 */

void cancel_pending_request()
{
    if(gd.io_info.outstanding_request == 0) return;

    if(gd.io_info.mode != DSMDVX_DATA) {
	close(gd.io_info.fd);
        if(gd.io_info.incoming_data_pointer != NULL) {
            free(gd.io_info.incoming_data_pointer);
	    gd.io_info.incoming_data_pointer = NULL;
	}
    } else {
	 switch(gd.io_info.request_type) {
	     case HORIZ_REQUEST:
	     case TIMELIST_REQUEST:
		 gd.io_info.mr->h_mdvx->cancelThread();
	     break;

	     case VERT_REQUEST: 
		 gd.io_info.mr->v_mdvx->cancelThread();
	     break;

	     case SYMPROD_REQUEST: 
		 DsSpdbThreaded *spdb = gd.io_info.prod->getSpdbObj();
		 spdb->cancelThread();
	     break;
	}
    }

    gd.io_info.outstanding_request = 0;
    gd.io_info.expire_time = time(0) + _params.data_timeout_secs;

   if(gd.debug1) fprintf(stderr,"Timeout during data service access - giving up\n");
   if(!_params.run_once_and_exit)  PMU_auto_register("Server Request Timeout");
   if(_params.show_data_messages) gui_label_h_frame("Data Service Request Timed Out!... Retrying... ",-1);
   add_message_to_status_win("Data Service Request Timed Out!... Retrying... ",1);
   set_busy_state(0);

}
