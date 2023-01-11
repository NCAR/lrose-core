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
// OutputFmq.hh
//
// OutputFmq object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////

#ifndef OutputFmq_H
#define OutputFmq_H

#include <string>
#include <vector>
#include "Params.hh"
#include "Beam.hh"
#include <Fmq/DsFmq.hh>
#include <Fmq/DsRadarQueue.hh>
class IwrfTsInfo;
using namespace std;

////////////////////////
// This class

class OutputFmq {
  
public:

  // constructor
  
  OutputFmq(const string &prog_name,
	    const Params &params);
  
  // destructor
    
  ~OutputFmq();

  // write the params
  
  int writeParams(const Beam &beam);
  
  // write the calib info
  
  int writeCalib(const Beam &beam);
  
  // write the status XML
  
  int writeStatusXml(const Beam &beam);
  
  // write the beam data
  
  int writeBeam(const Beam &beam);
  
  // put flags
  
  void putEndOfVolume(int volNum, const Beam &beam);
  void putStartOfVolume(int volNum, const Beam &beam);

  void putEndOfTilt(int tiltNum, const Beam &beam);
  void putStartOfTilt(int tiltNum, const Beam &beam);

  void putNewScanType(int scanType, const Beam &beam);

  // constructor status

  bool constructorOK;

protected:
  
private:

  static const double _missingDbl;
  
  string _progName;
  const Params &_params;
  bool _useRadx;

  // DsRadar moments queue

  DsRadarQueue *_dsrQueue;
  DsRadarMsg _msg;
  int _nFields;

  // Radx moments queue

  DsFmq *_radxQueue;

  // mutex to prevent main thread and write thread from
  // accessing these functions concurrently

  pthread_mutex_t _busy;
  
  // status
  
  int _nBeamsWritten;
  
  // output field object, for computing offsets

  MomentsFields _flds;

  // functions

  int _writeParamsDsRadar(const Beam &beam);
  int _writePlatformRadx(const Beam &beam);
  
  int _writeCalibDsRadar(const Beam &beam);
  int _writeCalibRadx(const Beam &beam);
  
  int _writeStatusXmlDsRadar(const Beam &beam);
  int _writeStatusXmlRadx(const Beam &beam);
  
  int _writeBeamDsRadar(const Beam &beam);
  int _writeBeamRadx(const Beam &beam);
  
  void _putEndOfVolumeDsRadar(int volNum, const Beam &beam);
  void _putEndOfVolumeRadx(int volNum, const Beam &beam);

  void _putStartOfVolumeDsRadar(int volNum, const Beam &beam);
  void _putStartOfVolumeRadx(int volNum, const Beam &beam);

  void _putEndOfTiltDsRadar(int tiltNum, const Beam &beam);
  void _putEndOfTiltRadx(int tiltNum, const Beam &beam);

  void _putStartOfTiltDsRadar(int tiltNum, const Beam &beam);
  void _putStartOfTiltRadx(int tiltNum, const Beam &beam);

  void _putNewScanTypeDsRadar(int scanType, const Beam &beam);
  void _putNewScanTypeRadx(int scanType, const Beam &beam);

  int _openFmq();
  int _openDsRadarQueue();
  int _openRadxQueue();

  // Add a field to the dsradar field params message.

  inline void _addField(const string &name,
                        const string &units,
                        double scale,
                        double bias,
                        vector<DsFieldParams*> &fp)
  {
    DsFieldParams* fparams =
      new DsFieldParams(name.c_str(), units.c_str(),
                        scale, bias, sizeof(ui16));
    fp.push_back(fparams);
  }

  // convert double to ui16, applying scale and bias

  inline ui16 _convertDouble(double val, double scale, double bias) {
    if (val == _missingDbl) {
      return 0;
    }
    int ival = (int) ((val - bias) / scale + 0.5);
    if (ival < 1) {
      ival = 1;
    }
    if (ival > 65535) {
      ival = 65535;
    }
    return (ui16) ival;
  }

  // convert int to ui16, checking bias

  inline ui16 _convertInt(int val, int bias) {
    if (val == 0) {
      return 0;
    }
    int ival = val - bias;
    if (ival < 0) {
      ival = 0;
    }
    if (ival > 65535) {
      ival = 65535;
    }
    return (ui16) ival;
  }

  // get the offset for a given field ID
  // This is the offset of the data element in the MomentsFields object
  // realtive to the ncp field.
  
  int _findFieldOffset(Params::field_id_t fieldId);

};

#ifdef _in_OutputFmq_cc
const double OutputFmq::_missingDbl = MomentsFields::missingDouble;
#endif

#endif

