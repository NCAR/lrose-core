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
//////////////////////////////////////////////////////////////////
// CIDD_REMOTE_COMMANDS.CC 
//
// A message read through the queue consists of any number of the
// following commands. 
//
// Currently Supported Commands:
//
// SET_ALTITUDE Value   // Sets the altitude
// SET_REQUEST_TIME Time     // Data request Time - either "now" or integer unix_time
//                           // seconds since 1970
// PICK_LATEST_MODEL_RUN     // Requests  data from the latest model runs
// PICK_MODEL_RUN  Time      // Requests data from Model run at the specific time (integer unix time)
// SET_REFERENCE_LINES  ON | OFF // Turns on/off the route and height reference lines.
// #                    // Comment
// #
// SELECT_ROUTE Name             // Sets a Predefined route by name
// SELECT_ROUTE_NUM Number       // Sets a Predefined route by number
// SET_ROUTE Lat Lon Lat lon ... // Sets a route by lat,lon point list
//
// SELECT_V_PAGE Name            // Selects the Page/data to display by name
// SELECT_V_PAGE_NUM Number      // Selects the Page/data to display by number
// *
// SET_V_IMAGE_SIZE  X Y         // Sets the size of the output image
// SET_V_IMAGE_NAME Name         // Sets the output file name
// DUMP_V_IMAGE                  // Output a Cross Section Image
// #
// SELECT_DOMAIN_NAME name       // Sets a Predefined zoom by label
// SELECT_DOMAIN_NUM Number      // Sets a Predefined zoom by index number 
// SET_DOMAIN MIN_X MIN_Y MAX_X MAXY // Sets zoom by bounding box
// #
// SET_H_IMAGE_HEIGHT Height     // Sets the height of the output image - Width set by aspect ratio
// SET_H_IMAGE_SIZE Height Width // Sets the size of the output image - Establishes aspect ratio 
// SELECT_H_PAGE Name            // Selects the Page/data to display by name
// SELECT_H_PAGE_NUM Number      // Selects the Page/data to display by number
// #
// SET_H_IMAGE_NAME Name         // Sets the output file name 
// GEN_H_IMAGE_NAME              // Automatically Generates an Image name
// DUMP_H_IMAGE                  // Output a Plan View Image
// #
// SET_SYMPRODS_OFF_ALL          // Turns off All Symbolic Products 
// SET_SYMPRODS_ON Name          // Turns On a specific Symbolic Product by name
// #
// SET_MAPS_OFF_ALL              // Turns off All Map Overlays 
// SET_MAPS_ON Name              // Turns On a specific Map Overlay by name
// #
// SET_WINDS_OFF_ALL             // Turns off All Wind Fields 
// SET_WINDS_ON Name             // Turns On a specific Wind Field by name
// #
// NEW_MDV_AVAIL Name            // Indicates Mdv data with Menu label matching Name needs reloaded.
// NEW_SPDB_AVAIL Name           // Indicates Spdb data with Menu label matching Name needs reloaded.
// RELOAD_DATA 					 // Force all data to be reloaded.
// #

#define CIDD_REMOTE_COMMANDS
#include "cidd.h"
#include <toolsa/str.h>

// Space for sub string parsing
#define NUM_PARSE_FIELDS MAX_ROUTE_SEGMENTS * 2
#define PARSE_FIELD_SIZE 1024

char *parse_remote_commands(char *);

////////// INGEST_REMOTE_COMMANDS ////////////////////////////////////////////
void   ingest_remote_commands()
{
    int status;
    static char *buf = NULL;
    static char *buf_ptr = NULL;
    string  msg;


    if(buf_ptr != NULL) { // Partially finished message
        if(gd.debug1) fprintf(stderr, "Continuing to Process Message\n");
	buf_ptr = parse_remote_commands(buf_ptr); 
	if(buf_ptr == NULL) {
	    free(buf);
	    buf = NULL;
	} else {
	   return;
	}
    }

    msg = gd.remote_ui->readNextContents(status);

    if(status) return; // No Messages pending

    if(gd.debug1) fprintf(stderr, "Received Remote UI FMQ MESSAGE:\n");
    if(gd.debug2) fprintf(stderr, "Received Remote UI FMQ MESSAGE:\n%s\n",msg.c_str());

    if((buf = (char *) calloc(msg.length()+1,1)) == NULL) {
	perror("Calloc failure!");
	return ;
    }

    strncpy(buf,msg.c_str(),msg.length()+1);

    buf_ptr = parse_remote_commands(buf);

    if(buf_ptr == NULL) free(buf);
}

