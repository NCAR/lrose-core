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
//////////////////////////////////////////////////////////
// ProductMgr.cc
//
// Product rendering
//
// Mike Dixon, from Nancy Rehak
// EOL, NCAR, Boulder, CO, 80307, USA
//
// Jan 2000
//
//////////////////////////////////////////////////////////

#include "ProductMgr.hh"
#include "SymprodRender.hh"
#include "SymprodRenderObj.hh"
#include "SymprodRenderContext.hh"
#include "WorldPlot.hh"
#include <Spdb/DsSpdb.hh>
#include <didss/DsURL.hh>
#include <toolsa/DateTime.hh>
#include <QObject>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>

namespace {

class SymprodReadThread : public QThread
{
public:
  explicit SymprodReadThread(const SymprodReadRequest &request,
                             QObject *parent) :
          QThread(parent), _request(request), _result(nullptr)
  {
  }

  SymprodReadResult *takeResult()
  {
    SymprodReadResult *result = _result;
    _result = nullptr;
    return result;
  }

protected:
  void run() override
  {
    std::unique_ptr<SymprodReadResult> result(new SymprodReadResult);
    result->productIndex = _request.productIndex;
    result->generation = _request.generation;
    result->label = _request.label;
    result->key = _request.key;
    result->timeListStart = _request.timeListStart;
    result->timeListEnd = _request.timeListEnd;

    if (_request.requestTimeList) {
      // Translator and parameter-file options apply to graphic generation,
      // not to the underlying SPDB reference list queried for availability.
      DsURL timesUrl(_request.url);
      timesUrl.setTranslator("");
      timesUrl.setParamFile("");
      DsSpdb timeListSpdb;
      result->timeListStatus = timeListSpdb.compileTimeList(
        timesUrl.getURLStr(), _request.timeListStart, _request.timeListEnd);
      if (result->timeListStatus == 0) {
        result->availableTimes = timeListSpdb.getTimeList();
      } else {
        result->timeListError = timeListSpdb.getErrStr();
      }
    }

    DsSpdb spdb;
    spdb.setUniqueOff();
    if (_request.gzip) {
      spdb.setDataCompressForTransfer(Spdb::COMPRESSION_GZIP);
    }
    if (_request.key.useHorizLimits) {
      spdb.setHorizLimits(_request.key.minLat, _request.key.minLon,
                          _request.key.maxLat, _request.key.maxLon);
    } else {
      spdb.clearHorizLimits();
    }
    if (_request.key.useVertLimits) {
      spdb.setVertLimits(_request.key.minAlt, _request.key.maxAlt);
    } else {
      spdb.clearVertLimits();
    }

    int status = -1;
    if (_request.renderType == Params::SYMPROD_RENDER_GET_VALID) {
      if (_request.debug) {
        cerr << "SYMPROD getValid(), url, time, dataType: "
             << _request.url << ", "
             << DateTime::strm(_request.key.dataTime) << ", "
             << _request.dataType << endl;
      }
      status = spdb.getValid(_request.url, _request.key.dataTime,
                             _request.dataType);
    } else if (_request.shortRequests &&
               _request.renderType ==
                 Params::SYMPROD_RENDER_FIRST_BEFORE_FRAME_TIME) {
      const int margin = (int) (_request.frameStart -
                                _request.key.startTime);
      if (_request.debug) {
        cerr << "SYMPROD getFirstBefore(), url, time, margin, dataType: "
             << _request.url << ", "
             << DateTime::strm(_request.frameEnd) << ", "
             << margin << ", "
             << _request.dataType << endl;
      }
      status = spdb.getFirstBefore(_request.url, _request.frameEnd,
                                   margin, _request.dataType);
    } else if (_request.shortRequests &&
               _request.renderType ==
                 Params::SYMPROD_RENDER_GET_VALID_AT_FRAME_TIME) {
      if (_request.debug) {
        cerr << "SYMPROD getValid(), url, time, dataType: "
             << _request.url << ", "
             << DateTime::strm(_request.key.endTime) << ", "
             << _request.dataType << endl;
      }
      status = spdb.getValid(_request.url, _request.key.endTime,
                             _request.dataType);
    } else {
      if (_request.shortRequests &&
          _request.renderType == Params::SYMPROD_RENDER_LATEST_IN_FRAME) {
        spdb.setUniqueLatest();
      }
      if (_request.debug) {
        cerr << "SYMPROD getInterval(), url, startTime, endTime, dataType: "
             << _request.url << ", "
             << DateTime::strm(_request.key.startTime) << ", "
             << DateTime::strm(_request.key.endTime) << ", "
             << _request.dataType << endl;
      }
      status = spdb.getInterval(_request.url,
                                _request.key.startTime,
                                _request.key.endTime,
                                _request.dataType);
    }

    result->status = status;
    if (status != 0) {
      result->error = spdb.getErrStr();
    } else {
      const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
      result->chunks.reserve(chunks.size());
      for (size_t ii = 0; ii < chunks.size(); ++ii) {
        if (chunks[ii].data != nullptr && chunks[ii].len > 0) {
          result->chunks.push_back(QByteArray(
            static_cast<const char *>(chunks[ii].data), chunks[ii].len));
        }
      }
    }
    _result = result.release();
  }

private:
  SymprodReadRequest _request;
  SymprodReadResult *_result;
};

string addTunnelOptions(const string &url, const Params &params)
{
  if (strlen(params.http_tunnel_url) <= Constants::URL_MIN_SIZE) {
    return url;
  }
  string result = url;
  result += (url.find('?') == string::npos) ? "?" : "&";
  result += "tunnel_url=";
  result += params.http_tunnel_url;
  if (strlen(params.http_proxy_url) > Constants::URL_MIN_SIZE) {
    result += "&proxy_url=";
    result += params.http_proxy_url;
  }
  return result;
}

} // namespace

