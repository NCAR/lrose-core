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
 * MDVX_DATA_REQUEST.CC: 
 *
 */
#define MDVX_DATA_REQUEST

#include "cidd.h"

int mdvx_request_time_list(met_record_t *mr,
                           time_t start_time,
                           time_t end_time,
                           int page);

/**********************************************************************
 * REQUEST_HORIZ_DATA_PLANE: Query the server for a data plane
 *
 */

int mdvx_request_horiz_data_plane(met_record_t *mr,
                                  time_t start_time,
                                  time_t end_time,
                                  int page)
{

  double min_lat,max_lat,min_lon,max_lon;
  double height = 0.0;

  char url[2048];
  MEM_zero(url);
  strncpy(url, mr->url, 2048);
  
  // If using the tunnel - Add the tunnel_url to the Url as a name=value pair
  // If using the a proxy - Add the a proxy_url to the Url as a name=value pair
  // Note this must be used in conjunction with a tunnel_url.

  if(strlen(_params.http_tunnel_url) > URL_MIN_SIZE) { 
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
    // Append the arguments to the end of the Url string
    strncat(url, tmp_str, 1024);
  }
  
  // Construct URL, check is valid.
  
  DsURL URL(url);  
  if(URL.isValid() != TRUE) {
    fprintf(stderr,"111 Bogus URL: %s\n",url);
    mr->h_data_valid = 1;
    return -1;
  }

  // Off set the request time

  start_time += (int) (mr->time_offset * 60);
  end_time += (int) (mr->time_offset * 60);
  
  if(gd.debug1) {
    fprintf(stderr, "Get MDVX Horiz Plane - page : %d  -  %s\n", page, mr->url);
    // Disable threading while in debug mode
    mr->h_mdvx->setThreadingOff();
  }

  // Gather a data index for this field  
  if(mr->time_list_valid == 0) {
    mdvx_request_time_list(mr, start_time,end_time,  page);
    return 0; // Must wait until it's done
  }
  
  // get field name
  
  char fieldname[2048];
  MEM_zero(fieldname);
  strncpy(fieldname, mr->field_label, 256);
  
  // Replace carets with spaces
  char *ptr = fieldname;
  while ((ptr = strchr(fieldname,'^')) != NULL) {
    *ptr = ' ';
  }

  // add field to request
  
  mr->h_mdvx->clearRead(); 
  mr->h_mdvx->addReadField(fieldname);

  // set up read
  
  switch(_params.gather_data_mode ) {
    case CLOSEST_TO_FRAME_CENTER :
      gd.data_request_time = (time_t) start_time + ((end_time  - start_time)/2);
      if(gd.model_run_time != 0 && mr->h_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST) { 
        mr->h_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                                url,(int)(60 * mr->time_allowance),
                                gd.model_run_time,
                                (int) start_time + ((end_time  - start_time)/2) - gd.model_run_time);
      } else {
        mr->h_mdvx->setReadTime(Mdvx::READ_CLOSEST,
                                url,(int)(60 * mr->time_allowance),
                                (int) start_time + ((end_time  - start_time)/2));
      }
      break;

    case FIRST_BEFORE_END_OF_FRAME:
      gd.data_request_time = end_time;
      if(gd.model_run_time != 0 && mr->h_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST) { 
        mr->h_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                                url,(int)(60 * mr->time_allowance),
                                gd.model_run_time,
                                (end_time - gd.model_run_time));
      } else {
        mr->h_mdvx->setReadTime(Mdvx::READ_FIRST_BEFORE,
                                url,(int)(60 * mr->time_allowance), end_time+1,0);
      }
      if(gd.debug1) fprintf(stderr,
                            "{ READ_VOLUME_REQUEST, \"%s\", \"%s\", %d, %ld, %g, %g, %g, %g, %g },\n",
                            url, fieldname,
                            (int) (60 * mr->time_allowance), end_time + 1,
                            height, min_lat, min_lon, max_lat, max_lon);
      break;

    case FIRST_AFTER_START_OF_FRAME:
      gd.data_request_time = start_time;
      if(gd.model_run_time != 0 && mr->h_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST) { 
        mr->h_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                                url,(int)(60 * mr->time_allowance),
                                gd.model_run_time,
                                (start_time - gd.model_run_time));
      } else {
        mr->h_mdvx->setReadTime(Mdvx::READ_FIRST_AFTER,
                                url,(int)(60 * mr->time_allowance), start_time-1,0);
      }
      if(gd.debug1) fprintf(stderr,
                            "{ READ_VOLUME_REQUEST, \"%s\", \"%s\", %d, %ld, %g, %g, %g, %g, %g },\n",
                            url, fieldname,
                            (int) (60 * mr->time_allowance), start_time - 1,
                            height, min_lat, min_lon, max_lat, max_lon);
      break;
  }

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
    get_bounding_box(min_lat,max_lat,min_lon,max_lon); 
    if (!_params.do_not_clip_on_mdv_request) {
      mr->h_mdvx->setReadHorizLimits(min_lat,min_lon,max_lat,max_lon);
    }
  } else {
    if (!_params.do_not_clip_on_mdv_request) {
      mr->h_mdvx->setReadHorizLimits(-90.0, gd.h_win.origin_lon - 179.9999,
                                     90.0, gd.h_win.origin_lon + 179.9999);
    }
  }

  // Mdvx Decimation is based on the sqrt of the value. - Choose the longer edge 
  if (!_params.do_not_decimate_on_mdv_request) {
    if(_params.aspect_ratio > 1.0) {
      mr->h_mdvx->setReadDecimate( gd.h_win.img_dim.width *  gd.h_win.img_dim.width);
    } else {
      mr->h_mdvx->setReadDecimate( gd.h_win.img_dim.height *  gd.h_win.img_dim.height);
    }
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
    fprintf(stderr, "Get MDVX Horiz Plane - page : %d  -  %s\n", page,mr->url);
  }

  // Initiate the request - This becomes threaded if not in debug mode.
  mr->h_mdvx->readVolume();

  // The check_io function (thread) will poll for complettion of the data request.
  // Once the data's in or ithe request times out, this data is marked as valid

  return 0;
}
 
