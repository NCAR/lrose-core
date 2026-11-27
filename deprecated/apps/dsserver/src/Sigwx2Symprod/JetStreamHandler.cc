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
 * @file JetStreamHandler.cc
 *
 * @class JetStreamHandler
 *
 * Class for handling jet stream data.
 *  
 * @date 10/10/2009
 *
 */

#include <cmath>

#include <toolsa/toolsa_macros.h>
#include <xmlformats/SigwxJetStreamBuffer.hh>

#include "Server.hh"
#include "JetStreamHandler.hh"

using namespace std;


// Globals

const int JetStreamHandler::ARROWHEAD_LEN_PIXELS = 15;
const double JetStreamHandler::ARROWHEAD_HALF_ANGLE = 20.0;


/*********************************************************************
 * Constructors
 */

JetStreamHandler::JetStreamHandler(Params *params,
				   const int debug_level) :
  ProductHandler(params, debug_level)
{
}


/*********************************************************************
 * Destructor
 */

JetStreamHandler::~JetStreamHandler()
{
}


/*********************************************************************
 * convertToSymprod()
 */

int JetStreamHandler::convertToSymprod(const string &dir_path,
				       const int prod_id,
				       const string &prod_label,
				       const Spdb::chunk_ref_t &chunk_ref,
				       const Spdb::aux_ref_t &aux_ref,
				       const void *spdb_data,
				       const int spdb_len,
				       MemBuf &symprod_buf,
				       const BoundingBox &bbox) const
{
  static const string method_name = "JetStreamHandler::convertToSymprod()";
  
  _printEntryInfo("jetstream",
		  dir_path, prod_id, prod_label,
		  chunk_ref,
		  spdb_data, spdb_len, bbox);

  // Initialize the Symprod object

  time_t curTime = time(NULL);
  Symprod symprod(curTime,    // generate time
		  curTime,    // received time
		  chunk_ref.valid_time,     // start time
		  chunk_ref.expire_time,    // expire time
		  chunk_ref.data_type,
		  chunk_ref.data_type2,
		  SPDB_WAFS_SIGWX_JETSTREAM_LABEL);        // label

  // Extract the Jet Stream information from the SPDB buffer

  SigwxJetStreamBuffer jet_stream_buffer;
  
  if (!jet_stream_buffer.disassemble(spdb_data, spdb_len))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error disassembling SPDB buffer" << endl;
    cerr << "Skipping product" << endl;
    
    return -1;
  }
  
  // Display the jet stream products

  vector< SigwxJetStream > jet_streams = jet_stream_buffer.getJetStreams();
  vector< SigwxJetStream >::const_iterator jet_stream;
  
  for (jet_stream = jet_streams.begin(); jet_stream != jet_streams.end();
       ++jet_stream)
  {
    // Create the display point list

    vector< SigwxPoint > jet_stream_pts = jet_stream->getPoints();

    // Display the jet stream

    if (!_displayList(jet_stream->getPoints(), symprod, bbox))
      return -1;

  } /* endfor - jet_stream */
  
  if (_debugLevel >= 5)
  {
    cerr << method_name << ": final symprod:" << endl;
    symprod.print(cerr);
  }
  symprod.serialize(symprod_buf);

  if (_debugLevel >= 1) 
    cerr << endl << method_name << ": exit" << endl;

  return 0;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _addChangeBar()
 */

