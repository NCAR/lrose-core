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
// March 1999
//
///////////////////////////////////////////////////////////////

#include <toolsa/toolsa_macros.h>
#include <rapformats/tstorm_hull_smooth.h>
#include <rapformats/tstorm_spdb.h>
#include <toolsa/TaStr.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/MemBuf.hh>
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
  _entryNum = -1;
  return;
}

void Server::setEntryNum(int ie){
  _entryNum = ie;
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

   *serverParams = (void*)localParams;
   return( 0 );
}


/////////////////////////////////////////////////////////////////////
// transformData() - Transform the data from the database into
//                   symprod format.

void Server::transformData(const void *serverParams,
			   const string &dir_path,
			   int prod_id,
			   const string &prod_label,
			   int n_chunks_in,
			   const Spdb::chunk_ref_t *chunk_refs_in,
			   const Spdb::aux_ref_t *aux_refs_in,
			   const void *chunk_data_in,
			   int &n_chunks_out,
			   MemBuf &refBufOut,
			   MemBuf &auxBufOut,
			   MemBuf &dataBufOut){

  Params *myParams = (Params*) serverParams;

  // initialize the buffers
  
  refBufOut.free();
  auxBufOut.free();
  dataBufOut.free();

  // Transform each chunk and add it to the memory buffers

  n_chunks_out = 0;
  MemBuf symprodBuf;
  
  for (int i = 0; i < n_chunks_in; i++) {

    Spdb::chunk_ref_t ref = chunk_refs_in[i];
    Spdb::aux_ref_t aux = aux_refs_in[i];
    void *chunk_data = (void *)((char *)chunk_data_in + ref.offset);

    symprodBuf.free();

    if (myParams->plot_individual_storms){

      //
      // In order to plot the storms as individual products,
      // we have to first unvravel the header and see how many
      // storms we have as entries in this chunk, then
      // loop through these entries adding them individually.
      //
      MemBuf tempBuf;
      tempBuf.load(chunk_data, ref.len);
      tstorm_spdb_buffer_from_BE((ui08 *) tempBuf.getPtr());
      tstorm_spdb_header_t *header = 
	(tstorm_spdb_header_t *) tempBuf.getPtr();


      for (int ie=0; ie < header->n_entries; ie++){

	setEntryNum( ie );	
	
	if (convertToSymprod(serverParams, dir_path, prod_id, prod_label,
			     ref, aux, chunk_data, ref.len,
			     symprodBuf) == 0) {
	  
	  
	  Spdb::chunk_ref_t myRef;
	  MEM_zero(myRef);
	  myRef.offset = dataBufOut.getLen();
	  myRef.len = symprodBuf.getLen();
	  refBufOut.add(&myRef, sizeof(myRef));

	  Spdb::aux_ref_t myAux;
	  MEM_zero(myAux);
	  myAux.write_time = (ti32) time(NULL);
	  auxBufOut.add(&myAux, sizeof(myAux));
	  dataBufOut.add(symprodBuf.getPtr(), symprodBuf.getLen());
	  n_chunks_out++; // Actually more accurately n_products_out
	}
      }
    } else {

      //
      // If we are not plotting the storms individually,
      // set the entry number to the overloaded value of -1.
      // This will cause all storms to be plotted as one product.
      //
      setEntryNum( -1 );

      if (convertToSymprod(serverParams, dir_path, prod_id, prod_label,
			   ref, aux, chunk_data, ref.len,
			   symprodBuf) == 0) {
	
	ref.offset = dataBufOut.getLen();
	ref.len = symprodBuf.getLen();
	refBufOut.add(&ref, sizeof(ref));
	auxBufOut.add(&aux, sizeof(aux));
	dataBufOut.add(symprodBuf.getPtr(), symprodBuf.getLen());
	n_chunks_out++;
      }
    }
  }
  return;
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
  
  const char *routine_name = "convertToSymprod";
  Params *serverParams = (Params*) params;

  // check prod_id

  if (prod_id == SPDB_XML_ID) {
    
    // create an ascii buffer, make sure it is null-delimited

    MemBuf xmlBuf;
    xmlBuf.add(spdb_data, spdb_len);
    char cnull = '\0';
    xmlBuf.add(&cnull, 1);
    char *xml = (char *) xmlBuf.getPtr();
    
    int iret = 0;
    if (strstr(xml, "<tstorms") != NULL) {
      // TSTORMS XML
      if (_tstormsXmlToSymprod(serverParams, dir_path,
                               prod_id, prod_label,
                               chunk_ref, aux_ref,
                               xml, symprod_buf)) {
        iret = -1;
      }
    } else if (strstr(xml, "<wxml") != NULL) {
      // WXML
      if (_wxmlToSymprod(serverParams, dir_path,
                         prod_id, prod_label,
                         chunk_ref, aux_ref,
                         xml, symprod_buf)) {
        iret = -1;
      }
    } else {
      // do not recognize the buffer, return error
      iret = -1;
    }

    return iret;

  } // if (prod_id == SPDB_ASCII_ID)

  if (prod_id != SPDB_TSTORMS_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_TSTORMS_ID: " << SPDB_TSTORMS_ID << endl;
    return -1;
  }

  MemBuf tstormBuf;
  time_t now;
  
  // Copy the SPDB data to the local buffer

  tstormBuf.load(spdb_data, spdb_len);
  
  // Convert the SPDB data to native format so we can use it.

  tstorm_spdb_buffer_from_BE((ui08 *) tstormBuf.getPtr());

  // check buffer len

  tstorm_spdb_header_t *header = (tstorm_spdb_header_t *) tstormBuf.getPtr();

  int expected_len = tstorm_spdb_buffer_len(header);
  
  if (expected_len != spdb_len) {
    cerr << "ERROR - " << _executableName << "::" << routine_name << endl;
    cerr << "spdb_len is " << spdb_len << endl;
    cerr << "expected_len is " <<  expected_len << endl;
    cerr << "Aborting" << endl;
    return (-1);
  }

  // create Symprod object

  now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "TITAN storms");

  // Convert the SPDB data to symprod format

  tstorm_spdb_entry_t *entry = (tstorm_spdb_entry_t *)
    ((char *) tstormBuf.getPtr() + sizeof(tstorm_spdb_header_t));

  int ientry = 0;
  if (_entryNum != -1){
    ientry = _entryNum;
    entry += _entryNum;
  }
  
  do {

    // If there are no entries, break, we are done.

    if (header->n_entries == 0) break;


    if (serverParams->debug){
      cerr << "Processing entry number " << ientry << endl;
    }

    // Add the current shape polyline to the product buffer

    if (serverParams->plot_current)
      _addCurrentPolygon(serverParams, prod, header, entry);
    
    if (!serverParams->valid_forecasts_only || entry->forecast_valid) {

      if (_isDebug) {
	cerr << "Storm valid:" << endl;
	cerr << "     speed = " << entry->speed << 
	  ", top = " << entry->top << endl;
	cerr << "   num: " << entry->complex_track_num
	     << "/" << entry->simple_track_num << endl;
	cerr << "   lat/lon: " << entry->latitude
	     << "/" << entry->longitude << endl;
      }
      
      // Add the forecast shape polyline to the product buffer
      
      if (serverParams->plot_forecast) {
	_addForecastPolygon(serverParams, prod, header, entry);
      }
      
      // add arrow
      
      if (serverParams->plot_vectors) {
	_addForecastVector(serverParams, prod, entry);
      }
      
      // add text
      
      if (serverParams->plot_trend || 
          serverParams->plot_speed || 
          serverParams->plot_top ||
          serverParams->plot_track_numbers) {
	_addText(serverParams, prod, entry);
      }

    } else {

      if (_isDebug) {
	cerr <<"Skipping storm -- invalid" << endl;
	cerr << "     speed = " << entry->speed <<
	  ", top = " << entry->top << endl;
	cerr << "   num: " << entry->complex_track_num
	     << "/" << entry->simple_track_num << endl;
	cerr << "   lat/lon: " << entry->latitude
	     << "/" << entry->longitude << endl;
      }

    }
    
    if (ientry == header->n_entries-1) break;

    ientry++; entry++;
    
  } while(_entryNum == -1);
  
  // set return buffer

  if (_isVerbose) {
    prod.print(cerr);
  }
  prod.serialize(symprod_buf);

  return(0);
  
}


