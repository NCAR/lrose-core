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
// AlternatingVelocity.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////
//
// Compute secondary velocity estimate for alternating mode radars,
// using (a) the alternating mode velocity and (b) the velocity estimated
// from H and V pulses separately. (b) is less noisy than (a),
// but folds at half the nyquist. So we use (a) to unfold (b) to 
// produce a less noisy field which folds at the full nyquist.
// 
// We call this derived product vel2.
//
///////////////////////////////////////////////////////////////

#include <radar/AlternatingVelocity.hh>
#include <radar/IwrfCalib.hh>
using namespace std;

// Constructor

AlternatingVelocity::AlternatingVelocity()
  
{
  _loadTestFields = false;  
}

// destructor

AlternatingVelocity::~AlternatingVelocity()

{

}

//////////////////////////////////////////////////
// compute vel2 for alternating mode
// using vel_hv as the basis
//
// Assumes that the following fields have been computed:
//   vel - alternating mode velocity
//   vel_hv - velocity from H and V time series
//   noise_flag - presence of noise

void AlternatingVelocity::computeVelAlt(int nGates,
                                        MomentsFields *mfields,
                                        double nyquist)

{
  
  _nGates = nGates;
  _mfields = mfields;

  _nyquist = nyquist;
  _halfNyquist = nyquist / 2.0;
  _twiceNyquist = nyquist * 2.0;

  // initialize computational fields

  _compFields.resize(nGates);
  for (int igate = 0; igate < _nGates; igate++) {
    CompFields &cfield = _compFields[igate];
    cfield.foldInterval = -9999;
    cfield.unfoldInterval = -9999;
    cfield.velDiff = -9999;
    cfield.foldConfidence = -9999;
    cfield.meanConfidence = -9999;
    cfield.minVelRun = -9999;
    cfield.maxVelRun = -9999;
    cfield.unfoldedRun = -9999;
    cfield.velAlt = -9999;
    cfield.velZero = false;
    cfield.velNyquist = false;
    cfield.fracZero = 0;
    cfield.fracNyquist = 0;
    cfield.fixFlag = false;
  }

  // fix the alternating mode velocity in clutter

  _fixAltModeVel();

  // compute the difference between velocity from alternating mode
  // and velocity from H and V separately, and deduce the folding interval
  
  for (int igate = 0; igate < _nGates; igate++) {
    const MomentsFields &mfields = _mfields[igate];
    CompFields &cfield = _compFields[igate];
    double velDiff = mfields.vel - mfields.vel_hv;
    double cpa = mfields.cpa;
    cfield.velDiff = velDiff;
    double unfold = velDiff / _nyquist;
    int ifold = (int) floor(unfold + 0.5);
    double error = unfold - ifold;
    double confidence = 1.0 - (fabs(error) * 2.0);
    if (confidence < 0 || mfields.noise_flag) {
      confidence = 0;
    }
    cfield.foldInterval = ifold;
    cfield.foldConfidence = confidence;
    if (cpa > 0.5) {
      cfield.unfoldInterval = 0;
    } else if (confidence > 0.8) {
      cfield.unfoldInterval = ifold;
    } else {
      cfield.unfoldInterval = -9999;
    }
    if (mfields.noise_flag) {
      cfield.unfoldInterval = -9999;
    }
  }

  // find the gaps which need filling
  
  _findGapRuns();

  // compute the velocity interval for each gap
  // based on the predominant folding interval

  for (size_t igap = 0; igap < _gaps.size(); igap++) {
    _computeFoldInterval(_gaps[igap]);
  }

  // set velocity for gates with known interval

  for (int igate = 0; igate < _nGates; igate++) {
    const MomentsFields &mfields = _mfields[igate];
    CompFields &cfield = _compFields[igate];
    if (cfield.unfoldInterval > -9998) {
      double velAlt = mfields.vel_hv + cfield.foldInterval * _nyquist;
      while (velAlt > _nyquist) velAlt -= _twiceNyquist;
      while (velAlt < -_nyquist) velAlt += _twiceNyquist;
      cfield.velAlt = velAlt;
    }
  }

  // set velocity for gates in gaps
  
  for (size_t igap = 0; igap < _gaps.size(); igap++) {
    const GateRun &gap = _gaps[igap];
    // if (gap.meanConfidence < 0.6) continue;
    int startGate = gap.start;
    int endGate = gap.end;
    for (int igate = startGate; igate < endGate; igate++) {
      const MomentsFields &mfields = _mfields[igate];
      CompFields &cfield = _compFields[igate];
      double velAlt = mfields.vel_hv;
      // constrain between minVel and maxVel
      while (velAlt > gap.maxVel) velAlt -= _nyquist;
      while (velAlt < gap.minVel) velAlt += _nyquist;
      // constrain between -nyquist and nyquist
      while (velAlt > _nyquist) velAlt -= _twiceNyquist;
      while (velAlt < -_nyquist) velAlt += _twiceNyquist;
      cfield.velAlt = velAlt;
    }
  }

  // clean up remaining folds if possible

  for (size_t igap = 0; igap < _gaps.size(); igap++) {
    GateRun &gap = _gaps[igap];
    _correctBadFold(gap);
  }

  // for gates with only noise, set velAlt to original vel
  // copy computed values over to mfields array

  for (int igate = 0; igate < _nGates; igate++) {
    const MomentsFields &mfields = _mfields[igate];
    CompFields &cfield = _compFields[igate];
    if (mfields.noise_flag) {
      cfield.velAlt = mfields.vel;
    }
  }

  // save vel alt as the main vel field
  // and save the original vel as vel_alt
  
  for (int igate = 0; igate < _nGates; igate++) {
    MomentsFields &mfields = _mfields[igate];
    CompFields &cfield = _compFields[igate];
    mfields.vel_alt = mfields.vel;
    mfields.vel = cfield.velAlt;
    mfields.vel_alt_fold_interval = cfield.foldInterval;
    mfields.vel_alt_fold_confidence = cfield.foldConfidence;
    mfields.vel_diff = cfield.velDiff;
    mfields.vel_unfold_interval = cfield.unfoldInterval;
    if (_loadTestFields) {
      mfields.test2 = cfield.meanConfidence;
      mfields.test3 = cfield.minVelRun;
      mfields.test4 = cfield.maxVelRun;
      mfields.test5 = cfield.unfoldedRun;
    }
  } // igate

}