////////// REMOTE_SET_H_PAGE ////////////////////////////////////////////

void remote_set_h_page(char *name)
{
  int len = strlen(name);
  char *ptr = name;
  
  while(len) {
     if(_params.replace_underscores && *ptr == '_') *ptr = ' ';
     ptr++;
     len--;
  }

  for(int i = 0; i < gd.num_datafields; i++) {
      if(strcmp(name,gd.mrec[i]->button_name) == 0) {
	  gd.h_win.page = i;
	  gd.h_win.redraw_flag[i] = 1; 
	  gd.movie.frame[gd.movie.cur_frame].redraw_horiz = 1;
      }
  }
}

////////// REMOTE_NEW_MDV_AVAIL ////////////////////////////////////////////
void remote_new_mdv_avail(const char *name)
{
	int i;
	int need_redraw = 0;


    /* look thru primary data fields */
    for (i=0; i < gd.num_datafields; i++) {
      if (strncmp(gd.mrec[i]->button_name,name,MAX_CLIENT_EVENT_ARG) == 0) {
          gd.mrec[i]->h_data_valid = 0;
          gd.mrec[i]->v_data_valid = 0;
          gd.mrec[i]->time_list_valid = 0;
          need_redraw = 1;
      }
    }

    /* Look through wind field data too */
    for (i=0; i < gd.layers.num_wind_sets; i++ ) {
      if (strncmp(gd.layers.wind[i].wind_u->button_name,name,MAX_CLIENT_EVENT_ARG) == 0) {
          gd.layers.wind[i].wind_u->h_data_valid = 0;
          gd.layers.wind[i].wind_u->v_data_valid = 0;
          gd.layers.wind[i].wind_u->time_list_valid = 0;

          gd.layers.wind[i].wind_v->h_data_valid = 0;
          gd.layers.wind[i].wind_v->v_data_valid = 0;
          gd.layers.wind[i].wind_v->time_list_valid = 0;

          if(gd.layers.wind[i].wind_w != NULL) {
            gd.layers.wind[i].wind_w->h_data_valid = 0;
            gd.layers.wind[i].wind_w->v_data_valid = 0;
            gd.layers.wind[i].wind_w->time_list_valid = 0;
          }

          need_redraw = 1;
      }
    }

    if(need_redraw) set_redraw_flags(1,1);
}

////////// REMOTE_NEW_SPDB_AVAIL ////////////////////////////////////////////
void remote_new_spdb_avail(const char *name)
{
	int i;
	int need_redraw = 0;

    for(i=0; i < _params.symprod_prod_info_n; i++) {
      if (strncmp(_params._symprod_prod_info[i].menu_label,name,MAX_CLIENT_EVENT_ARG) == 0) {
          // Invalidate the data
          gd.prod_mgr->set_product_data_valid(i,0);
          if(gd.prod_mgr->get_product_active(i)) need_redraw = 1;

          // Invalidate the lists of data times
          gd.prod_mgr->set_product_times_valid(i,0);
      }
    }

    if(need_redraw) set_redraw_flags(1,1);
}

////////// REMOTE_SET_H_PAGE_NUM ////////////////////////////////////////////
void remote_set_h_page_num(int  num)
{
      if( num >= 0 && num < gd.num_datafields) {
	  gd.h_win.page = num;
	  gd.h_win.redraw_flag[num] = 1; 
	  gd.movie.frame[gd.movie.cur_frame].redraw_horiz = 1;
      }
}

////////// REMOTE_SET_V_PAGE ////////////////////////////////////////////

