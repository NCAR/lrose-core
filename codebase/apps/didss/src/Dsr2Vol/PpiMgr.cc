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
// PpiMgr.cc
//
// Manager for PPI (plan view) volume
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2007
//
///////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/TaArray.hh>
#include <toolsa/toolsa_macros.h>

#include "PpiMgr.hh"
#include "CartTransform.hh"
#include "PpiTransform.hh"
#include "PolarTransform.hh"
#include "RhiTransform.hh"

using namespace std;

// Constructor

PpiMgr::PpiMgr(const string &prog_name,
               const Params &params,
               const vector<FieldInfo> &fields,
               const vector<Beam *> &beamsStored,
               const BeamGeomMgr &beamGeomMgr) :
        _progName(prog_name),
        _params(params),
        _fields(fields),
        _beamsStored(beamsStored),
        _beamGeomMgr(beamGeomMgr)
  
{
  
  _regularArray = NULL;

  _naz = 0;
  _nazPer45 = 0;
  _deltaAzRequested = 0;
  _deltaAzUsed = 0;

  _elevHist = NULL;
  _nElevHist = 0;
  _elevHistOffset = 0;
  _elevHistIntv = 0;
  _elevHistSearchWidth = 0;
  _nElev = 0;

  _fractionOfFullCircle = 0;

  // elevation histogram
  
  _elevHistIntv = _params. elev_hist_resolution;
  _nElevHist = (int) ((_params. elev_hist_end - _params. elev_hist_start) /
		      _elevHistIntv + 0.5);
  _elevHistOffset = (int) ((0.0 - _params. elev_hist_start) /
			   _elevHistIntv + 0.5);
  _elevHist = new int[_nElevHist];
  _elevHistSearchWidth = _params. elev_hist_search_width;

  if (_params.debug) {
    cerr << "========== Az / elev params ==========" << endl;
    cerr << "  _elevHistIntv: " << _elevHistIntv << endl;
    cerr << "  _nElevHist: " << _nElevHist << endl;
    cerr << "  _elevHistOffset: " << _elevHistOffset << endl;
    cerr << "  _elevHistSearchWidth: " << _elevHistSearchWidth << endl;
    cerr << "=================================" << endl;
  }

  // clear all arrays
  
  _clearAll();
  
  // set up transforms

  if (_params.output_cart_files) {
    for (int i = 0; i < _params.cart_files_n; i++) {
      PlanTransform *tr =
	new CartTransform(_params,
			  _params._cart_files[i].mdv_url,
                          _fields,
			  _params._cart_files[i].nxy,
			  _params._cart_files[i].dxy,
			  _params._cart_files[i].nz,
			  _params._cart_files[i].minz,
			  _params._cart_files[i].dz,
			  _params._cart_files[i].interpolate,
			  _params._cart_files[i].max_range);
      _transforms.push_back(tr);
    }
  }
  
  if (_params.output_ppi_files) {
    for (int i = 0; i < _params.ppi_files_n; i++) {
      PlanTransform *tr =
	new PpiTransform(_params,
			 _params._ppi_files[i].mdv_url,
                         _fields,
			 _params._ppi_files[i].nxy,
			 _params._ppi_files[i].dxy,
			 _params._ppi_files[i].min_elev,
			 _params._ppi_files[i].max_elev,
			 _params._ppi_files[i].interpolate,
			 _params._ppi_files[i].min_ht,
			 _params._ppi_files[i].max_ht,
			 _params._ppi_files[i].max_range);
      _transforms.push_back(tr);
    }
  }
  
  if (_params.output_polar_files) {
    for (int i = 0; i < _params.polar_files_n; i++) {
      PlanTransform *tr =
	new PolarTransform(_params,
			   _params._polar_files[i].mdv_url,
                           _fields,
			   _params._polar_files[i].max_range,
			   _params._polar_files[i].min_elev,
			   _params._polar_files[i].max_elev);
      _transforms.push_back(tr);
    }
  }

  if (_params.output_rhi_cart_files) {
    for (int i = 0; i < _params.rhi_cart_files_n; i++) {
      PlanTransform *tr =
	new CartTransform(_params,
			  _params._rhi_cart_files[i].mdv_url,
                          _fields,
			  _params._rhi_cart_files[i].nxy,
			  _params._rhi_cart_files[i].dxy,
			  _params._rhi_cart_files[i].nz,
			  _params._rhi_cart_files[i].minz,
			  _params._rhi_cart_files[i].dz,
			  _params._rhi_cart_files[i].interpolate,
			  _params._rhi_cart_files[i].max_range);
      tr->setGeomType(RHI_CART_OUTPUT_GRID);
      _transforms.push_back(tr);
    }
  }
  
}

