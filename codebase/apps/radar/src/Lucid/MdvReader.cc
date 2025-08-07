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
// MdvReader.cc
//
// Field data object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2024
//
///////////////////////////////////////////////////////////////

#include "MdvReader.hh"
#include "TimeControl.hh"

#include <QCoreApplication>
#include <QThread>
#include <QObject>
#include <QDebug>
#include <QMutexLocker>

#include <toolsa/str.h>

///////////////////////////////////////////////
// constructor

MdvReader::MdvReader(Params &params) :
        _params(params),
        _gd(GlobalData::Instance())
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
  vert_type = 0;
  detail_thresh_min = 0.0;
  detail_thresh_max = 0.0;
  
  ht_pixel = 0.0;
  y_intercept = 0.0;
  
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
  
  h_date.setToNever();
  v_date.setToNever();
  
  MEM_zero(h_mhdr);
  MEM_zero(h_fhdr);
  MEM_zero(h_vhdr); 
  
  MEM_zero(ds_fhdr);
  MEM_zero(ds_vhdr);
  
  MEM_zero(v_mhdr);
  MEM_zero(v_fhdr);
  MEM_zero(v_vhdr);

  isRadar = false;
  
  _vLevelReq = -9999.0;
  _validH = false;
  _validV = false;
  _newH = false;
  _newV = false;

  _page = 0;
  _timeListValid = false;
  _vLevel = 0;
  _readBusyH = false;

}

/**********************************************************************
 * REQUEST_HORIZ_DATA_PLANE: Get data for a plane
 * This is implemented in a thread.
 * On error, isValidH is set to false.
 */

void MdvReader::requestHorizPlane(const DateTime &midTime,
                                  double vLevel,
                                  int page)
{

  _setValidH(false);
  _setNewH(false);
  
  if(_gd.debug1) {
    cerr << "=====>>> MdvReader::requestHorizPlane()" << endl;
    cerr << "   midTime: " << midTime.asString(0) << endl;
    cerr << "   vLevel: " << vLevel << endl;
    cerr << "   page: " << page << endl;
  }
  
  _midTime = midTime;
  _page = page;
  
  // check for change in request details
  
  if (!_checkRequestChangedH(_midTime, vLevel)) {
    // no changes to the request, use what we have, mark as new
    if(_gd.debug2) {
      cerr << "======>> requestHorizPlane - no change <<<<==" << endl;
    }
    _setNewH(true);
    return;
  }

  // Initiate the request in thread
  
  startReadVolH();
  
}

////////////////////////////////////////////////////////////////////
// REQUEST_HORIZ_DATA_PLANE: Get data for a plane
//
// This is run in a separate thread.
// Once complete, the data is copied back to the MdvReader object.
/////////////////////////////////////////////////////////////////////

