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
// SpolCal2Xml.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2010
//
///////////////////////////////////////////////////////////////
//
// SpolCal2Xml reads an SPOL ATE calibration file, analyzes it,
// and produces an XML file in DsRadarCal format
//
////////////////////////////////////////////////////////////////

#ifndef SpolCal2Xml_H
#define SpolCal2Xml_H

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>

using namespace std;

////////////////////////
// This class

class SpolCal2Xml {
  
public:

  // constructor

  SpolCal2Xml (int argc, char **argv);

  // destructor
  
  ~SpolCal2Xml();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const double piCubed;
  static const double lightSpeed;
  static const double kSquared;

  //////////////
  // data members

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  DsInputPath *_reader;

  time_t _calTime;
  double _radarConstH;
  double _radarConstV;
  
  vector<double> _siggenDbm, _waveguideDbmH, _waveguideDbmV;
  vector<double> _hcDbm, _hxDbm, _vcDbm, _vxDbm;
  vector<double> _hcDbmNs, _hxDbmNs, _vcDbmNs, _vxDbmNs;
  vector<double> _hcMinusVcDbm, _hxMinusVxDbm;

  // results

  class ChannelResult {
  public:
    double noiseDbm;
    double gainDbm;
    double slope;
    double corr;
    double stdErrEst;
    double radarConstant;
    double dbz0;
    ChannelResult() {
      noiseDbm = -9999;
      gainDbm = -9999;
      slope = -9999;
      corr = -9999;
      stdErrEst = -9999;
    }
  };

  ChannelResult _resultHc;
  ChannelResult _resultHx;
  ChannelResult _resultVc;
  ChannelResult _resultVx;

  // methods

  int _processFile(const char* filePath);
  int _readCal(const char* filePath);
  
  double _computeRadarConstant(double xmitPowerDbm,
                               double antennaGainDb,
                               double twoWayWaveguideLossDb,
                               double twoWayRadomeLossDb);

  void _computeCal(const string &label,
                   const vector<double> &inputDbm,
                   const vector<double> &outputDbm,
                   vector<double> &outputDbmNs,
                   ChannelResult &result,
                   double radarConst);

  int _writeResults();

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
