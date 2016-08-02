/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/********************************************************************
 * CIDD_RENDER_OVERLAYS.C : Rendering routines for the CIDD program
 *
 * F. Hage    Jan 1991    NCAR - RAP
 */

#define CIDD_RENDER_OVERLAYS

#include "cidd.h"
#define CLIP_BUFFER 25.0
#define LATLON_CLIP_BUFFER 0.4
/************************************************************************
 * CALC_LOCAL_OVER_COORDS: Convert the lat/Lon coords to local X,Y KM coords.
 *
 */

void calc_local_over_coords(void)
{
    int    i,j,k,l;
    int    npoints;
    int    clip_flag;
    double min_lat,max_lat;
    double min_lon,max_lon;
    double min_loc_x,max_loc_x;
    double min_loc_y,max_loc_y;
    Overlay_t    *ov;
    Geo_feat_polyline_t *poly;
    Geo_feat_icon_t    *ic;
    Geo_feat_label_t    *label;

  switch(gd.projection_mode) {
  case CARTESIAN_PROJ:
    PJGflat_init(gd.h_win.origin_lat,gd.h_win.origin_lon,gd.north_angle);
    PJGflat_xy2latlon(gd.h_win.min_x - CLIP_BUFFER ,
	gd.h_win.min_y - CLIP_BUFFER,&min_lat,&min_lon);
    PJGflat_xy2latlon(gd.h_win.max_x + CLIP_BUFFER ,
	gd.h_win.max_y + CLIP_BUFFER,&max_lat,&max_lon);

    for(i=0; i < gd.num_map_overlays; i++) {        /* For Each Overlay */
        ov =  gd.over[i];
	if(gd.debug)  printf("Converting Overlay %s ",ov->control_label);
	 
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
	    PJGflat_latlon2xy( ov->geo_label[j]->min_lat,ov->geo_label[j]->min_lon,
	       &ov->geo_label[j]->local_x,&ov->geo_label[j]->local_y);
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
	    PJGflat_latlon2xy( ic->lat,ic->lon, &ic->local_x,&ic->local_y);
	  }
        }

        /* Convert all Poly Line Objects */
        for(j=0; j < ov->num_polylines; j++) { 
            poly = ov->geo_polyline[j];
	    /* Reset the bounding box limits */
            min_loc_x = MAXDOUBLE;
	    max_loc_x = MINDOUBLE;
            min_loc_y = MAXDOUBLE;
	    max_loc_y = MINDOUBLE;
            npoints = 0;
            for(l=0; l < poly->num_points; l++) {
		clip_flag = 0;
		if(poly->lat[l] < min_lat || poly->lat[l] > max_lat)
		    clip_flag = 1;
		if(poly->lon[l] < min_lon || poly->lon[l] > max_lon)
		    clip_flag = 1;

                if(poly->lon[l] > -360.0 && clip_flag == 0 ) {
		   PJGflat_latlon2xy( poly->lat[l],poly->lon[l],
		       &poly->local_x[l],&poly->local_y[l]);

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

  case LAT_LON_PROJ:
    min_lon = gd.h_win.min_x - LATLON_CLIP_BUFFER;
    min_lat = gd.h_win.min_y - LATLON_CLIP_BUFFER;
    max_lon = gd.h_win.max_x + LATLON_CLIP_BUFFER;
    max_lat = gd.h_win.max_y + LATLON_CLIP_BUFFER;

    for(i=0; i < gd.num_map_overlays; i++) {        /* For Each Overlay */
        ov =  gd.over[i];
	if(gd.debug)  printf("Converting Overlay %s ",ov->control_label);
	 
        for(j=0; j < ov->num_labels; j++) {        /* Convert all labels   */
	  clip_flag = 0;
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
            npoints = 0;
	    /* Reset the bounding box limits */
            min_loc_x = MAXDOUBLE;
	    max_loc_x = MINDOUBLE;
            min_loc_y = MAXDOUBLE;
	    max_loc_y = MINDOUBLE;
            for(l=0; l < poly->num_points; l++) {
		clip_flag = 0;
		if(poly->lat[l] < min_lat || poly->lat[l] > max_lat) clip_flag = 1;
		if(poly->lon[l] < min_lon || poly->lon[l] > max_lon) clip_flag = 1;

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
  } /* End switch projection_type */
  if(gd.debug)  printf("Done\n");
}

/************************************************************************
 * RENDER_OVERLAYS: Render active overlays in the indicated field pixmap
 *
 */

void render_overlays(Drawable    xid)
{
    int    i,j,k,l;
    int    x1,y1;
    int    npoints;
    double detail_val;
    double    x[2048];
    double    y[2048];
    Overlay_t    *ov;
    Geo_feat_polyline_t *poly;
    Geo_feat_icon_t    *ic;
    Geo_feat_label_t    *label;
    XPoint    bpt[2048];

    /* compute this images pixels per km */
    detail_val = gd.h_win.img_dim.width / (gd.h_win.cmax_x - gd.h_win.cmin_x);

    for(i=0; i < gd.num_map_overlays; i++) {        /* For Each Overlay */
        if(gd.over[i]->active && gd.over[i]->detail_thresh <= detail_val) {
           ov =  gd.over[i];
	       XSetFont(gd.dpy,ov->color->gc,gd.ciddfont[gd.prod.prod_font_num]);
            for(j=0; j < ov->num_labels; j++) {        /* Draw all labels - Not Fully Implemented Yet */
		if(ov->geo_label[j]->local_x <= -32768.0) continue;
                km_to_pixel(&(gd.h_win.margin),ov->geo_label[j]->local_x,ov->geo_label[j]->local_y,&x1,&y1);

		if(gd.font_display_mode == 0)
                   XDrawString(gd.dpy,xid,ov->color->gc,x1,y1,ov->geo_label[j]->string,strlen(ov->geo_label[j]->string));
		else
                   XDrawImageString(gd.dpy,xid,ov->color->gc,x1,y1,ov->geo_label[j]->string,strlen(ov->geo_label[j]->string));
            }

            for(j=0; j < ov->num_icons; j++) {        /* Draw all Iconic Objects */
                ic = ov->geo_icon[j];
		if(ic->local_x <= -32768.0) continue;
                km_to_pixel(&(gd.h_win.margin),ic->local_x,ic->local_y,&x1,&y1);

                npoints = 0;
                for(l=0; l < ic->icon->num_points; l++) {    /* draw the Icon */
                    if(ic->icon->x[l] != 32767 ) {
                        bpt[npoints].x = ic->icon->x[l] + (short) x1;
                        bpt[npoints].y = ic->icon->y[l] + (short) y1;
                        if(npoints < 2047) {
                            npoints++;
                        } else {
                            fprintf(stderr,"Warning!: Single connected line too big! - Resize temp buffer!\n");
                        }
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
                    if(poly->local_x[l] > -32768.0 ) {
                        x[npoints] = poly->local_x[l];
                        y[npoints] = poly->local_y[l];
                        if(npoints < 2047) {
                            npoints++;
                        } else {
                            fprintf(stderr,"Warning!: Single connected line too big! - Resize temp buffer!\n");
                        }
                    } else {
                        for(k=0; k < npoints; k++) {
                            km_to_pixel(&(gd.h_win.margin),x[k],y[k],&x1,&y1);
                            bpt[k].x = x1;
                            bpt[k].y = y1;
                        }
                        if(npoints > 0) XDrawLines(gd.dpy,xid,ov->color->gc,bpt,npoints,CoordModeOrigin);
                        npoints = 0;
                    }
                }

		/* Handle Poly lines without pen-up's */ 
		if(npoints > 0) {
		    for(k=0; k < npoints; k++) {
                        km_to_pixel(&(gd.h_win.margin),x[k],y[k],&x1,&y1);
                        bpt[k].x = x1;
                        bpt[k].y = y1;
                    }
                    XDrawLines(gd.dpy,xid,ov->color->gc,bpt,npoints,CoordModeOrigin);
                    npoints = 0;
                }
            }

        }
    }
}
