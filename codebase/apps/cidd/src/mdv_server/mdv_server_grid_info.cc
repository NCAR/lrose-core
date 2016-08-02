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
 * MDV_SERVER_GRID_INFO
 * F. Hage - NCAR/RAP
 */

#define MDV_SERVER_GRID_INFO

#include "mdv_server.h"
#include <toolsa/str.h>

/*****************************************************************
 * COPY_TO_INT_INFO: 
 */

void copy_to_int_info(cdata_info_t *i_info, cdata_ieee_info_t  *info)
{
    double divisor;
    double hires_divisor;

    if(info->projection == PJG_LATLON){
      divisor = CDATA_LATLON_DIVISOR / 4;
      i_info->highres_divisor = CDATA_HIGHRES_DIVISOR;
      hires_divisor = CDATA_HIGHRES_DIVISOR;
    } else {
      divisor = CDATA_DIVISOR;
      hires_divisor = CDATA_DIVISOR;
      i_info->highres_divisor = 0;
    }

    /* Do the basic copy */
    memcpy((void *) i_info, (void *) info, sizeof(cdata_ieee_info_t));

    /* now fix all the floats */
    i_info->divisor = (int) divisor;
    i_info->lat_origin =  (int)(gd.origin_lat * divisor + 0.5);
    i_info->lon_origin =  (int)(gd.origin_lon * divisor + 0.5);
    i_info->ht_origin = (int) (info->ht_origin * divisor);

    i_info->dx = (int) (info->dx * hires_divisor);
    i_info->dy = (int) (info->dy * hires_divisor);

    i_info->min_x = (int) ((info->min_x)  * hires_divisor);
    i_info->max_x = (int) ((info->max_x)  * hires_divisor);
    i_info->min_y = (int) ((info->min_y)  * hires_divisor);
    i_info->max_y = (int) ((info->max_y)  * hires_divisor);
    i_info->dz = (int) (info->dz * divisor);
    i_info->min_z = (int) (info->min_z * divisor);
    i_info->max_z = (int) (info->max_z * divisor);
    if (!gd.daemonize_flag)
	  printf("Forwarded MIN_X,MIN_Y,MAX_X,MAX_Y: %.2f,%.2f %.2f,%.2f\n",
		   (info->min_x),
		   (info->min_y),
		   (info->max_x),
		   (info->max_y));
}

/*****************************************************************
 * COPY_HEADER_TO_INFO: Copy compatible data fields to our local
 *        info structure 
 */