int MdvReader::getHorizPlane()
{
  
  // initialize
  
  h_rgba32_data.clear();
  h_fl32_data.clear();
  h_data_valid = 0;
    
  // Construct URL, check is valid.
  
  string fullUrl(_getFullUrl());
  DsURL URL(fullUrl);  
  if(!URL.isValid()) {
    cerr << "ERROR - MdvReader::getHorizPlane, field: " << _getFieldName() << endl;
    cerr << "  Bad URL: " << fullUrl << endl;
    return -1;
  }

  /////////////////////////////////////
  // set up Mdvx object for reading

  h_mdvx.clear();
  
  // get time list
  
  if(!_timeListValid) {
    if (_getTimeList(fullUrl, _midTime,  _page, h_mdvx)) {
      cerr << "MdvReader::getHorizPlane()::_getTimeList() failed" << endl;
      _timeListValid = false;
      return -1;
    }
  }
  
  // add field to request
  
  string fieldName(_getFieldName());
  h_mdvx.addReadField(fieldName);
  
  // read times
  
  _setReadTimes(fullUrl, _timeReq, h_mdvx);
  
  // vlevel
  
  h_mdvx.setReadVlevelLimits(_vLevelReq, _vLevelReq);
  if(composite_mode) {
    // Request column max composite
    h_mdvx.setReadComposite();
  }
  
  // clip to zoom limits

  if (_params.clip_to_current_zoom_on_mdv_request) {
    
    double dlat = _zoomBoxReq.getMaxLat() - _zoomBoxReq.getMinLat();
    double dlon = _zoomBoxReq.getMaxLon() - _zoomBoxReq.getMinLon();
    double dlatExtra = fabs(dlat) / 4.0;
    double dlonExtra = fabs(dlon) / 4.0;
    
    h_mdvx.setReadHorizLimits(_zoomBoxReq.getMinLat() - dlatExtra,
                              _zoomBoxReq.getMinLon() - dlonExtra,
                              _zoomBoxReq.getMaxLat() + dlatExtra,
                              _zoomBoxReq.getMaxLon() + dlonExtra);

  }
  
  // Mdvx Decimation is based on the sqrt of the value. - Choose the longer edge 

  if (_params.decimate_resolution_on_mdv_request) {
    h_mdvx.setReadDecimate(_gd.h_win.img_dim.width * _gd.h_win.img_dim.width);
  }
  
  h_mdvx.setReadEncodingType(Mdvx::ENCODING_ASIS);
  h_mdvx.setReadCompressionType(Mdvx::COMPRESSION_ASIS);
  if(_params.request_compressed_data) {
    if(_params.request_gzip_vol_compression) {
      h_mdvx.setReadCompressionType(Mdvx::COMPRESSION_GZIP_VOL);
    } else {
      h_mdvx.setReadCompressionType(Mdvx::COMPRESSION_ZLIB);
    }
  }

  // request full field header for vert info

  h_mdvx.setReadFieldFileHeaders();
  
  //if(_gd.debug1) {
  fprintf(stderr, "Get MDVX Horiz Plane - page : %d  -  %s\n", _page, fullUrl.c_str());
  // }

  //////////////////////////////////////////
  // read the volume
  
  int iret = h_mdvx.readVolume();
  if (iret) {
    cerr << "1111111 MdvReader::getHorizPlane, iret: " << ", " << iret << endl;
    qDebug() << "MdvReader::getHorizPlane returned, iret: " << ", " << iret;
    qDebug() << h_mdvx.getErrStr();
  }
  iret_h_mdvx_read = iret;
  last_collected = time(0);

  // uncompress return field
  
  if (iret == 0) {
    MdvxField *hfld = h_mdvx.getFieldByNum(0);
    if (hfld) {
      Mdvx::field_header_t fhdr = hfld->getFieldHeader();
      if(fhdr.encoding_type == Mdvx::ENCODING_RGBA32) {
        // Image data, uncompress
        hfld->convertType(Mdvx::ENCODING_ASIS, Mdvx::COMPRESSION_NONE);
      } else {
        // field data, convert to fl32 if needed and uncompress
        hfld->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
      }
    } // if (hfld)
  } // if (iret == 0)

  // get radar info if applicable
  
  if (mdvxRadar.loadFromMdvx(h_mdvx) == 0) {
    isRadar = true;
    radarParams = mdvxRadar.getRadarParams();
  } else {
    isRadar = false;
  }

  return 0;
  
}

////////////////////////////////////////////////////////////////////
// HANDLE READ WHEN IT IS DONE
// This copies data from the Mdvx object into the reader object.
// This is run in the main thread.
/////////////////////////////////////////////////////////////////////

