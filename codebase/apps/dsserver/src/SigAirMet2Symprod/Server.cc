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
///////////////////////////////////////////////////////////////
// Server.cc
//
// File Server object
//
// Niles Oien, copying from Nancy Rehak,
//  RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 2003
//
///////////////////////////////////////////////////////////////


#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/pjg.h>
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
#include <vector>
#include <set>
#include <stdio.h> // For the formatting of the time labels
#include <stdio.h>
#include <cstring>

#include "Server.hh"
#include "IconDef.hh"


/*********************************************************************
 * Constructor
 *
 * Inherits from DsSymprodServer
 */

Server::Server(const string &prog_name,
	       Params *initialParams) :
  DsSymprodServer(prog_name,
		  initialParams->instance,
		  (void*)(initialParams),
		  initialParams->port,
		  initialParams->qmax,
		  initialParams->max_clients,
		  initialParams->no_threads,
		  initialParams->debug >= Params::DEBUG_NORM,
		  initialParams->debug >= Params::DEBUG_VERBOSE),
  _symprodFontStyle(Symprod::TEXT_NORM)
{
  // Do nothing
}


/*********************************************************************
 * loadLocalParams() - Load local params if they are to be overridden.
 */

int Server::loadLocalParams(const string &paramFile,
			    void **serverParams)
{
  Params  *localParams;
  char   **tdrpOverrideList = NULL;
  bool     expandEnvVars = true;

  const string routine_name = "_allocLocalParams";

  if (_isDebug)
    cerr << "Loading new params from file: " << paramFile << endl;

  localParams = new Params(*((Params*)_initialParams));
  if (localParams->load((char*)paramFile.c_str(),
			tdrpOverrideList,
			expandEnvVars,
                        _isVerbose) != 0)
  {
    cerr << "ERROR - " << _executableName << "::" << routine_name << endl;
    cerr << "Cannot load parameter file: " << paramFile << endl;
    
    delete localParams;
    return -1;
  }

  
  if (_isVerbose)
    localParams->print(stderr, PRINT_SHORT);

 // Convert param values to Symprod object values

  switch (localParams->text_vert_align)
  {
  case Params::VERT_ALIGN_TOP :
    _symprodVertAlign = Symprod::VERT_ALIGN_TOP;
    break;
    
  case Params::VERT_ALIGN_CENTER :
    _symprodVertAlign = Symprod::VERT_ALIGN_CENTER;
    break;
    
  case Params::VERT_ALIGN_BOTTOM :
    _symprodVertAlign = Symprod::VERT_ALIGN_BOTTOM;
    break;
  } /* endswitch - localParams->text_vert_align */

  switch (localParams->text_horiz_align)
  {
  case Params::HORIZ_ALIGN_LEFT :
    _symprodHorizAlign = Symprod::HORIZ_ALIGN_LEFT;
    break;
    
  case Params::HORIZ_ALIGN_CENTER :
    _symprodHorizAlign = Symprod::HORIZ_ALIGN_CENTER;
    break;
    
  case Params::HORIZ_ALIGN_RIGHT :
    _symprodHorizAlign = Symprod::HORIZ_ALIGN_RIGHT;
    break;
  } /* endswitch - localParams->text_horiz_align */
  
  switch (localParams->text_font_style)
  {
  case Params::TEXT_NORM :
    _symprodFontStyle = Symprod::TEXT_NORM;
    break;
    
  case Params::TEXT_BOLD :
    _symprodFontStyle = Symprod::TEXT_BOLD;
    break;
    
  case Params::TEXT_ITALICS :
    _symprodFontStyle = Symprod::TEXT_ITALICS;
    break;
    
  case Params::TEXT_SUBSCRIPT :
    _symprodFontStyle = Symprod::TEXT_SUBSCRIPT;
    break;
    
  case Params::TEXT_SUPERSCRIPT :
    _symprodFontStyle = Symprod::TEXT_SUPERSCRIPT;
    break;
    
  case Params::TEXT_UNDERLINE :
    _symprodFontStyle = Symprod::TEXT_UNDERLINE;
    break;
    
  case Params::TEXT_STRIKETHROUGH :
    _symprodFontStyle = Symprod::TEXT_STRIKETHROUGH;
    break;
  } /* endswitch - localParams->text_font_style */
  
  *serverParams = (void*)localParams;

  // Update objects dependent on the local parameters

  for (int i = 0; i < localParams->icon_defs_n; ++i)
  {
    // Create the list of icon points

    vector< GridPoint > point_list;
    
    char *x_string = strtok(localParams->_icon_defs[i].icon_points, " ");
    char *y_string;
    
    if (x_string == (char *)NULL)
    {
      cerr << "ERROR: Server::" << routine_name << endl;
      cerr << "Error in icon_points string for icon " <<
	localParams->_icon_defs[i].icon_name << endl;
      cerr << "The string must contain at least 1 point" << endl;
      
      continue;
    }
    
    bool string_error = false;
    
    while (x_string != (char *)NULL)
    {
      // Get the string representing the Y coordinate of the icon point

      y_string = strtok(NULL, " ");
      
      if (y_string == (char *)NULL)
      {
	cerr << "ERROR: Server::" << routine_name << endl;
	cerr << "Error in icon_points string for icon " <<
	  localParams->_icon_defs[i].icon_name << endl;
	cerr << "The string must contain an even number of values" << endl;
      
	string_error = true;
	
	break;
      }
      
      // Convert the string values to points

      GridPoint point(atoi(x_string), atoi(y_string));
      point_list.push_back(point);
      
      // Get the string representing the X coordinate of the icon point

      x_string = strtok(NULL, " ");
      
    } /* endwhile - x_string != (char *)NULL */
    
    // See if there was an error in the icon point processing

    if (string_error)
      continue;
    
    // Create the icon definition object and add it to our list

    string icon_name = localParams->_icon_defs[i].icon_name;
    
    IconDef *icon_def = new IconDef(icon_name, point_list);
    _iconDefList[icon_name] = icon_def;

  } /* endfor - i */

  return 0;
}


/*********************************************************************
 * convertToSymprod() - Convert the given data chunk from the SPDB
 *                      database to symprod format.
 *
 * Returns 0 on success, -1 on failure
 */

