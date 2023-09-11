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
 * RENDER_OVERLAYS.C : Rendering routines for the CIDD program
 *
 * F. Hage    Jan 1991    NCAR - EOL
 */

#define RENDER_OVERLAYS

#include "cidd.h"

#define KM_CLIP_BUFFER 25.0
#define LATLON_CLIP_BUFFER 0.4
#define XPOINT_BUF_SIZE  1024

void normalize_longitude(double min_lon, double max_lon,
			 double *normal_lon);

static void draw_label_centered(Drawable xid, GC gc,
				int xx, int yy,
				const char *label);

/************************************************************************
 * CALC_LOCAL_OVER_COORDS: Convert the lat/Lon coords to local X,Y KM coords.
 *
 */

void calc_local_over_coords()
{
    int    i,j,l;
    // int    npoints;
    int    clip_flag;
    double lat,lon;
    double min_lat,max_lat;
    double min_lon,max_lon;
    double min_loc_x,max_loc_x;
    double min_loc_y,max_loc_y;
    Overlay_t    *ov;
    Geo_feat_polyline_t *poly;
    Geo_feat_icon_t    *ic;

    switch(gd.display_projection) {
    default : 

      // condition longitudes to be in the same hemisphere as the origin
      gd.proj.setConditionLon2Origin(true);

      // Compute the bounding box
      max_lon = -360.0;
      min_lon = 360.0;
      max_lat = -180.0;
      min_lat = 180.0;


      // Check each corner of the projection + 2 center points, top, bottom
      // Lower left
      gd.proj.xy2latlon(gd.h_win.min_x - KM_CLIP_BUFFER , gd.h_win.min_y - KM_CLIP_BUFFER,lat,lon);
      if(lon > max_lon) max_lon = lon;
      if(lon < min_lon) min_lon = lon;
      if(lat > max_lat) max_lat = lat;
      if(lat < min_lat) min_lat = lat;

      // Lower midpoint
      gd.proj.xy2latlon((gd.h_win.min_x +gd.h_win.max_x)/2 - KM_CLIP_BUFFER , gd.h_win.min_y - KM_CLIP_BUFFER,lat,lon);
      if(lon > max_lon) max_lon = lon;
      if(lon < min_lon) min_lon = lon;
      if(lat > max_lat) max_lat = lat;
      if(lat < min_lat) min_lat = lat;

      // Lower right
      gd.proj.xy2latlon(gd.h_win.max_x - KM_CLIP_BUFFER , gd.h_win.min_y - KM_CLIP_BUFFER,lat,lon);
      if(lon > max_lon) max_lon = lon;
      if(lon < min_lon) min_lon = lon;
      if(lat > max_lat) max_lat = lat;
      if(lat < min_lat) min_lat = lat;

      // Upper right
      gd.proj.xy2latlon(gd.h_win.max_x + KM_CLIP_BUFFER , gd.h_win.max_y + KM_CLIP_BUFFER,lat,lon);
      if(lon > max_lon) max_lon = lon;
      if(lon < min_lon) min_lon = lon;
      if(lat > max_lat) max_lat = lat;
      if(lat < min_lat) min_lat = lat;

      // Upper midpoint
      gd.proj.xy2latlon((gd.h_win.min_x +gd.h_win.max_x)/2 - KM_CLIP_BUFFER , gd.h_win.max_y - KM_CLIP_BUFFER,lat,lon);
      if(lon > max_lon) max_lon = lon;
      if(lon < min_lon) min_lon = lon;
      if(lat > max_lat) max_lat = lat;
      if(lat < min_lat) min_lat = lat;

      // Upper left
      gd.proj.xy2latlon(gd.h_win.min_x + KM_CLIP_BUFFER , gd.h_win.max_y + KM_CLIP_BUFFER,lat,lon);
      if(lon > max_lon) max_lon = lon;
      if(lon < min_lon) min_lon = lon;
      if(lat > max_lat) max_lat = lat;
      if(lat < min_lat) min_lat = lat;

      //  Handle pathalogical cases where edges extend around the world.
      if(min_lat >= max_lat ||
			  min_lon >= max_lon ||
			  gd.proj.getProjType() == Mdvx::PROJ_POLAR_STEREO ||
			  gd.proj.getProjType() == Mdvx::PROJ_OBLIQUE_STEREO) {
	  min_lon = -360.0;
	  min_lat = -180.0;
	  max_lon = 360.0;
	  max_lat = 180.0;
      }

	  if(gd.debug)  printf("----> Overlay lon,lat Clip box: %g, %g to  %g, %g\n",
			  min_lon,min_lat,max_lon,max_lat);

      for(i=0; i < gd.num_map_overlays; i++) {        /* For Each Overlay */
        ov =  gd.over[i];
	if(gd.debug)  printf("Converting Overlay %s ... ",ov->control_label);
	 
        /* Convert all labels   */
        for(j=0; j < ov->num_labels; j++) { 
	  clip_flag = 0;
	  if(ov->geo_label[j]->min_lat < min_lat ||
	      ov->geo_label[j]->min_lat > max_lat) clip_flag = 1;
	  if(ov->geo_label[j]->min_lon < min_lon ||
	      ov->geo_label[j]->min_lon > max_lon) clip_flag = 1;

	  if(clip_flag) {
	    ov->geo_label[j]->local_x = -32768.0;
	  } else {
	    /* Current rendering only uses min_lat, min_lon to position text */
	    gd.proj.latlon2xy( ov->geo_label[j]->min_lat,ov->geo_label[j]->min_lon,
	          ov->geo_label[j]->local_x,ov->geo_label[j]->local_y);
	  }

        }

        /* Convert all Iconic Objects */
        for(j=0; j < ov->num_icons; j++) {
          ic = ov->geo_icon[j];
	  clip_flag = 0;
	  if(ic->lat < min_lat || ic->lat > max_lat) clip_flag = 1;
	  if(ic->lon < min_lon || ic->lon > max_lon) clip_flag = 1;

	  if(clip_flag) {
	    ic->local_x = -32768.0;
	  } else {
	    gd.proj.latlon2xy( ic->lat,ic->lon, ic->local_x,ic->local_y);
	  }
        }

        /* Convert all Poly Line Objects */
        for(j=0; j < ov->num_polylines; j++) { 
            poly = ov->geo_polyline[j];
	    /* Reset the bounding box limits */
            min_loc_x = DBL_MAX;
	    max_loc_x = DBL_MIN;
            min_loc_y = DBL_MAX;
	    max_loc_y = DBL_MIN;
            // npoints = 0;
            for(l=0; l < poly->num_points; l++) {
		clip_flag = 0;
		if(poly->lat[l] < min_lat || poly->lat[l] > max_lat)
		    clip_flag = 1;
		if(poly->lon[l] < min_lon || poly->lon[l] > max_lon)
		    clip_flag = 1;

                if(poly->lon[l] > -360.0 && clip_flag == 0 ) {
		   gd.proj.latlon2xy( poly->lat[l],poly->lon[l],
		       poly->local_x[l],poly->local_y[l]);

		   /* printf("LAT,LON: %10.6f, %10.6f   LOCAL XY: %10.6f, %10.6f\n",
				    poly->lat[l],poly->lon[l], poly->local_x[l],poly->local_y[l]); */

		    /* Gather the bounding box for this polyline in local coords */
		    if(poly->local_x[l] < min_loc_x) min_loc_x = poly->local_x[l];
		    if(poly->local_y[l] < min_loc_y) min_loc_y = poly->local_y[l];
		    if(poly->local_x[l] > max_loc_x) max_loc_x = poly->local_x[l];
		    if(poly->local_y[l] > max_loc_y) max_loc_y = poly->local_y[l];

                } else {
		    poly->local_x[l] = -32768.0;
		    poly->local_y[l] = -32768.0;
		}
            }
	    /* Set the bounding box */
	    poly->min_x = min_loc_x;
	    poly->min_y = min_loc_y;
	    poly->max_x = max_loc_x;
	    poly->max_y = max_loc_y;
        }
    }

  break;

  case Mdvx::PROJ_LATLON:
    min_lon = gd.h_win.min_x - LATLON_CLIP_BUFFER;
    min_lat = gd.h_win.min_y - LATLON_CLIP_BUFFER;
    max_lon = gd.h_win.max_x + LATLON_CLIP_BUFFER;
    max_lat = gd.h_win.max_y + LATLON_CLIP_BUFFER;

    for(i=0; i < gd.num_map_overlays; i++) {        /* For Each Overlay */
        ov =  gd.over[i];
	if(gd.debug)  printf("Converting Overlay %s ",ov->control_label);
	 
        for(j=0; j < ov->num_labels; j++) {        /* Convert all labels   */
	  clip_flag = 0;

	  normalize_longitude(gd.h_win.min_x, gd.h_win.max_x, &ov->geo_label[j]->min_lon);
	  
	  if(ov->geo_label[j]->min_lat < min_lat ||
	      ov->geo_label[j]->min_lat > max_lat) clip_flag = 1;

	  if(ov->geo_label[j]->min_lon < min_lon ||
	      ov->geo_label[j]->min_lon > max_lon) clip_flag = 1;

	  if(clip_flag) {
	    ov->geo_label[j]->local_x = -32768.0;
	  } else {
	    ov->geo_label[j]->local_x = ov->geo_label[j]->min_lon;
	    ov->geo_label[j]->local_y = ov->geo_label[j]->min_lat;
	  }

        }

        /* Convert all Iconic Objects */
        for(j=0; j < ov->num_icons; j++) {
          ic = ov->geo_icon[j];
	  clip_flag = 0;

	  normalize_longitude(gd.h_win.min_x, gd.h_win.max_x, &ic->lon);
	  
	  if(ic->lat < min_lat || ic->lat > max_lat) clip_flag = 1;
	  if(ic->lon < min_lon || ic->lon > max_lon) clip_flag = 1;

	  if(clip_flag) {
	    ic->local_x = -32768.0;
	  } else {
	    ic->local_x = ic->lon;
	    ic->local_y = ic->lat;
	  }
        }

        /* Convert all Poly Line Objects */
        for(j=0; j < ov->num_polylines; j++) {
            poly = ov->geo_polyline[j];
            // npoints = 0;
	    /* Reset the bounding box limits */
            min_loc_x = DBL_MAX;
	    max_loc_x = DBL_MIN;
            min_loc_y = DBL_MAX;
	    max_loc_y = DBL_MIN;
            for(l=0; l < poly->num_points; l++) {
		clip_flag = 0;

		normalize_longitude(gd.h_win.min_x, gd.h_win.max_x, &poly->lon[l]);
		
		if(poly->lat[l] < min_lat || poly->lat[l] > max_lat) clip_flag = 1;
		if(poly->lon[l] < min_lon || poly->lon[l] > max_lon) clip_flag = 1;

		if (l > 0 && fabs(poly->lon[l] - poly->lon[l-1]) > 330) {
		  clip_flag = 1;
		}

                if(poly->lon[l] > -360.0 && clip_flag == 0 ) {

		   poly->local_x[l] = poly->lon[l];
		   poly->local_y[l] = poly->lat[l];

		    /* Gather the bounding box for this polyline in local coords */
		    if(poly->local_x[l] < min_loc_x) min_loc_x = poly->local_x[l];
		    if(poly->local_y[l] < min_loc_y) min_loc_y = poly->local_y[l];
		    if(poly->local_x[l] > max_loc_x) max_loc_x = poly->local_x[l];
		    if(poly->local_y[l] > max_loc_y) max_loc_y = poly->local_y[l];

                } else {
		    poly->local_x[l] = -32768.0;
		    poly->local_y[l] = -32768.0;
		}
            }
	    /* Set the bounding box */
	    poly->min_x = min_loc_x;
	    poly->min_y = min_loc_y;
	    poly->max_x = max_loc_x;
	    poly->max_y = max_loc_y;
        }
    }
    break;
  }
  if(gd.debug)  printf("Done\n");
}

