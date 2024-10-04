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
 * Data handling module
 *
 */

#define DATA_HANDLING

#include <cerrno>
#include "cidd.h"

static string _getUrl(met_record_t *mr);

static string _getFieldName(met_record_t *mr);

static int _getTimeList(met_record_t *mr,
                        time_t start_time,
                        time_t end_time,
                        int page);

void cancel_pending_request();

static void _checkForHorizData(met_record_t *mr);
static void _checkForVertData(met_record_t *mr);
static void _checkForTimelistData(met_record_t *mr);
static void _checkForSymprodData();

/**********************************************************************
 * REQUEST_HORIZ_DATA_PLANE: Query the server for a data plane
 *
 */

int mdvx_request_horiz_data_plane(met_record_t *mr,
                                  time_t start_time,
                                  time_t end_time,
                                  int page)
{

  // Construct URL, check is valid.

  string url(_getUrl(mr));
  DsURL URL(url);  
  if(URL.isValid() != TRUE) {
    cerr << "111 Bogus URL: " << url << endl;
    mr->h_data_valid = 1;
    return -1;
  }

  // Offset the request time

  start_time += (int) (mr->time_offset * 60);
  end_time += (int) (mr->time_offset * 60);
  
  if(gd.debug1) {
    fprintf(stderr, "Get MDVX Horiz Plane - page : %d  -  %s\n", page, mr->url);
    // Disable threading while in debug mode
    mr->h_mdvx->setThreadingOff();
  }

  // Gather a data index for this field  
  if(mr->time_list_valid == 0) {
    _getTimeList(mr, start_time,end_time,  page);
    return 0; // Must wait until it's done
  }
  
  // get field name
  
  string fieldName(_getFieldName(mr));

  // add field to request
  
  mr->h_mdvx->clearRead(); 
  mr->h_mdvx->addReadField(fieldName);

  // set up read
  
  time_t mid_time = start_time + (end_time - start_time) / 2;
  
  switch(_params.gather_data_mode ) {

    case CLOSEST_TO_FRAME_CENTER:
      gd.data_request_time = mid_time;
      if(gd.model_run_time != 0 && mr->h_mhdr.data_collection_type == Mdvx::DATA_FORECAST) { 
        mr->h_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                                url,
                                (int) (60 * mr->time_allowance),
                                gd.model_run_time,
                                mid_time - gd.model_run_time);
      } else {
        mr->h_mdvx->setReadTime(Mdvx::READ_CLOSEST,
                                url,
                                (int) (60 * mr->time_allowance),
                                mid_time);
      }
      break;

    case FIRST_BEFORE_END_OF_FRAME:
      gd.data_request_time = end_time;
      if(gd.model_run_time != 0 && mr->h_mhdr.data_collection_type == Mdvx::DATA_FORECAST) { 
        mr->h_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                                url,
                                (int) (60 * mr->time_allowance),
                                gd.model_run_time,
                                (end_time - gd.model_run_time));
      } else {
        mr->h_mdvx->setReadTime(Mdvx::READ_FIRST_BEFORE,
                                url,
                                (int) (60 * mr->time_allowance),
                                end_time + 1, 0);
      }
      
      if(gd.debug1) {
        fprintf(stderr,
                "{ READ_VOLUME_REQUEST, \"%s\", \"%s\", %d, %ld },\n",
                url.c_str(), fieldName.c_str(),
                (int) (60 * mr->time_allowance), end_time + 1);
      }
      break;

    case FIRST_AFTER_START_OF_FRAME:
      gd.data_request_time = start_time;
      if(gd.model_run_time != 0 && mr->h_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST) { 
        mr->h_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                                url,
                                (int) (60 * mr->time_allowance),
                                gd.model_run_time,
                                (start_time - gd.model_run_time));
      } else {
        mr->h_mdvx->setReadTime(Mdvx::READ_FIRST_AFTER,
                                url,
                                (int) (60 * mr->time_allowance),
                                start_time-1, 0);
      }
      if(gd.debug1) {
        fprintf(stderr,
                "{ READ_VOLUME_REQUEST, \"%s\", \"%s\", %d, %ld },\n",
                url.c_str(), fieldName.c_str(),
                (int) (60 * mr->time_allowance), start_time - 1);
      }
      break;

  } // gather_data_mode

  if(mr->composite_mode) {
    // Ask for data that covers the whole vertical domain 
    mr->h_mdvx->setReadVlevelLimits((double)gd.h_win.min_ht,
                                    (double)gd.h_win.max_ht);
    mr->h_mdvx->setReadComposite();
  } else {
    // Ask for plane closest to the display's current average height
    mr->h_mdvx->setReadVlevelLimits(gd.h_win.cur_ht + mr->alt_offset,
                                    gd.h_win.cur_ht + mr->alt_offset);
  }
     
  if(_params.always_get_full_domain == 0) {
    double min_lat, max_lat, min_lon, max_lon;
    get_bounding_box(min_lat, max_lat, min_lon, max_lon); 
    if (!_params.do_not_clip_on_mdv_request) {
      mr->h_mdvx->setReadHorizLimits(min_lat, min_lon, max_lat, max_lon);
    }
  } else {
    if (!_params.do_not_clip_on_mdv_request) {
      mr->h_mdvx->setReadHorizLimits(-90.0, gd.h_win.origin_lon - 179.9999,
                                     90.0, gd.h_win.origin_lon + 179.9999);
    }
  }
  
  // Mdvx Decimation is based on the sqrt of the value. - Choose the longer edge 
  if (!_params.do_not_decimate_on_mdv_request) {
    // if(_params.aspect_ratio > 1.0) {
    mr->h_mdvx->setReadDecimate(gd.h_win.img_dim.width * gd.h_win.img_dim.width);
    // } else {
    //   mr->h_mdvx->setReadDecimate(gd.h_win.img_dim.height * gd.h_win.img_dim.height);
    // }
  }
     
  mr->h_mdvx->setReadEncodingType(Mdvx::ENCODING_ASIS);
  mr->h_mdvx->setReadCompressionType(Mdvx::COMPRESSION_ASIS);
  if(_params.request_compressed_data) {
    if(_params.request_gzip_vol_compression) {
      mr->h_mdvx->setReadCompressionType(Mdvx::COMPRESSION_GZIP_VOL);
    } else {
      mr->h_mdvx->setReadCompressionType(Mdvx::COMPRESSION_ZLIB);
    }
  }

  // request full field header for vert info

  mr->h_mdvx->setReadFieldFileHeaders();

  gd.io_info.mode = DSMDVX_DATA;
  gd.io_info.request_type = HORIZ_REQUEST;    /* Horizontal data access */
  gd.io_info.expire_time = time(0) + _params.data_timeout_secs;
  gd.io_info.busy_status = 0;
  gd.io_info.page = page;
  gd.io_info.outstanding_request = 1;
  gd.io_info.mr = mr;
  
  if(gd.debug1) {
    fprintf(stderr, "Get MDVX Horiz Plane - page : %d  -  %s\n", page, mr->url);
  }
  
  // Initiate the request - This becomes threaded if not in debug mode.
  
  mr->h_mdvx->readVolume();

  // The check_for_io function (thread) will poll for complettion of the data request.
  // Once the data's in or ithe request times out, this data is marked as valid
  
  return 0;
  
}
 
/**********************************************************************
 * REQUEST_VERT_DATA_PLANE: Query the server for a data plane
 *
 */