bool SymprodRequestKey::operator==(const SymprodRequestKey &rhs) const
{
  return startTime == rhs.startTime && endTime == rhs.endTime &&
         dataTime == rhs.dataTime &&
         useHorizLimits == rhs.useHorizLimits &&
         (!useHorizLimits ||
          (minLat == rhs.minLat && minLon == rhs.minLon &&
           maxLat == rhs.maxLat && maxLon == rhs.maxLon)) &&
         useVertLimits == rhs.useVertLimits &&
         (!useVertLimits ||
          (minAlt == rhs.minAlt && maxAlt == rhs.maxAlt));
}

//////////////////////////////////////////
// default constructor

ProductMgr::ProductMgr() :
        _params(Params::Instance()),
        _gd(GlobalData::Instance())
          
{
  _active_Rob = nullptr;
  _completionContext = make_unique<QObject>();
  _shuttingDown = false;
  _lastDisplayStateValid = false;
  _lastFrameStart = 0;
  _lastFrameEnd = 0;
  _lastDataTime = 0;
  _lastSelectedLevel = 0.0;
  _lastMinLat = _lastMinLon = 0.0;
  _lastMaxLat = _lastMaxLon = 0.0;

  for (int i = 0; i < _params.symprod_prod_info_n; i++) {
    
    unique_ptr<Product> product =
      make_unique<Product>(_params._symprod_prod_info[i]);
    _products.push_back(std::move(product));
    
  }

  _generations.resize(_products.size(), 0);
  _requestInFlight.resize(_products.size(), false);
  _installedKeyValid.resize(_products.size(), false);
  _timeListValid.resize(_products.size(), false);
  _timeListStarts.resize(_products.size(), 0);
  _timeListEnds.resize(_products.size(), 0);
  _retryNotBefore.resize(_products.size(), 0);
  _desiredKeys.resize(_products.size());
  _installedKeys.resize(_products.size());

}

//////////////
// destructor
  
ProductMgr::~ProductMgr()
{
  _shuttingDown = true;
  _productsChangedCallback = std::function<void()>();

  // A worker owns all SPDB request state, but its completion handler targets
  // this manager. Wait for blocking calls to return before releasing the
  // QObject owner, which also cancels queued completion lambdas.
  const QSet<QThread *> threads = _readThreads;
  for (QThread *thread : threads) {
    QObject::disconnect(thread, nullptr, nullptr, nullptr);
    thread->requestInterruption();
    thread->quit();
  }
  for (QThread *thread : threads) {
    thread->wait();
    SymprodReadThread *readThread = static_cast<SymprodReadThread *>(thread);
    delete readThread->takeResult();
  }
  _readThreads.clear();
  _completionContext.reset();

  _products.clear();
}

