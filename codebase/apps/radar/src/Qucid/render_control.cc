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
 * RENDER_CONTROL
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_CONTROL

#include "cidd.h"

/**********************************************************************
 * RENDER_HORIZ_DISPLAY: Render a complete horizontal plane of data a
 *        and its associated overlays and labels  labels. 
 */

int render_horiz_display( QPaintDevice *pdev, int page, time_t start_time, time_t end_time)
{

#ifdef NOTYET
  
  cerr << "HHHHHHHHHHHHHHHHHHHHHHHHHHHHH pdev: " << pdev << endl;
  
  int i;
  contour_info_t cont; // contour params 
  met_record_t *mr;

  if(!_params.run_once_and_exit)  PMU_auto_register("Rendering (OK)");
  if(gd.debug2) fprintf(stderr,"Rendering Plan View Image, page :%d\n",page);

  // compute distance across the image for setting font sizes, etc.
  switch(gd.display_projection) {
    default:
    case Mdvx::PROJ_FLAT :
    case Mdvx::PROJ_LAMBERT_CONF :
      /* compute km across the image */
      gd.h_win.km_across_screen = (gd.h_win.cmax_x - gd.h_win.cmin_x);
      break;

    case Mdvx::PROJ_LATLON :
      gd.h_win.km_across_screen = (gd.h_win.cmax_x - gd.h_win.cmin_x) * KM_PER_DEG_AT_EQ;
      break;
  }

  mr = gd.mrec[page];

  /* Clear drawing area */
  cerr << "XXXXXXXXXXXXXXXXXXXX" << endl;
  if(pdev == 0) return CIDD_FAILURE;
  XFillRectangle(gd.dpy,xid,gd.legends.background_color->gc,
                 0,0,gd.h_win.can_dim.width,gd.h_win.can_dim.height);

  // Clear time lists
  if(gd.time_plot) gd.time_plot->clear_grid_tlist();
  if(gd.time_plot) gd.time_plot->clear_prod_tlist();

  if(_params.show_data_messages) gui_label_h_frame("Rendering",-1);

  // RENDER the LAND_USE field first
  if(gd.layers.earth.landuse_active) {
    render_grid(xid,gd.layers.earth.land_use,start_time,end_time,1);
  }

  if(!_params.draw_main_on_top) {
    if(mr->render_method == LINE_CONTOURS) {
      cont.min = mr->cont_low;
      cont.max = mr->cont_high;
      cont.interval = mr->cont_interv;
      cont.active = 1;
      cont.field = page;
      cont.labels_on = _params.label_contours;
      cont.color = gd.legends.foreground_color;
      cont.vcm = &mr->h_vcm;
      if (gd.layers.use_alt_contours) {
        RenderLineContours(xid,&cont);
      } else {
        render_line_contours(xid,&cont);
      }
    } else {
      render_grid(xid,mr,start_time,end_time,0); 
    }
    if(gd.layers.earth.terrain_active && 
       ((mr->vert[mr->ds_fhdr.nz -1].max - mr->vert[0].min) != 0.0) &&
       (mr->composite_mode == FALSE) && (mr->ds_fhdr.nz > 1) &&
       mr->ds_fhdr.vlevel_type != Mdvx::VERT_TYPE_ELEV) {
      render_h_terrain(xid, page);
    }
  }
     
  /* Render each of the gridded_overlay fields */
  for(i=0; i < NUM_GRID_LAYERS; i++) {
    if(gd.layers.overlay_field_on[i]) {
      render_grid(xid,gd.mrec[gd.layers.overlay_field[i]],start_time,end_time,1);
    }
  }

  if(_params.draw_main_on_top) {
    if(mr->render_method == LINE_CONTOURS) {
      cont.min = mr->cont_low;
      cont.max = mr->cont_high;
      cont.interval = mr->cont_interv;
      cont.active = 1;
      cont.field = page;
      cont.labels_on = _params.label_contours;
      cont.color = gd.legends.foreground_color;
      cont.vcm = &mr->h_vcm;
      if (gd.layers.use_alt_contours) {
        RenderLineContours(xid,&cont);
      } else {
        render_line_contours(xid,&cont);
      }
    } else {
      render_grid(xid,mr,start_time,end_time,0); 
    }
    if(gd.layers.earth.terrain_active && 
       ((mr->vert[mr->ds_fhdr.nz -1].max - mr->vert[0].min) != 0.0) &&
       (mr->composite_mode == FALSE) && (mr->ds_fhdr.nz > 1)) {

      render_h_terrain(xid, page);
    }
  }

  /* render contours if selected */
  for(i= 0; i < NUM_CONT_LAYERS; i++) {
    if(gd.layers.cont[i].active) {
      if (gd.layers.use_alt_contours) {
        RenderLineContours(xid, &(gd.layers.cont[i]));
      } else {
        render_line_contours(xid, &(gd.layers.cont[i]));
      }
    }
  }

  /* render Winds if selected */
  if(gd.layers.wind_vectors) {
    switch(gd.layers.wind_mode) {
      default:
      case WIND_MODE_ON:  /* winds get rendered in each frame */
        render_wind_vectors(xid,start_time,end_time);
        break;

      case WIND_MODE_LAST: /* Winds get rendered in last farame only */
        if(gd.movie.cur_frame == gd.movie.end_frame)
          render_wind_vectors(xid,start_time,end_time);
        break;

      case WIND_MODE_STILL: /* Winds get rendered in the last frame only
                             * if the movie loop is off
                             */
        if(!gd.movie.movie_on && gd.movie.cur_frame == gd.movie.end_frame)
          render_wind_vectors(xid,start_time,end_time);
        break;
    }
  }


  render_top_layers(xid);  // Range rings X section reference etc.

  // Native Symprod products.
  render_products(xid,start_time,end_time);

  render_horiz_margins(xid,page,start_time,end_time);

  update_frame_time_msg(gd.movie.cur_frame);

#endif
  
  return CIDD_SUCCESS;    /* avaliable data has been rendered */
}
