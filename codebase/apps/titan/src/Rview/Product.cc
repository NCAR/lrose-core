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
// Product.cc
//
// Single product data retrieval and rendering
//
// Mike Dixon, from Nancy Rehak
// RAP, NCAR, Boulder, CO, 80307, USA
//
// Jan 2000
//
//////////////////////////////////////////////////////////

#include "Product.hh"
using namespace std;

//////////////////////////////////////////
// default constructor

Product::Product(Display *display)
  : _display(display)
  
{

  _debug = false;
  MEM_zero(_prodInfo);
  _prevRequestTime = 0;
  
}

//////////////
// destructor
  
Product::~Product()
{
  clearSymprods();
}

///////////////////////
// clear symprod vector
  
void Product::clearSymprods()
{
  for (size_t i = 0; i < _symprods.size(); i++) {
    delete _symprods[i];
  }
  _symprods.erase(_symprods.begin(), _symprods.end());
}

//////////////////////////////////
// draw the Product
//
// returns 0 on success, -1 on failure

void Product::draw(RenderContext *context)
  
{

  // draw
  
  for (size_t i = 0; i < _symprods.size(); i++) {
    _symprods[i]->draw(context);
  }

}

//////////////////////////////////
// get the data in realtime mode

int Product::getDataRealtime(const time_t frame_time,
			     const time_t data_time)
  
{

  time_t requestTime;
  if (_prodInfo.use_frame_time) {
    requestTime = frame_time;
  } else {
    requestTime = data_time;
  }

  // check if we are in genuine realtime mode or playback mode.

  time_t now = time(NULL);
  
  if (abs(now - requestTime) < 3600) {
    // genuine realtime mode
    // replace request time with current time
    requestTime = now;
  } else {
    // playback mode
  }
  
  if (requestTime == _prevRequestTime) {
    return 0;
  }
  
  if (_getData(requestTime)) {
    cerr << "ERROR - Product::getDataRealtime" << endl;
    cerr << "  Cannot get data for time: " << utimstr(requestTime) << endl;
    cerr << "  URL: " << _prodInfo.url << endl;
    return -1;
  }
  
  _prevRequestTime = requestTime;

  return 0;

}

//////////////////////////////////
// get the data in archive mode

int Product::getDataArchive(const time_t frame_time,
			    const time_t data_time)
  
{

  time_t requestTime;
  if (_prodInfo.use_frame_time) {
    requestTime = frame_time;
  } else {
    requestTime = data_time;
  }

  if (_getData(requestTime)) {
    cerr << "ERROR - Product::getDataArchive" << endl;
    cerr << "  Cannot get data for time: " << utimstr(requestTime) << endl;
    cerr << "  URL: " << _prodInfo.url << endl;
    return -1;
  }
  
  _prevRequestTime = requestTime;

  return 0;

}

//////////////////////////////////
// generic get

int Product::_getData(const time_t request_time)
  
