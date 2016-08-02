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

#include <cmath>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <Mdv/Mdvx.hh>
#include <dsserver/DsLdataInfo.hh>
#include "RhiTransform.hh"

using namespace std;

RhiTransform::RhiTransform (const Params &params,
			    const string &mdv_url,
			    const vector<FieldInfo> &fields,
			    double oversampling_ratio,
			    bool interp_in_elevation,
			    const vector<Beam *> &beamsStored,
			    bool scan_info_from_headers,
			    int n_gates,
			    double start_range,
			    double gate_spacing,
                            double angular_res,
			    double beam_width,
			    double radar_lat,
			    double radar_lon,
			    double radar_alt) :
  
        Transform(params, mdv_url, fields),
        _oversamplingRatio(oversampling_ratio),
        _interpInElevation(interp_in_elevation),
	_scanInfoFromHeaders(scan_info_from_headers),
        _beamsStored(beamsStored)
  
{

  _nFieldsOut = _params.output_fields_n;
  _outputFields = NULL;
  _azHist = NULL;
  _geomType = RHI_OUTPUT_GRID;

  _nGates = n_gates;
  _startRange = start_range;
  _gateSpacing = gate_spacing;
  _angularRes = angular_res;

  _beamWidth = beam_width;
  _radarLat = radar_lat;
  _radarLon = radar_lon;
  _radarAlt = radar_alt;

  // azimuth histogram
  
  _azHistIntv = _params.rhi_az_hist_resolution;
  _nAzHist = (int) (360.0 / _azHistIntv + 0.5);
  _azHist = new int[_nAzHist];
  _azHistSearchWidth = _params.rhi_az_hist_search_width;

  _nEl = 0;
  _minEl = _deltaEl = 0.0;
  _regArray = NULL;
  
  _nx = _ny = _nz = 0;
  _dx = _dy = _dz = 0.0;
  _minx = _miny = _minz = 0.0;

}