int Server::convertToSymprod(const void *params,
			     const string &dir_path,
			     const int prod_id,
			     const string &prod_label,
			     const Spdb::chunk_ref_t &chunk_ref,
                             const Spdb::aux_ref_t &aux_ref,
			     const void *spdb_data,
			     const int spdb_len,
			     MemBuf &symprod_buf)
{
  // check prod_id

  if (prod_id != SPDB_SIGAIRMET_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_SIGAIRMET_ID: " << SPDB_SIGAIRMET_ID << endl;
    return -1;
  }

  const string routine_name = "convertToSymprod";
  Params *serverParams = (Params*) params;

  // Convert the SPDB data to a weather hazards buffer

  SigAirMet sam;
  
  sam.disassemble(spdb_data, spdb_len);

  // Forget it if it has been cancelled.
  // This test is no longer used.
  // The SigAirMet object now has the expire time set to the cancel time.
  // if (sam.getCancelFlag()) return 0;

  // Forget it if it has no polygon, if
  // we are requiring polygons.

  if ((serverParams->requirePolygon) && (sam.getNVertices() < 2)) {
    return 0;
  }

  // Forget it if it is the wrong group.

  if ((serverParams->plotDataGroup == Params::PLOT_SIGMETS) &&
      (sam.getGroup() != SIGMET_GROUP)) {
    return 0;
  }

  if ((serverParams->plotDataGroup == Params::PLOT_AIRMETS) &&
      (sam.getGroup() != AIRMET_GROUP)) {
    return 0;
  }

  // Also forget it if it does not match a wildcard spec and
  // we are requiring this.

  int iMatch = -1;
  string WxString = sam.getWx();
  
  for (int iw=0; iw < serverParams->sigmet_items_n; iw++) {
    if (_WildCard(serverParams->_sigmet_items[iw].WxWildcard,
		  (char *)WxString.c_str(),0)) {
      iMatch = iw;
      break;
    }
  }

  if (iMatch == -1) {
    return 0;
  }

  // ignore if no centroid

  if (!(sam.centroidSet())) {
    if (sam.computeCentroid()) {
      return 0;
    }
  }

  // Get some variables pulled out of the wildcard match.

  bool plotID = serverParams->_sigmet_items[iMatch].plotID;
  bool plotWx = serverParams->_sigmet_items[iMatch].plotWx;
  bool plotFlightLevels =
    serverParams->_sigmet_items[iMatch].plotFlightLevels;
  bool plotSource = serverParams->_sigmet_items[iMatch].plotSource;
  bool plotTimes = serverParams->_sigmet_items[iMatch].plotTimes;
  bool plotText = serverParams->_sigmet_items[iMatch].plotText;
  bool renderPolygonSpokes =
    serverParams->_sigmet_items[iMatch].renderPolygonSpokes;
  bool renderPolygon = serverParams->_sigmet_items[iMatch].renderPolygon;
  bool includeHiddenPolygon =
    serverParams->_sigmet_items[iMatch].includeHiddenPolygon;
  bool renderForecasts = serverParams->_sigmet_items[iMatch].renderForecasts;
  bool renderOutlooks = serverParams->_sigmet_items[iMatch].renderOutlooks;
  bool renderIcon = serverParams->_sigmet_items[iMatch].renderIcon;
  char *iconName = serverParams->_sigmet_items[iMatch].iconName;
  char *iconNameSouth = serverParams->_sigmet_items[iMatch].iconNameSouth;
  float iconScale = serverParams->_sigmet_items[iMatch].iconScale;
  bool allowClientScaling =
    serverParams->_sigmet_items[iMatch].allowClientScaling;

  char *mainColor = serverParams->_sigmet_items[iMatch].mainColor;
  char *forecastColor = serverParams->_sigmet_items[iMatch].forecastColor;
  char *outlookColor = serverParams->_sigmet_items[iMatch].outlookColor;

  // create Symprod object

  time_t now = time(NULL);
  
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       SPDB_SIGAIRMET_LABEL);

  string text_string;
  int currentOffset = 0;
  
  // Get the centroid and normalize it
  
  double centLat = sam.getCentroidLat();
  double centLon = sam.getCentroidLon();
  double lonOffset = 0.0;

  if (_horizLimitsSet)
  {
    while (centLon < _minLon)
      centLon += 360.0;
    while (centLon >= _minLon + 360.0)
      centLon -= 360.0;

    lonOffset = centLon - sam.getCentroidLon();
  }

  // override the icon name if southern hemisphere
  
  if (centLat < 0 && strlen(iconNameSouth) > 0) {
    iconName = iconNameSouth;
  }

  // check icon name is available

  if (renderIcon &&
      (_iconDefList.find(iconName) == _iconDefList.end())) {
    cerr << "ERROR: Server::" << routine_name << endl;
    cerr << "Icon <" << iconName <<
      "> not found in icon list created from parameter file" << endl;
    cerr << "Not rendering icon" << endl;
    renderIcon = false;
  }
    
  // Get the vertices and normalize them

  vector<sigairmet_vertex_t> Vertices = sam.getVertices();

  for (size_t i = 0; i < Vertices.size(); ++i)
    Vertices[i].lon += lonOffset;

  // Debugging

  if (_isVerbose) {
    cerr << "Found a product to plot..." << endl;
    sam.print(cerr);
    cerr << "  flags: plotFL: " << plotFlightLevels
         << ", plotID plotSource: "  << plotID << " " << plotSource
         << ", plotTimes: " << plotTimes << endl;
    cerr << "         plotText: " << plotText
         << ", renderOutlooks: " << renderOutlooks
         << ", renderPolygon: " << renderPolygon
         << ", includeHiddenPolygon: " << includeHiddenPolygon << endl;
    cerr << "         renderIcon: " << renderIcon
         << ", iconName: " << iconName
         << ", icon scale: " << iconScale
         << ", allowClientScaling: " << allowClientScaling << endl;
  }

  // initialize movement direction

  double movementDirn = -1.0;

  // Plot the SIG/AIRMET. Order is important. Do outlooks, then forecasts,
  // then icon, then polygon, then text.
  
  // Render outlooks if set and if there are any
  // retrieve movement dirn if available

  bool outlookLabelsRendered = false;

  if (renderOutlooks) {

    double prevLat = centLat;
    double prevLon = centLon;

    const vector <sigairmet_forecast_t> &forecasts = sam.getForecasts();
    if (renderForecasts && forecasts.size() > 0) {
      prevLat = forecasts[forecasts.size() - 1].lat;
      prevLon = forecasts[forecasts.size() - 1].lon;
    }

    // draw outlooks
    
    _drawOutlooks(sam.getOutlooks(), serverParams,
                  outlookColor, prevLat, prevLon,
                  renderIcon, iconName, iconScale, allowClientScaling,
                  prod, movementDirn, outlookLabelsRendered);
    
  } // endif renderOutlooks

  // Render forecasts if set and if there are any
  // retrieve movement dirn if available

  bool forecastLabelsRendered = false;

  if (renderForecasts) {
    
    // draw the forecasts
    
    _drawForecasts(sam.getForecasts(), serverParams,
                   forecastColor, centLat, centLon,
                   renderIcon, iconName, iconScale, allowClientScaling,
                   prod, movementDirn, forecastLabelsRendered);
    
  } // endif renderOutlooks
  
  // Render the icon if set. Do this first so can set text offset below icon.

  if ((renderIcon == Params::ALWAYS_ON) ||
      ((renderIcon == Params::ON_IF_NO_POLYGON) && (Vertices.size() <= 1))) {
    
    // Set the scaling based on the polygon
    
    int clientCanScale=allowClientScaling;
    if ((allowClientScaling == Params::NO_SCALE_IF_NO_POLYGON) && 
	(Vertices.size() <= 1)) {
      clientCanScale=Params::DO_NOT_SCALE;
    }
    
    // Find the icon in the icon list. If not found, set render to off
    
    if (!_drawIcon(serverParams, iconName, mainColor,
                   centLat, centLon, iconScale, renderIcon,
                   clientCanScale, prod)) {
      renderIcon = Params::ALWAYS_OFF;
    }
    
  }
    
  // Render polygon if set

  if (renderPolygon || includeHiddenPolygon) {

    MemBuf ptBuf;
    int nPoints=0;

    for (int pt = 0; pt < (int) Vertices.size(); pt++) {
      Symprod::wpt_t wpt;
      wpt.lat = Vertices[pt].lat;
      wpt.lon = Vertices[pt].lon;
      ptBuf.add(&wpt, sizeof(wpt));
      nPoints++;
      if (pt == ((int)Vertices.size()-1)) {
	// Add first point at end to close polygon, if needed
        if (Vertices[pt].lat != Vertices[0].lat &&
            Vertices[pt].lon != Vertices[0].lon) {
          Symprod::wpt_t wpt;
          wpt.lat = Vertices[0].lat;
          wpt.lon = Vertices[0].lon;
          ptBuf.add(&wpt, sizeof(wpt));
          nPoints++;
        }
      }
    }

    if (renderPolygonSpokes){
      
      Symprod::wpt_t cent_wpt;
      cent_wpt.lat = centLat;
      cent_wpt.lon = centLon;

      for (unsigned int pt = 0; pt < Vertices.size(); pt++) {
	Symprod::wpt_t wpt;
	wpt.lat = Vertices[pt].lat;
	wpt.lon = Vertices[pt].lon;

	ptBuf.add(&cent_wpt, sizeof(cent_wpt));
	ptBuf.add(&wpt, sizeof(wpt));
	ptBuf.add(&cent_wpt, sizeof(cent_wpt));
	nPoints = nPoints + 3;
      }

    } // if (renderPolygonSpokes)

    if (renderPolygon) {
      if (sam.polygonIsFirBdry()) {
        prod.addPolyline(nPoints,
                         (Symprod::wpt_t *) ptBuf.getPtr(),
                         centLat, centLon,
                         mainColor,
                         _convertLineType(serverParams->fir_polygon_line_type),
                         serverParams->polygon_line_width,
                         _convertCapstyle(serverParams->polygon_capstyle),
                         _convertJoinstyle(serverParams->polygon_joinstyle));
      } else {
        prod.addPolyline(nPoints,
                         (Symprod::wpt_t *) ptBuf.getPtr(),
                         centLat, centLon,
                         mainColor,
                         _convertLineType(serverParams->polygon_line_type),
                         serverParams->polygon_line_width,
                         _convertCapstyle(serverParams->polygon_capstyle),
                         _convertJoinstyle(serverParams->polygon_joinstyle));
      }
    }

    if (includeHiddenPolygon) {

      Symprod::fill_t fillAlpha;
      if (serverParams->hidden_polygons_alpha_value < 0.05) {
        fillAlpha = Symprod::FILL_NONE;
      } else if (serverParams->hidden_polygons_alpha_value < 0.15) {
        fillAlpha = Symprod::FILL_ALPHA10;
      } else if (serverParams->hidden_polygons_alpha_value < 0.25) {
        fillAlpha = Symprod::FILL_ALPHA20;
      } else if (serverParams->hidden_polygons_alpha_value < 0.35) {
        fillAlpha = Symprod::FILL_ALPHA30;
      } else if (serverParams->hidden_polygons_alpha_value < 0.45) {
        fillAlpha = Symprod::FILL_ALPHA40;
      } else if (serverParams->hidden_polygons_alpha_value < 0.55) {
        fillAlpha = Symprod::FILL_ALPHA50;
      } else if (serverParams->hidden_polygons_alpha_value < 0.65) {
        fillAlpha = Symprod::FILL_ALPHA60;
      } else if (serverParams->hidden_polygons_alpha_value < 0.75) {
        fillAlpha = Symprod::FILL_ALPHA70;
      } else if (serverParams->hidden_polygons_alpha_value < 0.85) {
        fillAlpha = Symprod::FILL_ALPHA80;
      } else if (serverParams->hidden_polygons_alpha_value < 0.95) {
        fillAlpha = Symprod::FILL_ALPHA90;
      } else {
        fillAlpha = Symprod::FILL_SOLID;
      }

      if (sam.polygonIsFirBdry()) {
        prod.addPolyline(nPoints,
                         (Symprod::wpt_t *) ptBuf.getPtr(),
                         centLat, centLon,
                         mainColor,
                         _convertLineType(serverParams->fir_polygon_line_type),
                         serverParams->polygon_line_width,
                         _convertCapstyle(serverParams->polygon_capstyle),
                         _convertJoinstyle(serverParams->polygon_joinstyle),
                         true, fillAlpha,
                         0, // Object ID
                         Symprod::DETAIL_LEVEL_USUALLY_HIDDEN | 
                         Symprod::DETAIL_LEVEL_SHOWAS_POPOVER |
                         Symprod::DETAIL_LEVEL_IGNORE_TRESHOLDS );
      } else {
        prod.addPolyline(nPoints,
                         (Symprod::wpt_t *) ptBuf.getPtr(),
                         centLat, centLon,
                         mainColor,
                         _convertLineType(serverParams->polygon_line_type),
                         serverParams->polygon_line_width,
                         _convertCapstyle(serverParams->polygon_capstyle),
                         _convertJoinstyle(serverParams->polygon_joinstyle),
                         true, fillAlpha,
                         0, // Object ID
                         Symprod::DETAIL_LEVEL_USUALLY_HIDDEN | 
                         Symprod::DETAIL_LEVEL_SHOWAS_POPOVER |
                         Symprod::DETAIL_LEVEL_IGNORE_TRESHOLDS );
      }
    }

  } /* End of if (renderPolygon) */


  //
  // Render text
  // 

  vector<string> textLines;

  // render observed location if there are fcasts or outlooks

  int textOffsetX = serverParams->text_offset.x;
  int textOffsetY = serverParams->text_offset.y;

  if (outlookLabelsRendered || forecastLabelsRendered) {
    
    // label

    if (movementDirn >= 0.0) {
      _computeMovementTextOffsets(serverParams,
                                  movementDirn,
                                  textOffsetX,
                                  textOffsetY);
    }
    
    // compute the text
    
    char obsFcastStr[128];
    string label;
    DateTime obsFcastTime;
    if (sam.getObsTime() == 0) {
      if (sam.getFcastTime() == 0) {
	obsFcastTime.set(sam.getIssueTime());
      } else {
	label = "FCST";
	obsFcastTime.set(sam.getFcastTime());
      }
    } else {
      label = "OBS";
      obsFcastTime.set(sam.getObsTime());
    }

    if (serverParams->label_forecast_and_outlook_times) {
      label += " %.2dZ";
      sprintf(obsFcastStr, label.c_str(), obsFcastTime.getHour());
    } else {
      sprintf(obsFcastStr, "%.2dZ", obsFcastTime.getHour());
    }

    textLines.push_back(obsFcastStr);
    
  }
      
  // Plot the helpful text if using FIR as boundary
  
  if (serverParams->plotPolygonIsFirBoundary_PrintFirName &&
      sam.polygonIsFirBdry()) {
    text_string = "Using FIR bdry: ";
    text_string = text_string + sam.getFir();
    textLines.push_back(text_string);
  }

  // Plot the weather type if set

  if (plotWx) {
    string wx_string = sam.getWx();
    if (wx_string == "UNKNOWN") {
      wx_string = serverParams->unknown_wx_text;
    }
    textLines.push_back(wx_string);
  }
  
  // Plot the flight levels if set

  if (plotFlightLevels){
    text_string = "FL: ";
    if (sam.flightLevelsSet()){
      char fl_str[32];
      if (sam.getBottomFlightLevel() == 0){
	sprintf(fl_str, "SFC to %g", 
		sam.getBottomFlightLevel());
      }
      else {
	sprintf(fl_str, "%g to %g", 
		sam.getBottomFlightLevel(),
		sam.getTopFlightLevel());
      }
       text_string = text_string + fl_str;
    } else {
      text_string = text_string + "UNKNOWN";
    }
    textLines.push_back(text_string);
  }

  // Plot the source and ID if set

  if ((plotID) || (plotSource)) {
    text_string = sam.getSource();
    text_string = text_string +  ":";
    text_string = text_string + sam.getId();
    textLines.push_back(text_string);
  }

  // Plot the start/end times if set

  if (plotTimes) {
    text_string = "Start : ";
    text_string = text_string + DateTime::strm(sam.getStartTime());
    textLines.push_back(text_string);
    text_string = "Expire: ";
    text_string = text_string + DateTime::strm(sam.getEndTime());
    textLines.push_back(text_string);
  }

  // Plot the text if set

  if (plotText){
    
    int textLeft = 1;
    int first = 1;
    
    int istart = 0;
    int iend;
    do{
      //
      // Fill the buffer.
      //
      char buffer[2048];
      sprintf(buffer,"%s",sam.getText().c_str());
      //
      // Find the first space in the text.
      //
      int keepParsing = 1;
      iend = istart;
      
      do {
	//
	// See if we are at the end of the sting.
	//
	if  (buffer[iend] == char(0)){
	  textLeft = 0;
	  keepParsing = 0;
	  break;
	}
	
	if (iend-istart > (serverParams->plotTextLineLen-1)){
	  if  (buffer[iend] == ' '){
	    buffer[iend] = char(0);
	    keepParsing = 0;
	    break;
	  }
	}
	iend++;
      } while(keepParsing);


     if (first){
       first = 0;
       text_string = "MESSAGE: ";
     } else {
       text_string = "  ";
     }

     text_string = text_string + (buffer + istart);
     textLines.push_back(text_string);

     //
     // Progress to the next line.
     //
     istart = iend + 1;
     iend = istart;

    } while(textLeft);

  }
  
  int lineOffset = abs(serverParams->plot_text_line_offset);
  int lineSpacing = abs((int) (serverParams->text_font_size * 1.3 + 0.5));
  if (textOffsetY < 0) {
    lineOffset *= -1;
    lineSpacing *= -1;
  }
  currentOffset = lineOffset;

  Symprod::vert_align_t vertAlign = _symprodVertAlign;
  Symprod::horiz_align_t horizAlign = _symprodHorizAlign;
  if (outlookLabelsRendered || forecastLabelsRendered) {
    horizAlign = Symprod::HORIZ_ALIGN_LEFT;
    vertAlign = Symprod::VERT_ALIGN_CENTER;
  }

  if (lineOffset > 0) {
    for (int ii = (int) textLines.size() - 1; ii >= 0; ii--) {
      prod.addText(textLines[ii].c_str(),
                   centLat, centLon,
                   mainColor, serverParams->text_background_color,
                   textOffsetX, 
                   textOffsetY + currentOffset,
                   vertAlign, horizAlign,
                   serverParams->text_font_size,
                   _symprodFontStyle, serverParams->font_name);
      currentOffset = currentOffset + lineSpacing;
    }
  } else {
    for (int ii = 0; ii < (int) textLines.size(); ii++) {
      prod.addText(textLines[ii].c_str(),
                   centLat, centLon,
                   mainColor, serverParams->text_background_color,
                   textOffsetX, 
                   textOffsetY + currentOffset,
                   vertAlign, horizAlign,
                   serverParams->text_font_size,
                   _symprodFontStyle, serverParams->font_name);
      currentOffset = currentOffset + lineSpacing;
    }
  }

  // add message as hidden text

  if (serverParams->activate_hidden_text) {
    char *foreground_color = serverParams->hidden_text_foreground_color;
    if (strlen(foreground_color) == 0) {
      foreground_color = mainColor;
    }
    prod.addText(sam.getText().c_str(),
		 centLat, centLon,
                 foreground_color,
		 serverParams->hidden_text_background_color,
		 serverParams->hidden_text_x_offset,
		 serverParams->hidden_text_y_offset,
		 (Symprod::vert_align_t)
		 serverParams->hidden_text_vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->hidden_text_horiz_align,
		 serverParams->hidden_text_font_size,
		 Symprod::TEXT_NORM,
		 serverParams->font_name,
		 0, // Object ID
		 Symprod::DETAIL_LEVEL_USUALLY_HIDDEN | 
                 Symprod::DETAIL_LEVEL_SHOWAS_POPOVER |
                 Symprod::DETAIL_LEVEL_IGNORE_TRESHOLDS );
  }
  
  // set return buffer

  if (_isVerbose) {
    prod.print(cerr);
  }

  prod.serialize(symprod_buf);

  return 0;
}

