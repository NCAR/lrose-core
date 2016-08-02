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
#include <toolsa/TaArray.hh>
#include <euclid/GridPoint.hh>
#include <vector>
#include <set>
#include <stdio.h> // For the formatting of the time labels
#include <stdio.h>
#include <string.h>

#include "Server.hh"

/*********************************************************************
 * Constructor
 *
 * Inherits from DsSymprodServer
 */

Server::Server(const string &prog_name,
	       const Params *initialParams) :
  DsSymprodServer(prog_name,
		  initialParams->instance,
		  (void*)(initialParams),
		  initialParams->port,
		  initialParams->qmax,
		  initialParams->max_clients,
		  initialParams->no_threads,
		  initialParams->debug >= Params::DEBUG_NORM,
		  initialParams->debug >= Params::DEBUG_VERBOSE)
{
  
  _stationsLoaded = false;
  _posOccupied = NULL;

}

//////////////////////////////////////////////////////////////////////
// Destructor
//

Server::~Server()

{
  if (_posOccupied) {
    ufree2((void **) _posOccupied);
  }
}


//////////////////////////////////////////////////////////////////////
// loadLocalParams() - Load local params if they are to be overridden.
//

int Server::loadLocalParams(const string &paramFile,
			    void **serverParams)

{
  
  Params *localParams;
  char **tdrpOverrideList = NULL;
  bool expandEnvVars = true;

  const string routine_name = "loadLocalParams";
  
  if (_isDebug) {
    cerr << "Loading new params from file: " << paramFile << endl;
  }

  localParams = new Params(*((Params*)_initialParams));
  if (localParams->load((char*)paramFile.c_str(),
			tdrpOverrideList,
			expandEnvVars,
                        _isVerbose) != 0) {
    cerr << "ERROR - " << _executableName << "::" << routine_name << endl;
    cerr << "Cannot load parameter file: " << paramFile << endl;
    
    delete localParams;
    return -1;
  }

  
  if (_isVerbose) {
    localParams->print(stderr, PRINT_SHORT);
  }
  
  // Convert param values to Symprod object values

  _normalTextVertAlign = _vertAlign(localParams->normal_text_vert_align);
  _normalTextHorizAlign = _horizAlign(localParams->normal_text_horiz_align);
  _normalTextFontStyle = _fontStyle(localParams->normal_text_font_style);

  _hiddenTextVertAlign = _vertAlign(localParams->hidden_text_vert_align);
  _hiddenTextHorizAlign = _horizAlign(localParams->hidden_text_horiz_align);
  _hiddenTextFontStyle = _fontStyle(localParams->hidden_text_font_style);

  // set up vectors for accepted, rejected and required stations
  
  _acceptedCodes.clear();
  _rejectedCodes.clear();
  _requiredCodes.clear();

  if (localParams->useAcceptedStationsList) {
    for (int i = 0; i < localParams->acceptedStations_n; i++) {
      int code = 
	Spdb::hash4CharsToInt32(localParams->_acceptedStations[i]);
      _acceptedCodes.push_back(code);
      if (_isVerbose) {
	cerr << "Adding to accepted stations list: "
	     << localParams->_acceptedStations[i]
	     << ", code: " << code << endl;
      }
    } // i
  }
  
  if (localParams->useRejectedStationsList) {
    for (int i = 0; i < localParams->rejectedStations_n; i++) {
      int code = 
	Spdb::hash4CharsToInt32(localParams->_rejectedStations[i]);
      _rejectedCodes.push_back(code);
      if (_isVerbose) {
	cerr << "Adding to rejected stations list: "
	     << localParams->_rejectedStations[i]
	     << ", code: " << code << endl;
      }
    } // i
  }
  
  if (localParams->decimate_spatially &&
      localParams->decimate_required_stations_n > 0) {
    for (int i = 0; i < localParams->decimate_required_stations_n; i++) {
      int code = 
	Spdb::hash4CharsToInt32(localParams->_decimate_required_stations[i]);
      _requiredCodes.push_back(code);
    } // i
  }
  
  // Check that the station position is within the bounding box, if requested
  
  if (localParams->useBoundingBox && !_horizLimitsSet) {
    _minLat = localParams->bounding_box.min_lat;
    _minLon = localParams->bounding_box.min_lon;
    _maxLat = localParams->bounding_box.max_lat;
    _maxLon = localParams->bounding_box.max_lon;
    _horizLimitsSet = true;
  }
  
  if (_isDebug && _horizLimitsSet){
    cerr << "Horizontal limits set." << endl;
    cerr << "  Min lat: " << _minLat << endl;
    cerr << "  Min lon: " << _minLon << endl;
    cerr << "  Max lat: " << _maxLat << endl;
    cerr << "  Max lon: " << _maxLon << endl;
    cerr << endl;
  }
  
  if (localParams->decimate_spatially) {
    if (_posOccupied) {
      ufree2((void **) _posOccupied);
    }
    _posOccupied = (bool **) ucalloc2(localParams->decimate_n_lat,
                                      localParams->decimate_n_lon,
                                      sizeof(bool));
  }
  
  *serverParams = (void*)localParams;

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
  
  if (prod_id != SPDB_ASCII_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_ASCII_ID: " << SPDB_ASCII_ID << endl;
    return -1;
  }

  const string routine_name = "convertToSymprod";
  Params *serverParams = (Params*) params;

  // get the station name by dehashing the data type

  string stationId = Spdb::dehashInt32To4Chars(chunk_ref.data_type);

  if (_isVerbose) {
    cerr << "Starting to convert taf, stationId: " << stationId << endl;
  }

  // if needed, read read station locations


  string locationPath = serverParams->station_location_path;
  if (!_stationsLoaded) {
    if (_isDebug) {
      cerr << "Loading station info from file: "
	   << locationPath << endl;
    }
    _stationLocations.clear();
    if (_stationLocations.ReadData(locationPath.c_str())) {
      // cannot find locations, cannot proceed
      cerr << "ERROR - StnAscii2Symprod::Server" << endl;
      cerr << "  Cannot read station location data" << endl;
      cerr << "  File: " << locationPath << endl;
      return -1;
    } else {
      _stationsLoaded = true;
    }
    if (_isDebug) {
      cerr << "Loading station info from file - done" << endl;
    }
  }

  // get lat and lon of station

  double lat, lon, alt;
  string type;
  
  if (_stationLocations.FindPosition(stationId, lat, lon, alt, type)) {
    if (_isDebug) {
      cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
      cerr << "  Cannot locate station: " << stationId << endl;
    }
    return -1;
  }

  ///////////////////////
  // check for acceptance
  
  // Check that the station position is within the bounding box, if requested

  if (_horizLimitsSet) {
    
    // Normalize the longitude value

    while (lon < _minLon){
      lon += 360.0;
    }
    while (lon >= _minLon + 360.0){
      lon -= 360.0;
    }

    // Check to see if the report lies within the bounding box

    bool accept = false;

    if (lat >= _minLat && lat <= _maxLat &&
	lon >= _minLon && lon <= _maxLon) {
	accept = true;
    }

    if (!accept) {
      for (size_t is = 0; is < _requiredCodes.size(); is++) {
        if (_requiredCodes[is] == chunk_ref.data_type) {
          accept = true;
          break;
        }
      }
    }
    
  }

  if (_acceptedCodes.size() > 0) {
    bool accept = false;
    for (size_t is = 0; is < _acceptedCodes.size(); is++) {
      if (_acceptedCodes[is] == chunk_ref.data_type) {
	accept = true;
	break;
      }
    }
    if (!accept) {
      return -1;
    }
  } 

  if (_rejectedCodes.size() > 0) {
    for (size_t is = 0; is < _rejectedCodes.size(); is++) {
      if (_rejectedCodes[is] == chunk_ref.data_type) {
	return -1;
      }
    }
  }
  
  if (_horizLimitsSet &&
      serverParams->decimate_spatially && _posOccupied) {
    
    // decimation is active, so check to make sure that there is 
    // not already a product close by
    
    int ilat = (int) (((lat - _minLat) /
                       (_maxLat - _minLat))
                      * serverParams->decimate_n_lat);
    if (ilat < 0) {
      ilat = 0;
    }
    if (ilat > serverParams->decimate_n_lat - 1) {
      ilat = serverParams->decimate_n_lat - 1;
    }
    
    int ilon = (int) (((lon - _minLon) /
                       (_maxLon - _minLon))
                      * serverParams->decimate_n_lon);
    if (ilon < 0) {
      ilon = 0;
    }
    if (ilon > serverParams->decimate_n_lon - 1) {
      ilon = serverParams->decimate_n_lon - 1;
    }
    
    if (_posOccupied[ilat][ilon]) {
      if (_isVerbose) {
        cerr << "---->> Reject: lat, lon, ilat, ilon: "
             << lat << ", " << lon << ", "
             << ilat << ", " << ilon
             << endl;
      }
      return -1; // do not process this one.
    }
    if (_isVerbose) {
      cerr << "---->> Accept: lat, lon, ilat, ilon: "
           << lat << ", " << lon << ", "
           << ilat << ", " << ilon
           << endl;
    }
    _posOccupied[ilat][ilon] = true;
    
  }
  
  // get the text string

  string text;
  text.insert(0, (char *) spdb_data, spdb_len - 1);

  // create Symprod object

  time_t now = time(NULL);
  
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       SPDB_ASCII_LABEL);

  // normal text

  if (serverParams->plot_normal_text) {

    vector<string> lines;
    _tokenize(text, "\n\r", lines);

    int offset_y = serverParams->normal_text_offset.y;
    for (int ii = 0; ii < (int) lines.size(); ii++) {
      prod.addText(lines[ii].c_str(),
		   lat, lon,
		   serverParams->normal_text_foreground_color,
		   serverParams->normal_text_background_color,
		   serverParams->normal_text_offset.x,
		   offset_y,
		   _normalTextVertAlign,
		   _normalTextHorizAlign,
		   serverParams->normal_text_font_size,
		   _normalTextFontStyle,
		   serverParams->normal_text_font_name);
      offset_y -= serverParams->normal_text_line_offset;
    }
  }

  // hidden text

  if (serverParams->activate_hidden_text) {
    prod.addText(text.c_str(),
                 lat, lon,
                 serverParams->hidden_text_foreground_color,
                 serverParams->hidden_text_background_color,
                 serverParams->hidden_text_offset.x,
                 serverParams->hidden_text_offset.y,
                 _hiddenTextVertAlign,
                 _hiddenTextHorizAlign,
                 serverParams->hidden_text_font_size,
                 _hiddenTextFontStyle,
                 serverParams->hidden_text_font_name,
		 0, // Object ID
		 Symprod::DETAIL_LEVEL_USUALLY_HIDDEN | 
                 Symprod::DETAIL_LEVEL_SHOWAS_POPOVER |
                 Symprod::DETAIL_LEVEL_IGNORE_TRESHOLDS);
  }

  // icon

  if (serverParams->plot_icon) {
    _drawIcon(serverParams,
              serverParams->icon_color,
              lat, lon,
              serverParams->icon_scale,
              serverParams->icon_allow_client_scaling,
              prod);
  }

  if (_isVerbose) {
    prod.print(cerr);
  }

  // set return buffer

  prod.serialize(symprod_buf);

  return 0;
}

