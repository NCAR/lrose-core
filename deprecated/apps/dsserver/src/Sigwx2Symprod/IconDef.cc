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
//   $Date: 2016/03/04 02:29:42 $
//   $Id: IconDef.cc,v 1.3 2016/03/04 02:29:42 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * IconDef.cc : IconDef methods.  This class represents a stroked
 *              icon definition for a Symprod object.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2000
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <vector>

#include <toolsa/mem.h>

#include "IconDef.hh"

using namespace std;


// Global constants

const int IconDef::INPUT_PENUP = 32767;

/*********************************************************************
 * Constructors
 */

IconDef::IconDef() :
  _iconName("")
{
}


IconDef::IconDef(const string &icon_name,
		 vector< GridPoint > &icon_points) :
  _iconName(icon_name)
{
  // Create the array of points

  Symprod::ppt_t icon_point;
  
  vector< GridPoint >::iterator point;
  
  for (point = icon_points.begin(); point != icon_points.end(); ++point)
  {
    // Copy the X value

    if (point->x == INPUT_PENUP)
      icon_point.x = Symprod::PPT_PENUP;
    else
      icon_point.x = point->x;
    
    // Copy the Y value

    if (point->y == INPUT_PENUP)
      icon_point.y = Symprod::PPT_PENUP;
    else
      icon_point.y = point->y;

    // Add the new point to the list

    _points.push_back(icon_point);

  } /* endfor - point */
  
}


/*********************************************************************
 * Destructor
 */

IconDef::~IconDef()
{
}


/*********************************************************************
 * draw()
 */

void IconDef::draw(const string &color_to_use,
		   const int line_width,
		   const double center_lat,
		   const double center_lon,
		   const double icon_scale,
		   const bool allow_client_scaling,
		   Symprod *symprod) const
{
  static const string method_name = "IconDef::draw()";
  
  // Apply the scale factor and create the array of points
  // also get the icon width so we can offset the text correctly

  size_t num_pts = _points.size();
  Symprod::ppt_t *scaled_points = new Symprod::ppt_t[num_pts];
  
  for (size_t ii = 0; ii < num_pts; ++ii)
  {
    // Copy the original icon point

    scaled_points[ii].x = _points[ii].x;
    scaled_points[ii].y = _points[ii].y;

    // Apply the scale factor
      
    if (scaled_points[ii].x != Symprod::PPT_PENUP &&
	scaled_points[ii].y != Symprod::PPT_PENUP)
    {
      scaled_points[ii].x = (int)((double)scaled_points[ii].x * icon_scale);
      scaled_points[ii].y = (int)((double)scaled_points[ii].y * icon_scale);
    }

  } /* endfor - ii */
	 
  Symprod::wpt_t icon_origin;
  icon_origin.lat = center_lat;
  icon_origin.lon = center_lon;
  
  Symprod::detail_level_t detail_level = Symprod::DETAIL_LEVEL_NONE;
  if (!allow_client_scaling)
    detail_level = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;

  symprod->addStrokedIcons(color_to_use.c_str(),
			   num_pts,
			   scaled_points,
			   1,
			   &icon_origin,
			   0,
			   detail_level,
			   line_width);

  delete [] scaled_points;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
