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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 01:23:06 $
//   $Id: LatlonFieldProcessor.cc,v 1.5 2016/03/07 01:23:06 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * LatlonFieldProcessor : Class the converts TeraScan data to a lat/lon
 *                        projection Mdv field.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <math.h>

#include "LatlonFieldProcessor.hh"

using namespace std;


/*********************************************************************
 * Constructor
 */

LatlonFieldProcessor::LatlonFieldProcessor(SETP input_dataset) :
  FieldProcessor(input_dataset)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

LatlonFieldProcessor::~LatlonFieldProcessor()
{
  // Do nothing
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _calcIndex() - Calculate the MDV field index for the given satellite
 *                field location.
 */

int LatlonFieldProcessor::_calcIndex(const int x_index, const int y_index)
{
  return x_index * _numSamples + y_index;
}


/*********************************************************************
 * _initLocal() - Do any initialization needed locally by the derived
 *                class.  This method is called at the end of the
 *                FieldProcessor::_init() method.
 */

bool LatlonFieldProcessor::_initLocal(ETXFORM mxfm)
{
  // Calculate delta lat and delta lon by comparing the location of the
  // point diagonally adjacent to the lower left corner point with
  // the location of the lower left corner.  The lower left corner
  // location is calculated in the base class before this method is
  // called.

  double adjacent_lat, adjacent_lon;
  
  etxll(mxfm, _numLines - 1, 2, &adjacent_lat, &adjacent_lon);

  _deltaLat = fabs(adjacent_lat - _lowerLeftLat);
  _deltaLon = fabs(_lowerLeftLon - adjacent_lon);
  
  return true;
}


/*********************************************************************
 * _setProjectionInfo() - Set the projection information in the given
 *                        field header.
 */

void LatlonFieldProcessor::_setProjectionInfo(Mdvx::field_header_t &fld_hdr)
{
  fld_hdr.proj_type = Mdvx::PROJ_LATLON;

  fld_hdr.grid_dy = _deltaLat;
  fld_hdr.grid_dx = _deltaLon;
  fld_hdr.grid_minx = _lowerLeftLon;
  fld_hdr.grid_miny = _lowerLeftLat;
}
