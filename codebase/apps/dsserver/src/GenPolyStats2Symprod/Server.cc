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
// February 2009
//
///////////////////////////////////////////////////////////////

#include <toolsa/toolsa_macros.h>
#include <toolsa/str.h>

#include "Server.hh"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Constructor
//
// Inherits from DsSymprodServer
///

Server::Server(const string &prog_name,
	       Params *initialParams,
               bool    allowParamOverride)
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
   const char *routine_name = "loadLocalParams";

   Params  *localParams;
   char   **tdrpOverrideList = NULL;
   bool     expandEnvVars = true;

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
  static const string method_name = "Server::convertToSymprod()";

  // check prod_id

  if (prod_id != SPDB_GENERIC_POLYLINE_ID) {
    cerr << "ERROR - " << _executableName
         << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_GENERIC_POLYLINE_ID: "
	 << SPDB_GENERIC_POLYLINE_ID << endl;

    return -1;
  }

  Params *serverParams = (Params*) params;

  MemBuf polygonBuf;
  time_t now;
  
  // Copy the SPDB data to the local buffer
  
  polygonBuf.load(spdb_data, spdb_len);
  
  // Convert the SPDB data to native format so we can use it.

  GenPoly polygon;
  if (!polygon.disassemble(polygonBuf.getPtr(), polygonBuf.getLen()))
  {
    cerr << "ERROR - " << _executableName
         << ":Server::convertToSymprod" << endl;
    cerr << "  Error disassembling GenPoly buffer" << endl;
    
    return -1;
  }
  
  // Only render this polygon if it is within the defined vertical
  // limits.  Note that the _vertLimitsSet, _minHt and _maxHt members
  // are inherited from the DsSpdbServer class where they are set based
  // on the message received from CIDD.

  if (_vertLimitsSet)
  {
    // If this polygon doesn't have the elevation angle stored, we're
    // going to go ahead and render it.

    int field_num = polygon.getFieldNum("elev angle");
    
    if (field_num >= 0)
    {
      double elev_angle = polygon.get1DVal(field_num);
      
      if (elev_angle < _minHt || elev_angle > _maxHt)
      {
	if (serverParams->debug >= Params::DEBUG_NORM)
	{
	  cerr << "WARNING: " << method_name << endl;
	  cerr << "Polygon not being rendered because it is outside of the defined vertical limits" << endl;
	  cerr << "Min height = " << _minHt
	       << ", max height = " << _maxHt << endl;
	  cerr << "Polygon height = " << elev_angle << endl;
	}
	  
	return 0;
      } /* endif - elev_angle < ... */
    }
  } /* endif - _vertLimitsSet */
  
  // create Symprod object

  now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "GenPoly");

  // Convert the SPDB data to symprod format

  if (serverParams->render_polygon)
    _addPolygon(serverParams, prod, polygon,
		serverParams->polygon_color, serverParams->polygon_dashed);
  
  // Display the text

  _addText(serverParams, prod, polygon);
  
  // set return buffer

  if (_isVerbose)
    prod.print(cerr);

  prod.serialize(symprod_buf);

  return 0;
}


//////////////////////////////////////////////////////////////////////
// _addPolygon() - Add a SYMPROD polygon object to the product buffer.