///////////////////////////////////////////////////////////////
// request products for the current display state

void ProductMgr::requestData(time_t frameStart,
                             time_t frameEnd,
                             time_t dataTime,
                             double selectedLevel,
                             const WorldPlot &plot)
{
  _lastFrameStart = frameStart;
  _lastFrameEnd = frameEnd;
  _lastDataTime = dataTime;
  _lastSelectedLevel = selectedLevel;
  _computeLatLonBounds(plot, _lastMinLat, _lastMinLon,
                       _lastMaxLat, _lastMaxLon);
  _lastDisplayStateValid = true;
  _requestCapturedState();
}

void ProductMgr::refreshActiveProducts()
{
  if (_lastDisplayStateValid) {
    _requestCapturedState();
  }
}

void ProductMgr::_requestCapturedState()
{
  if (_shuttingDown || !_gd.prod.products_on) {
    return;
  }

  for (size_t ii = 0; ii < _products.size(); ++ii) {
    Product &product = *_products[ii];
    if (!product._active) {
      continue;
    }

    const Params::symprod_prod_info_t &info = product._prodInfo;
    const time_t baseStart = _params.symprod_short_requests ?
      _lastFrameStart : _gd.epoch_start;
    const time_t baseEnd = _params.symprod_short_requests ?
      _lastFrameEnd : _gd.epoch_end;

    SymprodRequestKey key;
    key.startTime = baseStart - (time_t) (info.minutes_allow_before * 60.0);
    key.endTime = baseEnd + (time_t) (info.minutes_allow_after * 60.0);
    // Data time affects the server query only for GET_VALID. Other temporal
    // modes select from the cached interval during rendering.
    key.dataTime = info.render_type == Params::SYMPROD_RENDER_GET_VALID ?
      _lastDataTime : 0;
    key.useHorizLimits = _params.clip_to_current_zoom_on_symprod_request ||
                         info.request_data_on_zoom;
    key.minLat = _lastMinLat;
    key.minLon = _lastMinLon;
    key.maxLat = _lastMaxLat;
    key.maxLon = _lastMaxLon;
    key.useVertLimits = info.request_data_on_vert_change;
    key.minAlt = _lastSelectedLevel;
    key.maxAlt = _lastSelectedLevel;

    const bool sameDesiredKey = key == _desiredKeys[ii];
    if (!sameDesiredKey) {
      _retryNotBefore[ii] = 0;
    } else if (time(nullptr) < _retryNotBefore[ii]) {
      continue;
    }

    if (_requestInFlight[ii] && sameDesiredKey) {
      continue;
    }
    if (product._data_valid && _installedKeyValid[ii] &&
        key == _installedKeys[ii]) {
      continue;
    }

    SymprodReadRequest request;
    request.productIndex = (int) ii;
    request.generation = ++_generations[ii];
    request.label = info.menu_label;
    request.url = addTunnelOptions(info.url, _params);
    request.dataType = info.data_type;
    request.renderType = info.render_type;
    request.debug = _params.symprod_debug != Params::SYMPROD_DEBUG_OFF;
    request.shortRequests = _params.symprod_short_requests;
    request.gzip = _params.symprod_gzip_requests;
    request.frameStart = _lastFrameStart;
    request.frameEnd = _lastFrameEnd;
    request.timeListStart = _gd.epoch_start;
    request.timeListEnd = _gd.epoch_end;
    request.requestTimeList = !_timeListValid[ii] ||
      _timeListStarts[ii] != request.timeListStart ||
      _timeListEnds[ii] != request.timeListEnd;
    request.key = key;

    _desiredKeys[ii] = key;
    _requestInFlight[ii] = true;
    _startRequest(request);
  }
}

