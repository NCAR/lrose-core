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
// MdvPull.hh
//
// MdvPull object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2011
//
///////////////////////////////////////////////////////////////
//
// MdvPull reads data from a remote server, and duplicates it on
// a local host or a target server.
//
////////////////////////////////////////////////////////////////

#ifndef MdvPull_H
#define MdvPull_H

#include <Ncxx/Nc3File.hh>
#include <string>
#include "Args.hh"
#include "Params.hh"
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxRemapLut.hh>
#include <Mdv/Mdv2NcfTrans.hh>

using namespace std;

////////////////////////
// This class

class MdvPull {
  
public:

  // constructor

  MdvPull (int argc, char **argv);

  // destructor
  
  ~MdvPull();

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

  time_t _nextSearchTime;
  time_t _startTime, _endTime;

  vector<time_t> _sourceValidTimes;
  vector<time_t> _sourceGenTimes;
  vector<vector<time_t> > _sourceForecastTimesArray;

  vector<time_t> _outputValidTimes;
  vector<time_t> _outputGenTimes;
  vector<vector<time_t> > _outputForecastTimesArray;

  MdvxRemapLut _remapLut;

  int _runRealtime();
  int _runArchive();

  int _retrieveData();
  int _retrieveValidData();
  int _retrieveForecastData();

  int _retrieveForValidTime(time_t validTime);
  int _retrieveForGenAndForecastTime(time_t genTime,
                                     time_t forecastTime);

  int _compileSourceTimeList();
  int _compileOutputTimeList();

  void _setupRead(DsMdvx &mdvx);
  
  void _remap(DsMdvx &mdvx);

  void _autoRemapToLatLon(DsMdvx &mdvx);

  void _applyTransform(DsMdvx &mdvx);

  void _convertOutput(DsMdvx &mdvx);

  void _renameFields(DsMdvx &mdvx);

  void _setupWrite(DsMdvx &mdvx);
  int _performWrite(DsMdvx &mdvx);
  string _computeNcfOutputPath(const DsMdvx &mdvx,
                               string &outputDir);
  void _writeNcfLdataInfo(const DsMdvx &mdvx,
                          const string &outputDir,
                          const string &outputPath);

};

#endif

