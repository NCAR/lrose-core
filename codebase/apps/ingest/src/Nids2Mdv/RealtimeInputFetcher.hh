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
// RealtimeInputFetcher.hh
//
// Fetch file names for processing when in archive mode.
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2001
//
/////////////////////////////////////////////////////////////

#ifndef RealtimeInputFetcher_HH
#define RealtimeInputFetcher_HH

#include "InputFetcher.hh"
using namespace std;

class RealtimeInputFetcher : public InputFetcher {
  
public:

  // constructor

  RealtimeInputFetcher(const string & progName,
                       const bool & verbose,
                       const string & inputDir,
                       const int & maxDataAge,
                       const bool & useLDataInfo,
                       const bool & getLatestFileOnly);

  // destructor

  virtual ~RealtimeInputFetcher();

  // process a file

  virtual int initInputPaths();
  virtual int fetchNextFile(string & nextFile);

protected:

  string _inputDir;
  int _maxDataAge;
  bool _useLDataInfo;
  bool _getLatestFileOnly;
  time_t _lastProcessedTime;

private:

  // Private default constructor with no impl -- do not use.
  RealtimeInputFetcher();

};

#endif

