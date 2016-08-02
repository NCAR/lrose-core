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
/*********************************************************************
 * MDV_SERVER_GET_V_PLANE
 * F. Hage - NCAR/RAP
 */

#define MDV_SERVER_GET_V_PLANE
 
#include "mdv_server.h" 

int get_v_grid_array(cdata_ieee_comm_t *com, 
                              cdata_ieee_comm2_t * com2,
			      int **x_grid,    /* RETURNED VALUES */
			      int **y_grid);

void get_v_grid_zlimits(cdata_ieee_comm_t *com, 
                              cdata_ieee_comm2_t * com2,
			      int *rz1, /* RETURNED VALUES */
			      int *rz2);

/*****************************************************************
 * GET_V_PLANE:  Obtain a Vertical plane of data from the correct file
 *        in the area designated. Fills in grid limits in reply
 *        structure 
 */

int get_v_plane(cdata_ieee_comm_t *com, cdata_ieee_comm2_t * com2,
		       ui08 **mdv_plane)
{
  int    i,j;    /* Simple counters */
  int    ny, nz, xadd;
  int	 *x_grid, *y_grid;
  int    num_points, array_size;
  int    npoints_plane;
  ui08   *ptr, *ptr2, *mdv_pl, *mdv_vol;  /* pointers for data manipulation */
  ui08   missing_val;
  ui08   bad_data;
  cdata_ieee_reply_t *reply = &gd.current_ieee_reply;

  /* Initialize the returned plane in case there are any errors */
  *mdv_plane = NULL;

  /* get the data volume */

  MDV_field_header_t locFhdr = *gd.cur_f_head->fld_hdr;
  int vol_len;

  if ((mdv_vol = (ui08 *) MDV_read_field_volume(gd.data_file,
						&locFhdr,
						MDV_INT8,
						MDV_COMPRESSION_NONE,
						MDV_SCALING_ROUNDED, 0.0, 0.0,
						&vol_len)) == NULL) {
    return 0;
  }

  /* Set grid limits */
  get_v_grid_zlimits(com, com2, &reply->z1, &reply->z2);

  ny = ABSDIFF(reply->y1,reply->y2);
  nz = (reply->z2 - reply->z1) +1;

  // Get the grid points along the segments at dz interval (km) increments
  num_points = get_v_grid_array(com, com2, &x_grid, &y_grid);

  reply->nx = num_points;
  reply->ny = nz;
  reply->nz = 1;
  reply->orient = V_PLANE;

  array_size = reply->nx * reply->ny;

  // Size of one row in data array
  xadd = gd.cur_f_head->fld_hdr->nx;

  bad_data = (ui08) locFhdr.bad_data_value;
  missing_val = (ui08) locFhdr.missing_data_value;
 
  *mdv_plane = (ui08 *)  malloc(array_size * sizeof(ui08));
  ptr = *mdv_plane;

  /* do each plane */
  npoints_plane = gd.cur_f_head->fld_hdr->nx * gd.cur_f_head->fld_hdr->ny;

  for (i = reply->z1; i <= reply->z2; i++) {
    
     mdv_pl = mdv_vol + i * npoints_plane;

     for(j = 0; j < num_points; j++) {
	  if(x_grid[j] < 0 || y_grid[j] < 0) {
	      *ptr++ = bad_data;
	  } else {
             /* move to correct row and column */
	     ptr2 = mdv_pl + (xadd * y_grid[j]) + x_grid[j];
	     // Map missing data to bad for cdata protocol 
	     if(*ptr2 == missing_val) {
	         *ptr++ = bad_data;
	     } else {
	         *ptr++ = *ptr2;
	     }
	  }
     }
  }

  reply->bad_data_val = (int) locFhdr.bad_data_value;
  reply->scale = locFhdr.scale;
  reply->bias = locFhdr.bias;

  ufree(mdv_vol);
  return(array_size);
}
 
#define MAX_WP_POINTS 128
#define MAX_XSECT_POINTS 8192
/*****************************************************************
 * GET_V_GRID_array:  Compute the Grid coordinates of points along
 * a great circle route defined by a set of route way points.
 * These way points define N-1 segments. This routine maintains
 * compatibility with the V1 protocol by assuming there are only
 * two points (One segment), and it extracts the values out of the
 * first com struct.
 */