//////////////////////////////////////////////////////////////////////
// Convert the TDRP capstyle parameter to the matching symprod value.

Symprod::capstyle_t Server::_convertCapstyle(int capstyle)
{
  switch(capstyle)
  {
  case Params::CAPSTYLE_BUTT :
    return(Symprod::CAPSTYLE_BUTT);
    
  case Params::CAPSTYLE_NOT_LAST :
    return(Symprod::CAPSTYLE_NOT_LAST);
    
  case Params::CAPSTYLE_PROJECTING :
    return(Symprod::CAPSTYLE_PROJECTING);

  case Params::CAPSTYLE_ROUND :
    return(Symprod::CAPSTYLE_ROUND);
  }
  
  return(Symprod::CAPSTYLE_BUTT);
}


//////////////////////////////////////////////////////////////////////
// Convert the TDRP joinstyle parameter to the matching symprod value.

Symprod::joinstyle_t Server::_convertJoinstyle(int joinstyle)
{
  switch(joinstyle)
  {
  case Params::JOINSTYLE_BEVEL :
    return(Symprod::JOINSTYLE_BEVEL);
    
  case Params::JOINSTYLE_MITER :
    return(Symprod::JOINSTYLE_MITER);
    
  case Params::JOINSTYLE_ROUND :
    return(Symprod::JOINSTYLE_ROUND);
  }
  
  return(Symprod::JOINSTYLE_BEVEL);
}