/////////////////////////////////////////////////////////
// destructor

PpiMgr::~PpiMgr()

{

  _clearAll();

  if (_elevHist) {
    delete[] _elevHist;
  }

  for (size_t ii = 0; ii < _transforms.size(); ii++) {
    delete _transforms[ii];
  }

  _transforms.clear();

}

////////////////////////////////////////////////////////////////
// process PPI volume

void PpiMgr::processPpi(int volNum,
                        scan_mode_t scanMode,
			bool scan_info_from_headers,
                        const DsRadarParams &predomRadarParams,
                        const DsRadarCalib *radarCalib,
                        const string &statusXml,
                        time_t startTime, time_t endTime,
                        double beamWidth, double nyquist,
                        double radarLat, double radarLon, double radarAlt)
  
{
  
  PMU_auto_register("Processing PPI");
  if (_params.debug) {
    cerr << "**** Start PpiMgr::_processPpi() ****" << endl;
    cerr << "     Processing PPI volume" << endl;
  }

  // clear all arrays
  
  _clearAll();

  // set ppi mode and beam width

  _scanInfoFromHeaders = scan_info_from_headers;
  _beamWidth = beamWidth;

  // set the azimuth grid

  bool deltaAzChanged = false;
  if (scanMode == SCAN_MODE_RHI) {
    if (_setDeltaAz(_params.rhi_cart_delta_az)) {
      deltaAzChanged = true;
    }
  } else {
    if (_setDeltaAz(_beamGeomMgr.getPredomAngularRes())) {
      deltaAzChanged = true;
    }
  }

  // load up elev table
  
  _loadElevTable();
  if (_params.debug) {
    _printElevs();
  }

  if (_elevTable.size() == 0) {
    if (_params.debug) {
      cerr << "No elevations found, volume will be discarded" << endl;
    }
    return;
  }

  // determine scan mode if not already set in calling object

  if (scanMode != SCAN_MODE_VERT) {
    if (!_params.use_input_scan_mode || !_scanInfoFromHeaders) {
      if (_fractionOfFullCircle >= _params.min_fraction_for_surveillance) {
        scanMode = SCAN_MODE_SURVEILLANCE;
      } else {
        scanMode = SCAN_MODE_SECTOR;
      }
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "_fractionOfFullCircle: " << _fractionOfFullCircle << endl;
        if (scanMode == SCAN_MODE_SURVEILLANCE) {
          cerr << "scanMode: SURVEILLANCE" << endl;
        } else {
          cerr << "scanMode: SECTOR" << endl;
        }
      }
    }
  }    

  // load regular PPI array
  
  if (_params.use_input_tilt_numbers && _scanInfoFromHeaders) {
    
    // load up the beam array from the beam vector
    
    _loadRegularArrayFromTilts();
    
  } else {
    
    _loadRegularArrayFromHist();
    
  } // if (_params.use_tilt_numbers) 
    
  if (_params.bridge_missing_in_azimuth) {
    _bridgeMissingInAzimuth();
  }
  if (_params.bridge_missing_in_elevation) {
    _bridgeMissingInElevation();
  }
  
  // loop through transforms
  
  for (size_t ii = 0; ii < _transforms.size(); ii++) {

    PMU_auto_register("Transform");

    PlanTransform *tr = _transforms[ii];
    output_grid_geom_t geomType = tr->getGeomType();

    if (scanMode == SCAN_MODE_VERT && geomType != POLAR_OUTPUT_GRID) {
      // vertically-pointing data only applies to polar grid
      continue;
    }
    
    if (scanMode == SCAN_MODE_RHI && geomType != RHI_CART_OUTPUT_GRID) {
      // rhi data only applies to rhi cart grid
      continue;
    }

    if (scanMode != SCAN_MODE_RHI && geomType == RHI_CART_OUTPUT_GRID) {
      // rhi data only applies to rhi cart grid
      continue;
    }

    // set regular beam array
    
    tr->setRegularBeamArray(_regularArray);
    
    // set grid geometry
    // This also allocates the output field memory

    bool geomChanged = deltaAzChanged;
    if (tr->setGeom(_elevTable,
                    _beamGeomMgr.getPredomMaxNGates(),
                    _beamGeomMgr.getPredomStartRange(),
                    _beamGeomMgr.getPredomGateSpacing(),
                    _naz, _nazPer45, _deltaAzUsed,
                    beamWidth, radarLat, radarLon, radarAlt)) {
      geomChanged = true;
    }
  
    // If grid geometry changes, recompute the lookup
    
    if (geomChanged) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "===>>> PpiMgr: Calculating new lookup <<<===" << endl;
      }
      tr->calcLookup();
    }
    
    // transform to the output projection, using interpolation
    // or nearest neighbor as appropriate
    
    tr->doTransform(nyquist);
    
    // write the volume to MDV file
    
    tr->writeVol(predomRadarParams, radarCalib, statusXml,
                 volNum, startTime, endTime, scanMode);
    
    // free up the output fields between writes, to keep the
    // memory footprint reasonably small

    tr->freeOutputFields();

  } // ii

  if (_params.debug) {
    cerr << "**** End PpiMgr::_processPpi() ****" << endl;
  }

  // save elev table

  _prevElevTable = _elevTable;

}