RhiTransform::~RhiTransform ()
{
  _clearAll();
  if (_azHist) {
    delete[] _azHist;
  }
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

  // load up azimuth table

  _loadAzTable();

  // load beam vector
  
  if (_params.use_input_tilt_numbers && _scanInfoFromHeaders) {
    _loadBeamVectorFromTilts();
  } else {
    _loadBeamVectorFromHist();
  }

  // check size of az table

  if (_azTable.size() == 0) {
    if (_params.debug) {
      cerr << "No RHI azimuths found, volume will be discarded" << endl;
    }
    return -1;
  }

  // sort the beam vector

  _sortBeamVector();

  // load regular beam array
  // if angularRes is 0, the beams are not indexed
  // if greater than 0, the beams are indexed

  if (_angularRes > 0) {
    _loadRegBeamArrayIndexed();
  } else {
    _loadRegBeamArrayNonIndexed();
  }

  // load the MDV output data

  _loadMdvOutput();

  if (_params.debug) {
    cerr << "**** End RhiTransform::load() ****" << endl;
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// write the output volume

int RhiTransform::writeVol(const DsRadarParams &radarParams,
                           const DsRadarCalib *calib,
                           const string &statusXml,
			   int volNum,
                           time_t startTime, time_t endTime,
                           scan_mode_t scanMode)
  
{

  PMU_auto_register("RhiTransform::writeVol");

  if (_params.debug) {
    cerr << "**** Start RhiTransform::writeVol() ****" << endl;
  }
  
  OutputMdv mdv("Dsr2Vol-RHI", _params, _geomType);
  
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
  
  mdv.setMasterHeader(volNum, startTime, midTime, endTime,
		      _nx, _ny, _nz,
		      _radarLat, _radarLon, _radarAlt,
		      radarParams.radarName.c_str());

  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    const FieldInfo &fld = _fields[ifield];
    if (fld.isLoaded) {
      mdv.addField(fld.name.c_str(),
		   fld.units.c_str(),
		   fld.isDbz,
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
  
  mdv.addChunks(radarParams, calib, statusXml, _vlevels);

  if (mdv.writeVol(_mdvUrl.c_str())) {
    cerr << "ERROR - RhiTransform::writeVol" << endl;
    cerr << "  Cannot write output volume" << endl;
    return -1;
  }
  
  // optionally register the master URL with the DataMapper

  if (_params.write_master_ldata_info) {
    mdv.writeMasterLdataInfo();
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

void RhiTransform::_loadAzTable()
  
{

  _azTable.clear();
  
  if (_params.use_input_tilt_numbers && _scanInfoFromHeaders) {
    _loadAzTableFromTilts();
  } else {
    _loadAzTableFromHist();
  }
  
}
  
////////////////////////////////////////////////////////////////
// load the RHI azimuth table from tilt numbers
//
// Returns 0 on success, -1 on failure

void RhiTransform::_loadAzTableFromTilts()
  
{

  // compute number of tilts
  
  int maxTilt = -9999;
  for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {
    Beam *beam = _beamsStored[ibeam];
    if (beam->accept) {
      maxTilt = MAX(maxTilt, beam->tiltNum);
    }
  }
  int nTilts = maxTilt + 1;
  if (nTilts > MDV_MAX_VLEVELS) {
    cerr << "WARNING: loadAzTableFromTilts()" << endl;
    cerr << "  Too many tilts: " << nTilts << endl;
    cerr << "  MDV_MAX_VLEVELS: " << MDV_MAX_VLEVELS << endl;
    cerr << "  Reducing nTilts to: " << MDV_MAX_VLEVELS << endl;
    nTilts = MDV_MAX_VLEVELS;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  nTilts: " << nTilts << endl;
  }

  // Did we find tilt numbers?
  // if not, use elevation histogram technique
  
  if (nTilts < 1) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - loadAzTableFromTilts()" << endl;
      cerr << "  Tilt numbers not valid" << endl;
      cerr << "  Using HIST instead of TILTS" << endl;
    }
    _loadAzTableFromHist();
  }
  
  // initialize tilt vector
  
  vector<az_t> tilts;
  for (int ii = 0; ii < nTilts; ii++) {
    az_t tilt;
    MEM_zero(tilt);
    tilts.push_back(tilt);
  }

  // compute mean elev angle for each tilt, as well as fraction active
  
  double nBeamsTotal = 0.0;
  for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {
    Beam *beam = _beamsStored[ibeam];
    if (beam->accept) {
      int tiltNum = beam->tiltNum;
      if (tiltNum >= 0 && tiltNum < (int) tilts.size()) {
	az_t &tilt = tilts[tiltNum];
	tilt.sumAz += beam->az;
	tilt.nBeams++;
	nBeamsTotal++;
      }
    }
  }
  
  int maxBeamsInTilt = 0;
  for (int ii = 0; ii < nTilts; ii++) {
    az_t &tilt = tilts[ii];
    tilt.tiltNum = ii;
    if (tilt.nBeams > 0) {
      tilt.meanAz = tilt.sumAz / (double) tilt.nBeams;
    }
    if (tilt.nBeams > maxBeamsInTilt) {
      maxBeamsInTilt = tilt.nBeams;
    }
  }

  // compute fraction in each tilt compared with max number of beams

  for (int ii = 0; ii < nTilts; ii++) {
    az_t &tilt = tilts[ii];
    tilt.fractionFilled = (double) tilt.nBeams / maxBeamsInTilt;
  }

  // go through the tilts, determining which are acceptable
  
  for (int ii = 0; ii < nTilts; ii++) {

    az_t &tilt = tilts[ii];
    
    if (_params.check_min_beams_in_rhi &&
	tilt.nBeams < _params.min_beams_in_rhi) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Rejecting tilt at " << tilt.meanAz << endl;
	cerr << "  Too few beams:" << tilt.nBeams << endl;
      }
      continue;
    }
    
    if (_params.check_min_fraction_in_rhi) {
      if (tilt.fractionFilled < _params.min_fraction_in_rhi) {
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "Rejecting tilt at " << tilt.meanAz << endl;
	  cerr << "  Fraction too low:" << tilt.fractionFilled << endl;
	}
	continue;
      }
    }
    
    // accept
    
    tilt.inUse = true;
    
  } // ii
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (int ii = 0; ii < nTilts; ii++) {
      const az_t &tilt = tilts[ii];
      cerr << "Tilt num, nBeams, meanAz, fractionFilled, inUse: "
           << ii << ", "
           << tilt.nBeams << ", "
           << tilt.meanAz << ", "
           << tilt.fractionFilled << ", "
           << (tilt.inUse? "T" : "F") << endl;
    }
    cerr << "Max beams in tilt: " << maxBeamsInTilt << endl;
  }
  
  // load up az table with those in use
  
  for (int ii = 0; ii < nTilts; ii++) {
    const az_t &tilt = tilts[ii];
    if (tilt.inUse) {
      _azTable.push_back(tilt);
    }
  }

}
  
