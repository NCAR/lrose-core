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
 * RENDER_LEGENDS
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage    July 1991 NCAR, Research Applications Program
 */

#define RENDER_LEGENDS

#include "cidd.h"
#include <toolsa/str.h>

/**********************************************************************
 * VLEVEL_LABEL: Return an appropriate units label for a field's
 *               vertical coordinate system
 */                  
const char *vlevel_label(Mdvx::field_header_t *fhdr)
{
  switch(fhdr->vlevel_type)  {
  case Mdvx::VERT_TYPE_SIGMA_P:
    return("Sigma P");
  case Mdvx::VERT_TYPE_PRESSURE:
    return("mb");
  case Mdvx::VERT_TYPE_Z:
    return("km");
  case Mdvx::VERT_TYPE_SIGMA_Z:
    return("Sigma Z");
  case Mdvx::VERT_TYPE_ETA:
    return("ETA");
  case Mdvx::VERT_TYPE_THETA:
    return("Theta");
  case Mdvx::VERT_TYPE_MIXED:
    return("Mixed");
  case Mdvx::VERT_TYPE_ELEV:
  case Mdvx::VERT_VARIABLE_ELEV:
    return("Deg");
  case Mdvx::VERT_FLIGHT_LEVEL:
    return("FL");
  default:
    return "";
  }
}

/**********************************************************************
 * FIELD_LABEL: Return a Label string for a field 
 */

const char * field_label( met_record_t *mr)
{
  // time_t  now;
    struct tm tms;
    char   tlabel[1024];
    static char label[2048];

    /* Convert to a string */
    if(_params.use_local_timestamps) {
      strftime(tlabel,256,_params.label_time_format,localtime_r(&mr->h_date.unix_time,&tms));
    } else {
      strftime(tlabel,256,_params.label_time_format,gmtime_r(&mr->h_date.unix_time,&tms));
    }

    // now = time(NULL);
    label[0] = '\0';
     
    if(mr->composite_mode == FALSE) {
      if(!gd.wsddm_mode) {
	// Check for surface fields 
	if(((mr->vert[mr->plane].cent == 0.0) && (mr->h_fhdr.grid_dz == 0)) ||
	   mr->h_fhdr.vlevel_type == Mdvx::VERT_TYPE_SURFACE ||
	   mr->h_fhdr.vlevel_type == Mdvx::VERT_TYPE_COMPOSITE ||
	   mr->ds_fhdr.nz == 1) {  
	   //snprintf(label,"%s At Surface %s",
          snprintf(label,2048,"%s: %s",
                mr->legend_name,
                tlabel);
	} else {
	 // Reverse order of label and value if units are "FL" 
	 if(strcmp(mr->units_label_sects,"FL") == 0) {
           snprintf(label,2048,"%s: %s %03.0f %s",
                mr->legend_name,
                mr->units_label_sects,
                mr->vert[mr->plane].cent,
		tlabel);
	 }else {
           snprintf(label,2048,"%s: %g %s %s",
                mr->legend_name,
                mr->vert[mr->plane].cent,
                mr->units_label_sects,
                tlabel);
	  }
        }
      } else { // Use an abbreviated label in Wsddm mode.
         snprintf(label,2048,"%s %s", mr->legend_name, tlabel);
      }
    } else {
        snprintf(label,2048,"%s: All levels %s",
                mr->legend_name,
                tlabel);
    }

    /* If data is Forecast, add a  label */
    if( mr->h_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST) {
      //strcat(label," FORECAST Gen");
    }

    return label;
}

#define LABEL_LEN  256
/*************************************************************************
 * HEIGHT_LABEL: Return the height label
 */
const char* height_label()
{
   static char label[256];

   met_record_t *mr;       /* pointer to record for convienence */
   mr = choose_ht_sel_mr(gd.h_win.page);

   switch(mr->h_vhdr.type[0]) {
     case Mdvx::VERT_TYPE_SURFACE:
         strcpy(label,"   " );    // For Now-  Because many Taiwan fields are wrongly
				  // set to "Surface", example: Freezing Level.
         //strcpy(label,"Surface" );
     break;

     case 0:
     case Mdvx::VERT_VARIABLE_ELEV:
     case Mdvx::VERT_FIELDS_VAR_ELEV:
     case Mdvx::VERT_TYPE_COMPOSITE:
         strcpy(label,"   " );
     break;

     case Mdvx::VERT_SATELLITE_IMAGE:
         strcpy(label,"Sat Image" );
     break;

     case Mdvx::VERT_FLIGHT_LEVEL:
       snprintf(label,256,"Flight Level %03d", (int)gd.h_win.cur_ht);
     break;

     default:
       snprintf(label,256,"%g %s",gd.h_win.cur_ht,
		 mr->units_label_sects);
     break;
  }

   return label;
}