bool JetStreamHandler::_addChangeBar(Symprod &symprod,
				     const BoundingBox &bbox,
				     const SigwxPoint &point,
				     const SigwxPoint &prev_point,
				     const Symprod::wpt_t *waypoints,
				     const int num_pts,
				     const int ipt) const
{
  static const string method_name = "JetStreamHandler::_addChangeBar()";

  double prev_wind_speed = prev_point.windSpeed / KNOTS_TO_MS;
  double wind_speed = point.windSpeed / KNOTS_TO_MS;
  
  int num_bars = (int)round(fabs(prev_wind_speed - wind_speed) / 10.0);
  
  // Don't render change bars outside of the bounding box

  if (!bbox.isInterior(waypoints[ipt].lat, waypoints[ipt].lon))
    return true;
  
  // Roughly center the change bar over xpos = 0

  int gap = _params->jetstream_wind_barb_gap;
  int height = _params->jetstream_wind_barb_height;
  
  int icon_width = gap * (num_bars - 1);
  
  if (icon_width < 1)
    icon_width = 1;
  
  // Generate the icon

  vector< Symprod::ppt_t > change_bar_icon;
  Symprod::ppt_t icon_pt;
  
  int xpos = -(icon_width / 2);
  
  for (int i = 0; i < num_bars; ++i)
  {
    icon_pt.x = xpos;
    icon_pt.y = height;
    change_bar_icon.push_back(icon_pt);
    
    icon_pt.x = xpos;
    icon_pt.y = -height;
    change_bar_icon.push_back(icon_pt);
    
    icon_pt.x = Symprod::PPT_PENUP;
    icon_pt.y = Symprod::PPT_PENUP;
    change_bar_icon.push_back(icon_pt);
    
    xpos += gap;
    
  }
  
  // Create the point list for the Symprod object, along the way rotating
  // the points along the new coordinate system

  Symprod::wpt_t line_pt1;
  Symprod::wpt_t line_pt2;
	
  if (ipt == 0)
  {
    line_pt1 = waypoints[ipt];
    line_pt2 = waypoints[ipt+1];
  }
  else if (ipt == num_pts - 1)
  {
    line_pt1 = waypoints[ipt-1];
    line_pt2 = waypoints[ipt];
  }
  else
  {
    line_pt1 = waypoints[ipt-1];
    line_pt2 = waypoints[ipt+1];
  }

  Symprod::ppt_t *symprod_pts = _rotateIcon(change_bar_icon,
					    line_pt1, line_pt2, true);
  
  // Add the change bar to the symprod buffer

  symprod.addStrokedIcons(_params->jetstream_line_color,
			  change_bar_icon.size(),
			  symprod_pts,
			  1,
			  &(waypoints[ipt]),
			  0,
			  Symprod::DETAIL_LEVEL_DO_NOT_SCALE,
			  _params->jetstream_line_width);

  delete [] symprod_pts;
  
  return true;
}


/*********************************************************************
 * _addWindFleche()
 */

bool JetStreamHandler::_addWindFleche(Symprod &symprod,
				      const BoundingBox &bbox,
				      const SigwxPoint &point,
				      const Symprod::wpt_t *waypoints,
				      const int num_pts,
				      const int ipt) const
{
  static const string method_name = "JetStreamHandler::_addWindFleche()";
  
  // Don't render fleches outside of the bounding box

  if (!bbox.isInterior(waypoints[ipt].lat, waypoints[ipt].lon))
    return true;
  
  char textbuf[1000];

  // Flight level label

  double flMeters = point.flightLevel;
  if (flMeters != SigwxPoint::MISSING_VALUE)
  {
    int fl_value = (int)round(flMeters / M_PER_FT / 100);
    sprintf(textbuf, "FL%d", fl_value);

    symprod.addText(textbuf,
		    point.lat,
		    point.lon,
		    _params->jetstream_text_color,
		    _params->jetstream_text_bg_color,
		    0,
		    -12,
		    Symprod::VERT_ALIGN_CENTER,
		    Symprod::HORIZ_ALIGN_CENTER,
		    _params->font_size,
		    Symprod::TEXT_NORM,
		    _params->font_name);

    // Jet depth information -- only included where available

    double fl_80_below = point.height80KnotIsotachBelow;
    double fl_80_above = point.height80KnotIsotachAbove;
	
    if (fl_80_below != SigwxPoint::MISSING_VALUE &&
	fl_80_above != SigwxPoint::MISSING_VALUE)
    {
      int fl_80_below_value = (int)round(fl_80_below / M_PER_FT / 100);
      int fl_80_above_value = (int)round(fl_80_above / M_PER_FT / 100);
      sprintf(textbuf, "%d/%d", fl_80_below_value, fl_80_above_value);
	  
      symprod.addText(textbuf,
		      point.lat,
		      point.lon,
		      _params->jetstream_text_color,
		      _params->jetstream_text_bg_color,
		      0,
		      -12 - _params->font_size,
		      Symprod::VERT_ALIGN_CENTER,
		      Symprod::HORIZ_ALIGN_CENTER,
		      _params->font_size,
		      Symprod::TEXT_NORM,
		      _params->font_name);
	  
    } /* endif -- fl_80_above != SigwxPoint::MISSING_VALUE ... */
	
  } /* endif -- flMeters != SigwxPoint::MISSING_VALUE */
      
  // Create the wind fleche

  Symprod::wpt_t line_pt1;
  Symprod::wpt_t line_pt2;
	
  if (ipt == 0)
  {
    line_pt1 = waypoints[ipt];
    line_pt2 = waypoints[ipt+1];
  }
  else if (ipt == num_pts - 1)
  {
    line_pt1 = waypoints[ipt-1];
    line_pt2 = waypoints[ipt];
  }
  else
  {
    line_pt1 = waypoints[ipt-1];
    line_pt2 = waypoints[ipt+1];
  }

  double windSpeedMeters = point.windSpeed;
  if (windSpeedMeters != SigwxPoint::MISSING_VALUE)
  {
    if (!_drawFleche(symprod,
		     windSpeedMeters,
		     waypoints[ipt],
		     line_pt1, line_pt2,
		     waypoints[0].lat >= 0.0))
      return false;
  } /* endif - windSpeedMeters != SigwxPoint::MISSING_VALUE */

  return true;
}


