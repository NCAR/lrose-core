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
// PpiTransform.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2001
//
///////////////////////////////////////////////////////////////////////

#include "PpiTransform.hh"
#include "Dsr2Rapic.hh"
#include <math.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
using namespace std;

PpiTransform::PpiTransform (const Params &params,
			    int nxy, double dxy,
			    double min_elev, double max_elev,
			    bool interp,
			    const string &mdv_url,
			    double min_ht, double max_ht,
			    double max_range) :
  PlanTransform(params, mdv_url)
  
{

  _interp = interp;
  _geomType = PPI_OUTPUT_GRID;

  if (nxy % 2 != 0) {
    _nxy = nxy + 1;
    cerr << "WARNING - PpiTransform::PpiTransform" << endl;
    cerr << "  Forcing nxy to be even." << endl;
    cerr << "  Params nxy: " <<  nxy << endl;
    cerr << "  Using _nxy: " << _nxy << endl;
    cerr << endl;
  } else {
    _nxy = nxy;
  }
  _nxyHalf = _nxy / 2;
  
  _nx = _nxy;
  _ny = _nxy;
  _nz = 1; // will be updated in setGeom()

  _dxy = dxy;
  _dx = _dxy;
  _dy = _dxy;
  _dz = 1.0; // will be updated in setGeom()

  _minx = -1.0 * _dxy * (_nxy / 2.0 - 0.5);
  _miny = -1.0 * _dxy * (_nxy / 2.0 - 0.5);
  _minz = 0; // will be updated in setGeom()

  _minElev = min_elev;
  _maxElev = max_elev;
  _minHt = min_ht;
  _maxHt = max_ht;
  _elevOffset = 0;

  _maxRange = max_range;

}

PpiTransform::~PpiTransform ()
{

}

int PpiTransform::setGeom(const vector<double> vlevel_array,
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

  _azOffset[0] = 0;
  _azOffset[1] = n_az_per_45 * 2;
  _azOffset[2] = n_az_per_45 * 2;
  _azOffset[3] = n_az_per_45 * 4;
  _azOffset[4] = n_az_per_45 * 4;
  _azOffset[5] = n_az_per_45 * 6;
  _azOffset[6] = n_az_per_45 * 6;
  _azOffset[7] = n_az_per_45 * 8;

  _nGates = n_gates;
  _startRange = start_range;
  _gateSpacing = gate_spacing;

  _beamWidth = beam_width;
  _radarLat = radar_lat;
  _radarLon = radar_lon;
  _radarAlt = radar_alt;

  // fill the elev array
  
  _elevArray.clear();
  for (size_t ii = 0; ii < vlevel_array.size(); ii++) {
    double elev = vlevel_array[ii];
    if(elev >= _minElev && elev <= _maxElev) {
      if (_elevArray.size() == 0) {
	_elevOffset = ii;
      }
      _elevArray.push_back(elev);
    }
  }
  _nElev = _elevArray.size();
  _nz = _nElev;
  if (_nElev > 0) {
    _minz = _elevArray[0];
  }

  // allocate space for output data

  _allocOutputFields();

  if (_params.debug) {
    cerr << "========== Ppi grid params ==========" << endl;
    cerr << "  _nx:  " << _nx << endl;
    cerr << "  _dx:  " << _dx << endl;
    cerr << "  _minx:  " << _minx << endl;
    cerr << "  _ny:  " << _ny << endl;
    cerr << "  _dy:  " << _dy << endl;
    cerr << "  _miny:  " << _miny << endl;
    cerr << "  _nz:  " << _nz << endl;
    cerr << "  _dz:  " << _dz << endl;
    cerr << "  _minz:  " << _minz << endl;
    cerr << "  _elevOffset: " << _elevOffset << endl;
    cerr << "=================================" << endl;
  }

  if (_nElev > 0) {
    return 0;
  } else {
    return -1;
  }

}

void PpiTransform::calcLookup()

