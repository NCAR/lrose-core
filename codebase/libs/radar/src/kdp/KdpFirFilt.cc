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
// KdpFirFilt.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2008
//
///////////////////////////////////////////////////////////////

#include <cmath>
#include <cstring>
#include <radar/KdpFirFilt.hh>
using namespace std;

const double KdpFirFilt::firCoeff_125[FIR_LEN_125+1] = {
  2.5107443e-003,2.6960328e-003,2.8834818e-003,3.0729344e-003,
  3.2642298e-003,3.4572038e-003,3.6516884e-003,3.8475124e-003,
  4.0445018e-003,4.2424792e-003,4.4412651e-003,4.6406770e-003,
  4.8405305e-003,5.0406391e-003,5.2408146e-003,5.4408670e-003,
  5.6406054e-003,5.8398373e-003,6.0383699e-003,6.2360093e-003,
  6.4325618e-003,6.6278331e-003,6.8216293e-003,7.0137569e-003,
  7.2040230e-003,7.3922356e-003,7.5782040e-003,7.7617385e-003,
  7.9426516e-003,8.1207572e-003,8.2958715e-003,8.4678134e-003,
  8.6364038e-003,8.8014671e-003,8.9628303e-003,9.1203239e-003,
  9.2737821e-003,9.4230427e-003,9.5679474e-003,9.7083424e-003,
  9.8440780e-003,9.9750093e-003,1.0100996e-002,1.0221904e-002,
  1.0337601e-002,1.0447965e-002,1.0552875e-002,1.0652219e-002,
  1.0745888e-002,1.0833782e-002,1.0915804e-002,1.0991867e-002,
  1.1061886e-002,1.1125785e-002,1.1183495e-002,1.1234953e-002,
  1.1280103e-002,1.1318895e-002,1.1351287e-002,1.1377243e-002,
  1.1396735e-002,1.1409742e-002,1.1416248e-002,1.1416248e-002,
  1.1409742e-002,1.1396735e-002,1.1377243e-002,1.1351287e-002,
  1.1318895e-002,1.1280103e-002,1.1234953e-002,1.1183495e-002,
  1.1125785e-002,1.1061886e-002,1.0991867e-002,1.0915804e-002,
  1.0833782e-002,1.0745888e-002,1.0652219e-002,1.0552875e-002,
  1.0447965e-002,1.0337601e-002,1.0221904e-002,1.0100996e-002,
  9.9750093e-003,9.8440780e-003,9.7083424e-003,9.5679474e-003,
  9.4230427e-003,9.2737821e-003,9.1203239e-003,8.9628303e-003,
  8.8014671e-003,8.6364038e-003,8.4678134e-003,8.2958715e-003,
  8.1207572e-003,7.9426516e-003,7.7617385e-003,7.5782040e-003,
  7.3922356e-003,7.2040230e-003,7.0137569e-003,6.8216293e-003,
  6.6278331e-003,6.4325618e-003,6.2360093e-003,6.0383699e-003,
  5.8398373e-003,5.6406054e-003,5.4408670e-003,5.2408146e-003,
  5.0406391e-003,4.8405305e-003,4.6406770e-003,4.4412651e-003,
  4.2424792e-003,4.0445018e-003,3.8475124e-003,3.6516884e-003,
  3.4572038e-003,3.2642298e-003,3.0729344e-003,2.8834818e-003,
  2.6960328e-003,2.5107443e-003};

