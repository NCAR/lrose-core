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
 * @file TurbulenceHandler.cc
 *
 * @class TurbulenceHandler
 *
 * Class for handling turbulence data.
 *  
 * @date 10/10/2009
 *
 */

#include <cmath>

#include <xmlformats/SigwxTurbBuffer.hh>

#include "Server.hh"
#include "TurbulenceHandler.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

TurbulenceHandler::TurbulenceHandler(Params *params,
				     const int debug_level) :
  ProductHandler(params, debug_level)
{
}


/*********************************************************************
 * Destructor
 */

TurbulenceHandler::~TurbulenceHandler()
{
}


/*********************************************************************
 * convertToSymprod()
 */

int TurbulenceHandler::convertToSymprod(const map< string, IconDef > &icon_def_list,
					const string &dir_path,
					const int prod_id,
					const string &prod_label,
					const Spdb::chunk_ref_t &chunk_ref,
					const Spdb::aux_ref_t &aux_ref,
					const void *spdb_data,
					const int spdb_len,
					MemBuf &symprod_buf,
					const BoundingBox &bbox) const
{
  static const string method_name = "TurbulenceHandler::ConvertTurbulenceToSymprod()";
  
  _printEntryInfo("turbulence",
		  dir_path, prod_id, prod_label,
		  chunk_ref,
		  spdb_data, spdb_len, bbox);

  // Initialize the symprod object

  time_t curTime = time(0);
  Symprod symprod(
    curTime,    // generate time
    curTime,    // received time
    chunk_ref.valid_time,     // start time
    chunk_ref.expire_time,    // expire time
    chunk_ref.data_type,
    chunk_ref.data_type2,
    SPDB_WAFS_SIGWX_TURBULENCE_LABEL);        // label

  // Get the turbulence information

  SigwxTurbBuffer turb_buffer(_debugLevel >= 1, _debugLevel >= 5);
  
  if (!turb_buffer.disassemble(spdb_data, spdb_len))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error disassembling SPDB buffer" << endl;
    cerr << "Skipping product" << endl;
    
    return 0;
  }
  
  // Display all of the turbulence objects in the buffer

  vector< SigwxTurb > turbs = turb_buffer.getTurbs();
  vector< SigwxTurb >::const_iterator turb;
  
  for (turb = turbs.begin(); turb != turbs.end(); ++turb)
  {
    // Put together the display messages

    vector< string > msgs;
    vector< bool > hiddens;  //are the msgs hidden(popup)?
    
    if (_params->hidden_text_flag)
    {
      char text_buffer[1024];
      string msg = "";
      msg += "Turbulence\n";

      // Degree

      msg += "DIST: " + turb->getDegreeString() + "\n";

      // Maximum height

      double max_ht = turb->getMaxHt();
      if (max_ht >= 0.0)
      {
	sprintf(text_buffer, "MAX FL: %g", round(max_ht / 30.48));
	msg += text_buffer;
	msg += "\n";
      }
      else
      {
	msg += "MAX FL: UNKNOWN\n";
      }

      // Minimum height

      double min_ht = turb->getMinHt();
      if (min_ht >= 0.0)
      {
	sprintf(text_buffer, "MIN FL: %g", round(min_ht / 30.48));
	msg += text_buffer;
	msg += "\n";
      }
      else
      {
	msg += "MIN FL: UNKNOWN\n";
      }

      // Analysis time

      DateTime analysis_time = turb->getAnalysisTime();
      sprintf(text_buffer, "ANALYSIS DATE: %04d-%02d-%02d %02d:%02d",
              analysis_time.getYear(),
              analysis_time.getMonth(),
              analysis_time.getDay(),
              analysis_time.getHour(),
              analysis_time.getMin());
      msg += text_buffer;
      msg += "\n";

      // Forecast time

      DateTime forecast_time = turb->getForecastTime();
      sprintf(text_buffer, "FORECAST DATE: %04d-%02d-%02d %02d:%02d",
              forecast_time.getYear(),
              forecast_time.getMonth(),
              forecast_time.getDay(),
              forecast_time.getHour(),
              forecast_time.getMin());
      msg += text_buffer;
      msg += "\n";

      msgs.push_back(msg);
      hiddens.push_back(true);
   } // if hidden_text_flag (that is, if JMDS)
    
    //display the simple text?
    if (_params->basic_text_flag)
      {
	// Minimum height
	char min_string[1000];
	double min_ht = turb->getMinHt();
      
	if (min_ht >= 0.0)
	  sprintf(min_string, "%g", round(min_ht / 30.48));
	else
	  strcpy( min_string, "--");

	// Maximum height
	char max_string[1000];
	double max_ht = turb->getMaxHt();
      
	if (max_ht >= 0.0)
	  sprintf(max_string, "%g", round(max_ht / 30.48));
	else
	  strcpy(max_string, "--");

	msgs.push_back(string("FL ") + min_string + " TO " + max_string);
	hiddens.push_back(false);
      }

    // Display the polyline and text
    if (!_displayList(false,
		      turb->getPoints(),
		      &symprod, icon_def_list,
		      bbox,
		      _params->turbulence_line_color,
		      Symprod::LINETYPE_DASH,
		      _params->turbulence_line_width,
		      _params->cloud_screen_size,
		      _params->cloud_scallop_size,
		      Symprod::FILL_NONE,
		      _params->turbulence_text_color,
		      _params->turbulence_text_bg_color,
		      _params->font_name,
		      _params->font_size,
		      _params->hidden_text_flag,
		      msgs,hiddens))
      return -1;

  } /* endfor - turb */
  
  if (_debugLevel >= 5)
  {
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
 * _displayList()
 */

bool TurbulenceHandler::_displayList(const bool scallopFlag,
				     const vector< SigwxPoint > &points,
				     Symprod * symprod,
				     const map< string, IconDef > &icon_def_list,
				     const BoundingBox &bbox,
				     const string &line_color,
				     const Symprod::linetype_t line_type,
				     const int line_width,
				     const int screenSize,
				     const int scallopSize,
				     const int fill_type,
				     const string &text_color,
				     const string &text_bg_color,
				     const string &font_name,
				     const int font_size,
				     const bool hidden_text_flag,
				     const vector< string > &msgs,
				     const vector< bool > &hiddens) const
{
  static const string method_name = "TurbulenceHandler::_displayList()";
  
  // First create the polyline for symprod, so the labels
  // go on top later.

  if (_debugLevel >= 1)
  {
    cerr << method_name << ": num_pts: " << points.size() << endl;
    for (size_t ipt= 0; ipt < points.size(); ipt++)
    {
      cerr << "  " << setw(3) << ipt
	   << "  itemList lat: " << points[ipt].lat
	   << "  lon: " << points[ipt].lon
	   << endl;
    }
  }

  // Get the display points from the jet stream points.  Here we allocate
  // space for the display points and normalize the longitudes based on the
  // defined bounding box.  If the returned list is null, then there are no
  // points to display.  This is not considered an error condition.

  Symprod::wpt_t *waypoints;
  size_t num_pts;

  if ((waypoints = _getWaypoints(points, bbox, num_pts)) == 0)
    return true;
  
  // Calc lat, lon averages for the text label

  double avgLat = 0.0;
  double avgLon = 0.0;
  for (size_t ipt= 0; ipt < num_pts; ipt++)
  {
    double tlat = waypoints[ipt].lat;
    double tlon = waypoints[ipt].lon;
    avgLat += tlat;

    // Adjust longitudes so they are near waypoints[0].
    // This gets rid of the problem spanning from 179 to -179.

    while (waypoints[0].lon - tlon > 180)
      tlon += 360.;
    while (tlon - waypoints[0].lon > 180)
      tlon -= 360.;
    avgLon += tlon;
  }

  avgLat /= (double) num_pts;
  avgLon /= (double) num_pts;

  if (_debugLevel >= 1)
  {
    cerr << "  " << method_name << endl;
    cerr << "  hidden_text_flag: " << hidden_text_flag << endl;
    cerr << "  num_pts: " << num_pts << endl;
    cerr << "  avgLat: " << avgLat << endl;
    cerr << "  avgLon: " << avgLon << endl;
    cerr << "  Msgs: " << endl;

    vector< string >::const_iterator msg;
    
    for (msg = msgs.begin(); msg != msgs.end(); ++msg)
      cerr << "    "  << *msg << endl;

    cerr << method_name << ":" << endl;
    for (size_t ipt= 0; ipt < num_pts; ipt++)
    {
      cerr << "  lat: " << waypoints[ipt].lat << "  lon: "
	   << waypoints[ipt].lon << endl;
    }
  }

  if (scallopFlag)
  {
    // scalloped edges for clouds

    // Make arcs for the scalloped edge effect.

    for (size_t ipt= 0; ipt < num_pts-1; ipt++)
    {
      Symprod::wpt_t * pta = &(waypoints[ipt]);
      Symprod::wpt_t * ptb = &(waypoints[ipt+1]);
      if (bbox.isInterior(pta->lat, pta->lon) ||
	  bbox.isInterior(ptb->lat, ptb->lon))
      {
        _mkScallops(pta->lat, pta->lon, ptb->lat, ptb->lon,
		    symprod, line_color, line_width,
		    screenSize, scallopSize, bbox);

        // Display point numbers to show the ordering.
        //char textbuf[1000];
        //sprintf( textbuf, "pt %d", ipt);
        //symprod->addText(
        //  textbuf,            // message
        //  pta->lat,           // latitude
        //  pta->lon,           // longitude
        //  text_color,
        //  text_bg_color,
        //  0,                // offset_x
        //  0,                // offset_y
        //  Symprod::VERT_ALIGN_CENTER,      // BOTTOM, CENTER, TOP
        //  Symprod::HORIZ_ALIGN_CENTER,     // LEFT, CENTER, RIGHT
        //  font_size,
        //  Symprod::TEXT_NORM,              // NORM, BOLD, ITALICS, etc.
        //  font_name);
      }
    }
  }
  else
  {
    // Not scalloped: turbulence

    symprod->addPolyline(num_pts,
			 waypoints,
			 line_color.c_str(),
			 line_type,
			 line_width,
			 Symprod::CAPSTYLE_BUTT,
			 Symprod::JOINSTYLE_BEVEL,
			 true,
			 Symprod::FILL_NONE,
			 0,
			 0);
  }

  delete[] waypoints;

  if (hidden_text_flag)
  {
    // Add central icon.
    string iconName = "turb_icon";
    float icon_scale = _params->turb_icon_scale;
    if (scallopFlag)
    {
      iconName = "cloud_icon";
      icon_scale = _params->cloud_icon_scale;
    }

    _drawIcon(iconName.c_str(),
	      icon_def_list,
	      line_color.c_str(),
	      line_width,
	      avgLat,
	      avgLon,
	      icon_scale,
	      false,
	      symprod);

    //    // Add central icon.
    //    // See: libs/Spdb/src/Symprod/Symprod_add.cc
    //    symprod->addArc(
    //      avgLat,        // circle origin_lat
    //      avgLon,        // circle origin_lon
    //      4,             // radius_x == point on circumf, degrees or pixels
    //      4,             // radius_y == point on circumf, degrees or pixels
    //      line_color,    // color
    //      true,          // true: radii in pixels.  false: in degrees.
    //      0.0,           // start angle in degrees
    //      360.0,         // end angle in degrees
    //      0.0,           // axis rotation
    //      20,            // nsegments
    //      Symprod::LINETYPE_SOLID,   // SOLID, DASH, DOT_DASH
    //      line_width,                // line_width
    //      Symprod::CAPSTYLE_BUTT,    // BUTT, NOT_LAST, PROJECTING, ROUND
    //      Symprod::JOINSTYLE_BEVEL,  // BEVEL, MITER, ROUND
    //      Symprod::FILL_SOLID,
    //             // NONE, STIPPLE10 - STIPPLE90, SOLID, ALPHA10 - ALPHA90
    //      0,                         // object id
    //      Symprod::DETAIL_LEVEL_DO_NOT_SCALE); // detail level

  } // if hidden_text_flag

  // Now add the text labels on top.

  _addTextToSymprod(symprod,
		    avgLat,
		    avgLon,
		    msgs,
		    hiddens,
		    text_color,
		    text_bg_color);

  return true;
}


/*********************************************************************
 * _mkScallops()
 */

void TurbulenceHandler::_mkScallops(const double pt1_lat,
				    const double pt1_lon,
				    const double pt2_lat,
				    const double pt2_lon,
				    Symprod * symprod,
				    const string &line_color,
				    const int line_width,
				    const int screenSize,
				    const int scallopSize,
				    const BoundingBox &bbox) const
{
  static const string method_name = "TurbulenceHandler::_mkScallops()";
  
  // Cloud areas are to the left of the line as the points
  // are traversed.
  // See section 6 of:
  // Representing WAFS Significant Weather (SIGWX) Data in BUFR
  // Version 4.1, December 2007
  // by World Area Forecast Centres (WAFCs) London and Washington

  double maxRange = bbox.getMaxDim();
  double distLat = pt2_lat - pt1_lat;
  double distLon = pt2_lon - pt1_lon;
  double dist = sqrt( distLat*distLat + distLon*distLon );
  int numScallops = int( screenSize * dist / (maxRange * scallopSize));

  double theta2 = atan2( pt2_lat - pt1_lat, pt2_lon - pt1_lon);
  double theta1 = theta2 + M_PI;
  if (theta1 >= 2 * M_PI)
    theta1 -= 2 * M_PI;

  // Insure theta1 < theta2 so
  // In libs/rapplot/src/gplot/gplot.c GDrawArc
  // we need delta_theta to be positive to draw the
  // arc to the right of the direction.
  // GDrawArc uses: delta_theta = (wangle2 - wangle1) *...
  // So insure theta2 < theta1.

  while (theta2 < theta1)
  {
    theta2 += 2 * M_PI;
  }
  theta1 *= 180 / M_PI;     // to degrees
  theta2 *= 180 / M_PI;     // to degrees

  if (_debugLevel >= 5)
  {
    cerr << endl;
    cerr << method_name << ": pt1_lat: " << pt1_lat << "  pt1_lon: " << pt1_lon
	 << endl;
    cerr << method_name << ": pt2_lat: " << pt2_lat << "  pt2_lon: " << pt2_lon
	 << endl;
    cerr << method_name << ": theta1: " << theta1 << endl;
    cerr << method_name << ": theta2: " << theta2 << endl;
  }

  for (int ii = 0; ii < numScallops; ii++)
  {
    double radius = 0.5 * dist / numScallops;
    double centLat = pt1_lat + (pt2_lat - pt1_lat) * (0.5 + ii) / numScallops;
    double centLon = pt1_lon + (pt2_lon - pt1_lon) * (0.5 + ii) / numScallops;

    symprod->addArc(centLat,
		    centLon,
		    radius,
		    radius,
		    line_color.c_str(),
		    false,
		    theta1,
		    theta2,
		    0.0,
		    6,
		    Symprod::LINETYPE_SOLID,
		    line_width,
		    Symprod::CAPSTYLE_BUTT,
		    Symprod::JOINSTYLE_BEVEL,
		    Symprod::FILL_NONE,
		    0,
		    Symprod::DETAIL_LEVEL_DO_NOT_SCALE);
  } // for ii
}
