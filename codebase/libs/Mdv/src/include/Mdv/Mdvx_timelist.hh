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
////////////////////////////////////////////////
//
// Mdvx_timelist.hh
//
// Time list functions for Mdvx class
//
////////////////////////////////////////////////

// This header file can only be included from within Mdvx.hh
#ifdef _in_Mdvx_hh

/////////////////////////////////////////////////////////////////
// Before calling compileTimeList(), you must first clear the
// time list mode and then set the mode with one of the
// set functions.
//
// For data sets which could switch on the domain,
// you must also set the domain using setReadHorizLimits().

/////////////////////////////////////////////////////////////////
// NOTE: you can constrain the lead times to be considered in
// the time search, if data is stored in forecast format.
//
// Does not apply to SPECIFIED_FORECAST mode.
//
// See Mdvx_read.hh, setConstrainFcastLeadTimes().
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// setTimeListModeValid
//
// Set the time list so that it finds all of the valid data
// times between the start and end times.
// For forecast data where multiple forecasts exist for the same
// valid time, a single valid time will be returned.

virtual void
  setTimeListModeValid(const string &dir,
                       time_t start_time,
                       time_t end_time);

/////////////////////////////////////////////////////////////////
// setTimeListModeGen
//
// Set the time list so that it finds all of the
// generate times between the start and end times

virtual void
  setTimeListModeGen(const string &dir,
                     time_t start_gen_time,
                     time_t end_gen_time);

/////////////////////////////////////////////////////////////////
// setTimeListModeForecast
// setTimeListModeLead (equivalent, deprecated)
//
// Set the time list mode so that it returns all of the forecast
// times for the given generate time.

virtual void
  setTimeListModeForecast(const string &dir,
                          time_t gen_time);

virtual void
  setTimeListModeLead(const string &dir,
                      time_t gen_time);

/////////////////////////////////////////////////////////////////
// setTimeListModeGenPlusForecasts
//
// Set the time list so that it finds all of the
// generate times between the start and end gen times.
// Then, for each generate time, all of the forecast times are
// found. These are made available in the
// _forecastTimesArray, which is represented by vector<vector<time_t> >
  
virtual void
  setTimeListModeGenPlusForecasts(const string &url,
                                  time_t start_gen_time,
                                  time_t end_gen_time);

/////////////////////////////////////////////////////////////////
// setModeValidMultGen
//
// Set the time list so that it finds all of the forecasts
// within the time interval specified. For each forecast found
// the associated generate time is also determined.
// The forecast times will be available in the _timeList array.
// The generate times will be available in the _genTimes array.

virtual void
  setTimeListModeValidMultGen(const string &dir,
                              time_t start_time,
                              time_t end_time);
 
/////////////////////////////////////////////////////////////////
// setTimeListModeFirst
//
// set the time list so that it finds the first available data time

virtual void setTimeListModeFirst(const string &dir);

/////////////////////////////////////////////////////////////////
// setTimeListModeLast
//
// set the time list so that it finds the last available data time

virtual void setTimeListModeLast(const string &dir);

/////////////////////////////////////////////////////////////////
// setTimeListModeClosest
//
// set the time list mode so that it finds the closest available data time
// to the search time within the search margin

virtual void
  setTimeListModeClosest(const string &dir,
                         time_t search_time,
                         int time_margin);

/////////////////////////////////////////////////////////////////
// setTimeListModeFirstBefore
//
// set the time list mode so that it finds the first available data time
// before the search time within the search margin

virtual void
  setTimeListModeFirstBefore(const string &dir,
                             time_t search_time,
                             int time_margin);

/////////////////////////////////////////////////////////////////
// setTimeListModeFirstAfter
//
// set the time list mode so that it finds the first available data time
// after the search time within the search margin

virtual void
  setTimeListModeFirstAfter(const string &dir,
                            time_t search_time,
                            int time_margin);

/////////////////////////////////////////////////////////////////
// setTimeListModeBestForecast
//
// Set the time list so that it returns the best forecast
// for the search time, within the time margin

virtual void
  setTimeListModeBestForecast(const string &dir,
                              time_t search_time,
                              int time_margin);

/////////////////////////////////////////////////////////////////
// setTimeListModeSpecifiedForecast
//
// Set the time list so that it returns the forecast for the given
// generate time, closest to the search time, within the time margin

virtual void
  setTimeListModeSpecifiedForecast(const string &dir,
                                   time_t gen_time,
                                   time_t search_time,
                                   int time_margin);

/////////////////////////////////////////////////////////////////
// clearTimeListMode
//
// clear out time list mode info - do this first

virtual void clearTimeListMode();

//////////////////////////
// print time list request

virtual void printTimeListRequest(ostream &out);

//////////////////////////
// print time height request

virtual void printTimeHeightRequest(ostream &out);

//////////////////////////////////////////////////////////
// compile time list
//
// Compile a list of available data times in the specified
// directory between the start and end times.
//
// You must call one of the mode set functions above before calling
// this function.
//
// Returns 0 on success, -1 on failure
// getErrStr() retrieves the error string.
//
// After a successful call to compileTimeList(), access the
// time list via the following functions:
//   getTimeList(): vector of time_t
//   getForecastTimesArray(): vector of vector of time_t
//   getNTimesInList(): n entries in list
//   getTimeFromList(int i): time_t from list
//   timeListHasForecasts(): does list contain forecast times?

virtual int compileTimeList();

////////////////////////////////////////////////////////////////////
// Compile time-height (time-series) profile, according to the
// read settings and time list specifications.
//
// Before using this call:
//
// (a) Set up read parameters, such as fields, vlevels, encoding
//     type etc.
// (b) Set up the lat/lon of the point to be sampled, using:
//     addReadWayPt(lat, lon)
// (c) Set up the time list, using setTimeListMode????()
//
// Returns 0 on success, -1 on failure.
// 
// Data fields returned will be time-height profiles, with time
// in the x dimension, ny = 1, height in the z dimension.
//
// Actual times for the data will be returned in the time lists,
// namely validTimes and genTimes.

virtual int compileTimeHeight();

#endif
