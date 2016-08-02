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
// Nov 2010
//
///////////////////////////////////////////////////////////////

#include <toolsa/toolsa_macros.h>
#include <toolsa/TaXml.hh>
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
  
  Params *serverParams = (Params*) params;

  // check prod_id
  
  if (prod_id != SPDB_XML_ID) {
    cerr << "ERROR - SunCal2Symprod::convertToSymprod" << endl;
    cerr << "Incorrect product ID: " << prod_id << endl;
    return -1;
  }
    
  // create an ascii buffer to store the XML
  // make sure it is null-delimited
  
  MemBuf xmlBuf;
  xmlBuf.add(spdb_data, spdb_len);
  char cnull = '\0';
  xmlBuf.add(&cnull, 1);
  char *xml = (char *) xmlBuf.getPtr();

  // create Symprod object

  time_t now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "SunCal");

  // get the centroid offset

  string radarName;
  string radarSite;
  double centroidAzOffset = -9999;
  double centroidElOffset = -9999;
  double meanSunEl = -9999;
  double meanSunAz = -9999;

  TaXml::readString(xml, "radarName", radarName);
  TaXml::readString(xml, "radarSite", radarSite);
  TaXml::readDouble(xml, "centroidAzOffset", centroidAzOffset);
  TaXml::readDouble(xml, "centroidElOffset", centroidElOffset);
  TaXml::readDouble(xml, "meanSunEl", meanSunEl);
  TaXml::readDouble(xml, "meanSunAz", meanSunAz);

  // plot centroid location
    
  if (serverParams->plot_centroid_location) {

    Symprod::wpt_t line[2];
    line[0].lat = -90;
    line[0].lon = centroidAzOffset;
    line[1].lat = 90;
    line[1].lon = centroidAzOffset;
    
    prod.addPolyline(2, line, serverParams->centroid_line_color,
                     Symprod::LINETYPE_SOLID,
                     serverParams->centroid_line_width);
    
    line[0].lat = centroidElOffset;
    line[0].lon = -10;
    line[1].lat = centroidElOffset;
    line[1].lon = 10;
    
    prod.addPolyline(2, line, serverParams->centroid_line_color,
                     Symprod::LINETYPE_SOLID,
                     serverParams->centroid_line_width);

  }

  // plot sun circles?
    
  if (serverParams->plot_sun_circles) {


    for (int ii = 0; ii < serverParams->sun_circle_diameter_deg_n; ii++) {

      // compute radius in km
      // this has to be done since we are using a latlon projection
      // to represent the sun scan data
      
      double radius =
        (serverParams->_sun_circle_diameter_deg[ii] / 2.0) * KM_PER_DEG_AT_EQ;

      prod.addArc(centroidElOffset, centroidAzOffset,
                  radius, radius,
                  serverParams->sun_circle_color,
                  FALSE, 0.0, 360.0, 0, 360,
                  Symprod::LINETYPE_SOLID,
                  serverParams->sun_circle_line_width);

    }

  }

  // plot centroid offset text
  
  if (serverParams->plot_centroid_text) {

    char text[128];
    int horizOffset = serverParams->centroid_text_horizontal_offset;
    int vertOffset = serverParams->centroid_text_vertical_offset;

    sprintf(text, "%s %s", radarName.c_str(), radarSite.c_str());
    prod.addText(text, 0.0, 0.0,
                 serverParams->centroid_text_color,
                 serverParams->centroid_text_background_color,
                 horizOffset, vertOffset,
                 Symprod::VERT_ALIGN_CENTER,
                 Symprod::HORIZ_ALIGN_LEFT,
                 serverParams->centroid_text_font_size,
                 Symprod::TEXT_NORM,
                 serverParams->centroid_text_font);

    vertOffset += serverParams->centroid_text_line_spacing;
    sprintf(text, "Azim offset: %.2f deg", centroidAzOffset);
    prod.addText(text, 0.0, 0.0,
                 serverParams->centroid_text_color,
                 serverParams->centroid_text_background_color,
                 horizOffset, vertOffset,
                 Symprod::VERT_ALIGN_CENTER,
                 Symprod::HORIZ_ALIGN_LEFT,
                 serverParams->centroid_text_font_size,
                 Symprod::TEXT_NORM,
                 serverParams->centroid_text_font);

    vertOffset += serverParams->centroid_text_line_spacing;
    sprintf(text, "Elev offset: %.2f deg", centroidElOffset);
    prod.addText(text, 0.0, 0.0,
                 serverParams->centroid_text_color,
                 serverParams->centroid_text_background_color,
                 horizOffset, vertOffset,
                 Symprod::VERT_ALIGN_CENTER,
                 Symprod::HORIZ_ALIGN_LEFT,
                 serverParams->centroid_text_font_size,
                 Symprod::TEXT_NORM,
                 serverParams->centroid_text_font);

    vertOffset += serverParams->centroid_text_line_spacing;
    sprintf(text, "Sun mean el: %.2f deg", meanSunEl);
    prod.addText(text, 0.0, 0.0,
                 serverParams->centroid_text_color,
                 serverParams->centroid_text_background_color,
                 horizOffset, vertOffset,
                 Symprod::VERT_ALIGN_CENTER,
                 Symprod::HORIZ_ALIGN_LEFT,
                 serverParams->centroid_text_font_size,
                 Symprod::TEXT_NORM,
                 serverParams->centroid_text_font);

    vertOffset += serverParams->centroid_text_line_spacing;
    sprintf(text, "Sun mean az: %.2f deg", meanSunAz);
    prod.addText(text, 0.0, 0.0,
                 serverParams->centroid_text_color,
                 serverParams->centroid_text_background_color,
                 horizOffset, vertOffset,
                 Symprod::VERT_ALIGN_CENTER,
                 Symprod::HORIZ_ALIGN_LEFT,
                 serverParams->centroid_text_font_size,
                 Symprod::TEXT_NORM,
                 serverParams->centroid_text_font);

    // thumbwheel guidance

    if (serverParams->plot_spol_thumbwheel_guidance) {

      vertOffset += serverParams->centroid_text_line_spacing;
      sprintf(text, "Adjust az thumbwheel by: %.2f deg", centroidAzOffset);
      prod.addText(text, 0.0, 0.0,
                   serverParams->centroid_text_color,
                   serverParams->centroid_text_background_color,
                   horizOffset, vertOffset,
                   Symprod::VERT_ALIGN_CENTER,
                   Symprod::HORIZ_ALIGN_LEFT,
                   serverParams->centroid_text_font_size,
                   Symprod::TEXT_NORM,
                   serverParams->centroid_text_font);

      vertOffset += serverParams->centroid_text_line_spacing;
      sprintf(text, "Adjust el thumbwheel by: %.2f deg", centroidElOffset * -1.0);
      prod.addText(text, 0.0, 0.0,
                   serverParams->centroid_text_color,
                   serverParams->centroid_text_background_color,
                   horizOffset, vertOffset,
                   Symprod::VERT_ALIGN_CENTER,
                   Symprod::HORIZ_ALIGN_LEFT,
                   serverParams->centroid_text_font_size,
                   Symprod::TEXT_NORM,
                   serverParams->centroid_text_font);

    }

  } // if (serverParams->plot_centroid_text) {

  if (serverParams->plot_zdr_calibration_text) {

    // get the ZDR cal info

    int nXpolPoints = 0;
    double meanXpolRatioDb = -9999;
    double S1S2 = -9999;
    double zdrCorr = -9999;
    double xpolRatioDbFromSpdb = -9999;
    double siteTempFromSpdb = -9999;

    TaXml::readInt(xml, "nXpolPoints", nXpolPoints);
    TaXml::readDouble(xml, "meanXpolRatioDb", meanXpolRatioDb);
    TaXml::readDouble(xml, "S1S2", S1S2);
    TaXml::readDouble(xml, "zdrCorr", zdrCorr);
    TaXml::readDouble(xml, "xpolRatioDbFromSpdb", xpolRatioDbFromSpdb);
    TaXml::readDouble(xml, "siteTempFromSpdb", siteTempFromSpdb);

    char text[128];
    int horizOffset = serverParams->zdr_cal_text_horizontal_offset;
    int vertOffset = serverParams->zdr_cal_text_vertical_offset;

    sprintf(text, "==== ZDR correction ===");
    prod.addText(text, 0.0, 0.0,
                 serverParams->zdr_cal_text_color,
                 serverParams->zdr_cal_text_background_color,
                 horizOffset, vertOffset,
                 Symprod::VERT_ALIGN_CENTER,
                 Symprod::HORIZ_ALIGN_LEFT,
                 serverParams->zdr_cal_text_font_size,
                 Symprod::TEXT_NORM,
                 serverParams->zdr_cal_text_font);
    
    vertOffset += serverParams->zdr_cal_text_line_spacing;
    sprintf(text, "nXpolPoints: %d", nXpolPoints);
    prod.addText(text, 0.0, 0.0,
                 serverParams->zdr_cal_text_color,
                 serverParams->zdr_cal_text_background_color,
                 horizOffset, vertOffset,
                 Symprod::VERT_ALIGN_CENTER,
                 Symprod::HORIZ_ALIGN_LEFT,
                 serverParams->zdr_cal_text_font_size,
                 Symprod::TEXT_NORM,
                 serverParams->zdr_cal_text_font);

    vertOffset += serverParams->zdr_cal_text_line_spacing;
    sprintf(text, "meanXpolRatioDb: %.3f", meanXpolRatioDb);
    prod.addText(text, 0.0, 0.0,
                 serverParams->zdr_cal_text_color,
                 serverParams->zdr_cal_text_background_color,
                 horizOffset, vertOffset,
                 Symprod::VERT_ALIGN_CENTER,
                 Symprod::HORIZ_ALIGN_LEFT,
                 serverParams->zdr_cal_text_font_size,
                 Symprod::TEXT_NORM,
                 serverParams->zdr_cal_text_font);

    vertOffset += serverParams->zdr_cal_text_line_spacing;
    sprintf(text, "S1S2: %.3f", S1S2);
    prod.addText(text, 0.0, 0.0,
                 serverParams->zdr_cal_text_color,
                 serverParams->zdr_cal_text_background_color,
                 horizOffset, vertOffset,
                 Symprod::VERT_ALIGN_CENTER,
                 Symprod::HORIZ_ALIGN_LEFT,
                 serverParams->zdr_cal_text_font_size,
                 Symprod::TEXT_NORM,
                 serverParams->zdr_cal_text_font);

    vertOffset += serverParams->zdr_cal_text_line_spacing;
    sprintf(text, "zdrCorr: %.3f", zdrCorr);
    prod.addText(text, 0.0, 0.0,
                 serverParams->zdr_cal_text_color,
                 serverParams->zdr_cal_text_background_color,
                 horizOffset, vertOffset,
                 Symprod::VERT_ALIGN_CENTER,
                 Symprod::HORIZ_ALIGN_LEFT,
                 serverParams->zdr_cal_text_font_size,
                 Symprod::TEXT_NORM,
                 serverParams->zdr_cal_text_font);
    
    if (xpolRatioDbFromSpdb > -9990) {
      vertOffset += serverParams->zdr_cal_text_line_spacing;
      sprintf(text, "xpolRatioDbFromSpdb: %.3f", xpolRatioDbFromSpdb);
      prod.addText(text, 0.0, 0.0,
                   serverParams->zdr_cal_text_color,
                   serverParams->zdr_cal_text_background_color,
                   horizOffset, vertOffset,
                   Symprod::VERT_ALIGN_CENTER,
                   Symprod::HORIZ_ALIGN_LEFT,
                   serverParams->zdr_cal_text_font_size,
                   Symprod::TEXT_NORM,
                   serverParams->zdr_cal_text_font);
    }

    if (siteTempFromSpdb > -9990) {
      vertOffset += serverParams->zdr_cal_text_line_spacing;
      sprintf(text, "siteTemp: %.3f", siteTempFromSpdb);
      prod.addText(text, 0.0, 0.0,
                   serverParams->zdr_cal_text_color,
                   serverParams->zdr_cal_text_background_color,
                   horizOffset, vertOffset,
                   Symprod::VERT_ALIGN_CENTER,
                   Symprod::HORIZ_ALIGN_LEFT,
                   serverParams->zdr_cal_text_font_size,
                   Symprod::TEXT_NORM,
                   serverParams->zdr_cal_text_font);
    }

  } // if (_params.plot_zdr_calibration_text)

  // set return buffer

  if (_isVerbose) {
    prod.print(cerr);
  }
  prod.serialize(symprod_buf);

  return(0);
  
}