int MdvReader::_handleHorizReadDone()
{

  // save time
  
  last_collected = time(0);
  
  if (iret_h_mdvx_read ||
      h_mdvx.getNFields() < 1 ||
      h_mdvx.getFieldByNum(0) == nullptr)  {
    
    // ERROR
    // Save the master header for the file, even if we couldn't
    // get data for this field.  This is needed in case we are
    // dealing with forecast data
    
    h_mhdr = h_mdvx.getMasterHeader();
    
    cerr << "1111111111111111111 _handleHorizReadDone ERROR" << endl;
    return -1;
    
  }

  ////////////////////////
  // success - data is in

  cerr << "111111111111111111111111111111111111111 _handleHorizReadDone GOT DATA" << endl;
  
  h_data_valid = 1;
  
  // copy the requested field

  MdvxField *hfld = h_mdvx.getFieldByNum(0);
  Mdvx::field_header_t fhdr = hfld->getFieldHeader();
  int nptsPlane = fhdr.ny * fhdr.nx;
  
  if(fhdr.encoding_type == Mdvx::ENCODING_RGBA32) {

    // save the data to the rgba32 vector
    
    h_rgba32_data.resize(nptsPlane);
    memcpy(h_rgba32_data.data(), hfld->getVol(), nptsPlane * sizeof(ui32));
    
  } else {
    
    // save the data to the fl32 vector
    
    h_fl32_data.resize(nptsPlane);
    memcpy(h_fl32_data.data(), hfld->getVol(), nptsPlane * sizeof(fl32));
    
  }
  
  // save headers for use later
  
  h_mhdr = h_mdvx.getMasterHeader();
  h_fhdr = hfld->getFieldHeader();
  h_vhdr = hfld->getVlevelHeader();
    
  // Init projection

  proj.init(h_fhdr);
    
  // condition longitudes to be in same hemisphere
  // as origin
  proj.setConditionLon2Origin(true);
    
  // Implemented for MOBILE RADARS - 

  if (_params.zoom_domain_follows_data &&
      this == _gd.mread[_gd.h_win.page]) { // Only for the primary field

    double dx,locx;
    double dy,locy;
    // int index = _gd.h_win.zoom_level;
      
    if(h_fhdr.proj_origin_lat != _gd.h_win.origin_lat ||
       h_fhdr.proj_origin_lon != _gd.h_win.origin_lon) {

      dx = _gd.h_win.cmax_x - _gd.h_win.cmin_x;
      dy = _gd.h_win.cmax_y - _gd.h_win.cmin_y;

      switch(_gd.display_projection) {
        case Mdvx::PROJ_LATLON:
          _gd.h_win.origin_lat = h_fhdr.proj_origin_lat;
          _gd.h_win.origin_lon = h_fhdr.proj_origin_lon;
          _gd.h_win.cmin_x = h_fhdr.proj_origin_lon - (dx / 2.0);
          _gd.h_win.cmax_x = h_fhdr.proj_origin_lon + (dx / 2.0);
          _gd.h_win.cmin_y = h_fhdr.proj_origin_lat - (dy / 2.0);
          _gd.h_win.cmax_y = h_fhdr.proj_origin_lat + (dy / 2.0);
          break;
        default:
          _gd.proj.latlon2xy(h_fhdr.proj_origin_lat,h_fhdr.proj_origin_lon,locx,locy);
          _gd.h_win.cmin_x = locx - (dx / 2.0);
          _gd.h_win.cmax_x = locx + (dx / 2.0);
          _gd.h_win.cmin_y = locy - (dy / 2.0);
          _gd.h_win.cmax_y = locy + (dy / 2.0);
          break;
      }
      
      /* Set current area to our indicated zoom area */
      // _gd.h_win.cmin_x = _gd.h_win.zmin_x;
      // _gd.h_win.cmax_x = _gd.h_win.zmax_x;
      // _gd.h_win.cmin_y = _gd.h_win.zmin_y;
      // _gd.h_win.cmax_y = _gd.h_win.zmax_y;
      
    }

  } // if (_params.zoom_domain_follows_data
  
  if (_gd.debug1) {
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
  if(hfld->getFieldHeaderFile() == nullptr) {
    ds_fhdr = (hfld->getFieldHeader());
  } else {
    ds_fhdr = *(hfld->getFieldHeaderFile());
  }
    
  if(hfld->getVlevelHeaderFile() == nullptr) {
    ds_vhdr = (hfld->getVlevelHeader());
  } else {
    ds_vhdr = *(hfld->getVlevelHeaderFile());
  }
    
  // Compute the vertical levels 
  plane = 0;
  vert.resize(ds_fhdr.nz);
  for(int ii = 0; ii < ds_fhdr.nz; ii++) { 
    // Find out which plane we received
    if(ds_vhdr.level[ii] == h_vhdr.level[0]) plane = ii;
    if(ii == 0) { // Lowest plane 
      double delta = (ds_vhdr.level[ii+1] - ds_vhdr.level[ii]) / 2.0;
      vert[ii].min = ds_vhdr.level[0] - delta;
      vert[ii].cent = ds_vhdr.level[0];
      vert[ii].max = ds_vhdr.level[0] + delta;
      
    } else if (ii == ds_fhdr.nz -1) { // highest plane
      double delta = (ds_vhdr.level[ii] - ds_vhdr.level[ii-1]) / 2.0;
      vert[ii].min = ds_vhdr.level[ii] - delta;
      vert[ii].cent = ds_vhdr.level[ii];
      vert[ii].max = ds_vhdr.level[ii] + delta;
        
    } else { // Middle planes
      double delta = (ds_vhdr.level[ii] - ds_vhdr.level[ii-1]) / 2.0;
      vert[ii].min = ds_vhdr.level[ii] - delta;
      vert[ii].cent = ds_vhdr.level[ii];
        
      delta = (ds_vhdr.level[ii+1] - ds_vhdr.level[ii]) / 2.0;
      vert[ii].max = ds_vhdr.level[ii] + delta;
    }
  } // ii
    
  // Record the dimensional Units of the volume
  
  units_label_cols = h_mdvx.projType2XUnits(h_fhdr.proj_type);
  units_label_rows = h_mdvx.projType2YUnits(h_fhdr.proj_type);
  units_label_sects = h_mdvx.vertTypeZUnits(h_vhdr.type[0]);
    
  // Record the time

  h_date.set(h_mhdr.time_centroid);
  
  cerr << "1111111111111111111 _handleHorizReadDone END" << endl;

  // free up

  h_mdvx.clear();

  return 0;
  
}

////////////////////////////////////////////////////////////
// check whether H request has changed
// returns true if changed, false if no change

bool MdvReader::_checkRequestChangedH(const DateTime &midTime,
                                      double vLevel)
{

  DateTime reqTime;
  _computeReqTime(midTime, reqTime);
  
  // compute requested zoom domain

  LatLonBox zoomBoxReq;
  if(_params.clip_to_current_zoom_on_mdv_request) {
    double min_lat, max_lat, min_lon, max_lon;
    _getBoundingBox(min_lat, max_lat, min_lon, max_lon);
    zoomBoxReq.setLimits(min_lat, max_lat, min_lon, max_lon);
  } else {
    zoomBoxReq.clearLimits();
  }

  // check if vlevel has changed
  
  bool vLevelChanged = false;
  if(!composite_mode) {
    if (fabs(vLevel - _vLevelReq) > 1.0e-5) {
      vLevelChanged = true;
    }
  }

  // check if request is unchanged from previous call

  if(_gd.debug1) {
    cerr << "MdvReader::_checkRequestChangedH" << endl;
    if (!_validH) {
      cerr << "  _validH is false" << endl;
    }
    if (reqTime != _timeReq) {
      cerr << "  reqTime has changed: "
           << _timeReq.asString(0) << " to " << reqTime.asString(0) << endl;
    }
    if (zoomBoxReq != _zoomBoxReq) {
      cerr << "  zoomBoxReq has changed:" << endl;
      _zoomBoxReq.print(cerr);
      cerr << "  to:" << endl;
      zoomBoxReq.print(cerr);
    }
    if (vLevelChanged) {
      cerr << "vLevelChanged, vLevel, _vLevelReq, vLevelChanged: "
           << vLevel << ", " << _vLevelReq << endl;
    }
  }
  
  QMutexLocker locker(&_statusMutex);
  if (_validH &&
      reqTime == _timeReq &&
      zoomBoxReq == _zoomBoxReq &&
      !vLevelChanged) {
    // no changes, use what we have, mark as new
    return false;
  }
  
  if(_gd.debug2) {
    cerr << "MdvReader::_checkRequestChangedH - TRUE" << endl;
  }
  
  // save request for comparison next time

  _timeReq = reqTime;
  _zoomBoxReq = zoomBoxReq;
  _vLevelReq = vLevel;
  
  return true;

}

/**********************************************************************
 * Request data for a vertical section
 */

int MdvReader::requestVertSection(const DateTime &midTime,
                                  int page)
{

#ifdef NOTYET
  
  // Construct URL, check is valid.
  
  string fullUrl(_getFullUrl());
  DsURL URL(fullUrl);
  if(URL.isValid() != TRUE) {
    v_data_valid = 1;
    return -1;
  }
  URL.getURLStr();
  
  // add field to request
  
  string fieldName(_getFieldName());
  v_mdvx->clearRead(); 
  v_mdvx->addReadField(fieldName);
  // v_mdvx->setThreadingOff();
  if(_gd.debug1) {
    fprintf(stderr, "Get MDVX Vert Plane - page : %d  -  %s\n", page, fullUrl.c_str());
    // Disable threading while in debug mode
  }

  // time
  
  _midTime = midTime;
  DateTime reqTime;
  _computeReqTime(midTime, reqTime);

  // set read time

  _setReadTimes(fullUrl, reqTime, v_mdvx);

  // other settings
  
  v_mdvx->setReadEncodingType(Mdvx::ENCODING_ASIS);
  v_mdvx->setReadCompressionType(Mdvx::COMPRESSION_ASIS);
  
  if(_params.request_compressed_data) {
    if(_params.request_gzip_vol_compression) {
      v_mdvx->setReadCompressionType(Mdvx::COMPRESSION_GZIP_VOL);
    } else {
      v_mdvx->setReadCompressionType(Mdvx::COMPRESSION_ZLIB);
    }
  }
  
  int num_way_points = _gd.h_win.route.num_segments + 1;

  for(int ii = 0; ii < num_way_points; ii++) {
    double tlat, tlon;
    _gd.proj.xy2latlon(_gd.h_win.route.x_world[ii],
                      _gd.h_win.route.y_world[ii],
                      tlat, tlon);
    v_mdvx->addReadWayPt(tlat,tlon);
    if (ii == 0) {
      // lat1 = tlat;
      // lon1 = tlon;
    } else if (ii == num_way_points - 1) {
      // lat2 = tlat;
      // lon2 = tlon;
    }
    
    if(_gd.debug1) {
      fprintf(stderr,"Way pt Lat, Lon, Len: %g, %g, %g\n",
              tlat, tlon, _gd.h_win.route.seg_length[ii]);
    }
    
  } // ii

  // Initiate the request
  v_mdvx->readVsection();

#endif
  
  return 0;
}

////////////////////////////////////////////////////////////
// compute request time from mid time

void MdvReader::_computeReqTime(const DateTime &midTime,
                                DateTime &reqTime)
{

  TimeControl *tc = TimeControl::getInstance();
  double frameIntervalSecs = _params.frame_interval_secs;
  if (tc != nullptr) {
    frameIntervalSecs = TimeControl::getInstance()->getFrameIntervalSecs();
  }
  
  // compute requested time
  
  switch(_params.gather_data_mode) {
    case Params::FIRST_BEFORE_END_OF_FRAME: {
      reqTime = midTime + frameIntervalSecs / 2.0;
      break;
    }
    case Params::FIRST_AFTER_START_OF_FRAME: {
      reqTime = midTime - frameIntervalSecs / 2.0;
      break;
    }
    case Params::CLOSEST_TO_FRAME_CENTER:
    default: {
      reqTime = midTime;
      break;
    }
  } // gather_data_mode

}

////////////////////////////////////////////////////////////
// set read times for Mdvx object

void MdvReader::_setReadTimes(const string &url,
                              const DateTime &reqTime,
                              Mdvx &mdvx)
{
  
  _gd.data_request_time = reqTime.utime();
  
  switch(_params.gather_data_mode ) {
    
    case Params::CLOSEST_TO_FRAME_CENTER:
      if(_gd.model_run_time != 0 && h_mhdr.data_collection_type == Mdvx::DATA_FORECAST) { 
        mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                          url,
                          (int) (60 * time_allowance),
                          _gd.model_run_time,
                          reqTime.utime() - _gd.model_run_time);
      } else {
        mdvx.setReadTime(Mdvx::READ_CLOSEST,
                          url,
                          (int) (60 * time_allowance),
                          reqTime.utime());
      }
      if(_gd.debug1) {
        string fieldName(_getFieldName());
        fprintf(stderr,
                "{ READ_VOLUME CLOSEST_TIME, \"%s\", \"%s\", %d, %ld },\n",
                url.c_str(), fieldName.c_str(),
                (int) (60 * time_allowance), reqTime.utime() + 1);
      }
      return;
      
    case Params::FIRST_BEFORE_END_OF_FRAME:
      if(_gd.model_run_time != 0 && h_mhdr.data_collection_type == Mdvx::DATA_FORECAST) { 
        mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                          url,
                          (int) (60 * time_allowance),
                          _gd.model_run_time,
                          (reqTime.utime() - _gd.model_run_time));
      } else {
        mdvx.setReadTime(Mdvx::READ_FIRST_BEFORE,
                          url,
                          (int) (60 * time_allowance),
                          reqTime.utime() + 1, 0);
      }
      if(_gd.debug1) {
        string fieldName(_getFieldName());
        fprintf(stderr,
                "{ READ_VOLUME FIRST_BEFORE_END, \"%s\", \"%s\", %d, %ld },\n",
                url.c_str(), fieldName.c_str(),
                (int) (60 * time_allowance), reqTime.utime() + 1);
      }
      return;
      
    case Params::FIRST_AFTER_START_OF_FRAME:
      if(_gd.model_run_time != 0 && h_mhdr.data_collection_type ==  Mdvx::DATA_FORECAST) { 
        mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                          url,
                          (int) (60 * time_allowance),
                          _gd.model_run_time,
                          (reqTime.utime() - _gd.model_run_time));
      } else {
        mdvx.setReadTime(Mdvx::READ_FIRST_AFTER,
                          url,
                          (int) (60 * time_allowance),
                          reqTime.utime()-1, 0);
      }
      if(_gd.debug1) {
        string fieldName(_getFieldName());
        fprintf(stderr,
                "{ READ_VOLUME FIRST_AFTER_START, \"%s\", \"%s\", %d, %ld },\n",
                url.c_str(), fieldName.c_str(),
                (int) (60 * time_allowance), reqTime.utime() - 1);
      }
      return;
      
  } // gather_data_mode

}

