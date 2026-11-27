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
 * @file CloudHandler.cc
 *
 * @class CloudHandler
 *
 * Class for handling cloud data.
 *  
 * @date 10/10/2009
 *
 */

#include <toolsa/toolsa_macros.h>
#include <xmlformats/SigwxCloudBuffer.hh>

#include "Server.hh"
#include "CloudHandler.hh"

using namespace std;


// Globals

const string CloudHandler::CLOUD_ICON_NAME = "cloud_icon";


/*********************************************************************
 * Constructors
 */

CloudHandler::CloudHandler(Params *params,
			   const int debug_level) :
  ProductHandler(params, debug_level)
{
}


/*********************************************************************
 * Destructor
 */

CloudHandler::~CloudHandler()
{
}


/*********************************************************************
 * convertToSymprod()
 */

int CloudHandler::convertToSymprod(const map< string, IconDef > &icon_def_list,
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
  static const string method_name = "CloudHandler::convertToSymprod()";
  
  _printEntryInfo("cloud",
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
    SPDB_WAFS_SIGWX_CLOUD_LABEL);        // label

  // Get the cloud information

  SigwxCloudBuffer cloud_buffer(_debugLevel >= 1, _debugLevel >= 5);
  
  if (!cloud_buffer.disassemble(spdb_data, spdb_len))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error disassembling SPDB buffer" << endl;
    cerr << "Skipping product" << endl;
    
    return -1;
  }
  
  // Display all of the clouds in the buffer

  vector< SigwxCloud > clouds = cloud_buffer.getClouds();
  vector< SigwxCloud >::const_iterator cloud;
  
  for (cloud = clouds.begin(); cloud != clouds.end(); ++cloud)
  {
    if (!_displayCloud(bbox,
		       icon_def_list,
		       &symprod,
		       *cloud))
      return -1;
  } /* endfor - cloud */
  
  if (_debugLevel >= 5)
  {
    cerr << method_name << ": final symprod:" << endl;
    symprod.print(cerr);
  }
  symprod.serialize(symprod_buf);

  if (_debugLevel >= 1) 
    cerr << endl << method_name << ": exit" << endl;

  return 0;
} // end ConvertCloudToSymprod



/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _addTextBoxToSymprod()
 */