///////////////////////////////////////////////////////
// fix the alternating mode velocity, which oscillates
// between 0 and the nyquist in clutter

void AlternatingVelocity::_fixAltModeVel()

{
  
  // find gates with vel close to 0 or the nyquist

  for (int igate = 0; igate < _nGates; igate++) {
    CompFields &cfield = _compFields[igate];
    double vel = _mfields[igate].vel;
    double nyqFrac = fabs(vel / _nyquist);
    if (nyqFrac < 0.05) {
      cfield.velZero = true;
    } else {
     cfield.velZero = false;
    }
    if (nyqFrac > 0.95) {
      cfield.velNyquist = true;
    } else {
      cfield.velNyquist = false;
    }
  }

  // set kernel size

  int kernelSize = 11;
  int kernelHalf = kernelSize / 2;
  if ((int) _nGates < kernelSize) {
    return;
  }

  // set up gate limits
  
  for (int igate = 0; igate < _nGates; igate++) {
    CompFields &cfield = _compFields[igate];
    int istart = igate - kernelHalf;
    int iend = igate + kernelHalf;
    if (istart < 0) {
      int iadj = 0 - istart;
      istart += iadj;
      iend += iadj;
    } else if (iend > (int) _nGates - 1) {
      int iadj = iend - (_nGates - 1);
      iend -= iadj;
      istart -= iadj;
    }
    cfield.startGate = istart;
    cfield.endGate = iend;
  }

  // compute fraction of zero and nyquist gates in each kernel
  
  for (int igate = 0; igate < _nGates; igate++) {
    
    CompFields &cfield = _compFields[igate];
    double sumZero = 0.0;
    double sumNyquist = 0.0;
    
    for (int jgate = cfield.startGate; jgate <= cfield.endGate; jgate++) {
      if (_compFields[jgate].velZero) sumZero++;
      if (_compFields[jgate].velNyquist) sumNyquist++;
    }
    
    cfield.fracZero = sumZero / kernelSize;
    cfield.fracNyquist = sumNyquist / kernelSize;
    
    if (cfield.fracZero > 0.075 && cfield.fracNyquist > 0.075) {
      cfield.fixFlag = true;
    } else {
      cfield.fixFlag = false;
    }
    
  } // igate

  for (int igate = 0; igate < _nGates; igate++) {
    CompFields &cfield = _compFields[igate];
    if (cfield.fixFlag && cfield.velNyquist) {
      if (_mfields[igate].vel > 0) {
        _mfields[igate].vel -= _nyquist;
      } else {
        _mfields[igate].vel += _nyquist;
      }
    }
  }

}

