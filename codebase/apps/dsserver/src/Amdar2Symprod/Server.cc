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
// Paul Prestopnik, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jul 2013
//
///////////////////////////////////////////////////////////////

#include <cstdio>  
#include <rapmath/math_macros.h>
#include "Server.hh"
#include "Params.hh"
using namespace std;

//////////////////////////////////////////////////////////////////////
// Constructor
//
// Inherits from DsSymprodServer
///

Server::Server(const string &prog_name,
	       Params *initialParams)
  : DsSymprodServer(prog_name,
		 initialParams->instance,
		 (void*)(initialParams),
		 initialParams->port,
		 initialParams->qmax,
		 initialParams->max_clients,
		 initialParams->no_threads,
		 initialParams->debug >= Params::DEBUG_NORM,
		 initialParams->debug >= Params::DEBUG_VERBOSE)
{
}

//////////////////////////////////////////////////////////////////////
// load local params if they are to be overridden.

int
Server::loadLocalParams( const string &paramFile, void **serverParams)

{
   Params  *localParams;
   char   **tdrpOverrideList = NULL;
   bool     expandEnvVars = true;

   const char *routine_name = "_allocLocalParams";

   if (_isDebug) {
     cerr << "Loading new params from file: " << paramFile << endl;
   }

   localParams = new Params( *((Params*)_initialParams) );
   if ( localParams->load( (char*)paramFile.c_str(),
                           tdrpOverrideList,
                           expandEnvVars,
                           _isVerbose ) != 0 ) {
      cerr << "ERROR - " << _executableName << "::" << routine_name << endl
           << "Cannot load parameter file: " << paramFile << endl;
      delete localParams;
      return( -1 );
   }

   if (_isVerbose) {
     localParams->print(stderr, PRINT_SHORT);
   }

   *serverParams = (void*)localParams;
   return( 0 );
}

//////////////////////////////////////////////////////////////////////
// convertToSymprod() - Convert the given data chunk from the SPDB
//                      database to symprod format.
//
// Returns 0 on success, -1 on failure

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

  if (prod_id != SPDB_AMDAR_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_AMDAR_ID: " << SPDB_AMDAR_ID << endl;
    return -1;
  }

  Params *serverParams = (Params*) params;

  // decode chunk

  Amdar amdar;
  if (amdar.disassemble(spdb_data, spdb_len)) {
    if (_isDebug) {
      cerr << "ERROR - convertToSymprod" << endl;
      cerr << "  Cannot dissamble chunk, size: " << spdb_len << endl;
    }
    return 0;
  }

  // check clipping

  if (serverParams->apply_flight_level_limits) {
	  double fl = amdar.getPressAltitude() / 100.0;
    if (fl < serverParams->min_flight_level ||
	fl > serverParams->max_flight_level) {
      return -1;
    }
  }

  if (serverParams->apply_lat_lon_limits) {
	  double lat = amdar.getLatitude();
	  double lon = amdar.getLongitude();

	  if ( lat < serverParams->min_lat ||
		       lat  > serverParams->max_lat) {
		  return -1;
	  }

	  if (lon < serverParams->min_lon ||
		      lon > serverParams->max_lon) {
		  return -1;
	  }
  }

  // Check that the station position is within the bounding box, if requested
  if (_horizLimitsSet) {

    // Normalize the longitude of the report

    double report_lon = amdar.getLongitude();
    while (report_lon < _minLon)
      report_lon += 360.0;
    while (report_lon >= _minLon + 360.0)
      report_lon -= 360.0;
    amdar.setLongitude(report_lon);

    // Don't process the report if it isn't in the defined bounding box

    if (amdar.getLatitude() < _minLat || amdar.getLatitude() > _maxLat ||
	amdar.getLongitude() < _minLon || amdar.getLongitude() > _maxLon)
      return -1;
  }

  if (serverParams->send_turb_amdars_only && 
      amdar.getTurbulenceCode() == Amdar::MISSING_VALUE) {
    return -1;
  }

  if (serverParams->do_translation) {
    // create Symprod object

    time_t now = time(NULL);
    Symprod prod(now, now,
	         chunk_ref.valid_time,
                 chunk_ref.expire_time,
	         chunk_ref.data_type,
	         chunk_ref.data_type2,
	         "Amdar2Symprod"); 
  
    // icon

    if (serverParams->plot_icon) {
      _addIcon(serverParams, amdar, prod);
    }

    if (serverParams->draw_wind_barb) {
      _addWindBarb(serverParams, amdar, prod);
    }

    // flight-level/callsign text
  
    _addText(serverParams, amdar, prod);

    // turbulence icons

    if (serverParams->plot_turb_icon) {
      _addTurbIcon(serverParams, amdar, prod);
    }

    // hidden text

    if (serverParams->activate_hidden_text) {
      _addHiddenText(serverParams, amdar, amdar.getTranslatedText(), prod);
    }

    // copy internal representation of product to output buffer

    prod.serialize(symprod_buf);
  } else {
    // just copy SPDB data from input to output
    symprod_buf.add(spdb_data, spdb_len);
  }

  return(0);
  
}

