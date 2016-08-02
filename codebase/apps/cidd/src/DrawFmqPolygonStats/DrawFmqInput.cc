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
 * @file DrawFmqInput.cc
 *
 * @class DrawFmqInput
 *
 * Base class for input handlers.
 *  
 * @date 3/13/2009
 *
 */

#include <string.h>

#include "DrawFmqInput.hh"

using namespace std;


/*********************************************************************
 * Constructor
 */

DrawFmqInput::DrawFmqInput(const bool apply_polygon_to_all_elevations,
			   const bool debug_flag) :
  Input(debug_flag),
  _applyPolygonToAllElevations(apply_polygon_to_all_elevations)
{
}


/*********************************************************************
 * Destructor
 */

DrawFmqInput::~DrawFmqInput()
{
}


/*********************************************************************
 * init()
 */

bool DrawFmqInput::init(const string &prog_name,
			const string &cidd_draw_fmq)
{
  static const string method_name = "DrawFmqInput::init()";
  
  // Initialize the CIDD drawing FMQ for input.

  if (_inputQueue.initReadBlocking(cidd_draw_fmq.c_str(),
				   prog_name.c_str(),
				   _debug) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error initializing CIDD draw queue for reading: "
	 << cidd_draw_fmq << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * endOfInput()
 */

bool DrawFmqInput::endOfInput()
{
  return false;
}


/*********************************************************************
 * getNextProd()
 */

bool DrawFmqInput::getNextProd(Human_Drawn_Data_t &input_prod)
{
  static const string method_name = "DrawFmqInput::getNextProd()";
  
  // Get the next product from the queue

  int status = 0;
  Human_Drawn_Data_t local_input_prod = _inputQueue.nextProduct(status);
    
  if (status != 0)
  {
    if (_debug)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading product from CIDD queue, status = " << status << endl;
    }
    
    sleep(1);
      
    return false;
  }
    
  // Make a copy of the product to return to the caller so the caller can
  // reclaim the space when finished.

  input_prod = local_input_prod;
  input_prod.lat_points = new double[local_input_prod.num_points];
  input_prod.lon_points = new double[local_input_prod.num_points];
  memcpy(input_prod.lat_points, local_input_prod.lat_points,
	 local_input_prod.num_points * sizeof(double));
  memcpy(input_prod.lon_points, local_input_prod.lon_points,
	 local_input_prod.num_points * sizeof(double));
  
  // If this polygon should be applied to all elevation angles, set the
  // elevation angle in the message to -1.0

  if (_applyPolygonToAllElevations)
  {
    input_prod.vlevel_num = -1;
    input_prod.vlevel_ht_min = -1.0;
    input_prod.vlevel_ht_cent = -1.0;
    input_prod.vlevel_ht_max = -1.0;
  }
  
  return true;
}

