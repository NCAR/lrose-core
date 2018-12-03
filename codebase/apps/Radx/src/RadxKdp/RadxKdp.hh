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
// RadxKdp.hh
//
// RadxKdp object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2018
//
///////////////////////////////////////////////////////////////
//
// RadxKdp reads moments from Radx-supported format files, 
// computes the KDP and attenuation and writes out the results 
// to Radx-supported format files
//
///////////////////////////////////////////////////////////////

#ifndef RadxKdp_H
#define RadxKdp_H

#include "Args.hh"
#include "Params.hh"
#include "ComputeEngine.hh"
#include <string>
#include <deque>
#include <toolsa/TaThread.hh>
#include <toolsa/TaThreadPool.hh>
#include <radar/KdpFiltParams.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxArray.hh>
class RadxVol;
class RadxFile;
class RadxRay;
class RadxField;
using namespace std;

class RadxKdp {
  
public:

  // constructor
  
  RadxKdp (int argc, char **argv);

  // destructor
  
  ~RadxKdp();

  // run 

  int Run();

  // data members

  int OK;

  // get methods for threading

  const Params &getParams() const { return _params; }
  const KdpFiltParams &getKdpFiltParams() const { return _kdpFiltParams; }

protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  KdpFiltParams _kdpFiltParams;
  vector<string> _readPaths;

  // radar volume container
  
  RadxVol _vol;

  // computations object for single threading

  ComputeEngine *_engineSingle;

  // derived rays - after compute

  vector <RadxRay *> _derivedRays;

  // radar properties

  double _wavelengthM;

  //////////////////////////////////////////////////////////////
  // inner thread class for calling Moments computations
  
  pthread_mutex_t _debugPrintMutex;
  
  class ComputeThread : public TaThread
  {  
  public:
    // constructor
    ComputeThread(RadxKdp *obj, 
                  const Params &params,
                  const KdpFiltParams &kdpFiltParams,
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
    RadxKdp *_this;
    // params
    const Params &_params;
    const KdpFiltParams &_kdpFiltParams;
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
  int _writeVol();
  int _processFile(const string &filePath);
  void _encodeFieldsForOutput();
  
  int _compute();
  int _computeSingleThreaded();
  int _computeMultiThreaded();
  int _storeDerivedRay(ComputeThread *thread);

};

#endif