////////////////////////////////////////////////////////////////
// Set the delta azimuth for the azimuth grid
//
// Returns true if grid has changed, false otherwise

bool PpiMgr::_setDeltaAz(double angular_res)
  
{

  // If the beams are indexed, the angular_res will be non-zero.
  // In that case, use it.
  // Otherwise use the delta azimuth from the param file
  
  double angularRes = _params.delta_az;
  if (angular_res > 0) {
    angularRes = angular_res;
  }
  
  if (fabs(angularRes - _deltaAzRequested) > 0.00001) {

    _deltaAzRequested = angularRes;

    if (_params.debug) {
      cerr << "PpiMgr::setDeltaAz()" << endl;
      cerr << "  Setting new delta azimuth: " << _deltaAzRequested << endl;
      if (angular_res > 0) {
        cerr << "  Indexed beams are in use" << endl;
      }
    }

    // number of beams
    
    _nazPer45 = (int) (45.0 / _deltaAzRequested + 0.5);
    _deltaAzUsed = 45.0 / _nazPer45;
    _naz = _nazPer45 * 8;

    if (_deltaAzUsed != _deltaAzRequested) {
      cerr << "WARNING - adjusting delta_az so that there are an" << endl
           << "          number of beams per 45 degree arc." << endl;
      cerr << endl;
    }
  
    if (_params.debug) {
      cerr << "  _deltaAz requested: " << _deltaAzRequested << endl;
      cerr << "  _deltaAz in use: " << _deltaAzUsed << endl;
      cerr << "  _nazPer45: " << _nazPer45 << endl;
      cerr << "  _naz: " << _naz << endl;
    }

    return true;

  }

  return false;

}

////////////////////////////////////////////////////////////////
// load the elevation table
//
// Returns true if elevation table has changed,
//         false otherwise

void PpiMgr::_loadElevTable()