/////////////////////////////////////////////////////
// find the gaps which need filling

void AlternatingVelocity::_findGapRuns()
{
  
  _gaps.clear();
  bool inGap = false;
  int gateStart = 0;
  for (int igate = 0; igate < _nGates; igate++) {
    int foldInt = _compFields[igate].unfoldInterval;
    bool noiseFlag = _mfields[igate].noise_flag;
    if (!inGap) {
      if (foldInt < -9998 && !noiseFlag) {
        // start of gap
        gateStart = igate;
        inGap = true;
      }
    } else {
      if (foldInt > -9998 || noiseFlag) {
        // end of gap
        int gateEnd = igate;
        GateRun gap(gateStart, gateEnd);
        _gaps.push_back(gap);
        inGap = false;
      }
    }
  }
  if (inGap) { // end condition
    size_t gateEnd = _nGates;
    GateRun gap(gateStart, gateEnd);
    gap.mid = gateEnd; // fill all from the front
    _gaps.push_back(gap);
  }

}

/////////////////////////////////////////////////////
// compute velocity interval for a run
// this is done by computing the predominant folding
// values for the interval

void AlternatingVelocity::_computeFoldInterval(GateRun &run)
{

  // include the gate on either side of the run

  int startGate = run.start - 1;
  if (startGate < 0) {
    startGate = 0;
  }
  int endGate = run.end;
  if (endGate > _nGates - 1) {
    endGate = _nGates - 1;
  }

  // add up the number of gates for each folding interval

  double sumMid = 0.0;
  double sumUpper = 0.0;
  double sumLower = 0.0;
  double sumConfidence = 0.0;
  double nConfidence = 0.0;
  
  for (int igate = startGate; igate <= endGate; igate++){
    const MomentsFields &mfields = _mfields[igate];
    CompFields &cfield = _compFields[igate];
    double ifold = cfield.foldInterval;
    double confidence = cfield.foldConfidence;
    bool noiseFlag = mfields.noise_flag;
    if (!noiseFlag) {
      sumConfidence += confidence;
      nConfidence++;
      if (ifold > -0.5 && ifold < 0.5) {
        sumMid += confidence;
      } else if (ifold > 0.5) {
        sumUpper += confidence;
      } else {
        sumLower += confidence;
      }
    }
  }

  if (nConfidence > 0) {
    run.meanConfidence = sumConfidence / nConfidence;
  }

  if (sumUpper < 0.1 && sumLower < 0.1) {
    // fold interval is in mid section
    double mean = 0.0;
    run.minVel = mean - _halfNyquist;
    run.maxVel = mean + _halfNyquist;
  } else if (sumLower < sumMid && sumLower < sumUpper) {
    // fold interval lies from mid to upper
    double sumTotal = sumUpper + sumMid;
    double interpPt = ((sumUpper / sumTotal) * 0.75) * _nyquist;
    run.minVel = interpPt - _halfNyquist;
    run.maxVel = interpPt + _halfNyquist;
  } else if (sumUpper < sumMid && sumUpper < sumLower ) {
    // fold interval lies from lower to mid
    double sumTotal = sumLower + sumMid;
    double interpPt = ((sumLower / sumTotal) * -0.75) * _nyquist;
    run.minVel = interpPt - _halfNyquist;
    run.maxVel = interpPt + _halfNyquist;
  } else {
    // interval folds from upper back down to lower
    double sumTotal = sumLower + sumUpper;
    double interpPt = (0.75 + (sumLower / sumTotal) * 0.5) * _nyquist;
    run.minVel = interpPt - _halfNyquist;
    run.maxVel = interpPt + _halfNyquist;
  }
  
  for (int igate = startGate + 1; igate < endGate; igate++){
    CompFields &cfield = _compFields[igate];
    cfield.minVelRun = run.minVel;
    cfield.maxVelRun = run.maxVel;
    cfield.meanConfidence = run.meanConfidence;
  }

}

/////////////////////////////////////////////////////
// Check for bad fold in a run, fix if appropriate

