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
 * RENDER_XSECT_GRIDS.C: 
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_XSECT_GRIDS

#include "cidd.h"
#include <vector>
#define MESSAGE_LEN 1024

static int render_polar_rhi( Drawable xid, met_record_t *mr, time_t start_time, time_t end_time, int is_overlay_field);

/**********************************************************************
 * RENDER_XSECT_GRIDS
 */

int render_xsect_grid( Drawable xid, met_record_t *mr, time_t start_time, time_t end_time, int is_overlay_field)
{

    int    i,j;
    int    ht,wd;                /* Dims of drawing canvas */
    int    out_of_date;
    int    stretch_secs;
    int    startx,starty;        /* Pixel limits of data area */
    int    endx,endy;            /* Pixel limits of data area */
    int    xmid,ymid;
    double    dist = 0;          /* distance between corner of vert section and grid point */
    double    height = 0;
    double    val;
    double    r_wd;        /*  data point rectangle dims */
    unsigned short    *ptr;
    char    message[MESSAGE_LEN];
    Font    font;

    if(xid == 0) return CIDD_FAILURE;
     
    ptr =  (unsigned short *) mr->v_data;

    stretch_secs =  (int) (60.0 * mr->time_allowance);
    out_of_date = 0;

    if(_params.check_data_times) {                         
        if(mr->v_date.unix_time < start_time - stretch_secs) out_of_date = 1;
        if(mr->v_date.unix_time > end_time + stretch_secs) out_of_date = 1;  
    } 
     
    if(ptr == NULL || out_of_date) {    /* If no data - Draw warning message */
        if(out_of_date) {
          snprintf(message,MESSAGE_LEN,"%s - Data too Old",_params.no_data_message);
        } else {
          STRcopy(message, _params.no_data_message,MESSAGE_LEN);
        }
        font = choose_font(message,gd.v_win.img_dim.width,gd.v_win.img_dim.height,&xmid,&ymid);
        XSetFont(gd.dpy,gd.legends.foreground_color->gc,font);
        XDrawImageString(gd.dpy,xid,gd.legends.foreground_color->gc,
            ((gd.v_win.win_dim.width /  2) + xmid),
            ((gd.v_win.win_dim.height / 2) + ymid),
            message,strlen(message));

        fprintf(stderr, "Message with no data received from\n");
        fprintf(stderr, "  default service at: %s\n",
                gd.io_info.mr->url);
        fflush(stderr);
        
        return CIDD_SUCCESS;
    }

    // is this a polar RHI? If so, special case.
    
    if (mr->v_fhdr.vlevel_type == Mdvx::VERT_TYPE_AZ) {

      return render_polar_rhi(xid, mr, start_time, end_time, is_overlay_field);

    }
        
    height = mr->vert[0].min;
    disp_proj_to_pixel_v(&gd.v_win.margin,0.0,height,&startx,&starty);

    height = mr->vert[mr->v_fhdr.nz -1].max;
    disp_proj_to_pixel_v(&gd.v_win.margin,gd.h_win.route.total_length,height,&endx,&endy);
    r_wd =  (double)ABSDIFF(endx,startx) / (double) mr->v_fhdr.nx;        /* get data point rectangle width */

    vector<vert_spacing_t> vert;
    vert.resize(mr->v_fhdr.nz);
    //    vert_spacing_t *vert = vert_.alloc(mr->v_fhdr.nz);

    for(i=0; i < mr->v_fhdr.nz; i++) { 
      if(i == 0) { // Lowest plane 
	double delta = fabs(mr->v_vhdr.level[i+1] - mr->v_vhdr.level[i]) / 2.0;
	vert[i].min = mr->v_vhdr.level[0] - delta;
	vert[i].cent = mr->v_vhdr.level[0];
	vert[i].max = mr->v_vhdr.level[0] + delta;
      } else if (i == mr->v_fhdr.nz -1) { // highest plane
	double delta = fabs(mr->v_vhdr.level[i] - mr->v_vhdr.level[i-1]) / 2.0;
	vert[i].min = mr->v_vhdr.level[i] - delta;
	vert[i].cent = mr->v_vhdr.level[i];
	vert[i].max = mr->v_vhdr.level[i] + delta;
      } else { // Middle planes
	vert[i].min = (mr->v_vhdr.level[i] + mr->v_vhdr.level[i-1]) / 2.0;
	vert[i].cent = mr->v_vhdr.level[i];
	vert[i].max = (mr->v_vhdr.level[i] + mr->v_vhdr.level[i+1]) /2.0;
      }
    }

    vector<int> y_start, y_end;
    y_start.resize(mr->v_fhdr.nz); /* canvas  rectangle begin coords */
    y_end.resize(mr->v_fhdr.nz); /* canvas  rectangle end coords */
    for(i= 0; i < mr->v_fhdr.nz; i++) {    /* Calc starting/ending coords for the array */
          height = vert[i].min;
          disp_proj_to_pixel_v(&gd.v_win.margin,dist,height,&endx,&(y_start[i]));
          height = vert[i].max;
          disp_proj_to_pixel_v(&gd.v_win.margin,dist,height,&endx,&(y_end[i]));
		  if ((y_start[i] - y_end[i]) < 0 ) {
			  int tval = y_start[i];
			  y_start[i] = y_end[i];
			  y_end[i] = tval;
		  }
    }

    vector<int> x_start;
    x_start.resize(mr->v_fhdr.nx);    /* canvas rectangle begin  coords */
    for(j=0;j< mr->v_fhdr.nx; j++) {
        x_start[j] = (int)(((double) j * r_wd) + startx);
    }

 
    wd = (int)(r_wd + 1.0);
     
    if(is_overlay_field) {
      for(i= 0; i < mr->v_fhdr.nz; i++) {
        ht = y_start[i] - y_end[i];
        for(j=0;j< mr->v_fhdr.nx; j++) {
            if(mr->v_vcm.val_gc[*ptr] != NULL) {
		val =  (mr->v_fhdr.scale * *ptr) + mr->v_fhdr.bias;
		if(val >= mr->overlay_min && val <= mr->overlay_max) 
                XFillRectangle(gd.dpy,xid,mr->v_vcm.val_gc[*ptr],x_start[j],y_end[i],wd,ht);
            }
            ptr++;
        }
      }
    } else {

    int rend_m = mr->render_method;

   // Handle case where contours really don't apply (vertical sides)
   if(mr->v_fhdr.min_value == mr->v_fhdr.max_value ) rend_m = POLYGONS;
      switch (rend_m) { 
	case  POLYGONS: 
          //printf("Drawing Filled Rectangle image: field %s\n", mr->field_label);
          for(i= 0; i < mr->v_fhdr.nz; i++) {
            ht = (y_start[i] - y_end[i]);
            for(j=0;j< mr->v_fhdr.nx; j++) {
                if(mr->v_vcm.val_gc[*ptr] != NULL) {
                    XFillRectangle(gd.dpy,xid,mr->v_vcm.val_gc[*ptr],x_start[j],y_end[i],wd,ht);
                }
                ptr++;
            }
          }
	break;

	case FILLED_CONTOURS:
	case DYNAMIC_CONTOURS:
            for(j=0;j <= mr->v_fhdr.nx; j++) {
                  x_start[j] =  (int) (x_start[j] + (r_wd / 2));
                  if(x_start[j] > gd.v_win.can_dim.width) x_start[j] = gd.v_win.can_dim.width;
             }

              for(i= 0; i <= mr->v_fhdr.nz; i++) {    /* Calc starting coords for the array */
                  y_start[i] =  (int) ((y_start[i] + y_end[i]) / 2.0);
                  if(y_start[i] < 0) y_start[i] = 0;
                  if(y_start[i] >= gd.v_win.can_dim.height) y_start[i] = gd.v_win.can_dim.height -1;
              }
              // printf("Drawing Filled Contour image: field %s\n", mr->field_label);
	      if (gd.layers.use_alt_contours) {
		RenderFilledPolygons(xid, mr, true);
	      } else {
		draw_xsect_filled_contours(xid,x_start.data(),y_start.data(),mr);
	      }
	break;
      }
    }
    
    return CIDD_SUCCESS;
}

