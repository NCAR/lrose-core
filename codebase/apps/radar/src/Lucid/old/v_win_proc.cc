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
 * V_WIN_PROC: Callback procedures for the Vertical display window
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */


#define V_WIN_PROC 1
#include "cidd.h"

/*************************************************************************
 * Set field for the vertical cross section 
 */
void set_v_field(int field_no)
{
    int    i;
	int tmp;
	static int last_field = 0;
    
	if(field_no < 0) {
		tmp = last_field;
		last_field = gd.v_win.page;
		gd.v_win.page = tmp;
	} else {
      last_field = gd.v_win.page;
      gd.v_win.page = field_no;
	}

    for(i=0; i < gd.num_datafields; i++) {
        if(gd.mrec[i]->auto_render == 0) gd.v_win.redraw_flag[i] = 1;
    }

    for(i=0; i < MAX_FRAMES; i++) {
        gd.movie.frame[i].redraw_vert = 1;
    }

    if(gd.movie.movie_on ) reset_data_valid_flags(0,1);
}

#ifdef NOTNOW

/*************************************************************************
 * Event callback function for `controls1'.
 */
Notify_value
v_pan_event_proc( Xv_window win, Event *event, Notify_arg  arg, Notify_event_type type)
{
    if(event_id(event) >= 49 && event_id(event) <= 49 +gd.num_datafields) {
        gd.v_win.page = event_id(event) - 49;
    }

    return notify_next_event_func(win, (Notify_event) event, arg, type);
}
 
/*************************************************************************
 * SET_V_FIELD_PROC
 */
void set_v_field_proc( Menu menu, Menu_item item)
{
    // Use unused parameters
    menu = 0;

    set_v_field((int) xv_get(item,XV_KEY_DATA,MENU_KEY));
}
 
/*************************************************************************
 * Menu handler for `v_field_mu'.
 */
Menu v_field_mu_gen_proc(Menu menu, Menu_generate op)
{
    int    i;
    Menu_item mi;
    static Menu field_menu;

    // Use unused parameters
    menu = 0;


        switch (op) {
        case MENU_DISPLAY:
            if(field_menu == 0) {
                field_menu = xv_create(0,MENU,NULL);
            }
            /* Remove any old items */
            for(i=(int) xv_get(field_menu,MENU_NITEMS) ; i > 0; i--) {
                xv_set(field_menu,MENU_REMOVE,i,NULL);
                xv_destroy(xv_get(field_menu,MENU_NTH_ITEM,i));
            }

            /* Build the field menu */
            for(i=0; i < gd.num_datafields; i++) {
              if(gd.mrec[i]->currently_displayed) {
                  mi = xv_create(0,MENUITEM,
                    MENU_STRING, gd.mrec[i]->button_name,
                    MENU_NOTIFY_PROC,    set_v_field_proc,
                    XV_KEY_DATA,    MENU_KEY, i,
                    NULL);
                  xv_set(field_menu,MENU_APPEND_ITEM,mi,NULL);
              }
            }

                break;

        case MENU_DISPLAY_DONE:
                break;

        case MENU_NOTIFY:
                break;
 
        case MENU_NOTIFY_DONE:
                break;
        }
        return field_menu;
}

/*************************************************************************
 * Notify callback function for `route_st'.
 */