//////////////////////////////////////////////////////////////////////
// Convert the TDRP line type parameter to the matching symprod value.

Symprod::linetype_t Server::_convertLineType(int line_type)
{
  switch(line_type)
  {
  case Params::LINETYPE_SOLID :
    return(Symprod::LINETYPE_SOLID);
    
  case Params::LINETYPE_DASH :
    return(Symprod::LINETYPE_DASH);
    
  case Params::LINETYPE_DOT_DASH :
    return(Symprod::LINETYPE_DOT_DASH);
  }
  
  return(Symprod::LINETYPE_SOLID);
}

//////////////////////////////////////////////////////////////////////
// Wildcard checker.

int Server::_WildCard(char *WildSpec, char *String, int debug){

  //
  // WildCard returns zero if a wild card specification is not
  // matched, 1 if there is a match.
  //
  // You pass in :
  //
  // * WildSpec - the wild carded specification, ie.
  //              "*.dat" or "20*.mdv"
  //
  // * String - the string to test against the wildcard,
  //            ie. "Station.dat" or "200402.mdv"
  //
  //
  // * debug - if non-zero, then messages are printed to stderr
  //           explaining the string parsing as it happens.
  //


  int istart = 0;
  int iend = 0;

  char *searchString = String;

  do {
    //
    // Position istart at first non-wildcard.
    //

    char buffer[1024];
    do{
      //
      // If it is a wildcard, skip it.
      //
      if (WildSpec[istart] == '*'){
	istart++;
      }

      //
      // If we have reached the end of the string, return OK.
      //
      if (WildSpec[istart] == char(0)){
	return 1;
      }

      //
      // If it is not a wildcard, copy it into the buffer.
      //
      if (
	  (WildSpec[istart] != '*') &&
	  (WildSpec[istart] != char(0))
	  ){
	//
	// Find the end of this item.
	//
	iend = istart;
	do {
	  iend++;

	} while (
		 (WildSpec[iend] != '*') &&
		 (WildSpec[iend] != char(0))
		 );

	for (int i=istart; i < iend; i++){
	  buffer[i-istart] = WildSpec[i];
	  buffer[i-istart+1] = char(0); 
	}

	if (debug) fprintf(stderr,"ITEM : %s : ",buffer);

	int MustMatchExact = 0;
	if (WildSpec[iend] == char(0)){
	   MustMatchExact = 1;
	}


	if (MustMatchExact){
	  if (debug) fprintf(stderr, "MUST MATCH EXACT : ");

	  if (strlen(searchString) <  strlen(buffer)){
	    //
	    // What we are looking for cannot be here - string too short.
	    //
	    if (debug) fprintf(stderr, "IT CANNOT.\n");
	    return 0;
	  }

	  char *p = searchString + strlen(searchString) - strlen(buffer);
	  if (debug) fprintf(stderr,"%s cf. %s : ",
		  buffer, p);
	  if (!(strcmp(buffer,p))){
	    //
	    // It is an exact match - at the end of the wild card.
	    // We have a match.
	    //
	    if (debug) fprintf(stderr,"IT DOES.\n");
	    return 1;
	    //
	  } else {
	    //
	    // No match - a failure.
	    //
	    if (debug) fprintf(stderr,"IT DOES NOT.\n");
	    return 0;
	  }
	}

	if (istart == 0){
	  if (debug) fprintf(stderr, "MUST BE THE START OF %s: ", searchString);
	  if (!(strncmp(buffer, searchString,strlen(buffer)))){
	    if (debug) fprintf(stderr,"IT IS.\n");
	    searchString = searchString + strlen(buffer);
	  } else {
	    if (debug) fprintf(stderr,"IT IS NOT.\n");
	    return 0;
	  }
	}



	if (istart != 0){
	  if (debug) fprintf(stderr, "MUST BE PRESENT IN %s: ", searchString);
	  char *p = strstr(searchString, buffer);
	  if (p == NULL){
	    if (debug) fprintf(stderr,"IT IS NOT.\n");
	    return 0;
	  } else {
	    if (debug) fprintf(stderr,"IT IS.\n");
	    searchString = p + strlen(buffer);
	  }
	}


	//
	// If that was the last item, return success.
	//
	if (WildSpec[iend] == char(0)){
	  return 1;
	}
	
	istart = iend;
      }

    } while (1);

  } while (1);


}


