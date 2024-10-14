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

#include "cidd.h"
#include <dsserver/DmapAccess.hh>

#include "RenderContext.hh"
#include "Product.hh"
#include "SymprodRenderObj.hh"


///////////////////////////////////////////////////////////////
// class definition

class ProductMgr

{

public:

  // default constructor
  
  ProductMgr(RenderContext &context, int debug = 0);
  
  // destructor
  
  virtual ~ProductMgr();

  // Get data from each product marked as having invalid data 
  int getData(const time_t start_time,
	      const time_t end_time);
  
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

  // Set the data times valid state for one product
  void set_product_times_valid(int product, int valid);

  // Get the validity flag for the indicated product
  int get_product_data_valid(int product);
  
  // Reset all times valid flags to invalid
  void reset_times_valid_flags(void);

  // Render all products using the current context
  void draw(void);
  
  SymprodRenderObj *get_closest_Rob() { return _active_Rob; }

  void draw_pick_obj(void); // Using the XOR GC

  void draw_pick_obj_plain(void); // Without changing to XOR GC.

  // Returns the number of products marked as invalid.
  int num_products_invalid(void);

  // Returns the number of products its supervising
  int get_num_products(void);

  void check_product_validity(time_t tm, DmapAccess &dmap);  // Check datamapper info for updated data

  void pick_closest_obj(double lat, double lon);

  void fill_export_fields(vector<world_pt_t> &wpt, string &label);
  
  const vector<Product *> getProducts() const { return _products; }

  const Product *getProduct(int index) const {
    cerr << "iiiiiiiiiii index, _products.size(): " << index << ", " << _products.size() << endl;
    if (index < (int) _products.size()) {
      return _products[index];
    } else {
      return NULL;
    }
  }

protected:
  
  RenderContext &_context; // Drawing context for window 
  
  vector<Product *> _products;

  SymprodRenderObj *_active_Rob;
  
private:

};

#endif


