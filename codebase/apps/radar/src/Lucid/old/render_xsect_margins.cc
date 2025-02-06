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
/************************************************************************
 * RENDER_XSECT_MARGINS.C: 
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_XSECT_MARGINS

#include "cidd.h"
#include <toolsa/str.h>

/**********************************************************************
 * DRAW_VWIN_RIGHT_LABEL:     Displays a labeled color scale if the
 * margin is set large enough
 */

void draw_vwin_right_margin( QPaintDevice *pdev, int page)
{

#ifdef NOTYET
  
  int x_start = 0;
  int y_start = 0;
    int ht,wd;
    MetRecord *mr;       /* pointer to record for convienence */

    if(gd.v_win.margin.right <= 20) return;

    mr = gd.mrec[page];    /* get pointer to data record */
 
    /* calc dimensions of drawable area */
    // y_start = gd.v_win.margin.top;
    x_start = gd.v_win.can_dim.width - gd.v_win.margin.right;
    ht = gd.v_win.can_dim.height - gd.v_win.margin.top - gd.v_win.margin.bot;
    wd = gd.v_win.margin.right;

    /* clear margin */
    XFillRectangle(gd.dpy, xid, gd.legends.margin_color->gc,
                 x_start, y_start, wd, ht);
     

    if(strncasecmp(mr->color_file,"none",4) != 0) {
        draw_colorbar(gd.dpy,xid,gd.legends.foreground_color->gc,
		      x_start,y_start,
		      wd,ht,mr->v_vcm.nentries,mr->v_vcm.vc,0,mr->field_units);
   }

#endif
    
}
 
/**********************************************************************
 * DRAW_LEFT_LABEL: Draw an axis in the left margin & label it
 */

void draw_vwin_left_margin( QPaintDevice *pdev, int page) 
{
#ifdef NOTYET
    int x_start;
    // int y_start;
    int tick_xstart,tick_xend;
    int tick_ystart;
    int xmid,ymid;
    int    label_space;
    double  range;
    double  min_val;
    double  tick_spacing;
    double  current_tick;
    char    label[128];
    Font    font;
    MetRecord *mr;       /* pointer to record for convienence */
 
    if(gd.v_win.margin.left == 0) return;
    mr = choose_ht_sel_mr(page);
    /* calc dimensions of drawable area */
    x_start = 0;
    // y_start =  gd.v_win.margin.top;
    tick_xstart = (int)(gd.v_win.margin.left * 0.75);
 

    range = gd.v_win.cmax_y - gd.v_win.cmin_y;
    tick_spacing = compute_tick_interval(range);
    label_space = (int)(gd.v_win.img_dim.height * (tick_spacing / range));

    // Select which font we'll use for Labels - Use Hightest value for
    // the template.
    if(fabs(range) > 20.0) {
      snprintf(label,128,"%.0f",mr->v_vhdr.level[mr->ds_fhdr.nz-1]);
    } else {
      snprintf(label,128,"%.1f",mr->v_vhdr.level[mr->ds_fhdr.nz-1]);
    }
    font = choose_font(label,tick_xstart,label_space,&xmid,&ymid);
    XSetFont(gd.dpy,gd.legends.height_axis_color->gc,font);
 
    if(range > 0.0) {
        min_val = rint(gd.v_win.cmin_y);
        current_tick = min_val + abs((int)min_val %  (int)((tick_spacing > 1.0) ? tick_spacing:  1.0));

        while(current_tick <= gd.v_win.cmax_y) {
            disp_proj_to_pixel_v(&gd.v_win.margin,gd.v_win.cmin_x,current_tick,&tick_xend,&tick_ystart);
            XDrawLine(gd.dpy,xid,gd.legends.height_axis_color->gc,tick_xstart,tick_ystart,tick_xend-1,tick_ystart);
 
            if(range > 20.0 ) {
              snprintf(label,128,"%.0f",current_tick);
			} else if( range > 2.0) {
              snprintf(label,128,"%.1f",current_tick);
			} else if( range >.5) {
              snprintf(label,128,"%.2f",current_tick);
			}


            XDrawString(gd.dpy,xid,gd.legends.height_axis_color->gc,
            x_start,(tick_ystart + ymid),label,strlen(label));
 
            current_tick += tick_spacing;
        };
    }  else {   // SIGMA  LEVELS or Pressure - REVERSED vertical coords 
        min_val = gd.v_win.cmax_y;
        current_tick = gd.v_win.cmin_y;

        while(current_tick >= gd.v_win.cmax_y) {
            disp_proj_to_pixel_v(&gd.v_win.margin,gd.v_win.cmin_x,current_tick,&tick_xend,&tick_ystart);
            XDrawLine(gd.dpy,xid,gd.legends.height_axis_color->gc,tick_xstart,tick_ystart,tick_xend-1,tick_ystart);
 
            if(fabs(range) > 20.0 ) {
              snprintf(label,128,"%.0f",current_tick);
			} else if( fabs(range) > 2.0) {
              snprintf(label,128,"%.1f",current_tick);
			} else if( fabs(range) >.5) {
              snprintf(label,128,"%.2f",current_tick);
			}
            if(fabs(range) > 20.0) 
              snprintf(label,128,"%.0f",current_tick);
	    else 
              snprintf(label,128,"%.1f",current_tick);

            XDrawString(gd.dpy,xid,gd.legends.height_axis_color->gc,
            x_start,(tick_ystart + ymid),label,strlen(label));
 
            current_tick -= tick_spacing;
        };
    }

    snprintf(label,128,"%s",vlevel_label(&mr->v_fhdr));

    XDrawString(gd.dpy,xid,gd.legends.height_axis_color->gc,
		x_start + 2, 2 * ymid + 1, label, strlen(label));

#endif
    
}