////////////////////////
// compute the centroid

void Server::_computeCentroid(const vector <double> &lats, 
			      const vector <double> &lons,
			      double &centroidLat,
			      double &centroidLon)
{

  if (lats.size() < 1) {
    
    centroidLat = 0.0;
    centroidLon = 0.0;
    
  } else {

    // condition the longitudes to avoid problems crossing the
    // 180 or 0 degree line

    vector <double> conditionLons;
    conditionLons.clear();
    conditionLons=lons;
    _conditionLongitudes(conditionLons);

    // compute means

    double sumLon = 0.0;
    double sumLat = 0.0;
    for (size_t ii = 0; ii < lats.size(); ii++) {
      sumLon += conditionLons[ii];
      sumLat += lats[ii];
    }
    
    centroidLon = sumLon / lons.size();
    centroidLat = sumLat / lats.size();

  }
}

////////////////////////////////////////////////////////////////
// condition the longitude values to avoid problems crossing the
// 180 or 0 degree line


void Server::_conditionLongitudes(vector <double> &lons)

{
  // compute min and max longitudes

  double min_lon = lons[0];
  double max_lon = min_lon;
  
  for (size_t ii = 1; ii < lons.size(); ii++) {
    min_lon = MIN(min_lon, lons[ii]);
    max_lon = MAX(max_lon, lons[ii]);
  }

  // check if longitudes span either the 0 or 180 lines

  if (fabs(min_lon - max_lon) < 180) {
    return;
  }
    
  if (min_lon < 0) {

    // spans the 180 line

    for (size_t ii = 0; ii < lons.size(); ii++) {
      if (fabs(min_lon - lons[ii]) < 180) {
	lons[ii] += 360.0;
      }
    }
    
  } else {
    
    // spans the 0 line
    
    for (size_t ii = 0; ii < lons.size(); ii++) {
      if (fabs(max_lon - lons[ii]) < 180) {
	lons[ii] -= 360.0;
      }
    }

  } // if (min_lon < 0)

}

