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
 * FIELDS_PU_PROC.C - Notify and event callback functions for the fields
 *    popup.
 *
 * For the Configurable Interactive Data Display (CIDD) 
 */

#define FIELDS_PU_PROC 1

#include "cidd.h"

void update_grid_config_gui();
void update_wind_config_gui();
void update_prod_config_gui();

void config_grid_proc(Menu menu, Menu_item item);
void config_wind_proc(Menu menu, Menu_item item);
void config_prod_proc(Menu menu, Menu_item item);

void wind_color_proc(Panel_item, Event *); 

static int grid_config_field = 0;
static int wind_config_field = 0;
static int prod_config_field = 0;

/*************************************************************************
 * FIELD_MU_GEN_PROC
 */
Menu field_mu_gen_proc( Menu  menu, Menu_generate op)
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
                    MENU_NOTIFY_PROC,    config_grid_proc,
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
 * Menu handler for `wind_mu'.
 */
Menu
wind_menu_gen_proc(Menu menu, Menu_generate op)
{
    int    i;
    Menu_item mi;
    static Menu wind_menu;

    // Use unused parameters
    menu = 0;
 
    switch (op) {
    case MENU_DISPLAY:
	if(wind_menu == 0) {
	    wind_menu = xv_create(0,MENU,NULL); 
	}
            /* Remove any old items */
            for(i=(int) xv_get(wind_menu,MENU_NITEMS) ; i > 0; i--) {
                xv_set(wind_menu,MENU_REMOVE,i,NULL);
                xv_destroy(xv_get(wind_menu,MENU_NTH_ITEM,i));
            }

            /* Build the wind menu */
            for(i=0; i < gd.layers.num_wind_sets; i++) {
              if(gd.mrec[i]->currently_displayed) {
                  mi = xv_create(0,MENUITEM,
                    MENU_STRING, gd.layers.wind[i].wind_u->button_name,
                    MENU_NOTIFY_PROC,    config_wind_proc,
                    XV_KEY_DATA,    MENU_KEY, i,
                    NULL);
                  xv_set(wind_menu,MENU_APPEND_ITEM,mi,NULL);
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
    return wind_menu;
}

/*************************************************************************
 * Menu handler for `prod_mu'.
 */
Menu
prod_mu_gen_proc(Menu menu, Menu_generate op)
{
    int    i;
    Menu_item mi;
    static Menu prod_menu;

    // Use unused parameters
    menu = 0;
	
	switch (op) {
	case MENU_DISPLAY:
	  if(prod_menu == 0) {
	    prod_menu = xv_create(0,MENU,NULL); 
	  }
            /* Remove any old items */
            for(i=(int) xv_get(prod_menu,MENU_NITEMS) ; i > 0; i--) {
                xv_set(prod_menu,MENU_REMOVE,i,NULL);
                xv_destroy(xv_get(prod_menu,MENU_NTH_ITEM,i));
            }

            /* Build the wind menu */
            for(i=0; i < gd.syprod_P->prod_info_n; i++) {
              mi = xv_create(0,MENUITEM,
                MENU_STRING, gd.syprod_P->_prod_info[i].menu_label,
                MENU_NOTIFY_PROC,    config_prod_proc,
                XV_KEY_DATA,    MENU_KEY, i,
                NULL);
              xv_set(prod_menu,MENU_APPEND_ITEM,mi,NULL);
            }
		break;

	case MENU_DISPLAY_DONE:
		break;

	case MENU_NOTIFY:
		break;

	case MENU_NOTIFY_DONE:
		break;
	}
	return prod_menu;
}

/*************************************************************************
 * Reset WIND URLs
 */
