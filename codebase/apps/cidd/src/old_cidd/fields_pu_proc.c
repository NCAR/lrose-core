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
 * FIELDS_PU_PROC.C - Notify and event callback functions for the fields
 *    popup.
 *
 * For the Cartesian Radar Display (CIDD) 
 * N. Rehak   12 January 1993   NCAR, Research Applications Program
 */

#include "cidd.h"

void update_config_gui();
/*************************************************************************
 * Notify callback function for `display_list'.
 */

int field_display_proc(item, string, client_data, op, event, row)

Panel_item    item;
char        *string;
Xv_opaque    client_data;
Panel_list_op    op;
Event        *event;
int          row;

{
    int      num_render_lines;  /* # of fields in the render fields list */
    int      display_field;      /* field position in the display fields list */
    int      render_line;       /* field position in the render fields list */
    int      i;
    char     field_name[NAME_LENGTH];
    int      current_h_field,    /* current choice list selection for horiz/ */
             current_v_field;    /*   ... vertical display fields            */
    int      new_h_field = FALSE,  /* flag indicating change in selected     */
             new_v_field = FALSE;  /*  ... horiz/vertical display fields     */
    

    switch(op)
    {
    case PANEL_LIST_OP_DESELECT:

        gd.mrec[client_data]->currently_displayed = FALSE;
        
        /*
         * Delete the proper field from the render fields list.  Use the
         * client data (which contains the field position in the field display
         * list and the gd.mrec array) to identify the proper field in the
         * field render list in case two fields have the same name (unlikely,
         * but you never know).
         */

        num_render_lines = gd.num_menu_fields;
        
        for (render_line = 0; render_line < num_render_lines; render_line++) {

        } /* endfor - render_line */

        num_render_lines--;

        /*
         * Reset the field choice lists on the horizontal and vertical display
         * windows.  Note that other code expects the choice list values to be
         * in the same order as the render list values, so be very careful if
         * you change this.
         */

        if (num_render_lines <= 0)
        {
            xv_set(gd.fields_pu->display_list,
                   PANEL_LIST_SELECT, 0, TRUE,
                   NULL);
            
            num_render_lines = 1;
            
            current_h_field = 0;
            current_v_field = 0;
            
            new_h_field = TRUE;
            new_v_field = TRUE;
        } else {
            current_h_field = gd.h_win.field;
        
            current_v_field = gd.v_win.field;
        }
        
        /*
         * Make sure the correct field is selected in each of the field
         * choice lists.
         */

        if (current_h_field == render_line)     /* field was deselected */
        {
            current_h_field = 0;
            new_h_field = TRUE;
        }
        else if (current_h_field > render_line) /* field position changed */
            current_h_field--;
        
        if (current_v_field == render_line)     /* field was deselected */
        {
            current_v_field = 0;
            new_v_field = TRUE;
        }
        else if (current_v_field > render_line) /* field position changed */
            current_v_field--;
        
        break;

    case PANEL_LIST_OP_SELECT:

        gd.mrec[client_data]->currently_displayed = TRUE;

        /*
         * Add the selected field to the field render list.  Add the field
         * to the end of the list to avoid selection problems.  If we are
         * rendering the field in the background, the field should be
         * selected in the render fields list and the name of the field
         * should include the render string in the choice lists on the
         * horizontal and vertical display windows.
         */

        num_render_lines = gd.num_menu_fields;

        /*
         * Add the selected field to the field choice lists on the horizontal
         * and vertical data display windows.  Note that we do not have to
         * worry about the correct field being selected as we are always
         * adding on to the end of the choice list.  Note also that other
         * code expects the choice list values to be in the same order as the
         * render list values, so be very careful if you change this.
         */

        break;

    } /* endswitch - op */

    init_field_menus();

    return(XV_OK);

} /* end of field_display_proc */



/*************************************************************************
 * CONFIG_FIELD_PROC
 */
void
config_field_proc(menu, item)
    Menu        menu;
    Menu_item    item;
{
    int field;
    char string[128];

    field =  xv_get(item,XV_KEY_DATA,MENU_KEY);
    gd.config_field = field;

	update_config_gui();

}


/*************************************************************************
 * Update Field Config gui elements to indicate the settings of the 
 * Selected field
 */

