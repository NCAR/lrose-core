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
//
// This is a small class to parse the command line arguments
// to see if a start and end time has been specified. Typically
// a client will use the constructor and then access
// the class's public variables to get the start and end times.
//
// It will probably get used something like this :
//
///////////////////////////////////////////////////
//
// #include <toolsa/ArchiveDates.hh>
//
// int main(int argc, char*argv[]){
// 
//   ArchiveDates A(argc,argv);
// 
//   if ((!(A.timesAreValid)) && (InArchiveMode)){
//     cerr << "Failed to get archive times." << endl;
//     cerr << A.errorString << endl;
//     exit(-1);
//   }
// 
//   time_t myStartTime = A.startUnixTime;
//   time_t myEndTime   = A.endUnixTime;
//         .
//         .
//         .
//         .
//
//
// Niles Oien, October 2001
//
//
#ifndef _ARCHIVE_DATES_INC_
#define _ARCHIVE_DATES_INC_   
//
//
#include <string>         // For error and help strings
//
#include <toolsa/udatetime.h> // For date_time_t
#include <time.h>         // For time_t
using namespace std;
//
//
//
class ArchiveDates {
  //
public:
  //
  // String that attempts to state what, if anything, went wrong.
  //
  string errorString;
  //
  // String that contains help information. This will
  // be printed if the -helpdate option is specified (and
  // the program will then exit).
  //
  string helpString;
  //
  //
  //
  // Constructor - pass in command line args.
  //
  ArchiveDates(int argc,      // From command line 
	       char *argv[],  // From command line
	       int MaxYear = 2500,  // Max value for year to be considered valid
	       int MinYear = 1970); // Min value for year to be considered valid
  //
  // If MaxYear or MinYear is negative, no checks are made on the
  // year values that are read in.
  //
  //
  // Destructor - does nothing.
  //
  ~ArchiveDates();
  //
  //
  // Public data
  //
  //
  // timesAreValid is set to true only if the start and end times
  // are both valid.
  //
  bool timesAreValid;
  //
  // The actual start and end times.
  //
  date_time_t startTime, endTime;
  //
  // Unix times associated with start and end times.
  //
  time_t startUnixTime, endUnixTime;
  //
  //
  // Individual booleans for start or end times (client
  // may want to know which date is not parsable).
  //
  bool startTimeValid, endTimeValid;
  //
  //
  // The indicies of the arguments used to specify the
  // start and end times. These are returned so that
  // the client can parse other arguments. There will probably
  // be less than 8 of them - the unused ones are set to -1.
  //
  // For example, if a program is run with the command
  //
  // prompt> MyProg -fast -archive 20011023123000 20011023200000 -params My.params
  // arg # :   0      1      2          3              4            5       6
  //
  // Then     argsUsedForDate will be { 2, 3, 4, -1, -1, -1, -1, -1}. This
  // will probably not get used that much.
  //
  static const int argsUsedForDateSize=8;
  int argsUsedForDate[argsUsedForDateSize];

  private :
  //
  // Internal routine that parses times from strings.
  // returns TRUE if it went OK.
  //
  bool parseTime(char *S, date_time_t *T, int MaxYear, int MinYear);
  //
  // Internal routine that checks for a time's validity,
  // and converts it to unix_time if valid.
  //
  bool checkTime( int MaxYear, int MinYear,  date_time_t *T );
  //
};

#endif

