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
 * PAGE_PU_PROC.C - Notify and event callback functions for the page
 * configuration  popup.
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define PAGE_PU_PROC

#include "cidd.h"


void update_layered_field_panel();
void update_layered_contour_panel();
// void cont_field_proc(Menu menu, Menu_item item);

// static Color_change_t cs;
/*************************************************************************
 * SET_COLOR : Set a overlay element's color - Called through gcc_activate
 *    routine, via color mname message callbacks
 */
 
void set_color( char *cname, void* c_data)
{
    XColor  rgb_def;
    Color_change_t *ptr;
    char string2[1024];
 
 
    ptr = (Color_change_t *) c_data;
 
    /* Get the RGB for the color */
    XParseColor(gd.dpy,gd.cmap,cname,&rgb_def);
    rgb_def.flags = DoRed | DoGreen | DoBlue;

    // if ( PseudoColor == xv_get(gd.h_win_horiz_bw->horiz_bw,XV_VISUAL_CLASS)) {
    if (true) {
        rgb_def.pixel = ptr->cgcp->pixval;
        XStoreColor(gd.dpy,gd.cmap,&rgb_def);
    } else {
	XAllocColor(gd.dpy,gd.cmap,&rgb_def);
	ptr->cgcp->pixval = rgb_def.pixel;
	XSetForeground(gd.dpy,ptr->cgcp->gc,rgb_def.pixel);
    }

    ptr->cgcp->r = rgb_def.red;    /* save the color values & pixel */
    ptr->cgcp->g = rgb_def.green;
    ptr->cgcp->b = rgb_def.blue;

 
    /* copy new name into objects storage */
    STRcopy(ptr->cgcp->name,cname,NAME_LENGTH);

    /* Update the GUI widgets */

    typedef enum
    {
     color_bt_not_set = 0,
     map_set_color_bt = 1,
     vect_set_color_bt = 2,
    } page_pu_t;

    page_pu_t page_pu_handle = color_bt_not_set;
    
    if(page_pu_handle == map_set_color_bt) {
      // if(ptr->handle == gd.page_pu->map_set_color_bt) {

      snprintf(string2,1024,"%s  %s",
                    gd.overlays[gd.layers.cur_map_overlay]->control_label.c_str(),
                    gd.overlays[gd.layers.cur_map_overlay]->color->name);
            // xv_set(gd.page_pu->overlay_list,
	    //     PANEL_LIST_STRING, gd.layers.cur_map_overlay, string, NULL);

    } else if (page_pu_handle == vect_set_color_bt) {
      // } else if (ptr->handle ==  gd.page_pu->vect_set_color_bt) {

      snprintf(string2,1024,"%s  %s",
              gd.layers.wind[gd.layers.cur_wind_overlay].wind_u->button_name,
              gd.layers.wind[gd.layers.cur_wind_overlay].color->name);
      
      // xv_set(gd.page_pu->wind_on_list,
      //        PANEL_LIST_STRING, gd.layers.cur_wind_overlay, string, NULL);

    } else{
            /* set the message string to the new name */
      // xv_set(ptr->handle,PANEL_LABEL_STRING,cname,NULL);
    }

 
    // gcc_suspend(TRUE);
 
    set_redraw_flags(1,1); /*  */
}

#ifdef NOTNOW

/*************************************************************************
 * Notify callback function for `extra_st1'. - Range Rings, etc
 */
void ref_mark_proc(Panel_item item, int value, Event *event)
{
    int    bit;
    static int last_value;
    static int first_time;
    item = 0; event = NULL;
    
    if(first_time == 0) {
        last_value = gd.legends.range | gd.legends.azimuths;
        first_time = 1;
    }

    bit = last_value ^ value;
    last_value = value;

    switch(bit) {
        case RANGE_BIT:
            gd.legends.range = value & RANGE_BIT;
        break;

        case AZIMUTH_BIT:
            gd.legends.azimuths = value & AZIMUTH_BIT;
        break;
    }

    set_redraw_flags(1,1);
}

