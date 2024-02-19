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

//////////////////////////////////////////
// default constructor

ProductMgr::ProductMgr(RenderContext &context, int debug) : _context(context)
          
{
  _active_Rob = NULL;

  for (int i = 0; i < _params.symprod_prod_info_n; i++) {
    
    Product *product = new Product(debug, _params._symprod_prod_info[i]);
    
    if (_params.symprod_debug != Params::SYMPROD_DEBUG_OFF ) {
      product->setDebug(true);
    } else {
      product->setDebug(false);
    }
    _products.push_back(product);
    
  }

}

//////////////
// destructor
  
ProductMgr::~ProductMgr()
{
  for (size_t i = 0; i < _products.size(); i++) {
    delete _products[i];
  }
  _products.erase(_products.begin(), _products.end());
}

//////////////////////////////////////////////////
// get data in interval mode
//
// returns 0 on success, -1 on failure

int ProductMgr::getData(time_t start_time, time_t end_time)
  
{

  // set context times

  gd.r_context->set_times((time_t) gd.epoch_start,
  			  (time_t) gd.epoch_end,
  			  (time_t) start_time,
  			  (time_t) end_time,
			  gd.mrec[gd.h_win.page]->h_date.unix_time);

  // get the data
  
  for (size_t i = 0; i < _products.size(); i++) {
    if(_products[i]->_data_valid == 0 && _products[i]->_active  ) {

      gd.io_info.prod = _products[i];
      gd.io_info.outstanding_request = 1; 
      gd.io_info.request_type = SYMPROD_REQUEST;
      gd.io_info.mode = SYMPROD_DATA;
      gd.io_info.expire_time = time(0) + _params.data_timeout_secs;
      gd.io_info.last_read = 0;

      if(gd.debug1) _products[i]->setThreadingOff();

      char label[128];
      sprintf(label,"Requesting %s Product Data", _params._symprod_prod_info[i].menu_label);

      if(_params.show_data_messages) gui_label_h_frame(label,-1);

      if (_products[i]->getData(
          (time_t) (start_time - (_params._symprod_prod_info[i].minutes_allow_before * 60)),
          (time_t) (end_time + (_params._symprod_prod_info[i].minutes_allow_after * 60)),
	  _context)) {

          _products[i]->_data_valid = 1;
           return CIDD_FAILURE;
     }
    return INCOMPLETE;
    }
  }

  return CIDD_SUCCESS;

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
	_context.set_gc(gd.ol_gc);
	_context.set_drawable(gd.hcan_xid);
	_context.set_xid(gd.hcan_xid);

	if(_active_Rob != NULL) {
		_active_Rob->container->draw(_context);
	} else {
		 fprintf(stderr," NO Pickable product!\n");
	}

	_context.set_gc(gd.def_gc);
	_context.set_drawable(gd.h_win.vis_xid);
	_context.set_xid(gd.h_win.vis_xid);
}

//////////////////////////////////
// DRAW the picked product Using existing GC
//

void ProductMgr::draw_pick_obj_plain()
  
{
	_context.set_gc(gd.def_gc);
	_context.set_drawable(gd.hcan_xid);
	_context.set_xid(gd.hcan_xid);
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
    _products[i]->_data_valid = 0;
  }
  
}

//////////////////////////////////
// RESET_PRODUCT_VALID_FLAGS_ZOOM
//

void ProductMgr::reset_product_valid_flags_zoom()
  
{
  for (size_t i = 0; i < _products.size(); i++) {
    if (_products[i]->_prodInfo.request_data_on_zoom)
      _products[i]->_data_valid = 0;
  }
  
}

//////////////////////////////////
// RESET_PRODUCT_VALID_FLAGS_VERT
//

void ProductMgr::reset_product_valid_flags_vert()
  
{
  for (size_t i = 0; i < _products.size(); i++) {
    if (_products[i]->_prodInfo.request_data_on_vert_change)
      _products[i]->_data_valid = 0;
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
}

///////////////////////////////////////////////////////////////////////
// SET_PRODUCT_DATA_VALID
//
void ProductMgr::set_product_data_valid(int product, int valid)
{
    _products[product]->_data_valid  = valid;
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
    return _products[product]->_data_valid;
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
    if(_products[i]->_active && _products[i]->_data_valid ==  0) num++;
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
                      _products[i]->_data_valid = 0;
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
