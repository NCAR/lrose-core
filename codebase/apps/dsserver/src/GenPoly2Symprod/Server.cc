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
// December 2003
//
///////////////////////////////////////////////////////////////

#include <toolsa/toolsa_macros.h>

#include "Server.hh"

using namespace std;

/*----------------------------------------------------------------*/
static string _mapping(const double v, double *x, const int nx,
		       char **y, const int ny,
		       const char *polyline_color)
{
  if (nx != ny)
  {
    printf("Ooops %d %d\n", nx, ny);
    return polyline_color;
  }

  if (v <= x[0]) return y[0];
  if (v >= x[nx-1]) return y[nx-1];

  for (int i=1; i<nx; ++i)
    if (v >= x[i-1] && v <= x[i])
      return y[i];
  return y[nx-1];
}

/*----------------------------------------------------------------*/
static string  _get_color(Params *p, const GenPoly &polyline)
{
  if (p->polyline_color_mapped)
  {
    return _mapping(polyline.get1DVal(0), 
		    p->_polyline_color_mapped_x, p->polyline_color_mapped_x_n,
		    p->_polyline_color_mapped_y, p->polyline_color_mapped_y_n,
		    p->polyline_color);
  }
  else
  {
    return p->polyline_color;
  }
}

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

  if (prod_id != SPDB_GENERIC_POLYLINE_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_GENERIC_POLYLINE_ID: "
	 << SPDB_GENERIC_POLYLINE_ID << endl;

    return -1;
  }

  Params *serverParams = (Params*) params;

  MemBuf polylineBuf;
  time_t now;
  
  // Copy the SPDB data to the local buffer
  
  polylineBuf.load(spdb_data, spdb_len);
  
  // Convert the SPDB data to native format so we can use it.

  GenPoly polyline;
  if (!polyline.disassemble(polylineBuf.getPtr(), polylineBuf.getLen()))
  {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Error disassembling GenPoly buffer" << endl; 
    return -1;
  }
  
  // create Symprod object

  now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "GenPoly");

  // Convert the SPDB data to symprod format

  char polyline_color[64];

  if (serverParams->render_met_mode)
    {
      _setMetModePolyColor( polyline_color, polyline);
    }
  else
    {
      string color = _get_color(serverParams, polyline);
      sprintf(polyline_color, "%s", color.c_str());
    }
   
   _addPolygon(serverParams, prod, polyline,
	       polyline_color, serverParams->polyline_dashed);
  
  // Label the polyline, if indicated.

  if ( serverParams->plot_genpoly_id )
    _addText(polyline.getId(), serverParams->id_format_string,
	     prod, polyline,
	     polyline_color,
	     serverParams->id_label.background_color,
	     serverParams->id_label.x_offset,
	     serverParams->id_label.y_offset,
	     serverParams->id_label.font_size,
	     serverParams->id_label.font_name,
	     _convertVertAlignParam(serverParams->id_label.vert_align),
	     _convertHorizAlignParam(serverParams->id_label.horiz_align));
  
  // put int the text, if indicated

  if ( serverParams->plot_text)
  {
    string s = polyline.getText();
    _addText(s.c_str(), "%s",  prod, polyline,
	     serverParams->text_info.text_color,
	     serverParams->text_info.background_color,
	     serverParams->text_info.x_offset,
	     serverParams->text_info.y_offset,
	     serverParams->text_info.font_size,
	     serverParams->text_info.font_name,
	     _convertVertAlignParam(serverParams->text_info.vert_align),
	     _convertHorizAlignParam(serverParams->text_info.horiz_align));
  }

  // Add vectors, if requested

  if (serverParams->plot_vectors)
    {
      if( serverParams->render_met_mode)
	_addMetModeVectors(serverParams, prod, polyline, polyline_color);
      else
	_addForecastVector(serverParams, prod, polyline);
    }
  
  // Display any requested fields

  for (int i = 0; i < serverParams->display_fields_n; ++i)
  {
    int field_num =
      polyline.getFieldNum(serverParams->_display_fields[i].field_name);
    
    if (field_num < 0)
      continue;
    
    char format_string[80];

    if (serverParams->show_field_names){
      if (serverParams->_display_fields[i].display_units)
	sprintf(format_string, "%s %s (%s)",
		serverParams->_display_fields[i].field_name,
		serverParams->_display_fields[i].format_string,
		polyline.getFieldUnits(field_num).c_str());
      else
	sprintf(format_string, "%s %s",
		serverParams->_display_fields[i].field_name,
		serverParams->_display_fields[i].format_string);
    } else {
      if (serverParams->_display_fields[i].display_units)
	sprintf(format_string, "%s (%s)",
		serverParams->_display_fields[i].format_string,
		polyline.getFieldUnits(field_num).c_str());
      else
	sprintf(format_string, "%s",
		serverParams->_display_fields[i].format_string);
    }
    
    _addText(polyline.get1DVal(field_num), format_string,
	     prod, polyline,
	     serverParams->_display_fields[i].text_color,
	     serverParams->_display_fields[i].background_color,
	     serverParams->_display_fields[i].x_offset,
	     serverParams->_display_fields[i].y_offset,
	     serverParams->_display_fields[i].font_size,
	     serverParams->_display_fields[i].font_name,
	     _convertVertAlignParam(serverParams->_display_fields[i].vert_align),
	     _convertHorizAlignParam(serverParams->_display_fields[i].horiz_align));
  } /* endfor - i */
  


  //
  // Add time labels, if requested
  //
  if ((serverParams->display_time.displayValidTime) ||
      (serverParams->display_time.displayExpireTime) ||
      (serverParams->display_time.displayGenTime)
      ){

    char timeLabel[1024];
    memset(timeLabel,0,1024);


    bool ok = true;

    if (serverParams->display_time.displayGenTime){

      int field_num = polyline.getFieldNum("leadTime");
    
      if (field_num == -1) {
	ok = false;
      } else {

	int leadTime  = (int) polyline.get1DVal(field_num);
	date_time_t t;
	t.unix_time = chunk_ref.valid_time - leadTime;

	uconvert_from_utime( &t );
      
	sprintf(timeLabel, "%sGenerated %d/%02d/%02d %02d:%02d:%02d ",
		timeLabel, t.year, t.month, t.day, t.hour, t.min, t.sec);
      }
    }


    if ((ok) && (serverParams->display_time.displayValidTime)){

      date_time_t t;
      t.unix_time = chunk_ref.valid_time;
      uconvert_from_utime( &t );
      
      sprintf(timeLabel, "%sValid %d/%02d/%02d %02d:%02d:%02d ",
	      timeLabel, t.year, t.month, t.day, t.hour, t.min, t.sec);
      
    }

    if ((ok) && (serverParams->display_time.displayExpireTime)){

	date_time_t t;
	t.unix_time = chunk_ref.expire_time;
	uconvert_from_utime( &t );

	sprintf(timeLabel, "%sExpire %d/%02d/%02d %02d:%02d:%02d ",
		timeLabel, t.year, t.month, t.day, t.hour, t.min, t.sec);

    }

    double clat, clon;
    polyline.calcCentroid(clat, clon);

    prod.addText(timeLabel,
                 clat, clon,
		 serverParams->display_time.text_color,
		 serverParams->display_time.background_color,
		 serverParams->display_time.x_offset,
		 serverParams->display_time.y_offset,
		 _convertVertAlignParam(serverParams->display_time.vert_align),
		 _convertHorizAlignParam(serverParams->display_time.horiz_align),
		 serverParams->display_time.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->display_time.font_name);


  }

  // set return buffer

  if (_isVerbose)
    prod.print(cerr);

  prod.serialize(symprod_buf);

  return 0;
}

