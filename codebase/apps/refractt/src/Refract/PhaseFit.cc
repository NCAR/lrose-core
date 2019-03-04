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
/**
 *
 * @file PhaseFit.cc
 *
 * @class PhaseFit
 *
 * Class for performing the phase fitting algorithm.
 *  
 * @date 3/29/2010
 *
 */

#include <stdlib.h>
#include <string.h>

#include "PhaseFit.hh"
#include "TargetVector.hh"
#include "LinearInterpArgs.hh"
#include <Refract/VectorData.hh>
#include <Refract/FieldDataPair.hh>
#include <Refract/RefractInput.hh>
#include <Refract/RefractConstants.hh>
#include <rapmath/math_macros.h>
#include <toolsa/LogStream.hh>


// Global variables

const int PhaseFit::SMEAR_AZ = 10;
const int PhaseFit::SMEAR_AZ_INIT = 30;
const int PhaseFit::SMEAR_RA = 2;
const int PhaseFit::INITIAL_SLOPE_LEN_M = 4000;
const int PhaseFit::MIN_ITER = 3;
const int PhaseFit::MAX_ITER = 1000;
const double PhaseFit::MIN_ABS_CONSISTENCY = 4.0;

/*********************************************************************
 * Constructor
 */

PhaseFit::PhaseFit() :
  _nOutput(0),
  _nError(0)
{
}


//----------------------------------------------------------------------------
/*********************************************************************
 * Destructor
 */

PhaseFit::~PhaseFit()
{
}


//----------------------------------------------------------------------------
/*********************************************************************
 * init()
 */

bool PhaseFit::init(const int num_beams, const int num_gates,
		    const double gate_spacing, const double wavelength,
		    const double min_consistency, const int r_min)
{
  // Save the radar and phase fitting information
  _numBeams = num_beams;
  _numGates = num_gates;
  _gateSpacing = gate_spacing;
  _wavelength = wavelength;
  _minConsistency = min_consistency;
  _rMin = r_min;
  
  // Allocate space for the internal arrays
  _scanSize = _numBeams * _numGates;
  int scan_size = _scanSize;
  
  _phase = VectorData(scan_size, 0.0);
  _quality = VectorData(scan_size, 0.0);
  _iq = VectorIQ(scan_size, 0.0);
  _phaseFit = VectorData(scan_size, 0.0);
  _phaseError = VectorData(scan_size, 0.0);
  _smoothIQ = VectorIQ(scan_size, 0.0);

  // Calculate other needed values
  _azimSpacing = 360.0 / (double)_numBeams;
  
  return true;
}

/*********************************************************************/
void PhaseFit::initFit(const TargetVector &target,
		       const FieldDataPair &difData, bool isPhaseDiff)
{      
  // copy from target phasediff or phase into _phase local array
  if (isPhaseDiff)
  {
    target.copyPhaseDiff(_phase);
  }
  else
  {
    target.copyPhase(_phase);
  }

  // set iq based on difData
  _iq.setAllZero();
  _iq.copyLargeR(_rMin, _numGates, _numBeams, difData);

  // set quality and iq based on difData or target iq
  _quality.setAllZero();

  if (isPhaseDiff)
  {
    _quality.setInitialQuality(_rMin, target.getPhaseDiffErr(), 
			       difData, _numBeams, _numGates);
  }
  else
  {
    _quality.setInitialQuality(_rMin, target.getPhaseDiffErr(), 
			       target.getIQ(), _numBeams, _numGates);
  }
			     
// #ifdef NOT
//   for (int az=0; az < _numBeams; ++az)
//   {
//     for (int r = 0 ; r < _numGates ; ++r, ++offset)
//     {
//       if (r >= _rMin)
//       {
// 	if (target.phase_dif_er_is_valid(offset))
// 	// if (target[offset].phase_diff_er != refract::INVALID)
// 	{
// 	  if (isPhaseDiff)
// 	  {
// 	    _quality[offset] = difData[offset].norm();
// 	  }
// 	  else
// 	  {
// 	    _quality[offset] = target.iqNorm(offset); //target[offset].iq.norm();
// 	  }
// 	}
// 	// _iq[offset] = difData[offset];
//       }
//     } /* endfor - r */
//   } /* endfor - az */
// #endif
}

//----------------------------------------------------------------------------
/*********************************************************************
 * fitPhaseField()
 */

