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
// SweepMgr.cc
//
// Manager for sweep scan strategy
//
// Sets sweep numbers in input beams as appropriate
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2011
//
///////////////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/TaArray.hh>
#include <toolsa/toolsa_macros.h>

#include "SweepMgr.hh"

using namespace std;

// Constructor

SweepMgr::SweepMgr(const string &prog_name,
                   const Params &params) :
        _progName(prog_name),
        _params(params)
  
{

  _isRhi = false;
  _hist = NULL;
  _nHist = 0;
  _histOffset = 0;
  _histIntv = 0;
  _histSearchWidth = 0;

}

/////////////////////////////////////////////////////////
// destructor

SweepMgr::~SweepMgr()

{

  _freeHist();

}

////////////////////////////////////////////////////////////////
// set sweep numbers, from the histogram

void SweepMgr::setSweepNumbers(bool isRhi,
                               const vector<RadxRay *> &rays)
  
{
  
  PMU_auto_register("Setting sweep numbers");
  if (_params.debug) {
    cerr << "**** Start SweepMgr::setSweepNumbers ****" << endl;
  }
  
  _isRhi = isRhi;

  // load up angle table from histogram
  
  _loadAngleTableFromHist(rays);

  if (_params.debug) {
    _printAngles();
  }

  // check we have some sweeps
  
  if (_angleTable.size() == 0) {

    if (_params.debug) {
      cerr << "WARNING - SweepMgr::setSweepNumbers()" << endl;
      cerr << "  No sweeps found" << endl;
      cerr << "  All sweep numbers will be set to 0" << endl;
    }
    
    for (size_t ii = 0; ii < rays.size(); ii++) {
      RadxRay *ray = rays[ii];
      if (ray->getSweepNumber() < 0) {
        ray->setSweepNumber(0);
      }
    }

    return;

  }

  // set sweep numbers from histogram

  int nAngles = _angleTable.size();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    RadxRay *ray = rays[ii];
    double angle = ray->getElevationDeg();
    if (_isRhi) {
      angle = ray->getAzimuthDeg();
    }
    int iang = nAngles - 1;
    for (int jj = 0; jj < nAngles - 1; jj++) {
      double mean = (_angleTable[jj] + _angleTable[jj+1]) / 2.0;
      if (angle <= mean) {
        iang = jj;
        break;
      }
    } // jj
    ray->setSweepNumber(iang);
    if (!_params.use_target_angles_for_hist) {
      // set fixed angle
      ray->setFixedAngleDeg(_angleTable[iang]);
    }
  }
  
  if (_params.debug) {
    cerr << "**** End SweepMgr::setSweepNumbers() ****" << endl;
  }

}

////////////////////////////////////////////////////////////////
// load the angle table from the histogram
//
// Returns true if angle table has changed,
//         false otherwise

void SweepMgr::_loadAngleTableFromHist(const vector<RadxRay *> rays)

{

  // initialize

  _angleTable.clear();

  // compute histogram
  
  _computeHist(rays);
    
  // search through the histogram, looking for peaks
  // within a given search angle

  vector<sweep_peak_t> peaks;
  int maxBeamsInSweep = 0;

  for (int ii = _histSearchWidth; ii < _nHist - _histSearchWidth; ii++) {

    int count = _hist[ii];
    
    // test for peak

    bool isPeak = true;
    if (count == 0) {
      isPeak = false;
    } else {
      if (count < _hist[ii - 1] || count < _hist[ii + 1]) {
        isPeak = false;
      }
      for (int jj = 2; jj <= _histSearchWidth; jj++) {
        if (count <= _hist[ii - jj] || count <= _hist[ii + jj]) {
          isPeak = false;
        }
      }
    }

    if (isPeak) {
      double thisPeak = (ii - _histOffset) * _histIntv;
      bool storePeak = false;
      if (peaks.size() == 0) {
	storePeak = true;
      } else {
	double prevPeak = peaks[peaks.size()-1].angle;
	double diff = thisPeak - prevPeak;
	if (diff > _histIntv * 3) {
	  storePeak = true;
	}
      }
	
      if (storePeak) {

	// store the characteristics of this peak
	
	sweep_peak_t peak;
	peak.angle = thisPeak;
	peak.nBeams = 0;
	for (int kk = ii - _histSearchWidth;
             kk <= ii + _histSearchWidth; kk++) {
	  peak.nBeams += _hist[kk];
	}
	if (peak.nBeams > maxBeamsInSweep) {
	  maxBeamsInSweep = peak.nBeams;
	}
	peaks.push_back(peak);
	
      }

    } // if (isPeak)

  } // ii

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "========== Histogram counts ==========" << endl;
    for (int ii = 0; ii < _nHist; ii++) {
      if (_hist[ii] > 0) {
        double angleBin = (_histOffset * -1.0 + ii) * _histIntv;
        cerr << "  hist[" << ii << "], angle(deg), count: "
             << angleBin << ", " << _hist[ii] << endl;
      }
    } // ii
    cerr << "========== Histogram peaks ==========" << endl;
    cerr << "  No of peaks: " << peaks.size() << endl;
    for (size_t ii = 0; ii < peaks.size(); ii++) {
      cerr << "Sweep angle (deg): " << peaks[ii].angle
	   << ", nBeams: " << peaks[ii].nBeams << endl;
    }
    cerr << "  Max beams in sweep: " << maxBeamsInSweep << endl;
  }

  // go through the peaks, deciding if they are acceptable

  double nBeamsUsed = 0.0;
  for (size_t ii = 0; ii < peaks.size(); ii++) {
    
    const sweep_peak_t &peak = peaks[ii];

    if (_params.check_min_rays_in_sweep &&
	peak.nBeams < _params.min_rays_in_sweep) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Rejecting peak at " << peak.angle << endl;
	cerr << " Too few rays:" << peak.nBeams << endl;
      }
      continue;
    }

    // accept
    
    _angleTable.push_back(peak.angle);
    nBeamsUsed += peaks[ii].nBeams;
    
  }

}