const double KdpFirFilt::firCoeff_60[FIR_LEN_60+1] = {
  0.005192387815,0.006000584633,0.006826878703,
  0.007668199579,0.008521340618,0.009382975237,
  0.01024967409,0.01111792308,0.0119841421,
  0.01284470437,0.01369595621,0.01453423726,
  0.01535590085,0.01615733448,0.01693498027,
  0.01768535525,0.01840507134,0.01909085485,
  0.01973956552,0.02034821473,0.02091398302,
  0.02143423665,0.02190654305,0.02232868523,
  0.0226986749,0.02301476423,0.02327545631,
  0.02347951399,0.02362596732,0.02371411929,
  0.02374355002,0.02371411929,0.02362596732,
  0.02347951399,0.02327545631,0.02301476423,
  0.0226986749,0.02232868523,0.02190654305,
  0.02143423665,0.02091398302,0.02034821473,
  0.01973956552,0.01909085485,0.01840507134,
  0.01768535525,0.01693498027,0.01615733448,
  0.01535590085,0.01453423726,0.01369595621,
  0.01284470437,0.0119841421,0.01111792308,
  0.01024967409,0.009382975237,0.008521340618,
  0.007668199579,0.006826878703,0.006000584633,
  0.005192387815 };

const double KdpFirFilt::firCoeff_40[FIR_LEN_40+1] = {
  0.007806525986, 0.009628559511, 0.01150585082,
  0.01342243276, 0.01536143961, 0.01730530352,
  0.0192359639, 0.02113508696, 0.02298429218,
  0.02476538263, 0.02646057561, 0.02805273044,
  0.02952556994, 0.03086389233, 0.03205377043,
  0.03308273518, 0.03393994069, 0.03461630839,
  0.03510464806, 0.0353997539, 0.03549847428,
  0.0353997539, 0.03510464806, 0.03461630839,
  0.03393994069, 0.03308273518, 0.03205377043,
  0.03086389233, 0.02952556994, 0.02805273044,
  0.02646057561, 0.02476538263, 0.02298429218,
  0.02113508696, 0.0192359639, 0.01730530352,
  0.01536143961, 0.01342243276, 0.01150585082,
  0.009628559511, 0.007806525986 };

const double KdpFirFilt::firCoeff_30[FIR_LEN_30+1] = {
  0.01040850049,0.0136551033,0.01701931136,0.0204494327,
  0.0238905658,0.02728575662,0.03057723021,0.03370766631,
  0.03662148602,0.03926611662,0.04159320123,0.04355972181,
  0.04512900539,0.04627158699,0.04696590613,0.04719881804,
  0.04696590613,0.04627158699,0.04512900539,0.04355972181,
  0.04159320123,0.03926611662,0.03662148602,0.03370766631,
  0.03057723021,0.02728575662,0.0238905658,0.0204494327,
  0.01701931136,0.0136551033,0.01040850049};

const double KdpFirFilt::firCoeff_20[FIR_LEN_20+1] = {
  0.016976991942, 0.023294989742, 0.030244475217,
  0.037550056394, 0.044888313214, 0.051908191403,
  0.058254532798, 0.063592862330, 0.067633391375,
  0.070152221980, 0.071007947209, 0.070152221980,
  0.067633391375, 0.063592862330, 0.058254532798,
  0.051908191403, 0.044888313214, 0.037550056394,
  0.030244475217, 0.023294989742, 0.016976991942
};

const double KdpFirFilt::firCoeff_10[FIR_LEN_10+1] = {
  0.03064579383,0.0603038422,0.09022859603,0.1159074511,
  0.1332367851,0.1393550634,0.1332367851,0.1159074511,
  0.09022859603,0.0603038422,0.03064579383 };

// Constructor

KdpFirFilt::KdpFirFilt()
  
{

  // FIR filter defaults to length 10

  setFIRFilterLen(FIR_LENGTH_10);
  
  _nFiltIterUnfolded = 2;
  _nFiltIterCond = 4;

  _nGatesPad = 21;
  setNGates(0);

  _useIterativeFiltering = false;
  _phidpDiffThreshold = 4.0;

  // feature len
  
  setFeatureLength(3.0, 0.25);

  // debugging
  
  _debug = false;

}

// Destructor

KdpFirFilt::~KdpFirFilt()
  
{

}

/////////////////////////////////////////////////////////////////////
// phidp feature length for filtering
// selects the FIR length

