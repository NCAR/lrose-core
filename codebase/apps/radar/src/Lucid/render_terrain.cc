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
 * RENDER_TERRAIN
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_TERRAIN 1

#include "cidd.h"

/**********************************************************************
 * RENDER__H_TERRAIN: Render masking terrain whereever the terrain is
 *   above the data plane
 *    
 */

void render_h_terrain( QPaintDevice *pdev, int page)
{

#ifdef NOTYET
  
    int    i,j;
    int    ht,wd;               /* Dims of data rectangles  */
    int    startx,endx;        /* pixel limits of data area */
    int    starty,endy;        /* pixel limits of data area */
    double    x_dproj,y_dproj;
    double val;
    double altitude;           /* plane height of main grid */
    int    x_start[MAX_COLS + 4];    /* canvas rectangle begin  coords */
    int    y_start[MAX_ROWS + 4];    /* canvas  rectangle begin coords */
    double    r_ht,r_wd;        /*  data point rectangle dims */
    unsigned short    *ptr;
    unsigned short    miss;           /* Missing data value */
    unsigned short    bad;            /* Bad data value */

    met_record_t *mr = gd.layers.earth.terr;

    // Convert altitude into the Grid's vertical units
    altitude = gd.mrec[page]->vert[gd.mrec[page]->plane].min / _params.terrain_height_scaler;;

    //fprintf(stderr, "Terrain Masking above %g\n",altitude);
    // If the projections match - Can use fast Rectangle rendering.
	if(mr->h_fhdr.proj_type == gd.proj.getProjType() &&
		  (gd.proj.getProjType() == Mdvx::PROJ_LATLON ||
		  ((fabs(gd.h_win.origin_lat - mr->h_fhdr.proj_origin_lat) < 0.01) &&
		(fabs(gd.h_win.origin_lon - mr->h_fhdr.proj_origin_lon) < 0.01)))) {

      ptr = (unsigned short *) mr->h_data;
      if(ptr == NULL) return;

      /* Calculate Data to Screen Mapping */
      grid_to_disp_proj(mr,0,0,&x_dproj,&y_dproj);
      x_dproj -= mr->h_fhdr.grid_dx/2.0;    /* expand grid coord by half width */
      y_dproj -= mr->h_fhdr.grid_dy/2.0;
      disp_proj_to_pixel(&gd.h_win.margin,x_dproj,y_dproj,&startx,&starty);

      grid_to_disp_proj(mr,mr->h_fhdr.nx-1,mr->h_fhdr.ny-1,&x_dproj,&y_dproj);
      x_dproj += mr->h_fhdr.grid_dx/2.0;    /* expand grid coord by half width */
      y_dproj += mr->h_fhdr.grid_dy/2.0;
      disp_proj_to_pixel(&gd.h_win.margin,x_dproj,y_dproj,&endx,&endy);

      /* get data point rectangle size */
      r_ht =  (double)(ABSDIFF(starty,endy))  / (double) mr->h_fhdr.ny;
      r_wd =  (double)(endx - startx)/ (double) mr->h_fhdr.nx;    

      /* Calc starting coords for the X,Y array */
      for(j=0;j <= mr->h_fhdr.nx; j++) {
        x_start[j] = (int) (((double) j * r_wd) + startx);
        if(x_start[j] < 0) x_start[j] = 0;
        if(x_start[j] > gd.h_win.can_dim.width) x_start[j] = gd.h_win.can_dim.width;
      }

      for(i= 0; i <= mr->h_fhdr.ny; i++) {
        y_start[i] = (int) (starty - ((double) i * r_ht));
        if(y_start[i] < 0) y_start[i] = 0;
        if(y_start[i] >= gd.h_win.can_dim.height) y_start[i] = gd.h_win.can_dim.height -1;
      }

      if(mr->num_display_pts <=0) mr->num_display_pts = mr->h_fhdr.ny * mr->h_fhdr.nx;

      miss = (unsigned short) mr->h_fhdr.missing_data_value;
      bad = (unsigned short) mr->h_fhdr.bad_data_value;
      ht = (int) (r_ht + 1.0); 
      wd = (int) (r_wd + 1.0);

      for(i= 0; i < mr->h_fhdr.ny; i++) y_start[i]-= (ht -1);
        for(i= 0; i < mr->h_fhdr.ny; i++) {
            for(j=0;j< mr->h_fhdr.nx; j++) {
                if(*ptr != miss && *ptr != bad) {
                    val =  ((mr->h_fhdr.scale * *ptr) + mr->h_fhdr.bias);
                    if(val >= altitude) 
                        XFillRectangle(gd.dpy,xid,gd.layers.earth.color2->gc,
				       x_start[j],y_start[i],wd,ht);
                }
                ptr++;
            }
       }

	 } else {
	   if(gd.debug)
	     fprintf(stderr,"Terrain masking not implemented for distorted grids yet\n");
	 }


#endif
        
    return;
}