void reset_wind_urls()
{
    char *	url_value = (char *) xv_get(gd.fields_pu->w_config_url_tx, PANEL_VALUE);
    char *	u_name = (char *) xv_get(gd.fields_pu->w_config_u_tx, PANEL_VALUE);
    char *	v_name = (char *) xv_get(gd.fields_pu->w_config_v_tx, PANEL_VALUE);

    // U FIELD
    STRcopy(gd.layers.wind[wind_config_field].wind_u->url,url_value,URL_LENGTH);
    strncat(gd.layers.wind[wind_config_field].wind_u->url,"&",URL_LENGTH);
    strncat(gd.layers.wind[wind_config_field].wind_u->url,u_name,URL_LENGTH);
    gd.layers.wind[wind_config_field].wind_u->h_data_valid = 0;
    gd.layers.wind[wind_config_field].wind_u->h_data_valid = 0;

    // V Field
    STRcopy(gd.layers.wind[wind_config_field].wind_v->url,url_value,URL_LENGTH);
    strncat(gd.layers.wind[wind_config_field].wind_v->url,"&",URL_LENGTH);
    strncat(gd.layers.wind[wind_config_field].wind_v->url,v_name,URL_LENGTH);
    gd.layers.wind[wind_config_field].wind_u->h_data_valid = 0;
    gd.layers.wind[wind_config_field].wind_u->h_data_valid = 0;

    // W Field
    if(gd.layers.wind[wind_config_field].wind_w != NULL) {
        char *	w_name = (char *) xv_get(gd.fields_pu->w_config_w_tx, PANEL_VALUE);
        STRcopy(gd.layers.wind[wind_config_field].wind_w->url,url_value,URL_LENGTH);
        strncat(gd.layers.wind[wind_config_field].wind_w->url,"&",URL_LENGTH);
        strncat(gd.layers.wind[wind_config_field].wind_w->url,w_name,URL_LENGTH);
        gd.layers.wind[wind_config_field].wind_w->h_data_valid = 0;
        gd.layers.wind[wind_config_field].wind_w->h_data_valid = 0;
    }
    set_redraw_flags(1,1);
}

/*************************************************************************
 * Notify callback function for `w_config_url_tx'.
 */
