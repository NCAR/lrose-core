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
// GemVolXml2Dsr.hh
//
// GemVolXml2Dsr object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2006
//
///////////////////////////////////////////////////////////////

#ifndef GemVolXml2Dsr_hh
#define GemVolXml2Dsr_hh

#include <string>
#include <vector>
#include <didss/DsInputPath.hh>
#include <Fmq/DsRadarQueue.hh>
#include "Args.hh"
#include "Params.hh"
#include "InputFile.hh"
using namespace std;

////////////////////////
// This class

class GemVolXml2Dsr {
  
public:

  // constructor

  GemVolXml2Dsr (int argc, char **argv);

  // destructor
  
  ~GemVolXml2Dsr();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsInputPath *_input;
  DsRadarQueue _rQueue;

  time_t _currentTime;
  int _currentNn;
  vector<InputFile *> _inputFiles;

  int _volNum;
  int _nTilts;
  int _nAz;
  int _nGates;
  int _nFields;
  int _outputByteWidth;
  double _elev;
  
  int _processFile(const char *inputPath);
  int _parseFileName(const string &fileName, time_t &fileTime,
                     int &num, string &fieldName);
  int _processCurrent();
  int _writeOut();
  int _writeParams(int tiltNum);
  int _writeBeams(int tiltNum, time_t startTime, time_t endTime);
  bool _allFieldsPresent();
  int _computeNTilts();
  int _computeTiltParams(int tiltNum);
  
};

#endif

