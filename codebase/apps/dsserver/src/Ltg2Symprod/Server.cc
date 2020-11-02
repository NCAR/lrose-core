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

#include <toolsa/toolsa_macros.h>
#include <toolsa/TaArray.hh>
#include "Server.hh"
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
  return;
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
  //
  // Set up the rendering parameters.
  //
  // Convert param values to Symprod object values
  //
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

  if (prod_id != SPDB_LTG_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_LTG_ID: " << SPDB_LTG_ID << endl;
    return -1;
  }

  Params *serverParams = (Params*) params;

   // check for extended data

  ui32 cookie;
  if ((ui32)spdb_len >= sizeof(cookie)) {
    memcpy(&cookie, spdb_data, sizeof(cookie));
  } else {
    cookie = 0;
  }

  // compute number of strikes, set up array

  int nstrikes;
  if (cookie == LTG_EXTENDED_COOKIE) {
    nstrikes = spdb_len / sizeof(LTG_extended_t);
  } else {
    nstrikes = spdb_len / sizeof(LTG_strike_t);
  }
  TaArray<LTG_extended_t> strikes_;
  LTG_extended_t *strikes = strikes_.alloc(nstrikes);
  
  // load up strike data array
  
  if (cookie == LTG_EXTENDED_COOKIE) {
    
    MemBuf ltgBuf;
    ltgBuf.load(spdb_data, spdb_len);
    LTG_extended_t *_strikes = (LTG_extended_t *) ltgBuf.getPtr();
    for (int ii = 0; ii < nstrikes; ii++) {
      LTG_extended_from_BE(&_strikes[ii]);
      strikes[ii] = _strikes[ii];
    }

  } else {

    MemBuf ltgBuf;
    ltgBuf.load(spdb_data, spdb_len);
    LTG_strike_t *_strikes = (LTG_strike_t *) ltgBuf.getPtr();
    for (int ii = 0; ii < nstrikes; ii++) {
      LTG_from_BE(&_strikes[ii]);
      strikes[ii].time = _strikes[ii].time;
      strikes[ii].latitude = _strikes[ii].latitude;
      strikes[ii].longitude = _strikes[ii].longitude;
      strikes[ii].amplitude = _strikes[ii].amplitude;
      strikes[ii].type = _strikes[ii].type;
    }
    
  }

  // set up the icon

  int icon_size = serverParams->icon_size;

  // Negative - Icon
  Symprod::ppt_t ltg_neg_icon[2] =
  {
    { -icon_size,  0 },
    {  icon_size,  0 } 
  };

  // Positive + Icon
  Symprod::ppt_t ltg_icon[5] =
  {
    { -icon_size,  0 },
    {  icon_size,  0 }, 
    {  0,          0 }, 
    {  0,          -icon_size }, 
    {  0,          icon_size  }
  };

  // Negative - Icon for cloud-cloud (cc)
  Symprod::ppt_t ltg_neg_icon_cc[2] =
  {
    { -icon_size,  -icon_size },
    {  icon_size,   icon_size } 
  };

  // Positive + Icon cloud-cloud
  Symprod::ppt_t ltg_icon_cc[5] =
  {
    { -icon_size,  -icon_size },
    {  icon_size,   icon_size }, 
    {  0,           0 }, 
    {  icon_size,  -icon_size }, 
    {  -icon_size,  icon_size  }
  };


  // create Symprod object
  
  time_t now = time(NULL);
  
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "Lightning");

  time_t recentTime = 0L;
  if (serverParams->useWallClockForAge){
    recentTime = now - serverParams->recent_strike_age;
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "recent time is taken to be " << utimstr(recentTime) << endl;
    }
  } else {
    const DsSpdbMsg& readMsg = this->getReadMsg();
    const time_t refTime = (const time_t) readMsg.getRefTime();
    recentTime = refTime - serverParams->recent_strike_age;
    if (serverParams->debug >= Params::DEBUG_VERBOSE){
      cerr << "Recent time is taken to be " << utimstr(recentTime) << endl;
    }
  }






  //
  // Do the rendering. This is a little laborious since we
  // loop through the strikes twice. The reason for this
  // is that since the ltg data are stored as arrays, we
  // are not assured that the data are in temporal order.
  // And since we don't want a new strike to be drawn and then
  // subsequently have an old strike to be drawn over the top of
  // it - it's confusing - the loop is done twice, first
  // drawing all the old ones and then all the new ones.
  //
  // The other option would be to sort them all into temporal order
  // but that would be time consuming.
  //

  //
  // Keep count of what we're sending, if only for
  // debugging.
  //
  int numIcon = 0;
  int numText = 0;

  //
  // First loop through strikes - draw older strikes.
  //
  char *colorToUse = serverParams->display_color;
  //
  for (int ii = 0; ii < nstrikes; ii++) {
    //
    // If we're using the bounding box and this point is outside it,
    // then skip it.
    //
    if (serverParams->useBoundingBox){
      if (
	  (strikes[ii].latitude > serverParams->bounding_box.max_lat) ||
	  (strikes[ii].longitude > serverParams->bounding_box.max_lon) ||
	  (strikes[ii].latitude < serverParams->bounding_box.min_lat) ||
	  (strikes[ii].longitude < serverParams->bounding_box.min_lon)
	  ){
	continue;
      }
    }

    //
    // If this strike is recent, skip it, we'll get it in the next loop.
    //
    if (strikes[ii].time >= recentTime)
      continue;

    //
    // Set up a wpt object so we can add this thing.
    //
    Symprod::wpt_t wpt;
    wpt.lat = strikes[ii].latitude;
    wpt.lon = strikes[ii].longitude;

    //
    // The icon we add may depend on the polarity of the strike and
    // if it is cloud-cloud or cloud-ground.
    //
    numIcon++;

    if ((serverParams->use_cc_icons) && (strikes[ii].type == LTG_CLOUD_STROKE)){
      if (serverParams->render_polarity && (strikes[ii].amplitude < 0)) {
	prod.addStrokedIcons(colorToUse,
			     2, ltg_neg_icon_cc,
			     1, &wpt,
			     0,0,serverParams->line_width);
      } else {
	prod.addStrokedIcons(colorToUse,
			     5, ltg_icon_cc,
			     1, &wpt,
			     0,0,serverParams->line_width);
      }
    } else {
      if (serverParams->render_polarity && (strikes[ii].amplitude < 0)) {
	prod.addStrokedIcons(colorToUse,
			     2, ltg_neg_icon,
			     1, &wpt,
			     0,0,serverParams->line_width);
      } else {
	prod.addStrokedIcons(colorToUse,
			     5, ltg_icon,
			     1, &wpt,
			     0,0,serverParams->line_width);
      }
    }
    //
    // So much for the icon. Add text labels, if requested.
    //
    if (serverParams->do_time_labelling){

      date_time_t dataTime;
      dataTime.unix_time = strikes[ii].time;
      uconvert_from_utime( &dataTime );
      //
      // Set the format for the label appropriately.
      //
      char timeLabel[64];
      
      switch( serverParams->time_format ){
	
      case Params::TIME_LABEL_HHMM :
	sprintf(timeLabel,"%02d:%02d",
		dataTime.hour, dataTime.min);
	break;
	
      case Params::TIME_LABEL_HHMMSS :
	sprintf(timeLabel,"%02d:%02d:%02d",
		dataTime.hour, dataTime.min, dataTime.sec);
	break;
	//
	// If time_format is set to something weird, we default to the following.
	//
      case Params::TIME_LABEL_YYYYMMDDHHMMSS :
      default :
	sprintf(timeLabel,"%d/%02d/%02d %02d:%02d:%02d",
		dataTime.year, dataTime.month, dataTime.day,
		dataTime.hour, dataTime.min, dataTime.sec);
	break;
	
      }
      
      numText++;
      prod.addText(timeLabel,
		   strikes[ii].latitude, strikes[ii].longitude,
		   serverParams->time_label_color,
		   serverParams->text_background_color,
		   serverParams->time_text_offset.x, 
		   serverParams->time_text_offset.y,
		   _symprodVertAlign, _symprodHorizAlign,
		   serverParams->text_font_size,
		   _symprodFontStyle, serverParams->font_name);

    }

    if ((serverParams->do_type_labelling) && (strikes[ii].type == LTG_CLOUD_STROKE)){
      prod.addText(serverParams->type_cloud_cloud_label,
		   strikes[ii].latitude, strikes[ii].longitude,
		   serverParams->type_label_color,
		   serverParams->text_background_color,
		   serverParams->type_text_offset.x, 
		   serverParams->type_text_offset.y,
		   _symprodVertAlign, _symprodHorizAlign,
		   serverParams->text_font_size,
		   _symprodFontStyle, serverParams->font_name);
      
    }
  }

  //
  // Now, the next loop, in which we draw only recent strikes.
  // First, set the color for recent strikes, if desired.
  //
  if (serverParams->different_color_for_recent_strikes)
    colorToUse = serverParams->recent_strike_color;
  //
  for (int ii = 0; ii < nstrikes; ii++) {
    //
    // If we're using the bounding box and this point is outside it,
    // then skip it.
    //
    if (serverParams->useBoundingBox){
      if (
	  (strikes[ii].latitude > serverParams->bounding_box.max_lat) ||
	  (strikes[ii].longitude > serverParams->bounding_box.max_lon) ||
	  (strikes[ii].latitude < serverParams->bounding_box.min_lat) ||
	  (strikes[ii].longitude < serverParams->bounding_box.min_lon)
	  ){
	continue;
      }
    }

    //
    // If this strike is not recent, skip it, we did it in the last loop.
    //
    if (!(strikes[ii].time >= recentTime))
      continue;

    //
    // Set up a wpt object so we can add this thing.
    //
    Symprod::wpt_t wpt;
    wpt.lat = strikes[ii].latitude;
    wpt.lon = strikes[ii].longitude;

    //
    // The icon we add may depend on the polarity of the strike and
    // if it is cloud-cloud or cloud-ground.
    //
    numIcon++;

    if ((serverParams->use_cc_icons) && (strikes[ii].type == LTG_CLOUD_STROKE)){
      if (serverParams->render_polarity && (strikes[ii].amplitude < 0)) {
	prod.addStrokedIcons(colorToUse,
			     2, ltg_neg_icon_cc,
			     1, &wpt,
			     0,0,serverParams->line_width);
      } else {
	prod.addStrokedIcons(colorToUse,
			     5, ltg_icon_cc,
			     1, &wpt,
			     0,0,serverParams->line_width);
      }
    } else {
      if (serverParams->render_polarity && (strikes[ii].amplitude < 0)) {
	prod.addStrokedIcons(colorToUse,
			     2, ltg_neg_icon,
			     1, &wpt,
			     0,0,serverParams->line_width);
      } else {
	prod.addStrokedIcons(colorToUse,
			     5, ltg_icon,
			     1, &wpt,
			     0,0,serverParams->line_width);
      }
    }

    //
    // So much for the icon. Add text labels, if requested.
    //
    if (serverParams->do_time_labelling){

      date_time_t dataTime;
      dataTime.unix_time = strikes[ii].time;
      uconvert_from_utime( &dataTime );
      //
      // Set the format for the label appropriately.
      //
      char timeLabel[64];
      
      switch( serverParams->time_format ){
	
      case Params::TIME_LABEL_HHMM :
	sprintf(timeLabel,"%02d:%02d",
		dataTime.hour, dataTime.min);
	break;
	
      case Params::TIME_LABEL_HHMMSS :
	sprintf(timeLabel,"%02d:%02d:%02d",
		dataTime.hour, dataTime.min, dataTime.sec);
	break;
	//
	// If time_format is set to something weird, we default to the following.
	//
      case Params::TIME_LABEL_YYYYMMDDHHMMSS :
      default :
	sprintf(timeLabel,"%d/%02d/%02d %02d:%02d:%02d",
		dataTime.year, dataTime.month, dataTime.day,
		dataTime.hour, dataTime.min, dataTime.sec);
	break;
	
      }
      
      numText++;
      prod.addText(timeLabel,
		   strikes[ii].latitude, strikes[ii].longitude,
		   serverParams->time_label_color,
		   serverParams->text_background_color,
		   serverParams->time_text_offset.x, 
		   serverParams->time_text_offset.y,
		   _symprodVertAlign, _symprodHorizAlign,
		   serverParams->text_font_size,
		   _symprodFontStyle, serverParams->font_name);

    }

    if ((serverParams->do_type_labelling) && (strikes[ii].type == LTG_CLOUD_STROKE)){
      prod.addText(serverParams->type_cloud_cloud_label,
		   strikes[ii].latitude, strikes[ii].longitude,
		   serverParams->type_label_color,
		   serverParams->text_background_color,
		   serverParams->type_text_offset.x, 
		   serverParams->type_text_offset.y,
		   _symprodVertAlign, _symprodHorizAlign,
		   serverParams->text_font_size,
		   _symprodFontStyle, serverParams->font_name); 
    }
  }


  // set return buffer
  
  if (_isVerbose) {
    prod.print(cerr);
  }
  prod.serialize(symprod_buf);

   if (serverParams->debug >= Params::DEBUG_NORM){
     cerr << "Products sent : " << numIcon << " icons and ";
     cerr << numText << " text messages - " << nstrikes << " strikes." << endl;
   }

  return(0);
  
}