bool PhaseFit::fitPhaseField(const bool do_relax)
{
  // Initialize

  float slope = _meanPhaseSlopeInit();
  if (slope == refract::INVALID)
    return false;
  
  // diff between average phase angle at _rMin and what the slope indicates
  // (linear values, I guess asuming phase at 0 is 0)
  _expectedPhaseRange0 = _phaseRange0() - _rMin*slope;

  // Get the smoothed phase field
  _rangeSlope = _doSmoothing(slope);
  if (_rangeSlope == refract::INVALID)
  {
    return false;
  }
  
  LOG(DEBUG_VERBOSE) << "Smoothing complete!";

  // Compute the resulting refractivity field and its error
  float slope_to_n = _defaultSlope()/DEG_TO_RAD;
  // float slope_to_n = _defaultSlope()*_numGates/_numGates/DEG_TO_RAD;
  // float slope_to_n = 1000000.0/_gateSpacing*_wavelength/720.0*_numGates/_numGates/DEG_TO_RAD;
  // float tmp = _rangeSlope*1000000.0/_gateSpacing*_wavelength/720.0 + _refN;
  float tmp = _rangeSlope*_defaultSlope() + _refN;
  // float er_decorrel = 2.0*_gateSpacing*_numGates/_numGates/_smoothSideLen;
  float er_decorrel = 2.0*_gateSpacing*_smoothSideLen;
  if (er_decorrel > 1.0)
    er_decorrel = 1.0;

  LOG(DEBUG) << "---> Initializing output data";
  LOG(DEBUG) << "      num az = " << _numBeams;
  LOG(DEBUG) << "      num range = " <<  _numGates;
  
  for (int azn = 0; azn < _numBeams; ++azn)
  {
    _setNOneBeam(azn, slope_to_n, tmp, er_decorrel);
  } /* endfor - azn */

  LOG(DEBUG_VERBOSE) << "N and N-error derived from smoothed field.";

  // Improve estimates of refractivity through a relaxation process
  if (do_relax)
  {
    _relax(_nOutput);
    LOG(DEBUG_VERBOSE) << "Relaxation complete.";
  }

  return true;
}

//----------------------------------------------------------------------------
void PhaseFit::_setNOneBeam(int azn, float slope_to_n, float tmp,
			    float er_decorrel)
{
  int offsetn = azn*_numGates;
  _nOutput[offsetn] = tmp;
  _nError[offsetn] = refract::VERY_LARGE;

  int r;
  for (r = 1; r < _numGates - 1; ++r)
  {
    int ri = offsetn + r;
    _nOutput[ri] = _smoothIQ.refractivity(ri, slope_to_n) + _refN;
    _nError[ri] =
      er_decorrel*sqrt(_phaseError.sumSquares(3, ri)/6.0)*slope_to_n*DEG_TO_RAD;
      // er_decorrel * sqrt((RefractUtil::SQR(_phaseError[ri+1]) +
      // 			  RefractUtil::SQR(_phaseError[ri]) +
      // 			  RefractUtil::SQR(_phaseError[ri-1])) / 6.0) *
      // slope_to_n * DEG_TO_RAD ;
  } /* endfor - rn */

  int ri = offsetn + _numGates -1;
  _nOutput[ri] = tmp;
  _nError[ri] = refract::VERY_LARGE;
}

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

//----------------------------------------------------------------------------
/*********************************************************************
 * _doSmoothing()
 *
 * This method smooths the phase measurements of a given map.  This is one
 * of the ugliest routines, primarily because of the noisy character of
 * the phase data and because of the steps that were taken to minimize
 * execution time of this very slow routine (look-up tables, pre-prepared
 * arrays...).  As a result, the smoothing is done very carefully and very
 * gradually:
 *   1) Estimate the average d(Phase)/d(range) of the phase data to smooth;
 *   2) Then, going out in range, the routine generates a smooth dealiased
 *      "synthetic" phase field (stored in _phaseFit) that follows the real
 *      field within the constraints of the real field data quality and of
 *      the smoothing area.  In detail, as range increases, it:
 *	a) Computes the smoothing area in polar coordinates;
 *	b) Do an azimuth-by-azimuth smoothing of phases by trying to remove
 *	   the effect of the sharp slopes of phase in range;
 *	c) Combine some of the azimuths together to complete the smoothing;
 *	d) Evaluates the quality of the smoothed phase value obtained.
 *   3) In data gap regions, make your best guess of what the phase should be.
 *   Note that although an effort is made to dealias the phase field, no use
 *   will be made of it because it proved to be unreliable.
 *
 *   Important variables:
 *   - slope_a/b[]: Table of sin/cos components of slope correction with
 *                  slope;
 *   - slope_in_range[]: Slope of d(Phase)/d(range) for each azimuth;
 *   - smooth_i/q[]: I/Q component of smoothed phase;
 *   - smooth_range: Number of gates in range used for smoothing;
 *   - sum_inphase[]: Used as I component of phase data smoothed for
 *                    _phaseFit[];
 *   - sum_quadra[]: Used as Q component of phase data smoothed for
 *                   _phaseFit[];
 *   - _phaseFit[]: Smoothed dealiased phase data;
 */