////////////////////////////////////////////////////////////////
// draw the forecasts
// 

void Server::_drawForecasts(const vector <sigairmet_forecast_t> &forecasts,
                            const Params *serverParams,
                            const char *colorToUse,
                            const double centerLat,
                            const double centerLon,
                            const int renderIcon,
                            const char *iconName,
                            const float iconScale,
                            const int allowClientScaling,
                            Symprod &prod,
                            double &movementDirn,
                            bool &labelsRendered)
{

  // find sets of forecast times

  set<time_t, less<time_t> > fcastTimes;
  for (int ii = 0; ii < (int) forecasts.size(); ii++) {
    fcastTimes.insert(fcastTimes.begin(), forecasts[ii].time);
  }
  
  if (fcastTimes.size() <= 0) {
    return;
  }

  // loop through forecast times

  double prevCenterLat = centerLat;
  double prevCenterLon = centerLon;

  for (set<time_t>::iterator jj = fcastTimes.begin();
       jj != fcastTimes.end(); jj++) {

    // compile lat/lon polygon for this forecast time

    vector<double> lats, lons;
    time_t ftime = *jj;
    // int id;
    for (size_t ii = 0; ii < forecasts.size(); ii++) {
      if (forecasts[ii].time == ftime) {
        lats.push_back(forecasts[ii].lat);
        lons.push_back(forecasts[ii].lon);
        // id = forecasts[ii].id;
      }
    } // ii
    
    // Get the centroid for the forecast
    
    double fLat, fLon;
    _computeCentroid(lats, lons, fLat, fLon);

    // add polyline if more than 1 point in forecast
    
    if (lats.size() > 1) {
      
      // set up vertex buffer
      
      MemBuf ptBuf;
      int nPoints=0;
      for (int pt = 0; pt < (int) lats.size(); pt++) {
        Symprod::wpt_t wpt;
        wpt.lat = lats[pt];
        wpt.lon = lons[pt];
        ptBuf.add(&wpt, sizeof(wpt));
        nPoints++;
        if (pt == ((int)lats.size()-1)) {
          // Add first point at end to close polygon, if needed
          if (lats[pt] != lats[0] && lons[pt] != lons[0]) {
            wpt.lat = lats[0];
            wpt.lon = lons[0];
            ptBuf.add(&wpt, sizeof(wpt));
            nPoints++;
          }
        }
      } // pt
      
      prod.addPolyline(nPoints,
                       (Symprod::wpt_t *) ptBuf.getPtr(),
                       colorToUse,
                       _convertLineType(serverParams->forecast_line_type),
                       serverParams->polygon_line_width,
                       _convertCapstyle(serverParams->polygon_capstyle),
                       _convertJoinstyle(serverParams->polygon_joinstyle));
      
    } else {

      // for single point forecasts,
      // draw an arrow connecting the previous polygon centroid
      // to this centroid
      
      int head_len_in_pixels = serverParams->forecast_arrow_head_len_pixels;
      double head_half_angle_in_deg =
        serverParams->forecast_arrow_head_half_angle;
      int object_id = 0;
      int detail_level = 0;
      
      prod.addArrowBothPts(colorToUse,
                           _convertLineType(serverParams->polygon_line_type),
                           serverParams->polygon_line_width,
                           _convertCapstyle(serverParams->polygon_capstyle),
                           _convertJoinstyle(serverParams->polygon_joinstyle),
                           prevCenterLat, prevCenterLon,
                           fLat, fLon,
                           head_len_in_pixels,
                           head_half_angle_in_deg,
                           object_id,
                           detail_level);
      
    } // if (lats.size() ...
    
    // compute movement
    
    double rr, theta;
    PJGLatLon2RTheta(prevCenterLat, prevCenterLon, fLat, fLon,
                     &rr, &theta);
    if (theta < 0) {
      theta += 360.0;
    }
    movementDirn = theta;
    
    // Draw the icon at the center if set
    
    if ((renderIcon == Params::ALWAYS_ON) ||
        ((renderIcon == Params::ON_IF_NO_POLYGON) && (lats.size() == 1))) {
      
      // Set the scaling based on the polygon

      int clientCanScale=allowClientScaling;
      if ((allowClientScaling == Params::NO_SCALE_IF_NO_POLYGON) && 
	  (lats.size() <= 1)) {
	clientCanScale=Params::DO_NOT_SCALE;
      }
      _drawIcon(serverParams, iconName, colorToUse,
                fLat, fLon, iconScale, renderIcon, clientCanScale, prod);

      if (serverParams->render_forecast_and_outlook_times) {
        
        // render times
        
        int offsetX, offsetY;
        _computeMovementTextOffsets(serverParams,
                                    movementDirn,
                                    offsetX,
                                    offsetY);

        // compute the text
        
        char fStr[128];
        DateTime Ftime(ftime);
        if (serverParams->label_forecast_and_outlook_times) {
          sprintf(fStr, "FCST %.2dZ", Ftime.getHour());
        } else {
          sprintf(fStr, "%.2dZ", Ftime.getHour());
        }
    
        prod.addText(fStr, fLat, fLon,
                     colorToUse, serverParams->text_background_color,
                     offsetX, offsetY,
                     Symprod::VERT_ALIGN_CENTER, Symprod::HORIZ_ALIGN_LEFT,
                     serverParams->text_font_size,
                     _symprodFontStyle, serverParams->font_name);
        
        labelsRendered = true;
        
      } // if (serverParams->render_forecast_and_outlook_times)
      
    } // if ((renderIcon == Params::ALWAYS_ON) ...

    // save centroid

    prevCenterLat = fLat;
    prevCenterLon = fLon;
    
  } // jj

}

