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

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <rapmath/math_macros.h>

#include "PhaseFit.hh"

using namespace std;

// Global variables

const double PhaseFit::VERY_LARGE = 2147483647.0;
const float PhaseFit::INVALID = -99999.0;

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
  phase(0),
  quality(0),
  inphase(0),
  quadrature(0),
  _debug(false),
  _verbose(false),
  _phaseFit(0),
  _phaseError(0),
  _smoothI(0),
  _smoothQ(0),
  _nOutput(0),
  _nError(0)
{
}


/*********************************************************************
 * Destructor
 */

PhaseFit::~PhaseFit()
{
  delete [] phase;
  delete [] quality;
  delete [] inphase;
  delete [] quadrature;
  delete [] _phaseFit;
  delete [] _phaseError;
  delete [] _smoothI;
  delete [] _smoothQ;
}


/*********************************************************************
 * init()
 */

bool PhaseFit::init(const int num_beams, const int num_gates,
		    const double gate_spacing,
		    const double wavelength,
		    const double min_consistency, const int r_min,
		    const bool debug_flag, const bool verbose_flag)
{
  // Save the debug flags

  _debug = debug_flag;
  _verbose = verbose_flag;
  
  // Save the radar and phase fitting information

  _numBeams = num_beams;
  _numGates = num_gates;
  _gateSpacing = gate_spacing;
  _wavelength = wavelength;
  _minConsistency = min_consistency;
  _rMin = r_min;
  
  // Allocate space for the internal arrays

  int scan_size = _numBeams * _numGates;
  
  phase = new float[scan_size];
  quality = new float[scan_size];
  inphase = new float[scan_size];
  quadrature = new float[scan_size];

  _phaseFit = new float[scan_size];
  _phaseError = new float[scan_size];
  _smoothI = new float[scan_size];
  _smoothQ = new float[scan_size];

  // Calculate other needed values

  _azimSpacing = 360.0 / (double)_numBeams;
  
  return true;
}


/*********************************************************************
 * fitPhaseField()
 */