float PhaseFit::_doSmoothing(double phaseSlopeInit)
{
  LOG(DEBUG_VERBOSE) << "    Entered _doSmoothing()";
  if (phaseSlopeInit == refract::INVALID)
  {
    return refract::INVALID;
  }
  
  // Estimate the initial slope (slope near range = 0) and average one
  // (throughout the whole field).  These will act as anchors to prevent
  // the algorithm from running away from the reasonable.
  // float init_slope = _meanPhaseSlopeInit();   // this is done previously
  _rangeSlope = phaseSlopeInit; //init_slope;
  float range_slope = _meanPhaseSlopeAvg();
  if (range_slope == refract::INVALID)
  {
    return refract::INVALID;
  }
  LOG(DEBUG) << "    range_slope = " << (range_slope * 1000.0 / _gateSpacing)
	     << " deg/km; init_slope = "
	     << (phaseSlopeInit * 1000 / _gateSpacing) << " deg/km";

  // Compute smoothed field where enough data is available.
  // Allocate memory and prepare some tables
  VectorData slope_in_range(_numBeams, phaseSlopeInit);
  VectorData next_slope_in_range(_numBeams, phaseSlopeInit);

  int numAb = _numBeams*(2*_smoothRange + 1);
  VectorIQ slope_ab(numAb, 0.0);

  _phaseFit.setAllToValue(refract::INVALID);
  _phaseError.setAllToValue(refract::VERY_LARGE);
  // for (int index = 0; index < _numGates * _numBeams; ++index)
  // {
  //   outOfBounds(index);
  //   _phaseFit[index] = refract::INVALID;
  //   _phaseError[index] = refract::VERY_LARGE;
  // }

  // slope_ab is the slope along each azimuth (angle) at range indices
  // 0 to twoSmoothRange+1, which is the smoothing box length
  // slope_ab.setSlopes(_numBeams, _smoothRange);
  slope_ab.setSlopes(_numBeams, _smoothRange);

  // for (int angle = 0, index = 0; angle < _numBeams; angle++)
  // {
  //   for (int dr = 0; dr <= _twoSmoothRange; dr++, index++)
  //   {
  //     outOfBounds(numAb, index);
  //     slope_ab[index] = IQ((_smoothRange-dr)*angle);
  //   } /* endfor - dr */
  // } /* endfor - angle, index */
  
  // For each range, we determine the smoothing area in range-azim system 
  for (int r = 0; r < _numGates; ++r)
  {
    _doSmoothingRange(r, slope_in_range, next_slope_in_range, slope_ab,
		      range_slope, phaseSlopeInit);
    slope_in_range = next_slope_in_range;
  } /* endfor - r */
  
  // Task completed!
  return _rangeSlope;
}

//----------------------------------------------------------------------------
void PhaseFit::_doSmoothingRange(int r, const VectorData &slope_in_range,
				 VectorData &next_slope_in_range,
				 VectorIQ &slope_ab,
				 float range_slope, float init_slope)
{
  LinearInterpArgs l_args(r, _smoothSideLen, _azimSpacing, _gateSpacing,
  			  _numBeams,_twoSmoothRange, _minConsistency,
   			  MIN_ABS_CONSISTENCY, range_slope, init_slope);
  
  // int num_sum_iq = _numBeams/4;
  int smooth_azim = (int)(_smoothSideLen*360.0/
  			  (_azimSpacing*r*_gateSpacing*4.0*M_PI));
  if (smooth_azim >= _numBeams/8) smooth_azim = _numBeams/8 - 1;
  if (smooth_azim <= 0) smooth_azim = 1;
  int two_smooth_azim = 2*smooth_azim;
  float minconsistency = (_twoSmoothRange + 1)*(two_smooth_azim + 1)*
    _minConsistency;
  if (minconsistency < MIN_ABS_CONSISTENCY)
    minconsistency = MIN_ABS_CONSISTENCY;

  VectorIQ sum_iq(_numBeams/4, 0.0);
  VectorData max_quality(_numBeams/4, 0.0);
  VectorData guess_phase(_numBeams, 0.0);

  // The smoothed value of phase(r+dr) is precomputed for the last azimuth
  // (done to speed up computation afterwards when azimuths are shifted).
  // The weighting function of I/Q data follows a (1-a(r-r0)^2) like function

  // int az2 = _numBeams - 1;

  sum_iq.setRangeZero(0, l_args._two_smooth_azim);
  max_quality.setRangeZero(0, l_args._two_smooth_azim);

  // for (int daz = 0; daz <= l_args._two_smooth_azim; daz++)
  // {
  //   if (daz >= _scanSize)
  //   {
  //     printf("Out of bounds daz\n");
  //   }
  //   outOfBounds(num_sum_iq, daz);
  //   sum_iq[daz].set(0.0, 0.0);
  //   max_quality[daz] = 0.0;
  // } /* endfor - daz */

  for (int dr = 0; dr <= _twoSmoothRange; dr++)
  {
    float weight_fact =
      sqrt(2.0)*(1.0 - RefractUtil::SQR((_smoothRange-dr)/
					((float)_smoothRange + 0.5)));
    if ((r + dr - _smoothRange >= _rMin) &&
	(r + dr - _smoothRange < _numGates))
    {
      for (int daz = -l_args._smooth_azim; daz <= l_args._smooth_azim; daz++)
      {
	//int az = (_numBeams - 1 + daz + _numBeams) % _numBeams;
	int az = (_numBeams - 1 + daz) % _numBeams;
	int offset = az * _numGates + r + dr - _smoothRange;
	int k = slope_in_range.slopeFloor(az)*(2*_smoothRange + 1);
	IQ tmp = _iq[offset].phaseDiffFitC(slope_ab[k+dr]);
	tmp *= weight_fact;
	sum_iq[daz+l_args._smooth_azim] += tmp;
	outOfBounds(offset);
	max_quality[daz+l_args._smooth_azim] += weight_fact*_quality[offset];
      } /* endfor - daz */
    }
      
  } /* endfor - dr */

    // For each azimuth, we guess what the next phase(range) should be based on
    // the previously computed d(Phase)/d(range) at that azimuth.  This step will
    // be needed for optimum smoothing.

  int rjump = 0;
  for (int az = 0; az < _numBeams; az++)
  {
    guess_phase[az] = _guessPhase(az, r, slope_in_range[az], rjump);
  }

    // Additional step: If num_azim > output_numa, interpolate linearly
    // guess_phase[az] entries between az rays instead of interpolating to
    // nearest.  (We now require num_azim == output_numa, so output_numa
    // has gone away.  However, I don't see this check in this code so don't
    // know what I can get rid of.  NR)

    // Then compute the true smoothed I/Q values by updating
    // the precomputed sum_I/Q at r+dr and combining them in azimuth and
    // range taking into account the slope d(Phase)/d(range) and the shape
    // of the weighting function.

  int oldaz = _numBeams - 1;
  for (int azn = 0; azn < _numBeams; azn++)
  {
    int azjump = (azn - oldaz + _numBeams) % _numBeams;
    l_args.initForAz(azn, azjump, rjump);
    _linearInterp(l_args,
		  slope_in_range, slope_ab, guess_phase, sum_iq, max_quality,
		  next_slope_in_range);
    oldaz = azn;
  } /* endfor - azn */

}

