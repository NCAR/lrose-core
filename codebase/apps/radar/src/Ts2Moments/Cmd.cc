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
// Cmd.cc
//
// Cmd object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2007
//
///////////////////////////////////////////////////////////////
//
// Cmd computes Clutter Mitigation Decision field
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cassert>
#include <toolsa/TaArray.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/sincos.h>
#include <radar/KdpFilt.hh>
#include <radar/RadarMoments.hh>
#include "Cmd.hh"
#include "c_code/cmd_gap_filter.c"
#include "c_code/cmd_speckle_filter.c"
using namespace std;

////////////////////////////////////////////////////
// Constructor

Cmd::Cmd(const string &prog_name,
         const Params &params,
         vector<GateData *> &gate_data) :
        _progName(prog_name),
        _params(params),
        _gateData(gate_data)

{

  _tdbzInterestMap = NULL;
  _spinInterestMap = NULL;
  _cpaInterestMap = NULL;
  _zdrSdevInterestMap = NULL;
  _phidpSdevInterestMap = NULL;

  _startRangeKm = 0.0;
  _gateSpacingKm = 0.0;
  
  _xmitRcvMode = IWRF_XMIT_RCV_MODE_NOT_SET;
  
  _createInterestMaps(_params);
  
  _nSamples = 0;
  _nSamplesRect = 0;

}

//////////////////////////////////////////////////////////////////
// destructor

Cmd::~Cmd()

{

  if (_tdbzInterestMap != NULL) {
    delete _tdbzInterestMap;
  }

  if (_spinInterestMap != NULL) {
    delete _spinInterestMap;
  }

  if (_cpaInterestMap != NULL) {
    delete _cpaInterestMap;
  }

  if (_zdrSdevInterestMap != NULL) {
    delete _zdrSdevInterestMap;
  }

  if (_phidpSdevInterestMap != NULL) {
    delete _phidpSdevInterestMap;
  }
  
}

/////////////////////////////////////////////////////////
// create interest maps
//
// These are static on the class, so are only created once.

int Cmd::_createInterestMaps(const Params &params)

{  

  // TDBZ

  if (_tdbzInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("tdbz",
				    params._tdbz_interest_map,
				    params.tdbz_interest_map_n,
				    pts)) {
      return -1;
    }
    _tdbzInterestMap =
      new InterestMap("DbzTexture", pts, params.tdbz_interest_weight);
  } // if (_tdbzInterestMap ...

  // spin
  
  if (_spinInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("spin",
				    params._spin_interest_map,
				    params.spin_interest_map_n,
				    pts)) {
      return -1;
    }
    _spinInterestMap =
      new InterestMap("Spin", pts, params.spin_interest_weight);
  } // if (_spinInterestMap ...

  // clutter phase alignment
  
  if (_cpaInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("cpa",
				    params._cpa_interest_map,
				    params.cpa_interest_map_n,
				    pts)) {
      return -1;
    }
    _cpaInterestMap =
      new InterestMap("cpa", pts,
		      params.cpa_interest_weight);
  } // if (_cpaInterestMap ...

  // sdev of zdr

  if (_zdrSdevInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("zdr_sdev",
				    params._zdr_sdev_interest_map,
				    params.zdr_sdev_interest_map_n,
				    pts)) {
      return -1;
    }
    _zdrSdevInterestMap =
      new InterestMap("zdr sdev", pts, params.zdr_sdev_interest_weight);
  } // if (_zdrSdevInterestMap ...

  // sdev of phidp

  if (_phidpSdevInterestMap == NULL) {
    vector<InterestMap::ImPoint> pts;
    if (_convertInterestMapToVector("phidp_sdev",
				    params._phidp_sdev_interest_map,
				    params.phidp_sdev_interest_map_n,
				    pts)) {
      return -1;
    }
    _phidpSdevInterestMap =
      new InterestMap("phidp sdev", pts, params.phidp_sdev_interest_weight);
  } // if (_phidpSdevInterestMap ...
  
  return 0;
  
}

/////////////////////////////////////////////////
// compute CMD

void Cmd::compute(int nGates, const RadarMoments *mom,
                  bool useDualPol, bool useRhohvTest)
  
