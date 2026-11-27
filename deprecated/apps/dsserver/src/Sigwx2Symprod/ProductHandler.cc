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
 * @file ProductHandler.cc
 *
 * @class ProductHandler
 *
 * Base class for product handlers.
 *  
 * @date 10/10/2009
 *
 */

#include <stdarg.h>

#include "ProductHandler.hh"
#include "Server.hh"

using namespace std;


const string ProductHandler::colorBlue    = "blue";
const string ProductHandler::colorGreen   = "green";
const string ProductHandler::colorMagenta = "magenta";
const string ProductHandler::colorPink    = "pink";
const string ProductHandler::colorPurple  = "purple";
const string ProductHandler::colorRed     = "red";


/*********************************************************************
 * Constructors
 */

ProductHandler::ProductHandler(Params *params,
			       const int debug_level) :
  _debugLevel(debug_level),
  _params(params)
{
}


/*********************************************************************
 * Destructor
 */

ProductHandler::~ProductHandler()
{
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _findFirstInBox()
 */

int ProductHandler::_findFirstInBox(const vector< SigwxPoint > &points,
				    const BoundingBox &bbox) const
{
  for (size_t ipt = 0; ipt < points.size(); ipt++)
  {
    double lat = points[ipt].lat;
    double lon = points[ipt].lon;
    
    if (bbox.isInterior(lat, lon) ||
	bbox.isInterior(lat, lon - 360.0) ||
	bbox.isInterior(lat, lon + 360.0))
      return ipt;
  }

  return -1;
}


/*********************************************************************
 * _adjustLongitudes()
 */

bool ProductHandler::_adjustLongitudes(Symprod::wpt_t *waypoints,
				       const int numPts,
				       const int ifirst,
				       const BoundingBox &bbox) const
{
  static const string method_name = "ProductHandler::_adjustLongitudes()";
  
  // First do ifirst

  double tlon = waypoints[ifirst].lon;

  if (tlon - 360 >= bbox.getMinLon() && tlon - 360 <= bbox.getMaxLon())
  {
    waypoints[ifirst].lon -= 360.0;
  }
  else if (tlon + 360 >= bbox.getMinLon() && tlon + 360 <= bbox.getMaxLon())
  {
    waypoints[ifirst].lon += 360.0;
  }

  // Do the points before ifirst

  double prev_lon = waypoints[ifirst].lon;
  for (int ipt = ifirst-1; ipt >= 0; ipt--)
  {
    _setWaypointDelta(waypoints[ipt], prev_lon, bbox);
    prev_lon = waypoints[ipt].lon;
  }
     
  // Do the points after ifirst

  prev_lon = waypoints[ifirst].lon;
  for (int ipt = ifirst+1; ipt < numPts; ipt++)
  {
    _setWaypointDelta(waypoints[ipt], prev_lon, bbox);
    prev_lon = waypoints[ipt].lon;
  }

  if (_debugLevel >= 1)
  {
    cerr << method_name << endl;
    cerr << "  ifirst: " << ifirst << endl;
    cerr << "  minLat: " << bbox.getMinLat() << endl;
    cerr << "  maxLat: " << bbox.getMaxLat() << endl;
    cerr << "  minLon: " << bbox.getMinLon() << endl;
    cerr << "  maxLon: " << bbox.getMaxLon() << endl;
  }

  return true;
}


//////////////////////// UPDATE /////////////////////////////
// Move this to IconDef
//

/*********************************************************************
 * _drawIcon()
 */

void ProductHandler::_drawIcon(const string &icon_name,
			       const map< string, IconDef > &icon_def_list,
			       const string &color_to_use,
			       const int line_width,
			       const double center_lat,
			       const double center_lon,
			       const double icon_scale,
			       const bool allow_client_scaling,
			       Symprod *symprod) const
{
  static const string method_name = "ProductHandler::_drawIcon()";
  
  map< string, IconDef >::const_iterator icon_iter;
  
  if ((icon_iter = icon_def_list.find(icon_name)) == icon_def_list.end())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Icon <" << icon_name <<
      "> not found in icon list created from parameter file" << endl;
    cerr << "Not rendering icon" << endl;
    cerr << "Defined icons:" << endl;

    map< string, IconDef >::const_iterator icon_def;
    for (icon_def = icon_def_list.begin(); icon_def != icon_def_list.end();
	 ++icon_def)
    {
      cerr << "   " << icon_def->first << endl;
    }
    
    return;
  }
    
  (icon_iter->second).draw(color_to_use, line_width, center_lat, center_lon,
			   icon_scale, allow_client_scaling, symprod);
}


