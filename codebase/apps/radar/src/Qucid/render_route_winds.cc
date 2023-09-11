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
 * RENDER_ROUTE_WINDS
 *
 * For the Configurable Interactive Data Display (CIDD) 
 * Frank Hage   July 1991 NCAR, Research Applications Program
 */


#define RENDER_ROUTE_WINDS

#include "cidd.h"
#include <toolsa/str.h>

typedef struct route_seg {
  XPoint *pixel;
  int    num_points;
} route_seg_t;

int calc_route_points(XPoint **pixel, route_seg_t **, int *num_route_segs);
void  ave_winds(met_record_t *mr_u, met_record_t *mr_v,
               double start_km, double end_km, double *ave_deg, double *ave_spd);
double peak_turb(met_record_t *mr_turb, double start_km, double end_km);
double peak_icing(met_record_t *mr_icing, double start_km, double end_km);
/**********************************************************************
 * RENDER_ROUTE_WINDS: Render the route winds line 
 */

void render_route_winds( Drawable xid)
{
    int i,loop_count, index;
    int num_route_segs,num_route_points;
    double dir,spd;
    int idir,ispd;
    char wind_label[1024];
    char hazard_label[1024];
    int xmid,ymid;
    Font    font;

	route_seg_t *rseg;
    XPoint *pixel;

    static double units_scale_factor = 0.0;
    static const char* units_label;

    if(units_scale_factor == 0.0) { // first time
	units_scale_factor = gd.uparams->getDouble( "cidd.wind_units_scale_factor", 1.0); 
	units_label = gd.uparams->getString( "cidd.wind_units_label", "m/sec");
    }

    if(gd.h_win.route.num_segments <=0) return;

    num_route_points = calc_route_points(&pixel,&rseg, &num_route_segs);

    // Set GC to draw 4 pixel wide, Solid lines
    XSetLineAttributes(gd.dpy,gd.legends.route_path_color->gc,
		 gd.layers.route_wind._P->route_track_line_width,
		 LineSolid,CapButt,JoinRound);


   // Draw route segments.
   for(i = 0; i < num_route_segs; i++ ) {
       XDrawLines(gd.dpy,xid,gd.legends.route_path_color->gc,
		rseg[i].pixel,rseg[i].num_points,CoordModeOrigin);
   }

   if(gd.layers.route_wind._P->add_waypoints_labels) {
     // Label Each way point with the NAVAID ID 
     for(i=0; i <= gd.h_win.route.num_segments; i++) {
	 font = choose_font(gd.h_win.route.navaid_id[i],150,gd.layers.route_wind._P->font_height,&xmid,&ymid);
	 XSetFont(gd.dpy,gd.legends.route_path_color->gc,font);
	 XDrawImageString(gd.dpy,xid,gd.legends.route_path_color->gc,
	     gd.h_win.route.x_pos[i] + (xmid/2) ,
	     gd.h_win.route.y_pos[i] - 5,
	     gd.h_win.route.navaid_id[i],
	     strlen(gd.h_win.route.navaid_id[i]));
     }
   }

   // Clean out labels
   memset(wind_label,0,128);
   memset(hazard_label,0,128);

   if(gd.layers.route_wind.u_wind == NULL || gd.layers.route_wind.v_wind == NULL ||
      gd.layers.route_wind.u_wind->v_data_valid == 0 ||
      gd.layers.route_wind.v_wind->v_data_valid == 0 ||
      gd.layers.route_wind.u_wind->v_data == NULL ||
      gd.layers.route_wind.v_wind->v_data == NULL ) {
      // Do nothing  except clear out message labels
      // xv_set (gd.v_win_v_win_pu->route_msg,PANEL_LABEL_STRING,"",NULL);
      // xv_set (gd.v_win_v_win_pu->hazard_msg,XV_X,xv_get(gd.v_win_v_win_pu->route_msg,XV_WIDTH)+5,NULL);
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

      sprintf(wind_label,"Avg Wind: %03d/%.2d %s",idir,ispd,units_label);
      // xv_set (gd.v_win_v_win_pu->route_msg,PANEL_LABEL_STRING,wind_label,NULL);
      // xv_set (gd.v_win_v_win_pu->hazard_msg,XV_X,xv_get(gd.v_win_v_win_pu->route_msg,XV_WIDTH)+5,NULL);

      // Compute and display average winds along the route
      if(gd.layers.route_wind._P->add_wind_text) {
       switch(gd.layers.route_wind._P->label_style) {
	 case Croutes_P::REGULAR_INTERVALS:
	    loop_count = (int) (gd.h_win.route.total_length / gd.layers.route_wind._P->label_interval);
	    for(i=0; i < loop_count; i++) {
	       // Compute the average for the interval
               ave_winds(gd.layers.route_wind.u_wind,gd.layers.route_wind.v_wind, 
	          i * gd.layers.route_wind._P->label_interval,
		  (i +1) * gd.layers.route_wind._P->label_interval,
		  &dir,&spd);

	       // scale to the desired units and round
	       spd *= units_scale_factor;
	       ispd = (int) (spd + 2.5);
	       ispd -= ispd % 5;

	       // normalize to 0 - 360
	       if(dir < 0.0) dir += 360.0;
	       
	       // Round to the nearest 5 degrees
	       idir = (int) (dir + 2.5);
	       idir -= idir % 5;

               sprintf(wind_label,"%03d/%.2d",idir,ispd);
	       font = choose_font(wind_label,100,gd.layers.route_wind._P->font_height,&xmid,&ymid);
	       XSetFont(gd.dpy,gd.legends.route_path_color->gc,font);

	       // Compute which Xpoint in the route line is over the center of the segment
	       index =(int) (((i + .5) * gd.layers.route_wind._P->label_interval/
			     gd.h_win.route.total_length) * num_route_points);

	        XDrawImageString(gd.dpy,xid,gd.legends.route_path_color->gc,
	              pixel[index].x  + (xmid/2) ,
	              pixel[index].y  + (ymid/2) ,
	              wind_label,
	              strlen(wind_label));
	    }
	 break;

	 case Croutes_P::EQUAL_DIVISIONS:
	    loop_count =  gd.layers.route_wind._P->num_route_labels;
	    for(i=0; i < loop_count; i++) {
	       // Compute the average for the interval
               ave_winds(gd.layers.route_wind.u_wind,gd.layers.route_wind.v_wind, 
	          (i/(double) loop_count) * gd.h_win.route.total_length,
		  ((i +1)/(double) loop_count) * gd.h_win.route.total_length,
		  &dir,&spd);

		  // scale to the desired units
                  spd *= units_scale_factor;
		  ispd = (int) (spd + 2.5);
		  ispd -= ispd % 5;

	       // normalize to 0 - 360
	       if(dir < 0.0) dir += 360.0;

	       // Round to the nearest 5 degrees
	       idir = (int) (dir + 2.5);
	       idir -= idir % 5;

               sprintf(wind_label,"%03d/%.2d",idir,ispd);
	       font = choose_font(wind_label,60,12,&xmid,&ymid);
	       XSetFont(gd.dpy,gd.legends.route_path_color->gc,font);

	       // Compute which Xpoint in the route line is over the center of the segment
	       index =(int) ((i + 0.5)/(double) gd.layers.route_wind._P->num_route_labels *  num_route_points);

	        XDrawImageString(gd.dpy,xid,gd.legends.route_path_color->gc,
	              pixel[index].x  + (xmid/2) ,
	              pixel[index].y  + (ymid/2) ,
	              wind_label,
	              strlen(wind_label));
	    }
	 break;

	}
      }

  }
  // Compute Peak turbulence if available  - Add it to hazard_label
  if(gd.layers.route_wind.turb != NULL ) {
	if(gd.layers.route_wind.turb->v_data_valid != 0 && 
	   gd.layers.route_wind.turb->v_data != NULL) {
	   double pk_turb = peak_turb(gd.layers.route_wind.turb,
	                              0.0, gd.h_win.route.total_length);

	   if(pk_turb >= gd.layers.route_wind._P->turb_low_thresh &&
	      pk_turb < gd.layers.route_wind._P->turb_mod_thresh) {
	     strncat(hazard_label,"! MOD CAT",128);
	   }

	   if(pk_turb >= gd.layers.route_wind._P->turb_mod_thresh &&
	      pk_turb < gd.layers.route_wind._P->turb_hi_thresh) {
	     strncat(hazard_label,"! HIGH CAT",128);
	   }

	   if(pk_turb >= gd.layers.route_wind._P->turb_hi_thresh) {
	     strncat(hazard_label,"! VHIGH CAT",128);
	   }
	}
  }

  // Compute Peak icing if available  - Add it to hazard_label
  if(gd.layers.route_wind.icing != NULL ) {
	if(gd.layers.route_wind.icing->v_data_valid != 0 && 
	   gd.layers.route_wind.icing->v_data != NULL) {
	   double pk_icing = peak_icing(gd.layers.route_wind.icing,
	                              0.0, gd.h_win.route.total_length);

	   if(pk_icing >= gd.layers.route_wind._P->icing_low_thresh &&
	      pk_icing < gd.layers.route_wind._P->icing_mod_thresh) {
	     strncat(hazard_label,"! LGT ICE",128);
	   }

	   if(pk_icing >= gd.layers.route_wind._P->icing_mod_thresh &&
	      pk_icing < gd.layers.route_wind._P->icing_hi_thresh) {
	     strncat(hazard_label,"! MOD ICE",128);
	   }

	   if(pk_icing >= gd.layers.route_wind._P->icing_hi_thresh) {
	     strncat(hazard_label,"! HVY ICE",128);
	   }
	}
  }

   // xv_set (gd.v_win_v_win_pu->hazard_msg,
   //       PANEL_LABEL_STRING,hazard_label,
   //       PANEL_ITEM_COLOR, CMS_CONTROL_COLORS + 0,
   //       NULL);
   // xv_set (gd.v_win_v_win_pu->hazard_msg,XV_X,xv_get(gd.v_win_v_win_pu->route_msg,XV_WIDTH)+5,NULL);
}

