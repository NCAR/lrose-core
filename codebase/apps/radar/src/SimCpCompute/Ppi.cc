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
///////////////////////////////////////////////////////////////
// Ppi.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2006
//
///////////////////////////////////////////////////////////////

#include "Ppi.hh"
#include <toolsa/DateTime.hh>
#include <toolsa/file_io.h>
#include <toolsa/toolsa_macros.h>
#include <iomanip>
#include <cerrno>
#include <cmath>
using namespace std;

// Constructor

Ppi::Ppi(const Params &params) :
        _params(params)
  
{

  // compute number of azimuths etc

  _startAz = _params.start_az;
  _endAz = _params.end_az;
  
  if (_startAz > _endAz) {
    _startAz -= 360.0;
  }

  _deltaAz = _params.delta_az;
  _nAz = (int) ((_endAz - _startAz) / _deltaAz + 0.5);

  // create an array of beam pointers

  _beams = new Beam*[_nAz];
  memset(_beams, 0, _nAz * sizeof(Beam *));

  _startFound = false;
  _propsComputed = false;
  _diffsComputed = false;

  _xmitMode = UNKNOWN;

}

// destructor

Ppi::~Ppi()

{

  clearData();
  if (_beams) {
    delete[] _beams;
  }

}

// clear data

void Ppi::clearData()

{
  _startFound = false;
  if (_beams) {
    for (int ii = 0; ii < _nAz; ii++) {
      if (_beams[ii]) {
        delete _beams[ii];
        _beams[ii] = NULL;
      }
    }
  }
}

// add a beam

void Ppi::addBeam(Beam *beam)

{

  // compute the azimuth index

  int azIndex = _computeAzIndex(beam->getAz());

  if (azIndex < 0 || azIndex > _nAz - 1) {
    delete beam;
    return;
  }

  if (!_startFound) {
    if (_params.clockwise_rotation) {
      if (azIndex != 0) {
	delete beam;
	return;
      }
    } else {
      if (azIndex != _nAz - 1) {
	delete beam;
	return;
      }
    }
    _startFound = true;
    if (_params.debug) {
      cerr << "PPI started, az: " << beam->getAz() << endl;
    }
  }

  _beams[azIndex] = beam;

}
  
// does an azimuth already exist

bool Ppi::azExists(double az)

{
  
  int azIndex = _computeAzIndex(az);

  if (azIndex < 0 || azIndex > _nAz - 1) {
    return false;
  }

  if (_beams[azIndex] != NULL) {
    return true;
  }

  return false;

}
  
// is the ppi complete - i.e. all azimuths exist?

bool Ppi::complete()

{

  return true;

  for (int ii = 0; ii < _nAz; ii++) {
    if (_beams[ii] == NULL) {
      return false;
    }
  }

  return true;

}

// compute properties of this ppi

void Ppi::computeProps()

