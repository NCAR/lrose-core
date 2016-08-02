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
// PolarTransform.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2001
//
///////////////////////////////////////////////////////////////////////

#include "PolarTransform.hh"
#include "Dsr2Rapic.hh"
#include <math.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
using namespace std;

PolarTransform::PolarTransform (const Params &params,
				double max_range,
				double min_elev, double max_elev,
				const string &mdv_url) :
  PlanTransform(params, mdv_url)
  
{

  _geomType = POLAR_OUTPUT_GRID;
  
  _nx = 1; // will be updated in setGeom()
  _ny = 1;
  _nz = 1;

  _dx = 1.0; // will be updated in setGeom()
  _dy = 1.0;
  _dz = 1.0;

  _minx = 0; // will be updated in setGeom()
  _miny = 0;
  _minz = 0;

  _maxRange = max_range;
  _maxGates = 0; // will be updated in setGeom()
  _minElev = min_elev;
  _maxElev = max_elev;
  _elevOffset = 0; // will be updated in setGeom()

}

PolarTransform::~PolarTransform ()
{

}

int PolarTransform::setGeom(const vector<double> elev_array,
			    int n_az,
			    int n_az_per_45,
			    double delta_az,
			    int n_gates,
			    double start_range,
			    double gate_spacing,
			    double beam_width,
			    double radar_lat,
			    double radar_lon,
			    double radar_alt)

{

  // copy members

  _naz = n_az;
  _deltaAz = delta_az;

  _beamWidth = beam_width;
  _radarLat = radar_lat;
  _radarLon = radar_lon;
  _radarAlt = radar_alt;

  // fill the elev array
  
  _elevArray.clear();
  for (size_t ii = 0; ii < elev_array.size(); ii++) {
    double elev = elev_array[ii];
    if(elev >= _minElev && elev <= _maxElev) {
      if (_elevArray.size() == 0) {
	_elevOffset = ii;
      }
      _elevArray.push_back(elev);
    }
  }
  _nElev = _elevArray.size();

  // compute max number of gates

  _startRange = start_range;
  _gateSpacing = gate_spacing;
  _nGates = n_gates;
  _maxGates = (int) ((_maxRange - _startRange) / _gateSpacing + 0.5);
  if (_maxGates < 0) {
    _maxGates = 0;
  }
  if (_maxGates > _nGates) {
    _maxGates = _nGates;
  }

  _nx = _maxGates;
  _dx = _gateSpacing;
  _minx = _startRange;
  
  _ny = _naz;
  _dy = _deltaAz;
  _miny = 0.0;
  
  _nz = _nElev;
  if (_nElev > 0) {
    _minz = _elevArray[0];
  }
  _dz = 1.0;
  
  // allocate space for output data

  _allocOutputFields();

  if (_params.debug) {
    cerr << "========== Polar grid params ==========" << endl;
    cerr << "  _nx:  " << _nx << endl;
    cerr << "  _dx:  " << _dx << endl;
    cerr << "  _minx:  " << _minx << endl;
    cerr << "  _ny:  " << _ny << endl;
    cerr << "  _dy:  " << _dy << endl;
    cerr << "  _miny:  " << _miny << endl;
    cerr << "  _nz:  " << _nz << endl;
    cerr << "  _dz:  " << _dz << endl;
    cerr << "  _minz:  " << _minz << endl;
    cerr << "=================================" << endl;
  }

  if (_nElev > 0) {
    return 0;
  } else {
    return -1;
  }

}

void PolarTransform::calcLookup()

{

  if (_params.debug) {
    cerr << "**** Start PolarTransform::calcLookup() ****" << endl;
  }

  if (_params.debug) {
    cerr << "** end PolarTransform::calcLookup()" << endl;
  }

}

////////////////////////////////////////////////////////////////
// transform to polar output projection
//

void PolarTransform::doTransform(const vector<FieldInfo> &fields,
				 double nyquist,
				 Beam ***beam_array)

{

  PMU_auto_register("Polar::transform");

  if (_params.debug) {
    cerr << "**** Start PolarTransform::doTransform() ****" << endl;
  }
  
  int nbytesPpi = _naz * _maxGates;
  
  for (int ielev = 0; ielev < _nElev; ielev++) {

    PMU_auto_register("Polar::transform");

    int jelev = ielev + _elevOffset;

    for (int iaz = 0; iaz < _naz; iaz++) {

      Beam *beam = beam_array[jelev][iaz];

      if (beam != NULL) {
	
	int outPos = ielev * nbytesPpi + iaz * _maxGates;

	for (int ifield = 0; ifield < _nFieldsOut; ifield++) {

	  int ngatesCopy;
	  if (beam->nGates < _maxGates) {
	    ngatesCopy  = beam->nGates;
	  } else {
	    ngatesCopy = _maxGates;
	  }

	  for (int igate = 0; igate < ngatesCopy; igate++) {
	    _outputFields[ifield][outPos + igate] =
	      beam->getValue(ifield, igate);
	  } // igate
	  
	} // ifield

      } // if (beam_array[jelev][iaz] ...

    } // iaz

  } // ielev

  if (_params.debug) {
    cerr << "**** End PolarTransform::doTransform() ****" << endl;
  }

}