{

  _elevTable.clear();

  if (_params.use_input_tilt_numbers && _scanInfoFromHeaders) {
    _tiltTable.clear();
    _tiltIndexLookup.clear();
    _loadElevTableFromTilts();
  } else if (_params.specify_elev_delta) {
    _calculateElevTable();
  } else {
    _loadElevTableFromHist();
  }
  
  _nElev = _elevTable.size();

}

////////////////////////////////////////////////////////////////
// load the elevation table from tilts
//
// Returns true if elevation table has changed,
//         false otherwise

void PpiMgr::_loadElevTableFromTilts()

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
    cerr << "WARNING: loadElevTableFromTilts(): " << endl;
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
      cerr << "WARNING - loadElevTableFromTilts()" << endl;
      cerr << "  Tilt numbers not valid" << endl;
      cerr << "  Using HIST instead of TILTS" << endl;
    }
    return _loadElevTableFromHist();
  }
  
  // compute mean elev angle for each tilt
  
  vector<tilt_t> tilts;

  for (int ii = 0; ii < nTilts; ii++) {
    tilt_t tilt;
    tilt.num = ii;
    tilt.sumElev = 0.0;
    tilt.meanElev = -9999.0;
    tilt.nBeams = 0;
    tilt.fractionFilled = 0;
    tilt.inUse = false;
    tilts.push_back(tilt);
  }

  double nBeamsTotal = 0.0;
  for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {
    Beam *beam = _beamsStored[ibeam];
    if (beam->accept) {
      int tiltNum = beam->tiltNum;
      if (tiltNum >= 0 && tiltNum < (int) tilts.size()) {
	double elev = beam->elev;
	tilts[tiltNum].sumElev += elev;
	tilts[tiltNum].nBeams++;
	nBeamsTotal++;
      }
    }
  }

  int maxBeamsInTilt = 0;
  for (int ii = 0; ii < nTilts; ii++) {
    tilt_t &tilt = tilts[ii];
    if (tilt.nBeams > 0) {
      double meanElev = tilt.sumElev / (double) tilt.nBeams;
      // round to 0.01
      meanElev = ((int) floor(meanElev * 100.0 + 0.5)) / 100.0;
      tilt.meanElev = meanElev;
    }
    if (tilt.nBeams > maxBeamsInTilt) {
      maxBeamsInTilt = tilt.nBeams;
    }
  }

  // compute fraction in each tilt compared with max number of beams

  if (maxBeamsInTilt > _naz) {
    maxBeamsInTilt = _naz;
  }
  
  for (int ii = 0; ii < nTilts; ii++) {
    tilt_t &tilt = tilts[ii];
    double fraction = (double) tilt.nBeams / maxBeamsInTilt;
    tilt.fractionFilled = fraction;
  }

  // go through the elevations, deciding if they are acceptable

  for (int ii = 0; ii < nTilts; ii++) {
    tilt_t &tilt = tilts[ii];
    
    if (_params.check_min_beams_in_tilt &&
	tilt.nBeams < _params.min_beams_in_tilt) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Rejecting tilt at " << tilt.meanElev << endl;
	cerr << "  Too few beams:" << tilt.nBeams << endl;
      }
      continue;
    }

    if (_params.check_min_fraction_in_tilt) {
      if (tilt.fractionFilled < _params.min_fraction_in_tilt) {
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "Rejecting tilt at " << tilt.meanElev << endl;
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
      const tilt_t &tilt = tilts[ii];
      cerr << "Tilt num, nBeams, meanElev, fractionFilled, inUse: "
           << ii << ", "
           << tilt.nBeams << ", "
           << tilt.meanElev << ", "
           << tilt.fractionFilled << ", "
           << (tilt.inUse? "T" : "F") << endl;
    }
    cerr << "Max beams in tilt: " << maxBeamsInTilt << endl;
  }
  
  // sort tilts into order

  vector<tilt_t> sorted = tilts;
  sort(sorted.begin(), sorted.end(), TiltCompare());
  
  // are the elevations in reverse order?

  //   bool elevsReversed = false;
  //   if (tilts[0].meanElev > tilts[nTilts-1].meanElev) {
  //     elevsReversed = true;
  //   }
  
  // load up elev table, tilt table and tilt index lookup
  
  int nBeamsUsed = 0;

  for (int ii = 0; ii < nTilts; ii++) {
    int tiltNum = sorted[ii].num;
    const tilt_t &tilt = tilts[tiltNum];
    if (tilt.inUse) {
      _tiltTable.push_back(tiltNum);
      _elevTable.push_back(tilt.meanElev);
      nBeamsUsed += tilt.nBeams;
    }
  }
  
