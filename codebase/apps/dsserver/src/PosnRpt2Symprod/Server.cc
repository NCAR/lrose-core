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
#include <rapformats/fos.h>
#include <toolsa/pjg.h>
#include <toolsa/str.h>
#include "Server.hh"
using namespace std;

#define POSN_RPT_ICON_SIZE 16

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

  if (prod_id != SPDB_POSN_RPT_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_POSN_RPT_ID: " << SPDB_POSN_RPT_ID << endl;
    return -1;
  }

  Params *serverParams = (Params*) params;

  // Copy the SPDB data to the local buffer
  
  MemBuf inBuf;
  inBuf.add(spdb_data, spdb_len);

  // Convert the SPDB data to a position report

  PosnRpt posn_rpt(inBuf.getPtr(),
		   serverParams->debug >= Params::DEBUG_VERBOSE);
  
  // See if this report should be displayed

  if (serverParams->flight_list_n > 0) {
    const char *flight_num = posn_rpt.getFlightNum().c_str();
    bool found = false;
    for (int i = 0; i < serverParams->flight_list_n; i++) {
      if (STRequal_exact(flight_num, serverParams->_flight_list[i])) {
	found = true;
      }
    }
    if (!found) {
      return 0;
    }
  }
  
  // create Symprod object
  
  time_t now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "Posn report");
  
  // create icons
  
  int iconSize = POSN_RPT_ICON_SIZE * POSN_RPT_ICON_SIZE;
  MemBuf currentPosIconBuf;
  MemBuf wayPt0Buf;
  MemBuf wayPt1Buf;
  MemBuf wayPt2Buf;
  
  _createIcons(serverParams, iconSize,
	       currentPosIconBuf, wayPt0Buf, wayPt1Buf, wayPt2Buf);
  
  // add graphics

  prod.setLabel(posn_rpt.getFlightNum().c_str());
  
  _addCurrentPos(prod, serverParams, posn_rpt, currentPosIconBuf);
  
  if (serverParams->render_way_pt0_icon) {
    _addWayPt(prod,
	      posn_rpt.getWayPoint0(),
	      serverParams->way_pt0_color,
	      (ui08 *) wayPt0Buf.getPtr());
  }
  
  if (serverParams->render_way_pt1_icon) {
    _addWayPt(prod,
	      posn_rpt.getWayPoint1(),
	      serverParams->way_pt1_color,
	      (ui08 *) wayPt1Buf.getPtr());
  }
  
  if (serverParams->render_way_pt2_icon) {
    _addWayPt(prod,
	      posn_rpt.getWayPoint2(),
	      serverParams->way_pt2_color,
	      (ui08 *) wayPt2Buf.getPtr());
  }
  
  if (serverParams->render_way_pt_line) {
    _addWayPtLine(prod, serverParams, posn_rpt);
  }
  
  // set return buffer
  
  if (_isVerbose) {
    prod.print(cerr);
  }
  prod.serialize(symprod_buf);

  return(0);
  
}

////////////////////////////////////////////////////////////////////
// create icons

void Server::_createIcons(Params *serverParams,
			  int iconSize,
			  MemBuf &currentPosIconBuf,
			  MemBuf &wayPt0Buf,
			  MemBuf &wayPt1Buf,
			  MemBuf &wayPt2Buf)
  
{

  int copySize;
  ui08 zero = 0;

  // current pos

  if (serverParams->current_pos_icon_n != iconSize) {
    cerr << "WARNING - " << _executableName << ":Server::_createIcons" << endl;
    cerr << "  Incorrect icon size - current_pos_icon." << endl;
    cerr << "  Paramdef icon has " << serverParams->current_pos_icon_n
	 << " points." << endl;
    cerr << "  Should have " << iconSize << " points." << endl;
  }

  currentPosIconBuf.free();
  copySize = MIN(serverParams->current_pos_icon_n, iconSize);
  for (int i = 0; i < copySize; i++) {
    ui08 val = serverParams->_current_pos_icon[i];
    currentPosIconBuf.add(&val, sizeof(val));
  }
  for (int i = copySize; i < iconSize; i++) {
    currentPosIconBuf.add(&zero, sizeof(zero));
  }

  // way pt 0
  
  if (serverParams->way_pt0_icon_n != iconSize) {
    cerr << "WARNING - " << _executableName << ":Server::_createIcons" << endl;
    cerr << "  Incorrect icon size - way_pt0_icon." << endl;
    cerr << "  Paramdef icon has " << serverParams->way_pt0_icon_n
	 << " points." << endl;
    cerr << "  Should have " << iconSize << " points." << endl;
  }

  wayPt0Buf.free();
  copySize = MIN(serverParams->way_pt0_icon_n, iconSize);
  for (int i = 0; i < copySize; i++) {
    ui08 val = serverParams->_way_pt0_icon[i];
    wayPt0Buf.add(&val, sizeof(val));
  }
  for (int i = copySize; i < iconSize; i++) {
    wayPt0Buf.add(&zero, sizeof(zero));
  }

  // way pt 1
  
  if (serverParams->way_pt1_icon_n != iconSize) {
    cerr << "WARNING - " << _executableName << ":Server::_createIcons" << endl;
    cerr << "  Incorrect icon size - way_pt1_icon." << endl;
    cerr << "  Paramdef icon has " << serverParams->way_pt1_icon_n
	 << " points." << endl;
    cerr << "  Should have " << iconSize << " points." << endl;
  }

  wayPt1Buf.free();
  copySize = MIN(serverParams->way_pt1_icon_n, iconSize);
  for (int i = 1; i < copySize; i++) {
    ui08 val = serverParams->_way_pt1_icon[i];
    wayPt1Buf.add(&val, sizeof(val));
  }
  for (int i = copySize; i < iconSize; i++) {
    wayPt1Buf.add(&zero, sizeof(zero));
  }

  // way pt 2
  
  if (serverParams->way_pt2_icon_n != iconSize) {
    cerr << "WARNING - " << _executableName << ":Server::_createIcons" << endl;
    cerr << "  Incorrect icon size - way_pt2_icon." << endl;
    cerr << "  Paramdef icon has " << serverParams->way_pt2_icon_n
	 << " points." << endl;
    cerr << "  Should have " << iconSize << " points." << endl;
  }

  wayPt2Buf.free();
  copySize = MIN(serverParams->way_pt2_icon_n, iconSize);
  for (int i = 2; i < copySize; i++) {
    ui08 val = serverParams->_way_pt2_icon[i];
    wayPt2Buf.add(&val, sizeof(val));
  }
  for (int i = copySize; i < iconSize; i++) {
    wayPt2Buf.add(&zero, sizeof(zero));
  }

}

