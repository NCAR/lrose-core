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
 * MDV_SERVER_GET_XY_PLANE
 * F. Hage - NCAR/RAP
 */

#define MDV_SERVER_GET_XY_PLANE
 
#include "mdv_server.h"

void native_units_to_grid(double native_x, double native_y,
                          int *return_grid_x, int *return_grid_y);

/*****************************************************************
 * GET_XY_PLANE:  Obtain an XY plane of data from the correct file
 *        in the area designated. Fills in grid limits in reply
 *        structure 
 */

int get_xy_plane(cdata_ieee_comm_t *com , ui08 **mdv_plane)
{
  int    i, j;
  ui08   *mdv_data, *mdv_ptr;  /* pointers for data manipulation */
  ui08   *mdv_pptr;  /* pointers for data manipulation */
  ui08   bad_val,missing_val;
  int    num_points;
  int    index;
  int    flag;
  long   offset, jump;             /* offset into file */
  double height;
  double dist, min_dist;
  cdata_ieee_reply_t *reply = &gd.current_ieee_reply;

  /* Initialize returned pointer */
  *mdv_plane = NULL;
  
  /* Check for out of bound limits */
//   if (com->max_z < gd.cur_f_head->fld_hdr->grid_minz)
//     return 0;

  /* Find closest plane to desired height */
  height = (com->min_z + com->max_z) / 2.0;
  if (gd.cur_m_head.vlevel_included && gd.cur_f_head->vlv_hdr != NULL) {
    min_dist = fabs(gd.cur_f_head->vlv_hdr->vlevel_params[0] - height);  
    index = 0;
    for (i = 1; i < gd.cur_f_head->fld_hdr->nz; i++ ) {
      dist = fabs(gd.cur_f_head->vlv_hdr->vlevel_params[i] - height);
      if (dist < min_dist) {
	min_dist = dist;
	index = i;
      }
    }
    reply->z1 = index;
  } else {
    height -= gd.cur_f_head->fld_hdr->grid_minz;
    if(gd.cur_f_head->fld_hdr->grid_dz == 0) {
       reply->z1 =  0;
    } else {
       reply->z1 = (int) ((height / ((double) gd.cur_f_head->fld_hdr->grid_dz)) + 0.5);
    }
  }

  reply->z1 = CLIP(reply->z1, 0, (gd.cur_f_head->fld_hdr->nz - 1), flag);
  reply->z2 = reply->z1;
  reply->data_type = CDATA_CHAR;
  reply->orient = XY_PLANE;

  /* Find Horizontal grid limits */
  native_units_to_grid(com->min_x, com->min_y,
		       &reply->x1, &reply->y1);

  native_units_to_grid(com->max_x, com->max_y,
		       &reply->x2, &reply->y2);

  reply->x1 = CLIP(reply->x1, 0, (gd.cur_f_head->fld_hdr->nx - 1), flag);
  reply->x2 = CLIP(reply->x2, 0, (gd.cur_f_head->fld_hdr->nx - 1), flag);
  reply->y1 = CLIP(reply->y1, 0, (gd.cur_f_head->fld_hdr->ny - 1), flag);
  reply->y2 = CLIP(reply->y2, 0, (gd.cur_f_head->fld_hdr->ny - 1), flag);

  if (!gd.daemonize_flag) printf("Plane %d: X1,Y1,X2,Y2: %d,%d %d,%d\n",
      reply->z1, reply->x1 ,reply->x2 ,reply->y1, reply->y2);

  reply->nx = reply->x2 - reply->x1 + 1;
  reply->ny = reply->y2 - reply->y1 + 1;
  reply->nz = 1;

  reply->dx = gd.cur_f_head->fld_hdr->grid_dx;
  reply->dy = gd.cur_f_head->fld_hdr->grid_dy;
  reply->dz = gd.cur_f_head->fld_hdr->grid_dz;
    
  num_points = (reply->x2 - reply->x1 + 1) * (reply->y2 - reply->y1 + 1);
  if (num_points <= 0)
    return 0;

  /* Get a Plane of data from the disk */
  
  MDV_field_header_t locFhdr = *gd.cur_f_head->fld_hdr;
  int plane_len;

  if ((mdv_data = (ui08 *) MDV_read_field_plane(gd.data_file,
						&locFhdr,
						MDV_INT8,
						MDV_COMPRESSION_NONE,
						MDV_SCALING_ROUNDED, 0.0, 0.0,
						reply->z1,
						&plane_len)) == NULL) {
    return 0;
  }

  reply->bad_data_val = (int) locFhdr.bad_data_value;
  reply->scale = locFhdr.scale;
  reply->bias = locFhdr.bias;

  /* Get area for sub array */
  *mdv_plane =  (ui08 *) malloc(num_points * sizeof(ui08));
  mdv_pptr = *mdv_plane;    /* copy pointer */
  if (mdv_pptr == NULL)
  {
    ufree(mdv_data);
    return 0;
  }
  
  /* Calc sub region offsets */
  offset = (reply->y1 * gd.cur_f_head->fld_hdr->nx) + reply->x1;    /* first offset into array */
  jump = gd.cur_f_head->fld_hdr->nx - reply->nx;                    /* gap in bytes between rows */

  bad_val = (ui08) gd.cur_f_head->fld_hdr->bad_data_value;
  missing_val = (ui08) gd.cur_f_head->fld_hdr->missing_data_value;

  mdv_ptr = mdv_data + offset;    /* set pointer to starting point */

  /* Loop through array and get data */
  for (i = reply->ny; i--; /* NULL */)
  {
    for (j = reply->nx; j--; /* NULL */)
    {
         // Map missing data to bad for cdata protocol 
         if(*mdv_ptr == missing_val) {
             *mdv_pptr = bad_val;   
	     mdv_pptr++;
             mdv_ptr++;
         } else {
             *mdv_pptr++ = *mdv_ptr++;
         }

    }
    mdv_ptr += jump;
  }
  ufree(mdv_data);        /* free up area allocated for whole plane */
  
  return num_points;
}

/**************************************************************************
 * NATIVE_UNITS_TO_GRID: Return the grid coords that represent the given
 *                       native (km or deg) coords.
 *
 */

void native_units_to_grid(double native_x, double native_y,
                                 int *return_grid_x, int *return_grid_y)
{
  *return_grid_x = (int) ((native_x - ((double)gd.cur_f_head->fld_hdr->grid_minx )) /
    ((double)gd.cur_f_head->fld_hdr->grid_dx));                                
  *return_grid_y = (int) ((native_y - ((double)gd.cur_f_head->fld_hdr->grid_miny)) /
    ((double)gd.cur_f_head->fld_hdr->grid_dy));                               

  return;
}
