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
//////////////////////////////////////////////////////////

#include <string>
#include <toolsa/str.h>
#include "Product.hh"

//////////////////////////////////////////
// default constructor

Product::Product(int debug,
                 Params::symprod_prod_info_t &prodInfo) :
        _prodInfo(prodInfo), _debug(debug)
        
  
{
  _data_valid = 0;
  _times_valid = 0;
  _prev_epoch_start = 0;
  _prev_epoch_end = 0;
  _chunksNeedProcess = false;
  _timesNeedProcess = false;
}

//////////////
// destructor
  
Product::~Product()
{
  clearSymprods();
  clearTimes();
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

///////////////////////
// clear times vector
  
void Product::clearTimes()
{
  _times.clear();
}

//////////////////////////////////
// DRAW the Product
//
// returns 0 on success, -1 on failure

void Product::draw(RenderContext &context)
  
{
  bool already_done = false;
  bool reverse_order = false;
  int index;
  Symprod::prod_hdr_props_t props;
  Symprod::prod_hdr_props_t props2;
  
  // Every time we render we start with a clean list.
  obj_list.erase(obj_list.begin(), obj_list.end());

  // If product is selected and its data is valid
  if(_active && _data_valid && _symprods.size() > 0 ) {

    // Determine order - Shouldn't have to do this - was promised increasing!
    props  = _symprods[0]->getProps();
    props2 = _symprods[_symprods.size() -1]->getProps();
    if(props.start_time > props2.start_time) reverse_order = true;

    for (int i = 0; i < (int) _symprods.size(); i++) {
	 if(reverse_order) {
	     index = _symprods.size() -1 - i;
	 } else {
	     index = i;
	 }
	props = _symprods[index]->getProps();

	 switch(_prodInfo.render_type) {

	   case Params::SYMPROD_RENDER_ALL:
	   case Params::SYMPROD_RENDER_GET_VALID:
	   case Params::SYMPROD_RENDER_GET_VALID_AT_FRAME_TIME:
	   case Params::SYMPROD_RENDER_FIRST_BEFORE_FRAME_TIME:
	       _symprods[index]->draw(context);  // Render the product
	   break;

	   case Params::SYMPROD_RENDER_ALL_VALID: // Valid in the given time frame
             if(props.start_time <= context.frame_end + (_prodInfo.minutes_allow_after * 60) &&
	        props.expire_time >= context.frame_start - (_prodInfo.minutes_allow_before * 60)) {
	           _symprods[index]->draw(context);  // Render the product

	     }
	   break;

	   case Params::SYMPROD_RENDER_VALID_IN_LAST_FRAME:
             if(props.start_time <= context.epoch_end + (_prodInfo.minutes_allow_after * 60) &&
	        props.expire_time >= context.epoch_end - (context.frame_end - context.frame_start) -
	        (_prodInfo.minutes_allow_before * 60)) {

	           _symprods[index]->draw(context);  // Render the product

	     }
	   break;

	   case Params::SYMPROD_RENDER_LATEST_IN_FRAME:
	     already_done = false;

	     // search through the object list 
	     for(size_t j = 0; j <  obj_list.size(); j++)  {
                 // prods with same start time are allowed
	         if (obj_list[j].start_time == props.start_time) {
	  	   continue;
	         }
		 // If the data type matches
		 if(obj_list[j].data_type == props.data_type) {
		   already_done = true;
		   break;
		 }
	      }

	      // If its wasn't already in the list and its valid, render it.
              if( !already_done && 
		 props.start_time <= context.frame_end + (_prodInfo.minutes_allow_after * 60) &&
	         props.expire_time >= context.frame_start - (_prodInfo.minutes_allow_before * 60)) {

	          _symprods[index]->draw(context);  // Render the product
		  obj_list.push_back(props); // add it to the list
	     } 
	   break;

	   case Params::SYMPROD_RENDER_LATEST_IN_LOOP:
	     already_done = false;

	     // search through the object list 
	     for(size_t j = 0; j < obj_list.size() && !already_done; j++) {
                 // prods with same start time are allowed
	         if (obj_list[j].start_time == props.start_time) {
	  	   continue;
	         }
		 // If the data type matches
		 if(obj_list[j].data_type == props.data_type) {
		   already_done = true; 
		   break;
		 }
	      }

	      // If its wasn't already in the list and its valid, render it.
              if( !already_done && 
		 props.start_time <= context.epoch_end + (_prodInfo.minutes_allow_after * 60) &&
	         props.expire_time >= context.epoch_start - (_prodInfo.minutes_allow_before * 60)) {

	          _symprods[index]->draw(context);  // Render the product
		  obj_list.push_back(props); // add it to the list
	     } 
	   break;

	   case Params::SYMPROD_RENDER_FIRST_AFTER_DATA_TIME:
	     already_done = false;

	     // search through the object list 
	     for(size_t j = 0 ; j < obj_list.size(); j++) {
                 // prods with same start time are allowed
	         if (obj_list[j].start_time == props.start_time) {
	  	   continue;
	         }
		 // If the data type matches
		 if(obj_list[j].data_type == props.data_type) {
		   already_done = true; 
		   break;
		 }
	      }

	      // If its wasn't already in the list and its valid, render it.
              if( !already_done && 
		 props.start_time <= context.data_time + (_prodInfo.minutes_allow_after * 60) &&
		 props.expire_time >= context.epoch_start - (_prodInfo.minutes_allow_before * 60)) {

	          _symprods[index]->draw(context);  // Render the product
		  obj_list.push_back(props); // add it to the list
	     } 
	   break;

	   case Params::SYMPROD_RENDER_FIRST_BEFORE_DATA_TIME:
	     already_done = false;

	     // search through the object list 
	     for(size_t j = 0; j < obj_list.size(); j++) {
                 // prods with same start time are allowed
	         if (obj_list[j].start_time == props.start_time) {
	  	   continue;
	         }
		 // If the data type matches
		 if(obj_list[j].data_type == props.data_type) {
		   already_done = true; 
		   break;
		 }
	     }

	      // If its wasn't already in the list and its valid, render it.
              if( !already_done && 
		 props.start_time >= context.data_time - (_prodInfo.minutes_allow_before * 60) &&
		 props.start_time <= context.epoch_end + (_prodInfo.minutes_allow_after * 60)) {

	          _symprods[index]->draw(context);  // Render the product
		  obj_list.push_back(props); // add it to the list
	     } 
	   break;

	   case Params::SYMPROD_RENDER_ALL_BEFORE_DATA_TIME:

	      // If it's valid, render it.
              if( props.start_time <= context.data_time + (_prodInfo.minutes_allow_after * 60) &&
		 props.start_time >= context.epoch_start - (_prodInfo.minutes_allow_before * 60)) {

	          _symprods[index]->draw(context);  // Render the product
	     } 
	   break;

	   case Params::SYMPROD_RENDER_ALL_AFTER_DATA_TIME:

	      // If it's valid, render it.
              if( props.start_time >= context.data_time - (_prodInfo.minutes_allow_before * 60) &&
		 props.start_time <= context.epoch_end + (_prodInfo.minutes_allow_after * 60)) {

	          _symprods[index]->draw(context);  // Render the product
		  obj_list.push_back(props); // add it to the list
	     } 
	   break;

	} 

    } // foreach symprod object

    if(_times.size() > 0) {
      //       if(gd.time_plot) gd.time_plot->add_prod_tlist(_prodInfo.menu_label,_times);
    }

  } // If product is selected and its data is valid 

}

///////////////////////////////////////////////////////
// Request/Get data
//

int Product::getData(const time_t data_start_time,
		     const time_t data_end_time,
		     RenderContext &context)
  
{

  struct timezone tz;
  char msg[128];
  char spdbp_url[1024];
  char tmp_str[1024];

  clearSymprods();
  _chunksNeedProcess = true;

  snprintf(msg,128,"Gathering %s data",_prodInfo.menu_label);
  // add_message_to_status_win(msg,0);

  if (_debug) {
    cerr << "Product:" << _prodInfo.menu_label << endl;
    cerr << "  dataStartTime: " << utimstr(data_start_time) ;
    cerr << "  dataEndTime: " << utimstr(data_end_time) << endl;
    cerr << "  data_type: " << _prodInfo.data_type ;
    cerr << "  URL: " << _prodInfo.url << endl;
  }

  gettimeofday(&gd.io_info.request_time,&tz);

  *spdbp_url = '\0'; // insure null termination
  *tmp_str = '\0';
  STRcopy(spdbp_url,_prodInfo.url,1024);

  // If using the tunnel - Add the tunnel_url to the Url as a name=value pair
  if(strlen(_params.http_tunnel_url) > URL_MIN_SIZE) { 
    // If using the a proxy - Add the a proxy_url to the Url as a name=value pair 
    // Note this must be used in conjunction with a tunnel_url.
    if((strlen(_params.http_proxy_url)) > URL_MIN_SIZE) {
      snprintf(tmp_str,1024, "?tunnel_url=%s&proxy_url=%s",_params.http_tunnel_url,_params.http_proxy_url);
    } else {
      snprintf(tmp_str,1024, "?tunnel_url=%s",_params.http_tunnel_url);
    }

    // Append the arguments to the end of the Url string
    STRconcat(spdbp_url,tmp_str,1023); 
  }

  if (_active &&
      (!_times_valid ||
       _prev_epoch_start != gd.epoch_start ||
       _prev_epoch_end != gd.epoch_end)) {
    
    // request the time list
    
    DsURL timesUrl(spdbp_url);
    timesUrl.setTranslator("");
    timesUrl.setParamFile("");
    
    if (_debug) {
      cerr << "orig url: " << spdbp_url << endl;
      cerr << "times url: " << timesUrl.getURLStr() << endl;
      cerr << "epoch_start: " << DateTime::str(gd.epoch_start) << endl;
      cerr << "epoch_end: " << DateTime::str(gd.epoch_end) << endl;
      cerr << "_prev_epoch_start: "
	   << DateTime::str(_prev_epoch_start) << endl;
      cerr << "_prev_epoch_end: "
	   << DateTime::str(_prev_epoch_end) << endl;
    }
    
    clearTimes();
    _timesNeedProcess = true;
    // double pixelsPerSec = ( gd.time_plot) ? gd.time_plot->getPixelsPerSec() : 1.0 ;
    double pixelsPerSec = 1.0;
    int secsPerPixel = (int) (1.0 / pixelsPerSec);
    int delta = gd.epoch_end - gd.epoch_start;
    if (_spdbTimes.compileTimeList(timesUrl.getURLStr(),
				   gd.epoch_start - delta,
				   gd.epoch_end + delta,
				   secsPerPixel)) {
      cerr << "ERROR - Product:" << _prodInfo.menu_label << endl;
      cerr << "  Cannot get times from" ;
      cerr << "  URL: " << _prodInfo.url << endl;
      cerr << "  startTime: " << utimstr(gd.epoch_start);
      cerr << "  endTime: " << utimstr(gd.epoch_end) << endl;
      cerr << _spdbTimes.getErrStr() << endl;
    }
    
    _times_valid = 1;
    _prev_epoch_start = gd.epoch_start;
    _prev_epoch_end = gd.epoch_end;
    
  }
  
  // request the data

  _spdb.clearHorizLimits();
  _spdb.clearVertLimits();
  _spdb.setUniqueOff();
  if(_params.symprod_gzip_requests) {
    _spdb.setDataCompressForTransfer(Spdb::COMPRESSION_GZIP);
  }

  // Send the information about the current display limits, just in case
  // the server cares

  int vlevel_num = gd.mread[gd.h_win.page]->plane;
    
  _spdb.setHorizLimits(context.min_lat, context.min_lon,
		       context.max_lat, context.max_lon);
  _spdb.setVertLimits(gd.mread[gd.h_win.page]->vert[vlevel_num].min,
		      gd.mread[gd.h_win.page]->vert[vlevel_num].max);

  int iret;
  if (_prodInfo.render_type == Params::SYMPROD_RENDER_GET_VALID) {
    time_t dataTime = context.data_time;
    if (_debug) {
      cerr << "Getting symprod data valid at time: " << utimstr(dataTime) << endl;
    }
    iret = _spdb.getValid(spdbp_url, dataTime, 
			  _prodInfo.data_type);

  } else if (_params.symprod_short_requests && _prodInfo.render_type == Params::SYMPROD_RENDER_FIRST_BEFORE_FRAME_TIME) {
    
    time_t frameTime = context.frame_end;
    int margin = (int) (_prodInfo.minutes_allow_before * 60);
    if (_debug) {
      cerr << "Getting symprod data FIRST BEFORE frame time: " << utimstr(frameTime) << endl;
    }
    iret = _spdb.getFirstBefore(spdbp_url, frameTime, margin, _prodInfo.data_type);

  } else if (_params.symprod_short_requests && _prodInfo.render_type == Params::SYMPROD_RENDER_GET_VALID_AT_FRAME_TIME) {

    int fwdSecs = (int) (_prodInfo.minutes_allow_after * 60);
    time_t validTime = context.frame_end + fwdSecs;
    if (_debug) {
      cerr << "Getting symprod data VALID AT TIME: "
	   << utimstr(validTime) << endl;
      cerr << "  Frame start: " << utimstr(context.frame_start) << endl;
      cerr << "  Frame end: " << utimstr(context.frame_end) << endl;
    }
    iret = _spdb.getValid(spdbp_url, validTime, _prodInfo.data_type);

  } else if ( _params.symprod_short_requests && _prodInfo.render_type == Params::SYMPROD_RENDER_LATEST_IN_FRAME) {

     _spdb.setUniqueLatest();
    iret = _spdb.getInterval(spdbp_url,
			     data_start_time, data_end_time,
			     _prodInfo.data_type);
  } else {
    iret = _spdb.getInterval(spdbp_url,
			     data_start_time, data_end_time,
			     _prodInfo.data_type);
  }
  
  if (iret) {
    cerr << "ERROR - Product:" << _prodInfo.menu_label << endl;
    cerr << "  Cannot get data from" ;
      cerr << "  URL: " << _prodInfo.url << endl;
      cerr << "  startTime: " << utimstr(data_start_time);
      cerr << "  endTime: " << utimstr(data_end_time) << endl;
      cerr << _spdb.getErrStr() << endl;
      _data_valid = 1;
      _last_collected = time(0);
      return -1;
  }
  
  _data_valid = 1;
  _last_collected = time(0);
  return 0;
}

////////////////////////////////////////////////////////////////
// Process Product chunks data 
//  Call this after the data transfer has completed
//

bool Product::processChunks()
  
{

  if (!_chunksNeedProcess) {
    return false;
  }
  _chunksNeedProcess = false;
		      
  struct timeval tm2;
  struct timezone tz;
  double t3;
  char msg[128];

  if (_debug) {
    cerr << "_spdb.getNChunks(): " << _spdb.getNChunks() << endl;
  }

  if (_spdb.getNChunks() == 0) {
    return true;
  }

  // create a symprod for each chunk
  
  vector<Spdb::chunk_t> chunks = _spdb.getChunks();

  gettimeofday(&tm2,&tz);

  t3 = _elapsedTime(gd.io_info.request_time,tm2);
  
  snprintf(msg, 128,
           "                                  ->   %d Chunks retrieved in %.3g seconds\n",
           (int)chunks.size(),t3);

  // add_message_to_status_win(msg,0);

  if(_debug) {
      int i = chunks.size();
      cerr << i << " Chunks retrieved in " << t3 << " seconds" << endl;
  }

  for (size_t ii = 0; ii < chunks.size(); ii++) {
    if (chunks[ii].len > 0) {
      SymprodRender *symprod = new SymprodRender(*this);
      if (symprod->deserialize(chunks[ii].data, chunks[ii].len) == 0) {
	_symprods.push_back(symprod);
      } else {
	cerr << "WARNING - Product::getData" << endl;
	cerr << "  Cannot dserialize product." << endl;
	cerr << "  URL: " << _prodInfo.url << endl;
	cerr << "  Chunk len: " << chunks[ii].len << endl;
	delete symprod;
      }
    } // if (chunks[ii].len > 0) 
  }

  return true;

}

////////////////////////////////////////////////////////////////
// Process the times from the refs
//  Call this after the data transfer has completed
//

bool Product::processTimeRefs()
  
{

  if (!_timesNeedProcess) {
    return false;
  }
  _timesNeedProcess = false;
		      
  if (_debug) {
    cerr << "Product::processTimeRefs()" << endl;
    cerr << "prod name: " << _prodInfo.menu_label << endl;
    cerr << "nTimes: " << _spdbTimes.getTimeList().size() << endl;
  }

  _times = _spdbTimes.getTimeList();
  // if(gd.time_plot) gd.time_plot->Draw(); 

  return true;

}

//////////////////////////////////
// pick_closest_obj Find the closest Symprod object.
//
// returns the distance to the closest object.

double Product::pick_closest_obj(double lat, double lon, RenderContext &context)
  
{
  double min_dist = 999999.0;
  double dist = 999999.0;

  bool already_done = false;
  bool reverse_order = false;
  int index;
  Symprod::prod_hdr_props_t props;
  Symprod::prod_hdr_props_t props2;
  
  // Every time we search we start with a clean list.
  obj_list.erase(obj_list.begin(), obj_list.end());

  // If product is selected and its data is valid
  if(_active && _data_valid && _symprods.size() > 0 ) {

    // Determine order - Shouldn't have to do this - was promised increasing!
    props  = _symprods[0]->getProps();
    props2 = _symprods[_symprods.size() -1]->getProps();
    if(props.start_time > props2.start_time) reverse_order = true;

    for (int i = _symprods.size() -1; i >= 0;  i--) {
	 if(reverse_order) {
	     index = _symprods.size() -1 - i;
	 } else {
	     index = i;
	 }
	props = _symprods[index]->getProps();

	 switch(_prodInfo.render_type) {

	   case Params::SYMPROD_RENDER_ALL:
	   case Params::SYMPROD_RENDER_GET_VALID:
	       dist = _symprods[index]->pick_closest_obj(lat,lon);  // Pick the closest product
	   break;

	   case Params::SYMPROD_RENDER_ALL_VALID: // Valid in the given time frame
             if(props.start_time <= context.frame_end + (_prodInfo.minutes_allow_after * 60) &&
	        props.expire_time >= context.frame_start - (_prodInfo.minutes_allow_before * 60)) {
	           dist =_symprods[index]->pick_closest_obj(lat,lon);  // Pick the closest product

	     }
	   break;

	   case Params::SYMPROD_RENDER_VALID_IN_LAST_FRAME:
             if(props.start_time <= context.epoch_end + (_prodInfo.minutes_allow_after * 60) &&
	        props.expire_time >= context.epoch_end - (context.frame_end - context.frame_start) -
	        (_prodInfo.minutes_allow_before * 60)) {

	           dist =_symprods[index]->pick_closest_obj(lat,lon);  // Pick the closest product

	     }
	   break;

	   case Params::SYMPROD_RENDER_LATEST_IN_FRAME:
	     already_done = false;

	     // search through the object list 
	     for(size_t j = 0; j <  obj_list.size(); j++)  {
                 // prods with same start time are allowed
	         if (obj_list[j].start_time == props.start_time) {
	  	   continue;
	         }
		 // If the data type matches
		 if(obj_list[j].data_type == props.data_type) {
		   already_done = true;
		   break;
		 }
	      }

	      // If its wasn't already in the list and its valid, check it .
              if( !already_done && 
		 props.start_time <= context.frame_end + (_prodInfo.minutes_allow_after * 60) &&
	         props.expire_time >= context.frame_start - (_prodInfo.minutes_allow_before * 60)) {

	          dist =_symprods[index]->pick_closest_obj(lat,lon);  // Pick the closest product
		  obj_list.push_back(props); // add it to the list
	     } 
	   break;

	   case Params::SYMPROD_RENDER_LATEST_IN_LOOP:
	     already_done = false;

	     // search through the object list 
	     for(size_t j = 0; j < obj_list.size() && !already_done; j++) {
                 // prods with same start time are allowed
	         if (obj_list[j].start_time == props.start_time) {
	  	   continue;
	         }
		 // If the data type matches
		 if(obj_list[j].data_type == props.data_type) {
		   already_done = true; 
		   break;
		 }
	      }

	      // If its wasn't already in the list and its valid, render it.
              if( !already_done && 
		 props.start_time <= context.epoch_end + (_prodInfo.minutes_allow_after * 60) &&
	         props.expire_time >= context.epoch_start - (_prodInfo.minutes_allow_before * 60)) {

	          dist = _symprods[index]->pick_closest_obj(lat,lon);  // Pick the closest product
		  obj_list.push_back(props); // add it to the list
	     } 
	   break;

	   case Params::SYMPROD_RENDER_FIRST_BEFORE_DATA_TIME:
	     already_done = false;

	     // search through the object list 
	     for(size_t j = 0 ; j < obj_list.size(); j++) {
                 // prods with same start time are allowed
	         if (obj_list[j].start_time == props.start_time) {
	  	   continue;
	         }
		 // If the data type matches
		 if(obj_list[j].data_type == props.data_type) {
		   already_done = true; 
		   break;
		 }
	      }

	      // If its wasn't already in the list and its valid, render it.
              if( !already_done && 
		 props.start_time <= context.data_time + (_prodInfo.minutes_allow_after * 60) &&
		 props.expire_time >= context.epoch_start - (_prodInfo.minutes_allow_before * 60)) {

	          dist = _symprods[index]->pick_closest_obj(lat,lon);  // Pick the closest product
		  obj_list.push_back(props); // add it to the list
	     } 
	   break;

	   case Params::SYMPROD_RENDER_FIRST_AFTER_DATA_TIME:
	     already_done = false;

	     // search through the object list 
	     for(size_t j = 0; j < obj_list.size(); j++) {
                 // prods with same start time are allowed
	         if (obj_list[j].start_time == props.start_time) {
	  	   continue;
	         }
		 // If the data type matches
		 if(obj_list[j].data_type == props.data_type) {
		   already_done = true; 
		   break;
		 }
	     }

	      // If its wasn't already in the list and its valid, render it.
              if( !already_done && 
		 props.start_time >= context.data_time - (_prodInfo.minutes_allow_before * 60) &&
		 props.start_time <= context.epoch_end + (_prodInfo.minutes_allow_after * 60)) {

	          dist = _symprods[index]->pick_closest_obj(lat,lon);  // Pick the closest product
		  obj_list.push_back(props); // add it to the list
	     } 
	   break;

	   case Params::SYMPROD_RENDER_ALL_BEFORE_DATA_TIME:

	      // If it's valid, render it.
              if( props.start_time <= context.data_time + (_prodInfo.minutes_allow_after * 60) &&
		 props.start_time >= context.epoch_start - (_prodInfo.minutes_allow_before * 60)) {

	          dist = _symprods[index]->pick_closest_obj(lat,lon);  // Pick the closest product
	     } 
	   break;

	   case Params::SYMPROD_RENDER_ALL_AFTER_DATA_TIME:

	      // If it's valid, render it.
              if( props.start_time >= context.data_time - (_prodInfo.minutes_allow_before * 60) &&
		 props.start_time <= context.epoch_end + (_prodInfo.minutes_allow_after * 60)) {

	          dist = _symprods[index]->pick_closest_obj(lat,lon);  // Pick the closest product
		  obj_list.push_back(props); // add it to the list
	     } 
	   break;

	 default: {}

	}  // Switch on product render type

	if(dist < min_dist) {
			min_dist = dist;
			closest_symprod_obj = _symprods[index]->get_closest_obj();
	}
   } // foreach symprod object


  } // If product is selected and its data is valid 

  return min_dist;
}

/**************************************************************************
 * ELAPSED_TIME: Compute the elapsed time in seconds
 */

double Product::_elapsedTime(struct timeval &tm1, struct timeval &tm2) 
{
  long t1,t2;
  
  t1 = tm2.tv_sec - tm1.tv_sec;
  t2 = tm2.tv_usec - tm1.tv_usec; 
  if(t2 < 0) {  // Must borrow a million microsecs from the seconds column  
    t1--;
    t2 += 1000000;
  }  
  
  return (double) t1 + ((double) t2 / 1000000.0);
}