////////////////////////////////////////////////////////////////////
// Add the current position

void Server::_addCurrentPos(Symprod &prod,
			    const Params *serverParams,
			    const PosnRpt& posn_rpt,
			    MemBuf &currentPosIconBuf)
  

{

  Symprod::wpt_t icon_origin;
  
  icon_origin.lat = posn_rpt.getCurrentLat();
  icon_origin.lon = posn_rpt.getCurrentLon();
  
  if (icon_origin.lat == WayPoint::BAD_POSITION ||
      icon_origin.lon == WayPoint::BAD_POSITION)
    return;
  
  // Add the icon to the symprod product

  prod.addBitmapIcons(serverParams->current_pos_color,
		      1, &icon_origin,
		      POSN_RPT_ICON_SIZE, POSN_RPT_ICON_SIZE,
		      (ui08 *) currentPosIconBuf.getPtr());
  
  // Add the flight number to the symprod product
  
  if (serverParams->render_flight_num) {
    prod.addText((char *)posn_rpt.getFlightNum().c_str(),
		 posn_rpt.getCurrentLat(), posn_rpt.getCurrentLon(),
		 serverParams->current_pos_color,
		 "",
		 serverParams->flight_num_text_offsets.x_offset,
		 serverParams->flight_num_text_offsets.y_offset,
		 Symprod::VERT_ALIGN_BOTTOM,
		 Symprod::HORIZ_ALIGN_LEFT,
		 10, Symprod::TEXT_NORM,
		 serverParams->text_font);
  }
  
  // Add the position report time to the symprod product

  if (serverParams->render_report_time) {
    DateTime report_time = posn_rpt.getCurrentTime();
    char report_time_string[12];
    
    sprintf(report_time_string, "%.2d%.2d%.2d",
	    report_time.getHour(),
	    report_time.getMin(),
	    report_time.getSec());
    
    prod.addText(report_time_string,
		 posn_rpt.getCurrentLat(), posn_rpt.getCurrentLon(),
		 serverParams->current_pos_color,
		 "",
		 serverParams->report_time_text_offsets.x_offset,
		 serverParams->report_time_text_offsets.y_offset,
		 Symprod::VERT_ALIGN_BOTTOM,
		 Symprod::HORIZ_ALIGN_LEFT,
		 10, Symprod::TEXT_NORM,
		 serverParams->text_font);
  }
  
}


////////////////////////////////////////////////////////////////////
//  Add the given way point

void Server::_addWayPt(Symprod &prod,
		       const WayPoint& way_point,
		       const char *color,
		       const ui08 *icon)
{

  Symprod::wpt_t icon_origin;
  
  icon_origin.lat = way_point.getLat();
  icon_origin.lon = way_point.getLon();
  
  if (icon_origin.lat == WayPoint::BAD_POSITION ||
      icon_origin.lon == WayPoint::BAD_POSITION)
    return;
  
  // Add the icon to the symprod product

  prod.addBitmapIcons((char *) color,
		      1, &icon_origin,
		      POSN_RPT_ICON_SIZE, POSN_RPT_ICON_SIZE,
		      (ui08 *) icon);

}


////////////////////////////////////////////////////////////////////
// Add the way point line

void Server::_addWayPtLine(Symprod &prod,
			   const Params *serverParams,
			   const PosnRpt& posn_rpt)

{

  Symprod::wpt_t polyline_pts[3];
  
  polyline_pts[0].lat = posn_rpt.getWayPoint0().getLat();
  polyline_pts[0].lon = posn_rpt.getWayPoint0().getLon();
  
  polyline_pts[1].lat = posn_rpt.getWayPoint1().getLat();
  polyline_pts[1].lon = posn_rpt.getWayPoint1().getLon();
  
  polyline_pts[2].lat = posn_rpt.getWayPoint2().getLat();
  polyline_pts[2].lon = posn_rpt.getWayPoint2().getLon();
  
  for (int i = 0; i < 3; i++)
    if (polyline_pts[i].lat == WayPoint::BAD_POSITION ||
	polyline_pts[i].lon == WayPoint::BAD_POSITION)
      return;
  
  // Add the polyline to the symprod product

  prod.addPolyline(3, polyline_pts,
		   serverParams->way_pt_line_color,
		   Symprod::LINETYPE_SOLID,
		   serverParams->way_pt_line_width);
}
  