/**********************************************************************
 * REQUEST_VERT_DATA_PLANE: Query the server for a data plane
 *
 */

int mdvx_request_vert_data_plane(met_record_t *mr, time_t start_time,time_t end_time, int page)
{
  int i,num_way_points;
  int field_num;
  char url[2048];
  char tmp_str[1024];
  char fieldname[256];
  char *ptr;
  double tlat,tlon;
  // double lat1, lon1, lat2, lon2;

  mr->v_mdvx->clearRead(); 

  // Pick out field name or number;
  if((ptr = strchr(mr->url,'&')) == NULL) return -1;

  cerr << "bbbbbbbbbbbbb url: " << mr->url << endl;
  cerr << "bbbbbbbbbbbbb field_label: " << mr->field_label << endl;
    
  strncpy(url,mr->url,(ptr - mr->url));
  url[ptr - mr->url] = '\0'; // Null terminate

  // If using the tunnel - Add the tunnel_url to the Url as a name=value pair
  if(strlen(_params.http_tunnel_url) > URL_MIN_SIZE) { 
    // If using the a proxy - Add the a proxy_url to the Url as a name=value pair
    // Note this must be used in conjunction with a tunnel_url.
    if((strlen(_params.http_proxy_url)) > URL_MIN_SIZE) {
      sprintf(tmp_str,"?tunnel_url=%s&proxy_url=%s",_params.http_tunnel_url,_params.http_proxy_url);
    } else {
      sprintf(tmp_str,"?tunnel_url=%s",_params.http_tunnel_url);
    }
      
    // Append the arguments to the end of the Url string
    strncat(url,tmp_str,1024);
  }

  // Construct a valid DsUrl.
  DsURL URL(url);  

  if(URL.isValid() != TRUE) {
    fprintf(stderr,"222 Bogus URL: %s\n",url);
    mr->v_data_valid = 1;
    return -1;
  }

  URL.getURLStr();

  // Set which field to read.
  ptr++; // Skip over the ampersand
  if(*ptr == '#') {
    ptr++; // skip over the pound sign
    field_num = atoi(ptr);
    if(field_num < 0 || field_num > 65536) field_num =0;
    mr->v_mdvx->addReadField(field_num);
  } else {
    strncpy(fieldname,ptr,128);

    // Replace carets with spaces
    while((ptr = strchr(fieldname,'^')) != NULL) *ptr = ' ';

    mr->v_mdvx->addReadField(fieldname);
  }

  if(gd.debug1) {
    fprintf(stderr, "Get MDVX Vert Plane - page : %d  -  %s\n", page,mr->url);

    // Disable threading while in debug mode
    mr->v_mdvx->setThreadingOff();
  }

     
  /* Off set the request time */
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

  num_way_points = gd.h_win.route.num_segments + 1;

  for(i=0; i < num_way_points; i++) {
    gd.proj.xy2latlon(gd.h_win.route.x_world[i],gd.h_win.route.y_world[i],tlat,tlon);
    mr->v_mdvx->addReadWayPt(tlat,tlon);
    if (i == 0) {
      // lat1 = tlat;
      // lon1 = tlon;
    } else if (i == num_way_points - 1) {
      // lat2 = tlat;
      // lon2 = tlon;
    }

    if(gd.debug1) fprintf(stderr,"Way pt Lat,Lon: %g,%g   Len: %g\n",
                          tlat,tlon, gd.h_win.route.seg_length[i]);
  }

  //     fprintf(stderr,
  // 	    "{ READ_VSECTION_REQUEST, \"%s\", \"%s\", %d, %d, %g, %g, %g, %g, %g },\n",
  // 	    url, fieldname,
  // 	    (int) (60 * mr->time_allowance), end_time,
  // 	    -99.9, lat1, lon1, lat2, lon2);

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

int mdvx_request_time_list(met_record_t *mr,
                           time_t start_time,
                           time_t end_time,
                           int page)
{
  
  double min_lat, max_lat, min_lon, max_lon;
  char url[2048];
  char tmp_str[1024];
  char fieldname[256];
  char label[1024];
  // char *ptr;
  time_t delta;

  memset(fieldname,0,128);
  memset(label,0,128);
  memset(url,0,1024);
    
  cerr << "ccccccccccccc url: " << mr->url << endl;
  cerr << "ccccccccccccc field_label: " << mr->field_label << endl;
    
  // Pick out field name or number;
  // if((ptr = strchr(mr->url,'&')) == NULL) {
  //     fprintf(stderr,"333 Bogus URL: %s -\n Missing ampersand before field name/number\n",mr->url);
  //     return -1;
  // }
  // strncpy(url,mr->url,(ptr - mr->url));
  // url[ptr - mr->url] = '\0'; // Null terminate

  strncpy(url, mr->url, 1024);

  // If using the tunnel - Add the tunnel_url to the Url as a name=value pair
  if(strlen(_params.http_tunnel_url) > URL_MIN_SIZE) { 
    // If using the a proxy - Add the a proxy_url to the Url as a name=value pair
    // Note this must be used in conjunction with a tunnel_url.
    if((strlen(_params.http_proxy_url)) > URL_MIN_SIZE) {
      sprintf(tmp_str,"?tunnel_url=%s&proxy_url=%s",_params.http_tunnel_url,_params.http_proxy_url);
    } else {
      sprintf(tmp_str,"?tunnel_url=%s",_params.http_tunnel_url);
    }
      
    // Append the arguments to the end of the Url string
    strncat(url,tmp_str,1024);
  }

  // Construct a valid DsUrl.
  DsURL URL(url);  

  if(URL.isValid() != TRUE) {
    fprintf(stderr,"444 Bogus URL: %s\n",url);
    mr->h_data_valid = 1;
    return -1;
  }

  // URL.getURLStr();

  /* Off set the request time */
  start_time += (int) (mr->time_offset * 60);
  end_time += (int) (mr->time_offset * 60);
  delta = gd.epoch_end - gd.epoch_start;


  if(gd.debug1) {
    fprintf(stderr, "Get MDVX Time List page : %d  -  %s\n", page,mr->url);
    // Disable threading while in debug mode
    mr->h_mdvx->setThreadingOff();
  }

  mr->h_mdvx->clearTimeListMode();
  if(gd.model_run_time != 0 && mr->h_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST) { 
    mr->h_mdvx->setTimeListModeLead(url,gd.model_run_time);
  } else {
    mr->h_mdvx->setTimeListModeValid(url,gd.epoch_start - delta,gd.epoch_end + delta);
  }
  if(gd.debug1) fprintf(stderr, "Gathering Time List for %s\n",mr->url);
  sprintf(label, "Requesting time list for %s data",mr->legend_name);
  if(_params.show_data_messages) {
    gui_label_h_frame(label,1);
  } else {
    // set_busy_state(1);
  }

  // This is important because different scales can have diffrerent
  // temporal resolutions. - Must pass in the bounding box to get an 
  // accurate time list.
  get_bounding_box(min_lat,max_lat,min_lon,max_lon);

  // Set up the DsMdvx request object
  if (!_params.do_not_clip_on_mdv_request) {
    mr->h_mdvx->setReadHorizLimits(min_lat,min_lon,max_lat,max_lon);
  }

  // Gather time list
  gd.io_info.mode = DSMDVX_DATA;
  gd.io_info.request_type = TIMELIST_REQUEST;    /* List of data times */
  gd.io_info.expire_time = time(0) + _params.data_timeout_secs;
  gd.io_info.busy_status = 0;
  gd.io_info.page = page;
  gd.io_info.outstanding_request = 1;
  gd.io_info.mr = mr;

  if(gd.debug1) fprintf(stderr,
                        "{ TIME_LIST_REQUEST, \"%s\", \"%s\", %ld, %ld, %g, %g, %g, %g, %g },\n",
                        url, "none",
                        gd.epoch_start - delta, gd.epoch_end + delta,
                        -99.9, min_lat, min_lon, max_lat, max_lon);

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
  // When the thread is done the check_io function will gather and store the
  // timelist info and then set mr->time_list_valid = 1 (true);
}
