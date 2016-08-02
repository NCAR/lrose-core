/*
 * svissr_constants.h
 *
 */

#ifndef __SVISSR_CONSTANTS_H__
#define __SVISSR_CONSTANTS_H__

// if true, will allow use of bad crc lines to complete navcal data
// don't use it, it has an excellent chance of crashing due to bad 
// values e.g. subcommutation id
#define SVISSR_ALLOW_BAD_CRC_DATA false

const int svissrMaxLines = 2290;
const int svissrMaxElements = 2291;

#define SVISSR_DECOMMUTATION_GROUPS 25  /* Number of unique commutated lines */

#define SVISSR_SMAP_BLOCK_LENGTH 100
#define SVISSR_MANAM_BLOCK_LENGTH 410
#define SVISSR_OANDA_BLOCK_LENGTH 128
#define SVISSR_CALIBRATION_BLOCK_LENGTH 256



enum gmschannel {gms_ch_undef, gms_ch_ir1, gms_ch_ir2, gms_ch_wv, 
	gms_ch_vis, gms_ch_blend, gms_ch_rgb};

#endif

