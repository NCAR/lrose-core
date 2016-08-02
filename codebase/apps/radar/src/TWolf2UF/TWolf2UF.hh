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
// TWolf2UF.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2010
//
///////////////////////////////////////////////////////////////
//
// TWolf2UF reads TWolf moments data in ASCII format and
// converts to UF.
//
////////////////////////////////////////////////////////////////

#ifndef TWolf2UF_HH
#define TWolf2UF_HH

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "UfRecord.hh"

using namespace std;

////////////////////////
// This class

class TWolf2UF {
  
public:

  // constructor

  TWolf2UF (int argc, char **argv);

  // destructor
  
  ~TWolf2UF();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  // data

  string _progName;
  Args _args;
  UfRecord _rec;
  DateTime _startTime;
  DateTime _prevTime;
  DateTime _rayTime;

  int _volNum;
  int _sweepNum;
  int _rayNum;
  double _prevFixedAngle;
  double _fixedAngle;
  bool _fixedAngleChanging;
  bool _isRhi;

  string _ufPath;
  FILE *_ufFile;
  
  // methods

  int _processFile(const string &filePath);
  
  int _processRay(const char *line);

  bool _isScanRhi(FILE *in);
  
  void _tokenize(const string &str,
                 const string &spacer,
                 vector<string> &toks);
  
  string _computeUfFileName(int volNum,
                            string instrumentName,
                            string scanType,
                            int year, int month, int day,
                            int hour, int min, int sec);

  int _openUfFile();
  void _closeUfFile();

  int _writeRecord();

  int _makeDir(const string &dir);
  int _makeDirRecurse(const string &dir);

  int _getDataTime(const string& file_path,
                   DateTime &data_time);

};

#endif
