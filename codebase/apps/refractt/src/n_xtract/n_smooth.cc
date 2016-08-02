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
/* n_smooth.c:  Module of the programs n_xtract (to extract refractive index
   fields from ground targets).  This file contains all the routines required
   to fit a refractivity pattern to the obatined phase field (steps ####).

   Code that should be changed for uses in systems other than the
   McGill Radar is indicated by the string @@@@ , and has been isolated
   in subroutines for the most part to make the task easier for you.

------------------------------------------------------------------------------
*/

/* Includes and global variables used primarily here */
#ifndef N_XTRACT_H
#include	<refractt/n_xtract.h>		/* Include & defines for this program */
#endif
#ifdef DUMP_NC
#include "n_util.h"
#endif
#include	<refractt/nrutil.h>	/* Include & defines for Numerical Recipes */

#define		LatestMods	TRUE

/*****************************************************************************/

/* dif_phase() :  Computes the phase difference between the last two scans
   as well as between the current scan and the reference scan.  These two
   arrays will be the base data for the processing to follow elsewhere.
   Input: Phase of targets from current, previous, and reference scan with
          data quality info for each
   Output: Phase difference arrays with data quality info
   Called by: get_targets() */

void dif_phase()
{
	int     az, r, offset ;
	float	tmp_a, tmp_b, norm ;

	if( DebugLevel != QUIET )
	    debug_print("And phase / phase difference arrays\n") ;
	for( az = 0, offset = 0 ; az < NumAzim ; az++ )
	    for( r = 0 ; r < NumRangeBins ; r++, offset++ ) {

/* Compute phase difference wrt previous scan (now in dif_prev_scan).
   Normalize I & Q with sqrt(quality old * quality new). */
		tmp_a = dif_prev_scan[offset].inphase ;
		tmp_b = dif_prev_scan[offset].quadrature ;
		dif_prev_scan[offset].inphase = tmp_a * raw_phase[offset].inphase + tmp_b * raw_phase[offset].quadrature ;
		dif_prev_scan[offset].quadrature = tmp_a * raw_phase[offset].quadrature - tmp_b * raw_phase[offset].inphase ;
#if (LatestMods == TRUE)
		norm = pow( SQR(dif_prev_scan[offset].inphase) + SQR(dif_prev_scan[offset].quadrature), .375 ) ;
#else
		norm = pow( SQR(dif_prev_scan[offset].inphase) + SQR(dif_prev_scan[offset].quadrature), .25 ) ;
#endif
		if( norm != 0.) {
		    target[offset].phase_diff = atan2( dif_prev_scan[offset].quadrature, dif_prev_scan[offset].inphase ) / DEGTORAD ;
		    dif_prev_scan[offset].inphase /= norm ;
		    dif_prev_scan[offset].quadrature /= norm ;
		    target[offset].phase_diff_er = sqrt(-2. * log( norm ) / norm) / DEGTORAD ;
		    target[offset].dif_i = dif_prev_scan[offset].inphase ;
		    target[offset].dif_q = dif_prev_scan[offset].quadrature ;
		}
		else {
		    target[offset].phase_diff = INVALID ;
		    target[offset].phase_diff_er = INVALID ;
		    target[offset].dif_i = 0. ;
		    target[offset].dif_q = 0. ;
		}

/* Compute phase difference wrt master reference */
		dif_from_ref[offset].inphase = calib_ref[offset].av_i * raw_phase[offset].inphase + calib_ref[offset].av_q * raw_phase[offset].quadrature ;
		dif_from_ref[offset].quadrature = calib_ref[offset].av_i * raw_phase[offset].quadrature - calib_ref[offset].av_q * raw_phase[offset].inphase ;
		if( (dif_from_ref[offset].inphase != 0.) || (dif_from_ref[offset].quadrature != 0.) ) {
		    target[offset].phase = atan2( dif_from_ref[offset].quadrature, dif_from_ref[offset].inphase ) / DEGTORAD ;
		    target[offset].phase_cor = target[offset].phase ;
		}
		else {
		    target[offset].phase = INVALID ;
		    target[offset].phase_cor = INVALID ;
		}
            }

/* And normalize for the size of calib_ref[].av_i/q */
	for( az = 0, offset = 0 ; az < NumAzim ; az++ )
	    for( r = 0 ; r < NumRangeBins ; r++, offset++ ) {
		norm = sqrt( SQR(calib_ref[offset].av_i) + SQR(calib_ref[offset].av_q)) ;
		if( norm != 0. ) {
		    dif_from_ref[offset].inphase /= norm ;
		    dif_from_ref[offset].quadrature /= norm ;
		}
		target[offset].i = dif_from_ref[offset].inphase ;
		target[offset].q = dif_from_ref[offset].quadrature ;
	    }

	if( DebugLevel != QUIET ) {
	    fp = fopen("target_phase.n_debug", "wb") ;
	    for( offset = 0 ; offset < NumAzim * NumRangeBins ; offset++ )
		fwrite( &(target[offset].phase), sizeof(float), 1, fp ) ;
	    fclose( fp ) ;
	}
}


/**************************************************************************/

/* fit_phases():  Manages the generation of estimates of N(x,y) and dN(x,y)/dt.
   The routine directs the phase smoothing and N generation processes for
   both the current-to-previous-scan (dN/dt) and the current-to-reference (N)
   phase difference fields.
   Reason of existence:  Good quality phase data is required to compute
   refractivity data (reminder: N is derived from the derivative of phase; 
   noisy phases won't do); this is the most important (and one of the most
   painful) task of the process.  First, the current-to-previous-scan phase
   difference map is processed, then the more complex absolute phase
   difference map.
   Input: Phase difference arrays and data quality info
   Output: N and dN/dt fields and their errors
   Called by: main()
*/

#ifdef DUMP_NC
static char name_buf[300];
#endif

