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
 * Callbacks.c - Notify and event callback functions for EventMan
 */

#include <stdio.h>
#include <string>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>

#define EVENTMAN_CALLBACKS

#include "eventman.h"

void update_popup()
{
	char t_buf[1024];
	struct tm *t;
	
	if(gd.Wev.size() > 0) {
	 for(int i = 0; i < gd.Wev.size(); i++) {
	     xv_set(gd.Eventman_em_bw->event_list, PANEL_LIST_STRING,i,gd.Wev[i]->title.c_str(), NULL);
	 }

	xv_set(gd.Eventman_em_bw->summary_tx,PANEL_VALUE,gd.Wev[gd.cur_event]->title.c_str(),NULL);

	if(gd.params.use_localtime) {
	  t = localtime(&gd.Wev[gd.cur_event]->start_time);
	  strftime(t_buf,1024,"%Y/%m/%d %T %Z",t);
	} else {
	  t = gmtime(&gd.Wev[gd.cur_event]->start_time);
	  strftime(t_buf,1024,"%Y/%m/%d %T UTC",t);
	}
	xv_set(gd.Eventman_em_bw->date_tx,PANEL_VALUE,t_buf,NULL);

	xv_set(gd.Eventman_em_bw->fname_tx,PANEL_VALUE,gd.params.event_list_file,NULL);

	if(gd.params.use_localtime) {
	  t = localtime(&gd.Wev[gd.cur_event]->end_time);
	  strftime(t_buf,1024,"%Y/%m/%d %T %Z",t);
	} else {
	  t = gmtime(&gd.Wev[gd.cur_event]->end_time);
	  strftime(t_buf,1024,"%Y/%m/%d %T UTC",t);
	}
	xv_set(gd.Eventman_em_bw->end_time_tx,PANEL_VALUE,t_buf,NULL);
	
	textsw_erase(gd.Eventman_em_bw->em_tp,0,TEXTSW_INFINITY);
	
	char n_buf[1024*1024];
	strncpy(n_buf,gd.Wev[gd.cur_event]->notes.c_str(),1024*1024);
	textsw_insert(gd.Eventman_em_bw->em_tp,n_buf,gd.Wev[gd.cur_event]->notes.length());
	xv_set(gd.Eventman_em_bw->em_tp,TEXTSW_FIRST_LINE,0,NULL);
	

   } else { // Empty Set.
	xv_set(gd.Eventman_em_bw->summary_tx,PANEL_VALUE,"None",NULL);
	xv_set(gd.Eventman_em_bw->date_tx,PANEL_VALUE,"None",NULL);
	xv_set(gd.Eventman_em_bw->end_time_tx,PANEL_VALUE,"None",NULL);
	textsw_insert(gd.Eventman_em_bw->em_tp,"None",4);
   }
   PMU_force_register("User Input");
}

void init_xview(int argc, char **argv)
{
	/*
	 * Initialize XView.
	 */
	xv_init(XV_INIT_ARGC_PTR_ARGV, &argc, argv, NULL);
	INSTANCE = xv_unique_key();
	
	/*
	 * Initialize user interface components.
	 * Do NOT edit the object initializations by hand.
	 */
	gd.Eventman_em_bw = (eventman_em_bw_objects*)  eventman_em_bw_objects_initialize(NULL, 0);
	
}

void modify_xview_objects()
{
	Xv_opaque   image1;
	static unsigned short bbits[] =  {
#include "black.icon"
	};

	image1 = xv_create(XV_NULL, SERVER_IMAGE,
			SERVER_IMAGE_DEPTH, 1,
			SERVER_IMAGE_BITS, bbits,
			XV_WIDTH, 64,
			XV_HEIGHT, 64,
			NULL);

	xv_set(gd.Eventman_em_bw->em_bw,
			FRAME_SHOW_RESIZE_CORNER, TRUE,
			FRAME_ICON, xv_create(XV_NULL, ICON,
				ICON_IMAGE,image1,
				ICON_HEIGHT, 1,
				ICON_WIDTH, 64,
				NULL),
			NULL);

	for(int i = 0; i < gd.Wev.size(); i++) {

		 xv_set(gd.Eventman_em_bw->event_list,PANEL_LIST_INSERT, i,
				            PANEL_LIST_STRING, i, gd.Wev[i]->title.c_str(),
							PANEL_LIST_CLIENT_DATA, i, i, NULL);

     }
	 xv_set(gd.Eventman_em_bw->event_list, PANEL_LIST_SELECT,0, TRUE, NULL);
	 xv_set(gd.Eventman_em_bw->mailto_bt, XV_SHOW, FALSE, NULL);

	 // Ignore changes when closing
	 xv_set(gd.Eventman_em_bw->em_tp, TEXTSW_IGNORE_LIMIT,TEXTSW_INFINITY,NULL);

	update_popup();
}



