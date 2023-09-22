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
 * SHMEM_INIT
 *
 */

#define SHMEM_INIT

#include "cidd.h"

/************************************************************************
 * INIT_SHARED:  Initialize the shared memory communications
 *
 */

void init_shared()
{

  if((gd.coord_expt =
      (coord_export_t *) ushm_create(gd.coord_key,
                                     sizeof(coord_export_t),
                                     0666)) == NULL) {
    fprintf(stderr, "Couldn't create shared memory segment for Aux process communications\n");
    exit(-1);
  }
  
  memset(gd.coord_expt, 0, sizeof(coord_export_t));

  /* Initialize shared memory area for coordinate/selection communications */

  gd.coord_expt->button =  0;
  gd.coord_expt->selection_sec = 0;
  gd.coord_expt->selection_usec = 0;
  
  gd.epoch_start = (time_t)
    (gd.movie.start_time - (gd.movie.time_interval * 30.0));   
  gd.epoch_end = (time_t)
    (gd.movie.start_time +
     (gd.movie.num_frames * gd.movie.time_interval * 60.0) -
     (gd.movie.time_interval * 30.0)); 

  gd.coord_expt->epoch_start = gd.epoch_start;
  gd.coord_expt->epoch_end = gd.epoch_end;
  
  gd.coord_expt->time_min = gd.movie.frame[gd.movie.num_frames -1].time_start;
  gd.coord_expt->time_max = gd.movie.frame[gd.movie.num_frames -1].time_end;
  if(gd.movie.movie_on) {
    gd.coord_expt->time_cent = gd.epoch_end; 
  } else {
    gd.coord_expt->time_cent = gd.coord_expt->time_min +
      (gd.coord_expt->time_max - gd.coord_expt->time_min) / 2;
  } 
  gd.coord_expt->pointer_seq_num = 0;

  gd.coord_expt->datum_latitude = gd.uparams->getDouble(
          "cidd.origin_latitude", 39.8783);
  gd.coord_expt->datum_longitude = gd.uparams->getDouble( 
          "cidd.origin_longitude", -104.7568);
  gd.coord_expt->pointer_x = 0.0;
  gd.coord_expt->pointer_y = 0.0;
  gd.coord_expt->pointer_lon = gd.coord_expt->datum_longitude;
  gd.coord_expt->pointer_lat = gd.coord_expt->datum_latitude;

  gd.coord_expt->focus_x = 0.0;
  gd.coord_expt->focus_y = 0.0;
  gd.coord_expt->focus_lat = gd.coord_expt->datum_latitude;
  gd.coord_expt->focus_lon = gd.coord_expt->datum_longitude;

  gd.coord_expt->click_type = CIDD_RESET_CLICK;
}

