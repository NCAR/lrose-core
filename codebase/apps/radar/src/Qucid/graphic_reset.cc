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
/*************************************************************************
 * GRAPHIC_RESET.C : 
 *
 * For the Configurable Interactive Data Display (CIDD)
 * Frank Hage   July 1991 NCAR, Research Applications Program
 */

#define GRAPHIC_RESET 1

#include "cidd.h"

void next_cache_image();
void prev_cache_image();
/*************************************************************************
 * RESET_time_list_valid_flags: Indicate time list data are invalid
 *
 */

void reset_time_list_valid_flags()
{

    int    i;

    for(i=0; i < gd.num_datafields; i++) {
         gd.mrec[i]->time_list_valid = 0;
    }

    for(i=0; i < gd.layers.num_wind_sets; i++) {
        gd.layers.wind[i].wind_u->time_list_valid = 0;
        gd.layers.wind[i].wind_v->time_list_valid = 0;
        if(gd.layers.wind[i].wind_w != NULL) {
            gd.layers.wind[i].wind_w->time_list_valid = 0;
        }
    }
    
    if (gd.prod_mgr) {
      gd.prod_mgr->reset_times_valid_flags();
    }

}

/*************************************************************************
 * RESET_TERRAIN_VALID_FLAGS: invalidate the terrain and landuse fields
 *
 */

void reset_terrain_valid_flags(int hflag,int vflag)
{
    if(hflag) {
	if(gd.layers.earth.terrain_active) {
	 gd.layers.earth.terr->h_data_valid = 0;
	 gd.layers.earth.terr->h_date.unix_time = 0;
	}

	if(gd.layers.earth.landuse_active) {
	 gd.layers.earth.land_use->h_data_valid = 0;
	 gd.layers.earth.land_use->h_date.unix_time = 0;
	}
    }

    if(vflag) {

	if(gd.layers.earth.terrain_active) {
	 gd.layers.earth.terr->v_data_valid = 0;
	 gd.layers.earth.terr->v_date.unix_time = 0;
	}

    }
}

/*************************************************************************
 * RESET_TIME_ALLOWANCES
 *
 */

void reset_time_allowances()
{
    int    i;

	double allow = gd.movie.mr_stretch_factor * gd.movie.time_interval;

    for(i=0; i < gd.num_datafields; i++) {
                 gd.mrec[i]->time_allowance = allow;
    }

    for(i=0; i < gd.layers.num_wind_sets; i++) {
        gd.layers.wind[i].wind_u->time_allowance  = allow;
        gd.layers.wind[i].wind_v->time_allowance = allow;
        if(gd.layers.wind[i].wind_w != NULL) {
            gd.layers.wind[i].wind_w->time_allowance = allow;
        }
    }

	if(gd.layers.earth.terrain_active) {
	 gd.layers.earth.terr->time_allowance = allow;
	}

	if(gd.layers.route_wind.u_wind != NULL) {
	 gd.layers.route_wind.u_wind->time_allowance = allow;
	}

	if(gd.layers.route_wind.v_wind != NULL) {
	 gd.layers.route_wind.v_wind->time_allowance = allow;
	}

	if(gd.layers.route_wind.turb != NULL) {
	 gd.layers.route_wind.turb->time_allowance = allow;
	}

	if(gd.layers.route_wind.icing != NULL) {
	 gd.layers.route_wind.icing->time_allowance = allow;
	}

    gd.data_status_changed = 1;
}


/*************************************************************************
 * RESET_DATA_VALID_FLAGS: Indicate data arrays are invalid
 *
 */

