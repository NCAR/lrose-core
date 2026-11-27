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
/**
 *
 * @file ProductHandler.hh
 *
 * @class ProductHandler
 *
 * Base class for product handlers.
 *  
 * @date 10/10/2009
 *
 */

#ifndef _ProductHandler_hh
#define _ProductHandler_hh

#include <vector>
#include <iostream>
#include <iomanip>
#include <cstdarg>

#include <Spdb/Symprod.hh>
#include <Spdb/DsSymprodServer.hh>
#include <xmlformats/SigwxPoint.hh>

#include "BoundingBox.hh"
#include "IconDef.hh"
#include "Params.hh"

using namespace std;


/**
 * @class ProductHandler
 */

class ProductHandler
{

public:

  /**
   * @brief Constructor.
   */

  ProductHandler(Params *params, const int debug_level = 0);
  
  /**
   * @brief Destructor.
   */

  virtual ~ProductHandler();
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const string colorBlue;
  static const string colorGreen;
  static const string colorMagenta;
  static const string colorPink;
  static const string colorPurple;
  static const string colorRed;


  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Debug level.
   */

  int _debugLevel;
  

  /**
   * @brief Parameter file parameters.
   */

  Params *_params;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Find the first point in the list that falls within the defined
   *        bounding box.
   *
   * @param[in] points The list of points.
   * @param[in] bbox Bounding box of the display area.
   *
   * @return Returns the index of the first point if there is one, -1 if no
   *         points fall within the box.
   */

  int _findFirstInBox(const vector< SigwxPoint > & points,
		      const BoundingBox &bbox) const;
  

  /**
   * @brief Adjust the longitudes of the given list of waypoints to be
   *        within the defined bounding box.  Replace each point outside of
   *        the box with a point on the box edge.
   *
   * @param[out] The waypoints representing the points in the box.
   * @param[in] numPts The number of points in the list.
   * @param[in] ifirst The index of the first point in the list that falls
   *                   within the bounding box.
   * @param[in] bbox Bounding box of the display area.
   */

  bool _adjustLongitudes(Symprod::wpt_t * waypoints,
			 const int numPts,
			 const int ifirst,
			 const BoundingBox &bbox) const;
  

  /**
   * @brief Draw an icon.
   *
   * @param[in] icon_name Icon name.
   * @param[in] icon_def_list List of defined icons.
   * @param[in] color_to_use Color to use.
   * @param[in] line_width Line width to use.
   * @param[in] center_lat Center latitude of icon.
   * @param[in] center_lon Center longitude of icon.
   * @param[in] icon_scale Icon scale.
   * @param[in] allow_client_scaling Flag indicating whether to allow client
   *                                 scaling.
   * @param[in,out] symprod Symprod buffer.
   */

  void _drawIcon(const string &icon_name,
		 const map< string, IconDef > &icon_def_list,
		 const string &color_to_use,
		 const int line_width,
		 const double center_lat,
		 const double center_lon,
		 const double icon_scale,
		 const bool allow_client_scaling,
		 Symprod *symprod) const;
  

  /**
   * @brief Get the list of waypoints from the WAFS SIGWX point list.  In the
   *        process, normalize the point longitudes based on the given
   *        bounding box.
   *
   * @param[in] points The points in the SIGWX event.
   * @param[in] bbox The bounding box.
   * @param[out] num_pts The number of points in the waypoint list.
   *
   * @return Returns a pointer to the waypoint list on success, 0 if there
   *         are no points to process.
   */

  Symprod::wpt_t *_getWaypoints(const vector< SigwxPoint > &points,
				const BoundingBox &bbox,
				size_t &num_pts) const;
  

  /**
   * @brief Set the waypoint location to the item location, moving the
   *        waypoint location to the edge of the bounding box if it lies
   *        outside of that box.
   *
   * @param[in] point The original point.
   * @param[out] waypoint The location of the point within the bounding box.
   * @param[in] prevLon Previous transformed longitude value.
   * @param[in] bbox Bounding box of the display area.
   */

  void _setWaypoint(const SigwxPoint &point,
		    Symprod::wpt_t * waypoint,
		    double prevLon,		    
		    const BoundingBox &bbox) const;
  

  /**
   * @brief Set the waypoint location to the item location, moving the
   *        waypoint location to the edge of the bounding box if it lies
   *        outside of that box.
   *
   * @param[out] waypoint The location of the point within the bounding box.
   * @param[in] prevLon Previous transformed longitude value.
   * @param[in] bbox Bounding box of the display area.
   */

  void _setWaypointDelta(Symprod::wpt_t &waypoint,
			 const double prevLon,
			 const BoundingBox &bbox) const;
  

  /**
   * @brief Add the given text buffer to the Symprod buffer.
   *
   * @param[in,out] symprod Symprod buffer.
   * @param[in] txtLat Text latitude.
   * @param[in] txtLon Text longitude.
   * @param[in] msgs List of text messages.
   * @param[in] text_color Text color.
   * @param[in] text_bg_color Text background color.
   */

  void _addTextToSymprod(Symprod *symprod,
			 const double txtLat,
			 const double txtLon,
			 const vector< string > &msgs,
			 const vector< bool > &hiddens,
			 const string &text_color,
			 const string &text_bg_color) const;
  

  /**
   * @brief Print general information upon entry to one of the
   *        convertToSymprod methods.
   *
   * @param[in] funcLable Function label.
   * @param[in] dir_path Directory path.
   * @param[in] prod_id SPDB product id.
   * @param[in] prod_label SPDB product label.
   * @param[in] chunk_ref SPDB chunk header.
   * @param[in] spdb_data SPDB data buffer.
   * @param[in] spdb_len Length of SPDB data buffer in bytes.
   * @param[in] bbox Bounding box of the display area.
   */

  void _printEntryInfo(const string &funcLabel,
		       const string &dir_path,
		       const int prod_id,
		       const string &prod_label,
		       const Spdb::chunk_ref_t &chunk_ref,
		       const void *spdb_data,
		       const int spdb_len,
		       const BoundingBox &bbox) const;
  

};

  
#endif
