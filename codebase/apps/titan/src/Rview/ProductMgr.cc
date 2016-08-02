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
// RAP, NCAR, Boulder, CO, 80307, USA
//
// Jan 2000
//
//////////////////////////////////////////////////////////

#include "ProductMgr.hh"
#include "Product.hh"
using namespace std;

//////////////////////////////////////////
// default constructor

ProductMgr::ProductMgr(const ProdParams &params,
		       Display *display)
  : _params(params)
  
{
  
  for (int i = 0; i < _params.prod_info_n; i++) {
    
    Product *product = new Product(display);
    if (_params.debug) {
      product->setDebug(true);
    }
    product->setProdInfo(_params._prod_info[i]);
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
// get data in realtime mode
//
// returns 0 on success, -1 on failure

int ProductMgr::getDataRealtime(time_t frame_time,
				time_t data_time)
  
{
 
  int iret = 0;

  // get the data
  
  for (size_t i = 0; i < _products.size(); i++) {
    if (_products[i]->getDataRealtime(frame_time, data_time)) {
      iret = -1;
    }
  }

  return (iret);

}

//////////////////////////////////////////////////
// get data in archive mode
//
// returns 0 on success, -1 on failure

int ProductMgr::getDataArchive(time_t frame_time,
			       time_t data_time)
  
{
  
  int iret = 0;

  // get the data
  
  for (size_t i = 0; i < _products.size(); i++) {
    if (_products[i]->getDataArchive(frame_time, data_time)) {
      iret = -1;
    }
  }

  return (iret);

}

//////////////////////////////////////////////////
// get data in interval mode
//
// returns 0 on success, -1 on failure

int ProductMgr::getDataInterval(time_t start_time,
				time_t end_time)
  
{
  
  int iret = 0;

  // get the data
  
  for (size_t i = 0; i < _products.size(); i++) {
    if (_products[i]->getDataInterval(start_time, end_time)) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////
// draw the products
//
// returns 0 on success, -1 on failure

void ProductMgr::draw(Colormap cmap,
		      x_color_list_index_t *colorIndex,
		      int dev,
		      const gframe_t *frame,
		      const MdvxProj &proj)
  
{
  
  // set context

  _context.cmap = cmap;
  _context.colorIndex = colorIndex;
  _context.dev = dev;
  _context.frame = *frame;
  _context.psgc = *frame->psgc;
  _context.proj = proj;
  
  for (size_t i = 0; i < _products.size(); i++) {
    _products[i]->draw(&_context);
  }
  
}