int mdvx_request_vert_data_plane(met_record_t *mr,
                                 time_t start_time,
                                 time_t end_time,
                                 int page)
{

  // Construct URL, check is valid.
  
  string url(_getUrl(mr));
  DsURL URL(url);  
  if(URL.isValid() != TRUE) {
    cerr << "222 Bogus URL: " << url << endl;
    mr->h_data_valid = 1;
    return -1;
  }
  URL.getURLStr();
  
  // get field name
  
  string fieldName(_getFieldName(mr));

  // add field to request
  
  mr->h_mdvx->clearRead(); 
  mr->h_mdvx->addReadField(fieldName);

  if(gd.debug1) {
    fprintf(stderr, "Get MDVX Vert Plane - page : %d  -  %s\n", page,mr->url);
    // Disable threading while in debug mode
    mr->v_mdvx->setThreadingOff();
  }
  
  // offset the request time

  start_time += (int) (mr->time_offset * 60);
  end_time += (int) (mr->time_offset * 60);
  
  switch(_params.gather_data_mode ) {
    case CLOSEST_TO_FRAME_CENTER :
      if(gd.model_run_time != 0 && mr->v_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST) { 
        mr->v_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                                url,(int)(60 * mr->time_allowance),
                                gd.model_run_time,
                                (int)start_time + ((end_time  - start_time)/2) - gd.model_run_time);
      } else {
        mr->v_mdvx->setReadTime(Mdvx::READ_CLOSEST,
                                url,(int)(60 * mr->time_allowance),
                                (int)start_time + ((end_time  - start_time)/2));
      }
      break;

    case FIRST_BEFORE_END_OF_FRAME:
      if(gd.model_run_time != 0 && mr->v_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST) { 
        mr->v_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                                url,(int)(60 * mr->time_allowance),
                                gd.model_run_time,
                                (end_time - gd.model_run_time));
      } else {
        mr->v_mdvx->setReadTime(Mdvx::READ_FIRST_BEFORE,
                                url,(int)(60 * mr->time_allowance), end_time+1,0);
      }
      break;

    case FIRST_AFTER_START_OF_FRAME:
      if(gd.model_run_time != 0 && mr->v_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST) { 
        mr->v_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                                url,(int)(60 * mr->time_allowance),
                                gd.model_run_time,
                                (start_time - gd.model_run_time));
      } else {
        mr->v_mdvx->setReadTime(Mdvx::READ_FIRST_AFTER,
                                url,(int)(60 * mr->time_allowance), start_time-1,0);
      }
      break;
  }

  mr->v_mdvx->setReadEncodingType(Mdvx::ENCODING_ASIS);
  mr->v_mdvx->setReadCompressionType(Mdvx::COMPRESSION_ASIS);

  if(_params.request_compressed_data) {
    if(_params.request_gzip_vol_compression) {
      mr->v_mdvx->setReadCompressionType(Mdvx::COMPRESSION_GZIP_VOL);
    } else {
      mr->v_mdvx->setReadCompressionType(Mdvx::COMPRESSION_ZLIB);
    }
  }

  int num_way_points = gd.h_win.route.num_segments + 1;

  for(int ii = 0; ii < num_way_points; ii++) {
    double tlat, tlon;
    gd.proj.xy2latlon(gd.h_win.route.x_world[ii],
                      gd.h_win.route.y_world[ii],
                      tlat, tlon);
    mr->v_mdvx->addReadWayPt(tlat,tlon);
    if (ii == 0) {
      // lat1 = tlat;
      // lon1 = tlon;
    } else if (ii == num_way_points - 1) {
      // lat2 = tlat;
      // lon2 = tlon;
    }
    
    if(gd.debug1) {
      fprintf(stderr,"Way pt Lat, Lon, Len: %g, %g, %g\n",
              tlat, tlon, gd.h_win.route.seg_length[ii]);
    }
    
  } // ii

  gd.io_info.mode = DSMDVX_DATA;
  gd.io_info.expire_time = time(0) + _params.data_timeout_secs;
  gd.io_info.busy_status = 0;
  gd.io_info.page = page;
  gd.io_info.request_type = VERT_REQUEST;    /* a vertical data access */
  gd.io_info.outstanding_request = 1;
  gd.io_info.mr = mr;

  // Initiate the request
  mr->v_mdvx->readVsection();

  return 0;
}

/**********************************************************************
 * REQUEST_TIME_LIST: Query the server for a Time list
 *
 */

int _getTimeList(met_record_t *mr,
                 time_t start_time,
                 time_t end_time,
                 int page)
{
  
  // Construct URL, check is valid.
  
  string url(_getUrl(mr));
  DsURL URL(url);  
  if(URL.isValid() != TRUE) {
    cerr << "333 Bogus URL: " << url << endl;
    mr->h_data_valid = 1;
    return -1;
  }
  URL.getURLStr();
  
  // offset the request time
  
  start_time += (int) (mr->time_offset * 60);
  end_time += (int) (mr->time_offset * 60);
  time_t delta = gd.epoch_end - gd.epoch_start;
  
  if(gd.debug1) {
    fprintf(stderr, "Get MDVX Time List page : %d  -  %s\n",
            page, mr->url);
    // Disable threading while in debug mode
    mr->h_mdvx->setThreadingOff();
  }

  mr->h_mdvx->clearTimeListMode();
  if(gd.model_run_time != 0 &&
     mr->h_mhdr.data_collection_type == Mdvx::DATA_FORECAST) { 
    mr->h_mdvx->setTimeListModeLead(url,
                                    gd.model_run_time);
  } else {
    mr->h_mdvx->setTimeListModeValid(url,
                                     gd.epoch_start - delta,
                                     gd.epoch_end + delta);
  }
  if(gd.debug1) {
    fprintf(stderr, "Gathering Time List for %s\n", mr->url);
  }

  char label[1024];
  snprintf(label, 1024, "Requesting time list for %s data", mr->legend_name);
  if(_params.show_data_messages) {
    gui_label_h_frame(label, 1);
  } else {
    // set_busy_state(1);
  }

  // This is important because different scales can have diffrerent
  // temporal resolutions. - Must pass in the bounding box to get an 
  // accurate time list.

  double min_lat, max_lat, min_lon, max_lon;
  get_bounding_box(min_lat, max_lat, min_lon, max_lon);
  
  // Set up the DsMdvx request object
  if (!_params.do_not_clip_on_mdv_request) {
    mr->h_mdvx->setReadHorizLimits(min_lat, min_lon, max_lat, max_lon);
  }

  // Gather time list
  gd.io_info.mode = DSMDVX_DATA;
  gd.io_info.request_type = TIMELIST_REQUEST;    /* List of data times */
  gd.io_info.expire_time = time(0) + _params.data_timeout_secs;
  gd.io_info.busy_status = 0;
  gd.io_info.page = page;
  gd.io_info.outstanding_request = 1;
  gd.io_info.mr = mr;

  if(gd.debug1) {
    fprintf(stderr,
            "{ TIME_LIST_REQUEST, \"%s\", \"%s\", %ld, %ld, %g, %g, %g, %g, %g },\n",
            url.c_str(), "none",
            gd.epoch_start - delta, gd.epoch_end + delta,
            -99.9, min_lat, min_lon, max_lat, max_lon);
  }

  // Limit List requests to given maximum number of days 
  if((gd.epoch_end - gd.epoch_start) <
     (_params.climo_max_time_span_days * 1440  * 60)) {
    if (mr->h_mdvx->compileTimeList()) {
      cout << "ERROR -CIDD:  setTimeListModeValid" << endl;
      cout << mr->h_mdvx->getErrStr();
      mr->time_list_valid = 1;
    }  
  } else {
    gd.io_info.outstanding_request = 0;
    mr->time_list_valid = 1;
  }

  return 0;  // return from this request.
  
  // When the thread is done the check_for_io function will gather and store the
  // timelist info and then set mr->time_list_valid = 1 (true);
  
}


