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
/**********************************************************************
 * RENDER_LINE_CONTOURS.CC
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_LINE_CONTOURS

#include "cidd.h"

void render_line_contours(QPaintDevice *pdev, contour_info_t *crec)
{

#ifdef NOTYET
  
    int    i,j,len;
    int    startx,starty,endx,endy;
    int    x_grid[MAX_COLS],y_grid[MAX_ROWS];
    int    cont_features;
    int    using_temp = 0;

    double delt;
    double levels[MAX_CONT_LEVELS];
    GC    gc_array[MAX_CONT_LEVELS];
    GC    special_value_gc = gd.def_gc;

    double    x_dproj,y_dproj;

    float    val;
    float    bad_data;
    float    miss_data;
    float   *ptr;
    float   *ptr2;
    float *tmp_data = NULL;
    MetRecord *mr;       /* convienence pointer to a data record */

    mr = gd.mrec[crec->field];

    // Add the list of times to the time plot
    if(mr->time_list.num_entries > 0) {
        gd.time_plot->add_grid_tlist(mr->button_name,mr->time_list.tim,
    mr->time_list.num_entries,mr->h_date.unix_time);
    }  


    MdvxField *f = mr->h_mdvx->getField(0);
    if(f == NULL) return;

    const Mdvx::field_header_t &fhdr = f->getFieldHeader();

    cont_features = 0;
    if(crec->labels_on)  cont_features |= RASCON_DOLABELS;  
    if(gd.layers.smooth_contours == 1 )  cont_features |= RASCON_DOSMOOTH;  
    if(gd.layers.smooth_contours == 2 )  cont_features |= RASCON_DOSMOOTH2;  
    if(gd.layers.add_noise)  cont_features |= RASCON_ADDNOISE;
    if (fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
      cont_features |= RASCON_DATA_IS_LOG;
    }

    ptr = (float *)  f->getVol();
    if(ptr == NULL) return;
    
    if((crec->max < crec->min) && (crec->interval != 0.0)) {
         fprintf(stderr,"CIDD: ERROR Degenerate Contour Specification:\n");
         fprintf(stderr," Data: %s  MIN: %f Max: %f, Interval: %f :\n",
             mr->button_name,
             crec->min,crec->max,crec->interval);
         if(crec->interval < 0.0) {
           fprintf(stderr,"Contour interval must be > 0.0\n");
           fprintf(stderr,"Contour interval of 0.0 - Uses Colorscale values\n");
         }
         fprintf(stderr,"Skipping Contour rendering for this data\n");
         return;

    }

    // Establish Contour - Color mapping
    if(crec->interval == 0.0) {  // Use the colorscale info
        Valcolormap_t *vcm =  &(mr->h_vcm);

        bool increasing = true;
        if (vcm->nentries > 1 && 
           (vcm->vc[vcm->nentries - 1]->min < vcm->vc[0]->min))  increasing = false;

        if (increasing) {
          crec->num_levels = 0;
          for(i=0; i < vcm->nentries; i++) {
          levels[crec->num_levels] = vcm->vc[i]->min;
          if(fabs(levels[crec->num_levels] - gd.layers.special_contour_value) < 10e-6) {
              XCopyGC(gd.dpy,vcm->vc[i]->gc,0xFFFFFFFF,special_value_gc);
              gc_array[crec->num_levels] = special_value_gc;
                  XSetLineAttributes(gd.dpy, gc_array[crec->num_levels],
                     gd.layers.contour_line_width * 2 , LineSolid, CapButt, JoinMiter);
          } else {
              gc_array[crec->num_levels] = vcm->vc[i]->gc;
                  XSetLineAttributes(gd.dpy, gc_array[crec->num_levels],
                     gd.layers.contour_line_width , LineSolid, CapButt, JoinMiter);
          }

                  XSetFont(gd.dpy,gc_array[crec->num_levels],gd.ciddfont[gd.prod.prod_font_num]);
          crec->num_levels++;
          }
          levels[crec->num_levels] = vcm->vc[vcm->nentries -1]->max;
          if(fabs(levels[crec->num_levels] - gd.layers.special_contour_value) < 10e-6) {
              XCopyGC(gd.dpy,vcm->vc[vcm->nentries -1]->gc,0xFFFFFFFF,special_value_gc);
              gc_array[crec->num_levels] = special_value_gc;
                  XSetLineAttributes(gd.dpy, gc_array[crec->num_levels],
                     gd.layers.contour_line_width * 2 , LineSolid, CapButt, JoinMiter);
          } else {
              gc_array[crec->num_levels] = vcm->vc[vcm->nentries -1]->gc;
                  XSetLineAttributes(gd.dpy, gc_array[crec->num_levels],
                     gd.layers.contour_line_width , LineSolid, CapButt, JoinMiter);
          }

              XSetFont(gd.dpy,gc_array[crec->num_levels],gd.ciddfont[gd.prod.prod_font_num]);
          crec->num_levels++;

        } else {  
          crec->num_levels = 0;
          for(i=vcm->nentries -1 ; i >= 0; i--) {
          levels[crec->num_levels] = vcm->vc[i]->min;
          if(fabs(levels[crec->num_levels] - gd.layers.special_contour_value) < 10e-6) {
              XCopyGC(gd.dpy,vcm->vc[i]->gc,0xFFFFFFFF,special_value_gc);
              gc_array[crec->num_levels] = special_value_gc;
                  XSetLineAttributes(gd.dpy, gc_array[crec->num_levels],
                     gd.layers.contour_line_width * 2 , LineSolid, CapButt, JoinMiter);
          } else {
              gc_array[crec->num_levels] = vcm->vc[i]->gc;
                  XSetLineAttributes(gd.dpy, gc_array[crec->num_levels],
                     gd.layers.contour_line_width , LineSolid, CapButt, JoinMiter);
          }

                  XSetFont(gd.dpy,gc_array[crec->num_levels],gd.ciddfont[gd.prod.prod_font_num]);
          crec->num_levels++;
          }
          levels[crec->num_levels] = vcm->vc[0]->max;
          if(fabs(levels[crec->num_levels] - gd.layers.special_contour_value) < 10e-6) {
              XCopyGC(gd.dpy,vcm->vc[0]->gc,0xFFFFFFFF,special_value_gc);
              gc_array[crec->num_levels] = special_value_gc;
                  XSetLineAttributes(gd.dpy, gc_array[crec->num_levels],
                     gd.layers.contour_line_width * 2 , LineSolid, CapButt, JoinMiter);
          } else {
              gc_array[crec->num_levels] = vcm->vc[0]->gc;
                  XSetLineAttributes(gd.dpy, gc_array[crec->num_levels],
                     gd.layers.contour_line_width , LineSolid, CapButt, JoinMiter);
          }
              XSetFont(gd.dpy,gc_array[crec->num_levels],gd.ciddfont[gd.prod.prod_font_num]);

          crec->num_levels++;
        }

    } else {
      crec->num_levels = 0;
          val = crec->min;
          for(i=0; (crec->num_levels < MAX_CONT_LEVELS ) && (val <= crec->max);i++) {
            val = crec->min + ( i * crec->interval);
            levels[crec->num_levels] = val;
        if(fabs(levels[crec->num_levels] - gd.layers.special_contour_value) < 10e-6) {
              XCopyGC(gd.dpy,crec->color->gc,0xFFFFFFFF,special_value_gc);
              gc_array[crec->num_levels] = special_value_gc;
                  XSetLineAttributes(gd.dpy, gc_array[crec->num_levels],
                     gd.layers.contour_line_width * 2 , LineSolid, CapButt, JoinMiter);
        } else {
                  gc_array[crec->num_levels] = crec->color->gc;
                  XSetLineAttributes(gd.dpy, gc_array[crec->num_levels],
                     gd.layers.contour_line_width , LineSolid, CapButt, JoinMiter);
        }
            crec->num_levels++;
      }
          XSetFont(gd.dpy,crec->color->gc,gd.ciddfont[gd.prod.prod_font_num]);
      XSetLineAttributes(gd.dpy, crec->color->gc,
         gd.layers.contour_line_width , LineSolid, CapButt, JoinMiter);

        }

       // Deal With Data Options
       bad_data =  fhdr.bad_data_value;
       miss_data = fhdr.missing_data_value;

       if( bad_data !=  miss_data) { // Map bad to missing
           using_temp = 1;
           if((tmp_data = (float *) calloc(fhdr.nx*fhdr.ny,sizeof(float))) == NULL) {
                perror("CIDD: Alloc failure");
                exit(-1);
           }
           ptr = (float *)  (mr->h_mdvx->getFieldByNum(0))->getVol();

           ptr2 = tmp_data;
           len = fhdr.nx * fhdr.ny;
           while(len) {
               if(*ptr == bad_data) {
                  *ptr2++ =  miss_data;
               ptr++;
               } else {
                   *ptr2++ = *ptr++;
               } 
               len--;
           }
        }
    
       if( gd.layers.map_missing_to_min_value) { // Map missing to Min value in levels
           if(!using_temp) { // Allocate a temporary array
             using_temp = 1;
             if((tmp_data = (float *) calloc(fhdr.nx*fhdr.ny,sizeof(float))) == NULL) {
                perror("CIDD: Alloc failure");
                exit(-1);
             }
             ptr = (float *) (float *) (mr->h_mdvx->getFieldByNum(0))->getVol();
           } else {  // Already have temp_array
             ptr = tmp_data;
           }
    
       ptr2 = tmp_data;
           len = fhdr.nx*fhdr.ny;
           while(len) {
               if(*ptr == miss_data) {
                *ptr2++ =  fhdr.min_value - 1.0;
            ptr++;
           } else {
                    *ptr2++ = *ptr++;
           }
               len--;
           }
        }

       if( gd.layers.map_bad_to_min_value) { // Map bad data to Min value in levels
           if(!using_temp) { // Allocate a temporary array
             using_temp = 1;
             if((tmp_data = (float *) calloc(fhdr.nx*fhdr.ny,sizeof(float))) == NULL) {
                perror("CIDD: Alloc failure");
                exit(-1);
             }
             ptr = (float *)  (mr->h_mdvx->getFieldByNum(0))->getVol();
           } else {  // Already have temp_array
             ptr = tmp_data;
           }

       ptr2 = tmp_data;
           len = fhdr.nx*fhdr.ny;
           while(len) {
               if(*ptr == bad_data)  {
                *ptr2 =  fhdr.min_value - 1.0;
            ptr++;
           } else {
                    *ptr2++ = *ptr++;
           }
               len--;
           }
        }


    // If the projects match - Can use fast Rectangle rendering.
    int do_fast = 0;
    switch(gd.proj.getProjType()) {
	case  Mdvx::PROJ_LATLON:
          if(fhdr.proj_type == Mdvx::PROJ_LATLON) {
	      do_fast = 1;
	  } else {
	      do_fast = 0;
	  }
	break;

	default:
          if(fhdr.proj_type == gd.proj.getProjType() &&
            (fabs(gd.h_win.origin_lat - mr->h_fhdr.proj_origin_lat) < 0.01) &&
            (fabs(gd.h_win.origin_lon - mr->h_fhdr.proj_origin_lon) < 0.01)) {
	      do_fast = 1;
	      //
	      // Retract that for rotated grids - Niles.
	      //
	      if ((fhdr.proj_type == Mdvx::PROJ_FLAT) && (gd.proj_param[0] != fhdr.proj_rotation))
		do_fast = 0;
	  } else {
	      do_fast = 0;
	  }
	break;
    }

    if(do_fast) {
        // Compute Grid Coordinates
        grid_to_disp_proj(mr,0,0,&x_dproj,&y_dproj);
        disp_proj_to_pixel(&gd.h_win.margin,x_dproj,y_dproj,&startx,&starty);
        
        grid_to_disp_proj(mr,fhdr.nx-1,fhdr.ny-1,&x_dproj,&y_dproj);
        disp_proj_to_pixel(&gd.h_win.margin,x_dproj,y_dproj,&endx,&endy);
            
        delt = (endx - startx) / (double) (fhdr.nx - 1.0);
        for(i=0; i < fhdr.nx; i++) {
            x_grid[i] = (int)(startx + (i * delt));
        }
            
        delt = (endy - starty) / (double) (fhdr.ny - 1.0);
        for(i=0; i < fhdr.ny; i++) {
            y_grid[i] = (int)(starty + (i * delt));
        }

        if(crec->num_levels > 0) {
           if(using_temp) {
               ptr = tmp_data;
           } else {
               ptr = (float *) (mr->h_mdvx->getFieldByNum(0))->getVol();
           }
           RASCONinit(gd.dpy,xid,fhdr.nx,fhdr.ny,x_grid,y_grid);
           RASCONcontour(gc_array,ptr,&miss_data,RASCON_FLOAT,
               crec->num_levels,
               levels,
               RASCON_LINE_CONTOURS, cont_features,
               1.0,0.0);

        }
             
    } else {   // Render on a distorted grid

       RASCONBoxEdge *bedge = NULL;
       double lat,lon;
       double x_km,y_km;
       float *row1_ptr;
       float *row2_ptr;

	   

       if(using_temp) {
	 ptr = tmp_data;
       } else { 
	 ptr = (float *) (mr->h_mdvx->getFieldByNum(0))->getVol();
       }

       if((bedge = (RASCONBoxEdge *) calloc(fhdr.nx,sizeof(RASCONBoxEdge))) == NULL) {
            if(using_temp) free(tmp_data);
	    perror("Unable to allocate temp space for contours ");
	    return;
       }

	   int clip_flag = 0;
	   int minx = -CLIP_BUFFER;
	   int miny = -CLIP_BUFFER;
	   int maxx = gd.h_win.can_dim.width + CLIP_BUFFER;
	   int maxy = gd.h_win.can_dim.height + CLIP_BUFFER;

       // Compute the first row of points
       for(i=0; i < fhdr.nx; i++) {  // for each Column
		   clip_flag = 0;
           mr->proj->xyIndex2latlon(i,0,lat,lon); // Grid to World 
	   gd.proj.latlon2xy(lat,lon,x_km,y_km); // World to Map

	   // Map to Screen
	   disp_proj_to_pixel(&(gd.h_win.margin),x_km,y_km,&(bedge[i].x2),&(bedge[i].y2));
	   if(_params.check_clipping) {
	     if(bedge[i].x2 < minx) clip_flag = 1;
	     if(bedge[i].x2 > maxx) clip_flag = 1;
	     if(bedge[i].y2 < miny) clip_flag = 1;
	     if(bedge[i].y2 > maxy) clip_flag = 1;
	     if(clip_flag) *(ptr+i) = fhdr.missing_data_value;
	  }
       }

	double ratio = 0.0;
	int diffx = 0;
	int diffy = 0;
       for(j = 1; j < fhdr.ny; j++) { // For each row
	   for(i=0; i < fhdr.nx; i++) {  // for each Column
	       // First rotate point 2 to point 1.
	       bedge[i].x1 = bedge[i].x2;
	       bedge[i].y1 = bedge[i].y2;

	       mr->proj->xyIndex2latlon(i,j,lat,lon); // Grid to World
	       gd.proj.latlon2xy(lat,lon,x_km,y_km); // World to Map

	       // Map to Screen 
	       disp_proj_to_pixel(&(gd.h_win.margin),x_km,y_km,&(bedge[i].x2),&(bedge[i].y2));
	       if(_params.check_clipping) {
	         clip_flag = 0;
	         if(bedge[i].x2 < minx) clip_flag = 1;
	         if(bedge[i].x2 > maxx) clip_flag = 1;
	         if(bedge[i].y2 < miny) clip_flag = 1;
	         if(bedge[i].y2 > maxy) clip_flag = 1;
	         if(clip_flag) *(ptr+i) = fhdr.missing_data_value;

                 if( i > 0) {
                   diffx = abs(bedge[i].x2 - bedge[i-1].x2) +1;
                   diffy = abs(bedge[i].y2 - bedge[i-1].y2) +1;
                   ratio = (double) diffy / diffx;
                   if(ratio > MAX_ASPECT_RATIO ) clip_flag = 1;
                   if(ratio < 1/MAX_ASPECT_RATIO ) clip_flag = 1;
                 }
              }
              if(clip_flag) {
                  *(ptr + (j * fhdr.nx) +i) = fhdr.missing_data_value;
              }

	   }
	   row1_ptr = ptr + ((j-1) * fhdr.nx);
	   row2_ptr = ptr + (j * fhdr.nx);

       RASCONcontour2Rows(gd.dpy,xid,gc_array,fhdr.nx,j,
	       (char *) row1_ptr, (char *) row2_ptr, (char *) &miss_data,
	       RASCON_FLOAT, crec->num_levels,levels,bedge,
	       RASCON_LINE_CONTOURS, cont_features, 1.0,0.0);


      }
      if(bedge != NULL) free(bedge);
	}
    if(using_temp) free(tmp_data);

#endif
    
    return;
}
