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
// $Id: MdvBlend.hh,v 1.5 2016/03/04 02:22:10 dixon Exp $
//
// MdvBlend
//
// Yan Chen, RAL, NCAR
//
// Dec. 2007
//
///////////////////////////////////////////////////////////////

#ifndef MdvBlend_HH
#define MdvBlend_HH

#include <set>
#include <deque>
#include <toolsa/DateTime.hh>
#include "Args.hh"
#include "Params.hh"

using namespace std;

/////////////////////////
// Forward declarations

class MdvBlend {
  
public:

  // constructor

  MdvBlend (int argc, char **argv);

  // destructor
  
  ~MdvBlend();

  // run 

  int Run();

  void childProcessesReduced();

  // data members

  bool isOK;

protected:
  
  date_time_t startTime;
  date_time_t endTime;
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  set<time_t> arrivedSHSet;
  set<time_t> arrivedNHSet;

  int _numChildren;
  int _maxChildren;
  deque<time_t> timesWaitingToProcess;

  int _processRealtime();
  int _processRealtimeInChild(time_t process_time);
  int _processArchiveInChild(time_t start, time_t end);
  int _processArchive(time_t start, time_t end);
  int _processFilelist();
};

#endif
