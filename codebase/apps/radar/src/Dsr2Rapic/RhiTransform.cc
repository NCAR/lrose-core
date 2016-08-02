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
// RhiTransform.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2003
//
///////////////////////////////////////////////////////////////////////

#include "RhiTransform.hh"
#include <math.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <Mdv/Mdvx.hh>

using namespace std;

RhiTransform::RhiTransform (const Params &params,
			    const string &mdv_url,
			    double oversampling_ratio,
			    bool interp_in_elevation,
			    const vector<Beam *> &beams,
			    const vector<FieldInfo> &fields,
			    int n_gates,
			    double start_range,
			    double gate_spacing,
			    double beam_width,
			    double radar_lat,
			    double radar_lon,
			    double radar_alt) :
  
  Transform(params, mdv_url),
  _oversamplingRatio(oversampling_ratio),
  _interpInElevation(interp_in_elevation),
  _beams(beams),
  _fields(fields)
  
{

  _nFieldsOut = _params.output_fields_n;
  _outputFields = NULL;
  _geomType = RHI_OUTPUT_GRID;

  _nGates = n_gates;
  _startRange = start_range;
  _gateSpacing = gate_spacing;
  _beamWidth = beam_width;
  _radarLat = radar_lat;
  _radarLon = radar_lon;
  _radarAlt = radar_alt;

  // azimuth histogram
  
  _azHistIntv = _params. rhi_az_hist_resolution;
  _nAzHist = (int) (360.0 / _azHistIntv + 0.5);
  _azHist = new int[_nAzHist];
  _azHistSearchWidth = _params.rhi_az_hist_search_width;

  _nRhi = _nElRhi = 0;
  _minElRhi = _deltaElRhi = 0.0;
  _rhiArray = NULL;
  
  _nx = _ny = _nz = 0;
  _dx = _dy = _dz = 0.0;
  _minx = _miny = _minz = 0.0;

}

RhiTransform::~RhiTransform ()
{
  _clearAll();
}

////////////////////////////////////////////////////////////////
// Load RHI from beams
//
// Returns 0 on success, -1 on failure

int RhiTransform::load()