{

  // compute variables required by TDBZ and SPIN

  _prepareForTdbzAndSpin(nGates);
  
  // compute CMD interest fields
  
  _computeTdbz(nGates);
  _computeSpin(nGates);
  _computeMaxTdbzAndSpinInterest(nGates);

  // dual pol fields
  
  if (useDualPol) {
    if (_params.zdr_sdev_interest_weight > 0) {
      _computeZdrSdev(nGates);
    }
    if (_params.phidp_sdev_interest_weight > 0) {
      _computePhidpSdevNew(nGates);
    }
    if (useRhohvTest) {
      _computeRhohvTest(mom, nGates);
    }
  } // if (dualPol ...
  
  // compute cmd clutter field

  double cmdSnrThreshold = _params.cmd_snr_threshold;
  double cmdThreshold = _params.cmd_threshold_for_clutter;
  bool checkForWindfarms = _params.cmd_check_for_windfarm_clutter;
  double minSpecSnrForWindfarms =
    _params.min_spectral_snr_for_windfarm_clutter;
  double offZeroThreshold = _params.cmd_threshold_for_offzero_weather;
  double tdbzWt = _params.tdbz_interest_weight;
  double spinWt = _params.spin_interest_weight;
  double tdbzSpinWt = _params.max_of_tdbz_and_spin_interest_weight;
  double cpaWt = _params.cpa_interest_weight;
  double zdrSdevWt = _params.zdr_sdev_interest_weight;
  double phidpSdevWt = _params.phidp_sdev_interest_weight;
  
  for (int igate = 0; igate < nGates; igate++) {
    
    MomentsFields *flds = _gateData[igate]->flds;
    
    if (flds->dbz == MomentsFields::missingDouble) {
      continue;
    }
    if (flds->snr == MomentsFields::missingDouble ||
        flds->snr < cmdSnrThreshold) {
      continue;
    }

    double sumInterest = 0.0;
    double sumWeights = 0.0;
    
    flds->tdbz_interest = _tdbzInterestMap->getInterest(flds->tdbz);
    if (tdbzWt > 0) {
      sumInterest += (flds->tdbz_interest * tdbzWt);
      sumWeights += tdbzWt;
    }

    flds->spin_interest = _spinInterestMap->getInterest(flds->spin);
    if (spinWt > 0) {
      sumInterest += (flds->spin_interest * spinWt);
      sumWeights += spinWt;
    }

    if (tdbzSpinWt > 0) {
      sumInterest += (flds->max_tdbz_spin * tdbzSpinWt);
      sumWeights += tdbzSpinWt;
    }

    flds->cpa_interest = _cpaInterestMap->getInterest(flds->cpa);
    if (cpaWt > 0) {
      sumInterest += flds->cpa_interest * cpaWt;
      sumWeights += cpaWt;
    }

    flds->zdr_sdev_interest =
      _zdrSdevInterestMap->getInterest(flds->zdr_sdev);
    if (zdrSdevWt > 0) {
      sumInterest += flds->zdr_sdev_interest * zdrSdevWt;
      sumWeights += zdrSdevWt;
    }

    flds->phidp_sdev_interest =
      _phidpSdevInterestMap->getInterest(flds->phidp_sdev);
    if (phidpSdevWt > 0) {
      sumInterest += flds->phidp_sdev_interest * phidpSdevWt;
      sumWeights += phidpSdevWt;
    }

    flds->cmd = sumInterest / sumWeights;
    
    double checkThreshold = cmdThreshold;

    if (_params.cmd_check_for_offzero_weather &&
        flds->ozsnr > _params.min_snr_for_offzero_weather) {
      if (checkThreshold > offZeroThreshold) {
        checkThreshold = offZeroThreshold;
      }
    }
    
    if (checkForWindfarms) {
      if (flds->spectral_snr > minSpecSnrForWindfarms) {
        checkThreshold = 0;
      }
    }
    
    flds->cmd_flag = 0;
    if (flds->cmd >= checkThreshold) {
      flds->cmd_flag = 1;
    }
    
    flds->rhohv_test_flag = 0;
    if (_params.apply_rhohv_test_in_cmd) {
      if (flds->rhohv_test_improv >= _params.rhohv_improv_thresh_for_power ||
          flds->rhohv_test_improv >= _params.rhohv_improv_thresh_for_vel ||
          flds->rhohv_test_improv >= _params.rhohv_improv_thresh_for_phase ||
          flds->rhohv_test_improv >= _params.rhohv_improv_thresh_for_rho) {
        flds->rhohv_test_flag = 1;
      }
    }
    
  } // igate

  // first remove speckle

  if (_params.apply_cmd_speckle_filter) {
    _applySpeckleFilter(nGates);
  }
  
  // then fill in the gaps
  
  if (_params.apply_cmd_gap_filter) {
    _applyGapFilter(nGates);
  }

  // now apply NEXRAD SPIKE filter

  if (_params.apply_nexrad_spike_filter_after_cmd) {
    _applyNexradSpikeFilter(nGates);
  }

}

/////////////////////////////////////////////////
// Prepare for Spin and Tdbz
    
void Cmd::_prepareForTdbzAndSpin(int nGates)
  
{

  double spinThresh = _params.cmd_spin_dbz_threshold;
  double snrThresh = _params.cmd_snr_threshold;

  for (int igate = 1; igate < nGates - 1; igate++) {

    MomentsFields *flds = _gateData[igate]->flds;

    // check SNR
    double snrThis = flds->snr;
    if (snrThis < snrThresh) {
      continue;
    }
    double snrPrev = _gateData[igate-1]->flds->snr;
    double snrNext = _gateData[igate+1]->flds->snr;

    double dbzPrev = _gateData[igate-1]->flds->dbz;
    double dbzThis = flds->dbz;
    double dbzNext = _gateData[igate+1]->flds->dbz;
    
    if (dbzPrev != MomentsFields::missingDouble &&
        dbzThis != MomentsFields::missingDouble &&
        snrPrev > snrThresh && snrThis > snrThresh) {
      double dbzDiff = dbzThis - dbzPrev;
      flds->dbz_diff_sq = dbzDiff * dbzDiff;
    }

    if (dbzPrev != MomentsFields::missingDouble &&
        dbzThis != MomentsFields::missingDouble &&
        dbzNext != MomentsFields::missingDouble &&
        snrPrev > snrThresh && snrThis > snrThresh && snrNext > snrThresh) {
      double prevDiff = dbzThis - dbzPrev;
      double nextDiff = dbzNext - dbzThis;
      if (prevDiff * nextDiff < 0) {
        // sign change
        double spinChange = (fabs(prevDiff) + fabs(nextDiff)) / 2.0;
        if (spinChange > spinThresh) {
          flds->dbz_spin_change = 1;
        } else {
          flds->dbz_spin_change = 0;
        }
      }
    }
    
  } // ii
  
  // set first and last gates to nearest neighbor

  if (nGates > 2) {
    _gateData[0]->flds->dbz_diff_sq = _gateData[1]->flds->dbz_diff_sq;
    _gateData[0]->flds->dbz_spin_change = _gateData[0]->flds->dbz_spin_change;
    _gateData[nGates-1]->flds->dbz_diff_sq = _gateData[nGates-2]->flds->dbz_diff_sq;
    _gateData[nGates-1]->flds->dbz_spin_change = _gateData[nGates-2]->flds->dbz_spin_change;
  }
 
}

/////////////////////////////////////////////////
// compute TDBZ

void Cmd::_computeTdbz(int nGates)
  