Panel_setting
wind_url_proc(Panel_item item, Event * event)
{
    reset_wind_urls();
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `w_config_u_tx'.
 */
Panel_setting
u_name_proc(Panel_item item, Event *event)
{
    reset_wind_urls();
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `w_config_v_tx'.
 */
Panel_setting
v_name_proc(Panel_item item, Event *event)
{
    reset_wind_urls();
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `w_config_w_tx'.
 */
Panel_setting
w_name_proc(Panel_item item, Event *event)
{
    reset_wind_urls();
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `w_style_tx'.
 */
void
wind_style_proc(Panel_item, int value, Event *)
{
	
    gd.layers.wind[wind_config_field].marker_type = value + 1;
    set_redraw_flags(1,1);
}

/*************************************************************************
 * Notify callback function for `w_config_line_width_tx'.
 */
Panel_setting
w_line_width_proc(Panel_item item, Event *event)
{
	
    gd.layers.wind[wind_config_field].line_width = (int) xv_get(item, PANEL_VALUE);
    XSetLineAttributes(gd.dpy,gd.layers.wind[wind_config_field].color->gc,
		       gd.layers.wind[wind_config_field].line_width,
		       LineSolid,CapButt,JoinBevel);
    set_redraw_flags(1,1);

    return panel_text_notify(item, event);
}

/*************************************************************************
 * Event callback function for `w_config_color_msg'.
 */
void
fields_pu_fields_pu_w_config_color_msg_event_callback(Panel_item item, Event *event)
{
	static Color_change_t cs;
	extern void set_color( char *cname, void* c_data);

	if (event_action(event) == ACTION_SELECT) {
	    cs.handle = item;
	    cs.cgcp = gd.layers.wind[wind_config_field].color;
	    gcc_activate("Choose Wind Color","",set_color,(char *)&cs,
		gd.layers.wind[wind_config_field].color->name);
	}
}

/*************************************************************************
 * Notify callback function for `p_config_url_tx'.
 */
Panel_setting
prod_url_proc(Panel_item item, Event *event)
{
   gd.syprod_P->_prod_info[prod_config_field].url =  (char *) xv_get(item, PANEL_VALUE);

   gd.prod_mgr->set_product_data_valid(prod_config_field,0);
   set_redraw_flags(1,0);

   return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `allow_after_tx'.
 */
Panel_setting
allow_after_proc(Panel_item item, Event *event)
{

    char *	value = (char *) xv_get(item, PANEL_VALUE);
	
    gd.syprod_P->_prod_info[prod_config_field].minutes_allow_after = atof(value);
	
    gd.prod_mgr->set_product_data_valid(prod_config_field,0);

    set_redraw_flags(1,0);

    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `p_config_type_tx'.
 */
Panel_setting
prod_data_type_proc(Panel_item item, Event *event)
{
    char *	value = (char *) xv_get(item, PANEL_VALUE);
    int data_type;

    if(sscanf(value,"%d",&data_type) != 1) {
       gd.syprod_P->_prod_info[prod_config_field].data_type  = Spdb::hash4CharsToInt32(value);
    } else {
	gd.syprod_P->_prod_info[prod_config_field].data_type = data_type;
    }
	
    gd.prod_mgr->set_product_data_valid(prod_config_field,0);

    set_redraw_flags(1,0);

    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `allow_before_tx'.
 */
Panel_setting
allow_before_proc(Panel_item item, Event *event)
{
    char *	value = (char *) xv_get(item, PANEL_VALUE);

    gd.syprod_P->_prod_info[prod_config_field].minutes_allow_before = atof(value);
	
    set_redraw_flags(1,0);

    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `render_type_st'.
 */
void
prod_render_type_proc(Panel_item item, int value, Event *event)
{
    gd.syprod_P->_prod_info[prod_config_field].render_type = (Csyprod_P::render_type_t) value;
    set_redraw_flags(1,0);
}

/*************************************************************************
 * Notify callback function for `text_thresh_tx'.
 */
Panel_setting
test_thresh_proc(Panel_item item, Event *event)
{
    char *	value = (char *) xv_get(item, PANEL_VALUE);

    gd.syprod_P->_prod_info[prod_config_field].text_off_threshold = atof(value);
	
    set_redraw_flags(1,0);

    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `display_list'.
 */

int field_display_proc(Panel_item item, char *str, Xv_opaque client_data,
                       Panel_list_op op, Event* eventr, int row)

{
    // Use unused parameters
    item = 0; str = NULL; eventr = NULL; row = 0;

    switch(op)
    {
    case PANEL_LIST_OP_DESELECT:
    case PANEL_LIST_OP_DELETE:

        gd.mrec[client_data]->currently_displayed = FALSE;
        
        break;

    case PANEL_LIST_OP_SELECT:

        gd.mrec[client_data]->currently_displayed = TRUE;

        break;

    case PANEL_LIST_OP_VALIDATE:
        break; // do nothing

    case PANEL_LIST_OP_DBL_CLICK:
        break; // do nothing

    } /* endswitch - op */

    init_field_menus();

    return(XV_OK);

} /* end of field_display_proc */


/*************************************************************************
 * CONFIG_GRID_PROC
 */
void config_grid_proc(Menu menu, Menu_item item)
{
    
    // Use unused parameters
    menu = 0;

    grid_config_field =  xv_get(item,XV_KEY_DATA,MENU_KEY);

    update_grid_config_gui();
}

/*************************************************************************
 * CONFIG_WIND_PROC
 */
void config_wind_proc(Menu menu, Menu_item item)
{
    
    // Use unused parameters
    menu = 0;

    wind_config_field = xv_get(item,XV_KEY_DATA,MENU_KEY);

    update_wind_config_gui();
}

/*************************************************************************
 * CONFIG_PROD_PROC
 */
void config_prod_proc(Menu menu, Menu_item item)
{
    
    // Use unused parameters
    menu = 0;

    prod_config_field =  xv_get(item,XV_KEY_DATA,MENU_KEY);

    update_prod_config_gui();
}

/*************************************************************************
 * UPDATE GRID  Config gui elements to indicate the settings of the 
 * Selected field
 */

void update_grid_config_gui()
{
    char str[128];
	 
    /* Now set all the panel labels appropriately */
    sprintf(str,"Label: %s\n",gd.mrec[grid_config_field]->button_name);
    xv_set(gd.fields_pu->f_config_msg2,PANEL_LABEL_STRING,str,NULL);

    sprintf(str,"Colorscale: %s\n",gd.mrec[grid_config_field]->color_file);
    xv_set(gd.fields_pu->f_config_msg3,PANEL_LABEL_STRING,str,NULL);
     
    sprintf(str,"Units: %s\n",gd.mrec[grid_config_field]->field_units);
    xv_set(gd.fields_pu->f_config_msg4,PANEL_LABEL_STRING,str,NULL);

    xv_set(gd.fields_pu->f_config_tx1,PANEL_VALUE,gd.mrec[grid_config_field]->url,NULL);

    sprintf(str,"%g",gd.mrec[grid_config_field]->alt_offset);
    xv_set(gd.fields_pu->f_config_tx3,PANEL_VALUE,str,NULL);

    sprintf(str,"%.2f",gd.mrec[grid_config_field]->time_allowance);
    xv_set(gd.fields_pu->f_config_tx4,PANEL_VALUE,str,NULL);

    sprintf(str,"%.2f",gd.mrec[grid_config_field]->cscale_min);
    xv_set(gd.fields_pu->f_config_tx7,PANEL_VALUE,str,NULL);

    sprintf(str,"%.2f",gd.mrec[grid_config_field]->cscale_delta);
    xv_set(gd.fields_pu->f_config_tx8,PANEL_VALUE,str,NULL);

    sprintf(str,"%.2f",gd.mrec[grid_config_field]->time_offset);
    xv_set(gd.fields_pu->f_config_tx9,PANEL_VALUE,str,NULL);

    xv_set(gd.fields_pu->ren_meth_st,PANEL_VALUE,gd.mrec[grid_config_field]->render_method & 1,NULL);
    xv_set(gd.fields_pu->update_st,PANEL_VALUE,gd.mrec[grid_config_field]->auto_render & 1,NULL);
    xv_set(gd.fields_pu->max_st,PANEL_VALUE,gd.mrec[grid_config_field]->composite_mode & 1,NULL);
}


/*************************************************************************
 * UPDATE Wind Config gui elements to indicate the settings of the 
 * Selected field
 */

void update_wind_config_gui()
{
    char str[URL_LENGTH];
    char *ptr;
	 
    if(gd.layers.num_wind_sets == 0) {
        xv_set(gd.fields_pu->w_config_msg1,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->w_config_msg2,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->w_config_sel_bt,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->w_config_url_tx,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->wind_alt_tx,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->textfield1,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->textfield2,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->w_config_u_tx,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->w_config_v_tx,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->w_config_w_tx,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->w_style_tx,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->w_config_line_width_tx,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->w_config_color_msg,PANEL_INACTIVE,TRUE,NULL);
	
	return;
    }

    /* Now set all the panel labels appropriately */
    sprintf(str,"Label: %s\n",gd.layers.wind[wind_config_field].wind_u->button_name);
    xv_set(gd.fields_pu->w_config_msg2,PANEL_LABEL_STRING,str,NULL);

    strncpy(str,gd.layers.wind[wind_config_field].wind_u->url,URL_LENGTH);
    if((ptr = strchr(str,'&')) != NULL) {
	*ptr = '\0';
    }
    xv_set(gd.fields_pu->w_config_url_tx,PANEL_VALUE,str,NULL);

    if(ptr != NULL) {
       ptr++;
       xv_set(gd.fields_pu->w_config_u_tx,PANEL_VALUE,ptr,NULL);
    }

    strncpy(str,gd.layers.wind[wind_config_field].wind_v->url,URL_LENGTH);
    if((ptr = strchr(str,'&')) != NULL) {
       ptr++;
       xv_set(gd.fields_pu->w_config_v_tx,PANEL_VALUE,ptr,NULL);
    }

    if(gd.layers.wind[wind_config_field].wind_w == NULL) {
           xv_set(gd.fields_pu->w_config_w_tx,PANEL_VALUE," ",
	          PANEL_INACTIVE,TRUE,NULL);
    } else {
        strncpy(str,gd.layers.wind[wind_config_field].wind_w->url,URL_LENGTH);
        if((ptr = strchr(str,'&')) != NULL) {
           ptr++;
           xv_set(gd.fields_pu->w_config_w_tx,PANEL_VALUE,ptr,
	          PANEL_INACTIVE,FALSE,NULL);
        }
    }

    sprintf(str,"%g\n",gd.layers.wind[wind_config_field].wind_u->alt_offset);
    xv_set(gd.fields_pu->wind_alt_tx,PANEL_VALUE,str,NULL);
      
    sprintf(str,"%g\n",gd.layers.wind[wind_config_field].wind_u->time_offset);
    xv_set(gd.fields_pu->textfield1,PANEL_VALUE,str,NULL);
      
    sprintf(str,"%g\n",gd.layers.wind[wind_config_field].wind_u->time_allowance);
    xv_set(gd.fields_pu->textfield2,PANEL_VALUE,str,NULL);
      
    xv_set(gd.fields_pu->w_style_tx,PANEL_VALUE,gd.layers.wind[wind_config_field].marker_type-1,NULL);
    xv_set(gd.fields_pu->w_config_line_width_tx,PANEL_VALUE,gd.layers.wind[wind_config_field].line_width,NULL);
    xv_set(gd.fields_pu->w_config_color_msg,PANEL_LABEL_STRING,gd.layers.wind[wind_config_field].color->name,NULL);
}


/*************************************************************************
 * UPDATE Product Config gui elements to indicate the settings of the 
 * Selected product
 */

void update_prod_config_gui()
{
    char str[128];
	 
    if(gd.syprod_P == NULL || gd.syprod_P->prod_info_n <= 0) {
        xv_set(gd.fields_pu->prod_config_msg1,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->p_config_sel_bt,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->prod_config_msg2,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->p_config_url_tx,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->p_config_type_tx,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->allow_before_tx,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->allow_after_tx,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->p_config_msg3,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->render_type_st,PANEL_INACTIVE,TRUE,NULL);
        xv_set(gd.fields_pu->text_thresh_tx,PANEL_INACTIVE,TRUE,NULL);

	return;
    }

    xv_set(gd.fields_pu->prod_config_msg2,PANEL_LABEL_STRING,gd.syprod_P->_prod_info[prod_config_field].menu_label,NULL);

    xv_set(gd.fields_pu->p_config_url_tx,PANEL_VALUE,gd.syprod_P->_prod_info[prod_config_field].url,NULL);

    sprintf(str,"%d",gd.syprod_P->_prod_info[prod_config_field].data_type);
    xv_set(gd.fields_pu->p_config_type_tx,PANEL_VALUE,str,NULL);

    sprintf(str,"%g",gd.syprod_P->_prod_info[prod_config_field].minutes_allow_before);
    xv_set(gd.fields_pu->allow_before_tx,PANEL_VALUE,str,NULL);

    sprintf(str,"%g",gd.syprod_P->_prod_info[prod_config_field].minutes_allow_after);
    xv_set(gd.fields_pu->allow_after_tx,PANEL_VALUE,str,NULL);

    xv_set(gd.fields_pu->render_type_st,PANEL_VALUE,gd.syprod_P->_prod_info[prod_config_field].render_type,NULL);

    sprintf(str,"%g",gd.syprod_P->_prod_info[prod_config_field].text_off_threshold);
    xv_set(gd.fields_pu->text_thresh_tx,PANEL_VALUE,str,NULL);

}


/*************************************************************************
 * Notify callback function for `f_config_tx1'.
 */
Panel_setting set_host_proc(Panel_item item, Event *event)
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);

    STRcopy(gd.mrec[grid_config_field]->url,value,URL_LENGTH);
    set_redraw_flags(1,1);
    gd.mrec[grid_config_field]->h_data_valid = 0;
    gd.mrec[grid_config_field]->v_data_valid = 0;
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `f_config_tx3'.
 */
Panel_setting set_alt_offset(Panel_item item, Event *event)
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);

    gd.mrec[grid_config_field]->alt_offset = atof(value);
    gd.mrec[grid_config_field]->h_data_valid = 0;
    set_redraw_flags(1,0);
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `f_config_tx4'.
 */
Panel_setting set_stretch_proc(Panel_item item, Event *event)
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);

    gd.mrec[grid_config_field]->time_allowance = atof(value);
    gd.mrec[grid_config_field]->h_data_valid = 0;
    gd.mrec[grid_config_field]->v_data_valid = 0;
    set_redraw_flags(1,1);
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `max_st'.
 */
void set_composite_proc(Panel_item item, int value, Event *event)
{
    // Use unused parameters
    item = 0; event = NULL;

    if(value) {
	gd.mrec[grid_config_field]->composite_mode = TRUE;
    } else {
	gd.mrec[grid_config_field]->composite_mode = FALSE;
    }

    set_redraw_flags(1,0);
    reset_data_valid_flags(1,0);
}


/*************************************************************************
 * Notify callback function for `ren_meth_st'.
 */
void set_ren_meth_proc(Panel_item item, int value, Event *event)
{
    // Use unused parameters
    item = 0; event = NULL;

    switch(value)  {
     case 0: gd.mrec[grid_config_field]->render_method = POLYGONS;
     break;

     case 1: gd.mrec[grid_config_field]->render_method = FILLED_CONTOURS;
     break;

     case 2: gd.mrec[grid_config_field]->render_method = DYNAMIC_CONTOURS;
     break;

     case 3: gd.mrec[grid_config_field]->render_method = LINE_CONTOURS;
     break;
    }
     
    set_redraw_flags(1,0);
    reset_data_valid_flags(1,0);
}

/*************************************************************************
 * Notify callback function for `update_st'.
 */
void set_update_proc(Panel_item item, int value, Event *event)
{
    // Use unused parameters
    item = 0; event = NULL;

    if(value) {
        gd.mrec[grid_config_field]->auto_render = TRUE;
        /* Create the background pixmaps for this field */
        if(gd.h_win.page_xid[grid_config_field] == 0) {
           gd.h_win.page_xid[grid_config_field] = XCreatePixmap(gd.dpy,
                                       gd.h_win.vis_xid,
                                       gd.h_win.can_dim.width,
                                       gd.h_win.can_dim.height,
                                       gd.h_win.can_dim.depth);
        }

        if(gd.v_win.page_xid[grid_config_field] == 0) {
            gd.v_win.page_xid[grid_config_field] = XCreatePixmap(gd.dpy,
                                       gd.v_win.vis_xid,
                                       gd.v_win.can_dim.width,
                                       gd.v_win.can_dim.height,
                                       gd.v_win.can_dim.depth);
        }

    } else {
        gd.mrec[grid_config_field]->auto_render = FALSE;
        /* free up the pixmaps used for this field and redraw the image */
        if(gd.h_win.page_xid[grid_config_field]) {
                XFreePixmap(gd.dpy,gd.h_win.page_xid[grid_config_field]);
                gd.h_win.page_xid[grid_config_field] = 0;
         }
        if(gd.v_win.page_xid[grid_config_field]) {
                XFreePixmap(gd.dpy,gd.v_win.page_xid[grid_config_field]);
                gd.v_win.page_xid[grid_config_field] = 0;
         }

    }
    gd.h_win.redraw[grid_config_field] = 1;
    gd.v_win.redraw[grid_config_field] = 1;


}

/*************************************************************************
 * Notify callback function for `f_config_color_apply_bt'.
 */
void set_cscale_apply_proc(Panel_item item, Event *event)
{
    int i;
    double delta;
    met_record_t    *mr;
    // Use unused parameters
    item = 0; event = NULL;

    mr = gd.mrec[grid_config_field];

    delta = mr->cscale_delta;

    if(delta > 0) {
         for(i=0; i < mr->h_vcm.nentries; i++) {
        	 mr->h_vcm.vc[i]->min = mr->cscale_min + (delta * i);
    		 mr->v_vcm.vc[i]->min = mr->cscale_min + (delta * i);
    		 mr->h_vcm.vc[i]->max = mr->h_vcm.vc[i]->min + delta;
    		 mr->v_vcm.vc[i]->max = mr->v_vcm.vc[i]->min + delta;
         }
         setup_color_mapping(&(mr->h_vcm), mr->h_fhdr.scale,
			     mr->h_fhdr.bias,
			     mr->h_fhdr.transform_type,
			     mr->h_fhdr.bad_data_value,
			     mr->h_fhdr.missing_data_value);
         setup_color_mapping(&(mr->v_vcm), mr->v_fhdr.scale,
			     mr->v_fhdr.bias,
			     mr->v_fhdr.transform_type,
			     mr->v_fhdr.bad_data_value,
			     mr->v_fhdr.missing_data_value);
         set_redraw_flags(1,1);
    } else {
         mr->cscale_min = mr->h_vcm.vc[0]->min;
         mr->cscale_delta = (mr->h_vcm.vc[mr->h_vcm.nentries-1]->max -
	     mr->h_vcm.vc[0]->min) / mr->h_vcm.nentries;
         update_grid_config_gui();
    }
}

/*************************************************************************
 * Notify callback function for `f_config_tx7'.
 */
Panel_setting set_cscale_min_proc(Panel_item item, Event *event)
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);

    gd.mrec[grid_config_field]->cscale_min = atof(value);
 
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `f_config_tx8'.
 *  Now labeled as a delta.
 */
Panel_setting set_cscale_max_proc(Panel_item item, Event *event)
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);

    gd.mrec[grid_config_field]->cscale_delta = atof(value);

    return panel_text_notify(item, event);
}

/***************************************************************************
  * Notify callback function for `f_config_tx9'.
 */
Panel_setting set_time_offset_proc(Panel_item item, Event *event)
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);

    gd.mrec[grid_config_field]->time_offset = atof(value);

    set_redraw_flags(1,1);
    reset_data_valid_flags(1,1);
    return panel_text_notify(item, event);
}

