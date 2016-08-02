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
/////////////////////////////////////////////////////////////
// DataTimes.hh
//
// File time limits class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#ifndef DataTimes_HH
#define DataTimes_HH

#include "Worker.hh"
#include <ctime>
#include <dsserver/DsLdataInfo.hh>
#include <Mdv/DsMdvxTimes.hh>
using namespace std;

////////////////////////////////
// DataTimes

class DataTimes : public Worker {
  
public:

  // REALTIME mode constructor

  DataTimes(const string &prog_name, const Params &params);

  // ARCHIVE mode constructor

  DataTimes(const string &prog_name, const Params &params,
	    time_t start_time, time_t end_time);

  // destructor
  
  virtual ~DataTimes();

  // get latest input data time
  // forces a read

  time_t getLatest();

  // get next input data time
  // blocks until new data

  time_t getNext();

  // get the storm file date
  time_t getStormFileDate();

  // compute the restart time and related times,
  // given the trigger time and restart parameters

  static void computeRestartTime(time_t trigger_time, 
				 int restart_hour,
				 int restart_min,
				 int overlap_period,
				 time_t &overlap_start_time,
				 time_t &start_time,
				 time_t &restart_time);
  
protected:
  
private:

  time_t _startTime;
  time_t _endTime;
  time_t _latestTime;
  time_t _stormFileDate;
  DsMdvxTimes _mdvxTimes;

  void _setStormFileDate(time_t date);
  void _setTimeLimitsRealtime();
  int _setStartTimeFromStormFile();
  
};

#endif