void Server::_addText(const Params *serverParams,
		      const Amdar &amdar,
		      Symprod &prod)

{
  
  int iconSize = serverParams->icon_size;
  int detail_level = 0;
  if (serverParams->plot_unscaled) {
    detail_level = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
  }

  const float lat = amdar.getLatitude();
  const float lon = amdar.getLongitude();
  const float alt = amdar.getPressAltitude();
  const float temp = amdar.getTemperature();
  const float windSpeed = amdar.getWindSpeed();
  const float windDir = amdar.getWindDir();

  // callsign
  
  if (serverParams->callsign_label.do_draw &&
      amdar.getAircraftId().length() > 0) {
    
	  const char *text = amdar.getAircraftId().c_str();
	  const char *color = serverParams->callsign_label.color;
	  if (serverParams->debug >= Params::DEBUG_VERBOSE){
		  cerr << "Aircraft ID text : " << text << endl;
	  }
    
    prod.addText(text,
		 lat, lon, color,
		 serverParams->callsign_label.background_color,
		 serverParams->callsign_label.x_offset,
		 serverParams->callsign_label.y_offset - iconSize / 2,
		 (Symprod::vert_align_t)
		 serverParams->callsign_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->callsign_label.horiz_align,
		 serverParams->callsign_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->callsign_label.font_name,
		 0, detail_level);
  }

  // flight level
  
  if (serverParams->flight_level_label.do_draw &&
      alt != Amdar::MISSING_VALUE) {
    char *color = serverParams->flight_level_label.color;
    char text[64];
    int flevel = (int) (alt / 100.0 + 0.5);
    sprintf(text, "FL%.3d", flevel);
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Flight_Level text : " << text << endl;
    }

    prod.addText(text,
		 lat, lon, color,
		 serverParams->flight_level_label.background_color,
		 serverParams->flight_level_label.x_offset,
		 serverParams->flight_level_label.y_offset - iconSize / 2,
		 (Symprod::vert_align_t)
		 serverParams->flight_level_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->flight_level_label.horiz_align,
		 serverParams->flight_level_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->flight_level_label.font_name,
		 0, detail_level);
  }

  // temperature
  
  if (serverParams->temperature_label.do_draw &&
      temp != Amdar::MISSING_VALUE) {
    char *color = serverParams->temperature_label.color;
    char text[64];
    sprintf(text, "T %.0fC", temp);
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Temperature text : " << text << endl;
    }

    prod.addText(text,
		 lat, lon, color,
		 serverParams->temperature_label.background_color,
		 serverParams->temperature_label.x_offset,
		 serverParams->temperature_label.y_offset - iconSize / 2,
		 (Symprod::vert_align_t)
		 serverParams->temperature_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->temperature_label.horiz_align,
		 serverParams->temperature_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->temperature_label.font_name,
		 0, detail_level);
  }

  // wind
  
  if (serverParams->wind_label.do_draw &&
      windSpeed != Amdar::MISSING_VALUE &&
      windDir != Amdar::MISSING_VALUE) {
    char *color = serverParams->wind_label.color;
    char text[64];
    sprintf(text, "%.0f/%.0fkt", windDir, windSpeed);
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Wind text : " << text << endl;
    }

    prod.addText(text,
		 lat, lon, color,
		 serverParams->wind_label.background_color,
		 serverParams->wind_label.x_offset,
		 serverParams->wind_label.y_offset - iconSize / 2,
		 (Symprod::vert_align_t)
		 serverParams->wind_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->wind_label.horiz_align,
		 serverParams->wind_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->wind_label.font_name,
		 0, detail_level);
  }

  // Message

  if (serverParams->message_label.do_draw &&
      amdar.getText().length() > 0) {
    const char *color = serverParams->message_label.color;
    const char *text = amdar.getText().c_str();
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Message text : " << text << endl;
    }
    
    prod.addText(text,
		 lat, lon, color,
		 serverParams->message_label.background_color,
		 serverParams->message_label.x_offset,
		 serverParams->message_label.y_offset - iconSize / 2,
		 (Symprod::vert_align_t)
		 serverParams->message_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->message_label.horiz_align,
		 serverParams->message_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->message_label.font_name,
		 0, detail_level);
  }

}