/**********************************************************************
 * REQUEST_TIME_LIST: Query the server for a data set time list
 */

int MdvReader::_getTimeList(const string &url,
                            const DateTime &midTime,
                            int page,
                            Mdvx &mdvx)
{

  TimeControl *tc = TimeControl::getInstance();
  if (tc == nullptr) {
    // TimeControl does not yet exist
    return -1;
  }
  
  double movieDurationSecs = tc->getMovieDurationSecs();
  DateTime movieStartTime(tc->getStartTime());
  DateTime movieEndTime(movieStartTime + movieDurationSecs);
  time_t delta = movieEndTime - movieStartTime;
  DateTime listStartTime(movieStartTime - delta);
  DateTime listEndTime(movieEndTime + delta);
  
  if(_gd.debug1) {
    cerr << "Get MDVX Time List, page: " << page << ", url: " << url << endl;
    cerr << "  Movie start time: " << movieStartTime.asString(0) << endl;
    cerr << "  Movie end time: " << movieEndTime.asString(0) << endl;
    cerr << "  List start time: " << listStartTime.asString(0) << endl;
    cerr << "  List end time: " << listEndTime.asString(0) << endl;
  }

  mdvx.clearTimeListMode();
  if(_gd.model_run_time != 0 &&
     h_mhdr.data_collection_type == Mdvx::DATA_FORECAST) { 
    mdvx.setTimeListModeLead(url, _gd.model_run_time);
  } else {
    mdvx.setTimeListModeValid(url,
                               listStartTime.utime(), listEndTime.utime());
  }
  if(_gd.debug1) {
    cerr << "Gathering Time List for url: " << url << endl;
  }

  char label[1024];
  snprintf(label, 1024, "Requesting time list for %s data", legend_name.c_str());

  double min_lat, max_lat, min_lon, max_lon;
  _getBoundingBox(min_lat, max_lat, min_lon, max_lon);
  
  if (_params.clip_to_current_zoom_on_mdv_request) {
    mdvx.setReadHorizLimits(min_lat, min_lon, max_lat, max_lon);
  }
  
  if(_gd.debug1) {
    string fullUrl(_getFullUrl());
    fprintf(stderr,
            "{ TIME_LIST_REQUEST, \"%s\", \"%s\", %ld, %ld, %g, %g, %g, %g, %g },\n",
            fullUrl.c_str(), "none",
            _gd.epoch_start - delta, _gd.epoch_end + delta,
            -99.9, min_lat, min_lon, max_lat, max_lon);
  }

  // Limit List requests to given maximum number of days 
  if((_gd.epoch_end - _gd.epoch_start) <
     (_params.climo_max_time_span_days * 1440  * 60)) {
    if (mdvx.compileTimeList()) {
      cerr << "ERROR -CIDD:  setTimeListModeValid" << endl;
      cerr << mdvx.getErrStr();
      _timeListValid = true;
    }  
  } else {
    // _gd.io_info.outstanding_request = 0;
    _timeListValid = true;
  }

  return 0;  // return from this request.
  
  // When the thread is done the check_for_io function will gather and store the
  // timelist info and then set _timeListValid = 1 (true);
  
}