void ProductMgr::_startRequest(const SymprodReadRequest &request)
{
  SymprodReadThread *thread =
    new SymprodReadThread(request, _completionContext.get());
  _readThreads.insert(thread);

  if (_statusCallback) {
    _statusCallback("Requesting " + request.label, false);
  }

  // A manager-owned QObject supplies the GUI-thread connection context. Its
  // destruction cancels queued lambdas before ProductMgr itself is released.
  QObject::connect(thread, &QThread::finished, _completionContext.get(),
                   [this, thread]() {
    _readThreads.remove(thread);
    std::unique_ptr<SymprodReadResult> result(thread->takeResult());
    _handleResult(result.release());
    thread->deleteLater();
  });
  thread->start();
}

void ProductMgr::_handleResult(SymprodReadResult *rawResult)
{
  std::unique_ptr<SymprodReadResult> result(rawResult);
  if (_shuttingDown || !result || result->productIndex < 0 ||
      result->productIndex >= (int) _products.size()) {
    return;
  }

  const int index = result->productIndex;
  const bool stale = result->generation != _generations[index];
  if (_params.symprod_debug != Params::SYMPROD_DEBUG_OFF) {
    size_t totalBytes = 0;
    for (const QByteArray &chunk : result->chunks) {
      totalBytes += static_cast<size_t>(chunk.size());
    }
    // Keep request diagnostics at the snapshot boundary: one line shows
    // whether the server returned usable bytes and whether Lucid accepted it.
    cerr << "SYMPROD result: label=" << result->label
         << " generation=" << result->generation
         << " requested=" << DateTime::str(result->key.startTime)
         << " to " << DateTime::str(result->key.endTime)
         << " status=" << result->status
         << " chunks=" << result->chunks.size()
         << " bytes=" << totalBytes
         << " stale=" << (stale ? "yes" : "no") << endl;
  }
  if (stale) {
    return;
  }
  _requestInFlight[index] = false;

  Product &product = *_products[index];
  if (result->timeListStatus == 0) {
    product.setAvailableTimes(result->availableTimes);
    _timeListValid[index] = true;
    _timeListStarts[index] = result->timeListStart;
    _timeListEnds[index] = result->timeListEnd;
  } else if (!result->timeListError.empty()) {
    // Availability is supplementary: preserve the previous ticks and allow
    // a successful graphics request to be installed normally.
    cerr << "WARNING - ProductMgr: cannot retrieve availability times for "
         << product._prodInfo.menu_label << endl;
    cerr << result->timeListError << endl;
  }
  if (result->status != 0) {
    cerr << "ERROR - ProductMgr: cannot retrieve "
         << product._prodInfo.menu_label << endl;
    cerr << "  URL: " << product._prodInfo.url << endl;
    cerr << result->error << endl;
    // The main GUI timer is intentionally very frequent. Bound retries here
    // so an unavailable SPDB server cannot create a tight request loop.
    _retryNotBefore[index] = time(nullptr) + 5;
    if (_statusCallback) {
      _statusCallback("Cannot retrieve " +
                      string(product._prodInfo.menu_label) + ": " +
                      result->error, true);
    }
    if (_productsChangedCallback) {
      _productsChangedCallback();
    }
    return;
  }

  string decodeError;
  if (!product.installChunks(result->chunks, decodeError)) {
    cerr << "ERROR - ProductMgr: cannot decode "
         << product._prodInfo.menu_label << endl;
    cerr << decodeError << endl;
    _retryNotBefore[index] = time(nullptr) + 5;
    if (_statusCallback) {
      _statusCallback("Cannot decode " +
                      string(product._prodInfo.menu_label) + ": " +
                      decodeError, true);
    }
    if (_productsChangedCallback) {
      _productsChangedCallback();
    }
    return;
  }

  // An object selected from the old snapshot cannot survive its atomic swap.
  _active_Rob = nullptr;
  _installedKeys[index] = _desiredKeys[index];
  _installedKeyValid[index] = true;
  _retryNotBefore[index] = 0;
  if (_productsChangedCallback) {
    _productsChangedCallback();
  }
  if (_statusCallback) {
    _statusCallback(string(product._prodInfo.menu_label) + " ready", false);
  }
}

