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
 * @file WindshearAlphas.hh 
 * @brief WindshearAlphas information for all runways regarding runway alerts,
 *                        written to XML
 * @class WindshearAlphas
 * @brief WindshearAlphas information for all runways regarding runway alerts,
 *                        written to XML 
 *
 * This is the alphanumeric message data
 */

#ifndef WINDSHEAR_ALPHAS_H
#define WINDSHEAR_ALPHAS_H

#include <rapformats/WindshearAlpha.hh>
#include <string>
#include <vector>
#include <map>

//------------------------------------------------------------------
class WindshearAlphas
{
public:

  /**
   * Empty
   */
  WindshearAlphas(void);

  /**
   * Construct from inputs for a valid set of alphanumeric alerts
   *
   * @param[in] time  Time
   * @param[in] alphas  The individual alphanumeric alerts
   */
  WindshearAlphas(const time_t &time, const std::vector<WindshearAlpha> alphas);
		 
  // /**
  //  * Construct for impaired state
  //  *
  //  * @param[in] time  Time
  //  * @param[in] msg  Impaired message
  //  * @param[in] alphas  The individual alphanumeric alerts
  //  * @param[in] max_nchars  Maximum number of characters per line
  //  */
  // WindshearAlphas(const time_t &time, const std::string &msg,
  // 		  const std::vector<WindshearAlpha> alphas);
  /**
   * Construct from XML content
   * @param[in] s  String with XML in it, as from writeXml()
   *
   * If there are problems in input string, calls to
   * isWellFormed() will return false
   */
  WindshearAlphas(const std::string &s);

  /**
   * Destructor
   */
  virtual ~WindshearAlphas(void);

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
   * @return true if object is good
   */
  inline bool isWellFormed(void) const {return _wellFormed;}

  /**
   * Change time value to real time
   */
  void changeToRealTime(void);

  /**
   * Set internal color value for a particular alert type, to be used
   * for writing Alpha messages in the ARC (Advanced Radar Corporation)
   * format.  These colors are strings in hex RGB representation.
   *
   * @param[in] type  The alert type
   * @param[in] color The color in 0xRRGGBB notation.
   */
  void setColorForARCFormat(WindshearAlpha::Alert_t type,
			    const std::string &color);


  /**
   * Set internal number of display lines, to be used for writing Alpha
   * messages in the ARC (Advanced Radar Coporation) format.  If not set,
   * the default is to not deal with the number of display lines, and instead
   * to just output the message lines that have been specified.
   *
   * @param[in] numLines   Total number of lines
   */
  void setNumDisplayLinesForARCFormat(int numLines);

  /** 
   * Write internal state as alphanumeric lines, which could be used
   * for operational alphanumeric alerts
   *
   * @return The alert strings, one per line
   */
  std::vector<std::string> writeAlphanumeric(void) const;

  /** 
   * Write internal state as alphanumeric lines, which could be used
   * for operational alphanumeric alerts, in the ARC format.
   *
   * @param[in] ymd  True to output with yyyy/mm/dd as part of time string,
   *                 false to just give hh:mm:ss
   *
   * @return The alert strings, one per line
   *
   * @note this is not const because it accesses the map, will fix with C++11

   */
  std::vector<std::string> writeAlphanumericARC(bool ymd);

  /**
   * Return initial message for use in ARC format outputs, to clear display
   * @param[in] ymd  True to output with yyyy/mm/dd as part of time string,
   *                 false to just give hh:mm:ss
   *
   * @return The strings, one per line
   */
  std::vector<std::string> initialMessageARC(bool ymd);

  /**
   * @return time value
   */
  inline time_t getTime(void) const {return _time;}

  /**
   * return the worst alert, as a string.
   * @param[out] alert  The worst alert
   * @return true if there was any alerting at all
   */
  bool worstAlert(std::string &alert) const;

protected:

  time_t _time;  /**< Time */
  std::vector<WindshearAlpha> _alpha;  /**< Individual alphas */

  /**
   * Colors for each type of alert, used in ARC output
   */
  std::map<WindshearAlpha::Alert_t, std::string> _color;

  int _numDisplayLinesARC;  /**< Initialized to 0, can be set to the total
			     * number of lines in the ARC display */

private:

  bool _wellFormed;      /**< True if object is well formed */
};

#endif