{
  
  // compute number of gates in kernel, making sure there is an odd number
  
  int nGatesKernel = _params.cmd_kernel_ngates_tdbz;
  int nGatesHalf = nGatesKernel / 2;

  // set up gate limits
  
  vector<int> startGate;
  vector<int> endGate;
  for (int igate = 0; igate < nGates; igate++) {
    int start = igate - nGatesHalf;
    if (start < 0) {
      start = 0;
    }
    startGate.push_back(start);
    int end = igate + nGatesHalf;
    if (end > nGates - 1) {
      end = nGates - 1;
    }
    endGate.push_back(end);
  } // igate
  
  // tdbz is based on spatial variation in range

  for (int igate = 0; igate < nGates; igate++) {
    
    // compute sums etc. for stats over the kernel space
    
    double nTexture = 0.0;
    double sumDbzDiffSq = 0.0;
    
    for (int jgate = startGate[igate]; jgate <= endGate[igate]; jgate++) {
      
      double dds = _gateData[jgate]->flds->dbz_diff_sq;
      if (dds != MomentsFields::missingDouble) {
        sumDbzDiffSq += dds;
        nTexture++;
      }
      
    } // jgate
    
    if (nTexture > 0) {
      double texture = sumDbzDiffSq / nTexture;
      _gateData[igate]->flds->tdbz = texture;
    }

  } // igate

}

/////////////////////////////////////////////////
// compute SPIN

void Cmd::_computeSpin(int nGates)
  
{

  // compute number of gates in kernel, making sure there is an odd number
  
  int nGatesKernel = _params.cmd_kernel_ngates_spin;
  int nGatesHalf = nGatesKernel / 2;

  // set up gate limits
  
  vector<int> startGate;
  vector<int> endGate;
  for (int igate = 0; igate < nGates; igate++) {
    int start = igate - nGatesHalf;
    if (start < 0) {
      start = 0;
    }
    startGate.push_back(start);
    int end = igate + nGatesHalf;
    if (end > nGates - 1) {
      end = nGates - 1;
    }
    endGate.push_back(end);
  } // igate
  
  // spin is based on spatial variation in range and azimuth

  for (int igate = 0; igate < nGates; igate++) {
    
    // compute sums etc. for stats over the kernel space
    
    double nSpinChange = 0.0;
    double nSpinTotal = 0.0;
    
    for (int jgate = startGate[igate]; jgate <= endGate[igate]; jgate++) {
        
      MomentsFields *flds = _gateData[jgate]->flds;
      double dsc = flds->dbz_spin_change;
      double dbz = flds->dbz;
      if (dsc != MomentsFields::missingDouble &&
          dbz != MomentsFields::missingDouble) {
        nSpinChange += dsc;
        nSpinTotal++;
      }

    } // jgate
      
    if (nSpinTotal > 0) {
      _gateData[igate]->flds->spin = (nSpinChange / nSpinTotal) * 100.0;
    }

  } // igate

}

/////////////////////////////////////////////////
// compute max of TDBZ and SPIN interest

void Cmd::_computeMaxTdbzAndSpinInterest(int nGates)
  
{
  
  for (int igate = 0; igate < nGates; igate++) {
    
    MomentsFields *flds = _gateData[igate]->flds;
    double tdbz = flds->tdbz;
    double spin = flds->spin;
    
    if (tdbz == MomentsFields::missingDouble ||
        spin == MomentsFields::missingDouble) {
      continue;
    }
    
    double tdbzInterest = _tdbzInterestMap->getInterest(tdbz);
    double spinInterest = _spinInterestMap->getInterest(spin);
    
    if (tdbzInterest > spinInterest) {
      flds->max_tdbz_spin = tdbzInterest;
    } else {
      flds->max_tdbz_spin = spinInterest;
    }

  } // igate

}

/////////////////////////////////////////////////
// compute for ZDR SDEV

void Cmd::_computeZdrSdev(int nGates)
  
{
  
  // compute number of gates in kernel, making sure there is an odd number
  
  int nGatesKernel = _params.cmd_kernel_ngates_zdr_sdev;
  int nGatesHalf = nGatesKernel / 2;
  
  // set up gate limits
  
  vector<int> startGate;
  vector<int> endGate;
  for (int igate = 0; igate < nGates; igate++) {
    int start = igate - nGatesHalf;
    if (start < 0) {
      start = 0;
    }
    startGate.push_back(start);
    int end = igate + nGatesHalf;
    if (end > nGates - 1) {
      end = nGates - 1;
    }
    endGate.push_back(end);
  } // igate
  
  // sdve computed in range
  
  double snrThresh = _params.cmd_snr_threshold;
  for (int igate = 0; igate < nGates; igate++) {
    
    MomentsFields *iflds = _gateData[igate]->flds;
    if (iflds->snr < snrThresh) {
      continue;
    }

    // compute sums etc. for stats over the kernel space
    
    double nZdr = 0.0;
    double sumZdr = 0.0;
    double sumZdrSq = 0.0;
    
    for (int jgate = startGate[igate]; jgate <= endGate[igate]; jgate++) {
      
      const MomentsFields *jflds = _gateData[jgate]->flds;
    
      double zz = jflds->zdr;
      if (zz != MomentsFields::missingDouble) {
        sumZdr += zz;
        sumZdrSq += (zz * zz);
        nZdr++;
      }
      
    } // jgate
    
    if (nZdr > 0) {
      double meanZdr = sumZdr / nZdr;
      if (nZdr > 2) {
        double term1 = sumZdrSq / nZdr;
        double term2 = meanZdr * meanZdr;
        if (term1 >= term2) {
          iflds->zdr_sdev = sqrt(term1 - term2);
        }
      }
    }
    
  } // igate

}

/////////////////////////////////////////////////
// compute SDEV for PHIDP SDEV
//
// Old method, ignored folding

void Cmd::_computePhidpSdevOld(int nGates)
  