void Server::_addMetModeVectors(Params *serverParams,
                                Symprod &prod,
				                GenPoly &polyline, 
								char *polyline_color)
{
  int startLatFieldNum = polyline.getFieldNum("FcstClusterLat");
  int startLonFieldNum = polyline.getFieldNum("FcstClusterLon");

  int endLatFieldNum = polyline.getFieldNum("ObsClusterLat");
  int endLonFieldNum = polyline.getFieldNum("ObsClusterLon");
  
  double startLat = polyline.get1DVal(startLatFieldNum);
  double startLon = polyline.get1DVal(startLonFieldNum);
  double endLat = polyline.get1DVal(endLatFieldNum);
  double endLon = polyline.get1DVal(endLonFieldNum);
 
  //
  // Check that the data is not missing (== -9999.0)
  //
  if ( fabs(endLat + 9999.0) > .00001 &&  
       fabs(endLon + 9999.0) > .00001 &&
       fabs(startLat + 9999.0) > .00001 &&  
       fabs(startLon + 9999.0) > .00001 )
    { 
      prod.addArrowBothPts
	(polyline_color,
	 _convertLineTypeParam(serverParams->suggested_line_type),
	 serverParams->suggested_arrow_line_width,
	 _convertCapstyleParam(serverParams->suggested_capstyle),
	 _convertJoinstyleParam(serverParams->suggested_joinstyle),
	 startLat, startLon,
	 endLat, endLon,
	 serverParams->arrow_head_len,
	 serverParams->arrow_head_half_angle);
    }
}