/**********************************************************************
 * get a fully-qualified URL
 *
 */

string _getUrl(met_record_t *mr)
{

  // url from met record
  
  string url(mr->url);

  // check for tunnel
  
  if(strlen(_params.http_tunnel_url) < URL_MIN_SIZE) {
    return url;
  }

  // Using a tunnel - add the tunnel_url to the Url as a name=value pair.
  // If using the a proxy - Add the a proxy_url to the Url as a name=value pair
  
  char tmp_str[1024];
  MEM_zero(tmp_str);
  if((strlen(_params.http_proxy_url)) > URL_MIN_SIZE) {
    snprintf(tmp_str, 1024,
             "?tunnel_url=%s&proxy_url=%s",
             _params.http_tunnel_url,
             _params.http_proxy_url);
  } else {
    snprintf(tmp_str, 1024,
             "?tunnel_url=%s",
             _params.http_tunnel_url);
  }

  url += tmp_str;

  return url;

}
  
/**********************************************************************
 * get field name
 *
 */

string _getFieldName(met_record_t *mr)
{

  char field_name[512];
  MEM_zero(field_name);
  strncpy(field_name, mr->field_label, 256);
  
  // Replace carets with spaces

  char *ptr = field_name;
  while ((ptr = strchr(field_name,'^')) != NULL) {
    *ptr = ' ';
  }

  return field_name;

}


/*****************************************************************
 * AUTOSCALE_VCM: Autoscale the value-color mapping
 *        color to use
 *
 */

void autoscale_vcm( Valcolormap_t *vcm, double min, double max)
{
  int    i;
  double delta;

  delta = (max - min) / (double) vcm->nentries;

  if(delta == 0.0) delta = 0.1;

  for(i=0; i < vcm->nentries; i++) {
    vcm->vc[i]->min = min + (delta * i);
    vcm->vc[i]->max = vcm->vc[i]->min + delta;
  }

}

/**********************************************************************
 * CHECK_FOR_IO: This routine checks to see if any new data
 *  has arrived and updates the frame message
 */

void check_for_io()
{

  switch (gd.io_info.mode) {
    
    case DSMDVX_DATA: {  // Handle MDV data
      met_record_t *mr = gd.io_info.mr;
      switch(gd.io_info.request_type) {
        case HORIZ_REQUEST:
          _checkForHorizData(mr);
	  break;
        case VERT_REQUEST:
          _checkForVertData(mr);
	  break;
        case TIMELIST_REQUEST:
          _checkForTimelistData(mr);
          break;  // end of case  TIMELIST_REQUEST
      }
      break;   // End of case DXMDVX_DATA
    }
      
    case SYMPROD_DATA: { // Handle Symbolic product data 
      _checkForSymprodData();
      break;
    }
      
    default: { // do nothing
      break;
    }
      
  } // switch

}

/**********************************************************************
 * GATHER_HWIN_DATA: Collect all the data necessary for horizontal 
 *    display windows 
 */

int gather_hwin_data(int page, time_t start_time, time_t end_time)
{
  int i;
  time_t now;
  met_record_t *mr;       /* pointer to record for convienence */

  now = time(0);
  /* Check to make sure we are not currently waiting on an I/O request */
  if(gd.io_info.outstanding_request > 0) {
    if(now > gd.io_info.expire_time) {
      cancel_pending_request();
      return CIDD_FAILURE;
    }
    return INCOMPLETE;
  }

  // MAIN GRID
  mr = gd.mrec[page];    /* get pointer to data record */
  if(mr->h_data_valid == 0) {
    if(gd.debug1) {
      fprintf(stderr,
              "Requesting Data Field %d data time %s %s\n",
              page, utimstr(start_time), utimstr(end_time));
    }
    gd.data_status_changed = 0;
    if(mdvx_request_horiz_data_plane(mr,start_time,end_time,page) < 0) {
      return CIDD_FAILURE;
    } else {
      return INCOMPLETE;
    }
  }

  // TERRAIN GRID
  if(gd.layers.earth.terrain_active && mr->ds_fhdr.nz > 1) {
    mr = gd.layers.earth.terr;    /* get pointer to data record */
    if(mr->h_data_valid == 0) {
      if(gd.debug1) {
        fprintf(stderr,
                "Requesting Terrain data time %ld %ld\n",
                start_time, end_time);
      }
      gd.data_status_changed = 0;
      if(mdvx_request_horiz_data_plane(mr,start_time,end_time,page) < 0) {
        return CIDD_FAILURE;
      } else {
        return INCOMPLETE;
      }
    }
  } 

  // LANDUSE GRID
  if(gd.layers.earth.landuse_active) {
    mr = gd.layers.earth.land_use;    /* get pointer to data record */
    if(mr->h_data_valid == 0) {
      if(gd.debug1) {
        fprintf(stderr,
                "Requesting Landuse data time %ld %ld\n",
                start_time,end_time);
      }
      gd.data_status_changed = 0;
      if(mdvx_request_horiz_data_plane(mr,start_time,end_time,page) < 0) {
        return CIDD_FAILURE;
      } else {
        return INCOMPLETE;
      }
    }
  } 

  /* LAYERED GRIDS -  Request any needed data for gridded layers */
  for(i=0; i < NUM_GRID_LAYERS; i++) {
    if(gd.layers.overlay_field_on[i]) {
      mr = gd.mrec[gd.layers.overlay_field[i]];
      if(mr->h_data_valid == 0) {
        if(gd.debug1) {
          fprintf(stderr,
                  "Requesting Overlay Field %d data\n",
                  gd.layers.overlay_field[i]);
        }
        gd.data_status_changed = 0;
        mdvx_request_horiz_data_plane(mr,start_time,end_time,page);
        return INCOMPLETE;
      }
    }
  }

  /* CONTOURS: request any needed data for contours */
  for(i=0; i < NUM_CONT_LAYERS; i++) {
    if(gd.layers.cont[i].active) {
      mr = gd.mrec[gd.layers.cont[i].field];
      if(mr->h_data_valid == 0) {
        if(gd.debug1) {
          fprintf(stderr,
                  "Requesting Contour Layer %d, field %d data\n",
                  i, gd.layers.cont[i].field);
        }
        gd.data_status_changed = 0;
        mdvx_request_horiz_data_plane(mr,start_time,end_time,page);
        return INCOMPLETE;
      }
    }
  }

  // WINDS
  for(i=0; i < gd.layers.num_wind_sets; i++) {
    switch(gd.layers.wind_mode) {
      default:
      case WIND_MODE_ON:  /* gather data as usual */
        break;

      case WIND_MODE_LAST: /* Gather data for last frame only */
        if(gd.movie.cur_frame != gd.movie.end_frame) continue;
        break;

      case WIND_MODE_STILL: /* Gather data for the last frame only
                             * if the movie loop is off
                             */
        if(gd.movie.movie_on || gd.movie.cur_frame != gd.movie.end_frame) continue;
        break;
    }

    if(gd.layers.wind_vectors  && gd.layers.wind[i].active ) {
      mr = gd.layers.wind[i].wind_u;
      if(mr->h_data_valid == 0) {
        if(gd.debug1) fprintf(stderr, "Requesting Wind %d data - U\n", i);
        gd.data_status_changed = 0;
        mdvx_request_horiz_data_plane(mr,start_time,end_time,page);
        return INCOMPLETE;
      }
    
      mr = gd.layers.wind[i].wind_v;
      if(mr->h_data_valid == 0) {
        if(gd.debug1) fprintf(stderr, "Requesting Wind %d data - V\n", i);
        gd.data_status_changed = 0;
        mdvx_request_horiz_data_plane(mr,start_time,end_time,page);
        return INCOMPLETE;
      }
    
      mr = gd.layers.wind[i].wind_w;
      if(mr != NULL) {
        if(mr->h_data_valid == 0) {
          if(gd.debug1) fprintf(stderr, "Requesting Wind %d  data - W\n", i);
          gd.data_status_changed = 0;
          mdvx_request_horiz_data_plane(mr,start_time,end_time,page);
          return INCOMPLETE;
        }
      }
    }
  }

  // Native SYMPROD DATA Gather
  if(gd.prod.products_on) {
    if(_params.symprod_short_requests) { 
      return gd.prod_mgr->getData(start_time, end_time);
    } else {
      return gd.prod_mgr->getData(gd.epoch_start, gd.epoch_end);
    }
  } else {
    return CIDD_SUCCESS;
  }
    
}