{
  
  // compute number of gates in kernel, making sure there is an odd number
  
  int nGatesKernel = _params.cmd_kernel_ngates_phidp_sdev;
  int nGatesHalf = nGatesKernel / 2;
  
  // set up gate limits
  
  vector<int> startGate;
  vector<int> endGate;
  for (int igate = 0; igate < nGates; igate++) {
    int start = igate - nGatesHalf;
    if (start < 0) {
      start = 0;
    }
    startGate.push_back(start);
    int end = igate + nGatesHalf;
    if (end > nGates - 1) {
      end = nGates - 1;
    }
    endGate.push_back(end);
  } // igate
  
  // sdve computed in range
  
  double snrThresh = _params.cmd_snr_threshold;
  for (int igate = 0; igate < nGates; igate++) {
    
    MomentsFields *iflds = _gateData[igate]->flds;
    if (iflds->snr < snrThresh) {
      continue;
    }

    // compute sums etc. for stats over the kernel space
    
    double nPhidp = 0.0;
    double sumPhidp = 0.0;
    double sumPhidpSq = 0.0;
    
    for (int jgate = startGate[igate]; jgate <= endGate[igate]; jgate++) {
      
      const MomentsFields *jflds = _gateData[jgate]->flds;
    
      double ph = jflds->phidp;
      if (ph != MomentsFields::missingDouble) {
        sumPhidp += ph;
        sumPhidpSq += (ph * ph);
        nPhidp++;
      }
      
    } // jgate
    
    if (nPhidp > 0) {
      double meanPhidp = sumPhidp / nPhidp;
      if (nPhidp > 2) {
        double term1 = sumPhidpSq / nPhidp;
        double term2 = meanPhidp * meanPhidp;
        if (term1 >= term2) {
          iflds->phidp_sdev = sqrt(term1 - term2);
        }
      }
    }
    
  } // igate
  
  // try KDP object method
  
  KdpFilt kdp;
  kdp.setNGatesStats(_params.cmd_kernel_ngates_phidp_sdev);
  TaArray<double> phidp_;
  double *phidp = phidp_.alloc(nGates);
  for (int igate = 0; igate < nGates; igate++) {
    MomentsFields *iflds = _gateData[igate]->flds;
    phidp[igate] = iflds->phidp;
  }
  kdp.computePhidpStats(nGates,
                        _startRangeKm, _gateSpacingKm,
                        phidp,
                        MomentsFields::missingDouble);
  const double *phidpSdev = kdp.getPhidpSdev();
  for (int igate = 0; igate < nGates; igate++) {
    MomentsFields *iflds = _gateData[igate]->flds;
    iflds->phidp_sdev_4kdp = phidpSdev[igate];
  }

}

/////////////////////////////////////////////////
// compute SDEV for PHIDP SDEV
//
// New methods, takes account of folding

void Cmd::_computePhidpSdevNew(int nGates)
  
{

  // compute number of gates in kernel, making sure there is an odd number
  
  int nGatesKernel = _params.cmd_kernel_ngates_phidp_sdev;
  int nGatesHalf = nGatesKernel / 2;
  
  // save phidp to array

  _phidpStates.resize(nGates);
  
  for (int igate = 0; igate < nGates; igate++) {
    PhidpState &state = _phidpStates[igate];
    state.init(MomentsFields::missingDouble);
    state.phidp = _gateData[igate]->flds->phidp;
    if (state.phidp != MomentsFields::missingDouble) {
      state.missing = false;
    }
  }

  // compute folding range

  _computePhidpFoldingRange(nGates);
  
  // init (x,y) representation of phidp
  
  for (int igate = 0; igate < nGates; igate++) {
    PhidpState &state = _phidpStates[igate];
    if (!state.missing) {
      double phase = state.phidp;
      if (_phidpFoldsAt90) {
        phase *= 2.0;
      }
      double sinVal, cosVal;
      ta_sincos(phase * DEG_TO_RAD, &sinVal, &cosVal);
      state.xx = cosVal;
      state.yy = sinVal;
    }
  }

  // compute mean phidp at each gate

  for (int igate = 0; igate < nGates; igate++) {
  
    PhidpState &istate = _phidpStates[igate];
    
    double count = 0.0;
    double sumxx = 0.0;
    double sumyy = 0.0;
    
    for (int jj = igate - nGatesHalf; jj <= igate + nGatesHalf; jj++) {
      if (jj < 0 || jj >= nGates) {
        continue;
      }
      PhidpState &jstate = _phidpStates[jj];
      if (jstate.missing) {
        continue;
      }
      double xx = jstate.xx;
      double yy = jstate.yy;
      sumxx += xx;
      sumyy += yy;
      count++;
    }
  
    if (count <= nGatesHalf) {
      continue;
    }
    
    istate.meanxx = sumxx / count;
    istate.meanyy = sumyy / count;
    
    double phase = atan2(istate.meanyy, istate.meanxx) * RAD_TO_DEG;
    if (_phidpFoldsAt90) {
      phase *= 0.5;
    }
    istate.phidpMean = phase;

  } // igate
  
  // compute standard deviation at each gate
  // we center on the mean value
  
  for (int igate = 0; igate < nGates; igate++) {
    
    PhidpState &istate = _phidpStates[igate];
    
    double count = 0.0;
    double sum = 0.0;
    double sumSq = 0.0;
    
    for (int jj = igate - nGatesHalf; jj <= igate + nGatesHalf; jj++) {
      
      if (jj < 0 || jj >= nGates) {
        continue;
      }

      PhidpState &jstate = _phidpStates[jj];
      if (jstate.missing) {
        continue;
      }

      // compute difference between this value and the mean

      double diff = jstate.phidp - istate.phidpMean;

      // constrain diff
      
      while (diff < -_phidpFoldVal) {
        diff += 2 * _phidpFoldVal;
      }
      while (diff > _phidpFoldVal) {
        diff -= 2 * _phidpFoldVal;
      }

      // sum up

      count++;
      sum += diff;
      sumSq += diff * diff;
      
    }

    if (count <= nGatesHalf || count < 3) {
      istate.phidpSdev = MomentsFields::missingDouble;
    } else {
      double meanDiff = sum / count;
      double term1 = sumSq / count;
      double term2 = meanDiff * meanDiff;
      if (term1 >= term2) {
        istate.phidpSdev = sqrt(term1 - term2);
      }
    }

    _gateData[igate]->flds->phidp_sdev = istate.phidpSdev;
    
  } // igate
  
}

