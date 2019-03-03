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
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2000
//
///////////////////////////////////////////////////////////////

#include <cstdio>  
#include <toolsa/toolsa_macros.h>
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

  Params *serverParams = (Params*) params;  
  // check prod_id

  if(serverParams->use_pirep_t)
  {
    if (prod_id != SPDB_PIREP_ID)
    {
      cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
      cerr << "  Incorrect prod_id: " << prod_id << endl;
      cerr << "  Should be SPDB_PIREP_ID: " << SPDB_PIREP_ID << endl;
      return -1;
    }
  }
  else
  {
    if (prod_id != SPDB_XML_ID)
    {
      cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
      cerr << "  Incorrect prod_id: " << prod_id << endl;
      cerr << "  Should be SPDB_XML_ID: " << SPDB_XML_ID << endl;
      return -1;
    }
  }

  Pirep *pirep = new Pirep();
  string message;

  if(serverParams->use_pirep_t)
  {
    if (spdb_len < (int) sizeof(pirep_t)) {
      if (_isDebug) {
        cerr << "ERROR - convertToSymprod" << endl;
        cerr << "  Chunk size too small: " << spdb_len << endl;
        cerr << "  Min size: " << sizeof(pirep_t) << endl;
      }
      return 0;
    }

    // decode chunk
    pirepXml pirepX;
    if (pirepX.disassemble(spdb_data, spdb_len)) {
      if (_isDebug) {
        cerr << "ERROR - convertToSymprod" << endl;
        cerr << "  Cannot dissamble chunk, size: " << spdb_len << endl;
      }
      return 0;
    }
    message = pirepX.getMessage();
    const pirep_t &p = pirepX.getStruct();
    PirepTranslate pt = new PirepTranslate(serverParams->use_pirep_t);
    pt.pirep_t_to_Pirep(p, pirep);
  }
  else
  {
    int estimatedSmallestChunkSize = 300;
    if (spdb_len < estimatedSmallestChunkSize) { // estimated smallest chunk size
      if (_isDebug) {
        cerr << "ERROR - convertToSymprod" << endl;
        cerr << "  Chunk size too small: " << spdb_len << endl;
        cerr << "  Min size: " << estimatedSmallestChunkSize << endl;
      }
      return 0;
    }

    // decode chunk
    char xml[spdb_len];
    memcpy(&xml,spdb_data,spdb_len);
    const string s(xml);
    pirep->setXml(s);
    pirep->fromXml();
    message = pirep->getRaw();
  }
  // check clipping

  if (serverParams->apply_flight_level_limits) {
    double fl = pirep->getAltitude() / 100.0;
    if (fl < serverParams->min_flight_level ||
	fl > serverParams->max_flight_level) {
      return -1;
    }
  }

  if (serverParams->apply_lat_lon_limits) {
    if (pirep->getLatitude() < serverParams->min_lat ||
        pirep->getLatitude() > serverParams->max_lat) {
      return -1;
    }
    if (pirep->getLongitude() < serverParams->min_lon ||
        pirep->getLongitude() > serverParams->max_lon) {
      return -1;
    }
  }

  if (serverParams->plot_only_if_message &&
      strlen(pirep->getRaw()) == 0) {
    return -1;
  }

  if (serverParams->send_turb_pireps_only && pirep->getTurbObs1().intensity < 0) {
    return -1;
  }

  if (serverParams->send_ice_pireps_only && pirep->getIceObs1().intensity < 0) {
    return -1;
  }

  // Check that the station position is within the bounding box, if requested
  if (_horizLimitsSet) {

    bool accept = false;
    // Is latitude OK?
    if (_minLat <= pirep->getLatitude() && _maxLat >= pirep->getLatitude()) {

      // If lat is OK, do a rather more complex check for longitude
      if (_minLon <= pirep->getLongitude() && _maxLon >= pirep->getLongitude()) {
        accept = true;
      } else if (_minLon <= (pirep->getLongitude() + 360.0) &&
                 _maxLon >= (pirep->getLongitude() + 360.0)) {
        accept = true;
      } else if (_minLon <= (pirep->getLongitude() - 360.0) &&
                 _maxLon >= (pirep->getLongitude() - 360.0)) {
        accept = true;
      }

    }

    if (!accept) {
      return -1;
    }
  }

  if (serverParams->do_translation) {
    // create Symprod object

    time_t now = time(NULL);
    Symprod prod(now, now,
	         chunk_ref.valid_time,
                 chunk_ref.expire_time,
	         chunk_ref.data_type,
	         chunk_ref.data_type2,
	         "Pirep2Symprod"); 
  
    // icon

    if (serverParams->plot_icon) {
      _addIcon(serverParams, *pirep, prod);
    }

    if (serverParams->draw_wind_barb) {
      _addWindBarb(serverParams, *pirep, prod);
    }

    // flight-level/callsign text
  
    _addText(serverParams, *pirep, prod);

    // turbulence icons

    if (serverParams->plot_turb_icon) {
      _addTurbIcon(serverParams, *pirep, prod);
    }

    // icing icons

    if (serverParams->plot_ice_icon) {
      _addIcingIcon(serverParams, *pirep, prod);
    }

    // hidden text

    if (serverParams->activate_hidden_text) {
        _addHiddenText(serverParams, *pirep, message, prod);
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
		      Pirep &pirep,
		      Symprod &prod)

{
  
  int iconSize = serverParams->icon_size;
  int detail_level = 0;
  if (serverParams->plot_unscaled) {
    detail_level = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
  }

  // callsign
  
  if (serverParams->callsign_label.do_draw &&
       strlen(pirep.getAircraftId()) > 0) {
   
    const char *text = pirep.getAircraftId();
    const char *color = serverParams->callsign_label.color;
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Callsign text : " << text << endl;
    }
    
    prod.addText(text,
		 pirep.getLatitude(), pirep.getLongitude(), color,
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

  // flight level1
  
  if (serverParams->flight_level_label.do_draw &&
      pirep.getAltitude() != PIREP_FLOAT_MISSING) {
    char *color = serverParams->flight_level_label.color;
    char text[64];
    int flevel = (int) (pirep.getAltitude() / 100.0 + 0.5);
    sprintf(text, "FL%.3d", flevel);
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Flight_Level text : " << text << endl;
    }

    prod.addText(text,
		 pirep.getLatitude(), pirep.getLongitude(), color,
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
      pirep.getWeatherObs().temperature != PIREP_FLOAT_MISSING) {
    char *color = serverParams->temperature_label.color;
    char text[64];
    sprintf(text, "T %.0fC", pirep.getWeatherObs().temperature);
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Temperature text : " << text << endl;
    }

    prod.addText(text,
		 pirep.getLatitude(), pirep.getLongitude(), color,
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
      pirep.getWeatherObs().wind_speed != PIREP_FLOAT_MISSING &&
      pirep.getWeatherObs().wind_dir != PIREP_FLOAT_MISSING) {
    char *color = serverParams->wind_label.color;
    char text[64];
    sprintf(text, "%.0f/%.0fkt", pirep.getWeatherObs().wind_dir, pirep.getWeatherObs().wind_speed);
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Wind text : " << text << endl;
    }

    prod.addText(text,
		 pirep.getLatitude(), pirep.getLongitude(), color,
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
      strlen(pirep.getRaw()) > 0) {
    const char *color = serverParams->message_label.color;
    const char *text = pirep.getRaw();
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Message text : " << text << endl;
    }
    
    prod.addText(text,
		 pirep.getLatitude(), pirep.getLongitude(), color,
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
		      Pirep &pirep,
		      Symprod &prod)

{
  
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
  
  prod.addIconline(pirep.getLatitude(), pirep.getLongitude(),
		   3, icon,
		   serverParams->icon_color,
		   Symprod::LINETYPE_SOLID, 1,
		   Symprod::CAPSTYLE_BUTT,
		   Symprod::JOINSTYLE_BEVEL,
		   true, Symprod::FILL_SOLID,
		   0, detail_level);
  
  if (_isVerbose) {
    cerr << "Adding icon, lat, lon: "
	 << pirep.getLatitude() << ", "
	 << pirep.getLongitude() << endl;
  }

}

void Server::_addTurbIcon(const Params *serverParams,
                          Pirep &pirep,
			  Symprod &prod)

{
  
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
  
  Symprod::ppt_t trace_icon[] =
  {
    { -sz*2, -sz },
    { -sz, -sz },
    { up, up },
    { -sz/2, -sz },
    { sz/2, -sz },
    { up, up },
    { sz, -sz },
    { sz*2, -sz }
 };

  Symprod::ppt_t light_icon[] =
  {
    { -sz, -sz },
    { 0, sz },
    { sz, -sz }
  };

  Symprod::ppt_t light_mod_icon[] =
  {
    { -sz, -sz },
    { 0, sz },
    { sz, -sz },
    { up, up },
    { -sz*2, -sz },
    { -sz, -sz },
    { up, up },
    { -sz/2, -sz },
    { sz/2, -sz },
    { up, up },
    { sz, -sz },
    { sz*2, -sz }
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

  Symprod::ppt_t mod_severe_icon[] =
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
    { -sz/2, -sz },
    { sz/2, -sz },
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

  Symprod::ppt_t extreme_icon[] =
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
    { sz*2, -sz },
    { up, up },
    { 0, sz },
    { -sz/2, -sz },
    { up, up },
    { 0, sz },
    { 0, -sz },
    { up, up },
    { 0, sz },
    { sz/2, -sz },
    { up, up }
  };

  // origin
  
  Symprod::wpt_t origin;
  origin.lat = pirep.getLatitude();
  origin.lon = pirep.getLongitude();

  //  switch (pirep.turb_index) {
  int turb_index = pirep.getTurbObs1().intensity;
  switch (turb_index) {

  case Pirep::NONE_SMOOTH_TI:
    prod.addStrokedIcons(serverParams->_turb_icon_colors[turb_index],
			 sizeof(none_icon) / sizeof(Symprod::ppt_t),
			 none_icon, 1, &origin, 0, detail_level);
    break;
  case Pirep::SMOOTH_LGHT_TI:
    prod.addStrokedIcons(serverParams->_turb_icon_colors[turb_index],
			 sizeof(trace_icon) / sizeof(Symprod::ppt_t),
			 trace_icon, 1, &origin, 0, detail_level);
    break;
 case Pirep::LGHT_TI:
    prod.addStrokedIcons(serverParams->_turb_icon_colors[turb_index],
			 sizeof(light_icon) / sizeof(Symprod::ppt_t),
			 light_icon, 1, &origin, 0, detail_level);
    break;
 case Pirep::LGHT_MOD_TI:
    prod.addStrokedIcons(serverParams->_turb_icon_colors[turb_index],
			 sizeof(light_mod_icon) / sizeof(Symprod::ppt_t),
			 light_mod_icon, 1, &origin, 0, detail_level);
    break;
 case Pirep::MOD_TI:
    prod.addStrokedIcons(serverParams->_turb_icon_colors[turb_index],
			 sizeof(mod_icon) / sizeof(Symprod::ppt_t),
			 mod_icon, 1, &origin, 0, detail_level);
    break;
 case Pirep::MOD_SEVR_TI:
    prod.addStrokedIcons(serverParams->_turb_icon_colors[turb_index],
			 sizeof(mod_severe_icon) / sizeof(Symprod::ppt_t),
			 mod_severe_icon, 1, &origin, 0, detail_level);
    break;
 case Pirep::SEVR_TI:
    prod.addStrokedIcons(serverParams->_turb_icon_colors[turb_index],
			 sizeof(severe_icon) / sizeof(Symprod::ppt_t),
			 severe_icon, 1, &origin, 0, detail_level);
    break;
 case Pirep::SEVR_EXTRM_TI:
 case Pirep::EXTRM_TI:
    prod.addStrokedIcons(serverParams->_turb_icon_colors[turb_index],
			 sizeof(extreme_icon) / sizeof(Symprod::ppt_t),
			 extreme_icon, 1, &origin, 0, detail_level);
    break;

  } // switch
    
}

void Server::_addIcingIcon(const Params *serverParams,
			   Pirep &pirep,
			   Symprod &prod)

{
  
  int detail_level = 0;
  if (serverParams->plot_unscaled) {
    detail_level = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
  }

  // set up the icon
  
  int sz = serverParams->ice_icon_size;
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
  
  Symprod::ppt_t trace_icon[] =
  {
    { -sz, sz },
    { -sz, sz/2 },
    { -sz/2, 0 },
    { sz/2, 0 },
    { sz, sz/2 },
    { sz, sz }
  };

Symprod::ppt_t trace_light_icon[] =
  {
    { -sz, sz },
    { -sz, sz/2 },
    { -sz/2, 0 },
    { sz/2, 0 },
    { sz, sz/2 },
    { sz, sz },
    { up, up },
    { 0, 0 },
    { 0, sz/2 }    
  };

  Symprod::ppt_t light_icon[] =
  {
    { -sz, sz },
    { -sz, sz/2 },
    { -sz/2, 0 },
    { sz/2, 0 },
    { sz, sz/2 },
    { sz, sz },
    { up, up },
    { 0, -sz/2 },
    { 0, sz/2 }
  };

  Symprod::ppt_t light_mod_icon[] =
  {
    { -sz, sz },
    { -sz, sz/2 },
    { -sz/2, 0 },
    { sz/2, 0 },
    { sz, sz/2 },
    { sz, sz },
    { up, up },
    { sz/4, 0 },
    { sz/4, sz/2 },    
    { up, up },
    { -sz/4, 0 },
    { -sz/4, sz/2 }    
  };

  Symprod::ppt_t mod_icon[] =
  {
    { -sz, sz },
    { -sz, sz/2 },
    { -sz/2, 0 },
    { sz/2, 0 },
    { sz, sz/2 },
    { sz, sz },
    { up, up },
    { sz/4, -sz/2 },
    { sz/4, sz/2 },    
    { up, up },
    { -sz/4, -sz/2 },
    { -sz/4, sz/2 }    
  };

  Symprod::ppt_t mod_hvy_icon[] =
  {
    { -sz, sz },
    { -sz, sz/2 },
    { -sz/2, 0 },
    { sz/2, 0 },
    { sz, sz/2 },
    { sz, sz },
    { up, up },
    { 0, 0 },
    { 0, sz/2 },    
    { up, up },
    { sz/4, 0 },
    { sz/4, sz/2 },    
    { up, up },
    { -sz/4, 0 },
    { -sz/4, sz/2 }    
  };

  Symprod::ppt_t heavy_icon[] =
  {
    { -sz, sz },
    { -sz, sz/2 },
    { -sz/2, 0 },
    { sz/2, 0 },
    { sz, sz/2 },
    { sz, sz },
    { up, up },
    { 0, -sz/2 },
    { 0, sz/2 },
    { up, up },
    { sz/4, -sz/2 },
    { sz/4, sz/2 },    
    { up, up },
    { -sz/4, -sz/2 },
    { -sz/4, sz/2 }    
  };

  Symprod::ppt_t severe_icon[] =
  {
    { -sz, sz },
    { -sz, sz/2 },
    { -sz/2, 0 },
    { sz/2, 0 },
    { sz, sz/2 },
    { sz, sz },
    { up, up },
    { 0, -sz/2 },
    { 0, sz/2 },
    { up, up },
    { sz/4, -sz/2 },
    { sz/4, sz/2 },    
    { up, up },
    { -sz/4, -sz/2 },
    { -sz/4, sz/2 }    
  };

  // origin
  
  Symprod::wpt_t origin;
  origin.lat = pirep.getLatitude();
  origin.lon = pirep.getLongitude();

  //  switch (pirep.turb_index) {
  int ice_index = pirep.getIceObs1().intensity;

  if (serverParams->debug >= Params::DEBUG_VERBOSE){
    cerr << "ice_index is " << ice_index << endl;
  }

  switch (ice_index) {

  case Pirep::NONE_II:  // 0
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "NONE_II  type" << endl;
    }
    prod.addStrokedIcons(serverParams->_ice_icon_colors[ice_index],
    			 sizeof(none_icon) / sizeof(Symprod::ppt_t),
    			 none_icon, 1, &origin, 0, detail_level);

    break;
  case Pirep::TRC_II:  //  1
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "TRC_II  type" << endl;
    }
    prod.addStrokedIcons(serverParams->_ice_icon_colors[ice_index],
			 sizeof(trace_icon) / sizeof(Symprod::ppt_t),
			 trace_icon, 1, &origin, 0, detail_level);
    break;
  case Pirep::TRC_LGHT_II:  // 2
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "TRC_LGHT_II  type" << endl;
    }
    prod.addStrokedIcons(serverParams->_ice_icon_colors[ice_index],
			 sizeof(trace_light_icon) / sizeof(Symprod::ppt_t),
			 trace_light_icon, 1, &origin, 0, detail_level);
    break;
  case Pirep::LGHT_II:  // 3
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "LGHT_II  type" << endl;
    }
    prod.addStrokedIcons(serverParams->_ice_icon_colors[ice_index],
			 sizeof(light_icon) / sizeof(Symprod::ppt_t),
			 light_icon, 1, &origin, 0, detail_level);
    break;
  case Pirep::LGHT_MOD_II:  // 4
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "LGHT_MOD_II  type" << endl;
    }
    prod.addStrokedIcons(serverParams->_ice_icon_colors[ice_index],
			 sizeof(light_mod_icon) / sizeof(Symprod::ppt_t),
			 light_mod_icon, 1, &origin, 0, detail_level);
    break;
  case Pirep::MOD_II:  // 5
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "MOD_II  type" << endl;
    }
    prod.addStrokedIcons(serverParams->_ice_icon_colors[ice_index],
			 sizeof(mod_icon) / sizeof(Symprod::ppt_t),
			 mod_icon, 1, &origin, 0, detail_level);
    break;
  case Pirep::MOD_HVY_II:  // 6
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "MOD_HVY  type" << endl;
    }
    prod.addStrokedIcons(serverParams->_ice_icon_colors[ice_index],
			 sizeof(mod_hvy_icon) / sizeof(Symprod::ppt_t),
			 mod_hvy_icon, 1, &origin, 0, detail_level);
    break;
  case Pirep::HVY_II:  // 7
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "HVY_II  type" << endl;
    }
    prod.addStrokedIcons(serverParams->_ice_icon_colors[ice_index],
			 sizeof(heavy_icon) / sizeof(Symprod::ppt_t),
			 heavy_icon, 1, &origin, 0, detail_level);
  case Pirep::SEVR_II:  // 8
  if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "SEVR_II  type" << endl;
  }
    prod.addStrokedIcons(serverParams->_ice_icon_colors[ice_index],
			 sizeof(severe_icon) / sizeof(Symprod::ppt_t),
			 severe_icon, 1, &origin, 0, detail_level);
    break;

  case Pirep::FILL_II:  // -9
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "FILL_II  type" << endl;
    }
   // do nothing
   break;

  default:
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr <<  "unknown ice index: " << ice_index;
    }
  } // switch
    
}

/////////////////////////
// add hidden text

void Server::_addHiddenText(Params *serverParams,
                            Pirep &pirep,
                            const string &message,
                            Symprod &prod)

  
{
  
  // add full PIREP message as hidden text

  prod.addText(message.c_str(),
               pirep.getLatitude(), pirep.getLongitude(),
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
		      Pirep &pirep,
		      Symprod &prod)

{
  int detail_level = 0;
  if (serverParams->plot_unscaled) {
    detail_level = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
  }

  prod.addWindBarb(pirep.getLatitude(), pirep.getLongitude(),
		serverParams->wind_barb_color,
	   pirep.getWeatherObs().wind_speed, pirep.getWeatherObs().wind_dir,
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