//   if (elevsReversed) {
//     for (int ii = nTilts - 1; ii >= 0; ii--) {
//       const tilt_t &tilt = tilts[ii];
//       if (tilt.inUse) {
//         _tiltTable.push_back(ii);
//         _elevTable.push_back(tilt.meanElev);
//         nBeamsUsed += tilt.nBeams;
//       }
//     }
//   } else {
//     for (int ii = 0; ii < nTilts; ii++) {
//       const tilt_t &tilt = tilts[ii];
//       if (tilt.inUse) {
//         _tiltTable.push_back(ii);
//         _elevTable.push_back(tilt.meanElev);
//         nBeamsUsed += tilt.nBeams;
//       }
//     }
//   }

  for (int ii = 0; ii < nTilts; ii++) {
    int tiltIndex = -1;
    for (size_t jj = 0; jj < _tiltTable.size(); jj++) {
      if (_tiltTable[jj] == ii) {
        tiltIndex = jj;
        break;
      }
    }
    _tiltIndexLookup.push_back(tiltIndex);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "  tilt, index: " << ii << ", " << tiltIndex << endl;
    }
  }
  
  // compute full circle fraction

  _fractionOfFullCircle = (double) nBeamsUsed / ((double) _elevTable.size() * _naz);

}

////////////////////////////////////////////////////////////////
// load the elevation table
//
// Returns true if elevation table has changed,
//         false otherwise

void PpiMgr::_loadElevTableFromHist()

{

  // compute elevation histogram

  _computeElevHist();

  // search through the histogram, looking for peaks
  // within a given search angle

  vector<tilt_peak_t> peaks;
  int maxBeamsInTilt = 0;

  for (int i = _elevHistSearchWidth;
       i < _nElevHist - _elevHistSearchWidth; i++) {

    int count = _elevHist[i];
    
    // test for peak

    bool isPeak = true;
    if (count == 0) {
      isPeak = false;
    } else {
      if (count < _elevHist[i - 1] || count < _elevHist[i + 1]) {
        isPeak = false;
      }
      for (int j = 2; j <= _elevHistSearchWidth; j++) {
        if (count <= _elevHist[i - j] || count <= _elevHist[i + j]) {
          isPeak = false;
        }
      }
    }
      
    if (isPeak) {
      double thisPeak = (i - _elevHistOffset) * _elevHistIntv;
      bool storePeak = false;
      if (peaks.size() == 0) {
	storePeak = true;
      } else {
	double prevPeak = peaks[peaks.size()-1].elev;
	double diff = thisPeak - prevPeak;
	if (diff > _elevHistIntv * 3) {
	  storePeak = true;
	}
      }
	
      if (storePeak) {

	// store the characteristics of this peak
	
	tilt_peak_t peak;
	peak.elev = thisPeak;
	peak.nBeams = 0;
	for (int k = i - _elevHistSearchWidth;
	     k <= i + _elevHistSearchWidth; k++) {
	  peak.nBeams += _elevHist[k];
	}
	if (peak.nBeams > maxBeamsInTilt) {
	  maxBeamsInTilt = peak.nBeams;
	}
	peaks.push_back(peak);
	
      }

    } // if (isPeak)

  } // i

  if (maxBeamsInTilt > _naz) {
    maxBeamsInTilt = _naz;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < peaks.size(); ii++) {
      cerr << "Tilt elev: " << peaks[ii].elev
	   << ", nBeams: " << peaks[ii].nBeams << endl;
    }
    cerr << "Max beams in tilt: " << maxBeamsInTilt << endl;
  }

  // go through the peaks, deciding if they are acceptable

  double nBeamsUsed = 0.0;
  for (size_t ii = 0; ii < peaks.size(); ii++) {

    const tilt_peak_t &peak = peaks[ii];

    if (_params.check_min_beams_in_tilt &&
	peak.nBeams < _params.min_beams_in_tilt) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Rejecting peak at " << peak.elev << endl;
	cerr << "  Too few beams:" << peak.nBeams << endl;
      }
      continue;
    }

    if (_params.check_min_fraction_in_tilt) {
      double fraction = (double) peak.nBeams / maxBeamsInTilt;
      if (fraction < _params.min_fraction_in_tilt) {
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "Rejecting peak at " << peak.elev << endl;
	  cerr << "  Fraction too low:" << fraction << endl;
	}
	continue;
      }
    }
    
    // accept
    
    if (_elevTable.size() >= MDV_MAX_VLEVELS) {
      cerr << "WARNING: loadElevTableFromHist" << endl;
      cerr << "  Max number of elevations exceeded" << endl;
      cerr << "  MDV_MAX_VLEVELS: " << MDV_MAX_VLEVELS << endl;
      cerr << "  Rest of elevations ignored" << endl;
      continue;
    }

    _elevTable.push_back(peak.elev);
    nBeamsUsed += peaks[ii].nBeams;
    
  }

  // compute full circle fraction

  _fractionOfFullCircle = (double) nBeamsUsed / ((double) _elevTable.size() * _naz);

}

