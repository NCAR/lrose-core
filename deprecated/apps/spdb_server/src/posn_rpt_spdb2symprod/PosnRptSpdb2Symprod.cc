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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 18:41:15 $
//   $Id: PosnRptSpdb2Symprod.cc,v 1.3 2016/03/07 18:41:15 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * WxhazardsSpdb2Symprod.cc: wxhazards_spdb2symprod program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <signal.h>

#include <os_config.h>
#include <euclid/WorldPoint2D.hh>
#include <euclid/WorldPolygon2D.hh>
#include <spdb/spdb_products.h>
#include <spdbFormats/PosnRpt.hh>
#include <spdbFormats/WayPoint.hh>
#include <symprod/symprod.h>
#include <toolsa/DateTime.hh>
#include <toolsa/membuf.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Params.hh"
#include "PosnRptSpdb2Symprod.hh"

// Global variables

PosnRptSpdb2Symprod *PosnRptSpdb2Symprod::_instance =
  (PosnRptSpdb2Symprod *)NULL;

// Global constants

const int FOREVER = true;


/*********************************************************************
 * Constructor
 */

PosnRptSpdb2Symprod::PosnRptSpdb2Symprod(int argc, char **argv)
{
  static char *routine_name = "Constructor";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (PosnRptSpdb2Symprod *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = (char *) "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr,
	    "Problem with TDRP parameters in file <%s>\n",
	    params_path);
    
    okay = false;
    
    return;
  }

  // Create the server object

  SpdbServerDebugLevel spdb_debug_level;
  
  switch (_params->debug)
  {
  case Params::DEBUG_OFF:
    spdb_debug_level = SPDB_SERVER_DEBUG_OFF;
    break;
    
  case Params::DEBUG_ERRORS:
    spdb_debug_level = SPDB_SERVER_DEBUG_ERRORS;
    break;
    
  case Params::DEBUG_MSGS:
    spdb_debug_level = SPDB_SERVER_DEBUG_MSGS;
    break;
    
  case Params::DEBUG_ROUTINES:
    spdb_debug_level = SPDB_SERVER_DEBUG_ROUTINES;
    break;
    
  case Params::DEBUG_ALL:
    spdb_debug_level = SPDB_SERVER_DEBUG_ALL;
    break;
    
  } /* endswitch - Params.debug */
  
  _spdbServer = new SpdbServer(_params->port,
			       _params->product_label,
			       _params->product_id,
			       _params->database_dir,
			       _progName,
			       _params->servmap_type,
			       _params->servmap_subtype,
			       _params->servmap_instance,
			       64,
			       NULL,
			       -1,
			       _convertToSymprod,
			       SPDB_SYMPROD_ID,
			       1000,
			       _params->realtime_avail,
			       spdb_debug_level);
  

  // Create the icons

  int icon_size = 16 * 16;
  
  if (_params->current_pos_icon_n != icon_size)
  {
    fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr, "Problem with TDRP parameters in file <%s>\n",
	    params_path);
    fprintf(stderr,
	    "Current position icon array contains %d values, should contain %d values\n",
	    _params->current_pos_icon_n, icon_size);
    
    okay = false;
    
    return;
  }
  
  _currentPosIcon = new ui08[icon_size];
  for (int i = 0; i < icon_size; i++)
    _currentPosIcon[i] = _params->_current_pos_icon[i];
  
  if (_params->way_pt0_icon_n != icon_size)
  {
    fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr, "Problem with TDRP parameters in file <%s>\n",
	    params_path);
    fprintf(stderr,
	    "Way pt 0 icon array contains %d values, should contain %d values\n",
	    _params->way_pt0_icon_n, icon_size);
    
    okay = false;
    
    return;
  }
  
  _wayPt0Icon = new ui08[icon_size];
  for (int i = 0; i < icon_size; i++)
    _wayPt0Icon[i] = _params->_way_pt0_icon[i];
  
  if (_params->way_pt1_icon_n != icon_size)
  {
    fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr, "Problem with TDRP parameters in file <%s>\n",
	    params_path);
    fprintf(stderr,
	    "Way pt 1 icon array contains %d values, should contain %d values\n",
	    _params->way_pt1_icon_n, icon_size);
    
    okay = false;
    
    return;
  }
  
  _wayPt1Icon = new ui08[icon_size];
  for (int i = 0; i < icon_size; i++)
    _wayPt1Icon[i] = _params->_way_pt1_icon[i];
  
  if (_params->way_pt2_icon_n != icon_size)
  {
    fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr, "Problem with TDRP parameters in file <%s>\n",
	    params_path);
    fprintf(stderr,
	    "Way pt 2 icon array contains %d values, should contain %d values\n",
	    _params->way_pt2_icon_n, icon_size);
    
    okay = false;
    
    return;
  }
  
  _wayPt2Icon = new ui08[icon_size];
  for (int i = 0; i < icon_size; i++)
    _wayPt2Icon[i] = _params->_way_pt2_icon[i];
  
  // initialize process registration

  PMU_auto_init(_progName, _params->servmap_instance,
		PROCMAP_REGISTER_INTERVAL);
}