int fit_phases()
{
	int	az, r, offset, az2, r2, offset2 ;
	float	*inphase, *quadrature ;
	float	*smooth_i, *smooth_q, *phasefit, *phase_error ;
	float	*phasedata, *qualitydata ;
	float	guess, slope, slope_to_n ;
	char	tmp_str[200] ;
	struct F_info phases_to_fit ;
	FILE	*fp_tmp ;

/* Allocate some work arrays and set some variables */
	inphase = (float *) malloc( NumRangeBins*NumAzim*sizeof(float) ) ;
	quadrature = (float *) malloc( NumRangeBins*NumAzim*sizeof(float) ) ;
	offset = DNOutNumRanges*DNOutNumAzim ;
	if( offset < NOutNumRanges*NOutNumAzim )
	    offset = NOutNumRanges*NOutNumAzim ;
	phasefit = (float *) malloc( offset*sizeof(float) ) ;
	phase_error = (float *) malloc( offset*sizeof(float) ) ;
	smooth_i = (float *) malloc( offset*sizeof(float) ) ;
	smooth_q = (float *) malloc( offset*sizeof(float) ) ;
	phasedata = (float *) malloc( NumRangeBins*NumAzim*sizeof(float) ) ;
	qualitydata = (float *) malloc( NumRangeBins*NumAzim*sizeof(float) ) ;
	if( qualitydata == NULL ) {
	    sprintf( tmp_str, "Out of memory allocating arrays inphase -> qualitydata") ;
	    error_out( tmp_str, 3 ) ;
	}
	phases_to_fit.phase = phasedata ;	/* Fill phases_to_fit */
	phases_to_fit.inphase = inphase ;
	phases_to_fit.phasefit = phasefit ;
	phases_to_fit.phase_error = phase_error ;
	phases_to_fit.quadrature = quadrature ;
	phases_to_fit.quality = qualitydata ;
	phases_to_fit.smooth_i = smooth_i ;
	phases_to_fit.smooth_q = smooth_q ;
	phases_to_fit.num_range = NumRangeBins ;
	phases_to_fit.num_azim = NumAzim ;
	phases_to_fit.gatespacing = GateSpacing ;
	phases_to_fit.azimspacing = 360. / NumAzim ;
	phases_to_fit.range0 = RMin ;
	phases_to_fit.output_numx = CartesianX ;
	phases_to_fit.output_numy = CartesianY ;
	phases_to_fit.output_dx = CartesianResol ;
	phases_to_fit.output_dy = CartesianResol ;

/* Start with DeltaN: Set the data structure */
	if( (just_started == FALSE) ) {
	    if( DebugLevel != QUIET )
		debug_print("Fitting the difference between last two scans\n") ;

	    for( az = 0, offset = 0 ; az < phases_to_fit.num_azim ; az++ )
		for( r = 0 ; r < phases_to_fit.num_range ; r++, offset++ )
		    if( r < RMin ) {
			phasedata[offset] = target[offset].phase_diff ;
			qualitydata[offset] = 0. ;
			inphase[offset] = 0. ;
			quadrature[offset] = 0. ;
		    }
		    else {
			phasedata[offset] = target[offset].phase_diff ;
			if( target[offset].phase_diff_er == INVALID )
			    qualitydata[offset] = 0. ;
			else
			    qualitydata[offset] = sqrt(SQR(dif_prev_scan[offset].inphase)+SQR(dif_prev_scan[offset].quadrature)) ;
/*			    qualitydata[offset] = 1. / target[offset].phase_diff_er ;	#### Anything better if gnd coverage or wind unusual? */
			inphase[offset] = dif_prev_scan[offset].inphase ;
			quadrature[offset] = dif_prev_scan[offset].quadrature ;
		    }
	    phases_to_fit.ref_n = 0. ;
	    phases_to_fit.n_output = dn_array_polar ;
	    phases_to_fit.n_output_xy = dn_array_xy ;
	    phases_to_fit.n_error = dn_er_array_polar ;
	    phases_to_fit.n_error_xy = dn_er_array_xy ;
	    phases_to_fit.output_numr = DNOutNumRanges ;
	    phases_to_fit.output_numa = DNOutNumAzim ;
	    /* #### If time difference is long, use N (instead of DN) values */
	    phases_to_fit.smooth_sidelen = DNSmoothingSideLen ;
	    slope = mean_phase_slope( phases_to_fit, FALSE ) ;
	    phases_to_fit.expected_phase_range0 = phase_range0( phases_to_fit ) - RMin * slope ;
	    n_change = slope * 1000000. / GateSpacing * Wavelength / 720. ;

/* Perform the fit for DN! */
	    n_change = fit_phase_field( phases_to_fit ) ;
	    if( DebugLevel != QUIET ) {
		sprintf( tmp_str, "Delta-N between scans = %f\n", n_change) ;
		debug_print(tmp_str) ;
	    }
	}
	if( (DebugLevel & 0xF) >= FULL_DEBUG ) {
	    fp_tmp = fopen("AfterDNFit.ppm", "wb") ;
	    fprintf(fp_tmp, "P5 %d %d 255 ", phases_to_fit.output_numr, phases_to_fit.output_numa) ;
	    for( offset = 0 ; offset < phases_to_fit.output_numa*phases_to_fit.output_numr; offset++ ) {
		if( phasefit[offset] != INVALID )
		    tmp_str[0] = (char)((((int)(phasefit[offset]+360000) % 360) - 180) / 1.42) ;
		else
		    tmp_str[0] = (offset % 2) * 255 ;
		fwrite(&tmp_str[0], sizeof(char), 1, fp_tmp) ;
	    }
	    fclose(fp_tmp) ;

	    fp_tmp = fopen("DPhasesToFit.dat", "wb") ;
	    for( offset = 0 ; offset < NumAzim*NumRangeBins ; offset++ )
		fwrite(&phasedata[offset], sizeof(float), 1, fp_tmp) ;
	    fclose(fp_tmp) ;
	    fp_tmp = fopen("DPhasesToFit.ppm", "wb") ;
	    fprintf(fp_tmp, "P5 %d %d 255 ", phases_to_fit.output_numr, phases_to_fit.output_numa) ;
	    for( offset = 0 ; offset < phases_to_fit.output_numa*phases_to_fit.output_numr; offset++ ) {
		tmp_str[0] = (char)((((int)(phasedata[offset]+360000) % 360) - 180) / 1.42) ;
		fwrite(&tmp_str[0], sizeof(char), 1, fp_tmp) ;
	    }
	    fclose(fp_tmp) ;

	    fp_tmp = fopen("DPhasesQuality.ppm", "wb") ;
	    fprintf(fp_tmp, "P5 %d %d 255 ", phases_to_fit.output_numr, phases_to_fit.output_numa) ;
	    for( offset = 0 ; offset < phases_to_fit.output_numa*phases_to_fit.output_numr; offset++ ) {
		if( qualitydata[offset]> 1. )
		    tmp_str[0] = 255 ;
		else
		    tmp_str[0] = (char)(qualitydata[offset]*255.) ;
		fwrite(&tmp_str[0], sizeof(char), 1, fp_tmp) ;
	    }
	    fclose(fp_tmp) ;
	}

/* Now proceed with N calculation: reset the data structure */
	if( DebugLevel != QUIET )
	    debug_print("Now fitting the absolute phase difference\n") ;
	for( az = 0, offset = 0 ; az < phases_to_fit.num_azim ; az++ )
	    for( r = 0 ; r < phases_to_fit.num_range ; r++, offset++ )
		if( r < RMin ) {
		    phasedata[offset] = target[offset].phase ;
		    qualitydata[offset] = 0. ;
		    inphase[offset] = 0. ;
		    quadrature[offset] = 0. ;
		}
		else {
		    phasedata[offset] = target[offset].phase ;
		    if( target[offset].phase_er == INVALID )
			qualitydata[offset] = 0. ;
		    else
			qualitydata[offset] = sqrt(SQR(target[offset].i)+SQR(target[offset].q)) ;
/*			qualitydata[offset] = 1. / target[offset].phase_er ; */
		    inphase[offset] = dif_from_ref[offset].inphase ;
		    quadrature[offset] = dif_from_ref[offset].quadrature ;
		}
	phases_to_fit.ref_n = ref_n  ;
	phases_to_fit.n_output = n_array_polar ;
	phases_to_fit.n_output_xy = n_array_xy ;
	phases_to_fit.n_error = n_er_array_polar ;
	phases_to_fit.n_error_xy = n_er_array_xy ;
	phases_to_fit.output_numr = NOutNumRanges ;
	phases_to_fit.output_numa = NOutNumAzim ;
	phases_to_fit.smooth_sidelen = NSmoothingSideLen ;
	slope = mean_phase_slope( phases_to_fit, FALSE ) ;
	phases_to_fit.expected_phase_range0 = phase_range0( phases_to_fit ) - RMin * slope ;

/* Perform the fit for N! */
	new_av_n = fit_phase_field( phases_to_fit ) ;

	if( (DebugLevel & 0xF) >= FULL_DEBUG ) {
	    fp_tmp = fopen("AfterNFit.dat", "wb") ;
	    for( offset = 0 ; offset < phases_to_fit.output_numa*phases_to_fit.output_numr; offset++ )
		fwrite(&phasefit[offset], sizeof(float), 1, fp_tmp) ;
	    fclose( fp_tmp ) ;
	    fp_tmp = fopen("AfterNFit.ppm", "wb") ;
	    fprintf(fp_tmp, "P5 %d %d 255 ", phases_to_fit.output_numr, phases_to_fit.output_numa) ;
	    for( offset = 0 ; offset < phases_to_fit.output_numa*phases_to_fit.output_numr; offset++ ) {
		if( phasefit[offset] != INVALID )
		    tmp_str[0] = (char)((((int)(phasefit[offset]+360000) % 360) - 180) / 1.42) ;
		else
		    tmp_str[0] = (offset % 2) * 255 ;
		fwrite(&tmp_str[0], sizeof(char), 1, fp_tmp) ;
	    }
	    fclose(fp_tmp) ;

	    fp_tmp = fopen("PhasesToFit.dat", "wb") ;
	    for( offset = 0 ; offset < NumAzim*NumRangeBins ; offset++ )
		fwrite(&phasedata[offset], sizeof(float), 1, fp_tmp) ;
	    fclose(fp_tmp) ;
	    fp_tmp = fopen("PhasesToFit.ppm", "wb") ;
	    fprintf(fp_tmp, "P5 %d %d 255 ", phases_to_fit.output_numr, phases_to_fit.output_numa) ;
	    for( offset = 0 ; offset < phases_to_fit.output_numa*phases_to_fit.output_numr; offset++ ) {
		tmp_str[0] = (char)((((int)(phasedata[offset]+360000) % 360) - 180) / 1.42) ;
		fwrite(&tmp_str[0], sizeof(char), 1, fp_tmp) ;
	    }
	    fclose(fp_tmp) ;

	    fp_tmp = fopen("PhasesQuality.dat", "wb") ;
	    for( offset = 0 ; offset < NumAzim*NumRangeBins ; offset++ )
		fwrite(&qualitydata[offset], sizeof(float), 1, fp_tmp) ;
	    fclose(fp_tmp) ;
	    fp_tmp = fopen("PhasesQuality.ppm", "wb") ;
	    fprintf(fp_tmp, "P5 %d %d 255 ", phases_to_fit.output_numr, phases_to_fit.output_numa) ;
	    for( offset = 0 ; offset < phases_to_fit.output_numa*phases_to_fit.output_numr; offset++ ) {
		if( qualitydata[offset]> 1. )
		    tmp_str[0] = 255 ;
		else
		    tmp_str[0] = (char)(qualitydata[offset]*255.) ;
		fwrite(&tmp_str[0], sizeof(char), 1, fp_tmp) ;
	    }
	    fclose(fp_tmp) ;

	    if( (DebugLevel & 0x100) != 0 ) {
		fp_tmp = fopen("Perryton.dat", "ab") ;
		guess = hour + minute / 60. + sec / 3600. ;
		fwrite( &guess, sizeof(float), 1, fp_tmp ) ;
		for( az = 180 ; az < 190 ; az++ ) {
		    offset = az * NumRangeBins + 117 ;
		    fwrite(&phasedata[offset], sizeof(float), 20, fp_tmp) ;
		}
		for( az = 180 ; az < 190 ; az++ ) {
		    offset = az * NumRangeBins + 117 ;
		    fwrite(&phasefit[offset], sizeof(float), 20, fp_tmp) ;
		}
		fclose(fp_tmp) ;
	    }
	}

	free( qualitydata ) ;
	free( phasedata ) ;
	free( smooth_q ) ;
	free( smooth_i ) ;
	free( phase_error ) ;
	free( phasefit ) ;
	free( quadrature ) ;
	free( inphase ) ;
	return( 1 ) ;		/* #### Return validity flag */
}


