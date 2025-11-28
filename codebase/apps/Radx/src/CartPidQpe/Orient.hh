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
// Orient.hh
//
// Orient class.
// Compute echo orientation:
//   vertical (convective)
//   horizontal (stratiform, bright-band, anvil)
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2020
//
///////////////////////////////////////////////////////////////

#ifndef Orient_HH
#define Orient_HH

#include <toolsa/TaThread.hh>
#include <toolsa/TaThreadPool.hh>
#include <radar/ConvStratFinder.hh>
#include "BaseInterp.hh"
#include "Params.hh"

class RadxVol;
class DsMdvx;
class RhiOrient;

class Orient {

public:

  // constructor

  Orient(const Params &params,
         RadxVol &readVol,
         size_t gridNx,
         size_t gridNy,
         const vector<double> &gridZLevels);

  // destructor
  
  virtual ~Orient();

  // determine the echo orientation

  int findEchoOrientation();
  
  // load the sdev fields on the cartesian grid

  void loadSdevFields(BaseInterp::GridLoc ****gridLoc,
                      BaseInterp::DerivedField *sdevDbzH,
                      BaseInterp::DerivedField *sdevDbzV);

  // set RHI mode

  void setRhiMode(bool state) { _rhiMode = state; }
  
protected:
private:

  const Params &_params;
  RadxVol &_readVol;
  bool _rhiMode;
  
  // checking timing performance
  
  struct timeval _timeA;

  // synthetic rhis

  double _startAz;
  double _deltaAz;
  vector<RhiOrient *> _rhis;

  // radar altitude
  
  double _radarAltKm;
  
  // gate geometry
  
  int _nGates;
  double _startRangeKm;
  double _gateSpacingKm;
  double _maxRangeKm;

  // output grid geometry

  size_t _gridNx, _gridNy, _gridNz;
  vector<double> _gridZLevels;

  // private methods

  void _createThreads();
  void _freeThreads();
  
  void _computeOrientInRhis();
  void _computeOrientMultiThreaded();
  void _computeOrientSingleThreaded();

  int _setRadarParams();
  
  void _printRunTime(const string& str, bool verbose = false);
  
  //////////////////////////////////////////////////////////////
  // Classes for threads
  //////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////
  // inner thread class for computing orientation in RHI
  
  class ComputeOrientInRhi : public TaThread
  {  
  public:
    // constructor
    ComputeOrientInRhi(Orient *obj);
    // set the RHI index to be used
    inline void setRhiIndex(size_t index) { _index = index; }
    // override run method
    virtual void run();
  private:
    Orient *_this; // context
    size_t _index; // index of RHI to be used
  };
  // instantiate thread pool for grid relative computations
  TaThreadPool _threadPoolOrientInRhi;

};

#endif