//----------------------------------------------------------------------------
void PhaseFit::_linearInterp(const LinearInterpArgs &args,
			     const VectorData &slope_in_range,
			     const VectorIQ &slope_ab,
			     const VectorData &guess_phase,
			     VectorIQ &sum_iq, VectorData &max_quality,
			     VectorData &next_slope_in_range)
{
  int off2 = args._azn*_numGates + args._r;
  float weight_fact = (float)((_twoSmoothRange+1)*(args._two_smooth_azim+1));

  for (int j = 0; j < args._azjump; j++)
  {
    sum_iq.shiftDown(0, args._two_smooth_azim);
    max_quality.shiftDown(0, args._two_smooth_azim);
    sum_iq[args._two_smooth_azim] = _linearInterpUpdateSumIQ(args, j,
							     slope_in_range,
							     slope_ab);
    max_quality[args._two_smooth_azim] = _linearInterpUpdateMaxQual(args, j);
	
  } /* endfor - j */
      
  // Combine all azimuths of smoothing area, correcting for mean phase
  // of row

  IQ tmp_ab(0.0, 0.0);
  float maxconsistency = 0.0;
  for (int daz = 0; daz <= args._two_smooth_azim; daz++)
  {
    _linearInterpIncConsistency(daz, args, sum_iq, slope_ab, guess_phase,
				max_quality, tmp_ab, maxconsistency);
  }

  // Save results.  Update next_slope_in_range.
  outOfBounds(off2);
  _smoothIQ[off2] = tmp_ab;
  _smoothIQ[off2] /= weight_fact;

  float consistency, quality;
  _setConsistencyAndQuality(tmp_ab, weight_fact, maxconsistency, consistency,
			    quality);
  if (consistency > args._minconsistency)
  {
    _setPhaseAndNextSlopeInRange(args, tmp_ab, guess_phase, quality,
				 consistency, maxconsistency,
				 slope_in_range, next_slope_in_range);
  }
  else
  {
    _resetSmoothIQAndNextSlopeInRange(args, guess_phase, slope_in_range,
				      next_slope_in_range);
  }

  // If quality is really poor, slope might be wrong; compute it the
  // raw way

  if (quality < sqrt(2.0 / weight_fact))
  {
    _lowQualityAdjust(args, next_slope_in_range);

  }

  if (args._r < INITIAL_SLOPE_LEN_M / _gateSpacing)
  {
    next_slope_in_range[args._azn] +=
      0.1 * (1.0 - args._r / (INITIAL_SLOPE_LEN_M / _gateSpacing)) *
      (args._init_slope - next_slope_in_range[args._azn]);
  }
      
  if (next_slope_in_range[args._azn] > args._range_slope + 60.0)    // Occasionally-needed railing
    next_slope_in_range[args._azn] = args._range_slope + 60.0;
  if (next_slope_in_range[args._azn] < args._range_slope - 60.0)
    next_slope_in_range[args._azn] = args._range_slope - 60.0;
}

//----------------------------------------------------------------------------
double PhaseFit::_linearInterpUpdateMaxQual(const LinearInterpArgs &args,
					    int j) const
{
  double ret = 0.0;
  for (int dr = 0; dr <= _twoSmoothRange; dr++)
  {
    int r_i = args._r + dr - _smoothRange;
    if (r_i >= _rMin && r_i < _numGates)
    {
      float weight_fact = sqrt(2.0) *
	(1.0 - RefractUtil::SQR((_smoothRange-dr) /
				((float)_smoothRange + 0.5)));

      int offset = args.offsetAzimuthIndex(j)*_numGates + r_i;
      if (offset >= _numBeams*_numGates)
	offset -= _numBeams*_numGates;
      ret += weight_fact*_quality[offset];
    }
  }
  return ret;
}

