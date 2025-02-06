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
 * RENDER_CART_GRID
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_CART_GRID 1

#include "cidd.h"

/**********************************************************************
 * RENDER_CART_GRID: Render a horizontal plane of  gridded data 
 *    Returns 1 on success, 0 on failure
 */

int render_cart_grid( QPaintDevice *pdev, MetRecord *mr, time_t start_time, time_t end_time, int is_overlay_field)
{
#ifdef NOTYET
  int    i,j;
  int    ht,wd;               /* Dims of data rectangles  */
  int    startx,endx;        /* pixel limits of data area */
  int    starty,endy;        /* pixel limits of data area */
  long   num_points;
  render_method_t render_method;
  double    x_dproj,y_dproj;
  double val;
  int    x_start[MAX_COLS + 4];    /* canvas rectangle begin  coords */
  int    y_start[MAX_ROWS + 4];    /* canvas  rectangle begin coords */
  double    r_ht,r_wd;        /*  data point rectangle dims */
  unsigned short    *ptr;

  ptr = (unsigned short *) mr->h_data;
  if(ptr == NULL) return CIDD_FAILURE;

  // Use unused parameters
  start_time = 0; end_time = 0;
     
  // Establish with method we're going to use to render the grid.
  render_method = (render_method_t) mr->render_method;
  num_points = mr->h_fhdr.nx * mr->h_fhdr.ny;
  if(render_method == DYNAMIC_CONTOURS) {
    if(num_points < _params.dynamic_contour_threshold) {
      render_method = FILLED_CONTOURS;
    } else {
      render_method = POLYGONS;
    }
  }
  
  // int psc = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_VISUAL_CLASS);
  int psc = 0;
  
  // Images don't get countoured
  if(mr->h_fhdr.encoding_type == Mdvx::ENCODING_RGBA32)render_method = POLYGONS;

  /* Calculate Data to Screen Mapping */
  grid_to_disp_proj(mr,0,0,&x_dproj,&y_dproj);
  if(render_method == POLYGONS) x_dproj -= mr->h_fhdr.grid_dx/2.0;    /* expand grid coord by half width */
  if(render_method == POLYGONS) y_dproj -= mr->h_fhdr.grid_dy/2.0;
  disp_proj_to_pixel(&gd.h_win.margin,x_dproj,y_dproj,&startx,&starty);

  grid_to_disp_proj(mr,mr->h_fhdr.nx-1,mr->h_fhdr.ny-1,&x_dproj,&y_dproj);
  if(render_method == POLYGONS) x_dproj += mr->h_fhdr.grid_dx/2.0;    /* expand grid coord by half width */
  if(render_method == POLYGONS) y_dproj += mr->h_fhdr.grid_dy/2.0;
  disp_proj_to_pixel(&gd.h_win.margin,x_dproj,y_dproj,&endx,&endy);

  /* get data point rectangle size */
  if(render_method == POLYGONS) {
    r_ht =  (double)(ABSDIFF(starty,endy))  / ((double) mr->h_fhdr.ny);
    r_wd =  (double)(endx - startx)/ ((double) mr->h_fhdr.nx);    
  } else {
    r_ht =  (double)(ABSDIFF(starty,endy))  / ((double) mr->h_fhdr.ny - 1.0);
    r_wd =  (double)(endx - startx)/ ((double) mr->h_fhdr.nx - 1.0);    
  }

  //fprintf(stderr,"X: %d to %d  Y: %d to %d   WD,HT: %g %g   NX,NY: %d,%d\n",
  //		  startx,endx,starty,endy,r_wd,r_ht,mr->h_fhdr.nx,mr->h_fhdr.ny);

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

  ht = (int) (r_ht + 1.0); 
  wd = (int) (r_wd + 1.0);

  if (_params.clip_overlay_fields) {
      
    if(gd.debug2) printf("Drawing Rectangle Fill Overlay image field: %s\n",
                         mr->field_label);

    // X windows paints downward on the screen. Move starting point to the top of row.
    for(i= 0; i < mr->h_fhdr.ny; i++) {
      y_start[i] -= (ht -1);
    }
      
    // An Image
    if(mr->h_fhdr.encoding_type == Mdvx::ENCODING_RGBA32) {
      
      ui32 pixel;
      ui32 *uptr;
      uptr = (ui32 *) mr->h_data;
        
      XStandardColormap best_map;
      if(! XGetStandardColormap(gd.dpy,RootWindow(gd.dpy,0),&best_map,XA_RGB_BEST_MAP)){
        // fprintf(stderr,"  (Try running 'xstdcmap -best')\n");
        // assume 24-bit depth, 8-bit colors
        fprintf(stderr,"WARNING - XGetStandardColormap() failed\n");
        fprintf(stderr,"  Setting base_pixel = 0\n"); 
        fprintf(stderr,"  Setting   red_mult = 65536, green_mult = 256, blue_mult = 1\n"); 
        fprintf(stderr,"  Setting   red_max  = 255,   green_max  = 255, blue_max  = 255\n"); 
        best_map.base_pixel = 0;
        best_map.red_mult = 65536;
        best_map.green_mult = 256;
        best_map.blue_mult = 1;
        best_map.red_max = 255;
        best_map.green_max = 255;
        best_map.blue_max = 255;
        // return CIDD_FAILURE;
      }
      uptr =  (ui32 * ) mr->h_data;

      for(i= 0; i < mr->h_fhdr.ny; i++) {
        for(j=0;j< mr->h_fhdr.nx; j++) {
          if(MdvGetA(*uptr) != 0) {
            pixel = best_map.base_pixel + 
              ((ui32) (0.5 + ((_params.image_inten * MdvGetR(*uptr) / 255.0) * best_map.red_max)) * best_map.red_mult) +
              ((ui32) (0.5 + ((_params.image_inten * MdvGetG(*uptr) / 255.0) * best_map.green_max)) * best_map.green_mult) +
              ((ui32) (0.5 + ((_params.image_inten * MdvGetB(*uptr) / 255.0) * best_map.blue_max)) * best_map.blue_mult);

            XSetForeground(gd.dpy,gd.def_gc, pixel);
            XFillRectangle(gd.dpy,xid,gd.def_gc, x_start[j],y_start[i],wd,ht);
          }
          uptr++;
        }
      }

    } else { // if(mr->h_fhdr.encoding_type == Mdvx::ENCODING_RGBA32)
      
      mr->num_display_pts = 0;
      for(i= 0; i < mr->h_fhdr.ny; i++) {
        for(j=0;j< mr->h_fhdr.nx; j++) {
          if(mr->h_vcm.val_gc[*ptr] != NULL) {
            val =  (mr->h_fhdr.scale * *ptr) + mr->h_fhdr.bias;
            if(val >= mr->overlay_min && val <= mr->overlay_max) 
              XFillRectangle(gd.dpy,xid,mr->h_vcm.val_gc[*ptr],x_start[j],y_start[i],wd,ht);
            mr->num_display_pts++;
          }
          ptr++;
        }
      }
    }

  } else { // if (_params.clip_overlay_fields) ..
	 
    // Handle case where contours really don't apply (vertical sides)
    if(mr->h_fhdr.min_value == mr->h_fhdr.max_value ) render_method = POLYGONS;
    switch (render_method) {

      case POLYGONS:
        if(( PseudoColor == psc ) && !_params.draw_main_on_top && mr->num_display_pts > _params.image_fill_threshold) {
          draw_filled_image(xid,x_start,y_start,mr);
        } else {
          if(gd.debug2) printf("Drawing Rectangle Fill image, field: %s \n",
                               mr->field_label);
          mr->num_display_pts = 0;
             
          // X windows paints downward on the screen. Move starting point to the top of row.
          for(i= 0; i < mr->h_fhdr.ny; i++) y_start[i]-= (ht-1);
             
          // An Image
          if(mr->h_fhdr.encoding_type == Mdvx::ENCODING_RGBA32) {
            ui32 pixel;
            ui32 *uptr;
            uptr = (ui32 *) mr->h_data;
               
            XStandardColormap best_map;
            if(! XGetStandardColormap(gd.dpy,RootWindow(gd.dpy,0),&best_map,XA_RGB_BEST_MAP)){
              // try to fix the problem
              safe_system("xstdcmap -best",_params.simple_command_timeout_secs);
              if(! XGetStandardColormap(gd.dpy,RootWindow(gd.dpy,0),&best_map,XA_RGB_BEST_MAP)){
                fprintf(stderr,"Failed XGetStandardColormap!\n");
                fprintf(stderr,"Can't Render RGB images - Try Running X Server in 24 bit mode\n");
                fprintf(stderr,"Run 'xstdcmap -best -verbose' to see the problem\n");
                // assume 24-bit depth, 8-bit colors
                fprintf(stderr,"Setting base_pixel = 0, red_mult = 65536, green_mult = 256, blue_mult = 1\n");
                best_map.base_pixel = 0;
                best_map.red_mult = 65536;
                best_map.green_mult = 256;
                best_map.blue_mult = 1;
                // return CIDD_FAILURE;
              }
            } // if(! XGetStandardColormap ...

            for(i= 0; i < mr->h_fhdr.ny; i++) {
              for(j=0;j< mr->h_fhdr.nx; j++) {
                if(MdvGetA(*uptr) != 0) {
                  pixel = best_map.base_pixel + 
                    ((ui32) (0.5 + ((_params.image_inten * MdvGetR(*uptr) / 255.0) * best_map.red_max)) * best_map.red_mult) +
                    ((ui32) (0.5 + ((_params.image_inten * MdvGetG(*uptr) / 255.0) * best_map.green_max)) * best_map.green_mult) +
                    ((ui32) (0.5 + ((_params.image_inten * MdvGetB(*uptr) / 255.0) * best_map.blue_max)) * best_map.blue_mult);
                     
                  XSetForeground(gd.dpy,gd.def_gc, pixel);
                  XFillRectangle(gd.dpy,xid,gd.def_gc, x_start[j],y_start[i],wd,ht);
                  mr->num_display_pts++;
                }
                uptr++;
              }
            } // i
               
          } else { // if(mr->h_fhdr.encoding_type ...
               
            //
            // Not an image
            //
            for(i= 0; i < mr->h_fhdr.ny; i++) {
              for(j=0;j< mr->h_fhdr.nx; j++) {
                if(mr->h_vcm.val_gc[*ptr] != NULL) {
                  if ( is_overlay_field ) {
                    //
                    // If it is an overlay field, need to check value against
                    // min, max specified in GUI. If it is not a value we want to plot,
                    // just increment the pointer and move on. Niles Oien 5/17/2010
                    //
                    val =  (mr->h_fhdr.scale * *ptr) + mr->h_fhdr.bias;
                    if(val < mr->overlay_min || val > mr->overlay_max){
                      ptr++; continue;
                    }
                  }
                  XFillRectangle(gd.dpy,xid,mr->h_vcm.val_gc[*ptr],x_start[j],y_start[i],wd,ht);
                  mr->num_display_pts++;
                }
                ptr++;
              }
            }
          } // if(mr->h_fhdr.encoding_type ...
             
          if(gd.debug2) {
            printf("NUM POLYGONS: %d of %ld \n",
                   mr->num_display_pts,
                   mr->h_fhdr.nx*mr->h_fhdr.ny);
          }
             
        }
        break;
           
      case FILLED_CONTOURS:
        if(gd.debug2) printf("Drawing Filled Contour image: field %s\n",
                             mr->field_label);
        if (gd.layers.use_alt_contours) {
          RenderFilledPolygons(xid, mr);
        } else {
          draw_filled_contours(xid,x_start,y_start,mr);
        }
        break;

      case DYNAMIC_CONTOURS:
      case LINE_CONTOURS:
        break;
        
    } // switch (render_method)

  } // if (_params.clip_overlay_fields) ..

#endif  

  return CIDD_SUCCESS;
  
}