/*****************************************************************
 * PEAK_ICING:  Return the Peak Icing value along the route
 */

double peak_icing(met_record_t *mr_icing, double start_km, double end_km)
{
    int i;
    int start_index;
    int end_index;
    int plane;
    int n_points;
    double ice_peak;
    double height;
    double min_ht;
    double peak;
    unsigned short miss;
    unsigned short bad;
    unsigned short *ice_ptr;

    start_index = (int)  (start_km / mr_icing->v_fhdr.grid_dx);
    if(start_index < 0) start_index = 0;
    if(start_index >=  mr_icing->v_fhdr.nx) start_index = mr_icing->v_fhdr.nx-1;

    end_index =  (int) ((end_km / mr_icing->v_fhdr.grid_dx) + 0.5);
    if(end_index < 0) end_index = 0;
    if(end_index >=  mr_icing->v_fhdr.nx) end_index = mr_icing->v_fhdr.nx-1;
    n_points = end_index - start_index +1;

    // Pick up missing/bad data values
    miss = (unsigned short) mr_icing->v_fhdr.missing_data_value;
    bad = (unsigned short) mr_icing->v_fhdr.bad_data_value;

    // Calc the plane of interest (vertically)
    plane = 0;
    height = gd.h_win.cur_ht;
    // find the closest data set plane to the displays set height
    min_ht = 999999.0;
    for(i=0; i < mr_icing->v_fhdr.nz; i++) {
	 if(fabs(mr_icing->v_vhdr.level[i] - height) < min_ht ) {
	      min_ht = fabs(mr_icing->v_vhdr.level[i] - height);
	      plane = i;
	 }
    }

    // Move to the begining of the data row.
    ice_ptr = mr_icing->v_data + (mr_icing->v_fhdr.nx * plane) + start_index;

    // go through the array and seek out the maximum value
    ice_peak = 0.0;
    for(i=0; i < n_points; i++) {
       if(*ice_ptr != miss && *ice_ptr != bad ) {
	   peak = *ice_ptr * mr_icing->v_fhdr.scale + mr_icing->v_fhdr.bias;
	   if(peak > ice_peak) ice_peak = peak;
       }
       ice_ptr++;
    }
    return ice_peak;
}