bool PhaseFit::fitPhaseField(const bool do_relax)
{
  // Initialize

  float slope = _meanPhaseSlopeInit();
  if (slope == INVALID)
    return false;
  
  _expectedPhaseRange0 = _phaseRange0() - _rMin * slope;

  // Get the smoothed phase field

  _rangeSlope = _doSmoothing();
  if (_rangeSlope == INVALID)
    return false;
  
  if (_verbose)
    cerr << "Smoothing complete!" << endl;

  // Compute the resulting refractivity field and its error

  float slope_to_n =
    1000000.0 / _gateSpacing * _wavelength / 720.0 *
    _numGates / _numGates / DEG_TO_RAD;
  float tmp = _rangeSlope * 1000000.0 /
    _gateSpacing * _wavelength / 720.0 + _refN;
  float er_decorrel = 2.0 * _gateSpacing * _numGates /
    _numGates / _smoothSideLen;
  if (er_decorrel > 1.0)
    er_decorrel = 1.0;

  if (_debug)
  {
    cerr << "---> Initializing output data" << endl;
    cerr << "      num az = " << _numBeams << endl;
    cerr << "      num range = " <<  _numGates << endl;
  }
  
  for (int azn = 0, offsetn = 0; azn < _numBeams; ++azn, ++offsetn)
  {
    _nOutput[offsetn] = tmp;
    _nError[offsetn] = VERY_LARGE;

    int rn;
    
    for (rn = 1, ++offsetn; rn < _numGates - 1;
	 ++rn, ++offsetn)
    {
      float tmp_a = _smoothI[offsetn] * _smoothI[offsetn-1] +
	_smoothQ[offsetn] * _smoothQ[offsetn-1] +
	_smoothI[offsetn+1] * _smoothI[offsetn] +
	_smoothQ[offsetn+1] * _smoothQ[offsetn];
      float tmp_b = _smoothQ[offsetn] * _smoothI[offsetn-1] -
	_smoothI[offsetn] * _smoothQ[offsetn-1] +
	_smoothQ[offsetn+1] * _smoothI[offsetn] -
	_smoothI[offsetn+1] * _smoothQ[offsetn];
      _nOutput[offsetn] = atan2(tmp_b, tmp_a) * slope_to_n + _refN;
      _nError[offsetn] =
	er_decorrel * sqrt((SQR(_phaseError[offsetn+1]) +
			    SQR(_phaseError[offsetn]) +
			    SQR(_phaseError[offsetn-1])) / 6.0) *
	slope_to_n * DEG_TO_RAD ;
    } /* endfor - rn */

    _nOutput[offsetn] = tmp;
    _nError[offsetn] = VERY_LARGE;
  } /* endfor - azn */

  if (_verbose)
    cerr << "N and N-error derived from smoothed field." << endl;

  // Improve estimates of refractivity through a relaxation process

  if (do_relax)
  {
    _relax(_nOutput);
    if (_verbose)
      cerr << "Relaxation complete." << endl;
  }

  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

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

float PhaseFit::_doSmoothing()
{
  if (_debug)
    cerr << "    Entered _doSmoothing()" << endl;

  // Estimate the initial slope (slope near range = 0) and average one
  // (throughout the whole field).  These will act as anchors to prevent
  // the algorithm from running away from the reasonable.

  float init_slope = _meanPhaseSlopeInit();
  _rangeSlope = init_slope;
  float range_slope = _meanPhaseSlopeAvg();

  if (init_slope == INVALID || range_slope == INVALID)
    return INVALID;
  
  if (_debug)
  {
    cerr << "    range_slope = " << (range_slope * 1000.0 / _gateSpacing)
	 << " deg/km; init_slope = "
	 << (init_slope * 1000 / _gateSpacing) << " deg/km" << endl;
  }

  // Compute smoothed field where enough data is available.

  // Allocate memory and prepare some tables

  int maxbins = _numBeams * _numGates;

  float *guess_phase = new float[_numBeams];

  float *slope_in_range = new float[_numBeams];
  float *next_slope_in_range = new float[_numBeams];
  
  int smooth_range =
    (int)(_smoothSideLen / 2.0 / _gateSpacing);
  if (smooth_range <= 0)
    smooth_range = 1;
  int two_smooth_range = 2 * smooth_range;

  double *sum_inphase = new double[_numBeams / 4];
  double *sum_quadra = new double[_numBeams / 4];
  double *max_quality = new double[_numBeams / 4];
  float *slope_a = new float[360 * (two_smooth_range + 1)];
  float *slope_b = new float[360 * (two_smooth_range + 1)];

  for (int index = 0;
       index < _numGates * _numBeams;
       ++index)
  {
    _phaseFit[index] = INVALID;
    _phaseError[index] = VERY_LARGE;
  }

  for (int angle = 0, index = 0; angle < 360; angle++)
  {
    for (int dr = 0; dr <= two_smooth_range; dr++, index++)
    {
      slope_a[index] = cos((smooth_range - dr) * angle * DEG_TO_RAD);
      slope_b[index] = sin((smooth_range - dr) * angle * DEG_TO_RAD);
    } /* endfor - dr */
  } /* endfor - angle, index */
  
  for (int az = 0; az < _numBeams; ++az)
  {
    slope_in_range[az] = init_slope;
    next_slope_in_range[az] = init_slope;
  }
  
  // For each range, we determine the smoothing area in range-azim system 

  float minconsistency;

  for (int r = 0; r < _numGates; ++r)
  {
    int smooth_azim =
      (int)(_smoothSideLen * 360.0 /
	    (_azimSpacing * r *
	     _gateSpacing * 4.0 * M_PI));
    if (smooth_azim >= _numBeams / 8)
      smooth_azim = _numBeams / 8 - 1;
    if (smooth_azim <= 0)
      smooth_azim = 1;

    int two_smooth_azim = 2 * smooth_azim;
    minconsistency =
      (two_smooth_range + 1) * (two_smooth_azim + 1) * _minConsistency;
    if (minconsistency < MIN_ABS_CONSISTENCY)
      minconsistency = MIN_ABS_CONSISTENCY;

    // The smoothed value of phase(r+dr) is precomputed for the last azimuth
    // (done to speed up computation afterwards when azimuths are shifted).
    // The weighting function of I/Q data follows a (1-a(r-r0)^2) like function

    int az2 = _numBeams - 1;
    
    for (int daz = 0; daz <= two_smooth_azim; daz++)
    {
      sum_inphase[daz] = 0.0;
      sum_quadra[daz] = 0.0;
      max_quality[daz] = 0.0;
    } /* endfor - daz */

    for (int dr = 0; dr <= two_smooth_range; dr++)
    {
      float weight_fact =
	sqrt(2.0) * (1.0 - SQR((smooth_range-dr) / ((float)smooth_range + 0.5)));
      if ((r + dr - smooth_range >= _rMin) &&
	  (r + dr - smooth_range < _numGates))
      {
	for (int daz = -smooth_azim; daz <= smooth_azim; daz++)
	{
	  int az =
	    (az2 + daz + _numBeams) % _numBeams;
	  int offset = az * _numGates + r + dr - smooth_range;
	  int k =
	    (((int)(floor(slope_in_range[az] + 0.5)) + 360000) % 360) *
	    (2 * smooth_range + 1);
	  sum_inphase[daz + smooth_azim] +=
	    weight_fact * (inphase[offset] * slope_a[k+dr] -
			   quadrature[offset] * slope_b[k+dr]);
	  sum_quadra[daz + smooth_azim] +=
	    weight_fact * (quadrature[offset] * slope_a[k+dr] +
			   inphase[offset] * slope_b[k+dr]);
	  max_quality[daz + smooth_azim] += weight_fact * quality[offset];
	} /* endfor - daz */
      }
      
    } /* endfor - dr */

    // For each azimuth, we guess what the next phase(range) should be based on
    // the previously computed d(Phase)/d(range) at that azimuth.  This step will
    // be needed for optimum smoothing.

    int rjump = 0;

    for (int az = 0; az < _numBeams; az++)
    {
      rjump = 1;
      
      int off2 = az * _numGates + r;

      if (r <= _rMin)
      {
	guess_phase[az] = _expectedPhaseRange0;
      }
      else
      {
	while (_phaseFit[off2-rjump] == INVALID && rjump < r)
	  rjump++;

	if (r - rjump >= _rMin)
	{
	  guess_phase[az] =
	    _phaseFit[off2 - rjump] + rjump * slope_in_range[az];
	}
	else
	{
	  rjump = r - _rMin;
	  
	  guess_phase[az] =
	    _expectedPhaseRange0 + rjump * slope_in_range[az];
	}
      } /* endelse - r <= _rMin */
    } /* endfor - az */

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
      int azjump =
	(azn - oldaz + _numBeams) % _numBeams;
      int off2 = azn * _numGates + r;

      for (int j = 0; j < azjump; j++)
      {
	for (int daz = 0; daz < two_smooth_azim; daz++)
	{
	  // Shift sums

	  sum_inphase[daz] = sum_inphase[daz+1];
	  sum_quadra[daz] = sum_quadra[daz+1];
	  max_quality[daz] = max_quality[daz+1];
	} /* endfor - daz */

	sum_inphase[two_smooth_azim] = 0.0; // Sum slope-corrected I/Q
	sum_quadra[two_smooth_azim] = 0.0;
	max_quality[two_smooth_azim] = 0.0;

	int az = (azn + j + smooth_azim) % _numBeams;

	int k = (((int)(floor(slope_in_range[az] + 0.5)) + 360000) % 360) *
	  (two_smooth_range + 1);

	for (int dr = 0; dr <= two_smooth_range; dr++)
	{
	  if ((r + dr - smooth_range >= _rMin) &&
	      (r + dr - smooth_range < _numGates))
	  {
	    float weight_fact = sqrt(2.0) *
	      (1.0 - SQR((smooth_range-dr) / ((float)smooth_range + 0.5)));
	    int offset =
	      off2 + (smooth_azim + j) * _numGates + dr - smooth_range;
	    if (offset >= maxbins)
	      offset -= maxbins;

	    sum_inphase[two_smooth_azim] +=
	      weight_fact * (inphase[offset] * slope_a[k + dr] -
			     quadrature[offset] * slope_b[k + dr]);
	    sum_quadra[two_smooth_azim] +=
	      weight_fact * (quadrature[offset] * slope_a[k + dr] +
			     inphase[offset] * slope_b[k + dr]);
	    max_quality[two_smooth_azim] += weight_fact * quality[offset];
	  }
	} /* endfor - dr */
	
      } /* endfor - j */
      
      oldaz = azn;

      // Combine all azimuths of smoothing area, correcting for mean phase
      // of row

      float tmp_a = 0.0;
      float tmp_b = 0.0;
      float maxconsistency = 0.0;

      for (int daz = 0; daz <= two_smooth_azim; daz++)
      {
	int az = (azn + daz - smooth_azim + _numBeams) % _numBeams;

	float cor_i;
	float cor_q;
	
	if (r > 0 &&
	    _phaseFit[az * _numGates + r - 1] != INVALID &&
	    _phaseFit[off2 - 1] != INVALID)
	{
	  int k = (((int)(floor(guess_phase[azn] - guess_phase[az] + 0.5)) + 
		360000) % 360);
	  if (k < 180)    // Dampen phase correction, otherwise it misbehaves
	    k = k / 2;
	  else
	    k = k + (360 - k) / 2;
	  k = k * (two_smooth_range + 1) + smooth_range - 1;
	  cor_i = sum_inphase[daz] * slope_a[k] - sum_quadra[daz] * slope_b[k];
	  cor_q = sum_quadra[daz] * slope_a[k] + sum_inphase[daz] * slope_b[k];
	}
	else
	{
	  cor_i = sum_inphase[daz];
	  cor_q = sum_quadra[daz];
	}
	float weight_fact =
	  sqrt(2.0) * (1.0 - SQR(smooth_azim-daz) / SQR(smooth_azim + 0.5));
	tmp_a += weight_fact * cor_i;
	tmp_b += weight_fact * cor_q;
	maxconsistency += weight_fact * max_quality[daz];
      } /* endfor - daz */

      // Save results.  Update next_slope_in_range.

      float weight_fact =
	(float)((two_smooth_range + 1) * (two_smooth_azim + 1));
      _smoothI[off2] = tmp_a / weight_fact;
      _smoothQ[off2] = tmp_b / weight_fact;
      float consistency = sqrt(SQR(tmp_a) + SQR(tmp_b));
      if (consistency < sqrt(2.0 / weight_fact) * maxconsistency)
	consistency = 0.0;
      else
	consistency =
	  sqrt(SQR(consistency) - 2.0 / weight_fact * SQR(maxconsistency));

      float quality;
      
      if (weight_fact > maxconsistency)    // Should always be true, but...
	quality = consistency / sqrt(maxconsistency * weight_fact);
      else
	quality = consistency / maxconsistency;
      if (quality > 0.99)
	quality = 0.99;
      if (quality < _minConsistency)
	consistency = 0.0;
      if (consistency > minconsistency)
      {
	float tmp_phase = atan2(tmp_b, tmp_a) / DEG_TO_RAD;
	while (tmp_phase - guess_phase[azn] < -180.0)
	  tmp_phase += 360.0;
	while (tmp_phase - guess_phase[azn] >= 180.0)
	  tmp_phase -= 360.0;
	_phaseFit[off2] = tmp_phase;
	_phaseError[off2] =
	  sqrt(-2.0 * log(quality) / quality) / DEG_TO_RAD /
	  sqrt(maxconsistency / 2.0);
	if (consistency > 4.0 * minconsistency)
	{
	  next_slope_in_range[azn] +=
	    2.0 * _gateSpacing / _smoothSideLen /
	    rjump * (tmp_phase - guess_phase[azn]);
	}
	else
	{
	  next_slope_in_range[azn] +=
	    0.5 * consistency / minconsistency *
	    _gateSpacing / _smoothSideLen / rjump *
	    (tmp_phase - guess_phase[azn]);

	  int prev_az = (azn + _numBeams - 1) % _numBeams;
	  int next_az = (azn + 1) % _numBeams;
	  
	  next_slope_in_range[azn] +=
	    (1.0 - 0.25 * consistency / minconsistency) *
	    _gateSpacing / _smoothSideLen *
	    (slope_in_range[prev_az]
	     + slope_in_range[next_az] +
	     0.25 * range_slope - 2.25 * slope_in_range[azn]);

	}
      }
      else
      {
	next_slope_in_range[azn] +=
	  _gateSpacing / _smoothSideLen *
	  (slope_in_range[(azn + _numBeams - 1) %
			  _numBeams] +
	   slope_in_range[(azn + 1) % _numBeams] +
	   0.25 * range_slope - 2.25 * slope_in_range[azn]);

	_smoothI[off2] =
	  0.1 * minconsistency * cos(guess_phase[azn]*DEG_TO_RAD);
	_smoothQ[off2] =
	  0.1 * minconsistency * sin(guess_phase[azn]*DEG_TO_RAD);
      }

      // If quality is really poor, slope might be wrong; compute it the
      // raw way

      if (quality < sqrt(2.0 / weight_fact))
      {
	double rawslope_a = 0.0;
	double rawslope_b = 0.0;
	tmp_a = 0.0;
	tmp_b = 0.0;
	for (int dr = -two_smooth_range; dr <= two_smooth_range; dr++)
	{
	  if ((r+dr >= _rMin) &&
	      (r+dr < _numGates))
	  {
	    float old_tmp_a = tmp_a;
	    float old_tmp_b = tmp_b;
	    tmp_a = 0.0;
	    tmp_b = 0.0;
	    for (int daz = -two_smooth_azim; daz <= two_smooth_azim; daz++)
	    {
	      int az =
		(azn + daz + _numBeams) % _numBeams;
	      int offset = az * _numGates + r + dr;
	      tmp_a += inphase[offset];
	      tmp_b += quadrature[offset];
	    } /* endfor - daz */
	    rawslope_a += tmp_a * old_tmp_a + tmp_b * old_tmp_b;
	    rawslope_b += tmp_b * old_tmp_a - tmp_a * old_tmp_b;
	  }
	} /* endfor - dr */
	
	consistency = sqrt(SQR(rawslope_a) + SQR(rawslope_b));
	if (consistency >
	    _minConsistency * SQR(2 * two_smooth_azim + 1) *
	    (2 * two_smooth_range + 1))
	{
	  slope_in_range[azn] = atan2(rawslope_b, rawslope_a) / DEG_TO_RAD;
	  if (fabs(slope_in_range[azn] - range_slope) < 60.0)
	  {
	    next_slope_in_range[azn] +=
	      0.2 * (slope_in_range[azn] - next_slope_in_range[azn]);
	  }
	  
	}
      }

      if (r < INITIAL_SLOPE_LEN_M / _gateSpacing)
      {
	next_slope_in_range[azn] +=
	  0.1 * (1.0 - r / (INITIAL_SLOPE_LEN_M / _gateSpacing)) *
	  (init_slope - next_slope_in_range[azn]);
      }
      
      if (next_slope_in_range[azn] > range_slope + 60.0)    // Occasionally-needed railing
	next_slope_in_range[azn] = range_slope + 60.0;
      if (next_slope_in_range[azn] < range_slope - 60.0)
	next_slope_in_range[azn] = range_slope - 60.0;
    } /* endfor - azn */

    memcpy(slope_in_range, next_slope_in_range, _numBeams * sizeof(float));
  } /* endfor - r */
  
  // Task completed!

  delete [] slope_b;
  delete [] slope_a;
  delete [] max_quality;
  delete [] sum_quadra;
  delete [] sum_inphase;
  delete [] next_slope_in_range;
  delete [] slope_in_range;
  delete [] guess_phase;

  return _rangeSlope;
}