////////////////////////////////////////////////////////////////
// load the RHI azimuth table from histogram
//
// Returns 0 on success, -1 on failure

void RhiTransform::_loadAzTableFromHist()
  
{
  
  // compute the azimuth histogram
  
  _computeAzHist();
  
  // search through the histogram, looking for peaks
  // within a given search angle
  
  vector<az_t> azimuths;
  int maxBeamsInRhi = 0;
  
  for (int i = 0; i < _nAzHist; i++) {
    
    int count = _azHist[i];
    
    // test for peak
    
    bool isPeak = true;
    if (count == 0) {
      isPeak = false;
    } else {
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
    }
    
    if (isPeak) {
      double thisPeak = i * _azHistIntv;
      bool storePeak = false;
      if (azimuths.size() == 0) {
	storePeak = true;
      } else {
	double prevPeak = azimuths[azimuths.size()-1].peakAz;
	double diff = thisPeak - prevPeak;
	if (diff > _azHistIntv * 3) {
	  storePeak = true;
	}
      }
      
      if (storePeak) {
	
	// store the characteristics of this peak
	
	az_t peak;
        MEM_zero(peak);
	peak.peakAz = thisPeak;
	for (int k = i - _azHistSearchWidth;
	     k <= i + _azHistSearchWidth; k++) {
	  peak.nBeams += _azHist[_azIndex(k)];
	}
	if (peak.nBeams > maxBeamsInRhi) {
	  maxBeamsInRhi = peak.nBeams;
	}
	azimuths.push_back(peak);
	
      }

    } // if (isPeak)

  } // i
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < azimuths.size(); ii++) {
      cerr << "RHI az: " << azimuths[ii].peakAz
	   << ", nBeams: " << azimuths[ii].nBeams << endl;
    }
    cerr << "Max beams in RHI: " << maxBeamsInRhi << endl;
  }
  
  // go through the peaks, deciding if they are acceptable

  for (size_t ii = 0; ii < azimuths.size(); ii++) {

    if (_params.check_min_beams_in_rhi &&
	azimuths[ii].nBeams < _params.min_beams_in_rhi) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Rejecting RHI peak at " << azimuths[ii].peakAz << endl;
	cerr << "  Too few beams:" << azimuths[ii].nBeams << endl;
      }
      continue;
    }
    
    if (_params.check_min_fraction_in_rhi) {
      double fraction = (double) azimuths[ii].nBeams / maxBeamsInRhi;
      if (fraction < _params.min_fraction_in_rhi) {
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "Rejecting RHI peak at " << azimuths[ii].peakAz << endl;
	  cerr << "  Fraction too low: " << fraction << endl;
	}
	continue;
      }
    }
    
    // accept

    if (_azTable.size() < MDV_MAX_VLEVELS) {
      azimuths[ii].inUse = true;
      _azTable.push_back(azimuths[ii]);
    } else {
      cerr << "ERROR - Dsr2Vol::RhiTransform::_loadAzTableFromHist()" << endl;
      cerr << "  Too many azimuths found" << endl;
      cerr << "  Only " << MDV_MAX_VLEVELS << " allowed" << endl;
      break;
    }

  } // ii

  // sort by peak azimuth

  sort(_azTable.begin(), _azTable.end(), RhiCompareAz());

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "VALID RHI LIST:" << endl;
    for (size_t ii = 0; ii < _azTable.size(); ii++) {
      cerr << "  index, az, nBeams: " << ii
	   << ", " << _azTable[ii].peakAz
	   << ", " << _azTable[ii].nBeams << endl;
    }
  }

}

