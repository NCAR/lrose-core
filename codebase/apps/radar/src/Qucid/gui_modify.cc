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
/*****************************************************************
 * GUI_MODIFY.CC: 
 */

#define GUI_MODIFY 1

#include "cidd.h"

void init_field_menus();  // Forward references
void update_frame_time_msg(int index);

/*****************************************************************
 * MODIFY_GUI_OBJECTS : Modify any Xview objects that couldn't
 *    be set up in Devguide. This is primarily to avoid manually
 *    changing any *ui.c file
 */

void modify_gui_objects()
{
    int    i,j;
#ifdef NOTNOW
    int    height;
#endif
    int    width = 0;
    int    render_line = 0;
    int    value = 0;
    extern void  field_mi_proc();
    extern void  zoom_mi_proc();

    char    str[NAME_LENGTH * 4];
    char    string[1024];
#ifdef NOTNOW
    const char  *resource;
#endif
    const char  *fname;
    char   panel_choice_string[256];

    // Xv_opaque   image1;
    // Xv_opaque   image2;
#ifdef NOTNOW
    static unsigned short bbits[] =  {
#include "icons/black.icon" 
    };

    static unsigned short wbits[] =  {
#include "icons/white.icon"
    };
#endif

    // image1 = xv_create(XV_NULL, SERVER_IMAGE,
    //   SERVER_IMAGE_DEPTH, 1,
    //   SERVER_IMAGE_BITS, bbits,
    //   XV_WIDTH, 64,
    //   XV_HEIGHT, 64,
    //   NULL);

    // image2 = xv_create(XV_NULL, SERVER_IMAGE,
    //   SERVER_IMAGE_DEPTH, 1,
    //   SERVER_IMAGE_BITS, wbits,
    //   XV_WIDTH, 64,
    //   XV_HEIGHT, 64,
    //   NULL);




    for(i=0; i < MAX_DATA_FIELDS; i++) {
        gd.h_win.page_xid[i] = 0;
        gd.v_win.page_xid[i] = 0;
    }

    XSetBackground(gd.dpy,gd.def_gc,gd.legends.background_color->pixval);
     
    for(i=0; i < NUM_CONT_LAYERS; i++) {
        XSetLineAttributes(gd.dpy,gd.layers.cont[i].color->gc,
            gd.layers.contour_line_width,LineSolid,CapButt,JoinBevel);
    }

    /* determine margin and other window assoc sizes for HORIZ window */
    gd.h_win.margin.top = _params.horiz_top_margin;
    gd.h_win.margin.bot = _params.horiz_bot_margin;
    gd.h_win.margin.left = _params.horiz_left_margin;
    gd.h_win.margin.right = _params.horiz_right_margin;

    if (_params.horiz_legends_start_x == 0) {
      gd.h_win.legends_start_x = gd.h_win.margin.left + 5;
    } else {
      gd.h_win.legends_start_x = _params.horiz_legends_start_x;
    }
    
    if (_params.horiz_legends_start_y == 0) {
      gd.h_win.legends_start_y = gd.h_win.margin.top * 2;
    } else {
      gd.h_win.legends_start_y = _params.horiz_legends_start_y;
    }

    if (_params.horiz_legends_delta_y == 0) {
      gd.h_win.legends_delta_y = gd.h_win.margin.top;
    } else {
      gd.h_win.legends_delta_y = _params.horiz_legends_delta_y;
    }

    gd.h_win.min_height = _params.horiz_min_height;
    gd.h_win.min_width = _params.horiz_min_width;
    gd.h_win.active = 1;

    fname = _params.status_info_file; 
    if(strlen(fname) < 2) {
      gd.status.status_fname = NULL;
    } else {
      gd.status.status_fname = fname;
    }

    // xv_set(gd.h_win_horiz_bw->horiz_bw,
    //     XV_X, _params.horiz_default_x_pos,
    //     XV_Y, _params.horiz_default_y_pos,
    //     XV_HEIGHT, _params.horiz_default_height,
    //     XV_LABEL, _params.frame_label,
    //     FRAME_ICON, xv_create(XV_NULL, ICON, 
    //                           ICON_IMAGE,image1,
    //                           ICON_HEIGHT, 1,
    //                           ICON_WIDTH, 64,
    //                           NULL),
    //     NULL);

    // XSetWindowColormap(gd.dpy,((Window) xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID,NULL)),gd.cmap);
    XSetWindowColormap(gd.dpy,gd.hcan_xid,gd.cmap);
    XSetWindowColormap(gd.dpy,gd.vcan_xid,gd.cmap);

    // xv_set(gd.h_win_horiz_bw->cp,PANEL_ITEM_X_GAP,2,NULL);
    // Setup the Main Menu Bar Cell Labels
//     for(i=1; i <= gd.menu_bar.num_menu_bar_cells; i++) {

//         sprintf(string,"cidd.menu_bar_label%d",i);
// #ifdef NOTNOW
//         resource = gd.uparams->getString(string,"Not Defined");
// #endif

//         // xv_set(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_STRING,i-1,resource,NULL);
//     }
   
    update_ticker(time(0));

    // Set the Jiffy Image Cache Widget labels
    // for(i=0; i < _params.num_cache_zooms; i++) {
    //   xv_set(gd.h_win_horiz_bw->im_cache_st,PANEL_CHOICE_STRING,i," ",NULL);
    // }
     
    // if(_params.num_cache_zooms > 1) {  // Have a visible Image Cache Widget
    //     xv_set(gd.h_win_horiz_bw->cur_time_msg,
    //        XV_X,
    //          (xv_get(gd.h_win_horiz_bw->cp,XV_WIDTH) -
    //          xv_get(gd.h_win_horiz_bw->cur_time_msg,XV_WIDTH) -
    //          xv_get(gd.h_win_horiz_bw->im_cache_st,XV_WIDTH) - 10),
    //        NULL);

    //     xv_set(gd.h_win_horiz_bw->im_cache_st,
    //        XV_X,
    //          xv_get(gd.h_win_horiz_bw->cp,XV_WIDTH) -
    //          xv_get(gd.h_win_horiz_bw->im_cache_st,XV_WIDTH) - 2,
    //        NULL);
    //  } else {  // The current time goes on the right - Image 
    //     xv_set(gd.h_win_horiz_bw->cur_time_msg,
    //        XV_X,
    //          (xv_get(gd.h_win_horiz_bw->cp,XV_WIDTH) -
    //          xv_get(gd.h_win_horiz_bw->cur_time_msg,XV_WIDTH)), NULL);

    //     xv_set(gd.h_win_horiz_bw->im_cache_st, XV_SHOW, FALSE, NULL);
    //  }

	// Bail out on setting all widgets, if running unmapped 
    if(gd.run_unmapped) return;

    // xv_set(gd.movie_pu->movie_type_st, PANEL_VALUE,gd.movie.mode, NULL);

    // Hide the opuput loop button if we have no series save command defined
    if(_params.series_convert_script == NULL) {
      // xv_set(gd.movie_pu->save_loop_bt, XV_SHOW,FALSE, NULL);
    } 

    // xv_set(gd.h_win_horiz_bw->movie_frame_msg, XV_SHOW,TRUE, NULL);

   // Turn on the Winds ON/OFF Button.
   // if(gd.layers.wind_vectors > 0 &&  gd.menu_bar.winds_onoff_bit > 0) {
   //     // value = xv_get(gd.h_win_horiz_bw->main_st,PANEL_VALUE,NULL);
   //     value |= gd.menu_bar.winds_onoff_bit;
   //     // xv_set(gd.h_win_horiz_bw->main_st,PANEL_VALUE,value,NULL);
   //     gd.menu_bar.last_callback_value = value;
   // }

   // Turn on the Land Use ON/OFF Button.
   // if(gd.layers.earth.landuse_active > 0 &&  gd.menu_bar.landuse_onoff_bit > 0) {
   //     // value = xv_get(gd.h_win_horiz_bw->main_st,PANEL_VALUE,NULL);
   //     value |= gd.menu_bar.landuse_onoff_bit;
   //     // xv_set(gd.h_win_horiz_bw->main_st,PANEL_VALUE,value,NULL);
   //     gd.menu_bar.last_callback_value = value;
   // }

   // Turn on the SYMPRODS ON/OFF Button.
   // if(gd.prod.products_on > 0 &&  gd.menu_bar.symprods_onoff_bit > 0) {
   //     // value = xv_get(gd.h_win_horiz_bw->main_st,PANEL_VALUE,NULL);
   //     value |= gd.menu_bar.symprods_onoff_bit;
   //     // xv_set(gd.h_win_horiz_bw->main_st,PANEL_VALUE,value,NULL);
   //     gd.menu_bar.last_callback_value = value;
   // }
   // Turn on the Loop ON/OFF Button.
    // if(gd.movie.movie_on) {
    //     if(gd.menu_bar.loop_onoff_bit > 0) {
    //        // value = xv_get(gd.h_win_horiz_bw->main_st,PANEL_VALUE,NULL);
    //        value |= gd.menu_bar.loop_onoff_bit;
    //        // xv_set(gd.h_win_horiz_bw->main_st,PANEL_VALUE,value,NULL);
    //        gd.menu_bar.last_callback_value = value;
    //     }

    //     // xv_set(gd.movie_pu->start_st, PANEL_VALUE,1, NULL);
    // } else {
    //     // xv_set(gd.movie_pu->start_st, PANEL_VALUE,0, NULL);
    // }


    /* Set the labels on the Bookmark choice_panel  */
    for(i=0; i < _params.bookmarks_n; i++) {
        // xv_set(gd.bookmk_pu->bookmk_st,PANEL_CHOICE_STRING,i,gd.bookmark[i].label,NULL);
    }
    // Size the panel appropriately
    // height = xv_get(gd.bookmk_pu->bookmk_st,XV_HEIGHT);
    // width = xv_get(gd.bookmk_pu->bookmk_st,XV_WIDTH);
    if(width <= 0) width = 100;  // IN cases where no bookmarkds are defined
    // xv_set(gd.bookmk_pu->bookmk_pu, XV_HEIGHT,height, XV_WIDTH, width,NULL);

    /* Set the labels on the Forecast choice_panel  */
    {
    int k;
    int interval = (int) (gd.movie.forecast_interval/ 25.0 + 1.0);
    j = (int) rint(gd.movie.forecast_interval); 
    sprintf(panel_choice_string,"Now");
    // xv_set(gd.fcast_pu->fcast_st,PANEL_CHOICE_STRING,0,panel_choice_string,NULL);
    if(j > 0) { 
      for(i=1 * interval, k=1; i <= j; i+= interval, k++) {
        sprintf(panel_choice_string,"+ %d Hrs",i);
        // xv_set(gd.fcast_pu->fcast_st,PANEL_CHOICE_STRING,k,panel_choice_string,NULL);
      }
    }
    // xv_set(gd.fcast_pu->fcast_st,XV_X,0,XV_Y,0,PANEL_CHOICE_NCOLS,1,NULL);
    // // Size the panel appropriately
    // xv_set(gd.fcast_pu->fcast_pu,XV_HEIGHT,
    //        xv_get(gd.fcast_pu->fcast_st,XV_HEIGHT),NULL);
    // xv_set(gd.fcast_pu->fcast_pu,XV_WIDTH,
    //        xv_get(gd.fcast_pu->fcast_st,XV_WIDTH),NULL);

    }

    /* Set the labels on the Past choice_panel  */
    {
    int k;
    int interval = (int) (gd.movie.past_interval/ 25.0 + 1.0);
    j = (int) rint(gd.movie.past_interval); 
    sprintf(panel_choice_string,"Now");
    // xv_set(gd.past_pu->past_hr_st,PANEL_CHOICE_STRING,0,panel_choice_string,NULL);
    if(j > 0) { 
      for(i=1 * interval, k=1; i <= j; i+= interval, k++) {
        sprintf(panel_choice_string,"- %d Hrs",i);
        // xv_set(gd.past_pu->past_hr_st,PANEL_CHOICE_STRING,k,panel_choice_string,NULL);
      }
    }
    // xv_set(gd.past_pu->past_hr_st,XV_X,0,XV_Y,0,PANEL_CHOICE_NCOLS,1,NULL);

    // Size the panel appropriately
    // xv_set(gd.past_pu->past_pu,XV_HEIGHT,
    //        xv_get(gd.past_pu->past_hr_st,XV_HEIGHT),NULL);
    // xv_set(gd.past_pu->past_pu,XV_WIDTH,
    //        xv_get(gd.past_pu->past_hr_st,XV_WIDTH),NULL);

    }

    if(gd.layers.route_wind.has_params) {
        /* Set the labels on the Route panel " */
        for(i=0; i < gd.layers.route_wind.num_predef_routes; i++) {
            // xv_set(gd.route_pu->route_st,PANEL_CHOICE_STRING,i,
            // gd.layers.route_wind.route[i].route_label,NULL);
        }
        // xv_set(gd.route_pu->route_st,PANEL_CHOICE_STRING,
	//    gd.layers.route_wind.num_predef_routes,"Custom",NULL);
        // xv_set(gd.route_pu->route_pu,
    	//     XV_HEIGHT, xv_get(gd.route_pu->route_st,XV_HEIGHT) + 4,
        //     XV_WIDTH, xv_get(gd.route_pu->route_st,XV_WIDTH) + 4,
	//     XV_LABEL, "Routes...",
	//     NULL);


	// The first route is the default in this case
        // xv_set(gd.route_pu->route_st,PANEL_VALUE,0,NULL);
    } else {
	// Hide the label checkboxes
	// xv_set(gd.route_pu->route_st,XV_SHOW,FALSE,NULL);
	// Hide the route popup selection
	// xv_set(gd.v_win_v_win_pu->route_st,XV_SHOW,FALSE,NULL);
    }

  // xv_set (gd.v_win_v_win_pu->route_msg,PANEL_LABEL_STRING," ",NULL);
  // xv_set (gd.v_win_v_win_pu->hazard_msg,PANEL_LABEL_STRING," ",NULL);

    /* Set the labels on the Zoom choice_panel " */
    for(i=0; i < gd.h_win.num_zoom_levels-NUM_CUSTOM_ZOOMS -1; i++) {
        sprintf(panel_choice_string,"%d",i+1);
        // sprintf(string,"cidd.level%d_label",i+1);
        // sprintf(panel_choice_string,"%s",gd.uparams->getString(string , panel_choice_string));
        strncpy(panel_choice_string, _params._zoom_levels[i].label, 255);
        // xv_set(gd.zoom_pu->domain_st,PANEL_CHOICE_STRING,i,panel_choice_string,NULL);
    }

    for(i=gd.h_win.num_zoom_levels-NUM_CUSTOM_ZOOMS-1, j=1; i < gd.h_win.num_zoom_levels-1; i++, j++) {
        sprintf(panel_choice_string,"Goto %d",j);
        // xv_set(gd.zoom_pu->domain_st,PANEL_CHOICE_STRING,
        //       i,panel_choice_string,NULL);
    }

    // xv_set(gd.zoom_pu->domain_st,PANEL_CHOICE_STRING,
    //        gd.h_win.num_zoom_levels-1,
    //        "Current",NULL);

    for(i=gd.h_win.num_zoom_levels, j=1; j <= NUM_CUSTOM_ZOOMS ; i++, j++) {
        sprintf(panel_choice_string,"Save %d",j);
        // xv_set(gd.zoom_pu->domain_st,PANEL_CHOICE_STRING,
        //       i,panel_choice_string,NULL);
    }

    // xv_set(gd.zoom_pu->domain_st, PANEL_VALUE, gd.h_win.zoom_level,XV_X,0,XV_Y,0, NULL);
    // Size the panel appropriately
    // xv_set(gd.zoom_pu->zoom_pu,XV_HEIGHT,xv_get(gd.zoom_pu->domain_st,XV_HEIGHT),NULL);
    // xv_set(gd.zoom_pu->zoom_pu,XV_WIDTH,xv_get(gd.zoom_pu->domain_st,XV_WIDTH),NULL);
        /* Set the labels on the Overlay choice_panel */
    value = 0;

	int total = 0;  // Total unique menu tiems.
	int used = 0;  // Check for duplicate labels
    if(gd.num_map_overlays <= 32) {                      // Use a simple Choice widget
      for(i=0; i < gd.num_map_overlays && i < 32; i++) {
	    used = 0;
		for( j=0; j < i; j++) {
                  if(gd.over[j]->control_label == gd.over[i]->control_label) {
                    used = 1;
                  }
		}
			
		if(!used) { // Dont use duplicate labels.
            // xv_set(gd.over_pu->over_pu_st,PANEL_CHOICE_STRING,total,gd.over[i]->control_label,NULL);
            if(gd.over[i]->active) value |= 1 <<total;
			total++;
		}
      }
      // Set the correct buttons up/down
      // xv_set(gd.over_pu->over_pu_st, PANEL_VALUE, value,XV_X,0,XV_Y,0, NULL);

      // Hide the scrolling list
      // xv_set(gd.over_pu->over_lst,XV_SHOW,FALSE,NULL);

      // Make the window fit the choice item
      // xv_set(gd.over_pu->over_pu,XV_HEIGHT,xv_get(gd.over_pu->over_pu_st,XV_HEIGHT),NULL);
      // xv_set(gd.over_pu->over_pu,XV_WIDTH,xv_get(gd.over_pu->over_pu_st,XV_WIDTH),NULL);

    } else {     // Use a scrolling list
	for(i=0; i < gd.num_map_overlays; i++) {
	    used = 0;
		for( j=0; j < i; j++) {
                  if(gd.over[j]->control_label == gd.over[i]->control_label) {
                    used = 1;
                  }
		}
		if(!used) {  //  Don't use duplicate labels.
	      // xv_set(gd.over_pu->over_lst,
	      //      PANEL_LIST_INSERT, total,
	      //      PANEL_LIST_STRING, total, gd.over[i]->control_label,
	      //      PANEL_LIST_CLIENT_DATA, total, i,
	      //      NULL);
                  // if(gd.over[i]->active) xv_set(gd.over_pu->over_lst, PANEL_LIST_SELECT, total, TRUE, NULL);
		  total++;
		}
	}

	// Make the scrollbar a reasonable length
        // // xv_set(gd.over_pu->over_lst,PANEL_LIST_DISPLAY_ROWS,24,NULL);

        // // Hide the choice widget list
	// xv_set(gd.over_pu->over_pu_st,XV_SHOW,FALSE,NULL);

      // Make the window fit the choice item
      // xv_set(gd.over_pu->over_pu,XV_HEIGHT,xv_get(gd.over_pu->over_lst,XV_HEIGHT),NULL);
      // xv_set(gd.over_pu->over_pu,XV_WIDTH,xv_get(gd.over_pu->over_lst,XV_WIDTH),NULL);
    }


    /* determine margin and other window assoc sizes for VERT window */
    gd.v_win.margin.top = _params.vert_top_margin;
    gd.v_win.margin.bot =  _params.vert_bot_margin;
    gd.v_win.margin.left = _params.vert_left_margin;
    gd.v_win.margin.right = _params.vert_right_margin;

    if (_params.vert_legends_start_x == 0) {
      gd.v_win.legends_start_x = gd.v_win.margin.left + 5;
    } else {
      gd.v_win.legends_start_x = _params.vert_legends_start_x;
    }
    
    if (_params.vert_legends_start_y == 0) {
      gd.v_win.legends_start_y = gd.v_win.margin.top * 2;
    } else {
      gd.v_win.legends_start_y = _params.vert_legends_start_y;
    }

    if (_params.vert_legends_delta_y == 0) {
      gd.v_win.legends_delta_y = gd.v_win.margin.top;
    } else {
      gd.v_win.legends_delta_y = _params.vert_legends_delta_y;
    }

    gd.v_win.min_height = _params.vert_min_height;
    gd.v_win.min_width = _params.vert_min_width;
    if(gd.v_win.min_width < 280) {
      // Don't let hidden XView widgets obsure others 
      fprintf(stderr,"Warning: Minimum X-Section width minimum of 280 being enforced.\n");
      gd.v_win.min_width = 280;
    }
    gd.v_win.active = 0;

    sprintf(string,"%g",gd.v_win.min_ht);
    // xv_set(gd.v_win_v_win_pu->scale_base_tx,PANEL_VALUE,string,NULL);
    sprintf(string,"%g",gd.v_win.max_ht);
    // xv_set(gd.v_win_v_win_pu->ht_top_tx,PANEL_VALUE,string,NULL);
    
    // xv_set(gd.v_win_v_win_pu->v_win_pu,
    //        WIN_HEIGHT, _params.vert_default_height,
    //        WIN_WIDTH,  _params.vert_default_width,
    //        FRAME_SHOW_HEADER,    TRUE,
    //        XV_SHOW,    FALSE,
    //        NULL);

    // Create a color map segmet for the vert panel widgets.
    // static Xv_singlecolor colors[] = {
    //     {255, 0, 0},  // red  (CMS_CONTROL_COLORS + 0) 
    //     {0, 255, 0},  // green  (CMS_CONTROL_COLORS + 1) 
    //     {0, 0, 255},  // blue  (CMS_CONTROL_COLORS + 2) 
    //     {0, 0, 0},   // black  (CMS_CONTROL_COLORS + 3) 
    // };

    // Cms cms = xv_create(0,CMS,CMS_CONTROL_CMS,TRUE,
    //     		     CMS_SIZE, CMS_CONTROL_COLORS + 4,
    //     		     CMS_COLORS, colors,
    //     		     NULL);

    // xv_set(gd.v_win_v_win_pu->controls1, WIN_CMS, cms, NULL);

    if(gd.layers.route_wind.has_params) {
	// Set the initial state of the route winds label controls
	int pval = 0;
	if( _params.route_add_waypoints_labels) pval |= 1;
	if( _params.route_add_wind_text) pval |= 2;
	// xv_set(gd.v_win_v_win_pu->route_labels_st,PANEL_VALUE,pval,NULL);
    } else {
	// xv_set(gd.v_win_v_win_pu->route_labels_st,XV_SHOW,FALSE,NULL);
	// xv_set(gd.v_win_v_win_pu->route_labels_st,XV_SHOW,FALSE,NULL);
    }


    /* Build the Field menus & lists */
    for (i = 0, render_line = 0; i < gd.num_datafields; i++) {

        // xv_set(gd.fields_pu->display_list,
        //        PANEL_LIST_INSERT, i,
        //        PANEL_LIST_STRING, i, gd.mrec[i]->button_name,
        //        PANEL_LIST_CLIENT_DATA, i, i,
        //        NULL);

        if (gd.mrec[i]->currently_displayed) {
            // xv_set(gd.fields_pu->display_list,
            //        PANEL_LIST_SELECT, i, TRUE,
            //        NULL);

            render_line++;
        }

    }

    // if(gd.gui_P->field_list_n > 1) { 
    //     for(i=0; i < gd.gui_P->field_list_n ; i++) {
    //         // xv_set(gd.data_pu->group_list,
    //         //     	PANEL_LIST_INSERT, i,
    //         //     	PANEL_LIST_STRING, i, gd.gui_P->_field_list[i].id_label,
    //         //     	PANEL_LIST_CLIENT_DATA, i, i,
    //         //     	NULL);
    //     }
    //     // xv_set(gd.data_pu->group_list,XV_SHOW,TRUE,XV_X,0, XV_Y,0,NULL);
    //     // xv_set(gd.data_pu->group_list,PANEL_LIST_SELECT,0,TRUE,NULL);
    // } else {
    //     // xv_set(gd.data_pu->group_list,XV_SHOW,FALSE,XV_X,0, XV_Y,0,NULL);
    // }

    // /* Build all of the field menu's and choice panels. */
    // if(gd.gui_P->field_list_n > 1 ) {
    //   // set_group_proc(0,(char *) NULL, (Xv_opaque) NULL, (Panel_list_op) 0,(Event *) NULL, 0);
    // }else {
    //   init_field_menus();
    // }

    /* make sure there is at least one field in the choice lists */
    if (render_line <= 0) {
        gd.mrec[0]->currently_displayed = TRUE;
        // xv_set(gd.fields_pu->display_list,
        //        PANEL_LIST_SELECT, 0, TRUE,
        //        NULL);
    }

    /* set the current field for each window */
    gd.h_win.page = gd.v_win.page = gd.field_index[0];

    // xv_set(gd.page_pu->density_sl,
    //     PANEL_VALUE,1,
    //     NULL);

    // xv_set(gd.page_pu->prod_line_sl,
    //     PANEL_VALUE,gd.prod.prod_line_width,
    //     NULL);

    // xv_set(gd.page_pu->prod_font_sl,
    //     PANEL_MAX_VALUE,    _params.num_fonts -1 ,
    //     PANEL_VALUE,gd.prod.prod_font_num,
    //     NULL);

    // xv_set(gd.page_pu->dim_sl,
    //     PANEL_MAX_VALUE,    _params.inten_levels,
    //     PANEL_VALUE,(int)(_params.inten_levels * _params.data_inten),
    //     PANEL_NOTIFY_LEVEL,PANEL_DONE,
    //     NULL);

    // xv_set(gd.h_win_horiz_bw->cur_ht_msg,
    //        PANEL_LABEL_STRING, "",
    //        XV_X, (xv_get(gd.h_win_horiz_bw->cp,XV_WIDTH) / 3),
    //        NULL);


    /* set page configuration popup parameters */
    // xv_set(gd.page_pu->page_pu,
    //        XV_SHOW, FALSE,
    //        FRAME_CMD_PUSHPIN_IN,    TRUE,
    //        FRAME_SHOW_HEADER, TRUE,
    //        NULL);

    /* Fill in WIND Vector scrolling List */
    for (i = 0; i < gd.layers.num_wind_sets; i++ ) {
        sprintf(str,"%s  %s",
            gd.layers.wind[i].wind_u->button_name,
            gd.layers.wind[i].color->name);

        // xv_set(gd.page_pu->wind_on_list,
        //                 PANEL_LIST_STRING, i, str,
        //                 PANEL_LIST_SELECT, i, gd.layers.wind[i].active,
        //                 NULL); 
                                      
    }

    // xv_set(gd.page_pu->wind_sl,
    //        PANEL_VALUE, gd.layers.wind_scaler -1,
    //        NULL);
    sprintf(str,"%g min",gd.layers.wind_time_scale_interval * gd.layers.wind_scaler);
    // xv_set(gd.page_pu->wind_msg,
    //        PANEL_LABEL_STRING, str,
    //        NULL);

    // xv_set(gd.page_pu->ref_mark_color_msg,
    //        PANEL_LABEL_STRING, gd.legends.range_ring_color->name,
    //        NULL);


    /* Set default condition of reference marker setting */
    value = 0;
    if (gd.legends.range) value |= RANGE_BIT;
    // xv_set(gd.page_pu->ref_st,
    //        PANEL_VALUE, value,
    //        NULL);

    // xv_set(gd.page_pu->land_use_st,
    //        PANEL_VALUE, gd.layers.earth.landuse_active,
    //        NULL);

    // xv_set(gd.page_pu->topo_st,
    //        PANEL_VALUE, gd.layers.earth.terrain_active,
    //        NULL);

     if(gd.layers.earth.land_use) 
         // xv_set(gd.fields_pu->land_use_url_tx,PANEL_VALUE,gd.layers.earth.land_use->url,NULL);
     if(gd.layers.earth.terr) 
         // xv_set(gd.fields_pu->topo_url_tx,PANEL_VALUE,gd.layers.earth.terr->url,NULL);


    /* Fill in Geographic Overlay Scrolling List */
    if(gd.debug) printf("%d Overlays sets found\n",gd.num_map_overlays);
    for(i=0; i < gd.num_map_overlays; i++) {
      sprintf(string,"%s  %s",
              gd.over[i]->control_label.c_str(),
              gd.over[i]->color_name.c_str());
        // xv_set(gd.page_pu->overlay_list,
        //     PANEL_LIST_STRING, i, string,
        //     PANEL_LIST_CLIENT_DATA, i,i,
        //     PANEL_LIST_SELECT, i, gd.over[i]->active,
        //     NULL);
    }

    // Set the on/off state of the legend plotting controls
    // xv_set(gd.page_pu->layer_legend_st,
    //        PANEL_VALUE, gd.layers.layer_legends_on,
    //        NULL);

    // xv_set(gd.page_pu->cont_legend_st,
    //        PANEL_VALUE, gd.layers.cont_legends_on,
    //        NULL);

    // xv_set(gd.page_pu->wind_legend_st,
    //        PANEL_VALUE, gd.layers.wind_legends_on,
    //        NULL);

    /* set movie popup parameters */

    if(gd.movie.num_frames > 1) {
      // xv_set(gd.movie_pu->movie_frame_sl,
      //   PANEL_MIN_VALUE, 1,
      //   PANEL_MAX_VALUE,    gd.movie.num_frames,
      //   PANEL_VALUE,        gd.movie.cur_frame + 1,
      //   NULL);
    } else {
      // xv_set(gd.movie_pu->movie_frame_sl, XV_SHOW, FALSE,NULL);
    }

    // xv_set(gd.movie_pu->movie_dwell_sl,
    //     PANEL_MIN_VALUE, 0,
    //     PANEL_MAX_VALUE,    10,
    //     PANEL_VALUE,        2,
    //     NULL);


    // xv_set(gd.movie_pu->movie_speed_sl,
    //     PANEL_MIN_VALUE, 0,
    //     PANEL_MAX_VALUE,    MOVIE_SPEED_RANGE,
    //     PANEL_VALUE,        (MOVIE_SPEED_RANGE - (gd.movie.display_time_msec/ MOVIE_DELAY_INCR)),
    //     NULL);

    if( gd.movie.movie_on ) {
        // xv_set(gd.movie_pu->start_st, PANEL_VALUE,1, NULL);
    } else {
        // xv_set(gd.movie_pu->start_st, PANEL_VALUE,0, NULL);
    }


    reset_time_points();

    // Initialize the Timplot class 
    gd.time_plot = new TimePlot(*gd.legends.background_color,
			     *gd.legends.time_axis_color,
			     *gd.legends.time_frame_color,
			     *gd.legends.epoch_indicator_color,
			     *gd.legends.now_time_color,
			     gd.legends.time_tick_color[0],
			     gd.legends.time_tick_color[1],
			     gd.legends.time_tick_color[2],
			     gd.legends.time_tick_color[3],
			     gd.legends.time_tick_color[4],
			     gd.legends.time_tick_color[5]);

    gd.time_plot->Init(gd.dpy,gd.def_gc,gd.cmap /*,gd.movie_pu->time_can */);

    update_movie_popup();

    update_frame_time_msg(gd.movie.cur_frame);

    // xv_set(gd.movie_pu->movie_pu,
    //        XV_SHOW, FALSE,
    //        FRAME_CMD_PUSHPIN_IN, TRUE,
    //        FRAME_SHOW_HEADER, TRUE,
    //        NULL);

    /* set fields popup parameters */
    // xv_set(gd.fields_pu->fields_pu,
    //        XV_SHOW, FALSE,
    //        FRAME_CMD_PUSHPIN_IN, TRUE,
    //        FRAME_SHOW_HEADER, TRUE,
    //        NULL);

    /* set data selector popup parameters */
    // xv_set(gd.data_pu->data_pu,
    //        XV_SHOW, FALSE,
    //        FRAME_CMD_PUSHPIN_IN, TRUE,
    //        FRAME_SHOW_HEADER, TRUE,
    //        NULL);


    /* set overlay selector popup parameters */
    // xv_set(gd.over_pu->over_pu,
    //        XV_SHOW, FALSE,
    //        FRAME_CMD_PUSHPIN_IN, TRUE,
    //        FRAME_SHOW_HEADER, TRUE,
    //        NULL);

    /* set zoom level popup parameters */
    // xv_set(gd.zoom_pu->zoom_pu,
    //        XV_SHOW, FALSE,
    //        FRAME_CMD_PUSHPIN_IN, TRUE,
    //        FRAME_SHOW_HEADER, TRUE,
    //        NULL);

    /* set Route popup parameters */
    // xv_set(gd.route_pu->route_pu,
    //        XV_SHOW, FALSE,
    //        FRAME_CMD_PUSHPIN_IN, TRUE,
    //        FRAME_SHOW_HEADER, TRUE,
    //        NULL);

    // xv_set(gd.h_win_horiz_bw->canvas1, XV_HELP_DATA,
    //        // WIN_CURSOR, cursor,
    //        gd.uparams->getString( "cidd.canvas_help", "cidd_help:canvas"),
    //        NULL);

    // xv_set(gd.h_win_horiz_bw->movie_frame_msg, XV_HELP_DATA,
    //        gd.uparams->getString( "cidd.frame_message_help", "cidd_help:frame_message"),
    //        NULL);

    update_layered_contour_panel();
    update_layered_field_panel();

    /* Set the labels on the Draw_export choice item  */
    for(i=0; i < gd.draw.num_draw_products; i++) {
        sprintf(panel_choice_string,"%s",gd.draw.dexport[i].product_id_label);
        // xv_set(gd.draw_pu->type_st,PANEL_CHOICE_STRING,i,panel_choice_string,NULL);
    }
    // Set text fields in panel
    //update_draw_export_panel();

}