//**************************************************************************
//
//
void
autoscale_proc(Panel_item item, int value, Event *event)
{
    if(value & 01) {
        gd.mrec[grid_config_field]->auto_scale = TRUE;
    } else {
        gd.mrec[grid_config_field]->auto_scale = FALSE;
    }
    set_redraw_flags(1,1);
    reset_data_valid_flags(1,1);
}

/*************************************************************************
 * Notify callback function for `wind_alt_tx'.
 */
Panel_setting
wind_alt_offset_proc(Panel_item item, Event *event)
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);

    gd.layers.wind[wind_config_field].wind_u->alt_offset = atof(value);
    gd.layers.wind[wind_config_field].wind_v->alt_offset = atof(value);
 
 
    gd.layers.wind[wind_config_field].wind_u->h_data_valid = 0;
    gd.layers.wind[wind_config_field].wind_u->v_data_valid = 0;
    gd.layers.wind[wind_config_field].wind_v->h_data_valid = 0;
    gd.layers.wind[wind_config_field].wind_v->v_data_valid = 0;

    if(gd.layers.wind[wind_config_field].wind_w) {
      gd.layers.wind[wind_config_field].wind_w->alt_offset = atof(value);
      gd.layers.wind[wind_config_field].wind_w->h_data_valid = 0;
      gd.layers.wind[wind_config_field].wind_w->v_data_valid = 0;
    }
    set_redraw_flags(1,0);
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `textfield1'.
 */
Panel_setting
wind_time_offset_proc(Panel_item item, Event *event)
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);
    gd.layers.wind[wind_config_field].wind_u->time_offset = atof(value);
    gd.layers.wind[wind_config_field].wind_v->time_offset = atof(value);
 
    gd.layers.wind[wind_config_field].wind_u->h_data_valid = 0;
    gd.layers.wind[wind_config_field].wind_u->v_data_valid = 0;
    gd.layers.wind[wind_config_field].wind_v->h_data_valid = 0;
    gd.layers.wind[wind_config_field].wind_v->v_data_valid = 0;

    if(gd.layers.wind[wind_config_field].wind_w) {
      gd.layers.wind[wind_config_field].wind_w->time_offset = atof(value);
      gd.layers.wind[wind_config_field].wind_w->h_data_valid = 0;
      gd.layers.wind[wind_config_field].wind_w->v_data_valid = 0;
    }
    set_redraw_flags(1,1);
 
    return panel_text_notify(item, event);
}
 
/*************************************************************************
 * Notify callback function for `textfield2'.
 * 
 */