/*
 * Notify callback function for `goto_bt'.
 */
void
goto_proc(Panel_item item, Event *event)
{
	char cmd_buf[1024];
	char fmt_buf[1024];

	if(gd.Wev.size() <= 0) return;

	struct tm *t= gmtime(&gd.Wev[gd.cur_event]->start_time);
	sprintf(fmt_buf,"%s SET_TIME  %s",gd.params.command_string,"\"%Y %m %d %H %M %S\"");
	strftime(cmd_buf,1024,fmt_buf,t);
	if(gd.params.debug >= Params::DEBUG_NORM) fprintf(stderr," Running: %s\n",cmd_buf);
	system(cmd_buf);

	PMU_force_register(cmd_buf);
}

/*
 * Notify callback function for `add_bt'.
 */
void
add_proc(Panel_item item, Event *event)
{
   // Instantiate a new Weather Event Object.
   Wevent *W = new Wevent();

   gd.Wev.push_back(W);

   xv_set(gd.Eventman_em_bw->event_list, PANEL_LIST_SELECT,gd.cur_event,FALSE, NULL);

   gd.cur_event = gd.Wev.size() -1;
   
   

   update_popup();
}

/*
 * Notify callback function for `del_bt'.
 */
void
del_proc(Panel_item item, Event *event)
{
	if(gd.Wev.size() <= 0) return;

	Wevent *W = gd.Wev[gd.cur_event];
	delete W;

	gd.Wev.erase(gd.Wev.begin() + gd.cur_event);

	xv_set(gd.Eventman_em_bw->event_list, PANEL_LIST_DELETE,gd.cur_event,NULL);

	if(gd.cur_event >= gd.Wev.size() && gd.cur_event > 0)  gd.cur_event--;

	if(gd.Wev.size() > 0) {
    	xv_set(gd.Eventman_em_bw->event_list, PANEL_LIST_SELECT,gd.cur_event,TRUE, NULL);
	}

    update_popup();
}

/*
 * Notify callback function for `mailto_bt'.
 */
void
mail_proc(Panel_item item, Event *event)
{
	if(gd.Wev.size() <= 0) return;
	eventman_em_bw_objects *ip = (eventman_em_bw_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);
	
	fputs("eventman: mail_proc\n", stderr);
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

}

/*
 * Notify callback function for `save_bt'.
 */
void
save_proc(Panel_item item, Event *event)
{
	if(gd.Wev.size() <= 0) return;
	 FILE *ofile;

	 if((ofile = fopen(gd.params.event_list_file,"w+")) == NULL) {
		 fprintf(stderr,"Problems opening %s\n",gd.params.event_list_file);
		 perror("EventMan");
		 return;
	 }
	 gd.E->SaveFile(ofile);

	 fclose(ofile);
	PMU_force_register("Save Event List File");
}

/*
 * Notify callback function for `close_bt'.
 */
void
close_proc(Panel_item item, Event *event)
{
	xv_set(gd.Eventman_em_bw->em_bw,FRAME_CLOSED,TRUE,NULL);
	PMU_force_register("User Close");
}

/*
 * Notify callback function for `event_list'.
 */
int
ev_list_proc(Panel_item item, char *string, Xv_opaque client_data, Panel_list_op op, Event *event, int row)
{
	if(gd.Wev.size() <= 0) return XV_OK;
	eventman_em_bw_objects *ip = (eventman_em_bw_objects *) xv_get(item, XV_KEY_DATA, INSTANCE);

	
	int num;
	switch(op) {
	case PANEL_LIST_OP_DESELECT:
		break;

	case PANEL_LIST_OP_SELECT:
		gd.cur_event = row;
		break;

	case PANEL_LIST_OP_VALIDATE:
		break;

	case PANEL_LIST_OP_DELETE:
		break;
	}
	
	/* gxv_start_connections DO NOT EDIT THIS SECTION */

	/* gxv_end_connections */

	update_popup();

	return XV_OK;
}

/*
 * Notify callback function for `summary_tx'.
 */
