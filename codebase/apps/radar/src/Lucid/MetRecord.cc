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

#include "MetRecord.hh"

#include <QCoreApplication>
#include <QThread>
#include <QObject>
#include <QDebug>
#include <QMutexLocker>

#include <qtplot/ColorMap.hh>
#include "cidd.h"

///////////////////////////////////////////////
// constructor

MetRecord::MetRecord(QObject* parent) :
        _lucid(parent)
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
  _timeListValid = false;
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
  
  _vLevelMinReq = -9999.0;
  _vLevelMaxReq = -9999.0;
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
                                 int page,
                                 double vlevel)
{

  cerr << "HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH MetRecord::requestHorizPlane()" << endl;
  cerr << "HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH page, vlevel: " << page << ", " << vlevel << endl;
  
  // apply offset the request time
  
  _startTime.set(start_time + time_offset * 60);
  _endTime.set(end_time + time_offset * 60);
  _page = page;
  _vLevelMinReq = vlevel;
  _vLevelMaxReq = vlevel;
  
  // check for change in request details
  
  if (!_checkRequestChangedH(_startTime.utime(), _endTime.utime())) {
    // no changes to the request, use what we have, mark as new
    cerr << "1111111111111111111 ==>>>> requestHorizPlane - no change <<<<==" << endl;
    _setNewH(true);
    return 0;
  }

  // Initiate the request in thread

  cerr << "1111111111111111111 requestHorizPlane before startReadVolH" << endl;
  startReadVolH();
  cerr << "1111111111111111111 requestHorizPlane after startReadVolH" << endl;
  
  // h_mdvx->readVolume();

  // The check_for_io function (thread) will poll for completion of the data request.
  // Once the data's in or ithe request times out, this data is marked as valid
  
  cerr << "1111111111111111111 requestHorizPlane END" << endl;
  return 0;
  
}

/**********************************************************************
 * REQUEST_HORIZ_DATA_PLANE: Get data for a plane
 */

