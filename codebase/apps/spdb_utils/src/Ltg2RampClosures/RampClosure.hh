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
 * @file RampClosure.hh
 * @brief RampClosure
 * @class RampClosure
 * @brief RampClosure 
 * @note
 * @todo
 */

#ifndef RAMP_CLOSURE_HH
#define RAMP_CLOSURE_HH

#include <ctime>

class RampClosure {

public:

  /**
   *  Constructor
   */
  RampClosure(const time_t begin, const time_t end); 

  /**
   *  Constructor
   */
  RampClosure(const time_t begin);

  /**
   *  Destructor
   */
  ~RampClosure();
 
  /**
   * Get start of time interval
   */
  int getStart() const {return pStart;} 
  
  /**
   * Get end of time interval
   */
  int getEnd() const  {return pEnd;}

  /**
   * Get length of time interval
   */
  time_t getDuration() const { return pDuration; }

  bool intervalIsClosed() const; 

  /**
   * Constant for missing time. This may be useful in realtime when there is 
   * and end time to an interval. 
   */
  static const int timeMissing;

protected:
   
private:

  /**
   * Start of time interval
   */
  time_t pStart;

  /**
   * End of interval, could be set to missing if data not available
   */
  time_t pEnd;

  /**
   * Duration of time interval, could be set to missing if end time not
   * available
   */
  int pDuration;

 
};

#endif
