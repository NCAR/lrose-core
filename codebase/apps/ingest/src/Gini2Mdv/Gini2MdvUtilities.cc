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

#include <cstdlib>

// RAP Include Files
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>

// Local Include Files
#include "Gini2MdvUtilities.hh"

using namespace std;

namespace gini2MdvUtilities
   {
   ////////////////////////////////////////////////////////////////////////////////
   //                                                                            //
   // Method:                                                                    //
   //  Gini2MdvUtilities::formattedStrFromTimeT()                                //
   //                                                                            //
   // Description:                                                               //
   //  Returns a formatted string corresponding to a time_t structure.           //
   //                                                                            //
   // Input:                                                                     //
   //  time_t structure representing a time.                                     //
   //                                                                            //
   // Output:                                                                    //
   //  String representing formatted date-time (e.g., '2006-08-29 12:15:00').    //
   //                                                                            //
   ////////////////////////////////////////////////////////////////////////////////
   string formattedStrFromTimeT(time_t timeStruct)
      {
      const string methodName = "Gini2MdvUtilities::formattedStrFromTimeT()";

      string formattedDateTimeStr;

      tm timeDateProcessedStruct;
      timeDateProcessedStruct = *gmtime(&timeStruct);

      int yearInt  = 1900 + timeDateProcessedStruct.tm_year;
      int monthInt = 1 + timeDateProcessedStruct.tm_mon;
      int dayInt   = timeDateProcessedStruct.tm_mday;
      int hourInt  = timeDateProcessedStruct.tm_hour;
      int minInt   = timeDateProcessedStruct.tm_min;
      int secInt   = timeDateProcessedStruct.tm_sec;

      char yearCharArray[10];
      sprintf(yearCharArray, "%d", yearInt);
      string yearStr  = (string) yearCharArray;
      string monthStr = int2PaddedStr(monthInt, 2);
      string dayStr   = int2PaddedStr(dayInt, 2);
      string hourStr  = int2PaddedStr(hourInt, 2);
      string minStr   = int2PaddedStr(minInt, 2);
      string secStr   = int2PaddedStr(secInt, 2);

      formattedDateTimeStr = yearStr + "-" + monthStr + "-" + dayStr + " " + hourStr + ":" + minStr + ":" + secStr;
      return formattedDateTimeStr;
      } // End of formattedStrFromTimeT() method.


   ////////////////////////////////////////////////////////////////////////////////
   //                                                                            //
   // Method:                                                                    //
   //  Gini2MdvUtilities::int2PaddedStr()                                        //
   //                                                                            //
   // Description:                                                               //
   //  Converts an integer to a padded string.                                   //
   //                                                                            //
   // Input:                                                                     //
   //  number - Integer input.                                                   //
   //  padLength - Integer representing number of characters in output string.   //
   //                                                                            //
   // Output:                                                                    //
   //  String representing padded number (e.g., '01', '004', '014', etc.).       //
   //                                                                            //
   ////////////////////////////////////////////////////////////////////////////////
   string int2PaddedStr(int number, int padLength)
      {
      const string methodName = "Gini2MdvUtilities::int2PaddedStr()";

      if(number < 0)
         {
         cerr << "ERROR: " << methodName << endl;
         cerr << "   Not designed to process negative numbers." << endl;
         cerr << "   EXITING" << endl;
         exit(-1);
         }

      TaArray<char> numberCharArray_;
      char *numberCharArray = numberCharArray_.alloc(padLength+1);
      sprintf(numberCharArray, "%d", number);
      string numberStr = (string) numberCharArray;

      for(int padIndex=1; padIndex < padLength; padIndex++)
         {
         float padLimit = pow((double) 10, (double) padIndex);
         if(number < padLimit)
            {
            numberStr = '0' + numberStr;
            }
         }

      return numberStr;
      } // End of int2PaddedStr() method.


   ////////////////////////////////////////////////////////////////////////////////
   //                                                                            //
   // Method:                                                                    //
   //  Gini2MdvUtilities::yyyymmddFromUnixTime()                                 //
   //                                                                            //
   // Description:                                                               //
   //  Converts a UNIX time to a YYYYMMDD string.                                //
   //                                                                            //
   // Input:                                                                     //
   //  unixTime - time_t input value.                                            //
   //                                                                            //
   // Output:                                                                    //
   //  String representing date in YYYYMMDD format (e.g., '20060907').           //
   //                                                                            //
   ////////////////////////////////////////////////////////////////////////////////
   string yyyymmddFromUnixTime(time_t unixTime)
      {
      const string methodName = "Gini2MdvUtilities::yyyymmddFromUnixTime()";

      string yyyymmdd;

      tm unixTimeStruct;
      unixTimeStruct = *gmtime(&unixTime);

      // Extract year, month and day in int format.
      int yearInt  = 1900 + unixTimeStruct.tm_year;
      int monthInt = 1 + unixTimeStruct.tm_mon;
      int dayInt   = unixTimeStruct.tm_mday;

      // Convert year to string.
      char yearCharArray[10];
      sprintf(yearCharArray, "%d", yearInt);
      string yearStr  = (string) yearCharArray;

      // Convert month and day to strings (may need to pad one or both).
      string monthStr = int2PaddedStr(monthInt, 2);
      string dayStr   = int2PaddedStr(dayInt, 2);

      // Construct complete date string.
      yyyymmdd = yearStr + monthStr + dayStr;

      return yyyymmdd;
      } // End of yyyymmddFromUnixTime() method.


   ////////////////////////////////////////////////////////////////////////////////
   //                                                                            //
   // Method:                                                                    //
   //  Gini2MdvUtilities::unixTimeFromDateTimeStr()                              //
   //                                                                            //
   // Description:                                                               //
   //  Converts a YYYYMMDDHHMMSS string to a UNIX time.                          //
   //                                                                            //
   // Input:                                                                     //
   //  dateTimeStr - string representing date and time                           //
   //                                                                            //
   // Output:                                                                    //
   //  On success, UNIX time (time_t format) associated with dateTimeStr date    //
   //   and time. On failure, returns -1.                                        //
   //                                                                            //
   ////////////////////////////////////////////////////////////////////////////////
   time_t unixTimeFromDateTimeStr(string dateTimeStr)
      {
      const string methodName = "Gini2MdvUtilities::unixTimeFromDateTimeStr()";

      time_t returnTimeUnix;

      DateTime *dateTime = new DateTime(dateTimeStr);
      if(0 == dateTime)
         {
         cerr << "ERROR: " << methodName <<endl;
         cerr << "   Could not instantiate DateTime object" << endl;
	 return -1;
         }
      returnTimeUnix = dateTime->utime();

      delete dateTime;

      return returnTimeUnix;
      } // End of unixTimeFromDateTimeStr() method.


   ////////////////////////////////////////////////////////////////////////////////
   //                                                                            //
   // Method:                                                                    //
   //  Gini2MdvUtilities::str2UpperCase()                                        //
   //                                                                            //
   // Description:                                                               //
   //  Converts a string to upper case.                                          //
   //                                                                            //
   // Input:                                                                     //
   //  inputStr - input string.                                                  //
   //                                                                            //
   // Output:                                                                    //
   //  Upper-case version of input string.                                       //
   //                                                                            //
   ////////////////////////////////////////////////////////////////////////////////
   string str2UpperCase(string inputStr)
      {
      const string methodName = "Gini2MdvUtilities::str2UpperCase()";

      for(size_t strIndex=0; strIndex < inputStr.length(); strIndex++)
         {
         inputStr[strIndex] = toupper(inputStr[strIndex]);
         }

      return inputStr;
      } // End of str2UpperCase() method.


   ////////////////////////////////////////////////////////////////////////////////
   //                                                                            //
   // Method:                                                                    //
   //  Gini2MdvUtilities::getDateStrsFromUnixTimes()                             //
   //                                                                            //
   // Description:                                                               //
   //  Public method that returns a vector of YYYYMMDD strings corresponding to  //
   //  a range of input UNIX times.                                              //
   //                                                                            //
   // Input:                                                                     //
   //  minTimeUnix - time_t representation of the lower bound UNIX time.         //
   //  maxTimeUnix - time_t representation of the upper bound UNIX time.         //
   //                                                                            //
   // Output:                                                                    //
   //  Vector of YYYYMMDD strings (e.g., "20060912", "20060913", etc.).          //
   //                                                                            //
   ////////////////////////////////////////////////////////////////////////////////
   vector<string> getDateStrsFromUnixTimes(time_t minTimeUnix, time_t maxTimeUnix)
      {
      const string methodName = "Gini2MdvUtilities::getDateStrsFromUnixTimes()";

      string yyyymmdd;
      vector<string> yyyymmddStrVec;
      time_t unixTime = minTimeUnix;

      // Simple argument check.
      if(maxTimeUnix < minTimeUnix)
         {
         cerr << "ERROR: " << methodName << endl;
         cerr << "   Inconsistent input arguments - minTimeUnix exceeds maxTimeUnix." << endl;
         cerr << "   Returning date string for minTimeUnix only." << endl;
         yyyymmdd = gini2MdvUtilities::yyyymmddFromUnixTime(minTimeUnix);
         yyyymmddStrVec.push_back(yyyymmdd);
         return yyyymmddStrVec;
         }
      else
         {
         while(unixTime <= maxTimeUnix)
            {
            yyyymmdd = gini2MdvUtilities::yyyymmddFromUnixTime(unixTime);
            yyyymmddStrVec.push_back(yyyymmdd);
            unixTime = unixTime + gini2MdvUtilities::NBR_SECS_IN_DAY;
            }
         return yyyymmddStrVec;
         }
      } // End of getDateStrsFromUnixTimes() method.
   } // End of gini2MdvUtilities namespace.
