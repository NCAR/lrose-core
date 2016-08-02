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
/*----------------------------------------------------------------*/
/**
 * @file Handedness.hh
 * @brief handedness attribute
 * @class Handedness
 * @brief handedness attribute
 *
 * Handedness refers to the orientation of motion for a line segment when
 * the motion must be perpendicular to the line segment.
 *
 * This is either LEFT handed (to the 'left' when moving from the 0th to
 * the 1th endpoint), RIGHT handed (to the 'right' when moving from 0th to
 * 1th endpoints), NONE (motion is 0), or UNKNOWN (no motion)
 *
 * This class has the handedness value and several methods
 */

# ifndef    HANDEDNESS_H
# define    HANDEDNESS_H

#include <string>

class Handedness
{
public:

  typedef enum
  {
    UNKNOWN = -1,
    LEFT,  
    RIGHT, 
    NONE
  } e_hand_t;

  /**
   * Constructor to set UNKNOWN
   */
  Handedness(void);

  /**
   * Constructor sets type to input
   * @param[in] t
   */
  Handedness(e_hand_t t);

  /**
   * Destructor
   */
  virtual ~Handedness();

  /**
   * Operator== 
   * @param[in] l
   */
  bool operator==(const Handedness &l) const;

  /**
   * @return XML representation of state
   */
  std::string writeXml(void) const;

  /**
   * Parse an XML string to set state
   * @param[in] xml
   */
  bool readXml(const std::string &xml);

  /**
   * Set handedness to input type
   * @param[in] e
   */
  inline void setType(e_hand_t e) {_type = e;}

  /**
   * @return handedness type
   */
  inline e_hand_t getType(void) const {return _type;}

  /**
   * @return true if local handedness equals input
   * @param[in] htype
   */
  inline bool typesMatch(e_hand_t htype) const { return _type == htype;}

  /**
   * Replace the local handedness with input handedness, but only when
   * local is NONE 
   * @param[in] h
   */
  void average(const Handedness &h);

  /**
   * Debug print
   */
  void print(void) const;

  /**
   * Debug print
   * @param[in] fp
   */
  void print(FILE *fp) const;

  /**
   * Debug print
   */
  std::string sprint(void) const;

  /**
   * If local handedness is LEFT or RIGHT, reverse it
   */
  void reverseHandedness(void);

private:

  e_hand_t _type;    /**< Handedness type */
};


# endif
