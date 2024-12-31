/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
///////////////////////////////////////////////////////////////
// MetRecord.cc
//
// Field data object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2024
//
///////////////////////////////////////////////////////////////

#include <QMutexLocker>
#include <qtplot/ColorMap.hh>
#include "cidd.h"
#include "MetRecord.hh"

///////////////////////////////////////////////
// constructor

MetRecord::MetRecord()

{
  
  // initialize
  
  plane = 0;
  currently_displayed = 0;
  auto_render = 0;
  render_method = 0;
  composite_mode = 0;
  auto_scale = 0;
  use_landuse = 0;
  num_display_pts = 0;
  last_collected = 0;
  h_data_valid = 0;
  v_data_valid = 0;
  time_list_valid = 0;
  vert_type = 0;
  alt_offset = 0.0;
  detail_thresh_min = 0.0;
  detail_thresh_max = 0.0;
  
  // MEM_zero(vert);
  
  ht_pixel = 0.0;
  y_intercept = 0.0;
  
  last_elev = NULL;
  elev_size = 0;
  
  h_last_scale = 0.0;
  h_last_bias = 0.0;
  h_last_missing = 0.0;
  h_last_bad = 0.0;
  h_last_transform = 0;
  v_last_scale = 0.0;
  v_last_bias = 0.0;
  v_last_missing = 0.0;
  v_last_bad = 0.0;
  v_last_transform = 0;
  
  cscale_min = 0.0;
  cscale_delta = 0.0;
  overlay_min = 0.0;
  overlay_max = 0.0;
  cont_low = 0.0;
  cont_high = 0.0;
  cont_interv = 0.0;
  
  time_allowance = 0.0;
  time_offset = 0.0;
  
  MEM_zero(units_label_cols);
  MEM_zero(units_label_rows);
  MEM_zero(units_label_sects);
  MEM_zero(vunits_label_cols);
  MEM_zero(vunits_label_rows);
  MEM_zero(vunits_label_sects);
  MEM_zero(field_units);
  MEM_zero(button_name);
  MEM_zero(legend_name);
  MEM_zero(field_label);
  MEM_zero(url);
  MEM_zero(color_file);
  
  h_data = NULL;
  v_data = NULL;
  
  h_fl32_data = NULL;
  v_fl32_data = NULL;
  
  h_date.setToNever();
  v_date.setToNever();
  
  proj = NULL;
  
  h_mdvx = NULL;
  h_mdvx_int16 = NULL;
  
  MEM_zero(h_mhdr);
  MEM_zero(h_fhdr);
  MEM_zero(h_vhdr); 
  
  MEM_zero(ds_fhdr);
  MEM_zero(ds_vhdr);
  
  v_mdvx = NULL;
  v_mdvx_int16 = NULL;
  
  MEM_zero(v_mhdr);
  MEM_zero(v_fhdr);
  MEM_zero(v_vhdr);
  
  colorMap = NULL;
  
  _zlevelReq = -9999.0;
  _validH = false;
  _validV = false;
  _newH = false;
  _newV = false;

}

/**********************************************************************
 * REQUEST_HORIZ_DATA_PLANE: Get data for a plane
 */

