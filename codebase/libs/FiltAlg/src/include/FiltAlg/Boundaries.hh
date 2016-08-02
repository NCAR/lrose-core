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
 * @file Boundaries.hh 
 * @brief The handling of boundary data through static methods
 * @class Boundaries
 * @brief The handling of boundary data through static method
 */

#ifndef BOUNDARIES_H
#define BOUNDARIES_H
#include <ctime>
#include <vector>
#include <string>

class LineList;
class PointList;
class GridProj;
class MotionVector;

//------------------------------------------------------------------
class Boundaries 
{
public:

  /**
   * @enum e_data_t 
   * Types of spdb colide data.
   */
  typedef enum 
  {
    TYPE_UNKNOWN = -1,   /**< Unknown */
    DETECT_POINTS=0,     /**< boundary points for downstream and display. */
    EXTRAP_POINTS=1,     /**< extrapolated points for display. */
    CONNECTED_LINES=2,   /**< linelists where each line is chained to next. */
    UNCONNECTED_LINES=3, /**< linelists where lines are not associated */
    NUM_SPDB_DATA = 4    /**< Number of enums */
  } e_data_t;

  /**
   * Constructor
   */
  inline Boundaries(void) {}

  /**
   * Destructor
   */
  inline virtual ~Boundaries(void) {}

  /**
   * Write a bunch of line segments to SPDB
   * Each LineList has unconnected lines
   *
   * @param[in] time  Time of data to write
   * @param[in] sequence  Sequence number
   * @param[in] spdbUrl  Where to write
   * @param[in] l  Zero or more linelists
   * 
   * Each linelist should have these attributes:
   *   ID
   *   Quality
   *   Percentile_used
   *
   * @return true for success in writing.
   */
  static bool write_lines(const time_t &time, const int sequence,
			  const std::string &spdbUrl,
			  const std::vector<LineList> &l);

  /**
   * Write a bunch of polylines to SPDB
   * Each LineList has connected polylines
   *
   * @param[in] time  Time of data to write
   * @param[in] sequence  Sequence number
   * @param[in] spdbUrl  Where to write
   * @param[in] l  Zero or more linelists
   * 
   * Each linelist should have these attributes:
   *   ID
   *   Quality
   *
   * @return true for success in writing.
   */
  static bool write_connected_lines(const time_t &time,
				    const int sequence,
				    const std::string &spdbUrl,
				    const std::vector<LineList> &l);

  /**
   * Write a bunch of polylines to SPDB, and write a 60 minute extrapolation
   *
   * Each PointList has connected polylines
   *
   * @param[in] time  Time of data to write
   * @param[in] sequence  Sequence number
   * @param[in] spdbUrl  Where to write
   * @param[in] l  Zero or more linelists
   * 
   * Each PointList should have these attributes:
   *   ID
   *   Quality (optional, set to 1 if not present)
   *   MotionVector (optional, set to (0,0) if not present)
   *
   * Each Point in each PointList should have MotionVector (set to 0,0 if not)
   *
   * @return true for success in writing.
   */
  static bool write_connected_points(const time_t &time,
				    const int sequence,
				    const std::string &spdbUrl,
				    const std::vector<PointList> &l);

  /**
   * Read in unconnected lines from file as linelists
   * @param[in] time  Time to read for
   * @param[in] spdbUrl  Where to read from
   * 
   * @return 0 or more LineLists as read from file
   * 
   * Each linelist has attributes:
   *   ID
   *   Quality
   *   Percentile_used
   *
   * Each linelist is one chunk in the database.
   *
   * The chunk is assumed consisting of pairs of points that each make up
   * one line.  The lines are unconnected.
   *
   * Each line has the attribute Percentile_used
   *
   */
  static
  std::vector<LineList> read_lines(const time_t &time,
				   const std::string &spdbUrl);

  /**
   * Read in connected polygons from file as linelists
   * @param[in] time  Time to read for
   * @param[in] spdbUrl  Where to read from
   * 
   * @return 0 or more LineLists as read from file
   * 
   * Each linelist has attributes:
   *   ID
   *   Quality
   *
   * Each linelist is one chunk in the database.
   *
   * The chunk is assumed consisting of a sequence of 2 point polylines that
   * make up one line.  The lines are connected.
   *
   * @note so far do not need ability to read in motion vector info.
   *
   */
  static
  std::vector<LineList> read_connected_lines(const time_t &time,
					     const std::string &spdbUrl);

  /**
   * Erase all data from a URL at a time
   * @param[in] time
   * @param[in] spdbUrl
   */
  static void erase(const time_t &time, const std::string &spdbUrl);

protected:
private:

  static void _get_speed_dir_for_spdb(const MotionVector &mv,
				      double &speed, double &dir);

};

#endif