void ProductMgr::_computeLatLonBounds(const WorldPlot &plot,
                                      double &minLat, double &minLon,
                                      double &maxLat, double &maxLon) const
{
  const double xmin = plot.getXMinWorld();
  const double xmax = plot.getXMaxWorld();
  const double ymin = plot.getYMinWorld();
  const double ymax = plot.getYMaxWorld();
  const double xmid = (xmin + xmax) / 2.0;
  const double ymid = (ymin + ymax) / 2.0;
  const double points[8][2] = {
    {xmin, ymin}, {xmid, ymin}, {xmax, ymin}, {xmax, ymid},
    {xmax, ymax}, {xmid, ymax}, {xmin, ymax}, {xmin, ymid}
  };

  double centerLat = 0.0;
  double centerLon = 0.0;
  plot.getProjection().xy2latlon(xmid, ymid, centerLat, centerLon);
  minLat = 90.0;
  maxLat = -90.0;
  minLon = centerLon + 180.0;
  maxLon = centerLon - 180.0;
  for (int ii = 0; ii < 8; ++ii) {
    double lat = 0.0;
    double lon = 0.0;
    plot.getProjection().xy2latlon(points[ii][0], points[ii][1], lat, lon);
    while (lon - centerLon > 180.0) lon -= 360.0;
    while (lon - centerLon < -180.0) lon += 360.0;
    if (std::isfinite(lat) && std::isfinite(lon)) {
      minLat = std::min(minLat, lat);
      maxLat = std::max(maxLat, lat);
      minLon = std::min(minLon, lon);
      maxLon = std::max(maxLon, lon);
    }
  }
}


void ProductMgr::drawQt(QPainter &painter,
                        const SymprodRenderContext &context)
{
  if (!_gd.prod.products_on) {
    return;
  }
  for (size_t ii = 0; ii < _products.size(); ++ii) {
    _products[ii]->drawQt(painter, context);
  }
}

bool ProductMgr::pickClosestObject(
  double lat, double lon, const SymprodRenderContext &context,
  double maxDistanceKm)
{
  _active_Rob = nullptr;
  double closestDistance = maxDistanceKm;
  for (const unique_ptr<Product> &product : _products) {
    const double distance = product->pickClosestObject(lat, lon, context);
    if (distance <= closestDistance) {
      closestDistance = distance;
      _active_Rob = product->getClosestObject();
    }
  }
  return _active_Rob != nullptr;
}

void ProductMgr::drawPickedQt(
  QPainter &painter, const SymprodRenderContext &context) const
{
  if (!_active_Rob) {
    return;
  }

  // Hidden objects are alternate details within the selected Symprod. Draw
  // that complete product live so its normal and hidden pieces stay aligned.
  SymprodRenderContext pickedContext(context);
  pickedContext.showHidden = true;
  pickedContext.showingHidden = false;
  _active_Rob->container->drawQt(painter, pickedContext);
}

void ProductMgr::fillExportFields(vector<world_pt_t> &wpt,
                                  string &label) const
{
  if (_active_Rob) {
    _active_Rob->container->fill_export_fields(wpt, label);
  }
}

//////////////////////////////////
// RESET_PRODUCT_VALID_FLAGS 
//

void ProductMgr::reset_product_valid_flags()
  
{
  for (size_t i = 0; i < _products.size(); i++) {
    _installedKeyValid[i] = false;
    _timeListValid[i] = false;
    _requestInFlight[i] = false;
    _retryNotBefore[i] = 0;
  }
  
}

//////////////////////////////////
// RESET_PRODUCT_VALID_FLAGS_ZOOM
//

void ProductMgr::reset_product_valid_flags_zoom()
  
{
  for (size_t i = 0; i < _products.size(); i++) {
    if (_products[i]->_prodInfo.request_data_on_zoom) {
      _installedKeyValid[i] = false;
      _requestInFlight[i] = false;
      _retryNotBefore[i] = 0;
    }
  }
  
}

//////////////////////////////////
// RESET_PRODUCT_VALID_FLAGS_VERT
//

void ProductMgr::reset_product_valid_flags_vert()
  