//////////////////////////
// rendering for POLAR RHI

static int render_polar_rhi( Drawable xid, met_record_t *mr, time_t start_time, time_t end_time, int is_overlay_field)

{

  vector<double> elevLimit, slant, htcorr;
  elevLimit.resize(mr->v_fhdr.ny + 1);
  slant.resize(mr->v_fhdr.nx + 1);
  htcorr.resize(mr->v_fhdr.nx + 1);
  double **ht = (double **) umalloc2(mr->v_fhdr.ny + 1, mr->v_fhdr.nx + 1, sizeof(double));
  double **range = (double **) umalloc2(mr->v_fhdr.ny + 1, mr->v_fhdr.nx + 1, sizeof(double));
  int **yy = (int **) umalloc2(mr->v_fhdr.ny + 1, mr->v_fhdr.nx + 1, sizeof(int));
  int **xx = (int **) umalloc2(mr->v_fhdr.ny + 1, mr->v_fhdr.nx + 1, sizeof(int));

    // compute array of heights and ranges

    double twiceRad = PSEUDO_RADIUS * 2.0;
    double radarHt = mr->v_mhdr.sensor_alt;

    elevLimit[0] = mr->v_fhdr.grid_miny - 0.5 * mr->v_fhdr.grid_dy;
    for (int ii = 0; ii < mr->v_fhdr.ny; ii++) {
      elevLimit[ii + 1] = elevLimit[ii] + mr->v_fhdr.grid_dy;
    }
    
    slant[0] = mr->v_fhdr.grid_minx - 0.5 * mr->v_fhdr.grid_dx;
    for (int jj = 0; jj < mr->v_fhdr.nx; jj++) {
      slant[jj + 1] = slant[jj] + mr->v_fhdr.grid_dx;
    }
    // double maxRange = slant[mr->v_fhdr.nx];
    for (int jj = 0; jj <= mr->v_fhdr.nx; jj++) {
      htcorr[jj] = (slant[jj] * slant[jj]) / twiceRad;
    }
    
    for (int ii = 0; ii <= mr->v_fhdr.ny; ii++) {
      double sinel = sin(elevLimit[ii] * DEG_TO_RAD);
      double cosel = cos(elevLimit[ii] * DEG_TO_RAD);
      for (int jj = 0; jj <= mr->v_fhdr.nx; jj++) {
	range[ii][jj] = slant[jj] * cosel;
	ht[ii][jj] = slant[jj] * sinel + htcorr[jj] + radarHt;
      }
    }

    // convert to screen coords
    
    for (int ii = 0; ii <= mr->v_fhdr.ny; ii++) {
      for (int jj = 0; jj <= mr->v_fhdr.nx; jj++) {
	disp_proj_to_pixel_v(&gd.v_win.margin,
			     range[ii][jj], ht[ii][jj],
			     &xx[ii][jj], &yy[ii][jj]);
      }
    }
    
    // render polygons

    int num_points = 4;
    vector<XPoint> xpt;
    xpt.resize(num_points);
    
    unsigned short *ptr =  (unsigned short *) mr->v_data;
    for (int ii = 0; ii < mr->v_fhdr.ny; ii++) {
      for (int jj = 0; jj < mr->v_fhdr.nx; jj++, ptr++) {
	if(mr->v_vcm.val_gc[*ptr] != NULL) {
	  xpt[0].x = xx[ii][jj];
	  xpt[0].y = yy[ii][jj];
	  xpt[1].x = xx[ii+1][jj];
	  xpt[1].y = yy[ii+1][jj];
	  xpt[2].x = xx[ii+1][jj+1];
	  xpt[2].y = yy[ii+1][jj+1];
	  xpt[3].x = xx[ii][jj+1];
	  xpt[3].y = yy[ii][jj+1];
	  XFillPolygon(gd.dpy, xid, mr->v_vcm.val_gc[*ptr],
		       xpt.data(), num_points, Nonconvex, CoordModeOrigin);
	}
      } // jj
    } // ii

    // free up

    ufree2((void **) ht);
    ufree2((void **) range);
    ufree2((void **) xx);
    ufree2((void **) yy);
    
    return CIDD_SUCCESS;
}