//----------------------------------------------------------------------------
IQ PhaseFit::_linearInterpUpdateSumIQ(const LinearInterpArgs &args, int j,
				      const VectorData &slope_in_range,
				      const VectorIQ &slope_ab) const
{
  IQ ret(0.0, 0.0);
  int az = args.offsetAzimuthIndex(j) % _numBeams;
  int k = slope_in_range.slopeFloor(az)*(_twoSmoothRange + 1);
  for (int dr = 0; dr <= _twoSmoothRange; dr++)
  {
    int r_i = args._r + dr - _smoothRange;
    if (r_i >= _rMin && r_i < _numGates)
    {
      float weight_fact = sqrt(2.0) *
	(1.0 - RefractUtil::SQR((_smoothRange-dr) /
				((float)_smoothRange + 0.5)));

      int offset = args.offsetAzimuthIndex(j)*_numGates + r_i;
      if (offset >= _numBeams*_numGates)
	offset -= _numBeams*_numGates;

      IQ tmp = _iq[offset].phaseDiffFitC(slope_ab[k+dr]);
      tmp *= weight_fact;
      ret += tmp;
    }
  }
  return ret;
}

//----------------------------------------------------------------------------
void PhaseFit::_lowQualityAdjust(const LinearInterpArgs &args,
				 VectorData &next_slope_in_range)
{    
  IQ rawslope_ab(0.0, 0.0);
  IQ tmp_ab(0.0, 0.0);
  for (int dr = -_twoSmoothRange; dr <= _twoSmoothRange; dr++)
  {
    if ((args._r+dr >= _rMin) &&
	(args._r+dr < _numGates))
    {
      IQ old_tmp_ab = tmp_ab;
      tmp_ab.set(0.0, 0.0);
      for (int daz = -args._two_smooth_azim; daz <= args._two_smooth_azim; daz++)
      {
	int az =(args._azn + daz + _numBeams) % _numBeams;
	int offset = az * _numGates + args._r + dr;
	outOfBounds(offset);
	tmp_ab += _iq[offset];
      } /* endfor - daz */
	    
      rawslope_ab += old_tmp_ab.phaseDiffC(tmp_ab);
    }
  } /* endfor - dr */
	
  float consistency = rawslope_ab.norm();
  if (consistency > _minConsistency*
      RefractUtil::SQR(2*args._two_smooth_azim + 1)*
      (2 * _twoSmoothRange + 1))
  {
    float xxx = rawslope_ab.phase();
    if (fabs(xxx - args._range_slope) < 60.0)
    {
      next_slope_in_range[args._azn] += 0.2*(xxx - next_slope_in_range[args._azn]);
    }
  }
}

//----------------------------------------------------------------------------
void PhaseFit::_linearInterpIncConsistency(int daz,
					   const LinearInterpArgs &args,
					   const VectorIQ &sum_iq,
					   const VectorIQ &slope_ab,
					   const VectorData &guess_phase,
					   const VectorData &max_quality,
					   IQ &tmp_ab,
					   float &maxconsistency) const
{
  // int az = (args._azn + daz - args._smooth_azim + _numBeams) % _numBeams;
  int off2 = args._azn*_numGates + args._r;
  int az = (args.offsetAzimuthIndexNeg(daz) + _numBeams) % _numBeams;
  int off1 = az*_numGates + args._r;
  int num_sum_iq = sum_iq.num();
  outOfBounds(num_sum_iq, daz);
  IQ cor_iq = sum_iq[daz];
  if (args._r > 0)
  {
    if (_phaseFit[off1 - 1] != refract::INVALID && 
	_phaseFit[off2 - 1] != refract::INVALID)
    {
      int k = (((int)(floor(guess_phase[args._azn] - guess_phase[az] + 0.5)) + 
		360000) % 360);
      if (k < 180)    // Dampen phase correction, otherwise it misbehaves
	k = k / 2;
      else
	k = k + (360 - k) / 2;
      k = k * (_twoSmoothRange + 1) + _smoothRange - 1;
      cor_iq = sum_iq[daz].phaseDiffFitC(slope_ab[k]);
    }
  }
  float weight_fact =
    sqrt(2.0) * (1.0 - RefractUtil::SQR(args._smooth_azim-daz) /
		 RefractUtil::SQR(args._smooth_azim + 0.5));
  IQ x = cor_iq;
  x *= weight_fact;
  tmp_ab += x;
  maxconsistency += weight_fact * max_quality[daz];
}

//----------------------------------------------------------------------------
/*********************************************************************
 * _meanPhaseSlope()
 */

double PhaseFit::_meanPhaseSlope(const int max_r, const int smear_az) const
{
  IQ slope_iq(0.0, 0.0);

  // process over sectors smear_az beams wide,
  // incrementing slope_iq once for each sector
  for (int az = 0; az < _numBeams; az += smear_az)
  {
    slope_iq += _sectorMeanPhaseSlope(az, max_r, smear_az);
  }
  if (slope_iq.isZero())
  {
    LOG(ERROR) << "Cannot calculate mean phase slope I and Q values are all 0";
    return refract::INVALID;
  }
  return slope_iq.phase()/SMEAR_RA;
}

