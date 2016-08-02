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
 * get_grid_data.c
 *
 * Module for getting data
 * 
 * Frank Hage, RAP, NCAR, Boulder, CO, USA
 *
 * Updated by Mike Dixon, Dec 1991
 */

#include "MDV_server.hh"
#include <toolsa/compress.h>

using namespace std;

static ui08 *get_data_plane(MdvRead &mdv, int field, si32 plane);

static si32 get_max_xy_plane(cdata_comm_t *com,
			     cdata_reply_t *reply,
			     cdata_info_t *info,
			     MdvRead &mdv,
			     ui08 **rd_plane);

static si32 get_xy_plane(cdata_comm_t *com,
			 cdata_reply_t *reply,
			 cdata_info_t *info,
			 MdvRead &mdv,
			 ui08 **rd_plane);

static si32 get_v_plane(cdata_comm_t *com,
			cdata_reply_t *reply,
			cdata_info_t *info,
			MdvRead &mdv,
			ui08 **vert_plane);

static void get_v_grid_limits(cdata_comm_t *com,
			      cdata_info_t *info,
			      MdvRead &mdv,
			      si32 *rx1,
			      si32 *rx2,
			      si32 *ry1,
			      si32 *ry2,
			      si32 *rz1,
			      si32 *rz2);

static void load_reply(cdata_comm_t *com,
		       cdata_reply_t *reply,
		       MdvRead &mdv);

static int xy_km_to_grid(cdata_comm_t *com,
			 double km_x,
			 double km_y,
			 si32 *grid_x,
			 si32 *grid_y,
			 MdvRead &mdv);

static int z_km_to_grid(cdata_comm_t *com,
			double km_z,
			MdvRead &mdv);

extern time_t time(time_t *);

long atol(const char *);

/*
 * clipping with flag setting -  OR's flag with 1 if
 * clipped and "returns" a value >= l && <= u
 */
  
#define FCLIP(x,l,u,flag) (x < l) ? flag|=1,l : ((x > u)? flag|=1,u : x)

/*
 * clipping with no flag
 */

#define CLIP(x,l,u) (x < l) ? l : ((x > u)? u : x)

/*
 * absolute difference
 */

#define ABSDIFF(a,b) (((a) < (b))? (b)-(a): (a)-(b))

/*****************************************************************
 * GET_GRID_DATA: Gather data from the correct file
 */

ui08 *get_grid_data(cdata_comm_t *com,
		    cdata_reply_t *reply,
		    cdata_info_t *info,
		    MdvRead &mdv)
     
{
  
  si32 num_points;
  ui08 *rd_plane;
  
  switch(com->second_com) {
    
  case GET_XY_PLANE:
    num_points = get_xy_plane(com, reply, info, mdv, &rd_plane);
    break;
    
  case GET_V_PLANE: /* Arbitrary Vertical Plane */
    num_points = get_v_plane(com, reply, info, mdv, &rd_plane);
    break;
    
  case GET_MAX_XY_PLANE: /* MAX VALUE Projection */
    num_points = get_max_xy_plane(com, reply, info, mdv, &rd_plane);
    break;
    
  default:
    num_points = 0;
    rd_plane = NULL;
    break;
    
  }
  
  if(Glob->params.debug) {
    fprintf(stderr,"Data length: %d ",num_points);
    if (Glob->params.compress_for_transfer)
      fprintf(stderr, "(compressed)\n");
    else
      fprintf(stderr, "\n");
  }
  
  if(num_points == 0 || rd_plane == NULL) { /* No data available */
    reply->status |= NO_DATA_AVAILABLE;
    return NULL;
  }
  
  reply->n_points = num_points;
  
  return(rd_plane);
  
}

/*****************************************************************
 * GET_DATA_PLANE
 *
 */

static ui08 *get_data_plane(MdvRead &mdv, int field, si32 plane)

{
  
  return ((ui08 *) mdv.getField(field).getVol2D()[plane]);
  
}

/*****************************************************************
 * GET_XY_PLANE
 *
 * Obtain an XY plane of data in the area designated.
 * Fills in grid limits in reply structure 
 */

static si32 get_xy_plane(cdata_comm_t *com,
			 cdata_reply_t *reply,
			 cdata_info_t *info,
			 MdvRead &mdv,
			 ui08 **rd_plane)
     
