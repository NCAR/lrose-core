// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2018                                         
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
// WorkerThread.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2018
//
///////////////////////////////////////////////////////////////
//
// Thread class for handling computations
//
///////////////////////////////////////////////////////////////

#ifndef WorkerThread_HH
#define WorkerThread_HH

#include <toolsa/TaThread.hh>

class RadxRay;
class RadxKdp;
class Worker;
class Params;
class KdpFiltParams;

class WorkerThread : public TaThread
{  

public:
  
  // constructor
  
  WorkerThread(RadxKdp *parent, 
               const Params &params,
               const KdpFiltParams &kdpFiltParams,
               int threadNum);

  // destructor
  
  virtual ~WorkerThread();

  // compute engine object
  
  inline Worker *getWorker() const { return _worker; }
  
  // set input ray
  
  inline void setInputRay(RadxRay *val) { _inputRay = val; }
  
  // derived ray - result of computations
  
  inline RadxRay *getOutputRay() const { return _outputRay; }

  // override run method

  virtual void run();

  // constructor OK?

  bool OK;

private:

  // parent object

  RadxKdp *_parent;

  // params

  const Params &_params;
  const KdpFiltParams &_kdpFiltParams;

  // thread number

  int _threadNum;

  // computation engine

  Worker *_worker;

  // input ray

  RadxRay *_inputRay;

  // output ray

  RadxRay *_outputRay;

};

#endif
