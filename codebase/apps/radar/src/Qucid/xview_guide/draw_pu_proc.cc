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
 * draw_pu_proc.c - Notify and event callback functions for the Draw
 *  OK popup panel
 * 
 */

#define DRAW_PU_PROC

#include "cidd.h"

#include <Fmq/DrawQueue.hh>
class DrawQueue; 

/*************************************************************************
 * UPDATE_DRAW_EXPORT_PANEL: Update the widgets to reflect the current
 *  values for products
 */

void
update_draw_export_panel()
{    
	int cur_prod;
	draw_export_info_t *de;
	char    string[NAME_LENGTH];
	UTIMstruct  temp_utime;
	char    text[64];

	if(gd.draw.num_draw_products <= 0) return;

	cur_prod = gd.draw.cur_draw_product;
	de = &(gd.draw.dexport[cur_prod]);


	// Check to make sure a time is assigned
    if(de->data_time == 0 && gd.mrec[gd.h_win.page] != NULL) {
		 de->data_time = gd.mrec[gd.h_win.page]->h_date.unix_time;
	}

    if(de->data_time == 0) {
        notice_prompt(gd.h_win_horiz_bw->horiz_bw,NULL,
               NOTICE_MESSAGE_STRINGS,
				"Warning: Data Time Not Set",
				"Using Current Time",
               NULL,
               NOTICE_BUTTON,  "OK", 101,
               NULL);
		de->data_time = time(0);
	}


	sprintf(string,"%d",de->default_serial_no);
	xv_set(gd.draw_pu->serialno_tx,PANEL_VALUE,de->default_serial_no,NULL);

	sprintf(string,"%g",de->default_valid_minutes);
	xv_set(gd.draw_pu->validtm_tx,PANEL_VALUE,string,NULL);

	UTIMunix_to_date(de->data_time,&temp_utime);
	sprintf(text,"%ld:%02ld:%02ld %ld/%ld/%ld",
	    temp_utime.hour,
	    temp_utime.min,
	    temp_utime.sec,
	    temp_utime.month,  
	    temp_utime.day,
	    temp_utime.year);

	xv_set(gd.draw_pu->time_tx,PANEL_VALUE,text,NULL);

	xv_set(gd.draw_pu->label_tx,PANEL_VALUE,de->product_label_text,NULL);

	xv_set(gd.draw_pu->url_tx,PANEL_VALUE,de->product_fmq_url,NULL);
}


/*************************************************************************
 * Notify callback function for `type_st'.
 */
void
select_draw_type_proc(Panel_item item, int value, Event *event)
{

	// Check to make sure a time is assigned
    if( gd.mrec[gd.h_win.page] != NULL) {
		  gd.draw.dexport[value].data_time = 
			  gd.mrec[gd.h_win.page]->h_date.unix_time;
	} else {
	   gd.draw.dexport[value].data_time = 
	      gd.draw.dexport[gd.draw.cur_draw_product].data_time;
	}

	gd.draw.cur_draw_product = value;
	update_draw_export_panel();
}

/*************************************************************************
 * Notify callback function for `ok_bt'. - Send the Drawing on its way
 */
void
draw_ok_proc(Panel_item item, Event *event)
{
    int i;	
	int cur_prod;
	draw_export_info_t *de;

	DrawQueue    drawqueue;

	cur_prod = gd.draw.cur_draw_product;
	de = &gd.draw.dexport[cur_prod];

	// Prevent Data times of 0 being used.
    if(de->data_time == 0) {
        notice_prompt(gd.h_win_horiz_bw->horiz_bw,NULL,
               NOTICE_MESSAGE_STRINGS,
				"Warning: Data Time Not Set",
				"Select A field with data and then reset product type",
				"OR: Enter time Manually",
               NULL,
               NOTICE_BUTTON,  "OK", 101,
               NULL);

		return;
	}

	if (drawqueue.init( de->product_fmq_url, (char *) "CIDD", gd.debug2 ) != 0 ) {  
	    fprintf(stderr,"Problems initialising Draw Fmq: %s - aborting\n",de->product_fmq_url);
	    return;
	}
	
	if(gd.debug2) {
	  fprintf(stderr,"Product: %s, ID: %d, Valid: %d seconds\n",
				de->product_id_label,
				de->default_serial_no,
				(int) (de->default_valid_minutes * 60));
	  fprintf(stderr,"Label: %s\n URL: %s\n",
				de->product_label_text,
				de->product_fmq_url);



	  for(i=0; i < gd.h_win.route.num_segments + 1; i++) {
		      fprintf(stderr,"Way pt Lat,Lon: %g,%g   Len: %g\n",
		         gd.h_win.route.lat[i], gd.h_win.route.lon[i],gd.h_win.route.seg_length[i]);
	  }
	}
	
  	  si32 vlevel_num = gd.mrec[gd.h_win.page]->plane;
	  
	  drawqueue.sendProduct(time(0),
				(time_t) de->data_time,
				(int) de->default_serial_no,
				(int) (de->default_valid_minutes * 60),
				(ui32) (gd.h_win.route.num_segments + 1),
				vlevel_num,
				gd.mrec[gd.h_win.page]->vert[vlevel_num].min,
				gd.mrec[gd.h_win.page]->vert[vlevel_num].cent,
				gd.mrec[gd.h_win.page]->vert[vlevel_num].max,
				de->product_id_label,
				de->product_label_text,
				"CIDD",
				gd.h_win.route.lat,gd.h_win.route.lon);


	switch(gd.drawing_mode) {
		case  DRAW_FMQ_MODE:
		  redraw_route_line(&gd.h_win); // clears the line
	      // reset the instruction label 
	      gui_label_h_frame("Draw Mode:  Drag Mouse Button to Start  - Click to extend", 1);
		  add_message_to_status_win("Drag Mouse Button to Start  - Click to extend",0);
		break;

		case PICK_PROD_MODE:
		  gd.prod_mgr->draw_pick_obj(); // Erases the pick product - Using the XOR GC.
	      // reset the instruction label 
	      gui_label_h_frame("Pick Mode:  Click and Drag Objects", 1);
		  add_message_to_status_win("Click and Drag Objects",0);

		break;
	}


	// Hide the panel
	show_draw_panel(0);
	
}