/* phase_range0():  Evaluates the phase at range RMin.  Since it is at very 
   close range, azimuthal dependance of phase is unlikely and it provides a
   needed starting point to compute N from d(Phi)/dr. */

float	phase_range0( struct F_info phases_to_fit )
{
	int	az, offset ;
	float	tmp_a, tmp_b, tmp_phase ;
	float	*inphase, *quadrature ;

	inphase = phases_to_fit.inphase ;
	quadrature = phases_to_fit.quadrature ;
	tmp_a = 0. ;
	tmp_b = 0. ;
	for( az = 0 ; az < phases_to_fit.num_azim ; az++ ) {
	    offset = az * phases_to_fit.num_range + phases_to_fit.range0 ;
	    tmp_a += inphase[offset] ;
	    tmp_b += quadrature[offset] ;
	}
	if( (tmp_a != 0.) || (tmp_b != 0.) )
	    tmp_phase = atan2( tmp_b, tmp_a ) / DEGTORAD ;
	else
	    tmp_phase = INVALID ;
	return( tmp_phase ) ;
}


/* mean_phase_slope(): Compute average slope of phase data with range 
   using a pulse-pair method.
   Input: Data structure and flag determining whether average slope (TRUE)
          or initial slope (FALSE) is required.
   Output: Average slope in degrees per gatespacing.
   Called by: fit_phases() and do_smoothing() */

#define	SmearAz		10	/* When calc av slope, smooth over 10 deg */
#define	SmearAzInit	30	/* When calc init slope, smooth over 30 deg */
#define	SmearRa		2	/*  "    "   av    " , smooth over 2 bins */
#define	InitialSlopeLen	4000	/* Initial slope calculation over 5 km */

float	mean_phase_slope( struct F_info phases_to_fit, int which_slope )
{
	int	j, k, az, r, offset, max_r, smear_az ;
	float	slope_i, slope_q, weight, tmp_a, old_tmp_a, tmp_b, old_tmp_b ;
	float	slope ;
	float	*inphase, *quadrature ;

	inphase = phases_to_fit.inphase ;
	quadrature = phases_to_fit.quadrature ;
	slope_i = 0. ;
	slope_q = 0. ;
	weight = 0. ;
	if( which_slope == TRUE ) {
	    max_r = phases_to_fit.num_range - SmearRa ;
	    smear_az = (int)(SmearAz * phases_to_fit.num_azim / 360.) ;
	}
	else {
	    max_r = phases_to_fit.range0 + (int)(InitialSlopeLen/phases_to_fit.gatespacing) ;
	    smear_az = (int)(SmearAzInit * phases_to_fit.num_azim / 360.) ;
	}
	for( az = 0 ; az < phases_to_fit.num_azim ; az += smear_az ) {
	    offset = az * phases_to_fit.num_range + phases_to_fit.range0 ;
	    tmp_a = 0. ;
	    tmp_b = 0. ;
	    for( r = phases_to_fit.range0 ; r < max_r ; r += SmearRa, offset += SmearRa ) {
		old_tmp_a = tmp_a ;
		old_tmp_b = tmp_b ;
		tmp_a = 0. ;
		tmp_b = 0. ;
		for( j = 0 ; j < smear_az ; j++ )
		    for( k = 1 ; k <= SmearRa ; k++ ) {
			tmp_a += inphase[offset+j*phases_to_fit.num_range+k] ;
			tmp_b += quadrature[offset+j*phases_to_fit.num_range+k] ;
		    }
		slope_i += tmp_a * old_tmp_a + tmp_b * old_tmp_b ;
		slope_q += tmp_b * old_tmp_a - tmp_a * old_tmp_b ;
	    }
	}
	if( (slope_i != 0.) || (slope_q != 0.) )
	    slope = atan2( slope_q, slope_i ) / DEGTORAD / SmearRa ;
	else
	    slope = INVALID ;
	return( slope ) ;
}


/* fit_phase_field():  Manages the smoothing of the given phase field and
   takes care of the computation of the related N-field and of its error,
   both in polar and cartesian coordinates.
   Input: Phase difference arrays and data quality info
   Output: N and dN/dt fields
   Called by: fit_phases()
*/

#define		CovOneOverE	1000.