{

  // mid time

  double sumTime = 0.0;
  double sumEl = 0.0;
  double nn = 0.0;
  for (int ii = 0; ii < _nAz; ii++) {
    if (_beams[ii]) {
      sumTime += _beams[ii]->getDoubleTime();
      sumEl += _beams[ii]->getEl();
      nn++;
    }
  }
  if (nn == 0) {
    _meanTime = 0;
    _meanEl = -9999;
  } else {
    _meanTime = sumTime / nn;
    _meanEl = sumEl / nn;
  }

  // mean power

  double sum_hc = 0.0;
  double sum_vc = 0.0;
  double sum_hx = 0.0;
  double sum_vx = 0.0;
  nn = 0.0;
  
  for (int ii = 0; ii < _nAz; ii++) {

    Beam *beam = _beams[ii];
    if (beam == NULL) {
      continue;
    }

    const vector<MomentData> &moments = beam->getMoments();
    for (int jj = 0; jj < (int) moments.size(); jj++) {
      const MomentData &mom = moments[jj];
      sum_hc += mom.phc;
      sum_hx += mom.phx;
      sum_vc += mom.pvc;
      sum_vx += mom.pvx;
      nn++;
    } // jj

  } // ii

  if (nn == 0) {
    _meanHc = -9999;
    _meanHx = -9999;
    _meanVc = -9999;
    _meanVx = -9999;
  } else {
    _meanHc = 10.0 * log10(sum_hc / nn);
    _meanHx = 10.0 * log10(sum_hx / nn);
    _meanVc = 10.0 * log10(sum_vc / nn);
    _meanVx = 10.0 * log10(sum_vx / nn);
  }

  // power differences

  double sum_hc_minus_hx = 0.0;
  double sum_hc_minus_vc = 0.0;
  double sum_hc_minus_vx = 0.0;
  double sum_vc_minus_hx = 0.0;
  double sum_vc_minus_vx = 0.0;
  double sum_hx_minus_vx = 0.0;

  _validCount = 0.0;

  _minHcMinusHx = 9999;
  _minHcMinusVc = 9999;
  _minHcMinusVx = 9999;
  _minVcMinusHx = 9999;
  _minVcMinusVx = 9999;
  _minHxMinusVx = 9999;

  _maxHcMinusHx = -9999;
  _maxHcMinusVc = -9999;
  _maxHcMinusVx = -9999;
  _maxVcMinusHx = -9999;
  _maxVcMinusVx = -9999;
  _maxHxMinusVx = -9999;

  for (int ii = 0; ii < _nAz; ii++) {

    Beam *beam = _beams[ii];
    if (beam == NULL) {
      continue;
    }

    const vector<MomentData> &moments = beam->getMoments();
    for (int jj = 0; jj < (int) moments.size(); jj++) {
      const MomentData &mom = moments[jj];
      if (!mom.valid) {
        continue;
      }
      
      double hc_minus_hx = mom.dbmhc - mom.dbmhx;
      double hc_minus_vc = mom.dbmhc - mom.dbmvc;
      double hc_minus_vx = mom.dbmhc - mom.dbmvx;
      double vc_minus_hx = mom.dbmvc - mom.dbmhx;
      double vc_minus_vx = mom.dbmvc - mom.dbmvx;
      double hx_minus_vx = mom.dbmhx - mom.dbmvx;

      sum_hc_minus_hx += hc_minus_hx;
      sum_hc_minus_vc += hc_minus_vc;
      sum_hc_minus_vx += hc_minus_vx;
      sum_vc_minus_hx += vc_minus_hx;
      sum_vc_minus_vx += vc_minus_vx;
      sum_hx_minus_vx += hx_minus_vx;

      _minHcMinusHx = MIN(_minHcMinusHx, hc_minus_hx);
      _minHcMinusVc = MIN(_minHcMinusVc, hc_minus_vc);
      _minHcMinusVx = MIN(_minHcMinusVx, hc_minus_vx);
      _minVcMinusHx = MIN(_minVcMinusHx, vc_minus_hx);
      _minVcMinusVx = MIN(_minVcMinusVx, vc_minus_vx);
      _minHxMinusVx = MIN(_minHxMinusVx, hx_minus_vx);
      
      _maxHcMinusHx = MAX(_maxHcMinusHx, hc_minus_hx);
      _maxHcMinusVc = MAX(_maxHcMinusVc, hc_minus_vc);
      _maxHcMinusVx = MAX(_maxHcMinusVx, hc_minus_vx);
      _maxVcMinusHx = MAX(_maxVcMinusHx, vc_minus_hx);
      _maxVcMinusVx = MAX(_maxVcMinusVx, vc_minus_vx);
      _maxHxMinusVx = MAX(_maxHxMinusVx, hx_minus_vx);

      _validCount++;

    } // jj

  } // ii

  if (_validCount == 0) {
    _meanHcMinusHx = -9999;
    _meanHcMinusVc = -9999;
    _meanHcMinusVx = -9999;
    _meanVcMinusHx = -9999;
    _meanVcMinusVx = -9999;
    _meanHxMinusVx = -9999;
  } else {
    _meanHcMinusHx = sum_hc_minus_hx / _validCount;
    _meanHcMinusVc = sum_hc_minus_vc / _validCount;
    _meanHcMinusVx = sum_hc_minus_vx / _validCount;
    _meanVcMinusHx = sum_vc_minus_hx / _validCount;
    _meanVcMinusVx = sum_vc_minus_vx / _validCount;
    _meanHxMinusVx = sum_hx_minus_vx / _validCount;
    if (_meanHcMinusVc > _params.hc_minus_vc_threshold) {
      _xmitMode = HORIZ;
    } else if (_meanHcMinusVc < _params.hc_minus_vc_threshold) {
      _xmitMode = VERT;
    } else {
      _xmitMode = BOTH;
    }
  }

  _propsComputed = true;

}

