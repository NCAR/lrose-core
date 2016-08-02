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
  cerr << "Entered Server constructor\n";
  cerr << "prog_name: " << prog_name << endl;
  cerr << "port: " <<  initialParams->port << endl;
  cerr << "debug: " <<  initialParams->debug << endl;
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

  //  Params *myParams = (Params*) serverParams;

  //  cerr << "Entering Server::transformData()\n";

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
  //  cerr << "Entering convertToSymprod\n";
  //  const char *routine_name = "convertToSymprod";
  Params *serverParams = (Params*) params;

  // check prod_id
  int iret = -1;

  if (prod_id == SPDB_XML_ID) {
    
    // create an ascii buffer, make sure it is null-delimited

    MemBuf xmlBuf;
    xmlBuf.add(spdb_data, spdb_len);
    char cnull = '\0';
    xmlBuf.add(&cnull, 1);
    char *xml = (char *) xmlBuf.getPtr();
    
    if (strstr(xml, "<icing-airmet") != NULL) {
      // TSTORMS XML
      if (!_xmlToSymprod(serverParams, dir_path,
                               prod_id, prod_label,
                               chunk_ref, aux_ref,
                               xml, symprod_buf)) {
        iret = 0;
      }
    } 
  }
    return iret;
}

//////////////////////////////////////////////////////////////////////
// Convert XML icing airmet data to symprod format.
// Returns 0 on success, -1 on failure

int Server::_xmlToSymprod(Params *serverParams,
                                 const string &dir_path,
                                 const int prod_id,
                                 const string &prod_label,
                                 const Spdb::chunk_ref_t &chunk_ref,
                                 const Spdb::aux_ref_t &aux_ref,
                                 const char *xml_str,
                                 MemBuf &symprod_buf)
  
{

  // clear cases vector
  
  //  _instances.clear();

  // create Symprod object
  
  time_t now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "Icing Airmet from XML");

  // remove comments

  string bufWithComments = (char *) xml_str;
  string bufNoComments = TaXml::removeComments(bufWithComments);
  
  // find xml between icing-airmet tags
  
  string iaBuf;
  if (TaXml::readString(bufNoComments, "icing-airmet", iaBuf)) {
    cerr << "ERROR - Server::_xmlToSymprod" << endl;
    cerr << "  XML buffer has no <icing-airmet> tag" << endl;
    cerr << "========= decoding buffer ================" << endl;
    cerr << bufNoComments << endl;
    cerr << "==========================================" << endl;
    return -1;
  }

  vector<TaXml::attribute> attrs;
  vector<string> cells;
  if (TaXml::readTagBufArray(iaBuf, "cell", cells)) {
    // no cell data
    return 0;
  }

  // loop through cells

  int iret = 0;
  for (int icell = 0; icell < (int) cells.size(); icell++) {

    // get cell attributes

    string cell;
    attrs.clear();
     
    if (TaXml::readString(cells[icell], "cell", cell, attrs)) {
      cerr << "ERROR - Server::_xmlToSymprod" << endl;
      cerr << "  Cannot read <cell>" << endl;
      cerr << "========= cell buffer =================" << endl;
      cerr << cells[icell] << endl;
      cerr << "========================================" << endl;
      iret = -1;
      continue;
    }

 
 // get ceiling & floor & centroid
  int ceiling = -9999;
  if (TaXml::readIntAttr(attrs, "ceiling", ceiling)) {
    cerr << "WARNING - Server::_xmlToSymprod" << endl;
    cerr << "  Cannot read ceiling" << endl;
    iret = -1;
  }

  int floor = -9999;
  if (TaXml::readIntAttr(attrs, "floor", floor)) {
    cerr << "WARNING - Server::_xmlToSymprod" << endl;
    cerr << "  Cannot read floor" << endl;
    iret = -1;
  }
  double center_lat = -9999;
  if (TaXml::readDoubleAttr(attrs, "cent-lat", center_lat)) {
    cerr << "WARNING - Server::_xmlToSymprod" << endl;
    cerr << "  Cannot read cent-lat" << endl;
    iret = -1;
  }

  double center_lon = -9999;
  if (TaXml::readDoubleAttr(attrs, "cent-lon", center_lon)) {
    cerr << "WARNING - Server::_xmlToSymprod" << endl;
    cerr << "  Cannot read cent-lon" << endl;
    iret = -1;
  }




  // get data time  
  time_t dataTime;
  if (TaXml::readTime(cells[icell], "data-time", dataTime)) {
    cerr << "ERROR - Server::_xmlToSymprod" << endl;
    cerr << "  Cannot read <data-time>" << endl;
    cerr << "========= cell buffer =================" << endl;
    cerr << cells[icell] << endl;
    cerr << "==========================================" << endl;
    return -1;
  }

  // get array of points

  vector<string> points;
  if (TaXml::readTagBufArray(cells[icell], "point", points)) {
    // no storms
    return 0;
  }

  // loop through points (simple track numbers)

  //  cerr << "Got point array: " << cells[icell] << endl << endl;
  
  vector<double> lats;
  vector<double> lons;
  for (int ipoint = 0; ipoint < (int) points.size(); ipoint++) {

    // get point attributes
    attrs.clear();
    string point;
    //    cerr << "Reading point: " << points[ipoint] << endl;
    if (TaXml::readString(points[ipoint], "point", point, attrs)) {
      cerr << "ERROR - Server::_xmlToSymprod" << endl;
      cerr << "  Cannot read <point>" << endl;
      cerr << "========= point buffer =================" << endl;
      cerr << points[ipoint] << endl;
      cerr << "========================================" << endl;
      iret = -1;
      continue;
    }

    // get lat/lon
    double lat = -9999;
    if (TaXml::readDoubleAttr(attrs, "lat", lat)) {
      cerr << "WARNING - Server::_xmlToSymprod" << endl;
      cerr << "  Cannot read lat" << endl;
      iret = -1;
    }
    lats.push_back(lat);

    double lon = -9999;
    if (TaXml::readDoubleAttr(attrs, "lon", lon)) {
      cerr << "WARNING - Server::_xmlToSymprod" << endl;
      cerr << "  Cannot read lon" << endl;
      iret = -1;
    }
    lons.push_back(lon);
    //    cerr << "Found lat,lon: " << points[ipoint] << endl;
    //    cerr << "Adding lat,lon: " << lat << "," << lon << endl;

  }
  // plot airmet shape
  _addCell(serverParams, prod, floor, ceiling, 
	   center_lat, center_lon, lats, lons);
	      
  }

  if (_isVerbose) {
    prod.print(cerr);
  }
  prod.serialize(symprod_buf);

  return iret;
}