void Server::_addIcon(const Params *serverParams,
		      const Amdar &amdar,
		      Symprod &prod)

{
  const float lat = amdar.getLatitude();
  const float lon = amdar.getLongitude();
  
  int detail_level = 0;
  if (serverParams->plot_unscaled) {
    detail_level = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
  }

  // set up the icon
  
  int sz = serverParams->icon_size;
  
  Symprod::ppt_t icon[3] =
  {
    {  0,          sz  },
    { (si32) (sz * 1.1 + 0.5),   (si32) (-sz * 0.5 + 0.5) },
    { (si32) (-sz * 1.1 + 0.5),  (si32) (-sz * 0.5 + 0.5) }
  };
  
  // Add icon
  
  prod.addIconline(lat, lon,
		   3, icon,
		   serverParams->icon_color,
		   Symprod::LINETYPE_SOLID, 1,
		   Symprod::CAPSTYLE_BUTT,
		   Symprod::JOINSTYLE_BEVEL,
		   true, Symprod::FILL_SOLID,
		   0, detail_level);
  
  if (_isVerbose) {
    cerr << "Adding icon, lat, lon: "
	 << lat << ", "
	 << lon << endl;
  }

}

void Server::_addTurbIcon(const Params *serverParams,
			  const Amdar &amdar,
			  Symprod &prod)

