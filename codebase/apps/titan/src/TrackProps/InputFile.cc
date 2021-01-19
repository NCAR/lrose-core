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
// InputFile.cc : Input file handling
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// January 1998
//
//////////////////////////////////////////////////////////
//
// In REALTIME mode, the input data stream is monitored using
// the toolsa LDATA module. The input file path is computed
// from the latest data time.
//
// In ARCHIVE mode, the file list supplied on the command line
// is traversed.
//
///////////////////////////////////////////////////////////

#include "InputFile.hh"
#include <toolsa/umisc.h>
#include <toolsa/str.h>
using namespace std;

// Constructor

InputFile::InputFile (const char *prog_name,
		      int debug,
                      const vector<string> &filePaths)
  
{

  // initialize
  
  _progName = STRdup(prog_name);
  _debug = (debug? 1 : 0);
  
  _fileNum = 0;
  _filePaths = filePaths;

}

// Destructor

InputFile::~InputFile()

{

  STRfree(_progName);

}

/////////////////////
// get next file path
//
// returns NULL on failure - end of list

string InputFile::next()

{

  // in archive mode, go through the file list
  
  if (_fileNum < _filePaths.size()) {
    _fileNum++;
    return _filePaths[_fileNum - 1];
  } else {
    return "";
  }

}