/**********************************************************************
 * GATHER_VWIN_DATA: Collect all the data necessary for vertical 
 *    display windows 
 *
 */

int gather_vwin_data(int page, time_t start_time, time_t end_time)
{
  int    i;
  met_record_t *mr;       /* pointer to record for convienence */
  time_t now;

  now = time(0);
  /* Check to make sure we are not currently waiting on an I/O request */
  if(gd.io_info.outstanding_request) {
    if(now > gd.io_info.expire_time) {
      cancel_pending_request();
      return CIDD_FAILURE;
    }
    return INCOMPLETE;
  }
     
  // MAIN GRID
  mr = gd.mrec[page];    /* get pointer to data record */
  if(mr->v_data_valid == 0) {
    if(mr->h_data_valid == 0) {  // Need Header info obtained from horiz request
      if(mdvx_request_horiz_data_plane(mr,start_time,end_time,page) < 0) {
        return CIDD_FAILURE;
      } else {
        return INCOMPLETE;
      }
    }
	    
    if(gd.debug1) fprintf(stderr, "Requesting False Color Field %d data\n", page);
    gd.data_status_changed = 0;
    if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
      return CIDD_FAILURE;
    } else {
      return INCOMPLETE;
    }
  }

  // TERRAIN GRID
  if(gd.layers.earth.terrain_active) {
    mr = gd.layers.earth.terr;    /* get pointer to data record */
    if(mr->v_data_valid == 0) {
      if(gd.debug1) {
        fprintf(stderr,
                "Requesting Terrain cross section data time %ld %ld\n",
                start_time,end_time);
      }
      gd.data_status_changed = 0;
      if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
        return CIDD_FAILURE;
      } else {
        return INCOMPLETE;
      }
    }
  } 

  // Land use is not valid in cross section

  // CONTOURS
  for(i=0; i < NUM_CONT_LAYERS ; i++) {
    if(gd.layers.cont[i].active) {
      mr = gd.mrec[gd.layers.cont[i].field];
      if(mr->v_data_valid == 0) {
        if(gd.debug1) {
          fprintf(stderr,
                  "Requesting VERT Contour Field %d data\n",
                  gd.layers.cont[i].field);
        }
        gd.data_status_changed = 0;
        if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
          return CIDD_FAILURE;
        } else {
          return INCOMPLETE;
        }
      }
    }
  }

  // ROUTE WINDs, TURBULENCE, ICING
  if(gd.layers.route_wind.u_wind != NULL) {
    mr = gd.layers.route_wind.u_wind;
    if(mr->v_data_valid == 0) {
      if(gd.debug1) {
        fprintf(stderr, "Requesting  %s Route data \n",mr->legend_name);
      }
      gd.data_status_changed = 0;
      if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
        return CIDD_FAILURE;
      } else {
        return INCOMPLETE;
      }
    }
  }

  if(gd.layers.route_wind.v_wind != NULL) {
    mr = gd.layers.route_wind.v_wind;
    if(mr->v_data_valid == 0) {
      if(gd.debug1) {
        fprintf(stderr, "Requesting  %s Route data \n",mr->legend_name);
      }
      gd.data_status_changed = 0;
      if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
        return CIDD_FAILURE;
      } else {
        return INCOMPLETE;
      }
    }
  }

  if(gd.layers.route_wind.turb != NULL) {
    mr = gd.layers.route_wind.turb;
    if(mr->v_data_valid == 0) {
      if(gd.debug1) {
        fprintf(stderr, "Requesting - %s Route data \n",mr->legend_name);
      }
      gd.data_status_changed = 0;
      if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
        return CIDD_FAILURE;
      } else {
        return INCOMPLETE;
      }
    }
  }

  if(gd.layers.route_wind.icing != NULL) {
    mr = gd.layers.route_wind.icing;
    if(mr->v_data_valid == 0) {
      if(gd.debug1) {
        fprintf(stderr, "Requesting Route Icing  - %s data\n",mr->legend_name);
      }
      gd.data_status_changed = 0;
      if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
        return CIDD_FAILURE;
      } else {
        return INCOMPLETE;
      }
    }
  }
  
  // WINDS
  for(i=0; i < gd.layers.num_wind_sets; i++) {
    if(gd.layers.wind_vectors  && gd.layers.wind[i].active) {
      mr = gd.layers.wind[i].wind_u;
      if(mr->v_data_valid == 0) {
        if(gd.debug1) {
          fprintf(stderr, "Requesting Wind %d - %s data - U\n", i,mr->legend_name);
        }
        gd.data_status_changed = 0;
        if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
          return CIDD_FAILURE;
        } else {
          return INCOMPLETE;
        }
      }
    
      mr = gd.layers.wind[i].wind_v;
      if(mr->v_data_valid == 0) {
        if(gd.debug1) {
          fprintf(stderr, "Requesting Wind %d - %s  data - V\n", i,mr->legend_name);
        }
        gd.data_status_changed = 0;
        if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
          return CIDD_FAILURE;
        } else {
          return INCOMPLETE;
        }
      }
    
      mr = gd.layers.wind[i].wind_w;
      if(mr != NULL) {
        if(mr->v_data_valid == 0) {
          if(gd.debug1) {
            fprintf(stderr, "Requesting Wind %d - %s data - W\n", i,mr->legend_name);
          }
          gd.data_status_changed = 0;
          if(mdvx_request_vert_data_plane(mr,start_time,end_time,page) < 0) {
            return CIDD_FAILURE;
          } else {
            return INCOMPLETE;
          }
        }
      }
    }
  }

  return CIDD_SUCCESS;
}

/**********************************************************************
 * CANCEL_PENDING_REQUEST: 
 *
 */

void cancel_pending_request()
{
  if(gd.io_info.outstanding_request == 0) return;

  if(gd.io_info.mode != DSMDVX_DATA) {
    close(gd.io_info.fd);
    if(gd.io_info.incoming_data_pointer != NULL) {
      free(gd.io_info.incoming_data_pointer);
      gd.io_info.incoming_data_pointer = NULL;
    }
  } else {
    switch(gd.io_info.request_type) {
      case HORIZ_REQUEST:
      case TIMELIST_REQUEST:
        gd.io_info.mr->h_mdvx->cancelThread();
        break;

      case VERT_REQUEST: 
        gd.io_info.mr->v_mdvx->cancelThread();
        break;

      case SYMPROD_REQUEST: 
        DsSpdbThreaded *spdb = gd.io_info.prod->getSpdbObj();
        spdb->cancelThread();
        break;
    }
  }

  gd.io_info.outstanding_request = 0;
  gd.io_info.expire_time = time(0) + _params.data_timeout_secs;

  if(gd.debug1) fprintf(stderr,"Timeout during data service access - giving up\n");
  if(!_params.run_once_and_exit)  PMU_auto_register("Server Request Timeout");
  if(_params.show_data_messages) gui_label_h_frame("Data Service Request Timed Out!... Retrying... ",-1);
  add_message_to_status_win("Data Service Request Timed Out!... Retrying... ",1);
  set_busy_state(0);

}