////////////////////////////////////////////////////////////////
// calculate the elevation table
//
// Returns true if elevation table has changed,
//         false otherwise

void PpiMgr::_calculateElevTable()

{

  // get min and max elevations

  double minElev = 1000.0;
  double maxElev = -1000.0;
  for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {
    Beam *beam = _beamsStored[ibeam];
    if (beam->accept) {
      double elev = beam->elev;
      minElev = MIN(minElev, elev);
      maxElev = MAX(maxElev, elev);
    }
  }

  // compute start elevation and delta elev

  double deltaElev = _params.elev_delta;
  double startElev = ((int) (minElev / deltaElev)) * deltaElev;
  double elevRange = maxElev - minElev;
  int nElev = (int) (elevRange / deltaElev + 1.0);

  // load elevation table

  for (int ii = 0; ii < nElev; ii++) {
    double elev = startElev + ii * deltaElev;
    _elevTable.push_back(elev);
  }

}

////////////////////////////////////////////////////////////////
// load the regular beam array from tilts

void PpiMgr::_loadRegularArrayFromTilts()

{

  if (_params.debug) {
    cerr << "**** Start PpiMgr::_loadPpiRegularArrayFromTilts ****" << endl;
  }

  // allocate the regular beam array - we allocate an extra
  // azimuth because the first az needs to be repeated as the
  // last az

  _freeRegularArray();
  _regularArray = (const Beam ***) ucalloc2(_nElev, _naz + 1, sizeof(Beam *));
  
  // load up the beam array from the beam vector
  
  for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {

    Beam *beam = _beamsStored[ibeam];
    if (!beam->accept) {
      continue;
    }

    // azimuth

    int iaz = (int) ((beam->az + _params.az_correction) / _deltaAzUsed + 0.5);
    if (iaz < 0) {
      iaz += _naz;
    } else if (iaz > _naz - 1) {
      iaz -= _naz;
    }
    
    // tilt
    
    int tiltNum = beam->tiltNum;
    int ielev = -1;
    if (tiltNum >= 0 && tiltNum  < (int) _tiltIndexLookup.size())  {
      ielev = _tiltIndexLookup[tiltNum];
    }
    if (ielev < 0) {
      // don't use this tilt
      continue;
    }
    
    _regularArray[ielev][iaz] = beam;

  } // ibeam

  // copy the last az from the first

  for (int ielev = 0; ielev < _nElev; ielev++) {
    _regularArray[ielev][_naz] = _regularArray[ielev][0];
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    for (int ielev = 0; ielev < _nElev; ielev++) {
      cerr << "Elevation: " << _elevTable[ielev] << endl;
      for (int iaz = 0; iaz <= _naz; iaz++) {
	if (_regularArray[ielev][iaz] == NULL) {
	  cerr << "missing ielev, iaz: " << ielev << ", " << iaz << ". ";
	} else {
	  cerr << _regularArray[ielev][iaz]->az << ",";
	}
      }
      cerr << endl;
    }
  }

  if (_params.debug) {
    cerr << "**** End PpiMgr::_loadPpiRegularArrayFromTilts ****" << endl;
  }

}