{
  
  ui08 *data, *data_ptr; /* pointers for data manipulation */
  ui08 *plane_data, *plane_ptr, *comp_data;
  
  si32 i;
  si32 nbytes_data_line;
  si32 nbytes_plane_line;
  si32 nbytes_compressed;
  si32 num_points;
  si32 offset;
  si32 nlines;
  si32 plane_num;
  
  double max_z, min_z, requested_z;

  if (Glob->params.debug)
    fprintf(stderr, "%s:get_xy_plane\n", Glob->prog_name);

  /*
   * check that the requested data order is valid
   */

  if (com->min_x > com->max_x)
  {
    if (Glob->params.debug)
      fprintf(stderr, "Error in X values in request.\n");
    return (0);
  }
  
  if (com->min_y > com->max_y)
  {
    if (Glob->params.debug)
      fprintf(stderr, "Error in Y values in request.\n");
    return (0);
  }
  
  if (com->min_z > com->max_z)
  {
    if (Glob->params.debug)
      fprintf(stderr, "Error in Z values in request.\n");
    return (0);
  }
  
  /*
   * get the relevant plane
   */
  
  min_z = (double) com->min_z / (double) com->divisor;
  max_z = (double) com->max_z / (double) com->divisor;
  requested_z = (min_z + max_z) / 2.0;

  // plane_num = z_km_to_grid(com, requested_z, mdv);

  plane_num = mdv.getPlaneNum(com->data_field);

  reply->z1 = plane_num;
  reply->z2 = plane_num;
  
  if (Glob->params.debug)
    fprintf(stderr,
	    "Returning plane %d (requested min_z = %d, max_z = %d).\n",
	    plane_num, com->min_z, com->max_z);
  
  /*
   * Find Horizontal grid limits
   */
  
  xy_km_to_grid(com,
		(double) com->min_x / (double) com->divisor,
		(double) com->min_y / (double) com->divisor,
		&reply->x1, &reply->y1, mdv);
  
  xy_km_to_grid(com,
		(double) com->max_x / (double) com->divisor,
		(double) com->max_y / (double) com->divisor,
		&reply->x2, &reply->y2, mdv);
  
  reply->x1 = CLIP(reply->x1, 0, (info->nx -1));
  reply->x2 = CLIP(reply->x2, 0, (info->nx -1));
  reply->y1 = CLIP(reply->y1, 0, (info->ny -1));
  reply->y2 = CLIP(reply->y2, 0, (info->ny -1));
  
  reply->nx = reply->x2 - reply->x1 + 1;
  reply->ny = reply->y2 - reply->y1 + 1;
  reply->nz = 1;
  
  reply->dx = info->dx;
  reply->dy = info->dy;
  reply->dz = info->dz;
  reply->data_type = CDATA_CHAR;
  reply->orient = XY_PLANE;
  
  load_reply(com, reply, mdv);
  
  if(Glob->params.debug) {
    fprintf(stderr, "Reply Time_cent = %ld, %s\n",
	    (long) reply->time_cent, utimstr(reply->time_cent));
  }
  
  nbytes_plane_line = reply->x2 - reply->x1 + 1;
  nlines = reply->y2 - reply->y1 + 1;
  num_points = nbytes_plane_line * nlines;
  
  if(num_points <= 0) return (0);
  
  /*
   * Get a full plane of data
   */
  
  data = (ui08 *) mdv.getField(com->data_field).getPlane1D();
  
  /*
   * Get area for sub array
   */
  
  plane_data = (ui08 *) umalloc((ui32) (num_points * sizeof(ui08)));
  if(plane_data == NULL) return (0);
  
  /*
   * compute first offset into array
   */
  
  offset = (reply->y1 * info->nx) + reply->x1;
  
  /*
   * set pointer to starting point
   */
  
  data_ptr = data + offset;
  
  /*
   * Loop through array setting the sub-plane
   */
  
  nbytes_data_line = info->nx;
  plane_ptr = plane_data;
  
  for (i = 0; i < reply->ny; i++) {
    
    memcpy ((void *) plane_ptr,
            (void *) data_ptr,
            (size_t) nbytes_plane_line);
    
    data_ptr += nbytes_data_line;
    plane_ptr += nbytes_plane_line;
    
  } /* i */

  /*
   * compress as required
   */

  if (Glob->params.compress_for_transfer) {

    comp_data = uRLEncode8((ui08 *) plane_data,
			   (ui32) num_points,
			   (ui32) 255,
			   (ui32 *) &nbytes_compressed);

    ufree ((char *) plane_data);
    *rd_plane = comp_data;
    return (nbytes_compressed);

  } else {

    *rd_plane = plane_data;
    return (num_points);

  } /* if (Glob->compress_data) */
  
}

