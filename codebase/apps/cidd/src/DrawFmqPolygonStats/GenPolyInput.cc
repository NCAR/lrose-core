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
/**
 *
 * @file GenPolyInput.cc
 *
 * @class GenPolyInput
 *
 * Base class for input handlers.
 *  
 * @date 3/13/2009
 *
 */

#include <Spdb/Product_defines.hh>

#include "GenPolyInput.hh"

using namespace std;


/*********************************************************************
 * Constructor
 */

GenPolyInput::GenPolyInput(const bool debug_flag) :
  Input(debug_flag)
{
}


/*********************************************************************
 * Destructor
 */

GenPolyInput::~GenPolyInput()
{
}


/*********************************************************************
 * init()
 */

bool GenPolyInput::init(const string &gen_poly_url,
			const DateTime &start_time,
			const DateTime &end_time)
{
  static const string method_name = "GenPolyInput::init()";
  
  // Make sure we are using a GenPoly database

  _spdb.setProdId(SPDB_GENERIC_POLYLINE_ID);
  
  // Get the data from the database

  if (_spdb.getInterval(gen_poly_url.c_str(),
			start_time.utime(),
			end_time.utime()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving polygons from SPDB database" << endl;
    cerr << "URL = " << gen_poly_url << endl;
    cerr << _spdb.getErrStr() << endl;
    
    return false;
  }
  
  _spdbChunks = _spdb.getChunks();
  _currChunk = _spdbChunks.begin();
  
  return true;
}


/*********************************************************************
 * endOfInput()
 */

bool GenPolyInput::endOfInput()
{
  return _currChunk == _spdbChunks.end();
}


/*********************************************************************
 * getNextProd()
 */

bool GenPolyInput::getNextProd(Human_Drawn_Data_t &input_prod)
{
  static const string method_name = "GenPolyInput::getNextProd()";
  
  // Get the GenPolyStats information from the current chunk

  GenPolyStats polygon;
  
  if (!polygon.disassemble(_currChunk->data, _currChunk->len))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error disassembling polygon information from database" << endl;
    cerr << "Skipping chunk" << endl;
    
    ++_currChunk;
    
    return false;
  }
  
  int valid_secs = _currChunk->expire_time - _currChunk->valid_time;
  
  ++_currChunk;
  
  // Set the values in the input product

  input_prod.issueTime = polygon.getTime();
  input_prod.data_time = polygon.getTime();
  input_prod.id_no = polygon.getId();
//  input_prod.valid_seconds = polygon.getExpireTime() - polygon.getTime();
  input_prod.valid_seconds = valid_secs;
  input_prod.num_points = polygon.getNumVertices();
  input_prod.vlevel_num = (int)polygon.getVlevelIndex();
  input_prod.vlevel_ht_min = polygon.getElevAngle();
  input_prod.vlevel_ht_cent = input_prod.vlevel_ht_min;
  input_prod.vlevel_ht_max = input_prod.vlevel_ht_min;
  input_prod.id_label = polygon.getName();
  input_prod.prod_label = polygon.getName();
  input_prod.sender = "DrawFmqPolygonStats";
  input_prod.lat_points = new double[input_prod.num_points];
  input_prod.lon_points = new double[input_prod.num_points];
  
  for (int i = 0; i < input_prod.num_points; ++i)
  {
    GenPoly::vertex_t vertex = polygon.getVertex(i);
    
    input_prod.lat_points[i] = vertex.lat;
    input_prod.lon_points[i] = vertex.lon;
  }
  
  return true;
}

