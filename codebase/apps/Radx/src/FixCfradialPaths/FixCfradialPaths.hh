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
// FixCfradialPaths.hh
//
// FixCfradialPaths object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2010
//
///////////////////////////////////////////////////////////////

#ifndef FixCfradialPaths_H
#define FixCfradialPaths_H

#include "Args.hh"
#include "Params.hh"
#include <string>
class RadxVol;
class RadxFile;
class DoradeRadxFile;
using namespace std;

class FixCfradialPaths {
  
public:

  // constructor
  
  FixCfradialPaths (int argc, char **argv);

  // destructor
  
  ~FixCfradialPaths();

  // run 

  int Run();

  // data members

  int OK;

protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  time_t _startTime, _endTime, _renameTime;
  int _startMillisecs, _endMillisecs, _renameMillisecs;

  int _runFilelist();
  int _runArchive();
  int _runRealtime();
  void _setupRead(RadxFile &file);
  int _processFile(const string &filePath);

  int _computeNewName(const RadxVol &vol,
                      const string &filePath,
                      string &newName);
  
  void _setupWritePath(RadxFile &wFile);
  
  int _renameInPlace(const string &filePath,
                     const string &newName);

  int _copyFile(const string &filePath,
                const string &newName,
                const string &outDir);
  
  int _createSymbolicLink(const string &filePath, 
                          const string &newName,
                          const string &outDir);

  string _computeSubdirPath(const string &outDir);

};

#endif