/************************************************************************
 * NORMALIZE_LONGITUDE: Normalize the given longitude value to fall within
 *                      min_lon and min_lon + 360.0.
 *
 */

void normalize_longitude(double min_lon, double max_lon, double *normal_lon)
{

  if ((*normal_lon < min_lon || *normal_lon > max_lon) &&
      (min_lon >= -540.0 && max_lon < 540.0)) {
    
    while (*normal_lon < min_lon) {
      if (max_lon < *normal_lon + 360.0)
	break;
      *normal_lon += 360.0;
    }
    
    while (*normal_lon >= min_lon + 360.0) {
      *normal_lon -= 360.0;
    }

  }

}


/************************************************************************
 * RENDER_OVERLAYS: Render active overlays in the indicated field pixmap
 *
 */

void render_map_overlays(Drawable  xid)
{

    int    i,j,l;
    unsigned int k;
    int    x1,y1;
    unsigned int    npoints;
    double    scale;
    Overlay_t    *ov;
    Geo_feat_polyline_t *poly;
    Geo_feat_icon_t    *ic;
    static size_t buf_size = 0;
    static XPoint    *bpt;
    static double    *x;
    static double    *y;

    if(buf_size == 0) {
	if((bpt = (XPoint *) calloc(XPOINT_BUF_SIZE,sizeof(XPoint))) == NULL) {
	    perror("Malloc Error in render_map_overlays");
	    exit(-1);
	}
	if((x = (double *) calloc(XPOINT_BUF_SIZE,sizeof(double))) == NULL) {
	    perror("Malloc Error in render_map_overlays");
	    exit(-1);
	}
	if((y = (double *) calloc(XPOINT_BUF_SIZE,sizeof(double))) == NULL) {
	    perror("Malloc Error in render_map_overlays");
	    exit(-1);
	}
	buf_size = XPOINT_BUF_SIZE;
    }

    // Trial Concept for TAIWAN DEMO - Scale the Icons
    scale = 1.0;
    for(i=NUM_PRODUCT_DETAIL_THRESHOLDS -1; i >=0; i--)  {
      if( gd.h_win.km_across_screen <= gd.prod.detail[i].threshold) {
         scale *= 1.5; // increase by 50%
      }
    }

    for(i=gd.num_map_overlays -1 ; i >= 0; i--) {        /* For Each Overlay */

        if(gd.over[i]->active && (gd.over[i]->detail_thresh_min <= gd.h_win.km_across_screen) && 
           (gd.over[i]->detail_thresh_max >= gd.h_win.km_across_screen))  {

           ov =  gd.over[i];
		XSetLineAttributes(gd.dpy,ov->color->gc, ov->line_width,LineSolid,CapButt,JoinBevel);
	       XSetFont(gd.dpy,ov->color->gc,gd.ciddfont[gd.prod.prod_font_num]);
            for(j=0; j < ov->num_labels; j++) {        /* Draw all labels - Not Fully Implemented Yet */
		if(ov->geo_label[j]->local_x <= -32768.0) continue;
                disp_proj_to_pixel(&(gd.h_win.margin),ov->geo_label[j]->local_x,ov->geo_label[j]->local_y,&x1,&y1);
		draw_label_centered(xid, ov->color->gc,
				    x1, y1,
				    ov->geo_label[j]->string);
            }

            for(j=0; j < ov->num_icons; j++) {        /* Draw all Iconic Objects */
                ic = ov->geo_icon[j];
		if(ic->local_x <= -32768.0) continue;
                disp_proj_to_pixel(&(gd.h_win.margin),ic->local_x,ic->local_y,&x1,&y1);

                npoints = 0;
                for(l=0; l < ic->icon->num_points; l++) {    /* draw the Icon */
                    if(ic->icon->x[l] != 32767 ) {
                        bpt[npoints].x = (short) (ic->icon->x[l] * scale) + (short) x1;
                        bpt[npoints].y = (short) (ic->icon->y[l] * scale) + (short) y1;
                        if(npoints >= buf_size -1 ) {
			    if((bpt = (XPoint *) realloc(bpt,buf_size *2 * sizeof(XPoint))) == NULL) { 
				 perror("Realloc Error in render_map_overlays");
				 exit(-1);
			    }
			    if((x = (double *) realloc(x,buf_size *2 * sizeof(double))) == NULL) { 
				 perror("Realloc Error in render_map_overlays");
				 exit(-1);
			    }
			    if((y = (double *) realloc(y,buf_size *2 * sizeof(double))) == NULL) { 
				 perror("Realloc Error in render_map_overlays");
				 exit(-1);
			    }
			    buf_size *= 2;
                        }

                        npoints++;
                    } else {
                        XDrawLines(gd.dpy,xid,ov->color->gc,bpt,npoints,CoordModeOrigin);
                        npoints = 0;
                    }
                }

		if(gd.font_display_mode == 0)
                   XDrawString(gd.dpy,xid,ov->color->gc,
                        (int)(ic->text_x + x1),
                        (int)(ic->text_y + y1),
                        ic->label,strlen(ic->label));
		else
                   XDrawImageString(gd.dpy,xid,ov->color->gc,
                        (int)(ic->text_x + x1),
                        (int)(ic->text_y + y1),
                        ic->label,strlen(ic->label));
            }

            for(j=0; j < ov->num_polylines; j++) {        /* Draw all Poly Line Objects */
                poly = ov->geo_polyline[j];
                npoints = 0;
                for(l=0; l < poly->num_points; l++) {

		    // If the line is off screen, don't use it.
                    if(poly->local_x[l] <= -16383.0 || poly->local_x[l] >= 16383.0 ||
		       poly->local_y[l] <= -16383.0 || poly->local_y[l] >= 16383.0) {
                        for(k=0; k < npoints; k++) {
                            disp_proj_to_pixel(&(gd.h_win.margin),x[k],y[k],&x1,&y1);

			    // If the line is way too long 
			    if(x1 < -32767 || x1 > 32767 || y1  < -32767 || y1 > 32767) {
			       npoints = k;  // abort on this line
			    } else {
                               bpt[k].x = x1;
                               bpt[k].y = y1;
			    }

                        }
                        if(npoints > 0) XDrawLines(gd.dpy,xid,ov->color->gc,bpt,npoints,CoordModeOrigin);
                        npoints = 0;
                    } else {
                        x[npoints] = poly->local_x[l];
                        y[npoints] = poly->local_y[l];
                        if(npoints >= buf_size -1 ) {
			    if((bpt = (XPoint *) realloc(bpt,buf_size *2 * sizeof(XPoint))) == NULL) { 
				 perror("Realloc Error in render_map_overlays");
				 exit(-1);
			    }
			    if((x = (double *) realloc(x,buf_size *2 * sizeof(double))) == NULL) { 
				 perror("Realloc Error in render_map_overlays");
				 exit(-1);
			    }
			    if((y = (double *) realloc(y,buf_size *2 * sizeof(double))) == NULL) { 
				 perror("Realloc Error in render_map_overlays");
				 exit(-1);
			    }
			    buf_size *= 2;
                        }

                        npoints++;
                    }
                }

		/* Handle Poly lines without pen-up's */ 
		if(npoints > 0) {
		    for(k=0; k < npoints; k++) {
                        disp_proj_to_pixel(&(gd.h_win.margin),x[k],y[k],&x1,&y1);
			// If the line is way too long 
			if(x1 < -32767 || x1 > 32767 || y1  < -32767 || y1 > 32767) {
			   npoints = k;  // abort on this line
			} else {
                           bpt[k].x = x1;
                           bpt[k].y = y1;
			}

                    }

                    XDrawLines(gd.dpy,xid,ov->color->gc,bpt,npoints,CoordModeOrigin);
                    npoints = 0;
                }
            }

        }
    }
}

//////////////////////////////////
// draw label, in different modes


static void draw_label_centered(Drawable xid, GC gc,
				int xx, int yy,
				const char *label)
  
{

  int direct, ascent, descent;
  XCharStruct overall;

  XFontStruct *fontst = gd.fontst[gd.prod.prod_font_num];
  XTextExtents(fontst, label, strlen(label),
	       &direct, &ascent, &descent, &overall);
  
  int xoffset =  -(overall.width) / 2;
  int yoffset =  - overall.descent;
  
  if(gd.font_display_mode == 0) {
    XDrawString(gd.dpy, xid, gc,
		xx + xoffset, yy + yoffset,
		label, strlen(label));
  } else {
    XDrawImageString(gd.dpy, xid, gc,
		     xx + xoffset, yy + yoffset,
		     label, strlen(label));
  }

}