{

  PMU_auto_register("Loading RHI");
  
  if (_params.debug) {
    cerr << "**** Start RhiTransform::load() ****" << endl;
  }

  if (_loadRhiTable()) {
    if (_params.debug) {
      cerr << "  WARNING - no RHIs found" << endl;
    }
    return -1;
  }
  
  _loadRhiVector();
  _loadRhiRegularArray();
  _loadMdvOutput();

  if (_params.debug) {
    cerr << "**** End RhiTransform::load() ****" << endl;
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// write the output volume

int RhiTransform::writeVol(const DsRadarParams &radarParams,
			   time_t startTime, time_t endTime,
			   const vector<FieldInfo> &fields)
  
{

  PMU_auto_register("RhiTransform::writeVol");

  if (_params.debug) {
    cerr << "**** Start RhiTransform::writeVol() ****" << endl;
  }
  
  OutputMdv mdv("Dsr2Rapic-RHI", _params, _geomType);
  
  int volDuration = endTime - startTime;
  if (volDuration > _params.max_vol_duration) {
    if (_params.debug) {
      cerr << "WARNING - RhiTransform::_writeVol" << endl;
      cerr << "  Vol duration exceed max allowable" << endl;
      cerr << "  Vol duration: " << volDuration << " secs" << endl;
      cerr << "  Max duration: " << _params.max_vol_duration
	   << " secs" << endl;
    }
    return -1;
  }

  time_t midTime;
  if (_params.auto_mid_time) {
    midTime = startTime + (endTime - startTime) / 2;
  } else {
    midTime = endTime - _params.age_at_end_of_volume;
  }
  
  if (_params.debug) {
    cerr << "startTime: " << DateTime::str(startTime) << endl;
    cerr << "endTime: " << DateTime::str(endTime) << endl;
    cerr << "midTime: " << DateTime::str(midTime) << endl;
  }
  
  mdv.setMasterHeader(startTime, midTime, endTime,
		      _nx, _ny, _nz,
		      _radarLat, _radarLon, _radarAlt,
		      radarParams.radarName.c_str());

  for (size_t ifield = 0; ifield < fields.size(); ifield++) {
    const FieldInfo &fld = fields[ifield];
    if (fld.isLoaded) {
      mdv.addField(fld.name.c_str(),
		   fld.units.c_str(),
		   false,
		   _nx, _dx, _minx,
		   _ny, _dy, _miny,
		   _nz, _dz, _minz,
		   _vlevels,
		   _radarLat,
		   _radarLon,
		   _radarAlt,
		   fld.byteWidth,
		   fld.scale,
		   fld.bias,
		   fld.encoding,
		   fld.compression,
		   _outputFields[ifield]);
    }
  } // ifield
  
  mdv.addChunks(radarParams, _vlevels);

  if (mdv.writeVol(_mdvUrl.c_str())) {
    cerr << "ERROR - RhiTransform::writeVol" << endl;
    cerr << "  Cannot write output volume" << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "**** End RhiTransform::writeVol() ****" << endl;
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// load the RHI azimuth table
//
// Returns 0 on success, -1 on failure

int RhiTransform::_loadRhiTable()
  
{
  
  // compute the azimuth histogram
  
  _computeAzHist();
  
  // search through the histogram, looking for peaks
  // within a given search angle
  
  vector<rhi_t> rhis;
  int maxBeamsInRhi = 0;
  
  for (int i = 0; i < _nAzHist; i++) {
    
    int count = _azHist[i];
    
    // test for peak
    
    bool isPeak = true;
    if (count < _azHist[_azIndex(i - 1)] ||
	count < _azHist[_azIndex(i + 1)]) {
      isPeak = false;
    }
    for (int j = 2; j <= _azHistSearchWidth; j++) {
      if (count <= _azHist[_azIndex(i - j)] ||
	  count <= _azHist[_azIndex(i + j)]) {
	isPeak = false;
      }
    }
    
    if (isPeak) {
      double thisPeak = i * _azHistIntv;
      bool storePeak = false;
      if (rhis.size() == 0) {
	storePeak = true;
      } else {
	double prevPeak = rhis[rhis.size()-1].peakAz;
	double diff = thisPeak - prevPeak;
	if (diff > _azHistIntv * 3) {
	  storePeak = true;
	}
      }
      
      if (storePeak) {
	
	// store the characteristics of this peak
	
	rhi_t peak;
	peak.peakAz = thisPeak;
	peak.nbeams = 0;
	for (int k = i - _azHistSearchWidth;
	     k <= i + _azHistSearchWidth; k++) {
	  peak.nbeams += _azHist[_azIndex(k)];
	}
	if (peak.nbeams > maxBeamsInRhi) {
	  maxBeamsInRhi = peak.nbeams;
	}
	rhis.push_back(peak);
	
      }

    } // if (isPeak)

  } // i
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < rhis.size(); ii++) {
      cerr << "RHI az: " << rhis[ii].peakAz
	   << ", nbeams: " << rhis[ii].nbeams << endl;
    }
    cerr << "Max beams in RHI: " << maxBeamsInRhi << endl;
  }
  
  // go through the peaks, deciding if they are acceptable

  _rhiTable.clear();
  
  for (size_t ii = 0; ii < rhis.size(); ii++) {

    if (_params.check_min_beams_in_rhi &&
	rhis[ii].nbeams < _params.min_beams_in_rhi) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Rejecting RHI peak at " << rhis[ii].peakAz << endl;
	cerr << "  Too few beams:" << rhis[ii].nbeams << endl;
      }
      continue;
    }
    
    if (_params.check_min_fraction_in_rhi) {
      double fraction = (double) rhis[ii].nbeams / maxBeamsInRhi;
      if (fraction < _params.min_fraction_in_rhi) {
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "Rejecting RHI peak at " << rhis[ii].peakAz << endl;
	  cerr << "  Fraction too low: " << fraction << endl;
	}
	continue;
      }
    }
    
    // accept

    if (_rhiTable.size() < MDV_MAX_VLEVELS) {
      _rhiTable.push_back(rhis[ii]);
    } else {
      cerr << "ERROR - Dsr2Rapic::RhiTransform::_loadRhiTable()" << endl;
      cerr << "  Too many RHIs found" << endl;
      cerr << "  Only " << MDV_MAX_VLEVELS << " allowed" << endl;
      break;
    }

  } // ii

  // sort by peak azimuth

  sort(_rhiTable.begin(), _rhiTable.end(), RhiComparePeakAz());

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "VALID RHI LIST:" << endl;
    for (size_t ii = 0; ii < _rhiTable.size(); ii++) {
      cerr << "  index, az, nbeams: " << ii
	   << ", " << _rhiTable[ii].peakAz
	   << ", " << _rhiTable[ii].nbeams << endl;
    }
  }

  if (_rhiTable.size() == 0) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// compute azimuth histogram

