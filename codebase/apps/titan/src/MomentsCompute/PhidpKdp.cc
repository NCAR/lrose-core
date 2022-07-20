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
////////////////////////////////////////////////////////////
// pol_attr_corr.cc
//
// Code originated at CSU CHILL, Fort Collins, CO

/**
\file moment_svr/pol_att_corr.c
\author Jim George
\brief This module implements PHIDP unwrapping/smoothing,
KDP calculation and attenuation correction based on KDP.

This code is a re-implementation of the code originally written
by Yuxiang Liu, which is in turn, based on PLT code

Algorithms are by Yuxiang Liu, V.N. Bringi and V. Chandrasekar,
minor improvements by Jim George

Smoothing filter coefficients by Yuxiang Liu

*/

#include <cmath>
#include <cstdio>
#include <iostream>
using namespace std;

static double gsl_stats_double_mean(const double *xx, int stride, int nn);
static double gsl_stats_double_sd(const double *xx, int stride, int nn);
static double gsl_stats_double_variance(const double *xx, int stride, int nn);
static double gsl_stats_double_variance_m(const double *xx, int stride, int nn, double mean);

#include "PhidpKdp.hh"

static double FIR24M_coeffs[126]={
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
static double FIR100M_coeffs[31]={
	0.01040850049,0.0136551033,0.01701931136,0.0204494327,
	0.0238905658,0.02728575662,0.03057723021,0.03370766631,
	0.03662148602,0.03926611662,0.04159320123,0.04355972181,
	0.04512900539,0.04627158699,0.04696590613,0.04719881804,
	0.04696590613,0.04627158699,0.04512900539,0.04355972181,
	0.04159320123,0.03926611662,0.03662148602,0.03370766631,
	0.03057723021,0.02728575662,0.0238905658,0.0204494327,
	0.01701931136,0.0136551033,0.01040850049};
static double FIR150M_coeffs[21]={
	1.625807356e-2,2.230852545e-2,2.896372364e-2,3.595993808e-2,
	4.298744446e-2,4.971005447e-2,5.578764970e-2,6.089991897e-2,
	6.476934523e-2,6.718151185e-2,6.80010000e-2,6.718151185e-2,
	6.476934523e-2,6.089991897e-2,5.578764970e-2,4.971005447e-2,
	4.298744446e-2,3.595993808e-2,2.896372364e-2,2.230852545e-2,
	1.625807356e-2 };
static double FIR300M_coeffs[11]={
	0.03064579383,0.0603038422,0.09022859603,0.1159074511,
	0.1332367851,0.1393550634,0.1332367851,0.1159074511,
	0.09022859603,0.0603038422,0.03064579383 };

/* Table must be stored in ascending order of range bin spacing */
phidp_filter_t filt_tbl[] = {
	{24.0, 1.0, 126, FIR24M_coeffs},
	{100.0, 1.0, 31, FIR100M_coeffs},
	{150.0, 1.044222, 21, FIR150M_coeffs},
	{300.0, 1.0, 11, FIR300M_coeffs}
};

#define NUM_FILTERS sizeof(filt_tbl) / sizeof(phidp_filter_t)

static double alpha_arr[23]={
0.075,0.100,0.125,0.150,0.175,0.200,0.225,0.250,0.275,0.300,
0.325,0.350,0.375,0.400,0.425,0.450,0.475,0.500,0.525,0.550,
0.575,0.600,0.625
};

#define PAC_ALPHA_ARRAY_SIZE (sizeof(alpha_arr) / sizeof(double))		

/**
\brief Find a least-squares linear fit to a set of points
\param n Number of points
\param *x X coordinates of points
\param x_stride Stride used to get X coordinates
\param *y Y coordinates of points
\param ystride Stride used to get Y coordinates
\param *slope Pointer to variable where slope is stored
	(if NULL, slope is not calculated)
\param *intercept Pointer to variable where intercept
	is stored (if NULL, intercept is not calculated)

The vertical offsets method is used.
*/
static void fit_leastsquares(int n,
	const double *x, int x_stride,
	const double *y, int y_stride,
	double *slope, double *intercept)
{
	int i;
	double xsum=0, ysum=0, xxsum=0, xysum=0, det;
	const double *xptr, *yptr;
	
	xptr = x;
	yptr = y;
	
	for (i = 0; i < n; i++) {
		xsum += (*xptr);
		ysum += (*yptr);
		xxsum += (*xptr) * (*xptr);
		xysum += (*xptr) * (*yptr);
		
		xptr += x_stride;
		yptr += y_stride;
	}
	det = (double)n * xxsum - xsum * xsum;
	if (slope != NULL) *slope = ((double)n * xysum - xsum * ysum) / det;
	if (intercept != NULL) *intercept = (ysum * xxsum - xsum * xysum) / det;
}

/**
\brief Compute Standard Deviation of PHIDP
\param gates Number of gates to process
\param *PHIDP Input PHIDP to unwrap
\param PHIDP_stride Stride used when reading PHIDP vector
\param *PHIDP_sd Output standard deviation
\param PHIDP_sd_stride Stride used when reading PHIDP_sd vector
\param sd_n Number of gates over which to compte SD
*/
void pac_get_phidp_sd(int gates,
	double *PHIDP, int PHIDP_stride,
	double *PHIDP_sd, int PHIDP_sd_stride,
	int sd_n)
{
	int gate;
	double *PHIDP_ptr = PHIDP;
	double *PHIDP_sd_ptr = PHIDP_sd;
	
	for(gate = 0; gate < sd_n / 2; gate++) {
		*(PHIDP_sd_ptr) = gsl_stats_double_sd(PHIDP, PHIDP_stride, sd_n);
		// PHIDP_ptr += PHIDP_stride;
		PHIDP_sd_ptr += PHIDP_sd_stride;
	}
	for(; gate < (gates - sd_n / 2); gate++) {
		*(PHIDP_sd_ptr) = gsl_stats_double_sd(PHIDP_ptr, PHIDP_stride, sd_n);
		PHIDP_ptr += PHIDP_stride;
		PHIDP_sd_ptr += PHIDP_sd_stride;
	}
	double *PHIDP_last_ptr = PHIDP_ptr;
	for (; gate < gates; gate++) {
		*(PHIDP_sd_ptr) = gsl_stats_double_sd(PHIDP_last_ptr, PHIDP_stride, sd_n);
		// PHIDP_ptr += PHIDP_stride;
		PHIDP_sd_ptr += PHIDP_sd_stride;
	}
}

/**
\brief Unwrap PHIDP
\param gates Number of gates to process
\param *PHIDP Input PHIDP to unwrap
\param PHIDP_stride Stride used to read PHIDP vector
\param *PHIDP_sd Standard deviation of PHIDP
\param PHIDP_sd_stride Stride used to read PHIDP_sd vector
\param *snr SNR per gate
\param snr_stride Stride used to read snr vector
\param *range Vector of range per gate
\param range_stride Stride used to read range vector
\param method Method to use for unwrapping
\param min_gate Minimum range gate to start processing from
\param system_phase System phase to use
\param wrap_threshold Threshold point (PHIDP - system phase) at
	which the algorithm attempts to find a new wrap point.
	This should be set to 90 degrees in VH, 180 degress in VHS
\param *PHIDP_u Output PHIDP after unwrap
\param PHIDP_u_stride Stride used to write PHIDP_u vector
\return The estimate of system phase

The automatic system phase detection method will search for PAC_SYSPHASE_N_GOOD
contiguous gates starting from min_gate out to the specified number of gates.
Once at least 5 such continuous regions are found, the system phase is set to
the mean PHIDP of PAC_SYSPHASE_N_BACKTRACK previous gates. If no such regions
are found, the supplied system_phase parameter is used. The function returns
the detected system phase, which can then be passed back in the system_phase
parameter on the next call, where it will be used to detect wrapping and unwrap
the initial estimate of system phase. If system_phase is NAN, this unwrapping
is disabled. The idea is that the software can maintain a running system phase
variable. The caller should, however, periodically check that the system phase
has not drifted too far off spec (which can happen for rays where there is
heavy clutter contamination, and clutter filtering is off). If such a condition
is detected, the running system phase can be set to NAN, and this function will
re-estimate it without attempting to unwrap the phase.

The first part of the algorithm detects the presence of a possibly wrapped 
PHIDP value. This is decided from the SNR and PHIDP Standard deviation.
One difference from the PLT unwrapping algorithm is that if wrapping of the system
phase is detected, the algorithm bypasses this detecton stage and goes directly
into "phase unwrap" mode

In phase unwrap mode, a 'telescoped PHIDP' is internally maintained, based on
the assumption that the PHIDP is monotonically increasing. This is then compared
against the input PHIDP in order to determine if the true PHIDP has wrapped around
or not. If the input PHIDP has wrapped around, then it is unwrapped.

On return, PHIDP_u is unwrapped using the specified method
*/
double pac_unfold_phidp(int gates,
	const double *PHIDP, int PHIDP_stride,
	const double *RHOHV, int RHOHV_stride,
	const double *PHIDP_sd, int PHIDP_sd_stride,
	const double *snr, int snr_stride,
	const double *range, int range_stride,
	pac_unfold_method_t method, int min_gate,
	double system_phase, double wrap_threshold,
	double *PHIDP_u, int PHIDP_u_stride)
{
	double internal_system_phase;
	int gate;
	int n_phidp_ok = 0;
	double telescoped_phidp = 0.0;
	int init_phase_wrapped = 0;
	
	const double *PHIDP_ptr = PHIDP;
	const double *RHOHV_ptr = RHOHV;
	const double *PHIDP_sd_ptr = PHIDP_sd;
	const double *snr_ptr = snr;
	const double *range_ptr = range;
	double *PHIDP_u_ptr = PHIDP_u;
	
	gate = min_gate;
	
	PHIDP_ptr = PHIDP + gate * PHIDP_stride;
	RHOHV_ptr = RHOHV + gate * RHOHV_stride;
	PHIDP_sd_ptr = PHIDP_sd + gate * PHIDP_sd_stride;
	snr_ptr = snr + gate * snr_stride;
	range_ptr = range + gate * range_stride;
	PHIDP_u_ptr = PHIDP_u + gate * PHIDP_u_stride;
	
	switch (method) {
	case PAC_UNFOLD_AUTO:
		internal_system_phase = system_phase;
		for (; gate < gates - PAC_SYSPHASE_N_GOOD; gate++) {
			if ((*snr_ptr > PAC_SYSPHASE_SNR_THRESHOLD) &&
				(*PHIDP_sd_ptr < PAC_SYSPHASE_SD_THRESHOLD)) {
				n_phidp_ok++;
			}
			else {
				n_phidp_ok = 0;
			}
			if (n_phidp_ok > PAC_SYSPHASE_N_OK_THRESHOLD) {
				internal_system_phase =
					gsl_stats_double_mean(PHIDP_ptr - (PAC_SYSPHASE_N_BACKTRACK + 1) * PHIDP_stride,
			 		PHIDP_stride, PAC_SYSPHASE_N_BACKTRACK);
			 	break;
			}
			PHIDP_ptr += PHIDP_stride;
			PHIDP_sd_ptr += PHIDP_sd_stride;
			snr_ptr += snr_stride;
		}
		/* Check if the detected system phase has wrapped around compared to the estimate,
		if so, unwrap it */
		if (!std::isnan(system_phase) && (fabs(internal_system_phase - system_phase) > wrap_threshold)) {
			init_phase_wrapped = 1;
			if (internal_system_phase > system_phase) {
				internal_system_phase -= wrap_threshold * 2;
			}
			else {
				internal_system_phase += wrap_threshold * 2;
			}
		}
		break;
	case PAC_UNFOLD_USE_SYS_PHIDP:
	default:
		internal_system_phase = system_phase;
		break;
	}
	
	int start_gate = gate;
	
	PHIDP_ptr = PHIDP;
	RHOHV_ptr = RHOHV;
	PHIDP_sd_ptr = PHIDP_sd;
	snr_ptr = snr;
	range_ptr = range;
	PHIDP_u_ptr = PHIDP_u;
	
	for (gate = 0; gate < start_gate; gate++) {
		*PHIDP_u_ptr = internal_system_phase;
		
		PHIDP_ptr += PHIDP_stride;
		RHOHV_ptr += RHOHV_stride;
		PHIDP_sd_ptr += PHIDP_sd_stride;
		snr_ptr += snr_stride;
		range_ptr += range_stride;
		PHIDP_u_ptr += PHIDP_u_stride;
	}
	if (!init_phase_wrapped) {
		/* Detect the first occurence of phase wrapping  */
		for (; gate < (gates - 5); gate++) {
			double avg_phidp;
			
			*PHIDP_u_ptr = *PHIDP_ptr;
			if (*PHIDP_sd_ptr < PAC_SYSPHASE_MODECHANGE_SD_THRESHOLD) {
				avg_phidp = *PHIDP_ptr + *(PHIDP_ptr + PHIDP_stride);
				if (avg_phidp > (internal_system_phase + wrap_threshold - PAC_WRAP_THRESHOLD_EXTRA)) {
					break;
				}
			}
			PHIDP_ptr += PHIDP_stride;
			RHOHV_ptr += RHOHV_stride;
			PHIDP_sd_ptr += PHIDP_sd_stride;
			snr_ptr += snr_stride;
			range_ptr += range_stride;
			PHIDP_u_ptr += PHIDP_u_stride;
		}
	}
	
	/* Go to phase-unwrap mode */
	for (; gate < (gates - 5); gate++) {
		double slope;
		
		/* telescoped_phidp is the predicted PHIDP based on a linear least-squares fit.
		If the difference between normalized PHIDP (without system phase) and the predicted
		phidp is greater than the specified threshold, the phidp is assumed to have wrapped around */
		if ((telescoped_phidp - (*PHIDP_ptr - internal_system_phase)) > (wrap_threshold - PAC_WRAP_THRESHOLD_EXTRA)) {
			double unwrapped_gate = *PHIDP_ptr + (wrap_threshold * 2.0);
			if (fabs(unwrapped_gate - *(PHIDP_u_ptr - PHIDP_u_stride)) < wrap_threshold) {
				*PHIDP_u_ptr = unwrapped_gate;
			}
			else {
				*PHIDP_u_ptr = *PHIDP_ptr;
			}
		}
		else {
			*PHIDP_u_ptr = *PHIDP_ptr;
		}
		/* Find the least-squares fit to the data, and use it to update the predicted phidp */
		fit_leastsquares(5,
			(range_ptr - 5*range_stride), range_stride,
			(PHIDP_u_ptr - 5*PHIDP_u_stride), PHIDP_u_stride,
			&slope, NULL);
		if ((slope > PAC_SYSPHASE_MIN_SLOPE) &&
			(slope < PAC_SYSPHASE_MAX_SLOPE) &&
			(*RHOHV_ptr > 0.9) &&
			(*snr_ptr > PAC_SYSPHASE_EXTRA_SNR_THRESHOLD)) {
			telescoped_phidp += slope * (*(range_ptr + 1) - *range_ptr);
		}
		PHIDP_ptr += PHIDP_stride;
		RHOHV_ptr += RHOHV_stride;
		PHIDP_sd_ptr += PHIDP_sd_stride;
		snr_ptr += snr_stride;
		range_ptr += range_stride;
		PHIDP_u_ptr += PHIDP_u_stride;
	}
	return internal_system_phase;
}

/**
\brief Obtain a data mask used by PHIDP smoothing
\param gates Number of gates to process
\param *Z Reflectivity in dBZ
\param Z_stride Stride used to access Z vector
\param *PHIDP PHIDP in degrees
\param PHIDP_stride Stride used to access PHIDP vector
\param *RHOHV Correlation between H&V
\param RHOHV_stride Stride used to access RHOHV vector
\param *snr SNR, in linear units
\param snr_stride Stride used to access snr vector
\param *range Range of each gate
\param range_stride Stride used to access range vector
\param start_gate Gate to start processing from
\param *datamask Output data mask

On return, datamask is set to 1 for gates where PHIDP
data is usable, 0 otherwise
*/
void pac_get_datamask(int gates,
	const double *Z, int Z_stride,
	const double *PHIDP, int PHIDP_stride,
	const double *RHOHV, int RHOHV_stride,
	const double *snr, int snr_stride,
	const double *range, int range_stride,
	int start_gate,
	char *datamask, int datamask_stride)
{
	int gate;
	double mean, std;
	int n_good, n_bad;
	int count_good = 0, count_bad = 0; /* Number of *consecutive* good/bad gates */

	const double *Z_ptr = Z;
	const double *PHIDP_ptr = PHIDP;
	const double *RHOHV_ptr = RHOHV;
	const double *snr_ptr = snr;
	const double *range_ptr = range;
	char *datamask_ptr = datamask;
	
	enum {
		FIND_GOOD_GATES = 0,
		FIND_BAD_GATES
	} current_mode = FIND_GOOD_GATES;
	
	n_good = MAX((int) (PAC_DATAMASK_GOOD_RNG / (*(range + range_stride) - *range)),
                     PAC_MIN_DATAMASK_GOOD_GATES);
	n_bad = MAX((int) (PAC_DATAMASK_BAD_RNG / (*(range + range_stride) - *range)),
                    PAC_MIN_DATAMASK_BAD_GATES);
	
	for (gate = 0; gate < start_gate; gate++) {
		*datamask_ptr = PAC_DATAMASK_BAD;

		Z_ptr += Z_stride;
		PHIDP_ptr += PHIDP_stride;
		RHOHV_ptr += RHOHV_stride;
		snr_ptr += snr_stride;
		range_ptr += range_stride;
		datamask_ptr += datamask_stride;
	}
	for (;gate < (gates - n_good); gate++) {
		mean = gsl_stats_double_mean(PHIDP_ptr, PHIDP_stride, n_good);
		std = gsl_stats_double_variance_m(PHIDP_ptr, PHIDP_stride, n_good, mean);
		switch (current_mode) {
		case FIND_GOOD_GATES:
		default:
			*datamask_ptr = PAC_DATAMASK_BAD;
			if ((std < PAC_DATAMASK_PHIDP_STD_THRESHOLD) &&
				(*snr_ptr > PAC_DATAMASK_SNR_THRESHOLD) &&
				(*RHOHV_ptr > PAC_DATAMASK_RHOHV_THRESHOLD)) {
				count_good++;
				if (count_good >= n_good) {
					count_good = 0;
					current_mode = FIND_BAD_GATES;
				}
			}
			else {
				count_good = 0;
			}
			break;
		case FIND_BAD_GATES:
			*datamask_ptr = PAC_DATAMASK_GOOD;
			if ((std > PAC_DATAMASK_PHIDP_STD_THRESHOLD) ||
				(*snr_ptr < PAC_DATAMASK_SNR_THRESHOLD) ||
				(*RHOHV_ptr < PAC_DATAMASK_RHOHV_THRESHOLD)) {
				count_bad++;
				if (count_bad >= n_bad) {
					count_bad = 0;
					/* Find the average Z in the 'bad' gates */
					double z_avg = 0.0;
					int bad_gate;
					for (bad_gate = 0; bad_gate < n_bad; bad_gate++) {
						z_avg += IDBS(*(Z_ptr - bad_gate * Z_stride));
					}
					z_avg = DBS(z_avg / (double)n_bad);
					if (z_avg >= PAC_DATAMASK_Z_HAIL_THRESHOLD) {
						if ((gsl_stats_double_mean(RHOHV_ptr - n_bad * RHOHV_stride, RHOHV_stride, n_bad) >
							PAC_DATAMASK_RHOHV_HAIL_THRESHOLD) &&
							(gsl_stats_double_variance(PHIDP_ptr - n_bad * PHIDP_stride, PHIDP_stride, n_bad) >
							PAC_DATAMASK_PHIDP_STD_HAIL_THRESHOLD)) {
							char *temp_dm_ptr = datamask;
							temp_dm_ptr = datamask_ptr - datamask_stride * n_bad;
							for (bad_gate = MAX(0, gate - n_bad); bad_gate < gate; bad_gate++) {
								*temp_dm_ptr = PAC_DATAMASK_HAIL;
								temp_dm_ptr += datamask_stride;
							}
						}
					}
					else {
						current_mode = FIND_GOOD_GATES;
					}
				}
			}
			else {
				count_bad = 0;
			}
			break;
		}
		Z_ptr += Z_stride;
		PHIDP_ptr += PHIDP_stride;
		RHOHV_ptr += RHOHV_stride;
		snr_ptr += snr_stride;
		range_ptr += range_stride;
		datamask_ptr += datamask_stride;
	}
	for (; gate < gates; gate++) {
		*datamask_ptr = PAC_DATAMASK_BAD;

		datamask_ptr += datamask_stride;
	}
}

/**
\brief Adaptively filter PHIDP and obtain KDP
\param gates Number of gates to process
\param *PHIDP Input PHIDP
\param PHIDP_stride Stride used to access PHIDP vector
\param *range Range per gate, in km
\param range_stride Stride used to access range vector
\param *PHIDPinterp Temporary buffer, storing interpolated PHIDP
\param PHIDPinterp_stride Stride used to access PHIDPinterp vector
\param *datamask Mask indicating valid PHIDP data
\param datamask_stride Stride used to access datamask vector
\param *PHIDPf Output smoothed PHIDP
\param PHIDPf_stride Stride used to access PHIDPf vector
\return -1 if the ray contained no data, 0 if smoothing is OK

This routine first interpolates across sections of missing data.
The datamask parameter indicates which gates are valid and which
must be interpolated over. The interpolated gates are then filtered
using an appropriate FIR filter, based on the input range gates.

The filtering is adaptive, in that it will check for points which
differ significantly in power between filtered and unfiltered versions,
such points are replaced with the original data and the results are
re-filtered. This process is repeated until there are no points that
differ in power or until a timeout.

On return, PHIDPf is filled with the smoothed PHIDP
*/
int pac_smooth_phidp(int gates,
	const double *PHIDP, int PHIDP_stride,
	const double *range, int range_stride,
	const char *datamask, int datamask_stride,
	double *PHIDPinterp, int PHIDPinterp_stride,
	double *PHIDPf, int PHIDPf_stride)
{
	int gate;
	phidp_filter_t *filter;
	double start_phidp, end_phidp;
	int n_bad;
	double interp_slope;
	int ctr;
	int adapt_iter;
	int save_gate;

	const double *PHIDP_ptr = PHIDP;
	const double *range_ptr = range;
	const char *datamask_ptr = datamask;
	double *PHIDPinterp_ptr = PHIDPinterp;
	double *PHIDPf_ptr = PHIDPf;
	const double *PHIDP_save_ptr = PHIDP_ptr;
	const double *range_save_ptr = range_ptr;
	
	enum {
		PSP_INIT = 0,
		PSP_BAD_DATA,
		PSP_GOOD_DATA
	} state = PSP_INIT;

	n_bad = MAX((int) (PAC_DATAMASK_BAD_RNG / (range[1] - range[0])),
                    PAC_MIN_DATAMASK_BAD_GATES);
	
	for (ctr = 0; ctr < (int) (NUM_FILTERS); ctr++) {
		filter = &filt_tbl[ctr];
		if (filter->gate_spacing >= ((range[1] - range[0]) * 1000.0)) break;
	}

	save_gate = 0;
	PHIDP_save_ptr = PHIDP_ptr;
	for (gate = 0; gate < (gates - n_bad); gate++) {
		switch (state) {
		case PSP_INIT:
		default:
			datamask_ptr = datamask + gate * datamask_stride;
			
			if (*datamask_ptr != PAC_DATAMASK_BAD) {
				PHIDPinterp_ptr = PHIDPinterp + save_gate * PHIDPinterp_stride;
				PHIDP_ptr = PHIDP + gate * PHIDP_stride;
				for (ctr = save_gate; ctr <= gate; ctr++) {
					PHIDPinterp_ptr = PHIDPinterp + ctr * PHIDPinterp_stride;
					*PHIDPinterp_ptr = *PHIDP_ptr;
					
					PHIDPinterp_ptr += PHIDPinterp_stride;
				}
				state = PSP_GOOD_DATA;
			}
			break;
		case PSP_GOOD_DATA:
			save_gate = gate;
			PHIDP_save_ptr = PHIDP_ptr;
			range_save_ptr = range_ptr;
			datamask_ptr = datamask + gate * datamask_stride;
			if (*datamask_ptr == PAC_DATAMASK_BAD) {
				state = PSP_BAD_DATA;
			}
			else {
				PHIDPinterp_ptr = PHIDPinterp + gate * PHIDPinterp_stride;
				PHIDP_ptr = PHIDP + gate * PHIDP_stride;

				*PHIDPinterp_ptr = *PHIDP_ptr;

				PHIDPinterp_ptr += PHIDPinterp_stride;
			}
			break;
		case PSP_BAD_DATA:
			datamask_ptr = datamask + gate * datamask_stride;
			PHIDP_ptr = PHIDP + gate * PHIDP_stride;
			range_ptr = range + gate * range_stride;
			if ((!std::isnan(*PHIDP_ptr)) && (*datamask_ptr != PAC_DATAMASK_BAD)) {
				PHIDP_save_ptr = PHIDP + save_gate * PHIDP_stride;
				start_phidp = gsl_stats_double_mean((PHIDP_save_ptr - n_bad * PHIDP_stride), PHIDP_stride, n_bad);
				end_phidp = gsl_stats_double_mean(PHIDP_ptr, PHIDP_stride, n_bad);
				range_save_ptr = range + save_gate * range_stride;
				interp_slope = (end_phidp - start_phidp) / (*range_ptr - *range_save_ptr);
				PHIDPinterp_ptr = PHIDPinterp + save_gate * PHIDPinterp_stride;
				if (interp_slope >= 0.0) {
					const double *rp = range_save_ptr;
					for (ctr = save_gate; ctr <= gate; ctr++) {
						*PHIDPinterp_ptr = start_phidp + interp_slope * (*rp - *range_save_ptr);
						PHIDPinterp_ptr += PHIDPinterp_stride;
						rp += range_stride;
					}
				}
				else {
					for (ctr = save_gate; ctr <= gate; ctr++) {
						*PHIDPinterp_ptr = start_phidp;
						PHIDPinterp_ptr += PHIDPinterp_stride;
					}
				}
				state = PSP_GOOD_DATA;
			}
			break;
		}
		PHIDP_ptr += PHIDP_stride;
		range_ptr += range_stride;
		datamask_ptr += datamask_stride;
	}
	/* If the entire ray contained no useable data */
	if (save_gate == 0) {
		PHIDPinterp_ptr = PHIDPinterp;
		PHIDP_ptr = PHIDP;
		for (gate = 0; gate < gates; gate++) {
			*PHIDPinterp_ptr = *PHIDP_ptr;
			
			PHIDP_ptr += PHIDP_stride;
			PHIDPinterp_ptr += PHIDPinterp_stride;
                        return -1;
		}
	}
	else {
		PHIDP_ptr = PHIDP_save_ptr;
		PHIDPinterp_ptr = PHIDPinterp + save_gate * PHIDPinterp_stride;
		for (gate = save_gate; gate < gates; gate++) {
			*PHIDPinterp_ptr = *PHIDP_ptr;
			
			PHIDPinterp_ptr += PHIDPinterp_stride;
		}
	}
	
	PHIDPinterp_ptr = PHIDPinterp;
	PHIDPf_ptr = PHIDPf;
	PHIDP_ptr = PHIDP;
	
	/* Smooth the data */
	for (adapt_iter = 0; adapt_iter < PAC_SMOOTHING_ADAPT_ITERS; adapt_iter++) {
		PHIDPf_ptr = PHIDPf;
		for (gate = 0; gate < gates; gate++) {
			double accum = 0.0;
			for (ctr = 0; ctr < filter->length; ctr++) {
				int phi_idx = CLAMP(gate - filter->length/2 + ctr, 0, gates - 1);
				accum += PHIDPinterp[phi_idx * PHIDPinterp_stride] * filter->coeffs[ctr];
			}
			*PHIDPf_ptr = accum * filter->gain;
			PHIDPf_ptr += PHIDPf_stride;
		}
		PHIDPf_ptr = PHIDPf;
		PHIDPinterp_ptr = PHIDPinterp;
		int retry = 0;
		for (gate = 0; gate < gates; gate++) {
			double delta = fabs(*PHIDPf_ptr - *PHIDPinterp_ptr);
			if (delta > PAC_SMOOTHING_DELTA_THRESHOLD) {
				*PHIDPf_ptr = *PHIDPinterp_ptr;
				retry = 1;
			}
			PHIDPf_ptr += PHIDPf_stride;
			PHIDPinterp_ptr += PHIDPinterp_stride;
		}
		if (!retry) break;
	}
	return 0;
}

/**
\brief Obtain KDP from smoothed PHIDP
\param gates Number of gates to process
\param *PHIDP Input PHIDP (smoothed)
\param PHIDP_stride Stride used to access PHIDP vector
\param *Z Reflectivity
\param Z_stride Stride used to access Z vector
\param *range Range per gate, in km
\param range_stride Stride used to access range vector
\param *KDP Output KDP
\param KDP_stride Stride used to access KDP vector

The smoothed PHIDP estimate is used to obtain KDP by performing a least-squares
fit on local regions of the smoothed PHIDP. The size of the least-squares fit
depends on the reflectivity at the given gate.
*/
void pac_calc_kdp(int gates,
	const double *PHIDP, int PHIDP_stride,
	const double *Z, int Z_stride,
	const double *range, int range_stride,
	double *KDP, int KDP_stride)
{
	int gate, n_kdp;
	double slope;
	
	const double *PHIDP_ptr = PHIDP;
	const double *Z_ptr = Z;
	const double *range_ptr = range;
	double *KDP_ptr = KDP;
	
	for (gate = 0; gate < (PAC_KDP_N_KDP_MAX / 2); gate++) {
		*KDP_ptr = NAN;
		
		PHIDP_ptr += PHIDP_stride;
		Z_ptr += Z_stride;
		range_ptr += range_stride;
		KDP_ptr += KDP_stride;
	}
	for (; gate < gates - (PAC_KDP_N_KDP_MAX / 2); gate++) {
		if (*Z_ptr < PAC_KDP_BAND1_Z) {
			n_kdp = PAC_KDP_N_KDP_1;
		}
		else if ((*Z_ptr > PAC_KDP_BAND1_Z) && (*Z_ptr < PAC_KDP_BAND2_Z)) {
			n_kdp = PAC_KDP_N_KDP_2;
		}
		else {
			n_kdp = PAC_KDP_N_KDP_1;
		}
		fit_leastsquares(n_kdp,
			range_ptr - (n_kdp * range_stride)/2, range_stride,
			PHIDP_ptr - (n_kdp * PHIDP_stride)/2, PHIDP_stride,
			&slope, NULL);
		*KDP_ptr = slope / 2.0;
		
		PHIDP_ptr += PHIDP_stride;
		Z_ptr += Z_stride;
		range_ptr += range_stride;
		KDP_ptr += KDP_stride;
	}
	for (; gate < gates; gate++) {
		*KDP_ptr = NAN;
		
		KDP_ptr += KDP_stride;
	}
}

/**
\brief Get the true height for each gate
\param gates Number of gates to process
\param elevation Elevation of the ray, in degrees
\param *range_km Range to each bin, in km
\param range_km_stride Stride used to access range_km vector
\param *truehgt_km True height of each bin, in km
\param truehgt_km_stride Stride used to access truehgt_km vector

TODO: write a paragraph about this function
*/
void pac_get_true_height(int gates, double elevation,
	const double *range_km, int range_km_stride,
	double *truehgt_km, int truehgt_km_stride)
{
	int gate;
	const double *range_ptr = range_km;
	double *truehgt_ptr = truehgt_km;
	
	for (gate = 0; gate < gates; gate++) {
		double r = (*range_ptr);
		*truehgt_ptr = sqrt(r*r +
			EARTH_RADIUS * EARTH_RADIUS * K_E * K_E +
			2.0 * r * EARTH_RADIUS * K_E * sin(DEG_TO_RADIAN(elevation)));
		range_ptr += range_km_stride;
		truehgt_ptr += truehgt_km_stride;
	}
}

/**
\brief Apply Gaseous attenuation correction
\param gates Number of gates to process
\param elevation Elevation of the ray, in degrees
\param *Z Reflectivity
\param Z_stride Stride used to access Z vector
\param *range_km Range to each bin, in km
\param range_km_stride Stride used to access range_km vector
\param *Zc Reflectivity after correction for gaseous attenuation
\param Zc_stride Stride used to access Zc vector

TODO: write a paragraph about this function
*/
void pac_gaseous_attn_correction(int gates, double elevation,
	const double *Z, int Z_stride,
	const double *range_km, int range_km_stride,
	double *Zc, int Zc_stride)
{
	double t1, t2;
	int gate;
	const double *range_ptr = range_km;
	const double *Z_ptr = Z;
	double *Zc_ptr = Zc;

	t1 = 0.4 + 3.45 * exp(-elevation / 1.8);
	t2 = 27.8 + 154.0 * exp(-elevation / 2.2);
	for (gate = 0; gate < gates; gate++) {
		if ((elevation < PAC_GAS_CORR_MAXELEV) &&
			(*range_ptr < PAC_GAS_CORR_MAXRNG)) {
			*Zc_ptr = *Z_ptr + t1 * (1.0 - exp(-(*range_ptr) / t2));
		}
		else {
			*Zc_ptr = *Z_ptr;
		}
		Z_ptr += Z_stride;
		Zc_ptr += Zc_stride;
		range_ptr += range_km_stride;
	}
}

/**
\brief Obtain KDP from smoothed PHIDP
\param gates Number of gates to process
\param *Zc Corrected Reflectivity, corrected in-place
\param Zc_stride Stride used to access Z vector
\param *PHIDP Input PHIDP (smoothed)
\param PHIDP_stride Stride used to access PHIDP vector
\param *range_km Range per gate, in km
\param range_km_stride Stride used to access range vector

TODO: write a paragraph about this function
*/
void pac_pol_att_corr(int gates, double elevation,
	double *Zc, int Zc_stride,
	const double *ZDR, int ZDR_stride,
	double *Zlin, int Zlin_stride,
	double *ZDRlin, int ZDRlin_stride,
	const double *PHIDP, int PHIDP_stride,
	const double *KDP, int KDP_stride,
	const char *datamask, int datamask_stride,
	double *phidpr, double *Ir,
	const double *range_km, int range_km_stride,
	const double *truehgt_km, int truehgt_km_stride)
{
	int gate;
	int cell_start, cell_end;
	double *Zc_ptr, *Zlin_ptr, *ZDRlin_ptr;
	const double *ZDR_ptr, *PHIDP_ptr, *KDP_ptr;
	const double *range_km_ptr, *truehgt_km_ptr;
	const char *datamask_ptr;
	double pia, Ah;
	enum {
		PAC_GOOD_DATA = 0,
		PAC_BAD_DATA
	} state = PAC_BAD_DATA;
	
	Zc_ptr = Zc;
	ZDR_ptr = ZDR;
	Zlin_ptr = Zlin;
	ZDRlin_ptr = ZDRlin;
	for (gate = 0; gate < gates; gate++) {
		*Zlin_ptr = IDBS(*Zc);
		*ZDRlin_ptr = IDBS(*ZDR);
		
		Zc_ptr += Zc_stride;
		ZDR_ptr += ZDR_stride;
		Zlin_ptr += Zlin_stride;
		ZDRlin_ptr += ZDRlin_stride;
	}
	
	Zc_ptr = Zc;
	ZDR_ptr = ZDR;
	Zlin_ptr = Zlin;
	ZDRlin_ptr = ZDRlin;
	PHIDP_ptr = PHIDP;
	KDP_ptr = KDP;
	range_km_ptr = range_km;
	truehgt_km_ptr = truehgt_km;
	datamask_ptr = datamask;
	
	double gate_dist = (*(range_km_ptr + range_km_stride) - *range_km_ptr);
	double cos_el2 = cos(DEG_TO_RADIAN(elevation)) * cos(DEG_TO_RADIAN(elevation));
	//double sin_el2 = sin(DEG_TO_RADIAN(elevation)) * sin(DEG_TO_RADIAN(elevation));

	for (gate = 0; gate < gates; gate++) {
		switch (state) {
		case PAC_BAD_DATA:
			if (*datamask_ptr == PAC_DATAMASK_GOOD) {
				state = PAC_GOOD_DATA;
			}
			Ah = 0.0;
			break;
		case PAC_GOOD_DATA:
			break;
		}
	}
	
	pia = 0.0;
	
	do {
		switch(*datamask_ptr) {
                  case PAC_DATAMASK_GOOD: {
			cell_start = gate;
			cell_end = gate;
			const char *dm = datamask_ptr;
			const double *PHIDP_cell_end_ptr = PHIDP_ptr;
			while (cell_end < gates) {
				dm += datamask_stride;
				PHIDP_cell_end_ptr += PHIDP_stride;
				cell_end++;
				if (*dm == PAC_DATAMASK_BAD) break;
			}
			double delta_phidp = (*PHIDP_cell_end_ptr - *PHIDP_ptr);
			if (delta_phidp < 30.0) {
				do {
					if ((*KDP_ptr > 0.0) &&
						(*truehgt_km_ptr  < PAC_BRIGHTBAND_U)) {
						Ah = 0.25 * (*KDP_ptr);
					}
					else {
						Ah = 0.0;
					}
					pia += 2.0 * Ah * gate_dist;
					*Zc_ptr += pia;
					
					gate++;

					Zc_ptr += Zc_stride;
					ZDR_ptr += ZDR_stride;
					Zlin_ptr += Zlin_stride;
					ZDRlin_ptr += ZDRlin_stride;
					PHIDP_ptr += PHIDP_stride;
					KDP_ptr += KDP_stride;
					range_km_ptr += range_km_stride;
					truehgt_km_ptr += truehgt_km_stride;
					datamask_ptr += datamask_stride;
				} while (gate < cell_end);
			}
			else {
				double int_g = 0.0;
				int ctr;
				const double *zl_tmp = Zlin_ptr + Zlin_stride * (cell_end - cell_start);
				for (ctr = cell_end; ctr > cell_start; ctr--) {
					Ir[ctr - cell_start] = int_g;
					int_g += pow(*zl_tmp, PAC_B_EXP) * PAC_B_EXP * 0.46 * gate_dist;
					zl_tmp -= Zlin_stride;
				}
				double min_abs_err = 1e12;
				double phidp_factor;
				double opt_alpha = 0.25;
				int k;
				for(k = 0; k < (int) (PAC_ALPHA_ARRAY_SIZE); k++) {
					phidp_factor = pow(10.0, (0.1 * PAC_B_EXP * alpha_arr[k] * delta_phidp / cos_el2)) - 1;
					phidpr[0] = *PHIDP_ptr;
					double abs_err = 0.0;
					zl_tmp = Zlin_ptr;
					const double *PHIDP_tmp = PHIDP_ptr;
					for (ctr = cell_start; ctr < cell_end; ctr++) {
						Ah = (pow(*zl_tmp, PAC_B_EXP) * phidp_factor) / 
							(Ir[0] + phidp_factor * Ir[ctr - cell_start]);
						phidpr[ctr - cell_start + 1] = phidpr[ctr - cell_start] + 
							2.0 * (cos_el2 * Ah / alpha_arr[k]) * gate_dist;
						abs_err = abs_err + fabs(phidpr[ctr - cell_start] - *PHIDP_tmp);
						
						zl_tmp += Zlin_stride;
						PHIDP_tmp += PHIDP_stride;
					}
					if (abs_err < min_abs_err) {
						abs_err = min_abs_err;
						opt_alpha = alpha_arr[k];
					}
				}
				if ((opt_alpha == alpha_arr[0]) || (opt_alpha == alpha_arr[PAC_ALPHA_ARRAY_SIZE - 1])) {
					opt_alpha = 0.25;
				}
				phidp_factor = pow(10.0, (0.1 * PAC_B_EXP * opt_alpha * delta_phidp / cos_el2)) - 1;
				phidpr[0] = *PHIDP_ptr;
				do {
					Ah = (pow(*Zlin_ptr, PAC_B_EXP) * phidp_factor) / 
						(Ir[0] + phidp_factor * Ir[ctr - cell_start]);
					pia += 2.0 * Ah * gate_dist;
					*Zc_ptr += pia;
					
					gate++;

					Zc_ptr += Zc_stride;
					ZDR_ptr += ZDR_stride;
					Zlin_ptr += Zlin_stride;
					ZDRlin_ptr += ZDRlin_stride;
					PHIDP_ptr += PHIDP_stride;
					KDP_ptr += KDP_stride;
					range_km_ptr += range_km_stride;
					truehgt_km_ptr += truehgt_km_stride;
					datamask_ptr += datamask_stride;
				} while (gate < cell_end);
			}
			break;
                  }
		case PAC_DATAMASK_BAD:
                  default: {
			gate++;
			Zc_ptr += Zc_stride;
			ZDR_ptr += ZDR_stride;
			Zlin_ptr += Zlin_stride;
			ZDRlin_ptr += ZDRlin_stride;
			PHIDP_ptr += PHIDP_stride;
			KDP_ptr += KDP_stride;
			range_km_ptr += range_km_stride;
			truehgt_km_ptr += truehgt_km_stride;
			datamask_ptr += datamask_stride;
                  }
		}
	} while (gate < gates);
}

/////////////////////////////////////////////////////////////
// local implementation of gsl functions
//

static double gsl_stats_double_mean(const double *xx, int stride, int nn)

{
  
  double sum = 0.0;
  double count = 0.0;
  for (int ii = 0; ii < nn; ii++, xx += stride) {
    sum += *xx;
    count++;
  }
  if (count > 0) {
    return sum / count;
  } else {
    return 0.0;
  }

}

static double gsl_stats_double_sd(const double *xx, int stride, int nn)

{

  double sum = 0.0;
  double sumsq = 0.0;
  double count = 0.0;
  for (int ii = 0; ii < nn; ii++, xx += stride) {
    double val = *xx;
    sum += val;
    sumsq += val * val;
    count++;
  }

  if (count > 1) {
    double mean_sumsq = sumsq / count;
    double mean = sum / count;
    double sq_of_mean = mean * mean;
    if (mean_sumsq >= sq_of_mean) {
      return sqrt(mean_sumsq - sq_of_mean);
    } else {
      return 0.0;
    }
  } else {
    return 0.0;
  }

}

static double gsl_stats_double_variance(const double *xx, int stride, int nn)

{

  double sum = 0.0;
  double sumsq = 0.0;
  double count = 0.0;
  for (int ii = 0; ii < nn; ii++, xx += stride) {
    double val = *xx;
    sum += val;
    sumsq += val * val;
    count++;
  }
  
  if (count > 1) {
    double mean_sumsq = sumsq / count;
    double mean = sum / count;
    double sq_of_mean = mean * mean;
    if (mean_sumsq >= sq_of_mean) {
      return mean_sumsq - sq_of_mean;
    } else {
      return 0.0;
    }
  } else {
    return 0.0;
  }

}

static double gsl_stats_double_variance_m(const double *xx, int stride, int nn, double mean)

{

  double sumsq = 0.0;
  double count = 0.0;
  for (int ii = 0; ii < nn; ii++, xx += stride) {
    double val = *xx;
    double diff = val - mean;
    sumsq += diff * diff;
    count++;
  }
  if (count > 0) {
    return sumsq / count;
  } else {
    return 0.0;
  }
  
}