/*********************************************************************
 * _addWindFleches()
 */

bool JetStreamHandler::_addWindFleches(const vector< SigwxPoint > &points,
				       const Symprod::wpt_t *waypoints,
				       const size_t num_pts,
				       Symprod &symprod,
				       const BoundingBox &bbox) const
{
  static const string method_name = "JetStreamHandler::_addWindFleches()";
  
  // The instructions for rendering the fleches is that you render a fleche
  // at each maximum wind location.  For the locations in between, you render
  // a fleche if there is space, otherwise you render a change bar.    The
  // minimum distance between fleches is given by the
  // jetstream_min_fleche_distance parameter and is specified in number of
  // characters to match the WAFS representation documentation.

  // So, here we'll loop through the jet stream until we reach a point of
  // maximum wind or the end of the point list.  If we find a maximum wind,
  // we'll render a fleche at that point and then go back through the list
  // rendering fleches or change bars, as appropriate.  If we reach the end
  // of the list, we'll go back to the previous maximum wind and render
  // fleches or change bars as appropriate for these last points.

  double min_fleche_dist = (double)_params->jetstream_min_fleche_distance;
  
  int prev_max_fleche = -1;
  int prev_fleche = -1;
  int prev_loc = -1;
  
  int curr_pt = 0;
  
  while (true)
  {
    // Look for the next maximum wind location

    while (curr_pt < (int)points.size() &&
	   (points[curr_pt].windSpeed == SigwxPoint::MISSING_VALUE ||
	    !points[curr_pt].isMaxWind()))
      ++curr_pt;
    
    if (curr_pt >= (int)points.size())
      break;
    
    // If we get here, we've found a max wind location.  We need to render
    // this wind fleche and then process all of the points up to this point

    _addWindFleche(symprod, bbox, points[curr_pt],
		   waypoints, points.size(), curr_pt);

    prev_fleche = curr_pt;
    prev_loc = curr_pt;
    
    for (int i = curr_pt - 1; i > prev_max_fleche; --i)
    {
      // Skip points with no wind speed information

      if (points[i].windSpeed == SigwxPoint::MISSING_VALUE)
	continue;
    
      if (_calcDistance(bbox,
			waypoints[prev_fleche],
			waypoints[i]) > min_fleche_dist)
      {
	_addWindFleche(symprod, bbox, points[i],
		       waypoints, points.size(), i);

	prev_fleche = i;
      }
      else
      {
	_addChangeBar(symprod, bbox, points[i], points[prev_loc],
		      waypoints, points.size(), i);
      }
      
      prev_loc = i;
      
    } /* endfor - i */
    
    prev_max_fleche = curr_pt;
    ++curr_pt;
    
  } /* endwhile - true */
  
  // When we get here, we've rendered all of the points up to and including
  // the last max wind point.  So, now we render from the last max wind point
  // to the end of the list.

  prev_fleche = prev_max_fleche;
  prev_loc = prev_max_fleche;
  
  for (int i = prev_max_fleche + 1; i < (int)points.size(); ++i)
  {
    // Skip points with no wind speed information

    if (points[i].windSpeed == SigwxPoint::MISSING_VALUE)
      continue;
    
    if (prev_fleche < 0 ||
	_calcDistance(bbox,
		      waypoints[prev_fleche],
		      waypoints[i]) > min_fleche_dist)
    {
      _addWindFleche(symprod, bbox, points[i],
		     waypoints, points.size(), i);

      prev_fleche = i;
    }
    else
    {
      _addChangeBar(symprod, bbox, points[i], points[prev_loc],
		    waypoints, points.size(), i);
    }
    
    prev_loc = i;
    
  } /* endfor - i */

  return true;
}


/*********************************************************************
 * _calcDistance()
 */

double JetStreamHandler::_calcDistance(const BoundingBox &bbox,
				       const Symprod::wpt_t &point1,
				       const Symprod::wpt_t &point2) const
{
  double y_diff = bbox.degLatToPixels(point1.lat - point2.lat,
				      _params->cloud_screen_size);
  double x_diff = bbox.degLonToPixels(point1.lon - point2.lon,
				      _params->cloud_screen_size);
  
  double dist_pixels = sqrt((x_diff * x_diff) +(y_diff * y_diff));
  
  return dist_pixels / (double)_params->font_size;
}


