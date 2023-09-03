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
/***************************************************************************
 * GUI_INIT:  Build All GUI Objects for CIDD
 *
 */

#define GUI_INIT
#include "cidd.h"  
extern int fullscreendebug;

/***************************************************************************
 * INIT_XVIEW : Initialize the base frame and other global objects
 */ 
 
void init_xview(int *argc_ptr, char    *argv[])
{
    // xv_init(XV_INIT_ARGC_PTR_ARGV, argc_ptr, argv,
    //         XV_X_ERROR_PROC, x_error_proc,
    //         NULL);
    fullscreendebug = 1;

    gd.h_win_horiz_bw = h_win_horiz_bw_objects_initialize(NULL, 0);    
	
	// Set frame label as soon as possible - Make persistant for 10 seconds
	gui_label_h_frame(gd.frame_label,-10);

        // gd.dpy = (Display *) xv_get(gd.h_win_horiz_bw->horiz_bw, XV_DISPLAY);

    gd.v_win_v_win_pu = v_win_v_win_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);
	
    gd.movie_pu = movie_pu_movie_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

	// These objects are not requiured if running unmapped  
	//if(!gd.run_unmapped )
	if(1)  {

      gd.page_pu = page_pu_page_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

      gd.zoom_pu = zoom_pu_zoom_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

      gd.bookmk_pu = zoom_pu_bookmk_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

      gd.data_pu = data_pu_data_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

      gd.draw_pu = draw_pu_draw_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

      gd.over_pu = over_pu_over_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

      gd.prod_pu = prod_pu_prod_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

      gd.save_pu = save_pu_save_im_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

      gd.status_pu = status_pu_status_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

      gd.fields_pu = fields_pu_fields_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

      gd.fcast_pu = fcast_pu_fcast_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

      gd.past_pu = past_pu_past_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

      gd.gen_time_pu = gen_time_pu_popup1_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);

      gd.route_pu = route_pu_route_pu_objects_initialize(NULL, gd.v_win_v_win_pu->v_win_pu);
 
      gd.cmd_pu = cmd_pu_cmd_pu_objects_initialize(NULL, gd.h_win_horiz_bw->horiz_bw);
 
      gcc_initialize(gd.h_win_horiz_bw->horiz_bw, "Overlay Color Chooser"); 
	}

    /* get a default global GC */
    gd.def_gc = DefaultGC(gd.dpy, DefaultScreen(gd.dpy));

    /* get the horizontal view canvas's XID */
    // gd.hcan_xid = xv_get(canvas_paint_window(gd.h_win_horiz_bw->canvas1),
    //                      XV_XID);
    gd.h_win.vis_xid = gd.hcan_xid;

    /* Trap destroy events for main base window - cleanup */
    notify_interpose_destroy_func(gd.h_win_horiz_bw->horiz_bw,
                             (notify_value (*)(...)) base_win_destroy);

    /* Trap resize events in main base window */
    notify_interpose_event_func(gd.h_win_horiz_bw->horiz_bw,
                                (notify_value (*)(...)) h_win_events, NOTIFY_SAFE);

    /* get the vertical view canvas's XID */
    // gd.vcan_xid = xv_get(canvas_paint_window(gd.v_win_v_win_pu->canvas1),
    //                      XV_XID);
    gd.v_win.vis_xid = gd.vcan_xid;

    /* Trap resize events in vertical display  popup window */
    notify_interpose_event_func(gd.v_win_v_win_pu->v_win_pu,
                          ( notify_value (*)(...))   v_win_events, NOTIFY_SAFE);

    load_fonts(gd.dpy);

}
