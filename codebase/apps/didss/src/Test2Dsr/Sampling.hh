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
// Sampling.hh
//
// Sampling class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 1998
//
///////////////////////////////////////////////////////////////

#ifndef Sampling_HH
#define Sampling_HH

#include "Params.hh"
#include <Mdv/mdv/mdv_handle.h>
#include <rapformats/radar_scan_table.h>
#include <rapformats/ds_radar.h>
#include <string>
using namespace std;

#define PSEUDO_RADIUS 8533.0  // 4/3 Earth radius in km

class Sampling {
  
public:
  
  // constructor
  
  Sampling(const string &prog_name,
           const Params &params,
	   int n_fields);
  
  // destructor
  
  virtual ~Sampling();

  int OK;

  // load up the beam

  ui08 *loadBeam(DsBeamHdr_t *beamHdr, int *ndata_p,
		 int *new_scan_type_p,
		 int *end_of_tilt_p,
		 int *end_of_volume_p);

  // return members

  inline double zScale() { return _zScale; }
  inline double zBias() { return _zBias; }
  inline double vScale() { return _vScale; }
  inline double vBias() { return _vBias; }
  inline double htScale() { return _htScale; }
  inline double htBias() { return _htBias; }


protected:
  
private:

  string _progName;
  const Params &_params;

  MDV_handle_t _mdv;

  time_t _startTime;

  ui08 *_noise;
  ui08 *_beamBuf;

  int _nFields;
  int _nGates;
  int _nData;
  int _vField;
  int _zField;
  int _nX, _nY, _nZ;

  double _zScale, _zBias;
  double _vScale, _vBias;
  double _htScale, _htBias;
  double *_cosPhi, *_sinPhi;
  double **_beamHt, **_gndRange;
  double _minX, _minY, _minZ, _dX, _dY, _dZ;

  bool _scanTableActive;
  radar_scan_table_t _scanTable;

  int _currElev;
  int _currAz;
  int _volNum;
  int _tiltNum;
  int _count;
  
  void _setupNoiseArray();
  int _setupScanTable();
  void _calcSamplingGeom();

};

#endif