/*************************************************************************
 * Set proper flags which switching fields 
 */
void set_field(int value)
{
  int i;
  int tmp;
  static int last_page = 0;
  static int last_value = 0;
  static int cur_value = 0;
  
  if(value < 0) {
    tmp = last_page;
    last_page = gd.h_win.page;
    gd.h_win.page = tmp;
    tmp = cur_value;
    cur_value = last_value;
    value = last_value;
    last_value = tmp;
  } else {
    last_page = gd.h_win.page;
    last_value = cur_value;
    cur_value = value;
    gd.h_win.page = gd.field_index[value];
    cerr << "FFFFFFFFFFFF value, new page: " << cur_value << ", " << gd.h_win.page << endl;
  }
  
  for(i=0; i < gd.num_datafields; i++) {
    if(gd.mrec[i]->auto_render == 0) gd.h_win.redraw[i] = 1;
  }
  
  if(gd.mrec[gd.h_win.page]->auto_render && 
     gd.h_win.page_pdev[gd.h_win.page] != 0 &&
     gd.h_win.redraw[gd.h_win.page] == 0) {
    
    save_h_movie_frame(gd.movie.cur_frame,
                       gd.h_win.page_pdev[gd.h_win.page],
                       gd.h_win.page);
  }
  
  for(i=0; i < MAX_FRAMES; i++) {
    gd.movie.frame[i].redraw_horiz = 1;
  }
  
  if(gd.movie.movie_on ) {
    reset_data_valid_flags(1,0);
  }
  
  // xv_set(gd.data_pu->data_st,PANEL_VALUE,value,NULL);
  
  /* make sure the horiz window's slider has the correct label */
  set_height_label();

}

/**********************************************************************
 * CHECK HORIZ REQUEST: This routine checks to see if any new horiz data
 * has arrived
 */

static void _checkForHorizData(met_record_t *mr)