/*********************************************************************
 * _displayList()
 */

bool JetStreamHandler::_displayList(const vector< SigwxPoint > &points,
				    Symprod &symprod,
				    const BoundingBox &bbox) const
{
  static const string method_name = "JetStreamHandler::_displayList()";
  
  // First create the polyline for symprod, so the labels
  // go on top later.  Apparently CIDD puts the last
  // thing specified on top.

  if (_debugLevel >= 1)
  {
    cerr << method_name << ": numPts: " << points.size() << endl;
    cerr << "  minLon: " << bbox.getMinLon() << endl;
    cerr << "  maxLon: " << bbox.getMaxLon() << endl;
  }
  
  // Get the display points from the jet stream points.  Here we allocate
  // space for the display points and normalize the longitudes based on the
  // defined bounding box.  If the returned list is null, then there are no
  // points to display.  This is not considered an error condition.

  Symprod::wpt_t *waypoints;
  size_t num_pts;

  if ((waypoints = _getWaypoints(points, bbox, num_pts)) == 0)
    return true;
  
  // We must have at least 2 points to draw a jet stream.  Note that this is
  // not considered an error condition.

  if (num_pts < 2)
  {
    delete [] waypoints;
    return true;
  }
  
  symprod.addPolyline(num_pts,
		      waypoints,
		      _params->jetstream_line_color,
		      Symprod::LINETYPE_SOLID,
		      _params->jetstream_line_width,
		      Symprod::CAPSTYLE_BUTT,
		      Symprod::JOINSTYLE_BEVEL,
		      false,
		      Symprod::FILL_NONE,
		      0,
		      0);

  // Add arrowhead if the last point is not at a boundary

  if (bbox.isInterior(waypoints[num_pts-1].lat, waypoints[num_pts-1].lon))
  {
    symprod.addArrowBothPts(_params->jetstream_line_color,
			    Symprod::LINETYPE_SOLID,
			    _params->jetstream_line_width,
			    Symprod::CAPSTYLE_BUTT,
			    Symprod::JOINSTYLE_BEVEL,
			    waypoints[num_pts-2].lat,
			    waypoints[num_pts-2].lon,
			    waypoints[num_pts-1].lat,
			    waypoints[num_pts-1].lon,
			    ARROWHEAD_LEN_PIXELS,
			    ARROWHEAD_HALF_ANGLE,
			    0,
			    Symprod::DETAIL_LEVEL_DO_NOT_SCALE);
  }
  
  
  // Now add the text labels and wind fleches on top.

  if (!_addWindFleches(points, waypoints, num_pts, symprod, bbox))
  {
    delete [] waypoints;
    return false;
  }
  
  // Reclaim memory

  delete[] waypoints;

  return true;
}


/*********************************************************************
 * _drawFleche()
 */