Panel_setting
summary_proc(Panel_item item, Event *event)
{
	if(gd.Wev.size() <= 0) return panel_text_notify(item, event);;
	char *	value = (char *) xv_get(item, PANEL_VALUE);
    gd.Wev[gd.cur_event]->title = value;


	update_popup();
	
	return panel_text_notify(item, event);
}

/*
 * Notify callback function for `end_time_tx'.
 */
Panel_setting
end_time_proc(Panel_item item, Event *event)
{
	if(gd.Wev.size() <= 0) return panel_text_notify(item, event);;
	date_time_t t;
	char *	value = (char *) xv_get(item, PANEL_VALUE);

	if(sscanf(value,"%d/%d/%d %d:%d:%d",&t.year,&t.month,&t.day,&t.hour,&t.min,&t.sec) == 6) {
		  uconvert_to_utime(&t);
          gd.Wev[gd.cur_event]->end_time = t.unix_time;
	}
	if(gd.params.use_localtime) {
		struct tm *t;
	    t = localtime(&gd.Wev[gd.cur_event]->end_time);
		gd.Wev[gd.cur_event]->end_time  -= t->tm_gmtoff;
	}
	update_popup();
	
	return panel_text_notify(item, event);
}

/*
 * Event callback function for `em_tp'.
 */
Notify_value
comment_text_proc(Xv_window win, Event *event, Notify_arg arg, Notify_event_type type)
{
	if(gd.Wev.size() <= 0) return notify_next_event_func(win, (Notify_event) event, arg, type);;

	char n_buf[65536];
	xv_get(gd.Eventman_em_bw->em_tp,TEXTSW_CONTENTS,0,n_buf,65536);
	gd.Wev[gd.cur_event]->notes = n_buf;

	return notify_next_event_func(win, (Notify_event) event, arg, type);
}


/*
 * Event callback function for `start_time_tx'.
 */
Panel_setting
start_time_proc(Panel_item item, Event *event)
{
	if(gd.Wev.size() <= 0) return  panel_text_notify(item, event);

	date_time_t t;
	char *	value = (char *) xv_get(item, PANEL_VALUE);

	if(sscanf(value,"%d/%d/%d %d:%d:%d",&t.year,&t.month,&t.day,&t.hour,&t.min,&t.sec) == 6) {
		  uconvert_to_utime(&t);
          gd.Wev[gd.cur_event]->start_time = t.unix_time;
	}
	if(gd.params.use_localtime) {
		struct tm *t;
	    t = localtime(&gd.Wev[gd.cur_event]->start_time);
		gd.Wev[gd.cur_event]->start_time  -= t->tm_gmtoff;
	}

	update_popup();
	return panel_text_notify(item, event);
}


/*
 *  * Notify callback function for `help_bt'.
 *   */
void
help_proc(Panel_item item, Event *event)
{
	char cmd_buf[2048];
	strncpy(cmd_buf,gd.params.help_command,2046);
	strcat(cmd_buf," &");
	system(cmd_buf);
	PMU_force_register("User Help Request");
}


/*
 *  Notify callback function for `rt_bt'.
 */

void
gort_proc(Panel_item item, Event *event)
{
	char cmd_buf[1024];

	struct tm *t= gmtime(&gd.Wev[gd.cur_event]->start_time);
	sprintf(cmd_buf,"%s SET_REALTIME",gd.params.command_string);
	if(gd.params.debug >= Params::DEBUG_NORM) fprintf(stderr," Running: %s\n",cmd_buf);
	system(cmd_buf);
	PMU_force_register("User Go real-time Request");
}


/*
 *  * Notify callback function for `fname_tx'.
 *   */
Panel_setting
load_file_proc(Panel_item item, Event *event)
{
  char *  value = (char *) xv_get(item, PANEL_VALUE);
  gd.params.event_list_file = value;

  xv_set(gd.Eventman_em_bw->event_list,PANEL_LIST_DELETE_ROWS,0,gd.Wev.size()-1,NULL);

  gd.Wev.clear();

  FILE *evfile;

  if((evfile = fopen(gd.params.event_list_file,"rw")) == NULL) {
      fprintf(stderr,"Problems opening %s\n",gd.params.event_list_file);
	        perror("EventMan");
	        return panel_text_notify(item, event);
    }

  gd.E->LoadFile(evfile);
  fclose(evfile);
  gd.cur_event = 0;

  update_popup();

  return panel_text_notify(item, event);
}

