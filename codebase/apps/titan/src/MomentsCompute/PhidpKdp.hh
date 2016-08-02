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
#ifndef POL_ATT_CORR_H
#define POL_ATT_CORR_H

#include <cmath>

#ifndef DBS
#define DBS(x) (10.0*log10(x))
#endif

#ifndef IDBS
#define IDBS(x) (pow(10.0, (x)/10.0))
#endif

#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif

#ifndef CLAMP
#define CLAMP(x,min,max) ((x)<(min)?(min):(((x)>(max))?(max):(x)))
#endif

#ifndef RADIAN_TO_DEG
#define RADIAN_TO_DEG(x) (57.2957795130823208768*(x))
#endif

#ifndef DEG_TO_RADIAN
#define DEG_TO_RADIAN(x) (0.01745329251994329577*(x))
#endif

#ifndef EARTH_RADIUS
#define EARTH_RADIUS		6378137.0
#endif

#ifndef K_E
#define K_E					(4.0 / 3.0)
#endif

#define PAC_MGOOD 10
#define PAC_MBAD 5

#define PAC_SYSPHASE_N_GOOD 10
#define PAC_SYSPHASE_SD_THRESHOLD 10.0
#define PAC_SYSPHASE_MODECHANGE_SD_THRESHOLD 20.0
#define PAC_SYSPHASE_N_OK_THRESHOLD	10
#define PAC_SYSPHASE_N_BACKTRACK 5
#define PAC_SYSPHASE_SNR_THRESHOLD 1.5
#define PAC_WRAP_THRESHOLD_EXTRA 10.0
#define PAC_SYSPHASE_MIN_SLOPE -5.0
#define PAC_SYSPHASE_MAX_SLOPE 20.0
#define PAC_SYSPHASE_EXTRA_SNR_THRESHOLD 3.0

#define PAC_DATAMASK_GOOD_RNG 1.0
#define PAC_DATAMASK_BAD_RNG 0.5
#define PAC_MIN_DATAMASK_GOOD_GATES 10
#define PAC_MIN_DATAMASK_BAD_GATES 5
#define PAC_DATAMASK_PHIDP_STD_THRESHOLD 20
#define PAC_DATAMASK_SNR_THRESHOLD 3
#define PAC_DATAMASK_RHOHV_THRESHOLD 0.9
#define PAC_DATAMASK_Z_HAIL_THRESHOLD 30.0
#define PAC_DATAMASK_RHOHV_HAIL_THRESHOLD 0.6
#define PAC_DATAMASK_PHIDP_STD_HAIL_THRESHOLD 15

#define PAC_SMOOTHING_ADAPT_ITERS 10
#define PAC_SMOOTHING_DELTA_THRESHOLD 5.0

#define PAC_KDP_N_KDP_MAX 30
#define PAC_KDP_BAND1_Z 35.0
#define PAC_KDP_BAND2_Z 45.0
#define PAC_KDP_N_KDP_1 30
#define PAC_KDP_N_KDP_2 20
#define PAC_KDP_N_KDP_3 10

#define PAC_DATAMASK_BAD 0
#define PAC_DATAMASK_GOOD 1
#define PAC_DATAMASK_HAIL 2

#define PAC_GAS_CORR_MAXELEV 10.0
#define PAC_GAS_CORR_MAXRNG 200

#define PAC_B_EXP 0.78
#define PAC_BRIGHTBAND_L 1.5
#define PAC_BRIGHTBAND_U 2.5

#define KDP_PROC_START_OFFSET 5

typedef struct phidp_filter {
	double gate_spacing;
	double gain;
	int length;
	double *coeffs;
} phidp_filter_t;

/**
\brief Method used to unwrap PHIDP
*/
typedef enum pac_unfold_method {
	PAC_UNFOLD_AUTO = 0, /**< Automatically identify system phase */
	PAC_UNFOLD_USE_SYS_PHIDP /**< Use the specified system phase */
} pac_unfold_method_t;

void pac_get_phidp_sd(int gates,
	double *PHIDP, int PHIDP_stride,
	double *PHIDP_sd, int PHIDP_sd_stride,
	int sd_n);

double pac_unfold_phidp(int gates,
	const double *PHIDP, int PHIDP_stride,
	const double *RHOHV, int RHOHV_stride,
	const double *PHIDP_sd, int PHIDP_sd_stride,
	const double *snr, int snr_stride,
	const double *range, int range_stride,
	pac_unfold_method_t method, int min_gate,
	double system_phase, double wrap_threshold,
	double *PHIDP_u, int PHIDP_u_stride);
void pac_get_datamask(int gates,
	const double *Z, int Z_stride,
	const double *PHIDP, int PHIDP_stride,
	const double *RHOHV, int RHOHV_stride,
	const double *snr, int snr_stride,
	const double *range, int range_stride,
	int start_gate,
	char *datamask, int datamask_stride);
int pac_smooth_phidp(int gates,
	const double *PHIDP, int PHIDP_stride,
	const double *range, int range_stride,
	const char *datamask, int datamask_stride,
	double *PHIDPinterp, int PHIDPinterp_stride,
	double *PHIDPf, int PHIDPf_stride);
void pac_calc_kdp(int gates,
	const double *PHIDP, int PHIDP_stride,
	const double *Z, int Z_stride,
	const double *range, int range_stride,
	double *KDP, int KDP_stride);
void pac_get_true_height(int gates, double elevation,
	const double *range_km, int range_km_stride,
	double *truehgt_km, int truehgt_km_stride);
void pac_gaseous_attn_correction(int gates, double elevation,
	const double *Z, int Z_stride,
	const double *range_km, int range_km_stride,
	double *Zc, int Zc_stride);

#endif /* POL_ATT_CORR_H */
