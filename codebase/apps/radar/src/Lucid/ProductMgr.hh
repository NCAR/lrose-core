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
////////////////////////////////////////////////////////////////
// ProductMgr.hh
//
// Product Rendering Manager
//
// Jan 2000
//
////////////////////////////////////////////////////////////////

#ifndef ProductMgr_HH
#define ProductMgr_HH

#include "GlobalData.hh"
#include "Params.hh"
#include "Product.hh"
#include <dsserver/DmapAccess.hh>
#include <QSet>
#include <QThread>
#include <functional>
#include <memory>

class SymprodRenderObj;
class QPainter;
struct SymprodRenderContext;
class WorldPlot;
class QObject;

struct SymprodRequestKey {
  time_t startTime = 0;
  time_t endTime = 0;
  time_t dataTime = 0;
  bool useHorizLimits = false;
  double minLat = 0.0;
  double minLon = 0.0;
  double maxLat = 0.0;
  double maxLon = 0.0;
  bool useVertLimits = false;
  double minAlt = 0.0;
  double maxAlt = 0.0;

  bool operator==(const SymprodRequestKey &rhs) const;
};

struct SymprodReadRequest {
  int productIndex = -1;
  int generation = 0;
  string label;
  string url;
  int dataType = 0;
  Params::symprod_render_type_t renderType = Params::SYMPROD_RENDER_ALL;
  bool debug = false;
  bool shortRequests = false;
  bool gzip = false;
  time_t frameStart = 0;
  time_t frameEnd = 0;
  bool requestTimeList = false;
  time_t timeListStart = 0;
  time_t timeListEnd = 0;
  SymprodRequestKey key;
};

struct SymprodReadResult {
  int productIndex = -1;
  int generation = 0;
  string label;
  SymprodRequestKey key;
  int status = -1;
  string error;
  vector<QByteArray> chunks;
  int timeListStatus = -1;
  string timeListError;
  time_t timeListStart = 0;
  time_t timeListEnd = 0;
  vector<time_t> availableTimes;
};

///////////////////////////////////////////////////////////////
// class definition

class ProductMgr

{

public:

  // default constructor
  
  ProductMgr();
  
  // destructor
  
  virtual ~ProductMgr();

  // Start all required SPDB requests for the current display state. Each
  // product runs independently, in parallel with the MDV request.
  void requestData(time_t frameStart,
                   time_t frameEnd,
                   time_t dataTime,
                   double selectedLevel,
                   const WorldPlot &plot);

  // Re-evaluate enabled products using the most recently captured display
  // state, for example after a menu toggle or explicit reload.
  void refreshActiveProducts();

  void setProductsChangedCallback(const std::function<void()> &callback) {
    _productsChangedCallback = callback;
  }

  void setStatusCallback(
    const std::function<void(const string &, bool)> &callback) {
    _statusCallback = callback;
  }
  
  // Turn on/off the product - Skip rendering if active != TRUE
  void set_product_active(int product, int active);

  // Get  the product's on/off state 
  int get_product_active(int product);

  // Reset all valid flags to invalid
  void reset_product_valid_flags(void);
  
  // Reset valid flags of products that should retrieve new data when the
  // current zoom changes

  void reset_product_valid_flags_zoom(void);
  
  // Reset valid flags of products that should retrieve new data when the
  // vertical level changes in the display

  void reset_product_valid_flags_vert(void);
  
  // Set the data valid state for one product
  void set_product_data_valid(int product, int valid);

  // Get the validity flag for the indicated product
  int get_product_data_valid(int product);
  
  void drawQt(QPainter &painter,
              const SymprodRenderContext &context);

  bool pickClosestObject(double lat, double lon,
                         const SymprodRenderContext &context,
                         double maxDistanceKm);
  void clearPickedObject() { _active_Rob = nullptr; }
  void drawPickedQt(QPainter &painter,
                    const SymprodRenderContext &context) const;

  // Returns the number of products marked as invalid.
  int num_products_invalid(void);

  // True only when every active product has installed the exact request key
  // currently desired by the display. Movie capture uses this stronger test
  // so an older but still valid snapshot is never recorded for a new frame.
  bool activeProductsReady() const;

  // Best-effort image generation may proceed once every active request has
  // returned, even when one of the servers could not provide a snapshot.
  bool activeProductRequestsSettled() const;
  int numActiveProductsUnavailable() const;

  // Returns the number of products its supervising
  int get_num_products(void);

  void check_product_validity(time_t tm, DmapAccess &dmap);  // Check datamapper info for updated data

  void fillExportFields(vector<world_pt_t> &wpt, string &label) const;
  
  const Product *getProduct(int index) const {
    if (index >= 0 && index < (int) _products.size()) {
      return _products[index].get();
    } else {
      return NULL;
    }
  }

protected:
  
  Params &_params;
  GlobalData &_gd;
  vector<unique_ptr<Product>> _products;
  SymprodRenderObj *_active_Rob;
  
private:

  void _startRequest(const SymprodReadRequest &request);
  void _handleResult(SymprodReadResult *result);
  void _computeLatLonBounds(const WorldPlot &plot,
                            double &minLat, double &minLon,
                            double &maxLat, double &maxLon) const;
  void _requestCapturedState();

  vector<int> _generations;
  vector<bool> _requestInFlight;
  vector<bool> _installedKeyValid;
  vector<bool> _timeListValid;
  vector<time_t> _timeListStarts;
  vector<time_t> _timeListEnds;
  vector<time_t> _retryNotBefore;
  vector<SymprodRequestKey> _desiredKeys;
  vector<SymprodRequestKey> _installedKeys;
  QSet<QThread *> _readThreads;
  // This QObject owns the request threads and is also their queued-completion
  // context. Releasing it after joining workers cancels every pending callback.
  unique_ptr<QObject> _completionContext;
  bool _shuttingDown;

  bool _lastDisplayStateValid;
  time_t _lastFrameStart;
  time_t _lastFrameEnd;
  time_t _lastDataTime;
  double _lastSelectedLevel;
  double _lastMinLat;
  double _lastMinLon;
  double _lastMaxLat;
  double _lastMaxLon;
  std::function<void()> _productsChangedCallback;
  std::function<void(const string &, bool)> _statusCallback;

};

#endif


