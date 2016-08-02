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
////////////////////////////////////////////////////////////////////
// didss/DsTimeList
//
// Time list class for files in the DIDSS infrastructure
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2000
//
////////////////////////////////////////////////////////////////////

#ifndef DsTimeList_hh
#define DsTimeList_hh

#include <vector>
#include <string>
#include <set>
#include <iostream>
#include <ctime>
using namespace std;

class DsTimeList
{

public:

  ///////////////////////
  // default constructor
  
  DsTimeList();

  ///////////////////
  // copy constructor
  
  DsTimeList(const DsTimeList &rhs);

  ///////////////////////
  // destructor

  virtual ~DsTimeList();

  // assignment
  
  DsTimeList & operator=(const DsTimeList &rhs);

  // set the debugging state

  void setDebug(bool debug = true) { _debug = debug; }

  // clear error string
  
  void clearErrStr() const;

  //////////////////////////////////////////////////////////
  // getting a time list
  //
  // The following methods compile various types of time list.
  //
  // They return 0 on success, -1 on failure
  // getErrStr() retrieves the error string.
  //
  // After a successful call to a get(), access the
  // time list via the following functions:
  //   getTimeList(): vector of time_t
  //   getNTimesInList(): n entries in list
  //   getTimeFromList(int i): time_t from list
  //   timeListHasForecasts(): does list contain forecast times?
  
  /////////////////////////////////////////////////////////////////
  // getValid
  //
  // get the time list of all of the valid data
  // times between the start and end times
  //
  // If purge is true, multiple time entries are purged from the
  // list. These can occur with forecast data, when forecasts
  // from multiple gen times refer to the same valid time.
  //
  // Returns 0 on success, -1 on error.
  // Retrieve error with GetErrStr()
  
  int getValid(const string &dir,
	       const time_t start_time,
	       const time_t end_time,
	       bool purge = true);
  
  /////////////////////////////////////////////////////////////////
  // getGen
  //
  // get the time list of all of the forecast generate times
  // between the start and end times
  //
  // Returns 0 on success, -1 on error.
  // Retrieve error with GetErrStr()
  
  int getGen(const string &dir,
	     const time_t start_time,
	     const time_t end_time);
  
  /////////////////////////////////////////////////////////////////
  // getLead
  //
  // get the time list of all the lead times for the given generate
  // time. time values are loaded in the list. To compute the
  // lead time subtract the gen_time.
  //
  // Returns 0 on success, -1 on error.
  // Retrieve error with GetErrStr()
  
  int getLead(const string &dir,
	      const time_t gen_time);
  
  /////////////////////////////////////////////////////////////////
  // getStartAndEnd
  //
  // Returns 0 on success, -1 on error.
  // Retrieve error with GetErrStr()
  //
  // On success, sets start_time and end_time.
  
  int getStartAndEnd(const string &dir,
		     time_t &start_time,
		     time_t &end_time);

  //////////////////
  // print time list
  
  virtual void print(ostream &out);

  ///////////////////////
  // access to time list

  const vector<time_t> &getTimeList() const { return (_timeList); }
  int getNTimesInList() const { return (_timeList.size()); }
  time_t getTimeFromList(int i) const { return (_timeList[i]); }
  bool timeListHasForecasts() const { return (_hasFcasts); }

  ///////////////////////
  // Get the Error String.
  // This has contents when an error is returned.

  string getErrStr() const { return _errStr; }

  // clear the list
  // Note: this is called automatically at the start of each get() call

  void clearList();

protected:

  // error string
  mutable string _errStr;

  // debug state
  bool _debug;

  // time list

  string _dir;
  time_t _startTime;
  time_t _endTime;
  time_t _genTime;
  vector<time_t> _timeList;
  bool _hasFcasts;

  DsTimeList &_copy(const DsTimeList &rhs);

  void _compileByDay(bool is_gen);

  void _addForecasts(const string &dayDirPath,
		     const string &genSubDir,
		     time_t gen_time);
  
  void _compileFlat(const string &dir);

  void _compileLead();

  int _getStartAndEndByDay(time_t &start_time,
			   time_t &end_time);
  
  void _getStartAndEndFlat(const string &dir,
				       bool &found,
				       time_t &start_time,
				       time_t &end_time);
  
  bool _isByDay();

  void _purgeMultEntries();

private:

};

#endif


