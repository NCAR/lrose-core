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
// BeamMgr.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2019
//
///////////////////////////////////////////////////////////////
//
// BeamMgr manages the beam data and printing
//
///////////////////////////////////////////////////////////////

#ifndef BeamMgr_hh
#define BeamMgr_hh

#include <iostream>
#include <string>
#include <vector>
#include <radar/RadarMoments.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfCalib.hh>
#include <radar/GateData.hh>
#include "RadarSpectra.hh"
#include "Params.hh"
class Beam;
using namespace std;

////////////////////////
// This class

class BeamMgr {
  
public:

  // constructor
  
  BeamMgr (const string &prog_name,
	   const Params &params);
  
  // destructor
  
  ~BeamMgr();
  
  // read cal file
  // Returns 0 on success, -1 on failure

  int readCalFromFile(const string &calPath);

  // Load beam data into manager.
  // Apply window as appropriate.
  // Set loadBeamData methods below.

  void loadBeamData(const Beam &beam);

  // Load beam data into manager.
  // Apply window as appropriate.
  // How this is loaded depends on the polarization mode.
  //
  // Assumptions:
  // 1. For alternating mode, first pulse in 77sequence is H.
  // 2. For non-switching dual receivers,
  //    H is channel 0 and V channel 1.
  // 3. For single pol mode, data is in channel 0.
  
  void loadBeamData(const IwrfTsInfo &opsInfo,
                    time_t beamTime,
		    double doubleBeamTime,
		    double el, double az,
                    double prt,
                    int nGatesIn,
                    const fl32 **iqChan0,
                    const fl32 **iqChan1);

  // Load beam data into manager for staggered PRT mode
  // How this is loaded depends on the polarization mode.
  //
  // Assumptions:
  // 1. First pulse is a short PRT pulse
  // 2. For non-switching dual receivers,
  //    H is channel 0 and V channel 1.
  // 3. For single pol mode, data is in channel 0.
  
  void loadBeamDataStaggeredPrt(const IwrfTsInfo &opsInfo,
                                time_t beamTime,
                                double doubleBeamTime,
                                double el, double az,
                                double prtShort,
                                double prtLong,
                                int nGatesPrtShort,
                                int nGatesPrtLong,
                                const fl32 **iqChan0,
                                const fl32 **iqChan1);

  // get methods
  
  double getStartRange() const { return _startRange; }
  double getGateSpacing() const { return _gateSpacing; }

  iwrf_xmit_rcv_mode_t getXmitRcvMode() const { return _xmitRcvMode; }

  int getNSamples() const { return _nSamples; }
  int getNSamplesHalf() const { return _nSamplesHalf; }
  
  const vector<GateData *> getGateData() const { return _gateData; }

  // copy data into spectra object

  void fillSpectraObj(RadarSpectra &spectra,
                      RadarSpectra::polarization_channel_t channel);
  
  // check on successful construction

  bool isOK;

protected:
  
private:

  string _progName;
  const Params &_params;
  
  // beam details
  
  IwrfTsInfo _opsInfo;
  time_t _beamTime;
  double _doubleBeamTime;
  double _el, _az;
  double _prt;
  double _nyquist;
  int _nGatesIn;

  // geometry
  
  double _startRange;
  double _gateSpacing;

  // number of pulse samples
  
  int _nSamples;
  int _nSamplesHalf;

  // flags

  iwrf_xmit_rcv_mode_t _xmitRcvMode;

  // staggered PRT mode

  bool _isStaggeredPrt;
  double _prtShort;
  double _prtLong;
  int _nGatesPrtShort;
  int _nGatesPrtLong;
  int _nGatesStaggeredPrt;
  double _nyquistPrtShort;
  double _nyquistPrtLong;
  int _staggeredM;
  int _staggeredN;

  // gate data

  vector<GateData *> _gateData;

  // spectra

  vector<RadarSpectra *> _spectraHc;
  vector<RadarSpectra *> _spectraHx;
  vector<RadarSpectra *> _spectraVc;
  vector<RadarSpectra *> _spectraVx;

  // calibration
  
  IwrfCalib _calib;

  // functions
  
  void _allocGateData(int nGates);
  void _freeGateData();
  void _initFieldData();

  void _loadGateIq(int nGates,
                   const fl32 **iqChan0,
                   const fl32 **iqChan1);

  void _loadGateIqStaggeredPrt(const fl32 **iqChan0,
                               const fl32 **iqChan1);
  
};

#endif

