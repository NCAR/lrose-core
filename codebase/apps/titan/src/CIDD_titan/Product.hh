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
// Product.hh
//
// Single product data retrieval and rendering
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2000
//
////////////////////////////////////////////////////////////////

#ifndef Product_HH
#define Product_HH

#include "Csyprod_P.hh"
#include <Spdb/DsSpdbThreaded.hh>
#include "SymprodRender.hh"
#include "cidd.h"

///////////////////////////////////////////////////////////////
// class definition

class Product

{

public:

  // default constructor
  
  Product(int debug, Csyprod_P::prod_info_t &prodInfo);
  
  // destructor
  
  virtual ~Product();

  // clear vectors
  
  void clearSymprods();
  void clearTimes();

  // set members
  
  void setDebug(bool debug = true) { _debug = debug; }

  int getData(const time_t data_start_time,
	      const time_t data_end_time,
	      RenderContext &context);

  DsSpdbThreaded *getSpdbObj(void) { return &_spdb;}
  DsSpdbThreaded *getSpdbTimesObj(void) { return &_spdbTimes;}

  void setThreadingOff(void) {
    _spdb.setThreadingOff();
    _spdbTimes.setThreadingOff();
  }

  bool processChunks();
  bool processTimeRefs();

  int _active;

  int _data_valid;
  int _times_valid;

  time_t _last_collected;
  
  // draw
  void draw(RenderContext &context);

  // Returns closest distance.
  double pick_closest_obj(double lat, double lon, RenderContext &context);

  SymprodRenderObj * get_closest_symprod_obj() {return closest_symprod_obj; }

  Csyprod_P::prod_info_t &_prodInfo;

  // List of Object props

  vector<Symprod::prod_hdr_props_t>  obj_list;  

protected:
  
  bool _debug;

  DsSpdbThreaded _spdb;

  vector<SymprodRender *> _symprods;

  SymprodRenderObj *closest_symprod_obj;

  double closest_dist;

  DsSpdbThreaded _spdbTimes;
  vector<time_t> _times;
  time_t _prev_epoch_start;
  time_t _prev_epoch_end;
  bool _chunksNeedProcess;
  bool _timesNeedProcess;
  
private:

};

#endif


