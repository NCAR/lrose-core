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
 * EXTRAS_PU_PROC.C - Notify and event callback functions for the extra feature
 *    popup.
 *
 * For the Cartesian Radar Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#include "cidd.h"

struct  color_stuff {    
    long   handle;     /* handle to Xview object */
    Color_gc_t *cgcp; /* Color Cell info */
    long  cval;       /* pointer to color value data */
    int   c_num;      /* color cell number in our allocated arrary */
};
struct    color_stuff cs;


void update_layered_field_panel(void);
void update_layered_contour_panel(void);

/*************************************************************************
 * SET_COLOR : Set a overlay element's color - Called through gcc_activate
 *    routine, via color mname message callbacks
 */
 
set_color(cname,c_data)
    char    *cname;
    caddr_t c_data;
{
    XColor  rgb_def;
    struct  color_stuff *ptr;
    char string[128];
 
 
    ptr = (struct color_stuff *) c_data;
 
    /* Get the RGB for the color */
    XParseColor(gd.dpy,gd.cmap,cname,&rgb_def);
    rgb_def.flags = DoRed | DoGreen | DoBlue;

    if(gd.inten_levels > 0) { /* Pseudo color mode */
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
    if(ptr->handle == gd.extras_pu->map_set_color_bt) {

            sprintf(string,"%s  %s",
	       gd.over[gd.extras.cur_map_overlay]->control_label,
	       gd.over[gd.extras.cur_map_overlay]->color->name);
            xv_set(gd.extras_pu->overlay_list,
		PANEL_LIST_STRING, gd.extras.cur_map_overlay, string, NULL);

    } else if (ptr->handle ==  gd.extras_pu->vect_set_color_bt) {

            sprintf(string,"%s  %s",
	       gd.extras.wind[gd.extras.cur_wind_overlay].wind_u->field_name,
	       gd.extras.wind[gd.extras.cur_wind_overlay].color->name);

              xv_set(gd.extras_pu->wind_on_list,
		PANEL_LIST_STRING, gd.extras.cur_wind_overlay, string, NULL);

    } else{
            /* set the message string to the new name */
            xv_set(ptr->handle,PANEL_LABEL_STRING,cname,NULL);
    }

 
    gcc_suspend(TRUE);
 
    set_redraw_flags(1,1); /*  */
}
/*************************************************************************
 * Notify callback function for `extra_st1'. - Range Rings, etc
 */
void
ref_mark_proc(item, value, event)
    Panel_item    item;
    int        value;
    Event        *event;
{
    int    bit;
    static int last_value;
    static int first_time;
    
    if(first_time == 0) {
        last_value = gd.extras.range | gd.extras.azmiths;
        first_time = 1;
    }

    bit = last_value ^ value;
    last_value = value;

    switch(bit) {
        case RANGE_BIT:
            gd.extras.range = value & RANGE_BIT;
        break;

        case AZMITH_BIT:
            gd.extras.azmiths = value & AZMITH_BIT;
        break;
    }

    set_redraw_flags(1,1);
}

/*************************************************************************
 * Notify callback function for `wind_sl'.
 */
void
wind_scale_proc(item, value, event)
    Panel_item  item;
    int     value;
    Event       *event;
{
    int    i;
    double    val;
    char    label[16];
     
    val =  (value + 1) * gd.extras.wind_time_scale_interval;  
    sprintf(label,"%g min",val);
    xv_set(gd.extras_pu->wind_msg,PANEL_LABEL_STRING,label,NULL);
     
    for(i=0; i < gd.extras.num_wind_sets; i++) {
        if(gd.extras.wind[i].wind_u->dx != 0.0) {
            gd.extras.wind[i].wind_ms = ((val* 60.0) / 1000.0)/ gd.extras.wind[i].wind_u->dx;
        }
        if(gd.extras.wind[i].wind_w != NULL) {
            set_redraw_flags(1,1);
        } else {
            set_redraw_flags(1,0);
        }
    }
}   
 
/*************************************************************************
 * Notify callback function for `cont_st'.
 */
void
cont_activate_proc(item, value, event)
    Panel_item  item;
    int     value;
    Event       *event;
{
    gd.extras.cont[gd.extras.cur_contour_layer].active = value ;

    set_redraw_flags(1,1);

}
 
/*************************************************************************
 * Notify callback function for `cont_label_st'.
 */
void
cont_label_set_proc(item, value, event)
    Panel_item  item;
    int     value;
    Event       *event;
{
    gd.extras.cont[gd.extras.cur_contour_layer].labels_on = value ;

    set_redraw_flags(1,1);

}

/*************************************************************************
 * Notify callback function for `cont_fr_tx'.
 */
Panel_setting
cont_fr_proc(item, event)
    Panel_item    item;
    Event        *event;
{
    char *    value = (char *) xv_get(item, PANEL_VALUE);
    gd.extras.cont[gd.extras.cur_contour_layer].min = atof(value);
    gd.mrec[gd.extras.cont[gd.extras.cur_contour_layer].field]->cont_low = 
	gd.extras.cont[gd.extras.cur_contour_layer].min;

    if(gd.extras.cont[gd.extras.cur_contour_layer].active) 
	set_redraw_flags(1,1);
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `cont_to_tx'.
 */
Panel_setting
cont_to_proc(item, event)
    Panel_item  item;
    Event       *event;
{
    char *    value = (char *) xv_get(item, PANEL_VALUE);
    
    gd.extras.cont[gd.extras.cur_contour_layer].max = atof(value);
    gd.mrec[gd.extras.cont[gd.extras.cur_contour_layer].field]->cont_high = 
	gd.extras.cont[gd.extras.cur_contour_layer].max;

    if(gd.extras.cont[gd.extras.cur_contour_layer].active) 
	set_redraw_flags(1,1);
    return panel_text_notify(item, event);
}
 
/*************************************************************************
 * Notify callback function for `cont_int_tx'.
 */
Panel_setting
cont_int_proc(item, event)
    Panel_item    item;
    Event        *event;
{
    char *    value = (char *) xv_get(item, PANEL_VALUE);
    
    gd.extras.cont[gd.extras.cur_contour_layer].interval = atof(value);
    gd.mrec[gd.extras.cont[gd.extras.cur_contour_layer].field]->cont_interv =
        gd.extras.cont[gd.extras.cur_contour_layer].interval;

    if(gd.extras.cont[gd.extras.cur_contour_layer].active)
	set_redraw_flags(1,1);
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Menu handler for `cont_mu'.
 */
Menu
cont_field_mu_gen(menu, op)
    Menu        menu;
    Menu_generate    op;
{
    int    i;
    Menu_item mi;
    static Menu field_menu;
	void cont_field_proc();
    
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
                    MENU_STRING, gd.mrec[i]->field_name,
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
void
dim_im_proc(item, value, event)
    Panel_item  item;
    int     value;
    Event       *event;
{
    int    i;
    double    mult;
    XColor    c_defs[MAX_COLORS];

    gd.image_inten = (double) value / gd.inten_levels;

    for(i=0; i < gd.num_colors; i++) {
        mult = (i < gd.num_draw_colors)? 1.0 : gd.image_inten;
        c_defs[i].red = gd.color[i].r * mult;
        c_defs[i].green = gd.color[i].g * mult;
        c_defs[i].blue = gd.color[i].b * mult;
        c_defs[i].pixel = gd.color[i].pixval;
        c_defs[i].flags = DoRed | DoGreen | DoBlue;
    }

    XStoreColors(gd.dpy,gd.cmap,c_defs,gd.num_colors);

}

 
/*************************************************************************
 * Event callback function for `ref_mark_color_msg'.
 */
void
ref_color_proc(item, event)
    Panel_item  item;
    Event       *event;
{

    if(event_action(event) == ACTION_SELECT && event_is_down(event)) {
        cs.handle =  item;
	cs.cgcp = gd.extras.range_color;
        gcc_activate("Reference Marker Color","",set_color,&cs,cs.cgcp->name);
    }

    panel_default_handle_event(item, event);
}


/*************************************************************************
 * Event callback function for `cont_color_msg'.
 */
void
cont_color_proc(item, event)
    Panel_item  item;
    Event       *event;
{

    if(event_action(event) == ACTION_SELECT && event_is_down(event)) {
        cs.handle =  item;
	cs.cgcp = (gd.extras.cont[gd.extras.cur_contour_layer].color);
        gcc_activate("Contours Color","",set_color,&cs,cs.cgcp->name);
    }

    panel_default_handle_event(item, event);
}

/*************************************************************************
 * VECT_SET_COLOR_PROC : Set a WIND_VECTOR Set's color
 * Notify callback function for `vect_set_color_bt'.
 */
void
vect_set_color_proc(item, event)
        Panel_item      item;
        Event           *event;
{
    cs.handle = item;
    cs.cgcp = gd.extras.wind[gd.extras.cur_wind_overlay].color;

    gcc_activate("Choose Overlay Color","",set_color,&cs,
	gd.extras.wind[gd.extras.cur_wind_overlay].color->name);
}

/*************************************************************************
 * SET_OV_COLOR_PROC : Set a overlay's color
 * Notify callback function for `over_color_bt'.
 */
void
set_ov_color_proc(item, event)
        Panel_item      item;
        Event           *event;
{
    cs.handle = item;
    cs.cgcp = gd.over[gd.extras.cur_map_overlay]->color;

    gcc_activate("Choose Overlay Color","",set_color,&cs,
	gd.over[gd.extras.cur_map_overlay]->color->name);
}

/*************************************************************************
 * VECT_FIELD_NUM_SET_PROC : Notify callback function for `wind_on_list'.
 */
int
vect_field_num_set_proc(item, string, client_data, op, event, row)
        Panel_item      item;
        char            *string;
        Xv_opaque       client_data;
        Panel_list_op   op;
        Event           *event;
        int             row;
{
    int i;

    switch(op) {
    case PANEL_LIST_OP_DESELECT:
            gd.extras.wind[row].active = 0;
            break;

    case PANEL_LIST_OP_SELECT:
            gd.extras.wind[row].active = 1;
            break;

    case PANEL_LIST_OP_VALIDATE:
            /* Do nothing */
            break;

    case PANEL_LIST_OP_DELETE:
            /* Do nothing */
            break;
    }

    gd.extras.cur_wind_overlay = row;
    set_redraw_flags(1,0);

    return XV_OK;
}
/*************************************************************************
 * OVER_SELECT_PROC : Notify callback function for `overlay_list'.
 */
int
over_select_proc(item, string, client_data, op, event, row)
        Panel_item      item;
        char            *string;
        Xv_opaque       client_data;
        Panel_list_op   op;
        Event           *event;
        int             row;
{
    int i;

    switch(op) {
    case PANEL_LIST_OP_DESELECT:
            gd.over[row]->active = 0;
            break;

    case PANEL_LIST_OP_SELECT:
            gd.over[row]->active = 1;
            break;

    case PANEL_LIST_OP_VALIDATE:
            /* Do nothing */
            break;

    case PANEL_LIST_OP_DELETE:
            /* Do nothing */
            break;
    }

    gd.extras.cur_map_overlay = row;
    set_redraw_flags(1,0);

    return XV_OK;
}

/*************************************************************************
 * Notify callback function for `prod_font_sl'.
 */
void
prod_font_sel_proc(item, value, event)
        Panel_item      item;
        int             value;
        Event           *event;
{
    int i;
    GC gc;

    gd.extras.prod.prod_font_num = value;

    for(i=0; i < PRDS_NUM_COLORS; i++) {
	gc = gd.extras.prod.prds_color[i]->gc;
	XSetFont(gd.dpy,gc,gd.ciddfont[gd.prod.prod_font_num]);
    }

    for(i=0; i < NUM_CONT_LAYERS; i++) {
        XSetFont(gd.dpy,gd.extras.cont[i].color->gc,
	    gd.ciddfont[gd.prod.prod_font_num]);
    }

    set_redraw_flags(1,0);
}

/*************************************************************************
 * Notify callback function for `prod_line_sl'.
 */
void
prod_line_width_proc(item, value, event)
        Panel_item      item;
        int             value;
        Event           *event;
{
    int i;
    GC gc;

    gd.prod.prod_line_width = value;

    for(i=0; i < PRDS_NUM_COLORS; i++) {
	gc = gd.extras.prod.prds_color[i]->gc;
	XSetLineAttributes(gd.dpy,gc,gd.prod.prod_line_width,
	    LineSolid,CapButt,JoinBevel);
    }

    for(i=0; i < NUM_CONT_LAYERS; i++) {
	XSetLineAttributes(gd.dpy,gd.extras.cont[i].color->gc,
	    gd.prod.prod_line_width,LineSolid,CapButt,JoinBevel);
    }

    for(i=0; i < gd.extras.num_wind_sets; i++) {
	gc = gd.extras.wind[i].color->gc;
	XSetLineAttributes(gd.dpy,gc,gd.prod.prod_line_width,
	    LineSolid,CapButt,JoinBevel);
    }

    set_redraw_flags(1,0);
}


/*************************************************************************
 * User-defined action for `prod_tm_sel_st'.
 */
void
prod_time_sel_proc(item, value, event)
        Panel_item      item;
        int             value;
        Event           *event;
{
        gd.prod.product_time_select = value;
        set_redraw_flags(1,0);
}

/*************************************************************************
 * Notify callback function for `prod_tm_wdth_sl'.
 */
void
prod_time_width_proc(item, value, event)
        Panel_item      item;
        int             value;
        Event           *event;
{
        gd.prod.product_time_width = value;
        set_redraw_flags(1,0);
}

/*************************************************************************
 * Notify callback function for `ov_fld_min_tx'.
 */
Panel_setting
overlay_fld_min_proc(item, event)
    Panel_item  item;
    Event       *event;
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);

    gd.extras.overlay_min[gd.extras.cur_overlay_layer] = atof(value);
    gd.mrec[gd.extras.overlay_field[gd.extras.cur_overlay_layer]]->overlay_min =
          gd.extras.overlay_min[gd.extras.cur_overlay_layer];

    if(gd.extras.overlay_field_on[gd.extras.cur_overlay_layer]) set_redraw_flags(1,1);
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `ov_fld_max_tx'.
 */
Panel_setting
overlay_fld_max_proc(item, event)
    Panel_item  item;
    Event       *event;
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);

    gd.extras.overlay_max[gd.extras.cur_overlay_layer] = atof(value);
    gd.mrec[gd.extras.overlay_field[gd.extras.cur_overlay_layer]]->overlay_max =
         gd.extras.overlay_max[gd.extras.cur_overlay_layer];
    if(gd.extras.overlay_field_on[gd.extras.cur_overlay_layer]) set_redraw_flags(1,1);
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `over_fld_st'.
 */
void
overlay_fld_on_proc(item, value, event)
    Panel_item  item;
    int     value;
    Event       *event;
{
    gd.extras.overlay_field_on[gd.extras.cur_overlay_layer] = value;
    set_redraw_flags(1,1);
}

/*************************************************************************
 * Event callback function for `cont_bt'.
 */
void
set_cont_field_proc(item, event)
    Panel_item  item;
    Event       *event;
{
    gd.extras.set_field_mode = 1;
    panel_default_handle_event(item, event);
}

/*************************************************************************
 * Event callback function for `ov_fld_mu_bt'.
 */
void
set_ov_fld_proc(item, event)
    Panel_item  item;
    Event       *event;
{
    gd.extras.set_field_mode = 2;
    panel_default_handle_event(item, event);
}
 
/*************************************************************************
 * Menu callback for `cont_mu'. Establish Which data field should be contoured
 */
void
cont_field_proc(menu, item)
    Menu        menu;
    Menu_item    item;
{
  int        field;
  char    string[NAME_LENGTH];

  
  field  =  xv_get(item,XV_KEY_DATA,MENU_KEY);
 
  switch(gd.extras.set_field_mode) {
   case 1:
    gd.extras.cont[gd.extras.cur_contour_layer].field  =  field;

    /* Reset Contour Limits, interval */
    gd.extras.cont[gd.extras.cur_contour_layer].min = gd.mrec[field]->cont_low;
     
    gd.extras.cont[gd.extras.cur_contour_layer].max = gd.mrec[field]->cont_high;
    gd.extras.cont[gd.extras.cur_contour_layer].interval = gd.mrec[field]->cont_interv;

    update_layered_contour_panel();
     
    /* indicate displays are out of date */
    if(gd.extras.cont[gd.extras.cur_contour_layer].active) set_redraw_flags(1,1);
   break;

   case 2:
    gd.extras.overlay_field[gd.extras.cur_overlay_layer] = field;
     
    update_layered_field_panel();

    /* indicate images are out of date */
    if(gd.extras.overlay_field_on[gd.extras.cur_overlay_layer]) set_redraw_flags(1,1);
   break;
 }
}

/*************************************************************************
 * Notify callback function for `cont_layer_st'.
 */
void cont_layer_set_proc( Panel_item item, int value, Event *event)
{
    gd.extras.cur_contour_layer = value;
    update_layered_contour_panel();
}

/*************************************************************************
 * UPDATE_CONTOUR_FIELD_PANEL
 */
void update_layered_contour_panel(void)
{
  int        field;
  char    string[NAME_LENGTH];

  field = gd.extras.cont[gd.extras.cur_contour_layer].field;

  xv_set(gd.extras_pu->cont_st,PANEL_VALUE,
	 gd.extras.cont[gd.extras.cur_contour_layer].active,NULL);

  xv_set(gd.extras_pu->cont_label_st,PANEL_VALUE,
	 gd.extras.cont[gd.extras.cur_contour_layer].labels_on,NULL);

  /* Display a field label in the popup */
  xv_set(gd.extras_pu->cont_msg,PANEL_LABEL_STRING,
	gd.mrec[field]->field_name,NULL);

  xv_set(gd.extras_pu->cont_color_msg,PANEL_LABEL_STRING,
	gd.extras.cont[gd.extras.cur_contour_layer].color->name,NULL);

  
  /* Reset Contour Limits, interval */
  sprintf(string,"%g",gd.extras.cont[gd.extras.cur_contour_layer].min);
  xv_set(gd.extras_pu->cont_fr_tx,PANEL_VALUE,string,NULL);
     
  sprintf(string,"%g",gd.extras.cont[gd.extras.cur_contour_layer].max);
  xv_set(gd.extras_pu->cont_to_tx,PANEL_VALUE,string,NULL);
     
  sprintf(string,"%g",gd.extras.cont[gd.extras.cur_contour_layer].interval);
  xv_set(gd.extras_pu->cont_int_tx,PANEL_VALUE,string,NULL);

}

/*************************************************************************
 * Notify callback function for `ov_field_num_st'.
 */
void
ov_field_num_sel_proc( Panel_item item, int value, Event *event)
{
    gd.extras.cur_overlay_layer = value;
    update_layered_field_panel();
}

/*************************************************************************
 * UPDATE_LAYERED_FIELD_PANEL
 */

void update_layered_field_panel(void)
{
    int field;
    char string[256];

    field = gd.extras.overlay_field[gd.extras.cur_overlay_layer];

    /* Display Overlay data thresholds */
    gd.extras.overlay_min[gd.extras.cur_overlay_layer] = gd.mrec[field]->overlay_min;
    sprintf(string,"%g",gd.extras.overlay_min[gd.extras.cur_overlay_layer]);
    xv_set(gd.extras_pu->ov_fld_min_tx,PANEL_VALUE,string,NULL);
     
    gd.extras.overlay_max[gd.extras.cur_overlay_layer] = gd.mrec[field]->overlay_max;
    sprintf(string,"%g",gd.extras.overlay_max[gd.extras.cur_overlay_layer]);
    xv_set(gd.extras_pu->ov_fld_max_tx,PANEL_VALUE,string,NULL);
     
    /* Display a field label in the popup */
    xv_set(gd.extras_pu->ov_fld_msg,PANEL_LABEL_STRING,
        gd.mrec[field]->field_name,NULL);
    xv_set(gd.extras_pu->over_fld_st,PANEL_VALUE,
        gd.extras.overlay_field_on[gd.extras.cur_overlay_layer],NULL);
}

/*************************************************************************
 * Notify callback function for `save_bt'.
 */
void
save_h_image_proc(item, event)
    Panel_item  item;
    Event       *event;
{
    void update_save_panel();

    gd.save_im_win = 0; 
    update_save_panel();
    xv_set(gd.save_pu->save_im_pu,FRAME_CMD_PUSHPIN_IN, TRUE,XV_SHOW, TRUE,NULL);
}
 
/*************************************************************************
 * Notify callback function for `layer_mode_st'.
 */
void
layer_mode_proc(item, value, event)
    Panel_item  item;
    int     value;
    Event       *event;
{
     gd.draw_main_on_top = value;
     set_redraw_flags(1,0);
}