////////////////////////////////////////////////////////////////
// draw the outlooks
// 

void Server::_drawOutlooks(const vector <sigairmet_forecast_t> &outlooks,
                           const Params *serverParams,
                           const char *colorToUse,
                           const double centerLat,
                           const double centerLon,
                           const int renderIcon,
                           const char *iconName,
                           const float iconScale,
                           const int allowClientScaling,
                           Symprod &prod,
                           double &movementDirn,
                           bool &labelsRendered)
{
  
  // find sets of outlook times
  
  set<time_t, less<time_t> > outlookTimes;
  for (int ii = 0; ii < (int) outlooks.size(); ii++) {
    outlookTimes.insert(outlookTimes.begin(), outlooks[ii].time);
  }
  
  if (outlookTimes.size() <= 0) {
    return;
  }

  // loop through the outlook times
  
  double prevCenterLat = centerLat;
  double prevCenterLon = centerLon;
  
  for (set<time_t>::iterator jj = outlookTimes.begin();
       jj != outlookTimes.end(); jj++) {
    
    // compile lat/lon polygon for this outlook time
    
    vector<double> lats, lons;
    time_t ftime = *jj;
    // int id;
    for (size_t ii = 0; ii < outlooks.size(); ii++) {
      if (outlooks[ii].time == ftime) {
        lats.push_back(outlooks[ii].lat);
        lons.push_back(outlooks[ii].lon);
        // id = outlooks[ii].id;
      }
    } // ii
    
    // Get the centroid for the outlook

    double fLat, fLon;
    _computeCentroid(lats, lons, fLat, fLon);
    
    // add polyline if more than 1 point in outlook
    
    if (lats.size() > 1) {
      
      // set up vertex buffer
      
      MemBuf ptBuf;
      int nPoints=0;
      for (int pt = 0; pt < (int) lats.size(); pt++) {
        Symprod::wpt_t wpt;
        wpt.lat = lats[pt];
        wpt.lon = lons[pt];
        ptBuf.add(&wpt, sizeof(wpt));
        nPoints++;
        if (pt == ((int)lats.size()-1)) {
          // Add first point at end to close polygon, if needed
          if (lats[pt] != lats[0] && lons[pt] != lons[0]) {
            wpt.lat = lats[0];
            wpt.lon = lons[0];
            ptBuf.add(&wpt, sizeof(wpt));
            nPoints++;
          }
        }
      } // pt
      
      prod.addPolyline(nPoints,
                       (Symprod::wpt_t *) ptBuf.getPtr(),
                       colorToUse,
                       _convertLineType(serverParams->outlook_line_type),
                       serverParams->polygon_line_width,
                       _convertCapstyle(serverParams->polygon_capstyle),
                       _convertJoinstyle(serverParams->polygon_joinstyle));
      
    } else {

      // for single point outlooks,
      // draw an arrow connecting the previous polygon centroid
      // to this centroid
      
      int head_len_in_pixels = serverParams->outlook_arrow_head_len_pixels;
      double head_half_angle_in_deg =
        serverParams->outlook_arrow_head_half_angle;
      int object_id = 0;
      int detail_level = 0;
      
      prod.addArrowBothPts(colorToUse,
                           _convertLineType(serverParams->polygon_line_type),
                           serverParams->polygon_line_width,
                           _convertCapstyle(serverParams->polygon_capstyle),
                           _convertJoinstyle(serverParams->polygon_joinstyle),
                           prevCenterLat, prevCenterLon,
                           fLat, fLon,
                           head_len_in_pixels,
                           head_half_angle_in_deg,
                           object_id,
                           detail_level);
      
    } // if (lats.size() ... 

    // compute movement
    
    double rr, theta;
    PJGLatLon2RTheta(prevCenterLat, prevCenterLon, fLat, fLon,
                     &rr, &theta);
    if (theta < 0) {
      theta += 360.0;
    }
    movementDirn = theta;
    
    // Draw the icon at the centroid if needed
    
    if ((renderIcon == Params::ALWAYS_ON) ||
        ((renderIcon == Params::ON_IF_NO_POLYGON) && (lats.size() == 1))) {
      
      // Set the scaling based on the polygon

      int clientCanScale=allowClientScaling;
      if ((allowClientScaling == Params::NO_SCALE_IF_NO_POLYGON) && 
	  (lats.size() <= 1)) {
	clientCanScale=Params::DO_NOT_SCALE;
      }
      _drawIcon(serverParams, iconName, colorToUse,
                fLat, fLon, iconScale, renderIcon, clientCanScale, prod);

      if (serverParams->render_forecast_and_outlook_times) {

        // render times

        int offsetX, offsetY;
        _computeMovementTextOffsets(serverParams,
                                    movementDirn,
                                    offsetX,
                                    offsetY);

        // compute the text
        
        char oStr[128];
        DateTime Ftime(ftime);
        if (serverParams->label_forecast_and_outlook_times) {
          sprintf(oStr, "OTLK %.2dZ", Ftime.getHour());
        } else {
          sprintf(oStr, "%.2dZ", Ftime.getHour());
        }
    
        prod.addText(oStr, fLat, fLon,
                     colorToUse, serverParams->text_background_color,
                     offsetX, offsetY,
                     Symprod::VERT_ALIGN_CENTER, Symprod::HORIZ_ALIGN_LEFT,
                     serverParams->text_font_size,
                     _symprodFontStyle, serverParams->font_name);

        labelsRendered = true;
        
      } // if (serverParams->render_forecast_and_outlook_times)
      
    } // if ((renderIcon == Params::ALWAYS_ON) ...

    // save centroid

    prevCenterLat = fLat;
    prevCenterLon = fLon;
    
  } // jj

}

