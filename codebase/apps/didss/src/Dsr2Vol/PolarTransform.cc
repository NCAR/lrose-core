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
#include "Dsr2Vol.hh"
#include <math.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/TaArray.hh>
using namespace std;

PolarTransform::PolarTransform (const Params &params,
				const string &mdv_url,
                                const vector<FieldInfo> &fields,
				double max_range,
				double min_elev, double max_elev) :
  PlanTransform(params, mdv_url, fields)
  
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

  _trimToSector = false;
  _dataStartIaz = 0;
  _dataEndIaz = 0;

}

PolarTransform::~PolarTransform ()
{

}

////////////////////////////////////////////////////////////////
// set the radar and beam geometry and elevation angles
// returns true if geometry has changed, false otherwise
// This also allocates the output field memory

bool PolarTransform::setGeom(const vector<double> elevations,
                             int n_gates,
                             double start_range,
                             double gate_spacing,
                             int n_az,
                             int n_az_per_45,
                             double delta_az,
                             double beam_width,
                             double radar_lat,
                             double radar_lon,
                             double radar_alt)

{

  // always assume geom has changed

  bool geomChanged = true;

  // first call parent class method

  PlanTransform::setGeom(elevations,
			 n_gates, start_range, gate_spacing,
			 n_az, n_az_per_45, delta_az,
			 beam_width, radar_lat, radar_lon, radar_alt);
  _nAzTrimmed = n_az;

  // find empty sectors

  if (_params.trim_polar_sectors) {
    _findEmptySectors();
  } else {
    _trimToSector = false;
  }

  // fill the elev array, cropping to minElev and maxElev
  
  _elevArray.clear();
  for (size_t ii = 0; ii < elevations.size(); ii++) {
    double elev = elevations[ii];
    if(elev >= _minElev && elev <= _maxElev) {
      if (_elevArray.size() == 0) {
	_elevOffset = ii;
      }
      _elevArray.push_back(elev);
    }
  }
  _nElev = _elevArray.size();
  
  // compute max number of gates

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
  
  if (_trimToSector) {

    _ny = _nAzTrimmed;
    _dy = _deltaAz;
    _miny = _dataStartIaz * _deltaAz;

  } else {

    _ny = _naz;
    _dy = _deltaAz;
    _miny = 0.0;

  }

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

  return geomChanged;

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

void PolarTransform::doTransform(double nyquist)

{

  PMU_auto_register("Polar::transform");

  if (_params.debug) {
    cerr << "**** Start PolarTransform::doTransform() ****" << endl;
  }

  if (_trimToSector) {
    _doTransformTrim(nyquist);
  } else {
    _doTransformFull(nyquist);
  }

  if (_params.debug) {
    cerr << "**** End PolarTransform::doTransform() ****" << endl;
  }

}

////////////////////////////////////////////////////////////////
// transform to polar output projection - full 360
//

void PolarTransform::_doTransformFull(double nyquist)

{

  int nbytesPpi = _naz * _maxGates;
  
  for (int ielev = 0; ielev < _nElev; ielev++) {

    PMU_auto_register("Polar::transform");

    int jelev = ielev + _elevOffset;

    for (int iaz = 0; iaz < _naz; iaz++) {

      const Beam *beam = _regularBeamArray[jelev][iaz];

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

      } // if (_regularBeamArray[jelev][iaz] ...

    } // iaz

  } // ielev

}

////////////////////////////////////////////////////////////////
// transform to polar output projection
// trim to sector

void PolarTransform::_doTransformTrim(double nyquist)

{

  int nbytesPpi = _nAzTrimmed * _maxGates;
  
  for (int ielev = 0; ielev < _nElev; ielev++) {
    
    PMU_auto_register("Polar::transform");
    
    int jelev = ielev + _elevOffset;
    
    for (int iaz = 0; iaz < _nAzTrimmed; iaz++) {

      int jaz = (iaz + _dataStartIaz + _naz) % _naz;
      
      const Beam *beam = _regularBeamArray[jelev][jaz];

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

      } // if (_regularBeamArray[jelev][iaz] ...

    } // iaz

  } // ielev

}

////////////////////////////////////////////////////////////////
// find empty sectors
//
// Sets the data region

void PolarTransform::_findEmptySectors()

