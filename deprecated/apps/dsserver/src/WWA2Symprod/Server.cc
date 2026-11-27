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
// Frank Hage  RAL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
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

  if (prod_id != SPDB_NWS_WWA_ID) {
    return -1;
  }

  Params *serverParams = (Params*) params;

  MemBuf mbuf;
  time_t now;
  
  // Copy the SPDB data to the local buffer
  mbuf.load(spdb_data, spdb_len);
  
  // Convert the SPDB data to native format so we can use it.

  NWS_WWA W;  // Instantiate a Warning/Watch Object.
  if (!W.disassemble(mbuf.getPtr(), mbuf.getLen())) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Error disassembling NWS_WWA buffer" << endl;
    
    return -1;
  }
 
  if (W._hdr.action == ACT_CAN ){
     cerr << "WARNING: Action is CAN, event was cancelled " << endl;
     return -1;
  }
   
  // create Symprod object

  now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "NWS_WWA");

  // Convert the SPDB data to symprod format

  _convert2Symprod(serverParams, prod, W, chunk_ref.data_type, chunk_ref.data_type2);
  
  // set return buffer

  if (_isVerbose) prod.print(cerr);

  prod.serialize(symprod_buf);

  return 0;
}


//////////////////////////////////////////////////////////////////////
// _convert2Symprod() - Add the SYMPROD objects for the WWA
//  to the product buffer. 

void Server::_convert2Symprod(Params *serverParams, Symprod &prod, NWS_WWA &W,
                              int data_type, int data_type2)
{


  double clat, clon;
  //
  // Get the centroid of the polyline points.
   _calcCentroid(W, clat, clon);

  // Label the polyline, if indicated.
  if ( serverParams->plot_id ){

    // Assemble the string.
    char labelString[256];
    string s1 = Spdb::dehashInt32To4Chars(data_type);
    string s2 = Spdb::dehashInt32To4Chars(data_type2);
    string s3 = s1 + string(" etn: ") + s2; 
    sprintf(labelString, serverParams->id_format_string,s3.c_str());

    // Add this ID text to the prod.
    prod.addText(labelString,
		 clat, clon, wwa[W._hdr.hazard_type -1].color,
		 serverParams->id_label.background_color,
		 serverParams->id_label.x_offset,
		 serverParams->id_label.y_offset,
		 (Symprod::vert_align_t) serverParams->id_label.vert_align,
		 (Symprod::horiz_align_t) serverParams->id_label.horiz_align,
		 serverParams->id_label.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->id_label.font_name,
		 0, // Object ID
		 Symprod::DETAIL_LEVEL_NONE);
  }


 
  MemBuf pointBuf;
  Symprod::wpt_t point;

  // Add the outline, making sure that the shape is closed 
  for (unsigned i = 0; i < W._hdr.num_points; ++i) {
    point.lat = W._hdr.wpt[i].lat;
    point.lon = W._hdr.wpt[i].lon;
    pointBuf.add(&point, sizeof(point));
  } 

  // Close the polygon
  point.lat = W._hdr.wpt[0].lat;
  point.lon = W._hdr.wpt[0].lon;
  pointBuf.add(&point, sizeof(point));
  
  // Add the polyline to the product
  int npts = pointBuf.getLen() / sizeof(Symprod::wpt_t);
 
  // Set defaults for color, line type, and line width 
  const char *polylineColor = wwa[W._hdr.hazard_type -1].color;
  Params::line_type_t lineType = serverParams->suggested_line_type;
  int lineWidth = serverParams->suggested_line_width;
 
  // Search for hazard in warn_config_override list, if found use those 
  // values for color, line type, line width. 
  for (int i = 0; i < serverParams->warn_config_override_n; i++)
  {
      if ( serverParams->_warn_config_override[i].hazard == W._hdr.hazard_type)
      {
        polylineColor = serverParams->_warn_config_override[i].hex_color_str;
        lineType =  serverParams->_warn_config_override[i].line_type;
        lineWidth = serverParams->_warn_config_override[i].line_width;
        cerr << "Found override for warn config : hazard: " << (int) W._hdr.hazard_type <<  " line color: " << polylineColor << " line width: " << lineWidth << endl; 
        i = serverParams->warn_config_override_n; 
      }
  }
  prod.addPolyline(npts,
		   (Symprod::wpt_t *) pointBuf.getPtr(),
		   polylineColor,
		   _convertLineTypeParam(lineType),
		   lineWidth,
		   _convertCapstyleParam(serverParams->suggested_capstyle),
		   _convertJoinstyleParam(serverParams->suggested_joinstyle));
  
  // Add the raw WWA to the prod as Hidden, popup text.
   prod.addText(W._text.c_str(),
		 clat, clon, wwa[W._hdr.hazard_type -1].color,
		 serverParams->wwa_text.background_color,
		 serverParams->wwa_text.x_offset,
		 serverParams->wwa_text.y_offset,
		 (Symprod::vert_align_t)
		 serverParams->wwa_text.vert_align,
		 (Symprod::horiz_align_t)
		 serverParams->wwa_text.horiz_align,
		 serverParams->wwa_text.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->wwa_text.font_name,
		 0, // Object ID
		 Symprod::DETAIL_LEVEL_USUALLY_HIDDEN |  Symprod::DETAIL_LEVEL_SHOWAS_POPOVER | Symprod::DETAIL_LEVEL_IGNORE_TRESHOLDS );
  
  return;
}

//////////////////////////////////////////////////////////////////////
// Calculate the centroid of a polyline.
//  return data in centroid_lat, centroid_lon

void Server::_calcCentroid(NWS_WWA &W, double &centroid_lat, double &centroid_lon)
{ 
  centroid_lat = centroid_lon = 0.0;

  for (unsigned ip=0; ip < W._hdr.num_points; ip++ ){
    centroid_lat += W._hdr.wpt[ip].lat;
	centroid_lon += W._hdr.wpt[ip].lon;
  }

  centroid_lat /= W._hdr.num_points;
  centroid_lon /= W._hdr.num_points;

  return;
}


//////////////////////////////////////////////////////////////////////
// _convertCapstyleParam() - Convert the TDRP capstyle parameter to
//                           the matching symprod value.

Symprod::capstyle_t Server::_convertCapstyleParam(int capstyle)
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
// _convertJoinstyleParam() - Convert the TDRP joinstyle parameter to
//                            the matching symprod value.

Symprod::joinstyle_t Server::_convertJoinstyleParam(int joinstyle)
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
// _convertLineTypeParam() - Convert the TDRP line type parameter to
//                           the matching symprod value.

Symprod::linetype_t Server::_convertLineTypeParam(int line_type)
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