/////////////////////////////////////////////////////
// compute cross-polar power diffs from previous ppi
//
// Returns 0 on success, -1 on failure.
// On success, meanVxMinusHx is set.

int Ppi::computeCpDiff(const Ppi &prev,
                       double &meanVcMinusHc,
                       double &meanVxMinusHx)

{

  FILE *out = NULL;

  if (_params.write_output_files) {

    // open output file
    
    if (ta_makedir_recurse(_params.output_dir)) {
      int errNum = errno;
      cerr << "ERROR - Ppi::computeCpDiff";
      cerr << "  Cannot create output dir: " << _params.output_dir << endl;
      cerr << "  " << strerror(errNum) << endl;
    }
    
    time_t ppiTime = (time_t) _meanTime;
    DateTime ptime(ppiTime);
    char outPath[1024];
    sprintf(outPath, "%s/ppi_cp_%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
            _params.output_dir,
            ptime.getYear(),
            ptime.getMonth(),
            ptime.getDay(),
            ptime.getHour(),
            ptime.getMin(),
            ptime.getSec());
    
    if (_params.debug) {
      cerr << "-->> Writing ppi cp data to file: " << outPath << endl;
    }
    
    // open file
    
    if ((out = fopen(outPath, "w")) == NULL) {
      int errNum = errno;
      cerr << "ERROR - Ppi::computeCpDiff";
      cerr << "  Cannot create file: " << outPath << endl;
      cerr << "  " << strerror(errNum) << endl;
    }

  }

  // power differences

  double sum_hc_minus_prev_vc = 0.0;
  double sum_vc_minus_prev_hc = 0.0;

  double sum_prev_hc_minus_vc = 0.0;
  double sum_prev_vc_minus_hc = 0.0;

  double sum_hx_minus_prev_vx = 0.0;
  double sum_vx_minus_prev_hx = 0.0;

  double sum_prev_hx_minus_vx = 0.0;
  double sum_prev_vx_minus_hx = 0.0;

  _bothValidCount = 0.0;

  for (int ii = 0; ii < _nAz; ii++) {
    
    Beam *thisBeam = _beams[ii];
    if (thisBeam == NULL) {
      continue;
    }

    Beam *prevBeam = prev._beams[ii];
    if (prevBeam == NULL) {
      continue;
    }
    
    const vector<MomentData> &thisMoments = thisBeam->getMoments();
    const vector<MomentData> &prevMoments = prevBeam->getMoments();

    int nGates = (int) thisMoments.size();
    if (nGates > (int) prevMoments.size()) {
      nGates = (int) prevMoments.size();
    }
    for (int jj = 0; jj < nGates; jj++) {

      const MomentData &thisMom = thisMoments[jj];
      const MomentData &prevMom = prevMoments[jj];
      if (!thisMom.valid || !prevMom.valid) {
        continue;
      }

      double hc_minus_hx = thisMom.dbmhc - thisMom.dbmhx;
      double hc_minus_vc = thisMom.dbmhc - thisMom.dbmvc;
      double hc_minus_vx = thisMom.dbmhc - thisMom.dbmvx;
      double vc_minus_hx = thisMom.dbmvc - thisMom.dbmhx;
      double vc_minus_vx = thisMom.dbmvc - thisMom.dbmvx;
      double hx_minus_vx = thisMom.dbmhx - thisMom.dbmvx;

      double hc_minus_prev_vc = thisMom.dbmhc - prevMom.dbmvc;
      double vc_minus_prev_hc = thisMom.dbmvc - prevMom.dbmhc;

      double prev_hc_minus_vc = prevMom.dbmhc - thisMom.dbmvc;
      double prev_vc_minus_hc = prevMom.dbmvc - thisMom.dbmhc;

      double hx_minus_prev_vx = thisMom.dbmhx - prevMom.dbmvx;
      double vx_minus_prev_hx = thisMom.dbmvx - prevMom.dbmhx;

      double prev_hx_minus_vx = prevMom.dbmhx - thisMom.dbmvx;
      double prev_vx_minus_hx = prevMom.dbmvx - thisMom.dbmhx;

      sum_hc_minus_prev_vc += hc_minus_prev_vc;
      sum_vc_minus_prev_hc += vc_minus_prev_hc;
      
      sum_prev_hc_minus_vc += prev_hc_minus_vc;
      sum_prev_vc_minus_hc += prev_vc_minus_hc;
      
      sum_hx_minus_prev_vx += hx_minus_prev_vx;
      sum_vx_minus_prev_hx += vx_minus_prev_hx;
      
      sum_prev_hx_minus_vx += prev_hx_minus_vx;
      sum_prev_vx_minus_hx += prev_vx_minus_hx;
      
      _bothValidCount++;

      // write to file

      if (out) {
        fprintf(out,
                "%8.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f "
                "%8.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f "
                "%8.3f %8.3f %8.3f %8.3f %8.3f %8.3f\n",
                hc_minus_hx,
                hc_minus_vc,
                hc_minus_vx,
                vc_minus_hx,
                vc_minus_vx,
                hx_minus_vx,
                hc_minus_prev_vc,
                vc_minus_prev_hc,
                prev_hc_minus_vc,
                prev_vc_minus_hc,
                hx_minus_prev_vx,
                vx_minus_prev_hx,
                prev_hx_minus_vx,
                prev_vx_minus_hx,
                thisMom.dbmhc,
                thisMom.dbmhx,
                thisMom.dbmvc,
                thisMom.dbmvx,
                prevMom.dbmhc,
                prevMom.dbmhx,
                prevMom.dbmvc,
                prevMom.dbmvx);
      }
                
    } // jj

  } // ii

  if (out) {
    fclose(out);
  }

  if (_bothValidCount == 0) {
    return -1;
  }

  _meanHcMinusPrevVc = sum_hc_minus_prev_vc / _bothValidCount;
  _meanVcMinusPrevHc = sum_vc_minus_prev_hc / _bothValidCount;

  _meanPrevHcMinusVc = sum_prev_hc_minus_vc / _bothValidCount;
  _meanPrevVcMinusHc = sum_prev_vc_minus_hc / _bothValidCount;

  _meanHxMinusPrevVx = sum_hx_minus_prev_vx / _bothValidCount;
  _meanVxMinusPrevHx = sum_vx_minus_prev_hx / _bothValidCount;

  _meanPrevHxMinusVx = sum_prev_hx_minus_vx / _bothValidCount;
  _meanPrevVxMinusHx = sum_prev_vx_minus_hx / _bothValidCount;

  _diffsComputed = true;

  double timeDiff = fabs(_meanTime - prev._meanTime);
  if (timeDiff > _params.max_time_between_ppis) {
    return -1;
  }

  if (_xmitMode == HORIZ && prev._xmitMode == VERT) {
    meanVcMinusHc = _meanPrevVcMinusHc;
    meanVxMinusHx = _meanVxMinusPrevHx;
    return 0;
  } else if (_xmitMode == VERT && prev._xmitMode == HORIZ) {
    meanVcMinusHc = _meanVcMinusPrevHc;
    meanVxMinusHx = _meanPrevVxMinusHx;
    return 0;
  }

  return -1;
  
}

