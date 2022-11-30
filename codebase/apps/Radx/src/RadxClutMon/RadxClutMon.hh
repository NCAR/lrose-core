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
// RadxClutMon.hh
//
// RadxClutMon object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2015
//
///////////////////////////////////////////////////////////////
//
// Monitors clutter, to check radar calibration over time
//
////////////////////////////////////////////////////////////////

#ifndef RadxClutMon_HH
#define RadxClutMon_HH

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <Radx/RadxVol.hh>
class RadxFile;
class RadxRay;
using namespace std;

class RadxClutMon {
  
public:

  // constructor
  
  RadxClutMon (int argc, char **argv);

  // destructor
  
  ~RadxClutMon();

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
  vector<string> _readPaths;

  string _clutMapPath;
  string _momentsPath;

  RadxVol _clutMapVol;
  RadxVol _momentsVol;

  int _nDbzStrong, _nDbzWeak;
  int _nDbmhcStrong, _nDbmhcWeak;
  int _nDbmvcStrong, _nDbmvcWeak;
  int _nDbmhxStrong, _nDbmhxWeak;
  int _nDbmvxStrong, _nDbmvxWeak;
  int _nZdrStrong, _nZdrWeak;
  int _nXpolrStrong, _nXpolrWeak;

  double _sumDbzStrong, _sumDbzWeak;
  double _sumDbmhcStrong, _sumDbmhcWeak;
  double _sumDbmvcStrong, _sumDbmvcWeak;
  double _sumDbmhxStrong, _sumDbmhxWeak;
  double _sumDbmvxStrong, _sumDbmvxWeak;
  double _sumZdrStrong, _sumZdrWeak;
  double _sumXpolrStrong, _sumXpolrWeak;

  double _meanDbzStrong, _meanDbzWeak;
  double _meanDbmhcStrong, _meanDbmhcWeak;
  double _meanDbmvcStrong, _meanDbmvcWeak;
  double _meanDbmhxStrong, _meanDbmhxWeak;
  double _meanDbmvxStrong, _meanDbmvxWeak;
  double _meanZdrStrong, _meanZdrWeak;
  double _meanXpolrStrong, _meanXpolrWeak;

  int _nGatesStrong;
  int _nGatesWeak, _nWxWeak;
  double _fractionWxWeak;

  double _xmitPowerDbmH, _xmitPowerDbmV, _xmitPowerDbmBoth;

  int _runFilelist();
  int _runArchive();
  int _runRealtime();

  int _readMoments(const string &filePath);
  int _readClutterMap();

  int _processVol();
  
  int _processRay(const RadxRay &clutMapRay,
                  const RadxRay &momentsRay);
  
  int _checkGeom(const RadxVol &clutMapVol,
                 const RadxVol &momentsVol);

  void _initForStats();
  void _computeStats();
  void _printStats(ostream &out);
  int _writeStatsToSpdb();

};

#endif