/*****************************************************************
 * PEAK_TURB:  Return the Peak turbulence value along the route
 */

double peak_turb(met_record_t *mr_turb, double start_km, double end_km)
{
    int i;
    int start_index;
    int end_index;
    int plane;
    int n_points;
    double turb_peak;
    double height;
    double min_ht;
    double peak;
    unsigned short miss;
    unsigned short bad;
    unsigned short *turb_ptr;

    start_index = (int)  (start_km / mr_turb->v_fhdr.grid_dx);
    if(start_index < 0) start_index = 0;
    if(start_index >=  mr_turb->v_fhdr.nx) start_index = mr_turb->v_fhdr.nx-1;

    end_index =  (int) ((end_km / mr_turb->v_fhdr.grid_dx) + 0.5);
    if(end_index < 0) end_index = 0;
    if(end_index >=  mr_turb->v_fhdr.nx) end_index = mr_turb->v_fhdr.nx-1;
    n_points = end_index - start_index +1;

    // Pick up missing/bad data values
    miss = (unsigned short) mr_turb->v_fhdr.missing_data_value;
    bad = (unsigned short) mr_turb->v_fhdr.bad_data_value;

    // Calc the plane of interest (vertically)
    plane = 0;
    height = gd.h_win.cur_ht;
    // find the closest data set plane to the displays set height
    min_ht = 999999.0;
    for(i=0; i < mr_turb->v_fhdr.nz; i++) {
	 if(fabs(mr_turb->v_vhdr.level[i] - height) < min_ht ) {
	      min_ht = fabs(mr_turb->v_vhdr.level[i] - height);
	      plane = i;
	 }
    }

    // Move to the begining of the data row.
    turb_ptr = mr_turb->v_data + (mr_turb->v_fhdr.nx * plane) + start_index;
    //turb_ptr = mr_turb->v_data;
    //n_points =  mr_turb->v_fhdr.nz * mr_turb->v_fhdr.nx;

    // go through the array and seek out the maximum value
    turb_peak = 0.0;
    for(i=0; i < n_points; i++) {
       if(*turb_ptr != miss && *turb_ptr != bad ) {
	   peak = *turb_ptr * mr_turb->v_fhdr.scale + mr_turb->v_fhdr.bias;
	   if(peak > turb_peak) turb_peak = peak;
       }
       turb_ptr++;
    }
    return turb_peak;
}

