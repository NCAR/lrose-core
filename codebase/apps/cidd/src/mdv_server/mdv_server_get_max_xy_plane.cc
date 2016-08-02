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
 * MDV_SERVER_GET_MAX_XY_PLANE
 * F. Hage - NCAR/RAP
 */

#define MDV_SERVER_GET_MAX_XY_PLANE
 
#include "mdv_server.h"

#define MAXOF(a, b) ((a) > (b) ? (a) : (b))
/*****************************************************************
 * GET_MAX_XY_PLANE:  Obtain an  composite XY plane of data,
 * taking the max data from any plane.  
 *  Fills in grid limits in reply structure 
 */

int get_max_xy_plane(cdata_ieee_comm_t *com , cdata_ieee_comm2_t *com2, ui08 **mdv_plane)
{
  int    i, j;
  ui08   *mdv_data, *mdv_ptr, *mdv_vol;  /* pointers for data manipulation */
  ui08   *mdv_pptr;  /* pointers for data manipulation */
  ui08   bad_val,missing_val; 
  int    npoints_plane;
  int    num_points;
  int    plane;
  int    flag;
  long   offset, jump;             /* offset into file */
  cdata_ieee_reply_t *reply = &gd.current_ieee_reply;

  /* Initialize returned pointer */
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

  if (!gd.daemonize_flag) printf("Composite Plane: X1,Y1,X2,Y2,Z1,Z2: %d,%d %d,%d %d,%d\n",
       reply->x1 ,reply->x2 ,reply->y1, reply->y2, reply->z1, reply->z2);

  reply->nx = reply->x2 - reply->x1 + 1;
  reply->ny = reply->y2 - reply->y1 + 1;
  reply->nz = 1;

  reply->dx = gd.cur_f_head->fld_hdr->grid_dx;
  reply->dy = gd.cur_f_head->fld_hdr->grid_dy;
  reply->dz = gd.cur_f_head->fld_hdr->grid_dz;

  num_points = (reply->x2 - reply->x1 + 1) * (reply->y2 - reply->y1 + 1);
  if (num_points <= 0) {
    ufree (mdv_vol);
    return 0;
  }

  reply->bad_data_val = (int) locFhdr.bad_data_value;
  reply->scale = locFhdr.scale;
  reply->bias = locFhdr.bias;

  /* Get area for sub array */
  *mdv_plane =  (ui08 *) calloc(num_points, sizeof(ui08));
  if (mdv_pptr == NULL) {
    ufree(mdv_vol);
    return 0;
  }

  // Check for the maximum value being used to indicate missing or bad
  bad_val = (ui08) locFhdr.bad_data_value;
  missing_val = (ui08) locFhdr.missing_data_value;
  if(bad_val > missing_val) missing_val = bad_val;
  
  /* Calc sub region offsets */
  offset = (reply->y1 * gd.cur_f_head->fld_hdr->nx) + reply->x1; /* first offset into array */
  jump = gd.cur_f_head->fld_hdr->nx - reply->nx;                 /* gap in bytes between rows */

  // Loop through each plane
  npoints_plane = gd.cur_f_head->fld_hdr->nx * gd.cur_f_head->fld_hdr->ny;
  for(plane=reply->z1; plane <= reply->z2; plane++ ) {

      /* Get a pointer to plane of data */
      mdv_data = mdv_vol + plane * npoints_plane;

      mdv_ptr = mdv_data + offset;  /* set source pointer to starting point */
      mdv_pptr = *mdv_plane;        /* set destination pointer */

      /* Loop through array and get data */
      for (i = reply->ny; i--; /* NULL */) {
        for (j = reply->nx; j--; /* NULL */) {
	  // Don't use if missing
	  if(*mdv_ptr != missing_val) {
             *mdv_pptr = MAXOF(*mdv_pptr, *mdv_ptr);
	  } else {
	  }
          mdv_pptr++;
	  mdv_ptr++;
        }
        mdv_ptr += jump;
      }
   } // Each plane

  ufree(mdv_vol);        /* free up area allocated for whole plane */
  
  return num_points;
}