void remote_set_v_page(char *name)
{
  int len = strlen(name);
  char *ptr = name;
  
  while(len) {
     if(_params.replace_underscores && *ptr == '_') *ptr = ' ';
     ptr++;
     len--;
  }
  for(int i = 0; i < gd.num_datafields; i++) {
      if(strcmp(name,gd.mrec[i]->button_name) == 0) {
	  gd.v_win.page = i;
	  gd.v_win.redraw_flag[i] = 1; 
	  gd.movie.frame[gd.movie.cur_frame].redraw_vert = 1;
      }
  }
}

////////// REMOTE_SET_V_PAGE_NUM ////////////////////////////////////////////
void remote_set_v_page_num(int  num)
{
      if( num >= 0 && num < gd.num_datafields) {
	  gd.v_win.page = num;
	  gd.v_win.redraw_flag[num] = 1; 
	  gd.movie.frame[gd.movie.cur_frame].redraw_vert = 1;
      }
}

//////////// REMOTE_SET_DOMAIN /////////////////////////////////////////////////
void remote_set_domain(double x1, double y1, double x2, double y2)
{
    int index = gd.h_win.num_zoom_levels -1;
    double dx,dy;

   /* put coords in ascending order */ 
    if(x1 < x2) {   
	gd.h_win.zmin_x[index] = x1;
	gd.h_win.zmax_x[index] = x2;
    } else {
	gd.h_win.zmin_x[index] = x2;
	gd.h_win.zmax_x[index] = x1;
    }
    if(y1 < y2) {    
	gd.h_win.zmin_y[index] = y1;
	gd.h_win.zmax_y[index] = y2;
    } else {
	gd.h_win.zmin_y[index] = y2;
	gd.h_win.zmax_y[index] = y1;
    }

    dx = gd.h_win.zmax_x[index] - gd.h_win.zmin_x[index];
    dy = gd.h_win.zmax_y[index] - gd.h_win.zmin_y[index];

    switch(gd.display_projection) { 
	/* forshorten the Y coords to make things look better */
	case Mdvx::PROJ_LATLON:
		gd.aspect_correction = cos(((gd.h_win.zmax_y[index] + gd.h_win.zmin_y[index])/2.0) * DEG_TO_RAD);
	    dy /= gd.aspect_correction;
	break;
    }

    /* Force the domain into the aspect ratio */
    // dy *= _params.aspect_ratio;
    if(dx > dy)  {
        gd.h_win.zmax_y[index] += (dx - dy) /2.0;
	gd.h_win.zmin_y[index] -= (dx - dy) /2.0;
    } else {
	gd.h_win.zmax_x[index] += (dy - dx) /2.0;
	gd.h_win.zmin_x[index] -= (dy - dx) /2.0;
    }

    /* make sure coords are within the limits of the display */
    if(gd.h_win.zmax_x[index] > gd.h_win.max_x) {
	gd.h_win.zmin_x[index] -= (gd.h_win.zmax_x[index] - gd.h_win.max_x);
	gd.h_win.zmax_x[index] = gd.h_win.max_x;
    }
    if(gd.h_win.zmax_y[index] > gd.h_win.max_y) {
	gd.h_win.zmin_y[index] -= (gd.h_win.zmax_y[index] - gd.h_win.max_y);
	gd.h_win.zmax_y[index] = gd.h_win.max_y;
    }
    if(gd.h_win.zmin_x[index] < gd.h_win.min_x) {
        gd.h_win.zmax_x[index] += (gd.h_win.min_x - gd.h_win.zmin_x[index]);
        gd.h_win.zmin_x[index] = gd.h_win.min_x;
    }
    if(gd.h_win.zmin_y[index] < gd.h_win.min_y) {
        gd.h_win.zmax_y[index] += (gd.h_win.min_y - gd.h_win.zmin_y[index]);
        gd.h_win.zmin_y[index] = gd.h_win.min_y;
    } 

    /* Set current area to our indicated zoom area */
    gd.h_win.cmin_x = gd.h_win.zmin_x[index];
    gd.h_win.cmax_x = gd.h_win.zmax_x[index];
    gd.h_win.cmin_y = gd.h_win.zmin_y[index];
    gd.h_win.cmax_y = gd.h_win.zmax_y[index];

    set_redraw_flags(1,0);  
    if(!_params.always_get_full_domain) {
      reset_time_list_valid_flags();
      reset_data_valid_flags(1,0);
      reset_terrain_valid_flags(1,0);
    }

    gd.h_win.zoom_level = index;   
}