////////////////////////////////////////////////////////////////
// load the regular beam array from Histogram

void PpiMgr::_loadRegularArrayFromHist()

{

  if (_params.debug) {
    cerr << "**** Start PpiMgr::loadPpiRegularArrayFromHist ****" << endl;
  }

  // allocate the regular beam array - we allocate an extra
  // azimuth because the first az needs to be repeated as the
  // last az

  _freeRegularArray();
  _regularArray = (const Beam ***) ucalloc2(_nElev, _naz + 1, sizeof(Beam *));

  // load up the beam array from the beam vector
  
  double minElev = _elevTable[0] - _beamWidth / 2.0;
  double maxElev = _elevTable[_nElev - 1] + _beamWidth / 2.0;

  for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {

    Beam *beam = _beamsStored[ibeam];
    if (!beam->accept) {
      continue;
    }

    // azimuth

    int iaz = (int) ((beam->az + _params.az_correction) / _deltaAzUsed + 0.5);
    if (iaz < 0) {
      iaz += _naz;
    } else if (iaz > _naz - 1) {
      iaz -= _naz;
    }
    
    // elevation

    double elev = beam->elev;
    if (elev < minElev || elev > maxElev) {
      continue;
    }

    int ielev = _nElev - 1;
    for (int ii = 0; ii < _nElev - 1; ii++) {
      double mean = (_elevTable[ii] + _elevTable[ii+1]) / 2.0;
      if (elev <= mean) {
	ielev = ii;
	break;
      }
    } // ii

    if (fabs(elev - _elevTable[ielev]) > (_beamWidth / 2.0)) {
      continue;
    }
    
    _regularArray[ielev][iaz] = beam;

  } // ibeam

  // copy the last az from the first

  for (int ielev = 0; ielev < _nElev; ielev++) {
    _regularArray[ielev][_naz] = _regularArray[ielev][0];
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    for (int ielev = 0; ielev < _nElev; ielev++) {
      cerr << "Elevation: " << _elevTable[ielev] << endl;
      for (int iaz = 0; iaz <= _naz; iaz++) {
	if (_regularArray[ielev][iaz] == NULL) {
	  cerr << "missing ielev, iaz: " << ielev << ", " << iaz << ". ";
	} else {
	  cerr << _regularArray[ielev][iaz]->az << ",";
	}
      }
      cerr << endl;
    }
  }

  if (_params.debug) {
    cerr << "**** End PpiMgr::loadPpiRegularArrayFromHist ****" << endl;
  }

}

////////////////////////////////////////////////////////////////
// compute elevation histogram
//
// Returns the total number of beams used

int PpiMgr::_computeElevHist()

{
  
  _clearElevHist();

  int nBeamsHist = 0;
  for (size_t ibeam = 0; ibeam < _beamsStored.size(); ibeam++) {
    Beam *beam = _beamsStored[ibeam];
    if (beam->accept) {
      int elevBin = (int) (beam->elev / _elevHistIntv + _elevHistOffset + 0.5);
      if (elevBin < 0) {
	elevBin = 0;
      }
      if (elevBin > _nElevHist - 1) {
	elevBin = _nElevHist - 1;
      }
      _elevHist[elevBin]++;
      nBeamsHist++;
    }
  }

  return nBeamsHist;

}

////////////////////////////////////////////////////////////////
// bridge a missing beam in azimuth

