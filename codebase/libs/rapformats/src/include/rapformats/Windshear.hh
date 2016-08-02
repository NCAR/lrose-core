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
 * @file Windshear.hh 
 * @brief Windshear information from alg. written to XML in MDV and SPDB
 * @class Windshear
 * @brief Windshear information from alg. written to XML in MDV and SPDB
 *
 * This is in the form of polygons. The polygons are colored when
 * displayed depending on alert status.
 * 
 * The polygons handled by this class are for 3 different product types:
 *  - alert shapes
 *  - runway boxes (both on the runway ahd off runeay 'arenas'
 *  - The so called 'red X' which is to indicate an impaired state
 *
 * See WindshearArena.hh for an alternative way to represent runways.
 */

#ifndef WINDSHEAR_H
#define WINDSHEAR_H

#include <string>
#include <vector>

//------------------------------------------------------------------
class Windshear
{
public:
  /**
   * @enum Event_t
   * @brief Types of events
   *
   * This covers the shapes and the arenas
   */
  typedef enum
  {
    NONE = -1,
    WS_GAIN = 0,
    WS_LOSS = 1,
    MICROBURST = 2,
    MODERATE_TURB = 3,
    SEVERE_TURB = 4,
    ARENA_NONE = 5,
    ARENA_WS_GAIN = 6,
    ARENA_WS_LOSS = 7,
    ARENA_MICROBURST = 8,
    ARENA_MODERATE_TURB = 9,
    ARENA_SEVERE_TURB = 10,
    RED_X = 11,
    EMPTY_X = 12
  } Event_t;

  /**
   * Empty Construct
   */
  Windshear(void);

  /**
   * Construct from input arguments
   *
   * @param[in] t  Time
   * @param[in] type Event type
   * @param[in] magnitude
   * @param[in] latlon  The pairs of points that define the polygon,
   *                    as latitude/longitudes (first = latitude).
   *
   * The latlon vector should be closed, i.e. last point = first point
   */
  Windshear(const time_t &t, Event_t type, double magnitude, 
	    std::vector<std::pair<double,double> > &latlon);
  /**
   * Construct from XML content
   *
   * @param[in] s  String with XML in it, as from writeXml().
   *
   * If there are problems in input string, calls to
   * isWellFormed() will return false
   */
  Windshear(const std::string &s);

  /**
   * Destructor
   */
  virtual ~Windshear(void);

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
   * @param[in] name  A string that should correspond to a known type,
   *                  as from sprintType()
   * @param[out] type
   * @return true if type was translated from name
   *
   */
  static bool setType(const std::string &name, Event_t &type);

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
   * @return objet type
   */
  inline Event_t getType(void) const {return _type;}

  /**
   * @return object number of points
   */
  inline int numPt(void) const {return static_cast<int>(_latlon.size());}

  /**
   * @return a particular points latitude and longitude
   * @param[in] i  Index to points
   * @param[out] lat  Latitude
   * @param[out] lon  Longitude
   */
  inline void ithPt(int i, double &lat, double &lon) const
  {
    lat = _latlon[i].first;
    lon = _latlon[i].second;
  }

  /**
   * @return time value
   */
  inline time_t getTime(void) const {return _time;}


protected:

  time_t _time;           /**< Data time */
  Event_t _type;          /**< Event type */
  double _magnitude;     /**< Event magnitude, loss or gain only */

  /**
   * The polygon endpoint latitude/longitudes, in order
   * first = latitude, second = longitude.
   * Assumed closed (last point = first)
   */
  std::vector<std::pair<double,double> > _latlon;

private:

  bool _wellFormed;      /**< True if object is well formed */
};

#endif