// print

void Ppi::print(ostream &out)

{

  out << endl;
  out << "============ Ppi ==============" << endl;

  out << "  nAz        : " << _nAz << endl;
  out << "  startAz    : " << _startAz << endl;
  out << "  endAz      : " << _endAz << endl;
  out << "  deltaAz    : " << _deltaAz << endl;

  if (!_propsComputed) {
    out << "Props not computed" << endl;
    out << "===============================" << endl;
    out << endl;
    return;
  }

  out << "  meanEl     : " << _meanEl << endl;

  time_t ttime = (time_t) _meanTime;
  int psecs = (int) ((_meanTime - ttime) * 1000.0);
  char psecsStr[32];
  sprintf(psecsStr, "%.3d", psecs);
  out << "  mean time  : " << DateTime::strm(ttime) << "." << psecsStr << endl;
  
  out << "-------------------------------" << endl;

  switch (_xmitMode) {
    case HORIZ:
      out << "  Xmit mode: HORIZ" << endl;
      break;
    case VERT:
      out << "  Xmit mode: VERT" << endl;
      break;
    case BOTH:
      out << "  Xmit mode: BOTH" << endl;
      break;
    case UNKNOWN:
      out << "  Xmit mode: UNKNWON" << endl;
      break;
  }

  out << "  meanHc     : " << _meanHc << endl;
  out << "  meanHx     : " << _meanHx << endl;
  out << "  meanVc     : " << _meanVc << endl;
  out << "  meanVx     : " << _meanVx << endl;

  out << "-------------------------------" << endl;

  out << "  n valid: " << _validCount << endl;
  out << endl;
  out << "  meanHcMinusHx: " << _meanHcMinusHx << endl;
  out << "  meanHcMinusVc: " << _meanHcMinusVc << endl;
  out << "  meanHcMinusVx: " << _meanHcMinusVx << endl;
  out << "  meanVcMinusHx: " << _meanVcMinusHx << endl;
  out << "  meanVcMinusVx: " << _meanVcMinusVx << endl;
  out << "  meanHxMinusVx: " << _meanHxMinusVx << endl;
  out << endl;
  out << "  minHcMinusHx: " << _minHcMinusHx << endl;
  out << "  minHcMinusVc: " << _minHcMinusVc << endl;
  out << "  minHcMinusVx: " << _minHcMinusVx << endl;
  out << "  minVcMinusHx: " << _minVcMinusHx << endl;
  out << "  minVcMinusVx: " << _minVcMinusVx << endl;
  out << "  minHxMinusVx: " << _minHxMinusVx << endl;
  out << endl;
  out << "  maxHcMinusHx: " << _maxHcMinusHx << endl;
  out << "  maxHcMinusVc: " << _maxHcMinusVc << endl;
  out << "  maxHcMinusVx: " << _maxHcMinusVx << endl;
  out << "  maxVcMinusHx: " << _maxVcMinusHx << endl;
  out << "  maxVcMinusVx: " << _maxVcMinusVx << endl;
  out << "  maxHxMinusVx: " << _maxHxMinusVx << endl;
  out << endl;

  if (!_diffsComputed) {
    out << "Diffs not computed" << endl;
    out << "===============================" << endl;
    out << endl;
    return;
  }

  out << "-------------------------------" << endl;

  out << "  n valid for both: " << _bothValidCount << endl;
  out << endl;
  out << "  meanHcMinusPrevVc: " << _meanHcMinusPrevVc << endl;
  out << "  meanVcMinusPrevHc: " << _meanVcMinusPrevHc << endl;
  out << "  meanPrevHcMinusVc: " << _meanPrevHcMinusVc << endl;
  out << "  meanPrevVcMinusHc: " << _meanPrevVcMinusHc << endl;
  out << "  meanHxMinusPrevVx: " << _meanHxMinusPrevVx << endl;
  out << "  meanVxMinusPrevHx: " << _meanVxMinusPrevHx << endl;
  out << "  meanPrevHxMinusVx: " << _meanPrevHxMinusVx << endl;
  out << "  meanPrevVxMinusHx: " << _meanPrevVxMinusHx << endl;
  out << endl;

  out << "===============================" << endl;
  out << endl;

}

// compute the azimuth index

int Ppi::_computeAzIndex(double az)

{

  // get into the right sector

  if (_startAz < 0 && az > _endAz) {
    az -= 360.0;
  }
  
  if (az < _startAz || az > _endAz) {
    return -1;
  }

  int azIndex = (int) ((az - _startAz) / _deltaAz);

  return azIndex;

}

