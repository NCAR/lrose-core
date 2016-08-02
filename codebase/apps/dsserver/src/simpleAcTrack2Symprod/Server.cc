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


#include <cstdio>
#include <rapformats/ac_posn.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include <Spdb/DsSpdbMsg.hh>

#include <stdio.h> // For the formatting of the time labels

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

  if (prod_id != SPDB_AC_POSN_ID)
  {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_AC_POSN_ID: " << SPDB_AC_POSN_ID << endl;

    return -1;
  }
  const string routine_name = "convertToSymprod";
  Params *serverParams = (Params*) params;


  // If we are doing temporal pruning, check that first.

  if (serverParams->do_temporal_pruning){

    double actualTime = double(chunk_ref.valid_time);    

    double roundedTime = double(serverParams->temporal_pruning.temporalRound) *
      rint(actualTime / double(serverParams->temporal_pruning.temporalRound));
    
    double lowerBound = roundedTime - serverParams->temporal_pruning.temporalTolerance;
    double upperBound = roundedTime + serverParams->temporal_pruning.temporalTolerance;

    if (
	(actualTime < lowerBound) ||
	(actualTime > upperBound)
	){
      return 0;
    }

  }


  // Convert the SPDB data to ac_posn type.

  ac_posn_t *A = (ac_posn_t *) spdb_data;
  BE_to_ac_posn(A);

  // create Symprod object

  time_t now = time(NULL);

  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       SPDB_AC_POSN_LABEL);

  //
  // Set the color up according to the callsign, if requested.
  //
  char *colorToUse = serverParams->text_color; // The default.
  //
  for (int i=0; i < serverParams->callsign_colors_n; i++){
    if (!(strncmp(serverParams->_callsign_colors[i].callsignSubString,
		  A->callsign, strlen(serverParams->_callsign_colors[i].callsignSubString)))){
      colorToUse = serverParams->_callsign_colors[i].color;
      break;
    }
  }

  if (
      (serverParams->do_text_labelling) ||
      (serverParams->do_time_labelling)
      ){

    char label[64];
    memset(label, 0, 64); // Probably a little excessive but best to make sure

    if (serverParams->do_text_labelling)
      sprintf(label, "%s", A->callsign);

    if (serverParams->do_time_labelling){
      date_time_t T;
      T.unix_time = chunk_ref.valid_time;
      uconvert_from_utime( &T );
      sprintf(label, "%s %02d:%02d",label, T.hour, T.min);
    }


    // Add the callsign string to the Symprod object

    prod.addText(label,
		 A->lat, A->lon,
		 colorToUse, serverParams->text_background_color,
		 serverParams->text_offset.x, serverParams->text_offset.y,
		 _symprodVertAlign, _symprodHorizAlign,
		 serverParams->text_font_size,
		 _symprodFontStyle, serverParams->font_name);
    
  } /* End of   if (serverParams->do_text_labelling) */



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
  wpt.lat = A->lat;
  wpt.lon = A->lon;
  crossBuf.add(&wpt, sizeof(wpt));


  prod.addStrokedIcons(colorToUse,
		       5, cross_icon,
		       1,
		       (Symprod::wpt_t *) crossBuf.getPtr(),
		       0,0,serverParams->cross_line_width);


  // set return buffer

  if (_isVerbose)
    prod.print(cerr);

  prod.serialize(symprod_buf);

  return 0;
}

