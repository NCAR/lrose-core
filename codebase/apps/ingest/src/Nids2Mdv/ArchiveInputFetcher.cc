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

#include "ArchiveInputFetcher.hh"
using namespace std;

ArchiveInputFetcher::ArchiveInputFetcher(const string & progName,
                                         const bool & verbose,
                                         const vector<string> & inputDirs) :
  InputFetcher(progName, verbose),
  _inputDirs(inputDirs)

{

}

ArchiveInputFetcher::~ArchiveInputFetcher()

{
  if (_input != NULL) {
    delete _input;
    _input = NULL;
  }
}

// Virtual
int ArchiveInputFetcher::initInputPaths()

{
  _input = new DsInputPath( (char *) _progName.c_str(), _verbose, _inputDirs);

  if (_input == NULL) {
    return -1;
  }
  else {
    return 0;
  }
}

// 
// Fetch the next available file.
//  Returns:  1 if there is a next file,
//           -1 if there are no more files.
//
//
// Virtual
int ArchiveInputFetcher::fetchNextFile(string & nextFile)

{
  char *file_path;
  file_path = _input->next();

  if (file_path != NULL) {
    nextFile = file_path;
    return 1;
  }
  else {
    return (-1);
  }
}