//----------------------------------------------------------------------------
IQ PhaseFit::_sectorMeanPhaseSlope(int az, int max_r, int smear_az) const
{
  IQ ret(0.0, 0.0);
  IQ tmp_ab(0.0, 0.0);
  // process SMEAR_RA (=2) ranges at a time out to max
  // _rMin = 4 in current settings, max_r is 30, so from 4 to 30, 2 at a time
  // compare results from one range with previous doing a phase diff
  // for example r=10, first set old_tmp_ab to mean from r=8 and 9, then
  // compute for 10 and 11  a new slope, and increment result by phase diff
  // between the two
  for (int r = _rMin; r < max_r; r += SMEAR_RA) 
  {
    IQ old_tmp_ab = tmp_ab;
    tmp_ab = _meanPhaseSlopeAtRange(r, az, smear_az);
    ret += old_tmp_ab.phaseDiffC(tmp_ab);
  }
  return ret;
}

//----------------------------------------------------------------------------
IQ PhaseFit::_meanPhaseSlopeAtRange(int r, int az, int smear_az) const
{
  // take the average of a ring of width 2 gates over the sector with
  // starting point az
  bool debug = false;
  IQ tmp_ab(0.0, 0.0);

  // for the entire sector
  for (int j = 0; j < smear_az; ++j)
  {
    // for the SMEAR_RA ranges (2)
    for (int k = 1; k <= SMEAR_RA; ++k)
    {
      // accumulate _iq by summing
      int bin_index = (az+j)*_numGates + r + k;
      if (bin_index > 0 && bin_index < _scanSize)
      {
	// it was going out of bounds like crazy
	tmp_ab += _iq[bin_index];
      }
      else
      {
	if (debug) printf("bin index %d out of bounds\n", bin_index);
      }
    } /* endfor - k */
  } /* endfor - j */
  return tmp_ab;
}

//----------------------------------------------------------------------------
void
PhaseFit::_resetSmoothIQAndNextSlopeInRange(const LinearInterpArgs &args,
					    const VectorData &guess_phase,
					    const VectorData &slope_in_range,
					    VectorData &next_slope_in_range)
{
  int off2 = args._azn*_numGates + args._r;
  double minConsist = args._minconsistency;
  int ai = args._azn;
  
  int nextBeamAi = (ai + _numBeams -1)%_numBeams;
  int nextAi = (ai+1)%_numBeams;

  next_slope_in_range[ai] += _gateSpacing / _smoothSideLen *
    (slope_in_range[nextBeamAi]
     + slope_in_range[nextAi] +
     0.25 * args._range_slope - 2.25 * slope_in_range[ai]);

  outOfBounds(off2);
  _smoothIQ[off2].set(
		      0.1*minConsist*cos(guess_phase[ai]*DEG_TO_RAD),
		      0.1*minConsist*sin(guess_phase[ai]*DEG_TO_RAD));
}
 
//----------------------------------------------------------------------------
void PhaseFit::_setPhaseAndNextSlopeInRange(const LinearInterpArgs &args,
					    const IQ &tmp_ab,
					    const VectorData &guess_phase,
					    float quality, float consistency,
					    float maxconsistency,
					    const VectorData &slope_in_range,
					    VectorData &next_slope_in_range)
{    
  int off2 = args._azn*_numGates + args._r;
  float tmp_phase = tmp_ab.phase();
  while (tmp_phase - guess_phase[args._azn] < -180.0)
    tmp_phase += 360.0;
  while (tmp_phase - guess_phase[args._azn] >= 180.0)
    tmp_phase -= 360.0;

  outOfBounds(off2);
  _phaseFit[off2] = tmp_phase;
  _phaseError[off2] =
    sqrt(-2.0 * log(quality) / quality) / DEG_TO_RAD /
    sqrt(maxconsistency / 2.0);
  if (consistency > 4.0 * args._minconsistency)
  {
    next_slope_in_range[args._azn] +=
      2.0 * _gateSpacing / _smoothSideLen /
      args._rjump * (tmp_phase - guess_phase[args._azn]);
  }
  else
  {
    next_slope_in_range[args._azn] +=
      0.5 * consistency / args._minconsistency *
      _gateSpacing / _smoothSideLen / args._rjump *
      (tmp_phase - guess_phase[args._azn]);

    int prev_az = (args._azn + _numBeams - 1) % _numBeams;
    int next_az = (args._azn + 1) % _numBeams;
	  
    next_slope_in_range[args._azn] +=
      (1.0 - 0.25 * consistency / args._minconsistency) *
      _gateSpacing / _smoothSideLen * (slope_in_range[prev_az]
       + slope_in_range[next_az] +
       0.25 * args._range_slope - 2.25 * slope_in_range[args._azn]);
  }
}

//----------------------------------------------------------------------------
/*********************************************************************
 * _meanPhaseSlopeAvg()
 */

double PhaseFit::_meanPhaseSlopeAvg() const
{
  int max_r = _numGates - SMEAR_RA;
  int smear_az = (int)(SMEAR_AZ * _numBeams / 360.0);
  return _meanPhaseSlope(max_r, smear_az);
}