/*********************************************************************
 * _getWaypoints()
 */

Symprod::wpt_t *ProductHandler::_getWaypoints(const vector< SigwxPoint > &points,
					      const BoundingBox &bbox,
					      size_t &num_pts) const
{
  static const string method_name = "ProductHandler::_getWaypoints()";
  
  num_pts = points.size();

  // If there aren't any points then we don't need to render anything.

  if (num_pts == 0)
    return 0;
  
  // Find the first point that's inside our bounding box

  int ifirst = _findFirstInBox(points, bbox);
  if (_debugLevel >= 1)
    cerr << method_name << ": ifirst: " << ifirst << endl;

  // If none of the points are inside our bounding box then we don't need to
  // render anything.

  if (ifirst < 0)
    return 0;
  
  Symprod::wpt_t *waypoints = new Symprod::wpt_t[num_pts];
  for (size_t i = 0; i < num_pts; ++i)
  {
    waypoints[i].lat = points[i].lat;
    waypoints[i].lon = points[i].lon;
  }
  
  // Adjust longitudes to be inside our bounding box.
  // Replace each point outside the box with a point
  // on the box edge.
  // Start at ifirst and work both down and up,
  // so we handle all longitude wrapping in a
  // consistent manner.

  if (!_adjustLongitudes(waypoints, num_pts, ifirst, bbox))
  {
    delete [] waypoints;
    return false;
  }
  

  return waypoints;
}


/*********************************************************************
 * _setWaypoint()
 */

void ProductHandler::_setWaypoint(const SigwxPoint &point,
				  Symprod::wpt_t * waypoint,
				  double prevLon,
				  const BoundingBox &bbox) const
{
  double tlat = point.lat;
  double tlon = point.lon;
  double tlonMinus = tlon - 360;
  double tlonPlus = tlon + 360;

  if (tlat < bbox.getMinLat())
    waypoint->lat = bbox.getMinLat();
  else if (tlat > bbox.getMaxLat())
    waypoint->lat = bbox.getMaxLat();
  else
    waypoint->lat = tlat;

  if (tlon >= bbox.getMinLon() && tlon <= bbox.getMaxLon())
    waypoint->lon = tlon;
  else if (tlonMinus >= bbox.getMinLon() && tlonMinus <= bbox.getMaxLon())
    waypoint->lon = tlonMinus;
  else if (tlonPlus >= bbox.getMinLon() && tlonPlus <= bbox.getMaxLon())
    waypoint->lon = tlonPlus;
  else
  {
    // tlon is out of bounds.
    // Put it on the boundary closest to the previous point.
    double boundDeltaLow = fabs( prevLon - bbox.getMinLon());
    double boundDeltaHigh = fabs( prevLon - bbox.getMaxLon());
    if (boundDeltaLow <= boundDeltaHigh)
      waypoint->lon = bbox.getMinLon();
    else
      waypoint->lon = bbox.getMaxLon();
  }

}


/*********************************************************************
 * _setWaypointDelta()
 */

void ProductHandler::_setWaypointDelta(Symprod::wpt_t &waypoint,
				       const double prev_lon,
				       const BoundingBox &bbox) const
{
  double tlat = waypoint.lat;
  double tlon = waypoint.lon;

  // Adjust the point latitude to be within the box

  if (tlat < bbox.getMinLat())
    waypoint.lat = bbox.getMinLat();
  else if (tlat > bbox.getMaxLat())
    waypoint.lat = bbox.getMaxLat();
  else
    waypoint.lat = tlat;

  // Now adjust the longitude value based on the longitude of the point
  // next to this one.  I'm not sure why we are using 30.0 degrees here,
  // but I don't have time to test changes to this.  This means that some
  // points could fall through here unexpectedly, but I'm guessing this is
  // unlikely.

  double max_diff = 30.0;
  if (fabs(tlon - prev_lon) < max_diff)
  {
    // Do nothing
  }
  else if (fabs( tlon + 360 - prev_lon) < max_diff)
  {
    tlon += 360;
  }
  else if (fabs( tlon - 360 - prev_lon) < max_diff)
  {
    tlon -= 360;
  }
  
  waypoint.lon = tlon;

}


/*********************************************************************
 * _addTextToSymprod()
 */

