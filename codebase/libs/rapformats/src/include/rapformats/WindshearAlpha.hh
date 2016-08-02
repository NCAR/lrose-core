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
 * @file WindshearAlpha.hh 
 * @brief WindshearAlpha information regarding runway alerts, written to XML 
 * @class WindshearAlpha
 * @brief WindshearAlpha information regarding runway alerts, written to XML 
 *
 * This is a single alphanumeric alert, shown on a single line in an
 * alphanumeric display
 */

#ifndef WINDSHEAR_ALPHA_H
#define WINDSHEAR_ALPHA_H

#include <string>

//------------------------------------------------------------------
class WindshearAlpha
{
public:

  /**
   * @enum Event_t
   * @brief Types of alert events
   *
   * NOTE it must be ordered such that bigger number = worse alert
   */
  typedef enum
  {
    IMPAIRED = -2,
    NONE = -1,
    WS_GAIN = 0,
    WS_LOSS = 1,
    MODERATE_TURB = 2,
    SEVERE_TURB = 3,
    MICROBURST = 4
  } Alert_t;

  /**
   * @enum Runway_t
   * @brief Types of runways, (arrival or depart)
   */
  typedef enum
  {
    BAD = -1,
    ARRIVAL = 0,
    DEPART = 1,
  } Runway_t;

  /**
   * Empty construct
   */
  WindshearAlpha(void);

  // /**
  //  * Construct for empty line
  //  *
  //  * type  Expect IMPAIRED or NONE
  //  *
  //  */
  // WindshearAlpha(Alert_t type);
		 
  /**
   * Construct from input arguments, for an alert
   *
   * @param[in] name  Runway name
   * @param[in] type Alert type
   * @param[in] runwayType runway type
   * @param[in] magnitude
   * @param[in] location  0, 1, 2, 3,...  nautical miles from runway
   */
  WindshearAlpha(const std::string &name, Alert_t type, Runway_t runwayType,
		 int magnitude, int location);
		 
  /**
   * Construct from input arguments, for no alert
   *
   * @param[in] name  Runway name
   * @param[in] runwayType runway type
   */
  WindshearAlpha(const std::string &name, Runway_t runwayType);

		 
  /**
   * Construct from input arguments, for impaired
   *
   * @param[in] impairedMsg
   * @param[in] maxLength  Length per line, message is padded or truncated
   *                       to this length
   */
  WindshearAlpha(const std::string &impairedMsg, const int maxLength);
		 
		 
  /**
   * Construct from XML content
   * @param[in] s  String with XML in it, as from writeXml()
   *
   * If there are problems in input string, calls to
   * isWellFormed() will return false
   */
  WindshearAlpha(const std::string &s);

  /**
   * Destructor
   */
  virtual ~WindshearAlpha(void);

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
   * Set alert type using input name
   * @param[in] name
   * @param[out] type
   * @return true if type was translated from name
   */
  static bool setType(const std::string &name, Alert_t &type);

  /**
   * Set runway type using input name
   * @param[in] name
   * @param[out] type
   * @return true if type was translated from name
   */
  static bool setRunwayType(const std::string &name, Runway_t &type);


  /**
   * @return the XML tag used to contain this classes XML output
   */
  inline static std::string tag(void) { return "WindshearAlpha";}

  /**
   * @return a string to go with an alert type, for debugging
   * @param[in] type
   */
  static std::string sprintType(Alert_t type);

  /**
   * @return a string to go with an alert type, shorter format
   * @param[in] type
   *
   * Values are "MB  ", "GAIN",  "LOSS", "    ", ...
   */
  static std::string sprintTypeShort(Alert_t type);

  /**
   * @return units to go with an alert type, , "kt" for windshears, "  " 
   * otherwise
   *
   * @param[in] type
   *
   */
  static std::string sprintUnits(Alert_t type);

  /**
   * @return a string to go with a runway type, for debugging
   * @param[in] type
   */
  static std::string sprintRunwayType(Runway_t type);

  /**
   * @return a string to go with a runway type, shorter format
   *  "A" or "D"
   * @param[in] type
   */
  static std::string sprintRunwayTypeShort(Runway_t type);

  /**
   * @return a string to go with an arena indicating final or depart
   * "F" or "D"
   * 
   * @param[in] type
   */
  static std::string sprintArenaDesignator(Runway_t type);

  /**
   * @return string for magnitude, for use in alphanumeric messages
   * @param[in] mag  The magnitude
   */
  static std::string sprintMagnitude(int mag);

  /**
   * @return object runway type that is opposite
   * (DEPART returns ARRIVAL, vice versa)
   *
   * @param[in] type
   */
  static Runway_t oppositeRunwayType(Runway_t type);

  /**
   * @return true if object is good
   */
  inline bool isWellFormed(void) const {return _wellFormed;}

  /**
   * @return object magnitude 
   */
  inline int getMagnitude(void) const {return _magnitude;}

  /**
   * @return object alert type
   */
  inline Alert_t getType(void) const {return _type;}

  /**
   * @return object runway type
   */
  inline Runway_t getRunwayType(void) const {return _runwayType;}

  /**
   * @return object location 0, 1, 2, ..
   */
  inline int getLocation(void) const {return _location;}


  /** 
   * Write internal state as an alphanumeric line, which could be used
   * for operational alphanumeric alerts
   *
   * @return The string
   */
  std::string writeAlphanumeric(void) const;


  // /** 
  //  * Write internal state as an alphanumeric line, which could be used
  //  * for operational alphanumeric alerts, for impaired state
  //  *
  //  * @param[in] impairedMsg  The message to put into the line
  //  *
  //  * @return The string
  //  */
  // std::string writeAlphanumeric(const std::string &impairedMsg) const;

protected:

  std::string _name;     /**< runway name */
  Alert_t _type;         /**< Alert type */
  Runway_t _runwayType;  /**< Runway type */
  int _magnitude;        /**< Event magnitude, loss or gain only */
  int _location;         /**< 0, 1, 2, .. */

  std::string _impairedMsg;  /**< Message when _type=IMPAIRED, all other
			      * state variable ignored in this case */


private:

  bool _wellFormed;      /**< True if object is well formed */
};

#endif

