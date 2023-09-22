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
// RadxQc.hh
//
// RadxQc object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2016
//
///////////////////////////////////////////////////////////////
//
// RadxQc reads moments from Radx-supported format files, and
// performs Quality Control operations on the data. It
// optionally adds QC fields to the output, and optionally
// censors the input data based on the QC results.
//
///////////////////////////////////////////////////////////////

#ifndef RadxQc_H
#define RadxQc_H

#include "Args.hh"
#include "Params.hh"
#include "ComputeEngine.hh"
#include <string>
#include <deque>
#include <toolsa/TaThread.hh>
#include <toolsa/TaThreadPool.hh>
#include <radar/TempProfile.hh>
#include <radar/BeamHeight.hh>
#include <Radx/RadxArray.hh>
class RadxVol;
class RadxFile;
class RadxRay;
class RadxField;
using namespace std;

class RadxQc {
  
public:

  // constructor
  
  RadxQc (int argc, char **argv);

  // destructor
  
  ~RadxQc();

  // run 

  int Run();

  // data members

  int OK;

  // get methods

  const Params &getParams() const { return _params; }

protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  vector<string> _readPaths;

  // computations object

  ComputeEngine *_engineSingle;

  // derived rays - after compute

  vector <RadxRay *> _derivedRays;

  // transitions - same size as derivedRays

  vector<bool> _transitionFlags;

  // radar properties

  double _radarHtKm;
  double _wavelengthM;
  BeamHeight _beamHt;

  // temperature profile from sounding, if appropriate

  TempProfile _tempProfile;

  // site temp

  double _siteTempC;
  time_t _timeForSiteTemp;

  // checking timing performance

  struct timeval _timeA;

  //////////////////////////////////////////////////////////////
  // inner thread class for calling Moments computations
  
  pthread_mutex_t _debugPrintMutex;
  
  class ComputeThread : public TaThread
  {  
  public:
    // constructor
    ComputeThread(RadxQc *obj, 
                  const Params &params,
                  int threadNum);
    // destructor
    virtual ~ComputeThread();
    // compute engine object
    inline ComputeEngine *getComputeEngine() const { return _engine; }
    // set input ray
    inline void setInputRay(RadxRay *val) { _inputRay = val; }
    // derived ray - result of computations
    inline RadxRay *getDerivedRay() const { return _derivedRay; }
    // override run method
    virtual void run();
    // constructor OK?
    bool OK;
  private:
    // parent object
    RadxQc *_this;
    // params
    const Params &_params;
    // thread number
    int _threadNum;
    // computation engine
    ComputeEngine *_engine;
    // input ray
    RadxRay *_inputRay;
    // result of computation - ownership gets passed to parent
    RadxRay *_derivedRay;
  };
  // instantiate thread pool for computations
  TaThreadPool _threadPool;

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

  void _findTransitions(vector<RadxRay *> &rays);

  int _retrieveTempProfile(const RadxVol &vol);
  int _retrieveSiteTempFromSpdb(const RadxVol &vol,
                                double &tempC,
                                time_t &timeForTemp);

  int _addHeightField(RadxVol &vol);
  int _computeDbzGradient(RadxVol &vol);
  int _computeDbzGradient(RadxRay &lowerRay, RadxRay &upperRay);
  void _copyDbzGradient(const RadxRay &lowerRay, RadxRay &upperRay);
  void _printRunTime(const string& str);
  
};

#endif