/**********************************************************************
 * DRAW_VWIN_TOP_LABEL: Label the top of the horizontal image
 */

void draw_vwin_top_margin(QPaintDevice *pdev, int page)
{

#ifdef NOTYET
  
    int    x_start,y_start;
    int    xmid,ymid;
    int    ht,wd;
    MetRecord *mr;        /* pointer to record for convienence */
    char    timestr[1024];
    char    label[TITLE_LENGTH * 10];
    Font    font;

    if(gd.v_win.margin.top == 0) return;
    mr = gd.mrec[page];    /* get pointer to data record */


     
    /* calc dimensions of drawable area */
    ht = gd.v_win.margin.top;
    wd = gd.v_win.can_dim.width - gd.v_win.margin.left - gd.v_win.margin.right;
    x_start =  (wd/2) + gd.v_win.margin.left;
    y_start =  ht/2;


    if(mr->v_data != NULL) {
      struct tm *gmt;
      struct tm res;
      if(_params.use_local_timestamps) {
          gmt = localtime_r((time_t *)&(mr->v_date.unix_time),&res);
      } else {
          gmt = gmtime_r((time_t *)&(mr->v_date.unix_time),&res);
      }
      strftime(timestr,64,_params.label_time_format,gmt);
      if( gd.mrec[page]->v_fhdr.proj_type == Mdvx::PROJ_RHI_RADAR) {
	if (gd.mrec[page]->v_fhdr.vlevel_type == Mdvx::VERT_TYPE_AZ) {
	  snprintf(label,TITLE_LENGTH * 10," %s  RHI: %g deg  -  %s" ,
		  mr->legend_name, mr->v_fhdr.grid_minz,timestr);
	} else {
	  snprintf(label,TITLE_LENGTH * 10," %s  RHI: %g deg  -  %s" ,
		  mr->legend_name, mr->v_fhdr.grid_miny,timestr);
	}
      } else {
        snprintf(label,TITLE_LENGTH * 10,"%s   %s Cross Section %s",
		gd.h_win.route.route_label,mr->legend_name,timestr);      
      }
    } else {
      snprintf(label,TITLE_LENGTH * 10,"%s   %s Cross Section - Data Not Available",
	gd.h_win.route.route_label,
        mr->legend_name);
    }

    font = choose_font(label,wd,ht,&xmid,&ymid);
    XSetFont(gd.dpy,gd.legends.foreground_color->gc,font);
    XDrawString(gd.dpy,xid,gd.legends.foreground_color->gc,
        x_start + xmid,y_start + ymid,label,strlen(label));

#endif

}

/**********************************************************************
 * DRAW_VWIN_BOT_LABEL: Draw an axis and lable the bottom margin
 */