//////////////////////////////////////////////////////////////////////
// Create a symprod from the cell, and add it to prod
void Server::_addCell(Params *serverParams, Symprod &prod,
		     int floor, int ceiling, float center_lat,
		     float center_lon, vector<double> lats,
		     vector<double> lons)
{

  _addPolygon(serverParams, prod, center_lat, center_lon, 
	      lats, lons, serverParams->airmet_color);

  _addText(serverParams, prod, floor, ceiling,
	   center_lat, center_lon);

}
 //////////////////////////////////////////////////////////////////////
// Add text for the floor/ceiling of  the airmet
void Server::_addText(Params *serverParams, Symprod &prod, int floor, int ceiling,
			   float center_lat,float center_lon)
{
  // load the text line vector

  vector<string> textLines;
  char text[1024];

 
  sprintf(text, "Floor: %d Ft\n", floor);
  textLines.push_back(text);

  sprintf(text, "Ceiling: %d Ft\n", ceiling);
  textLines.push_back(text);

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
		 center_lat, center_lon,
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

  } 
  
if (serverParams->plot_text) {

   int currentOffset = 0;
    int  xOffset = serverParams->plot_text_horizontal_offset;  
    for (int ii = 0; ii < (int) textLines.size(); ii++) {
      prod.addText(textLines[ii].c_str(),
		   center_lat, center_lon,
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
// Add a polygon representing the airmet
 void Server::_addPolygon(Params *serverParams, Symprod &prod,
			   float center_lat,
			   float center_lon, vector<double> lats,
			   vector<double> lons, char *color)
{
  //  cerr << "Entered _addPolygon()\n" ;
      int npts = (int) lats.size();
      TaArray<Symprod::wpt_t> _pts;
      Symprod::wpt_t *pts = _pts.alloc(npts+1);
      for (int ii = 0; ii < npts; ii++) 
	{
	  pts[ii].lat = lats[ii];
	  pts[ii].lon = lons[ii];
	  //	  cerr << "lat,lon: " << lats[ii] << "," << lons[ii] << endl;
	}
      pts[npts].lat = lats[0];
      pts[npts].lon = lons[0];

      prod.addPolyline
	(npts+1, pts, color,
	 _convertLineTypeParam(serverParams->suggested_line_type),
	 serverParams->suggested_line_width,
	 _convertCapstyleParam(serverParams->suggested_capstyle),
	 _convertJoinstyleParam(serverParams->suggested_joinstyle));

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