/*****************************************************************
 * INIT_FIELD_MENUS();
 *
 */


void init_field_menus()
{
    int i;
#ifdef NOTNOW
    int x,y;
    int ncols;
#endif

    /* Clear out any old items */
    // xv_set(gd.data_pu->data_st, PANEL_CHOICE_STRINGS, "", NULL, NULL);

    gd.num_menu_fields = 0;
    for(i=0; i < gd.num_datafields; i++) {
        if(gd.mrec[i]->currently_displayed) {
           // xv_set(gd.data_pu->data_st, PANEL_CHOICE_STRING,gd.num_menu_fields, gd.mrec[i]->button_name,NULL);
           gd.field_index[gd.num_menu_fields] = i;
           gd.num_menu_fields++;
        }
    }
#ifdef NOTNOW
    ncols = (_params.num_field_menu_cols <=0 ) ? (int)(gd.num_menu_fields / 30.0) + 1
                                         : _params.num_field_menu_cols;
#endif

//     if(gd.gui_P->field_list_n > 1 ) {
//         // xv_set(gd.data_pu->data_st,XV_X,0,
// 	//        XV_Y, xv_get(gd.data_pu->group_list,XV_HEIGHT) + 4,
// 	//        PANEL_CHOICE_NCOLS,ncols,NULL);

//         // int x1 = xv_get(gd.data_pu->group_list,XV_WIDTH);
//         // int x2 = xv_get(gd.data_pu->data_st,XV_WIDTH);
// #ifdef NOTNOW
//       int x1 = 0;
//       int x2 = 0;
//       x = x1 > x2 ? x1 : x2;
// #endif

//         // y = xv_get(gd.data_pu->data_st,XV_HEIGHT) +  xv_get(gd.data_pu->group_list,XV_HEIGHT) + 4;
//         // xv_set(gd.data_pu->data_st,PANEL_LIST_WIDTH,x,NULL);


//     } else {
//         // xv_set(gd.data_pu->data_st,XV_X,0,XV_Y,0,PANEL_CHOICE_NCOLS,ncols,NULL);
//         // x = xv_get(gd.data_pu->data_st,XV_WIDTH);
//         // y = xv_get(gd.data_pu->data_st,XV_HEIGHT);
// 	//     xv_set(gd.data_pu->group_list,XV_Y,y,NULL);
//     }

    // xv_set(gd.data_pu->data_pu,XV_HEIGHT,y,XV_WIDTH,x,NULL);

}