void copy_header_to_info(cdata_ieee_info_t *info)
{
  int    i;
  fl32    *ptr;
  double  delz, minz, maxz;

  info->divisor = 1;
  info->order = DATA_ORDER;

  switch (gd.cur_f_head->fld_hdr->proj_type)
  {
  case MDV_PROJ_FLAT :
    info->projection = PJG_FLAT;
    break;
      
  case MDV_PROJ_LATLON :
    info->projection = PJG_LATLON;
    break;
      
  default:
    /*
     * We currently only have flat and lat/lon projections.  If we need
     * to support another projection, this will need to be fixed.  Most
     * likely, no other projections will be needed in the lifetime of
     * this module, so I'm not going to worry about it now.
     */

    info->projection = PJG_FLAT;
    break;
  }
    
  info->data_field = gd.found_field_index;
  info->lat_origin =  gd.origin_lat;
  info->lon_origin =  gd.origin_lon;

  info->source_x = 0.0;
  info->source_y = 0.0;
  info->ht_origin = gd.cur_f_head->fld_hdr->vert_reference;

  info->nx = gd.cur_f_head->fld_hdr->nx;
  info->ny = gd.cur_f_head->fld_hdr->ny;
  info->nz = gd.cur_f_head->fld_hdr->nz;

  info->dx = gd.cur_f_head->fld_hdr->grid_dx;
  info->dy = gd.cur_f_head->fld_hdr->grid_dy;

  info->min_x = (gd.cur_f_head->fld_hdr->grid_minx -
		   gd.cur_f_head->fld_hdr->grid_dx/2.0) + gd.x_offset;
  info->min_y = (gd.cur_f_head->fld_hdr->grid_miny -
		   gd.cur_f_head->fld_hdr->grid_dy/2.0) - gd.y_offset;

  info->max_x = (gd.cur_f_head->fld_hdr->grid_minx +
		   (gd.cur_f_head->fld_hdr->grid_dx * (info->nx - 1)) +
		   gd.cur_f_head->fld_hdr->grid_dx/2.0) + gd.x_offset;
  info->max_y = (gd.cur_f_head->fld_hdr->grid_miny +
	   (gd.cur_f_head->fld_hdr->grid_dy * (info->ny - 1)) +
		   gd.cur_f_head->fld_hdr->grid_dy/2.0) - gd.y_offset;
    
  ptr = gd.plane_heights;
  if (gd.cur_m_head.vlevel_included && gd.cur_f_head->vlv_hdr != NULL)
  {
    info->min_z = gd.cur_f_head->vlv_hdr->vlevel_params[0];
    info->max_z = gd.cur_f_head->vlv_hdr->vlevel_params[gd.num_planes - 1];

    delz =  (info->min_z + info->max_z) / gd.num_planes;

    /* Set the first level */
    *ptr++ = gd.cur_f_head->fld_hdr->vert_reference;
    *ptr++ = gd.cur_f_head->vlv_hdr->vlevel_params[0];
    *ptr++ = (gd.cur_f_head->vlv_hdr->vlevel_params[0] + delz);

    /* Set the rest of the levels except the last level */
    for (i = 1; i < gd.num_planes - 1; i++)
    {
      *ptr++ = ((gd.cur_f_head->vlv_hdr->vlevel_params[i-1] + 
		 gd.cur_f_head->vlv_hdr->vlevel_params[i]) / 2.0);
      *ptr++ = gd.cur_f_head->vlv_hdr->vlevel_params[i];
      *ptr++ = ((gd.cur_f_head->vlv_hdr->vlevel_params[i+1] + 
		 gd.cur_f_head->vlv_hdr->vlevel_params[i]) / 2.0);
    }

    /* now set the last level */
    *ptr++ = ((gd.cur_f_head->vlv_hdr->vlevel_params[gd.num_planes-2] +
	       gd.cur_f_head->vlv_hdr->vlevel_params[gd.num_planes-1]) / 2.0);
    *ptr++ = gd.cur_f_head->vlv_hdr->vlevel_params[gd.num_planes-1];
    *ptr++ = (gd.cur_f_head->vlv_hdr->vlevel_params[gd.num_planes-1] + delz);
    
  } else {
    delz = gd.cur_f_head->fld_hdr->grid_dz;
    minz = gd.cur_f_head->fld_hdr->grid_minz -
      (gd.cur_f_head->fld_hdr->grid_dz / 2.0);
    maxz = gd.cur_f_head->fld_hdr->grid_minz -
      (gd.cur_f_head->fld_hdr->grid_dz / 2.0) + 
	(gd.cur_f_head->fld_hdr->grid_dz *
	 gd.cur_f_head->fld_hdr->nz);

    info->dz = delz;
    info->min_z = minz;
    info->max_z = maxz;

    for (i = 0; i < gd.num_planes; i++) {
      *ptr++ = (minz  + ((double)i * delz));
      *ptr++ = (minz  + ((double)i+0.5) * delz);
      *ptr++ = (minz  + ((double)i+1.0) * delz);
    }
  }

  if (gd.cur_f_head->fld_hdr->proj_type == MDV_PROJ_FLAT) {
    info->north_angle = gd.cur_f_head->fld_hdr->proj_param[0];
    STRncopy(info->units_label_x, "KM", LAB_LEN);
    STRncopy(info->units_label_y, "KM", LAB_LEN);
  } else {
    info->north_angle = 0;
    STRncopy(info->units_label_x, "DEG", LAB_LEN);
    STRncopy(info->units_label_y, "DEG", LAB_LEN);
  }

  switch(gd.cur_m_head.vlevel_type) {
    default:     
      STRncopy(info->units_label_z, "Units", LAB_LEN);
     break;

    case  MDV_VERT_TYPE_ELEV:
      STRncopy(info->units_label_z, "DEG", LAB_LEN);
     break;

    case  MDV_VERT_TYPE_Z:
      STRncopy(info->units_label_z, "KM", LAB_LEN);
     break;

    case  MDV_VERT_SATELLITE_IMAGE:
      STRncopy(info->units_label_z, "IMAGE", LAB_LEN);
     break;

    case  MDV_VERT_FLIGHT_LEVEL:
      STRncopy(info->units_label_z, "FL", LAB_LEN);
     break;

    case  MDV_VERT_TYPE_SURFACE:
      STRncopy(info->units_label_z, "SFC", LAB_LEN);
     break;

  }
    if (!gd.daemonize_flag)
	fprintf(stderr,"Units Labels: X:%s Y:%s Z:%s\n",
	    info->units_label_x,
	    info->units_label_y,
	    info->units_label_z);

  info->gate_spacing = GATE_SPACING;
  info->wavelength = WAVELENGTH;
  info->frequency = FREQUENCY;

  info->min_range = MIN_RANGE;
  info->max_range = MAX_RANGE;
  info->num_gates = NUM_GATES;
  info->min_gates = MIN_GATES;
  info->max_gates = MAX_GATES;
  
  info->num_tilts = NUM_TILTS;
  info->min_elev = MIN_ELEV;
  info->max_elev = MAX_ELEV;

  info->radar_const = RADAR_CONST;
  // info->highres_divisor = 1;
  info->delta_azmith = DELTA_AZIMUTH;
  info->start_azmith = START_AZIMUTH;
  info->beam_width = BEAM_WIDTH;
  info->pulse_width = PULSE_WIDTH;

  info->noise_thresh = NOISE_THRESH;

  STRncopy(info->field_name, gd.cur_f_head->fld_hdr->field_name, LAB_LEN);
  STRncopy(info->field_units, gd.cur_f_head->fld_hdr->units, LAB_LEN);
  STRncopy(info->source_name, gd.cur_m_head.data_set_source, LAB_LEN);

  return;
}
