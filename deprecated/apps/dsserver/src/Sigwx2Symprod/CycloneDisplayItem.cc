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
 * @file CycloneDisplayItem.cc
 *
 * @class CycloneDisplayItem
 *
 * Class representing a cyclone display item.
 *  
 * @date 10/21/2009
 *
 */

#include <math.h>

#include "CycloneDisplayItem.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

CycloneDisplayItem::CycloneDisplayItem(const double lat, const double lon,
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

CycloneDisplayItem::~CycloneDisplayItem()
{
}


/*********************************************************************
 * draw()
 */

void CycloneDisplayItem::draw(Symprod &symprod,
			      const BoundingBox &bbox,
			      const int icon_size) const
{
  static const string method_name = "CycloneDisplayItem::draw()";
  
  if (_debugLevel >= 1)
    cerr << method_name << " for cyclone" << endl;

  // Normalize the longitude to the bounding box
  
  double display_lon = _lon;
  
  while (display_lon < bbox.getMinLon())
    display_lon += 360.0;

  // Don't display the item if it isn't within the bounding box

  if (!bbox.isInterior(_lat, display_lon))
    return;
  
  int radius = icon_size / 2;
  
  // Add circle

  symprod.addArc(_lat,
		 display_lon,
		 radius,
		 radius,
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
		 Symprod::FILL_NONE,
		 0,
		 Symprod::DETAIL_LEVEL_DO_NOT_SCALE);

  int radsq2 = (int) (radius / sqrt(2));
  Symprod::ppt_t alinePts[2];
  Symprod::ppt_t blinePts[2];
  
  int line_len = (int) (1.5 * radius);
  if (_lat >= 0)
  {
    // if northern hemisphere
    // Line from NW part of circle, extending toward NE
    alinePts[0].x = -radsq2;
    alinePts[0].y = radsq2;
    alinePts[1].x = alinePts[0].x + line_len;
    alinePts[1].y = alinePts[0].y + line_len;

    // Line from SE part of circle, extending toward SW
    blinePts[0].x = radsq2;
    blinePts[0].y = -radsq2;
    blinePts[1].x = blinePts[0].x - line_len;
    blinePts[1].y = blinePts[0].y - line_len;
  }
  else
  {
    // southern hemisphere
    // Line from NE part of circle, extending toward NW
    alinePts[0].x = radsq2;
    alinePts[0].y = radsq2;
    alinePts[1].x = alinePts[0].x - line_len;
    alinePts[1].y = alinePts[0].y + line_len;

    // Line from SW part of circle, extending toward SE
    blinePts[0].x = -radsq2;
    blinePts[0].y = -radsq2;
    blinePts[1].x = blinePts[0].x + line_len;
    blinePts[1].y = blinePts[0].y - line_len;
  }

  // Add aline

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

  // Add bline

  symprod.addIconline(_lat,
		      display_lon,
		      2,
		      blinePts,
		      _lineColor.c_str(),
		      Symprod::LINETYPE_SOLID,
		      _lineWidth,
		      Symprod::CAPSTYLE_BUTT,
		      Symprod::JOINSTYLE_BEVEL,
		      false,
		      Symprod::FILL_NONE,
		      0,
		      Symprod::DETAIL_LEVEL_DO_NOT_SCALE);

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
