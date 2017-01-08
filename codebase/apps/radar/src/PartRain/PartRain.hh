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
// PartRain.hh
//
// PartRain object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2008
//
///////////////////////////////////////////////////////////////
//
// PartRain reads dual pol moments in a DsRadar FMQ,
// compute rain rate and particle ID, and writes these out
// to a DsRadar FMQ
//
///////////////////////////////////////////////////////////////////////

#ifndef PartRain_HH
#define PartRain_HH

#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <radar/PrecipRate.hh>
#include <radar/KdpBringi.hh>
#include <radar/NcarParticleId.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;

////////////////////////
// This class

class PartRain {
  
public:

  // constructor

  PartRain (int argc, char **argv);

  // destructor
  
  ~PartRain();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const double _missingDouble;
  static const double _missingTest;

  // members

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  bool _debugPrintNeedsNewline;
  int _nErrorPrintPid;

  // FMQ input
  
  DsRadarQueue *_inputQueue;
  DsRadarMsg _inputMsg;
  int _inputContents;
  int _inputNFail;
  int _nFieldsIn;
  int _nGates;
  int _volNum;
  double _elev;
  double _az;
  double _wavelengthCm;
  double _radarHtKm;

  DsRadarParams _inputRadarParams;
  vector<DsFieldParams*> _inputFieldParams;
  DsRadarCalib _inputRadarCalib;

  // FMQ output
  
  DsRadarQueue *_outputQueue;
  DsRadarMsg _outputMsg;
  int _nFieldsOut;

  // algorithms

  PrecipRate _rate;
  NcarParticleId _pid;
  KdpBringi _kdpBringi;

  // data arrays

  int _nGatesAlloc;
  TaArray<double> _snr_, _dbz_, _vel_, _width_;
  TaArray<double> _zdr_, _ldr_, _kdp_, _phidp_, _rhohv_;
  TaArray<double> _tempForPid_;

  double *_snr, *_dbz, *_vel, *_width;
  double *_zdr, *_ldr, *_kdp, *_phidp, *_rhohv;
  double *_tempForPid;

  // field indexes

  int _snrIndex;
  int _dbzIndex;
  int _velIndex;
  int _widthIndex;
  int _zdrIndex;
  int _ldrIndex;
  int _kdpIndex;
  int _phidpIndex;
  int _rhohvIndex;

  // field availability

  bool _snrAvail;
  bool _dbzAvail;
  bool _velAvail;
  bool _widthAvail;
  bool _zdrAvail;
  bool _ldrAvail;
  bool _kdpAvail;
  bool _phidpAvail;
  bool _rhohvAvail;

  // sounding retrieval time

  time_t _latestSoundingRequestTime;

  // functions
  
  int _run();

  int _openInputQueue();
  int _openOutputQueue();

  int _processInputMessage();
  
  void _copyFlags();

  int _processBeam();

  int _getInputFieldIndex(const string &dsrName);

  void _loadInputField(const DsRadarBeam &beam, int index, double *fldData);

  void _computeSnrFromDbz(const double *dbz, double *snr);

  int _computeKdp();

  int _writeBeam();

  void _loadOutputField(const double *fld, int index,
			double scale, double bias,
			ui16 *outData);

  void _loadOutputField(const int *fld, int index,
			double scale, double bias,
			ui16 *outData);

  void _printDebugMsg(const string &msg, bool addNewline = true);

  static int _doubleCompare(const void *i, const void *j);

  void _allocateArrays(int nGates);

  int _overrideTempProfile(time_t beamTime);

};

#endif

