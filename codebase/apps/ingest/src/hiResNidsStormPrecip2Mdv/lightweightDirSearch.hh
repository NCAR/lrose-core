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
// lightweightDirSearch.hh
//
// Definitons for class
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#ifndef LWDS_H
#define LWDS_H

#include <time.h>

using namespace std;

class lightweightDirSearch {
  
public:

  // Constructor. Makes a copy of things we need.
  lightweightDirSearch (char *dir,       // Directory to search under
			int maxAgeSecs,  // Maximum file age, seconds
			bool debug,      // Print debug messages or not
			int sleepSecs    // Seconds to sleep betwen searches
			);


  // Destructor. Does nothing, but avoids use of the default destructor.
  ~lightweightDirSearch ();

  // Main routine - gets next file. Blocks, registers with procmap.
  char *nextFile( );

  // public data

protected:
  
private:

  // Note that variables and methods that are private to a class
  // typically start with an underscore at RAL.

  // Local copies of stuff we need.
  char _topDir[1024];
  int _maxAgeSecs;
  bool  _debug;
  time_t _lastTimeReturned;
  char _fileName[1024];
  bool _foundFile;
  int _sleepSecs;
  time_t _bestTimeSoFar;

  void _procDir(char *dirName);

};

#endif





