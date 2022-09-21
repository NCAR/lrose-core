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
// DsrDataMgr.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2022
//
///////////////////////////////////////////////////////////////
//
// Data manager for Dsr moments data
//
///////////////////////////////////////////////////////////////

#ifndef DsrDataMgr_H
#define DsrDataMgr_H

#include <string>
#include <vector>
#include <deque>
#include <cstdio>

#include "StatsMgr.hh"
#include <toolsa/TaArray.hh>
#include <Fmq/DsRadarQueue.hh>

using namespace std;

////////////////////////
// This class

class DsrDataMgr : public StatsMgr {
  
public:

  // constructor

  DsrDataMgr(const string &prog_name,
             const Args &args,
	     const Params &params);

  // destructor
  
  virtual ~DsrDataMgr();

  // run 

  virtual int run();

protected:
  
private:

  static const double _missingDouble;

  // FMQ input
  
  DsRadarQueue *_inputQueue;
  DsRadarMsg _inputMsg;
  int _inputContents;
  int _inputNFail;
  int _nFieldsIn;
  int _nGates;

  DsRadarParams _inputRadarParams;
  vector<DsFieldParams*> _inputFieldParams;
  DsRadarCalib _inputRadarCalib;

  double _startRangeKm, _gateSpacingKm;
  
  // int _totalBeamCount;

  // input moments data

  typedef struct {
    int id;
    string dsrName;
    int paramsIndex;
    int dataIndex;
    TaArray<double> data_;
    double *data;
  } moments_field_t;

  moments_field_t _dbmhc;
  moments_field_t _dbmhx;
  moments_field_t _dbmvc;
  moments_field_t _dbmvx;

  // methods

  int _openInputQueue();
  int _processInputMessage();
  void _processBeam();
  // void _processMoments(const RadxTime &beamTime);

  void _setMomentsIndices(Params::moments_id_t paramId,
			  moments_field_t &field);

  void _setMomentsParamsIndex(Params::moments_id_t paramId,
			      moments_field_t &field);

  void _setMomentsDataIndex(moments_field_t &field);

  string _getMomentsParamsName(Params::moments_id_t paramId);

  int _getMomentsParamsIndex(const string &dsrName);

  int _getInputDataIndex(const string &dsrName);

  void _loadMomentsData();

  void _loadMomentsData(moments_field_t &field);

  void _loadInputField(const DsRadarBeam &beam, int index, double *fldData);

};

#endif

