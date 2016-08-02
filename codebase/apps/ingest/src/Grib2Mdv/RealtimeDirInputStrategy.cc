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
//////////////////////////////////////////////////////////
// RealtimeDirInputStrategy : Strategy class for handling realtime
//                      input files in a directory with a
//                      latest_data_info file.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2002
//
//////////////////////////////////////////////////////////

#include <unistd.h>
#include <iostream>
#include <cstring>
#include "RealtimeDirInputStrategy.hh"

using namespace std;

RealtimeDirInputStrategy::RealtimeDirInputStrategy(const string &data_dir,
						   const string &input_substring,
						   const int max_valid_age,
						   heartbeat_t heartbeat_func,
						   const bool debug) :
  InputStrategy(debug),
  _maxValidAge(max_valid_age),
  _heartbeatFunc(heartbeat_func),
  _inputDir(new InputDir(data_dir.c_str(), input_substring.c_str(), 0))
{
}

RealtimeDirInputStrategy::~RealtimeDirInputStrategy()
{
}


const string& RealtimeDirInputStrategy::next()
{  
  _heartbeatFunc("Checking for data");

  // See if we already have a new input file waiting.  If so, return
  // this file's path.

  char *input_path = _inputDir->getNextFilename(0, _maxValidAge);
  
  if (input_path != NULL)
  {
    if (!isRsyncTempFile(input_path))
    {
      _currentInputPath = input_path;

      if (_debug)
      {
        const time_t timp = time(NULL);
        cout << "---> Next input file: " << _currentInputPath
          << " at " << ctime(&timp) << endl;
      }
  
      return _currentInputPath;
    }
    else if (_debug)
    {
      const time_t timp = time(NULL);
      cout << "rsync file " << input_path
        << " skipped at " << ctime(&timp) << endl;
    }
  }

  // Keeping checking the input directory for a new input file to
  // appear.

  bool foundNextFile = false;
  while (!foundNextFile)
  {
    input_path = _inputDir->getNextFilename(1, _maxValidAge);
    if (input_path != NULL)
    {
      if (!isRsyncTempFile(input_path))
      {
        foundNextFile = true;
      }
      else if (_debug)
      {
        const time_t timp = time(NULL);
        cout << "rsync file " << input_path
          << " skipped at " << ctime(&timp) << endl;
      }
    }

    if (!foundNextFile)
    {
      _heartbeatFunc("Checking for data");
      sleep(1);
    }
  }

  _currentInputPath = input_path;

  if (_debug)
  {
    const time_t timp = time(NULL);
    cout << "---> Next input file: " << _currentInputPath
      << " at " << ctime(&timp) << endl;
  }
  
  return _currentInputPath;
}


bool RealtimeDirInputStrategy::isRsyncTempFile(char *filepath)
{
  // move backwards until we find a slash
  char *sptr;
  for (sptr = filepath + strlen(filepath) - 1; sptr >= filepath; sptr--)
  {
    if (*sptr == '/')
      break;
  }

  // see if filename begins with a dot
  bool leadingDot = *(sptr + 1) == '.';
  return leadingDot;
}