/*********************************************************************
 * Destructor
 */

PosnRptSpdb2Symprod::~PosnRptSpdb2Symprod()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  // Free the icons

  delete [] _currentPosIcon;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

PosnRptSpdb2Symprod *PosnRptSpdb2Symprod::Inst(int argc, char **argv)
{
  if (_instance == (PosnRptSpdb2Symprod *)NULL)
    new PosnRptSpdb2Symprod(argc, argv);
  
  return _instance;
}

PosnRptSpdb2Symprod *PosnRptSpdb2Symprod::Inst()
{
  assert(_instance != (PosnRptSpdb2Symprod *)NULL);
  
  return _instance;
}


/*********************************************************************
 * run()
 */

void PosnRptSpdb2Symprod::run()
{
  _spdbServer->operate();
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _addCurrentPositionToBuffer() - Add the current position to the
 *                                 Symprod buffer.
 */

void PosnRptSpdb2Symprod::_addCurrentPositionToBuffer(symprod_product_t *prod,
						      const PosnRpt& posn_rpt,
						      const Params& params)
{
  symprod_wpt_t icon_origin;
  
  icon_origin.lat = posn_rpt.getCurrentLat();
  icon_origin.lon = posn_rpt.getCurrentLon();
  
  if (icon_origin.lat == WayPoint::BAD_POSITION ||
      icon_origin.lon == WayPoint::BAD_POSITION)
    return;
  
  // Add the icon to the symprod product

  SYMPRODaddBitmapIcon(prod,
		       params.current_pos_color,
		       16, 16,
		       1,
		       &icon_origin,
		       PosnRptSpdb2Symprod::Inst()->getCurrentPosIcon());
  
  // Add the flight number to the symprod product

  if (params.render_flight_num)
    SYMPRODaddText(prod,
		   params.current_pos_color,
		   "",
		   posn_rpt.getCurrentLat(), posn_rpt.getCurrentLon(),
		   params.flight_num_text_offsets.x_offset,
		   params.flight_num_text_offsets.y_offset,
		   SYMPROD_TEXT_VERT_ALIGN_BOTTOM,
		   SYMPROD_TEXT_HORIZ_ALIGN_LEFT,
		   10,
		   params.text_font,
		   (char *)posn_rpt.getFlightNum().c_str());
  
  // Add the position report time to the symprod product

  if (params.render_report_time)
  {
    DateTime report_time = posn_rpt.getCurrentTime();
    char report_time_string[12];
    
    sprintf(report_time_string, "%.2d%.2d%.2d",
	    report_time.getHour(),
	    report_time.getMin(),
	    report_time.getSec());
    
    SYMPRODaddText(prod,
		   params.current_pos_color,
		   "",
		   posn_rpt.getCurrentLat(), posn_rpt.getCurrentLon(),
		   params.report_time_text_offsets.x_offset,
		   params.report_time_text_offsets.y_offset,
		   SYMPROD_TEXT_VERT_ALIGN_BOTTOM,
		   SYMPROD_TEXT_HORIZ_ALIGN_LEFT,
		   10,
		   params.text_font,
		   report_time_string);
  }
  
}


/*********************************************************************
 * _addWayPtToBuffer() - Add the given way point to the Symprod buffer.
 */

void PosnRptSpdb2Symprod::_addWayPtToBuffer(symprod_product_t *prod,
					    const WayPoint& way_point,
					    const char *color,
					    const ui08 *icon)
{
  symprod_wpt_t icon_origin;
  
  icon_origin.lat = way_point.getLat();
  icon_origin.lon = way_point.getLon();
  
  if (icon_origin.lat == WayPoint::BAD_POSITION ||
      icon_origin.lon == WayPoint::BAD_POSITION)
    return;
  
  // Add the icon to the symprod product

  SYMPRODaddBitmapIcon(prod,
		       (char *)color,
		       16, 16,
		       1,
		       &icon_origin,
		       (ui08 *)icon);
}


/*********************************************************************
 * _addWayPtLineToBuffer() - Add the way point line to the Symprod buffer.
 */

void PosnRptSpdb2Symprod::_addWayPtLineToBuffer(symprod_product_t *prod,
						const PosnRpt& posn_rpt,
						const Params& params)
{
  symprod_wpt_t polyline_pts[3];
  
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

  SYMPRODaddPolyline(prod,
		     params.way_pt_line_color,
		     SYMPROD_LINETYPE_SOLID,
		     params.way_pt_line_width,
		     SYMPROD_CAPSTYLE_BUTT,
		     SYMPROD_JOINSTYLE_BEVEL,
		     FALSE,
		     FALSE,
		     polyline_pts,
		     3);
  
}


/*********************************************************************
 * _convertToSymprod() - Convert the data from the SPDB database to
 *                       symprod format.
 */

void *PosnRptSpdb2Symprod::_convertToSymprod(spdb_chunk_ref_t *spdb_hdr,
					       void *spdb_data,
					       int spdb_len,
					       int *symprod_len)

{
  symprod_product_t *prod;
  time_t now;
  
  // Get the rendering parameters

  Params *params = PosnRptSpdb2Symprod::Inst()->getParams();
  
  // Initialize returned values

  *symprod_len = 0;
  
  // Convert the SPDB data to a position report

  PosnRpt posn_rpt(spdb_data,
		   params->debug >= Params::DEBUG_MSGS);
  
  // See if this report should be displayed

  if (params->flight_list_n > 0)
  {
    const char *flight_num = posn_rpt.getFlightNum().c_str();
    
    bool found = false;
    
    for (int i = 0; i < params->flight_list_n; i++)
      if (STRequal_exact(flight_num, params->_flight_list[i]))
	found = true;
    
    if (!found)
      return NULL;
  }
  
  // create struct for internal representation of product

  now = time(NULL);

  if ((prod = SYMPRODcreateProduct(now, now,
				   spdb_hdr->valid_time,
				   spdb_hdr->expire_time)) == NULL)
    return NULL;

  // Convert the SPDB data to symprod format

  SYMPRODaddLabel(prod, (char *)posn_rpt.getFlightNum().c_str());
  
  _addCurrentPositionToBuffer(prod,
			      posn_rpt,
			      *params);

  if (params->render_way_pt0_icon)
    _addWayPtToBuffer(prod,
		      posn_rpt.getWayPoint0(),
		      params->way_pt0_color,
		      PosnRptSpdb2Symprod::Inst()->getWayPt0Icon());
  
  if (params->render_way_pt1_icon)
    _addWayPtToBuffer(prod,
		      posn_rpt.getWayPoint1(),
		      params->way_pt1_color,
		      PosnRptSpdb2Symprod::Inst()->getWayPt1Icon());
  
  if (params->render_way_pt2_icon)
    _addWayPtToBuffer(prod,
		      posn_rpt.getWayPoint2(),
		      params->way_pt2_color,
		      PosnRptSpdb2Symprod::Inst()->getWayPt2Icon());
  
  if (params->render_way_pt_line)
    _addWayPtLineToBuffer(prod,
			  posn_rpt,
			  *params);
  
  // set return buffer

  // copy internal representation of product to output buffer

  void *return_buffer = SYMPRODproductToBuffer(prod, symprod_len);

  if (params->debug >= Params::DEBUG_ALL)
    SYMPRODprintProductBuffer(stderr, (char *) return_buffer);

  // Put the product buffer in BE format for transmission
  
  SYMPRODproductBufferToBE((char *) return_buffer);

  // free up internal representation of product

  SYMPRODfreeProduct(prod);
  
  return return_buffer;

}