void CloudHandler::_addTextBoxToSymprod(Symprod *symprod,
					const double box_ll_lat,
					const double box_ll_lon,
					const double cloud_pt_lat,
					const double cloud_pt_lon,
					const vector< string > &msgs,
					const string &text_color,
					const string &text_bg_color) const
{
  static const string method_name = "CloudHandler::_addTextBoxToSymprod()";

  if (_debugLevel >= 5)
  {
    cerr << "---> Entering " << method_name << endl;
    cerr << "   box_ll_lat = " << box_ll_lat << endl;
    cerr << "   box_ll_lon = " << box_ll_lon << endl;
  }
  
  // Make all of the necessary calculations

  int text_max_chars = 0;
  
  vector< string >::const_iterator msg;
  for (msg = msgs.begin(); msg != msgs.end(); ++msg)
  {
    if ((int)msg->length() > text_max_chars)
      text_max_chars = msg->length();

    if (_debugLevel >= 5)
      cerr << "   " << *msg << endl;
  }
  
  double line_height = (double)_params->font_size * _params->line_space_factor;
  
  int height_pixels;
  int width_pixels;
  
  height_pixels =
    (int)round((double)msgs.size() * line_height);
  width_pixels = text_max_chars * _params->font_size;
  
  if (_debugLevel >= 5)
  {
    cerr << "   msgs.size() = " << msgs.size() << endl;
    cerr << "   text_max_chars = " << text_max_chars << endl;
    cerr << "   height_pixels = " << height_pixels << endl;
    cerr << "   width_pixels = " << width_pixels << endl;
  }
  
  // Render the text

  double yoffset =
    (double)height_pixels - (line_height / 2.0);

  for (msg = msgs.begin(); msg != msgs.end(); ++msg)
  {
    symprod->addText((*msg).c_str(),
		     box_ll_lat,
		     box_ll_lon,
		     text_color.c_str(),
		     text_bg_color.c_str(),
		     width_pixels / 2,
		     (int)yoffset,
		     Symprod::VERT_ALIGN_CENTER,
		     Symprod::HORIZ_ALIGN_CENTER,
		     _params->font_size,
		     Symprod::TEXT_NORM,
		     _params->font_name,
		     0,
		     Symprod::DETAIL_LEVEL_DO_NOT_SCALE);

    yoffset -= line_height;
  } /* endfor - msgs */

  // Render the box

  Symprod::ppt_t box_vertices[4];
  
  box_vertices[0].x = 0;
  box_vertices[0].y = 0;
  
  box_vertices[1].x = 0;
  box_vertices[1].y = height_pixels;
  
  box_vertices[2].x = width_pixels;
  box_vertices[2].y = height_pixels;
  
  box_vertices[3].x = width_pixels;
  box_vertices[3].y = 0;
  
  symprod->addIconline(box_ll_lat, box_ll_lon,
		       4, box_vertices,
		       text_color.c_str(),
		       Symprod::LINETYPE_SOLID,
		       1,
		       Symprod::CAPSTYLE_BUTT,
		       Symprod::JOINSTYLE_BEVEL,
		       true,
		       Symprod::FILL_NONE,
		       0,
		       Symprod::DETAIL_LEVEL_DO_NOT_SCALE);
  
  // Draw an error from the box to the cloud outline

  symprod->addArrowBothPts(text_color.c_str(),
			   Symprod::LINETYPE_SOLID,
			   1,
			   Symprod::CAPSTYLE_BUTT,
			   Symprod::JOINSTYLE_BEVEL,
			   box_ll_lat, box_ll_lon,
			   cloud_pt_lat, cloud_pt_lon,
			   15, 20, 0,
			   Symprod::DETAIL_LEVEL_DO_NOT_SCALE);
  
}


/*********************************************************************
 * _displayCloud()
 */

bool CloudHandler::_displayCloud(const BoundingBox &bbox,
				 const map< string, IconDef > &icon_def_list,
				 Symprod *symprod,
				 const SigwxCloud &cloud) const
{
  static const string method_name = "CloudHandler::_displayCloud()";
  
  // Display the cloud outline, if requested

  if (_params->cloud_display_outline)
    {
      vector< string > msgs;
      vector< bool > hiddens;  //are the msgs hidden(popup)?
  
      // Display the cloud text, if requested



      // If hidden_text_flag is
      // set, then we are displaying the clouds on JMDS (the Java-based
      // AOAWS display) which has the ability to display mouse-over text.
      if (_params->hidden_text_flag)
	{
	  char text_buffer[1024];
	  string msg = "";
	  msg +="CLOUD\n";
	  msg += "DIST: " + cloud.getCloudDistString() + "\n";
	  msg += "TYPE: " + cloud.getCloudTypeString() + "\n";

	  double max_ht = cloud.getCloudMaxHt();
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

	  double min_ht = cloud.getCloudMinHt();
	  if (min_ht >= 0.0)
	    {
	      sprintf(text_buffer, "MIN FL: %g", round(min_ht / 30.48));
	      msg += text_buffer;
	      msg += "\n";

	    }
	  else
	    {
	      msg +="MIN FL: UNKNOWN\n";
	    }

	  DateTime analysis_time = cloud.getAnalysisTime();
	  sprintf(text_buffer, "ANALYSIS DATE: %04d-%02d-%02d %02d:%02d",
		  analysis_time.getYear(),
		  analysis_time.getMonth(),
		  analysis_time.getDay(),
		  analysis_time.getHour(),
		  analysis_time.getMin());
	  msg += text_buffer;
	  msg += "\n";

	  DateTime forecast_time = cloud.getForecastTime();
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
	}

      // We also have a short text message that can be displayed
      // with the cloud outline.
      if (_params->basic_text_flag)
	{
 
	  //generate basic text
	  double min_cld_ht = cloud.getCloudMinHt();
	  char min_string[1000];

	  if (min_cld_ht >= 0.0)
	    sprintf(min_string, "%g", round(min_cld_ht / 30.48));
	  else
	    strcpy(min_string, "--");
  
	  double max_cld_ht = cloud.getCloudMaxHt();
	  char max_string[1000];

	  if (max_cld_ht >= 0.0)
	    sprintf(max_string, "%g", round(max_cld_ht / 30.48));
	  else
	    strcpy(max_string, "--");

	  msgs.push_back(string("FL ") + min_string + " TO " + max_string);
	  hiddens.push_back(false);
	}

      // Display the lines and text

      if (!_displayList(cloud.getPoints(),
			symprod, icon_def_list,
			bbox,
			_params->cloud_line_color,
			Symprod::LINETYPE_SOLID,
			_params->cloud_line_width,
			_params->cloud_screen_size,
			_params->cloud_scallop_size,
			Symprod::FILL_NONE,
			_params->cloud_text_color,
			_params->cloud_text_bg_color,
			_params->font_name,
			_params->font_size,
			_params->hidden_text_flag,
			msgs,
			hiddens))
	return false;
    }
  
  // Display the text box, if requested

  if (_params->cloud_display_text_box)
    {
      if (!_displayTextBox(cloud, bbox, symprod))
	return false;
    }
  return true;
}

