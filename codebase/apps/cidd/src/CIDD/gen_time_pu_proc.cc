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
/****************************************************************************
 * GEN_TIME_PU_PROC.cc:
 */

#define GEN_TIME_PU_PROC

#include "cidd.h"

int mdvx_request_timelist(met_record_t *mr);
met_record_t    *choose_model_mr(int page);
/*************************************************************************
 * SHOW_GEN_TIME_MENU
 */
void show_gen_time_menu( u_int value)
{
    Window  xid;
    Window  root;     /* Root window ID of drawable */
    Window  parent;   /* Root window ID of drawable */
    Window  *children;
    unsigned int nchild;
    unsigned int i;
    int x,y;            /* location of drawable relative to parent */
    int p_x,p_y;        /* parent window location */
    int x_pos,y_pos;
    unsigned int    width,height;/* dimensions of Drawable */
    unsigned int    border_width,depth; /* dimensions of Drawable */
    int  choice_num;
    char *label;   // What this button is called 
    struct tm tms;
    time_t t;
    char time_label[128];   // 

    static int first_time = 1;

      if(value) {
	met_record_t *mr = choose_model_mr(gd.h_win.page);

	// sanity checking
	if(mr == NULL) return;
	if(mdvx_request_timelist(mr) <0) {
	    return;
	}

	const vector<time_t> &timeList = mr->h_mdvx->getTimeList();

	if(gd.gen_time_list.num_alloc_entries == 0 && timeList.size() > 0 ) { 
           gd.gen_time_list.tim = (time_t *) calloc(timeList.size(),sizeof(time_t));
           if(gd.gen_time_list.tim != NULL) gd.gen_time_list.num_alloc_entries  = timeList.size();
        } else if (gd.gen_time_list.num_alloc_entries < timeList.size()) {
            gd.gen_time_list.tim = (time_t *) realloc(gd.gen_time_list.tim,(timeList.size() * sizeof(time_t)));
            if(gd.gen_time_list.tim != NULL) gd.gen_time_list.num_alloc_entries  = timeList.size();
        } 

	// Copy the Model run times
        gd.gen_time_list.num_entries = timeList.size(); 
	for(i = 0 ; i < gd.gen_time_list.num_entries; i++) {
	  gd.gen_time_list.tim[i] = timeList[i];
	}


	xv_set(gd.gen_time_pu->gen_time_st,PANEL_CHOICE_STRINGS,"Latest",NULL,NULL);
	for(i = 0; i < timeList.size(); i++) {
	    t = timeList[i];
	    strftime(time_label,128,"%y%m%d_%H:%M",gmtime_r(&t,&tms));
	    xv_set(gd.gen_time_pu->gen_time_st, PANEL_CHOICE_STRING, i+1, time_label,NULL);
	    if(gd.model_run_time == t)  xv_set(gd.gen_time_pu->gen_time_st,PANEL_VALUE,i+1,NULL);
	}
        if(gd.model_run_time == 0)  xv_set(gd.gen_time_pu->gen_time_st,PANEL_VALUE,0,NULL);
	int ncols = (int)( timeList.size() / 30.0) + 1;
	xv_set(gd.gen_time_pu->gen_time_st,XV_X,1, XV_Y, 1,PANEL_CHOICE_NCOLS,ncols, NULL);
	xv_set(gd.gen_time_pu->popup1,XV_HEIGHT, xv_get(gd.gen_time_pu->gen_time_st,XV_HEIGHT)+2,NULL);
	xv_set(gd.gen_time_pu->popup1,XV_WIDTH, xv_get(gd.gen_time_pu->gen_time_st,XV_WIDTH)+2,NULL);


	if(first_time) { /* Only posistion this panel one time */
             first_time = 0;
	     // calc the bit number of the widget 
	     choice_num =  (int)(rint(log((double) gd.menu_bar.show_gen_time_win_bit) /
			    log(2.0)));

	     label = (char *)  xv_get(gd.h_win_horiz_bw->main_st,PANEL_CHOICE_STRING,choice_num,NULL);
             xid = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);
             /* Windows position on xv_get is relative to the parent */
             /* so find out where the parent window is */
             XQueryTree(gd.dpy,xid,&root,&parent,&children,&nchild);
             XFree((caddr_t)children);
             XGetGeometry(gd.dpy,xid,&root,&x,&y,&width,&height,&border_width,&depth);
             /* take parents postion to get pos relative to root win */
             XGetGeometry(gd.dpy,parent,&root,&p_x,&p_y,&width,&height,&border_width,&depth);

             y_pos = xv_get(gd.h_win_horiz_bw->cp,XV_HEIGHT) + (4 * gd.h_win.margin.top);
             x_pos = xv_get(gd.data_pu->data_pu,XV_WIDTH) + gd.h_win.margin.left + 5;
             x_pos += xv_get(gd.zoom_pu->zoom_pu,XV_WIDTH) + 5;
             x_pos += xv_get(gd.over_pu->over_pu,XV_WIDTH) + 5;
             x_pos += xv_get(gd.prod_pu->prod_pu,XV_WIDTH) + 5;
             x_pos += xv_get(gd.fcast_pu->fcast_pu,XV_WIDTH) + 5;
             xv_set(gd.gen_time_pu->popup1,
               XV_X, p_x + x_pos,
               XV_Y, p_y + y_pos,
               FRAME_CMD_PUSHPIN_IN, TRUE,
               XV_SHOW, TRUE,
	       XV_LABEL, label,
               NULL);
	} else  {
             xv_set(gd.gen_time_pu->popup1,
               FRAME_CMD_PUSHPIN_IN, TRUE,
               XV_SHOW, TRUE,
               NULL);
	}
      } else {
         xv_set(gd.gen_time_pu->popup1,
           FRAME_CMD_PUSHPIN_IN, FALSE,
           XV_SHOW, FALSE,
           NULL);
      }
}

