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
#include <Fmq/DsRadarQueue.hh>
class OpsInfo;
using namespace std;

////////////////////////
// This class

class OutputFmq {
  
public:

  // constructor
  
  OutputFmq(const string &prog_name,
	    const Params &params,
            const OpsInfo &opsInfo);
  
  // destructor
  
  ~OutputFmq();

  // set methods

  void setNGates(int nGates) { _nGates = nGates; }
  void setNSamples(int nSamples) { _nSamples = nSamples; }
  void setPrf(double prf) { _prf = prf; }
  void setPulseWidth(double width) { _pulseWidth = width; }

  // put the params
  
  int writeParams(double start_range, double gate_spacing);
  
  // write the beam data
  
  int writeBeam(const Beam *beam, int volNum, int tiltNum);
  
  // put flags

  void putEndOfVolume(int volNum, time_t time);
  void putStartOfVolume(int volNum, time_t time);

  void putEndOfTilt(int tiltNum, time_t time);
  void putStartOfTilt(int tiltNum, time_t time);

  // constructor status

  bool isOK;

protected:
  
private:

  string _progName;
  const Params &_params;
  const OpsInfo &_opsInfo;

  DsRadarQueue _rQueue;
  DsRadarMsg _msg;

  int _nFields;
  int _nGates;
  int _nSamples;
  double _prf;
  double _pulseWidth;

  static const double _missingDbl;

  double _snrScale;
  double _snrBias;

  double _dbmScale;
  double _dbmBias;

  double _dbzScale;
  double _dbzBias;

  double _velScale;
  double _velBias;

  double _widthScale;
  double _widthBias;

  double _percentScale;
  double _percentBias;

  double _interestScale;
  double _interestBias;

  double _angleScale;
  double _angleBias;

  double _kdpScale;
  double _kdpBias;

  double _zdrScale;
  double _zdrBias;

  double _ldrScale;
  double _ldrBias;

  double _rhohvScale;
  double _rhohvBias;

  double _cmdScale;
  double _cmdBias;

  double _tdbzScale;
  double _tdbzBias;

  double _spinScale;
  double _spinBias;

  double _sdveScale;
  double _sdveBias;

  double _fracScale;
  double _fracBias;

  double _testScale;
  double _testBias;

  int _flagBias;

  double _prevAz; 

  // Add a field to the field params message.

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

};

#ifdef _in_OutputFmq_cc
const double OutputFmq::_missingDbl = -9999.0;
#endif

#endif