void KdpFirFilt::setFeatureLength(double featureLengthKm,
                                  double gateSpacingKm)
{

  _featureLengthKm = featureLengthKm;
  _gateSpacingKm = gateSpacingKm;
  _nGatesFeature = (int) (_featureLengthKm / _gateSpacingKm) + 1;

  if (_nGatesFeature < 20) {
    setFIRFilterLen(FIR_LENGTH_10);
  } else if (_nGatesFeature < 30) {
    setFIRFilterLen(FIR_LENGTH_20);
  } else if (_nGatesFeature < 40) {
    setFIRFilterLen(FIR_LENGTH_30);
  } else if (_nGatesFeature < 60) {
    setFIRFilterLen(FIR_LENGTH_40);
  } else if (_nGatesFeature < 125) {
    setFIRFilterLen(FIR_LENGTH_60);
  } else {
    setFIRFilterLen(FIR_LENGTH_125);
  }
  
}
  
/////////////////////////////////////
// set FIR filter length

void KdpFirFilt::setFIRFilterLen(fir_filter_len_t len)

{
  
  switch (len) {
    case FIR_LENGTH_125:
      _firLength = FIR_LEN_125 + 1;
      _firCoeff = firCoeff_125;
      break;
    case FIR_LENGTH_60:
      _firLength = FIR_LEN_60 + 1;
      _firCoeff = firCoeff_60;
      break;
    case FIR_LENGTH_40:
      _firLength = FIR_LEN_40 + 1;
      _firCoeff = firCoeff_40;
      break;
    case FIR_LENGTH_30:
      _firLength = FIR_LEN_30 + 1;
      _firCoeff = firCoeff_30;
      break;
    case FIR_LENGTH_20:
      _firLength = FIR_LEN_20 + 1;
      _firCoeff = firCoeff_20;
      break;
    case FIR_LENGTH_10:
    default:
      _firLength = FIR_LEN_10 + 1;
      _firCoeff = firCoeff_10;
  }

  _firLenHalf = _firLength / 2;

}

////////////////////////////////////////////////////////////////////////
// Initialize the object arrays for later use.
// Do this if you need access to the arrays, but have not yet called
// compute(), and do not plan to do so.
// For example, you may want to output missing fields that you have
// not computed, but the memory needs to be there.

void KdpFirFilt::initializeArrays(int nGates)

{
  setNGates(nGates);

  // allocate the arrays needed
  // copy input arrays, leaving extra space at the beginning
  // for negative indices and at the end for filtering as required

  _phidp_.resize(_nGates); _phidp = _phidp_.data();
  _phidpFilt_.resize(_nGates); _phidpFilt = _phidpFilt_.data();
  _phidpCond_.resize(_nGates); _phidpCond = _phidpCond_.data();
  _phidpCondFilt_.resize(_nGates); _phidpCondFilt = _phidpCondFilt_.data();
  
  // initialize computed arrays
  
  for (int ii = 0; ii < _nGates; ii++) {
    _phidpFilt[ii] = _missingValue;
    _phidpCond[ii] = _missingValue;
    _phidpCondFilt[ii] = _missingValue;
  }
  
}

/////////////////////////////////////////////
// filter the input PHIDP array

void KdpFirFilt::filterPhidp(const vector<double> &phidp)

{

  // apply FIR filter to unfolded phidp
  
  _applyIterativeFir(_phidpFilt, phidp.data(), _nFiltIterUnfolded);
  
  // compute conditioned phidp
  
  if (_useIterativeFiltering) {
    
    // use iterative filtering to remove phase shift on backscatter
    
    _applyIterativeFirCond(_phidpCondFilt, _phidpFilt, _nFiltIterCond);
    
  } else {
    
    // compute phidp conditioned to remove phase shift on backscatter
    
    // _computePhidpConditioned();
    
    // apply the FIR filter to the conditioned phidp

    _applyIterativeFir(_phidpCondFilt, _phidpCond, _nFiltIterCond);
    
  }

}

