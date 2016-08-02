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
// Product rendering
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2000
//
////////////////////////////////////////////////////////////////

#ifndef ProductMgr_HH
#define ProductMgr_HH

#include "ProdParams.hh"
#include "RenderContext.hh"
using namespace std;

class Product;

///////////////////////////////////////////////////////////////
// class definition

class ProductMgr

{

public:

  // default constructor
  
  ProductMgr(const ProdParams &params,
	     Display *display);
  
  // destructor
  
  virtual ~ProductMgr();

  // set the icon scale

  void setIconScale(double scale) { _context.iconScale = scale; }
  
  // get the data in realtime mode

  int getDataRealtime(const time_t frame_time,
		      const time_t data_time);

  // get the data in archive mode

  int getDataArchive(const time_t frame_time,
		     const time_t data_time);
  
  // get data in an interval
  
  int getDataInterval(const time_t start_time,
		      const time_t end_time);
  
  // draw
  
  void draw(Colormap cmap,
	    x_color_list_index_t *colorIndex,
	    int dev,
	    const gframe_t *frame,
	    const MdvxProj &proj);
  
protected:
  
  const ProdParams &_params;
  RenderContext _context;
  vector<Product *> _products;
  
private:

};

#endif


