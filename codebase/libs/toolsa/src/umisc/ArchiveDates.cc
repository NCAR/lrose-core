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
// to see if a start and end time have been specified. Typically
// a client will use the constructor and then access
// the class's public variables to get the start and end times.
//
// Niles Oien, October 2001
//
//
//
//

#include <toolsa/ArchiveDates.hh>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream>
using namespace std;

///////////////////////////////////////////////////////////
//
// Internal routine that checks for a time's validity,
// and converts it to unix_time if valid.
//
bool ArchiveDates::checkTime( int MaxYear, int MinYear,  date_time_t *T ){
  //
  // Perform check on year, if requested.
  // It is requested if MinYear and MaxYear 
  // are both positive.
  //
  if (
      (MinYear > 0) &&
      (MaxYear > 0)
      ){
    if (
	(T->year > MaxYear) ||
	(T->year < MinYear)
	){
      return false;
    }
  }
  //
  // Perform checks on other variables.
  //
  if (
      (T->month > 12) ||
      (T->month < 1)
      ){
    return false;
  }

  if (
      (T->day > 31) ||
      (T->day < 1)
      ){
    return false;
  }

  if (
      (T->hour > 23) ||
      (T->hour < 0)
      ){
    return false;
  }

  if (
      (T->min > 59) ||
      (T->min < 0)
      ){
    return false;
  }

  if (
      (T->sec > 59) ||
      (T->sec < 0)
      ){
    return false;
  }
  //
  // Checks passed - convert to unix time and return true.
  //  
  uconvert_to_utime( T );
  return true;
}
//////////////////////////////////////////////////////////
//
// Internal routine that parses times from strings.
// returns TRUE if it went OK.
//
bool ArchiveDates::parseTime(char *S, date_time_t *T, int MaxYear, int MinYear){
  //
  // Try the YYYYMMDDhhmmss format.
  //
  if (strlen(S) == strlen("YYYYMMDDhhmmss")){
    if (6 == sscanf(S,"%4d%2d%2d%2d%2d%2d",
		    &T->year, &T->month, &T->day,
		    &T->hour, &T->min, &T->sec)){
      return checkTime( MaxYear, MinYear,  T );
    }
  }
  //
  // Try the YYYY/MM/DD hh:mm:ss format.
  //
  if (6 == sscanf(S,"%d/%d/%d %d:%d:%d",
		  &T->year, &T->month, &T->day,
		  &T->hour, &T->min, &T->sec)){
    return checkTime( MaxYear, MinYear,  T );
  }
  //
  // Try the YYYY/MM/DD_hh:mm:ss format.
  //
  if (6 == sscanf(S,"%d/%d/%d_%d:%d:%d",
		  &T->year, &T->month, &T->day,
		  &T->hour, &T->min, &T->sec)){
    return checkTime( MaxYear, MinYear,  T );
  }
  //
  // Try the YYYY_MM_DD_hh_mm_ss format.
  //
  if (6 == sscanf(S,"%d_%d_%d_%d_%d_%d",
		  &T->year, &T->month, &T->day,
		  &T->hour, &T->min, &T->sec)){
    return checkTime( MaxYear, MinYear,  T );
  }

  //
  // Try the YYYY MM DD hh mm ss format.
  //
  if (6 == sscanf(S,"%d %d %d %d %d %d",
		  &T->year, &T->month, &T->day,
		  &T->hour, &T->min, &T->sec)){
    return checkTime( MaxYear, MinYear,  T );
  }

  //
  // Failed to parse all the above formats.
  //
  return false;
}