void RhiTransform::_computeAzHist()

{
  
  _clearAzHist();
  
  for (size_t ibeam = 0; ibeam < _beams.size(); ibeam++) {
    Beam *beam = _beams[ibeam];
    if (beam->accept) {
      int azBin = (int) (beam->az / _azHistIntv + 0.5);
      if (azBin < 0) {
	azBin = 0;
      }
      if (azBin > _nAzHist - 1) {
	azBin = _nAzHist - 1;
      }
      _azHist[azBin]++;
    }
  }

}

////////////////////////////////////////////////////////////////
// load the RHI vectors containing the beam pointers
//
// This is a 2-D vector<vector<Beam *> >.
// The outer dimension is for azimuth.
// The inner dimension is for elevation.

void RhiTransform::_loadRhiVector()
  
{

  _rhiVec.clear();

  for (size_t iaz = 0; iaz < _rhiTable.size(); iaz++) {

    vector<Beam *> rhi;
    
    double azTarget = _rhiTable[iaz].peakAz;
    double azMargin =
      _params.rhi_az_hist_search_width * _params.rhi_az_hist_resolution;
    double minAz = azTarget - azMargin;
    double maxAz = azTarget + azMargin;
    
    for (size_t ibeam = 0; ibeam < _beams.size(); ibeam++) {
      if (_beams[ibeam]->az >= minAz &&
	  _beams[ibeam]->az <= maxAz) {
	rhi.push_back(_beams[ibeam]);
      }
    } // ibeam

    // sort by elevation
    
    sort(rhi.begin(), rhi.end(), BeamCompareElev());
    
    // set min and max el

    _rhiTable[iaz].minElev = rhi[0]->elev;
    _rhiTable[iaz].maxElev = rhi[rhi.size()-1]->elev;

    // add to vector
  
    _rhiVec.push_back(rhi);
    
  } // iaz

  // compute mean az

  for (size_t iaz = 0; iaz < _rhiTable.size(); iaz++) {
    vector<Beam *> &rhi = _rhiVec[iaz];
    double sumAz = 0.0;
    for (size_t ielev = 0; ielev < rhi.size(); ielev++) {
      sumAz += rhi[ielev]->az;
    }
    double meanAz = ((int) ((sumAz / rhi.size()) * 10.0)) / 10.0;
    _rhiTable[iaz].meanAz = meanAz;
  } // iaz

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t iaz = 0; iaz < _rhiTable.size(); iaz++) {
      vector<Beam *> &rhi = _rhiVec[iaz];
      cerr << "RHI for target az: " << _rhiTable[iaz].peakAz << endl;
      cerr << "  Mean az: " << _rhiTable[iaz].meanAz << endl;
      cerr << "  Nelev: " << rhi.size() << endl;
      cerr << "  Min elev: " << _rhiTable[iaz].minElev << endl;
      cerr << "  Max elev: " << _rhiTable[iaz].maxElev << endl;
      cerr << "  elevs: ";
      for (size_t ielev = 0; ielev < rhi.size(); ielev++) {
	cerr << rhi[ielev]->elev << " ";
      }
      cerr << endl;
    } // iaz
  }

}

