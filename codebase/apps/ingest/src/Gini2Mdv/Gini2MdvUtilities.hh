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
#ifndef _Gini2MdvUtilities_
#define _Gini2MdvUtilities_

// C++ Standard Include Files
#include <string>
#include <vector>
#include <math.h>

// RAP Include Files
#include <toolsa/DateTime.hh>

// No Local Include Files

using namespace std;
namespace gini2MdvUtilities
   {
   const int NBR_SECS_IN_DAY = 86400;

   string formattedStrFromTimeT(time_t timeStruct);
   string int2PaddedStr(int number, int padLength);
   string yyyymmddFromUnixTime(time_t unixTime);
   time_t unixTimeFromDateTimeStr(string dateTimeStr);
   string str2UpperCase(string myString);
   vector<string> getDateStrsFromUnixTimes(time_t minTimeUnix, time_t maxTimeUnix);
   }

#endif // _Gini2MdvUtilities_