//////////////////////////////////////////////////////////////////////
// _addCurrentPolygon() - Add a SYMPROD polyline object for the
//                        current storm shape to the product buffer.

void Server::_addCurrentPolygon(Params *serverParams, 
                                Symprod &prod,
				tstorm_spdb_header_t *header,
				tstorm_spdb_entry_t *entry)
{

  tstorm_polygon_t tstorm_polygon;
  int npoints_polygon = header->n_poly_sides + 1;

  // load up polygon, 0 lead time (current pos)
  
  if (serverParams->storm_shape == Params::POLYGON_SHAPE) {

    if (serverParams->hull_smooth) {
      tstorm_growth_hull_smooth(header, entry,
                                serverParams->inner_bnd_multiplier,
                                serverParams->outer_bnd_multiplier,
                                &tstorm_polygon,
                                &npoints_polygon,
                                0,
                                0,
                                serverParams->grow_forecast);
    } else {
      tstorm_spdb_load_growth_polygon(header, entry,
			              &tstorm_polygon,
			              0.0,
                                      serverParams->grow_forecast);
    }

  } else {

    tstorm_spdb_load_growth_ellipse(header, entry,
                                    &tstorm_polygon,
                                    0.0,
                                    serverParams->grow_forecast);

  }
  
  // add polygon to prod struct  
  
  _addPolygon(serverParams, 
              prod, &tstorm_polygon,
	      npoints_polygon,
	      serverParams->current_color, FALSE);
  
}


//////////////////////////////////////////////////////////////////////
// _addForecastPolygon() - Add a SYMPROD polyline object for the
//                         forecast storm shape to the product buffer.

void Server::_addForecastPolygon(Params *serverParams,
                                 Symprod &prod,
				 tstorm_spdb_header_t *header,
				 tstorm_spdb_entry_t *entry)

{

  tstorm_polygon_t tstorm_polygon;
  int npoints_polygon = header->n_poly_sides + 1;
  
  // load up polygon, 0 lead time (current pos)
  
  if (serverParams->storm_shape == Params::POLYGON_SHAPE) {

    if (serverParams->hull_smooth) {

      tstorm_growth_hull_smooth(header, entry,
                                serverParams->inner_bnd_multiplier,
                                serverParams->outer_bnd_multiplier,
                                &tstorm_polygon,
                                &npoints_polygon,
                                serverParams->forecast_lead_time,
                                0,
                                serverParams->grow_forecast);

    } else {

      tstorm_spdb_load_growth_polygon(header, entry,
			              &tstorm_polygon,
			              serverParams->forecast_lead_time,
                                      serverParams->grow_forecast);

    }

  } else {

    tstorm_spdb_load_growth_ellipse(header, entry,
			            &tstorm_polygon,
			            serverParams->forecast_lead_time,
                                    serverParams->grow_forecast);
    
  }
  
  // add polygon to prod struct 
  
  _addPolygon(serverParams, 
              prod, &tstorm_polygon,
	      npoints_polygon,
	      serverParams->forecast_color,
	      serverParams->forecast_dashed);

}


//////////////////////////////////////////////////////////////////////
// _addPolygon() - Add a SYMPROD polyline object for the storm shape
//                 to the product buffer. Dashed for forecast shapes.