////////////////////////////////////////////////////////////////
// load the regular RHI beam array

void RhiTransform::_loadRhiRegularArray()

{

  // compute min and max elevation, and delta elevation
  
  double minEl = 90.0, maxEl = -90.0;
  double sumDeltaEl = 0.0;
  double count = 0.0;

  for (size_t iaz = 0; iaz < _rhiTable.size(); iaz++) {
    if (minEl > _rhiTable[iaz].minElev) {
      minEl = _rhiTable[iaz].minElev;
    }
    if (maxEl < _rhiTable[iaz].maxElev) {
      maxEl = _rhiTable[iaz].maxElev;
    }
    double delta = ((_rhiTable[iaz].maxElev - _rhiTable[iaz].minElev) /
		    (_rhiTable[iaz].nbeams - 1.0));
    sumDeltaEl += delta;
    count++;
  }

  double deltaEl = sumDeltaEl / count;

  // compute rhi elevation spacing to oversample
  
  double deltaElSample = deltaEl / _oversamplingRatio;
  _deltaElRhi = ((int) (deltaElSample * 10.0)) / 10.0;
  if (_deltaElRhi < 0.1) {
    _deltaElRhi = 0.1;
  }
  _minElRhi = floor(minEl * 10.0 + 0.5) / 10.0;
  _nElRhi = (int) ((maxEl - _minElRhi) / _deltaElRhi + 2.0);
  _nRhi = _rhiTable.size();

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "RHI geometry: " << endl;
    cerr << "  deltaEl: " << deltaEl << endl;
    cerr << "  _deltaElRhi: " << _deltaElRhi << endl;
    cerr << "  minEl: " << minEl << endl;
    cerr << "  maxEl: " << maxEl << endl;
    cerr << "  _minElRhi: " << _minElRhi << endl;
    cerr << "  _nElRhi: " << _nElRhi << endl;
    cerr << "  _nRhi: " << _nRhi << endl;
    double maxElRhi = _minElRhi + (_nElRhi - 1) * _deltaElRhi;
    cerr << "  max el RHI: " << maxElRhi << endl;
  } // iaz
  
  // allocate the regular beam array
  
  _clearRhiArray();
  _rhiArray = (Beam ***) ucalloc2(_nRhi, _nElRhi, sizeof(Beam *));

  // load up the beam array from the beam vector
  
  for (int iaz = 0; iaz < _nRhi; iaz++) {
    
    vector<Beam *> &rhi = _rhiVec[iaz];
    
    for (int iel = 0; iel < _nElRhi; iel++) {
      
      double elTarget = _minElRhi + iel * _deltaElRhi;
      
      // don't process outside the measured data
      
      if (elTarget < _rhiTable[iaz].minElev ||
	  elTarget > _rhiTable[iaz].maxElev) {
	continue;
      }
	
      if (_interpInElevation) {
	
	// interpolate
	
	for (size_t ii = 0; ii < rhi.size() - 1; ii++) {
	  if (rhi[ii]->elev <= elTarget &&
	      rhi[ii + 1]->elev >= elTarget) {
	    double range = rhi[ii + 1]->elev - rhi[ii]->elev;
	    double weight1 = (rhi[ii + 1]->elev - elTarget) / range;
	    Beam *beam =
	      new Beam(*rhi[ii], *rhi[ii + 1], weight1);
	    _rhiArray[iaz][iel] = beam;
	    _beamsInterp.push_back(beam);
	    break;
	  }
	} // ii

      } else {
	
	// nearest neighbor
	// find the closest beam in elevation
	
	double minDiff = 360.0;
	size_t iMin = 0;
	for (size_t ii = 0; ii < rhi.size(); ii++) {
	  double diff = fabs(rhi[ii]->elev - elTarget);
	  if (minDiff > diff) {
	    minDiff = diff;
	    iMin = ii;
	  }
	} // ii
	_rhiArray[iaz][iel] = rhi[iMin];

      }

    } // iel
    
  } // iaz

}