{
  // Check if our data access thread  is done
  if (!mr->h_mdvx->getThreadDone()) {
    // still waiting
    // Display a progress message
    char label[256];
    double comp = mr->h_mdvx->getPercentReadComplete();
    long n_read = mr->h_mdvx->getNbytesReadDone();
    
    if(n_read <= 1) {
      snprintf(label,256,"Waiting for %s service  %ld -  secs before timeout",
               mr->legend_name,
               (gd.io_info.expire_time - time(0)));
    } else {
      if(n_read > gd.io_info.last_read) {

        struct timeval tm;
        struct timezone tz;
        gettimeofday(&tm,&tz);
        
        double bps = (n_read - gd.io_info.last_read) /  elapsed_time(gd.io_info.last_time,tm);
        if(bps < 512) {
          snprintf(label,256,"READING %s Data  %.0f%% complete  - %.2g bytes/sec",
                   mr->legend_name,
                   comp,bps);
        } else {
          snprintf(label,256,"READING %s Data  %.0f%% complete  - %.2g Kb/sec",
                   mr->legend_name,
                   comp,bps/1000);
        }
        gd.io_info.expire_time++;
        gd.io_info.last_read = n_read;
        gd.io_info.last_time = tm;
      } else {
        snprintf(label,256,"READING %s Data  %.0f%% complete",
                 mr->legend_name,comp);
      }
    }
    if(_params.show_data_messages) {
      gui_label_h_frame(label,1);
    }
    return;
  }

  // OK - Data is now in.
    
  if(mr->h_mdvx->getThreadRetVal()) { // Error

    mr->h_data = NULL;
    mr->h_fl32_data = NULL;
    
    // Save the master header for the file, even if we couldn't
    // get data for this field.  This is needed in case we are
    // dealing with forecast data
      
    mr->h_mhdr = mr->h_mdvx->getMasterHeader();
      
    // If No data was available, mark this condition valid
    //if(mr->h_mdvx->getNoFilesFoundOnRead()) mr->h_data_valid = 1;
    mr->h_data_valid = 1;
    mr->last_collected = time(0);
      
    // Indicate data is no longer pending
    gd.io_info.busy_status = 0;
    gd.io_info.outstanding_request = 0;
    gd.io_info.request_type = 0;
    gd.h_win.redraw[gd.io_info.page] = 1;
      
    if(gd.debug || gd.debug1) {
      fprintf(stderr,"Aborted Read: Error %d - %s\n",
              mr->h_mdvx->getThreadRetVal(),
              mr->h_mdvx->getErrStr().c_str());
    }
    if(_params.show_data_messages) gui_label_h_frame("No Data Received",-1);
    add_message_to_status_win("Aborted Read",1);
    add_message_to_status_win((char *) mr->h_mdvx->getErrStr().c_str(),0);
      
    return;

  }
    
  if(gd.debug || gd.debug1 || gd.debug2) {
    fprintf(stderr,"Data Returned from: %s\n", mr->h_mdvx->getPathInUse().c_str());
  }
  
  // check for field
  
  MdvxField *hfld;
  if((hfld = mr->h_mdvx->getFieldByNum(0)) == NULL) {
    fprintf(stderr,"Aborted Read: Error %d - %s\n", 
            mr->h_mdvx->getThreadRetVal(),
            mr->h_mdvx->getErrStr().c_str());
    gd.io_info.busy_status = 0;
    gd.io_info.outstanding_request = 0;
    gd.io_info.request_type = 0;
    gd.h_win.redraw[gd.io_info.page] = 1;
    mr->h_data_valid = 1;
    mr->last_collected = time(0);
    return;
  }
  
  // Indicate data update is in progress.
  
  gd.io_info.busy_status = 1;
  
  Mdvx::field_header_t fh = hfld->getFieldHeader();
  if(fh.encoding_type != Mdvx::ENCODING_RGBA32) {

    *mr->h_mdvx_int16 = *hfld; // Copy for INT16 data
    
    if(mr->h_vcm.nentries < 2 || fh.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
      
      mr->h_mdvx_int16->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_NONE);

    } else {
      
      // Convert the copy to - Decompressed INT16 - Covering the range of the colorscale
      double range = (mr->h_vcm.vc[mr->h_vcm.nentries-1]->max - mr->h_vcm.vc[0]->min);
      double scale = range / (MAX_COLOR_CELLS -2);
      double bias = mr->h_vcm.vc[0]->min - (2 * scale); // Preserve 0, 1 as legitimate NAN values
        
      mr->h_mdvx_int16->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_NONE,
                                    Mdvx::SCALING_SPECIFIED,scale,bias);
    }
      
    // Record where int8 data is in memory. - Used for fast polygon fills.
    mr->h_data = (unsigned short *) mr->h_mdvx_int16->getVol();
    
    // Convert the AS-IS to 32 bits float. - Used for Contouring, Data reporting.
    (mr->h_mdvx->getFieldByNum(0))->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
    // Record where float data is in memory.
    mr->h_fl32_data = (fl32  *) mr->h_mdvx->getFieldByNum(0)->getVol();
      
    // Find Headers for quick reference
    mr->h_mhdr = mr->h_mdvx->getMasterHeader();
    mr->h_fhdr = mr->h_mdvx_int16->getFieldHeader();
    mr->h_vhdr = mr->h_mdvx_int16->getVlevelHeader();

  } else {
      
    // Decompress
    hfld->convertType(Mdvx::ENCODING_ASIS, Mdvx::COMPRESSION_NONE);
      
    // Record where data is in memory. 
    mr->h_data = (unsigned short *) hfld->getVol();
      
    // Record where float data is in memory.
    mr->h_fl32_data = (fl32  *) hfld->getVol();
      
    // Find Headers for quick reference
    mr->h_mhdr = mr->h_mdvx->getMasterHeader();
    mr->h_fhdr = hfld->getFieldHeader();
    mr->h_vhdr = hfld->getVlevelHeader();

  } // compression
    
  // Init projection

  mr->proj->init(mr->h_fhdr);
    
  // condition longitudes to be in same hemisphere
  // as origin
  mr->proj->setConditionLon2Origin(true);
    
  // Implemented for MOBILE RADARS - 
  if(_params.domain_follows_data && mr == gd.mrec[gd.h_win.page] ) { // Only for the primary field
    double dx,locx;
    double dy,locy;
    int index = gd.h_win.zoom_level;
      
    if(mr->h_fhdr.proj_origin_lat != gd.h_win.origin_lat ||
       mr->h_fhdr.proj_origin_lon != gd.h_win.origin_lon) {
        
      dx = gd.h_win.zmax_x[index] - gd.h_win.zmin_x[index];
      dy = gd.h_win.zmax_y[index] - gd.h_win.zmin_y[index];
        
      switch(gd.display_projection) {
        case Mdvx::PROJ_LATLON:
          gd.h_win.origin_lat = mr->h_fhdr.proj_origin_lat;
          gd.h_win.origin_lon = mr->h_fhdr.proj_origin_lon;
            
          gd.h_win.zmin_x[index] = mr->h_fhdr.proj_origin_lon - (dx / 2.0);
          gd.h_win.zmax_x[index] = mr->h_fhdr.proj_origin_lon + (dx / 2.0);
          gd.h_win.zmin_y[index] = mr->h_fhdr.proj_origin_lat - (dy / 2.0);
          gd.h_win.zmax_y[index] = mr->h_fhdr.proj_origin_lat + (dy / 2.0);
            
          break;
            
        default:
          gd.proj.latlon2xy(mr->h_fhdr.proj_origin_lat,mr->h_fhdr.proj_origin_lon,locx,locy);
            
          gd.h_win.zmin_x[index] = locx - (dx / 2.0);
          gd.h_win.zmax_x[index] = locx + (dx / 2.0);
          gd.h_win.zmin_y[index] = locy - (dy / 2.0);
          gd.h_win.zmax_y[index] = locy + (dy / 2.0);
            
          break;
            
      }
      /* Set current area to our indicated zoom area */
      gd.h_win.cmin_x = gd.h_win.zmin_x[index];
      gd.h_win.cmax_x = gd.h_win.zmax_x[index];
      gd.h_win.cmin_y = gd.h_win.zmin_y[index];
      gd.h_win.cmax_y = gd.h_win.zmax_y[index];
	
      reset_time_list_valid_flags();
      reset_data_valid_flags(1,0);
      reset_terrain_valid_flags(1,0);
      set_redraw_flags(1,0);
      mr->h_data_valid = 1;  // This field is still valid, though
      mr->time_list_valid = 1;
	
	
    }
  }
  
  if (gd.debug1) {
    cerr << "nx, ny: "
         << mr->h_fhdr.nx << ", "
         << mr->h_fhdr.ny << endl;
    cerr << "dx, dy: "
         << mr->h_fhdr.grid_dx << ", "
         << mr->h_fhdr.grid_dy << endl;
    cerr << "minx, miny: "
         << mr->h_fhdr.grid_minx << ", "
         << mr->h_fhdr.grid_miny << endl;
    cerr << "maxx, maxy: "
         << mr->h_fhdr.grid_minx + (mr->h_fhdr.nx - 1) * mr->h_fhdr.grid_dx << ", "
         << mr->h_fhdr.grid_miny + (mr->h_fhdr.ny - 1) * mr->h_fhdr.grid_dy << endl;
  }
    
  // Punt and use the field headers if the file headers are not avail
  if(hfld->getFieldHeaderFile() == NULL) 
    mr->ds_fhdr = (hfld->getFieldHeader());
  else 
    mr->ds_fhdr = *(hfld->getFieldHeaderFile());
    
  if(hfld->getVlevelHeaderFile() == NULL) 
    mr->ds_vhdr = (hfld->getVlevelHeader());
  else 
    mr->ds_vhdr = *(hfld->getVlevelHeaderFile());
    
  // Recompute the color scale lookup table if necessary
  if(mr->h_fhdr.scale != mr->h_last_scale ||
     mr->h_fhdr.bias != mr->h_last_bias   ||
     mr->h_fhdr.missing_data_value != mr->h_last_missing   ||
     mr->h_fhdr.bad_data_value != mr->h_last_bad ||
     mr->h_fhdr.transform_type != mr->h_last_transform ) {
      
    if(mr->auto_scale)
      autoscale_vcm(&(mr->h_vcm), mr->h_fhdr.min_value, mr->h_fhdr.max_value);
      
    if(fh.encoding_type != Mdvx::ENCODING_RGBA32) {
#ifdef NOTYET
      /* Remap the data values onto the colorscale */
      setup_color_mapping(&(mr->h_vcm),
                          mr->h_fhdr.scale,
                          mr->h_fhdr.bias,
                          mr->h_fhdr.transform_type,
                          mr->h_fhdr.bad_data_value,
                          mr->h_fhdr.missing_data_value);
#endif
    }
    // Update last values
    mr->h_last_scale = mr->h_fhdr.scale;
    mr->h_last_bias = mr->h_fhdr.bias;
    mr->h_last_missing = mr->h_fhdr.missing_data_value;
    mr->h_last_bad = mr->h_fhdr.bad_data_value;
    mr->h_last_transform = mr->h_fhdr.transform_type;
  }
    
  // Compute the vertical levels 
  mr->plane = 0;
  for(int i=0; i < mr->ds_fhdr.nz; i++) { 
    // Find out which plane we received
    if(mr->ds_vhdr.level[i] == mr->h_vhdr.level[0]) mr->plane = i;
    if(i == 0) { // Lowest plane 
      double delta = (mr->ds_vhdr.level[i+1] - mr->ds_vhdr.level[i]) / 2.0;
      mr->vert[i].min = mr->ds_vhdr.level[0] - delta;
      mr->vert[i].cent = mr->ds_vhdr.level[0];
      mr->vert[i].max = mr->ds_vhdr.level[0] + delta;
        
    } else if (i == mr->ds_fhdr.nz -1) { // highest plane
      double delta = (mr->ds_vhdr.level[i] - mr->ds_vhdr.level[i-1]) / 2.0;
      mr->vert[i].min = mr->ds_vhdr.level[i] - delta;
      mr->vert[i].cent = mr->ds_vhdr.level[i];
      mr->vert[i].max = mr->ds_vhdr.level[i] + delta;
        
    } else { // Middle planes
      double delta = (mr->ds_vhdr.level[i] - mr->ds_vhdr.level[i-1]) / 2.0;
      mr->vert[i].min = mr->ds_vhdr.level[i] - delta;
      mr->vert[i].cent = mr->ds_vhdr.level[i];
        
      delta = (mr->ds_vhdr.level[i+1] - mr->ds_vhdr.level[i]) / 2.0;
      mr->vert[i].max = mr->ds_vhdr.level[i] + delta;
    }
  }
    
    
  // Record the dimensional Units of the volume
  STRcopy(mr->units_label_cols,
          mr->h_mdvx->projType2XUnits(mr->h_fhdr.proj_type),LABEL_LENGTH);
  STRcopy(mr->units_label_rows,
          mr->h_mdvx->projType2YUnits(mr->h_fhdr.proj_type),LABEL_LENGTH);
  STRcopy(mr->units_label_sects,
          mr->h_mdvx->vertTypeZUnits(mr->h_vhdr.type[0]),LABEL_LENGTH);
    
  // Record the date
  UTIMunix_to_date(mr->h_mhdr.time_centroid,&mr->h_date);
    
  mr->h_data_valid = 1;
  mr->last_collected = time(0);
  gd.h_win.redraw[gd.io_info.page] = 1;
  // Indicate its safe to use the data
  gd.io_info.busy_status = 0;
  // Indicate data is no longer pending
  gd.io_info.outstanding_request = 0;
  gd.io_info.request_type = 0;
    
  if (_params.show_data_messages) {
    gui_label_h_frame("Done",-1);
  } else {
    set_busy_state(0);
  }

}

