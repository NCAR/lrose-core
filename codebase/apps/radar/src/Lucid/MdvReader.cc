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
#include <Mdv/MdvxField.hh>
#include <qtplot/ColorMap.hh>

///////////////////////////////////////////////
// constructor

MdvReader::MdvReader(QObject* parent) :
        _params(Params::Instance()),
        _gd(GlobalData::Instance()),
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
  vert_type = 0;
  // alt_offset = 0.0;
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
  
  MEM_zero(units_label_cols);
  MEM_zero(units_label_rows);
  MEM_zero(units_label_sects);
  MEM_zero(vunits_label_cols);
  MEM_zero(vunits_label_rows);
  MEM_zero(vunits_label_sects);
  
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

/**********************************************************************
 * REQUEST_HORIZ_DATA_PLANE: Get data for a plane
 */

int MdvReader::getHorizPlane()
{

  // Construct URL, check is valid.
  
  string fullUrl(_getFullUrl());
  DsURL URL(fullUrl);  
  if(!URL.isValid()) {
    cerr << "ERROR - MdvReader::getHorizPlane, field: " << _getFieldName() << endl;
    cerr << "  Bad URL: " << fullUrl << endl;
    h_data_valid = 1;
    return -1;
  }
  
  // clear
  
  h_mdvx->clearRead(); 
  
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
  h_mdvx->addReadField(fieldName);
  
  // read times
  
  _setReadTimes(fullUrl, _timeReq, h_mdvx);
  
  // vlevel
  
  h_mdvx->setReadVlevelLimits(_vLevelReq, _vLevelReq);
  if(composite_mode) {
    // Ask for data that covers the whole vertical domain 
    h_mdvx->setReadComposite();
  }
  
  // zoom

  double dlat = _zoomBoxReq.getMaxLat() - _zoomBoxReq.getMinLat();
  double dlon = _zoomBoxReq.getMaxLon() - _zoomBoxReq.getMinLon();
  double dlatExtra = fabs(dlat) / 4.0;
  double dlonExtra = fabs(dlon) / 4.0;

  h_mdvx->setReadHorizLimits(_zoomBoxReq.getMinLat() - dlatExtra,
                             _zoomBoxReq.getMinLon() - dlonExtra,
                             _zoomBoxReq.getMaxLat() + dlatExtra,
                             _zoomBoxReq.getMaxLon() + dlonExtra);
  
  // Mdvx Decimation is based on the sqrt of the value. - Choose the longer edge 
  if (!_params.do_not_decimate_on_mdv_request) {
    h_mdvx->setReadDecimate(_gd.h_win.img_dim.width * _gd.h_win.img_dim.width);
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

  // _gd.io_info.mode = DSMDVX_DATA;
  // _gd.io_info.request_type = HORIZ_REQUEST;    /* Horizontal data access */
  // _gd.io_info.expire_time = time(0) + _params.data_timeout_secs;
  // _gd.io_info.busy_status = 0;
  // _gd.io_info.page = _page;
  // _gd.io_info.outstanding_request = 1;
  // _gd.io_info.mr = this;
  
  //if(_gd.debug1) {
  fprintf(stderr, "Get MDVX Horiz Plane - page : %d  -  %s\n", _page, fullUrl.c_str());
  // }
  
  // Initiate the request in thread
  
  int iret = h_mdvx->readVolume();
  if (iret) {
    cerr << "1111111 MdvReader::getHorizPlane, iret: " << ", " << iret << endl;
    qDebug() << "MdvReader::getHorizPlane returned, iret: " << ", " << iret;
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
      
    // _gd.io_info.busy_status = 0;
    // _gd.io_info.outstanding_request = 0;
    // _gd.io_info.request_type = 0;
    // _gd.h_win.redraw_flag[_gd.io_info.page] = 1;

    cerr << "1111111111111111111 getHorizPlane ERROR" << endl;
    return -1;
    
  } else if (h_mdvx->getFieldByNum(0) == NULL) {
    
    // _gd.io_info.busy_status = 0;
    // _gd.io_info.outstanding_request = 0;
    // _gd.io_info.request_type = 0;
    // _gd.h_win.redraw_flag[_gd.io_info.page] = 1;
    h_data_valid = 1;
    last_collected = time(0);

    cerr << "1111111111111111111 getHorizPlane ERROR" << endl;
    return -1;

  }

  // data is in
  
  cerr << "111111111111111111111111111111111111111 getHorizPlane GOT DATA" << endl;

  if (h_mdvx->getNFields() < 1 || h_mdvx->getFieldByNum(0) == NULL)  {
    cerr << "1111111111111111111 getHorizPlane ERROR - no fields returned" << endl;
    return -1;
  }
  
  // Indicate data update is in progress.
  
  // _gd.io_info.busy_status = 1;
  
  MdvxField *hfld = h_mdvx->getFieldByNum(0);
  if(hfld->getFieldHeader().encoding_type == Mdvx::ENCODING_RGBA32) {
    hfld->convertType(Mdvx::ENCODING_ASIS, Mdvx::COMPRESSION_NONE);
  } else{
    hfld->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  }
  Mdvx::field_header_t fhdr = hfld->getFieldHeader();
  if(fhdr.encoding_type != Mdvx::ENCODING_RGBA32) {

    *h_mdvx_int16 = *hfld; // Copy for INT16 data
    
    if(h_vcm.nentries < 2 || fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
      
      h_mdvx_int16->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_NONE);

    } else {
      
      // Convert the copy to - Decompressed INT16 - Covering the range of the colorscale
      double range = (h_vcm.vc[h_vcm.nentries-1]->max - h_vcm.vc[0]->min);
      double scale = range / (Constants::MAX_COLOR_CELLS -2);
      double bias = h_vcm.vc[0]->min - (2 * scale); // Preserve 0, 1 as legitimate NAN values
      
      h_mdvx_int16->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_NONE,
                                Mdvx::SCALING_SPECIFIED,scale,bias);
    }
    
    // Record where int8 data is in memory. - Used for fast polygon fills.
    h_data = (unsigned short *) h_mdvx_int16->getVol();
    
    // Convert the AS-IS to 32 bits float. - Used for Contouring, Data reporting.
    // cerr << "1111111111111111111 getNFields(): " << h_mdvx->getNFields() << endl;
    // cerr << "11111111111111111 h_mdvx->getFieldByNum(0): " << h_mdvx->getFieldByNum(0) << endl;
    // hfld->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
    // Record where float data is in memory.
    h_fl32_data = (fl32  *) hfld->getVol();
      
    // Find Headers for quick reference
    h_mhdr = h_mdvx->getMasterHeader();
    h_fhdr = hfld->getFieldHeader();
    h_vhdr = hfld->getVlevelHeader();
    // h_fhdr = h_mdvx_int16->getFieldHeader();
    // h_vhdr = h_mdvx_int16->getVlevelHeader();

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
     this == _gd.mread[_gd.h_win.page] ) { // Only for the primary field
    double dx,locx;
    double dy,locy;
    int index = _gd.h_win.zoom_level;
      
    if(h_fhdr.proj_origin_lat != _gd.h_win.origin_lat ||
       h_fhdr.proj_origin_lon != _gd.h_win.origin_lon) {
        
      dx = _gd.h_win.zmax_x[index] - _gd.h_win.zmin_x[index];
      dy = _gd.h_win.zmax_y[index] - _gd.h_win.zmin_y[index];
        
      switch(_gd.display_projection) {
        case Mdvx::PROJ_LATLON:
          _gd.h_win.origin_lat = h_fhdr.proj_origin_lat;
          _gd.h_win.origin_lon = h_fhdr.proj_origin_lon;
            
          _gd.h_win.zmin_x[index] = h_fhdr.proj_origin_lon - (dx / 2.0);
          _gd.h_win.zmax_x[index] = h_fhdr.proj_origin_lon + (dx / 2.0);
          _gd.h_win.zmin_y[index] = h_fhdr.proj_origin_lat - (dy / 2.0);
          _gd.h_win.zmax_y[index] = h_fhdr.proj_origin_lat + (dy / 2.0);
            
          break;
            
        default:
          _gd.proj.latlon2xy(h_fhdr.proj_origin_lat,h_fhdr.proj_origin_lon,locx,locy);
            
          _gd.h_win.zmin_x[index] = locx - (dx / 2.0);
          _gd.h_win.zmax_x[index] = locx + (dx / 2.0);
          _gd.h_win.zmin_y[index] = locy - (dy / 2.0);
          _gd.h_win.zmax_y[index] = locy + (dy / 2.0);
            
          break;
            
      }
      /* Set current area to our indicated zoom area */
      _gd.h_win.cmin_x = _gd.h_win.zmin_x[index];
      _gd.h_win.cmax_x = _gd.h_win.zmax_x[index];
      _gd.h_win.cmin_y = _gd.h_win.zmin_y[index];
      _gd.h_win.cmax_y = _gd.h_win.zmax_y[index];
	
      // reset_time_list_valid_flags();
      // reset_data_valid_flags(1,0);
      // reset_terrain_valid_flags(1,0);
      // set_redraw_flags(1,0);
      // h_data_valid = 1;  // This field is still valid, though
      // _timeListValid = true;
	
    }
  }
  
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
      
    if(auto_scale) {
      _autoscaleVcm(&(h_vcm), h_fhdr.min_value, h_fhdr.max_value);
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
          h_mdvx->projType2XUnits(h_fhdr.proj_type),Constants::LABEL_LENGTH);
  STRcopy(units_label_rows,
          h_mdvx->projType2YUnits(h_fhdr.proj_type),Constants::LABEL_LENGTH);
  STRcopy(units_label_sects,
          h_mdvx->vertTypeZUnits(h_vhdr.type[0]),Constants::LABEL_LENGTH);
    
  // Record the date
  h_date.set(h_mhdr.time_centroid);
  // UTIMunix_to_date(h_mhdr.time_centroid,&h_date);
    
  h_data_valid = 1;
  last_collected = time(0);
  // _gd.h_win.redraw_flag[_gd.io_info.page] = 1;
  // // Indicate its safe to use the data
  // _gd.io_info.busy_status = 0;
  // // Indicate data is no longer pending
  // _gd.io_info.outstanding_request = 0;
  // _gd.io_info.request_type = 0;
    
  // if (_params.show_data_messages) {
  //   // gui_label_h_frame("Done",-1);
  // } else {
  //   // set_busy_state(0);
  // }


  cerr << "1111111111111111111 getHorizPlane END" << endl;

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
  if(!_params.always_get_full_domain) {
    double min_lat, max_lat, min_lon, max_lon;
    _getBoundingBox(min_lat, max_lat, min_lon, max_lon);
    zoomBoxReq.setLimits(min_lat, max_lat, min_lon, max_lon);
  } else if (_params.do_not_clip_on_mdv_request) {
    zoomBoxReq.clearLimits();
  } else {
    zoomBoxReq.setLimits(-90.0, 90.0,
                         _gd.h_win.origin_lon - 179.9999,
                         _gd.h_win.origin_lon + 179.9999);
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

  // _gd.io_info.mode = DSMDVX_DATA;
  // _gd.io_info.expire_time = time(0) + _params.data_timeout_secs;
  // _gd.io_info.busy_status = 0;
  // _gd.io_info.page = page;
  // _gd.io_info.request_type = VERT_REQUEST;    /* a vertical data access */
  // _gd.io_info.outstanding_request = 1;
  // _gd.io_info.mr = this;

  // Initiate the request
  v_mdvx->readVsection();

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
                              Mdvx *mdvx)
{
  
  _gd.data_request_time = reqTime.utime();
  
  switch(_params.gather_data_mode ) {
    
    case Params::CLOSEST_TO_FRAME_CENTER:
      if(_gd.model_run_time != 0 && h_mhdr.data_collection_type == Mdvx::DATA_FORECAST) { 
        mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                          url,
                          (int) (60 * time_allowance),
                          _gd.model_run_time,
                          reqTime.utime() - _gd.model_run_time);
      } else {
        mdvx->setReadTime(Mdvx::READ_CLOSEST,
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
        mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                          url,
                          (int) (60 * time_allowance),
                          _gd.model_run_time,
                          (reqTime.utime() - _gd.model_run_time));
      } else {
        mdvx->setReadTime(Mdvx::READ_FIRST_BEFORE,
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
        mdvx->setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                          url,
                          (int) (60 * time_allowance),
                          _gd.model_run_time,
                          (reqTime.utime() - _gd.model_run_time));
      } else {
        mdvx->setReadTime(Mdvx::READ_FIRST_AFTER,
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
 *
 */

int MdvReader::_getTimeList(const string &url,
                            const DateTime &midTime,
                            int page,
                            Mdvx *mdvx)
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

  mdvx->clearTimeListMode();
  if(_gd.model_run_time != 0 &&
     h_mhdr.data_collection_type == Mdvx::DATA_FORECAST) { 
    mdvx->setTimeListModeLead(url, _gd.model_run_time);
  } else {
    mdvx->setTimeListModeValid(url,
                               listStartTime.utime(), listEndTime.utime());
  }
  if(_gd.debug1) {
    cerr << "Gathering Time List for url: " << url << endl;
  }

  char label[1024];
  snprintf(label, 1024, "Requesting time list for %s data", legend_name.c_str());
  // if(_params.show_data_messages) {
  //   // gui_label_h_frame(label, 1);
  // } else {
  //   // set_busy_state(1);
  // }

  // This is important because different scales can have diffrerent
  // temporal resolutions. - Must pass in the bounding box to get an 
  // accurate time list.

  double min_lat, max_lat, min_lon, max_lon;
  _getBoundingBox(min_lat, max_lat, min_lon, max_lon);
  
  if (!_params.do_not_clip_on_mdv_request) {
    mdvx->setReadHorizLimits(min_lat, min_lon, max_lat, max_lon);
  }
  
  // Set up the DsMdvx request object

  // Gather time list
  // _gd.io_info.mode = DSMDVX_DATA;
  // _gd.io_info.request_type = TIMELIST_REQUEST;    /* List of data times */
  // _gd.io_info.expire_time = time(0) + _params.data_timeout_secs;
  // _gd.io_info.busy_status = 0;
  // _gd.io_info.page = page;
  // _gd.io_info.outstanding_request = 1;
  // _gd.io_info.mr = this;
  
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
    if (mdvx->compileTimeList()) {
      cerr << "ERROR -CIDD:  setTimeListModeValid" << endl;
      cerr << mdvx->getErrStr();
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
  
  if(_params.always_get_full_domain) {

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

void MdvReader::readDoneH() {
  if (h_mdvx->getFieldByNum(0) == nullptr) {
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
      if(strcmp(units_label_sects,"FL") == 0) {
        snprintf(label,2048,"%s: %s %03.0f %s",
                 legend_name.c_str(),
                 units_label_sects,
                 vert[plane].cent,
                 tlabel.c_str());
      }else {
        snprintf(label,2048,"%s: %g %s %s",
                 legend_name.c_str(),
                 vert[plane].cent,
                 units_label_sects,
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
               units_label_sects);
      break;
  }
  
  return label;
  
}