{

  PMU_auto_register("Ppi::calcLookup");

  if (_params.debug) {
    cerr << "**** Start PpiTransform::calcLookup() ****" << endl;
  }

  _lut.clear();
  
  // set up range limits
  
  double rangeToLastGate = _startRange + _nGates * _gateSpacing;
  double minRange = _startRange - _gateSpacing / 2.0;
  double maxRangeUsed = rangeToLastGate + _gateSpacing / 2.0;
  if (_maxRange > 0 && maxRangeUsed > _maxRange) {
    maxRangeUsed = _maxRange;
  }

  if (_params.debug) {
    cerr << "  rangeToLastGate: " << rangeToLastGate << endl;
    cerr << "  minRange: " << minRange << endl;
    cerr << "  maxRangeUsed: " << maxRangeUsed << endl;
  }

  // loop through y,x,elev
  
  for (int iy = 0; iy < _nxyHalf; iy++) {
    
    PMU_auto_register("Ppi::calcLookup");

    double yy = (iy + 0.5) * _dxy;
    
    for (int ix = 0; ix <= iy; ix++) {
      
      double xx = (ix + 0.5) * _dxy;
      
      // azimuth
      
      double az = atan2(xx, yy) * RAD_TO_DEG;
      double azNum = az / _deltaAz;

      // elevation
      
      for (int ielev = 0; ielev < _nElev; ielev++) {
	
	double elev = _elevArray[ielev];
	
	// compute range and check
	
	double gndRange = sqrt((xx * xx) + (yy * yy));
	double range = gndRange / cos(elev * DEG_TO_RAD);
	if (range < minRange || range > maxRangeUsed) {
	  continue;
	}
	
	// fill out lookup table entry
	
	plan_transform_lut_t entry;
	entry.iz = ielev;
	entry.iy = iy;
	entry.ix = ix;

	// azimuth

	entry.iaz = (int) azNum;
	entry.wtAz = 1.0 - (azNum - entry.iaz);
	
	// range
	
	double gate = (range - _startRange) / _gateSpacing;
	if (gate < 0) {
	  entry.igate = 0;
	  entry.wtGate = 1.0;
	} else if (gate > _nGates - 1) {
	  entry.igate = _nGates - 2;
	  entry.wtGate = 0.0;
	} else {
	  entry.igate = (int) gate;
	  entry.wtGate = 1.0 - (gate - entry.igate);
	}

	// elevation
	
	entry.ielev = ielev + _elevOffset;
	entry.wtElev = 1.0;

	// add entry to lookup table

	_lut.push_back(entry);
	
      } // ielev
      
    } // ix
    
  } // iy
  
  if (_params.debug) {
    cerr << "** end PpiTransform::calcLookup()" << endl;
    cerr << "  n lookup: " << _lut.size() << endl;
  }

}

////////////////////////////////////////////////////////////////
// transform to cart ppi
//

void PpiTransform::doTransform(const vector<FieldInfo> &fields,
			       double nyquist,
			       Beam ***beam_array)
  
{
  if (_interp) {
    _transformInterp(fields, nyquist, beam_array);
  } else {
    _transformNearest(fields, beam_array);
  }
}

////////////////////////////////////////////////////////////////
// transform to ppi output projection using interpolation
//

void PpiTransform::_transformInterp(const vector<FieldInfo> &fields,
				    double nyquist,
				    Beam ***beam_array)

{
  
  PMU_auto_register("Ppi::interp");

  if (_params.debug) {
    cerr << "**** Start PpiTransform::_transformInterp() ****" << endl;
  }

  // only doing 4-point interp, so only need half the number
  // of points

  int minValidForInterp = (_params.min_nvalid_for_interp + 1) / 2;
  
  // loop through the fields
  
  for (int ifield = 0; ifield < _nFieldsOut; ifield++) {
    
    PMU_auto_register("Ppi::interp");

    fl32 missFl32 = Beam::missFl32;
    const FieldInfo &field = fields[ifield];
    bool interpDbAsPower = field.interpDbAsPower;
    bool isVel = field.isVel;
    bool allowInterp = field.allowInterp;
    
    // loop through the lookup table
    
    int nbytesPlane = _nxy * _nxy;
    int count = 0;
    
    for (size_t ilut = 0; ilut < _lut.size(); ilut++) {
      
      // register every so often
      
      count++;
      if (count == 10000) {
	PMU_auto_register("Ppi::interp");
	count = 0;
      }
    
      const plan_transform_lut_t entry = _lut[ilut];
      
      // loop through the sectors
      
      for (int isector = 0; isector < 8; isector++) {
	
	int iaz1 = entry.iaz * _azSign[isector] + _azOffset[isector];
	int iaz2 = iaz1 + _azSign[isector];
	
	// load up the 4 data points around the target point
	
	fl32 lowAzLowGate = missFl32;
	fl32 lowAzHighGate = missFl32;
	fl32 highAzLowGate = missFl32;
	fl32 highAzHighGate = missFl32;
	
	Beam *beam;
	beam = beam_array[entry.ielev][iaz1];
	if (beam) {
	  if (beam->nGates > entry.igate) {
	    lowAzLowGate = beam->getValue(ifield, entry.igate);
	  }
	  if (beam->nGates > entry.igate + 1) {
	    lowAzHighGate = beam->getValue(ifield, entry.igate + 1);
	  }
	}

	beam = beam_array[entry.ielev][iaz2];
	if (beam) {
	  if (beam->nGates > entry.igate) {
	    highAzLowGate = beam->getValue(ifield, entry.igate);
	  }
	  if (beam->nGates > entry.igate + 1) {
	    highAzHighGate = beam->getValue(ifield, entry.igate + 1);
	  }
	}
	
	// check that we have sufficient contributing points

	int contribCount = 0;
	contribCount += (lowAzLowGate != missFl32);
	contribCount += (highAzLowGate != missFl32);
	contribCount += (lowAzHighGate != missFl32);
	contribCount += (highAzHighGate != missFl32);
	
	if (contribCount < minValidForInterp) {
	  continue;
	}
	
	// average in azimuth
	
	fl32 lowGate = interp(lowAzLowGate, highAzLowGate,
			      entry.wtAz, interpDbAsPower,
			      isVel, nyquist, allowInterp);
	
	fl32 highGate = interp(lowAzHighGate, highAzHighGate,
			       entry.wtAz, interpDbAsPower,
			       isVel, nyquist, allowInterp);
	
	// average in range
	
	fl32 interpVal = interp(lowGate, highGate,
				entry.wtGate, interpDbAsPower,
				isVel, nyquist, allowInterp);
	
	// compute ix and iy based on the sector

	int ix, iy;
	if (_swapXy[isector]) {
	  ix = entry.iy * _xSign[isector] + _nxyHalf;
	  iy = entry.ix * _ySign[isector] + _nxyHalf;
	} else {
	  ix = entry.ix * _xSign[isector] + _nxyHalf;
	  iy = entry.iy * _ySign[isector] + _nxyHalf;
	}

	// adjust by 1 if the sign is negative, because we are
	// using 0-based indices

	if (_xSign[isector] == -1) {
	  ix -= 1;
	}
	if (_ySign[isector] == -1) {
	  iy -= 1;
	}
	
	int ipos = entry.iz * nbytesPlane + iy * _nxy + ix;
	_outputFields[ifield][ipos] = interpVal;

      } // isector

    } // ilut

  } // ifield

  if (_params.debug) {
    cerr << "**** End PpiTransform::_transformInterp() ****" << endl;
  }

}