/**********************************************************************
 * CHECK VERT REQUEST: This routine checks to see if any new
 * vert section data has arrived
 */

static void _checkForVertData(met_record_t *mr)

{

  if(!mr->v_mdvx->getThreadDone()) {

    // still in progress

    double comp = mr->v_mdvx->getPercentReadComplete();
    long n_read = mr->v_mdvx->getNbytesReadDone();
    char label[256];
    
    if(n_read <= 1) {
      snprintf(label,256,"Waiting for %s service  %ld -  secs before timeout",
               mr->legend_name,
               (gd.io_info.expire_time - time(0)));
    } else {
      if(n_read > gd.io_info.last_read) {

        struct timeval tm;
        struct timezone tz;
        gettimeofday(&tm,&tz);
        
        double bps = (n_read - gd.io_info.last_read) /  elapsed_time(gd.io_info.last_time,tm);
        if(bps < 512) {
          snprintf(label,256,"Reading %s Data  %.0f%% complete  - %.2g bytes/sec",
                   mr->legend_name,
                   comp,bps);
        } else {
          snprintf(label,256,"Reading %s Data  %.0f%% complete  - %.2g Kb/sec",
                   mr->legend_name,
                   comp,bps/1000);
        }
        gd.io_info.expire_time++;
        gd.io_info.last_read = n_read;
        gd.io_info.last_time = tm;
      } else {
        snprintf(label,256,"READING %s Data  %.0f%% complete",
                 mr->legend_name,comp);
      }
    }
    if(_params.show_data_messages) {
      gui_label_h_frame(label,1);
    }

    return;
    
  } // if(!mr->v_mdvx->getThreadDone())
    
  // read error?
  
  if(mr->v_mdvx->getThreadRetVal() || mr->v_mdvx->getNFields() < 1) {
    
    mr->v_data = NULL;
    mr->v_fl32_data = NULL;
    
    // If No data was available, mark this condition valid
    //if(mr->v_mdvx->getNoFilesFoundOnRead()) mr->v_data_valid = 1;
    mr->v_data_valid = 1;
    
    // Indicate data is no longer pending
    gd.io_info.busy_status = 0;
    gd.io_info.outstanding_request = 0;
    gd.io_info.request_type = 0;
    gd.h_win.redraw[gd.io_info.page] = 1;
    
    if(gd.debug || gd.debug1) {
      fprintf(stderr,"Aborted Read: Error %d - %s\n",
              mr->v_mdvx->getThreadRetVal(),
              mr->v_mdvx->getErrStr().c_str());
    }
    if(_params.show_data_messages) gui_label_h_frame("No Cross Section Data Received - Aborting",-1);
    add_message_to_status_win("No Cross Section Data Received",0);
    add_message_to_status_win((char *) mr->v_mdvx->getErrStr().c_str(),1);
    
    return;
  }

  // OK - Data is now in.
  
  *mr->v_mdvx_int16 = *mr->v_mdvx->getFieldByNum(0); // Copy for INT16 data
  if(mr->v_vcm.nentries < 2 || mr->v_fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
    mr->v_mdvx_int16->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_NONE);
  } else {
    // Convert the copy to - Decompressed INT16 - Covering the range of the colorscale
    double range = (mr->v_vcm.vc[mr->v_vcm.nentries-1]->max - mr->v_vcm.vc[0]->min);
    double scale = range / (MAX_COLOR_CELLS -2);
    double bias = mr->v_vcm.vc[0]->min - (2 * scale); // Preserve 0, 1 as legitimate NAN values
    mr->v_mdvx_int16->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_NONE,
                                  Mdvx::SCALING_SPECIFIED,scale,bias);
  }
  gd.io_info.busy_status = 1;
  
  // Record where the data are.
  mr->v_data = (unsigned short *) mr->v_mdvx_int16->getVol();
  
  // Convert the AS-IS Encoding to fl32 for Contouring and reporting.
  (mr->v_mdvx->getFieldByNum(0))->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  // Record where the fl32 data are.
  mr->v_fl32_data = (fl32 *) mr->v_mdvx->getFieldByNum(0)->getVol();
  
  mr->v_mhdr = mr->v_mdvx->getMasterHeader();
  mr->v_fhdr = mr->v_mdvx_int16->getFieldHeader();
  mr->v_vhdr = mr->v_mdvx_int16->getVlevelHeader();
  
  // Punt and use the field headers if the file headers are not avail
  /* if(mr->v_mdvx_int16->getFieldHeaderFile() == NULL) 
     mr->ds_fhdr = (mr->v_mdvx_int16->getFieldHeader());
     else 
     mr->ds_fhdr = *(mr->v_mdvx_int16->getFieldHeaderFile());
  */
  
  if(mr->v_fhdr.scale != mr->v_last_scale ||
     mr->v_fhdr.bias != mr->v_last_bias ||
     mr->v_fhdr.bad_data_value !=  mr->v_last_bad ||
     mr->v_fhdr.missing_data_value !=  mr->v_last_missing ||
     mr->v_fhdr.transform_type != mr->v_last_transform ) {

    if(mr->auto_scale)
      autoscale_vcm(&(mr->v_vcm), mr->v_fhdr.min_value, mr->v_fhdr.max_value);
    
    /* Remap the data values onto the colorscale */
#ifdef NOTYET
    setup_color_mapping(&(mr->v_vcm),
                        mr->v_fhdr.scale,
                        mr->v_fhdr.bias,
                        mr->v_fhdr.transform_type,
                        mr->v_fhdr.bad_data_value,
                        mr->v_fhdr.missing_data_value);
#endif
    
    // Update last values
    mr->v_last_scale = mr->v_fhdr.scale;
    mr->v_last_bias = mr->v_fhdr.bias;
    mr->v_last_bad = mr->v_fhdr.bad_data_value;
    mr->v_last_missing = mr->v_fhdr.missing_data_value;
    mr->v_last_transform = mr->v_fhdr.transform_type;
  }
  
  // Record the date
  UTIMunix_to_date(mr->v_mhdr.time_centroid,&mr->v_date);
  
  // Record the actual way points returned, if it's the key field .
  if (gd.mrec[gd.h_win.page] == mr) {
    const vector<Mdvx::vsect_waypt_t> way_pts = mr->v_mdvx->getVsectWayPts();
    
    gd.h_win.route.num_segments = way_pts.size() -1;
    if(gd.debug || gd.debug1 || gd.debug2) fprintf(stderr,"Returned %d WayPoints\n",(int)way_pts.size());
    gd.h_win.route.total_length = 0;
    for(int i=0; i < (int) way_pts.size(); i++) {
      if(gd.debug1 || gd.debug2)
        fprintf(stderr,"WayPoint %d: Lat,Lon: %g,%g  \n",
                i, way_pts[i].lat,  way_pts[i].lon);
      
      gd.proj.latlon2xy(way_pts[i].lat,  way_pts[i].lon,
                        gd.h_win.route.x_world[i],gd.h_win.route.y_world[i]);
      if(i > 0) {
        gd.h_win.route.seg_length[i-1] =
          disp_proj_dist(gd.h_win.route.x_world[i],
                         gd.h_win.route.y_world[i],
                         gd.h_win.route.x_world[i-1],
                         gd.h_win.route.y_world[i-1]);
        gd.h_win.route.total_length += gd.h_win.route.seg_length[i-1];
      }
    }
  }
  
  mr->v_data_valid = 1;
  //gd.v_win.cmin_x = 0.0;
  //gd.v_win.cmax_x = gd.h_win.route.total_length;
  //gd.v_win.cmin_y =  gd.v_win.min_ht;
  //gd.v_win.cmax_y =  gd.v_win.max_ht;
  
  gd.v_win.redraw[gd.io_info.page] = 1;
  gd.io_info.busy_status = 0;
  gd.io_info.outstanding_request = 0;
  gd.io_info.request_type = 0;
  
  if(_params.show_data_messages) {
    gui_label_h_frame("Done",-1);
  } else {
    set_busy_state(0);
  }

}

