/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/////////////////////////////////////////////////////////////
// DspDriver.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// Controls the Digital Signal Processor
//
////////////////////////////////////////////////////////////////

#ifndef DspDriver_HH
#define DspDriver_HH

#include <string>
#include <dsp_lib.h>
using namespace std;

class DspDriver {
  
public:
  
  // Constructor
  
  DspDriver(bool dsp_available);
  
  // destructor
  
  ~DspDriver();

  // set debug

  void setDebug(bool state = true) { _debug = state; }
  void setVerbose(bool state = true) { _verbose = state; }
  
  // initialize
  // returns 0 on success, -1 on failure
  
  int init();

  // close dsp

  void close();
  
  // reset the FIFO
  // returns 0 on success, -1 on failure
  
  int resetFifo();
  
  // print state
  
  void print(ostream &out);

  // set proc

  int setProc(struct dsp_data_mask &fieldMask,
              int procMode);

  // set options

  int setOptions(int nSamples,
                 int majorMode,
                 int procFlags,
                 int prfMode,
                 int windowType,
                 int rangeSmooth);

  // range mask

  int setRangeMask(int nGates, double gateSpacing);
  
  // set the PRF
  
  int setPrf(double prf);
  
  // set the phase sequence (random, sz864 etc)

  int setPhaseCoding(int phaseCoding);
  
  // set the clutter filter number

  int setClutFiltNum(int clutFiltNum);

  // set the polarization

  int setPolarization(int polarization);

  // set the angle synchronization mode

  int setAngSyncMode(int angSyncMode);
  
  // Check the DSP status
  // Returns 0 on success, -1 on failure

  int checkStatus();

  // Read parameters from DSP
  // Returns 0 on success, -1 on failure

  int readParms();

  // Read data available from DSP
  // Returns 0 on success, -1 on failure

  int readAvailable();

  // get number of fields from the field mask
  
  int getNFields(struct dsp_data_mask &fieldMask);

protected:
  
private:

  bool _dspAvailable;
  bool _debug;
  bool _verbose;

  static const int MAXREADWORDS = 500000;
  UINT2 *_readBuf;

  static const int HDRSIZE = 70;
  int _nFields;
  int _nGates;
  
  // Dsp handle

  Dsp *_dsp;
  const struct dsp_manual_setup *_dspSetup;

  MESSAGE _antPowerUpMain( SINT4 iClientType_a, SINT4 iOptions_a );

};

#endif
