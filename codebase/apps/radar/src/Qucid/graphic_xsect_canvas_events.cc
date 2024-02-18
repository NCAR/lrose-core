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
 * GRAPHIC_XSECT_CANVAS_EVENTS.c: Callback procedures for the Veritcal display window
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#include <X11/Xlib.h>

#define GRAPHIC_XSECT_CANVAS_EVENTS 1
#include "cidd.h"

extern int    b_lastx,b_lasty;    /* Boundry end point */
extern int    b_startx,b_starty;    /* Boundry start point */

#ifdef NOTNOW

static void handle_click(Event *event, int click_x, int click_y);

/*************************************************************************
 * Event callback function for `canvas1'.
 */
Notify_value v_can_events( Window win, Event *event, Notify_arg arg, 
                           Notify_event_type type)
{
  /* Process mouse and Keyboard events here */
  int x_gd,y_gd;  /* Data grid coords */
  double  x_dproj,ht_km;      /* Km coords */
  int field;              /* Data field number */
  int out_of_range_flag;  /* 1 = no such grid point in visible data */
  fl32 *ptr;              /* pointer to the data */
  unsigned short *uptr;  /* pointer to short data */
  double value;           /* data value after scaling */
  met_record_t *mr;       /* convienence pointer to a data record */
  char    text[128];

  /* Move to front of and motion drag events */
  while (XCheckMaskEvent(gd.dpy, PointerMotionMask | ButtonMotionMask, event->ie_xevent))
    /* NULL BODY */; 


  if(event_action(event) == ACTION_MENU ) { /* Right button down  */
    if(event_is_down(event)) {
      menu_show(v_field_mu_gen_proc(0, MENU_DISPLAY),win,event,NULL);
    }
  }

  if(event_action(event) == ACTION_ADJUST ) { /* Middle button down  */
    if(event_is_down(event)) {
      gd.save_im_win = XSECT_VIEW;
      update_save_panel();
      if(_params.enable_save_image_panel)
        // xv_set(gd.save_pu->save_im_pu,FRAME_CMD_PUSHPIN_IN, TRUE,XV_SHOW, TRUE,NULL);
    }
  }
     
  if(event_action(event) == ACTION_SELECT ) { /* Left button up or down  */
    if(event_is_down(event)) {    //  Initiate Zooming
      gd.zoom_in_progress = 2;
      b_startx = event_x(event);
      b_starty = event_y(event);
      b_lastx = b_startx;
      b_lasty = b_starty;
    } else {                     // Button Up - Report or End Zoom.
      gd.zoom_in_progress = 0;
      b_lastx = event_x(event);
      b_lasty = event_y(event);
      field = gd.v_win.page;
      mr = gd.mrec[field];
      pixel_to_disp_proj_v(&gd.v_win.margin,b_startx,b_starty,&x_dproj,&ht_km);

#ifdef MARGIN_CLICK_CHANGES_HT

      if(b_startx <= gd.v_win.margin.left +2) {
        mr = choose_ht_sel_mr(gd.h_win.page);
        double dist1,dist2;
        int index;

        // Search for the closest plane
        dist1 = DBL_MAX;
        index = 0; 

        // printf("YPIX: %d,  height_from_pixel: %g\n",b_starty,ht_km);
        for(int i=0; i < mr->ds_fhdr.nz; i++ ) { 
          dist2 = fabs(ht_km - mr->vert[i].cent);
          if(dist2 < dist1) { 
            index = i;
            dist1 = dist2;
          }
        }

        // Move all of the height related stuff
        set_height(index);

        if(gd.debug1) fprintf(stderr,"Height = %.1f - Index: %d\n",ht_km,index);

      } else {  // Click release is in the view.

#endif

        if ((abs(b_lastx - b_startx) > 5 || abs(b_lasty - b_starty) > 5)) {
          do_vert_zoom();

        } else {
          
          // set click point info

          handle_click(event, b_lastx, b_lasty);

          // report the data value

          out_of_range_flag = 0;
          x_gd = CLIP(x_gd,0,(mr->v_fhdr.nx -1),out_of_range_flag);
          y_gd = CLIP(y_gd,0,(mr->v_fhdr.nz -1),out_of_range_flag);

          if(out_of_range_flag) {
            sprintf(text,"V X: %.2fkm   Y: %.2fkm  Is not in this data",x_dproj,ht_km);
            gui_label_v_frame(text);

          } else {

            ptr = mr->v_fl32_data;
            uptr = (unsigned short*) mr->v_data;
            if(ptr != NULL) {
              // Pick out correct grid point
              ptr += (mr->v_fhdr.nx * y_gd) + x_gd;
              uptr += (mr->v_fhdr.nx * y_gd) + x_gd;
              if( *ptr ==  (mr->v_mdvx->getFieldByNum(0))->getFieldHeader().missing_data_value ||
                  *ptr ==  (mr->v_mdvx->getFieldByNum(0))->getFieldHeader().bad_data_value) {
                sprintf(text,"Bad or Missing data at this location");
              } else {
                if (mr->v_fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
                  value = exp((double) *ptr);
                } else {
                  value = *ptr;
                }

                if(value < 0.0001 && value > -0.0001) value = 0.0; // Deal with values very close to 0  
                double uval = *uptr * mr->v_fhdr.scale + mr->v_fhdr.bias;
                sprintf(text,"Value: %.3g UVal: %.3g %s: %.3f km along line, %.3f %s",
                        value,uval,mr->legend_name,x_dproj,ht_km,mr->units_label_sects);
              }
              gui_label_v_frame(text);
            }
          }
        }
#ifdef MARGIN_CLICK_CHANGES_HT
      }
#endif


    }
  }



  if (event_action(event) == LOC_DRAG ) {
    if(event_left_is_down(event)) { //  Zoom
      redraw_vert_zoom_box(); // Erase the last one.
      b_lastx = event_x(event);
      b_lasty = event_y(event);
      redraw_vert_zoom_box(); // Redraw
    }
  }

  ///////REPORT MODE ////////////////////
  if(gd.report_mode && event_id(event) == LOC_MOVE ) {
    mr = gd.mrec[gd.v_win.page];

    if( gd.movie.cur_frame == gd.movie.last_frame &&
        gd.movie.movie_on == 0 &&
        mr->v_data_valid == 1 &&
        mr->v_fl32_data != NULL ) {

      // Keep track of where last report was placed
      static int report_visible = 0;
      static int x1 = gd.v_win.can_dim.x_pos;
      static int y1 = gd.v_win.can_dim.y_pos;
      static int xwidth = gd.v_win.can_dim.width;
      static int yheight = gd.v_win.can_dim.height;

      int len,direct,ascent,descent;
      int xpos,ypos;
      XCharStruct overall;

      xpos = event_x(event);
      ypos = event_y(event);
      pixel_to_disp_proj_v(&gd.v_win.margin,xpos,ypos,&x_dproj,&ht_km);
      disp_proj_to_grid_v(mr,x_dproj,ht_km,&x_gd,&y_gd);
      out_of_range_flag = 0;
      x_gd = CLIP(x_gd,0,(mr->v_fhdr.nx -1),out_of_range_flag);
      if (mr->v_fhdr.vlevel_type == Mdvx::VERT_TYPE_AZ) {
        y_gd = CLIP(y_gd,0,(mr->v_fhdr.ny -1),out_of_range_flag);
      } else {
        y_gd = CLIP(y_gd,0,(mr->v_fhdr.nz -1),out_of_range_flag);
      }
      ptr = mr->v_fl32_data;
      // Pick out correct grid cell
      ptr += (mr->v_fhdr.nx * (y_gd)) + (x_gd);
      if(report_visible) {
        XCopyArea(gd.dpy,gd.v_win.can_xid[gd.v_win.cur_cache_im],
                  gd.v_win.vis_xid,
                  gd.def_gc,    x1, y1,
                  xwidth, yheight, x1, y1);
      }

      if(xpos < gd.v_win.img_dim.x_pos ||
         xpos > gd.v_win.img_dim.x_pos + gd.v_win.img_dim.width ||
         ypos < gd.v_win.img_dim.x_pos ||
         ypos > gd.v_win.img_dim.x_pos + gd.v_win.img_dim.height)
        out_of_range_flag = 1;

      if(out_of_range_flag != 0 ||
         *ptr == (mr->v_mdvx->getFieldByNum(0))->getFieldHeader().missing_data_value ||
         *ptr == (mr->v_mdvx->getFieldByNum(0))->getFieldHeader().bad_data_value) {
        report_visible = 0;
      } else {
        report_visible = 1;
        value = *ptr;

        // Deal with values very close to 0
        if(value  < 0.0001 && value  > -0.0001) value = 0.0;

        sprintf(text,"%g",value);
        len = strlen(text);
        XTextExtents(gd.fontst[gd.prod.prod_font_num],text,len,&direct,&ascent,&descent,&overall);
        XSetFont(gd.dpy,gd.legends.foreground_color->gc,gd.ciddfont[gd.prod.prod_font_num]);
        XDrawImageString(gd.dpy,gd.v_win.vis_xid,gd.legends.foreground_color->gc,
                         xpos + 2,
                         ypos-gd.fontst[gd.prod.prod_font_num]->descent + 2,
                         text,len);

        xwidth =  overall.width;
        yheight = gd.fontst[gd.prod.prod_font_num]->ascent +
          gd.fontst[gd.prod.prod_font_num]->descent;

        x1 = xpos + 2;
        y1 = ypos - yheight + 2;
      }
    }
  } /////// End of Report Mode   ////////////

  /*
   * Process keyboard events.  Only process them when the key is
   * released to prevent repeated processing when the key is held
   * down (may want to change this in the future).
   */

  if (event_is_up(event)) {
    if(event_id(event) >= 49 && event_id(event) <= 49 +gd.num_datafields) {
      gd.v_win.page = event_id(event) - 49;
    } 

    // Extra Keys - Seen by Xview as event_ID 0
    if(event_id(event) == 0 && event->ie_xevent->xkey.keycode == 111) startup_snapshot(0);
    process_rotate_keys(event);

    // Rotate to next cached image
    if(event_id(event) == '=') {
      next_cache_image();
    }

    // Rotate to previous cached image 
    if(event_id(event) == '-') {
      prev_cache_image();
    }

    // FLIP Field is '.' (period)
    if (event_id(event) == '.' ) {
      set_field(-1);
      set_v_field(-1);
    }
 
    /*
     * Arrow keys:
     *    up    - move up one elevation
     *    down  - move down on elevation
     *    left  - move back one movie frame
     *    right - move forward one movie frame
     */

    int curr_frame;
    mr = gd.mrec[gd.v_win.page];
    switch(event_action(event)) {
      case ACTION_GO_COLUMN_BACKWARD :      /* ACTION_UP */
        if(mr->plane  < mr->ds_fhdr.nz -1)
          set_height( mr->plane + 1);
        break;

      case ACTION_GO_COLUMN_FORWARD :         /* ACTION_DOWN */
        if(mr->plane > 0)
          set_height( mr->plane - 1);
        break;

      case ACTION_GO_CHAR_BACKWARD :       /* ACTION_LEFT */
        // curr_frame = (int)xv_get(gd.movie_pu->movie_frame_sl,
        //                          PANEL_VALUE);

        if (curr_frame > 1) {
          curr_frame--;

          // xv_set(gd.movie_pu->movie_frame_sl,
          //        PANEL_VALUE, curr_frame,
          //        NULL);

          movie_frame_proc(gd.movie_pu->movie_frame_sl,
                           curr_frame,
                           (Event *)NULL);
        }

        break;
      case ACTION_GO_CHAR_FORWARD :         /* ACTION_RIGHT */
        // curr_frame = (int)xv_get(gd.movie_pu->movie_frame_sl, PANEL_VALUE);
        // int max_frame = (int)xv_get(gd.movie_pu->movie_frame_sl, PANEL_MAX_VALUE);
        int max_frame = 0;
        if (curr_frame < max_frame) { curr_frame++;
          // xv_set(gd.movie_pu->movie_frame_sl,
          //        PANEL_VALUE, curr_frame,
          //        NULL);

          movie_frame_proc(gd.movie_pu->movie_frame_sl,
                           curr_frame,
                           (Event *)NULL);
        }
        break;

    } /* endswitch - event_action(event) */

  }
	
  return notify_next_event_func(win, (Notify_event) event, arg, type);
}

