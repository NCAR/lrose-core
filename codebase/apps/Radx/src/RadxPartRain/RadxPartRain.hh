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
// RadxPartRain.hh
//
// RadxPartRain object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////
//
// RadxPartRain reads moments from Radx-supported format files, 
// computes the PID and PRECIP rates and writes out the results 
// to Radx-supported format files
//
///////////////////////////////////////////////////////////////

#ifndef RadxPartRain_H
#define RadxPartRain_H

#include "Args.hh"
#include "Params.hh"
#include "ComputeEngine.hh"
#include <string>
#include <deque>
#include <radar/NoiseLocator.hh>
#include <radar/KdpBringi.hh>
#include <radar/TempProfile.hh>
#include <Radx/RadxArray.hh>
class RadxVol;
class RadxFile;
class RadxRay;
class RadxField;
class ComputeThread;
using namespace std;

class RadxPartRain {
  
public:

  // constructor
  
  RadxPartRain (int argc, char **argv);

  // destructor
  
  ~RadxPartRain();

  // run 

  int Run();

  // data members

  int OK;

  // get methods for threading

  const Params &getParams() const { return _params; }
  pthread_mutex_t *getDebugPrintMutex() { return &_debugPrintMutex; }

protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  vector<string> _readPaths;

  // computations object

  ComputeEngine *_engine;

  // derived rays - after compute

  vector <RadxRay *> _derivedRays;

  // transitions - same size as derivedRays

  vector<bool> _transitionFlags;

  // radar properties

  double _radarHtKm;
  double _wavelengthM;

  // temperature profile from sounding, if appropriate

  TempProfile _tempProfile;

  // stats for ZDR bias

  class ZdrStats {
  public:
    void clear() {
      count = 0.0;
      sum = 0.0;
      mean = NAN;
      percentiles.clear();
    }
    double sum;
    double count;
    double mean;
    vector<double> percentiles;
  };
  ZdrStats _zdrmStatsIce;
  ZdrStats _zdrmStatsBragg;
  ZdrStats _zdrStatsIce;
  ZdrStats _zdrStatsBragg;
  
  vector<double> _zdrInIceResults;
  vector<double> _zdrInBraggResults;
  vector<double> _zdrmInIceResults;
  vector<double> _zdrmInBraggResults;

  // site temp

  double _siteTempC;
  time_t _timeForSiteTemp;

  // self consistency

  vector<ComputeEngine::self_con_t> _selfConResults;

  // threading
  
  deque<ComputeThread *> _activeThreads;
  deque<ComputeThread *> _availThreads;
  pthread_mutex_t _debugPrintMutex;
  
  // private methods
  
  int _runFilelist();
  int _runArchive();
  int _runRealtime();
  void _setupRead(RadxFile &file);
  void _setupWrite(RadxFile &file);
  int _writeVol(RadxVol &vol);
  int _processFile(const string &filePath);
  void _encodeFieldsForOutput(RadxVol &vol);
  
  int _compute(RadxVol &vol);
  int _computeSingleThreaded(RadxVol &vol);
  int _computeMultiThreaded(RadxVol &vol);
  int _storeDerivedRay(ComputeThread *thread);
  static void *_computeInThread(void *thread_data);

  void _findTransitions(vector<RadxRay *> &rays);

  int _retrieveTempProfile(const RadxVol &vol);
  int _retrieveSiteTempFromSpdb(const RadxVol &vol,
                                double &tempC,
                                time_t &timeForTemp);

  void _computeZdrBias(const RadxVol &vol);

  void _loadZdrResults(vector<double> &results,
                       ZdrStats &stats,
                       int nPercentiles,
                       double *percentiles);

  double _computeZdrPerc(const vector<double> &zdrmResults,
                         double percent);

  void _computeSelfConZBias(const RadxVol &vol);

};

#endif
