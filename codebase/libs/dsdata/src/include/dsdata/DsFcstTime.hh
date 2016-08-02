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
 * @file DsFcstTime.hh 
 * @brief  very simple class with a gen time and a lead time
 *        
 * @class DsFcstTime
 * @brief very simple class with a gen time and a lead time
 */

#ifndef DS_FCSTTIME_H
#define DS_FCSTTIME_H

#include <string>

class DsFcstTime
{
public:

  /**
   * Constructor
   */
  DsFcstTime(void);

  /**
   * Constructor with time and lead time
   * @param[in] t
   * @param[in] lt
   */
  DsFcstTime(const time_t &t, const int lt);

  /**
   * Destructor
   */
  ~DsFcstTime(void);

  /**
   * Print out the values
   */
  void print(void) const;

  /**
   * @return string showing values
   */
  std::string sprint(void) const;

  /**
   * @return true if values not both the same
   * @param[in] f  Comparison object
   */
  bool operator!=(const DsFcstTime f) const;

  /**
   * @return true if f0 <= f1
   * @param[in] f0
   * @param[in] f1
   */
  static bool lessOrEqual(const DsFcstTime f0, const DsFcstTime f1);
  
  time_t _gt;    /**< Gen time */
  int _lt;       /**< Lead seconds */

protected:
private:

};

#endif