void draw_vwin_bot_margin(QPaintDevice *pdev, int page)
{

#ifdef NOTYET
  
    int i;
    int tick_xstart;
    int tick_ystart,tick_yend;
    int xmid,ymid;
    int label_space;
    double  range;
    double  tick_spacing;
    double  current_tick;
    char    label[16];
    Font    font;
    // MetRecord *mr;       /* pointer to record for convienence */

    double unit_per_km;
    const char *u_label;

    unit_per_km = _params.scale_units_per_km;
    u_label = _params.scale_units_label;

    if(gd.v_win.margin.bot == 0) return;
    // mr = gd.mrec[page];    /* get pointer to data record */
    /* calc dimensions of drawable area */
    tick_yend = (int)(gd.v_win.can_dim.height - (gd.v_win.margin.bot * 0.75));

 
    range = gd.h_win.route.total_length;
    range *= unit_per_km;

    tick_spacing = compute_tick_interval(range);
    current_tick = 0.0;
 
    label_space = (int)(gd.v_win.img_dim.width * (tick_spacing / range));
    while(current_tick <= range) {
        disp_proj_to_pixel_v(&gd.v_win.margin,(current_tick / unit_per_km),gd.v_win.cmin_y,
				  &tick_xstart,&tick_ystart);
        XDrawLine(gd.dpy,xid,gd.legends.foreground_color->gc,tick_xstart,tick_ystart,
			tick_xstart,tick_yend);
 
	if(range > 7.5) {
          snprintf(label,16,"%.0f",current_tick);
	} else {
          snprintf(label,16,"%.1f",current_tick);
	}
        font = choose_font(label,label_space,gd.v_win.margin.bot-2,&xmid,&ymid);
        XSetFont(gd.dpy,gd.legends.foreground_color->gc,font);
        XDrawString(gd.dpy,xid,gd.legends.foreground_color->gc,
            (tick_xstart + xmid),gd.v_win.can_dim.height -2,label,strlen(label));         
 
        current_tick += tick_spacing;
    };

    // Draw route way point reference lines and labels
    if(gd.h_win.route.num_segments > 0) {
      // Set GC to draw 2 pixel wide, dashed lines
      XSetLineAttributes(gd.dpy,gd.legends.route_path_color->gc,
		       2,LineOnOffDash,CapButt,JoinRound);

      // Label the starting point
      font = choose_font(gd.h_win.route.navaid_id[0],
		    gd.v_win.img_dim.width/gd.h_win.route.num_segments,
		    gd.v_win.margin.bot-2,&xmid,&ymid); 
      XSetFont(gd.dpy,gd.legends.route_path_color->gc,font);
      XDrawString(gd.dpy,xid,gd.legends.route_path_color->gc,
			 gd.v_win.margin.left +1 ,gd.v_win.can_dim.height - gd.v_win.margin.bot -1,
			 gd.h_win.route.navaid_id[0],
			 strlen(gd.h_win.route.navaid_id[0]));
      range = 0;
      for(i=0; i < gd.h_win.route.num_segments -1 ; i++) {
	range += gd.h_win.route.seg_length[i];
	disp_proj_to_pixel_v(&gd.v_win.margin,range,gd.v_win.cmin_y,&tick_xstart,&tick_ystart);
        XDrawLine(gd.dpy,xid,gd.legends.route_path_color->gc,
		  tick_xstart,tick_ystart,tick_xstart,gd.v_win.margin.top);
	// Label each waypoint 
	font = choose_font(gd.h_win.route.navaid_id[i+1],
			    gd.v_win.img_dim.width/gd.h_win.route.num_segments,
			    gd.v_win.margin.bot-2,&xmid,&ymid); 
	XSetFont(gd.dpy,gd.legends.route_path_color->gc,font);
        XDrawString(gd.dpy,xid,gd.legends.route_path_color->gc,
			 tick_xstart + xmid,tick_ystart-1,
			 gd.h_win.route.navaid_id[i+1],
			 strlen(gd.h_win.route.navaid_id[i+1]));
      }
      // Label the End point
      font = choose_font(gd.h_win.route.navaid_id[gd.h_win.route.num_segments],
		    gd.v_win.img_dim.width/gd.h_win.route.num_segments,
		    gd.v_win.margin.bot-2,&xmid,&ymid); 
      XSetFont(gd.dpy,gd.legends.route_path_color->gc,font);
      XDrawString(gd.dpy,xid,gd.legends.route_path_color->gc,
			 gd.v_win.can_dim.width - gd.v_win.margin.right + (2 * xmid),
			 gd.v_win.can_dim.height - gd.v_win.margin.bot -1,
			 gd.h_win.route.navaid_id[gd.h_win.route.num_segments],
			 strlen(gd.h_win.route.navaid_id[gd.h_win.route.num_segments]));

    }

    // Add the scale units
    font = choose_font(u_label,label_space,gd.v_win.margin.bot,&xmid,&ymid);

    XSetFont(gd.dpy,gd.legends.foreground_color->gc,font);
    XDrawImageString(gd.dpy,xid,gd.legends.foreground_color->gc,
        gd.v_win.can_dim.width + (2 * xmid),
	gd.v_win.can_dim.height - 2,u_label,strlen(u_label));

#endif
    
}

