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
 * MDV_SERVER_REPLY
 * F. Hage - NCAR/RAP
 */

#define MDV_SERVER_REPLY

#include "mdv_server.h"

void copy_to_int_reply(cdata_reply_t *i_reply, cdata_ieee_reply_t *src_reply);
/*****************************************************************
 * INITIALIZE_REPLY: Set up default values in to the reply
 */

void initialize_reply(cdata_ieee_reply_t *reply)
{
  reply->data_length = 0;
  reply->status = 0;
  reply->status |= REPLY_USE_IEEE_FLOAT;

  /* Get these quanities from the file header */

  reply->time_begin = gd.cur_m_head.time_begin;
  reply->time_end = gd.cur_m_head.time_end;
  reply->time_cent = gd.cur_m_head.time_centroid;

  reply->expire_time = reply->time_end + VALID_TIME;

  /* These are set by get_data routines */
  reply->dx = 0.0;
  reply->dy = 0.0;
  reply->dz = 0.0; 

  reply->orient = 0;
  reply->nx = 0;
  reply->ny = 0;
  reply->nz = 0;
  reply->x1 = 0;
  reply->x2 = 0;
  reply->y1 = 0;
  reply->y2 = 0;
  reply->z1 = 0;
  reply->z2 = 0;
  reply->scale = 0.0;
  reply->bias = 0.0;
  reply->bad_data_val = 0;
  reply->data_type = 0;
  reply->n_points = 0;

  return;
}

/*****************************************************************
 * COPY_TO_INT_REPLY
 */

void copy_to_int_reply(cdata_reply_t *i_reply, cdata_ieee_reply_t *src_reply)
{
  double divisor;

  if(gd.cur_f_head->fld_hdr->proj_type == MDV_PROJ_LATLON) {
      divisor = CDATA_LATLON_DIVISOR / 4;
  } else {
      divisor = CDATA_DIVISOR;
  }
  i_reply->divisor = (int) divisor;

  i_reply->status = src_reply->status & ~(REPLY_USE_IEEE_FLOAT);

  i_reply->data_length = src_reply->data_length;

  i_reply->time_begin = src_reply->time_begin;
  i_reply->time_end =  src_reply->time_end;
  i_reply->time_cent =  src_reply->time_cent;
  i_reply->expire_time =  src_reply->expire_time;

  i_reply->dx = (int) (src_reply->dx  * divisor);
  i_reply->dy = (int) (src_reply->dy * divisor);
  i_reply->dz = (int) (src_reply->dz * divisor); 

  i_reply->orient = src_reply->orient;
  i_reply->nx = src_reply->nx;
  i_reply->ny = src_reply->ny;
  i_reply->nz = src_reply->nz;
  i_reply->x1 = src_reply->x1;
  i_reply->x2 = src_reply->x2;
  i_reply->y1 = src_reply->y1;
  i_reply->y2 = src_reply->y2;
  i_reply->z1 = src_reply->z1;
  i_reply->z2 = src_reply->z2;
  i_reply->scale = (int) (src_reply->scale * divisor);
  i_reply->bias = (int) (src_reply->bias * divisor);
  i_reply->bad_data_val = src_reply->bad_data_val;
  i_reply->data_type = src_reply->data_type;
  i_reply->n_points = src_reply->n_points;

  return;
}


/*****************************************************************
 * SEND_REPLY: Send a reply message to the client
 */