/////////////////////////////////////////////
// compute the folding values and range
// by inspecting the phidp values

void Cmd::_computePhidpFoldingRange(int nGates)
  
{
  
  // check if fold is at 90 or 180
  
  double phidpMin = 9999;
  double phidpMax = -9999;

  for (int igate = 0; igate < nGates; igate++) {
    if (!_phidpStates[igate].missing) {
      double phidp = _phidpStates[igate].phidp;
      if (phidp < phidpMin) phidpMin = phidp;
      if (phidp > phidpMax) phidpMax = phidp;
    }
  } // igate
  
  _phidpFoldsAt90 = false;
  _phidpFoldVal = 180.0;
  if (phidpMin > -90 && phidpMax < 90) {
    _phidpFoldVal = 90.0;
    _phidpFoldsAt90 = true;
  }
  _phidpFoldRange = _phidpFoldVal * 2.0;
  
  // if values range from (0 -> 360), normalize to (-180 -> 180)
  
  if (phidpMin >= 0 && phidpMax > 180) {
    for (int igate = 0; igate < nGates; igate++) {
      if (!_phidpStates[igate].missing) {
        _phidpStates[igate].phidp -= 180.0;
      }
    }
  }

}

/////////////////////////////////////////////////
// compute RHOHV test from unwindowed IQ data

void Cmd::_computeRhohvTest(const RadarMoments *mom, int nGates)
  
{

  if (nGates < 1) {
    return;
  }
  int nSamples = _gateData[0]->_nSamples;
  if (_nSamples > nSamples) {
    cerr << "ERROR - Cmd::_computeRhohvTest" << endl;
    cerr << "  Number of samples exceeds alloc: " << nSamples << endl;
    cerr << "  Number of samples now: " << _nSamples << endl;
    return;
  }

  // for the regression filter we need to use data with rectangular window

  int nSamplesRegr = _nSamplesRect;
  if (mom->getIsStagPrt()) {
    nSamplesRegr = _nSamplesRect / 2;
  }
  int iqOffset = 0;
  if (_nSamples != _nSamplesRect) {
    iqOffset = (_nSamples - _nSamplesRect) / 2;
  }
  
  // initialize regression filter

  _regr.setup(nSamplesRegr, false, 5, 1.0, 0.6667, mom->getWavelengthMeters());
  
  for (int igate = 0; igate < nGates; igate++) {
    
    // make copy of IQ for this gate, unwindowed
    
    GateData *gate = _gateData[igate];
    vector<RadarComplex_t> iqhc, iqvc;
    iqhc.resize(_nSamplesRect);
    iqvc.resize(_nSamplesRect);
    const RadarComplex_t *iqhcOrig = gate->iqhcOrig + iqOffset;
    const RadarComplex_t *iqvcOrig = gate->iqvcOrig + iqOffset;
    for (int ii = 0; ii < _nSamplesRect; ii++) {
      iqhc[ii] = iqhcOrig[ii];
      iqvc[ii] = iqvcOrig[ii];
    }
    
    if (_xmitRcvMode != IWRF_ALT_HV_CO_CROSS &&
        _xmitRcvMode != IWRF_ALT_HV_FIXED_HV &&
        _xmitRcvMode != IWRF_SIM_HV_FIXED_HV &&
        _xmitRcvMode != IWRF_SIM_HV_SWITCHED_HV) {
      // cannot compute it
      gate->flds->rhohv_test_improv = -1.0;
      continue;
    }

    // compute rhohv from unfiltered time series

    double rhohvUnfilt = -1.0;
    if (_xmitRcvMode == IWRF_ALT_HV_CO_CROSS ||
        _xmitRcvMode == IWRF_ALT_HV_FIXED_HV) {
      rhohvUnfilt = mom->rhohvAltHvCoCross(_nSamplesRect, iqhc.data(), iqvc.data());
    } else if (_xmitRcvMode == IWRF_SIM_HV_FIXED_HV ||
               _xmitRcvMode == IWRF_SIM_HV_SWITCHED_HV) {
      rhohvUnfilt = mom->rhohvDpSimHv(_nSamplesRect, iqhc.data(), iqvc.data());
    }

    // apply regression filter to time series

    vector<RadarComplex_t> filthc, filtvc;
    filthc.resize(_nSamplesRect);
    filtvc.resize(_nSamplesRect);

    if (mom->getIsStagPrt()) {
      _applyRegrFiltStag(iqhc, filthc);
      _applyRegrFiltStag(iqvc, filtvc);
    } else {
      _applyRegrFiltFixed(iqhc, filthc);
      _applyRegrFiltFixed(iqvc, filtvc);
    }

    // compute rhohv from filtered time series

    double rhohvFilt = -1.0;
    if (_xmitRcvMode == IWRF_ALT_HV_CO_CROSS ||
        _xmitRcvMode == IWRF_ALT_HV_FIXED_HV) {
      rhohvFilt = mom->rhohvAltHvCoCross(_nSamplesRect, filthc.data(), filtvc.data());
    } else if (_xmitRcvMode == IWRF_SIM_HV_FIXED_HV ||
               _xmitRcvMode == IWRF_SIM_HV_SWITCHED_HV) {
      rhohvFilt = mom->rhohvDpSimHv(_nSamplesRect, filthc.data(), filtvc.data());
    }

    // compute rhohv improvement

    double factorUnfilt = 1.0 - rhohvUnfilt;
    double factorFilt = 1.0 - rhohvFilt;
    if (factorFilt < 0.001) {
      factorFilt = 0.001;
    }
    double rhohvImprov = factorUnfilt / factorFilt;
    
    gate->flds->rhohv_test_unfilt = rhohvUnfilt;
    gate->flds->rhohv_test_filt = rhohvFilt;
    if (rhohvUnfilt > 0.98 || rhohvFilt < 0.5) {
      gate->flds->rhohv_test_improv = 0.0;
    } else {
      gate->flds->rhohv_test_improv = rhohvImprov;
    }
    
  } // igate

}

