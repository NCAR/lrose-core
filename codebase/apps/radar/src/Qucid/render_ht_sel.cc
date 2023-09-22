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
 * RENDER_HT_SEL.C :
 */

#define RENDER_HT_SEL

#include "cidd.h"

//////////////////////////////////////////////////////////////////////////
// HEIGHT_FROM_PIXEL Return the height given a pixel location, relative
//                   to the height axis

double height_from_pixel(int y_pixel,met_record_t    *mr)
{
    return (mr->ht_pixel * y_pixel + mr->y_intercept);
}

//////////////////////////////////////////////////////////////////////////
// CHOOSE_HT_SEL_MR : Choose an appripriate data record to render
//                    a height selector for the current page.

met_record_t    *choose_ht_sel_mr(int page)
{
     int i;
     met_record_t *mr;

     // First check the key field for the current page
     mr = gd.mrec[page];
     if((mr->vert[mr->ds_fhdr.nz -1].max - mr->vert[0].min) != 0.0 &&
	mr->composite_mode == FALSE && mr->ds_fhdr.nz > 1) {
       return mr; // Is a 3-D field
     }

     // Now look through any other data, that is displayed which
     // Might contain 3-D data

     // Layered GRIDS
     for(i=0; i < NUM_GRID_LAYERS; i++) {
       mr = gd.mrec[gd.layers.overlay_field[i]];
       if(gd.layers.overlay_field_on[i]  &&
	 (mr->vert[mr->ds_fhdr.nz -1].max - mr->vert[0].min) != 0.0 &&
	 mr->composite_mode == FALSE && mr->ds_fhdr.nz > 1) {
         return mr; // Is a 3-D field
       }
     }

     // CONTOURS
     for(i=0; i < NUM_CONT_LAYERS; i++) {
       mr = gd.mrec[gd.layers.cont[i].field];
       if(gd.layers.cont[i].active &&
	 (mr->vert[mr->ds_fhdr.nz -1].max - mr->vert[0].min) != 0.0 &&
	 mr->composite_mode == FALSE && mr->ds_fhdr.nz > 1) {
         return mr; // Is a 3-D field
       }
     }

     // WINDS
     if(gd.layers.wind_vectors) {
       for(i=0; i < gd.layers.num_wind_sets; i++) {
         mr = gd.layers.wind[i].wind_u;
         if(gd.layers.wind[i].active  &&  gd.layers.wind_vectors && 
	   (mr->vert[mr->ds_fhdr.nz -1].max - mr->vert[0].min) != 0.0 &&
	   mr->composite_mode == FALSE && mr->ds_fhdr.nz > 1) {
          return mr; // Is a 3-D field
         }
       }
     }

     // If nothing is 3-D go with the key data field
     return  gd.mrec[page];
}

/************************************************************************
 * DRAW_HEIGHT_SELECTOR : Draw a Vertical height Selector/indicator
 */

void
draw_height_selector(Display *dpy, Drawable xid, GC gc_axis, GC gc_ind,
                 int page, int x1,  int y1,  int width,  int height)
	      