void Server::_addPolygon(Params *serverParams, 
                         Symprod &prod,
			 GenPoly &polygon,
			 char *color,
			 int dashed)
{

  // Make sure that the polygon is closed if indicated

  if (polygon.isClosed())
  {
    int num_vertices = polygon.getNumVertices();
    
    GenPoly::vertex_t first_vertex = polygon.getVertex(0);
    GenPoly::vertex_t last_vertex = polygon.getVertex(num_vertices - 1);
    
    if (first_vertex.lat != last_vertex.lat ||
	first_vertex.lon != last_vertex.lon)
      polygon.addVertex(first_vertex);
  }
  
  // load polygon data into mbuf_points

  MemBuf pointBuf;

  for (int i = 0; i < polygon.getNumVertices(); ++i)
  {
    GenPoly::vertex_t vertex = polygon.getVertex(i);
    
    Symprod::wpt_t point;
    
    double prev_lat = 0.0;
    double prev_lon = 0.0;
    
    if (dashed && i > 0)
    {
      double dlon = vertex.lon - prev_lon;
      double dlat = vertex.lat - prev_lat;

      point.lon = prev_lon + 0.25 * dlon;
      point.lat = prev_lat + 0.25 * dlat;
      pointBuf.add(&point, sizeof(point));

      point.lon = Symprod::WPT_PENUP;
      point.lat = Symprod::WPT_PENUP;
      pointBuf.add(&point, sizeof(point));

      point.lon = prev_lon + 0.75 * dlon;
      point.lat = prev_lat + 0.75 * dlat;
      pointBuf.add(&point, sizeof(point));
    }
    
    point.lat = vertex.lat;
    point.lon = vertex.lon;
    pointBuf.add(&point, sizeof(point));
    
    prev_lat = vertex.lat;
    prev_lon = vertex.lon;

  } /* endfor i */
  
  // add the polygon

  int npts = pointBuf.getLen() / sizeof(Symprod::wpt_t);
  
  prod.addPolyline(npts,
		   (Symprod::wpt_t *) pointBuf.getPtr(),
		   color,
		   _convertLineTypeParam(serverParams->suggested_line_type),
		   serverParams->suggested_line_width,
		   _convertCapstyleParam(serverParams->suggested_capstyle),
		   _convertJoinstyleParam(serverParams->suggested_joinstyle));

  return;
  
}

//////////////////////////////////////////////////////////////////////
// _addText() - Add a SYMPROD text object containing the desired display
//              fields to the product.

void Server::_addText(Params *serverParams, 
		      Symprod &prod,
		      GenPoly &polygon)
{
  char value_string[80];
  string display_string = "";
  
  // Add the storm id to the display string

  if (serverParams->display_field_name)
    display_string += "storm id = ";
  sprintf(value_string, "%s", polygon.getName().c_str());
  display_string += string(value_string) + "\n";
  
  // Add the polygon number to the display string

  if (serverParams->display_field_name)
    display_string += "polygon # = ";
  sprintf(value_string, "%d", polygon.getId());
  display_string += string(value_string) + "\n";
  
  // Add the requested display fields

  for (int i = 0; i < serverParams->display_fields_n; ++i)
  {
    // Get the field information.  If we can't find the field, don't
    // display anything

    int field_num =
      polygon.getFieldNum(serverParams->_display_fields[i]);
    
    if (field_num < 0)
      continue;
    
    // Add the field name to the display string

    if (serverParams->display_field_name)
      display_string += string(serverParams->_display_fields[i]) + " = ";
    
    // Add the field value to the display string

    sprintf(value_string, serverParams->text_format_string,
	    polygon.get1DVal(field_num));
    display_string += value_string;
    
    // Add the field units to the display string

    if (serverParams->display_units)
      display_string += " " + polygon.getFieldUnits(field_num);

    // Add a return to the display string

    if (i < serverParams->display_fields_n - 1)
      display_string += "\n";
    
  } /* endfor - i */
  
  // Get the location for the text
  
  double top_lat = 0;
  double bottom_lat = 0;
  double right_lon = 0;
  
  for (int i = 0; i < polygon.getNumVertices(); ++i)
  {
    GenPoly::vertex_t vertex = polygon.getVertex(i);
    
    if (i == 0)
    {
      top_lat = vertex.lat;
      bottom_lat = vertex.lat;
      right_lon = vertex.lon;
    }
    else
    {
      if (top_lat < vertex.lat)
	top_lat = vertex.lat;
      if (bottom_lat > vertex.lat)
	bottom_lat = vertex.lat;
      if (right_lon < vertex.lon)
	right_lon = vertex.lon;
    }
  } /* endfor - i */

  double display_lat = (top_lat + bottom_lat) / 2.0;
  double display_lon = right_lon;
  
  // Add this text to the prod.
  
  prod.addText(display_string.c_str(),
	       display_lat, display_lon,
	       serverParams->text_color,
	       serverParams->text_background_color,
	       0, 5,
	       Symprod::VERT_ALIGN_CENTER,
	       Symprod::HORIZ_ALIGN_LEFT,
	       serverParams->font.size,
	       Symprod::TEXT_NORM,
	       serverParams->font.name);
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
