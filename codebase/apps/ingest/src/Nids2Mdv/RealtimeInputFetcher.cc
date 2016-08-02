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

#include "RealtimeInputFetcher.hh"
using namespace std;

RealtimeInputFetcher::RealtimeInputFetcher(const string & progName,
                                           const bool & verbose,
                                           const string & inputDir,
                                           const int & maxDataAge,
                                           const bool & useLDataInfo,
                                           const bool & getLatestFileOnly) :
  InputFetcher(progName, verbose),
  _inputDir(inputDir),
  _maxDataAge(maxDataAge),
  _useLDataInfo(useLDataInfo),
  _getLatestFileOnly(getLatestFileOnly),
  _lastProcessedTime(0)

{

}

RealtimeInputFetcher::~RealtimeInputFetcher()

{
  if (_input != NULL) {
    delete _input;
    _input = NULL;
  }
}

// Virtual
int RealtimeInputFetcher::initInputPaths()

{
  _input = new DsInputPath( (char *) _progName.c_str(),
                            _verbose,
                            (char *) _inputDir.c_str(),
                            _maxDataAge,
                            NULL,
                            _useLDataInfo,
                            _getLatestFileOnly );

  if (_input == NULL) {
    return -1;
  }
  else {
    return 0;
  }
}

// Virtual
int RealtimeInputFetcher::fetchNextFile(string & nextFile)

{
  if(_useLDataInfo) {
      // Get the ldata handle (this makes a copy, since get() returns const)
      LdataInfo ldata = _input->getLdataInfo();
      ldata.read(-1);
      time_t ldataTime = ldata.getLatestTime();
      if (ldataTime > _lastProcessedTime) {
        _lastProcessedTime = ldataTime;
        nextFile = ldata.getDataDir() + PATH_DELIM + ldata.getRelDataPath();
        return 1;
      } else {
        return (0);
      }
  }  else {
	 
	 char *file_path;
	 if((file_path = _input->next(false)) != NULL) {
		 nextFile = file_path;
		 return 1;
	 } else {
		 return 0;
	 }
  }
}