////////////////////////////////////////////////////////////////
// draw icon

void Server::_drawIcon(const Params *serverParams,
		       const char *color,
		       double centerLat,
		       double centerLon,
		       float iconScale,
		       bool allowClientScaling,
		       Symprod &prod)


{

  // create ICON shape

  int nPointsIcon = serverParams->icon_points_n;
  
  TaArray<Symprod::ppt_t> _ppts;
  Symprod::ppt_t *ppts = _ppts.alloc(nPointsIcon);
  
  for (int ii = 0; ii < nPointsIcon; ii++) {
    
    Params::icon_point_t point = serverParams->_icon_points[ii];
    
    if (point.x == -999 || point.y == -999) {
      
      ppts[ii].x = Symprod::PPT_PENUP;
      ppts[ii].y = Symprod::PPT_PENUP;
      
    } else {
      
      if (iconScale != 1.0) {
        
        double scaledX = point.x * iconScale;
        double scaledY = point.y * iconScale;
        
        ppts[ii].x = (int) floor(scaledX + 0.5);
        ppts[ii].y = (int) floor(scaledY + 0.5);

      } else {
        
        ppts[ii].x = point.x;
        ppts[ii].y = point.y;
        
      }
      
    }

  } // ii
  

  Symprod::wpt_t icon_origin;
  icon_origin.lat = centerLat;
  icon_origin.lon = centerLon;
  
  Symprod::detail_level_t detailLevel = Symprod::DETAIL_LEVEL_NONE;
  if (!allowClientScaling) {
    detailLevel = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
  }
  
  prod.addStrokedIcons(color,
                       nPointsIcon,
                       ppts,
                       1,
                       &icon_origin,
                       0,
                       detailLevel,
                       serverParams->icon_line_width);

}

