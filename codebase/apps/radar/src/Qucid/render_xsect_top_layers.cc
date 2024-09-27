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
 * RENDER_XSECT_TOP_LAYERS
 *        
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_XSECT_TOP_LAYERS
#include "cidd.h"
#include <toolsa/str.h>

/**********************************************************************
 * RENDER_XSECT_TOP_LAYERS: Render reference lines
 */

void render_xsect_top_layers(QPaintDevice *pdev, int page)
{

#ifdef NOTYET
  
    int x1,y1,x2,y2;
    double   x_dproj,km_ht;
    double dir,spd;
    int idir,ispd;
    char wind_label[1024];
    char hazard_label[1024];
    char screen_label[2048];
    int xmid,ymid;
    Font    font;

    static double units_scale_factor = 0.0;
    static const char* units_label = NULL;  

    if(units_scale_factor == 0.0) { // first time 
      units_scale_factor = _params.wind_units_scale_factor;
    }
    if (units_label == NULL) {
      units_label = _params.wind_units_label;
    }

    /* Render cross reference line showing the altitude of the plan view */
    if (_params.display_ref_lines && gd.mrec[page]->h_fhdr.proj_type != Mdvx::PROJ_POLAR_RADAR) {
        km_ht = gd.h_win.cur_ht; 

	x_dproj = gd.h_win.route.total_length; 

        disp_proj_to_pixel_v(&gd.v_win.margin,0.0,km_ht,&x1,&y1);    /* from 0.0 km */
        disp_proj_to_pixel_v(&gd.v_win.margin,x_dproj,km_ht,&x2,&y2);    /* to full range */
	// Set GC to draw 1 pixel wide, dashed lines
         XSetLineAttributes(gd.dpy,gd.legends.route_path_color->gc,
				   1,LineOnOffDash,CapButt,JoinRound);
        XDrawLine(gd.dpy,xid,gd.legends.route_path_color->gc,x1,y1,x2,y2);
    } 

    // Clean out labels
    memset(wind_label,0,128);
    memset(hazard_label,0,128); 
    memset(screen_label,0,128); 

    if(gd.layers.route_wind.u_wind == NULL || gd.layers.route_wind.v_wind == NULL ||
      gd.layers.route_wind.u_wind->v_data_valid == 0 ||
      gd.layers.route_wind.v_wind->v_data_valid == 0 ||
      gd.layers.route_wind.u_wind->v_data == NULL ||
      gd.layers.route_wind.v_wind->v_data == NULL ) {
      // Do nothing
    } else {
      // compute and display the average winds over the whole route.
      ave_winds(gd.layers.route_wind.u_wind,gd.layers.route_wind.v_wind,
              0.0, gd.h_win.route.total_length,&dir,&spd);
      spd *= units_scale_factor;
      ispd = (int) (spd + 2.5);
      ispd -= ispd % 5;

       // normalize to 0 - 360
       if(dir < 0.0) dir += 360.0;

       // Round to the nearest 5 degrees
       idir = (int) (dir + 2.5);
       idir -= idir % 5;

       snprintf(wind_label,1024,"Avg Wind: %03d/%.2d %s",idir,ispd,units_label);

    }

    // Compute Peak turbulence if available  - Add it to hazard_label
    if(gd.layers.route_wind.turb != NULL ) {
        if(gd.layers.route_wind.turb->v_data_valid != 0 &&
           gd.layers.route_wind.turb->v_data != NULL) {
           double pk_turb = peak_turb(gd.layers.route_wind.turb,
                                      0.0, gd.h_win.route.total_length);

           if(pk_turb >= _params.route_turb_low_thresh &&
              pk_turb < _params.route_turb_mod_thresh) {
             strncat(hazard_label," ! MOD CAT",128);
           }

           if(pk_turb >= _params.route_turb_mod_thresh &&
              pk_turb < _params.route_turb_high_thresh) {
             strncat(hazard_label," ! HIGH CAT",128);
           }

           if(pk_turb >= _params.route_turb_high_thresh) {
             strncat(hazard_label," ! VHIGH CAT",128);
           }
        }
    }

    // Compute Peak icing if available  - Add it to hazard_label
    if(gd.layers.route_wind.icing != NULL ) {
        if(gd.layers.route_wind.icing->v_data_valid != 0 &&
           gd.layers.route_wind.icing->v_data != NULL) {
           double pk_icing = peak_icing(gd.layers.route_wind.icing,
                                      0.0, gd.h_win.route.total_length);

           if(pk_icing >= _params.route_icing_low_thresh &&
              pk_icing < _params.route_icing_mod_thresh) {
             strncat(hazard_label," ! LGT ICE",128);
           }

           if(pk_icing >= _params.route_icing_mod_thresh &&
              pk_icing < _params.route_icing_high_thresh) {
             strncat(hazard_label," ! MOD ICE",128);
           }

           if(pk_icing >= _params.route_icing_high_thresh) {
             strncat(hazard_label," ! HVY ICE",128);
           }
        }
    }

    // Pack all labels together
    STRconcat(screen_label,wind_label,256);
    STRconcat(screen_label,hazard_label,256);

    if(strlen(screen_label) > 3) {
      font = choose_font(screen_label,gd.v_win.img_dim.width/2,gd.v_win.margin.top,&xmid,&ymid);
      XSetFont(gd.dpy,gd.legends.foreground_color->gc,font);
      XDrawImageString(gd.dpy,xid,gd.legends.foreground_color->gc,
		       gd.v_win.margin.left + (gd.v_win.img_dim.width / 2) + xmid,
		       (2 * gd.v_win.margin.top) + ymid,
		       screen_label,strlen(screen_label));
 
    }

#endif

}