//----------------------------------------------------------------------------
/*********************************************************************
 * _meanPhaseSlopeInit()
 */
double PhaseFit::_meanPhaseSlopeInit() const
{
  int max_r = _rMin + (int)(INITIAL_SLOPE_LEN_M / _gateSpacing);
  int smear_az = (int)(SMEAR_AZ_INIT * _numBeams / 360.0);
  return _meanPhaseSlope(max_r, smear_az);
}

//----------------------------------------------------------------------------
/*********************************************************************
 * _phaseRange0()
 */

double PhaseFit::_phaseRange0()
{
  IQ tmp_ab(0.0, 0.0);
  for (int az = 0; az < _numBeams; ++az)
  {
    int offset = az * _numGates + _rMin;
    tmp_ab += _iq[offset];
    outOfBounds(offset);
  }

  return tmp_ab.phase();
}

//----------------------------------------------------------------------------
float PhaseFit::_guessPhase(int az, int r, float slope_in_range,
			    int &rjump) const
{
  rjump = 1;
  int off2 = az*_numGates + r;
  if (r <= _rMin)
  {
    return _expectedPhaseRange0;
  }

  outOfBounds(off2-rjump);
  while (_phaseFit[off2-rjump] == refract::INVALID && rjump < r)
  {
    rjump++;
  }
  
  if (r - rjump >= _rMin)
  {
    return _phaseFit[off2-rjump] + rjump*slope_in_range;
  }
  else
  {
    rjump = r - _rMin;
    return _expectedPhaseRange0 + rjump*slope_in_range;
  }
}

//----------------------------------------------------------------------------
/*********************************************************************
 * _relax()
 *
 * This method performs a quality-dependent iterative smoothing of the
 * refractivity field, constrained by the smoothed phase field.  It uses
 * an iterative "diffusion and relaxation" approach to the problem, even if
 * it takes an eternity: first the values of N are nudged to match better
 * the phase data, then the field is smoothed.  As a result, the regions
 * where data are available gradually percolate into regions where they
 * are not.
 */