int MetRecord::getHorizPlane()
{

  cerr << "JJJJJJJJJJJJJJJJ 11111111111 MetRecord::getHorizPlane()" << endl;
  
  // Construct URL, check is valid.
  
  string fullUrl(_getFullUrl());
  DsURL URL(fullUrl);  
  if(!URL.isValid()) {
    cerr << "ERROR - MetRecord::requestHorizPlane, field: " << _getFieldName() << endl;
    cerr << "  Bad URL: " << fullUrl << endl;
    h_data_valid = 1;
    cerr << "1111111111111111111 requestHorizPlane bad URL: " << _getFullUrl() << endl;
    return -1;
  }
  
  // turn off threading, we are using QThread
  h_mdvx->setThreadingOff();
  if(gd.debug1) {
    fprintf(stderr, "Get MDVX Horiz Plane - page : %d  -  %s\n", _page, fullUrl.c_str());
    // Disable threading while in debug mode
  }
  
  // Gather a data index for this field  
  // TODO - check this!!!
  
  if(!_timeListValid) {
    if (_getTimeList(_startTime.utime(), _endTime.utime(),  _page)) {
      cerr << "1111111111111111111 requestHorizPlane getTimeList() failed" << endl;
      return -1;
    }
  }
  
  // get field name
  
  string fieldName(_getFieldName());
  
  // add field to request
  
  h_mdvx->clearRead(); 
  h_mdvx->addReadField(fieldName);
  
  // set up read
  
  gd.data_request_time = _timeReq.utime();
  
  switch(_params.gather_data_mode ) {
    
    case Params::CLOSEST_TO_FRAME_CENTER:
      if(gd.model_run_time != 0 && h_mhdr.data_collection_type == Mdvx::DATA_FORECAST) { 
        h_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                            fullUrl,
                            (int) (60 * time_allowance),
                            gd.model_run_time,
                            _timeReq.utime() - gd.model_run_time);
      } else {
        h_mdvx->setReadTime(Mdvx::READ_CLOSEST,
                            fullUrl,
                            (int) (60 * time_allowance),
                            _timeReq.utime());
      }
      break;

    case Params::FIRST_BEFORE_END_OF_FRAME:
      if(gd.model_run_time != 0 && h_mhdr.data_collection_type == Mdvx::DATA_FORECAST) { 
        h_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                            fullUrl,
                            (int) (60 * time_allowance),
                            gd.model_run_time,
                            (_timeReq.utime() - gd.model_run_time));
      } else {
        h_mdvx->setReadTime(Mdvx::READ_FIRST_BEFORE,
                            fullUrl,
                            (int) (60 * time_allowance),
                            _timeReq.utime() + 1, 0);
      }
      
      if(gd.debug1) {
        fprintf(stderr,
                "{ READ_VOLUME_REQUEST, \"%s\", \"%s\", %d, %ld },\n",
                fullUrl.c_str(), fieldName.c_str(),
                (int) (60 * time_allowance), _endTime.utime() + 1);
      }
      break;

    case Params::FIRST_AFTER_START_OF_FRAME:
      if(gd.model_run_time != 0 && h_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST) { 
        h_mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                            fullUrl,
                            (int) (60 * time_allowance),
                            gd.model_run_time,
                            (_timeReq.utime() - gd.model_run_time));
      } else {
        h_mdvx->setReadTime(Mdvx::READ_FIRST_AFTER,
                            fullUrl,
                            (int) (60 * time_allowance),
                            _timeReq.utime()-1, 0);
      }
      if(gd.debug1) {
        fprintf(stderr,
                "{ READ_VOLUME_REQUEST, \"%s\", \"%s\", %d, %ld },\n",
                fullUrl.c_str(), fieldName.c_str(),
                (int) (60 * time_allowance), _startTime.utime() - 1);
      }
      break;

  } // gather_data_mode

  // vlevel
  
  h_mdvx->setReadVlevelLimits(_vLevelMinReq, _vLevelMaxReq);
  if(composite_mode) {
    // Ask for data that covers the whole vertical domain 
    h_mdvx->setReadComposite();
  }

  // zoom

  h_mdvx->setReadHorizLimits(_zoomBoxReq.getMinLat(),
                             _zoomBoxReq.getMinLon(),
                             _zoomBoxReq.getMaxLat(),
                             _zoomBoxReq.getMaxLon());
  
  // Mdvx Decimation is based on the sqrt of the value. - Choose the longer edge 
  if (!_params.do_not_decimate_on_mdv_request) {
    h_mdvx->setReadDecimate(gd.h_win.img_dim.width * gd.h_win.img_dim.width);
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
  gd.io_info.page = _page;
  gd.io_info.outstanding_request = 1;
  gd.io_info.mr = this;
  
  //if(gd.debug1) {
  fprintf(stderr, "Get MDVX Horiz Plane - page : %d  -  %s\n", _page, fullUrl.c_str());
  // }
  
  // Initiate the request in thread
  
  int iret = h_mdvx->readVolume();
  if (iret) {
    cerr << "1111111 MetRecord::getHorizPlane, iret: " << ", " << iret << endl;
    qDebug() << "MetRecord::getHorizPlane returned, iret: " << ", " << iret;
    qDebug() << h_mdvx->getErrStr();
  }
  iret_h_mdvx_read = iret;

  if (iret) {

    // error - indicate data is no longer pending

    // Save the master header for the file, even if we couldn't
    // get data for this field.  This is needed in case we are
    // dealing with forecast data
      
    h_data = NULL;
    h_fl32_data = NULL;
    h_mhdr = h_mdvx->getMasterHeader();
    h_data_valid = 1;
    last_collected = time(0);
      
    gd.io_info.busy_status = 0;
    gd.io_info.outstanding_request = 0;
    gd.io_info.request_type = 0;
    gd.h_win.redraw_flag[gd.io_info.page] = 1;

    cerr << "1111111111111111111 requestHorizPlane ERROR" << endl;
    return -1;
    
  } else if (h_mdvx->getFieldByNum(0) == NULL) {
    
    gd.io_info.busy_status = 0;
    gd.io_info.outstanding_request = 0;
    gd.io_info.request_type = 0;
    gd.h_win.redraw_flag[gd.io_info.page] = 1;
    h_data_valid = 1;
    last_collected = time(0);

    cerr << "1111111111111111111 requestHorizPlane ERROR" << endl;
    return -1;

  }

  // data is in
  
  cerr << "111111111111111111111111111111111111111 requestHorizPlane GOT DATA" << endl;
  
  // Indicate data update is in progress.
  
  gd.io_info.busy_status = 1;
  
  MdvxField *hfld = h_mdvx->getFieldByNum(0);
  Mdvx::field_header_t fh = hfld->getFieldHeader();
  if(fh.encoding_type != Mdvx::ENCODING_RGBA32) {

    *h_mdvx_int16 = *hfld; // Copy for INT16 data
    
    if(h_vcm.nentries < 2 || fh.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
      
      h_mdvx_int16->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_NONE);

    } else {
      
      // Convert the copy to - Decompressed INT16 - Covering the range of the colorscale
      double range = (h_vcm.vc[h_vcm.nentries-1]->max - h_vcm.vc[0]->min);
      double scale = range / (MAX_COLOR_CELLS -2);
      double bias = h_vcm.vc[0]->min - (2 * scale); // Preserve 0, 1 as legitimate NAN values
        
      h_mdvx_int16->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_NONE,
                                    Mdvx::SCALING_SPECIFIED,scale,bias);
    }
      
    // Record where int8 data is in memory. - Used for fast polygon fills.
    h_data = (unsigned short *) h_mdvx_int16->getVol();
    
    // Convert the AS-IS to 32 bits float. - Used for Contouring, Data reporting.
    (h_mdvx->getFieldByNum(0))->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
    // Record where float data is in memory.
    h_fl32_data = (fl32  *) h_mdvx->getFieldByNum(0)->getVol();
      
    // Find Headers for quick reference
    h_mhdr = h_mdvx->getMasterHeader();
    h_fhdr = h_mdvx_int16->getFieldHeader();
    h_vhdr = h_mdvx_int16->getVlevelHeader();

  } else {
      
    // Decompress
    hfld->convertType(Mdvx::ENCODING_ASIS, Mdvx::COMPRESSION_NONE);
      
    // Record where data is in memory. 
    h_data = (unsigned short *) hfld->getVol();
      
    // Record where float data is in memory.
    h_fl32_data = (fl32  *) hfld->getVol();
      
    // Find Headers for quick reference
    h_mhdr = h_mdvx->getMasterHeader();
    h_fhdr = hfld->getFieldHeader();
    h_vhdr = hfld->getVlevelHeader();

  } // compression
    
  // Init projection

  proj->init(h_fhdr);
    
  // condition longitudes to be in same hemisphere
  // as origin
  proj->setConditionLon2Origin(true);
    
  // Implemented for MOBILE RADARS - 
  if(_params.domain_follows_data &&
     this == gd.mrec[gd.h_win.page] ) { // Only for the primary field
    double dx,locx;
    double dy,locy;
    int index = gd.h_win.zoom_level;
      
    if(h_fhdr.proj_origin_lat != gd.h_win.origin_lat ||
       h_fhdr.proj_origin_lon != gd.h_win.origin_lon) {
        
      dx = gd.h_win.zmax_x[index] - gd.h_win.zmin_x[index];
      dy = gd.h_win.zmax_y[index] - gd.h_win.zmin_y[index];
        
      switch(gd.display_projection) {
        case Mdvx::PROJ_LATLON:
          gd.h_win.origin_lat = h_fhdr.proj_origin_lat;
          gd.h_win.origin_lon = h_fhdr.proj_origin_lon;
            
          gd.h_win.zmin_x[index] = h_fhdr.proj_origin_lon - (dx / 2.0);
          gd.h_win.zmax_x[index] = h_fhdr.proj_origin_lon + (dx / 2.0);
          gd.h_win.zmin_y[index] = h_fhdr.proj_origin_lat - (dy / 2.0);
          gd.h_win.zmax_y[index] = h_fhdr.proj_origin_lat + (dy / 2.0);
            
          break;
            
        default:
          gd.proj.latlon2xy(h_fhdr.proj_origin_lat,h_fhdr.proj_origin_lon,locx,locy);
            
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
      h_data_valid = 1;  // This field is still valid, though
      setTimeListValid(true);
	
	
    }
  }
  
  if (gd.debug1) {
    cerr << "nx, ny: "
         << h_fhdr.nx << ", "
         << h_fhdr.ny << endl;
    cerr << "dx, dy: "
         << h_fhdr.grid_dx << ", "
         << h_fhdr.grid_dy << endl;
    cerr << "minx, miny: "
         << h_fhdr.grid_minx << ", "
         << h_fhdr.grid_miny << endl;
    cerr << "maxx, maxy: "
         << h_fhdr.grid_minx + (h_fhdr.nx - 1) * h_fhdr.grid_dx << ", "
         << h_fhdr.grid_miny + (h_fhdr.ny - 1) * h_fhdr.grid_dy << endl;
  }
    
  // Punt and use the field headers if the file headers are not avail
  if(hfld->getFieldHeaderFile() == NULL) 
    ds_fhdr = (hfld->getFieldHeader());
  else 
    ds_fhdr = *(hfld->getFieldHeaderFile());
    
  if(hfld->getVlevelHeaderFile() == NULL) 
    ds_vhdr = (hfld->getVlevelHeader());
  else 
    ds_vhdr = *(hfld->getVlevelHeaderFile());
    
  // Recompute the color scale lookup table if necessary
  if(h_fhdr.scale != h_last_scale ||
     h_fhdr.bias != h_last_bias   ||
     h_fhdr.missing_data_value != h_last_missing   ||
     h_fhdr.bad_data_value != h_last_bad ||
     h_fhdr.transform_type != h_last_transform ) {
      
    if(auto_scale)
      autoscale_vcm(&(h_vcm), h_fhdr.min_value, h_fhdr.max_value);
      
    if(fh.encoding_type != Mdvx::ENCODING_RGBA32) {
#ifdef NOTYET
      /* Remap the data values onto the colorscale */
      setup_color_mapping(&(h_vcm),
                          h_fhdr.scale,
                          h_fhdr.bias,
                          h_fhdr.transform_type,
                          h_fhdr.bad_data_value,
                          h_fhdr.missing_data_value);
#endif
    }
    // Update last values
    h_last_scale = h_fhdr.scale;
    h_last_bias = h_fhdr.bias;
    h_last_missing = h_fhdr.missing_data_value;
    h_last_bad = h_fhdr.bad_data_value;
    h_last_transform = h_fhdr.transform_type;
  }
    
  // Compute the vertical levels 
  plane = 0;
  for(int i=0; i < ds_fhdr.nz; i++) { 
    // Find out which plane we received
    if(ds_vhdr.level[i] == h_vhdr.level[0]) plane = i;
    if(i == 0) { // Lowest plane 
      double delta = (ds_vhdr.level[i+1] - ds_vhdr.level[i]) / 2.0;
      vert[i].min = ds_vhdr.level[0] - delta;
      vert[i].cent = ds_vhdr.level[0];
      vert[i].max = ds_vhdr.level[0] + delta;
        
    } else if (i == ds_fhdr.nz -1) { // highest plane
      double delta = (ds_vhdr.level[i] - ds_vhdr.level[i-1]) / 2.0;
      vert[i].min = ds_vhdr.level[i] - delta;
      vert[i].cent = ds_vhdr.level[i];
      vert[i].max = ds_vhdr.level[i] + delta;
        
    } else { // Middle planes
      double delta = (ds_vhdr.level[i] - ds_vhdr.level[i-1]) / 2.0;
      vert[i].min = ds_vhdr.level[i] - delta;
      vert[i].cent = ds_vhdr.level[i];
        
      delta = (ds_vhdr.level[i+1] - ds_vhdr.level[i]) / 2.0;
      vert[i].max = ds_vhdr.level[i] + delta;
    }
  }
    
    
  // Record the dimensional Units of the volume
  STRcopy(units_label_cols,
          h_mdvx->projType2XUnits(h_fhdr.proj_type),LABEL_LENGTH);
  STRcopy(units_label_rows,
          h_mdvx->projType2YUnits(h_fhdr.proj_type),LABEL_LENGTH);
  STRcopy(units_label_sects,
          h_mdvx->vertTypeZUnits(h_vhdr.type[0]),LABEL_LENGTH);
    
  // Record the date
  h_date.set(h_mhdr.time_centroid);
  // UTIMunix_to_date(h_mhdr.time_centroid,&h_date);
    
  h_data_valid = 1;
  last_collected = time(0);
  gd.h_win.redraw_flag[gd.io_info.page] = 1;
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


  cerr << "1111111111111111111 requestHorizPlane END" << endl;

  return 0;
  
}