////////////////////////////////////////////////////////////////
// compute azimuth histogram

void RhiTransform::_computeAzHist()

{
  
  _clearAzHist();
  
  for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {
    Beam *beam = _beamsStored[ibeam];
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
// load the vectors containing the beam pointers
//
// This is a 2-D vector<vector<Beam *> >.
// The outer dimension is for azimuth.
// The inner dimension is for elevation.

void RhiTransform::_loadBeamVectorFromTilts()
  
{

  _beamVec.clear();
  
  // make a temporary azimuth table, so we can not
  // include azimuths without any beams

  vector<az_t> tmpTable; 

  // loop through the tilts

  for (size_t iaz = 0; iaz < _azTable.size(); iaz++) {
    
    vector<Beam *> beams;
    const az_t &az = _azTable[iaz];
    int tiltNum = az.tiltNum;
    
    for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {
      if (_beamsStored[ibeam]->tiltNum == tiltNum) {
	beams.push_back(_beamsStored[ibeam]);
      }
    } // ibeam
    
    // add to vector

    if (beams.size() > 0) {
      _beamVec.push_back(beams);
      tmpTable.push_back(_azTable[iaz]);
    }
    
  } // iaz

  // copy the tmp table to the azimuth table
  
  _azTable = tmpTable;

}

////////////////////////////////////////////////////////////////
// load the vectors containing the beam pointers, from hist
//
// Also computes min and max elev
//
// This is a 2-D vector<vector<Beam *> >.
// The outer dimension is for azimuth.
// The inner dimension is for elevation.

void RhiTransform::_loadBeamVectorFromHist()
  
{

  _beamVec.clear();

  // make a temporary azimuth table, so we can not
  // include azimuths without any beams

  vector<az_t> tmpTable; 

  for (size_t iaz = 0; iaz < _azTable.size(); iaz++) {

    vector<Beam *> beams;
    
    double azTarget = _azTable[iaz].peakAz;
    double azMargin =
      _params.rhi_az_hist_search_width * _params.rhi_az_hist_resolution;
    double minAz = azTarget - azMargin;
    double maxAz = azTarget + azMargin;
    
    for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {
      if (_beamsStored[ibeam]->az >= minAz &&
	  _beamsStored[ibeam]->az <= maxAz) {
	beams.push_back(_beamsStored[ibeam]);
      }
    } // ibeam

    // add to vector
    
    if (beams.size() > 0) {
      _beamVec.push_back(beams);
      tmpTable.push_back(_azTable[iaz]);
    }
    
  } // iaz

  // copy the tmp table to the azimuth table

  _azTable = tmpTable;

}

////////////////////////////////////////////////////////////////
// Sort the beam vector, both in azimuth and elevation

void RhiTransform::_sortBeamVector()
  
{

  // loop through the tilts
  
  for (size_t iaz = 0; iaz < _beamVec.size(); iaz++) {
    
    // sort by elevation
    
    vector<Beam *> beams = _beamVec[iaz];
    sort(beams.begin(), beams.end(), BeamCompareElev());
    
    // set min and max el
    
    az_t &az = _azTable[iaz];
    az.minElev = beams[0]->elev;
    az.maxElev = beams[beams.size()-1]->elev;

  } // iaz

  // compute mean az, rounded to 0.01
  
  for (size_t iaz = 0; iaz < _azTable.size(); iaz++) {
    vector<Beam *> &beams = _beamVec[iaz];
    double sumAz = 0.0;
    for (size_t ielev = 0; ielev < beams.size(); ielev++) {
      sumAz += beams[ielev]->az;
    }
    double meanAz = ((int) ((sumAz / beams.size()) * 100.0)) / 100.0;
    _azTable[iaz].meanAz = meanAz;
  } // iaz

  // sort by mean azimuth

  sort(_azTable.begin(), _azTable.end(), RhiCompareAz());

  // check for north crossing
  // assume a gap in the RHI coverage of at least 45 degrees

  double azRange = _azTable[_azTable.size() - 1].meanAz - _azTable[0].meanAz;
  double azGap = 45.0;
  if (azRange > (360.0 - azGap)) {
    // set high az values to minus values
    for (int ii = (int)_azTable.size() - 1; ii > 0; ii--) {
      double deltaAz = _azTable[ii].meanAz - _azTable[ii-1].meanAz;
      _azTable[ii].meanAz -= 360.0;
      if (deltaAz > azGap) {
        break;
      }
    }
  }

  // re-sort by azimuth

  sort(_azTable.begin(), _azTable.end(), RhiCompareAz());
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t iaz = 0; iaz < _azTable.size(); iaz++) {
      const az_t &az = _azTable[iaz];
      const vector<Beam *> beams = _beamVec[iaz];
      cerr << "RHI num, az, nbeams, minEl, maxEl: "
           << iaz << ", "
           << az.meanAz << ", "
           << beams.size() << ", "
           << az.minElev << ", "
           << az.maxElev << endl;
    } // iaz
  }

}

