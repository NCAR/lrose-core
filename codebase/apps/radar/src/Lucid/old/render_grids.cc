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
 * RENDER_GRIDS
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_GRIDS

#include "cidd.h"

#define MESSAGE_LEN 1024
/**********************************************************************
 * RENDER_GRID: Render a horizontal plane of  gridded data 
 *    Returns 1 on success, 0 on failure
 */

int render_grid( QPaintDevice *pdev, met_record_t *mr, time_t start_time, time_t end_time, int is_overlay_field)
{

#ifdef NOTYET
  
  int    out_of_date;    
  int    stretch_secs;
  int    ht;            /* Dims of data rectangles  */
  int    startx;        /* pixel limits of data area */
  int    starty;        /* pixel limits of data area */
  int    xmid,ymid;
  char   message[MESSAGE_LEN];    /* Error message area */
  Font    font;

  out_of_date = 0;
  stretch_secs =  (int) (60.0 * mr->time_allowance);

  if(_params.check_data_times) {
    if(mr->h_date.unix_time < start_time - stretch_secs) out_of_date = 1;
    if(mr->h_date.unix_time > end_time + stretch_secs) out_of_date = 1;
  }

  // Add the list of times to the time plot
  if(mr->time_list.num_entries > 0) {
    if(gd.time_plot) gd.time_plot->add_grid_tlist(mr->legend_name,mr->time_list.tim,
                                                  mr->time_list.num_entries,
                                                  mr->h_date.unix_time);
  }

  /* For the Main Field - Clear the screen and  Print a special message 
   * and draw the wall clock time if the data is not availible 
   */
  if(!is_overlay_field) {


    /* If no data in current response or data is way out of date */
    if( mr->h_data == NULL || out_of_date ) {

      if(strncasecmp(mr->button_name,"None",4) &&
         strncasecmp(mr->button_name,"Empty",5)) {
        /* display "Data Not Available" message */
        if(out_of_date) {
          snprintf(message,MESSAGE_LEN,"%s - Data too Old", _params.no_data_message);
        } else {
          STRcopy(message,_params.no_data_message,MESSAGE_LEN);
        }

        font = choose_font(message, gd.h_win.img_dim.width, gd.h_win.img_dim.height, &xmid, &ymid);
        XSetFont(gd.dpy,gd.legends.foreground_color->gc,font);
        XDrawImageString(gd.dpy,xid,gd.legends.foreground_color->gc,
                         gd.h_win.margin.left + (gd.h_win.img_dim.width /2) + xmid  ,
                         gd.h_win.margin.top + (gd.h_win.img_dim.height /4) + ymid ,
                         message,strlen(message));
        
        if(gd.debug2) {
          fprintf(stderr, "No data from service: %s\n",
                  gd.io_info.mr->url );
        }
        
	if(_params.show_clock) {
          /* draw a clock */
          ht = (int) (gd.h_win.can_dim.height * 0.05);
          startx = gd.h_win.can_dim.width - gd.h_win.margin.right - ht - 5;
          starty = gd.h_win.margin.top + ht + 5;
          XUDRdraw_clock(gd.dpy,xid,gd.legends.foreground_color->gc,
                         startx,starty,ht, (start_time + (end_time - start_time) /2),1);
	}
        
      }  // if mr->button_name != "None"

      return CIDD_FAILURE;
    }
  }
     
  set_busy_state(1);


  // Decide Proper rendering routine
  switch(mr->h_fhdr.proj_type) {
    default: // Projections which need only matching types and origins.
      // If the projections match - Can use fast Rectangle rendering.
      if(mr->h_fhdr.proj_type == gd.proj.getProjType() &&
         (fabs(gd.h_win.origin_lat - mr->h_fhdr.proj_origin_lat) < 0.001) &&
         (fabs(gd.h_win.origin_lon - mr->h_fhdr.proj_origin_lon) < 0.001)) {
        if(gd.debug2) fprintf(stderr,"render_cart_grid() selected\n");
        render_cart_grid( xid, mr,start_time, end_time,
                          is_overlay_field);
        if(gd.debug2) fprintf(stderr,"render_cart_grid() done\n");
      } else { // Must use polygon rendering
        if(gd.debug2) fprintf(stderr,"render_distorted_grid() selected\n");
        render_distorted_grid( xid, mr,start_time, end_time,
                               is_overlay_field);
      }
      break;

    case  Mdvx::PROJ_FLAT: // Needs to test Param 1
      // If the projections match - Can use fast Rectangle rendering.
      if(mr->h_fhdr.proj_type == gd.proj.getProjType() &&
         (fabs(gd.proj_param[0] - mr->h_fhdr.proj_param[0]) < 0.001) &&
         (fabs(gd.h_win.origin_lat - mr->h_fhdr.proj_origin_lat) < 0.001) &&
         (fabs(gd.h_win.origin_lon - mr->h_fhdr.proj_origin_lon) < 0.001)) {

        if(gd.debug2) fprintf(stderr,"render_cart_grid() selected\n");
        render_cart_grid( xid, mr,start_time, end_time,
                          is_overlay_field);
        if(gd.debug2) fprintf(stderr,"render_cart_grid() done\n");
      } else { // Must use polygon rendering
        if(gd.debug2) fprintf(stderr,"render_distorted_grid() selected\n");
        render_distorted_grid( xid, mr,start_time, end_time,
                               is_overlay_field);
        if(gd.debug2) fprintf(stderr,"render_distorted_grid() done\n");
      }
      break;
      
    case  Mdvx::PROJ_LAMBERT_CONF: // Needs to test param 1 & 2
    case  Mdvx::PROJ_POLAR_STEREO: 
    case  Mdvx::PROJ_OBLIQUE_STEREO:
    case  Mdvx::PROJ_MERCATOR:
      // If the projections match - Can use fast Rectangle rendering.
      if(mr->h_fhdr.proj_type == gd.proj.getProjType() &&
         (fabs(gd.proj_param[0] - mr->h_fhdr.proj_param[0]) < 0.001) &&
         (fabs(gd.proj_param[1] - mr->h_fhdr.proj_param[1]) < 0.001) &&
         (fabs(gd.h_win.origin_lat - mr->h_fhdr.proj_origin_lat) < 0.001) &&
         (fabs(gd.h_win.origin_lon - mr->h_fhdr.proj_origin_lon) < 0.001)) {
        if(gd.debug2) fprintf(stderr,"render_cart_grid() selected\n");
        render_cart_grid( xid, mr,start_time, end_time,
                          is_overlay_field);
        if(gd.debug2) fprintf(stderr,"render_cart_grid() done\n");
      } else { // Must use polygon rendering
        if(gd.debug2) fprintf(stderr,"render_distorted_grid() selected\n");
        render_distorted_grid( xid, mr,start_time, end_time,
                               is_overlay_field);
        if(gd.debug2) fprintf(stderr,"render_distorted_grid() done\n");
      }
      break;
      
    case  Mdvx::PROJ_LATLON:
      if(mr->h_fhdr.proj_type == gd.proj.getProjType()) {
        if(gd.debug2) fprintf(stderr,"render_cart_grid() selected\n");
        render_cart_grid( xid, mr,start_time, end_time,
                          is_overlay_field);
        if(gd.debug2) fprintf(stderr,"render_cart_grid() done\n");
      } else {
        if(gd.debug2) fprintf(stderr,"render_distorted_grid() selected\n");
        render_distorted_grid( xid, mr,start_time, end_time,
                               is_overlay_field);
        if(gd.debug2) fprintf(stderr,"render_distorted_grid() done\n");
      }
      break;
      
    case  Mdvx::PROJ_POLAR_RADAR:
      if(gd.debug2) fprintf(stderr,"render_polar_grid() selected\n");
      render_polar_grid( xid, mr,start_time, end_time, 
                         is_overlay_field);
      if(gd.debug2) fprintf(stderr,"render_polar_grid() done\n");
      break;
      
  }

#endif
  
  set_busy_state(0);

  return CIDD_SUCCESS;
}
