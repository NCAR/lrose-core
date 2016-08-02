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
// Cov2Mom.hh
//
// Cov2Mom object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2008
//
///////////////////////////////////////////////////////////////
//
// Cov2Mom reads covariances from a DsRadar FMQ,
// computes moments, writes these out to a DsRadar FMQ
//
///////////////////////////////////////////////////////////////////////

#ifndef Cov2Mom_HH
#define Cov2Mom_HH

#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <radar/RadarComplex.hh>
#include <radar/RadarMoments.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;

////////////////////////
// This class

class Cov2Mom {
  
public:

  // constructor

  Cov2Mom (int argc, char **argv);

  // destructor
  
  ~Cov2Mom();

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

  // FMQ input
  
  DsRadarQueue *_inputQueue;
  DsRadarMsg _inputMsg;
  int _inputContents;
  int _inputNFail;
  int _nFieldsIn;
  int _nGates;
  int _volNum;

  DsRadarParams _inputRadarParams;
  vector<DsFieldParams*> _inputFieldParams;
  DsRadarCalib _inputRadarCalib;

  // FMQ output
  
  DsRadarQueue *_outputQueue;
  DsRadarMsg _outputMsg;
  int _nFieldsOut;

  // input covariance data

  typedef struct {
    int id;
    string dsrName;
    int paramsIndex;
    int dataIndex;
    TaArray<double> data_;
    double *data;
  } cov_field_t;

  cov_field_t _lag0_hc_db;
  cov_field_t _lag0_hx_db;
  cov_field_t _lag0_vc_db;
  cov_field_t _lag0_vx_db;
  cov_field_t _lag0_vchx_db;
  cov_field_t _lag0_vchx_phase;
  cov_field_t _lag0_hcvx_db;
  cov_field_t _lag0_hcvx_phase;
  cov_field_t _lag0_vxhx_db;
  cov_field_t _lag0_vxhx_phase;
  cov_field_t _lag1_hc_db;
  cov_field_t _lag1_hc_phase;
  cov_field_t _lag1_vc_db;
  cov_field_t _lag1_vc_phase;
  cov_field_t _lag1_hcvc_db;
  cov_field_t _lag1_hcvc_phase;
  cov_field_t _lag1_vchc_db;
  cov_field_t _lag1_vchc_phase;
  cov_field_t _lag1_vxhx_db;
  cov_field_t _lag1_vxhx_phase;
  cov_field_t _lag2_hc_db;
  cov_field_t _lag2_hc_phase;
  cov_field_t _lag2_vc_db;
  cov_field_t _lag2_vc_phase;
  cov_field_t _lag3_hc_db;
  cov_field_t _lag3_hc_phase;
  cov_field_t _lag3_vc_db;
  cov_field_t _lag3_vc_phase;

  // lag0 powers - derived from input data

  TaArray<double> _lag0_hc_;
  TaArray<double> _lag0_hx_;
  TaArray<double> _lag0_vc_;
  TaArray<double> _lag0_vx_;
  
  double *_lag0_hc;
  double *_lag0_hx;
  double *_lag0_vc;
  double *_lag0_vx;

  // complex data - derived from input data

  TaArray<RadarComplex_t> _lag0_vchx_;
  TaArray<RadarComplex_t> _lag0_hcvx_;
  TaArray<RadarComplex_t> _lag0_vxhx_;

  TaArray<RadarComplex_t> _lag1_hc_;
  TaArray<RadarComplex_t> _lag1_vc_;
  TaArray<RadarComplex_t> _lag1_hcvc_;
  TaArray<RadarComplex_t> _lag1_vchc_;
  TaArray<RadarComplex_t> _lag1_vxhx_;

  TaArray<RadarComplex_t> _lag2_hc_;
  TaArray<RadarComplex_t> _lag2_vc_;

  TaArray<RadarComplex_t> _lag3_hc_;
  TaArray<RadarComplex_t> _lag3_vc_;

  RadarComplex_t * _lag0_vchx;
  RadarComplex_t * _lag0_hcvx;
  RadarComplex_t * _lag0_vxhx;

  RadarComplex_t * _lag1_hc;
  RadarComplex_t * _lag1_vc;
  RadarComplex_t * _lag1_hcvc;
  RadarComplex_t * _lag1_vchc;
  RadarComplex_t * _lag1_vxhx;

  RadarComplex_t * _lag2_hc;
  RadarComplex_t * _lag2_vc;

  RadarComplex_t * _lag3_hc;
  RadarComplex_t * _lag3_vc;

  // output data arrays
  
  TaArray<double> _ncp_, _snr_, _dbm_, _dbz_, _vel_, _width_;
  TaArray<double> _zdr_, _ldrh_, _ldrv_, _rhohv_, _phidp_, _kdp_;
  TaArray<double> _snrhc_, _snrhx_, _snrvc_, _snrvx_;
  TaArray<double> _dbmhc_, _dbmhx_, _dbmvc_, _dbmvx_;
  
  double *_ncp, *_snr, *_dbm, *_dbz, *_vel, *_width;
  double *_zdr, *_ldrh, *_ldrv, *_rhohv, *_phidp, *_kdp;
  double *_snrhc, *_snrhx, *_snrvc, *_snrvx;
  double *_dbmhc, *_dbmhx, *_dbmvc, *_dbmvx;
  
  // functions
  
  int _run();

  int _openInputQueue();
  int _openOutputQueue();

  int _processInputMessage();
  
  void _copyFlags();

  int _processBeam();

  void _setCovIndices(Params::covariance_id_t paramId,
		      cov_field_t &field);

  void _loadCovData();
  
  void _loadCovData(cov_field_t &field);

  void _setCovParamsIndex(Params::covariance_id_t paramId,
			  cov_field_t &field);

  void _setCovDataIndex(cov_field_t &field);

  string _getCovParamsName(Params::covariance_id_t paramId);

  int _getCovParamsIndex(const string &dsrName);
  
  int _getInputDataIndex(const string &dsrName);

  void _loadInputField(const DsRadarBeam &beam, int index, double *fldData);

  void _loadPower(const double *db, double *power);

  void _loadComplex(const double *db,
		    const double *phase,
		    RadarComplex_t *complex);
  
  int _writeBeam();

  void _loadOutputField(double *fld, int index,
			double scale, double bias,
			ui16 *outData);
  
  void _printDebugMsg(const string &msg, bool addNewline = true);

  void _allocateArrays(int nGates);

  void _computeMoments();
  void _computeMomentsSp(RadarMoments &rmom);
  void _computeMomentsDpAltHvCoOnly(RadarMoments &rmom);
  void _computeMomentsDpAltHvCoCross(RadarMoments &rmom);
  void _computeMomentsDpAltHvFixedHv(RadarMoments &rmom);
  void _computeMomentsDpSimHvFixedHv(RadarMoments &rmom);
  void _computeMomentsDpSimHvSwitchedHv(RadarMoments &rmom);
  void _computeMomentsDpHOnlyFixedHv(RadarMoments &rmom);
  void _computeMomentsDpVOnlyFixedHv(RadarMoments &rmom);
  
};

#endif