/*****************************************************************
 * GET_MAX_XY_PLANE
 *
 * Obtain an XY plane of data in the area designated, filled with
 * the maximum vals between the given height limits
 *
 * Fills in grid limits in reply structure 
 */

static si32 get_max_xy_plane(cdata_comm_t *com,
			     cdata_reply_t *reply,
			     cdata_info_t *info,
			     MdvRead &mdv,
			     ui08 **rd_plane)
     
{
  
  ui08 *data, *data_ptr; /* pointers for data manipulation */
  ui08 *plane_ptr;
  ui08 *comp_data;
  
  si32 i;
  si32 nbytes_data_line;
  si32 nbytes_plane_line;
  si32 num_points;
  si32 offset;
  si32 nlines;
  si32 nbytes_compressed;
  
  if (Glob->params.debug)
    fprintf(stderr, "%s:get_max_xy_plane\n", Glob->prog_name);

  /*
   * check that the requested data order is valid
   */

  if (com->min_x > com->max_x)
    return (0);

  if (com->min_y > com->max_y)
    return (0);

  /*
   * set vertical limits - entire volume
   */
  
  reply->z1 = 0;
  reply->z2 = info->nz - 1;
  
  /*
   * Find Horizontal grid limits
   */
  
  xy_km_to_grid(com,
		(double) com->min_x / (double) com->divisor,
		(double) com->min_y / (double) com->divisor,
		&reply->x1, &reply->y1, mdv);
  
  xy_km_to_grid(com,
		(double) com->max_x / (double) com->divisor,
		(double) com->max_y / (double) com->divisor,
		&reply->x2, &reply->y2, mdv);
  
  reply->x1 = CLIP(reply->x1, 0, (info->nx -1));
  reply->x2 = CLIP(reply->x2, 0, (info->nx -1));
  reply->y1 = CLIP(reply->y1, 0, (info->ny -1));
  reply->y2 = CLIP(reply->y2, 0, (info->ny -1));
  
  reply->nx = reply->x2 - reply->x1 + 1;
  reply->ny = reply->y2 - reply->y1 + 1;
  reply->nz = 1;
  
  reply->dx = info->dx;
  reply->dy = info->dy;
  reply->dz = info->dz;
  reply->data_type = CDATA_CHAR;
  reply->orient = XY_PLANE;
  
  load_reply(com, reply, mdv);
  
  if(Glob->params.debug) {
    fprintf(stderr, "Reply Time_cent = %ld: %s\n",
	    (long) reply->time_cent, utimstr(reply->time_cent));
  }
  
  nbytes_plane_line = reply->nx;
  nlines = reply->ny;
  num_points = nbytes_plane_line * nlines;
  
  if(num_points <= 0) return (0);
  
  /*
   * Get plane of data
   */
  
  data = (ui08 *) mdv.getField(com->data_field).getPlane1D();
  
  /*
   * Get area for sub array
   */
  
  ui08 *plane_data = (ui08 *) umalloc((ui32) (num_points * sizeof(ui08)));
  if(plane_data == NULL) return (0);
  
  /*
   * compute first offset into array
   */
  
  offset = (reply->y1 * info->nx) + reply->x1;
  
  /*
   * set pointer to starting point
   */
  
  data_ptr = data + offset;
  
  /*
   * Loop through array setting the sub-plane
   */
  
  nbytes_data_line = info->nx;
  plane_ptr = plane_data;
  
  for (i = 0; i < reply->ny; i++) {
    
    memcpy ((void *) plane_ptr,
            (void *) data_ptr,
            (size_t) nbytes_plane_line);
    
    data_ptr += nbytes_data_line;
    plane_ptr += nbytes_plane_line;
    
  } /* i */

  /*
   * compress as required
   */

  if (Glob->params.compress_for_transfer) {

    comp_data = uRLEncode8((ui08 *) plane_data,
			   (ui32) num_points,
			   (ui32) 255,
			   (ui32 *) &nbytes_compressed);

    ufree ((char *) plane_data);
    *rd_plane = comp_data;
    return (nbytes_compressed);

  } else {

    *rd_plane = plane_data;
    return (num_points);

  } /* if (Glob->compress_data) */
  
}