////////////////////////////////////////////////////////////
// check whether H request has changed
// returns true if changed, false if no change

bool MetRecord::_checkRequestChangedH(time_t start_time,
                                      time_t end_time)
{
  
  // compute requested time
  
  DateTime timeReq(start_time);
  switch(_params.gather_data_mode) {
    case Params::CLOSEST_TO_FRAME_CENTER: {
      time_t midTime = start_time + (end_time - start_time) / 2;
      timeReq = midTime;
      break;
    }
    case Params::FIRST_BEFORE_END_OF_FRAME: {
      timeReq = end_time;
      break;
    }
    case Params::FIRST_AFTER_START_OF_FRAME: {
      timeReq = start_time;
      break;
    }

  } // gather_data_mode

  // compute requested vlevel limits
  
  double vLevelMinReq, vLevelMaxReq;
  if(composite_mode) {
    vLevelMinReq = gd.h_win.min_ht;
    vLevelMaxReq = gd.h_win.max_ht;
  } else {
    vLevelMinReq = gd.h_win.cur_ht + alt_offset;
    vLevelMaxReq = gd.h_win.cur_ht + alt_offset;
  }

  // compute requested zoom domain
  
  LatLonBox zoomBoxReq;
  if(!_params.always_get_full_domain) {
    double min_lat, max_lat, min_lon, max_lon;
    _getBoundingBox(min_lat, max_lat, min_lon, max_lon);
    zoomBoxReq.setLimits(min_lat, max_lat, min_lon, max_lon);
  } else if (_params.do_not_clip_on_mdv_request) {
    zoomBoxReq.clearLimits();
  } else {
    zoomBoxReq.setLimits(-90.0, 90.0,
                         gd.h_win.origin_lon - 179.9999,
                         gd.h_win.origin_lon + 179.9999);
  }

  // check if request is unchanged from previous call

  QMutexLocker locker(&_statusMutex);
  if (_validH &&
      timeReq == _timeReq &&
      fabs(vLevelMinReq - _vLevelMinReq) < 1.0e-5 &&
      fabs(vLevelMaxReq - _vLevelMaxReq) < 1.0e-5 &&
      zoomBoxReq == _zoomBoxReq) {
    // no changes, use what we have, mark as new
    return false;
  }
  
  // save request for comparison next time

  _timeReq = timeReq;
  _vLevelMinReq = vLevelMinReq;
  _vLevelMaxReq = vLevelMaxReq;
  _zoomBoxReq = zoomBoxReq;

  return true;

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
  
  v_mdvx->setThreadingOff();
  if(gd.debug1) {
    fprintf(stderr, "Get MDVX Vert Plane - page : %d  -  %s\n", page, fullUrl.c_str());
    // Disable threading while in debug mode
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
  
  h_mdvx->setThreadingOff();
  if(gd.debug1) {
    fprintf(stderr, "Get MDVX Time List page : %d  -  %s\n",
            page, fullUrl.c_str());
    // Disable threading while in debug mode
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
      _timeListValid = true;
    }  
  } else {
    gd.io_info.outstanding_request = 0;
    _timeListValid = true;
  }

  // {
  //   const vector<time_t> &validTimes = h_mdvx->getValidTimes();
  //   for (size_t ii = 0; ii < validTimes.size(); ii++) {
  //     cerr << "1111111111111 ii, validTime: " << ii
  //          << ", " << DateTime::strm(validTimes[ii]) << endl;
  //   }
  // }

  return 0;  // return from this request.
  
  // When the thread is done the check_for_io function will gather and store the
  // timelist info and then set _timeListValid = 1 (true);
  
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

void MetRecord::_setValidH(bool state) {
  QMutexLocker locker(&_statusMutex);
  _validH = state;
}

void MetRecord::_setValidV(bool state) {
  QMutexLocker locker(&_statusMutex);
  _validV = state;
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

void MetRecord::_setNewH(bool state) {
  QMutexLocker locker(&_statusMutex);
  _newH = state;
}

void MetRecord::_setNewV(bool state) {
  QMutexLocker locker(&_statusMutex);
  _newV = state;
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

///////////////////////////////////////////////
// Worker for read H volume in thread

ReadVolH::ReadVolH(MetRecord* parentObject, QObject* parent) :
        QObject(parent), _mr(parentObject)
{
}

void ReadVolH::doRead() {
  cerr << "111111111 ReadVolH thread started. Parent object:" << _mr << endl;
  qDebug() << "ReadVolH thread started. Parent object:" << _mr;
  int iret = _mr->getHorizPlane();
  cerr << "1111111 ReadVolH returned, iret: " << ", " << iret << endl;
  qDebug() << "ReadVolH returned, iret: " << ", " << iret;
  emit readDone();
}

//////////////////////////////
// start H vol read in thread

void MetRecord::startReadVolH() {
  cerr << "1111111111111111112222222222222222222223333333333333333333" << endl;
  ReadVolH* worker = new ReadVolH(this); // Pass the current object as reference
  QThread* thread = new QThread;
  worker->moveToThread(thread);
  // Connect signals and slots
  connect(thread, &QThread::started, worker, &ReadVolH::doRead);
  connect(worker, &ReadVolH::readDone, this, &MetRecord::readDoneH);
  // connect(worker, &ReadVolH::readDone, thread, &QThread::quit);
  connect(thread, &QThread::finished, worker, &ReadVolH::deleteLater);
  connect(thread, &QThread::finished, thread, &QThread::deleteLater);
  thread->start();
}

////////////////////////
// done with H vol read

void MetRecord::readDoneH() {
  cerr << "1111111111111 readDone in ReadVolH worker thread" << endl;
  qDebug() << "readDone in ReadVolH worker thread";
  _setValidH(iret_h_mdvx_read == 0);
  _setNewH(true);
}