float fit_phase_field( struct F_info phases_to_fit )
{
	int	j, k, az, r, offset, azn, rn, azl, azr, dazn, drn ;
	int	offsetn, numazrn, count ;
	float	*phasedata, *qualitydata ;
	float	*ndata, *ndata_xy, *n_error, *n_error_xy, *phasefit ;
	float	*smooth_i, *smooth_q, *phase_error ;
	int	num_iterat, x, y, dx, dy, size_xy, off_l, off_r, polar_indx ;
	float	cost, phase_dif, coverage_mask[21][6], tmp, tmp_a, tmp_b ;
	float	quality_fore, quality_aft, slope_to_n, weight_cov ;
	float	range, azimuth, er_decorrel ;
	double	n_value, weight, quality ;
	char	tmp_str[200] ;

/* Initialize */
	ndata = phases_to_fit.n_output ;
	phasedata = phases_to_fit.phase ;
	qualitydata = phases_to_fit.quality ;
	ndata_xy = phases_to_fit.n_output_xy ;
	phase_error = phases_to_fit.phase_error ;
	n_error = phases_to_fit.n_error ;
	n_error_xy = phases_to_fit.n_error_xy ;
	smooth_i = phases_to_fit.smooth_i ;
	smooth_q = phases_to_fit.smooth_q ;
	numazrn = phases_to_fit.output_numa * phases_to_fit.output_numr ;

/* Get the smoothed phase field */
	phases_to_fit.range_slope = do_smoothing(phases_to_fit) ;
	if( (DebugLevel & 0xF) >= VOLUBLE )
	    debug_print("Smoothing complete!\n") ;

/* Compute the resulting refractivity field and its error */
	slope_to_n = 1000000. / phases_to_fit.gatespacing * Wavelength / 720. * phases_to_fit.num_range / phases_to_fit.output_numr / DEGTORAD ;
	tmp = phases_to_fit.range_slope * 1000000. / phases_to_fit.gatespacing * Wavelength / 720. + phases_to_fit.ref_n ;
	er_decorrel = 2. * phases_to_fit.gatespacing * phases_to_fit.num_range / phases_to_fit.output_numr / phases_to_fit.smooth_sidelen ;
	if( er_decorrel > 1. )  er_decorrel = 1. ;
	for( azn = 0, offsetn = 0 ; azn < phases_to_fit.output_numa ; azn++ ) {
	    ndata[offsetn] = tmp ;
	    n_error[offsetn] = VERY_LARGE ;
	    for( rn = 1, offsetn++ ; rn < phases_to_fit.output_numr - 1 ; rn++, offsetn++ ) {
		tmp_a = smooth_i[offsetn] * smooth_i[offsetn-1] + smooth_q[offsetn] * smooth_q[offsetn-1] + smooth_i[offsetn+1] * smooth_i[offsetn] + smooth_q[offsetn+1] * smooth_q[offsetn] ;
		tmp_b = smooth_q[offsetn] * smooth_i[offsetn-1] - smooth_i[offsetn] * smooth_q[offsetn-1] + smooth_q[offsetn+1] * smooth_i[offsetn] - smooth_i[offsetn+1] * smooth_q[offsetn] ;
		ndata[offsetn] = atan2(tmp_b, tmp_a) * slope_to_n + phases_to_fit.ref_n ;
		n_error[offsetn] = er_decorrel * sqrt((SQR(phase_error[offsetn+1]) + SQR(phase_error[offsetn]) + SQR(phase_error[offsetn-1])) / 6.) * slope_to_n * DEGTORAD ;
	    }
	    ndata[offsetn] = tmp ;
	    n_error[offsetn] = VERY_LARGE ;
	    offsetn++ ;
	}
	if( (DebugLevel & 0xF) >= VOLUBLE )
	    debug_print("N and N-error derived from smoothed field.\n") ; 

/* Improve estimates of refractivity through a relaxation process */
	if( DoRelax == TRUE ) {
	    relax(ndata, phases_to_fit) ;
	    if( (DebugLevel & 0xF) >= VOLUBLE )
		debug_print("Relaxation complete.\n") ;
	}

/* Compute the average field value */
	n_value = 0. ;
	weight = 0. ;
	for( offsetn = 0 ; offsetn < numazrn ; offsetn++ ) {
		n_value += (float)(offsetn % phases_to_fit.output_numr) / n_error[offsetn] * ndata[offsetn] ;
		weight += (float)(offsetn % phases_to_fit.output_numr) / n_error[offsetn] ;
	    }
	if( weight < (offsetn * phases_to_fit.output_numr / (float)VERY_LARGE) )
	    n_value = INVALID ;
	else
	    n_value /= weight ;
	if( (DebugLevel & 0xF) >= VOLUBLE )
	    debug_print("Got average value for n.\n") ;

/* Generate cartesian map */
	    for( y = 0, k = 0 ; y < phases_to_fit.output_numy ; y++ )
		for( x = 0 ; x < phases_to_fit.output_numx ; x++, k++ ) {
		    range = sqrt( SQR((y-(phases_to_fit.output_numy-1)/2.)*phases_to_fit.output_dy) + SQR((x-(phases_to_fit.output_numx -1)/2.)*phases_to_fit.output_dx) ) ;
		    r = (int)((range-FirstRange) / phases_to_fit.gatespacing / phases_to_fit.num_range * phases_to_fit.output_numr) ;
		    if( (r >= phases_to_fit.output_numr) || (r < 0) )
			ndata_xy[k] = INVALID ;
		    else {
			azimuth = atan2( (x-(phases_to_fit.output_numx -1)/2.)*phases_to_fit.output_dx, ((phases_to_fit.output_numy-1)/2.-y)*phases_to_fit.output_dy ) / DEGTORAD ;
			if( azimuth < 0. )  azimuth += 360. ;
			polar_indx = (int)(azimuth / 360. * phases_to_fit.output_numa) ;
			if( polar_indx >= phases_to_fit.output_numa )   polar_indx = phases_to_fit.output_numa - 1 ;
			polar_indx *= phases_to_fit.output_numr ;
			polar_indx += r ;
			ndata_xy[k] = ndata[polar_indx] ;
			n_error_xy[k] = n_error[polar_indx] ;
		    }
		}
	if( (DebugLevel & 0xF) >= VOLUBLE )
	    debug_print("Cartesian map completed.\n") ;

	return( (float)n_value ) ;
}


/*****************************************************************************/

/* do_smoothing():  Smoothes the phase measurements of a given map.
   This is one of the ugliest routine, primarily because of the noisy
   character of the phase data and because of the steps that were taken
   to minimize execution time of this very slow routine (look-up tables,
   pre-prepared arrays...).
   As a result, the smoothing is done very carefully and very gradually:
   1) Estimate the average d(Phase)/d(range) of the phase data to smooth;
   2) Then, going out in range, the routine generates a smooth dealiased
      "synthetic" phase field that follows the real field within the
      constraints of the real field data quality and of the smoothing area.
      In details, as range increases, it:
	a) Computes the smoothing area in polar coordinates;
	b) Do an azimuth-by-azimuth smoothing of phases by trying to remove
	   the effect of the sharp slopes of phase in range;
	c) Combine some of the azimuths together to complete the smoothing;
	d) Evaluates the quality of the smoothed phase value obtained.
   3) In data gap regions, make your best guess of what the phase should be.
   Note that although an effort is made to dealias the phase field, no use
   will be made of it because it proved to be unreliable.
   Input: Structure of information describing phase data to dealias
   Output: Smoothed phase data
   Called by: fit_phase_field()
   Important variables:
   - phases_to_fit: Structure passed from parent routine;
   - slope_a/b[]: Table of sin/cos components of slope correction with slope;
   - slope_in_range[]: Slope of d(Phase)/d(range) for each azimuth;
   - smooth_i/q[]: I/Q component of smoothed phase;
   - smooth_range: Number of gates in range used for smoothing;
   - sum_inphase[]: Used as I component of phase data smoothed for synthetic[];
   - sum_quadra[]: Used as Q component of phase data smoothed for synthetic[];
   - synthetic[]: Smoothed dealiased phase data;
*/

