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
// SymprodRender.hh
//
// Rendering for Symprod class
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2000
//
////////////////////////////////////////////////////////////////

#ifndef SymprodRender_HH
#define SymprodRender_HH

#include <Spdb/Symprod.hh>
#include <Spdb/DsSpdb.hh>
#include "RenderContext.hh"
#include "Product.hh"

#include <dataport/bigend.h>
#include "cidd.h"

class SymprodRenderObj;

///////////////////////////////////////////////////////////////
// class definition

class SymprodRender : public Symprod

{

public:

  // default constructor
  
  SymprodRender(Product &p);

  // destructor
  
  virtual ~SymprodRender();

  // clear products

  void clear();
  
  // Convert a product buffer to internal format.  The buffer is assumed
  // to be in native format (any necessary byte - swapping has already
  // been done).
  // Checks on buffer overrun if buf_len is provided.
  // returns 0 on success, -1 on failure.
  
  int deserialize(void *in_buf, int buf_len = -1);

  // draw
  
  void draw(RenderContext &context);

  double pick_closest_obj(double lat, double lon);

  SymprodRenderObj * get_closest_obj() {return closest_obj;}

  void fill_export_fields(vector<world_pt_t> &wpt, string &label);
  
  int _objDataType;

protected:

  Product &prod; 

  vector<SymprodRenderObj *> _objs;

  void _freeObjs();

  SymprodRenderObj *closest_obj;
  
private:

};

#endif