//////////////////////////////////////////////
// tokenize a string into a vector of strings

void Server::_tokenize(const string &str,
		       const string &spacer,
		       vector<string> &toks)
  
{
    
  toks.clear();
  size_t pos = 0;
  while (true) {
    size_t start = str.find_first_not_of(spacer, pos);
    size_t end = str.find_first_of(spacer, start);
    if (start == string::npos) {
      return;
    } else if (end == string::npos) {
      string tok;
      tok.assign(str, start, string::npos);
      toks.push_back(tok);
      return;
    } else {
      string tok;
      tok.assign(str, start, end - start);
      toks.push_back(tok);
    }
    pos = end;
  }
}

///////////////////////////////////////////////////////////////////
// Convert param values to Symprod object values
  
Symprod::vert_align_t Server::_vertAlign(Params::vert_align_t align)

{
   
  switch (align) {

    case Params::VERT_ALIGN_TOP :
      return Symprod::VERT_ALIGN_TOP;
      break;
      
    case Params::VERT_ALIGN_CENTER :
      return Symprod::VERT_ALIGN_CENTER;
      break;
      
    case Params::VERT_ALIGN_BOTTOM :
      return Symprod::VERT_ALIGN_BOTTOM;
      break;

    default:
      return Symprod::VERT_ALIGN_CENTER;
      
  }

}

