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
// AcGeorefCompare.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2016
//
///////////////////////////////////////////////////////////////
//
// AcGeorefCompare reads multiple ac georef data sets from SPDB
// and compares them. It is designed to compare the NCAR GV INS
// with the HCR Gmigits unit.
//
////////////////////////////////////////////////////////////////

#ifndef AcGeorefCompare_H
#define AcGeorefCompare_H

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <rapformats/ac_georef.hh>
#include <Spdb/DsSpdb.hh>

using namespace std;

////////////////////////
// This class

class AcGeorefCompare {
  
public:

  // constructor

  AcGeorefCompare (int argc, char **argv);

  // destructor
  
  ~AcGeorefCompare();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  Args _args;
  char *_paramsPath;
  Params _params;

  static double _missingDbl;
  int _lineCount;

  time_t _startTime, _endTime;

  double _takeoffWtKg;
  double _aircraftWtKg;
  double _fuelRateClimbKgPerSec;
  double _fuelRateCruiseKgPerSec;
  bool _inInitialClimb;
  double _initialClimbSecs;
  double _topOfClimbAltitudeM;
  
  vector<ac_georef_t> _georefsPrimary;
  vector<ac_georef_t> _georefsSecondary;

  // methods

  int _runTimeSeriesTable();
  int _runSinglePeriodArchive();
  int _runSinglePeriodRealtime();

  int _retrieveTimeBlock(time_t startTime, time_t endTime);
  int _produceTimeSeriesTable();
  int _produceSinglePeriodProduct();

  void _computeMeanGeorefs(const vector<ac_georef_t> &vals,
                           ac_georef_t &mean);

  void _analyzeGeorefPair(const ac_georef_t &georefPrim,
                          const ac_georef_t &georefSec,
                          const DateTime &timePrim,
                          double timeDiff);

  string _formatVal(double val, const char * format);

  void _writeCommentedHeader(FILE *out);

};

#endif