#define	MinAbsConsistency 4.0 /* At least some consistency required in data */

float do_smoothing( struct F_info phases_to_fit )
{
	int	j, k, az, oldaz, az2, azn, r, rn, oldrn, daz, dr ;
	int	rjump, rnjump, azjump, maxbins ;
	int	offset, off2, smooth_range, smooth_azim, best_azim ;
	int	two_smooth_azim, two_smooth_range ;
	float	*aliased, *inphase, *quadrature, *dealiased, *skill ;
	float	*smooth_i, *smooth_q, *rawquality ;
	float	*slope_a, *slope_b, *synthetic, *synthetic_error ;
	double	*sum_inphase, *sum_quadra, *sum_weight, *max_quality ;
	float	tmp_a, tmp_b, cor_i, cor_q, *guess_phase, tmp_phase ;
	float	old_tmp_a, old_tmp_b, init_slope, range_slope;
	float	*slope_in_range, *next_slope_in_range ;
	float	minconsistency, consistency, maxconsistency, weight_fact ;
	double	rawslope_a, rawslope_b, weight ;
	float	cur_phase, end_phase, quality, minquality, step_range ;
	char    tmp_str[200] ;

/* Initialize needed variables */
	if( DebugLevel != QUIET)
	    debug_print("\tEntered do_smoothing()\n") ;
	inphase = phases_to_fit.inphase ;
	quadrature = phases_to_fit.quadrature ;
	smooth_i = phases_to_fit.smooth_i ;
	smooth_q = phases_to_fit.smooth_q ;
	synthetic = phases_to_fit.phasefit ;
	synthetic_error = phases_to_fit.phase_error ;
	rawquality = phases_to_fit.quality ;

/* Estimate the initial slope (slope near range = 0) and average one
   (throughout the whole field).  These will act as anchors to prevent
   the algorithm from running away from the reasonable. */
	init_slope = mean_phase_slope( phases_to_fit, FALSE ) ;
	phases_to_fit.range_slope = init_slope ;
	range_slope = mean_phase_slope( phases_to_fit, TRUE ) ;
	if (DebugLevel != QUIET) {
	    sprintf(tmp_str, "\trange_slope = %5.3f deg/km; init_slope = %5.3f deg/km\n", range_slope * 1000. / GateSpacing, init_slope * 1000 / GateSpacing ) ;
	    debug_print(tmp_str) ;
	}

/* Compute smoothed field where enough data is available.
   In detail, we: */

/**** Allocate memory and prepare some tables */
	maxbins = phases_to_fit.num_azim * phases_to_fit.num_range ; 
	guess_phase = (float *) malloc( phases_to_fit.num_azim * sizeof(float) ) ;
	slope_in_range = (float *) calloc( phases_to_fit.num_azim, sizeof(float) ) ;
	next_slope_in_range = (float *) calloc( phases_to_fit.num_azim, sizeof(float) ) ;
        if( next_slope_in_range == NULL ) {
            sprintf( tmp_str, "Out of memory allocating array next_slope_in_range in do_smoothing") ;
            error_out( tmp_str, 3 ) ;
        }
	smooth_range = (int)(phases_to_fit.smooth_sidelen / (2.) / phases_to_fit.gatespacing) ;
	if( smooth_range <= 0 )  smooth_range = 1 ;
	two_smooth_range = 2 * smooth_range ;
	sum_inphase = (double *) malloc( (phases_to_fit.num_azim/4) * sizeof(double) ) ;
	sum_quadra = (double *) malloc( (phases_to_fit.num_azim/4) * sizeof(double) ) ;
	max_quality = (double *) malloc( (phases_to_fit.num_azim/4) * sizeof(double) ) ;
	slope_a = (float *) malloc( 360 * (two_smooth_range+1)*sizeof(float) ) ;
	slope_b = (float *) malloc( 360 * (two_smooth_range+1)*sizeof(float) ) ;
	if( (slope_b == NULL) || (sum_quadra == NULL) ) {
	    sprintf( tmp_str, "Out of memory allocating array slope_a/b") ;
            error_out( tmp_str, 3 ) ;
	}

	for( offset = 0 ; offset < phases_to_fit.output_numr*phases_to_fit.output_numa ; offset++ ) {
	    synthetic[offset] = INVALID ;
	    synthetic_error[offset] = VERY_LARGE ;
	}
	for( j = 0, k = 0 ; j < 360 ; j++ )
	    for( dr = 0 ; dr <= two_smooth_range ; dr++, k++ ) {
		slope_a[k] = cos((smooth_range-dr) * j * DEGTORAD) ;
		slope_b[k] = sin((smooth_range-dr) * j * DEGTORAD) ;
	    }
	for( az = 0 ; az < phases_to_fit.num_azim ; az++ )
	    slope_in_range[az] = init_slope ;
	memcpy( next_slope_in_range, slope_in_range, phases_to_fit.num_azim * sizeof(float)) ;

/**** For each range, we determine the smoothing area in range-azim system */
	for( rn = 0, oldrn = INVALID ; rn < phases_to_fit.output_numr ; rn++ ) {
	    r = rn * phases_to_fit.num_range / phases_to_fit.output_numr ;
	    if( r >= phases_to_fit.output_numr )
		r = phases_to_fit.output_numr - 1 ;
/*	    if( (DebugLevel & 0xF) >= FULL_DEBUG) {
		sprintf(tmp_str, "\tSynthetic, r = %d\n", r) ;
		debug_print(tmp_str) ;
	    } */
	    smooth_azim = (int)(phases_to_fit.smooth_sidelen * 360. / (phases_to_fit.azimspacing * r * phases_to_fit.gatespacing * 4. * M_PI)) ;
	    if( smooth_azim >= phases_to_fit.num_azim / 8 )
		smooth_azim = phases_to_fit.num_azim / 8 - 1 ;
	    if( smooth_azim <= 0 )  smooth_azim = 1 ;
	    two_smooth_azim = 2 * smooth_azim ;
	    minconsistency = (two_smooth_range+1) * (two_smooth_azim+1) * MinConsistency ;
	    if( minconsistency < MinAbsConsistency )
		minconsistency = MinAbsConsistency ;

/**** The smoothed value of phase(r+dr) is precomputed for the last azimuth
      (done to speed up computation afterwards when azimuths are shifted).
      The weighting function of I/Q data follows a (1-a(r-r0)^2) like function */
	    azn = phases_to_fit.output_numa - 1 ;
	    az2 = (azn * phases_to_fit.num_azim / phases_to_fit.output_numa) % phases_to_fit.num_azim ;
	    for( daz = 0 ; daz <= two_smooth_azim ; daz++ ) {
		sum_inphase[daz] = 0. ;
		sum_quadra[daz] = 0. ;
		max_quality[daz] = 0. ;
	    }
	    for( dr = 0 ; dr <= two_smooth_range ; dr++ ) {
		weight_fact = sqrt(2.) * (1. - SQR((smooth_range-dr) / ((float)smooth_range+.5))) ;
		if( (r+dr-smooth_range >= phases_to_fit.range0) && (r+dr-smooth_range < phases_to_fit.num_range) )
		    for( daz = -smooth_azim ; daz <= smooth_azim ; daz++ ) {
			az = (az2 + daz + phases_to_fit.num_azim) % phases_to_fit.num_azim ;
			offset = az * phases_to_fit.num_range + r+dr-smooth_range ;
			k = (((int)(floor(slope_in_range[az]+.5))+360000) % 360) * (2*smooth_range+1) ;
			sum_inphase[daz+smooth_azim] += weight_fact * (inphase[offset]*slope_a[k+dr] - quadrature[offset]*slope_b[k+dr]) ;
			sum_quadra[daz+smooth_azim] += weight_fact * (quadrature[offset]*slope_a[k+dr] + inphase[offset]*slope_b[k+dr]) ;
			max_quality[daz+smooth_azim] += weight_fact * rawquality[offset] ;
		    }
	    }

/**** For each azimuth, we guess what the next phase(range) should be based on
      the previously computed d(Phase)/d(range) at that azimuth.  This step will
      be needed for optimum smoothing. */
	    for( az = 0 ; az < phases_to_fit.num_azim ; az++ ) {
		azn = az * phases_to_fit.output_numa / phases_to_fit.num_azim ;
		if( oldrn == INVALID )
		    rnjump = 1 ;
		else
		    rnjump = rn - oldrn ;
		off2 = azn*phases_to_fit.num_range + rn ;
		if( r == phases_to_fit.range0 ) {
		    guess_phase[az] = phases_to_fit.expected_phase_range0 ;
		}
		else {
		    while( (synthetic[off2-rnjump] == INVALID) && (rnjump < rn) )
			rnjump++ ;
		    rjump = rnjump * phases_to_fit.output_numr / phases_to_fit.num_range ;
		    if( r-rjump >= phases_to_fit.range0 ) {
			guess_phase[az] = synthetic[off2-rnjump] + rjump * slope_in_range[az] ;
		    }
		    else {
			rjump = r - phases_to_fit.range0 ;
			guess_phase[az] = phases_to_fit.expected_phase_range0 + rjump * slope_in_range[az] ;
		    }
		}
	    }
/* #### Additional step: If num_azim > output_numa, interpolate linearly
   guess_phase[az] entries between azn rays instead of interpolating to nearest */

/**** Then compute the true smoothed I/Q values by updating
      the precomputed sum_I/Q at r+dr and combining them in azimuth and
      range taking into account the slope d(Phase)/d(range) and the shape
      of the weighting function. */
	    oldaz = (phases_to_fit.output_numa - 1) * phases_to_fit.num_azim / phases_to_fit.output_numa ;
	    for( azn = 0 ; azn < phases_to_fit.output_numa ; azn++ ) {
		az2 = azn * phases_to_fit.num_azim / phases_to_fit.output_numa ;
		azjump = (az2 - oldaz + phases_to_fit.num_azim ) % phases_to_fit.num_azim ;
		off2 = azn*phases_to_fit.output_numr + rn ;

		for( j = 0 ; j < azjump ; j++ ) {
		    for( daz = 0 ; daz < two_smooth_azim ; daz++ ) { /* Shift sums */
			sum_inphase[daz] = sum_inphase[daz+1] ;
			sum_quadra[daz] = sum_quadra[daz+1] ;
			max_quality[daz] = max_quality[daz+1] ;
		    }
		    sum_inphase[two_smooth_azim] = 0. ; /* Sum slope-corrected I/Q */
		    sum_quadra[two_smooth_azim] = 0. ;
		    max_quality[two_smooth_azim] = 0. ;

		    az = (az2 + j + smooth_azim ) % phases_to_fit.num_azim ;
		    k = (((int)(floor(slope_in_range[az]+.5))+360000) % 360) * (two_smooth_range+1) ;
		    for( dr = 0 ; dr <= two_smooth_range ; dr++ )
			if( (r+dr-smooth_range >= phases_to_fit.range0) && (r+dr-smooth_range < phases_to_fit.num_range) ) {
			    weight_fact = sqrt(2.) * (1. - SQR((smooth_range-dr) / ((float)smooth_range+.5))) ;
			    offset = off2 + (smooth_azim + j) * phases_to_fit.num_range + dr-smooth_range ;
			    if( offset >= maxbins )
				offset -= maxbins ;
			    sum_inphase[two_smooth_azim] += weight_fact * (inphase[offset]*slope_a[k+dr] - quadrature[offset]*slope_b[k+dr]) ;
			    sum_quadra[two_smooth_azim] += weight_fact * (quadrature[offset]*slope_a[k+dr] + inphase[offset]*slope_b[k+dr]) ;
			    max_quality[two_smooth_azim] += weight_fact * rawquality[offset] ;
			}
		}
		oldaz = az2 ;

		tmp_a = 0. ;	/* Combine all azimuths of smoothing area, correcting for mean phase of row */
		tmp_b = 0. ;
		maxconsistency = 0. ;
		for( daz = 0 ; daz <= two_smooth_azim ; daz++ ) {
		    az = (az2 + daz - smooth_azim + phases_to_fit.num_azim ) % phases_to_fit.num_azim ;
		    if( (r>0) && (synthetic[az*phases_to_fit.num_range+r-1] != INVALID) && (synthetic[off2-1] != INVALID) ) {
			k = (((int)(floor(guess_phase[az2] - guess_phase[az] + .5))+360000) % 360) ;
			if( k < 180 )	/* Dampen phase correction, otherwise it misbehaves */
			    k = k / 2 ;
			else
			    k = k + (360-k) / 2 ;
			k = k * (two_smooth_range+1) + smooth_range - 1 ;
			cor_i = sum_inphase[daz]*slope_a[k] - sum_quadra[daz]*slope_b[k] ;
			cor_q = sum_quadra[daz]*slope_a[k] + sum_inphase[daz]*slope_b[k] ;
		    }
		    else {
			cor_i = sum_inphase[daz] ;
			cor_q = sum_quadra[daz] ;
		    }
		    weight_fact = sqrt(2.) * (1. - SQR(smooth_azim-daz) / SQR(smooth_azim+.5)) ;
		    tmp_a += weight_fact * cor_i ;
		    tmp_b += weight_fact * cor_q ;
		    maxconsistency += weight_fact * max_quality[daz] ;
		}

/**** Save results.  Update next_slope_in_range. */
		weight_fact = (float)((two_smooth_range+1) * (two_smooth_azim+1)) ;
		smooth_i[off2] = tmp_a / weight_fact ;
		smooth_q[off2] = tmp_b / weight_fact ;
		consistency = sqrt(SQR(tmp_a) + SQR(tmp_b)) ;
		if( consistency < sqrt(2./weight_fact) * maxconsistency )
		    consistency = 0. ;
		else
		    consistency = sqrt( SQR(consistency) - 2./weight_fact*SQR(maxconsistency) ) ;
		if( weight_fact > maxconsistency )	/* Should always be true, but... */
		    quality = consistency / sqrt(maxconsistency*weight_fact) ;
		else
		    quality = consistency / maxconsistency ;
		if( quality > 0.99 )  quality = .99 ;
		if( quality < MinConsistency )  consistency = 0. ;
		if( (consistency > minconsistency) ) {
		    tmp_phase = atan2( tmp_b, tmp_a ) / DEGTORAD ;
		    while( tmp_phase - guess_phase[az2] < -180. ) tmp_phase += 360. ;
		    while( tmp_phase - guess_phase[az2] >= 180. ) tmp_phase -= 360. ;
		    synthetic[off2] = tmp_phase ;
		    synthetic_error[off2] = sqrt(-2. * log( quality ) / quality) / DEGTORAD / sqrt(maxconsistency / 2.) ;
		    if( consistency > 4. * minconsistency )
			next_slope_in_range[az2] += 2. * phases_to_fit.gatespacing / phases_to_fit.smooth_sidelen / rjump * (tmp_phase - guess_phase[az2]) ;
		    else {
			next_slope_in_range[az2] += .5 * consistency / minconsistency * phases_to_fit.gatespacing / phases_to_fit.smooth_sidelen / rjump * (tmp_phase - guess_phase[az2]) ;
			next_slope_in_range[az2] += (1. - .25 * consistency / minconsistency) * phases_to_fit.gatespacing / phases_to_fit.smooth_sidelen * (slope_in_range[(az2+phases_to_fit.num_azim-1) % phases_to_fit.num_azim] + slope_in_range[(az2+1) % phases_to_fit.num_azim] + .25 * range_slope - 2.25 * slope_in_range[az2]) ;
		    }
		}
		else {
		    next_slope_in_range[az2] += phases_to_fit.gatespacing / phases_to_fit.smooth_sidelen * (slope_in_range[(az2+phases_to_fit.num_azim-1) % phases_to_fit.num_azim] + slope_in_range[(az2+1) % phases_to_fit.num_azim] + .25 * range_slope - 2.25 * slope_in_range[az2]) ;
		    smooth_i[off2] = 0.1 * minconsistency * cos(guess_phase[az2]*DEGTORAD) ;
		    smooth_q[off2] = 0.1 * minconsistency * sin(guess_phase[az2]*DEGTORAD) ;
		}

/**** If quality is really poor, slope might be wrong; compute it the raw way */
		if( quality < sqrt(2. / weight_fact) ) {
		    rawslope_a = 0. ;
		    rawslope_b = 0. ;
		    tmp_a = 0. ;
		    tmp_b = 0. ;
		    for( dr = -two_smooth_range ; dr <= two_smooth_range ; dr++ )
			if( (r+dr >= phases_to_fit.range0) && (r+dr < phases_to_fit.num_range) ) {
			    old_tmp_a = tmp_a ;
			    old_tmp_b = tmp_b ;
			    tmp_a = 0. ;
			    tmp_b = 0. ;
			    for( daz = -two_smooth_azim ; daz <= two_smooth_azim ; daz++ ) {
				az = (az2 + daz + phases_to_fit.num_azim) % phases_to_fit.num_azim ;
				offset = az * phases_to_fit.num_range + r+dr ;
				tmp_a += inphase[offset] ;
				tmp_b += quadrature[offset] ;
			    }
			    rawslope_a += tmp_a * old_tmp_a + tmp_b * old_tmp_b ;
			    rawslope_b += tmp_b * old_tmp_a - tmp_a * old_tmp_b ;
			}
		    consistency = sqrt(SQR(rawslope_a) + SQR(rawslope_b)) ;
		    if( consistency > MinConsistency*SQR(2*two_smooth_azim+1)*(2*two_smooth_range+1) ) {
			slope_in_range[az2] = atan2(rawslope_b, rawslope_a) / DEGTORAD ;
			if( fabs(slope_in_range[az2] - range_slope) < 60. )
			    next_slope_in_range[az2] += .2 * (slope_in_range[az2] - next_slope_in_range[az2] ) ;
		    }
		}
SkipSlopeFix:

		if( r < InitialSlopeLen / phases_to_fit.gatespacing )
		    next_slope_in_range[az2] += .1 * (1. - r / ( InitialSlopeLen / phases_to_fit.gatespacing)) * (init_slope - next_slope_in_range[az2]) ;
		if( next_slope_in_range[az2] > range_slope + 60. )	/* Occasionally-needed railing */
		    next_slope_in_range[az2] = range_slope + 60. ;
		if( next_slope_in_range[az2] < range_slope - 60. )
		    next_slope_in_range[az2] = range_slope - 60. ;
	    }

	    memcpy( slope_in_range, next_slope_in_range, phases_to_fit.num_azim * sizeof(float)) ;
	    oldrn = rn ;
	}
	goto SkipImprove ;	/* #### Next part is not fully functional */

/* Smoothing task completed.  Now try to improve somewhat the crude estimates
   in the data-void regions */
	for( azn = 0 ; azn < phases_to_fit.output_numa ; azn++ ) {
	    slope_in_range[azn] = init_slope * phases_to_fit.num_range / phases_to_fit.output_numr ;
	}
	step_range = phases_to_fit.gatespacing / phases_to_fit.smooth_sidelen *phases_to_fit.num_range / phases_to_fit.output_numr ;
	if( step_range > .5 )  step_range = .5 ;

	for( azn = 0 ; azn < phases_to_fit.output_numa ; azn++ ) {
	    cur_phase = INVALID ;
	    for( rn = 0 ; rn < phases_to_fit.output_numr ; rn++ ) {
		offset = azn * phases_to_fit.output_numr + rn ;
		if(synthetic[offset] != INVALID) {
		    cur_phase = synthetic[offset] ;
		    if((rn>0) && (synthetic[offset-1] != INVALID))
			slope_in_range[azn] += step_range * (synthetic[offset]-synthetic[offset-1]-slope_in_range[azn]) ;
		}
		else { /* If data gap, find neighbors in range and interpolate */
		    next_slope_in_range[azn] = slope_in_range[azn] ;
		    dr = INVALID ;
		    for( r = phases_to_fit.output_numr-1 ; r > rn+1 ; r-- ) {
			off2 = azn * phases_to_fit.output_numr + r ;
			if( (synthetic[off2] != INVALID) && (synthetic[off2-1] != INVALID) ) {
			    next_slope_in_range[azn] += step_range * (synthetic[off2]-synthetic[off2-1]-next_slope_in_range[azn]) ;
			    dr = r ;
			}
		    }
		    if( dr != INVALID ) {
			off2 = azn * phases_to_fit.output_numr + dr - 1 ;
			if( cur_phase != INVALID ) {
			    tmp_phase = cur_phase + (dr - r) * (slope_in_range[azn]+next_slope_in_range[azn]) / 2. ;
			    end_phase = synthetic[off2-1] ;
			    while( tmp_phase - end_phase < -180. ) end_phase -= 360. ;
			    while( tmp_phase - end_phase >= 180. ) end_phase += 360. ;
			    slope_in_range[azn] = (end_phase - cur_phase) / (dr - r) ;
			    for( ; rn < dr - 1 ; rn++, offset++ ) {
				cur_phase += slope_in_range[azn] ;
				synthetic[offset] = cur_phase ;
				smooth_i[offset] = 0.1 * minconsistency * cos(cur_phase*DEGTORAD) ;
				smooth_q[offset] = 0.1 * minconsistency * sin(cur_phase*DEGTORAD) ;
			    }
			}
			else {
			    slope_in_range[azn] = (init_slope+next_slope_in_range[azn]) / 2. ;
			    off2 = azn * phases_to_fit.output_numr + dr - 1 ;
			    for( ; rn < dr - 1 ; rn++, offset++ ) {
				cur_phase = synthetic[off2] - (dr-1-rn) * slope_in_range[azn] ;
				synthetic[offset] = cur_phase ;
				smooth_i[offset] = 0.1 * minconsistency * cos(cur_phase*DEGTORAD) ;
				smooth_q[offset] = 0.1 * minconsistency * sin(cur_phase*DEGTORAD) ;
			    }
			}
		    }
		    else
			rn = phases_to_fit.output_numr ;
		}
	    }
	}

SkipImprove:
/* Task completed! */
	if( (DebugLevel & 0x200) != 0 ) {
	    sprintf(tmp_str, "synth_final.%02d%02d.n_debug", hour, minute) ;
	    fp = fopen(tmp_str, "wb" ) ;
	    for( offset = 0 ; offset < NumRangeBins * NumAzim ; offset++ ) {
		fwrite( &synthetic[offset], sizeof(float), 1, fp ) ;
	    }
	    fclose( fp ) ;
	    debug_print("\tSynthetic (phasefit) field completed\n") ;
	}

	free( slope_b ) ;
	free( slope_a ) ;
	free( sum_quadra ) ;
	free( sum_inphase ) ;
	free( next_slope_in_range ) ;
	free( slope_in_range ) ;
	free( guess_phase ) ;
	return( phases_to_fit.range_slope ) ;
}


