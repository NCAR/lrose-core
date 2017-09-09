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
 * ThreeHourlyDiurnalFileFinder: Class for finding the correct climatology
 *                               file for the given data time for 3-hourly
 *                               diurnal climatologies.
 *
 * RAP, NCAR, Boulder CO
 *
 * Oct 2004
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef ThreeHourlyDiurnalFileFinder_HH
#define ThreeHourlyDiurnalFileFinder_HH

#include <Mdv/climo/ClimoFileFinder.hh>

using namespace std;


class ThreeHourlyDiurnalFileFinder : public ClimoFileFinder
{
 public:

  /////////////////////////////
  // Constructors/destructor //
  /////////////////////////////

  /**********************************************************************
   * Constructor
   */

  ThreeHourlyDiurnalFileFinder(const bool debug_flag = false);
  

  /**********************************************************************
   * Destructor
   */

  virtual ~ThreeHourlyDiurnalFileFinder(void);
  

  /**********************************************************************
   * init() - Initialization method.  Must be called before calling any
   *          other method or undefined results could occur.
   */

  bool init();
  

  /////////////////////
  // Utility methods //
  /////////////////////

  /**********************************************************************
   * calcClimoTime() - Determine the correct climo file time for storing
   *                   data for the given data time.
   */

  virtual DateTime calcClimoTime(const DateTime &data_time) const;


  /**********************************************************************
   * calcBeginTime() - Determine the correct begin time for the climo file
   *                   storing data for the given data time.
   */

  virtual DateTime calcBeginTime(const DateTime &data_time) const;
  

  /**********************************************************************
   * calcEndTime() - Determine the correct end time for the climo file
   *                 storing data for the given data time.
   */

  virtual DateTime calcEndTime(const DateTime &data_time) const;
  

  /**********************************************************************
   * calcDataTime() - Determine the correct data file time for storing
   *                  data for the given search time.
   */

  virtual DateTime calcDataTime(const DateTime &search_time) const;
  

  /**********************************************************************
   * calcDataBeginTime() - Determine the correct data begin time for the
   *                       climo file for the given search time.
   */

  virtual DateTime calcDataBeginTime(const DateTime &search_time) const;
  

  /**********************************************************************
   * calcDataEndTime() - Determine the correct data end time for the climo
   *                     file for the given search time.
   */

  virtual DateTime calcDataEndTime(const DateTime &search_time) const;
  

  /**********************************************************************
   * calcTimeList() - Create a list of climo times between the given
   *                  begin and end times.
   */

  virtual vector< DateTime > calcTimeList(const DateTime &begin_time,
					  const DateTime &end_time,
					  const string &climo_dir) const;
  

protected:

  /////////////////////////
  // Protected Constants //
  /////////////////////////

  static const int CLIMO_YEAR;
  static const int CLIMO_MONTH;
  static const int CLIMO_DAY;
  
};


#endif
