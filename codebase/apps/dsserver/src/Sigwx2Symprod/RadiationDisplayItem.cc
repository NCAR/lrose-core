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
 * @file RadiationDisplayItem.cc
 *
 * @class RadiationDisplayItem
 *
 * Class representing a radiation display item.
 *  
 * @date 10/21/2009
 *
 */

#include <math.h>
#include <stdlib.h>

#include "RadiationDisplayItem.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

RadiationDisplayItem::RadiationDisplayItem(const double lat, const double lon,
				       const string &name,
				       const string &line_color,
				       const int line_width,
				       const string &text_color,
				       const string &text_bg_color,
				       const string &font_name,
				       const int font_size,
				       const int debug_level) :
  DisplayItem(TYPE_CYCLONE, lat, lon, debug_level),
  _name(name),
  _lineColor(line_color),
  _lineWidth(line_width),
  _textColor(text_color),
  _textBgColor(text_bg_color),
  _fontName(font_name),
  _fontSize(font_size)
{
}


/*********************************************************************
 * Destructor
 */

RadiationDisplayItem::~RadiationDisplayItem()
{
}


/*********************************************************************
 * draw()
 */

void RadiationDisplayItem::draw(Symprod &symprod,
			      const BoundingBox &bbox,
			      const int icon_size) const
{
  static const string method_name = "RadiationDisplayItem::draw()";
  
  if (_debugLevel >= 1)
    cerr << method_name << " for cyclone" << endl;

  // Normalize the longitude to the bounding box

  double display_lon = _lon;
  
  while (display_lon < bbox.getMinLon())
    display_lon += 360.0;

  // Don't display the item if it isn't within the bounding box

  if (!bbox.isInterior(_lat, display_lon))
    return;
  
  Symprod::wpt_t wpt;
  wpt.lat = _lat;
  wpt.lon = display_lon;

  int radius = icon_size / 2;

  double radiusA = (int) (0.2 * radius);
  double radiusB = (int) (0.4 * radius);
  double cosVal = cos( M_PI / 3.0);        // cos(60)
  double tanVal = tan( M_PI / 3.0);        // tan(60)
  double sinHalfVal = sin( M_PI / 6.0);    // sin(30)
  double tanHalfVal = tan( M_PI / 6.0);    // tan(30)
  double badVal = -9999;

  // Add small inner circle

  symprod.addArc(_lat,
		 display_lon,
		 radiusA,
		 radiusA,
		 _lineColor.c_str(),
		 true,
		 0.0,
		 360.0,
		 0.0,
		 50,
		 Symprod::LINETYPE_SOLID,
		 _lineWidth,
		 Symprod::CAPSTYLE_BUTT,
		 Symprod::JOINSTYLE_BEVEL,
		 Symprod::FILL_SOLID,
		 0,
		 Symprod::DETAIL_LEVEL_DO_NOT_SCALE);

  // Add upper right wedge

  for (int ii = (int)(cosVal*radiusB); ii <= radius; ii++)
  {
    double ya, yb;
    if (ii >= cosVal * radiusB && ii <= radiusB)
    {
      ya = sqrt( radiusB*radiusB - ii*ii);
      yb = tanVal * ii;
    }
    else if (ii >= radiusB && ii <= cosVal*radius)
    {
      ya = 0;
      yb = tanVal * ii;
    }
    else if (ii >= cosVal*radius && ii <= radius)
    {
      ya = 0;
      yb = sqrt( radius*radius - ii*ii);
    }
    else
    {
      ya = badVal;
    }
    
    if (ya != badVal)
    {
      Symprod::ppt_t alinePts[2];
      alinePts[0].x = ii;
      alinePts[0].y = (int) ya;
      alinePts[1].x = ii;
      alinePts[1].y = (int) yb;

      // Add line

      symprod.addIconline(_lat,
			  display_lon,
			  2,
			  alinePts,
			  _lineColor.c_str(),
			  Symprod::LINETYPE_SOLID,
			  _lineWidth,
			  Symprod::CAPSTYLE_BUTT,
			  Symprod::JOINSTYLE_BEVEL,
			  false,
			  Symprod::FILL_NONE,
			  0,
			  Symprod::DETAIL_LEVEL_DO_NOT_SCALE);
    }
  }

  // Add upper left wedge

  for (int ii = -(int)(cosVal*radiusB); ii >= -radius; ii--)
  {
    double ya, yb;
    if (ii <= -cosVal * radiusB && ii >= -radiusB)
    {
      ya = sqrt( radiusB*radiusB - ii*ii);
      yb = fabs(tanVal * ii);
    }
    else if (ii <= -radiusB && ii >= -cosVal*radius)
    {
      ya = 0;
      yb = fabs(tanVal * ii);
    }
    else if (ii <= -cosVal*radius && ii >= -radius)
    {
      ya = 0;
      yb = sqrt( radius*radius - ii*ii);
    }
    else
    {
      ya = badVal;
    }

    if (ya != badVal)
    {
      Symprod::ppt_t alinePts[2];
      alinePts[0].x = ii;
      alinePts[0].y = (int) ya;
      alinePts[1].x = ii;
      alinePts[1].y = (int) yb;

      // Add line

      symprod.addIconline(_lat,
			  display_lon,
			  2,
			  alinePts,
			  _lineColor.c_str(),
			  Symprod::LINETYPE_SOLID,
			  _lineWidth,
			  Symprod::CAPSTYLE_BUTT,
			  Symprod::JOINSTYLE_BEVEL,
			  false,
			  Symprod::FILL_NONE,
			  0,
			  Symprod::DETAIL_LEVEL_DO_NOT_SCALE);
    }
  }

  // Add bottom wedge

  for (int ii = -(int)(sinHalfVal*radius);
       ii <= (int)(sinHalfVal*radius);
       ii++)
  {
    double ya, yb;
    if (ii <= -sinHalfVal * radiusB || ii >= sinHalfVal * radiusB)
    {
      ya = -abs(ii) / tanHalfVal;
      yb = -sqrt( radius*radius - ii*ii);
    }
    else
    {
      ya = -sqrt( radiusB*radiusB - ii*ii);
      yb = -sqrt( radius*radius - ii*ii);
    }

    Symprod::ppt_t alinePts[2];
    alinePts[0].x = ii;
    alinePts[0].y = (int) ya;
    alinePts[1].x = ii;
    alinePts[1].y = (int) yb;

    // Add line

    symprod.addIconline(_lat,
			display_lon,
			2,
			alinePts,
			_lineColor.c_str(),
			Symprod::LINETYPE_SOLID,
			_lineWidth,
			Symprod::CAPSTYLE_BUTT,
			Symprod::JOINSTYLE_BEVEL,
			false,
			Symprod::FILL_NONE,
			0,
			Symprod::DETAIL_LEVEL_DO_NOT_SCALE);
  }

  // Add name

  symprod.addText(_name.c_str(),
		  _lat,
		  display_lon,
		  _textColor.c_str(),
		  _textBgColor.c_str(),
		  15,
		  0,
		  Symprod::VERT_ALIGN_CENTER,
		  Symprod::HORIZ_ALIGN_LEFT,
		  _fontSize,
		  Symprod::TEXT_NORM,
		  _fontName.c_str());

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