/*************************************************************************
 * Notify callback function for `wind_sl'.
 */
void wind_scale_proc(Panel_item item, int value, Event *event)
{
    double    val;
    char    label[16];
    item = 0; event = NULL;
     
    val =  (value + 1) * gd.layers.wind_time_scale_interval;    /* value is scaled in N
    minute intervals*/
    snprintf(label,"%g min",val);
    xv_set(gd.page_pu->wind_msg,PANEL_LABEL_STRING,label,NULL);

	gd.layers.wind_scaler = value +1;
     
    set_redraw_flags(1,1);
}   
 
/*************************************************************************
 * Notify callback function for `cont_st'.
 */
void cont_activate_proc(Panel_item item, int value, Event *event)
{
    item = 0; event = NULL;
    gd.layers.cont[gd.layers.cur_contour_layer].active = value ;

    set_redraw_flags(1,1);

}
 
/*************************************************************************
 * Notify callback function for `cont_label_st'.
 */
void cont_label_set_proc(Panel_item item, int value, Event *event)
{
    item = 0; event = NULL;
    gd.layers.cont[gd.layers.cur_contour_layer].labels_on = value ;

    set_redraw_flags(1,1);

}

/*************************************************************************
 * Notify callback function for `cont_fr_tx'.
 */
Panel_setting cont_fr_proc(Panel_item item, Event *event)
{
    char *    value = (char *) xv_get(item, PANEL_VALUE);
    gd.layers.cont[gd.layers.cur_contour_layer].min = atof(value);
    gd.mrec[gd.layers.cont[gd.layers.cur_contour_layer].field]->cont_low = 
	gd.layers.cont[gd.layers.cur_contour_layer].min;

    if(gd.layers.cont[gd.layers.cur_contour_layer].active) 
	set_redraw_flags(1,1);
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `cont_to_tx'.
 */
Panel_setting cont_to_proc(Panel_item item, Event *event)
{
    char *    value = (char *) xv_get(item, PANEL_VALUE);
    
    gd.layers.cont[gd.layers.cur_contour_layer].max = atof(value);
    gd.mrec[gd.layers.cont[gd.layers.cur_contour_layer].field]->cont_high = 
	gd.layers.cont[gd.layers.cur_contour_layer].max;

    if(gd.layers.cont[gd.layers.cur_contour_layer].active) 
	set_redraw_flags(1,1);
    return panel_text_notify(item, event);
}
 
/*************************************************************************
 * Notify callback function for `cont_int_tx'.
 */
Panel_setting cont_int_proc(Panel_item item, Event *event)
{
    char *    value = (char *) xv_get(item, PANEL_VALUE);
    
    gd.layers.cont[gd.layers.cur_contour_layer].interval = atof(value);
    gd.mrec[gd.layers.cont[gd.layers.cur_contour_layer].field]->cont_interv =
        gd.layers.cont[gd.layers.cur_contour_layer].interval;

    if(gd.layers.cont[gd.layers.cur_contour_layer].active)
	set_redraw_flags(1,1);
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Menu handler for `cont_mu'.
 */
Menu cont_field_mu_gen(Menu menu, Menu_generate op)
{
    int    i;
    Menu_item mi;
    static Menu field_menu;
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

            /* Build the contour field menu */
            for(i=0; i < gd.num_datafields; i++) {
	      if(gd.mrec[i]->currently_displayed) {
                  mi = xv_create(0,MENUITEM,
                    MENU_STRING, gd.mrec[i]->button_name,
                    MENU_NOTIFY_PROC,    cont_field_proc,
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
 * Notify callback function for `dim_sl'.
 */
void dim_im_proc(Panel_item item, int value, Event *event)
{
    int    i;
    double    mult;
    XColor    c_defs[MAX_COLORS];
    item = 0; event = NULL;

    _params.data_inten = (double) value / _params.inten_levels;
    _params.image_inten = _params.data_inten;

    if ( PseudoColor == xv_get(gd.h_win_horiz_bw->horiz_bw,XV_VISUAL_CLASS)) {
      for(i=0; i < gd.num_colors; i++) {
        mult = (i < gd.num_draw_colors)? 1.0 : _params.data_inten;
        c_defs[i].red = (u_short) (gd.color[i].r * mult);
        c_defs[i].green = (u_short) (gd.color[i].g * mult);
        c_defs[i].blue = (u_short) (gd.color[i].b * mult);
        c_defs[i].pixel = (u_short) (gd.color[i].pixval);
        c_defs[i].flags = DoRed | DoGreen | DoBlue;
      }

      XStoreColors(gd.dpy,gd.cmap,c_defs,gd.num_colors);
    } else {
      set_redraw_flags(1,0);
    }

}

 
/*************************************************************************
 * Event callback function for `ref_mark_color_msg'.
 */
void ref_color_proc(Panel_item item, Event *event)
{

    if(event_action(event) == ACTION_SELECT && event_is_down(event)) {
        cs.handle =  item;
	cs.cgcp = gd.legends.range_ring_color;
        gcc_activate("Reference Marker Color","",set_color,(char *)(&cs),cs.cgcp->name);
    }

    panel_default_handle_event(item, event);
}


/*************************************************************************
 * Event callback function for `cont_color_msg'.
 */
void cont_color_proc(Panel_item item, Event *event)
{

    if(event_action(event) == ACTION_SELECT && event_is_down(event)) {
        cs.handle =  item;
	cs.cgcp = (gd.layers.cont[gd.layers.cur_contour_layer].color);
        gcc_activate("Contours Color","",set_color,(char *)&cs,cs.cgcp->name);
    }

    panel_default_handle_event(item, event);
}

/*************************************************************************
 * VECT_SET_COLOR_PROC : Set a WIND_VECTOR Set's color
 * Notify callback function for `vect_set_color_bt'.
 */
void vect_set_color_proc(Panel_item item, Event *event)
{
    event = NULL;
    cs.handle = item;
    cs.cgcp = gd.layers.wind[gd.layers.cur_wind_overlay].color;

    gcc_activate("Choose Overlay Color","",set_color,(char *)&cs,
	gd.layers.wind[gd.layers.cur_wind_overlay].color->name);
}

/*************************************************************************
 * SET_OV_COLOR_PROC : Set a overlay's color
 * Notify callback function for `over_color_bt'.
 */
void set_ov_color_proc(Panel_item item, Event *event)
{
    event = NULL;
    cs.handle = item;
    cs.cgcp = gd.overlays[gd.layers.cur_map_overlay]->color;

    gcc_activate("Choose Overlay Color","",set_color,(char *)&cs,
	gd.overlays[gd.layers.cur_map_overlay]->color->name);
}

/*************************************************************************
 * VECT_FIELD_NUM_SET_PROC : Notify callback function for `wind_on_list'.
 */
int vect_field_num_set_proc(Panel_item item, char *string,
                            Xv_opaque client_data, Panel_list_op  op,
                            Event *event, int row)
{
    // Use the unused parameters
    item = 0; string = NULL; client_data = 0; event = NULL;
    switch(op) {
    case PANEL_LIST_OP_DESELECT:
            gd.layers.wind[row].active = 0;
            break;

    case PANEL_LIST_OP_SELECT:
            gd.layers.wind[row].active = 1;
            break;

    case PANEL_LIST_OP_VALIDATE:
            /* Do nothing */
            break;

    case PANEL_LIST_OP_DELETE:
            /* Do nothing */
            break;

    case PANEL_LIST_OP_DBL_CLICK:
            /* Do nothing */
            break;

    }

    gd.layers.cur_wind_overlay = row;
    set_redraw_flags(1,0);

    return XV_OK;
}
/*************************************************************************
 * OVER_SELECT_PROC : Notify callback function for `overlay_list'.
 */
int over_select_proc(Panel_item item, char *string,
                      Xv_opaque client_data, Panel_list_op  op,
                      Event *event, int row)
{
    // Use the unused parameters
    item = 0; string = NULL; client_data = 0; event = NULL;
    switch(op) {
    case PANEL_LIST_OP_DESELECT:
            gd.overlays[row]->active = 0;
            break;

    case PANEL_LIST_OP_SELECT:
            gd.overlays[row]->active = 1;
            break;

    case PANEL_LIST_OP_VALIDATE:
            /* Do nothing */
            break;

    case PANEL_LIST_OP_DELETE:
            /* Do nothing */
            break;

    case PANEL_LIST_OP_DBL_CLICK:
            /* Do nothing */
            break;

    }

    gd.layers.cur_map_overlay = row;
    set_redraw_flags(1,0);

    return XV_OK;
}

/*************************************************************************
 * Notify callback function for `prod_font_sl'.
 */
void prod_font_sel_proc(Panel_item item, int value, Event *event)
{
    // Use the unused parameters
    item = 0;  event = NULL;
    int i;

    gd.layers.prod.prod_font_num = value;
    gd.prod.prod_font_num = value;

    for(i=0; i < NUM_CONT_LAYERS; i++) {
        XSetFont(gd.dpy,gd.layers.cont[i].color->gc,
	    gd.ciddfont[gd.prod.prod_font_num]);
    }

    set_redraw_flags(1,0);
}

/*************************************************************************
 * Notify callback function for `prod_line_sl'.
 *  Note: Used for Winds
 */
void prod_line_width_proc(Panel_item item, int value, Event *event)
{
    int i;
    QBrush brush;
    // Use the unused parameters
    item = 0;  event = NULL;

    for(i=0; i < gd.layers.num_wind_sets; i++) {
	  gc = gd.layers.wind[i].color->gc;
	  gd.layers.wind[i].line_width = value;
	  XSetLineAttributes(gd.dpy,gc,value, LineSolid,CapButt,JoinBevel);
    }

    update_wind_config_gui();
    set_redraw_flags(1,1);
}

/*************************************************************************
 * Notify callback function for `ov_fld_min_tx'.
 */
Panel_setting overlay_fld_min_proc(Panel_item item, Event *event)
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);

    gd.layers.overlay_min[gd.layers.cur_overlay_layer] = atof(value);
    gd.mrec[gd.layers.overlay_field[gd.layers.cur_overlay_layer]]->overlay_min =
          gd.layers.overlay_min[gd.layers.cur_overlay_layer];

    if(gd.layers.overlay_field_on[gd.layers.cur_overlay_layer]) set_redraw_flags(1,1);
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `ov_fld_max_tx'.
 */
Panel_setting overlay_fld_max_proc(Panel_item item, Event *event)
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);

    gd.layers.overlay_max[gd.layers.cur_overlay_layer] = atof(value);
    gd.mrec[gd.layers.overlay_field[gd.layers.cur_overlay_layer]]->overlay_max =
         gd.layers.overlay_max[gd.layers.cur_overlay_layer];
    if(gd.layers.overlay_field_on[gd.layers.cur_overlay_layer]) set_redraw_flags(1,1);
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `over_fld_st'.
 */
void overlay_fld_on_proc(Panel_item item, int value, Event *event)
{
    // Use the unused parameters
    item = 0;  event = NULL;
    gd.layers.overlay_field_on[gd.layers.cur_overlay_layer] = value;
    set_redraw_flags(1,1);
}

/*************************************************************************
 * Event callback function for `cont_bt'.
 */
void set_cont_field_proc(Panel_item item, Event *event)
{
    gd.layers.set_field_mode = 1;
    panel_default_handle_event(item, event);
}

/*************************************************************************
 * Event callback function for `ov_fld_mu_bt'.
 */
void set_ov_fld_proc(Panel_item item, Event *event)
{
    gd.layers.set_field_mode = 2;
    panel_default_handle_event(item, event);
}
 
/*************************************************************************
 * Menu callback for `cont_mu'. Establish Which data field should be contoured
 */
void cont_field_proc(Menu menu, Menu_item item)
{
  int        field;
  // use the unused parameters
  menu = 0;

  
  field  =  xv_get(item,XV_KEY_DATA,MENU_KEY);
 
  switch(gd.layers.set_field_mode) {
   case 1:
    gd.layers.cont[gd.layers.cur_contour_layer].field  =  field;

    /* Reset Contour Limits, interval */
    gd.layers.cont[gd.layers.cur_contour_layer].min = gd.mrec[field]->cont_low;
     
    gd.layers.cont[gd.layers.cur_contour_layer].max = gd.mrec[field]->cont_high;
    gd.layers.cont[gd.layers.cur_contour_layer].interval = gd.mrec[field]->cont_interv;

    update_layered_contour_panel();
     
    /* indicate displays are out of date */
    if(gd.layers.cont[gd.layers.cur_contour_layer].active) set_redraw_flags(1,1);
   break;

   case 2:
    gd.layers.overlay_field[gd.layers.cur_overlay_layer] = field;
     
    update_layered_field_panel();

    /* indicate images are out of date */
    if(gd.layers.overlay_field_on[gd.layers.cur_overlay_layer]) set_redraw_flags(1,1);
   break;
 }
}

/*************************************************************************
 * Notify callback function for `cont_layer_st'.
 */
void cont_layer_set_proc( Panel_item item, int value, Event *event)
{
    // Use the unused parameters
    item = 0;  event = NULL;
    gd.layers.cur_contour_layer = value;
    update_layered_contour_panel();
}

#endif

/*************************************************************************
 * UPDATE_CONTOUR_FIELD_PANEL
 */
void update_layered_contour_panel()
{
  // int        field;
  char    string2[NAME_LENGTH];

  // field = gd.layers.cont[gd.layers.cur_contour_layer].field;

  // xv_set(gd.page_pu->cont_st,PANEL_VALUE,
  //        gd.layers.cont[gd.layers.cur_contour_layer].active,NULL);

  // xv_set(gd.page_pu->cont_label_st,PANEL_VALUE,
  //        gd.layers.cont[gd.layers.cur_contour_layer].labels_on,NULL);

  /* Display a field label in the popup */
  // xv_set(gd.page_pu->cont_msg,PANEL_LABEL_STRING,
  //       gd.mrec[field]->button_name,NULL);

  // xv_set(gd.page_pu->cont_color_msg,PANEL_LABEL_STRING,
  //       gd.layers.cont[gd.layers.cur_contour_layer].color->name,NULL);

  
  /* Reset Contour Limits, interval */
  snprintf(string2,NAME_LENGTH,"%g",gd.layers.cont[gd.layers.cur_contour_layer].min);
  //   xv_set(gd.page_pu->cont_fr_tx,PANEL_VALUE,string,NULL);
     
  snprintf(string2,NAME_LENGTH,"%g",gd.layers.cont[gd.layers.cur_contour_layer].max);
  // xv_set(gd.page_pu->cont_to_tx,PANEL_VALUE,string,NULL);
     
  snprintf(string2,NAME_LENGTH,"%g",gd.layers.cont[gd.layers.cur_contour_layer].interval);
  // xv_set(gd.page_pu->cont_int_tx,PANEL_VALUE,string,NULL);

}

#ifdef NOTNOW

/*************************************************************************
 * Notify callback function for `ov_field_num_st'.
 */
void ov_field_num_sel_proc( Panel_item item, int value, Event *event)
{
    // Use the unused parameters
    item = 0;  event = NULL;
    gd.layers.cur_overlay_layer = value;
    update_layered_field_panel();
}
#endif

/*************************************************************************
 * UPDATE_LAYERED_FIELD_PANEL
 */

void update_layered_field_panel()
{
    int field;
    char string[256];

    field = gd.layers.overlay_field[gd.layers.cur_overlay_layer];

    /* Display Overlay data thresholds */
    gd.layers.overlay_min[gd.layers.cur_overlay_layer] = gd.mrec[field]->overlay_min;
    snprintf(string,256,"%g",gd.layers.overlay_min[gd.layers.cur_overlay_layer]);
    // xv_set(gd.page_pu->ov_fld_min_tx,PANEL_VALUE,string,NULL);
     
    gd.layers.overlay_max[gd.layers.cur_overlay_layer] = gd.mrec[field]->overlay_max;
    snprintf(string,256,"%g",gd.layers.overlay_max[gd.layers.cur_overlay_layer]);
    // xv_set(gd.page_pu->ov_fld_max_tx,PANEL_VALUE,string,NULL);
     
    /* Display a field label in the popup */
    // xv_set(gd.page_pu->ov_fld_msg,PANEL_LABEL_STRING,
    //     gd.mrec[field]->button_name,NULL);
    // xv_set(gd.page_pu->over_fld_st,PANEL_VALUE,
    //     gd.layers.overlay_field_on[gd.layers.cur_overlay_layer],NULL);
}

#ifdef NOTNOW
/*************************************************************************
 * Notify callback function for `save_bt'.
 */
void save_h_image_proc(Panel_item item, Event *event)
{
    // Use the unused parameters
    item = 0;  event = NULL;
    void update_save_panel();

    gd.save_im_win = PLAN_VIEW; 
    update_save_panel();
    xv_set(gd.save_pu->save_im_pu,FRAME_CMD_PUSHPIN_IN, TRUE,XV_SHOW, TRUE,NULL);
}
 
/*************************************************************************
 * Notify callback function for `layer_mode_st'.
 */
void layer_mode_proc(Panel_item item, int value, Event *event)
{
    // Use the unused parameters
    item = 0;  event = NULL;
     _params.draw_main_on_top = value;
     set_redraw_flags(1,0);
}


/*************************************************************************
 * Notify callback function for `layer_legend_st'.
 */
void layer_legend_proc(Panel_item item, int value, Event *event)
{
    item = 0; event = NULL;
    gd.layers.layer_legends_on = value;
    set_redraw_flags(1,0);
}

/*************************************************************************
 * Notify callback function for `cont_legend_st'.
 */
void cont_legend_proc(Panel_item item, int value, Event *event)
{   
    item = 0; event = NULL;
    gd.layers.cont_legends_on = value;
    set_redraw_flags(1,0);
}

/*************************************************************************
 * Notify callback function for `wind_legend_st'.
 */
void wind_legend_proc(Panel_item item, int value, Event *event)
{
    item = 0; event = NULL;
    gd.layers.wind_legends_on = value;
    set_redraw_flags(1,0);
}


/*************************************************************************
 * Notify callback function for `land_use_st'.
 */
void
land_use_activate(Panel_item item, int value, Event *event)
{
    item = 0; event = NULL;
    if (value ) {
	    gd.layers.earth.landuse_active = 1;
        gd.menu_bar.last_callback_value |= gd.menu_bar.landuse_onoff_bit;
    } else {
	    gd.layers.earth.landuse_active = 0;
        gd.menu_bar.last_callback_value &= ~gd.menu_bar.landuse_onoff_bit;
    }

	// Leave the Menubar button in the right state
    xv_set(gd.h_win_horiz_bw->main_st,PANEL_VALUE,gd.menu_bar.last_callback_value,NULL);

    set_redraw_flags(1,0);
}

/*************************************************************************
 * Notify callback function for `topo_st'.
 */
void
topo_onoff_proc(Panel_item item, int value, Event *event)
{
    item = 0; event = NULL;
    if (value ) {
	gd.layers.earth.terrain_active = 1;
    } else {
	gd.layers.earth.terrain_active = 0;
    }
    set_redraw_flags(1,0);
}

/*************************************************************************
*
*   Notify callback function for `density_sl'.
*/
void
set_wind_density_proc(Panel_item item, int value, Event *event)
{
  int x_vects = _params.ideal_x_vectors;
  int y_vects = _params.ideal_y_vectors;
  _params.ideal_x_vects = value * x_vects;
  _params.ideal_y_vects = value * y_vects;
  set_redraw_flags(1,1);
}
 
#endif
