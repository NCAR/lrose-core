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
 * @file JetStreamHandler.hh
 *
 * @class JetStreamHandler
 *
 * Class for handling jet stream data.
 *  
 * @date 10/10/2009
 *
 */

#ifndef _JetStreamHandler_hh
#define _JetStreamHandler_hh

#include <map>
#include <string>

#include <Spdb/Spdb.hh>
#include <Spdb/Symprod.hh>
#include <toolsa/MemBuf.hh>

#include "BoundingBox.hh"
#include "DisplayItem.hh"
#include "IconDef.hh"
#include "Params.hh"
#include "ProductHandler.hh"

using namespace std;

/**
 * @class JetStreamHandler
 */

class JetStreamHandler : public ProductHandler
{

public:

  /**
   * @brief Constructor.
   */

  JetStreamHandler(Params *params, const int debug_level = 0);
  
  /**
   * @brief Destructor.
   */

  virtual ~JetStreamHandler();
  

  ///////////////////////
  // Rendering methods //
  ///////////////////////

  /**
   * @brief Convert the volcano data to Symprod format.
   *
   * @param[in] dir_path Data directory path.
   * @param[in] prod_id SPDB product ID.
   * @param[in] prod_label SPDB product label.
   * @param[in] chunk_ref Chunk header for this data.
   * @param[in] aux_ref Auxilliary chunk header for this data.
   * @param[in] spdb_data Data pointer.
   * @param[in] spdb_len Length of data buffer.
   * @param[in,out] symprod_buf Symprod buffer.
   * @param[in] bbox Bounding box of the display area.
   *
   * @return Returns 0 on success, -1 on failure.
   */

  int convertToSymprod(const string &dir_path,
		       const int prod_id,
		       const string &prod_label,
		       const Spdb::chunk_ref_t &chunk_ref,
		       const Spdb::aux_ref_t &aux_ref,
		       const void *spdb_data,
		       const int spdb_len,
		       MemBuf &symprod_buf,
		       const BoundingBox &bbox) const;
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief The length of the arrowhead in pixels.
   */

  static const int ARROWHEAD_LEN_PIXELS;
  

  /**
   * @brief The half angle of the arrowhead.
   */

  static const double ARROWHEAD_HALF_ANGLE;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Add the defined change bar to the Symprod buffer.
   *
   * @param[in,out] symprod Symprod buffer.
   * @param[in] bbox The display bounding box.
   * @param[in] point The SIGWX information for this point.
   * @param[in] prev_point The SIGWX information for the previous point.
   * @param[in] waypoints The list of jetstream points.
   * @param[in] num_pts The number of waypoints in the list.
   * @param[in] ipt The index in waypoints of the location for this change bar.
   *
   * @return Returns true on success, false on failure.
   */

  bool _addChangeBar(Symprod &symprod,
		     const BoundingBox &bbox,
		     const SigwxPoint &point,
		     const SigwxPoint &prev_point,
		     const Symprod::wpt_t *waypoints,
		     const int num_pts,
		     const int ipt) const;
  

  /**
   * @brief Add the defined wind fleche to the Symprod buffer.
   *
   * @param[in,out] symprod Symprod buffer.
   * @param[in] bbox The display bounding box.
   * @param[in] point The WAFS SIGWX point information.
   * @param[in] waypoints The list of jetstream points.
   * @param[in] num_pts The number of waypoints in the list.
   * @param[in] ipt The index in waypoints of the location for this fleche.
   *
   * @return Returns true on success, false on failure.
   */

  bool _addWindFleche(Symprod &symprod,
		      const BoundingBox &bbox,
		      const SigwxPoint &point,
		      const Symprod::wpt_t *waypoints,
		      const int ipt,
		      const int num_pts) const;
  

  /**
   * @brief Add the list of wind fleches to the Symprod buffer.
   *
   * @param[in] points The WAFS SIGWX information for the points.
   * @param[in] waypoints The locations for the fleches.  The points in this
   *                      array must match the points in the points vector.
   * @param[in] num_pts The number of points in the points vector and the
   *                    waypoints array.
   * @param[in,out] symprod Symprod buffer.
   * @param[in] bbx The bounding box of the display.
   *
   * @return Returns true on success, false on failure.
   */

  bool _addWindFleches(const vector< SigwxPoint > &points,
		       const Symprod::wpt_t *waypoints,
		       const size_t num_pts,
		       Symprod &symprod,
		       const BoundingBox &bbox) const;
  

  /**
   * @brief Calculate the distance in number of characters between the given
   *        points assuming the given bounding box for the screen.
   *
   * @param[in] bbox The screen bounding box.
   * @param[in] point1 The first point.
   * @param[in] point2 The second point.
   *
   * @return Returns the distance between the two points in number of 
   *         characters.
   */

  double _calcDistance(const BoundingBox &bbox,
		       const Symprod::wpt_t &point1,
		       const Symprod::wpt_t &point2) const;
  

  /**
   * @brief Display the points in the point list.
   *
   * @param[in] points Points to display.
   * @param[in,out] symprod Symprod buffer.
   * @param[in] bbox Bounding box of the display area.
   */

  bool _displayList(const vector< SigwxPoint > &points,
		    Symprod &symprod,
		    const BoundingBox &bbox) const;
  

  /**
   * @brief Create the wind fleche representing this value.
   *
   * @param[in,out] symprod Symprod buffer.
   * @param[in] wind_speed_m Wind speed in meters.
   * @param[in] location Location for the fleche.
   * @param[in] line_pt1 First point of line defining angle for the rendering
   *                     of the fleche.
   * @param[in] line_pt2 Second point of line defining angle for the rendering
   *                     of the fleche.
   * @param[in] north_hem_flag Flag indicating whether this jet stream starts
   *                           in the northern hemisphere.
   *
   * @return Returns true on success, false on failure.
   */

  bool _drawFleche(Symprod &symprod,
		   const double wind_speed_m,
		   const Symprod::wpt_t location,
		   const Symprod::wpt_t line_pt1,
		   const Symprod::wpt_t line_pt2,
		   const bool north_hem_flag) const;
  

  /**
   * @brief Rotate the given icon and flip the icon if it is in the
   *        southern hemisphere.  Return the rotated icon in an array
   *        ready to send to a Symprod buffer.
   *
   * @param[in] icon List of icon points to be rotated.
   * @param[in] line_pt1 The first point of the line used to determine the
   *                     angle of rotation.
   * @param[in] line_pt2 The second point of the line used to determine the
   *                     angle of rotation.
   * @param[in] north_hem_flag Flag indicating whether we are in the northern
   *                           hemisphere.
   *
   * @return Returns a pointer to the array of rotated icon points.  This 
   *         pointer must be deleted by the calling method.
   */

  Symprod::ppt_t *_rotateIcon(vector< Symprod::ppt_t > icon,
			      const Symprod::wpt_t line_pt1,
			      const Symprod::wpt_t line_pt2,
			      const bool north_hem_flag) const;
  

};


#endif