/**********************************************************************
 * get a fully-qualified URL
 *
 */

string MdvReader::_getFullUrl()
{
  
  // url from met record
  
  string fullUrl(url);

  // check for tunnel
  
  if(strlen(_params.http_tunnel_url) < Constants::URL_MIN_SIZE) {
    return fullUrl;
  }

  // Using a tunnel - add the tunnel_url to the Url as a name=value pair.
  // If using the a proxy - Add the a proxy_url to the Url as a name=value pair
  
  char tmp_str[1024];
  MEM_zero(tmp_str);
  if((strlen(_params.http_proxy_url)) > Constants::URL_MIN_SIZE) {
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

string MdvReader::_getFieldName()
{
  string tmpName(field_label);
  while (true) {
    size_t pos = tmpName.find('^');
    if (pos == string::npos) {
      return tmpName;
    }
    tmpName.replace(pos, 1, '_', 1);
  }
  return tmpName;
}

//////////////////////////////////////////////////////
// is the data valid?

bool MdvReader::isValidH() const {
  QMutexLocker locker(&_statusMutex);
  return _validH;
}

bool MdvReader::isValidV() const {
  QMutexLocker locker(&_statusMutex);
  return _validV;
}

void MdvReader::_setValidH(bool state) {
  QMutexLocker locker(&_statusMutex);
  _validH = state;
}

void MdvReader::_setValidV(bool state) {
  QMutexLocker locker(&_statusMutex);
  _validV = state;
}

//////////////////////////////////////////////////////
// is the data new?
// if new is true, sets to false and returns true

bool MdvReader::isNewH() const {
  QMutexLocker locker(&_statusMutex);
  bool isNew = _newH;
  _newH = false;
  return isNew;
}

bool MdvReader::isNewV() const {
  QMutexLocker locker(&_statusMutex);
  bool isNew = _newV;
  _newV = false;
  return isNew;
}

void MdvReader::_setNewH(bool state) {
  QMutexLocker locker(&_statusMutex);
  _newH = state;
}

void MdvReader::_setNewV(bool state) {
  QMutexLocker locker(&_statusMutex);
  _newV = state;
}


/**************************************************************************
 * GET BOUNDING BOX
 * Compute the current lat,lon bounding box of data on the display
 */

void MdvReader::_getBoundingBox(double &min_lat, double &max_lat,
                                double &min_lon, double &max_lon)
{

  // init
  
  max_lon = -360.0;
  min_lon = 360.0;
  max_lat = -180.0;
  min_lat = 180.0;
    
  // condition the longitudes for this zoom
  
  double meanx = (_gd.h_win.cmin_x + _gd.h_win.cmax_x) / 2.0;
  double meany = (_gd.h_win.cmin_y + _gd.h_win.cmax_y) / 2.0;
  
  // Make sure meanLon makes sense
  
  double meanLat, meanLon;
  _gd.proj.xy2latlon(meanx, meany, meanLat, meanLon);
  if (meanLon > _gd.h_win.max_x) {
    meanLon -= 360.0;
  } else if (meanLon < _gd.h_win.min_x) {
    meanLon += 360.0;
  }
  _gd.proj.setConditionLon2Ref(true, meanLon);

  // set box limits
  
  if(!_params.clip_to_current_zoom_on_mdv_request) {

    // use win limits
    
    _gd.proj.xy2latlon(_gd.h_win.min_x, _gd.h_win.min_y, min_lat, min_lon);
    _gd.proj.xy2latlon(_gd.h_win.max_x, _gd.h_win.max_y, max_lat, max_lon);
    
  } else if (_gd.display_projection == Mdvx::PROJ_LATLON) {
    
    // latlon projection
    
    double lon1 = _gd.h_win.cmin_x;
    double lon2 = _gd.h_win.cmax_x;
    
    if((lon2 - lon1) > 360.0) {
      lon1 = _gd.h_win.min_x;
      lon2 = _gd.h_win.max_x; 
    }
    
    double lat1 = _gd.h_win.cmin_y;
    double lat2 = _gd.h_win.cmax_y;
    if((lat2 - lat1) > 360.0) {
      lat1 = _gd.h_win.min_y;
      lat2 = _gd.h_win.max_y; 
    }
    
    _gd.proj.xy2latlon(lon1, lat1, min_lat, min_lon);
    _gd.proj.xy2latlon(lon2, lat2, max_lat, max_lon);

  } else {

    // all other projections

    // Compute the bounding box
    // Check each corner of the projection + 4 mid points, top, bottom
    // Left and right 
    
    // Lower left
    double lat,lon;
    _gd.proj.xy2latlon(_gd.h_win.cmin_x , _gd.h_win.cmin_y, lat, lon);
    _adjustBoundingBox(lat, lon, min_lat, max_lat, min_lon, max_lon);
    
    // Lower midpoint
    _gd.proj.xy2latlon((_gd.h_win.cmin_x + _gd.h_win.cmax_x)/2, _gd.h_win.cmin_y, lat, lon);
    _adjustBoundingBox(lat, lon, min_lat, max_lat, min_lon, max_lon);
    
    // Lower right
    _gd.proj.xy2latlon(_gd.h_win.cmax_x,_gd.h_win.cmin_y, lat, lon);
    _adjustBoundingBox(lat, lon, min_lat, max_lat, min_lon, max_lon);
    
    // Right midpoint
    _gd.proj.xy2latlon(_gd.h_win.cmax_x, (_gd.h_win.cmin_y + _gd.h_win.cmax_y)/2, lat, lon);
    _adjustBoundingBox(lat, lon, min_lat, max_lat, min_lon, max_lon);
    
    // Upper right
    _gd.proj.xy2latlon(_gd.h_win.cmax_x, _gd.h_win.cmax_y, lat, lon);
    _adjustBoundingBox(lat, lon, min_lat, max_lat, min_lon, max_lon);
    
    // Upper midpoint
    _gd.proj.xy2latlon((_gd.h_win.cmin_x + _gd.h_win.cmax_x)/2, _gd.h_win.cmax_y, lat, lon);
    _adjustBoundingBox(lat, lon, min_lat, max_lat, min_lon, max_lon);
    
    // Upper left
    _gd.proj.xy2latlon(_gd.h_win.cmin_x, _gd.h_win.cmax_y, lat, lon);
    _adjustBoundingBox(lat, lon, min_lat, max_lat, min_lon, max_lon);
    
    // Left midpoint
    _gd.proj.xy2latlon(_gd.h_win.cmin_x, (_gd.h_win.cmin_y + _gd.h_win.cmax_y)/2, lat, lon);
    _adjustBoundingBox(lat, lon, min_lat, max_lat, min_lon, max_lon);
    
  } // if(_params.always_get_full_domain)
  
  if(_gd.display_projection == Mdvx::PROJ_LATLON ) {
    double originLon = (min_lon + max_lon) / 2.0;
    _gd.proj.initLatlon(originLon);
  }
  
}

///////////////////////////////////////////////////////////////////
// adjust bounding box lat lon limits

void MdvReader::_adjustBoundingBox(double lat, double lon,
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

ReadVolH::ReadVolH(MdvReader* parentObject, QObject* parent) :
        QObject(parent), _mr(parentObject)
{
}

void ReadVolH::doRead() {
  qDebug() << "ReadVolH thread started. Parent object:" << _mr;
  int iret = _mr->getHorizPlane();
  qDebug() << "ReadVolH returned, iret: " << ", " << iret;
  emit readDone();
}

//////////////////////////////
// start H vol read in thread

void MdvReader::startReadVolH() {
  if (_readBusyH) {
    qDebug() << "readVolH() is already in progress. Ignoring new request.";
    return;
  }
  _readBusyH = true;
  ReadVolH* worker = new ReadVolH(this); // Pass the current object as reference
  QThread* thread = new QThread;
  worker->moveToThread(thread);
  // Connect signals and slots
  connect(thread, &QThread::started, worker, &ReadVolH::doRead);
  connect(worker, &ReadVolH::readDone, this, &MdvReader::readDoneH);
  // connect(worker, &ReadVolH::readDone, thread, &QThread::quit);
  connect(thread, &QThread::finished, worker, &ReadVolH::deleteLater);
  connect(thread, &QThread::finished, thread, &QThread::deleteLater);
  thread->start();
}

////////////////////////
// done with H vol read

void MdvReader::readDoneH()
{
  int iret = _handleHorizReadDone();
  if (iret || !h_data_valid) {
    _setValidH(false);
    _setNewH(false);
    _readBusyH = false;
    return;
  }
  _setValidH(iret_h_mdvx_read == 0);
  _setNewH(true);
  qDebug() << "readDone in ReadVolH worker thread";
  _readBusyH = false;
}

/*****************************************************************
 * AUTOSCALE_VCM: Autoscale the value-color mapping
 *        color to use
 *
 */

void MdvReader::_autoscaleVcm(Valcolormap_t *vcm, double min, double max)
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
 * VLEVEL_LABEL: Return an appropriate units label for a field's
 *               vertical coordinate system
 */                  
string MdvReader::vlevelLabel()
{
  switch(h_fhdr.vlevel_type)  {
    case Mdvx::VERT_TYPE_SIGMA_P:
      return("Sigma P");
    case Mdvx::VERT_TYPE_PRESSURE:
      return("mb");
    case Mdvx::VERT_TYPE_Z:
      return("km");
    case Mdvx::VERT_TYPE_SIGMA_Z:
      return("Sigma Z");
    case Mdvx::VERT_TYPE_ETA:
      return("ETA");
    case Mdvx::VERT_TYPE_THETA:
      return("Theta");
    case Mdvx::VERT_TYPE_MIXED:
      return("Mixed");
    case Mdvx::VERT_TYPE_ELEV:
    case Mdvx::VERT_VARIABLE_ELEV:
      return("Deg");
    case Mdvx::VERT_FLIGHT_LEVEL:
      return("FL");
    default:
      return "";
  }
}

/**********************************************************************
 * FIELD_LABEL: Return a Label string for a field 
 */

string MdvReader::fieldLabel()
{

  
  // Convert time to a string
  
  string tlabel =
    h_date.strfTime(_params.label_time_format, !_params.use_local_timestamps);

  char label[2048];
  label[0] = '\0';
  
  if(composite_mode == FALSE) {
    // Check for surface fields 
    if(((vert[plane].cent == 0.0) && (h_fhdr.grid_dz == 0)) ||
       h_fhdr.vlevel_type == Mdvx::VERT_TYPE_SURFACE ||
       h_fhdr.vlevel_type == Mdvx::VERT_TYPE_COMPOSITE ||
       ds_fhdr.nz == 1) {  
      //snprintf(label,"%s At Surface %s",
      snprintf(label,2048,"%s: %s",
               legend_name.c_str(),
               tlabel.c_str());
    } else {
      // Reverse order of label and value if units are "FL" 
      if(units_label_sects == "FL") {
        snprintf(label, 2048, "%s: %s %03.0f %s",
                 legend_name.c_str(),
                 units_label_sects.c_str(),
                 vert[plane].cent,
                 tlabel.c_str());
      }else {
        snprintf(label,2048,"%s: %g %s %s",
                 legend_name.c_str(),
                 vert[plane].cent,
                 units_label_sects.c_str(),
                 tlabel.c_str());
      }
    }
  } else {
    snprintf(label,2048,"%s: All levels %s",
             legend_name.c_str(),
             tlabel.c_str());
  }
  
  /* If data is Forecast, add a  label */
  // if( h_mhdr.data_collection_type == Mdvx::DATA_FORECAST) {
  //   strcat(label," FORECAST Gen");
  // }
  
  return label;
  
}

/*************************************************************************
 * HEIGHT_LABEL: Return the height label
 */
string MdvReader::heightLabel()
{

  static char label[1024];
  
  switch(h_vhdr.type[0]) {
    case Mdvx::VERT_TYPE_SURFACE:
      strcpy(label,"   " );    // For Now-  Because many Taiwan fields are wrongly
      // set to "Surface", example: Freezing Level.
      //strcpy(label,"Surface" );
      break;
      
    case 0:
    case Mdvx::VERT_VARIABLE_ELEV:
    case Mdvx::VERT_FIELDS_VAR_ELEV:
    case Mdvx::VERT_TYPE_COMPOSITE:
      strcpy(label,"   " );
      break;
      
    case Mdvx::VERT_SATELLITE_IMAGE:
      strcpy(label,"Sat Image" );
      break;
      
    case Mdvx::VERT_FLIGHT_LEVEL:
      snprintf(label,1024,"Flight Level %03d", (int)_gd.h_win.cur_ht);
      break;
      
    default:
      snprintf(label,1024,"%g %s", _gd.h_win.cur_ht,
               units_label_sects.c_str());
      break;
  }
  
  return label;
  
}

