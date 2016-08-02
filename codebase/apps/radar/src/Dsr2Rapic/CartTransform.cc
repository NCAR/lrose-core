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
// CartTransform.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2001
//
///////////////////////////////////////////////////////////////////////

#include "CartTransform.hh"
#include "Dsr2Rapic.hh"
#include <math.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/TaArray.hh>
using namespace std;

CartTransform::CartTransform (const Params &params,
			      int nxy, double dxy,
			      int nz, double minz, double dz,
			      bool interp,
			      const string &mdv_url,
			      double max_range) :
  PlanTransform(params, mdv_url)
  
{

  _interp = interp;
  _geomType = CART_OUTPUT_GRID;

  if (nxy % 2 != 0) {
    _nxy = nxy + 1;
    cerr << "WARNING - CartTransform::CartTransform" << endl;
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
  _nz = nz;

  _dxy = dxy;
  _dx = _dxy;
  _dy = _dxy;
  _dz = dz;

  _minx = -1.0 * _dxy * (_nxy / 2.0 - 0.5);
  _miny = -1.0 * _dxy * (_nxy / 2.0 - 0.5);
  _minz = minz;

  _maxRange = max_range;

}

CartTransform::~CartTransform ()
{

}

int CartTransform::setGeom(const vector<double> vlevel_array,
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

  // copy the elev array

  _elevArray.clear();
  _elevArray = vlevel_array;
  _nElev = _elevArray.size();

  // allocate space for output data

  _allocOutputFields();

  if (_params.debug) {
    cerr << "========== Cart grid params ==========" << endl;
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

void CartTransform::calcLookup()

{

  PMU_auto_register("Cart::calcLookup");

  if (_params.debug) {
    cerr << "** start CartTransform::calcLookup()" << endl;
  }
  
  _lut.clear();

  // set up range and elevation limits

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
    cerr << "  nxyHalf: " << _nxyHalf << endl;
  }

  // set up for loop

  double minElev = _elevArray[0] - _beamWidth / 2.0;
  double maxElev = _elevArray[_nElev - 1] + _beamWidth / 2.0;

  TaArray<double> zzArray_;
  double *zzArray = zzArray_.alloc(_nz);
  for (int iz = 0; iz < _nz; iz++) {
    zzArray[iz] = (_minz + iz * _dz) - _radarAlt;
  }

  // loop through y,x,z

  for (int iy = 0; iy < _nxyHalf; iy++) {
    
    PMU_auto_register("Cart::calcLookup");

    double yy = (iy + 0.5) * _dxy;
    
    for (int ix = 0; ix <= iy; ix++) {
      
      double xx = (ix + 0.5) * _dxy;
      
      // azimuth
      
      double az = atan2(xx, yy) * RAD_TO_DEG;
      double azNum = az / _deltaAz;
      
      for (int iz = 0; iz < _nz; iz++) {

	double zz = zzArray[iz];

	// compute range and check
	
	double range = sqrt((xx * xx) + (yy * yy) + (zz * zz));
	if (range < minRange || range > maxRangeUsed) {
	  continue;
	}
	
	// compute elevation and check
	
	double zcorr = (range * range) / PSEUDO_DIAM;
	double zht = zz - zcorr;
	double elev = asin(zht/range) * RAD_TO_DEG;

	if (std::isnan(elev)){
	  cerr << "\a\a NAN for elevation! zht=" << zht;
	  cerr << " range =" << range << endl;
	  cerr << "Is altitude set correctly (in Km not meters)?" << endl;
	  continue;
	}

	if (elev < minElev || elev > maxElev) {
	  continue;
	}
	
	// fill out lookup table entry
	
	plan_transform_lut_t entry;
	entry.iz = iz;
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

	if (_nElev == 1) {
	  entry.ielev = 0;
	  entry.wtElev = 1.0;
	} else if (elev < _elevArray[0]) {
	  entry.ielev = 0;
	  entry.wtElev = 1.0;
	} else if (elev > _elevArray[_nElev - 1]) {
	  entry.ielev = _nElev - 2;
	  entry.wtElev = 0.0;
	} else {
	  bool elevFound = false;
	  for (int ielev = 0; ielev < _nElev - 1; ielev++) {
	    
	    if (elev >= _elevArray[ielev] &&
		elev <= _elevArray[ielev+1]) {
	      entry.ielev = ielev;
	      double dElev = _elevArray[ielev+1] - _elevArray[ielev];
	      entry.wtElev = (_elevArray[ielev+1] - elev) / dElev;
	      elevFound = true;
	      break;
	    }
	  } // ielev	  

	  if (!elevFound) {
	    cerr << "\a\a **** should not get here ****" << endl;
	    cerr << "elev : " << elev << endl;
	    for (int ni=0; ni < _nElev; ni++){
	      cerr << "elev[" << ni << "]: " << _elevArray[ni] << endl;
	    }
	    continue; // should not get here
	  }
	}

	// add entry to lookup table

	_lut.push_back(entry);
	
      } // iz
      
    } // ix
    
  } // iy
  
  if (_params.debug) {
    cerr << "** end CartTransform::calcLookup()" << endl;
    cerr << "  n lookup: " << _lut.size() << endl;
  }

}

////////////////////////////////////////////////////////////////
// transform to cart output projection
//

void CartTransform::doTransform(const vector<FieldInfo> &fields,
				double nyquist,
				Beam ***beam_array)

{
  if (_interp && _nElev > 1) {
    _transformInterp(fields, nyquist, beam_array);
  } else {
    _transformNearest(fields, beam_array);
  }
}

////////////////////////////////////////////////////////////////
// transform to cart output projection using interpolation
//

void CartTransform::_transformInterp(const vector<FieldInfo> &fields,
				     double nyquist,
				     Beam ***beam_array)

{

  PMU_auto_register("Cart::interp");

  if (_params.debug) {
    cerr << "**** Start CartTransform::_transformInterp() ****" << endl;
  }

  int nbytesPlane = _nxy * _nxy;
    
  // loop through the fields
  
  for (int ifield = 0; ifield < _nFieldsOut; ifield++) {
    
    PMU_auto_register("Cart::interp");
    
    fl32 missFl32 = Beam::missFl32;
    const FieldInfo &field = fields[ifield];
    bool interpDbAsPower = field.interpDbAsPower;
    bool isVel = field.isVel;
    bool allowInterp = field.allowInterp;
    bool replaceMissing = false;
    if (field.isDbz && _params.override_missing_dbz_for_interp) {
      replaceMissing = true;
    }
    fl32 lowDbz = -40.0;
    
    // loop through the lookup table
    
    int count = 0;

    for (size_t ilut = 0; ilut < _lut.size(); ilut++) {

      // register every so often

      count++;
      if (count == 10000) {
	PMU_auto_register("Cart::interp");
	count = 0;
      }
      
      const plan_transform_lut_t entry = _lut[ilut];
      if (entry.ielev < 0) {
	continue;
      }
      
      // loop through the sectors
      
      for (int isector = 0; isector < 8; isector++) {
	
	int iaz1 = entry.iaz * _azSign[isector] + _azOffset[isector];
	int iaz2 = iaz1 + _azSign[isector];
	
	// load up the 8 data points around the target point
	
	fl32 lowElLowAzLowGate = missFl32;
	fl32 lowElLowAzHighGate = missFl32;
	fl32 lowElHighAzLowGate = missFl32;
	fl32 lowElHighAzHighGate = missFl32;
	fl32 highElLowAzLowGate = missFl32;
	fl32 highElLowAzHighGate = missFl32;
	fl32 highElHighAzLowGate = missFl32;
	fl32 highElHighAzHighGate = missFl32;
	
	Beam *beam;
	beam = beam_array[entry.ielev][iaz1];
	if (beam) {
	  if (beam->nGates > entry.igate) {
	    lowElLowAzLowGate = beam->getValue(ifield, entry.igate);
	  }
	  if (beam->nGates > entry.igate + 1) {
	    lowElLowAzHighGate = beam->getValue(ifield, entry.igate + 1);
	  }
	}
	
	beam = beam_array[entry.ielev][iaz2];
	if (beam) {
	  if (beam->nGates > entry.igate) {
	    lowElHighAzLowGate = beam->getValue(ifield, entry.igate);
	  }
	  if (beam->nGates > entry.igate + 1) {
	    lowElHighAzHighGate = beam->getValue(ifield, entry.igate + 1);
	  }
	}
	
	beam = beam_array[entry.ielev + 1][iaz1];
	if (beam) {
	  if (beam->nGates > entry.igate) {
	    highElLowAzLowGate = beam->getValue(ifield, entry.igate);
	  }
	  if (beam->nGates > entry.igate + 1) {
	    highElLowAzHighGate = beam->getValue(ifield, entry.igate + 1);
	  }
	}

	beam = beam_array[entry.ielev + 1][iaz2];
	if (beam) {
	  if (beam->nGates > entry.igate) {
	    highElHighAzLowGate = beam->getValue(ifield, entry.igate);
	  }
	  if (beam->nGates > entry.igate + 1) {
	    highElHighAzHighGate = beam->getValue(ifield, entry.igate + 1);
	  }
	}
	
	// check that we have sufficient contributing points
	
	int contribCount = 0;
	contribCount += (lowElLowAzLowGate != missFl32);
	contribCount += (lowElHighAzLowGate != missFl32);
	contribCount += (lowElLowAzHighGate != missFl32);
	contribCount += (lowElHighAzHighGate != missFl32);
	contribCount += (highElLowAzLowGate != missFl32);
	contribCount += (highElHighAzLowGate != missFl32);
	contribCount += (highElLowAzHighGate != missFl32);
	contribCount += (highElHighAzHighGate != missFl32);

	if (contribCount < _params.min_nvalid_for_interp) {
	  continue;
	}
	
	// average in azimuth
	
	fl32 lowElLowGate = interp(lowElLowAzLowGate, lowElHighAzLowGate,
				   entry.wtAz, interpDbAsPower,
				   isVel, nyquist, allowInterp);
	
	fl32 lowElHighGate = interp(lowElLowAzHighGate, lowElHighAzHighGate,
				    entry.wtAz, interpDbAsPower,
				    isVel, nyquist, allowInterp);
	
	fl32 highElLowGate = interp(highElLowAzLowGate, highElHighAzLowGate,
				    entry.wtAz, interpDbAsPower,
				    isVel, nyquist, allowInterp);
	
	fl32 highElHighGate = interp(highElLowAzHighGate, highElHighAzHighGate,
				     entry.wtAz, interpDbAsPower,
				     isVel, nyquist, allowInterp);

	// average in range
	
	fl32 lowEl = interp(lowElLowGate, lowElHighGate,
			    entry.wtGate, interpDbAsPower,
			    isVel, nyquist, allowInterp);
	
	fl32 highEl = interp(highElLowGate, highElHighGate,
			     entry.wtGate, interpDbAsPower,
			     isVel, nyquist, allowInterp);
	
	// average in elevation
	
	fl32 interpVal = interp(lowEl, highEl, entry.wtElev,
				interpDbAsPower, isVel, nyquist,
				allowInterp, replaceMissing, lowDbz);
	
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

  // create coverage field if requested

  if (_params.output_coverage_field) {
    _loadCoverage();
  }

  if (_params.debug) {
    cerr << "**** End CartTransform::_transformInterp() ****" << endl;
  }

}

////////////////////////////////////////////////////////////////
// transform to cart output projection using nearest neighbor
//

void CartTransform::_transformNearest(const vector<FieldInfo> &fields,
				      Beam ***beam_array)

{

  PMU_auto_register("Cart::nearest");

  if (_params.debug) {
    cerr << "**** Start CartTransform::transformNearest() ****" << endl;
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
      PMU_auto_register("Cart::nearest");
      count = 0;
    }
    
    const plan_transform_lut_t entry = _lut[ilut];

    int ielev;
    if (entry.wtElev > 0.5) {
      ielev = entry.ielev;
    } else {
      ielev = entry.ielev + 1;
    }
    
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

  // create coverage field if requested

  if (_params.output_coverage_field) {
    _loadCoverage();
  }

  if (_params.debug) {
    cerr << "**** End CartTransform::transformNearest() ****" << endl;
  }

}

////////////////////////////////////////////////////////////////
// load coverage field
//

void CartTransform::_loadCoverage()

{

  int nbytesPlane = _nxy * _nxy;
    
  for (size_t ilut = 0; ilut < _lut.size(); ilut++) {
    
    const plan_transform_lut_t entry = _lut[ilut];
    if (entry.ielev < 0) {
      continue;
    }
    
    for (int isector = 0; isector < 8; isector++) {
      
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
      _coverage[ipos] = 1;
      
    } // isector
    
  } // ilut
  
}