int MetRecord::requestHorizPlane(time_t start_time,
                                 time_t end_time,
                                 int page)
{

  time_t mid_time = start_time + (end_time - start_time) / 2;

  // compute request time
  
  switch(_params.gather_data_mode) {
    case Params::CLOSEST_TO_FRAME_CENTER:
      gd.data_request_time = mid_time;
      break;
    case Params::FIRST_BEFORE_END_OF_FRAME:
      gd.data_request_time = end_time;
      break;
    case Params::FIRST_AFTER_START_OF_FRAME:
      gd.data_request_time = start_time;
      break;

  } // gather_data_mode

  // Construct URL, check is valid.
  
  string fullUrl(_getFullUrl());
  DsURL URL(fullUrl);  
  if(URL.isValid() != TRUE) {
    cerr << "111111 ==>> MetRecord::requestHorizPlane Bogus URL: "
         << fullUrl << endl;
    h_data_valid = 1;
    return -1;
  }

  // Offset the request time
  
  start_time += (int) (time_offset * 60);
  end_time += (int) (time_offset * 60);
  
  if(gd.debug1) {
    fprintf(stderr, "Get MDVX Horiz Plane - page : %d  -  %s\n", page, fullUrl.c_str());
    // Disable threading while in debug mode
    h_mdvx->setThreadingOff();
  }

  // Gather a data index for this field  
  if(time_list_valid == 0) {
    _getTimeList(start_time, end_time,  page);
    return 0; // Must wait until it's done
  }
  
  // get field name
  
  string fieldName(_getFieldName());
  
  // add field to request
  
  h_mdvx->clearRead(); 
  h_mdvx->addReadField(fieldName);

  // set up read
  
  switch(_params.gather_data_mode ) {
    
    case Params::CLOSEST_TO_FRAME_CENTER:
      gd.data_request_time = mid_time;
      if(gd.model_run_time != 0 && h_mhdr.data_collection_type == Mdvx::DATA_FORECAST) { 
        h_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                            fullUrl,
                            (int) (60 * time_allowance),
                            gd.model_run_time,
                            mid_time - gd.model_run_time);
      } else {
        h_mdvx->setReadTime(Mdvx::READ_CLOSEST,
                            fullUrl,
                            (int) (60 * time_allowance),
                            mid_time);
      }
      break;

    case Params::FIRST_BEFORE_END_OF_FRAME:
      gd.data_request_time = end_time;
      if(gd.model_run_time != 0 && h_mhdr.data_collection_type == Mdvx::DATA_FORECAST) { 
        h_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                            fullUrl,
                            (int) (60 * time_allowance),
                            gd.model_run_time,
                            (end_time - gd.model_run_time));
      } else {
        h_mdvx->setReadTime(Mdvx::READ_FIRST_BEFORE,
                            fullUrl,
                            (int) (60 * time_allowance),
                            end_time + 1, 0);
      }
      
      if(gd.debug1) {
        fprintf(stderr,
                "{ READ_VOLUME_REQUEST, \"%s\", \"%s\", %d, %ld },\n",
                fullUrl.c_str(), fieldName.c_str(),
                (int) (60 * time_allowance), end_time + 1);
      }
      break;

    case Params::FIRST_AFTER_START_OF_FRAME:
      gd.data_request_time = start_time;
      if(gd.model_run_time != 0 && h_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST) { 
        h_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                            fullUrl,
                            (int) (60 * time_allowance),
                            gd.model_run_time,
                            (start_time - gd.model_run_time));
      } else {
        h_mdvx->setReadTime(Mdvx::READ_FIRST_AFTER,
                            fullUrl,
                            (int) (60 * time_allowance),
                            start_time-1, 0);
      }
      if(gd.debug1) {
        fprintf(stderr,
                "{ READ_VOLUME_REQUEST, \"%s\", \"%s\", %d, %ld },\n",
                fullUrl.c_str(), fieldName.c_str(),
                (int) (60 * time_allowance), start_time - 1);
      }
      break;

  } // gather_data_mode

  if(composite_mode) {
    // Ask for data that covers the whole vertical domain 
    h_mdvx->setReadVlevelLimits((double)gd.h_win.min_ht,
                                (double)gd.h_win.max_ht);
    h_mdvx->setReadComposite();
  } else {
    // Ask for plane closest to the display's current average height
    h_mdvx->setReadVlevelLimits(gd.h_win.cur_ht + alt_offset,
                                gd.h_win.cur_ht + alt_offset);
  }
     
  if(_params.always_get_full_domain == 0) {
    double min_lat, max_lat, min_lon, max_lon;
    _getBoundingBox(min_lat, max_lat, min_lon, max_lon); 
    if (!_params.do_not_clip_on_mdv_request) {
      h_mdvx->setReadHorizLimits(min_lat, min_lon, max_lat, max_lon);
    }
  } else {
    if (!_params.do_not_clip_on_mdv_request) {
      h_mdvx->setReadHorizLimits(-90.0, gd.h_win.origin_lon - 179.9999,
                                 90.0, gd.h_win.origin_lon + 179.9999);
    }
  }
  
  // Mdvx Decimation is based on the sqrt of the value. - Choose the longer edge 
  if (!_params.do_not_decimate_on_mdv_request) {
    // if(_params.aspect_ratio > 1.0) {
    h_mdvx->setReadDecimate(gd.h_win.img_dim.width * gd.h_win.img_dim.width);
    // } else {
    //   h_mdvx->setReadDecimate(gd.h_win.img_dim.height * gd.h_win.img_dim.height);
    // }
  }
     
  h_mdvx->setReadEncodingType(Mdvx::ENCODING_ASIS);
  h_mdvx->setReadCompressionType(Mdvx::COMPRESSION_ASIS);
  if(_params.request_compressed_data) {
    if(_params.request_gzip_vol_compression) {
      h_mdvx->setReadCompressionType(Mdvx::COMPRESSION_GZIP_VOL);
    } else {
      h_mdvx->setReadCompressionType(Mdvx::COMPRESSION_ZLIB);
    }
  }

  // request full field header for vert info

  h_mdvx->setReadFieldFileHeaders();

  gd.io_info.mode = DSMDVX_DATA;
  gd.io_info.request_type = HORIZ_REQUEST;    /* Horizontal data access */
  gd.io_info.expire_time = time(0) + _params.data_timeout_secs;
  gd.io_info.busy_status = 0;
  gd.io_info.page = page;
  gd.io_info.outstanding_request = 1;
  gd.io_info.mr = this;
  
  if(gd.debug1) {
    fprintf(stderr, "Get MDVX Horiz Plane - page : %d  -  %s\n", page, fullUrl.c_str());
  }
  
  // Initiate the request - This becomes threaded if not in debug mode.
  
  h_mdvx->readVolume();

  // The check_for_io function (thread) will poll for complettion of the data request.
  // Once the data's in or ithe request times out, this data is marked as valid
  
  return 0;
  
}
 
