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
// Jan 2001
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
  
  // check prod_id

  if (prod_id != SPDB_ACARS_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_ACARS_ID: " << SPDB_ACARS_ID << endl;
    return -1;
  }

  Params *serverParams = (Params*) params;

  if (spdb_len < (int) sizeof(acars_t)) {
    return 0;
  }

  acarsXml acarsX;
  if (acarsX.disassemble(spdb_data, spdb_len)) {
    if (_isDebug) {
      cerr << "ERROR - convertToSymprod" << endl;
      cerr << "  Cannot dissamble chunk, size: " << spdb_len << endl;
    }
    return 0;
  }
  acars_t acars = acarsX.getStruct();

  // check clipping

  if (serverParams->apply_flight_level_limits) {
    double fl = acars.alt / 100.0;
    if (fl < serverParams->min_flight_level ||
	fl > serverParams->max_flight_level) {
      return -1;
    }
  }

  if (serverParams->apply_lat_lon_limits) {
    if (acars.lat < serverParams->min_lat ||
	acars.lat > serverParams->max_lat) {
      return -1;
    }
    if (acars.lon < serverParams->min_lon ||
	acars.lon > serverParams->max_lon) {
      return -1;
    }
  }
  
  // Check that the station position is within the bounding box, if requested

  if (_horizLimitsSet) {

    // Normalize the longitude of the report

    while (acars.lon < _minLon)
      acars.lon += 360.0;
    while (acars.lon >= _minLon + 360.0)
      acars.lon -= 360.0;

    // Is latitude/longitude OK?
    
    if ((acars.lat < _minLat || acars.lat > _maxLat) &&
	(acars.lon < _minLon || acars.lon > _maxLon)) {
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
	         "Acars2Symprod"); 
  
    // icon

    if (serverParams->plot_icon) {
      _addIcon(serverParams, acars, prod);
    }

    if (serverParams->draw_wind_barb) {
      _addWindBarb(serverParams, acars, prod);
    }

    // text fields
  
    _addText(serverParams, acars, prod);

    // hidden text

    if (serverParams->activate_hidden_text) {
      _addHiddenText(serverParams, acars, acarsX.getMessage(), prod);
    }

    // copy internal representation of product to output buffer
    prod.serialize(symprod_buf);

  } else {
    // just copy SPDB data from input to output 
    symprod_buf.add(spdb_data, spdb_len);
  }
    
  return(0);
  
}
void Server::_addWindBarb(const Params *serverParams,
		      const acars_t &acars,
		      Symprod &prod)

{
  int detail_level = 0;
  if (serverParams->plot_unscaled) {
    detail_level = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
  }

    prod.addWindBarb(acars.lat, acars.lon,
		serverParams->wind_barb_color,
		acars.wind_speed, acars.wind_dirn,
		serverParams->station_posn_circle_radius,
		serverParams->wind_barb_line_width,
		serverParams->wind_ticks_angle_to_shaft,
		serverParams->wind_barb_tick_len,
		serverParams->wind_barb_shaft_len,
		detail_level);
}

void Server::_addText(const Params *serverParams,
		      const acars_t &acars,
		      Symprod &prod)

