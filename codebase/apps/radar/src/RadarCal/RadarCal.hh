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
// RadarCal.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2006
//
///////////////////////////////////////////////////////////////

#ifndef RadarCal_H
#define RadarCal_H

#include <string>
#include <vector>
#include <deque>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <toolsa/DateTime.hh>

using namespace std;

////////////////////////
// This class

class RadarCal {
  
public:

  // constructor

  RadarCal (int argc, char **argv);

  // destructor
  
  ~RadarCal();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  // private classes for holding results

  class ChannelResult {
  public:
    double noiseDbm;
    double gainDbm;
    double slope;
    double corr;
    double stdErrEst;
    ChannelResult() {
      noiseDbm = -9999;
      gainDbm = -9999;
      slope = -9999;
      corr = -9999;
      stdErrEst = -9999;
    }
  };

  class Result {
  public:
    DateTime time;
    bool switchPos;
    double el;
    double az;
    double splitterPowerDbm;
    double tempPowerSensor;
    double tempLnaH;
    double tempLnaV;
    double tempIfd;
    double tempSiggen;
    double tempInside;
    double tempAmp;
    ChannelResult hc;
    ChannelResult hx;
    ChannelResult vc;
    ChannelResult vx;
  };

  //////////////
  // data members

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // calibration time

  time_t _calTime;
  bool _switchPos;
  double _el, _az;
  double _splitterPower;
  double _tempLnaH, _tempLnaV, _tempPowerSensor;
  double _tempIfd, _tempSiggen, _tempInside, _tempAmp;
  vector<double> _rawInjectedDbm, _waveguideDbmH, _waveguideDbmV;
  vector<double> _hcDbm, _hxDbm, _vcDbm, _vxDbm;

  // power ratios

  vector<Result> _results;

  // methods

  int _processFile(const char* filePath);
  int _readFile(const char* filePath);

  void _computeCal(const string &label,
                   const vector<double> &inputDbm,
                   const vector<double> &outputDbm,
                   ChannelResult &result);

  int _writeResults();

  int _appendToDiffsFile(const string &label,
                         const vector<double> &first,
                         const vector<double> &second);
  
  int _linearFit(const vector<double> &x,
                 const vector<double> &y,
                 double &gain,
                 double &slope,
                 double &xmean,
                 double &ymean,
                 double &xsdev,
                 double &ysdev,
                 double &corr,
                 double &stdErrEst,
                 double &rSquared);

};

#endif