/**********************************************************************
 * Request data for a vertical section
 */

int MetRecord::requestVertSection(time_t start_time,
                                  time_t end_time,
                                  int page)
{
  
  // Construct URL, check is valid.
  
  string fullUrl(_getFullUrl());
  DsURL URL(fullUrl);  
  if(URL.isValid() != TRUE) {
    cerr << "222222 ==>> MetRecord::requestVertSection Bogus URL: "
         << fullUrl << endl;
    v_data_valid = 1;
    return -1;
  }
  URL.getURLStr();
  
  // get field name
  
  string fieldName(_getFieldName());

  // add field to request
  
  h_mdvx->clearRead(); 
  h_mdvx->addReadField(fieldName);
  
  if(gd.debug1) {
    fprintf(stderr, "Get MDVX Vert Plane - page : %d  -  %s\n", page, fullUrl.c_str());
    // Disable threading while in debug mode
    v_mdvx->setThreadingOff();
  }
  
  // offset the request time

  start_time += (int) (time_offset * 60);
  end_time += (int) (time_offset * 60);
  
  switch(_params.gather_data_mode ) {
    case Params::CLOSEST_TO_FRAME_CENTER :
      if(gd.model_run_time != 0 && v_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST) { 
        v_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                            fullUrl,(int)(60 * time_allowance),
                            gd.model_run_time,
                            (int)start_time + ((end_time  - start_time)/2) - gd.model_run_time);
      } else {
        v_mdvx->setReadTime(Mdvx::READ_CLOSEST,
                            fullUrl,(int)(60 * time_allowance),
                            (int)start_time + ((end_time  - start_time)/2));
      }
      break;

    case Params::FIRST_BEFORE_END_OF_FRAME:
      if(gd.model_run_time != 0 && v_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST) { 
        v_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                            fullUrl,(int)(60 * time_allowance),
                            gd.model_run_time,
                            (end_time - gd.model_run_time));
      } else {
        v_mdvx->setReadTime(Mdvx::READ_FIRST_BEFORE,
                            fullUrl,(int)(60 * time_allowance), end_time+1,0);
      }
      break;

    case Params::FIRST_AFTER_START_OF_FRAME:
      if(gd.model_run_time != 0 && v_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST) { 
        v_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                            fullUrl,(int)(60 * time_allowance),
                            gd.model_run_time,
                            (start_time - gd.model_run_time));
      } else {
        v_mdvx->setReadTime(Mdvx::READ_FIRST_AFTER,
                            fullUrl,(int)(60 * time_allowance), start_time-1,0);
      }
      break;
  }

  v_mdvx->setReadEncodingType(Mdvx::ENCODING_ASIS);
  v_mdvx->setReadCompressionType(Mdvx::COMPRESSION_ASIS);

  if(_params.request_compressed_data) {
    if(_params.request_gzip_vol_compression) {
      v_mdvx->setReadCompressionType(Mdvx::COMPRESSION_GZIP_VOL);
    } else {
      v_mdvx->setReadCompressionType(Mdvx::COMPRESSION_ZLIB);
    }
  }

  int num_way_points = gd.h_win.route.num_segments + 1;

  for(int ii = 0; ii < num_way_points; ii++) {
    double tlat, tlon;
    gd.proj.xy2latlon(gd.h_win.route.x_world[ii],
                      gd.h_win.route.y_world[ii],
                      tlat, tlon);
    v_mdvx->addReadWayPt(tlat,tlon);
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
  gd.io_info.mr = this;

  // Initiate the request
  v_mdvx->readVsection();

  return 0;
}

