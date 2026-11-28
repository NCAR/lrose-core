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
// FilelistInputStrategy : Strategy class for handling a
//                         list of input files.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2002
//
//////////////////////////////////////////////////////////


#include "FilelistInputStrategy.hh"
#include <iostream>

using namespace std;

FilelistInputStrategy::FilelistInputStrategy(const vector<string>& fileList,
					     const bool debug) :
  InputStrategy(debug)
{
  for (size_t i = 0; i < fileList.size(); ++i) {
    _filePaths.push_back( fileList[i] );

  }
  _pathPtr = _filePaths.begin();
}

FilelistInputStrategy::~FilelistInputStrategy()
{
}


const string &FilelistInputStrategy::next()
{  
  // Get next file path in the list.  If we are finished
  // with the list, return the null string.

  if (_pathPtr != _filePaths.end())
  {
    _currInputPath = *_pathPtr;
    _pathPtr++;
  }
  else
  {
    _currInputPath = "";
  }

  if (_debug)
    cerr << "---> Next input file: " << _currInputPath << endl;
  
  return(_currInputPath);
}
