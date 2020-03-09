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
//   $Id: WxhazardsSpdb2Symprod.cc,v 1.2 2016/03/07 18:41:15 dixon Exp $
//   $Revision: 1.2 $
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
#include <spdbFormats/ConvRegionHazard.hh>
#include <spdbFormats/WxHazard.hh>
#include <spdbFormats/WxHazardBuffer.hh>
#include <symprod/symprod.h>
#include <toolsa/membuf.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Params.hh"
#include "WxhazardsSpdb2Symprod.hh"

// Global variables

WxhazardsSpdb2Symprod *WxhazardsSpdb2Symprod::_instance =
  (WxhazardsSpdb2Symprod *)NULL;

// Global constants

const int FOREVER = true;


/*********************************************************************
 * Constructor
 */

WxhazardsSpdb2Symprod::WxhazardsSpdb2Symprod(int argc, char **argv)
{
  static char *routine_name = "Constructor";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (WxhazardsSpdb2Symprod *)NULL);
  
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
  
  // Create the server object

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
  

  // initialize process registration

  PMU_auto_init(_progName, _params->servmap_instance,
		PROCMAP_REGISTER_INTERVAL);
}


/*********************************************************************
 * Destructor
 */

WxhazardsSpdb2Symprod::~WxhazardsSpdb2Symprod()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

WxhazardsSpdb2Symprod *WxhazardsSpdb2Symprod::Inst(int argc, char **argv)
{
  if (_instance == (WxhazardsSpdb2Symprod *)NULL)
    new WxhazardsSpdb2Symprod(argc, argv);
  
  return(_instance);
}

WxhazardsSpdb2Symprod *WxhazardsSpdb2Symprod::Inst()
{
  assert(_instance != (WxhazardsSpdb2Symprod *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run()
 */

void WxhazardsSpdb2Symprod::run()
{
  _spdbServer->operate();
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _addConvRegionToBuffer() - Add the given convective region to the
 *                            Symprod buffer.
 */

void WxhazardsSpdb2Symprod::_addConvRegionToBuffer(symprod_product_t *prod,
						   ConvRegionHazard *hazard,
						   Params *params)
{
  static MEMbuf *mbuf_points = NULL;
  
  // Initialize the MEMbuf object

  if (mbuf_points == NULL)
    mbuf_points = MEMbufCreate();
  else
    MEMbufReset(mbuf_points);
  
  // Load the polyline data into the point buffer

  WorldPolygon2D *polygon = hazard->getPolygon();
  
  for (WorldPoint2D *point = polygon->getFirstPoint();
       point != (WorldPoint2D *)NULL;
       point = polygon->getNextPoint())
  {
    symprod_wpt_t symprod_point;
    
    symprod_point.lat = point->lat;
    symprod_point.lon = point->lon;
    MEMbufAdd(mbuf_points, &symprod_point, sizeof(symprod_point));
  }
  
  SYMPRODaddPolyline(prod,
		     params->conv_region_color,
		     SYMPROD_LINETYPE_SOLID,
		     params->conv_region_line_width,
		     SYMPROD_CAPSTYLE_BUTT,
		     SYMPROD_JOINSTYLE_BEVEL,
		     TRUE,
		     SYMPROD_FILL_NONE,
		     (symprod_wpt_t *)MEMbufPtr(mbuf_points),
		     MEMbufLen(mbuf_points) / sizeof(symprod_wpt_t));
  
  return;
}


/*********************************************************************
 * _convertToSymprod() - Convert the data from the SPDB database to
 *                       symprod format.
 */

void *WxhazardsSpdb2Symprod::_convertToSymprod(spdb_chunk_ref_t *spdb_hdr,
					       void *spdb_data,
					       int spdb_len,
					       int *symprod_len)
{
  symprod_product_t *prod;
  time_t now;
  
  // Get the rendering parameters

  Params *params = WxhazardsSpdb2Symprod::Inst()->getParams();
  
  // Initialize returned values

  *symprod_len = 0;
  
  // Convert the SPDB data to a weather hazards buffer

  WxHazardBuffer hazard_buffer(spdb_hdr, spdb_data,
			       params->debug >= Params::DEBUG_MSGS);
  
  // create struct for internal representation of product

  now = time(NULL);

  if ((prod = SYMPRODcreateProduct(now, now,
				   spdb_hdr->valid_time,
				   spdb_hdr->expire_time)) == NULL)
    return (NULL);

  // Convert the SPDB data to symprod format

  for (WxHazard *hazard = hazard_buffer.getFirstHazard();
       hazard != 0;
       hazard = hazard_buffer.getNextHazard())
  {
    switch(hazard->getHazardType())
    {
    case WxHazard::CONVECTIVE_REGION_HAZARD :
      _addConvRegionToBuffer(prod,
			     (ConvRegionHazard *)hazard,
			     params);
      
      break;
    }
    
  }
  
  // set return buffer

  // copy internal representation of product to output buffer

  void *return_buffer = SYMPRODproductToBuffer(prod, symprod_len);

  if (params->debug >= Params::DEBUG_ALL)
  {
    SYMPRODprintProductBuffer(stderr, (char *) return_buffer);
  }

  // Put the product buffer in BE format for transmission
  
  SYMPRODproductBufferToBE((char *) return_buffer);

  // free up internal representation of product

  SYMPRODfreeProduct(prod);
  
  return(return_buffer);

}
