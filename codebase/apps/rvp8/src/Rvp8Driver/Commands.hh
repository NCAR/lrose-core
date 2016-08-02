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
// Commands.hh
//
// Commands object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////
//
// Current configuration of RVP8.
// Keeps rvp8 configuration state, provides services.
//
////////////////////////////////////////////////////////////////

#ifndef Commands_HH
#define Commands_HH

#include <string>
#include <sigtypes.h>
#include <dsp.h>
class DspDriver;
using namespace std;

class Commands {
  
public:
  
  // Constructor
  
  Commands();
  
  // destructor
  
  ~Commands();

  // set debug

  void setDebug(bool state = true) { _debug = state; }
  void setVerbose(bool state = true) { _verbose = state; }

  // set state
  
  void set(const string &xmlBuf);
  
  // set all change flags
  
  void setChangeFlags(bool state);

  // configure the DSP based on the command state

  int configureDsp(DspDriver *dspDriver);

  // print state
  
  void print(ostream &out);

  // get commands in XML format

  void getCommandXml(string &xml);

  // get simulated status in XML format

  void getSimulatedStatusXml(string &xml);

  // convert enums to strings

  static string fieldMask2String(const struct dsp_data_mask &fieldMask);
  static string majorMode2String(int majorMode);
  static string procFlags2SpeckleFiltString(int procFlags);
  static string prfMode2String(int prfMode);
  static string windowType2String(int windowType);
  static string procMode2String(int procMode);
  static string phaseCoding2String(int phaseCoding);
  static string polarization2String(int polarization);
  static string angSyncMode2String(int angSyncMode);

protected:
  
private:

  bool _debug;
  bool _verbose;
  bool _firstPass;

  // dspw_proc

  struct dsp_data_mask _fieldMask;
  int _procMode;

  // dspw_options

  int _nSamples;
  int _majorMode;
  int _procFlags;
  int _prfMode;
  int _windowType;
  int _rangeSmooth;
  bool _optionsChanged;

  // dspw_range_mask

  int _nGates;
  double _gateSpacing;  // km
  bool _rangeMaskChanged;

  // dsp_set_prf

  double _prf;
  bool _prfChanged;
  
  // dspw_phaseSeq - phase coding

  int _phaseCoding;
  bool _phaseCodingChanged;

  // dspw_filt - clutter filter

  int _clutFiltNum;
  bool _clutFiltNumChanged;

  // dsp_set_polarization
  
  int _polarization;
  bool _polarizationChanged;

  // dspw_syn_mode - angle synchronization

  int _angSyncMode;
  bool _angSyncModeChanged;

  // el and az in simulate mode

  double _el, _az;

  // functions

  void _clearFieldMask();

};

#endif