//////////////////////////////////////////////////////////////////////////
// CHOOSE_MODEL_MR : Choose an appripriate data record to use to
//                    gather a model run_time list

met_record_t    *choose_model_mr(int page)
{
     int i;
     met_record_t *mr;
     int found = 0;

     // First check the key field for the current page
     mr = gd.mrec[page];
     if(mr->h_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST)
       return mr; // Is a Model based field

     // Now look through any other data, that is displayed which
     // Might contain Model-D data

     // WINDS
     for(i=0; i < gd.layers.num_wind_sets && found == 0; i++) {
       mr = gd.layers.wind[i].wind_u;
       if(gd.layers.wind[i].active  && mr->h_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST)
	   found = 1;
     }

     // Layered GRIDS
     for(i=0; i < NUM_GRID_LAYERS && found == 0; i++) {
       mr = gd.mrec[gd.layers.overlay_field[i]];
       if(gd.layers.overlay_field_on[i]  && mr->h_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST)
	   found = 1;
     }

     // CONTOURS
     for(i=0; i < NUM_CONT_LAYERS && found == 0; i++) {
       mr = gd.mrec[gd.layers.cont[i].field];
       if(gd.layers.cont[i].active && mr->h_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST)
	   found = 1;
     }

     // If nothing is 3-D go with the key data field
     if(found ==0)  {
       mr = gd.mrec[page];
     }

     return mr;
}

/**********************************************************************
 * MDVX_REQUEST_TIMELIST
 *
 */