/*****************************************************************
 * GET_V_PLANE
 * Obtain a Vertical plane of data in the area designated.
 * Fills in grid limits in reply structure 
 */

static si32 get_v_plane(cdata_comm_t *com,
			cdata_reply_t *reply,
			cdata_info_t *info,
			MdvRead &mdv,
			ui08 **vert_plane)

{
  
  ui08 *ptr2, *ptr3, *dob_data; /* pointers for data manipulation */
  
  si32 iplane, d;
  si32 xadd, yadd;
  si32 nx, ny, nz, count;
  si32 inc1, inc2; /* Slope based decision variables */
  si32 num_points;
  si32 field_num;
  
  si32 offset;
  
  if (Glob->params.debug)
    fprintf(stderr, "%s:get_v_plane\n", Glob->prog_name);

  /*
   * Set grid limits
   */
  
  get_v_grid_limits(com, info, mdv,
		    &reply->x1, &reply->x2,
		    &reply->y1, &reply->y2,
		    &reply->z1, &reply->z2);
  
  /*
   * Calc grid dimensions of data array to be displayed
   */
  
  nx = ABSDIFF(reply->x1, reply->x2);
  ny = ABSDIFF(reply->y1, reply->y2);
  nz = (reply->z2 - reply->z1);
  
  reply->ny = nz + 1;
  reply->nz = 1;
  
  reply->dx = info->dx;
  reply->dy = info->dy;
  reply->dz = info->dz;
  reply->data_type = CDATA_CHAR;
  reply->orient = V_PLANE;
  
  /*
   * Get quanities from the appropriate header
   */
  
  load_reply(com, reply, mdv);
  
  /*
   * compute amount to move pointer in each dirn
   */
  
  xadd = ((reply->x1 < reply->x2) ? 1 : -1);
  
  yadd = ((reply->y1 < reply->y2) ?
	  info->nx : -(info->nx));
  
  field_num = com->data_field;
  
  if (nx > ny) {
    
    /*
     * X is dominate
     */
    
    reply->nx = nx + 1;
    num_points = reply->nx * reply->ny;
    
    *vert_plane = (ui08 *) umalloc((ui32) num_points);
    
    /* Movable pointer to new data */
    
    ptr2 = *vert_plane;
    offset = info->nx * reply->y1 + reply->x1;
    
    for (iplane = reply->z1; iplane <= reply->z2; iplane++ ) {
      
      /*
       * do each plane
       */
      
      if((dob_data = get_data_plane(mdv, field_num, iplane)) != NULL) {
	
	ptr3 = dob_data;
	
	/*
	 * move to correct row and column
	 */ 
	
	ptr3 += offset;
	*ptr2++ = *ptr3;
	inc1 = 2 * ny;
	inc2 = 2 * (ny - nx);
	d = inc1 - nx;
	
	count = nx;
	while(count-- > 0) {
	  if(d < 0) {
	    d += inc1;
	  } else {
	    d+= inc2;
	    ptr3 += yadd;
	  }
	  ptr3 += xadd;
	  *ptr2++ = *ptr3;
	} /* count */
	
      } /* if ((dob_data ... */
      
    } /* iplane */
    
  } else { /* if (nx > ny) */
    
    /*
     * Y is dominate
     */
    
    reply->nx = ny +1;
    num_points = reply->nx * reply->ny;
    
    *vert_plane = (ui08 *) umalloc((ui32) num_points); 
    
    /*
     * Movable pointer to new data
     */
    
    ptr2 = *vert_plane;
    offset = info->nx * reply->y1 + reply->x1;
    
    for (iplane = reply->z1; iplane <= reply->z2; iplane++ ) {
      
      /*
       * do each plane
       */
      
      if((dob_data = get_data_plane(mdv, field_num, iplane)) != NULL) {
	
	ptr3 = dob_data;
	
	/*
	 * move to correct row and column
	 */
	
	ptr3 += offset;
	
	*ptr2++ = *ptr3;
	inc1 = 2 * nx;
	inc2 = 2 * (nx - ny);
	d = inc1 - ny;
	
	count = ny;
	while(count-- > 0) {
	  if(d < 0) {
	    d += inc1;
	  } else {
	    d+= inc2;
	    ptr3 += xadd;
	  }
	  ptr3 += yadd;
	  *ptr2++ = *ptr3;
	} /* count */
	
      } /* if ((dob_data ... */
      
    } /* iplane */
    
  } /* if (nx > ny) */
  
  return(num_points);
  
}