/************************************************************************
 * RENDER_V_TERRAIN
 */


void render_v_terrain( QPaintDevice *pdev )
{

#ifdef NOTYET
  
    int    i;
    int    num_points;
    int    start_x,end_x;
    int    start_y,end_y;
    int    x_pix,y_pix;        /* pixel locations */
    double cell_width;
    double height,dist;
    double scaler;
    unsigned short    *ptr;
    unsigned short    miss;            /* Missing data value */
    unsigned short    bad;            /* Missing data value */
    XPoint      xpt[MAX_COLS];
    static int first_time = 1;
    met_record_t *mr = gd.layers.earth.terr;

    if(first_time) {
      XSetLineAttributes(gd.dpy, gd.layers.earth.color1->gc, 3,
                         LineSolid, CapButt, JoinMiter); 
      XSetLineAttributes(gd.dpy, gd.layers.earth.color2->gc, 3,
                         LineSolid, CapButt, JoinMiter);
      first_time = 0;
    }

    if(xid == 0) return;

    // Convert altitude into the Grid's vertical units 
    scaler  = _params.terrain_height_scaler;

    ptr =  (unsigned short *) mr->v_data;
    if(ptr == NULL ) return;
    
    miss = (unsigned short) mr->v_fhdr.missing_data_value; 
    bad = (unsigned short) mr->v_fhdr.bad_data_value; 

    // The far left pixel - at the first data point's height
    height = ((mr->v_fhdr.scale * *ptr) + mr->v_fhdr.bias) * scaler;
    disp_proj_to_pixel_v(&gd.v_win.margin,0.0,height,&start_x,&start_y);
    disp_proj_to_pixel_v(&gd.v_win.margin,gd.h_win.route.total_length,height,&end_x,&end_y);
    cell_width = (double)ABSDIFF(end_x,start_x) / (double) mr->v_fhdr.nx;   
    xpt[0].x = start_x;
    xpt[0].y = start_y;
    num_points = 1;

    dist = start_x + (cell_width / 2.0);  // start at midpoint of grid cell

    for(i= 0; i < mr->v_fhdr.nx; i++) {    /* Calc  coords for the array */
        if(*ptr != miss && *ptr != bad) {
	    // Put into local coords */
	    height =  ((mr->v_fhdr.scale * *ptr) + mr->v_fhdr.bias) * scaler;
            disp_proj_to_pixel_v(&gd.v_win.margin,0.0,height,&x_pix,&y_pix);
	    //printf("%d: height: %u  %g, y_pix: %d\n",i,*ptr,height,y_pix);
	    xpt[num_points].y = y_pix;
	    xpt[num_points].x = (int) rint(dist);
	    num_points++;
	}
	dist += cell_width;
        ptr++;
    }

    // The far right pixel - at the last data point's height
    disp_proj_to_pixel_v(&gd.v_win.margin,gd.h_win.route.total_length,height,&end_x,&end_y);
    xpt[num_points].x = end_x;
    xpt[num_points].y = end_y;
    num_points++;

    // Lower right corner
    xpt[num_points].x = end_x;
    xpt[num_points].y = gd.v_win.win_dim.height - gd.v_win.margin.bot-1;
    num_points++;

    // Lower left corner
    xpt[num_points].x = start_x;
    xpt[num_points].y = gd.v_win.win_dim.height - gd.v_win.margin.bot -1;
    num_points++;

    // Fill the terrain core
    XFillPolygon(gd.dpy, xid,gd.layers.earth.color2->gc,xpt, num_points, Nonconvex,CoordModeOrigin);

    // Add 2 pixels to the top for the earth's "skin"
    for(i = 0; i < num_points-2; i++) { xpt[i].y -= 2;  }
    // Add the "skin" color
    XDrawLines(gd.dpy,xid,gd.layers.earth.color1->gc,xpt,num_points-2,CoordModeOrigin);

#endif
    
    return;
}