/*********************************************************************
 * _displayTextBox()
 */

bool CloudHandler::_displayTextBox(const SigwxCloud &cloud,
				   const BoundingBox &bbox,
				   Symprod *symprod) const
{
  static const string method_name = "CloudHandler::_displayTextBox()";
  
  // Determine where to put the text.  The point on the cloud where
  // the text box points will be the furthest east point.  The text box
  // will be positioned relative to this point.

  const vector< SigwxPoint > points = cloud.getPoints();
  
  if (points.size() == 0)
  {
    return true;
  }
  
  double cloud_pt_lat = points[0].lat;
  double cloud_pt_lon = points[0].lon;
  
  for (size_t i = 1; i < points.size(); ++i)
  {
    if (cloud_pt_lon < points[i].lon)
    {
      cloud_pt_lat = points[i].lat;
      cloud_pt_lon = points[i].lon;
    }
    
  }
  
  double lat_offset = bbox.pixelsToDegLat(_params->cloud_text_box_y_offset,
					  _params->cloud_screen_size);
  double lon_offset = bbox.pixelsToDegLon(_params->cloud_text_box_x_offset,
					  _params->cloud_screen_size);
  
  double box_ll_lat = cloud_pt_lat + lat_offset;
  double box_ll_lon = cloud_pt_lon + lon_offset;
  
  // If the text doesn't fall within the bounding box, don't render it.
  // Note that this isn't an error.

  if (!bbox.isInterior(box_ll_lat, box_ll_lon))
    return true;
  
  // Accumulate the text

  vector< string > text;
  
  // Cloud distribution

  switch (cloud.getCloudDist())
  {
  case SigwxCloud::DIST_SKY_CLEAR :
    text.push_back("CLR");
    break;
  case SigwxCloud::DIST_FEW :
    text.push_back("FEW");
    break;
  case SigwxCloud::DIST_SCATTERED :
    text.push_back("SCT");
    break;
  case SigwxCloud::DIST_BROKEN :
    text.push_back("BKN");
    break;
  case SigwxCloud::DIST_OVERCAST :
    text.push_back("OVC");
    break;
  case SigwxCloud::DIST_SCT_BKN :
    text.push_back("SCT");
    text.push_back("BKN");
    break;
  case SigwxCloud::DIST_BKN_OVC :
    text.push_back("BKN");
    text.push_back("OVC");
    break;
  case SigwxCloud::DIST_ISOLATED :
    text.push_back("ISOL");
    break;
  case SigwxCloud::DIST_ISOL_EMBED :
    text.push_back("ISOL");
    text.push_back("EMBD");
    break;
  case SigwxCloud::DIST_OCCASIONAL :
    text.push_back("OCNL");
    break;
  case SigwxCloud::DIST_OCNL_EMBED :
    text.push_back("OCNL");
    text.push_back("EMBD");
    break;
  case SigwxCloud::DIST_FREQUENT :
    text.push_back("FREQ");
    break;
  case SigwxCloud::DIST_DENSE :
    text.push_back("DNS");
    break;
  case SigwxCloud::DIST_LAYERS :
    text.push_back("LAYR");
    break;
  case SigwxCloud::DIST_UNKNOWN :
    text.push_back("UNK");
    break;
  } /* endswitch - cloud.getCloudDist() */
  
  // Cloud type

  switch (cloud.getCloudType())
  {
  case SigwxCloud::TYPE_CI :
    text.push_back("CI");
    break;
  case SigwxCloud::TYPE_CC :
    text.push_back("CC");
    break;
  case SigwxCloud::TYPE_CS :
    text.push_back("CS");
    break;
  case SigwxCloud::TYPE_AC :
    text.push_back("AC");
    break;
  case SigwxCloud::TYPE_AS :
    text.push_back("AS");
    break;
  case SigwxCloud::TYPE_NS :
    text.push_back("NS");
    break;
  case SigwxCloud::TYPE_SC :
    text.push_back("SC");
    break;
  case SigwxCloud::TYPE_ST :
    text.push_back("ST");
    break;
  case SigwxCloud::TYPE_CU :
    text.push_back("CU");
    break;
  case SigwxCloud::TYPE_CB :
    text.push_back("CB");
    break;
  case SigwxCloud::TYPE_CH :
    text.push_back("CH");
    break;
  case SigwxCloud::TYPE_UNKNOWN :
    text.push_back("UNK");
    break;
  } /* endswitch - cloud.getCloudType() */
  
  // Top of cloud area

  char text_line[1024];
  int flight_level;
  
  if (cloud.getCloudMaxHt() >= 0.0)
  {
    flight_level = (int)round(cloud.getCloudMaxHt() / M_PER_FT / 100.0);
    sprintf(text_line, "%d", flight_level);
    text.push_back(text_line);
  }
  else
  {
    text.push_back("XXX");
  }
  
  // Base of cloud area

  if (cloud.getCloudMinHt() >= 0.0)
  {
    flight_level = (int)round(cloud.getCloudMinHt() / M_PER_FT / 100.0);
    sprintf(text_line, "%d", flight_level);
    text.push_back(text_line);
  }
  else
  {
    text.push_back("XXX");
  }
  
  // Add the text box to the Symprod buffer

  _addTextBoxToSymprod(symprod, box_ll_lat, box_ll_lon,
		       cloud_pt_lat, cloud_pt_lon, text,
		       _params->cloud_text_color, _params->cloud_text_bg_color);
  
  return true;
}



/*********************************************************************
 * _displayList()
 */

bool CloudHandler::_displayList(const vector< SigwxPoint > &points,
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
  static const string method_name = "CloudHandler::_displayList()";
  
  // First create the polyline for symprod, so the labels can be rendered
  // on top later.

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
  delete[] waypoints;

  if (hidden_text_flag)
  {
    // Add central icon.

    _drawIcon(CLOUD_ICON_NAME.c_str(),
	      icon_def_list,
	      line_color.c_str(),
	      line_width,
	      avgLat,
	      avgLon,
	      _params->cloud_icon_scale,
	      false,
	      symprod);

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

void CloudHandler::_mkScallops(const double pt1_lat,
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
  static const string method_name = "CloudHandler::_mkScallops()";
  
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


