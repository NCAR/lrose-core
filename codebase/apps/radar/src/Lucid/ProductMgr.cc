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
#include "RenderContext.hh"
#include "SymprodRender.hh"
#include "SymprodRenderObj.hh"
#include "MdvReader.hh"
#include "SymprodRenderContext.hh"
#include "WorldPlot.hh"
#include <Spdb/DsSpdb.hh>
#include <QObject>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>

namespace {

class SymprodReadThread : public QThread
{
public:
  explicit SymprodReadThread(const SymprodReadRequest &request) :
          _request(request), _result(nullptr)
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
      status = spdb.getValid(_request.url, _request.key.dataTime,
                             _request.dataType);
    } else if (_request.shortRequests &&
               _request.renderType ==
                 Params::SYMPROD_RENDER_FIRST_BEFORE_FRAME_TIME) {
      const int margin = (int) (_request.frameStart -
                                _request.key.startTime);
      status = spdb.getFirstBefore(_request.url, _request.frameEnd,
                                   margin, _request.dataType);
    } else if (_request.shortRequests &&
               _request.renderType ==
                 Params::SYMPROD_RENDER_GET_VALID_AT_FRAME_TIME) {
      status = spdb.getValid(_request.url, _request.key.endTime,
                             _request.dataType);
    } else {
      if (_request.shortRequests &&
          _request.renderType == Params::SYMPROD_RENDER_LATEST_IN_FRAME) {
        spdb.setUniqueLatest();
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

ProductMgr::ProductMgr(RenderContext &context, int debug) :
        _params(Params::Instance()),
        _gd(GlobalData::Instance()),
        _context(context)
          
{
  _active_Rob = NULL;
  _completionContext = new QObject;
  _shuttingDown = false;
  _lastDisplayStateValid = false;
  _lastFrameStart = 0;
  _lastFrameEnd = 0;
  _lastDataTime = 0;
  _lastSelectedLevel = 0.0;
  _lastMinLat = _lastMinLon = 0.0;
  _lastMaxLat = _lastMaxLon = 0.0;

  for (int i = 0; i < _params.symprod_prod_info_n; i++) {
    
    Product *product = new Product(debug, _params._symprod_prod_info[i]);
    
    if (_params.symprod_debug != Params::SYMPROD_DEBUG_OFF ) {
      product->setDebug(true);
    } else {
      product->setDebug(false);
    }
    _products.push_back(product);
    
  }

  _generations.resize(_products.size(), 0);
  _requestInFlight.resize(_products.size(), false);
  _installedKeyValid.resize(_products.size(), false);
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

  // Destroying the connection context cancels queued completion lambdas that
  // capture this manager, including signals emitted just before shutdown.
  delete _completionContext;
  _completionContext = nullptr;

  // A worker owns all SPDB request state, but its completion handler targets
  // this manager. Wait for blocking calls to return before freeing Products.
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
    delete readThread;
  }
  _readThreads.clear();

  for (size_t i = 0; i < _products.size(); i++) {
    delete _products[i];
  }
  _products.erase(_products.begin(), _products.end());
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
    request.shortRequests = _params.symprod_short_requests;
    request.gzip = _params.symprod_gzip_requests;
    request.frameStart = _lastFrameStart;
    request.frameEnd = _lastFrameEnd;
    request.key = key;

    _desiredKeys[ii] = key;
    _requestInFlight[ii] = true;
    _startRequest(request);
  }
}

void ProductMgr::_startRequest(const SymprodReadRequest &request)
{
  SymprodReadThread *thread = new SymprodReadThread(request);
  _readThreads.insert(thread);

  // A manager-owned QObject supplies the GUI-thread connection context. Its
  // destruction cancels queued lambdas before ProductMgr itself is released.
  QObject::connect(thread, &QThread::finished, _completionContext,
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
  if (result->generation != _generations[index]) {
    return;
  }
  _requestInFlight[index] = false;

  Product &product = *_products[index];
  if (result->status != 0) {
    cerr << "ERROR - ProductMgr: cannot retrieve "
         << product._prodInfo.menu_label << endl;
    cerr << "  URL: " << product._prodInfo.url << endl;
    cerr << result->error << endl;
    // The main GUI timer is intentionally very frequent. Bound retries here
    // so an unavailable SPDB server cannot create a tight request loop.
    _retryNotBefore[index] = time(nullptr) + 5;
    return;
  }

  string decodeError;
  if (!product.installChunks(result->chunks, decodeError)) {
    cerr << "ERROR - ProductMgr: cannot decode "
         << product._prodInfo.menu_label << endl;
    cerr << decodeError << endl;
    _retryNotBefore[index] = time(nullptr) + 5;
    return;
  }

  _installedKeys[index] = _desiredKeys[index];
  _installedKeyValid[index] = true;
  _retryNotBefore[index] = 0;
  if (_productsChangedCallback) {
    _productsChangedCallback();
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

//////////////////////////////////////////////////
// get data in interval mode
//
// returns 0 on success, -1 on failure

int ProductMgr::getData(time_t start_time, time_t end_time)
  
{

  // set context times

  _gd.r_context->set_times((time_t) _gd.epoch_start,
                           (time_t) _gd.epoch_end,
                           (time_t) start_time,
                           (time_t) end_time,
                           _gd.mread[_gd.h_win.page]->h_date.utime());

  // get the data
  
  for (size_t i = 0; i < _products.size(); i++) {
    if(_products[i]->_data_valid == 0 && _products[i]->_active  ) {

      // _gd.io_info.prod = _products[i];
      // _gd.io_info.outstanding_request = 1; 
      // _gd.io_info.request_type = SYMPROD_REQUEST;
      // _gd.io_info.mode = SYMPROD_DATA;
      // _gd.io_info.expire_time = time(0) + _params.data_timeout_secs;
      // _gd.io_info.last_read = 0;

      if(_gd.debug1) _products[i]->setThreadingOff();

      char label[128];
      snprintf(label,128,"Requesting %s Product Data", _params._symprod_prod_info[i].menu_label);
      
#ifdef NOTNOW
      if(_params.show_data_messages) gui_label_h_frame(label,-1);
#endif

      if (_products[i]->getData(
                  (time_t) (start_time - (_params._symprod_prod_info[i].minutes_allow_before * 60)),
                  (time_t) (end_time + (_params._symprod_prod_info[i].minutes_allow_after * 60)),
                  _context)) {

        _products[i]->_data_valid = 1;
        return Constants::CIDD_FAILURE;
      }
      return Constants::INCOMPLETE;
    }
  }

  return Constants::CIDD_SUCCESS;

}

//////////////////////////////////
// PICK_CLOSEST_OBJ Find the closest object
// to the click point for all products 
//

void ProductMgr::pick_closest_obj(double lat, double lon)
{
  double min_dist = 999999.0; // A large distance
  double dist;

  _active_Rob = (SymprodRenderObj *) NULL;

  for (size_t i = 0; i < _products.size(); i++) {

    dist = _products[i]->pick_closest_obj(lat, lon, _context);

#ifdef DEBUG
    fprintf(stderr, "Prod: %d Min Dist: %g\n",i,dist);
#endif

    if(dist <= min_dist) { // greater than or equal to allow latest to be closest.
      min_dist = dist;
      _active_Rob = _products[i]->get_closest_symprod_obj();
    }
  }

}

//////////////////////////////////
// DRAW the picked product Using the XOR GC
//

void ProductMgr::draw_pick_obj()
  
{
  _context.set_brush(_gd.ol_brush);
  _context.set_drawable(_gd.hcan_pdev);
  _context.set_xid(_gd.hcan_pdev);

  if(_active_Rob != NULL) {
    _active_Rob->container->draw(_context);
  } else {
    fprintf(stderr," NO Pickable product!\n");
  }

  _context.set_brush(_gd.def_brush);
  _context.set_drawable(_gd.h_win.vis_pdev);
  _context.set_xid(_gd.h_win.vis_pdev);
}

//////////////////////////////////
// DRAW the picked product Using existing GC
//

void ProductMgr::draw_pick_obj_plain()
  
{
  _context.set_brush(_gd.def_brush);
  _context.set_drawable(_gd.hcan_pdev);
  _context.set_xid(_gd.hcan_pdev);
  if(_active_Rob != NULL) {
    _active_Rob->container->draw(_context);
  }
}

//////////////////////////////////
// fill_export_fields
//

void ProductMgr::fill_export_fields(vector<world_pt_t> &wpt, string &label)
  
{
  if(_active_Rob != NULL) {
    _active_Rob->container-> fill_export_fields(wpt, label);
  }
}

//////////////////////////////////
// DRAW the products
//

void ProductMgr::draw()
  
{
  for (size_t i = 0; i < _products.size(); i++) {
    _products[i]->draw(_context);
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

//////////////////////////////////
// RESET_TIMES_VALID_FLAGS 
//

void ProductMgr::reset_times_valid_flags()
  
{
  for (size_t i = 0; i < _products.size(); i++) {
    _products[i]->_times_valid = 0;
  }
  
}

//////////////////////////////////
// RESET_PRODUCT_VALID_FLAGS 
//

void ProductMgr::reset_product_valid_flags()
  
{
  for (size_t i = 0; i < _products.size(); i++) {
    _installedKeyValid[i] = false;
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
  if (active) {
    refreshActiveProducts();
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
// SET_PRODUCT_TIMES_VALID
//
void ProductMgr::set_product_times_valid(int product, int valid)
{
  _products[product]->_times_valid  = valid;
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