void Server::_addPolygon(Params *serverParams, 
                         Symprod &prod,
			 tstorm_polygon_t *tstorm_polygon,
			 int npoints_polygon,
			 char *color,
			 int dashed)
{

  MemBuf pointBuf;

  // load polyline data into mbuf_points

  tstorm_pt_t *pt = tstorm_polygon->pts;
  double prev_lat=0.0, prev_lon=0.0; // Assigned 0 to avoid compiler warnings.
  for (int ipt = 0; ipt < npoints_polygon; ipt++, pt++) {

    Symprod::wpt_t point;
    double dlat, dlon;
    
    if (dashed && ipt > 0) {

      dlon = pt->lon - prev_lon;
      dlat = pt->lat - prev_lat;

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
    
    point.lat = pt->lat;
    point.lon = pt->lon;
    pointBuf.add(&point, sizeof(point));
    
    prev_lat = pt->lat;
    prev_lon = pt->lon;

  } // ipt

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
// _addForecastVector() - Add an arrow to show forecast movement vector

void Server::_addForecastVector(Params *serverParams,
                                Symprod &prod,
				tstorm_spdb_entry_t *entry)

{

  double length;

  if (serverParams->fixed_length_arrows) {
    length = serverParams->arrow_shaft_length;
  } else {
    length = entry->speed * serverParams->forecast_lead_time / 3600.0;
  }

  if (serverParams->arrow_head_len_in_pixels) {
    prod.addArrowStartPt
      (serverParams->vector_color,
       _convertLineTypeParam(serverParams->suggested_line_type),
       serverParams->suggested_line_width,
       _convertCapstyleParam(serverParams->suggested_capstyle),
       _convertJoinstyleParam(serverParams->suggested_joinstyle),
       entry->latitude,
       entry->longitude,
       length, entry->direction,
       (int) (serverParams->arrow_head_len + 0.5),
       serverParams->arrow_head_half_angle);
  } else {
    prod.addArrowStartPt
      (serverParams->vector_color,
       _convertLineTypeParam(serverParams->suggested_line_type),
       serverParams->suggested_line_width,
       _convertCapstyleParam(serverParams->suggested_capstyle),
       _convertJoinstyleParam(serverParams->suggested_joinstyle),
       entry->latitude,
       entry->longitude,
       length, entry->direction,
       serverParams->arrow_head_len,
       serverParams->arrow_head_half_angle);
  }

}

//////////////////////////////////////////////////////////////////////
// _addText() - Add a SYMPROD text object for speed and growth.

void Server::_addText(Params *serverParams,
                      Symprod &prod,
		      tstorm_spdb_entry_t *entry)

{

  vector<string> textLines;
  char text[1024];

  //
  // Plot the speed, if requested.
  //
  if (serverParams->plot_speed) {
    double speed_kmh = entry->speed;
    string speedStr = _formatSpeed(serverParams, speed_kmh);
    textLines.push_back(speedStr);
  }
  
  // Trend.

  if (serverParams->plot_trend) {
    // Intensity_trend can be -1 (decreasing), 0, or 1 (increasing).
    // Same for size trend. We switch on the sum of the two.
    int sum_trend = entry->intensity_trend + entry->size_trend;
    if (sum_trend > 0) {
      sprintf(text, "Trend : +");
    } else if (sum_trend < 0) {
      sprintf(text, "Trend : -");
    } else if (sum_trend == 0) {
      sprintf(text, "Trend : 0");
    } 
    textLines.push_back(text);
  }

  // top text

  if (serverParams->plot_top) {
    if (serverParams->top_km) {
      sprintf(text, "Top : %d Km", (int)(entry->top + 0.5));
    } else {
      double top_100s_ft =
	entry->top / KM_PER_MI * FT_PER_MI / 100.0;
      sprintf(text, "Top : FL%d", (int)(top_100s_ft + 0.5));
    }
    textLines.push_back(text);
  }

  // Track numbers.

  if (serverParams->plot_track_numbers) {
    sprintf(text, "Number %d/%d", entry->complex_track_num, 
	    entry->simple_track_num );
    textLines.push_back(text);
  }
    
  // add the text lines

  if (serverParams->plot_text) {
    int currentOffset = 0;
    int  xOffset = serverParams->plot_text_horizontal_offset;  
    for (int ii = 0; ii < (int) textLines.size(); ii++) {
      prod.addText(textLines[ii].c_str(),
		   entry->latitude, entry->longitude,
		   serverParams->text_color,
		   serverParams->text_background_color,
		   xOffset, currentOffset,
		   Symprod::VERT_ALIGN_CENTER,
		   Symprod::HORIZ_ALIGN_LEFT,
		   serverParams->text_font_size,
		   Symprod::TEXT_NORM,
		   serverParams->text_font);
      currentOffset -= serverParams->plot_text_line_offset;
    }
  }
    
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

//////////////////////////////////////////////////////////////////////
// Convert XML storm data to symprod format.
// XML schema location:
//   http://www.bom.gov.au/bmrc/wefor/projects/b08fdp/WxML
// Returns 0 on success, -1 on failure

int Server::_tstormsXmlToSymprod(Params *serverParams,
                                 const string &dir_path,
                                 const int prod_id,
                                 const string &prod_label,
                                 const Spdb::chunk_ref_t &chunk_ref,
                                 const Spdb::aux_ref_t &aux_ref,
                                 const char *xml_str,
                                 MemBuf &symprod_buf)
  
{

  // clear cases vector
  
  _instances.clear();

  // create Symprod object
  
  time_t now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "TITAN storms from XML");

  // remove comments

  string bufWithComments = (char *) xml_str;
  string bufNoComments = TaXml::removeComments(bufWithComments);
  
  // find xml between tstorms tags
  
  string tstormsBuf;
  if (TaXml::readString(bufNoComments, "tstorms", tstormsBuf)) {
    cerr << "ERROR - Server::_xmlToSymprod" << endl;
    cerr << "  XML buffer has no <tstorms> tag" << endl;
    cerr << "========= decoding buffer ================" << endl;
    cerr << bufNoComments << endl;
    cerr << "==========================================" << endl;
    return -1;
  }

  // get nowcast data, if present
  
  string stormBuf;
  if (TaXml::readString(tstormsBuf, "storm-data", stormBuf)) {
    // no storm section
    return 0;
  }

  // get observation time
  
  time_t obsTime;
  if (TaXml::readTime(stormBuf, "observation-time", obsTime)) {
    cerr << "ERROR - Server::_xmlToSymprod" << endl;
    cerr << "  Cannot read <observation-time>" << endl;
    cerr << "========= storm buffer =================" << endl;
    cerr << stormBuf << endl;
    cerr << "==========================================" << endl;
    return -1;
  }

  // get array of storms

  vector<string> storms;
  if (TaXml::readTagBufArray(stormBuf, "storm", storms)) {
    // no storms
    return 0;
  }

  // loop through storms (simple track numbers)

  int iret = 0;
  for (int istorm = 0; istorm < (int) storms.size(); istorm++) {

    // get storm attributes

    string storm;
    vector<TaXml::attribute> attrs;
    
    if (TaXml::readString(storms[istorm], "storm", storm, attrs)) {
      cerr << "ERROR - Server::_xmlToSymprod" << endl;
      cerr << "  Cannot read <storm>" << endl;
      cerr << "========= storm buffer =================" << endl;
      cerr << storms[istorm] << endl;
      cerr << "========================================" << endl;
      iret = -1;
      continue;
    }

    // get storm id

    int stormId = -9999;
    if (TaXml::readIntAttr(attrs, "ID", stormId)) {
      cerr << "WARNING - Server::_xmlToSymprod" << endl;
      cerr << "  Cannot read storm ID" << endl;
      iret = -1;
    }

    // get list of instances for this storm
    // instance can be in the past (history), at the obs time (current),
    // or in the future (forecast)

    vector<string> instances;
    if (TaXml::readTagBufArray(storm, "instance", instances)) {
      // no instances
      continue;
    }
    
    // loop through the instances

    double prevLat = 0.0;
    double prevLon = 0.0;
    
    for (int iinst = 0; iinst < (int) instances.size(); iinst++) {
      
      // get instance attributes
      
      string instanceXml;
      vector<TaXml::attribute> attrs;
      
      if (TaXml::readString(instances[iinst], "instance", instanceXml, attrs)) {
	cerr << "ERROR - Server::_xmlToSymprod" << endl;
	cerr << "  Cannot read <instance>" << endl;
	cerr << "========= instance buffer =================" << endl;
	cerr << instances[iinst] << endl;
	cerr << "========================================" << endl;
	iret = -1;
	continue;
      }
      
      // get the instance properties
      
      StormInstance instance;
      if (_getInstanceProps(stormId, instanceXml, instance)) {
	iret = -1;
	continue;
      }
      _instances.push_back(instance);
      
      if (instance.time < obsTime && serverParams->plot_past) {
	
	// this is in the past
	
	_plotXmlShape(serverParams, prod,
		      serverParams->past_color,
		      instance);
	
	// plot arrow if prev lat/lon is available
	
	if (prevLat != 0.0 || prevLon != 0.0) {
	  _plotXmlVector(serverParams, prod,
			 prevLat, prevLon, instance.lat, instance.lon,
			 serverParams->past_color);
	}
	
	prevLat = instance.lat;
	prevLon = instance.lon;
	
      } else if (instance.time == obsTime && serverParams->plot_current) {
	
	// this is a current observation
	
	_plotXmlShape(serverParams, prod,
		      serverParams->current_color,
		      instance);
	
	// plot arrow if prev lat/lon is available

	if (prevLat != 0.0 || prevLon != 0.0) {
	  _plotXmlVector(serverParams, prod,
			 prevLat, prevLon, instance.lat, instance.lon,
			 serverParams->past_color);
	}
	
	// plot storm parameters as text

	_plotXmlText(serverParams, prod, instance);
	
	prevLat = instance.lat;
	prevLon = instance.lon;
	
      } else if (instance.time > obsTime && serverParams->plot_forecast) {
	
	// this is a forecast
	
	_plotXmlShape(serverParams, prod,
		      serverParams->forecast_color,
		      instance);
        
	// plot arrow if prev lat/lon is available
        
	if (prevLat != 0.0 || prevLon != 0.0) {
	  _plotXmlVector(serverParams, prod,
			 prevLat, prevLon, instance.lat, instance.lon,
			 serverParams->forecast_color);
	}
	
	prevLat = instance.lat;
	prevLon = instance.lon;
	
      }

    } // iinst

  } // istorm

  // plot vectors from parents to children, where relevant
  
  for (int iinst = 0; iinst < (int) _instances.size(); iinst++) {

    const StormInstance &parent = _instances[iinst];

    // loop through children
    
    for (int ichild = 0; ichild< (int) parent.childIds.size(); ichild++) {
      
      int childId = parent.childIds[ichild];
      
      // look for child ID, and if it has parents, draw a vector to it
      
      for (int jinst = 0; jinst < (int) _instances.size(); jinst++) {
	
	const StormInstance &child = _instances[jinst];
	if (childId == child.stormId &&
	    child.parentIds.size() > 0) {

	  _plotXmlVector(serverParams, prod,
			 parent.lat, parent.lon,
			 child.lat, child.lon,
			 serverParams->past_color);
	  
	}
	
      } // jinst

    } // ichild

  } // iinst

  // set return buffer

  if (_isVerbose) {
    prod.print(cerr);
  }
  prod.serialize(symprod_buf);

  return iret;

}

//////////////////////////////////////////////////////////////////////
// Convert XML storm data to symprod format.
// XML schema location:
//   http://www.bom.gov.au/bmrc/wefor/projects/b08fdp/WxML
// Returns 0 on success, -1 on failure

int Server::_wxmlToSymprod(Params *serverParams,
                           const string &dir_path,
                           const int prod_id,
                           const string &prod_label,
                           const Spdb::chunk_ref_t &chunk_ref,
                           const Spdb::aux_ref_t &aux_ref,
                           const char *xml_str,
                           MemBuf &symprod_buf)
  
{

  // clear cases vector

  _instances.clear();
  
  // create Symprod object
  
  time_t now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "TITAN storms from XML");

  // remove comments

  string bufWithComments = (char *) xml_str;
  string bufNoComments = TaXml::removeComments(bufWithComments);
  
  // find xml between wxml tags
  
  string wxmlBuf;
  if (TaXml::readString(bufNoComments, "wxml", wxmlBuf)) {
    cerr << "ERROR - Server::_xmlToSymprod" << endl;
    cerr << "  XML buffer has no <wxml> tag" << endl;
    cerr << "========= decoding buffer ================" << endl;
    cerr << bufNoComments << endl;
    cerr << "==========================================" << endl;
    return -1;
  }

  // get nowcast data, if present

  string nowcastBuf;
  if (TaXml::readString(wxmlBuf, "nowcast-data", nowcastBuf)) {
    // no nowcast section
    return 0;
  }

  // get observation time

  time_t obsTime;
  if (TaXml::readTime(nowcastBuf, "observation-time", obsTime)) {
    cerr << "ERROR - Server::_xmlToSymprod" << endl;
    cerr << "  Cannot read <observation-time>" << endl;
    cerr << "========= nowcast buffer =================" << endl;
    cerr << nowcastBuf << endl;
    cerr << "==========================================" << endl;
    return -1;
  }

  // get array of events

  vector<string> events;
  if (TaXml::readTagBufArray(nowcastBuf, "event", events)) {
    // no events
    return 0;
  }

  // loop through events (simple track numbers)

  int iret = 0;
  for (int ievent = 0; ievent < (int) events.size(); ievent++) {

    // get event attributes

    string event;
    vector<TaXml::attribute> attrs;
    
    if (TaXml::readString(events[ievent], "event", event, attrs)) {
      cerr << "ERROR - Server::_xmlToSymprod" << endl;
      cerr << "  Cannot read <event>" << endl;
      cerr << "========= event buffer =================" << endl;
      cerr << events[ievent] << endl;
      cerr << "========================================" << endl;
      iret = -1;
      continue;
    }

    // get event id

    int stormId = -9999;
    if (TaXml::readIntAttr(attrs, "ID", stormId)) {
      cerr << "WARNING - Server::_xmlToSymprod" << endl;
      cerr << "  Cannot read event ID" << endl;
      iret = -1;
    }

    // get list of cases for this event
    // case can be in the past (history), at the obs time (current),
    // or in the future (forecast)

    vector<string> instances;
    if (TaXml::readTagBufArray(event, "case", instances)) {
      // no cases
      continue;
    }
    
    // loop through the instances

    double prevLat = 0.0;
    double prevLon = 0.0;
    
    for (int iinst = 0; iinst < (int) instances.size(); iinst++) {
      
      // get instance attributes
      
      string instanceXml;
      vector<TaXml::attribute> attrs;
      
      if (TaXml::readString(instances[iinst], "case", instanceXml, attrs)) {
	cerr << "ERROR - Server::_xmlToSymprod" << endl;
	cerr << "  Cannot read <case>" << endl;
	cerr << "========= case buffer =================" << endl;
	cerr << instances[iinst] << endl;
	cerr << "========================================" << endl;
	iret = -1;
	continue;
      }

      // get the case properties

      StormInstance instance;
      if (_getCaseProps(stormId, instanceXml, instance)) {
	iret = -1;
	continue;
      }
      _instances.push_back(instance);
      
      if (instance.time < obsTime && serverParams->plot_past) {
	
	// this is in the past
	
	_plotXmlShape(serverParams, prod,
		      serverParams->past_color,
		      instance);
	
	// plot arrow if prev lat/lon is available
	
	if (prevLat != 0.0 || prevLon != 0.0) {
	  _plotXmlVector(serverParams, prod,
			 prevLat, prevLon, instance.lat, instance.lon,
			 serverParams->past_color);
	}
	
	prevLat = instance.lat;
	prevLon = instance.lon;
	
      } else if (instance.time == obsTime && serverParams->plot_current) {
	
	// this is a current observation
	
	_plotXmlShape(serverParams, prod,
		      serverParams->current_color,
		      instance);
	
	// plot arrow if prev lat/lon is available

	if (prevLat != 0.0 || prevLon != 0.0) {
	  _plotXmlVector(serverParams, prod,
			 prevLat, prevLon, instance.lat, instance.lon,
			 serverParams->past_color);
	}
	
	// plot nowcast parameters as text

	_plotXmlText(serverParams, prod, instance);
	
	prevLat = instance.lat;
	prevLon = instance.lon;
	
      } else if (instance.time > obsTime && serverParams->plot_forecast) {
	
	// this is a forecast
	
	_plotXmlShape(serverParams, prod,
		      serverParams->forecast_color,
		      instance);

	// plot arrow if prev lat/lon is available

	if (prevLat != 0.0 || prevLon != 0.0) {
	  _plotXmlVector(serverParams, prod,
			 prevLat, prevLon, instance.lat, instance.lon,
			 serverParams->forecast_color);
	}
	
	prevLat = instance.lat;
	prevLon = instance.lon;
	
      }

    } // iinst

  } // ievent

  // plot vectors from parents to children, where relevant

  for (int iinst = 0; iinst < (int) _instances.size(); iinst++) {

    const StormInstance &parent = _instances[iinst];

    // loop through children
    
    for (int ichild = 0; ichild< (int) parent.childIds.size(); ichild++) {
      
      int childId = parent.childIds[ichild];
      
      // look for child ID, and if it has parents, draw a vector to it
      
      for (int jinst = 0; jinst < (int) _instances.size(); jinst++) {
	
	const StormInstance &child = _instances[jinst];
	if (childId == child.stormId &&
	    child.parentIds.size() > 0) {

	  _plotXmlVector(serverParams, prod,
			 parent.lat, parent.lon,
			 child.lat, child.lon,
			 serverParams->past_color);
	  
	}
	
      } // jinst

    } // ichild

  } // iinst

  // set return buffer

  if (_isVerbose) {
    prod.print(cerr);
  }
  prod.serialize(symprod_buf);

  return iret;

}

//////////////////////////////////////////////////////////////////////
// Plot shape from XML data

void Server::_plotXmlShape(Params *serverParams,
			   Symprod &prod,
			   const char *color,
			   const StormInstance &instance)
  
{
  
  if (serverParams->storm_shape == Params::ELLIPSE_SHAPE) {
    
    // plot ellipse if available
    
    if (instance.ellipseSet) {

      prod.addArc(instance.lat, instance.lon,
		  instance.ellipseMajorAxisKm,
		  instance.ellipseMinorAxisKm,
		  color,
		  false, 0.0, 360.0,
		  90.0 - instance.ellipseOrientationDegT,
		  90,
		  _convertLineTypeParam(serverParams->suggested_line_type),
		  serverParams->suggested_line_width);

    }

  } else if (serverParams->storm_shape == Params::POLYGON_SHAPE) {

    // plot polygon if available
    
    if (instance.polygonSet) {

      int npts = (int) instance.polygonPts.size();
      TaArray<Symprod::wpt_t> _pts;
      Symprod::wpt_t *pts = _pts.alloc(npts);
      for (int ii = 0; ii < npts; ii++) {
	pts[ii] = instance.polygonPts[ii];
      }
      
      prod.addPolyline
	(npts, pts, color,
	 _convertLineTypeParam(serverParams->suggested_line_type),
	 serverParams->suggested_line_width,
	 _convertCapstyleParam(serverParams->suggested_capstyle),
	 _convertJoinstyleParam(serverParams->suggested_joinstyle));

    }

  }

}

//////////////////////////////////////////////////////////////////////
// _plotForecastVector() - Add an arrow to show forecast movement vector

void Server::_plotXmlVector(Params *serverParams,
			    Symprod &prod,
			    double startLat, double startLon,
			    double endLat, double endLon,
			    const char *color)

{

  if (serverParams->arrow_head_len_in_pixels) {
    prod.addArrowBothPts
      (color,
       _convertLineTypeParam(serverParams->suggested_line_type),
       serverParams->suggested_line_width,
       _convertCapstyleParam(serverParams->suggested_capstyle),
       _convertJoinstyleParam(serverParams->suggested_joinstyle),
       startLat, startLon,
       endLat, endLon,
       (int) (serverParams->arrow_head_len + 0.5),
       serverParams->arrow_head_half_angle);
  } else {
    prod.addArrowBothPts
      (color,
       _convertLineTypeParam(serverParams->suggested_line_type),
       serverParams->suggested_line_width,
       _convertCapstyleParam(serverParams->suggested_capstyle),
       _convertJoinstyleParam(serverParams->suggested_joinstyle),
       startLat, startLon,
       endLat, endLon,
       serverParams->arrow_head_len,
       serverParams->arrow_head_half_angle);
  }

}

//////////////////////////////////////////////////////////////////////
// Plot XML text

void Server::_plotXmlText(Params *serverParams,
			  Symprod &prod,
			  const StormInstance &instance)
  
{
  
  // load the text line vector

  vector<string> textLines;
  char text[1024];

  // speed

  if (serverParams->plot_speed) {
    double speed_kmh = instance.speedKmh;
    string speedStr = _formatSpeed(serverParams, speed_kmh);
    textLines.push_back(speedStr);
  }
  
  // speedStr
  
  if (serverParams->plot_top) {
    if (serverParams->top_km) {
      sprintf(text, "Tops %dkm\n", (int)(instance.topKm + 0.5));
    } else {
      double top_100s_ft =
	instance.topKm / KM_PER_MI * FT_PER_MI / 100.0;
      sprintf(text, "Tops FL%d\n", (int)(top_100s_ft + 0.5));
    }
    textLines.push_back(text);
  }

  // Track numbers.

  if (serverParams->plot_track_numbers) {
    char text[32];
    sprintf(text, "Id %d\n", instance.stormId);
    textLines.push_back(text);
  }
    
  // volume

  if (serverParams->plot_volume && instance.volumeKm3 > 0) {
    sprintf(text, "Vol %.0fkm3\n", instance.volumeKm3);
    textLines.push_back(text);
  }
    
  // area

  if (serverParams->plot_area && instance.areaKm2 > 0) {
    sprintf(text, "Area %.0fkm2\n", instance.areaKm2);
    textLines.push_back(text);
  }
    
  // max_dbz

  if (serverParams->plot_max_dbz && instance.maxDbz > 0) {
    sprintf(text, "MaxDbz %.1f\n", instance.maxDbz);
    textLines.push_back(text);
  }
    
  // vil

  if (serverParams->plot_vil && instance.vil > 0) {
    sprintf(text, "Vil %.1f\n", instance.vil);
    textLines.push_back(text);
  }
    
  // storm intensity

  if (serverParams->plot_storm_intensity && instance.stormIntensity > 0) {
    sprintf(text, "Inten %d\n", instance.stormIntensity);
    textLines.push_back(text);
  }
    
  // hail probability

  if (serverParams->plot_hail_probability && instance.hailProb > 0) {
    sprintf(text, "Hail prob %.0f\n", instance.hailProb);
    textLines.push_back(text);
  }
    
  // hail mass

  if (serverParams->plot_hail_mass && instance.hailMass > 0) {
    sprintf(text, "Hail mass %.0f\n", instance.hailMass);
    textLines.push_back(text);
  }
    
  // hail mass_aloft

  if (serverParams->plot_hail_mass_aloft && instance.hailMassAloft > 0) {
    sprintf(text, "Hail aloft %.0f\n", instance.hailMassAloft);
    textLines.push_back(text);
  }
    
  // add the text lines
  
  if (serverParams->plot_hidden_text) {

    char *foreground_color = serverParams->hidden_text_foreground_color;
    if (strlen(foreground_color) == 0) {
      foreground_color = serverParams->text_color;
    }
    string combinedText;
    for (int ii = 0; ii < (int) textLines.size(); ii++) {
      combinedText += textLines[ii];
    }

    prod.addText(combinedText.c_str(),
		 instance.lat, instance.lon,
                 foreground_color,
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
		 0,  // Object ID
		 Symprod::DETAIL_LEVEL_USUALLY_HIDDEN | 
                 Symprod::DETAIL_LEVEL_SHOWAS_POPOVER |
                 Symprod::DETAIL_LEVEL_IGNORE_TRESHOLDS);

  } else if (serverParams->plot_text) {

    int currentOffset = 0;
    int  xOffset = serverParams->plot_text_horizontal_offset;  
    for (int ii = 0; ii < (int) textLines.size(); ii++) {
      prod.addText(textLines[ii].c_str(),
		   instance.lat, instance.lon,
		   serverParams->text_color,
		   serverParams->text_background_color,
		   xOffset, currentOffset,
		   Symprod::VERT_ALIGN_CENTER,
		   Symprod::HORIZ_ALIGN_LEFT,
		   serverParams->text_font_size,
		   Symprod::TEXT_NORM,
		   serverParams->text_font);
      currentOffset -= serverParams->plot_text_line_offset;
    }

  }
  
}
    
//////////////////////////////////////////////////////////////////////
// get instance properties from the XML
// Returns 0 on success, -1 on failure

int Server::_getInstanceProps(int stormId,
                              const string &instanceXml,
                              StormInstance &instance)
  
{

  int iret = 0;
  instance.stormId = stormId;

  // get description for this instance
  
  vector<TaXml::attribute> attrs;
  TaXml::readStringAttr(attrs, "relative-time", instance.relative_time);
      
  // get time for this instance
  
  if (TaXml::readTime(instanceXml, "time", instance.time)) {
    cerr << "ERROR - Server::_setInstanceProps" << endl;
    cerr << "  Cannot read instance <time>" << endl;
    cerr << "========= instance buffer =================" << endl;
    cerr << instanceXml << endl;
    cerr << "=======================================" << endl;
    iret = -1;
  }
  
  // get centroid and motion
  // this is presented twice, once in ellipse and once in polygon
  
  if (_getCentroidMovement(instanceXml,
			   instance.lat, instance.lon,
			   instance.speedKmh, instance.dirnDegT)) {
    cerr << "ERROR - Server::_setInstanceProps" << endl;
    cerr << "Cannot get centroid and movement" << endl;
    cerr << "===============================" << endl;
    cerr << instanceXml << endl;
    cerr << "===============================" << endl;
    iret = -1;
  }
  
  // get parents and children
  
  _getParentsAndChildren(instanceXml, instance.parentIds, instance.childIds);

  // get ellipse and polygon props
  
  _getEllipseProps(instanceXml, instance);
  _getPolygonProps(instanceXml, instance);

  // get the nowcast props
  
  _getNowcastProps(instanceXml, instance);

  return iret;

}

//////////////////////////////////////////////////////////////////////
// get case properties from the XML
// Returns 0 on success, -1 on failure

int Server::_getCaseProps(int stormId,
                          const string &instanceXml,
                          StormInstance &instance)
  
{

  int iret = 0;
  instance.stormId = stormId;

  // get description for this instance
  
  vector<TaXml::attribute> attrs;
  TaXml::readStringAttr(attrs, "description", instance.relative_time);
      
  // get time for this instance
  
  if (TaXml::readTime(instanceXml, "time", instance.time)) {
    cerr << "ERROR - Server::_setInstanceProps" << endl;
    cerr << "  Cannot read instance <time>" << endl;
    cerr << "========= instance buffer =================" << endl;
    cerr << instanceXml << endl;
    cerr << "=======================================" << endl;
    iret = -1;
  }
  
  // get centroid and motion
  // this is presented twice, once in ellipse and once in polygon
  
  if (_getCentroidMovement(instanceXml,
			   instance.lat, instance.lon,
			   instance.speedKmh, instance.dirnDegT)) {
    cerr << "ERROR - Server::_setInstanceProps" << endl;
    cerr << "Cannot get centroid and movement" << endl;
    cerr << "===============================" << endl;
    cerr << instanceXml << endl;
    cerr << "===============================" << endl;
    iret = -1;
  }
  
  // get parents and children
  
  _getParentsAndChildren(instanceXml, instance.parentIds, instance.childIds);

  // get ellipse and polygon props
  
  _getEllipseProps(instanceXml, instance);
  _getPolygonProps(instanceXml, instance);

  // get the nowcast props
  
  _getNowcastProps(instanceXml, instance);

  return iret;

}

///////////////////////////////////////
// Get centroid, speed and direction
// Returns 0 on success, -1 on failure

int Server::_getCentroidMovement(string xml,
				 double &lat, double &lon,
				 double &speed, double &dirn)
  
{
  
  string movPtXml;
  if (TaXml::readString(xml, "moving-point", movPtXml)) {
    return -1;
  }

  if (TaXml::readDouble(movPtXml, "latitude", lat)) {
    return -1;
  }

  if (TaXml::readDouble(movPtXml, "longitude", lon)) {
    return -1;
  }

  string motionXml;
  if (TaXml::readString(movPtXml, "polar_motion", motionXml)) {
    return -1;
  }

  if (TaXml::readDouble(motionXml, "speed", speed)) {
    return -1;
  }

  if (TaXml::readDouble(motionXml, "direction_to", dirn)) {
    return -1;
  }

  return 0;

}

/////////////////////////////////////////
// get parents and children for this instance

void Server::_getParentsAndChildren(const string &instanceXml,
				    vector<int> &parentIds,
				    vector<int> &childIds)
  
{
  
  vector<TaXml::attribute> attrs;
  string parentStr;
  if (TaXml::readString(instanceXml, "ID_parent", parentStr, attrs) == 0) {
    // tokenize parent str
    vector<string> toks;
    TaStr::tokenize(parentStr, ",", toks);
    for (int ii = 0; ii < (int) toks.size(); ii++) {
      int id;
      if (sscanf(toks[ii].c_str(), "%d", &id) == 1) {
	parentIds.push_back(id);
      }
    }
  }

  string childStr;
  if (TaXml::readString(instanceXml, "ID_child", childStr, attrs) == 0) {
    // tokenize child str
    vector<string> toks;
    TaStr::tokenize(childStr, ",", toks);
    for (int ii = 0; ii < (int) toks.size(); ii++) {
      int id;
      if (sscanf(toks[ii].c_str(), "%d", &id) == 1) {
	childIds.push_back(id);
      }
    }
  }

}

//////////////////////////////////////////////////////////////////////
// get ellipse properties
// Returns 0 on success, -1 on failure

int Server::_getEllipseProps(const string &instanceXml,
			     StormInstance &instance)

{

  int iret = 0;
  instance.ellipseSet = false;

  string ellipseXml;
  if (TaXml::readString(instanceXml, "ellipse", ellipseXml)) {
    // no ellipse included
    return -1;
  }
      
  if (TaXml::readDouble(ellipseXml, "major_axis",
			instance.ellipseMajorAxisKm)) {
    cerr << "Cannot decode ellipse <major_axis>" << endl;
    iret = -1;
  }
  
  if (TaXml::readDouble(ellipseXml, "minor_axis",
			instance.ellipseMinorAxisKm)) {
    cerr << "Cannot decode ellipse <minor_axis>" << endl;
    iret = -1;
  }
  
  if (TaXml::readDouble(ellipseXml, "orientation",
			instance.ellipseOrientationDegT)) {
    cerr << "Cannot decode ellipse <orientation>" << endl;
    iret = -1;
  }
  
  if (iret == 0) {
    instance.ellipseSet = true;
  }
  
  return iret;

}

//////////////////////////////////////////////////////////////////////
// get polygon properties
// Returns 0 on success, -1 on failure

int Server::_getPolygonProps(const string &instanceXml,
			     StormInstance &instance)

{

  instance.polygonSet = false;
  
  string polygonXml;
  if (TaXml::readString(instanceXml, "polygon", polygonXml)) {
    // no polygon included
    return -1;
  }
      
  int iret = 0;

  // polygon points
  
  vector<string> points;
  if (TaXml::readTagBufArray(polygonXml, "point", points)) {
    cerr << "ERROR - Server::_plotPolygon" << endl;
    cerr << "Cannot get polygon points" << endl;
    cerr << "===============================" << endl;
    cerr << polygonXml << endl;
    cerr << "===============================" << endl;
    return -1;
  }

  instance.polygonPts.clear();
  for (int ii = 0; ii < (int) points.size(); ii++) {

    const string &pointTagged = points[ii];
    string point;
    vector<TaXml::attribute> attrs;
    if (TaXml::readString(pointTagged, "point", point, attrs)) {
      iret = -1;
      continue;
    }
    
    double ptLat, ptLon;
    if (TaXml::readDoubleAttr(attrs, "latitude", ptLat)) {
      iret = -1;
    }
    if (TaXml::readDoubleAttr(attrs, "longitude", ptLon)) {
      iret = -1;
    }

    Symprod::wpt_t pt;
    pt.lat = ptLat;
    pt.lon = ptLon;
    instance.polygonPts.push_back(pt);

  }
  
  if (iret == 0) {
    instance.polygonSet = true;
  }
  
  return iret;

}

//////////////////////////////////////////////////////////////////////
// get nowcast properties
// Returns 0 on success, -1 on failure

int Server::_getNowcastProps(const string &instanceXml,
			     StormInstance &instance)

{
  
  
  string nowcastXml;
  if (TaXml::readString(instanceXml, "nowcast-parameters", nowcastXml)) {
    // no nowcast included
    instance.nowcastSet = false;
    return -1;
  }
  instance.nowcastSet = true;

  TaXml::readInt(nowcastXml, "age", instance.ageSecs);
  TaXml::readDouble(nowcastXml, "cell_top", instance.topKm);
  TaXml::readDouble(nowcastXml, "cell_volume", instance.volumeKm3);
  TaXml::readDouble(nowcastXml, "projected_area", instance.areaKm2);
  TaXml::readDouble(nowcastXml, "max_dbz", instance.maxDbz);
  TaXml::readDouble(nowcastXml, "height_max_dbz", instance.htMaxDbzKm);
  TaXml::readDouble(nowcastXml, "VIL", instance.vil);
  TaXml::readInt(nowcastXml, "storm_intensity", instance.stormIntensity);
  TaXml::readDouble(nowcastXml, "hail_mass", instance.hailMass);
  TaXml::readDouble(nowcastXml, "hail_mass_aloft", instance.hailMassAloft);

  string hailProbStr;
  if (TaXml::readString(nowcastXml, "nowcast-parameters", hailProbStr) == 0) {
    TaXml::readDouble(hailProbStr, "value", instance.hailProb);
  }

  return 0;

}

//////////////////////////////////////////////////////////////////////
// format the speed string, in the correct units

string Server::_formatSpeed(Params *serverParams,
			    double speed_kmh)

{
  
  double speed;
  int rounded_speed;
  char speedUnits[8];
  
  if (serverParams->speed_units == Params::SPEED_KNOTS) {
    speed = speed_kmh / 1.852;
    sprintf(speedUnits,"%s", "Kt");
  } else if (serverParams->speed_units == Params::SPEED_MPH) {
    speed = speed_kmh / 1.6;
    sprintf(speedUnits,"%s", "mph");
  } else {
    speed = speed_kmh;
    sprintf(speedUnits,"%s", "kmh"); // the default.
  }
  
  if (serverParams->speed_round) {
    rounded_speed = (int) ((speed + 2.5) / 5.0) * 5;
  } else {
    rounded_speed = (int) (speed + 0.5);
  }
  
  char speedText[32];
  sprintf(speedText, "%d %s\n", rounded_speed, speedUnits);
  
  return speedText;

}
    