bool JetStreamHandler::_drawFleche(Symprod &symprod,
				   const double wind_speed_m,
				   const Symprod::wpt_t location,
				   const Symprod::wpt_t line_pt1,
				   const Symprod::wpt_t line_pt2,
				   const bool north_hem_flag) const
{
  static const string method_name = "JetStreamHandler::_drawFleche()";
  
  // convert from meters/second

  double wind_speed_knots = wind_speed_m / KNOTS_TO_MS;

  // Make wind barbs
  // Create barbs = points in a stroked icon.
  // Initially create barbs assuming the jet is
  // headed East, then rotate to the correct direction.
  //
  // For now we make the barbs going up, assuming
  // the northern hemisphere.  Below we switch it if need be.
  // For jets starting in the northern hemisphere, the
  // barbs are on the left.  If starting in the southern
  // hemisphere, barbs are on the right.

  // fleche height in pixels

  int fheight = _params->jetstream_wind_barb_height;

  // triangle base width in pixels

  int fwidth = _params->jetstream_wind_barb_width;

  // gap between fleches, in pixels

  int fgap = _params->jetstream_wind_barb_gap;

  // Calculate the number of each type of barb in the fleche

  double knots_left = wind_speed_knots;

  int num_triangles = (int)(knots_left / 50.0);
  knots_left -= num_triangles * 50.0;
  
  int num_lines = (int)(knots_left / 10.0);
  knots_left -= num_lines * 10.0;
  
  int num_half_lines = (int)(knots_left / 5.0);
  
  // Create the vector of points for the stroked icon and create a point
  // object so we can add points as we go along

  vector< Symprod::ppt_t > fleche_icon;
  Symprod::ppt_t icon_pt;
  
  // Roughly center the fleche over xxpos = 0

  int icon_width =
    (num_triangles * fwidth) + (num_lines * fgap) + (num_half_lines * fgap);
  
  int xxpos = -(icon_width / 2);
  int yypos = 0;

  // Initial point

  icon_pt.x = xxpos;
  icon_pt.y = yypos;
  fleche_icon.push_back(icon_pt);
  
  // Make one triangle per 50 knots

  for (int i = 0; i < num_triangles; ++i)
  {
    // Draw a filled triangle bbLat drawing multiple vertical lines
    
    for (int ii = 0; ii < fwidth; ii++)
    {
      icon_pt.x = xxpos + ii;
      icon_pt.y = yypos;
      fleche_icon.push_back(icon_pt);

      icon_pt.x = xxpos + ii;
      icon_pt.y = yypos + fheight - fheight*ii/fwidth;
      fleche_icon.push_back(icon_pt);
      
      icon_pt.x = xxpos + ii;
      icon_pt.y = yypos;
      fleche_icon.push_back(icon_pt);
    }

    xxpos += fwidth;
  } /* endfor - i */

  // Make one fleche per 10 knots

  for (int i = 0; i < num_lines; ++i)
  {
    xxpos += fgap;

    // Draw a backward slanting line
	  
    icon_pt.x = xxpos;
    icon_pt.y = yypos;
    fleche_icon.push_back(icon_pt);
    
    icon_pt.x = xxpos - fwidth;
    icon_pt.y = yypos + fheight;
    fleche_icon.push_back(icon_pt);
    
    // Return to start

    icon_pt.x = xxpos;
    icon_pt.y = yypos;
    fleche_icon.push_back(icon_pt);

  } /* endfor - i */

  // Make one half fleche per 5 knots
  
  for (int i = 0; i < num_half_lines; ++i)
  {
    xxpos += fgap;

    // Draw a backward slanting line

    icon_pt.x = xxpos;
    icon_pt.y = yypos;
    fleche_icon.push_back(icon_pt);
    
    icon_pt.x = xxpos - fwidth/2;
    icon_pt.y = yypos + fheight/2;
    fleche_icon.push_back(icon_pt);
    
    // Return to start

    icon_pt.x = xxpos;
    icon_pt.y = yypos;
    fleche_icon.push_back(icon_pt);
    
    xxpos += fgap;
  } /* endfor - i */

  // Rotate to a new coordinate system that follows the jetstream line.
  
  Symprod::ppt_t *symprod_pts = _rotateIcon(fleche_icon,
					    line_pt1, line_pt2,
					    north_hem_flag);
  
  // Add the fleche to the symprod buffer

  symprod.addStrokedIcons(_params->jetstream_line_color,
			  fleche_icon.size(),
			  symprod_pts,
			  1,
			  &location,
			  0,
			  Symprod::DETAIL_LEVEL_DO_NOT_SCALE,
			  _params->jetstream_line_width);

  delete [] symprod_pts;
  
  return true;
}


/*********************************************************************
 * _rotateIcon()
 */

Symprod::ppt_t *JetStreamHandler::_rotateIcon(vector< Symprod::ppt_t > icon,
					      const Symprod::wpt_t line_pt1,
					      const Symprod::wpt_t line_pt2,
					      const bool north_hem_flag) const
{
  double deltax = line_pt2.lon - line_pt1.lon;
  double deltay = line_pt2.lat - line_pt1.lat;
  
  double theta = atan2(deltay, deltax);
  double ss = sin(theta);
  double cc = cos(theta);

  Symprod::ppt_t *symprod_pts = new Symprod::ppt_t[icon.size()];
  
  for (size_t i = 0; i < icon.size(); ++i)
  {
    // Copy the point

    symprod_pts[i] = icon[i];
    
    // Don't rotate pen up flags

    if (symprod_pts[i].x == Symprod::PPT_PENUP ||
	symprod_pts[i].y == Symprod::PPT_PENUP)
      continue;
    
    // Account for the hemisphere

    if (!north_hem_flag)
      symprod_pts[i].y *= (si32)-1.0;
    
    // Rotate the point

    int xpos = symprod_pts[i].x;
    int ypos = symprod_pts[i].y;

    symprod_pts[i].x = (int)(cc * xpos - ss * ypos);
    symprod_pts[i].y = (int)(ss * xpos + cc * ypos);

  } /* endfor - i */

  return symprod_pts;
}