//////////// REMOTE_SET_DOMAIN_NAME /////////////////////////////////////////////////

void remote_set_domain_name(char *name)
{
  char *label = NULL;
  for(int i = 0; i < gd.h_win.num_zoom_levels - NUM_CUSTOM_ZOOMS; i++) {
    // label = (char *) xv_get(gd.zoom_pu->domain_st,PANEL_CHOICE_STRING,i);
    if(label != NULL)  {
        if(strncmp(name,label,PARSE_FIELD_SIZE) == 0) {
          // set_domain_proc(gd.zoom_pu->domain_st,i, (Event *) NULL);
          return;
        }
      }
  }
}

//////////// REMOTE_SET_DOMAIN_NUM /////////////////////////////////////////////////

void remote_set_domain_num(int num)
{
  if(num >= 0 && num < gd.h_win.num_zoom_levels - NUM_CUSTOM_ZOOMS) {
       // set_domain_proc(gd.zoom_pu->domain_st,num, (Event *) NULL);
  }
}

//////////// REMOTE_SET_ROUTE /////////////////////////////////////////////////

void remote_set_route(char *rname)
{
  for(int i = 0; i < gd.layers.route_wind.num_predef_routes; i++) {
      if(strncmp(rname,gd.layers.route_wind.route[i].route_label,PARSE_FIELD_SIZE) == 0) {
	  memcpy(&gd.h_win.route,gd.layers.route_wind.route+i,sizeof(route_track_t));
	  setup_route_area(1);
      }
  }
}

//////////// REMOTE_SET_ROUTE_NUM /////////////////////////////////////////////////

void remote_set_route_num(int num)
{
  if(num >= 0 && num < gd.layers.route_wind.num_predef_routes) {
      memcpy(&gd.h_win.route,gd.layers.route_wind.route+num,sizeof(route_track_t));
      setup_route_area(1);
  }
}

//////////// REMOTE_SET_WINDS_OFF /////////////////////////////////////////////////

void remote_set_winds_off()
{
  for(int i = 0; i <  gd.layers.num_wind_sets; i++) gd.layers.wind[i].active = 0;
  set_redraw_flags(1,1);
}


//////////// REMOTE_SET_WINDS_ON /////////////////////////////////////////////////

void remote_set_winds_on(char *name)
{
  for(int i = 0; i <  gd.layers.num_wind_sets; i++)  {
      if(strncmp(name,gd.layers.wind[i].wind_u->legend_name,PARSE_FIELD_SIZE) == 0) {
          gd.layers.wind[i].active = 1;
      }
  }
  set_redraw_flags(1,1);
}


//////////// REMOTE_SET_MAPS_OFF /////////////////////////////////////////////////

void remote_set_maps_off()
{
  for(int i = 0; i <  gd.num_map_overlays; i++) gd.overlays[i]->active = 0;
  set_redraw_flags(1,0);
}


//////////// REMOTE_SET_MAPS_ON /////////////////////////////////////////////////

void remote_set_maps_on(char *name)
{
  char *label = NULL;
  if(gd.num_map_overlays <= 32) {
    for(int i = 0; i <  gd.num_map_overlays; i++)  {
      // label = (char *)xv_get(gd.over_pu->over_pu_st,PANEL_CHOICE_STRING,i);
      if(label != NULL ) {
        if(strncmp(name,label,PARSE_FIELD_SIZE) == 0) {
	  gd.overlays[i]->active = 1;
        }
      }
    }
  } else {
    for(int i = 0; i <  gd.num_map_overlays; i++)  {
      // label = (char *)xv_get(gd.over_pu->over_lst,PANEL_LIST_STRING,i);
      if(label != NULL ) {
        if(strncmp(name,label,PARSE_FIELD_SIZE) == 0) {
	  gd.overlays[i]->active = 1;
        }
      }
    }
  }
  set_redraw_flags(1,0);
}


//////////// REMOTE_SET_SYMPRODS_OFF /////////////////////////////////////////////////

void remote_set_symprods_off()
{
  for(int i = 0; i <  _params.symprod_prod_info_n; i++) {
      gd.prod_mgr->set_product_active(i,FALSE);
  }
  set_redraw_flags(1,0);
}


