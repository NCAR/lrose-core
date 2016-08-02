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
 * @file BoundingBox.cc
 *
 * @class BoundingBox
 *
 * Class controlling access to a bounding box.
 *  
 * @date 10/19/2009
 *
 */

#include "BoundingBox.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

BoundingBox::BoundingBox(const double min_lat, const double max_lat,
			 const double min_lon, const double max_lon) :
  _minLat(min_lat),
  _maxLat(max_lat),
  _minLon(min_lon),
  _maxLon(max_lon)
{
}


/*********************************************************************
 * Destructor
 */

BoundingBox::~BoundingBox()
{
}


/*********************************************************************
 * getMaxDim()
 */

double BoundingBox::getMaxDim() const
{
  double lat_dim = _maxLat - _minLat;
  double lon_dim = _maxLon - _minLon;
  
  if (lat_dim > lon_dim)
    return lat_dim;
  
  return lon_dim;
}


/*********************************************************************
 * isInterior()
 */

bool BoundingBox::isInterior(double lat,
			     double lon) const
{
  if (lat > _minLat && lat < _maxLat &&
      lon > _minLon && lon < _maxLon)
    return true;

  return false;
}


/*********************************************************************
 * degLatToPixels()
 */

double BoundingBox::degLatToPixels(const double deg_lat,
				   const int screen_size) const
{
  return (deg_lat / (_maxLat - _minLat)) * (double)screen_size;
}


/*********************************************************************
 * degLonToPixels()
 */

double BoundingBox::degLonToPixels(const double deg_lon,
				   const int screen_size) const
{
  return (deg_lon / (_maxLon - _minLon)) * (double)screen_size;
}


/*********************************************************************
 * pixelsToDegLat()
 */

double BoundingBox::pixelsToDegLat(const int pixels,
				   const int screen_size) const
{
  return ((_maxLat - _minLat) / (double) screen_size) * (double)pixels;
}


/*********************************************************************
 * pixelsToDegLon()
 */

double BoundingBox::pixelsToDegLon(const int pixels,
				   const int screen_size) const
{
  return ((_maxLon - _minLon) / (double) screen_size) * (double)pixels;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