/*****************************************************************
 * AVE_WINDS:  Average the winds along the cross section.
 */

void  ave_winds(met_record_t *mr_u, met_record_t *mr_v,
               double start_km, double end_km, 
	       double *ave_deg, double *ave_spd)  // Returned values
{
    int i;
    int start_index;
    int end_index;
    int u_plane,v_plane;
    int num_points,n_points;
    double u_sum,v_sum;
    double angle;
    double height;
    double min_ht;;
    unsigned short miss_u;
    unsigned short miss_v;
    unsigned short bad_u;
    unsigned short bad_v;
    unsigned short *u_ptr;
    unsigned short *v_ptr;

    start_index = (int)  (start_km / mr_u->v_fhdr.grid_dx);
    if(start_index < 0) start_index = 0;
    if(start_index >=  mr_u->v_fhdr.nx) start_index = mr_u->v_fhdr.nx-1;

    end_index =  (int) ((end_km / mr_u->v_fhdr.grid_dx) + 0.5);
    if(end_index < 0) end_index = 0;
    if(end_index >=  mr_u->v_fhdr.nx) end_index = mr_u->v_fhdr.nx-1;
    n_points = end_index - start_index +1;

    // Pick up missing/bad data values
    miss_u = (unsigned short) mr_u->v_fhdr.missing_data_value;
    miss_v = (unsigned short) mr_v->v_fhdr.missing_data_value;
    bad_u = (unsigned short) mr_u->v_fhdr.bad_data_value;
    bad_v = (unsigned short) mr_v->v_fhdr.bad_data_value;

    // Calc the plane of interest (vertically)
    u_plane = 0;
    v_plane = 0;
    height = gd.h_win.cur_ht;

    // find the closest data set plane to the displays set height
    min_ht = 999999.0;
    for(i=0; i < mr_u->v_fhdr.nz; i++) {
	 if(fabs(mr_u->v_vhdr.level[i] - height) < min_ht ) {
	      min_ht = fabs(mr_u->v_vhdr.level[i] - height);
	      u_plane = i;
	 }
    }

    min_ht = 999999.0;
    for(i=0; i < mr_v->v_fhdr.nz; i++) {
	 if( fabs(mr_v->v_vhdr.level[i] - height) < min_ht ) {
	      min_ht = fabs(mr_v->v_vhdr.level[i] - height);
	      v_plane = i;
	 }
    }

    // Move to the begining of the data row.
    u_ptr = mr_u->v_data + (mr_u->v_fhdr.nx * u_plane) + start_index;
    v_ptr = mr_v->v_data + (mr_v->v_fhdr.nx * v_plane) + start_index;

    u_sum = 0.0;
    v_sum = 0.0;
    num_points = 0;
    for(i=0; i < n_points; i++) {
       if((*u_ptr == miss_u) || (*v_ptr == miss_v) ||
	  (*u_ptr == bad_u) || (*v_ptr == bad_v))  {
	  // do not include it in the sum
       } else {
           u_sum +=  *u_ptr * mr_u->v_fhdr.scale + mr_u->v_fhdr.bias;
           v_sum +=  *v_ptr * mr_v->v_fhdr.scale + mr_v->v_fhdr.bias;
	   num_points++;  // count the number of points included in the sum
       }
       u_ptr++;
       v_ptr++;
    }
    // Compute the average
    u_sum /= (double) num_points;
    v_sum /= (double) num_points;

    angle = atan2( -v_sum, -u_sum) * DEG_PER_RAD; 
    angle = 90.0 - angle; /* convert to North = 0 */ 

    *ave_deg = angle;
    *ave_spd =  sqrt((u_sum * u_sum) + (v_sum * v_sum));
}