{
	const float lat = amdar.getLatitude();
  const float lon = amdar.getLongitude();

  int detail_level = 0;
  if (serverParams->plot_unscaled) {
    detail_level = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
  }

  // set up the icon
  
  int sz = serverParams->turb_icon_size;
  int up = Symprod::PPT_PENUP;
  
  Symprod::ppt_t none_icon[] =
   {
    { 0, sz },
    { sz/2, sz },
    { sz, sz/2 },
    { sz, 0 },
    { sz, -sz/2 },
    { sz/2, -sz },
    { 0, -sz },
    { -sz/2, -sz },
    { -sz, -sz/2 },
    { -sz, 0 },
    { -sz, sz/2 },
    { -sz/2, sz },
    { 0, sz },
    { up, up },
    { -sz*2, -sz*2 },
    { sz*2, sz*2 }
  };
  
  Symprod::ppt_t light_icon[] =
  {
    { -sz, -sz },
    { 0, sz },
    { sz, -sz }
  };

  Symprod::ppt_t mod_icon[] =
  {
    { -sz, -sz },
    { 0, sz },
    { sz, -sz },
    { up, up },
    { -sz*2, -sz },
    { -sz, -sz },
    { up, up },
    { sz, -sz },
    { sz*2, -sz }
  };

  Symprod::ppt_t severe_icon[] =
  {
    { -sz, 0 },
    { 0, sz*2 },
    { sz, 0 },
    { up, up },
    { -sz, -sz },
    { 0, sz },
    { sz, -sz },
    { up, up },
    { -sz*2, -sz },
    { -sz, -sz },
    { up, up },
    { sz, -sz },
    { sz*2, -sz }
  };


  // origin
  
  Symprod::wpt_t origin;
  origin.lat = lat;
  origin.lon = lon;

  switch (amdar.getTurbulenceCode()) {

  case Amdar::TURB_CODE_NONE:
    prod.addStrokedIcons(serverParams->_turb_icon_colors[0],
			 sizeof(none_icon) / sizeof(Symprod::ppt_t),
			 none_icon, 1, &origin, 0, detail_level);
    break;

  case Amdar::TURB_CODE_LIGHT:
    prod.addStrokedIcons(serverParams->_turb_icon_colors[1],
			 sizeof(light_icon) / sizeof(Symprod::ppt_t),
			 light_icon, 1, &origin, 0, detail_level);
    break;

  case Amdar::TURB_CODE_MODERATE:
    prod.addStrokedIcons(serverParams->_turb_icon_colors[2],
			 sizeof(mod_icon) / sizeof(Symprod::ppt_t),
			 mod_icon, 1, &origin, 0, detail_level);
    break;

  case Amdar::TURB_CODE_SEVERE:
    prod.addStrokedIcons(serverParams->_turb_icon_colors[3],
			 sizeof(severe_icon) / sizeof(Symprod::ppt_t),
			 severe_icon, 1, &origin, 0, detail_level);
    break;

  } // switch
    
}

/////////////////////////
// add hidden text

void Server::_addHiddenText(Params *serverParams,
                            const Amdar &amdar,
                            const string &message,
                            Symprod &prod)

  
{
  
  // add full PIREP message as hidden text
  const float lat = amdar.getLatitude();
  const float lon = amdar.getLongitude();
  if (_isVerbose) {
	  cerr << "Adding hidden text: " << message << endl;
  }

  prod.addText(message.c_str(),
               lat, lon,
               serverParams->hidden_text_foreground_color,
               serverParams->hidden_text_background_color,
               serverParams->hidden_text_x_offset,
               serverParams->hidden_text_y_offset,
               (Symprod::vert_align_t)
               serverParams->hidden_text_vert_align,
               (Symprod::horiz_align_t)
               serverParams->hidden_text_horiz_align,
               serverParams->hidden_text_font_size,
               Symprod::TEXT_NORM,
               serverParams->hidden_text_font_name,
               0, // Object ID
               Symprod::DETAIL_LEVEL_USUALLY_HIDDEN | 
               Symprod::DETAIL_LEVEL_SHOWAS_POPOVER |
               Symprod::DETAIL_LEVEL_IGNORE_TRESHOLDS);
}

void Server::_addWindBarb(const Params *serverParams,
		      const Amdar &amdar,
		      Symprod &prod)

{
	const float lat = amdar.getLatitude();
  const float lon = amdar.getLongitude();
  const float windSpeed = amdar.getWindSpeed();
  const float windDir = amdar.getWindDir();

  int detail_level = 0;
  if (serverParams->plot_unscaled) {
    detail_level = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
  }

    prod.addWindBarb(lat, lon,
		serverParams->wind_barb_color,
		windSpeed, windDir,
		serverParams->station_posn_circle_radius,
		serverParams->wind_barb_line_width,
		serverParams->wind_ticks_angle_to_shaft,
		serverParams->wind_barb_tick_len,
		serverParams->wind_barb_shaft_len,
		detail_level);
}

/////////////////////////////////////////////////////////////
// Sets the prod_id and prod_label in the info appropriately

void Server::_setProductId(DsSpdbMsg::info_t &info, const void *localParams) {
      Params *serverParams = (Params*) localParams;
      if (serverParams->do_translation) {
        info.prod_id = SPDB_SYMPROD_ID;
        STRncopy(info.prod_label, SPDB_SYMPROD_LABEL, SPDB_LABEL_MAX);
      }
}