//////////// REMOTE_SET_SYMPRODS_ON /////////////////////////////////////////////////

void remote_set_symprods_on(char *name)
{
  for(int i = 0; i <  _params.symprod_prod_info_n; i++)  {
      if(strncmp(name,_params._symprod_prod_info[i].menu_label,PARSE_FIELD_SIZE) == 0) {
          gd.prod_mgr->set_product_active(i,TRUE);
	  return;
      }
  }
  set_redraw_flags(1,0);
}

//////////// REMOTE_SET_HEIGHT /////////////////////////////////////////////////
void remote_set_height(double height)
{
    if(height != gd.h_win.cur_ht) {
        gd.h_win.cur_ht = height;
	reset_data_valid_flags(1,0);
	set_redraw_flags(1,1);
    }
}

//////////// REMOTE_SET_REFERENCE_LINES /////////////////////////////////////////////////
void remote_set_reference_lines(char *val_str) 
{
   if(strncasecmp(val_str,"on",2) == 0) {
       _params.display_ref_lines = pTRUE;
   } else {
       _params.display_ref_lines = pFALSE;
   }
  set_redraw_flags(1,0);
}

//////////// REMOTE_SET_TIME /////////////////////////////////////////////////
void remote_set_time(char *time_str) 
{
   time_t tm;
   time_t now = time(0);
   int start_time;

   if(strncasecmp(time_str,"now",3) == 0) {
       tm = now;
   } else {
       tm = (time_t) atoi(time_str);
   }

   // Decide what mode we need to be in.
   if(tm > (now - gd.movie.time_interval_mins * 60.0)) {
      gd.movie.mode = REALTIME_MODE;
	  gd.coord_expt->runtime_mode = RUNMODE_REALTIME;
	  gd.coord_expt->time_seq_num++;
   } else {
      gd.movie.mode = ARCHIVE_MODE;
	  gd.coord_expt->runtime_mode = RUNMODE_ARCHIVE;
	  gd.coord_expt->time_seq_num++;
   }

  // Adjust time allowances  if asking for forecast data
  if(tm > now + (gd.movie.time_interval_mins * 60.0)) { 
      if(gd.movie.magnify_mode == 0 ) {
	  gd.movie.time_interval_mins *= gd.movie.magnify_factor;
	  gd.movie.magnify_mode = 1;
      }
  } else {
      if(gd.movie.magnify_mode != 0 ) {
	  gd.movie.time_interval_mins /= gd.movie.magnify_factor;
	  gd.movie.magnify_mode = 0;
      }
  }

   start_time = tm - (time_t) ((gd.movie.num_frames -1) * gd.movie.time_interval_mins * 60.0);

   // If more than a half interval's distance from the current setup - shift the movie time
   if(abs(start_time - gd.movie.start_time) > gd.movie.time_interval_mins * 30.0) {
       gd.movie.start_time = start_time;
       gd.movie.start_time -= (gd.movie.start_time % gd.movie.round_to_seconds);

       gd.movie.cur_frame = gd.movie.end_frame;
       reset_time_points();
       if(! gd.run_unmapped) update_movie_popup(); 
       reset_data_valid_flags(1,1);
       set_redraw_flags(1,1);
   } 
}