void
update_config_gui()
{
    char string[128];
	 
    /* Now set all the panel labels appropriately */
    sprintf(string,"Label: %s\n",gd.mrec[gd.config_field]->field_name);
    xv_set(gd.fields_pu->f_config_msg2,PANEL_LABEL_STRING,string,NULL);

    sprintf(string,"Colorscale: %s\n",gd.mrec[gd.config_field]->color_file);
    xv_set(gd.fields_pu->f_config_msg3,PANEL_LABEL_STRING,string,NULL);
     
    sprintf(string,"Units: %s\n",gd.mrec[gd.config_field]->field_units);
    xv_set(gd.fields_pu->f_config_msg4,PANEL_LABEL_STRING,string,NULL);

    xv_set(gd.fields_pu->f_config_tx1,PANEL_VALUE,gd.mrec[gd.config_field]->data_hostname,NULL);
    xv_set(gd.fields_pu->f_config_tx5,PANEL_VALUE,gd.mrec[gd.config_field]->data_server_instance,NULL);
    xv_set(gd.fields_pu->f_config_tx6,PANEL_VALUE,gd.mrec[gd.config_field]->data_server_subtype,NULL);

    sprintf(string,"%d",gd.mrec[gd.config_field]->port);
    xv_set(gd.fields_pu->f_config_tx2,PANEL_VALUE,string,NULL);

    sprintf(string,"%d",gd.mrec[gd.config_field]->sub_field);
    xv_set(gd.fields_pu->f_config_tx3,PANEL_VALUE,string,NULL);

    sprintf(string,"%.2f",gd.mrec[gd.config_field]->time_allowance);
    xv_set(gd.fields_pu->f_config_tx4,PANEL_VALUE,string,NULL);

    sprintf(string,"%.2f",gd.mrec[gd.config_field]->cscale_min);
    xv_set(gd.fields_pu->f_config_tx7,PANEL_VALUE,string,NULL);

    sprintf(string,"%.2f",gd.mrec[gd.config_field]->cscale_delta);
    xv_set(gd.fields_pu->f_config_tx8,PANEL_VALUE,string,NULL);

    sprintf(string,"%.2f",gd.mrec[gd.config_field]->time_offset);
    xv_set(gd.fields_pu->f_config_tx9,PANEL_VALUE,string,NULL);

    xv_set(gd.fields_pu->ren_meth_st,PANEL_VALUE,gd.mrec[gd.config_field]->render_method & 1,NULL);
    xv_set(gd.fields_pu->update_st,PANEL_VALUE,gd.mrec[gd.config_field]->background_render & 1,NULL);
    xv_set(gd.fields_pu->max_st,PANEL_VALUE,gd.mrec[gd.config_field]->composite_mode & 1,NULL);
    xv_set(gd.fields_pu->use_servmap_st,PANEL_VALUE,gd.mrec[gd.config_field]->use_servmap & 1,NULL);
}

/*************************************************************************
 * FIELD_MU_GEN_PROC
 */
Menu
field_mu_gen_proc(menu, op)
        Menu            menu;
        Menu_generate   op;
{
    int    i;
    Menu_item mi;
    static Menu field_menu;

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
                    MENU_STRING, gd.mrec[i]->field_name,
                    MENU_NOTIFY_PROC,    config_field_proc,
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
 * Notify callback function for `f_config_tx1'.
 */
Panel_setting
set_host_proc(item, event)
        Panel_item      item;
        Event           *event;
{
        char *  value = (char *) xv_get(item, PANEL_VALUE);

    STRcopy(gd.mrec[gd.config_field]->data_hostname,value,NAME_LENGTH);
    set_redraw_flags(1,0);
    reset_data_valid_flags(1,0);
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `f_config_tx2'.
 */
Panel_setting
set_port_proc(item, event)
        Panel_item      item;
        Event           *event;
{
        char *  value = (char *) xv_get(item, PANEL_VALUE);

    gd.mrec[gd.config_field]->port = atoi(value);
    set_redraw_flags(1,0);
    reset_data_valid_flags(1,0);
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `f_config_tx3'.
 */
Panel_setting
set_field_no_proc(item, event)
        Panel_item      item;
        Event           *event;
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);

    gd.mrec[gd.config_field]->sub_field = atoi(value);
    set_redraw_flags(1,0);
    reset_data_valid_flags(1,0);
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `f_config_tx4'.
 */
Panel_setting
set_stretch_proc(item, event)
        Panel_item      item;
        Event           *event;
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);

    gd.mrec[gd.config_field]->time_allowance = atof(value);
    set_redraw_flags(1,1);
    reset_data_valid_flags(1,1);
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `max_st'.
 */
void
set_composite_proc(item, value, event)
    Panel_item  item;
    int     value;
    Event       *event;
{

    if(value) {
	gd.mrec[gd.config_field]->composite_mode = TRUE;
    } else {
	gd.mrec[gd.config_field]->composite_mode = FALSE;
    }

    set_redraw_flags(1,0);
    reset_data_valid_flags(1,0);
}


/*************************************************************************
 * Notify callback function for `ren_meth_st'.
 */
void
set_ren_meth_proc(item, value, event)
        Panel_item      item;
        int             value;
        Event           *event;
{
    if(value) gd.mrec[gd.config_field]->render_method = TRUE;
    else  gd.mrec[gd.config_field]->render_method = FALSE;
     
    set_redraw_flags(1,0);
    reset_data_valid_flags(1,0);
}

/*************************************************************************
 * Notify callback function for `update_st'.
 */
void
set_update_proc(item, value, event)
        Panel_item      item;
        int             value;
        Event           *event;
{

    if(value) {
        gd.mrec[gd.config_field]->background_render = TRUE;
        /* Create the background pixmaps for this field */
        if(gd.h_win.field_xid[gd.config_field] == 0) {
           gd.h_win.field_xid[gd.config_field] = XCreatePixmap(gd.dpy,
                                       gd.h_win.vis_xid,
                                       gd.h_win.can_dim.width,
                                       gd.h_win.can_dim.height,
                                       gd.h_win.can_dim.depth);
        }

        if(gd.v_win.field_xid[gd.config_field] == 0) {
            gd.v_win.field_xid[gd.config_field] = XCreatePixmap(gd.dpy,
                                       gd.v_win.vis_xid,
                                       gd.v_win.can_dim.width,
                                       gd.v_win.can_dim.height,
                                       gd.v_win.can_dim.depth);
        }

    } else {
        gd.mrec[gd.config_field]->background_render = FALSE;
        /* free up the pixmaps used for this field and redraw the image */
        if(gd.h_win.field_xid[gd.config_field]) {
                XFreePixmap(gd.dpy,gd.h_win.field_xid[gd.config_field]);
                gd.h_win.field_xid[gd.config_field] = 0;
         }
        if(gd.v_win.field_xid[gd.config_field]) {
                XFreePixmap(gd.dpy,gd.v_win.field_xid[gd.config_field]);
                gd.v_win.field_xid[gd.config_field] = 0;
         }

    }
    gd.h_win.redraw[gd.config_field] = 1;
    gd.v_win.redraw[gd.config_field] = 1;


}

/*************************************************************************
 * Notify callback function for `f_config_tx6'.
 */
Panel_setting
set_subtype_proc(item, event)
    Panel_item  item;
    Event       *event;
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);
   
    STRcopy(gd.mrec[gd.config_field]->data_server_subtype,value,NAME_LENGTH);
    set_redraw_flags(1,1);
    reset_data_valid_flags(1,1);

    return panel_text_notify(item, event);
}