void Server:: _setMetModePolyColor(char *polyline_color, GenPoly &polyline)
{
  //
  // Get the cluster group id and color each polyline according to 
  // cluster group id.
  //
  int clusterGrpFieldNum = (int) polyline.getFieldNum("ObjCat");
  
  int clusterNum = (int)polyline.get1DVal(clusterGrpFieldNum);
   
  if ( clusterNum % 10 == 0)
    sprintf(polyline_color, "blue");
  else if ( clusterNum % 10 == 1)
    sprintf(polyline_color, "cyan");
  else if ( clusterNum % 10 == 2)
    sprintf(polyline_color, "yellow");
  else if ( clusterNum % 10 == 3)
    sprintf(polyline_color, "red");
  else if ( clusterNum % 10 == 4)
    sprintf(polyline_color, "purple");
  else if ( clusterNum % 10 == 5)
    sprintf(polyline_color, "orange");
  else if ( clusterNum % 10 == 6)
    sprintf(polyline_color, "magenta");
  else if ( clusterNum % 10 == 7)
    sprintf(polyline_color, "brown");
  else if ( clusterNum % 10 == 8)
    sprintf(polyline_color, "salmon");
  else if ( clusterNum % 10 == 9)
    sprintf(polyline_color, "gold");

}


//////////////////////////////////////////////////////////////////////
// _addForecastVector() - Add an arrow to show forecast movement vector

void Server::_addForecastVector(Params *serverParams,
                                Symprod &prod,
				GenPoly &polyline)
{
  // Get the speed and direction values for the vector

  double speed, direction;
  
  if (serverParams->vector_field_names.use_speed_dir_fields)
  {
    int speed_field_num =
      polyline.getFieldNum(serverParams->vector_field_names.speed_field_name);
    int dir_field_num =
      polyline.getFieldNum(serverParams->vector_field_names.dir_field_name);
    
    if (speed_field_num < 0)
    {
      cerr << "WARNING: Couldn't find "
	   << serverParams->vector_field_names.speed_field_name
	   << " in GenPoly" << endl;
      cerr << "         Vector not rendered" << endl;
      
      return;
    }
    
    if (dir_field_num < 0)
    {
      cerr << "WARNING: Couldn't find "
	   << serverParams->vector_field_names.dir_field_name
	   << " in GenPoly" << endl;
      cerr << "         Vector not rendered" << endl;
      
      return;
    }
    
    speed = polyline.get1DVal(speed_field_num);
    direction = polyline.get1DVal(dir_field_num);
  }
  else
  {
    cerr << "WARNING: Using U/V vectors not yet implemented!!" << endl;
    cerr << "         Vectors not rendered" << endl;
    
    return;
  }
  
  // Get the location for the vector.  We will put the vectors at the
  // polygon centroids

  double centroid_lat, centroid_lon;
  
  _calcCentroid(polyline, centroid_lat, centroid_lon);
  
  // Calculate the length of the vector

  double length;

  if (serverParams->fixed_length_arrows)
    length = serverParams->arrow_shaft_length;
  else
    length = speed * serverParams->forecast_lead_time / 1000.0;
  
  // Finally, add the vector to the product

  prod.addArrowStartPt
    (serverParams->vector_color,
     _convertLineTypeParam(serverParams->suggested_line_type),
     serverParams->suggested_line_width,
     _convertCapstyleParam(serverParams->suggested_capstyle),
     _convertJoinstyleParam(serverParams->suggested_joinstyle),
     centroid_lat, centroid_lon,
     length, direction,
     serverParams->arrow_head_len,
     serverParams->arrow_head_half_angle);

}