//////////////////////////////////////////////////////////////////////////
// PARSE_REMOTE_COMMANDS: Search for proper commands on each line and
// Call sub function appropriately- Returns a pointer to the next line in the fils
// If a dump command was issued. - Returns Null if Commands have been
// Completed
//
char *parse_remote_commands(char *buf)
{
    int  i;
    char *line_ptr;
    char *lasts;
    static int first_time = 1;
    static int last_im_ht = 0;
    static int last_im_wd = 0;
    static char *cfield[NUM_PARSE_FIELDS];

    if(first_time) {
        /* get temp space for substrings */ 
        for(i = 0; i < NUM_PARSE_FIELDS; i++) { 
	    cfield[i] = (char *)  calloc(PARSE_FIELD_SIZE, 1);
        }
	first_time = 0;
    }

    gd.image_needs_saved = 0;
    gd.save_im_win  = 0;

    // Prime strtok; 
    line_ptr = strtok_r(buf,"\n",&lasts);

    // While there's lines in the command message
    while(line_ptr  != NULL) {
      if(*line_ptr != '#') {


	// Parse each line into character tokens
	STRparse(line_ptr, cfield, strlen(line_ptr), NUM_PARSE_FIELDS, PARSE_FIELD_SIZE); 

	// Process one command line

	// Altitude 
	if(strcmp(cfield[0],"SET_ALTITUDE") == 0) {
	    remote_set_height(atof(cfield[1]));
	}

	// Sets the time 
	if(strcmp(cfield[0],"SET_REQUEST_TIME") == 0) {
	    remote_set_time(cfield[1]);
	}

	// Request data from the most recent model run
	if(strcmp(cfield[0],"PICK_LATEST_MODEL_RUN") == 0) {
	    gd.model_run_time = 0;
	}

	// Request data from a specific model run
	if(strcmp(cfield[0],"PICK_MODEL_RUN") == 0) {
	    gd.model_run_time = atoi(cfield[1]);
	}

	// Sets the reference line on/off state 
	if(strcmp(cfield[0],"SET_REFERENCE_LINES") == 0) {
	    remote_set_reference_lines(cfield[1]);
	}

	// Sets a Predefined route by name 
	if(strcmp(cfield[0],"SELECT_ROUTE") == 0) {
	    remote_set_route(cfield[1]);
	}

	// Sets a Predefined route by number 
	if(strcmp(cfield[0],"SELECT_ROUTE_NUM") == 0) {
	    remote_set_route_num(atoi(cfield[1]));
	}

	// Sets a route by lat,lon point list 
	if(strcmp(cfield[0],"SET_ROUTE") == 0) {
	    fprintf(stderr,"WARNING; NOT IMPLEMENTED:  SET_ROUTE cmd - lat lon: %g, %g ...\n",
		    atof(cfield[1]),atof(cfield[2]));
	}

	// Selects the Page/data to display by name 
	if(strcmp(cfield[0],"SELECT_V_PAGE") == 0) {
	    remote_set_v_page(cfield[1]);
	}

	// Selects the Page/data to display by number 
	if(strcmp(cfield[0],"SELECT_V_PAGE_NUM") == 0) {
	    remote_set_v_page_num(atoi(cfield[1]));
	}

	// Sets the size of the output image 
	if(strcmp(cfield[0],"SET_V_IMAGE_SIZE") == 0) {
	    // xv_set(gd.v_win_v_win_pu->v_win_pu, 
	    //        WIN_HEIGHT, atoi(cfield[1]) + xv_get(gd.v_win_v_win_pu->controls1,XV_HEIGHT),
	    //        WIN_WIDTH, atoi(cfield[2]),
	    //        NULL);
	}

	// Sets the output file name 
	if(strcmp(cfield[0],"SET_V_IMAGE_NAME") == 0) {
	    STRcopy(gd.v_win.image_fname,cfield[1],MAX_PATH_LEN);
	    if(strstr(gd.v_win.image_fname,".png") == NULL) {
		 STRconcat(gd.v_win.image_fname,".png",MAX_PATH_LEN);
	    }
	}

	// Output a Cross Section Image 
	if(strcmp(cfield[0],"DUMP_V_IMAGE") == 0) {
	    gd.image_needs_saved  = 1;
	    gd.save_im_win  |= XSECT_VIEW;
	    // show_xsect_panel((u_int)1);
	    return (line_ptr + strlen(line_ptr) + 1);
	}

	// Sets a Predefined zoom by name 
	if(strcmp(cfield[0],"SELECT_DOMAIN_NAME") == 0) {
            remote_set_domain_name(cfield[1]);
	}

	// Sets a Predefined zoom by index number 
	if(strcmp(cfield[0],"SELECT_DOMAIN_NUM") == 0) {
            remote_set_domain_num(atoi(cfield[1]));
	}

	// Sets zoom by bounding box 
	if(strcmp(cfield[0],"SET_DOMAIN") == 0) {
            remote_set_domain(atof(cfield[1]),atof(cfield[2]),atof(cfield[3]),atof(cfield[4]));
	}

	// Selects the Page/data to display by name 
	if(strcmp(cfield[0],"SELECT_H_PAGE") == 0) {
	    remote_set_h_page(cfield[1]);
	}

	// Selects the Page/data to display by number 
	if(strcmp(cfield[0],"SELECT_H_PAGE_NUM") == 0) {
	    remote_set_h_page_num(atoi(cfield[1]));
	}

	// Sets the size of the output image 
	if(strcmp(cfield[0],"SET_H_IMAGE_HEIGHT") == 0) {
		int h = atoi(cfield[1]);
		// int cp_ht = h < gd.h_win.min_height ? 0 : xv_get(gd.h_win_horiz_bw->cp,XV_HEIGHT);
		if( h != last_im_ht ) {
                  // xv_set(gd.h_win_horiz_bw->horiz_bw,XV_HEIGHT, h + cp_ht + 2 , NULL);
		  last_im_ht = h;
		}
	}
	// Sets the size of the output image 
	if(strcmp(cfield[0],"SET_H_IMAGE_SIZE") == 0) {
		int h = atoi(cfield[1]);
		int w = atoi(cfield[2]);
		// _params.aspect_ratio = (double) w / (double) h;
		// int cp_ht = h < gd.h_win.min_height ? 0 : xv_get(gd.h_win_horiz_bw->cp,XV_HEIGHT);
		if( h != last_im_ht || w != last_im_wd) {
                  // xv_set(gd.h_win_horiz_bw->horiz_bw,XV_HEIGHT, h + cp_ht + 2 , NULL);
		  last_im_ht = h;
		  last_im_wd = w;
		}
	}

	// Sets the output file name 
	if(strcmp(cfield[0],"SET_H_IMAGE_NAME") == 0) {
	    STRcopy(gd.h_win.image_fname,cfield[1],MAX_PATH_LEN);
	    if(strstr(gd.h_win.image_fname,".png") == NULL) {
		 STRconcat(gd.h_win.image_fname,".png",MAX_PATH_LEN);
	    }
	}

	// Generates the output file name 
	if(strcmp(cfield[0],"GEN_H_IMAGE_NAME") == 0) {
	    gd.generate_filename = 1;
	}

	// Output a Plan View Image 
	if(strcmp(cfield[0],"DUMP_H_IMAGE") == 0) {
	    gd.save_im_win |= PLAN_VIEW;
	    gd.image_needs_saved = 1;
	    return (line_ptr + strlen(line_ptr) + 1);
	}

	// Turn off all Winds
	if(strcmp(cfield[0],"SET_WINDS_OFF_ALL") == 0) {
	   remote_set_winds_off();
	}

	// Turn on a Wind
	if(strcmp(cfield[0],"SET_WINDS_ON") == 0) {
	   remote_set_winds_on(cfield[1]);
	}

	// Turn off all Maps
	if(strcmp(cfield[0],"SET_MAPS_OFF_ALL") == 0) {
	   remote_set_maps_off();
	}

	// Turn on a Map
	if(strcmp(cfield[0],"SET_MAPS_ON") == 0) {
	   remote_set_maps_on(cfield[1]);
	}

	// Turn off all Symbolic Products
	if(strcmp(cfield[0],"SET_SYMPRODS_OFF_ALL") == 0) {
	   remote_set_symprods_off();
	}

	// Turn on a Wind
	if(strcmp(cfield[0],"SET_SYMPRODS_ON") == 0) {
	   remote_set_symprods_on(cfield[1]);
	}

	// Expire ALL Data 
	if(strcmp(cfield[0],"RELOAD_DATA") == 0) {
	   //remote_set_symprods_on(cfield[1]);
	}

	// Expire Mdv Data  
	if(strcmp(cfield[0],"NEW_MDV_AVAIL") == 0) {
	   //remote_set_symprods_on(cfield[1]);
	}

	// Expire Spdb Data  
	if(strcmp(cfield[0],"NEW_SPDB_AVAIL") == 0) {
	   //remote_set_symprods_on(cfield[1]);
	}

      } // End -  if line is not a comment

      //Get Next command line 
      line_ptr = strtok_r(NULL,"\n",&lasts);

    }  // End while lines in command

    return NULL;  // Commands ehausted.
}
