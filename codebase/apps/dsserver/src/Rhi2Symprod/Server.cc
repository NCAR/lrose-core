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
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2003
//
///////////////////////////////////////////////////////////////

#include <toolsa/toolsa_macros.h>
#include <toolsa/pjg.h>
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

  // Set up the rendering parameters.
  // Convert param values to Symprod object values

  switch (localParams->tick_line_style) {
  case Params::LINETYPE_SOLID :
    _tickLineType = Symprod::LINETYPE_SOLID;
    break;
  case Params::LINETYPE_DASH :
    _tickLineType = Symprod::LINETYPE_DASH;
    break;
  case Params::LINETYPE_DOT_DASH :
    _tickLineType = Symprod::LINETYPE_DOT_DASH;
    break;
  } /* endswitch - localParams->text_vert_align */

  switch (localParams->time_label_vert_align) {
  case Params::VERT_ALIGN_TOP :
    _timeLabelVertAlign = Symprod::VERT_ALIGN_TOP;
    break;
  case Params::VERT_ALIGN_CENTER :
    _timeLabelVertAlign = Symprod::VERT_ALIGN_CENTER;
    break;
  case Params::VERT_ALIGN_BOTTOM :
    _timeLabelVertAlign = Symprod::VERT_ALIGN_BOTTOM;
    break;
  } /* endswitch - localParams->text_vert_align */

  switch (localParams->time_label_horiz_align) {
  case Params::HORIZ_ALIGN_LEFT :
    _timeLabelHorizAlign = Symprod::HORIZ_ALIGN_LEFT;
    break;
  case Params::HORIZ_ALIGN_CENTER :
    _timeLabelHorizAlign = Symprod::HORIZ_ALIGN_CENTER;
    break;
  case Params::HORIZ_ALIGN_RIGHT :
    _timeLabelHorizAlign = Symprod::HORIZ_ALIGN_RIGHT;
    break;
  } /* endswitch - localParams->text_horiz_align */
  
  switch (localParams->time_label_font_style) {
  case Params::TEXT_NORM :
    _timeLabelFontStyle = Symprod::TEXT_NORM;
    break;
  case Params::TEXT_BOLD :
    _timeLabelFontStyle = Symprod::TEXT_BOLD;
    break;
  case Params::TEXT_ITALICS :
    _timeLabelFontStyle = Symprod::TEXT_ITALICS;
    break;
  case Params::TEXT_SUBSCRIPT :
    _timeLabelFontStyle = Symprod::TEXT_SUBSCRIPT;
    break;
  case Params::TEXT_SUPERSCRIPT :
    _timeLabelFontStyle = Symprod::TEXT_SUPERSCRIPT;
    break;
  case Params::TEXT_UNDERLINE :
    _timeLabelFontStyle = Symprod::TEXT_UNDERLINE;
    break;
  case Params::TEXT_STRIKETHROUGH :
    _timeLabelFontStyle = Symprod::TEXT_STRIKETHROUGH;
    break;
  } /* endswitch - localParams->text_font_style */
  
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

  if (prod_id != SPDB_GENERIC_POINT_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_GENERIC_POINT_ID: "
	 << SPDB_GENERIC_POINT_ID << endl;
    return -1;
  }
  
  Params *serverParams = (Params*) params;

  // create the GenPt object from the buffer

  GenPt rhi;
  if (rhi.disassemble(spdb_data, spdb_len)) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Cannot disassemble buffer into GenPt object" << endl;
    return -1;
  }
  
  // create Symprod object
  
  time_t now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "RHI");

  // add the tick marks for each azimuth

  int nRhi = rhi.getNLevels();
  int azField = rhi.getFieldNum("azimuth");
  int nGatesField = rhi.getFieldNum("n_gates");
  int startRangeField = rhi.getFieldNum("start_range");
  int gateSpacingField = rhi.getFieldNum("gate_spacing");

  for (int ii = 0; ii < nRhi; ii++) {

    double az = rhi.get2DVal(ii, azField);
    double nGates = rhi.get2DVal(ii, nGatesField);
    double startRange = rhi.get2DVal(ii, startRangeField);
    double gateSpacing = rhi.get2DVal(ii, gateSpacingField);
    double maxRange = startRange + nGates * gateSpacing;

    for (int jj = 0; jj < serverParams->tick_range_n; jj++) {
      
      double startRange = serverParams->_tick_range[jj].start_range;
      double endRange = serverParams->_tick_range[jj].end_range;

      if (startRange > maxRange) {
	continue;
      }
      if (endRange > maxRange) {
	endRange = maxRange;
      }

      double lat0, lon0;
      double lat1, lon1;
      
      PJGLatLonPlusRTheta(rhi.getLat(), rhi.getLon(),
			  startRange, az, &lat0, &lon0);
    
      PJGLatLonPlusRTheta(rhi.getLat(), rhi.getLon(),
			  endRange, az, &lat1, &lon1);
      
      Symprod::wpt_t tick[2];
      tick[0].lat = lat0;
      tick[0].lon = lon0;
      tick[1].lat = lat1;
      tick[1].lon = lon1;
    
      prod.addPolyline(2, tick, serverParams->tick_color,
		       _tickLineType, serverParams->tick_line_width);
      
    } // jj

  } // ii
  
  // radar icon

  int icon_size = serverParams->radar_icon_size;

  const int radarIconNpts = 5;
  Symprod::ppt_t radar_icon[radarIconNpts] = {
    {-icon_size, 0}, { icon_size, 0}, { 0, 0}, 
    { 0, -icon_size}, { 0, icon_size }
  };
  
  Symprod::wpt_t rpos;
  rpos.lat = rhi.getLat();
  rpos.lon = rhi.getLon();
  
  prod.addStrokedIcons(serverParams->radar_icon_color,
		       radarIconNpts, radar_icon, 1, &rpos,
		       0, 0, serverParams->radar_icon_line_width);

  // Label the time, if required
  //

  if (serverParams->label_time) {

    date_time_t dataTime;
    dataTime.unix_time = rhi.getTime();
    uconvert_from_utime(&dataTime);
    char timeLabel[64];
    
    switch( serverParams->time_format ){
      
    case Params::TIME_LABEL_HHMM :
      sprintf(timeLabel,"RHI - %02d:%02d",
	      dataTime.hour, dataTime.min);
      break;
      
    case Params::TIME_LABEL_HHMMSS :
      sprintf(timeLabel,"RHI - %02d:%02d:%02d",
	      dataTime.hour, dataTime.min, dataTime.sec);
      break;

    case Params::TIME_LABEL_YYYYMMDDHHMMSS :
    default :
      sprintf(timeLabel,"RHI - %d/%02d/%02d %02d:%02d:%02d",
	      dataTime.year, dataTime.month, dataTime.day,
	      dataTime.hour, dataTime.min, dataTime.sec);
      break;
      
    }
      
    prod.addText(timeLabel,
		 rhi.getLat(), rhi.getLon(),
		 serverParams->time_label_color,
		 serverParams->time_label_background_color,
		 serverParams->time_text_offset.x, 
		 serverParams->time_text_offset.y,
		 _timeLabelVertAlign, _timeLabelHorizAlign,
		 serverParams->time_label_font_size,
		 _timeLabelFontStyle, serverParams->time_label_font_name);

  } // if (serverParams->label_time)

  // set return buffer
  
  if (_isVerbose) {
    prod.print(cerr);
  }
  prod.serialize(symprod_buf);

  return 0;
  
}
