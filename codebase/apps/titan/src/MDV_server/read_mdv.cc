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
 * read_mdv.c
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307
 *
 * March 1997
 *
 * reads in the relevant volume file.
 *
 * returns 0 on success, -1 on failure
 */


#include "MDV_server.hh"
#include <toolsa/str.h>
#include <ctime>
using namespace std;

static int Latest_used;

/*
 * prototypes
 */

static char *file_path(cdata_comm_t *com);

int read_mdv(MdvRead &mdv, cdata_comm_t *com)
     
{

  char *path;

  path = file_path(com);

  if (path == NULL) {
    return (-1);
  }

  if (mdv.openFile(path)) {
    return (-1);
  }

  if (com->second_com == GET_XY_PLANE) {

    double min_z = (double) com->min_z / (double) com->divisor;
    double max_z = (double) com->max_z / (double) com->divisor;
    double requested_z = (min_z + max_z) / 2.0;

    if (mdv.readPlane(com->data_field, requested_z, MDV_INT8)) {
      mdv.closeFile();
      return (-1);
    }
    
  } else if (com->second_com == GET_MAX_XY_PLANE) {

    if (mdv.readComposite(com->data_field, MDV_INT8)) {
      mdv.closeFile();
      return (-1);
    }

  } else if (com->second_com == GET_V_PLANE) {

    if (mdv.readVol(com->data_field, MDV_INT8)) {
      mdv.closeFile();
      return (-1);
    }

  }

  mdv.closeFile();

  if (Glob->params.static_data_mode) {
    /* override time with time requested */
    mdv.getMasterHeader().time_begin = com->time_min;
    mdv.getMasterHeader().time_centroid = com->time_cent;
    mdv.getMasterHeader().time_end = com->time_max;
  }

  if (Latest_used) {
    Glob->latest_data_time = mdv.getMasterHeader().time_centroid;
  }

  return (0);

}

#define DEFAULT_SEARCH_DELTA 39600

static char *file_path(cdata_comm_t *com)

{

  static char path[MAX_PATH_LEN];

  /*
   * delta_for_time_min and delta_for_time_max are kept 
   * in static. They are used to determine the search period
   * for realtime requests in which time_offset is non-zero.
   * The DEFAULT_SEARCH_DELTA is used until an archive
   * request is made, after which the actual delta for
   * the latest archive call is kept in static and used.
   */
  
  static int delta_for_time_min = DEFAULT_SEARCH_DELTA;
  static int delta_for_time_max = DEFAULT_SEARCH_DELTA;

  int get_latest;
  long cent_time;

  /*
   * in static_data_mode, use static file path
   */
  
  if (Glob->params.static_data_mode) {
    return (Glob->params.static_data_file_path);
  }

  Latest_used = FALSE;

  if (com->primary_com & GET_MOST_RECENT ||
      com->primary_com & GET_NEW) {

    if (Glob->params.time_offset == 0) {

      get_latest = TRUE;

    } else {

      /*
       * if offset is non-zero, do not get latest, rather
       * use current time
       */

      get_latest = FALSE;
      com->time_cent = time(NULL);
      com->time_min = com->time_cent - delta_for_time_min;
      com->time_max = com->time_cent + delta_for_time_max;

    }

  } else {

    get_latest = FALSE;

  }

  if (get_latest) {

    if (Glob->params.use_realtime_file) {

      /*
       * use realtime file
       */

      STRncopy(path, Glob->params.realtime_file_path, MAX_PATH_LEN);
      Latest_used = TRUE;
      return (path);

    } else {

      /*
       * find latest file
       */

      /*
       * get most recent directory and file
       */
      
      if(get_latest_file(Glob->params.data_dirs.len,
			 Glob->params.data_dirs.val,
			 path)) {
	return (NULL);
      } else {
	return (path);
      }
      
    } /* if (Glob->params.use_realtime_file) */
    
  } else {

    delta_for_time_min = com->time_cent - com->time_min;
    delta_for_time_max = com->time_max - com->time_cent;

    
    /*
     * find the file closest to the given time
     */
    
    if (com->time_cent < com->time_min ||
	com->time_cent > com->time_max) {
      cent_time = com->time_min + (com->time_max - com->time_min) / 2;
    } else {
      cent_time = com->time_cent;
    }
  
    if (find_best_file (com->time_min + Glob->params.time_offset,
			cent_time + Glob->params.time_offset,
			com->time_max + Glob->params.time_offset,
			Glob->params.data_dirs.len,
			Glob->params.data_dirs.val,
			path)) {
      return (NULL);
    } else {
      return (path);
    }
    
  } /* if(com->primary_com & GET_MOST_RECENT ... */

}
  

