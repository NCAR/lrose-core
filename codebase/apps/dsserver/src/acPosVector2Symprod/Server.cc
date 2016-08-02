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
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2000
//
///////////////////////////////////////////////////////////////


#include <rapformats/acPosVector.hh>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include <Spdb/DsSpdbMsg.hh>

#include "Server.hh"
using namespace std;



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
  _symprodVertAlign(Symprod::VERT_ALIGN_CENTER),
  _symprodHorizAlign(Symprod::HORIZ_ALIGN_CENTER),
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

  if (prod_id != SPDB_AC_VECTOR_ID)
  {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_AC_VECTOR_ID: " << SPDB_AC_VECTOR_ID << endl;

    return -1;
  }

  const string routine_name = "convertToSymprod";
  Params *serverParams = (Params*) params;


  // Convert the SPDB data to an acPosVector

  acPosVector point;
  
  point.disassemble(spdb_data, spdb_len);

  //
  // If we don't have valid lat/lons for both points, return.
  //
  if (!( point.isComplete() ) ){
    if (serverParams->debug){
      cerr << "Ignoring incomplete point :" << endl;
      point.print(cerr);
    }
    return 0;
  }
  //
  // If we are checking the callsign, and this is not a
  // callsign we want, return.
  //
  bool skipThis = false;
  if (serverParams->checkCallsigns){
    skipThis = true;
    for (int k=0; k < serverParams->validCallsigns_n; k++){
      if (!(strcmp(serverParams->_validCallsigns[k], 
		   point.getCallsign().c_str()))){
	skipThis = false;
	break;
      }
    }
  }

  if (skipThis){
    if (serverParams->debug){
      cerr << "Skipping unlisted callsign : " << point.getCallsign() << endl;
    }
    return 0;
  }

  // create Symprod object

  time_t now = time(NULL);

  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       SPDB_AC_VECTOR_LABEL);


  //
  // Select the color to use.
  //
  char *colorToUse = serverParams->lineColor;

  if (serverParams->colorByCallsign){
    for (int i=0; i < serverParams->callsignColors_n; i++){
      if (0==strncmp(point.getCallsign().c_str(),
		     serverParams->_callsignColors[i].callSign,
		     strlen(serverParams->_callsignColors[i].callSign))){
	colorToUse = serverParams->_callsignColors[i].color;
	break;
      }   
    }
  }


  //
  // First, connect the start and end points. We always do this,
  // it's not optional.
  //

    MemBuf ptBuf;
    
    Symprod::wpt_t wpt;
    wpt.lat = point.getLatPrevious();
    wpt.lon = point.getLonPrevious();
    ptBuf.add(&wpt, sizeof(wpt));
    
    wpt.lat = point.getLatCurrent();
    wpt.lon = point.getLonCurrent();
    ptBuf.add(&wpt, sizeof(wpt));

    prod.addPolyline(2,
		     (Symprod::wpt_t *) ptBuf.getPtr(),
		     colorToUse,
		     _convertLineType(serverParams->display_line_type),
		     serverParams->display_line_width,
		     _convertCapstyle(serverParams->display_capstyle),
		     _convertJoinstyle(serverParams->display_joinstyle));



  //
  // Option to draw crosses at each point. Only do the second
  // point.
  //


  if (serverParams->draw_crosses){

    // Setup simple cross icon
    Symprod::ppt_t cross_icon[5] =
    {
      { -serverParams->cross_icon_size,  0                              },
      {  serverParams->cross_icon_size,  0                              }, 
      {  0,                              0                              }, 
      {  0,                              -serverParams->cross_icon_size }, 
      {  0,                              serverParams->cross_icon_size  }
    };


    MemBuf crossBuf;

    Symprod::wpt_t wpt;
    wpt.lat = point.getLatCurrent();
    wpt.lon = point.getLonCurrent();
    crossBuf.add(&wpt, sizeof(wpt));


    prod.addStrokedIcons(serverParams->crossColor,
			 5, cross_icon,
			 1,
			 (Symprod::wpt_t *) crossBuf.getPtr(),
			 0,0,serverParams->cross_line_width);


  } /* End of if (serverParams->draw_crosses) */


  if (serverParams->do_time_labelling){

    date_time_t dataTime;
    dataTime.unix_time = point.getTimeCurrent();
    uconvert_from_utime( &dataTime );
    //
    // Set the format for the label appropriately.
    //
    char timeLabel[64];

    switch( serverParams->time_format ){
      
    case Params::TIME_LABEL_HHMM :
      sprintf(timeLabel,"%02d:%02d",
	      dataTime.hour, dataTime.min);
      //
      break;

    case Params::TIME_LABEL_HHMMSS :
      sprintf(timeLabel,"%02d:%02d:%02d",
	      dataTime.hour, dataTime.min, dataTime.sec);
      //
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
		 point.getLatCurrent(), point.getLonCurrent(),
		 serverParams->timeLabelColor,
		 serverParams->text_background_color,
		 serverParams->time_text_offset.x, 
		 serverParams->time_text_offset.y,
		 _symprodVertAlign, _symprodHorizAlign,
		 serverParams->text_font_size,
		 _symprodFontStyle, serverParams->font_name);
    
  }

  if (serverParams->doAltLabelling){

    double alt = point.getAltCurrent();
    //
    // Set the format for the label appropriately.
    //
    char altLabel[64];
    sprintf(altLabel, serverParams->altLabelFormat, alt);

    prod.addText(altLabel,
		 point.getLatCurrent(), point.getLonCurrent(),
		 serverParams->altLabelColor,
		 serverParams->text_background_color,
		 serverParams->alt_text_offset.x, 
		 serverParams->alt_text_offset.y,
		 _symprodVertAlign, _symprodHorizAlign,
		 serverParams->text_font_size,
		 _symprodFontStyle, serverParams->font_name);
    
  }

  if (serverParams->doCallsignLabelling){
    //
    // Set the format for the label appropriately.
    //
    char callsignLabel[64];
    sprintf(callsignLabel, "%s", point.getCallsign().c_str());

    prod.addText(callsignLabel,
		 point.getLatCurrent(), point.getLonCurrent(),
		 serverParams->callsignLabelColor,
		 serverParams->text_background_color,
		 serverParams->callsign_text_offset.x, 
		 serverParams->callsign_text_offset.y,
		 _symprodVertAlign, _symprodHorizAlign,
		 serverParams->text_font_size,
		 _symprodFontStyle, serverParams->font_name);
    
  }




  // set return buffer

  if (_isVerbose)
    prod.print(cerr);

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