void ProductHandler::_addTextToSymprod(Symprod *symprod,
				       const double txtLat,
				       const double txtLon,
				       const vector< string > &msgs,
				       const vector< bool > &hiddens,
				       const string &text_color,
				       const string &text_bg_color
				       ) const
{
  int xoffset = _params->text_x_offset;
  double yoffset = _params->text_y_offset;
  int fontSize = _params->font_size;
  double lineSpaceFactor = _params->line_space_factor;

  int text_detail_level;

  vector< string >::const_iterator msg;
  vector< bool >::const_iterator hiddenIx;

  for (msg = msgs.begin(), hiddenIx = hiddens.begin(); msg != msgs.end(); 
       ++msg, ++hiddenIx)
  {

    if (*hiddenIx)
      {
	text_detail_level =
	  Symprod::DETAIL_LEVEL_USUALLY_HIDDEN |
	  Symprod::DETAIL_LEVEL_SHOWAS_POPOVER |
	  Symprod::DETAIL_LEVEL_IGNORE_TRESHOLDS;
      }
    else
      text_detail_level = Symprod::DETAIL_LEVEL_NONE;

    symprod->addText((*msg).c_str(),
		     txtLat,
		     txtLon,
		     text_color.c_str(),
		     text_bg_color.c_str(),
		     xoffset,
		     (int)yoffset,
		     Symprod::VERT_ALIGN_CENTER,
		     Symprod::HORIZ_ALIGN_LEFT,
		     fontSize,
		     Symprod::TEXT_NORM,
		     _params->font_name,
		     0,
		     text_detail_level);

    yoffset -= lineSpaceFactor * fontSize;
  } /* endfor - msgs */

}


/*********************************************************************
 * _printEntryInfo()
 */

void ProductHandler::_printEntryInfo(const string &funcLabel,
				     const string &dir_path,
				     const int prod_id,
				     const string &prod_label,
				     const Spdb::chunk_ref_t &chunk_ref,
				     const void *spdb_data,
				     const int spdb_len,
				     const BoundingBox &bbox) const
{
  static const string method_name = "ProductHandler::_printEntryInfo()";
  
  string dateFormat = "%Y-%m-%d_%H:%M";

  if (_debugLevel >= 1)
  {
    cerr << endl << method_name << ": " << funcLabel << ": entry" << endl;
    cerr << "  _debugLevel: " << _debugLevel << endl;
    cerr << "  dir_path: \"" << dir_path << "\"" << endl;
    cerr << "  prod_id: " << prod_id << endl;
    cerr << "  prod_label: \"" << prod_label << "\"" << endl;
    cerr << "  chunk_ref: " << endl;

    int buflen = 1000;
    char bufa[ buflen];
    time_t tim = chunk_ref.valid_time;
    struct tm stm;
    gmtime_r( &tim, &stm);
    strftime(bufa, buflen, dateFormat.c_str(), &stm);

    cerr << "    valid_time (utc): " << bufa << endl;
    tim = chunk_ref.expire_time;
    gmtime_r( &tim, &stm);
    strftime( bufa, buflen, dateFormat.c_str(), &stm);
    cerr << "    expire_time (utc): " << bufa << endl;

    cerr << "    data_type: " << chunk_ref.data_type << endl;
    cerr << "    data_type2: " << chunk_ref.data_type2 << endl;
    cerr << "    offset: " << chunk_ref.offset << endl;
    cerr << "    len: " << chunk_ref.len << endl;
    
    cerr << "  spdb_len: " << spdb_len << endl;
    cerr << "  minLat: " << bbox.getMinLat() << endl;
    cerr << "  maxLat: " << bbox.getMaxLat() << endl;
    cerr << "  minLon: " << bbox.getMinLon() << endl;
    cerr << "  maxLon: " << bbox.getMaxLon() << endl;
  }

  if (_debugLevel >= 5)
  {
    int buflen = 1000;
    char bufa[ buflen];
    time_t tim = chunk_ref.valid_time;
    struct tm stm;
    gmtime_r( &tim, &stm);
    strftime( bufa, buflen, dateFormat.c_str(), &stm);

    char fname[1000];
    sprintf( fname, "%s/temp.%s.valid.%s.rand.%ld.xml",
	     dir_path.c_str(), funcLabel.c_str(), bufa, random());
    cerr << "  writing xml to output file: " << fname << endl;
    FILE * fout = fopen( fname, "w");
    fwrite( spdb_data, spdb_len, 1, fout);
    fclose( fout);
  }
}