/*************************************************************************
 * get click point info, set the coord values in shmem
 */

static void handle_click(Event *event, int click_x, int click_y)

{

  struct timeval tp;
  struct timezone tzp;
  gettimeofday(&tp, &tzp); /* record when this event took place */
  gd.last_event_time = tp.tv_sec;

  // compute x distance in vsection, and ht

  double xDistKm, htKm;
  pixel_to_disp_proj_v(&gd.v_win.margin, click_x, click_y,
                       &xDistKm, &htKm);

  // get km location of click pt from origin
  
  double xclickKm, yclickKm;
  pixel_to_disp_proj_v(&gd.v_win.margin, click_x, click_y,
                       &xclickKm,&yclickKm);
  
  // get start and end pts for cross section
  
  double startLat, startLon;
  gd.proj.xy2latlon(gd.h_win.route.x_world[0],
                    gd.h_win.route.y_world[0],
                    startLat, startLon);

  double endLat, endLon;
  gd.proj.xy2latlon(gd.h_win.route.x_world[1],
                    gd.h_win.route.y_world[1],
                    endLat, endLon);
  
  // get length and direction of cross section
  
  double r, theta;
  PJGLatLon2RTheta(startLat, startLon, endLat, endLon, &r, &theta);
  
  // compute lat/lon of click point
  
  double clickLat, clickLon;
  PJGLatLonPlusRTheta(startLat, startLon, xDistKm, theta,
                      &clickLat, &clickLon);
  
  // compute x/y of click point in km
  
  double clickXKm, clickYKm;
  gd.proj.latlon2xy(clickLat, clickLon, clickXKm, clickYKm);

  // compute radar azimuth and elevation of click point, relative
  // to data origin

  double clickRangeKm = sqrt(clickXKm * clickXKm + clickYKm * clickYKm);
  
  double clickAzDeg = atan2(clickXKm, clickYKm) * RAD_TO_DEG;
  if (clickAzDeg < 0) clickAzDeg += 360.0;

  double twiceRad = PSEUDO_RADIUS * 2.0;
  met_record_t *mr = gd.mrec[gd.v_win.page];
  double radarHt = mr->v_mhdr.sensor_alt;
  double htCorr = (clickRangeKm * clickRangeKm) / twiceRad;
  double relHtKm = htKm - radarHt - htCorr;
  double clickElevDeg = (90.0 - (atan2(clickRangeKm,relHtKm) * DEG_PER_RAD));

  time_t startTime = mr->v_mhdr.time_begin;
  time_t endTime = mr->v_mhdr.time_end;

  if (gd.debug1) {

    cerr << "=================================" << endl;
    cerr << "Got click in vertical section" << endl;

    cerr << "  data startTime: " << DateTime::strm(startTime) << endl;
    cerr << "  data endTime: " << DateTime::strm(endTime) << endl;

    cerr << "  click_x, click_y: " 
         << click_x << ", " << click_y << endl;

    cerr << "  xclickKm, yclickKm: " 
         << xclickKm << ", " << yclickKm << endl;

    cerr << "  htKm: " << htKm << endl;
    cerr << "  xDistKm: " << xDistKm << endl;
    
    cerr << "  start vsect x, y: "
         << gd.h_win.route.x_world[0] << ", "
         << gd.h_win.route.y_world[0] << endl;
    
    cerr << "  end vsect x, y: "
         << gd.h_win.route.x_world[1] << ", "
         << gd.h_win.route.y_world[1] << endl;
        
    cerr << "  startLat: " << startLat << endl;
    cerr << "  startLon: " << startLon << endl;
    cerr << "  endLat: " << endLat << endl;
    cerr << "  endLon: " << endLon << endl;
    cerr << "  r: " << r << endl;
    cerr << "  theta: " << theta << endl;
    cerr << "  clickLat: " << clickLat << endl;
    cerr << "  clickLon: " << clickLon << endl;
    cerr << "  clickXKm: " << clickXKm << endl;
    cerr << "  clickYKm: " << clickYKm << endl;
    cerr << "  clickRangeKm: " << clickRangeKm << endl;
    cerr << "  radarHt: " << radarHt << endl;
    cerr << "  htCorr: " << htCorr << endl;
    cerr << "  relHtKm: " << relHtKm << endl;
    cerr << "  clickAzDeg: " << clickAzDeg << endl;
    cerr << "  clickElevDeg: " << clickElevDeg << endl;

    cerr << "=================================" << endl;

  }

  // set shared memory

  gd.coord_expt->button = event->ie_xevent->xbutton.button;
  gd.coord_expt->selection_sec = tp.tv_sec;
  gd.coord_expt->selection_usec = tp.tv_usec;
  gd.coord_expt->time_data_start = startTime;
  gd.coord_expt->time_data_end = endTime;
  gd.coord_expt->pointer_x = xclickKm;
  gd.coord_expt->pointer_y = yclickKm;
  gd.coord_expt->pointer_lon = clickLon;
  gd.coord_expt->pointer_lat = clickLat;
  gd.coord_expt->pointer_alt_min = htKm;
  gd.coord_expt->pointer_alt_max = htKm;
  gd.coord_expt->data_altitude = htKm;
  gd.coord_expt->pointer_ht_km = htKm;
  gd.coord_expt->pointer_range_km = clickRangeKm;
  gd.coord_expt->pointer_az_deg = clickAzDeg;
  gd.coord_expt->pointer_el_deg = clickElevDeg;
  gd.coord_expt->click_type = CIDD_USER_CLICK;
  gd.coord_expt->click_is_for_vsection = 1;
  gd.coord_expt->pointer_seq_num++;

}

/*************************************************************************
 * Repaint callback function for `canvas1'.
 */
void v_can_repaint( Canvas canvas, Window paint_window, Display *display,
                    Window xid) // , Xv_xrectlist    *rects )
{
  Drawable c_xid;
  int c_field = gd.v_win.page;
  // Use Unused parameters
  canvas = 0; paint_window = 0; display = NULL; xid = 0; /* rects = NULL; */


  if(gd.mrec[c_field]->auto_render) {
    c_xid = gd.v_win.page_xid[c_field];
  } else {
    c_xid = gd.v_win.tmp_xid;
  }
  if((gd.v_win.vis_xid != 0) && (c_xid != 0)) {

    if (gd.debug2) fprintf(stderr,"\nDisplaying Vert final image - xid: %ld to xid: %ld\n",
                           c_xid,gd.v_win.vis_xid);
    XCopyArea(gd.dpy,c_xid,
              gd.v_win.vis_xid,
              gd.def_gc,  0,0,
              gd.v_win.can_dim.width,
              gd.v_win.can_dim.height,
              gd.v_win.can_dim.x_pos,
              gd.v_win.can_dim.y_pos);
  }
}

#endif