{
  for (size_t i = 0; i < _products.size(); i++) {
    if (_products[i]->_prodInfo.request_data_on_vert_change) {
      _installedKeyValid[i] = false;
      _requestInFlight[i] = false;
      _retryNotBefore[i] = 0;
    }
  }
  
}

///////////////////////////////////////////////////////////////////////
// GET_PRODUCT_ACTIVE
//
int ProductMgr::get_product_active(int product)
{
  return _products[product]->_active;
}

///////////////////////////////////////////////////////////////////////
// SET_PRODUCT_ACTIVE
//
void ProductMgr::set_product_active(int product, int active)
{
  _products[product]->_active = active;
  if (!active) {
    _active_Rob = nullptr;
  }
  if (active) {
    refreshActiveProducts();
  }
  if (_productsChangedCallback) {
    _productsChangedCallback();
  }
}

///////////////////////////////////////////////////////////////////////
// SET_PRODUCT_DATA_VALID
//
void ProductMgr::set_product_data_valid(int product, int valid)
{
  if (valid) {
    _products[product]->_data_valid = 1;
  } else {
    _installedKeyValid[product] = false;
  }
}


///////////////////////////////////////////////////////////////////////
// GET_PRODUCT_DATA_VALID
//
int ProductMgr::get_product_data_valid(int product)
{
  return _installedKeyValid[product];
}

///////////////////////////////////////////////////////////////////////
// GET_NUM_PRODUCTS
//
int ProductMgr::get_num_products()
{
  return (int) _products.size();
}

///////////////////////////////////////////////////////////////////////
// NUM_PRODUCTS_INVALID
//
int ProductMgr::num_products_invalid()
{
  int num = 0;
  for (size_t i = 0; i < _products.size(); i++) {
    if (_products[i]->_active && !_installedKeyValid[i]) num++;
  }

  return num;
}

bool ProductMgr::activeProductsReady() const
{
  for (size_t ii = 0; ii < _products.size(); ++ii) {
    if (_products[ii]->_active &&
        (_requestInFlight[ii] || !_installedKeyValid[ii] ||
         !(_installedKeys[ii] == _desiredKeys[ii]))) {
      return false;
    }
  }
  return true;
}

bool ProductMgr::activeProductRequestsSettled() const
{
  for (size_t ii = 0; ii < _products.size(); ++ii) {
    if (_products[ii]->_active && _requestInFlight[ii]) {
      return false;
    }
  }
  return true;
}

int ProductMgr::numActiveProductsUnavailable() const
{
  int unavailable = 0;
  for (size_t ii = 0; ii < _products.size(); ++ii) {
    if (_products[ii]->_active &&
        (!_installedKeyValid[ii] ||
         !(_installedKeys[ii] == _desiredKeys[ii]))) {
      ++unavailable;
    }
  }
  return unavailable;
}


///////////////////////////////////////////////////////////////////////
// CHECK_PRODUCT_VALIDITY:  use the data mapper info to find data that 
//   has been updated. Mark any such  data as invalid.
// 
void ProductMgr::check_product_validity(time_t tm,  DmapAccess &dmap)
{
  int nsets = dmap.getNInfo();

  for (size_t i = 0; i < _products.size(); i++) {

    if( _products[i]->_active) {

      // pull out dir from url
      char * start_ptr = strrchr(_params._symprod_prod_info[i].url,':');
      if(start_ptr == NULL) { // Must be a local file/dir based URL
        start_ptr = _params._symprod_prod_info[i].url;
      } else {
        start_ptr++;  // Move up one character
      }

      // Compare against all entries in data list
      for(int j = 0; j < nsets; j++ ) {
        const DMAP_info_t &info = dmap.getInfo(j);

        // See if any data matches 
        if(strstr(info.dir,start_ptr) != NULL) {
          if(_products[i]->_last_collected < (int) info.last_reg_time) {
            _installedKeyValid[i] = false;
            // 		      fprintf(stderr,"Product %s expired: last:%s",
            // 			      _sparams._prod_info[i].menu_label,
            // 			      asctime(gmtime(&(_products[i]->_last_collected))));
            //		      fprintf(stderr,"Now: %s",
            //			      asctime(gmtime(&((time_t) info.last_reg_time))));
          }
        }
      }
    }
  }
}