int get_v_grid_array(cdata_ieee_comm_t *com,        /* the request */
                              cdata_ieee_comm2_t * com2,
			      int **x_grid,    /* RETURNED VALUES */
			      int **y_grid)
{
  int i;
  int nx, ny;
  int num_way_points, num_xsect_points, cur_segment;
  double total_dist;  
  double cur_dist;
  double seg_dist;
  double x_native,y_native, delta_km;
  double lat[MAX_WP_POINTS]; // Lat lons of  way points
  double lon[MAX_WP_POINTS];
  double seg_len[MAX_WP_POINTS]; // Lengths and direction of each segment
  double seg_theta[MAX_WP_POINTS];   
  double x_lat[MAX_XSECT_POINTS];  // Array of points ample - World Coords
  double x_lon[MAX_XSECT_POINTS];
  way_point_t *wp; // Pointer for extracting  wayb points

  static int x_gridx[MAX_XSECT_POINTS]; // Points to sample - Grid indices
  static int x_gridy[MAX_XSECT_POINTS];

  total_dist = 0.0;
  // Put way points into world coordinates and compute length and angles
  // of each segment.

  if(com2 == NULL || com2->num_way_points == 0) { // Support old protocol
      num_way_points = 2;
      if(gd.cur_f_head->fld_hdr->proj_type == MDV_PROJ_FLAT) {

	  // For "Cartesian" mode operation the coords are in km
	  // from the data's origin. - Convert to lat/ Lon
	  PJGflat_init(gd.cur_f_head->fld_hdr->proj_origin_lat,
		       gd.cur_f_head->fld_hdr->proj_origin_lon,
		       gd.cur_f_head->fld_hdr->proj_rotation);
	 PJGflat_xy2latlon(com->min_x,com->min_y,&lat[0],& lon[0]);
	 PJGflat_xy2latlon(com->max_x,com->max_y,&lat[1],& lon[1]);

	 // Now record the distance and angle of each segment
	 PJGLatLon2RTheta(lat[0],lon[0],
			  lat[1],lon[1],
			  &total_dist,&seg_theta[0]);
	 seg_len[0] = total_dist;
      } else {
	  // In "lat lon" mode the coords are in degrees.
          lat[0] = com->min_y;
          lat[1] = com->max_y;
          lon[0] = com->min_x;
          lon[1] = com->max_x;

	 // Now record the distance and angle of each segment
	 PJGLatLon2RTheta(com->min_y,com->min_x,
			  com->max_y,com->max_x,
			  &seg_len[0],&seg_theta[0]);
	 total_dist = seg_len[0];
      }
  } else {  // Use Version 2 Protocol

      num_way_points = com2->num_way_points;
      if(num_way_points > MAX_WP_POINTS) {
	  num_way_points = MAX_WP_POINTS;
	  if (!gd.daemonize_flag) {
	     fprintf(stderr, "Whoa! Found more than %d  way points\n",
		     MAX_WP_POINTS);
	     fprintf(stderr, "Only using the first %d s\n",
		     MAX_WP_POINTS);
	  }
      }

      wp = (way_point_t *) (com2->url + com2->url_len);

      for(i=0; i < num_way_points; i++, wp++) {
	    lat[i] = wp->lat;
	    lon[i] = wp->lon;
	    if(i > 0) {
		PJGLatLon2RTheta(lat[i-1],lon[i-1],
				 lat[i],lon[i],
			         &seg_len[i-1],&seg_theta[i-1]);

	        total_dist += seg_len[i-1];

	         if (!gd.daemonize_flag) 
	          fprintf(stderr,"Seg: %d; Len: %g, T: %g Lat,lon:  %.4f,%.4f\n",
		    i,seg_len[i-1],seg_theta[i-1],
		    lat[i-1],lon[i-1]);

	    }
      }
  }

  if (!gd.daemonize_flag)
      fprintf(stderr,"Total Distance of Route: %g\n",total_dist);

  // Compute the sampling interval along the route
  if(gd.cur_f_head->fld_hdr->proj_type == MDV_PROJ_LATLON) {
    delta_km = gd.cur_f_head->fld_hdr->grid_dx * KM_PER_DEG_AT_EQ;
  } else {
    delta_km = gd.cur_f_head->fld_hdr->grid_dx;
  }


  // Compute the lat,lons and grid indices along the route
  cur_segment = 0;
  cur_dist = 0.0;  // Position along the segment
  seg_dist = 0.0;
  num_xsect_points = 0;

  switch(gd.cur_f_head->fld_hdr->proj_type) {
    case MDV_PROJ_FLAT:
      PJGflat_init(gd.cur_f_head->fld_hdr->proj_origin_lat,
	       gd.cur_f_head->fld_hdr->proj_origin_lon,
	       gd.cur_f_head->fld_hdr->proj_rotation);

    break;
  }

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
    switch(gd.cur_f_head->fld_hdr->proj_type) {
        case MDV_PROJ_FLAT:
	     PJGflat_latlon2xy(x_lat[num_xsect_points], x_lon[num_xsect_points],
			       &x_native, &y_native);
             native_units_to_grid(x_native, y_native ,
                               &x_gridx[num_xsect_points], &x_gridy[num_xsect_points]);
	break;

        case MDV_PROJ_LATLON:
             native_units_to_grid(x_lon[num_xsect_points],
				x_lat[num_xsect_points] ,
                               &x_gridx[num_xsect_points],
                               &x_gridy[num_xsect_points]);
        break;
     }

     num_xsect_points++;

     cur_dist += delta_km; // move along the segment by delta km
     seg_dist += delta_km;

     // Check to see if we're past the segment
     if(seg_len[cur_segment] <= seg_dist) {

	 // Starting distance along the segment is what's left over
	 // after sampling the last segment.
	 seg_dist = - (seg_len[cur_segment] - seg_dist);

	 // Move to the next segment
	 cur_segment++;
     }
  }  // End while(cur_dist <= total_dist ...)

  // Indicate points outside of data grid with a -1
  nx = gd.cur_f_head->fld_hdr->nx;
  ny = gd.cur_f_head->fld_hdr->ny;
  for(i=0; i < num_xsect_points; i++) {
      if(x_gridx[i] >= nx) x_gridx[i] = -1;
      if(x_gridy[i] >= ny) x_gridy[i] = -1;
  }

  if (!gd.daemonize_flag)
      fprintf(stderr, "Delta km: %.3f Points sampled along route: %d\n",
	      delta_km,num_xsect_points);

  *x_grid = x_gridx;
  *y_grid = x_gridy;
  return num_xsect_points;  // The total number of sample points
}

