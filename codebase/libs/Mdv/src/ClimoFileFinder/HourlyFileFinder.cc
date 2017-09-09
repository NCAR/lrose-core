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
 * HourlyFileFinder: Class for finding the correct climatology file
 *                   for the given data time for hourly climatologies.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <algorithm>

#include <Mdv/MdvxTimeList.hh>
#include <Mdv/climo/HourlyFileFinder.hh>

using namespace std;


const int HourlyFileFinder::CLIMO_YEAR = 2003;


/**********************************************************************
 * Constructor
 */

HourlyFileFinder::HourlyFileFinder(const bool debug_flag) :
  ClimoFileFinder(debug_flag)
{
}


/**********************************************************************
 * Destructor
 */

HourlyFileFinder::~HourlyFileFinder(void)
{
}
  

/**********************************************************************
 * init() - Initialization method.  Must be called before calling any
 *          other method or undefined results could occur.
 */

bool HourlyFileFinder::init(void)
{
  return true;
}
  

/**********************************************************************
 * calcBeginTime() - Determine the correct begin time for the climo file
 *                   storing data for the given data time.
 */

DateTime HourlyFileFinder::calcBeginTime(const DateTime &data_time) const
{
  DateTime begin_time;
  
  begin_time.setYear(CLIMO_YEAR);
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

DateTime HourlyFileFinder::calcClimoTime(const DateTime &data_time) const
{
  DateTime climo_time;
  
  climo_time.setYear(CLIMO_YEAR);
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

DateTime HourlyFileFinder::calcEndTime(const DateTime &data_time) const
{
  DateTime end_time;
  
  end_time.setYear(CLIMO_YEAR);
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

DateTime HourlyFileFinder::calcDataTime(const DateTime &search_time) const
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

DateTime HourlyFileFinder::calcDataBeginTime(const DateTime &search_time) const
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

DateTime HourlyFileFinder::calcDataEndTime(const DateTime &search_time) const
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

vector< DateTime > HourlyFileFinder::calcTimeList(const DateTime &begin_time,
						  const DateTime &end_time,
						  const string &climo_dir) const
{
  // Create the return object

  vector< DateTime > time_list;
  
  MdvxTimeList mdv_time_list;
  
  // Get the list of actual climo files that currently exist

  DateTime climo_start_time("2003 1 1 0 0 0");
  DateTime climo_end_time("2003 12 31 23 59 59");
  
  mdv_time_list.clearMode();
  mdv_time_list.setModeValid(climo_dir,
			     climo_start_time.utime(), climo_end_time.utime());
  if (mdv_time_list.compile() != 0)
    return time_list;
  
  const vector< time_t > climo_time_list = mdv_time_list.getValidTimes();

  // Loop through the expected data times, keeping the ones for which
  // we find a climo file

  DateTime test_time(begin_time);
  int time_increment = 60 * 60;
  
  test_time.setHour(0);
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
    
    test_time += time_increment;
  }
  
  return time_list;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
