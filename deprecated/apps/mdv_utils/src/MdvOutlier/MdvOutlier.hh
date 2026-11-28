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
// $Id: MdvOutlier.hh,v 1.3 2016/03/04 02:22:12 dixon Exp $
//
// MdvOutlier
//
// Yan Chen, RAL, NCAR
//
// Jan. 2008
//
///////////////////////////////////////////////////////////////

#ifndef MdvOutlier_HH
#define MdvOutlier_HH

#include <deque>
#include <set>
#include <toolsa/DateTime.hh>
#include "Args.hh"
#include "Params.hh"

using namespace std;

/////////////////////////
// Forward declarations

class MdvOutlier {
  
public:

  // constructor

  MdvOutlier (int argc, char **argv);

  // destructor
  
  ~MdvOutlier();

  // run 

  int Run();

  void childProcessesReduced();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  set<time_t> _arrivedSet;

  int _numChildren;
  int _maxChildren;
  deque<time_t> timesWaitingToProcess;

  int _processRealtime();
  int _processRealtimeInChild(time_t process_time);
  int _processArchive();
};

#endif