void reset_data_valid_flags(int hflag,int vflag)
{
    int    i;

    if(hflag) {
        for(i=0; i < gd.num_datafields; i++) {
			 if(strcasecmp("none",gd.mrec[i]->button_name) == 0 ) {
				 gd.mrec[i]->h_data_valid = 1;
			 } else {
                 gd.mrec[i]->h_data_valid = 0;
                 gd.mrec[i]->h_date.unix_time = 0;
			 }
        }

        for(i=0; i < gd.layers.num_wind_sets; i++) {
            gd.layers.wind[i].wind_u->h_data_valid = 0;
            gd.layers.wind[i].wind_u->h_date.unix_time = 0;
            gd.layers.wind[i].wind_v->h_data_valid = 0;
            gd.layers.wind[i].wind_v->h_date.unix_time = 0;
            if(gd.layers.wind[i].wind_w != NULL) {
                gd.layers.wind[i].wind_w->h_data_valid = 0;
                gd.layers.wind[i].wind_w->h_date.unix_time = 0;
            }
        }

    }

    if(vflag) {
        for(i=0; i < gd.num_datafields; i++) {
			 if(strcasecmp("none",gd.mrec[i]->button_name) == 0 ) {
				 gd.mrec[i]->v_data_valid = 1;
			 } else {
               gd.mrec[i]->v_data_valid = 0;
               gd.mrec[i]->v_date.unix_time = 0;
			 }
        }

	if(gd.layers.earth.terrain_active) {
	 gd.layers.earth.terr->v_data_valid = 0;
	 gd.layers.earth.terr->v_date.unix_time = 0;
	}

	if(gd.layers.route_wind.u_wind != NULL) {
	 gd.layers.route_wind.u_wind->v_data_valid = 0;
	 gd.layers.route_wind.u_wind->v_date.unix_time = 0;
	}

	if(gd.layers.route_wind.v_wind != NULL) {
	 gd.layers.route_wind.v_wind->v_data_valid = 0;
	 gd.layers.route_wind.v_wind->v_date.unix_time = 0;
	}

	if(gd.layers.route_wind.turb != NULL) {
	 gd.layers.route_wind.turb->v_data_valid = 0;
	 gd.layers.route_wind.turb->v_date.unix_time = 0;
	}

	if(gd.layers.route_wind.icing != NULL) {
	 gd.layers.route_wind.icing->v_data_valid = 0;
	 gd.layers.route_wind.icing->v_date.unix_time = 0;
	}

        for(i=0; i < gd.layers.num_wind_sets; i++) {
            if(gd.layers.wind[i].wind_u != NULL) {
                gd.layers.wind[i].wind_u->v_data_valid = 0;
                gd.layers.wind[i].wind_u->v_date.unix_time = 0;
	    }

            if(gd.layers.wind[i].wind_v != NULL) {
                gd.layers.wind[i].wind_v->v_data_valid = 0;
                gd.layers.wind[i].wind_v->v_date.unix_time = 0;
	    }

            if(gd.layers.wind[i].wind_w != NULL) {
                gd.layers.wind[i].wind_w->v_data_valid = 0;
                gd.layers.wind[i].wind_w->v_date.unix_time = 0;
            }
        }
    }
    gd.data_status_changed = 1;
}

/*************************************************************************
 * SET_REDRAW_FLAGS : Set the redraw flags for each field or frame
 *    When set to 1, Frame need rerendering
 */

void set_redraw_flags(int h_flag,int v_flag)
{
    int    i;
    int   hit = 0;
    
    if(h_flag) {
        for(i=0; i < MAX_FRAMES; i++) {
            if(gd.movie.frame[i].redraw_horiz ==0) {
              gd.movie.frame[i].redraw_horiz = 1;
	      hit = 1;
	}

        }
        for(i=0; i < gd.num_datafields; i++) {
            if(gd.h_win.redraw[i] == 0) {
              gd.h_win.redraw[i] = 1;
	      hit = 1;
	    }
        }
    }

    if(v_flag) {
        for(i=0; i < MAX_FRAMES; i++) {
            gd.movie.frame[i].redraw_vert = 1;
        }
        for(i=0; i < gd.num_datafields; i++) {
             gd.v_win.redraw[i] = 1;
        }
    }

    // Rotate Cache images
    if(hit) {
	  next_cache_image();

	  // Prevents switching to cached image before being re-drawn
      gd.h_win.last_cache_im = gd.h_win.cur_cache_im;
    }
}

/*************************************************************************
 * Next_cache_image : Rotate the cache image indicators forward.
*/
void next_cache_image()
{
      gd.h_win.cur_cache_im++;
      if(gd.h_win.cur_cache_im >= gd.num_cache_zooms) gd.h_win.cur_cache_im = 0;
      // xv_set(gd.h_win_horiz_bw->im_cache_st,PANEL_VALUE,gd.h_win.cur_cache_im,NULL);
}

/*************************************************************************
 * Prev_cache_image : Rotate the cache image indicators backwards.
*/
void prev_cache_image()
{
      gd.h_win.cur_cache_im--;
      if(gd.h_win.cur_cache_im   < 0 ) gd.h_win.cur_cache_im = gd.num_cache_zooms -1 ;
      // xv_set(gd.h_win_horiz_bw->im_cache_st,PANEL_VALUE,gd.h_win.cur_cache_im,NULL);
}
