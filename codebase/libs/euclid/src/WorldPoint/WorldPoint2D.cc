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
/*********************************************************************
 * WorldPoint2D.cc: class implementing a 2-dimensional world point.
 *                  This point describes a position on the earth
 *                  using simply latitude and longitude.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <string>
#include <vector>
#include <cstdio>

#include <euclid/WorldPoint2D.hh>

using namespace std;


/**********************************************************************
 * Constructors
 */

WorldPoint2D::WorldPoint2D(double lat, double lon)
{
  setPoint(lat, lon);
}


WorldPoint2D::WorldPoint2D(WorldPoint2D *point)
{
  setPoint(point);
}


/**********************************************************************
 * Destructor
 */

WorldPoint2D::~WorldPoint2D(void)
{
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 *              Friends                                               *
 **********************************************************************/

ostream& operator<< (ostream &os, const WorldPoint2D &point)
{
  return os << string("lat: ") << point.lat
	    << string(", lon: ") << point.lon;
}

ostream& operator<< (ostream &os, const WorldPoint2D *point)
{
  return os << string("lat: ") << point->lat
	    << string(", lon: ") << point->lon;
}