////////////////////////////////////////////////////////////////////////
// Apply regression filter to IQ array.
// The array is filtered in-place.

void Cmd::_applyRegrFiltFixed(const vector<RadarComplex_t> &iq,
                              vector<RadarComplex_t> &filt)
{

  assert(iq.size() == filt.size());
  assert(iq.size() == _regr.getNSamples());
  
  // apply filter
  
  _regr.apply(iq.data(), 3, filt.data());
  
}

void Cmd::_applyRegrFiltStag(const vector<RadarComplex_t> &iq,
                             vector<RadarComplex_t> &filt)
{

  assert(iq.size() == filt.size());
  assert(iq.size() == _regr.getNSamples() * 2);
  
  int nSamples = iq.size();
  int nSamplesHalf = nSamples / 2;

  // split the IQ data into 2 array, long-prt and short-prt
  
  vector<RadarComplex_t> iqShort, iqLong;
  iqShort.resize(nSamplesHalf);
  iqLong.resize(nSamplesHalf);
  for (int jj = 0; jj < nSamplesHalf; jj++) {
    iqShort[jj] = iq[jj * 2];
    iqLong[jj] = iq[jj * 2 + 1];
  }

  // filter short and long separately
  
  vector<RadarComplex_t> filtShort, filtLong;
  filtShort.resize(nSamplesHalf);
  filtLong.resize(nSamplesHalf);
  
  _regr.apply(iqShort.data(), 3, filtShort.data());
  _regr.apply(iqLong.data(), 3, filtLong.data());

  // copy results to filt array
  
  for (int jj = 0; jj < nSamplesHalf; jj++) {
    filt[jj * 2] = filtShort[jj];
    filt[jj * 2 + 1] = filtLong[jj];
  }

}

////////////////////////////////////////////////////////////////////////
// Convert interest map points to vector
//
// Returns 0 on success, -1 on failure

int Cmd::_convertInterestMapToVector(const string &label,
                                     const Params::interest_map_point_t *map,
                                     int nPoints,
                                     vector<InterestMap::ImPoint> &pts)

{

  pts.clear();

  double prevVal = -1.0e99;
  for (int ii = 0; ii < nPoints; ii++) {
    if (map[ii].value <= prevVal) {
      cerr << "ERROR - Cmd::_convertInterestMapToVector" << endl;
      cerr << "  Map label: " << label << endl;
      cerr << "  Map values must increase monotonically" << endl;
      return -1;
    }
    InterestMap::ImPoint pt(map[ii].value, map[ii].interest);
    pts.push_back(pt);
    prevVal = map[ii].value;
  } // ii
  
  return 0;

}

/////////////////////////////////////////////
// apply gap in-fill filter to CMD flag
//
// After CMD is run, the CMD flag field tends to have gaps
// which really should be filtered, since they are surrounded by
// filtered gates. This infill process is designed to fill the
// gaps in the flag field.
//
// Initialization:
//
// A template of weights, of length 6, is computed with the following values:
//   1, 1/2, 1/3, 1/4, 1/5, 1/6
//
// Computing the forward sum of weights * cmd:
//
//   For each gate at which the flag is not yet set, compute the sum of
//   the (weight * cmd) for each of the previous 6 gates at which the
//   flag field is set.
//   A weight of 1*cmd applies to the previous gate,
//   (1/2)*cmd applies to the second previous gate, etc.
// 
// Computing the reverse sum of weights:
//
//   For each gate at which the flag is not yet set, compute the sum of
//   the (weight * cmd) for each of the next 6 gates at which the
//   flag field is set.
//   The weights are used in the reverse sense, i.e 1*cmd applies to the next
//   gate, (1/2)*cmd applies to the second next gate etc.
//
// If both the forward sum and the reverse sum exceed the threshold, then
// this gate is considered likely to have clutter, and the cmd_flag is set.

void Cmd::_applyGapFilter(int nGates)

{

  // set up weights for distance differences

  int nweights = _params.cmd_gap_filter_len;
  double *weights = new double[nweights];
  for (int ii = 0; ii < nweights; ii++) {
    weights[ii] = 1.0 / (ii + 1.0);
  }
  double sumWtThreshold = _params.cmd_gap_filter_threshold;
  
  // allocate arrays for sum of weights with the flag field
  
  double *sumWtsForward = new double[nGates];
  double *sumWtsReverse = new double[nGates];
  
  bool done = false;
  int count = 0;
  while (!done) {

    count++;
    done = true;
    
    // compute sum in forward direction
    
    for (int igate = 0; igate < nGates; igate++) {
      if (_gateData[igate]->flds->cmd_flag) {
        sumWtsForward[igate] = 1.0;
        continue; // flag already set, don't need to modify this gate
      }
      sumWtsForward[igate] = 0.0;
      for (int jj = 0; jj < nweights; jj++) {
        int kgate = igate - jj - 1;
        if (kgate >= 0) {
          if (_gateData[kgate]->flds->cmd_flag) {
            sumWtsForward[igate] += weights[jj] * _gateData[kgate]->flds->cmd;
          }
        }
      }
    }
    
    // compute sum in reverse direction
    
    for (int igate = nGates - 1; igate >= 0; igate--) {
      if (_gateData[igate]->flds->cmd_flag) {
        sumWtsReverse[igate] = 1.0;
        continue; // flag already set, don't need to modify this gate
      }
      sumWtsReverse[igate] = 0.0;
      for (int jj = 0; jj < nweights; jj++) {
        int kgate = igate + jj + 1;
        if (kgate < nGates) {
          if (_gateData[kgate]->flds->cmd_flag) {
            sumWtsReverse[igate] += weights[jj] * _gateData[kgate]->flds->cmd;
          }
        }
      }
    }
    
    // fill in flag field if flag field is not already set and
    // both forward sum and reverse sum exceed threshold
    
    for (int igate = 0; igate < nGates; igate++) {
      if (!_gateData[igate]->flds->cmd_flag) {
        if (sumWtsForward[igate] > sumWtThreshold &&
            sumWtsReverse[igate] > sumWtThreshold) {
          _gateData[igate]->flds->cmd_flag = 1;
          done = false;
        }
      }
    }

    if (count > 3) {
      done = true;
    }

  } // while (!done)

  delete[] sumWtsForward;
  delete[] sumWtsReverse;
  delete[] weights;

}