////////////////////////////////////////////////////////////////
// load the regular RHI beam array
// for indexed beams

void RhiTransform::_loadRegBeamArrayIndexed()

{

  // compute min and max elevation
  
  _minEl = 90.0;
  _maxEl = -90.0;
  
  for (size_t iaz = 0; iaz < _azTable.size(); iaz++) {
    if (_minEl > _azTable[iaz].minElev) {
      _minEl = _azTable[iaz].minElev;
    }
    if (_maxEl < _azTable[iaz].maxElev) {
      _maxEl = _azTable[iaz].maxElev;
    }
  }
  
  // delta elevation is the angular resolution
  
  _deltaEl = _angularRes;

  // compute dimensions

  _nEl = (int) ((_maxEl - _minEl) / _deltaEl + 2.0);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "RHI geometry - INDEXED beams: " << endl;
    cerr << "  _deltaEl: " << _deltaEl << endl;
    cerr << "  _minEl: " << _minEl << endl;
    cerr << "  _maxEl: " << _maxEl << endl;
    cerr << "  _nEl: " << _nEl << endl;
    cerr << "  _nAz: " << _azTable.size() << endl;
  } // iaz
  
  // allocate the regular beam array
  
  _clearRegArray();
  _regArray = (Beam ***) ucalloc2((int) _azTable.size(),
                                  _nEl, sizeof(Beam *));

  // load up the beam array from the beam vector
  
  for (int iaz = 0; iaz < (int) _azTable.size(); iaz++) {
    
    vector<Beam *> &az = _beamVec[iaz];
    
    for (int iel = 0; iel < _nEl; iel++) {
      
      double elTarget = _minEl + iel * _deltaEl;
      
      // don't process outside the measured data
      
      if (elTarget < _azTable[iaz].minElev ||
	  elTarget > _azTable[iaz].maxElev) {
	continue;
      }
	
      // find the closest beam in elevation
      
      double minDiff = 360.0;
      size_t iMin = 0;
      for (size_t ii = 0; ii < az.size(); ii++) {
        double diff = fabs(az[ii]->elev - elTarget);
        if (minDiff > diff) {
          minDiff = diff;
          iMin = ii;
        }
      } // ii
      _regArray[iaz][iel] = az[iMin];
      
    } // iel
    
  } // iaz

}

////////////////////////////////////////////////////////////////
// load the regular RHI beam array
// for non-indexed beams

void RhiTransform::_loadRegBeamArrayNonIndexed()