//
//////////////////////////////////////////////
//
// Constructor - pass in command line args. Does most everything.
//
ArchiveDates::ArchiveDates(int argc,      // From command line 
			   char *argv[],  // From command line
			   int MaxYear,   // Max value for year to be considered valid
			   int MinYear){  // Min value for year to be considered valid
  //
  // Initialize errorString and other variables.
  //
  errorString = "Use -helpdate option for help on specifying dates.\n";

  startTime.unix_time = 0; uconvert_from_utime( &startTime );
  endTime.unix_time = 0; uconvert_from_utime( &endTime );

  timesAreValid =  startTimeValid = endTimeValid = false;

  for (int i=0; i<argsUsedForDateSize; i++){
    argsUsedForDate[i]=-1;
  }

  startUnixTime=0;
  endUnixTime=0;

  /////////////////////////////////////////////////

  helpString = "\n\nStart and end times can be specified on\n"
    "the command line in several ways : \n\n"
    "-archive <start time> <end time>, or\n"
    "-interval <start time> <end time>, or\n"
    "-start <start time> -end <end time>\n\n"
    "Where the format for specifying a time can be any of :\n\n"
    "YYYYMMDDhhmmss, as in 20011020143000 (must specify month, hour etc. as double digits), or\n"
    "YYYY/MM/DD hh:mm:ss, as in 2001/10/20 14:30:0 (single digits are OK here), or\n"
    "YYYY/MM/DD_hh:mm:ss, as in 2001/10/20_14:30:0 (single digits are OK here), or\n"
    "YYYY MM DD hh mm ss, as in 2001 10 20 14 30 0 (single digits are OK here), or\n"
    "YYYY_MM_DD_hh_mm_ss, as in 2001_10_20_14_30_0 (single digits are OK here)\n\n"
    "So all the following are valid and equivalent : \n\n"
    "-archive 20011020143000 20011020183000\n"
    "-archive 20011020143000 \"2001/10/20 18:30:00\"\n"
    "-start \"2001/10/20 14:30:00\" -end 2001_10_20_18_30_0\n\n";
  
  /////////////////////////////////////////////////
  //
  // Loop through args to get the indicies of the start and end
  // time strings. Do not break from the loop early or skip
  // entries following -start  or -end since the -helpdate
  // option may come at any time and must be detected.
  //
  int usedArgIndex = 0;
  int startIndex = -1;
  int endIndex = -1;
  for (int i=1; i<argc; i++){ // Skip over program name in arg list - start at 1
    //
    // Are we asking for help?
    //
    if (!(strcmp(argv[i],"-helpdate"))){
      cerr << helpString;
      exit(0);
    }
    //
    // Are we using -interval or the equivalent -archive?
    //
    if (
	(!(strcmp(argv[i],"-archive"))) ||
	(!(strcmp(argv[i],"-interval")))
	){
      argsUsedForDate[usedArgIndex] = i; usedArgIndex++;
      startIndex = i+1; endIndex = i+2;
    }
    //
    // -start means stow the start time index
    //
    if (!(strcmp(argv[i],"-start"))){
      startIndex = i+1;
      argsUsedForDate[usedArgIndex] = i; usedArgIndex++;
    }
    //
    // -end means stow the end time index
    //
    if (!(strcmp(argv[i],"-end"))){
      endIndex = i+1;
      argsUsedForDate[usedArgIndex] = i; usedArgIndex++;
    }
  } // End of loop that gets start,end time string indicies in argv.
  //
  // Return if not archive mode times specified.
  //
  if ((startIndex == -1) && (endIndex == -1)){
    // No archive times specified
    return;
  }
  //
  // See if the user has not specified enough arguments, ie.
  // if they have done something like "MyProg -start" with no args.
  //
  // Exit in this case, since they are almost certainly trying to
  // get archive mode and failing.
  //
  if ((startIndex > argc-1) || (endIndex > argc-1)){ 
    cerr << helpString;
    cerr << "Not enough on command line to specify start and end times." << endl;
    exit(-1);
  }
  //
  // See if we can parse out the start time.
  //
  if (startIndex != -1){
    argsUsedForDate[usedArgIndex] = startIndex; usedArgIndex++;
    startTimeValid = parseTime(argv[startIndex], &startTime, MaxYear, MinYear);
    if (!(startTimeValid)){
      string startString( argv[startIndex] );
      errorString = "Cannot parse start time : " +
	startString + "\n" + errorString;
    }
  }
  //
  // See if we can parse out the end time.
  //
  if (endIndex != -1){
    argsUsedForDate[usedArgIndex] = endIndex; usedArgIndex++;
    endTimeValid = parseTime(argv[endIndex], &endTime, MaxYear, MinYear);
    if (!(endTimeValid)){
       string endString( argv[endIndex] );
      errorString = "Cannot parse end time : " +
	endString + "\n" + errorString;
    }
  }
  //
  // Fill in the UNIX time entries.
  //
  if (startTimeValid) startUnixTime = startTime.unix_time;
  if (endTimeValid) endUnixTime = endTime.unix_time;
  //
  // Set the main boolean and return.
  //
  timesAreValid = ((startTimeValid) && (endTimeValid));
  //
  // Check that the times are in the right order.
  //
  if (timesAreValid){
    if (startUnixTime > endUnixTime) {
      timesAreValid = false;
      errorString = "End time preceeds start time.\n" + errorString;
    }
  }
  return;
  //
}

////////////////////////////////////////////////////
//
// Destructor - does nothing.
//
ArchiveDates::~ArchiveDates(){

}