/////////////////////////////////////////////////
// apply an FIR filter, iteratively

void KdpFirFilt::_applyIterativeFir(double *out,
                                    const double *in,
                                    int nIterations)

{

  // compute required array sizes, given that we need to
  // have space for the FIR filter on each side
  
  int arrayOffset = _firLength + 1;
  int arrayLen = _nGates + 2 * arrayOffset;
  
  // allocate working arrays
  
  vector<double> work1_(arrayLen);
  double *work1 = work1_.data() + arrayOffset;
  
  vector<double> work2_(arrayLen);
  double *work2 = work2_.data() + arrayOffset;
  
  // initialize working array work2
  
  _copyArray(work2, in);
  _padArray(work2);
  
  // apply FIR filter, computing work1 from work2, iterate
    
  for (int iloop = 0; iloop < nIterations; iloop++) {
    _applyFirFilter(work1, work2);
    _copyArray(work2, work1);
  } // iloop
  
  // save result

  _copyArray(out, work2);

}

///////////////////////////////////////////////////////////////////
// apply an FIR filter, iteratively
// condionally check each iteration against the original
// keep the original if the diff is below the conditional threshold

void KdpFirFilt::_applyIterativeFirCond(double *out,
                                        const double *in,
                                        int nIterations)

{

  // compute required array sizes, given that we need to
  // have space for the FIR filter on each side
  
  int arrayOffset = _firLength + 1;
  int arrayLen = _nGates + 2 * arrayOffset;
  
  // allocate working arrays
  
  vector<double> work1_(arrayLen);
  double *work1 = work1_.data() + arrayOffset;
  
  vector<double> work2_(arrayLen);
  double *work2 = work2_.data() + arrayOffset;
  
  // initialize working array work2
  
  _copyArray(work2, in);
  _padArray(work2);
  
  // apply FIR filter, computing work1 from work2, iterate
    
  for (int iloop = 0; iloop < nIterations; iloop++) {
    _applyFirFilter(work1, work2);
    _copyArrayCond(work2, work1, in);
  } // iloop
  
  // save result

  _copyArray(out, work2);

}

/////////////////////////////////////////////
// load array ready for filter

void KdpFirFilt::_copyArray(double *out, const double *in)

{
  memcpy(out, in, _nGates * sizeof(double));
}

/////////////////////////////////////////////
// copy array conditionally

void KdpFirFilt::_copyArrayCond(double *out, const double *in,
                             const double *original)

{
  for (int ii = 0; ii < _nGates; ii++) {
    double diff = in[ii] - out[ii];
    if (fabs(diff) < _phidpDiffThreshold) {
      out[ii] = original[ii];
    } else {
      out[ii] = in[ii];
    }
  }
}

/////////////////////////////////////////////
// Pad array ready for filter

void KdpFirFilt::_padArray(double *array)

{
  for (int ii = -_firLength; ii < 0; ii++) {
    array[ii] = array[0];
  }
  for (int ii = _nGates; ii < _nGates + _firLength; ii++) {
    array[ii] = array[_nGates - 1];
  }
}

/////////////////////////////////////////////
// Apply FIR filter

void KdpFirFilt::_applyFirFilter(double *out, const double *in)

{

  for (int ii = -_firLenHalf; ii < _nGates + _firLenHalf; ii++) {
    double acc = 0.0;
    int kk = ii - _firLenHalf;
    for (int jj = 0; jj < _firLength; jj++, kk++) {
      acc = acc + _firCoeff[jj] * in[kk];
    }
    out[ii] = acc;
  } // ii

}
    
/////////////////////////////////////////////
// Get FIR filter gain

double KdpFirFilt::_getFirFilterGain()
  
{
  double sum = 0.0;
  for (int jj = 0; jj < _firLength; jj++) {
    sum += _firCoeff[jj];
  }
  return sum;
}
    