int PhaseFit::_relax(float *ndata)
{
  // Initialize variables and arrays

  LOG(DEBUG) << "Now starting relaxation";

  int nsmooth = 21*_numGates;

  float *smooth = new float[21 * _numGates];
  memset(smooth, 0, 21 * _numGates * sizeof(float));
  
  float *work_array = new float[_numBeams * _numGates];
  int *st_d_az = new int[_numGates];

  //float slope_to_n = 1000000.0 / _gateSpacing * _wavelength / 720.0;
  float slope_to_n = _defaultSlope();
  float force_factor = RefractUtil::SQR(_gateSpacing/_smoothSideLen);	/* Sets how field follows data (>0); small -> loose */
  int num_iterat = (int)(0.5*RefractUtil::SQR(0.5*_smoothSideLen/_gateSpacing));
  if (num_iterat < MIN_ITER)
    num_iterat = MIN_ITER;
  if (num_iterat > MAX_ITER)
    num_iterat = MAX_ITER;

  // Build the range-dependant smoothing function used during the diffusion part 
  // of the relaxation process.  This is made to have a relaxation that is more
  // or less similar in the r and phi direction despite the polar nature of the data.

  for (int r = 0; r < _numGates; ++r)
  {
    // Calculate the distance between beams at this gate.  The units of the
    // distance are number of gates.

    float equiv_dist = (float)(2 * r) * M_PI / (float)_numBeams;
    float tmp;
    
    if (equiv_dist < 1.0)
    {
      // In this case, the gates are closer together around the azimuths
      // than along the beam so we are closer to the radar

      tmp = -1.0;
      for (int d_az = 10; d_az <= 20; ++d_az)
      {
	outOfBounds(nsmooth, 21*r+d_az);
	outOfBounds(nsmooth, 21*r + 20 - d_az);
	if (((d_az + 0.5) - 10.0) * equiv_dist < 1.5)
	{
	  smooth[21 * r + d_az] = 1.0;
	  smooth[21 * r + 20 - d_az] = 1.0;
	  tmp += 2.0;
	  outOfBounds(_numGates, r);
	  st_d_az[r] = 20 - d_az;
	}
	else if (((d_az - 0.5) - 10.0) * equiv_dist < 1.5)
	{
	  smooth[21 * r + d_az] =
	    (1.5 - ((d_az - 0.5) - 10.0) * equiv_dist) / equiv_dist;
	  smooth[21 * r + 20 - d_az] = smooth[21 * r + d_az];
	  tmp += 2.0 * smooth[21 * r + d_az];
	  st_d_az[r] = 20 - d_az;
	}
      }
    }
    else
    {
      // In this case, the gates are closer together along the beam than they
      // are across azimuths so we are further out from the radar
      outOfBounds(nsmooth, 21*r+9);
      outOfBounds(nsmooth, 21*r+10);
      outOfBounds(nsmooth, 21*r+11);
      smooth[21 * r + 10] = 1.0;
      smooth[21 * r + 11] = 1.0 / sqrt(equiv_dist);  // #### sqrt for faster diffusion at far range with limited data
      smooth[21 * r + 9] = smooth[21 * r + 11];
      tmp = 1.0 + 2.0 * smooth[21 * r + 9];
      st_d_az[r] = 9;
    } /* endif - equiv_dist < 1.0 */

    for (int d_az = 0; d_az <= 20; ++d_az)
    {
      outOfBounds(nsmooth, 21*r+d_az);
      smooth[21 * r + d_az] *= 3.0 / tmp;
    }
  } /* endfor - r */

    // Main loop: perform relaxation: nudge each path towards the right sum
    // (account for errors)

  int iterat = 0;
  do
  {
    double forcing;
    // double old_forcing = 0;
    int count_forcing = 0;
    
    int k;
    
    memcpy(work_array, ndata, sizeof(float) * _numGates * _numBeams);
    if (iterat < 1)
    {
      // old_forcing = refract::VERY_LARGE;
      forcing = 0.5 * refract::VERY_LARGE;
      count_forcing = 1;
    }
    else
    {
      // old_forcing = forcing;
      forcing = 0.0;
      count_forcing = 0;

      for (int az = 0; az < _numBeams; ++az)
      {
	float prev_phase = _expectedPhaseRange0 - _rMin * _rangeSlope;
	float cur_phase = prev_phase;
	int prev_r = 0;
	k = az * _numGates + 1;

	for (int r = 1; r < _numGates; ++r, ++k)
	{
	  cur_phase += (ndata[k]- _refN) / slope_to_n;
	  outOfBounds(k);
	  if (_phaseFit[k] != refract::INVALID)
	  {
	    float delta_phase =
	      (float)((int)(100.0 * (_phaseFit[k] - cur_phase) + 36000000) %
		      36000) / 100.0;
	    if (delta_phase > 180.0)
	      delta_phase -= 360.0;
	    delta_phase *= atan(fabs(delta_phase)*force_factor /
				_phaseError[k]) / M_PI * 2.0;  // Error-corrected value
	    float tmp = delta_phase * slope_to_n / (float)(r - prev_r);
	    for (int dr = 0; dr > prev_r - r; --dr)
	    {
	      outOfBounds(k+dr);
	      work_array[k + dr] += tmp;
	    }
	    cur_phase += delta_phase;
	    prev_r = r;
	  } /* endif - _phaseFit[k] != refract::INVALID */
	} /* endfor - r */
      } /* endfor - az */
    } /* endif - iterat < 1 */

      // And then diffuse

    for (int az = 0, k = 0; az < _numBeams; ++az)
    {
      for (int r = 0; r < _numGates; ++r, ++k)
      {
	int count = 3;
	float tmp = 0;
	outOfBounds(k);
	if (r != 0)
	{
	  outOfBounds(k-1);
	  tmp = work_array[k-1] ; // #### May have more weight than local point!!!
	  count++;
	}
	if (r != _numGates - 1)
	{
	  outOfBounds(k+1);
	  tmp += work_array[k+1];
	  count++;
	}

	for (int d_az = st_d_az[r]; d_az <= 20 - st_d_az[r]; ++d_az)
	{
	  int az2 = (az - 10 + d_az + _numBeams) % _numBeams;
	  outOfBounds(az2*_numGates+1);
	  outOfBounds(nsmooth, 21*r+d_az);
	  tmp += smooth[21 * r + d_az] * work_array[az2 * _numGates + r];
	} /* endfor - d_az */

	forcing += fabs(tmp / count - ndata[k]);
	count_forcing++;
	ndata[k] = tmp / count;
      } /* endfor - r */
    } /* endfor - az */
    
      // Check how much change there was.  If changes were limited, call it quit

    forcing /= count_forcing;
    iterat++;

    //LOG(DEBUG_VERBOSE) << iterat << "    " << forcing;

  } while (iterat < num_iterat);
  
  // Reclaim memory

  delete [] smooth;
  delete [] work_array;
  delete [] st_d_az;

  return TRUE;
}

bool PhaseFit::outOfBounds(int offset) const
{
  if (offset >= _scanSize || offset < 0)
  {
    printf("Out of bounds %d\n", offset);
    return true;
  }
  else
  {
    return false;
  }
}
bool PhaseFit::outOfBounds(int max, int offset) const
{
  if (offset >= max || offset < 0)
  {
    printf("Out of bounds %d\n", offset);
    return true;
  }
  else
  {
    return false;
  }
}

//----------------------------------------------------------------------------
void PhaseFit::_setConsistencyAndQuality(const IQ &tmp_ab,
					 float weight_fact,
					 float maxconsistency,
					 float &consistency,
					 float &quality) const
{
  consistency = tmp_ab.norm();
  if (consistency < sqrt(2.0/weight_fact)*maxconsistency)
    consistency = 0.0;
  else
    consistency = sqrt(RefractUtil::SQR(consistency) -
		       2.0/weight_fact*RefractUtil::SQR(maxconsistency));

  if (weight_fact > maxconsistency)    // Should always be true, but...
    quality = consistency / sqrt(maxconsistency * weight_fact);
  else
    quality = consistency / maxconsistency;
  if (quality > 0.99)
    quality = 0.99;
  if (quality < _minConsistency)
    consistency = 0.0;
}