/*********************************************************************
 * _meanPhaseSlope()
 */

double PhaseFit::_meanPhaseSlope(const int max_r, const int smear_az) const
{
  static const string method_name = "PhaseFit::_meanPhaseSlope()";
  
  double slope_i = 0.0;
  double slope_q = 0.0;

  for (int az = 0; az < _numBeams; az += smear_az)
  {
    int offset = az * _numGates + _rMin;
    double tmp_a = 0.0;
    double tmp_b = 0.0;

    for (int r = _rMin; r < max_r; r += SMEAR_RA, offset += SMEAR_RA)
    {
      double old_tmp_a = tmp_a;
      double old_tmp_b = tmp_b;
      tmp_a = 0.0;
      tmp_b = 0.0;

      for (int j = 0; j < smear_az; ++j)
      {
	for (int k = 1; k <= SMEAR_RA; ++k)
	{
	  int bin_index = offset + (j * _numGates) + k;
	  
	  tmp_a += inphase[bin_index];
	  tmp_b += quadrature[bin_index];
	} /* endfor - k */
      } /* endfor - j */
      
      slope_i += tmp_a * old_tmp_a + tmp_b * old_tmp_b;
      slope_q += tmp_b * old_tmp_a - tmp_a * old_tmp_b;
    } /* endfor - r */
  } /* endfor - az */

  if (slope_i == 0.0 && slope_q == 0.0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Cannot calculate mean phase slope" << endl;
    cerr << "I and Q values are all 0" << endl;
    
    return INVALID;
  }
  
  return atan2(slope_q, slope_i) / DEG_TO_RAD / SMEAR_RA;
}