/*****************************************************************
 * GET_V_GRID_ZLIMITS: Return the grid's bottom and top indices
 */

void get_v_grid_zlimits(cdata_ieee_comm_t *com,        /* the request */
                              cdata_ieee_comm2_t * com2,
			      int *rz1,
			      int *rz2)
{
  int i;
  int z1,z2;
  int flag;

  if (!gd.daemonize_flag) {
    fprintf(stderr, "min_z, max_z, %g, %g,\n", com->min_z, com->max_z);
  }


  /* Find grid height limits  */

  /* Choose the plane with the alt < than the limit */
  if (gd.cur_m_head.vlevel_included && gd.cur_f_head->vlv_hdr != NULL) {
    z1 = 0;
    for (i = 0; i < gd.num_planes; i++) {
      if (gd.cur_f_head->vlv_hdr->vlevel_params[i] <= com->min_z)
        z1 = i;
	if(gd.cur_f_head->vlv_hdr->vlevel_params[i] > com->min_z)
	    i = gd.num_planes; // Break when over
    }
  } else { // Compute the index.
      if(gd.cur_f_head->fld_hdr->grid_dz != 0.0 ) {
        z1 = (int) ((com->min_z - gd.cur_f_head->fld_hdr->grid_minz) / 
	        gd.cur_f_head->fld_hdr->grid_dz); 

	if(z1 < 0) z1 = 0;
	if(z1 > gd.num_planes - 1) z1 = gd.num_planes - 1;
      } else {
         z1 = 0;
      }
  }
         
  /* Choose the plane with the max alt >= than the limit */
  if (gd.cur_m_head.vlevel_included && gd.cur_f_head->vlv_hdr != NULL) {
    z2 = gd.num_planes - 1;
    for (i = gd.num_planes - 1; i >= 0; i--) {
      if (gd.cur_f_head->vlv_hdr->vlevel_params[i] >= com->max_z)
        z2 = i;
        if(gd.cur_f_head->vlv_hdr->vlevel_params[i] < com->max_z)
	    i = -1; // Break when over
    }
  } else { // Has a fixed DZ - Compute the index - rounding up.
      if(gd.cur_f_head->fld_hdr->grid_dz != 0.0 ) {
        z2 = (int) (((com->max_z - gd.cur_f_head->fld_hdr->grid_minz) / 
	        gd.cur_f_head->fld_hdr->grid_dz) + 0.5); 

	if(z2 < 0) z2 = 0;
	if(z2 > gd.num_planes - 1) z2 = gd.num_planes - 1;
      } else {
         z2 = gd.num_planes - 1;
      }
  }

  *rz1 = CLIP(z1, 0, (gd.cur_f_head->fld_hdr->nz - 1), flag);
  *rz2 = CLIP(z2, 0, (gd.cur_f_head->fld_hdr->nz - 1), flag);

  return;
}