////////////////////////////////////////////////////////////////
// compute histogram
//
// Returns the total number of beams used

int SweepMgr::_computeHist(const vector<RadxRay *> rays)

{
  
  double histAngleStart = -90;
  double histAngleEnd = 90;
  
  if (_isRhi) {
    histAngleStart = 0;
    histAngleEnd = 360;
  }

  _histIntv = _params.hist_angle_resolution;
  _histOffset = (int) ((0.0 - histAngleStart) / _histIntv + 0.5);
  _histSearchWidth = _params.hist_search_width;
  _nHist = (int) ((histAngleEnd - histAngleStart) / _histIntv + 0.5);
  _freeHist();
  _hist = new int[_nHist];
  
  if (_params.debug) {
    cerr << "========== Histogram details ==========" << endl;
    if (_isRhi) {
      cerr << "  RHI mode" << endl;
    } else {
      cerr << "  PPI mode" << endl;
    }
    cerr << "  histAngleStart: " << histAngleStart << endl;
    cerr << "  histAngleEnd: " << histAngleEnd << endl;
    cerr << "  _histIntv: " << _histIntv << endl;
    cerr << "  _nHist: " << _nHist << endl;
    cerr << "  _histOffset: " << _histOffset << endl;
    cerr << "  _histSearchWidth: " << _histSearchWidth << endl;
    cerr << "=================================" << endl;
  }
  
  _clearHist();

  int nBeamsHist = 0;
  for (size_t ii = 0; ii < rays.size(); ii++) {
    RadxRay *ray = rays[ii];
    double angle = ray->getElevationDeg();
    if (_isRhi) {
      angle = ray->getAzimuthDeg();
    }
    if (_params.use_target_angles_for_hist) {
      angle = ray->getFixedAngleDeg();
    }
    int angleBin = (int) (angle / _histIntv + _histOffset + 0.5);
    if (angleBin < 0) {
      angleBin = 0;
    }
    if (angleBin > _nHist - 1) {
      angleBin = _nHist - 1;
    }
    _hist[angleBin]++;
    nBeamsHist++;
  } // ii

  return nBeamsHist;

}

/////////////////////////////////////////////////////////////////
// Print angle and fields

void SweepMgr::_printAngles()

{

  cerr << endl;

  bool histInUse = false;
  for (int ii = 0; ii < _nHist; ii++) {
    if (_hist[ii] > 0) {
      histInUse = true;
      break;
    }
  }

  if (histInUse > 0) {
    cerr << "Angle histogram" << endl;
    cerr << "===============" << endl;
    for (int ii = 0; ii < _nHist; ii++) {
      if (_hist[ii] > 0) {
        cerr << "  Angle: " << (ii - _histOffset) * _histIntv
             << ", count: " << _hist[ii] << endl;
      }
    } // ii
    cerr << endl;
  }

  if (_angleTable.size() > 0) {
    cerr << "Sweep table" << endl;
    cerr << "===========" << endl;
    for (size_t ii = 0; ii < _angleTable.size(); ii++) {
      cerr << "  Angle #: " << ii << ", angle: " << _angleTable[ii];
      if (ii < _sweepTable.size()) {
        cerr << ", Sweep: " << _sweepTable[ii];
      }
      cerr << endl;
    } // ii
    cerr << endl;
  }

}
  
/////////////////////////////////////////////////////////////////
// Clear histogram

void SweepMgr::_clearHist()
{
  memset(_hist, 0, _nHist * sizeof(int));
}

/////////////////////////////////////////////////////////////////
// Free histogram array

void SweepMgr::_freeHist()
{
  if (_hist != NULL) {
    delete[] _hist;
  }
  _hist = NULL;
}