////////////////////////////////////////////////////////////////
// load the output field data

void RhiTransform::_loadMdvOutput()

{

  _nx = _nGates;
  _dx = _gateSpacing;
  _minx = _startRange;
  
  _ny = _nElRhi;
  _miny = _minElRhi;
  _dy = _deltaElRhi;
  
  _vlevels.clear();
  for (size_t ii = 0; ii < _rhiTable.size(); ii++) {
    _vlevels.push_back(_rhiTable[ii].meanAz);
  }
  
  _nz = _nRhi;
  _minz = _vlevels[0];
  if (_vlevels.size() < 2) {
    _dz = 1.0;
  } else {
    _dz = ((_vlevels[_vlevels.size()-1] - _vlevels[0]) /
	   (_vlevels.size() - 1.0));
  }

  _allocOutputFields();
  
  int nbytesRhi = _nElRhi * _nGates;
  
  for (int iaz = 0; iaz < _nRhi; iaz++) {
    
    for (int ielev = 0; ielev < _nElRhi; ielev++) {

      Beam *beam = _rhiArray[iaz][ielev];
      
      if (beam != NULL) {
	
	int outPos = iaz * nbytesRhi + ielev * _nGates;
	
	for (int ifield = 0; ifield < _nFieldsOut; ifield++) {

	  int ngatesCopy;
	  if (beam->nGates < _nGates) {
	    ngatesCopy  = beam->nGates;
	  } else {
	    ngatesCopy = _nGates;
	  }
	  
	  for (int igate = 0; igate < ngatesCopy; igate++) {
	    _outputFields[ifield][outPos + igate] =
	      beam->getValue(ifield, igate);
	  } // igate

	} // ifield
	
      } // if (beam != NULL)

    } // ielev

  } // iaz

}

////////////////////////////////////////////////////////////////////
// azimuth index
//

int RhiTransform::_azIndex(int i)
{
  if (i < 0) {
    return i + _nAzHist;
  } else if (i >= _nAzHist) {
    return i - _nAzHist;
  } else {
    return i;
  }
}

//////////////////////////////////////////////////////////
// clear methods

void RhiTransform::_clearAll()

{
  freeOutputFields();
  _clearAzHist();
  _clearRhiArray();
  _clearBeamsInterp();
}

void RhiTransform::_clearAzHist()
{
  memset(_azHist, 0, _nAzHist * sizeof(int));
}

void RhiTransform::_clearRhiArray()
{
  if (_rhiArray != NULL) {
    ufree2((void **) _rhiArray);
    _rhiArray = NULL;
  }
}

void RhiTransform::_clearBeam(Beam *beam)
{
  if (beam != NULL) {
    ufree2((void **) beam->fieldData);
    delete beam;
  }
}

void RhiTransform::_clearBeamsInterp()
{
  for (size_t ii = 0; ii < _beamsInterp.size(); ii++) {
    _clearBeam(_beamsInterp[ii]);
  }
  _beamsInterp.clear();
}

///////////////////////////////////////
// function to compare RHI by azimuths

bool RhiComparePeakAz::operator()
  (rhi_t const &a, rhi_t const &b) const
{
  if (a.peakAz < b.peakAz) {
    return true;
  } else {
    return false;
  }
}

//////////////////////////////////////
// class to compare beams by elevation

bool BeamCompareElev::operator()
  (Beam* const &a, Beam* const &b) const
{
  if (a->elev < b->elev) {
    return true;
  } else {
    return false;
  }
}