/**********************************************************************
 * REQUEST_TIME_LIST: Query the server for a Time list
 *
 */

int MetRecord::_getTimeList(time_t start_time,
                            time_t end_time,
                            int page)
{
  
  // Construct URL, check is valid.
  
  string fullUrl(_getFullUrl());
  DsURL URL(fullUrl);  
  if(URL.isValid() != TRUE) {
    cerr << "333 Bogus URL: " << fullUrl << endl;
    h_data_valid = 1;
    return -1;
  }
  URL.getURLStr();
  
  // offset the request time
  
  start_time += (int) (time_offset * 60);
  end_time += (int) (time_offset * 60);
  time_t delta = gd.epoch_end - gd.epoch_start;
  
  if(gd.debug1) {
    fprintf(stderr, "Get MDVX Time List page : %d  -  %s\n",
            page, fullUrl.c_str());
    // Disable threading while in debug mode
    h_mdvx->setThreadingOff();
  }

  h_mdvx->clearTimeListMode();
  if(gd.model_run_time != 0 &&
     h_mhdr.data_collection_type == Mdvx::DATA_FORECAST) { 
    h_mdvx->setTimeListModeLead(fullUrl,
                                gd.model_run_time);
  } else {
    h_mdvx->setTimeListModeValid(fullUrl,
                                 gd.epoch_start - delta,
                                 gd.epoch_end + delta);
  }
  if(gd.debug1) {
    fprintf(stderr, "Gathering Time List for %s\n", fullUrl.c_str());
  }

  char label[1024];
  snprintf(label, 1024, "Requesting time list for %s data", legend_name);
  if(_params.show_data_messages) {
    gui_label_h_frame(label, 1);
  } else {
    // set_busy_state(1);
  }

  // This is important because different scales can have diffrerent
  // temporal resolutions. - Must pass in the bounding box to get an 
  // accurate time list.

  double min_lat, max_lat, min_lon, max_lon;
  _getBoundingBox(min_lat, max_lat, min_lon, max_lon);
  
  if (!_params.do_not_clip_on_mdv_request) {
    h_mdvx->setReadHorizLimits(min_lat, min_lon, max_lat, max_lon);
  }

  // Set up the DsMdvx request object

  // Gather time list
  gd.io_info.mode = DSMDVX_DATA;
  gd.io_info.request_type = TIMELIST_REQUEST;    /* List of data times */
  gd.io_info.expire_time = time(0) + _params.data_timeout_secs;
  gd.io_info.busy_status = 0;
  gd.io_info.page = page;
  gd.io_info.outstanding_request = 1;
  gd.io_info.mr = this;

  if(gd.debug1) {
    fprintf(stderr,
            "{ TIME_LIST_REQUEST, \"%s\", \"%s\", %ld, %ld, %g, %g, %g, %g, %g },\n",
            fullUrl.c_str(), "none",
            gd.epoch_start - delta, gd.epoch_end + delta,
            -99.9, min_lat, min_lon, max_lat, max_lon);
  }

  // Limit List requests to given maximum number of days 
  if((gd.epoch_end - gd.epoch_start) <
     (_params.climo_max_time_span_days * 1440  * 60)) {
    if (h_mdvx->compileTimeList()) {
      cout << "ERROR -CIDD:  setTimeListModeValid" << endl;
      cout << h_mdvx->getErrStr();
      time_list_valid = 1;
    }  
  } else {
    gd.io_info.outstanding_request = 0;
    time_list_valid = 1;
  }

  return 0;  // return from this request.
  
  // When the thread is done the check_for_io function will gather and store the
  // timelist info and then set time_list_valid = 1 (true);
  
}


