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
// June 2015
// 
// Brad Schoenrock, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// updated Mar 2017
//
///////////////////////////////////////////////////////////////

#ifndef Hsrl2Radx_HH
#define Hsrl2Radx_HH

#include "Args.hh"
#include "Params.hh"
#include <string>
#include "CalReader.hh"
#include "FullCals.hh"
#include <Radx/Radx.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxVol.hh>
#include <physics/IcaoStdAtmos.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>

class DerFieldCalcs;

class RadxRay;
class RadxFile;
class HsrlRawRay;
class MslFile;
class DsFmq;
class OutputFmq;

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
  FullCals _cals;

  IcaoStdAtmos _stdAtmos;

  DsMdvx _mdvx;
  MdvxField *_modelTempField, *_modelPresField;
  fl32 *_modelTempData, *_modelPresData;
  Mdvx::field_header_t _modelTempFhdr;
  Mdvx::vlevel_header_t _modelTempVhdr;
  MdvxProj _modelProj;

  int _nBinsInRay;
  int _nBinsPerGate;
  int _nGates;

  double _gateSpacingKm;
  double _startRangeKm;

  DerFieldCalcs *_calcs;
  DerFieldCalcs *_calcsFilt;

  // output data if split on time

  RadxTime _nextEndOfVolTime;
  RadxVol _splitVol;

  // methods

  int _readCals();
  int _runFilelist();
  int _runArchive();
  int _runRealtimeWithLdata();
  int _runRealtimeNoLdata();
  int _runRealtimeFmq();

  int _readFmq(DsFmq &inputFmq,
               OutputFmq &outputFmq);

  RadxRay *_convertRawToRadx(HsrlRawRay &rawRay);

  void _addRawFieldToRay(RadxRay *ray,
                         const string &name,
                         const string &units,
                         const string &standardName,
                         const Radx::fl32 *fcounts);
  
  int _processFile(const string &filePath);
  int _processUwCfRadialFile(const string &filePath);
  void _setupRead(MslFile &file);
  void _overrideGateGeometry(RadxVol &vol);
  void _setRangeRelToInstrument(MslFile &file,
                                RadxVol &vol);
  void _setupWrite(RadxFile &file);
  void _setGlobalAttr(RadxVol &vol);
  int _writeVol(RadxVol &vol);
  int _writeSplitOnTime(RadxVol &vol);
  int _writeSplitVol();
  void _setNextEndOfVolTime(RadxTime &refTime);
  void _censorGatesBelowSurface(RadxVol &vol);

  int _processUwRawFile(const string &filePath);
  int _processMattNcFile(const string &filePath);
  void _addEnvFields(RadxRay *ray);
  
  double _nonLinCountCor(Radx::fl32 count, double deadtime, 
                         double binWid, double shotCount);
  double _baselineSubtract(double arrivalRate, double profile, 
                           double polarization);
  double _backgroundSub(double arrivalRate, double backgroundBins);
  double _energyNorm(double arrivalRate, double totalEnergy);
  vector<double> _diffOverlapCor(vector<double> arrivalRate, vector<double> diffOverlap);
  vector<double> _processQWPRotation(vector<double>arrivalRate, vector<double> polCal);

  void _addDerivedMoments(RadxRay *ray);

  int _getModelData(time_t rayTime);

  int _setProfileFromModel(RadxRay *ray,
                           Radx::fl32 *htMeters,
                           Radx::fl32 *tempK,
                           Radx::fl32 *presHpa);

  // void _addFilteredCountsToRay(RadxRay *ray);
  
  // void _addFilteredCountsToRay(RadxRay *ray,
  //                              const string &name,
  //                              const string &filteredName);
  
  // void _setZeroCountsToMissing(RadxRay *ray);
  // void _setZeroCountsToMissing(RadxRay *ray, const string &name);
  
  // void _addFilteredMoments(RadxRay *ray);

  // void _applyMissingSpeckleFilter(int nGates, int minRunLen, Radx::fl32 *data);
  // void _applyZeroSpeckleFilter(int nGates, int minRunLen, Radx::fl32 *data);
  
  // double _hiAndloMerge(double hiRate, double loRate);
  // double _geoOverlapCor(double arrivalRate, double geoOverlap);
  // double _volDepol(double crossRate, double combineRate);
  // double _backscatRatio(double combineRate, double molRate);
  // double _partDepol(double volDepol, double backscatRatio);
  // double _backscatCo(double pressure, double temp, 
  //       		 double backscatRatio);

};

#endif

