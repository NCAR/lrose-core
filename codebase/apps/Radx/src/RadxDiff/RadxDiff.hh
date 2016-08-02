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
// RadxDiff.hh
//
// RadxDiff object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2013
//
///////////////////////////////////////////////////////////////

#ifndef RadxDiff_H
#define RadxDiff_H

#include <iostream>
#include <fstream>
#include <string>
#include <Radx/RadxVol.hh>
#include "Args.hh"
#include "Params.hh"
class RadxFile;
using namespace std;

class RadxDiff {
  
public:

  // constructor
  
  RadxDiff (int argc, char **argv);

  // destructor
  
  ~RadxDiff();

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

  time_t _readSearchTime;

  RadxVol _vol1;
  RadxVol _vol2;

  string _path1;
  string _path2;

  double _totalPoints;
  double _totalErrors;

  ostream *_out;
  ofstream _outFile;

  int _handleViaPaths(const string &path1, const string &path2);
  int _handleViaTime();
  int _readFile(const string &path, RadxVol &vol, bool isFile1);
  int _getPathForTime(const string &dir, string &path);
  void _setupRead(RadxFile &file, bool isFile1);
  int _performDiff();
  int _diffVolMetaData();
  int _diffSweeps();
  int _diffSweeps(int isweep,
                  const RadxSweep *sweep1,
                  const RadxSweep *sweep2);
  int _diffRays();
  int _diffRays(int iray,
                const RadxRay *ray1,
                const RadxRay *ray2);
  int _diffFields(int iray,
                  const RadxRay *ray1,
                  const RadxRay *ray2);
  int _diffFields(int iray,
                  int ifield,
                  const RadxField *field1,
                  const RadxField *field2);
  
};

#endif
