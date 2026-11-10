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

#include <algorithm>
#include <cmath>
#include <QCoreApplication>
#include <QThread>
#include <QObject>
#include <QDebug>
#include <QMutexLocker>
#include <QTimer>

#include <toolsa/str.h>
#include <toolsa/pjg.h>

namespace {

// The NetCDF/HDF5 libraries used by Radx on this platform are not built for
// concurrent access.  All MdvReader instances share this lock so horizontal
// and vertical workers cannot enter those libraries at the same time.

QMutex mdvLibraryReadMutex;

}

///////////////////////////////////////////////////////////////
// retain the loaded range for later interactive reset

bool LucidColorMap::saveOriginalRange()
{
  _originalRangeSet = false;
  if (_entries.empty()) {
    return false;
  }

  const double rangeMin = _entries.front().minVal;
  const double rangeMax = _entries.back().maxVal;
  if (!std::isfinite(rangeMin) || !std::isfinite(rangeMax) ||
      rangeMax <= rangeMin) {
    return false;
  }

  _originalRangeMin = rangeMin;
  _originalRangeMax = rangeMax;
  _originalRangeSet = true;
  return true;
}

///////////////////////////////////////////////////////////////
// rescale color boundaries without imposing equal intervals

bool LucidColorMap::rescaleRange(double rangeMin, double rangeMax)
{
  if (_entries.empty() || !std::isfinite(rangeMin) ||
      !std::isfinite(rangeMax) || rangeMax <= rangeMin) {
    return false;
  }

  const double oldMin = _entries.front().minVal;
  const double oldMax = _entries.back().maxVal;
  const double oldRange = oldMax - oldMin;
  if (!std::isfinite(oldMin) || !std::isfinite(oldMax) || oldRange <= 0.0) {
    return false;
  }

  // Normally captured immediately after the map is loaded, this fallback
  // also preserves reset behavior if another initialization path is added.
  if (!_originalRangeSet) {
    _originalRangeMin = oldMin;
    _originalRangeMax = oldMax;
    _originalRangeSet = true;
  }

  const double newRange = rangeMax - rangeMin;
  const auto rescaleValue = [oldMin, oldRange, rangeMin, newRange]
                            (double value) {
    return rangeMin + ((value - oldMin) / oldRange) * newRange;
  };

  // Apply one affine transform to every boundary.  This preserves unequal
  // color widths, gaps, and overlaps as fractions of the complete scale.
  for (CmapEntry &entry : _entries) {
    entry.minVal = rescaleValue(entry.minVal);
    entry.maxVal = rescaleValue(entry.maxVal);
  }
  for (CmapLabel &label : _specifiedLabels) {
    label.value = rescaleValue(label.value);
  }

  _rangeMin = rangeMin;
  _rangeMax = rangeMax;
  _range = newRange;
  _useLog10ForLut = false;
  _loadTransitions();
  _computeLut();
  return true;
}

///////////////////////////////////////////////////////////////
// restore the endpoints captured when the map was loaded

bool LucidColorMap::resetRange()
{
  if (!_originalRangeSet) {
    return false;
  }
  return rescaleRange(_originalRangeMin, _originalRangeMax);
}

///////////////////////////////////////////////
// constructor

MdvReader::MdvReader(QObject* parent) :
        QObject(parent),
        _params(Params::Instance()),
        _gd(GlobalData::Instance()),
        _lucid(*parent)
{
  
  // initialize
  
  plane = 0;
  currently_displayed = 0;
  auto_render = 0;
  render_method = 0;
  overlay_line_contours = false;
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
  _compositeModeReq = false;
  _compositeMinHtReq = 0.0;
  _compositeMaxHtReq = 0.0;
  _validH = false;
  _validV = false;
  _newH = false;
  _newV = false;

  _page = 0;
  _timeListValid = false;
  _vLevel = 0;
  _readBusyH = false;
  _vertSampleSpacingKm = 0.0;
  _vertTotalLengthKm = 0.0;
  _vertGeneration = 0;

  qRegisterMetaType<VertReadResultPtr>("VertReadResultPtr");

  // Use a Qt timer for retries so the MdvReader remains responsive between
  // attempts.  The timer belongs to this object and therefore fires on the
  // main Qt thread, where it is safe to start and account for another worker.

  _retryCountH = 0;
  _shuttingDown = false;
  _retryTimerH = new QTimer(this);
  _retryTimerH->setSingleShot(true);
  connect(_retryTimerH, &QTimer::timeout, this, [this]() {
    if (!_shuttingDown && !_readBusyH) {
      startReadVolH();
    }
  });

}