/**********************************************************************
 * get a fully-qualified URL
 *
 */

string MetRecord::_getFullUrl()
{
  
  // url from met record
  
  string fullUrl(url);

  // check for tunnel
  
  if(strlen(_params.http_tunnel_url) < URL_MIN_SIZE) {
    return fullUrl;
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

  fullUrl += tmp_str;

  return fullUrl;

}
  
/**********************************************************************
 * get field name, replacing ^ with spaces
 *
 */

string MetRecord::_getFieldName()
{
  
  char field_name[512];
  MEM_zero(field_name);
  strncpy(field_name, field_label, 256);
  
  // Replace carets with spaces

  char *ptr = field_name;
  while ((ptr = strchr(field_name,'^')) != NULL) {
    *ptr = ' ';
  }

  return field_name;

}

//////////////////////////////////////////////////////
// is the data valid?

bool MetRecord::isValidH() const {
  QMutexLocker locker(&_statusMutex);
  return _validH;
}

bool MetRecord::isValidV() const {
  QMutexLocker locker(&_statusMutex);
  return _validV;
}

//////////////////////////////////////////////////////
// is the data new?
// if new is true, sets to false and returns true

bool MetRecord::isNewH() const {
  QMutexLocker locker(&_statusMutex);
  bool isNew = _newH;
  _newH = false;
  return isNew;
}

bool MetRecord::isNewV() const {
  QMutexLocker locker(&_statusMutex);
  bool isNew = _newV;
  _newV = false;
  return isNew;
}


/**************************************************************************
 * GET BOUNDING BOX
 * Compute the current lat,lon bounding box of data on the display
 */

void MetRecord::_getBoundingBox(double &min_lat, double &max_lat,
                                double &min_lon, double &max_lon)
{

  // init
  
  max_lon = -360.0;
  min_lon = 360.0;
  max_lat = -180.0;
  min_lat = 180.0;
    
  // condition the longitudes for this zoom
  
  double meanx = (gd.h_win.cmin_x + gd.h_win.cmax_x) / 2.0;
  double meany = (gd.h_win.cmin_y + gd.h_win.cmax_y) / 2.0;
  
  // Make sure meanLon makes sense
  
  double meanLat, meanLon;
  gd.proj.xy2latlon(meanx, meany, meanLat, meanLon);
  if (meanLon > gd.h_win.max_x) {
    meanLon -= 360.0;
  } else if (meanLon < gd.h_win.min_x) {
    meanLon += 360.0;
  }
  gd.proj.setConditionLon2Ref(true, meanLon);

  // set box limits
  
  if(_params.always_get_full_domain) {

    // use win limits
    
    gd.proj.xy2latlon(gd.h_win.min_x, gd.h_win.min_y, min_lat, min_lon);
    gd.proj.xy2latlon(gd.h_win.max_x, gd.h_win.max_y, max_lat, max_lon);
    
  } else if (gd.display_projection == Mdvx::PROJ_LATLON) {
    
    // latlon projection
    
    double lon1 = gd.h_win.cmin_x;
    double lon2 = gd.h_win.cmax_x;
    
    if((lon2 - lon1) > 360.0) {
      lon1 = gd.h_win.min_x;
      lon2 = gd.h_win.max_x; 
    }
    
    double lat1 = gd.h_win.cmin_y;
    double lat2 = gd.h_win.cmax_y;
    if((lat2 - lat1) > 360.0) {
      lat1 = gd.h_win.min_y;
      lat2 = gd.h_win.max_y; 
    }
    
    gd.proj.xy2latlon(lon1, lat1, min_lat, min_lon);
    gd.proj.xy2latlon(lon2, lat2, max_lat, max_lon);

  } else {

    // all other projections

    // Compute the bounding box
    // Check each corner of the projection + 4 mid points, top, bottom
    // Left and right 
    
    // Lower left
    double lat,lon;
    gd.proj.xy2latlon(gd.h_win.cmin_x , gd.h_win.cmin_y, lat, lon);
    _adjustBoundingBox(lat, lon, min_lat, max_lat, min_lon, max_lon);
    
    // Lower midpoint
    gd.proj.xy2latlon((gd.h_win.cmin_x + gd.h_win.cmax_x)/2, gd.h_win.cmin_y, lat, lon);
    _adjustBoundingBox(lat, lon, min_lat, max_lat, min_lon, max_lon);
    
    // Lower right
    gd.proj.xy2latlon(gd.h_win.cmax_x,gd.h_win.cmin_y, lat, lon);
    _adjustBoundingBox(lat, lon, min_lat, max_lat, min_lon, max_lon);
    
    // Right midpoint
    gd.proj.xy2latlon(gd.h_win.cmax_x, (gd.h_win.cmin_y + gd.h_win.cmax_y)/2, lat, lon);
    _adjustBoundingBox(lat, lon, min_lat, max_lat, min_lon, max_lon);
    
    // Upper right
    gd.proj.xy2latlon(gd.h_win.cmax_x, gd.h_win.cmax_y, lat, lon);
    _adjustBoundingBox(lat, lon, min_lat, max_lat, min_lon, max_lon);
    
    // Upper midpoint
    gd.proj.xy2latlon((gd.h_win.cmin_x + gd.h_win.cmax_x)/2, gd.h_win.cmax_y, lat, lon);
    _adjustBoundingBox(lat, lon, min_lat, max_lat, min_lon, max_lon);
    
    // Upper left
    gd.proj.xy2latlon(gd.h_win.cmin_x, gd.h_win.cmax_y, lat, lon);
    _adjustBoundingBox(lat, lon, min_lat, max_lat, min_lon, max_lon);
    
    // Left midpoint
    gd.proj.xy2latlon(gd.h_win.cmin_x, (gd.h_win.cmin_y + gd.h_win.cmax_y)/2, lat, lon);
    _adjustBoundingBox(lat, lon, min_lat, max_lat, min_lon, max_lon);
    
  } // if(_params.always_get_full_domain)
  
  if(gd.display_projection == Mdvx::PROJ_LATLON ) {
    double originLon = (min_lon + max_lon) / 2.0;
    gd.proj.initLatlon(originLon);
  }
  
}

///////////////////////////////////////////////////////////////////
// adjust bounding box lat lon limits

void MetRecord::_adjustBoundingBox(double lat, double lon,
                                   double &minLat, double &maxLat,
                                   double &minLon, double &maxLon)
{
  if(lon > maxLon) {
    maxLon = lon;
  }
  if(lon < minLon) {
    minLon = lon;
  }
  if(lat > maxLat) {
    maxLat = lat;
  }
  if(lat < minLat) {
    minLat = lat;
  }
}