/////////////////////////////////////////////////////////////////////
// FRAME_TIME_MSG
//
const char * frame_time_msg(int index)
{
    struct tm *gmt = NULL;
    struct tm *gmt_s;
    struct tm *gmt_e;
    struct tm res;
    struct tm res_e;
    struct tm res_s;
    met_record_t    *mr;
    char    label1[32];
    char    label2[64];
    char    label3[64];
    char    label4[64];
    static char    label[256];         
    char str_fmt[64];
    
    mr = gd.mrec[gd.h_win.page];

    if(mr == NULL) return NULL;

    // IN HTML_MODE or remote ui mode, use an abbreviated label
    if(_params.html_mode || gd.remote_ui) {
        if(mr->h_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST) { 
             if(_params.use_local_timestamps) {
                 gmt = localtime_r((time_t *)&(mr->h_mhdr.time_gen),&res); 
			 } else {
                 gmt = gmtime_r((time_t *)&(mr->h_mhdr.time_gen),&res);
             }
             strftime(label3,64,_params.label_time_format,gmt);
             sprintf(label,"FORECAST Run: %s",label3);                                    
	} else {
	    label[0] = '\0';  // No label
	}

    // Use Full label
    } else {
         /* Frame Begin label */
         if(_params.use_local_timestamps) {
			 switch (_params.gather_data_mode) {
				 case CLOSEST_TO_FRAME_CENTER:
                     gmt = localtime_r(&gd.movie.frame[index].time_mid,&res);
				 break;
				 case FIRST_BEFORE_END_OF_FRAME:
                     gmt = localtime_r(&gd.movie.frame[index].time_end,&res);
				 break;
				 case FIRST_AFTER_START_OF_FRAME:
                     gmt = localtime_r(&gd.movie.frame[index].time_start,&res);
				 break;
			 }
			 gmt_s = localtime_r(&gd.movie.frame[index].time_start,&res_s);
             gmt_e = localtime_r(&gd.movie.frame[index].time_end,&res_e);
         } else {
			 switch (_params.gather_data_mode) {
				 case CLOSEST_TO_FRAME_CENTER:
                     gmt = gmtime_r(&gd.movie.frame[index].time_mid,&res);
				 break;
				 case FIRST_BEFORE_END_OF_FRAME:
                     gmt = gmtime_r(&gd.movie.frame[index].time_end,&res);
				 break;
				 case FIRST_AFTER_START_OF_FRAME:
                     gmt = gmtime_r(&gd.movie.frame[index].time_start,&res);
				 break;
			 }
			 gmt_s = gmtime_r(&gd.movie.frame[index].time_start,&res_s);
             gmt_e = gmtime_r(&gd.movie.frame[index].time_end,&res_e);
         }
         sprintf(label1,"Frame %d:", gd.movie.cur_frame + 1);
         strftime(label2,64,_params.label_time_format,gmt);
	 sprintf(str_fmt, "   (%s to", _params.frame_range_time_format);
         strftime(label3, 64, str_fmt, gmt_s);
	 sprintf(str_fmt, "%s)", _params.frame_range_time_format);
         strftime(label4, 64, str_fmt, gmt_e);

         if(mr->h_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST && gd.model_run_time != 0 ) {
             if(_params.use_local_timestamps) {
	             gmt = localtime_r(&gd.model_run_time,&res);
             } else {
	             gmt = gmtime_r(&gd.model_run_time,&res);
             }
             strftime(label3,64,_params.label_time_format,gmt);
             sprintf(label,"%s %s Run: %s",label1,label2,label3);                                    
         } else {
             sprintf(label,"%s %s %s %s",label1,label2,label3,label4);                                    
         }
    }

    return label;
}

/////////////////////////////////////////////////////////////////////
// UPDATE_FRAME_TIME_MSG
//
void update_frame_time_msg(int index)
{

#ifdef NOTNOW
  
    const char *label;
     
    label = frame_time_msg(index);
#endif

    // xv_set(gd.movie_pu->fr_begin_msg, PANEL_LABEL_STRING, label, NULL);
    // xv_set(gd.h_win_horiz_bw->movie_frame_msg, PANEL_LABEL_STRING, label, NULL);
}