////////////////////////////////////////////////
// destructor

MdvReader::~MdvReader()
{
  _shuttingDown = true;
  _retryTimerH->stop();

  // getHorizPlane() currently operates directly on this MdvReader.  Do not
  // allow the object to be destroyed until every outstanding read has
  // returned.  quit() stops each thread's event loop after the blocking MDV
  // request completes; the request timeout therefore bounds this wait.

  const QSet<QThread *> threads = _readThreadsH;
  for (QThread *thread : threads) {
    disconnect(thread, nullptr, this, nullptr);
    thread->requestInterruption();
    thread->quit();
  }
  for (QThread *thread : threads) {
    thread->wait();
    delete thread;
  }
  _readThreadsH.clear();

  // Vertical workers own their request and result, but their completion
  // signals still target this object.  Wait for them so shutdown cannot leave
  // a live thread behind after Qt has torn down the receiver.

  const QSet<QThread *> vertThreads = _readThreadsV;
  for (QThread *thread : vertThreads) {
    disconnect(thread, nullptr, this, nullptr);
    thread->requestInterruption();
    thread->quit();
  }
  for (QThread *thread : vertThreads) {
    thread->wait();
    delete thread;
  }
  _readThreadsV.clear();
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

  // This is a new user request, so cancel a pending retry and restart the
  // backoff sequence for the new request details.

  _retryTimerH->stop();
  _retryCountH = 0;

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

  // The GUI can change column-max mode while a worker is active.  Capture it
  // with the other request details so an in-flight MDV request remains
  // internally consistent; a queued GUI change will start a subsequent read.
  _compositeModeReq = (composite_mode != FALSE);
  _compositeMinHtReq = _gd.h_win.min_ht;
  _compositeMaxHtReq = _gd.h_win.max_ht;
  
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
  QMutexLocker libraryReadLock(&mdvLibraryReadMutex);
  
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
  
  if (_compositeModeReq) {
    // A composite must include the configured vertical domain.  Restricting
    // this request to the selected plan-view level leaves MDV no column over
    // which to compute the maximum.
    h_mdvx.setReadVlevelLimits(_compositeMinHtReq, _compositeMaxHtReq);
    h_mdvx.setReadComposite();
  } else {
    h_mdvx.setReadVlevelLimits(_vLevelReq, _vLevelReq);
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
  
  if(_gd.debug1) {
    fprintf(stderr, "Get MDVX Horiz Plane - page : %d  -  %s\n", _page, fullUrl.c_str());
  }

  //////////////////////////////////////////
  // read the volume
  
  int iret = h_mdvx.readVolume();
  if (iret) {
    cerr << "ERROR - MdvReader::getHorizPlane, iret: " << ", " << iret << endl;
    cerr << "MdvReader::getHorizPlane returned, iret: " << ", " << iret << endl;
    cerr << h_mdvx.getErrStr() << endl;
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
    
    return -1;
    
  }

  ////////////////////////
  // success - data is in

  h_data_valid = 1;
  _timeList = _pendingTimeList;
  
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
    
  // Compute the vertical extent represented by each level.  Neighboring
  // levels define the bounds for a multi-level field, but a single-level field
  // has no neighbor from which to infer its thickness.  In that case use a
  // valid grid spacing if one was supplied; otherwise retain a zero-thickness
  // interval at the level center rather than inventing an extent.

  if (ds_fhdr.nz <= 0 || h_fhdr.nz <= 0) {
    cerr << "ERROR - MdvReader::_handleHorizReadDone" << endl;
    cerr << "  MDV field has no vertical levels" << endl;
    vert.clear();
    return -1;
  }

  plane = 0;
  vert.resize(ds_fhdr.nz);

  if (ds_fhdr.nz == 1) {
    const double center = ds_vhdr.level[0];
    double halfWidth = 0.0;
    if (std::isfinite(ds_fhdr.grid_dz) && ds_fhdr.grid_dz > 0.0) {
      halfWidth = ds_fhdr.grid_dz / 2.0;
    }
    vert[0].min = center - halfWidth;
    vert[0].cent = center;
    vert[0].max = center + halfWidth;
  } else {
    for (int ii = 0; ii < ds_fhdr.nz; ii++) {
      if (ii == 0) { // Lowest plane
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
  }

  // Select the returned plane by proximity.  MDV level values may have passed
  // through different floating-point representations, so exact equality can
  // incorrectly leave the selection at plane zero.

  const double returnedLevel = h_vhdr.level[0];
  double minLevelDifference = fabs(ds_vhdr.level[0] - returnedLevel);
  for (int ii = 1; ii < ds_fhdr.nz; ii++) {
    const double levelDifference = fabs(ds_vhdr.level[ii] - returnedLevel);
    if (levelDifference < minLevelDifference) {
      minLevelDifference = levelDifference;
      plane = ii;
    }
  }
    
  // Record the dimensional Units of the volume
  
  units_label_cols = h_mdvx.projType2XUnits(h_fhdr.proj_type);
  units_label_rows = h_mdvx.projType2YUnits(h_fhdr.proj_type);
  units_label_sects = h_mdvx.vertTypeZUnits(h_vhdr.type[0]);
    
  // Record the time

  h_date.set(h_mhdr.time_centroid);
  
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
                                  int page,
                                  const WayPts &wayPts)
{
  if (wayPts.getNPoints() < 2) {
    cerr << "ERROR - MdvReader::requestVertSection" << endl;
    cerr << "  At least two waypoints are required" << endl;
    return -1;
  }

  DateTime reqTime;
  _computeReqTime(midTime, reqTime);

  VertReadRequest request;
  request.requestTime = reqTime;
  request.url = _getFullUrl();
  request.fieldName = _getFieldName();
  request.wayPts = wayPts;
  request.page = page;
  request.generation = ++_vertGeneration;
  request.gatherMode = _params.gather_data_mode;
  request.searchMarginSecs = (int) (60 * time_allowance);
  request.modelRunTime = _gd.model_run_time;
  request.isForecast =
    (_gd.model_run_time != 0 &&
     h_mhdr.data_collection_type == Mdvx::DATA_FORECAST);
  request.requestCompressedData = _params.request_compressed_data;
  request.requestGzipCompression = _params.request_gzip_vol_compression;

  _setValidV(false);
  _setNewV(false);
  v_data_valid = 0;

  if (_gd.debug1) {
    cerr << "Get MDVX vertical section" << endl;
    cerr << "  page: " << page << endl;
    cerr << "  URL: " << request.url << endl;
    cerr << "  field: " << request.fieldName << endl;
    cerr << "  waypoints: " << wayPts.getNPoints() << endl;
  }

  _startReadVolV(request);
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
      _timeListValid = false;
      // Availability is supplementary display information. Preserve the
      // previous list and continue the primary grid read if this query fails.
      return 0;
    }
    _pendingTimeList = mdvx.getTimeList();
    _timeListValid = true;
  } else {
    // _gd.io_info.outstanding_request = 0;
    _pendingTimeList.clear();
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
  cerr << "ReadVolH thread started. Parent object:" << _mr << endl;
  int iret = _mr->getHorizPlane();
  cerr << "ReadVolH returned, iret: " << iret << endl;
  emit readDone(iret);
}

//////////////////////////////
// start H vol read in thread

void MdvReader::startReadVolH() {
  if (_shuttingDown) {
    return;
  }
  if (_readBusyH) {
    cerr << "readVolH() is already in progress. Ignoring new request." << endl;
    return;
  }
  _readBusyH = true;

  // Each request gets its own worker and thread.  The worker performs the
  // blocking MDV call, while all interpretation of the result remains on the
  // MdvReader's Qt thread.  Retain the QThread until it finishes so shutdown
  // can wait for any worker that still refers to this MdvReader.

  ReadVolH* worker = new ReadVolH(this); // Pass the current object as reference
  QThread* thread = new QThread;
  _readThreadsH.insert(thread);
  worker->moveToThread(thread);

  // Stop the worker event loop as soon as the blocking read returns.  The
  // direct connection is intentional: it does not depend on the MdvReader's
  // event loop processing the completion signal first.  Object deletion and
  // removal from the tracked set occur after the thread has actually stopped.

  connect(thread, &QThread::started, worker, &ReadVolH::doRead);
  connect(worker, &ReadVolH::readDone, this, &MdvReader::readDoneH);
  connect(worker, &ReadVolH::readDone, thread, &QThread::quit,
          Qt::DirectConnection);
  connect(thread, &QThread::finished, worker, &ReadVolH::deleteLater);
  connect(thread, &QThread::finished, this, [this, thread]() {
    _readThreadsH.remove(thread);
    thread->deleteLater();
  });
  thread->start();
}

////////////////////////
// done with H vol read

void MdvReader::readDoneH(int readStatus)
{
  // A successful worker read has produced raw MDV data that still needs to be
  // installed in the display state on this Qt thread.  If either phase fails,
  // leave the data invalid and retry the same stored request.  A subsequent
  // user request will replace those details and restart the backoff sequence.

  int iret = readStatus;
  if (iret == 0) {
    iret = _handleHorizReadDone();
  }
  if (iret || !h_data_valid) {
    _setValidH(false);
    _setNewH(false);
    _readBusyH = false;
    _scheduleRetryH();
    return;
  }
  _setValidH(iret_h_mdvx_read == 0);
  _setNewH(true);
  cerr << "readDone in ReadVolH worker thread" << endl;
  _readBusyH = false;
  _retryCountH = 0;
}

////////////////////////////////////////////////////////
// schedule a retry of the most recent horizontal request

void MdvReader::_scheduleRetryH()
{
  if (_shuttingDown) {
    return;
  }

  // Retry indefinitely, but cap the delay so a transient outage recovers
  // without requiring another GUI state change and a long outage does not
  // cause a tight request loop.

  static const int retryDelaysMsecs[] = {
    1000, 2000, 5000, 10000, 30000
  };
  static const int nRetryDelays =
    sizeof(retryDelaysMsecs) / sizeof(retryDelaysMsecs[0]);
  const int delayIndex = std::min(_retryCountH, nRetryDelays - 1);
  const int delayMsecs = retryDelaysMsecs[delayIndex];
  _retryCountH++;

  if (_gd.debug) {
    cerr << "MdvReader: horizontal read failed for " << field_label
         << "; retrying in " << delayMsecs << " ms" << endl;
  }
  _retryTimerH->start(delayMsecs);
}

///////////////////////////////////////////////
// Worker for a vertical-section read

ReadVolV::ReadVolV(const VertReadRequest &request, QObject *parent) :
        QObject(parent),
        _request(request)
{
}

void ReadVolV::doRead()
{
  QMutexLocker libraryReadLock(&mdvLibraryReadMutex);

  VertReadResultPtr result = std::make_shared<VertReadResult>();
  result->status = -1;
  result->page = _request.page;
  result->generation = _request.generation;
  result->sampleSpacingKm = 0.0;
  result->totalLengthKm = 0.0;
  MEM_zero(result->mhdr);
  MEM_zero(result->fhdr);
  MEM_zero(result->vhdr);

  DsURL url(_request.url);
  if (!url.isValid()) {
    result->error = "Invalid MDV URL: " + _request.url;
    emit readDone(result);
    return;
  }

  DsMdvx mdvx;
  mdvx.clearRead();
  mdvx.addReadField(_request.fieldName);

  // Match the horizontal gather policy using only values captured when the
  // request was made.  The worker must not consult mutable movie or field
  // state while a blocking server request is in progress.

  switch (_request.gatherMode) {
    case Params::FIRST_BEFORE_END_OF_FRAME:
      if (_request.isForecast) {
        mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                         _request.url,
                         _request.searchMarginSecs,
                         _request.modelRunTime,
                         _request.requestTime.utime() -
                         _request.modelRunTime);
      } else {
        mdvx.setReadTime(Mdvx::READ_FIRST_BEFORE,
                         _request.url,
                         _request.searchMarginSecs,
                         _request.requestTime.utime() + 1, 0);
      }
      break;

    case Params::FIRST_AFTER_START_OF_FRAME:
      if (_request.isForecast) {
        mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                         _request.url,
                         _request.searchMarginSecs,
                         _request.modelRunTime,
                         _request.requestTime.utime() -
                         _request.modelRunTime);
      } else {
        mdvx.setReadTime(Mdvx::READ_FIRST_AFTER,
                         _request.url,
                         _request.searchMarginSecs,
                         _request.requestTime.utime() - 1, 0);
      }
      break;

    case Params::CLOSEST_TO_FRAME_CENTER:
    default:
      if (_request.isForecast) {
        mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
                         _request.url,
                         _request.searchMarginSecs,
                         _request.modelRunTime,
                         _request.requestTime.utime() -
                         _request.modelRunTime);
      } else {
        mdvx.setReadTime(Mdvx::READ_CLOSEST,
                         _request.url,
                         _request.searchMarginSecs,
                         _request.requestTime.utime());
      }
      break;
  }

  mdvx.setReadEncodingType(Mdvx::ENCODING_ASIS);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_ASIS);
  if (_request.requestCompressedData) {
    mdvx.setReadCompressionType(_request.requestGzipCompression ?
                                Mdvx::COMPRESSION_GZIP_VOL :
                                Mdvx::COMPRESSION_ZLIB);
  }

  for (size_t ii = 0; ii < _request.wayPts.getNPoints(); ++ii) {
    double lat = 0.0;
    double lon = 0.0;
    _request.wayPts.getPoint(ii, lat, lon);
    mdvx.addReadWayPt(lat, lon);
  }

  if (mdvx.readVsection()) {
    result->error = mdvx.getErrStr();
    emit readDone(result);
    return;
  }

  MdvxField *field = mdvx.getFieldByNum(0);
  if (field == nullptr) {
    result->error = "Vertical-section response did not contain the requested field";
    emit readDone(result);
    return;
  }
  if (field->convertType(Mdvx::ENCODING_FLOAT32,
                         Mdvx::COMPRESSION_NONE)) {
    result->error = field->getErrStr();
    emit readDone(result);
    return;
  }

  result->mhdr = mdvx.getMasterHeader();
  result->fhdr = field->getFieldHeader();
  result->vhdr = field->getVlevelHeader();
  const size_t nPoints = (size_t) result->fhdr.nx *
                         (size_t) result->fhdr.ny *
                         (size_t) result->fhdr.nz;
  result->data.resize(nPoints);
  memcpy(result->data.data(), field->getVol(), nPoints * sizeof(fl32));

  const vector<Mdvx::vsect_waypt_t> &returned = mdvx.getVsectWayPts();
  for (const Mdvx::vsect_waypt_t &point : returned) {
    result->returnedWayPts.addPoint(point.lat, point.lon);
  }
  if (result->returnedWayPts.getNPoints() < 2) {
    result->returnedWayPts = _request.wayPts;
  }
  result->segments = mdvx.getVsectSegments();
  result->sampleSpacingKm = mdvx.getVsectDxKm();
  result->totalLengthKm = mdvx.getvsectTotalLength();
  if (result->totalLengthKm <= 0.0) {
    for (const Mdvx::vsect_segment_t &segment : result->segments) {
      result->totalLengthKm += segment.length;
    }
  }
  if (result->totalLengthKm <= 0.0) {
    for (size_t ii = 1; ii < result->returnedWayPts.getNPoints(); ++ii) {
      double lat0 = 0.0;
      double lon0 = 0.0;
      double lat1 = 0.0;
      double lon1 = 0.0;
      double segmentLengthKm = 0.0;
      double azimuthDeg = 0.0;
      result->returnedWayPts.getPoint(ii - 1, lat0, lon0);
      result->returnedWayPts.getPoint(ii, lat1, lon1);
      PJGLatLon2RTheta(lat0, lon0, lat1, lon1,
                      &segmentLengthKm, &azimuthDeg);
      result->totalLengthKm += segmentLengthKm;
    }
  }
  result->status = 0;
  emit readDone(result);
}