/*****************************************************************
 * ROTATE_ROUTE:  Apply a rotation of the route points arount the
 *  first way point.
 * 
 */

void rotate_route(double rotation)
{
   rotate_points(rotation,
				 gd.h_win.route.x_world[0],
				 gd.h_win.route.y_world[0],
				 gd.h_win.route.x_world,
				 gd.h_win.route.y_world,
				 gd.h_win.route.num_segments+1);
   
   setup_route_area(0);

   // Copy the route definition into the space reserved for the Custom route
   if(gd.layers.route_wind.num_predef_routes > 0 ) {
		 memcpy(gd.layers.route_wind.route+gd.layers.route_wind.num_predef_routes,
		  &gd.h_win.route,sizeof(route_track_t));
   }
}

#define MAX_WP_POINTS 128
#define MAX_XSECT_POINTS 256
/*****************************************************************
 * CALC_ROUTE_POINTS:  Compute the pixel coordinates of points along
 * a great circle route defined by a set of route way points.
 * Broken into segments. - Returns number of total points to render
 */

int calc_route_points(XPoint **pixel, route_seg_t **rseg, int *num_line_segs)
{
  int i;
  int x1,y1;
  int num_way_points, num_xsect_points, num_segs, cur_segment;
  double total_dist;  
  double cur_dist;
  double seg_dist;
  double x_native,y_native, delta_km;
  double lat[MAX_WP_POINTS]; // Lat lons of  way points
  double lon[MAX_WP_POINTS];
  double seg_len[MAX_WP_POINTS]; // Lengths and direction of each segment
  double seg_theta[MAX_WP_POINTS];   
  double x_lat[MAX_XSECT_POINTS];  // Array of points - World Coords
  double x_lon[MAX_XSECT_POINTS];

  route_seg_t seg[MAX_XSECT_POINTS];

  static XPoint xpt[MAX_XSECT_POINTS]; // Points to sample - pixel coords 

  total_dist = 0.0;
  // Put way points into world coordinates and compute length and angles
  // of each segment.
  num_way_points = gd.h_win.route.num_segments + 1;

  if(num_way_points > MAX_WP_POINTS) {
	  num_way_points = MAX_WP_POINTS;
	  if (gd.debug) {
	     fprintf(stderr, "Whoa! Found more than %d  way points\n",
		     MAX_WP_POINTS);
	     fprintf(stderr, "Only using the first %d s\n",
		     MAX_WP_POINTS);
	  }
  }

  for(i=0; i < num_way_points; i++) {

	 if(gd.display_projection == Mdvx::PROJ_LATLON) {
		if(gd.h_win.route.x_world[i] < gd.h_win.min_x) gd.h_win.route.x_world[i] += 360.0;
		if(gd.h_win.route.x_world[i] > gd.h_win.max_x) gd.h_win.route.x_world[i] -= 360.0;
		if(gd.h_win.route.y_world[i] < gd.h_win.min_y) gd.h_win.route.y_world[i] += 360.0;
		if(gd.h_win.route.y_world[i] > gd.h_win.max_y) gd.h_win.route.y_world[i] -= 360.0;
	}
	    
     // compute pixel coords for each segment end point
     disp_proj_to_pixel(&gd.h_win.margin,gd.h_win.route.x_world[i],
			                 gd.h_win.route.y_world[i],
					 &gd.h_win.route.x_pos[i],
					 &gd.h_win.route.y_pos[i]);


    gd.proj.xy2latlon( gd.h_win.route.x_world[i], gd.h_win.route.y_world[i], lat[i],lon[i]);
    if(i > 0) {
	PJGLatLon2RTheta(lat[i-1],lon[i-1],
		 lat[i],lon[i],
	         &seg_len[i-1],&seg_theta[i-1]);

        total_dist += seg_len[i-1];

         if (gd.debug) 
          fprintf(stderr,"Seg: %d; Len: %g, T: %g Lat,lon:  %.4f,%.4f\n",
	    i,seg_len[i-1],seg_theta[i-1], lat[i-1],lon[i-1]);

      }
  }

  if (gd.debug)
      fprintf(stderr,"Total Distance of Route: %g\n",total_dist);

  // Compute the sampling interval along the route
  delta_km = total_dist / (MAX_XSECT_POINTS / 2) ;

  // Compute the lat,lons and grid indices along the route
  cur_segment = 0;
  cur_dist = 0.0;  // Position along the segment
  seg_dist = 0.0;
  num_xsect_points = 0;

  num_segs = 0;
  seg[num_segs].pixel = xpt;
  seg[num_segs].num_points = 0;

  while (cur_dist <= total_dist &&
	  cur_segment < (num_way_points -1) &&
	  num_xsect_points < MAX_XSECT_POINTS-1) {
      
    if(seg_dist == 0.0) {
      x_lat[num_xsect_points] = lat[cur_segment];
      x_lon[num_xsect_points] = lon[cur_segment];
    } else {
     PJGLatLonPlusRTheta(lat[cur_segment],lon[cur_segment],
			  seg_dist,seg_theta[cur_segment],
			  &x_lat[num_xsect_points],&x_lon[num_xsect_points]);
    }

    gd.proj.latlon2xy(x_lat[num_xsect_points],x_lon[num_xsect_points], x_native,y_native);

    if (gd.debug2) {
          fprintf(stderr,"Pt: %d: Lat,lon:  %.4f,%.4f\n", num_xsect_points,x_lat[num_xsect_points],x_lon[num_xsect_points]);
	}

	if(gd.display_projection == Mdvx::PROJ_LATLON) {
		if(x_native < gd.h_win.min_x) x_native += 360.0;
		if(x_native > gd.h_win.max_x) x_native -= 360.0;
		if(y_native < gd.h_win.min_y) y_native += 360.0;
		if(y_native > gd.h_win.max_y) y_native -= 360.0;
	}

    disp_proj_to_pixel(&gd.h_win.margin,x_native,y_native,&x1,&y1);

    if (gd.debug2) {
          fprintf(stderr,"Pt: %d: Lat,lon:  %.4f,%.4f, X,Y: %d, %d\n",
		    num_xsect_points,x_native,y_native,x1,y1);
	}

    xpt[num_xsect_points].x = x1;
    xpt[num_xsect_points].y = y1;

	if( num_xsect_points > 0) {

		// If line segment appears to span more then 75% of the image - drop this point and start a new segment.
		if((double) (abs(xpt[num_xsect_points -1].x - x1)) > (gd.h_win.img_dim.width * .75)) {

			num_segs++;

			seg[num_segs].pixel = &xpt[num_xsect_points];
			seg[num_segs].num_points = 0;
		}
	}

	seg[num_segs].num_points++; // Points in this sub-segment
    num_xsect_points++;        // Points sampled along the total route.

     cur_dist += delta_km; // move along the segment by delta km
     seg_dist += delta_km;

     // Check to see if we're past the segment
     if(seg_len[cur_segment] <= seg_dist) {

	 // Starting distance along the segment is what's left over
	 // after sampling the last segment.
	 seg_dist = - (seg_len[cur_segment] - seg_dist);

	 // Move to the next segment
	 cur_segment++;

	 // Make sure starting point in segment is drawn
	 disp_proj_to_pixel(&gd.h_win.margin,gd.h_win.route.x_world[cur_segment],
			   gd.h_win.route.y_world[cur_segment],&x1,&y1);

	 xpt[num_xsect_points].x = x1;
	 xpt[num_xsect_points].y = y1;

	  // If line segment appears to span more then 75% of the image - drop this point and start a new segment.
	  if((double) (abs(xpt[num_xsect_points -1].x - x1)) > (gd.h_win.img_dim.width * .75)) {

	   seg[num_segs].num_points++; // Points in this sub-segment
		num_segs++;

		seg[num_segs].pixel = &xpt[num_xsect_points];
		seg[num_segs].num_points = 0;
	  }

	   seg[num_segs].num_points++; // Points in this sub-segment
	   num_xsect_points++;

     }
  }  // End while(cur_dist <= total_dist ...)

  if (gd.debug)
      fprintf(stderr, "Delta km: %.3f Points sampled along route: %d\n",
	      delta_km,num_xsect_points);

  *rseg = seg;
  *num_line_segs = num_segs + 1;
  return num_xsect_points;  // The total number of route segments to render
}