{

  if (_params.debug) {
    cerr << "==== Searching for empty sectors ====" << endl;
  }

  // load up array to indicate which azimuths are active

  TaArray<bool> active_;
  bool *active = active_.alloc(_naz);
  
  for (int iaz = 0; iaz < _naz; iaz++) {
    active[iaz] = false;
  }

  for (int iaz = 0; iaz < _naz; iaz++) {
    for (int ielev = 0; ielev < _nElev; ielev++) {
      if (_regularBeamArray[ielev][iaz] != NULL) {
	active[iaz] = true;
	break;
      }
    }
  }

  // load up a vector of empty sectors

  vector<az_sector_t> emptySectors;
  int startIaz = 0;
  bool inEmptySector = false;
  if (!active[0]) {
    inEmptySector = true;
  }
  for (int iaz = 1; iaz < _naz; iaz++) {
    if (iaz < _naz - 1) {
      // not the last azimuth
      if (active[iaz]) {
	// this azimuth is active
	if (inEmptySector) {
	  az_sector_t sector;
	  sector.startIaz = startIaz;
	  sector.endIaz = iaz - 1;
	  emptySectors.push_back(sector);
	}
	inEmptySector = false;
      } else {
	// this azimuth is not active
	if (!inEmptySector) {
	  inEmptySector = true;
	  startIaz = iaz;
	}
      }
    } else {
      // last azimuth
      if (active[iaz]) {
	// this azimuth is active
	if (inEmptySector) {
	  az_sector_t sector;
	  sector.startIaz = startIaz;
	  sector.endIaz = iaz - 1;
	  emptySectors.push_back(sector);
	}
      } else {
	// this azimuth is not active
	if (inEmptySector) {
	  az_sector_t sector;
	  sector.startIaz = startIaz;
	  sector.endIaz = iaz;
	  emptySectors.push_back(sector);
	} else {
	  az_sector_t sector;
	  sector.startIaz = iaz;
	  sector.endIaz = iaz;
	  emptySectors.push_back(sector);
	}
      }
    }
  } // iaz

  int nEmpty = (int) emptySectors.size();
  if (_params.debug) {
    cerr << "===========================" << endl;
    for (int ii = 0; ii < nEmpty; ii++) {
      cerr << "------>> empty sector: " << ii << endl;
      cerr << "  start iaz, az: " << emptySectors[ii].startIaz << ", "
	   << emptySectors[ii].startIaz * _deltaAz << endl;
      cerr << "  end iaz, az: " << emptySectors[ii].endIaz << ", "
	   << emptySectors[ii].endIaz * _deltaAz << endl;
    }
    cerr << "===========================" << endl;
  }

  // loop through the empty sectors, finding the largest
  // contiguous block of azimuths
  // we ignore data blocks only 1 wide
  
  _dataStartIaz = 0;
  _dataEndIaz = 0;
  
  if (nEmpty == 0) {

    // no empty areas, use full circle
    
    _dataStartIaz = 0;
    _dataEndIaz = _naz - 1;
    _trimToSector = false;

  } else if (nEmpty == 1) {

    // only 1 empty sector

    _dataEndIaz = emptySectors[0].startIaz - 1;
    _dataStartIaz = emptySectors[0].endIaz + 1;
    _trimToSector = true;
    
  } else {

    // multiple empty sectors
    // search through them for the largest contiguous empty sector
    // with azimuth gaps of no greater than 3

    int maxNaz = 0;
    
    for (int ii = 0; ii < nEmpty; ii++) {

      int startIaz = emptySectors[ii].startIaz;
      int endIaz = 0;
      
      for (int jj = 0; jj < nEmpty; jj++) {
	
	int kk = (ii + jj) % nEmpty;
	int mm = (ii + jj + 1) % nEmpty;
	
	const az_sector_t &thisSec = emptySectors[kk];
	const az_sector_t &nextSec = emptySectors[mm];
	
	endIaz = thisSec.endIaz;
	
	int gap = 0;
	if (nextSec.startIaz >= thisSec.endIaz) {
	  gap = nextSec.startIaz - thisSec.endIaz;
	} else {
	  gap = nextSec.startIaz - thisSec.endIaz + _naz;
	}
	
	if (gap > 3) {
	  break;
	}

      } // jj

      int nAz = endIaz - startIaz;
      if (nAz < 0) {
	nAz += _naz;
      }
      
      if (nAz > maxNaz) {
	_dataStartIaz = endIaz + 1;
	_dataEndIaz = startIaz - 1;
	maxNaz = nAz;
	_trimToSector = true;
      }
      
    } // ii
    
  } // if (nEmpty == 0)

  if (_dataStartIaz > _dataEndIaz) {
    _dataStartIaz -= _naz;
  }
  _nAzTrimmed = _dataEndIaz - _dataStartIaz + 1;

  if (_params.debug) {
    cerr << "  storeAsSector: " << _trimToSector << endl;
    cerr << "  data startIaz, az: " << _dataStartIaz << ", "
	 << _dataStartIaz * _deltaAz << endl;
    cerr << "  data endIaz, az: " << _dataEndIaz << ", "
	 << _dataEndIaz * _deltaAz << endl;
    cerr << "===========================" << endl;
  }

}

