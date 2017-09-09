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

/************************************************************************
 * ClimoFileFinder: Base class for finding the correct climatology file
 *                  for the given data time.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2004
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef ClimoFileFinder_HH
#define ClimoFileFinder_HH

#include <string>
#include <vector>

#include <toolsa/DateTime.hh>

using namespace std;


class ClimoFileFinder
{
 public:

  /////////////////////////////
  // Constructors/destructor //
  /////////////////////////////

  /**********************************************************************
   * Constructor
   */

  ClimoFileFinder(const bool debug_flag = false);
  

  /**********************************************************************
   * Destructor
   */

  virtual ~ClimoFileFinder(void);
  

  /////////////////////
  // Utility methods //
  /////////////////////

  /**********************************************************************
   * calcClimoTime() - Determine the correct climo file time for storing
   *                   data for the given data time.
   */

  virtual DateTime calcClimoTime(const DateTime &data_time) const = 0;
  

  /**********************************************************************
   * calcBeginTime() - Determine the correct begin time for the climo file
   *                   storing data for the given data time.
   */

  virtual DateTime calcBeginTime(const DateTime &data_time) const = 0;
  

  /**********************************************************************
   * calcEndTime() - Determine the correct end time for the climo file
   *                 storing data for the given data time.
   */

  virtual DateTime calcEndTime(const DateTime &data_time) const = 0;
  

  /**********************************************************************
   * calcDataTime() - Determine the correct data file time for storing
   *                  data for the given search time.
   */

  virtual DateTime calcDataTime(const DateTime &search_time) const = 0;
  

  /**********************************************************************
   * calcDataBeginTime() - Determine the correct data begin time for the
   *                       climo file for the given search time.
   */

  virtual DateTime calcDataBeginTime(const DateTime &search_time) const = 0;
  

  /**********************************************************************
   * calcDataEndTime() - Determine the correct data end time for the climo
   *                     file for the given search time.
   */

  virtual DateTime calcDataEndTime(const DateTime &search_time) const = 0;
  

  /**********************************************************************
   * calcTimeList() - Create a list of climo times between the given
   *                  begin and end times.
   */

  virtual vector< DateTime > calcTimeList(const DateTime &begin_time,
					  const DateTime &end_time,
					  const string &climo_dir) const = 0;
  

protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;


  ///////////////////////
  // Protected methods //
  ///////////////////////

};


#endif
