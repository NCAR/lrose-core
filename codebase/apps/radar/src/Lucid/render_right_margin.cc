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
 * RENDER_RIGHT_MARGIN
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#include "cidd.h"

/**********************************************************************
 * DRAW_HWIN_RIGHT_LABEL: Displays a labeled color scale in the right 
 *        margin if it is set large enough
 */

int draw_hwin_right_margin( QPaintDevice *pdev, int  page)
{

#ifdef NOTYET
  
    int i;
    int x_start,y_start;
    int have_overlay_fields;
    int    ht,wd;
    met_record_t *mr;        /* pointer to record for convienence */

    if(gd.h_win.margin.right <= 0) return 0;

    /* clear right margin */
    XFillRectangle(gd.dpy, xid, gd.legends.margin_color->gc,
                 gd.h_win.can_dim.width - gd.h_win.margin.right,
                 gd.h_win.margin.top,
                 gd.h_win.margin.right, gd.h_win.img_dim.height);

    mr = gd.mrec[page];    /* get pointer to primary data record */

    have_overlay_fields = 0;
    for(i=0; i < NUM_GRID_LAYERS; i++) {
	if(gd.layers.overlay_field_on[i]) have_overlay_fields = 1;
    }
     
    /* split into halfs - & Layer 0 on top, Highest layer on Bottom */
    if(have_overlay_fields) { /* split into halfs - & Draw Two scales */

        /* calc dimensions of drawable area */
        x_start = gd.h_win.can_dim.width - gd.h_win.margin.right;
        y_start =  gd.h_win.margin.top;

        /* Use half the left margin minus 1/2 a top margin width */
        ht = (gd.h_win.can_dim.height - (gd.h_win.margin.top * 2) - gd.h_win.margin.bot) / 2;
        wd = gd.h_win.margin.right;
	if(_params.show_height_sel) wd /= 2;
    
        if(strncasecmp(mr->color_file,"none",4) != 0) {
          draw_colorbar(gd.dpy,xid,gd.legends.foreground_color->gc,x_start,y_start,wd,ht,
	    mr->h_vcm.nentries,mr->h_vcm.vc,0,mr->field_units);
	} else {
	       int xmid,ymid;
	       Font font = choose_font(mr->field_units, wd, wd, &xmid, &ymid);
	       XSetFont(gd.dpy, gd.legends.foreground_color->gc,font);
	       XDrawImageString(gd.dpy, xid, gd.legends.foreground_color->gc,
				x_start + 2, (y_start + (ymid *2) +1),
				mr->field_units,
				strlen(mr->field_units));

	}

        /* separate the colorscales by a top margin width */
        y_start =  y_start + ht + gd.h_win.margin.top;
    
	for(i=0; i < NUM_GRID_LAYERS; i++) {
	  if(gd.layers.overlay_field_on[i]) {
            mr = gd.mrec[gd.layers.overlay_field[i]];    /* get pointer to data record */
	    if(strncasecmp(mr->color_file,"none",4) != 0) {
              draw_colorbar(gd.dpy,xid,gd.legends.foreground_color->gc,x_start,y_start,wd,ht,
		mr->h_vcm.nentries,mr->h_vcm.vc,0,mr->field_units);
	    } else {
	       int xmid,ymid;
	       Font font = choose_font(mr->field_units, wd, wd, &xmid, &ymid);
	       XSetFont(gd.dpy, gd.legends.foreground_color->gc,font);
	       XDrawImageString(gd.dpy, xid, gd.legends.foreground_color->gc,
				x_start + 2, (y_start + (ymid *2) +1),
				mr->field_units,
				strlen(mr->field_units));

	    }
	  }
	}

    } else {  /* Draw the primary colorscale only */
        mr = gd.mrec[page];    /* get pointer to data record */
        /* calc dimensions of drawable area */
        x_start = gd.h_win.can_dim.width - gd.h_win.margin.right;
        y_start =  gd.h_win.margin.top;
        ht = gd.h_win.can_dim.height - gd.h_win.margin.top - gd.h_win.margin.bot;
        wd = gd.h_win.margin.right;
	if(_params.show_height_sel) wd /= 2;
    
        if(strncasecmp(mr->color_file,"none",4) != 0) {
            draw_colorbar(gd.dpy,xid,gd.legends.foreground_color->gc,
		     x_start,y_start,wd,ht,mr->h_vcm.nentries,mr->h_vcm.vc,0,mr->field_units);
	} else {
	       int xmid,ymid;
	       Font font = choose_font(mr->field_units, wd, wd, &xmid, &ymid);
	       XSetFont(gd.dpy, gd.legends.foreground_color->gc,font);
	       XDrawImageString(gd.dpy, xid, gd.legends.foreground_color->gc,
				x_start + 2, (y_start + (ymid *2) +1),
				mr->field_units,
				strlen(mr->field_units));

	}
    }

    if(_params.show_height_sel) {
	x_start = gd.h_win.can_dim.width - (gd.h_win.margin.right/2) - 1;
	y_start =  gd.h_win.margin.top;
	ht = gd.h_win.can_dim.height - gd.h_win.margin.top -
	              gd.h_win.margin.bot;
	wd = gd.h_win.margin.right / 2;
	draw_height_selector(gd.dpy,xid,gd.legends.height_axis_color->gc,
			       gd.legends.height_indicator_color->gc,
			       page,x_start,y_start,wd,ht);
    }

#endif
    
    return 1;
}