////////////////////////////////////////////////////////////
// apply C version of gap filter

void Cmd::_applyGapFilterCVersion(int nGates)

{

  TaArray<int> cmd_flag_;
  int *cmd_flag = cmd_flag_.alloc(nGates);
  TaArray<double> cmd_;
  double *cmd = cmd_.alloc(nGates);

  for (int igate = 0; igate < nGates; igate++) {
    cmd_flag[igate] = (int) (_gateData[igate]->flds->cmd_flag + 0.5);
    cmd[igate] = _gateData[igate]->flds->cmd;
  }
  
  apply_cmd_flag_gap_filter(nGates, cmd, cmd_flag,
                            _params.cmd_gap_filter_len,
                            _params.cmd_gap_filter_threshold);

  for (int igate = 0; igate < nGates; igate++) {
    _gateData[igate]->flds->cmd_flag = cmd_flag[igate];
  }

}

////////////////////////////////////////////////////////////
// apply C version of speckle filter

void Cmd::_applySpeckleFilterCVersion(int nGates)

{

  int igate, ii;
  TaArray<int> cmd_flag_;
  int *cmd_flag = cmd_flag_.alloc(nGates);
  TaArray<double> cmd_;
  double *cmd = cmd_.alloc(nGates);

  for (igate = 0; igate < nGates; igate++) {
    cmd_flag[igate] = (int) (_gateData[igate]->flds->cmd_flag + 0.5);
    cmd[igate] = _gateData[igate]->flds->cmd;
  }
  
  int nThresholds = _params.cmd_speckle_filter_thresholds_n;
  TaArray<int> runLen_;
  int *runLen = runLen_.alloc(nThresholds);
  TaArray<double> modThresholds_;
  double *modThresholds = modThresholds_.alloc(nThresholds);
  
  for (ii = 0; ii < nThresholds; ii++) {
    runLen[ii] = _params._cmd_speckle_filter_thresholds[ii].length;
    modThresholds[ii] =
      _params._cmd_speckle_filter_thresholds[ii].min_valid_cmd;
  }
  
  apply_cmd_speckle_filter(nGates, cmd, cmd_flag,
                           nThresholds, runLen, modThresholds);

  for (igate = 0; igate < nGates; igate++) {
    _gateData[igate]->flds->cmd_flag = cmd_flag[igate];
  }

}

/////////////////////////////////////////////////////////////
// apply old in-fill filter to CMD flag

void Cmd::_applyInfillFilter(int nGates)

{
  
  int *countSet = new int[nGates];
  int *countNot = new int[nGates];

  // compute the running count of gates which have the flag set and
  // those which do not

  // Go forward through the gates, counting up the number of gates set
  // or not set and assigning that number to the arrays as we go.

  int nSet = 0;
  int nNot = 0;
  for (int igate = 0; igate < nGates; igate++) {
    if (_gateData[igate]->flds->cmd_flag) {
      nSet++;
      nNot = 0;
    } else {
      nSet = 0;
      nNot++;
    }
    countSet[igate] = nSet;
    countNot[igate] = nNot;
  }

  // Go in reverse through the gates, taking the max non-zero
  // values and copying them across the set or not-set regions.
  // This makes all the counts equal in the gaps and set areas.

  for (int igate = nGates - 2; igate >= 0; igate--) {
    if (countSet[igate] != 0 &&
        countSet[igate] < countSet[igate+1]) {
      countSet[igate] = countSet[igate+1];
    }
    if (countNot[igate] != 0 &&
        countNot[igate] < countNot[igate+1]) {
      countNot[igate] = countNot[igate+1];
    }
  }

  // fill in gaps

  for (int igate = 1; igate < nGates - 1; igate++) {

    // is the gap small enough?

    nNot = countNot[igate];
    // if (nNot > 0 && nNot <= _params.cmd_speckle_max_ngates_infilled) {
    if (nNot > 0 && nNot <= 3) {

      // is it surrounded by regions at least as large as the gap?

      int minGateCheck = igate - nNot;
      if (minGateCheck < 0) {
        minGateCheck = 0;
      }
      int maxGateCheck = igate + nNot;
      if (maxGateCheck > nGates - 1) {
        maxGateCheck = nGates - 1;
      }

      int nAdjacentBelow = 0;
      for (int jgate = igate - 1; jgate >= minGateCheck; jgate--) {
        nSet = countSet[jgate];
        if (nSet != 0) {
          nAdjacentBelow = nSet;
          break;
        }
      } // jgate

      int nAdjacentAbove = 0;
      for (int jgate = igate + 1; jgate <= maxGateCheck; jgate++) {
        nSet = countSet[jgate];
        if (nSet != 0) {
          nAdjacentAbove = nSet;
          break;
        }
      } // jgate

      int minAdjacent = nAdjacentBelow;
      minAdjacent = MIN(minAdjacent, nAdjacentAbove);
      
      if (minAdjacent >= nNot) {
        _gateData[igate]->flds->cmd_flag = 1;
      }
    }
  } // igate

  delete[] countSet;
  delete[] countNot;

}

//////////////////////////////////////////////////
// apply speckle filter for various length speckle
// starting with the longest and working down

void Cmd::_applySpeckleFilter(int nGates)