{
  
  int iconSize = serverParams->icon_size;
  int detail_level = 0;
  if (serverParams->plot_unscaled) {
    detail_level = Symprod::DETAIL_LEVEL_DO_NOT_SCALE;
  }

  // flight num
  
  if (serverParams->flight_num_label.do_draw &&
      strlen(acars.flight_number) > 0) {
    
    const char *text = acars.flight_number;
    const char *color = serverParams->flight_num_label.color;
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Flight_Num text : " << text << endl;
    }
    
    prod.addText(text,
		 acars.lat, acars.lon, color,
		 serverParams->flight_num_label.background_color,
		 serverParams->flight_num_label.x_offset,
		 serverParams->flight_num_label.y_offset - iconSize / 2,
		 (Symprod::vert_align_t)
		 serverParams->flight_num_label.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->flight_num_label.horiz_align,
		 serverParams->flight_num_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->flight_num_label.font_name,
		 0, detail_level);
  }

  // flight level
  
  if (serverParams->flight_level_label.do_draw &&
      acars.alt != ACARS_FLOAT_MISSING) {
    char *color = serverParams->flight_level_label.color;
    char text[64];
    int flevel = (int) (acars.alt / 100.0 + 0.5);
    sprintf(text, "FL%.3d", flevel);
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Flight_Level text : " << text << endl;
    }

    prod.addText(text,
		 acars.lat, acars.lon, color,
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

  // Temperature

  if (serverParams->temperature_label.do_draw &&
      acars.temp != ACARS_FLOAT_MISSING) {
    char *color = serverParams->temperature_label.color;
    char text[64];
    sprintf(text, "%.0fC", acars.temp);
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Temperature text : " << text << endl;
    }

    prod.addText(text,
		 acars.lat, acars.lon, color,
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
      acars.wind_speed != ACARS_FLOAT_MISSING &&
      acars.wind_dirn != ACARS_FLOAT_MISSING) {
    
    char *color = serverParams->wind_label.color;
    char text[256];
    
    int wspeed = (int) (_nearest(acars.wind_speed, 5.0) + 0.5);
    int wdirn = (int) (_nearest(acars.wind_dirn, 10.0) + 0.5);
    sprintf(text, "%d/%dkt", wdirn, wspeed);

    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Wind text : " << text << endl;
    }
    
    prod.addText(text,
		 acars.lat, acars.lon, color,
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

}

void Server::_addIcon(const Params *serverParams,
		      const acars_t &acars,
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
  
  prod.addIconline(acars.lat, acars.lon,
		   3, icon,
		   serverParams->icon_color,
		   Symprod::LINETYPE_SOLID, 1,
		   Symprod::CAPSTYLE_BUTT,
		   Symprod::JOINSTYLE_BEVEL,
		   true, Symprod::FILL_SOLID,
		   0, detail_level);
  
  if (_isVerbose) {
    cerr << "Adding icon, lat, lon: "
	 << acars.lat << ", "
	 << acars.lon << endl;
  }

}

/////////////////////////
// add hidden text

void Server::_addHiddenText(Params *serverParams,
                            const acars_t &acars,
                            const string &message,
                            Symprod &prod)

  
{
  
  string text;
  char buf[1024];

  sprintf(buf, "ACARS\n");
  text += buf;
  sprintf(buf, "  flight#: %s\n", acars.flight_number);
  text += buf;
  sprintf(buf, "  depart_airport: %s\n", acars.depart_airport);
  text += buf;
  sprintf(buf, "  dest_airport: %s\n", acars.dest_airport);
  text += buf;
  sprintf(buf, "  time: %s\n", utimstr(acars.time));
  text += buf;
  sprintf(buf, "  lat: %g\n", acars.lat);
  text += buf;
  sprintf(buf, "  lon: %g\n", acars.lon);
  text += buf;
  sprintf(buf, "  alt (ft): %g\n", acars.alt);
  text += buf;

  if (acars.temp > ACARS_FLOAT_MISSING) {
    sprintf(buf, "  temp (C): %g\n", acars.temp);
    text += buf;
  }
  if (acars.wind_speed > ACARS_FLOAT_MISSING) {
    sprintf(buf, "  wind_speed (kts): %g\n", acars.wind_speed);
    text += buf;
  }
  if (acars.wind_dirn > ACARS_FLOAT_MISSING) {
    sprintf(buf, "  wind_dirn (deg T): %g\n", acars.wind_dirn);
    text += buf;
  }
  if (acars.accel_lateral > ACARS_FLOAT_MISSING) {
    sprintf(buf, "  accel_lateral (g): %g\n", acars.accel_lateral);
    text += buf;
  }
  if (acars.accel_vertical > ACARS_FLOAT_MISSING) {
    sprintf(buf, "  accel_vertical (g): %g\n", acars.accel_vertical);
    text += buf;
  }

#ifdef NOTNOW  
  if (acars.eta > 0) {
    sprintf(buf, "  eta: %s\n", utimstr(acars.eta));
    text += buf;
  }
  if (acars.fuel_remain > 0) {
    sprintf(buf, "  fuel_remain : %g\n", acars.fuel_remain);
    text += buf;
  }
#endif

  // add full ACARS message as hidden text

  prod.addText(text.c_str(),
               acars.lat, acars.lon,
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

///////////////////
// get nearest val

double Server::_nearest(double target, double delta)
{
  
  double answer;
  double rem;                                                                 
  
  delta = fabs(delta);                   
  rem = remainder(target,delta);
  
  if(target >= 0.0) {
    if(rem > (delta / 2.0)) {
      answer = target + (delta - rem);
    } else {
      answer = target -  rem;
    }
  } else {
    if(fabs(rem) > (delta / 2.0)) {
      answer = target - (delta + rem);
    } else {
      answer = target -  rem;
    }
  }
  
  return answer;

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

