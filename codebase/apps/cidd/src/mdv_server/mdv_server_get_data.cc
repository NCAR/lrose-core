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
 * MDV_SERVER_GET_DATA
 * F. Hage - NCAR/RAP
 */

#define MDV_SERVER_GET_DATA
 
#include "mdv_server.h"
#include <toolsa/compress.h>

/*****************************************************************
 * GET_GRID_DATA:  Gather data from the correct file
 */

ui08 *get_grid_data(cdata_ieee_comm_t *com,cdata_ieee_comm2_t * com2)
{
  unsigned int    size;
  ui08       *mdv_plane,*cmp_ptr;  /* pointers for data manipulation */
  int        num_points;

  switch(com->second_com)
  {
  case GET_XY_PLANE:
    num_points = get_xy_plane(com, &mdv_plane);
    break;

  case GET_XZ_PLANE:
    num_points = 0;        /* Not implemented yet */
    mdv_plane = NULL;
    break;

  case GET_YZ_PLANE:
    num_points = 0;        /* Not implemented yet */
    mdv_plane = NULL;
    break;

  case GET_V_PLANE:    /* Arbitrary Vertical Plane */
    num_points = get_v_plane(com, com2, &mdv_plane);
    break;

  case GET_VOLUME:    /* Entire field volume */
    num_points = 0;        /* Not implemented yet */
    mdv_plane = NULL;
    break;

  case GET_MAX_XY_PLANE:    /* MAX VALUE Projection */
    num_points = get_max_xy_plane(com, com2, &mdv_plane);
    break;

  case GET_MAX_XZ_PLANE:    /* MAX VALUE Projection */
    num_points = 0;        /* Not implemented yet */
    mdv_plane = NULL;
    break;

  case GET_MAX_YZ_PLANE:    /* MAX VALUE Projection */
    num_points = 0;        /* Not implemented yet */
    mdv_plane = NULL;
    break;

  }

  if (num_points == 0 || mdv_plane == NULL)    /* No data available */
  {
    if (mdv_plane != NULL)
      free(mdv_plane);
    gd.current_ieee_reply.status |= NO_DATA_AVAILABLE;
    return NULL;
  }

  gd.current_ieee_reply.n_points = num_points;
  gd.data_length = num_points;

  if (gd.compress_flag)
  {
    if ((cmp_ptr = uRLEncode8(mdv_plane, num_points, 
			      (ui32) 255, &size)) != NULL)
    {
      free(mdv_plane);
      mdv_plane = cmp_ptr;
      gd.current_ieee_reply.n_points = size;
      gd.data_length = size;
    }
  }

  return(mdv_plane);
}
