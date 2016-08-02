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
// ScoutOut.hh: Does the work of the scout program. Recursive.
//
// Niles Oien from
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 1998
//
/////////////////////////////////////////////////////////////

#ifndef SCOUT_OUT_H
#define SCOUT_OUT_H

#include <sys/stat.h>
#include <dsserver/DmapAccess.hh>
#include "Params.hh"
class DataFileNames;

class ScoutOut {
  
public:

  // constructor

  ScoutOut(const Params &params,
	   const string &dir_path,
	   int level);

  // recurse into sub dir
  
  void Recurse();
  
private:
  
  void HandleDir(const string &DirName,
		 const DataFileNames &Q);

  void HandleFile(const string &FileName,
		  const DataFileNames &Q);

  void RecurseDatedSubDir(const string &dir);

  void RecurseSubDir(const string &dir);

  void AssignTimes(const DataFileNames &Q);
  void AssignTimes(const struct stat &fileStat);

  int _nFilesCounted;
  int _nFilesPerSecDelay;

  double _nBytes, _nFiles;
  date_time_t _start, _end;
  bool _timeAssigned;

  Params _params;
  string _dirPath;
  int _level;
  FILE *_logFile;
  DmapAccess _dmap;
  bool _dmapAvail;

};

#endif







