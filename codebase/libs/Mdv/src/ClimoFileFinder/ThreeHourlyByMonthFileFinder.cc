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
 * ThreeHourlyByMonthFileFinder: Class for finding the correct climatology
 *                               file for the given data time for 3-hourly
 *                               by month climatologies.
 *
 * RAP, NCAR, Boulder CO
 *
 * Oct 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <algorithm>

#include <Mdv/MdvxTimeList.hh>
#include <Mdv/climo/ThreeHourlyByMonthFileFinder.hh>

using namespace std;


const int ThreeHourlyByMonthFileFinder::CLIMO_YEAR = 2003;
const int ThreeHourlyByMonthFileFinder::CLIMO_DAY = 15;


/**********************************************************************
 * Constructor
 */

ThreeHourlyByMonthFileFinder::ThreeHourlyByMonthFileFinder(const bool debug_flag) :
  ClimoFileFinder(debug_flag)
{
}


/**********************************************************************
 * Destructor
 */

ThreeHourlyByMonthFileFinder::~ThreeHourlyByMonthFileFinder(void)
{
}
  

/**********************************************************************
 * init() - Initialization method.  Must be called before calling any
 *          other method or undefined results could occur.
 */

bool ThreeHourlyByMonthFileFinder::init(void)
{
  return true;
}
  

/**********************************************************************
 * calcBeginTime() - Determine the correct begin time for the climo file
 *                   storing data for the given data time.
 */

DateTime ThreeHourlyByMonthFileFinder::calcBeginTime(const DateTime &data_time) const
{
  DateTime begin_time;
  
  begin_time.setYear(CLIMO_YEAR);
  begin_time.setMonth(data_time.getMonth());
  begin_time.setDay(CLIMO_DAY);

  int data_hour = data_time.getHour();
  if (data_hour >= 0 && data_hour < 3)
    begin_time.setHour(0);
  else if (data_hour >= 3 && data_hour < 6)
    begin_time.setHour(3);
  else if (data_hour >= 6 && data_hour < 9)
    begin_time.setHour(6);
  else if (data_hour >= 9 && data_hour < 12)
    begin_time.setHour(9);
  else if (data_hour >= 12 && data_hour < 15)
    begin_time.setHour(12);
  else if (data_hour >= 15 && data_hour < 18)
    begin_time.setHour(15);
  else if (data_hour >= 18 && data_hour < 21)
    begin_time.setHour(18);
  else
    begin_time.setHour(21);

  begin_time.setMin(0);
  begin_time.setSec(0);
  
  return begin_time;
}
  

/**********************************************************************
 * calcClimoTime() - Determine the correct climo file time for storing
 *                   data for the given data time.
 */

DateTime ThreeHourlyByMonthFileFinder::calcClimoTime(const DateTime &data_time) const
{
  DateTime climo_time;
  
  climo_time.setYear(CLIMO_YEAR);
  climo_time.setMonth(data_time.getMonth());
  climo_time.setDay(CLIMO_DAY);

  int data_hour = data_time.getHour();
  if (data_hour >= 0 && data_hour < 3)
    climo_time.setHour(1);
  else if (data_hour >= 3 && data_hour < 6)
    climo_time.setHour(4);
  else if (data_hour >= 6 && data_hour < 9)
    climo_time.setHour(7);
  else if (data_hour >= 9 && data_hour < 12)
    climo_time.setHour(10);
  else if (data_hour >= 12 && data_hour < 15)
    climo_time.setHour(13);
  else if (data_hour >= 15 && data_hour < 18)
    climo_time.setHour(16);
  else if (data_hour >= 18 && data_hour < 21)
    climo_time.setHour(19);
  else
    climo_time.setHour(22);

  climo_time.setMin(30);
  climo_time.setSec(0);
  
  return climo_time;
}


/**********************************************************************
 * calcEndTime() - Determine the correct end time for the climo file
 *                 storing data for the given data time.
 */

DateTime ThreeHourlyByMonthFileFinder::calcEndTime(const DateTime &data_time) const
{
  DateTime end_time;
  
  end_time.setYear(CLIMO_YEAR);
  end_time.setMonth(data_time.getMonth());
  end_time.setDay(CLIMO_DAY);

  int data_hour = data_time.getHour();
  if (data_hour >= 0 && data_hour < 3)
    end_time.setHour(2);
  else if (data_hour >= 3 && data_hour < 6)
    end_time.setHour(5);
  else if (data_hour >= 6 && data_hour < 9)
    end_time.setHour(8);
  else if (data_hour >= 9 && data_hour < 12)
    end_time.setHour(11);
  else if (data_hour >= 12 && data_hour < 15)
    end_time.setHour(14);
  else if (data_hour >= 15 && data_hour < 18)
    end_time.setHour(17);
  else if (data_hour >= 18 && data_hour < 21)
    end_time.setHour(18);
  else
    end_time.setHour(23);

  end_time.setMin(59);
  end_time.setSec(59);
  
  return end_time;
}


/**********************************************************************
 * calcDataTime() - Determine the correct data file time for storing
 *                  data for the given search time.
 */