/**********************************************************************
 * CHECK for TIMELIST DATA: This routine checks to see if any new
 * time list data has arrived
 */

static void _checkForTimelistData(met_record_t *mr)

{

  // Check if our data access thread  is done

  if(!mr->h_mdvx->getThreadDone()) {
    // still waiting for data
    char label[256];
    snprintf(label,256,"Waiting for %s Data Index  %ld -  secs before timeout",
             mr->legend_name,
             (gd.io_info.expire_time - time(0)));
    if(_params.show_data_messages) {
      gui_label_h_frame(label,1);
    }
    return;
  }

  // error?
  
  if(mr->h_mdvx->getThreadRetVal()) {
    gd.io_info.busy_status = 0;
    gd.io_info.outstanding_request = 0;
    gd.io_info.request_type = 0;
    gd.io_info.mode = 0;
    mr->time_list_valid = 1;
    if(gd.debug || gd.debug1) {
      fprintf(stderr,"TIMELIST_REQUEST Error %d - %s\n",
              mr->h_mdvx->getThreadRetVal(),
              mr->h_mdvx->getErrStr().c_str());
    }
    add_message_to_status_win("TIMELIST_REQUEST error",1);
    add_message_to_status_win((char *) mr->h_mdvx->getErrStr().c_str(),0);
    if(!_params.show_data_messages) {
      set_busy_state(0);
    }
    return;   
  }

  // success
  
  // get a pointer to the time list
  const vector<time_t> &timeList = mr->h_mdvx->getTimeList();
  
  // allocate enough space
  if(mr->time_list.num_alloc_entries == 0 && timeList.size() > 0 ) {
    mr->time_list.tim = (time_t *) calloc(timeList.size(),sizeof(time_t));
    if(mr->time_list.tim != NULL) mr->time_list.num_alloc_entries  = timeList.size();
    
  } else if (mr->time_list.num_alloc_entries < timeList.size()) {
    mr->time_list.tim = (time_t *) realloc(mr->time_list.tim,(timeList.size() * sizeof(time_t)));
    if(mr->time_list.tim != NULL) mr->time_list.num_alloc_entries  = timeList.size();
  }
  
  // copy the time list
  mr->time_list.num_entries = timeList.size();
  for (size_t i = 0; i < mr->time_list.num_entries; i++) {
    mr->time_list.tim[i] = timeList[i];
    
  }
  
  if(gd.debug1) {
    fprintf(stderr, "Found %d  Time List entries\n",
            mr->time_list.num_entries);
  }
  
  // indicate we're done 
  mr->time_list_valid = 1;
  gd.io_info.outstanding_request = 0;
  gd.io_info.request_type = 0;
  gd.io_info.mode = 0;
  if(_params.show_data_messages) {
    gui_label_h_frame("Done",-1);
  } else {
    set_busy_state(0);
  }

  return;

}
/**********************************************************************
 * CHECK for SYMPROD DATA: This routine checks to see if any new
 * symbolic product data has arrived
 */

static void _checkForSymprodData()

{
  
  if(gd.io_info.prod == NULL) {
    return;
  }
 
  // first check for times. Break if the time refs have not been
  // processed, so that io_mode is not reset until both the times
  // and data are in
  
  DsSpdbThreaded *spdbTimes = gd.io_info.prod->getSpdbTimesObj();
  if(spdbTimes->getThreadDone()) {
    if (gd.io_info.prod->processTimeRefs()) {
      return;
    }
  } else {
    return;
  }

  // Check for data

  DsSpdbThreaded *spdb = gd.io_info.prod->getSpdbObj();

  if(!spdb->getThreadDone()) {

    double comp = spdb->getPercentComplete();
    long n_read = spdb->getNbytesDone();
    char label[256];

    if(n_read <= 1) {
      snprintf(label,256,"Waiting for %s service  %ld -  secs before timeout",
               gd.io_info.prod->_prodInfo.menu_label,
               (gd.io_info.expire_time - time(0)));
    } else {
      if(n_read > gd.io_info.last_read) {
        struct timeval tm;
        struct timezone tz;
        gettimeofday(&tm,&tz);
        double bps = (n_read - gd.io_info.last_read) /  elapsed_time(gd.io_info.last_time,tm);
        if(bps < 512) {
          snprintf(label,256,"Reading %s Data  %.0f%% complete  - %.2g bytes/sec",
                   gd.io_info.prod->_prodInfo.menu_label,
                   comp,bps);
        } else {
          snprintf(label,256,"Reading %s Data  %.0f%% complete  - %.2g Kb/sec",
                   gd.io_info.prod->_prodInfo.menu_label,
                   comp,bps/1000);
        }

        gd.io_info.last_read = n_read;
        gd.io_info.last_time = tm;
        gd.io_info.expire_time++;
      } else {
        snprintf(label,256,"Reading %s Data  %.0f%% complete",
                 gd.io_info.prod->_prodInfo.menu_label, comp);
      }
    }
    if(_params.show_data_messages) {
      gui_label_h_frame(label,1);
    }

    return;
    
  }
  // check for error

  if(spdb->getThreadRetVal()) {
    
    gd.io_info.busy_status = 0;
    gd.io_info.outstanding_request = 0;
    gd.io_info.request_type = 0;
    gd.io_info.mode = 0;
    gd.io_info.prod->_data_valid = 1;
    
    if(gd.debug || gd.debug1) {
      fprintf(stderr,"Symprod Read Error: %d - %s\n",
              spdb->getThreadRetVal(),
              spdb->getErrStr().c_str());
    }
    if(_params.show_data_messages) {
      gui_label_h_frame("Error Reading SYMPROD Data - Aborting",-1);
    }
    add_message_to_status_win("Error Reading SYMPROD Data",0);
    add_message_to_status_win((char *) spdb->getErrStr().c_str(),1);
    
    return;
  }

  // good data
  
  gd.io_info.busy_status = 1;
  // store the Symprod data internally - Indicate the data's valid
  gd.io_info.prod->processChunks();
  
  // Indicate we're done
  gd.io_info.busy_status = 0;
  
  gd.io_info.outstanding_request = 0;
  gd.io_info.request_type = 0;
  gd.io_info.mode = 0;
  if(_params.show_data_messages) {
    gui_label_h_frame("Done",-1);
  } else {
    set_busy_state(0);
  }

}