/*************************************************************************
 * Notify callback function for `cancel_bt'.
 */
void
draw_cancel_proc(Panel_item item, Event *event)
{
	redraw_route_line(&gd.h_win); // clears the line
	
	// Hide the panel
	show_draw_panel(0);

	gui_label_h_frame("Draw Mode:  Drag Mouse Button to Start  - Click to extend", 1);
    add_message_to_status_win("Drag Mouse Button to Start  - Click to extend",0);
}

/*************************************************************************
 * Notify callback function for `serialno_tx'.
 */
Panel_setting
draw_serialno_proc(Panel_item item, Event *event)
{
	int cur_prod;
	draw_export_info_t *de;

	cur_prod = gd.draw.cur_draw_product;
	de = &gd.draw.dexport[cur_prod];

	int	value = (int) xv_get(item, PANEL_VALUE);

	de->default_serial_no = value;

	update_draw_export_panel();
	
	return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `validtm_tx'.
 */
Panel_setting
draw_validtm_proc(Panel_item item, Event *event)
{
	int cur_prod;
	draw_export_info_t *de;

	cur_prod = gd.draw.cur_draw_product;
	de = &gd.draw.dexport[cur_prod];

	char *	value = (char *) xv_get(item, PANEL_VALUE);
	
	de->default_valid_minutes = strtod(value,NULL);
	update_draw_export_panel();
	
	return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `label_tx'.
 */
Panel_setting
draw_label_proc(Panel_item item, Event *event)
{
	int cur_prod;
	draw_export_info_t *de;

	cur_prod = gd.draw.cur_draw_product;
	de = &gd.draw.dexport[cur_prod];

	char *	value = (char *) xv_get(item, PANEL_VALUE);

	strncpy(de->product_label_text,value,TITLE_LENGTH);

	update_draw_export_panel();
	
	return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `url_tx'.
 */
Panel_setting
draw_url_proc(Panel_item item, Event *event)
{
	int cur_prod;
	draw_export_info_t *de;

	cur_prod = gd.draw.cur_draw_product;
	de = &gd.draw.dexport[cur_prod];

	char *	value = (char *) xv_get(item, PANEL_VALUE);

	de->product_fmq_url = value;
	strncpy(de->product_fmq_url,value,URL_LENGTH);
	
	update_draw_export_panel();
	
	return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `time_tx'.
 */
Panel_setting
set_draw_time_proc(Panel_item item, Event *event)
{
    int cur_prod;
    draw_export_info_t *de;
    UTIMstruct    temp_utime;
    time_t utime;

    cur_prod = gd.draw.cur_draw_product;
    de = &gd.draw.dexport[cur_prod];

    char *  cvalue = (char *) xv_get(item, PANEL_VALUE); 

    if(strlen(cvalue) < 4) {
      utime = time(0);
    } else {
	  // Set the struct members
	  UTIMunix_to_date(de->data_time,&temp_utime);

	  // Replace members
      parse_string_into_time(cvalue,&temp_utime);
      utime = UTIMdate_to_unix(&temp_utime);
    }

    de->data_time = utime;
	
    update_draw_export_panel();

    return panel_text_notify(item, event);
}
