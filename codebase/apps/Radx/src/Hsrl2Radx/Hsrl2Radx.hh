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
// Hsrl2Radx.hh
//
// Hsrl2Radx object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2015
//
///////////////////////////////////////////////////////////////

#ifndef Hsrl2Radx_HH
#define Hsrl2Radx_HH

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <Radx/RadxTime.hh>
#include "CalReader.hh"
#include <Radx/Radx.hh>
class RadxVol;
class RadxFile;
class MslFile;
using namespace std;


class Hsrl2Radx {
  
public:

  // constructor
  
  Hsrl2Radx (int argc, char **argv);

  // destructor
  
  ~Hsrl2Radx();

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
  vector< vector<double> > blCor;
  vector< vector<double> > diffDGeoCor;
  vector< vector<double> > geoDefCor;
  vector< vector<double> > afPulCor;
  CalReader hi_CR_DT, lo_CR_DT, cross_CR_DT, mol_CR_DT, binWidth_CR;
  //holds the deadtime calibration blocks
  int hi_pos,lo_pos,cross_pos,mol_pos,binWidth_pos;
  //hold which spot in the vectors are the correct calibration point. 

  int _runFilelist();
  int _runArchive();
  int _runRealtimeWithLdata();
  int _runRealtimeNoLdata();
  int _processFile(const string &filePath);
  int _processUwCfRadialFile(const string &filePath);
  void _setupRead(MslFile &file);
  void _overrideGateGeometry(RadxVol &vol);
  void _setRangeRelToInstrument(MslFile &file,
                                RadxVol &vol);
  void _convertFields(RadxVol &vol);
  void _setupWrite(RadxFile &file);
  void _setGlobalAttr(RadxVol &vol);
  int _writeVol(RadxVol &vol);

  int _processUwRawFile(const string &filePath);
  void _addEnvFields(RadxVol &vol);
  void _addDerivedFields(RadxVol &vol);

  
  double _nonLinCountCor(Radx::fl32 count, double deadtime, 
			     double binWid, double shotCount);
  double _baselineSubtract(double arrivalRate, double profile, 
			       double polarization);
  double _backgroundSub(double arrivalRate, double backgroundBins);
  double _energyNorm(double arrivalRate, double totalEnergy);
  vector<double> _diffOverlapCor(vector<double> arrivalRate, vector<double> diffOverlap);
  vector<double> _processQWPRotation(vector<double>arrivalRate, vector<double> polCal);
  double _hiAndloMerge(double hiRate, double loRate);
  double _geoOverlapCor(double arrivalRate, double geoOverlap);
  double _volDepol(double crossRate, double combineRate);
  double _backscatRatio(double combineRate, double molRate);
  double _partDepol(double volDepol, double backscatRatio);
  double _backscatCo(double pressure, double temp, 
			 double backscatRatio);
  
};

#endif