/*****************************************************************************/

/* relax():  Performs a quality-dependent iterative smoothing of the
   refractivity field but constrained by the smoothed phase field.
   Uses an iterative "diffusion and relaxation" approach to the problem, 
   even if it takes an eternity:
   First the value of N are nudged to match better the phase data,
   then the field is smoothed.  As a result, the regions where data are
   available gradually percolate into regions where they are not.
   Input: Phases_to_fit structure and initial N field data.
   Output: Updated N field data.
   Called by: fit_phase_field(). */

#define		MaxIterat	1000	/* 1000 iterations absolute maximum */

int relax( float *ndata, struct F_info phases_to_fit )
{
	int     j, x, y, dx, dy, count, polar_indx ;
	int	num_range, num_azim ;
	long    k, k1, k2 ;
	int     az, az2, r, dr, prev_r, d_az, *st_d_az ;
	float   tmp, equiv_dist, force_factor ;
	float   *smooth, *work_array, *phase_array, *phase_error ;
        double  forcing, old_forcing ;
	int	count_forcing, num_iterat, iterat ;
	float	prev_phase, cur_phase, delta_phase, slope_to_n ;
	char	tmp_str[80] ;

/* Initialize variables and arrays */
	if (DebugLevel != QUIET)
	    debug_print( "Now starting relaxation\n" ) ;
	num_range = phases_to_fit.output_numr ;
	num_azim = phases_to_fit.output_numa ;
	phase_array = phases_to_fit.phasefit ;
	phase_error = phases_to_fit.phase_error ;

	smooth = (float *) calloc( 21 * num_range, sizeof(float) ) ;
	work_array = (float  *) malloc( num_azim * num_range * sizeof(float) ) ;
	if( work_array == NULL ) {
	    sprintf( tmp_str, "Relaxation step skipped: out of memory for work_array" ) ;
	    error_out( tmp_str, FALSE ) ;
	    if( smooth != NULL )
		free( smooth ) ;
	    return( FALSE ) ;
	}
	st_d_az = (int *) malloc( num_range * sizeof(int) ) ;
	if( st_d_az == NULL ) {
	    sprintf( tmp_str, "Relaxation step skipped: out of memory for st_d_az" ) ;
	    error_out( tmp_str, FALSE ) ;
	    free( work_array ) ;
	    free( smooth ) ;
	    return( FALSE ) ;
	}
	slope_to_n = 1000000. / phases_to_fit.gatespacing * Wavelength / 720. * phases_to_fit.num_range / phases_to_fit.output_numr ;
	force_factor = SQR(phases_to_fit.gatespacing * phases_to_fit.num_range / phases_to_fit.output_numr / phases_to_fit.smooth_sidelen) ;	/* Sets how field follows data (>0); small -> loose */
	num_iterat = (int)(0.5 * SQR( 0.5 * phases_to_fit.smooth_sidelen * phases_to_fit.output_numr / phases_to_fit.num_range / phases_to_fit.gatespacing )) ;
	if(num_iterat < 3)  num_iterat = 3 ;
	if(num_iterat > MaxIterat)  num_iterat = MaxIterat ;

/* Build the range-dependant smoothing function used during the diffusion part 
   of the relaxation process.  This is made to have a relaxation that is more
   or less similar in the r and phi direction despite the polar nature of the data. */
	for( r = 0 ; r < num_range ; r++ ) {
	    equiv_dist = (float)(2 * r) * M_PI / (float)num_azim ; 
	    if( equiv_dist < 1. )
		for ( d_az = 10, tmp = -1. ; d_az <= 20 ; d_az++ ) {
		    if(((d_az+.5)-10.)*equiv_dist < 1.5) {
			smooth[21*r + d_az] = 1. ;
			smooth[21*r + 20-d_az] = 1. ;
			tmp += 2. ;
			st_d_az[r] = 20 - d_az ;
		    }
		    else if(((d_az-.5)-10)*equiv_dist < 1.5) {
			smooth[21*r + d_az] = (1.5 - ((d_az-.5)-10.)*equiv_dist) / equiv_dist ;
			smooth[21*r + 20-d_az] = smooth[21*r + d_az] ;
			tmp += 2. * smooth[21*r + d_az] ;
			st_d_az[r] = 20 - d_az ;
		    }
		}
	    else {
		smooth[21*r + 10] = 1. ;
		smooth[21*r + 11] = 1. / sqrt(equiv_dist) ;  /* #### sqrt for faster diffusion at far range with limited data */
		smooth[21*r + 9] = smooth[21*r + 11] ;
		tmp = 1. + 2. * smooth[21*r + 9] ;
		st_d_az[r] = 9 ;
	    }
	    for ( d_az = 0 ; d_az <= 20 ; d_az++ )
		smooth[21*r + d_az] *= 3. / tmp ;
	}

/* Main loop: perform relaxation: nudge each path towards the right sum (account for errors) */
	iterat = 0 ;
	do {
	    memcpy( work_array, ndata, sizeof( float ) * num_range * num_azim ) ;
	    if( iterat < 1 ) {
		old_forcing = VERY_LARGE ;
		forcing = 0.5 * VERY_LARGE ;
		count_forcing = 1 ;
	    }
	    else {
		old_forcing = forcing ;
		forcing = 0. ;
		count_forcing = 0 ;
		for( az = 0 ; az < num_azim ; az++ ) {
		    prev_phase = phases_to_fit.expected_phase_range0 - phases_to_fit.range0 * phases_to_fit.range_slope ;
		    cur_phase = prev_phase ;
		    prev_r = 0 ;
		    k = az*num_range + 1 ;
		    for( r = 1 ; r < num_range ; r++, k++ ) {
			cur_phase += (ndata[k]- phases_to_fit.ref_n) / slope_to_n ;
			if( phase_array[k] != INVALID ) {
			    delta_phase = (float)((int)(100. * (phase_array[k] - cur_phase) + 36000000) % 36000) / 100. ;
			    if( delta_phase > 180. )  delta_phase -= 360. ;
			    delta_phase *= atan( fabs(delta_phase)*force_factor / phase_error[k] ) / M_PI * 2. ;  /* Error-corrected value */
			    tmp = delta_phase * slope_to_n / (float)(r - prev_r) ;
			    for( dr = 0 ; dr > prev_r-r ; dr-- )
				work_array[k+dr] += tmp ;
//			    forcing += fabs(tmp*(r - prev_r)) ;
//			    count_forcing += r - prev_r ;
			    cur_phase += delta_phase ;
			    prev_r = r ;
			}
		    }
		}
	    }

/* And then diffuse */
	    for( az = 0, k = 0 ; az < num_azim ; az++ )
		for( r = 0 ; r < num_range ; r++, k++ )  {
		    count = 3 ;
		    tmp = 0 ;
		    if ( r != 0 ) {
			tmp = work_array[k-1] ; /* #### May have more weight than local point!!! */
			count++ ;
		    }
		    if ( r != num_range - 1 ) {
			tmp += work_array[k+1] ;
			count++ ;
		    }
		    for( d_az = st_d_az[r] ; d_az <= 20-st_d_az[r] ; d_az++ ) {
			az2 = (az - 10 + d_az + num_azim) % num_azim ;
			tmp += smooth[21*r + d_az] * work_array[az2*num_range+r] ;
		    }
		    forcing += fabs(tmp / count - ndata[k]) ;
		    count_forcing++ ;
		    ndata[k] = tmp / count ;
		}

/* Check how much change there was.  If changes were limited, call it quit */
            forcing /= count_forcing ;
            iterat++ ;
	    if( (DebugLevel & 0xF) >= FULL_DEBUG ) {
		sprintf(tmp_str, "%d\t%f\n", iterat, forcing) ;
		debug_print(tmp_str) ;
	    }
	} while (( iterat < num_iterat )) ;

	free( st_d_az ) ;
	free( work_array ) ;

	return( TRUE ) ;
}