void
show_route_pu_proc(Panel_item item, unsigned int value, Event *event)
{
    Window  xid;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
    int v_x,v_y;
    int x_pos,y_pos;
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */

    static int first_time = 1;

      if(value) {
        if(1) { /* posistion this panel every time */
             first_time = 0;

             xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);
             /* Windows position on xv_get is relative to the parent */
             /* so find out where the parent window is */
             XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
             XFree((caddr_t)children);
             XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
             /* take parents postion to get pos relative to root win */
             XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth); 
	     v_x = xv_get(gd.v_win_v_win_pu->v_win_pu,XV_X);
	     v_y = xv_get(gd.v_win_v_win_pu->v_win_pu,XV_Y);
             y_pos =  2 * xv_get(gd.v_win_v_win_pu->controls1,XV_HEIGHT);
             x_pos =  30;

             xv_set(gd.route_pu->route_pu,
               XV_X, x_pos + p_x + v_x,
               XV_Y, y_pos + p_y + v_y,
               FRAME_CMD_PUSHPIN_IN, TRUE,
               XV_SHOW, TRUE,
               NULL);
        } else  {
             xv_set(gd.route_pu->route_pu,
               FRAME_CMD_PUSHPIN_IN, TRUE,
               XV_SHOW, TRUE,
               NULL);
        }
      } else {
        xv_set(gd.route_pu->route_pu,
               FRAME_CMD_PUSHPIN_IN, FALSE,
               XV_SHOW, FALSE,
               NULL);
      } 
}
 
/*************************************************************************
 * Notify callback function for `dismiss_bt'.
 */
void
v_panel_dismiss(Panel_item item, Event* )
{
    // close the route menu and reset the button
    xv_set(gd.route_pu->route_pu, FRAME_CMD_PUSHPIN_IN, FALSE, XV_SHOW, FALSE, NULL);
    xv_set(gd.v_win_v_win_pu->route_st, PANEL_VALUE,0,NULL);

    // Close the cross section panel
    xv_set(gd.v_win_v_win_pu->v_win_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL); 

    // Reset the main menu bar's show_xsect_panel button
    gd.menu_bar.last_callback_value = gd.menu_bar.last_callback_value & 
                                       ~(gd.menu_bar.show_xsect_panel_bit); 
    xv_set(gd.h_win_horiz_bw->main_st, 
	 PANEL_VALUE, gd.menu_bar.last_callback_value,
	 NULL);
    
    // Make sure the reference line disappears
    gd.v_win.active = 0;
    set_redraw_flags(1,0);
}          

/*************************************************************************
 * User-defined action for `route_labels_st'.
 */
void
route_labels_proc(Panel_item item, int value, Event *event)
{

   gd.layers.route_wind._P->add_waypoints_labels = value & 1? 1 : 0; 
   gd.layers.route_wind._P->add_wind_text = value & 2? 1 : 0; 

   set_redraw_flags(1,1);
}
 /**************************************************************************
 * Notify callback function for `ht_top_tx'.
 */
Panel_setting
scale_top_proc(Panel_item item, Event *event)
{
  // v_win_v_win_pu_objects *ip = (v_win_v_win_pu_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	char *	value = (char *) xv_get(item, PANEL_VALUE);
	
	gd.v_win.max_ht = atof(value);
	gd.v_win.cmax_y =  gd.v_win.max_ht;
	set_redraw_flags(0,1);

	return panel_text_notify(item, event);
}

/**************************************************************************
 * Notify callback function for `v_unzoom_bt'.
 */
void
v_unzoom_proc(Panel_item item, Event *event)
{
  // v_win_v_win_pu_objects *ip = (v_win_v_win_pu_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
     
     gd.v_win.cmin_x = 0.0;
     gd.v_win.cmax_x = gd.h_win.route.total_length;
     gd.v_win.cmin_y =  gd.v_win.min_ht;
     gd.v_win.cmax_y =  gd.v_win.max_ht;
     
     set_redraw_flags(0,1);
     xv_set(gd.v_win_v_win_pu->v_win_pu,FRAME_CMD_PUSHPIN_IN, TRUE,XV_SHOW, TRUE,NULL); 
}

/**************************************************************************
 * Notify callback function for `scale_base_tx'.
 */
Panel_setting
scale_base_proc(Panel_item item, Event *event)
{
  // v_win_v_win_pu_objects *ip = (v_win_v_win_pu_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	char *	value = (char *) xv_get(item, PANEL_VALUE);
	
	gd.v_win.min_ht = atof(value);
	gd.v_win.cmin_y =  gd.v_win.min_ht;
	set_redraw_flags(0,1);

	return panel_text_notify(item, event);
}
         
#endif