{
  
  clearSymprods();
  int iret = 0;

  if (_prodInfo.make_unique) {
    _spdb.setUniqueLatest();
  }
  
  switch (_prodInfo.retrieval_type) {
    
  case ProdParams::RETRIEVE_CLOSEST:
    if (_debug) {
      cerr << "Product::getData: RETRIEVE_CLOSEST" << endl;
      cerr << "  request_time: " << utimstr(request_time) << endl;
      cerr << "  time_margin: " << _prodInfo.time_margin << endl;
      cerr << "  data_type: " << _prodInfo.data_type << endl;
      cerr << "  URL: " << _prodInfo.url << endl;
    }
    if (_spdb.getClosest(_prodInfo.url,
			 request_time,
			 _prodInfo.time_margin,
			 _prodInfo.data_type)) {
      iret = -1;
    }
    break;
    
  case ProdParams::RETRIEVE_FIRST_BEFORE:
    if (_debug) {
      cerr << "Product::getData: RETRIEVE_FIRST_BEFORE" << endl;
      cerr << "  request_time: " << utimstr(request_time) << endl;
      cerr << "  interval_before: " << _prodInfo.interval_before << endl;
      cerr << "  data_type: " << _prodInfo.data_type << endl;
      cerr << "  URL: " << _prodInfo.url << endl;
    }
    if (_spdb.getFirstBefore(_prodInfo.url,
			     request_time,
			     _prodInfo.interval_before,
			     _prodInfo.data_type)) {
      iret = -1;
    }
    break;

  case ProdParams::RETRIEVE_FIRST_AFTER:
    if (_debug) {
      cerr << "Product::getData: RETRIEVE_FIRST_AFTER" << endl;
      cerr << "  request_time: " << utimstr(request_time) << endl;
      cerr << "  interval_after: " << _prodInfo.interval_after << endl;
      cerr << "  data_type: " << _prodInfo.data_type << endl;
      cerr << "  URL: " << _prodInfo.url << endl;
    }
    if (_spdb.getFirstAfter(_prodInfo.url,
			    request_time,
			    _prodInfo.interval_after,
			    _prodInfo.data_type)) {
      iret = -1;
    }
    break;

  case ProdParams::RETRIEVE_INTERVAL:
    if (_debug) {
      cerr << "Product::getData: RETRIEVE_INTERVAL" << endl;
      cerr << "  request_time: " << utimstr(request_time) << endl;
      cerr << "  interval_before: " << _prodInfo.interval_before << endl;
      cerr << "  interval_after: " << _prodInfo.interval_after << endl;
      cerr << "  data_type: " << _prodInfo.data_type << endl;
      cerr << "  URL: " << _prodInfo.url << endl;
    }
    if (_spdb.getInterval(_prodInfo.url,
			  request_time - _prodInfo.interval_before,
			  request_time + _prodInfo.interval_after,
			  _prodInfo.data_type)) {
      iret = -1;
    }
    break;

  case ProdParams::RETRIEVE_VALID:
    if (_debug) {
      cerr << "Product::getData: RETRIEVE_VALID" << endl;
      cerr << "  request_time: " << utimstr(request_time) << endl;
      cerr << "  data_type: " << _prodInfo.data_type << endl;
      cerr << "  URL: " << _prodInfo.url << endl;
    }
    if (_spdb.getValid(_prodInfo.url,
		       request_time,
		       _prodInfo.data_type)) {
      iret = -1;
    }
    break;

  } // switch

  if (iret) {
    cerr << "ERROR - Product::getData" << endl;
    cerr << "  Cannot get data for time: " << utimstr(request_time) << endl;
    cerr << "  URL: " << _prodInfo.url << endl;
    return -1;
  }

  if (_spdb.getNChunks() == 0) {
    return 0;
  }

  // create a symprod for each chunk
  
  vector<Spdb::chunk_t> chunks = _spdb.getChunks();

  for (size_t ii = 0; ii < chunks.size(); ii++) {
    
    SymprodRender *symprod = new SymprodRender(_display);
    if (symprod->deserialize(chunks[ii].data, chunks[ii].len) == 0) {
      if (_prodInfo.expire_secs >= 0) {
	const Symprod::prod_hdr_props_t &props = symprod->getProps();
	time_t expire_time = props.generate_time + _prodInfo.expire_secs;
	symprod->setExpireTime(expire_time);
      }
      _symprods.push_back(symprod);
    } else {
      cerr << "WARNING - Product::getData" << endl;
      cerr << "  Cannot dserialize product for time: "
	   << utimstr(request_time) << endl;
      cerr << "  URL: " << _prodInfo.url << endl;
      delete symprod;
    }

  }

  return 0;

}

///////////////////////////////////////////////////////
// get data in an interval
//
// This data will be selectively rendered by the display.

int Product::getDataInterval(const time_t start_time,
			     const time_t end_time)
  
{

  clearSymprods();

  if (_debug) {
    cerr << "Product::getDataInterval" << endl;
    cerr << "  startTime: " << utimstr(start_time) << endl;
    cerr << "  endTime: " << utimstr(end_time) << endl;
    cerr << "  data_type: " << _prodInfo.data_type << endl;
    cerr << "  URL: " << _prodInfo.url << endl;
  }
  if (_spdb.getInterval(_prodInfo.url,
			start_time, end_time,
			_prodInfo.data_type)) {
    cerr << "ERROR - Product::getDataInterval" << endl;
    cerr << "  Cannot get data." << endl;
    cerr << "  startTime: " << utimstr(start_time) << endl;
    cerr << "  endTime: " << utimstr(end_time) << endl;
    cerr << "  URL: " << _prodInfo.url << endl;
    return -1;
  }

  if (_spdb.getNChunks() == 0) {
    return 0;
  }

  // create a symprod for each chunk
  
  vector<Spdb::chunk_t> chunks = _spdb.getChunks();

  for (size_t ii = 0; ii < chunks.size(); ii++) {

    if (chunks[ii].len > 0) {
      SymprodRender *symprod = new SymprodRender(_display);
      if (symprod->deserialize(chunks[ii].data, chunks[ii].len) == 0) {
	if (_prodInfo.expire_secs >= 0) {
	  const Symprod::prod_hdr_props_t &props = symprod->getProps();
	  time_t expire_time = props.generate_time + _prodInfo.expire_secs;
	  symprod->setExpireTime(expire_time);
	}
	_symprods.push_back(symprod);
      } else {
      cerr << "WARNING - Product::getDataInterval" << endl;
      cerr << "  Cannot dserialize product." << endl;
      cerr << "  URL: " << _prodInfo.url << endl;
      delete symprod;
      }
    }
    
  }

  return 0;

}