DateTime ThreeHourlyByMonthFileFinder::calcDataTime(const DateTime &search_time) const
{
  DateTime data_time(search_time);
  
  data_time.setDay(CLIMO_DAY);

  int search_hour = search_time.getHour();
  if (search_hour >= 0 && search_hour < 3)
    data_time.setHour(1);
  else if (search_hour >= 3 && search_hour < 6)
    data_time.setHour(4);
  else if (search_hour >= 6 && search_hour < 9)
    data_time.setHour(7);
  else if (search_hour >= 9 && search_hour < 12)
    data_time.setHour(10);
  else if (search_hour >= 12 && search_hour < 15)
    data_time.setHour(13);
  else if (search_hour >= 15 && search_hour < 18)
    data_time.setHour(16);
  else if (search_hour >= 18 && search_hour < 21)
    data_time.setHour(19);
  else
    data_time.setHour(22);

  data_time.setMin(30);
  data_time.setSec(0);
  
  return data_time;
}
  

/**********************************************************************
 * calcDataBeginTime() - Determine the correct data begin time for the
 *                       climo file for the given search time.
 */

DateTime ThreeHourlyByMonthFileFinder::calcDataBeginTime(const DateTime &search_time) const
{
  DateTime begin_time(search_time);
  
  begin_time.setDay(CLIMO_DAY);

  int search_hour = search_time.getHour();
  if (search_hour >= 0 && search_hour < 3)
    begin_time.setHour(0);
  else if (search_hour >= 3 && search_hour < 6)
    begin_time.setHour(3);
  else if (search_hour >= 6 && search_hour < 9)
    begin_time.setHour(6);
  else if (search_hour >= 9 && search_hour < 12)
    begin_time.setHour(9);
  else if (search_hour >= 12 && search_hour < 15)
    begin_time.setHour(12);
  else if (search_hour >= 15 && search_hour < 18)
    begin_time.setHour(15);
  else if (search_hour >= 18 && search_hour < 21)
    begin_time.setHour(18);
  else
    begin_time.setHour(21);

  begin_time.setMin(0);
  begin_time.setSec(0);
  
  return begin_time;
}
  

/**********************************************************************
 * calcDataEndTime() - Determine the correct data end time for the climo
 *                     file for the given search time.
 */

DateTime ThreeHourlyByMonthFileFinder::calcDataEndTime(const DateTime &search_time) const
{
  DateTime end_time(search_time);
  
  end_time.setDay(CLIMO_DAY);

  int search_hour = search_time.getHour();
  if (search_hour >= 0 && search_hour < 3)
    end_time.setHour(2);
  else if (search_hour >= 3 && search_hour < 6)
    end_time.setHour(5);
  else if (search_hour >= 6 && search_hour < 9)
    end_time.setHour(8);
  else if (search_hour >= 9 && search_hour < 12)
    end_time.setHour(11);
  else if (search_hour >= 12 && search_hour < 15)
    end_time.setHour(14);
  else if (search_hour >= 15 && search_hour < 18)
    end_time.setHour(17);
  else if (search_hour >= 18 && search_hour < 21)
    end_time.setHour(18);
  else
    end_time.setHour(23);

  end_time.setMin(59);
  end_time.setSec(59);
  
  return end_time;
}
  

/**********************************************************************
 * calcTimeList() - Create a list of climo times between the given
 *                  begin and end times.
 */

vector< DateTime > ThreeHourlyByMonthFileFinder::calcTimeList(const DateTime &begin_time,
							      const DateTime &end_time,
							      const string &climo_dir) const
{
  // Create the return object

  vector< DateTime > time_list;
  
  MdvxTimeList mdv_time_list;
  
  // Get the list of actual climo files that currently exist

  DateTime climo_start_time;
  climo_start_time.setYear(CLIMO_YEAR);
  climo_start_time.setMonth(1);
  climo_start_time.setDay(1);
  climo_start_time.setHour(0);
  climo_start_time.setMin(0);
  climo_start_time.setSec(0);

  DateTime climo_end_time;
  climo_end_time.setYear(CLIMO_YEAR);
  climo_end_time.setMonth(12);
  climo_end_time.setDay(31);
  climo_end_time.setHour(23);
  climo_end_time.setMin(59);
  climo_end_time.setSec(59);
  
  mdv_time_list.clearMode();
  mdv_time_list.setModeValid(climo_dir,
			     climo_start_time.utime(), climo_end_time.utime());
  if (mdv_time_list.compile() != 0)
    return time_list;
  
  const vector< time_t > climo_time_list = mdv_time_list.getValidTimes();

  // Loop through the expected data times, keeping the ones for which
  // we find a climo file

  DateTime test_time(begin_time);
  int hour_increment = 3 * 60 * 60;
  
  test_time.setHour(1);
  test_time.setMin(30);
  test_time.setSec(0);
  
  while (test_time.utime() < end_time.utime())
  {
    if (test_time.utime() > begin_time.utime())
    {
      DateTime climo_test_time = calcClimoTime(test_time);
      
      if (find(climo_time_list.begin(), climo_time_list.end(),
	       climo_test_time.utime()) != climo_time_list.end())
	time_list.push_back(test_time);
    }
    
    test_time += hour_increment;
  }
  
  return time_list;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