void PpiMgr::_bridgeMissingInAzimuth()

{

  for (int ielev = 0; ielev < _nElev; ielev++) {

    for (int iaz = 1; iaz < _naz; iaz++) {

      if (_regularArray[ielev][iaz] == NULL) {
	
	if (_regularArray[ielev][iaz - 1] != NULL &&
	    _regularArray[ielev][iaz + 1] != NULL) {
	  Beam *beam = new Beam(*_regularArray[ielev][iaz - 1],
				*_regularArray[ielev][iaz + 1], 0.5);
	  _regularArray[ielev][iaz] =beam;
	  _beamsInterp.push_back(beam);
	}
	
      } // if (_regularArray[ielev][iaz] == NULL)

    } // iaz

    // north beam missing?

    if (_regularArray[ielev][0] == 0) {
      
      if (_regularArray[ielev][_naz - 1] != NULL &&
	  _regularArray[ielev][1] != NULL) {
	Beam *beam = new Beam(*_regularArray[ielev][_naz - 1],
			      *_regularArray[ielev][1], 0.5);
	_regularArray[ielev][0] =beam;
	_beamsInterp.push_back(beam);
      }

    }

  } // ielev

}

////////////////////////////////////////////////////////////////
// bridge a missing beam in elevation

void PpiMgr::_bridgeMissingInElevation()

{

  for (int iaz = 1; iaz <= _naz; iaz++) {

    for (int ielev = 1; ielev < _nElev - 1; ielev++) {

      if (_regularArray[ielev][iaz] == NULL) {
	
	if (_regularArray[ielev - 1][iaz] != NULL &&
	    _regularArray[ielev + 1][iaz] != NULL) {
	  Beam *beam = new Beam(*_regularArray[ielev - 1][iaz],
				*_regularArray[ielev + 1][iaz], 0.5);
	  _regularArray[ielev][iaz] = beam;
	  _beamsInterp.push_back(beam);
	}
	
      } // if (_regularArray[ielev][iaz] == NULL)

    } // ielev

  } // iaz

}

/////////////////////////////////////////////////////////////////
// Print elevation and fields

void PpiMgr::_printElevs()

{

  cerr << endl;

  bool histInUse = false;
  for (int ii = 0; ii < _nElevHist; ii++) {
    if (_elevHist[ii] > 0) {
      histInUse = true;
      break;
    }
  }

  if (histInUse > 0) {
    cerr << "Elevation histogram" << endl;
    cerr << "===================" << endl;
    for (int ii = 0; ii < _nElevHist; ii++) {
      if (_elevHist[ii] > 0) {
        cerr << "  Elev: " << (ii - _elevHistOffset) * _elevHistIntv
             << ", count: " << _elevHist[ii] << endl;
      }
    } // ii
    cerr << endl;
  }

  if (_elevTable.size() > 0) {
    cerr << "Elevation table" << endl;
    cerr << "===============" << endl;
    for (size_t ii = 0; ii < _elevTable.size(); ii++) {
      cerr << "  Elev #: " << ii << ", angle: " << _elevTable[ii];
      if (ii < _tiltTable.size()) {
        cerr << ", Tilt: " << _tiltTable[ii];
      }
      cerr << endl;
    } // ii
    cerr << endl;
  }

}
  
/////////////////////////////////////////////////////////////////
// reset and clear methods

void PpiMgr::_clearAll()

{
  _clearElevHist();
  _clearBeamsInterp();
  _freeRegularArray();
}

void PpiMgr::_clearBeamsInterp()
{

  for (size_t ii = 0; ii < _beamsInterp.size(); ii++) {
    delete _beamsInterp[ii];
  }
  _beamsInterp.clear();

}

void PpiMgr::_clearElevHist()
{
  memset(_elevHist, 0, _nElevHist * sizeof(int));
}

void PpiMgr::_freeRegularArray()
{
  if (_regularArray != NULL) {
    ufree2((void **) _regularArray);
    _regularArray = NULL;
  }
}