////////////////////////////////////////////////////////////////
// transform to ppi output projection using nearest neighbor
//

void PpiTransform::_transformNearest(const vector<FieldInfo> &fields,
				     Beam ***beam_array)

{

  PMU_auto_register("Ppi::nearest");

  if (_params.debug) {
    cerr << "**** Start PpiTransform::transformNearest() ****" << endl;
  }

  // set up arrays for converting the lookup for the first
  // sector to the other 7 sectors
  
  // loop through the lookup table

  int nbytesPlane = _nxy * _nxy;
  int count = 0;

  for (size_t ilut = 0; ilut < _lut.size(); ilut++) {
    
    // register every so often
    
    count++;
    if (count == 10000) {
      PMU_auto_register("Ppi::nearest");
      count = 0;
    }
    
    const plan_transform_lut_t entry = _lut[ilut];
    
    int ielev = entry.ielev;
    
    int igate;
    if (entry.wtGate > 0.5) {
      igate = entry.igate;
    } else {
      igate = entry.igate + 1;
    }
    
    // loop through the sectors
    
    for (int isector = 0; isector < 8; isector++) {
      
      int iaz1 = entry.iaz * _azSign[isector] + _azOffset[isector];
      int iaz2 = iaz1 + _azSign[isector];

      int iaz;
      if (entry.wtAz > 0.5) {
	iaz = iaz1;
      } else {
	iaz = iaz2;
      }

      if (beam_array[ielev][iaz] == NULL ||
	  igate >= beam_array[ielev][iaz]->nGates) {
	continue;
      }

      // loop through the fields

      for (int ifield = 0; ifield < _nFieldsOut; ifield++) {

	fl32 nearest = beam_array[ielev][iaz]->getValue(ifield, igate);

	// compute ix and iy based on the sector
	
	int ix, iy;
	if (_swapXy[isector]) {
	  ix = entry.iy * _xSign[isector] + _nxyHalf;
	  iy = entry.ix * _ySign[isector] + _nxyHalf;
	} else {
	  ix = entry.ix * _xSign[isector] + _nxyHalf;
	  iy = entry.iy * _ySign[isector] + _nxyHalf;
	}

	// adjust by 1 if the sign is negative, because we are
	// using 0-based indices

	if (_xSign[isector] == -1) {
	  ix -= 1;
	}
	if (_ySign[isector] == -1) {
	  iy -= 1;
	}

	int ipos = entry.iz * nbytesPlane + iy * _nxy + ix;
	_outputFields[ifield][ipos] = nearest;

      } // ifield

    } // isector

  } // ilut

  if (_params.debug) {
    cerr << "**** End PpiTransform::transformNearest() ****" << endl;
  }

}

