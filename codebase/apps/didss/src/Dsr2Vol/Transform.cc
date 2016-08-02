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
////////////////////////////////////////////////////////////////////////
// Transform.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2001
//
///////////////////////////////////////////////////////////////////////

#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include "Transform.hh"
using namespace std;

const double Transform::missFl32 = Beam::missFl32;

//////////////////////
// Abstract Base class

Transform::Transform(const Params &params,
		     const string &mdv_url,
                     const vector<FieldInfo> &fields) :
  _params(params),
  _mdvUrl(mdv_url),
  _fields(fields)
  
{

  _nFieldsOut = _params.output_fields_n;
  _outputFields = NULL;
  _coverage = NULL;

  _nx = 0;
  _ny = 0;
  _nz = 0;
  _dx = 0;
  _dy = 0;
  _dz = 0;
  _minx = 0;
  _miny = 0;
  _minz = 0;

  // gates
  
  _nGates = 0;
  _startRange = 0;
  _gateSpacing = 0;
  _angularRes = 0;

  // radar params

  _radarLat = 0;
  _radarLon = 0;
  _radarAlt = 0;
  _beamWidth = 0;

}

Transform::~Transform()

{

  freeOutputFields();

}

void Transform::_allocOutputFields()

{
  
  freeOutputFields();
  int npts = _nx * _ny * _nz;
  if (_params.debug) {
    cerr << "nx: " << _nx << endl;
    cerr << "ny: " << _ny << endl;
    cerr << "nz: " << _nz << endl;
    cerr << "npts: " << npts << endl;
  }
  _outputFields = (fl32 **) ucalloc2(_nFieldsOut, npts, sizeof(fl32));
  for (int ifield = 0; ifield < _nFieldsOut; ifield++) {
    for (int ii = 0; ii < npts; ii++) {
      _outputFields[ifield][ii] = Beam::missFl32;
    }
  } // ifield

  if (_params.output_coverage_field) {
    _coverage = new fl32[npts];
    memset(_coverage, 0, npts * sizeof(fl32));
  }

}

void Transform::freeOutputFields()

{
  
  if (_outputFields != NULL) {
    ufree2((void **) _outputFields);
    _outputFields = NULL;
  }
  if (_coverage) {
    delete[] _coverage;
    _coverage = NULL;
  }
}


