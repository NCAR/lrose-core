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
// EdgeVol2Dsr.hh
//
// EdgeVol2Dsr object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2010
//
///////////////////////////////////////////////////////////////
// EdgeVol2Dsr reads raw EEC EDGE volume data and reformats
// the contents into a DsRadar FMQ.
///////////////////////////////////////////////////////////////

#ifndef EdgeVol2Dsr_hh
#define EdgeVol2Dsr_hh

#include <string>
#include <vector>
#include <didss/DsInputPath.hh>
#include <Fmq/DsRadarQueue.hh>
#include <toolsa/MemBuf.hh>
#include "Args.hh"
#include "Params.hh"
#include "Beam.hh"
#include "sigmet_headers.h"
using namespace std;

////////////////////////
// This class

class EdgeVol2Dsr {
  
public:

  // constructor

  EdgeVol2Dsr (int argc, char **argv);

  // destructor
  
  ~EdgeVol2Dsr();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsInputPath *_input;
  DsRadarQueue _rQueue;

  // speed of light

  static const double _lightSpeed;

  // binary angles to float

  static const double _angleConversion;
  
  double _sweepStartTime;
  time_t _volStartTime;
  time_t _latestRayTime;

  int _volNum;
  int _sweepNum; // starts at 1
  int _tiltNum; // starts at 0
  int _nSweeps;

  int _nFields;
  int _nRaysSweep;
  int _nGates;
  int _nBytesData;
  int _outputByteWidth;

  // sigmet headers read in

  prod_header_t _prodHdr;
  ingest_header_t _inHdr;
  int _nbytesExtendedHdr;
  raw_product_header_t _rawHdr;
  vector<ingest_data_header_t> _inDatHdrs;
  
  // record to hold raw data as it is read in

  ui08 _record[RAW_RECORD_LEN];

  // derived values

  double _fixedAngle;
  double _pulseWidthUs;
  double _wavelengthCm;
  double _wavelengthM;
  double _prf;
  double _prtSec;
  double _nyquist;
  double _unambigRangeKm;

  // input data buffer

  MemBuf _inBuf;

  // field data buffer

  MemBuf _dataBuf;
  vector<int> _nBytesRayField;

//   int _nBytesPerRay;
//   vector<int> _nBytesField;
//   vector<int> _nBytesFieldRay;

  // byte swapping

  bool _needToSwap;

  // ray info in buffer

  typedef struct {
    ui08 *offset;
    ray_header_t hdr;
  } RayInfo_t;

  vector<RayInfo_t> _rayInfo;

  // functions

  int _processFile(const char *input_path);
  int _reformatFile(const char *input_path);

  int _readRecord(FILE *in);
  int _readHeaders(FILE *in);
  int _readSweepData(FILE *in);
  int _processSweep();
  int _setRayInfo();
  time_t _setBeamMetadata(DsRadarBeam &dsBeam,
                          const ray_header_t &rayHdr);
  int _writeParams();
  void _loadRadarParams(DsRadarParams &rParams);

  double _binAngleToDouble(ui16 binAngle);
  double _binAngleToDouble(si32 binAngle);
  string _fieldId2Name(int fieldId);
  string _fieldId2Units(int fieldId);
  void _fieldId2ScaleBias(int fieldId, double &scale, double &bias);

  void _printCharArray(ostream &out, const char *buf, int len);
  string _time2Str(const sigmet_time_t &time);
  string _label2Str(const char *label, int maxLen);

  void _print(const sigmet_id_hdr_t &hdr, ostream &out);
  void _print(const prod_header_t &hdr, ostream &out);
  void _print(const prod_conf_t &prod, ostream &out);
  void _print(const prod_end_t &end, ostream &out);
  void _print(const ingest_header_t &hdr, ostream &out);
  void _print(const ingest_conf_t &conf, ostream &out);
  void _print(const task_conf_t &conf, ostream &out);
  void _print(const task_sched_info_t &info, ostream &out);
  void _print(const string &label,
              const dsp_data_mask_t &mask, ostream &out);
  void _print(const task_dsp_info_t &info, ostream &out);
  void _print(const task_calib_info_t &info, ostream &out);
  void _print(const task_range_info_t &info, ostream &out);
  void _print(const task_scan_info_t &info, ostream &out);
  void _print(const task_misc_info_t &info, ostream &out);
  void _print(const task_end_info_t &info, ostream &out);
  void _print(const raw_product_header_t &hdr, ostream &out);
  void _print(const ingest_data_header_t &hdr, ostream &out);
  void _print(const ray_header_t &hdr, ostream &out);

  void _swap(si16 *vals, int n);
  void _swap(ui16 *vals, int n);
  void _swap(si32 *vals, int n);
  void _swap(ui32 *vals, int n);
  void _swap(fl32 *vals, int n);
  void _swap(sigmet_id_hdr_t &val);
  void _swap(sigmet_time_t &val);
  void _swap(prod_header_t &val);
  void _swap(prod_conf_t &val);
  void _swap(prod_end_t &val);
  void _swap(ingest_header_t &val);
  void _swap(ingest_conf_t &val);
  void _swap(task_conf_t &val);
  void _swap(task_sched_info_t &val);
  void _swap(dsp_data_mask_t &val);
  void _swap(task_dsp_info_t &val);
  void _swap(task_calib_info_t &val);
  void _swap(task_range_info_t &val);
  void _swap(scan_info_union_t &val);
  void _swap(task_scan_info_t &val);
  void _swap(task_misc_info_t &val);
  void _swap(task_end_info_t &val);
  void _swap(raw_product_header_t &val);
  void _swap(ingest_data_header_t &val);
  void _swap(ray_header_t &val);

};

#endif

