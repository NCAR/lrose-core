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
 * ZOOM_PU_PROC.C - Notify and event callback function stubs.
 */

#define ZOOM_PU_PROC 1

#include "cidd.h"

#ifdef NOTNOW

/*************************************************************************
 * Notify callback function for `domain_st'.
 */
void set_domain_proc(Panel_item item, int value, Event *event)
{

  // save current zoom

  save_current_zoom(gd.h_win.cmin_x, gd.h_win.cmin_y, 
                    gd.h_win.cmax_x, gd.h_win.cmax_y);
  
  if(value < gd.h_win.num_zoom_levels) {
    gd.h_win.zoom_level = value;

    /* get latest zoom coordinates */
    gd.h_win.cmin_x = gd.h_win.zmin_x[value];
    gd.h_win.cmax_x = gd.h_win.zmax_x[value];
    gd.h_win.cmin_y = gd.h_win.zmin_y[value];
    gd.h_win.cmax_y = gd.h_win.zmax_y[value];

    /* if called "externally" make sure choice item is set properly */
    if(event == NULL) xv_set(item,PANEL_VALUE,value,NULL);

  } else {
    if(value >= gd.h_win.num_zoom_levels) {
       value = value - (NUM_CUSTOM_ZOOMS +1);
       gd.h_win.zmin_x[value] = gd.h_win.cmin_x;
       gd.h_win.zmax_x[value] = gd.h_win.cmax_x;
       gd.h_win.zmin_y[value] = gd.h_win.cmin_y;
       gd.h_win.zmax_y[value] = gd.h_win.cmax_y;
    }
    xv_set(item,PANEL_VALUE,value,NULL);
  }

  set_domain_zoom(gd.h_win.cmin_x, gd.h_win.cmin_y,
                  gd.h_win.cmax_x, gd.h_win.cmax_y);

  if(event != NULL) gd.coord_expt->pointer_seq_num++; 

}

#endif

/*************************************************************************
 * Notify callback function for `domain_st'.
 */

void set_domain_zoom(double zoom_min_x, double zoom_min_y,
                     double zoom_max_x, double zoom_max_y)
  
{

  double lat,lon;
  struct timeval tp;
  struct timezone tzp;
  met_record_t *mr;
  gettimeofday(&tp, &tzp); 

  /* set latest zoom coordinates */

  gd.h_win.cmin_x = zoom_min_x;
  gd.h_win.cmax_x = zoom_max_x;
  gd.h_win.cmin_y = zoom_min_y;
  gd.h_win.cmax_y = zoom_max_y;

  int index = gd.h_win.zoom_level;
  gd.h_win.zmin_x[index] = zoom_min_x;
  gd.h_win.zmax_x[index] = zoom_max_x;
  gd.h_win.zmin_y[index] = zoom_min_y;
  gd.h_win.zmax_y[index] = zoom_max_y;

  mr = choose_ht_sel_mr(gd.h_win.page);
  
  gd.coord_expt->button = 0;
  gd.coord_expt->selection_sec = tp.tv_sec;
  gd.coord_expt->selection_usec = tp.tv_usec;
  gd.coord_expt->pointer_x = (gd.h_win.cmin_x + gd.h_win.cmax_x) / 2.0;
  gd.coord_expt->pointer_y = (gd.h_win.cmin_y + gd.h_win.cmax_y) / 2.0;
  
  gd.aspect_correction = cos(gd.coord_expt->pointer_y * DEG_TO_RAD);

  gd.proj.xy2latlon(gd.coord_expt->pointer_x,gd.coord_expt->pointer_y,lat,lon);

  gd.coord_expt->pointer_lon = lon;
  gd.coord_expt->pointer_lat = lat;
  gd.coord_expt->pointer_alt_min = gd.h_win.cur_ht;
  gd.coord_expt->pointer_alt_max = gd.h_win.cur_ht;
  gd.coord_expt->pointer_ht_km = gd.h_win.cur_ht;
  gd.coord_expt->data_altitude = mr->vert[mr->plane].cent;
  gd.coord_expt->click_type = CIDD_ZOOM_CLICK;
  gd.last_event_time = tp.tv_sec;

  double min_lat,max_lat,min_lon,max_lon;
  get_bounding_box(min_lat,max_lat,min_lon,max_lon);
  
  gd.r_context->set_clip_limits(min_lat, min_lon, max_lat, max_lon);
  gd.prod_mgr->reset_product_valid_flags_zoom();

  set_redraw_flags(1,0);
  if(!_params.always_get_full_domain) {
    reset_time_list_valid_flags();
    reset_data_valid_flags(1,0);
    reset_terrain_valid_flags(1,0);
  }

}

#ifdef NOTNOW

#define CMD_BUF_LEN 2048
/*************************************************************************
 * Notify callback function for `bookmark_st'.
 */
void bookmark_proc(Panel_item item, int value, Event *event)
{
    int len;
    char cmd_buf[CMD_BUF_LEN];
    char msg[CMD_BUF_LEN];
    char tail_buf[(CMD_BUF_LEN/2)];
    char *ptr,*ptr2;


    strncpy(cmd_buf,_params.bookmark_command,CMD_BUF_LEN);

    if((ptr = strstr(cmd_buf,"%U")) == NULL) return;

    *ptr = '\0'; // terminate for later concatination
    len = strlen(cmd_buf);

    ptr2 = ptr + 2; // skip over the %U
    strncpy(tail_buf,ptr2,(CMD_BUF_LEN/2)); // copy the tailing portion

    strncat(cmd_buf,gd.bookmark[value].url,(CMD_BUF_LEN - len));
    len = strlen(cmd_buf);

    strncat(cmd_buf,tail_buf,(CMD_BUF_LEN - len));
    len = strlen(cmd_buf);

    strncat(cmd_buf," &",(CMD_BUF_LEN - len));

    if(gd.debug) fprintf(stderr,"Executing: %s\n",cmd_buf);

    sprintf(msg,"Sending Browser to %s",gd.bookmark[value].url);
    gui_label_h_frame(msg,1);

    safe_system(cmd_buf,_params.simple_command_timeout_secs);

    // Pop back up the Menu bar button
    gd.menu_bar.last_callback_value &= ~gd.menu_bar.show_bookmark_menu_bit;
    xv_set(item,PANEL_VALUE,gd.menu_bar.last_callback_value,NULL);

    // Hide the menu 
    xv_set(gd.bookmk_pu->bookmk_pu,FRAME_CMD_PUSHPIN_IN, FALSE, XV_SHOW, FALSE, NULL); 
}

#endif