//////////////////////////////////////////////////////////////////////
// _addPolygon() - Add a SYMPROD polyline object for the storm shape
//                 to the product buffer. Dashed for forecast shapes.

void Server::_addPolygon(Params *serverParams, 
                         Symprod &prod,
			 GenPoly &polyline,
			 char *color,
			 int dashed)
{

  // Make sure that the polyline is closed if indicated

  if (polyline.isClosed())
  {
    int num_vertices = polyline.getNumVertices();
    
    GenPoly::vertex_t first_vertex = polyline.getVertex(0);
    GenPoly::vertex_t last_vertex = polyline.getVertex(num_vertices - 1);
    
    if (first_vertex.lat != last_vertex.lat ||
	first_vertex.lon != last_vertex.lon)
      polyline.addVertex(first_vertex);
  }
  
  // load polyline data into mbuf_points

  MemBuf pointBuf;

  for (int i = 0; i < polyline.getNumVertices(); ++i)
  {
    GenPoly::vertex_t vertex = polyline.getVertex(i);
    
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
  
  // add the polyline

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
// Calculate the centroid of a polyline. This is a method on the
// polyline in the Euclid library but does not seem to be available
// for the GenPoly class.

void Server::_calcCentroid(GenPoly &polyline, 
			   double &centroid_lat, 
			   double &centroid_lon ){


  centroid_lat = centroid_lon = 0.0;

  for (int ip=0; ip < polyline.getNumVertices(); ip++ ){

    GenPoly::vertex_t V;
    V = polyline.getVertex( ip );

    centroid_lat += V.lat; centroid_lon += V.lon;

  }

  centroid_lat /= polyline.getNumVertices();
  centroid_lon /= polyline.getNumVertices();

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
// _convertHorizAlignParam() - Convert the TDRP horizontal alignment
//                             parameter to the matching symprod value.

Symprod::horiz_align_t Server::_convertHorizAlignParam(int horiz_align)
{
  switch (horiz_align)
  {
  case Params::HORIZ_ALIGN_LEFT :
    return(Symprod::HORIZ_ALIGN_LEFT);
    
  case Params::HORIZ_ALIGN_CENTER :
    return(Symprod::HORIZ_ALIGN_CENTER);
    
  case Params::HORIZ_ALIGN_RIGHT :
    return(Symprod::HORIZ_ALIGN_RIGHT);
  }
  
  return(Symprod::HORIZ_ALIGN_LEFT);
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

//////////////////////////////////////////////////////////////////////
// _convertVertAlignParam() - Convert the TDRP vertical alignment
//                            parameter to the matching symprod value.

Symprod::vert_align_t Server::_convertVertAlignParam(int vert_align)
{
  switch (vert_align)
  {
  case Params::VERT_ALIGN_TOP :
    return(Symprod::VERT_ALIGN_TOP);
    
  case Params::VERT_ALIGN_CENTER :
    return(Symprod::VERT_ALIGN_CENTER);
    
  case Params::VERT_ALIGN_BOTTOM :
    return(Symprod::VERT_ALIGN_BOTTOM);
  }
  
  return(Symprod::VERT_ALIGN_TOP);
}