{

  // compute min and max elevation, and delta elevation
  
  double minEl = 90.0, maxEl = -90.0;
  double sumDeltaEl = 0.0;
  double count = 0.0;

  for (size_t iaz = 0; iaz < _azTable.size(); iaz++) {
    if (minEl > _azTable[iaz].minElev) {
      minEl = _azTable[iaz].minElev;
    }
    if (maxEl < _azTable[iaz].maxElev) {
      maxEl = _azTable[iaz].maxElev;
    }
    double delta = ((_azTable[iaz].maxElev - _azTable[iaz].minElev) /
		    (_azTable[iaz].nBeams - 1.0));
    sumDeltaEl += delta;
    count++;
  }

  double deltaEl = sumDeltaEl / count;

  // compute rhi elevation spacing to oversample
  
  double deltaElSample = deltaEl / _oversamplingRatio;
  _deltaEl = ((int) (deltaElSample * 10.0)) / 10.0;
  if (_deltaEl < 0.1) {
    _deltaEl = 0.1;
  }
  _minEl = floor(minEl * 10.0 + 0.5) / 10.0;
  _nEl = (int) ((maxEl - _minEl) / _deltaEl + 2.0);
  _maxEl = _minEl + (_nEl - 1) * _deltaEl;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "RHI geometry - NON-INDEXED beams: " << endl;
    cerr << "  deltaEl: " << deltaEl << endl;
    cerr << "  _deltaEl: " << _deltaEl << endl;
    cerr << "  minEl: " << minEl << endl;
    cerr << "  maxEl: " << maxEl << endl;
    cerr << "  _minEl: " << _minEl << endl;
    cerr << "  _maxEl: " << _maxEl << endl;
    cerr << "  _nEl: " << _nEl << endl;
    cerr << "  _nAz: " << _azTable.size() << endl;
  } // iaz
  
  // allocate the regular beam array
  
  _clearRegArray();
  _regArray = (Beam ***) ucalloc2(_azTable.size(),
                                  _nEl, sizeof(Beam *));

  // load up the beam array from the beam vector
  
  for (int iaz = 0; iaz < (int) _azTable.size(); iaz++) {
    
    vector<Beam *> &az = _beamVec[iaz];
    
    for (int iel = 0; iel < _nEl; iel++) {
      
      double elTarget = _minEl + iel * _deltaEl;
      
      // don't process outside the measured data
      
      if (elTarget < _azTable[iaz].minElev ||
	  elTarget > _azTable[iaz].maxElev) {
	continue;
      }
	
      if (_interpInElevation) {
	
	// interpolate
	
	for (size_t ii = 0; ii < az.size() - 1; ii++) {
	  if (az[ii]->elev <= elTarget &&
	      az[ii + 1]->elev >= elTarget) {
	    double range = az[ii + 1]->elev - az[ii]->elev;
	    double weight1 = (az[ii + 1]->elev - elTarget) / range;
	    Beam *beam =
	      new Beam(*az[ii], *az[ii + 1], weight1);
	    _regArray[iaz][iel] = beam;
	    _beamsInterp.push_back(beam);
	    break;
	  }
	} // ii

      } else {
	
	// nearest neighbor
	// find the closest beam in elevation
	
	double minDiff = 360.0;
	size_t iMin = 0;
	for (size_t ii = 0; ii < az.size(); ii++) {
	  double diff = fabs(az[ii]->elev - elTarget);
	  if (minDiff > diff) {
	    minDiff = diff;
	    iMin = ii;
	  }
	} // ii
	_regArray[iaz][iel] = az[iMin];

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
  
  _ny = _nEl;
  _miny = _minEl;
  _dy = _deltaEl;
  
  _vlevels.clear();
  for (size_t ii = 0; ii < _azTable.size(); ii++) {
    _vlevels.push_back(_azTable[ii].meanAz);
  }
  
  _nz = (int) _azTable.size();
  _minz = _vlevels[0];
  if (_vlevels.size() < 2) {
    _dz = 1.0;
  } else {
    _dz = ((_vlevels[_vlevels.size()-1] - _vlevels[0]) /
	   (_vlevels.size() - 1.0));
  }

  _allocOutputFields();
  
  int nbytesRhi = _nEl * _nGates;
  
  for (int iaz = 0; iaz < (int) _azTable.size(); iaz++) {
    
    for (int ielev = 0; ielev < _nEl; ielev++) {

      Beam *beam = _regArray[iaz][ielev];
      
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
  _clearRegArray();
  _clearBeamsInterp();
}

void RhiTransform::_clearAzHist()
{
  memset(_azHist, 0, _nAzHist * sizeof(int));
}

void RhiTransform::_clearRegArray()
{
  if (_regArray != NULL) {
    ufree2((void **) _regArray);
    _regArray = NULL;
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
// function to compare azimuths

bool RhiCompareAz::operator()
  (az_t const &a, az_t const &b) const
{
  if (a.meanAz < b.meanAz) {
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

