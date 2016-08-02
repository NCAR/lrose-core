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
// May 2003
//
///////////////////////////////////////////////////////////////

#ifndef OutputFmq_H
#define OutputFmq_H

#include <string>
#include <vector>
#include "Params.hh"
#include "Beam.hh"
#include <Fmq/DsRadarQueue.hh>
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

  // set methods

  void setNGates(int nGates) { _nGates = nGates; }
  void setNSamples(int nSamples) { _nSamples = nSamples; }
  void setPrf(double prf) { _prf = prf; }
  void setPulseWidth(double width) { _pulseWidth = width; }

  // put the params
  
  int writeParams(double start_range, double gate_spacing);
  
  // write the beam data
  
  int writeBeam(const Beam *beam, int volNum);
  
  // put volume flags

  void putEndOfVolume(int volNum, time_t time);
  void putStartOfVolume(int volNum, time_t time);

  // constructor status

  bool isOK;

protected:
  
private:

  string _progName;
  const Params &_params;
  DsRadarQueue _rQueue;
  DsRadarMsg _msg;
  bool _doWrite;

  int _nFields;
  int _nGates;
  int _nSamples;
  double _prf;
  double _pulseWidth;

  static const double _missingDbl = -9999.0;

  double _snrScale;
  double _snrBias;

  double _dbzScale;
  double _dbzBias;

  double _velScale;
  double _velBias;

  double _widthScale;
  double _widthBias;

  double _interestScale;
  double _interestBias;

  double _angleScale;
  double _angleBias;

  double _kdpScale;
  double _kdpBias;

  double _zdrScale;
  double _zdrBias;

  double _rhohvScale;
  double _rhohvBias;

  double _recScale;
  double _recBias;

  double _tdbzScale;
  double _tdbzBias;

  double _spinScale;
  double _spinBias;

  double _sdveScale;
  double _sdveBias;

  double _fracScale;
  double _fracBias;

  int _flagBias;

  void _loadDoubleField(const double *in, ui16 *out,
			double scale, double bias);

  void _loadIntField(const int *in, ui16 *out);

};

#endif