Panel_setting
wind_time_slop_proc(Panel_item item, Event *event)
{
    char *  value = (char *) xv_get(item, PANEL_VALUE);
    gd.layers.wind[wind_config_field].wind_u->time_allowance = atof(value);
    gd.layers.wind[wind_config_field].wind_v->time_allowance = atof(value);
 
    gd.layers.wind[wind_config_field].wind_u->h_data_valid = 0;
    gd.layers.wind[wind_config_field].wind_u->v_data_valid = 0;
    gd.layers.wind[wind_config_field].wind_v->h_data_valid = 0;
    gd.layers.wind[wind_config_field].wind_v->v_data_valid = 0;

    if(gd.layers.wind[wind_config_field].wind_w) {
      gd.layers.wind[wind_config_field].wind_w->time_allowance = atof(value);
      gd.layers.wind[wind_config_field].wind_w->h_data_valid = 0;
      gd.layers.wind[wind_config_field].wind_w->v_data_valid = 0;
    }
    set_redraw_flags(1,1);

    return panel_text_notify(item, event);
}
/*************************************************************************
 * Notify callback function for `topo_url_tx'.
 */
Panel_setting
topo_url_proc(Panel_item item, Event *event)
{
        char *  value = (char *) xv_get(item, PANEL_VALUE);
		if(gd.layers.earth.terr != NULL) {

            STRcopy(gd.layers.earth.terr->url,value,URL_LENGTH);
            gd.layers.earth.terr->h_data_valid = 0;
            gd.layers.earth.terr->v_data_valid = 0;
            set_redraw_flags(1,1);
		}

        return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `land_use_url_tx'.
 */
Panel_setting
land_use_url_proc(Panel_item item, Event *event)
{
        char *  value = (char *) xv_get(item, PANEL_VALUE);

		if(gd.layers.earth.land_use != NULL) {
            STRcopy(gd.layers.earth.land_use->url,value,URL_LENGTH);
            gd.layers.earth.land_use->h_data_valid = 0;
            set_redraw_flags(1,0);
		}

        return panel_text_notify(item, event);
}