{

  // compute the max specified speckle length
  int maxLen = 0;
  for (int ii = 0; ii < _params.cmd_speckle_filter_thresholds_n; ii++) {
    if (_params._cmd_speckle_filter_thresholds[ii].length > maxLen) {
      maxLen = _params._cmd_speckle_filter_thresholds[ii].length;
    }
  }

  // for each specified speckle length, run the speckle filter

  int thisLen = maxLen;
  while (thisLen > 0) {
    for (int ii = 0; ii < _params.cmd_speckle_filter_thresholds_n; ii++) {
      if (_params._cmd_speckle_filter_thresholds[ii].length == thisLen) {
        double minValidCmd =
          _params._cmd_speckle_filter_thresholds[ii].min_valid_cmd;
        _runSpeckleFilter(nGates, thisLen, minValidCmd);
        break;
      }
    }
    thisLen--;
  }

}

///////////////////////////////////////////////////////
// run speckle filter for a given length and threshold
//
// minRunLen: length of run being tested for
// minRunLen: if a run is less than or equal to the min length,
//            apply the more stringent CMD threshold to modify
//            the flag field

void Cmd::_runSpeckleFilter(int nGates,
                            int minRunLen,
                            double minValidCmd)
  
{

  int count = 0;
  // loop through all gates
  for (int ii = 0; ii < nGates; ii++) {
    // check for CMD flag status
    if (_gateData[ii]->flds->cmd_flag) {
      // set, so count up length of run
      count++;
    } else {
      // not set, end of run
      if (count <= minRunLen) {
        // run too short, indicates possible speckle
        for (int jj = ii - count; jj < ii; jj++) {
          // check for CMD values below threshold
          if (_gateData[jj]->flds->cmd < minValidCmd) {
            _gateData[jj]->flds->cmd_flag = false;
          }
        }
      }
      count = 0;
    }
  } // ii

}

////////////////////////////////////////////////////////////////////////
// Filter fields for spikes in dbz
//
// This routine filters the reflectivity data according to the
// NEXRAD specification DV1208621F, section 3.2.1.2.2, page 3-15.
//
// The algorithm is stated as follows:
//
// Clutter detection:
//
// The nth bin is declared to be a point clutter cell if its power value
// exceeds those of both its second nearest neighbors by a threshold
// value TCN. In other words:
//
//    if   P(n) exceeds TCN * P(n-2)
//    and  P(n) exceeds TCN * p(n+2)
//
//  where
//
//   TCN is the point clutter threshold factor, which is always
//       greater than 1, and typically has a value of 8 (9 dB)
//
//   P(n) if the poiwer sum value for the nth range cell
//
//   n is the range gate number
//
// Clutter censoring:
//
// The formulas for censoring detected strong point clutter in an
// arbitrary array A via data substitution are as follows. If the nth
// range cell is an isolated clutter cell (i.e., it si a clutter cell but
// neither of its immediate neighboring cells is a clutter cell) then the 
// replacement scheme is as follows:
//
//   Replace A(n-1) with  A(n-2)
//   Replace A(n)   with  0.5 * A(n-2) * A(n+2)
//   Replace A(n+1) with  A(n+2)
//
// If the nth and (n+1)th range bins constitute an isolated clutter pair,
// the bin replacement scheme is as follows:
//
//   Replace A(n-1) with  A(n-2)
//   Replace A(n)   with  A(n+2)
//   Replace A(n+1) with  A(n+3)
//   Replace A(n+2) with  A(n+3)
//
// Note that runs of more than 2 successive clutter cells cannot occur
// because of the nature of the algorithm.
 
void Cmd::_applyNexradSpikeFilter(int nGates)
  
{
  
  // set clutter threshold

  double tcn = 9.0;

  // loop through gates

  for (int ii = 2; ii < nGates - 3; ii++) {
    
    MomentsFields *fldMinus2F = _gateData[ii - 2]->fldsF;
    MomentsFields *fldMinus1F = _gateData[ii - 1]->fldsF;
    MomentsFields *fldF = _gateData[ii]->fldsF;
    MomentsFields *fldPlus1F = _gateData[ii + 1]->fldsF;
    MomentsFields *fldPlus2F = _gateData[ii + 2]->fldsF;
    MomentsFields *fldPlus3F = _gateData[ii + 3]->fldsF;

    // check for clutter at ii and ii + 1

    bool this_gate = false, next_gate = false;
    
    if ((fldF->dbz - fldMinus2F->dbz) > tcn &&
	(fldF->dbz - fldPlus2F->dbz) > tcn) {
      this_gate = true;
    }
    if ((fldPlus1F->dbz - fldMinus1F->dbz) > tcn &&
	(fldPlus1F->dbz - fldPlus3F->dbz) > tcn) {
      next_gate = true;
    }

    if (this_gate) {

      if (!next_gate) {

	// only gate ii has clutter, substitute accordingly
	
	fldMinus1F->dbz = fldMinus2F->dbz;
	fldPlus1F->dbz = fldPlus2F->dbz;
	if (fldMinus2F->dbz == MomentsFields::missingDouble ||
            fldPlus2F->dbz == MomentsFields::missingDouble) {
	  fldF->dbz = MomentsFields::missingDouble;
	  fldF->vel = MomentsFields::missingDouble;
	  fldF->width = MomentsFields::missingDouble;
	} else {
	  fldF->dbz = fldMinus2F->dbz;
	  fldF->vel = fldMinus2F->vel;
	  fldF->width = fldMinus2F->width;
	}
	
      } else {

	// both gate ii and ii+1 has clutter, substitute accordingly

	fldMinus1F->dbz = fldMinus2F->dbz;
	fldF->dbz = fldMinus2F->dbz;
	fldPlus1F->dbz = fldPlus3F->dbz;
	fldPlus2F->dbz = fldPlus3F->dbz;

	fldMinus1F->vel = fldMinus2F->vel;
	fldF->vel = fldMinus2F->vel;
	fldPlus1F->vel = fldPlus3F->vel;
	fldPlus2F->vel = fldPlus3F->vel;

	fldMinus1F->width = fldMinus2F->width;
	fldF->width = fldMinus2F->width;
	fldPlus1F->width = fldPlus3F->width;
	fldPlus2F->width = fldPlus3F->width;

      }

    }
    
  } // ii

}

