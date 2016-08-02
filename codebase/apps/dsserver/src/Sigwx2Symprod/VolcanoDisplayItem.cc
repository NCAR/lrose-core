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
 * @file VolcanoDisplayItem.cc
 *
 * @class VolcanoDisplayItem
 *
 * Class representing a volcano display item.
 *  
 * @date 10/21/2009
 *
 */

#include <math.h>

#include "VolcanoDisplayItem.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

VolcanoDisplayItem::VolcanoDisplayItem(const double lat, const double lon,
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

VolcanoDisplayItem::~VolcanoDisplayItem()
{
}


/*********************************************************************
 * draw()
 */

void VolcanoDisplayItem::draw(Symprod &symprod,
			      const BoundingBox &bbox,
			      const int icon_size) const
{
  static const string method_name = "VolcanoDisplayItem::draw()";
  
  if (_debugLevel >= 1)
    cerr << method_name << " for volcano" << endl;

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
  int width = (int) (0.5 * radius);
  int height = (int) (0.2 * radius);

  Symprod::ppt_t volcanoIcon[12] =
    {
      { -radius,        -radius },     // bottom left corner
      {  radius,        -radius },     // base, from left to right
      {  width,         height },      // right side
      {  0,             height },      // to top center
      {  width,         radius },      // 3 lines of volcanic ejecta
      {  0,             height },
      {  0,             radius },
      {  0,             height },
      {  -width,        radius },
      {  0,             height },      // top center
      {  -width,        height },      // to top left corner
      {  -radius,       -radius },     // to bottom left corner
    };

  symprod.addStrokedIcons(_lineColor.c_str(),
			  12,
			  volcanoIcon,
			  1,
			  &wpt,
			  0,
			  Symprod::DETAIL_LEVEL_DO_NOT_SCALE,
			  _lineWidth);

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