{
    int 	i;
    int 	y_size;
    int		y_pos, x_pos, y_pos2;
    int 	xmid, ymid;
    int		y_max;
    int		label_height = 0,label_width = 0;
    double      base,top;
    // double      range;
    double      value;

    met_record_t *mr;
    char    	label[LABEL_LENGTH];
    Font    	font;

    if(height < 20 || width < 5) return;

    mr = choose_ht_sel_mr(page);
    if(mr->composite_mode == TRUE) return;

    base = mr->vert[0].min;
    top = mr->vert[mr->ds_fhdr.nz -1].max;
    // range = top - base;


    // Calc slope of function. 
    // Reserve 5% of the space for the top label
    mr->ht_pixel = (top - base) / (-height * .95); // Slope
    y_max = (int) (y1 + height);
    mr->y_intercept = base - (mr->ht_pixel * y_max); // Y intercept

   if( mr->ds_fhdr.nz > 1 ||
       mr->ds_fhdr.vlevel_type == Mdvx::VERT_TYPE_SIGMA_P ||
       mr->ds_fhdr.vlevel_type == Mdvx::VERT_TYPE_PRESSURE ||
       mr->ds_fhdr.vlevel_type == Mdvx::VERT_TYPE_Z ||
       mr->ds_fhdr.vlevel_type == Mdvx::VERT_TYPE_SIGMA_Z ||
       mr->ds_fhdr.vlevel_type == Mdvx::VERT_TYPE_ETA ||
       mr->ds_fhdr.vlevel_type == Mdvx::VERT_TYPE_ELEV ||
       mr->ds_fhdr.vlevel_type == Mdvx::VERT_TYPE_ZAGL_FT ||
       mr->ds_fhdr.vlevel_type == Mdvx::VERT_TYPE_WRF_ETA ||
       mr->ds_fhdr.vlevel_type == Mdvx::VERT_FLIGHT_LEVEL ) { // 3D field

    // Draw the Current plane height indicator
    y_pos = (int) (y_max + ((mr->vert[mr->plane].min - base) / mr->ht_pixel));
    y_pos2 = (int) (y_max + ((mr->vert[mr->plane].max - base) / mr->ht_pixel));
    y_size = (y_pos - y_pos2) + 2;
    XFillRectangle(dpy,xid,gc_ind,x1,y_pos2,width +1,y_size);


    // Draw the axis
    x_pos =  x1 + 2 ;
    XDrawLine(dpy,xid,gc_axis,x_pos,y1,x_pos,y1 + height);

    // Calc allowable space for labels
    label_width = (width - 2); // 2 for tic 
    if(mr->ds_fhdr.nz <=0 ) {
        label_height = height;
    } else {
        label_height = height / (mr->ds_fhdr.nz * 2);
    }

    // Select which font we'll use for Labels - Use Hightest value for 
    // the template.
    sprintf(label,"%g",mr->vert[mr->ds_fhdr.nz-1].cent);
    font = choose_font(label,label_width,label_height,&xmid,&ymid);
    XSetFont(dpy,gc_axis,font);

    // Draw the axis from the bottom up - Note reversed
    // sense of the X coord system and the Axis's.
    for(i = 0; i < mr->ds_fhdr.nz; i++) {
       value = mr->vert[i].cent;
       y_pos = (int) (y_max + ((value - base) / mr->ht_pixel));

       // Draw Tics
       XDrawLine(dpy,xid,gc_axis,x_pos - 2,y_pos,x_pos + 2,y_pos);

       // Draw Label
       if(strncasecmp(mr->units_label_sects,"FL",2) == 0 ||
         (strlen(mr->units_label_sects) < 1) ) {
         sprintf(label,"%03d",(int) (mr->vert[i].cent + 0.5));
       } else {
         sprintf(label,"%g",mr->vert[i].cent);
       }
       XDrawString(dpy,xid,gc_axis, x_pos + 2, y_pos +  ymid,
		   label, strlen(label));
    }

    // Add the units label to the top - Centered, 2 pixels down
    // using the bigest font possible
    label_height = (int) (height * .05);  // We allow 5% for label
    font = choose_font(mr->units_label_sects,width,label_height,
		       &xmid,&ymid);
    XSetFont(dpy,gc_axis,font);

    XDrawImageString(dpy,xid,gc_axis,x1 + (width/ 2) + xmid,
                     y1 + (2 * ymid) + 2,
		     mr->units_label_sects, strlen(mr->units_label_sects));

      
    
  } else { // Is a 2 Dimensional field

    if (mr->ds_fhdr.vlevel_type == Mdvx::VERT_TYPE_COMPOSITE) {
      font = choose_font("Comp",width,label_height,
			 &xmid,&ymid);
      XSetFont(dpy,gc_axis,font);
      XDrawImageString(dpy,xid,gc_axis,x1 + (width/ 2) + xmid,
		       y1 + (2 * ymid) + (height / 2) ,
		       "Comp", 4);
    } else {
      font = choose_font("2D",width,label_height,
			 &xmid,&ymid);
      XSetFont(dpy,gc_axis,font);
      XDrawImageString(dpy,xid,gc_axis,x1 + (width/ 2) + xmid,
		       y1 + (2 * ymid) + (height / 2) ,
		       "2D", 2);
    }

  }
    // Surround the scale with a box
    XDrawRectangle(dpy,xid,gc_axis,x1,y1,width,height);
    return;
}