int mdvx_request_timelist(met_record_t *mr)
{
    int num_waits;
    time_t  start_time,end_time;
    double min_lat,max_lat,min_lon,max_lon;
    char url[1024];
    char tmp_str[1024];
    char label[128];
    char *ptr;


    memset(label,0,128);
    memset(url,0,1024);

    // Pick out field name or number;
    if((ptr = strchr(mr->url,'&')) == NULL) {
	fprintf(stderr,"Bogus URL: %s -\n Missing ampersand before field name/number\n",mr->url);
        return -1;
    }
    strncpy(url,mr->url,(ptr - mr->url));
    url[ptr - mr->url] = '\0'; // Null terminate

    // If using the tunnel - Add the tunnel_url to the Url as a name=value pair
    if(strlen(gd.http_tunnel_url) > URL_MIN_SIZE) {
      // If using the a proxy - Add the a proxy_url to the Url as a name=value pair
      // Note this must be used in conjunction with a tunnel_url.
      if((strlen(gd.http_proxy_url)) > URL_MIN_SIZE) {
         sprintf(tmp_str,"?tunnel_url=%s&proxy_url=%s",gd.http_tunnel_url,gd.http_proxy_url);
      } else {
         sprintf(tmp_str,"?tunnel_url=%s",gd.http_tunnel_url);
      }

      // Append the arguments to the end of the Url string
      strncat(url,tmp_str,1024);
    }

     // Construct a valid DsUrl.
     DsURL URL(url);  

     if(URL.isValid() != TRUE) {
	fprintf(stderr,"Bogus URL: %s\n",url);
	 mr->h_data_valid = 1;
	return -1;
     }

     start_time = gd.epoch_end - (gd.model_run_list_hours * 3600);
     end_time =   gd.epoch_end + (gd.model_run_list_hours * 3600);


     if(gd.debug1) {
	fprintf(stderr, "Get MDVX tIme List from %s \n",mr->url);
	fprintf(stderr, "   Start time = %s\n", DateTime::str(start_time).c_str());
	fprintf(stderr, "   End time = %s\n", DateTime::str(end_time).c_str());
	// Disable threading while in debug mode
	 mr->h_mdvx->setThreadingOff();
     }

     mr->h_mdvx->clearTimeListMode();
     sprintf(label, "Requesting Model Run times for %s data",mr->legend_name);
     if(gd.show_data_messages) gui_label_h_frame(label,-1);

     // Gather time list
     mr->h_mdvx->setTimeListModeGen(url,start_time,end_time);


     // Set up the DsMdvx request object's domain limits
     get_bounding_box(min_lat,min_lon,max_lat,max_lon);
     if (!gd.do_not_clip_on_mdv_request) {
       mr->h_mdvx->setReadHorizLimits(min_lat,min_lon,max_lat,max_lon);
     }

     if (mr->h_mdvx->compileTimeList()) {
	 cout << "ERROR -CIDD:  setTimeListModeValid" << endl;
	 cout << mr->h_mdvx->getErrStr();
     }  

     set_busy_state(1);
     num_waits = 0;
     // Wait up to 10 seconds for a response
     while(mr->h_mdvx->getThreadDone() == 0 && num_waits < 100) {
	 num_waits++;
	 uusleep(10000); // 10 miliseconds 
     }
     if(num_waits >= 100) mr->h_mdvx->cancelThread();
     set_busy_state(0);

     if(mr->h_mdvx->getThreadRetVal()) {  // Error condition

       if(gd.debug || gd.debug1) {
	 fprintf(stderr,"TIMELIST_REQUEST Error %d - %s\n", 
		 mr->h_mdvx->getThreadRetVal(),
		 mr->h_mdvx->getErrStr().c_str());
	 add_message_to_status_win("Model Run Time Request error",1); 
	 add_message_to_status_win((char *) mr->h_mdvx->getErrStr().c_str(),0);
       }
       return -1;
     } else {
        return 0;
    }
}

/*************************************************************************
 * Notify callback function for `gen_time_st'.
 */
void
gen_time_proc(Panel_item item, int value, Event *event)
{
    if(value == 0) {
       gd.model_run_time = 0;
    } else {
       gd.model_run_time = gd.gen_time_list.tim[value -1];
    }

    // Hide the Popup menu
    xv_set(gd.gen_time_pu->popup1,FRAME_CMD_PUSHPIN_IN, FALSE, XV_SHOW, FALSE, NULL);

    // Pop back up the Model Run time button
    gd.menu_bar.last_callback_value &= ~gd.menu_bar.show_gen_time_win_bit;
    xv_set(gd.h_win_horiz_bw->main_st,PANEL_VALUE,gd.menu_bar.last_callback_value,NULL);

    //fprintf(stderr,"MODEL RUN TIME: :%d,  %s",gd.model_run_time,asctime(gmtime(&gd.model_run_time)));

    // New time lists and data must be gathered - Mark all as invalid
    reset_time_list_valid_flags();
    reset_data_valid_flags(1,1);
    set_redraw_flags(1,1);
	
} 