/*****************************************************************
 * GET_V_GRID_LIMITS: Find the grid limits of a vertical plane
 */

static void get_v_grid_limits(cdata_comm_t *com,
			      cdata_info_t *info,
			      MdvRead &mdv,
			      si32 *rx1,
			      si32 *rx2,
			      si32 *ry1,
			      si32 *ry2,
			      si32 *rz1,
			      si32 *rz2)
     
{
  
  si32 x_1, y_1, x_2, y_2;
  si32 xflag, yflag;
  si32 done, xdiff, ydiff;
  
  double min_z, max_z;
  double slope, b;
  
  /*
   * Find grid height limits
   */
  
  min_z = (double) com->min_z / (double) com->divisor;
  max_z = (double) com->max_z / (double) com->divisor;

  *rz1 = z_km_to_grid(com, min_z, mdv);
  *rz2 = z_km_to_grid(com, max_z, mdv);
  
  /*
   * Find Horizontal grid limits
   */
  
  xy_km_to_grid(com,
		(double) com->min_x / (double) com->divisor,
		(double) com->min_y / (double) com->divisor,
		&x_1, &y_1, mdv);
  
  xy_km_to_grid(com,
		(double) com->max_x / (double) com->divisor,
		(double) com->max_y / (double) com->divisor,
		&x_2, &y_2, mdv);
  
  done = 0;
  xdiff = (x_1 - x_2);
  ydiff = (y_1 - y_2);
  
  /*
   * Make sure Grid limits are within the data
   */
  
  /*
   * If a point falls outside the grid, solve the equation for the line
   * at the limits of the grid
   */
  
  if(xdiff == 0) { /* slope == infinity */
    
    xflag = 0;
    
    x_1 = FCLIP(x_1, 0, info->nx - 1, xflag);
    
    if(xflag == 0) {
      *rx1 = x_1;
      *rx2 = x_1;
      *ry1 = CLIP(y_1, 0, info->ny - 1);
      *ry2 = CLIP(y_2, 0, info->ny - 1);
    } else {
      *rx1 = 0;
      *rx2 = 0;
      *ry1 = 0;
      *ry2 = 0;
    }
    
    done = 1;
    
  }
  
  if(ydiff == 0) { /* slope = 0 */
    
    yflag = 0;
    
    y_1 = FCLIP(y_1, 0, info->ny -1, yflag);
    
    if(yflag == 0) {
      *ry1 = y_1;
      *ry2 = y_1;
      *rx1 = CLIP(x_1, 0, info->nx - 1);
      *rx2 = CLIP(x_2, 0, info->nx - 1);
    } else {
      *rx1 = 0;
      *rx2 = 0;
      *ry1 = 0;
      *ry2 = 0;
    }
    
    done = 1;
    
  } 
  
  if(!done) {
    
    slope = (double) ydiff / (double) xdiff;
    b = (double) y_1 - (slope * (double) x_1);
    
    xflag = 0;
    x_1 = FCLIP(x_1, 0, info->nx - 1, xflag);
    if(xflag) {
      y_1 = (si32) floor((slope * (double) x_1 + b) + 0.5);
    } 
    
    yflag = 0;
    y_1 = FCLIP(y_1, 0, info->ny - 1, yflag);
    if(yflag) {
      x_1 = (si32) floor((((double) y_1 - b) / slope) + 0.5);
    } 
    
    xflag = 0;
    x_2 = FCLIP(x_2, 0, info->nx - 1, xflag);
    if(xflag) {
      y_2 = (si32) floor((slope * (double) x_2 + b) + 0.5);
    }
    
    yflag = 0;
    y_2 = FCLIP(y_2, 0, info->ny - 1, yflag);
    if(yflag) {
      x_2 = (si32) floor((((double) y_2 - b) / slope) + 0.5);
    }
    
    *rx1 = x_1;
    *rx2 = x_2;
    *ry1 = y_1;
    *ry2 = y_2;
    
  }
  
}