/**********************************************************************
 * DRAW_HWIN_INTERIOR_LABELS: Label the interior of the horizontal image
 *
 */

int draw_hwin_interior_labels( Drawable xid, int page, time_t start_time, time_t end_time)
{
    int    i;
    int    out_of_date;    
    int    stretch_secs;
    int    x_start,y_start;
    int    xmid,ymid;
    int    ht,wd;
    met_record_t *mr;        /* pointer to record for convienence */
    char    label[LABEL_LEN * 8];
    Font    font;

    label[0] = '\0';

    /* set ht between legend labels */

    ht = gd.h_win.legends_delta_y;
    if(ht <= 1) ht = 20;  // A back in case not set

  mr = gd.mrec[page];    /* get pointer to main gridded field record */
  stretch_secs =  (int) (60.0 * mr->time_allowance);
  out_of_date = 0;
  if(_params.check_data_times) {
      if(mr->h_date.unix_time < start_time - stretch_secs) out_of_date = 1;
      if(mr->h_date.unix_time > end_time + stretch_secs) out_of_date = 1;
  }

  gd.num_field_labels = 0;
  /* If data appears to be valid - use its time for a clock */
  if(mr->h_data != NULL && !out_of_date) { 
    /* draw a clock */
    if (_params.show_clock) {
      x_start = gd.h_win.can_dim.width - gd.h_win.margin.right - ht - 5;
      y_start = gd.h_win.legends_start_y + ht + 5;
      XUDRdraw_clock(gd.dpy,xid, gd.legends.foreground_color->gc,
		     x_start, y_start, ht, mr->h_date.unix_time,
		     _params.draw_clock_local);
    }
    
  }

  // Bail out completely if configured not to display labels
  if(_params.display_labels == 0) return CIDD_SUCCESS;
     
  x_start = gd.h_win.legends_start_x;
  y_start = gd.h_win.legends_start_y;
  wd = gd.h_win.can_dim.width - x_start - gd.h_win.margin.right;

  if(mr->h_data != NULL && !out_of_date) {  // If primary data appears to be valid
    strncat(label,field_label(mr),LABEL_LEN);
  } else {
    time_t now;
    struct tm *gmt;
    struct tm res;
    char fmt_str[2048];

    now = time(0);
    if(_params.use_local_timestamps) {
      gmt = localtime_r(&now,&res);
    } else {
      gmt = gmtime_r(&now,&res);
    }

    if(strncasecmp(mr->legend_name,"None",4) == 0 )  {
      strncpy(fmt_str,"Image Generated at ",128);
    } else if(strncasecmp(mr->legend_name,"Empty",5) == 0 )  {
      strncpy(fmt_str,"Frame time ",128);
    } else {
      snprintf(fmt_str,2048,"%s Not Available at ",mr->legend_name);
    }
    strncat(fmt_str,_params.label_time_format,128);
    if (gd.movie.mode == REALTIME_MODE) {
      strftime(label,LABEL_LEN,fmt_str,gmt);
    } else {
      if(_params.use_local_timestamps) {
        gmt = localtime_r(&gd.data_request_time,&res);
      } else {
        gmt = gmtime_r(&gd.data_request_time,&res);
      }
      strftime(label,LABEL_LEN,fmt_str, gmt);
    }
  }

  font = choose_font(label,wd,ht,&xmid,&ymid);
  XSetFont(gd.dpy,gd.legends.foreground_color->gc,font);

  if(_params.font_display_mode == 0)
    XDrawString(gd.dpy,xid,gd.legends.foreground_color->gc, x_start,y_start + ymid,label,strlen(label));
  else
    XDrawImageString(gd.dpy,xid,gd.legends.foreground_color->gc, x_start,y_start + ymid,label,strlen(label));
    
  gd.num_field_labels++;
  y_start += gd.h_win.legends_delta_y;

  if(gd.layers.layer_legends_on) {
   for(i=0 ; i < NUM_GRID_LAYERS; i++) {   /* Add a label for each active overlay field */
     if(gd.layers.overlay_field_on[i]) {   /* Add a label for the overlay field */
	mr = gd.mrec[gd.layers.overlay_field[i]]; 
	if(_params.check_data_times) {
	  if(mr->h_date.unix_time < start_time - stretch_secs) out_of_date = 1;
	  if(mr->h_date.unix_time > end_time + stretch_secs) out_of_date = 1;
	}

	if(mr->h_data != NULL && !out_of_date) {
	  snprintf(label,LABEL_LEN * 8,"Layer %d : ",i+1);
	  strncat(label,field_label(mr),256);
          font = choose_font(label,wd,ht,&xmid,&ymid);
          XSetFont(gd.dpy,gd.legends.foreground_color->gc,font);

	  if(_params.font_display_mode == 0)
            XDrawString(gd.dpy,xid,gd.legends.foreground_color->gc, x_start,y_start + ymid,label,strlen(label));
	  else
            XDrawImageString(gd.dpy,xid,gd.legends.foreground_color->gc, x_start,y_start + ymid,label,strlen(label));

          gd.num_field_labels++;
          y_start += gd.h_win.legends_delta_y;
	}
    }
   }
  }

  if(gd.layers.cont_legends_on) {
   for(i=0; i < NUM_CONT_LAYERS; i++) {
    if(gd.layers.cont[i].active) {     /* Add a label for the contour field */
	mr = gd.mrec[gd.layers.cont[i].field];
	if(_params.check_data_times) {
	   if(mr->h_date.unix_time < start_time - stretch_secs) out_of_date = 1;
	   if(mr->h_date.unix_time > end_time + stretch_secs) out_of_date = 1;
	}

	if(mr->h_data != NULL && !out_of_date) {
          snprintf(label,LABEL_LEN * 8,"Contour layer %d ",i);
	  strncat(label,field_label(mr),LABEL_LEN);
          font = choose_font(label,wd,ht,&xmid,&ymid);
          XSetFont(gd.dpy,gd.layers.cont[i].color->gc,font);
	  if(_params.font_display_mode == 0)
            XDrawString(gd.dpy,xid,gd.layers.cont[i].color->gc, x_start,y_start + ymid,label,strlen(label));
	  else 
            XDrawImageString(gd.dpy,xid,gd.layers.cont[i].color->gc, x_start,y_start + ymid,label,strlen(label));

          gd.num_field_labels++;
          y_start += gd.h_win.legends_delta_y;
	}
    }
   }
  }

    if(gd.layers.wind_vectors && gd.layers.wind_legends_on == 1 ) {    /* Add wind labels */
        for(i=0; i < gd.layers.num_wind_sets; i++) {
            if(gd.layers.wind[i].active == 0) continue;
            mr = gd.layers.wind[i].wind_u;
            out_of_date = 0;
            stretch_secs =  (int) (60.0 * mr->time_allowance);
	    if(_params.check_data_times) {
               if(mr->h_date.unix_time < start_time - stretch_secs) out_of_date = 1;
               if(mr->h_date.unix_time > end_time + stretch_secs) out_of_date = 1;
	    }

            if(gd.layers.wind[i].wind_u->h_data_valid  && gd.layers.wind[i].wind_v->h_data_valid &&
			        gd.layers.wind[i].wind_u->h_data != NULL && !out_of_date) {
              snprintf(label,LABEL_LEN * 8,"%s", field_label(gd.layers.wind[i].wind_u));
            } else {
	      if(gd.wsddm_mode) { // No error message for missing vectors in wsddm_mode mode.
		    label[0] = '\0';
	      } else {
                if(out_of_date) {
                  snprintf(label,LABEL_LEN * 8,"%s No Data ",mr->legend_name);
		} else {
                  snprintf(label,LABEL_LEN * 8,"%s - Winds Error - U or V missing",mr->legend_name);
	        }
	      }
            }
                
         font = choose_font(label,wd,ht,&xmid,&ymid);
         XSetFont(gd.dpy,gd.layers.wind[i].color->gc,font);

	if(_params.font_display_mode == 0)
           XDrawString(gd.dpy,xid,gd.layers.wind[i].color->gc, x_start,y_start + ymid,label,strlen(label));
	else 
           XDrawImageString(gd.dpy,xid,gd.layers.wind[i].color->gc, x_start,y_start + ymid,label,strlen(label));

	 gd.num_field_labels++;
	 y_start += gd.h_win.legends_delta_y;
        }
    }
    return CIDD_SUCCESS;
}
 