/*************************************************************************
 * Notify callback function for `f_config_tx5'.
 */
Panel_setting
set_instance_proc(item, event)
    Panel_item  item;
    Event       *event;
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);

    STRcopy(gd.mrec[gd.config_field]->data_server_instance,value,NAME_LENGTH);
    set_redraw_flags(1,1);
    reset_data_valid_flags(1,1);

    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `f_config_color_apply_bt'.
 */
void
set_cscale_apply_proc(item, event)
    Panel_item  item;
    Event       *event;
{
    int i;
    double delta;
    met_record_t    *mr;

	mr = gd.mrec[gd.config_field];

	delta = mr->cscale_delta;

	if(delta > 0) {
		 for(i=0; i < mr->h_vcm.nentries; i++) {
			 mr->h_vcm.vc[i]->min = mr->cscale_min + (delta * i);
			 mr->v_vcm.vc[i]->min = mr->cscale_min + (delta * i);
			 mr->h_vcm.vc[i]->max = mr->h_vcm.vc[i]->min + delta;
			 mr->v_vcm.vc[i]->max = mr->v_vcm.vc[i]->min + delta;
		 }
         setup_color_mapping(mr, &(mr->h_vcm), mr->h_scale, mr->h_bias);
         setup_color_mapping(mr, &(mr->v_vcm), mr->h_scale, mr->h_bias);
         set_redraw_flags(1,1);
	} else {
		mr->cscale_min = mr->h_vcm.vc[0]->min;
		mr->cscale_delta = (mr->h_vcm.vc[mr->h_vcm.nentries-1]->max - mr->h_vcm.vc[0]->min) / mr->h_vcm.nentries;
		update_config_gui();
	}
}

/*************************************************************************
 * Notify callback function for `f_config_tx7'.
 */
Panel_setting
set_cscale_min_proc(item, event)
    Panel_item  item;
    Event       *event;
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);

    gd.mrec[gd.config_field]->cscale_min = atof(value);
 
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `f_config_tx8'.
 *  Now labeled as a delta.
 */
Panel_setting
set_cscale_max_proc(item, event)
    Panel_item  item;
    Event       *event;
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);

    gd.mrec[gd.config_field]->cscale_delta = atof(value);

    return panel_text_notify(item, event);
}

/***************************************************************************
 * Notify callback function for `use_servmap_st'.
 */
void
set_servmap_proc(item, value, event)
    Panel_item  item;
    int     value;
    Event       *event;
{
     gd.mrec[gd.config_field]->use_servmap = value;
}

/***************************************************************************
  * Notify callback function for `f_config_tx9'.
 */
Panel_setting
set_time_offset_proc(item, event)
    Panel_item  item;
    Event       *event;
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);

    gd.mrec[gd.config_field]->time_offset = atof(value);

    set_redraw_flags(1,1);
    reset_data_valid_flags(1,1);
    return panel_text_notify(item, event);
}