void send_reply(int id,
		       cdata_ieee_comm_t    *com,        /* the request */
		       cdata_ieee_reply_t    *reply,
		       cdata_ieee_info_t    *info,
		       ui08    *ptr)                /* pointer to data */
{
  int i, num_vals;
  double divisor;
  cdata_reply_t   i_reply;
  cdata_info_t    i_info;
  int i_plane_hts[MAX_DATA_PLANES * 3];

  reply->data_length = 0;

  if(gd.cur_f_head != NULL && 
     gd.cur_f_head->fld_hdr != NULL && 
     gd.cur_f_head->fld_hdr->proj_type == MDV_PROJ_LATLON) {
      divisor = CDATA_LATLON_DIVISOR / 4;
  } else {
      divisor = CDATA_DIVISOR;
  }

  /*
   * See if we are sending the info structure.  We send info if
   * the client asked for info and the reply status doesn't indicate
   * there is no info available.
   */

  if ((com->primary_com & GET_INFO) &&
      ((reply->status & NO_INFO) == 0))
  {
    reply->status |= REQUEST_SATISFIED;
    reply->status |= INFO_FOLLOWS;
    reply->data_length += sizeof(*info);
  }

  /*
   * See if we are sending data.  We send data if the client asked 
   * for data and the reply status doesn't indicate there is no
   * data or no new data.
   */

  if ((com->primary_com & GET_DATA) &&
      ((reply->status & (NO_DATA_AVAILABLE | NO_NEW_DATA)) == 0))
  {
    reply->status |= REQUEST_SATISFIED;
    reply->status |= DATA_FOLLOWS;
    reply->data_length += gd.data_length;
  }

  /*
   * See if we are sending the plane heights.  We send plane heights
   * if the client requested them and the reply status doesn't indicate
   * there is no info available.
   */

  if ((com->primary_com & GET_PLANE_HEIGHTS) &&
      ((reply->status & NO_INFO) == 0))
  {
    reply->status |= PLANE_HEIGHTS_FOLLOW;
  }

  switch( gd.protocol_version) {
  case 2:  /* IEEE based Version 2 Protocol */
     /* Put reply in network byte order */
     BE_from_array_32((ui32 *)reply, (ui32)sizeof(*reply));

     /* Send the reply */

     if(!gd.daemonize_flag)
            fprintf(stderr,"Sending %d Reply Bytes\n",sizeof(*reply));
     if (SKU_write(gd.sockfd, reply, sizeof(*reply), 2) != sizeof(*reply)) {
        if(!gd.daemonize_flag)
            fprintf(stderr,"Problems writing reply to socket");
     }

     /* Put the reply back in host order so we can use the fields */
     BE_to_array_32((ui32 *)reply, (ui32)sizeof(*reply));

     /* Send info with reply if required */
     if(reply->status & INFO_FOLLOWS) {
       info->data_length = gd.data_length;

        BE_from_array_32((ui32 *)info, (ui32)sizeof(long) * NUM_INFO_LONGS);

     if(!gd.daemonize_flag)
            fprintf(stderr,"Sending %d Info Bytes\n",sizeof(*info));
        if(SKU_write(gd.sockfd,info,sizeof(*info),10) != sizeof(*info)) {
          if(!gd.daemonize_flag) 
	   fprintf(stderr,"Problems writing info to socket");
        }

        BE_to_array_32((ui32 *)info, (ui32)sizeof(*info));
      }


      /* Send height info if required */
      if (reply->status & PLANE_HEIGHTS_FOLLOW) {
	num_vals = gd.num_planes * 3;
        BE_from_array_32(gd.plane_heights, 
			 (ui32)(num_vals * sizeof(fl32)));
        if (SKU_write(gd.sockfd, gd.plane_heights,
		      (num_vals * sizeof(fl32)),10)
	    != (int) (num_vals * sizeof(fl32)))
	{
               if(!gd.daemonize_flag)
		   fprintf(stderr,
		      "Problems writing vertical plane heights message\n");
        }
        BE_to_array_32(gd.plane_heights,
                       (ui32)(num_vals * sizeof(ui32)));
 
      }
    break;

    case 0:  /* Old Scaled Int Protocol */
     copy_to_int_reply(&i_reply, reply);

     /* Put reply in network byte order */
     BE_from_array_32((ui32 *)&i_reply, (ui32)sizeof(i_reply));

     /* Send the reply */
      if(!gd.daemonize_flag)
            fprintf(stderr,"Sending %d Reply Bytes\n",sizeof(i_reply));
     if (SKU_write(gd.sockfd, &i_reply, sizeof(i_reply), 2) != sizeof(i_reply)) {
        if(!gd.daemonize_flag)
            fprintf(stderr,"Problems writing reply to socket");
     }

     /* Send info with reply if required */
     if(reply->status & INFO_FOLLOWS) {
       info->data_length = gd.data_length;

       copy_to_int_info(&i_info, info);
       BE_from_array_32((ui32 *)&i_info, (ui32)sizeof(long) * NUM_INFO_LONGS);

        if(!gd.daemonize_flag)
            fprintf(stderr,"Sending %d Info Bytes\n",sizeof(i_info));
        if(SKU_write(gd.sockfd,&i_info,sizeof(i_info),10) != sizeof(i_info)) {
          if(!gd.daemonize_flag) 
	    fprintf(stderr,"Problems writing info to socket");
        }
      }

      /* Send height info if required */
      if (reply->status & PLANE_HEIGHTS_FOLLOW) {
	/* Scale to ints */
	num_vals = gd.num_planes * 3;
	for(i= 0; i < num_vals ; i++) {
	    i_plane_hts[i] = (int) (gd.plane_heights[i] * divisor);
	}
	    
        BE_from_array_32(i_plane_hts, (ui32)(num_vals * sizeof(ui32)));
     if(!gd.daemonize_flag)
            fprintf(stderr,"Sending %d Vert Heights Bytes\n",(num_vals * sizeof(ui32)));
        if (SKU_write(gd.sockfd, i_plane_hts, (num_vals * sizeof(ui32)),10)
	    !=  (int) (num_vals * sizeof(ui32)))
	{
               if(!gd.daemonize_flag)
		   fprintf(stderr,
		      "Problems writing vertical plane heights message\n");
        }
 
      }
    break;
    }


  /*
   * Send data if requested. Note that we are not yet byte-swapping
   * the data.  This will cause problems when we start sending 
   * non-ui08 data between different architectures.
   */

  if (reply->status & DATA_FOLLOWS)
  {
     if(!gd.daemonize_flag)
            fprintf(stderr,"Sending %d Data Bytes\n",gd.data_length);
    if (SKU_write(gd.sockfd, ptr, gd.data_length, 10) != gd.data_length)
    {
      if(!gd.daemonize_flag) fprintf(stderr,"Problems writing message");
    }
    gd.data_length = 0;

  }

  return;
}
 