Symprod::horiz_align_t Server::_horizAlign(Params::horiz_align_t align)

{

  switch (align) {

    case Params::HORIZ_ALIGN_LEFT :
      return Symprod::HORIZ_ALIGN_LEFT;
      break;
      
    case Params::HORIZ_ALIGN_CENTER :
      return Symprod::HORIZ_ALIGN_CENTER;
      break;
      
    case Params::HORIZ_ALIGN_RIGHT :
      return Symprod::HORIZ_ALIGN_RIGHT;
      break;

    default:
      return Symprod::HORIZ_ALIGN_CENTER;
      
  }

}

Symprod::font_style_t Server::_fontStyle(Params::font_style_t style)

{
  
  switch (style) {

    case Params::TEXT_NORM :
      return Symprod::TEXT_NORM;
      break;
      
    case Params::TEXT_BOLD :
      return Symprod::TEXT_BOLD;
      break;
      
    case Params::TEXT_ITALICS :
      return Symprod::TEXT_ITALICS;
      break;
      
    case Params::TEXT_SUBSCRIPT :
      return Symprod::TEXT_SUBSCRIPT;
      break;
      
    case Params::TEXT_SUPERSCRIPT :
      return Symprod::TEXT_SUPERSCRIPT;
      break;
      
    case Params::TEXT_UNDERLINE :
      return Symprod::TEXT_UNDERLINE;
      break;
      
    case Params::TEXT_STRIKETHROUGH :
      return Symprod::TEXT_STRIKETHROUGH;
      break;

    default:
      return Symprod::TEXT_NORM;
      
  }

}