////////////////////////////////////////////////////////////////
// compute text offsets based on movement
// 

void Server::_computeMovementTextOffsets(const Params *serverParams,
                                         double dirn,
                                         int &offsetX,
                                         int &offsetY)

{

  if (dirn > 90 && dirn < 270) {
    dirn += 180;
  }
  
  double dx = cos(dirn * DEG_TO_RAD) * serverParams->text_font_size * 0.5;
  double dy = sin(dirn * DEG_TO_RAD) * serverParams->text_font_size * -1.5;

  offsetX = (int) (dx + 0.5);
  offsetY = (int) (dy + 0.5);

}

////////////////////////////////////////////////////////////////
// draw an icon
// 
// Return true if able to render the icon and is set, return
// false if unable to render the icon and is set.
// Return true if render turned off.

bool Server::_drawIcon(const Params *serverParams,
		       const char *iconName,
		       const char *colorToUse,
		       const double centerLat,
		       const double centerLon,
		       const float iconScale,
		       const int renderIcon,
		       const int allowClientScaling,
		       Symprod &prod)


{
  string routine_name="drawIcon";
  
  if (renderIcon == Params::ALWAYS_OFF) {
    return true;
  }

  if (_iconDefList.find(iconName) == _iconDefList.end()) {
    cerr << "ERROR: Server::" << routine_name << endl;
    cerr << "Icon <" << iconName <<
      "> not found in icon list created from parameter file" << endl;
    cerr << "Not rendering icon" << endl;
	
    // set render to OFF
    return false;
  }
    
  IconDef *icon_def = _iconDefList[iconName];

  // Apply the scale factor and create the array of points
  // also get the icon width so we can offset the text correctly

  Symprod::ppt_t *iconPoints = icon_def->getPointList();
  Symprod::ppt_t *scaledPoints = (Symprod::ppt_t *)
    umalloc(icon_def->getNumPoints() * sizeof(Symprod::ppt_t));

  int min = 0, max = 0, check_pt;
  bool isFirst=true;

  for (int ii = 0; ii<icon_def->getNumPoints(); ii++) {

    // Apply the scale factor

    if (iconPoints[ii].x != Symprod::PPT_PENUP &&
        iconPoints[ii].y != Symprod::PPT_PENUP) {
      
      double scaledX = iconPoints[ii].x * iconScale;
      double scaledY = iconPoints[ii].y * iconScale;
      
      scaledPoints[ii].x = (int) scaledX;
      scaledPoints[ii].y = (int) scaledY;
      
    } else {
      
      scaledPoints[ii].x = iconPoints[ii].x;
      scaledPoints[ii].y = iconPoints[ii].y;

    }

    // Get the min max

    bool doCheck=true;
    check_pt=scaledPoints[ii].y;
    if (check_pt == Symprod::PPT_PENUP) {
      doCheck=false;
    }
    
    if (doCheck) {
      if (isFirst) {
	min=check_pt;
	max=check_pt;
	isFirst=false;
      } else {
	if (check_pt < min) {
	  min=check_pt;
	}
	if (check_pt > max) {
	  max=check_pt;
	}
      }
    }
  }

  Symprod::wpt_t icon_origin;
	
  icon_origin.lat = centerLat;
  icon_origin.lon = centerLon;

  if (allowClientScaling == Params::DO_NOT_SCALE) {
    prod.addStrokedIcons(colorToUse,
			 icon_def->getNumPoints(),
			 scaledPoints,
			 1,
			 &icon_origin,
			 0,
			 Symprod::DETAIL_LEVEL_DO_NOT_SCALE,
			 serverParams->line_width);
  } else {
    prod.addStrokedIcons(colorToUse,
			 icon_def->getNumPoints(),
			 scaledPoints,
			 1,
			 &icon_origin,
			 0,
			 0,
			 serverParams->line_width);
  }

  ufree(scaledPoints);

  // Done
  
  return true;
}