////////////////////////////////////////////////////////
// start a self-contained vertical-section worker

void MdvReader::_startReadVolV(const VertReadRequest &request)
{
  if (_shuttingDown) {
    return;
  }

  ReadVolV *worker = new ReadVolV(request);
  QThread *thread = new QThread;
  _readThreadsV.insert(thread);
  worker->moveToThread(thread);

  connect(thread, &QThread::started, worker, &ReadVolV::doRead);
  connect(worker, &ReadVolV::readDone, this, &MdvReader::readDoneV);
  connect(worker, &ReadVolV::readDone, thread, &QThread::quit,
          Qt::DirectConnection);
  connect(thread, &QThread::finished, worker, &ReadVolV::deleteLater);
  connect(thread, &QThread::finished, this, [this, thread]() {
    _readThreadsV.remove(thread);
    thread->deleteLater();
  });
  thread->start();
}

////////////////////////////////////////////////////////
// install the newest vertical-section result on the GUI thread

void MdvReader::readDoneV(VertReadResultPtr result)
{
  if (!result || result->generation != _vertGeneration) {
    return;
  }

  if (result->status != 0) {
    cerr << "ERROR - MdvReader vertical-section read failed for "
         << field_label << endl;
    if (!result->error.empty()) {
      cerr << result->error << endl;
    }
    v_data_valid = 0;
    _setValidV(false);
    _setNewV(true);
    emit verticalReadComplete(this);
    return;
  }

  // Replace the complete section atomically from the GUI thread.  VertView
  // will never observe headers from one request with data from another.

  v_mhdr = result->mhdr;
  v_fhdr = result->fhdr;
  v_vhdr = result->vhdr;
  v_fl32_data = std::move(result->data);
  _vertWayPts = result->returnedWayPts;
  _vertSegments = std::move(result->segments);
  _vertSampleSpacingKm = result->sampleSpacingKm;
  _vertTotalLengthKm = result->totalLengthKm;
  v_date.set(v_mhdr.time_centroid);
  vunits_label_cols = "km";
  vunits_label_sects = Mdvx::vertTypeZUnits(v_fhdr.vlevel_type);
  v_data_valid = 1;
  _setValidV(true);
  _setNewV(true);
  emit verticalReadComplete(this);
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
string MdvReader::vlevelLabel() const
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
 * FIELD_LABEL: Return a Label string for a field in horiz window
 */

string MdvReader::fieldLabelHoriz() const
{

  
  // Convert time to a string
  
  string tlabel =
    h_date.strfTime(_params.label_time_format, !_params.use_local_timestamps);

  char vertLabel[256];
  vertLabel[0] = '\0';
  if (plane < (int) vert.size()) {
    if(units_label_sects == "FL") {
      snprintf(vertLabel, 256,"%03.0f",
               vert[plane].cent);
    } else {
      snprintf(vertLabel, 256 , "%g",
               vert[plane].cent);
    }
  }
  
  char label[2048];
  label[0] = '\0';
  
  if(composite_mode == FALSE) {
    // Check for surface fields 
    if((plane > (int) vert.size() - 1) ||
       ((vert[plane].cent == 0.0) && (h_fhdr.grid_dz == 0)) ||
       h_fhdr.vlevel_type == Mdvx::VERT_TYPE_SURFACE ||
       h_fhdr.vlevel_type == Mdvx::VERT_TYPE_COMPOSITE ||
       ds_fhdr.nz == 1) {  
      snprintf(label,2048,"%s: %s",
               legend_name.c_str(),
               tlabel.c_str());
    } else {
      // Reverse order of label and value if units are "FL" 
      if(units_label_sects == "FL") {
        snprintf(label, 2048, "%s: %s %s %s",
                 legend_name.c_str(),
                 units_label_sects.c_str(),
                 vertLabel,
                 tlabel.c_str());
      } else {
        snprintf(label,2048,"%s: %s %s %s",
                 legend_name.c_str(),
                 vertLabel,
                 units_label_sects.c_str(),
                 tlabel.c_str());
      }
    }
  } else {
    snprintf(label,2048,"%s: All levels %s",
             legend_name.c_str(),
             tlabel.c_str());
  }
  
  return label;
  
}

/**********************************************************************
 * FIELD_LABEL: Return a Label string for a field in vert window
 */

string MdvReader::fieldLabelVert() const
{

  
  // Convert time to a string
  
  string tlabel =
    v_date.strfTime(_params.label_time_format, !_params.use_local_timestamps);
  
  char label[2048];
  label[0] = '\0';
  
  snprintf(label,2048,"%s - %s (%s) %s",
           _params.vsection_title_prefix,
           legend_name.c_str(),
           v_fhdr.units,
           tlabel.c_str());

  return label;
  
}

/*************************************************************************
 * HEIGHT_LABEL: Return the height label
 */
string MdvReader::heightLabel() const
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

