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
 * @file WindshearArena.hh 
 * @brief Arena information from alg. written to XML in MDV and SPDB
 * @class WindshearArena
 * @brief Arena information from alg. written to XML in MDV and SPDB
 * 
 * This is the 'gsd' style Arena data class. For the Hong Kong GSD the
 * runway was represented as a rectangle, and the arrival and departure
 * arenas were represented as line segments centered along the path, with
 * crossing 'tick marks' at each end.
 *
 * Both the runway and the arrival/departure arenas also showed a dotted
 * enclosing rectangle.  This rectangle is given different colors depending
 * on the alert status.
 */

#ifndef WINDSHEAR_ARENA_H
#define WINDSHEAR_ARENA_H

#include <string>
#include <vector>

//------------------------------------------------------------------
class WindshearArena
{
public:
  /**
   * @enum Event_t
   * @brief Types of events
   */
  typedef enum
  {
    ARENA_NONE = 0,
    ARENA_WS_GAIN = 1,
    ARENA_WS_LOSS = 2,
    ARENA_MICROBURST = 3,
    ARENA_MODERATE_TURB = 4,
    ARENA_SEVERE_TURB = 5
  } Event_t;

  /**
   * Empty
   */
  WindshearArena(void);

  /**
   * Construct from input arguments, for the runway itself.
   * @param[in] t  Time
   * @param[in] type Event type
   * @param[in] magnitude
   * @param[in] alertLatlon The alerting region (dotted lines) polygon
   * @param[in] displayLatlon The runway box polygon
   *
   * The lat lon vectors are first=lat, and closed (last = first)
   */
  WindshearArena(const time_t &t, Event_t type, double magnitude, 
		 std::vector<std::pair<double,double> > &alertLatlon,
		 std::vector<std::pair<double,double> > &displayLatlon);

  /**
   * Construct from input arguments, for an off runway arena
   * @param[in] t  Time
   * @param[in] type Event type
   * @param[in] magnitude
   * @param[in] alertLatlon The alerting region (dotted lines) polygon
   * @param[in] displayLatlon The box that would extend the runway box
   *                          (currently not used in display, but stored).
   * @param[in] center   Center line, should be 2 points
   * @param[in] cross0   Crossing line 'tick mark', should be 2 points
   * @param[in] cross1   Crossing line 'tick mark', should be 2 points
   *
   * The lat lon vectors are first=lat
   *
   * The alertLatlon and displayLatlon should be closed (last = first)
   */
  WindshearArena(const time_t &t, Event_t type, double magnitude, 
		 std::vector<std::pair<double,double> > &alertLatlon,
		 std::vector<std::pair<double,double> > &displayLatlon,
		 std::vector<std::pair<double,double> > &center,
		 std::vector<std::pair<double,double> > &cross0,
		 std::vector<std::pair<double,double> > &cross1);

  /**
   * Construct from XML content
   * @param[in] s  String with XML in it, as from writeXml().
   * @param[in] showErrors  True to show error condition
   *
   * If there are problems in input string, calls to
   * isWellFormed() will return false
   */
  WindshearArena(const std::string &s, bool showErrors=true);

  /**
   * Destructor
   */
  virtual ~WindshearArena(void);

  /**
   * Clear to empty
   */
  void clear(void);

  /** 
   * Write internal state as an XML string
   * @return The string with XML
   */
  std::string writeXml(void) const;

  /**
   * Write internal state as an XML output string, written as a file
   * @param[in] path  Name of the file to write
   */
  void writeXml(const std::string &path) const;

  /**
   * Append internal state as an XML output string to a file
   * @param[in] path  Name of the file to append to
   */
  void appendXml(const std::string &path) const;

  /**
   * Set type using input name
   *
   * @param[in] name  Name as from sprintType()
   * @param[out] type  Returned type
   * @param[in] showErrors  True to show error condition
   *
   * @return true if type was translated from name
   */
  static bool setType(const std::string &name, Event_t &type, bool showErrors);

  /**
   * @return a string to go with a type
   * @param[in] type
   */
  static std::string sprintType(Event_t type);

  /**
   * @return true if object is good
   */
  inline bool isWellFormed(void) const {return _wellFormed;}

  /**
   * @return object magnitude 
   */
  inline double getMagnitude(void) const {return _magnitude;}

  /**
   * @return true if this is an on-runway object
   */
  inline bool isRunway(void) const {return _onRunway;}

  /**
   * @return objet type
   */
  inline Event_t getType(void) const {return _type;}

  /**
   * @return the points that make up the box as pairs of lat/lon, first=lat.
   *
   * This is typically only called only when isRunway()=true
   */
  inline std::vector<std::pair<double,double> > getDisplayPoints(void) const
  {
    return _displayLatlon;
  }

  /**
   * @return the alert polygon box (for dotted lines) as pairs of lat/lon,
   * first = lat.
   */
  inline std::vector<std::pair<double,double> > getAlertPoints(void) const
  {
    return _alertLatlon;
  }

  /**
   * @return the center line, which should be a vector of length 2,
   * with lat/lon, lat=first
   *
   * This is typically only called only when isRunway()=false
   */
  inline std::vector<std::pair<double,double> > getCenter(void) const
  {
    return _centerLatlon;
  }

  /**
   * @return one ends 'tickmark' which should be a vector of length 2,
   * with lat/lon, lat=first
   *
   * @param[in] which  0 or 1
   *
   * This is typically only called only when isRunway()=false
   */
  inline std::vector<std::pair<double,double> > getCross(int which) const
  {
    if (which == 0)
    {
      return _crossLatlon0;
    }
    else
    {
      return _crossLatlon1;
    }
  }
  
  /**
   * @return time value
   */
  inline time_t getTime(void) const {return _time;}


protected:

  time_t _time;           /**< Data time */
  Event_t _type;          /**< Event type */
  double _magnitude;     /**< Event magnitude, loss or gain only */
  bool _onRunway;        /**< True for arena on runway */

  /**
   * The polygon endpoint latitude/longitudes, in order for alert region
   * first = latitude, second = longitude, this is the region shown as
   * dotted lines
   */
  std::vector<std::pair<double,double> > _alertLatlon;

  /**
   * The polygon endpoint latitude/longitudes, in order for the box
   * first = latitude, second = longitude, which is typically displayed only
   * for the runway itself.
   */
  std::vector<std::pair<double,double> > _displayLatlon;

  /**
   * The center line endpoint latitude/longitudes, should be 2 points
   * first = latitude, second = longitude, should be meaningful only for off
   * runway
   */
  std::vector<std::pair<double,double> > _centerLatlon;

  /**
   * The crossing line endpoint latitude/longitudes, should be 2 points
   * first = latitude, second = longitude, should be meaningful only for off
   * runway
   */
  std::vector<std::pair<double,double> > _crossLatlon0;

  /**
   * The crossing line endpoint latitude/longitudes, should be 2 points
   * first = latitude, second = longitude, should be meaningful only for off
   * runway
   */
  std::vector<std::pair<double,double> > _crossLatlon1;


private:

  bool _wellFormed;      /**< True if object is well formed */
};

#endif