/*********************************************************************
 * _meanPhaseSlopeAvg()
 */

double PhaseFit::_meanPhaseSlopeAvg() const
{
  int max_r = _numGates - SMEAR_RA;
  int smear_az = (int)(SMEAR_AZ * _numBeams / 360.0);

  return _meanPhaseSlope(max_r, smear_az);
}


/*********************************************************************
 * _meanPhaseSlopeInit()
 */

double PhaseFit::_meanPhaseSlopeInit() const
{
  int max_r = _rMin +
    (int)(INITIAL_SLOPE_LEN_M / _gateSpacing);
  int smear_az = (int)(SMEAR_AZ_INIT * _numBeams / 360.0);

  return _meanPhaseSlope(max_r, smear_az);
}


/*********************************************************************
 * _phaseRange0()
 */

double PhaseFit::_phaseRange0()
{
  double tmp_a = 0.0;
  double tmp_b = 0.0;

  for (int az = 0; az < _numBeams; ++az)
  {
    int offset = az * _numGates + _rMin;
    tmp_a += inphase[offset];
    tmp_b += quadrature[offset];
  }

  if (tmp_a != 0.0 || tmp_b != 0.0)
    return atan2(tmp_b, tmp_a) / DEG_TO_RAD;

  return INVALID;
}


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

  if (_debug)
    cerr << "Now starting relaxation" << endl;

  float *smooth = new float[21 * _numGates];
  memset(smooth, 0, 21 * _numGates * sizeof(float));
  
  float *work_array = new float[_numBeams * _numGates];
  int *st_d_az = new int[_numGates];

  float slope_to_n = 1000000.0 / _gateSpacing * _wavelength / 720.0;
  float force_factor = SQR(_gateSpacing / _smoothSideLen);	/* Sets how field follows data (>0); small -> loose */
  int num_iterat = (int)(0.5 * SQR(0.5 * _smoothSideLen / _gateSpacing));
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
	if (((d_az + 0.5) - 10.0) * equiv_dist < 1.5)
	{
	  smooth[21 * r + d_az] = 1.0;
	  smooth[21 * r + 20 - d_az] = 1.0;
	  tmp += 2.0;
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

      smooth[21 * r + 10] = 1.0;
      smooth[21 * r + 11] = 1.0 / sqrt(equiv_dist);  // #### sqrt for faster diffusion at far range with limited data
      smooth[21 * r + 9] = smooth[21 * r + 11];
      tmp = 1.0 + 2.0 * smooth[21 * r + 9];
      st_d_az[r] = 9;
    } /* endif - equiv_dist < 1.0 */

    for (int d_az = 0; d_az <= 20; ++d_az)
      smooth[21 * r + d_az] *= 3.0 / tmp;
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
      // old_forcing = VERY_LARGE;
      forcing = 0.5 * VERY_LARGE;
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
	  if (_phaseFit[k] != INVALID)
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
	      work_array[k + dr] += tmp;
	    cur_phase += delta_phase;
	    prev_r = r;
	  } /* endif - _phaseFit[k] != INVALID */
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
	if (r != 0)
	{
	  tmp = work_array[k-1] ; // #### May have more weight than local point!!!
	  count++;
	}
	if (r != _numGates - 1)
	{
	  tmp += work_array[k+1];
	  count++;
	}

	for (int d_az = st_d_az[r]; d_az <= 20 - st_d_az[r]; ++d_az)
	{
	  int az2 = (az - 10 + d_az + _numBeams) % _numBeams;
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

    if (_verbose)
      cerr << iterat << "    " << forcing << endl;

  } while (iterat < num_iterat);
  
  // Reclaim memory

  delete [] smooth;
  delete [] work_array;
  delete [] st_d_az;

  return TRUE;
}
