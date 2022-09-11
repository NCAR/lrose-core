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
// RadxDataMgr.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2022
//
///////////////////////////////////////////////////////////////
//
// Data manager for Radx moments data
//
///////////////////////////////////////////////////////////////

#ifndef RadxDataMgr_H
#define RadxDataMgr_H

#include <string>
#include <vector>
#include <map>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include "StatsMgr.hh"
#include <Radx/RadxFile.hh>
#include <Radx/RadxVol.hh>
#include <toolsa/TaArray.hh>

using namespace std;

////////////////////////
// This class

class RadxDataMgr {
  
public:

  // constructor

  RadxDataMgr(const string &prog_name,
              const Args &args,
              const Params &params,
              StatsMgr &statsMgr);
  
  // destructor
  
  ~RadxDataMgr();

  // run 

  int run();

protected:
  
private:

  static const double _missingDouble;
  static const double _missingTest;
  
  string _progName;
  char *_paramsPath;
  
  const Args &_args;
  const Params &_params;
  const vector<string> _inputFileList;
  StatsMgr &_statsMgr;

  // data input

  RadxVol _readVol;
  
  // DsRadarQueue *_inputQueue;
  // DsRadarMsg _inputMsg;
  int _inputContents;
  int _inputNFail;
  int _nFieldsIn;
  int _nGates;

  int _volNum;

  // DsRadarParams _inputRadarParams;
  // vector<DsFieldParams*> _inputFieldParams;
  // DsRadarCalib _inputRadarCalib;

  int _totalRayCount;

  // input moments data
  
  map<int, string> _fieldNameMap;

  typedef struct {
    int id;
    string dsrName;
    TaArray<double> data_;
    double *data;
  } moments_field_t;

  moments_field_t _snr;
  moments_field_t _snrhc;
  moments_field_t _snrhx;
  moments_field_t _snrvc;
  moments_field_t _snrvx;
  moments_field_t _dbm;
  moments_field_t _dbmhc;
  moments_field_t _dbmhx;
  moments_field_t _dbmvc;
  moments_field_t _dbmvx;
  moments_field_t _dbz;
  moments_field_t _vel;
  moments_field_t _width;
  moments_field_t _zdrm;
  moments_field_t _ldrh;
  moments_field_t _ldrv;
  moments_field_t _phidp;
  moments_field_t _rhohv;

  // methods

  int _processFile(const string &filePath);
  int _readFile(const string &filePath);
  void _setupRead(RadxFile &file);

  void _processRay(const RadxRay *ray);
  void _loadMomentsData(const RadxRay *ray,
                        Params::moments_id_t id,
                        moments_field_t &field);
  void _processMoments(const RadxRay *ray);

};

#endif

