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

  if (prod_id != SPDB_SNDG_PLUS_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_SNDG_PLUS_ID: " << SPDB_SNDG_PLUS_ID << endl;
    return -1;
  }

  Params *serverParams = (Params*) params;

  // Copy the SPDB data to the local buffer, and byte-swap
  
  Sndg sndg;

  if (sndg.disassemble(spdb_data, spdb_len)) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "   Cannot disassemble chunk. " << endl;
  }

  
  // set up the icon

  int icon_size = serverParams->icon_size;

  // Positive + Icon
  Symprod::ppt_t icon[5] =
  {
    { -icon_size,  0 },
    {  icon_size,  0 }, 
    {  0,          0 }, 
    {  0,          -icon_size }, 
    {  0,          icon_size  }
  };

  // create Symprod object
  
  time_t now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "SoundingPlus");

    // create the arrays of locations for all Soundings
    
    MemBuf Buf;

	Sndg::header_t sndg_head =  sndg.getHeader();

	Symprod::wpt_t wpt;

	wpt.lat = sndg_head.lat;
	wpt.lon = sndg_head.lon;
	Buf.add(&wpt, sizeof(wpt));

    // Add icon arrays
    prod.addStrokedIcons(serverParams->display_color,
			 5, icon,
			 Buf.getLen() / sizeof(Symprod::wpt_t),
			 (Symprod::wpt_t *) Buf.getPtr(),
			 0,0,serverParams->line_width);

  //
  // Label the ID's, if desired.
  //

  if (serverParams->do_id_labelling){
      prod.addText(sndg_head.siteName,
		   sndg_head.lat, sndg_head.lon,
		   serverParams->id_label_color,
		   serverParams->text_background_color,
		   serverParams->id_text_offset.x, 
		   serverParams->id_text_offset.y,
		   _symprodVertAlign, _symprodHorizAlign,
		   serverParams->text_font_size,
		   _symprodFontStyle, serverParams->font_name);

  }
  //
  // Label the times, if desired.
  //

  if (serverParams->do_time_labelling){
      date_time_t dataTime;

      dataTime.unix_time = sndg_head.launchTime;
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
      
      prod.addText(timeLabel,
		   sndg_head.lat, sndg_head.lon,
		   serverParams->time_label_color,
		   serverParams->text_background_color,
		   serverParams->time_text_offset.x, 
		   serverParams->time_text_offset.y,
		   _symprodVertAlign, _symprodHorizAlign,
		   serverParams->text_font_size,
		   _symprodFontStyle, serverParams->font_name);

  }




  // set return buffer
  
  if (_isVerbose) {
    prod.print(cerr);
  }
  prod.serialize(symprod_buf);

  return(0);
  
}