void AlternatingVelocity::_correctBadFold(GateRun &run)
{
  
  // we only look for the 'intermediate' folds, i.e. those
  // caused by computing velocity from every second pulse,
  // which in turn leads to half the nyquist
  //
  // so we only consider velocity around half the nyquist

  double velLowerLimit = _nyquist * 0.25;
  double velUpperLimit = _nyquist * 0.75;
  
  // check velocity in run to assess if it might contain fold
  
  int nNegVelInRun = 0;
  int nPosVelInRun = 0;

  for (int igate = run.start; igate < run.end; igate++){
    double velAlt = _compFields[igate].velAlt;
    double absVelAlt = fabs(velAlt);
    if(absVelAlt < velLowerLimit || absVelAlt > velUpperLimit) {
      continue;
    }
    if (velAlt < 0) {
      nNegVelInRun++;
    } else {
      nPosVelInRun++;
    }
  }
  if (nNegVelInRun == 0 && nPosVelInRun == 0) {
    return;
  }

  // check vel in 10 gates before start of run

  int nNegVelBefore = 0;
  int nPosVelBefore = 0;

  int startGate = run.start - 10;
  if (startGate < 0) startGate = 0;
  
  for (int igate = startGate; igate < run.start; igate++){
    CompFields &cfield = _compFields[igate];
    double velAlt = cfield.velAlt;
    double absVelAlt = fabs(velAlt);
    if(absVelAlt < velLowerLimit || absVelAlt > velUpperLimit) {
      continue;
    }
    if (cfield.foldConfidence < 0.8) continue;
    if (velAlt < 0) {
      nNegVelBefore++;
    } else {
      nPosVelBefore++;
    }
  }

  // check vel in 10 gates after start of run

  int nNegVelAfter = 0;
  int nPosVelAfter = 0;

  int endGate = run.end + 10;
  if (endGate > _nGates) endGate = _nGates;
  
  for (int igate = run.start; igate < endGate; igate++){
    CompFields &cfield = _compFields[igate];
    double velAlt = cfield.velAlt;
    double absVelAlt = fabs(velAlt);
    if(absVelAlt < velLowerLimit || absVelAlt > velUpperLimit) {
      continue;
    }
    if (cfield.foldConfidence < 0.8) continue;
    if (velAlt < 0) {
      nNegVelAfter++;
    } else {
      nPosVelAfter++;
    }
  }

  // return now if we don't have enough info to do anything

  int nNegOutside = nNegVelBefore + nNegVelAfter;
  int nPosOutside = nPosVelBefore + nPosVelAfter;
  if (nNegOutside == 0 && nPosOutside == 0) {
    return;
  }

  // determine whether we have strong indication of the velocity
  // on either side of the run

  bool outsideIsPos = false;
  bool outsideIsNeg = false;

  if (nPosOutside > 0) {
    if (nPosOutside > (nNegOutside * 3)) {
      outsideIsPos = true;
    }
  } 
  if (nNegOutside > 0) {
    if (nNegOutside > (nPosOutside * 3)) {
      outsideIsNeg = true;
    }
  }
  if (!outsideIsPos && !outsideIsNeg) {
    return;
  }

  // determine whether we have a fold in the run, by checking the
  // velocity inside the run relative to that on either side

  if (outsideIsPos && nNegVelInRun == 0) {
    return;
  }
  if (outsideIsNeg && nPosVelInRun == 0) {
    return;
  }

  // do the correction

  if (outsideIsPos) {

    for (int igate = run.start; igate < run.end; igate++){
      CompFields &cfield = _compFields[igate];
      double velAlt = cfield.velAlt;
      double absVelAlt = fabs(velAlt);
      if(absVelAlt < velLowerLimit || absVelAlt > velUpperLimit) {
        continue;
      }
      if (velAlt < 0) {
        velAlt += _nyquist;
      }
      cfield.velAlt = velAlt;
      cfield.unfoldedRun = velAlt;
    }

  } else {

    for (int igate = run.start; igate < run.end; igate++){
      CompFields &cfield = _compFields[igate];
      double velAlt = cfield.velAlt;
      double absVelAlt = fabs(velAlt);
      if(absVelAlt < velLowerLimit || absVelAlt > velUpperLimit) {
        continue;
      }
      if (velAlt > 0) {
        velAlt -= _nyquist;
      }
      cfield.velAlt = velAlt;
      cfield.unfoldedRun = velAlt;
    }

  }

}