/*****************************************************************************
 * LOAD_REPLY
 *
 * loads up selected parts of the reply struct
 *
 */

static void load_reply(cdata_comm_t *com,
		       cdata_reply_t *reply,
		       MdvRead &mdv)
     
{
  
  si32 field_num;
  MDV_field_header_t *fhdr;
  
  field_num = com->data_field;
  fhdr = &mdv.getField(field_num).getFieldHeader();
  
  reply->bad_data_val = (int) fhdr->missing_data_value;
  reply->scale = (int) (fhdr->scale * reply->divisor + 0.5);
  reply->bias = (int) (fhdr->bias * reply->divisor + 0.5);
  
  if (com->primary_com & GET_MOST_RECENT ||
      com->primary_com & GET_NEW) {

    /*
     * realtime mode - use end time as time centroid so that data
     * which has just come is displayed as up to date
     */

    if (Glob->params.use_endtime_flag)
      reply->time_cent = mdv.getMasterHeader().time_end;
    else
      reply->time_cent = mdv.getMasterHeader().time_centroid;
    
  } else {

    /*
     * archive mode
     */

    reply->time_cent = mdv.getMasterHeader().time_centroid;
    
  }
    
  reply->time_begin = mdv.getMasterHeader().time_begin;
  reply->time_end = mdv.getMasterHeader().time_end;
  reply->expire_time = reply->time_end;
    
}

/**************************************************************************
 * XY_KM_TO_GRID
 *
 * For the (x,y) plane, return the grid coords that contain
 * the given Km coords.
 *
 */

static int xy_km_to_grid(cdata_comm_t *com,
			 double km_x,
			 double km_y,
			 si32 *grid_x,
			 si32 *grid_y,
			 MdvRead &mdv)

{
  
  int flag = 0;
  double dx, dy;
  double min_x, min_y;
  mdv_grid_t *grid =  &mdv.getField(com->data_field).getGrid();
  
  dx = grid->dx;
  dy = grid->dy;
  
  min_x = grid->minx;
  min_y = grid->miny;
  
  *grid_x =
    (si32) floor ((km_x + Glob->data_to_com_delta_x - min_x)  / dx + 0.5);
  *grid_y =
    (si32) floor ((km_y + Glob->data_to_com_delta_y - min_y)  / dy + 0.5);
  
  *grid_x = FCLIP(*grid_x, 0, grid->nx - 1, flag);
  *grid_y = FCLIP(*grid_y, 0, grid->ny - 1, flag);
  
  return (flag);
  
}

/**************************************************************************
 * Z_KM_TO_GRID
 *
 * In z, returns the grid coord closest to the ht given in km.
 *
 */

static int z_km_to_grid(cdata_comm_t *com,
			double km_z,
			MdvRead &mdv)

{

  si32 iz;
  si32 nz;
  si32 closest_plane = 0;
  
  double dz, min_dz = LARGE_DOUBLE;
  mdv_grid_t *grid =  &mdv.getField(com->data_field).getGrid();
  
  nz = grid->nz;

  if (nz < 2) {
    return (0);
  }

  if (mdv.getMasterHeader().vlevel_included) {

    for (iz = 0; iz < nz; iz++) {
      MDV_vlevel_header_t *vlvhdr =
	&mdv.getField(com->data_field).getVlevelHeader();
      dz = fabs(km_z - vlvhdr->vlevel_params[iz]);
      if (dz < min_dz) {
	closest_plane = iz;
	min_dz = dz;
      }
    }
    
  } else {

    closest_plane =
      (int) ((km_z - grid->minz) / grid->dz + 0.5);

  }

  if (closest_plane < 0) {
    closest_plane = 0;
  } else if (closest_plane > grid->nz - 1) {
    closest_plane = grid->nz - 1;
  }

  return (closest_plane);

}
