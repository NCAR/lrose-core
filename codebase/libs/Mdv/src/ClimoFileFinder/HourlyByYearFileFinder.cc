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
 * HourlyByYearFileFinder: Class for finding the correct climatology file
 *                         for the given data time for hourly by year
 *                         climatologies.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <algorithm>

#include <Mdv/MdvxTimeList.hh>
#include <Mdv/climo/HourlyByYearFileFinder.hh>

using namespace std;


/**********************************************************************
 * Constructor
 */

HourlyByYearFileFinder::HourlyByYearFileFinder(const bool debug_flag) :
  ClimoFileFinder(debug_flag)
{
}


/**********************************************************************
 * Destructor
 */

HourlyByYearFileFinder::~HourlyByYearFileFinder(void)
{
}
  

/**********************************************************************
 * init() - Initialization method.  Must be called before calling any
 *          other method or undefined results could occur.
 */

bool HourlyByYearFileFinder::init(void)
{
  return true;
}
  

/**********************************************************************
 * calcBeginTime() - Determine the correct begin time for the climo file
 *                   storing data for the given data time.
 */

DateTime HourlyByYearFileFinder::calcBeginTime(const DateTime &data_time) const
{
  DateTime begin_time;
  
  begin_time.setYear(data_time.getYear());
  begin_time.setMonth(data_time.getMonth());
  begin_time.setDay(data_time.getDay());
  begin_time.setHour(data_time.getHour());
  begin_time.setMin(0);
  begin_time.setSec(0);
  
  return begin_time;
}
  

/**********************************************************************
 * calcClimoTime() - Determine the correct climo file time for storing
 *                   data for the given data time.
 */

DateTime HourlyByYearFileFinder::calcClimoTime(const DateTime &data_time) const
{
  DateTime climo_time;
  
  climo_time.setYear(data_time.getYear());
  climo_time.setMonth(data_time.getMonth());
  climo_time.setDay(data_time.getDay());
  climo_time.setHour(data_time.getHour());
  climo_time.setMin(30);
  climo_time.setSec(0);
  
  return climo_time;
}


/**********************************************************************
 * calcEndTime() - Determine the correct end time for the climo file
 *                 storing data for the given data time.
 */

DateTime HourlyByYearFileFinder::calcEndTime(const DateTime &data_time) const
{
  DateTime end_time;
  
  end_time.setYear(data_time.getYear());
  end_time.setMonth(data_time.getMonth());
  end_time.setDay(data_time.getDay());
  end_time.setHour(data_time.getHour());
  end_time.setMin(59);
  end_time.setSec(59);
  
  return end_time;
}


/**********************************************************************
 * calcDataTime() - Determine the correct data file time for storing
 *                  data for the given search time.
 */

DateTime HourlyByYearFileFinder::calcDataTime(const DateTime &search_time) const
{
  DateTime data_time(search_time);
  
  data_time.setMin(30);
  data_time.setSec(0);
  
  return data_time;
}
  

/**********************************************************************
 * calcDataBeginTime() - Determine the correct data begin time for the
 *                       climo file for the given search time.
 */

DateTime HourlyByYearFileFinder::calcDataBeginTime(const DateTime &search_time) const
{
  DateTime begin_time(search_time);
  
  begin_time.setMin(0);
  begin_time.setSec(0);
  
  return begin_time;
}
  

/**********************************************************************
 * calcDataEndTime() - Determine the correct data end time for the climo
 *                     file for the given search time.
 */

DateTime HourlyByYearFileFinder::calcDataEndTime(const DateTime &search_time) const
{
  DateTime end_time(search_time);
  
  end_time.setMin(59);
  end_time.setSec(59);
  
  return end_time;
}
  

/**********************************************************************
 * calcTimeList() - Create a list of climo times between the given
 *                  begin and end times.
 */

vector< DateTime > HourlyByYearFileFinder::calcTimeList(const DateTime &begin_time,
						  const DateTime &end_time,
						  const string &climo_dir) const
{
  // Create the return object

  vector< DateTime > time_list;
  
  // Get the list of actual climo files that currently exist

  MdvxTimeList mdv_time_list;
  mdv_time_list.setModeValid(climo_dir,
			     begin_time.utime(), end_time.utime());
  if (mdv_time_list.compile() != 0)
    return time_list;
  
  const vector< time_t > climo_time_list = mdv_time_list.getValidTimes();
  
  // Convert the time_t objects returned by the MdvxTimeList object to
  // the DateTime objects we want.

  vector< time_t >::const_iterator actual_time;
  for (actual_time = climo_time_list.begin();
       actual_time != climo_time_list.end(); ++actual_time)
    time_list.push_back(DateTime(*actual_time));
  
  return time_list;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
