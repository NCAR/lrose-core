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
/*********************************************************************
 * ExternalDiurnalFileFinder: Class for finding the correct climatology file
 *                            for the given data time for diurnal climatologies
 *                            generated without using the utility methods
 *                            in this library.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <algorithm>

#include <Mdv/MdvxTimeList.hh>
#include <Mdv/climo/ExternalDiurnalFileFinder.hh>

using namespace std;


/**********************************************************************
 * Constructor
 */

ExternalDiurnalFileFinder::ExternalDiurnalFileFinder(const bool debug_flag) :
  ClimoFileFinder(debug_flag),
  _climoDir("")
{
}


/**********************************************************************
 * Destructor
 */

ExternalDiurnalFileFinder::~ExternalDiurnalFileFinder(void)
{
}
  

/**********************************************************************
 * init() - Initialization method.  Must be called before calling any
 *          other method or undefined results could occur.
 */

bool ExternalDiurnalFileFinder::init(const string &climo_dir)
{
  // Save the climo URL

  _climoDir = climo_dir;
  
  // Get the list of data times so we can use these for generating
  // the begin, end and climo times.

  MdvxTimeList mdv_time_list;
  
  mdv_time_list.setModeFirst(_climoDir);
  if (mdv_time_list.compile() != 0)
    return false;
  DateTime start_time = mdv_time_list.getValidTimes()[0];
  
  mdv_time_list.clearMode();
  mdv_time_list.setModeLast(_climoDir);
  if (mdv_time_list.compile() != 0)
    return false;
  DateTime end_time = mdv_time_list.getValidTimes()[0];

  // Now get all of the valid times for the data

  mdv_time_list.clearMode();
  mdv_time_list.setModeValid(_climoDir, start_time.utime(),
			     end_time.utime());
  if (mdv_time_list.compile() != 0)
    return false;
  const vector< time_t > time_list = mdv_time_list.getValidTimes();
  vector< time_t >::const_iterator utime;
  
  for (utime = time_list.begin(); utime != time_list.end(); ++utime)
    _dataTimeList.push_back(DateTime(*utime));
  
  return true;
}
  

/**********************************************************************
 * calcBeginTime() - Determine the correct begin time for the climo file
 *                   storing data for the given data time.
 */

DateTime ExternalDiurnalFileFinder::calcBeginTime(const DateTime &data_time) const
{
  DateTime best_time = calcClimoTime(data_time);
  return best_time - 1800;
}
  

/**********************************************************************
 * calcClimoTime() - Determine the correct climo file time for storing
 *                   data for the given data time.
 */

DateTime ExternalDiurnalFileFinder::calcClimoTime(const DateTime &data_time) const
{
  int data_hour = _calcTime(data_time);
  
  vector< DateTime >::const_iterator time_iter = _dataTimeList.begin();
  
  int curr_time_hour = _calcTime(*time_iter);
  int time_diff = abs(data_hour - curr_time_hour);
  DateTime best_time = *time_iter;
  
  for (++time_iter; time_iter != _dataTimeList.end(); ++time_iter)
  {
    curr_time_hour = _calcTime(*time_iter);
    int curr_time_diff = abs(data_hour - curr_time_hour);
    
    if (curr_time_diff < time_diff)
    {
      time_diff = curr_time_diff;
      best_time = *time_iter;
    }
    
  } /* endfor - time_iter */
  
  return best_time;
}


/**********************************************************************
 * calcEndTime() - Determine the correct end time for the climo file
 *                 storing data for the given data time.
 */

DateTime ExternalDiurnalFileFinder::calcEndTime(const DateTime &data_time) const
{
  DateTime best_time = calcClimoTime(data_time);
  return best_time + 1800;
}


/**********************************************************************
 * calcDataTime() - Determine the correct data file time for storing
 *                  data for the given search time.
 */

DateTime ExternalDiurnalFileFinder::calcDataTime(const DateTime &search_time) const
{
  return calcClimoTime(search_time);
}
  

/**********************************************************************
 * calcDataBeginTime() - Determine the correct data begin time for the
 *                       climo file for the given search time.
 */

DateTime ExternalDiurnalFileFinder::calcDataBeginTime(const DateTime &search_time) const
{
  DateTime best_time = calcClimoTime(search_time);
  best_time.setYear(search_time.getYear());
  best_time.setMonth(search_time.getMonth());
  best_time.setDay(search_time.getDay());
  
  return best_time - 1800;
}
  

/**********************************************************************
 * calcDataEndTime() - Determine the correct data end time for the climo
 *                     file for the given search time.
 */

DateTime ExternalDiurnalFileFinder::calcDataEndTime(const DateTime &search_time) const
{
  DateTime best_time = calcClimoTime(search_time);
  best_time.setYear(search_time.getYear());
  best_time.setMonth(search_time.getMonth());
  best_time.setDay(search_time.getDay());
  
  return best_time + 1800;
}
  

/**********************************************************************
 * calcTimeList() - Create a list of climo times between the given
 *                  begin and end times.
 */

vector< DateTime > ExternalDiurnalFileFinder::calcTimeList(const DateTime &begin_time,
							   const DateTime &end_time,
							   const string &climo_dir) const
{
  vector< DateTime > return_list;
  
  DateTime current_time(begin_time);
  current_time.setTime(0, 0, 0);
  
  // Process each day in the given time interval

  while (current_time < end_time)
  {
    DateTime test_time(current_time);
    
    vector< DateTime >::const_iterator time_iter;
    
    for (time_iter = _dataTimeList.begin(); time_iter != _dataTimeList.end();
	 ++time_iter)
    {
      test_time.setHour(time_iter->getHour());
      test_time.setMin(time_iter->getMin());
      test_time.setSec(time_iter->getSec());
      
      if (test_time >= begin_time && test_time <= end_time)
	return_list.push_back(test_time);
    }
    
    // Increment the day

    current_time += 86400;
  } /* endwhile */
  
  return return_list;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
