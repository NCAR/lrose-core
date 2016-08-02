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
// RadxCov2Mom.hh
//
// RadxCov2Mom object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2011
//
///////////////////////////////////////////////////////////////
//
// RadxCov2Mom reads covariances in Radx-supported format files,
// computes the moments and writes out the results to
// Radx-supported format files.
//
///////////////////////////////////////////////////////////////

#ifndef RadxCov2Mom_H
#define RadxCov2Mom_H

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <deque>
#include <set>
#include <radar/RadarMoments.hh>
#include <radar/AlternatingVelocity.hh>
#include <radar/InterestMap.hh>
#include <radar/NoiseLocator.hh>
#include <radar/IwrfCalib.hh>
#include <radar/KdpBringi.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxRcalib.hh>
class RadxVol;
class RadxFile;
class RadxRay;
class RadxRcalib;
class RadxField;
class IwrfCalib;
class Moments;
class ComputeThread;
using namespace std;

class RadxCov2Mom {
  
public:

  // constructor
  
  RadxCov2Mom (int argc, char **argv);

  // destructor
  
  ~RadxCov2Mom();

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

  // radar parameters

  double _wavelengthM;
  double _radarHtKm;
  
  // moments computations object

  vector<Moments *> _moments;

  // transmit power

  double _measXmitPowerDbmH;
  double _measXmitPowerDbmV;

  // ZDR temperature correction

  double _zdrTempCorrDb;
  double _siteTempC;
  time_t _timeForSiteTemp;

  // noise stats

  int _nNoiseDbmHc;
  int _nNoiseDbmVc;
  int _nNoiseDbmHx;
  int _nNoiseDbmVx;

  double _sumNoiseDbmHc;
  double _sumNoiseDbmVc;
  double _sumNoiseDbmHx;
  double _sumNoiseDbmVx;

  double _meanNoiseDbmHc;
  double _meanNoiseDbmVc;
  double _meanNoiseDbmHx;
  double _meanNoiseDbmVx;

  // threading

  deque<ComputeThread *> _activeThreads;
  deque<ComputeThread *> _availThreads;
  pthread_mutex_t _debugPrintMutex;

  // calibrations

  class Calib {
  public:
    double pulseWidthUs;
    IwrfCalib referenceCal;
    IwrfCalib workingCal;
    RadxRcalib radxCal;
    bool used;
  };

  vector<Calib> _calibs;
  
  // private methods
  
  int _runFilelist();
  int _runArchive();
  int _runRealtime();
  void _setupRead(RadxFile &file);
  void _setupWrite(RadxFile &file);
  int _writeVol(RadxVol &vol);
  int _processFile(const string &filePath);
  void _encodeFieldsForOutput(RadxVol &vol);
  
  string _loadProcXml();
  int _readCalFiles();
  void _initCals(const RadxVol &vol);
  int _loadZdrTempCorrection(time_t volTime);
  const IwrfCalib &_getBestCal(double pulseWidthUs);
  
  int _computeMoments(RadxVol &vol);
  int _computeMomentsSingleThreaded(RadxVol &vol,
                                    const vector<RadxRay *> &covRays,
                                    vector <RadxRay *> &momRays);
  int _computeMomentsMultiThreaded(RadxVol &vol,
                                   const vector<RadxRay *> &covRays,
                                   vector <RadxRay *> &momRays);
  static void *_computeMomentsInThread(void *thread_data);

  int _addEchoFields(const vector<RadxRay *> &covRays,
                     vector <RadxRay *> &momRays);

  void _loadMeasuredXmitPower(const RadxVol &vol);

  int _addMergedFields(RadxVol &primaryVol);
  void _setupMergeRead(RadxFile &mergeFile);
  int _mergeVol(const RadxVol &primaryVol, RadxVol &secondaryVol);
  int _mergeVolPpi(const RadxVol &primaryVol, RadxVol &secondaryVol);
  int _mergeVolRhi(const RadxVol &primaryVol, RadxVol &secondaryVol);
  void _mergeRay(RadxRay &primaryRay, const RadxRay &secondaryRay);

  void _initNoiseStats();
  void _addToNoiseStats(Moments &mom);
  void _computeNoiseStats();

  int _writeStatusXmlToSpdb(const RadxVol &vol,
                            const string &xml);

};

#endif
