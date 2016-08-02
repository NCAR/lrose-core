/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/**********************************************************************
 * zr.h
 *
 * ZR parameters
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * Oct 1997
 *
 **********************************************************************/

#ifndef zr_h
#define zr_h

#ifdef __cplusplus
extern "C" {
#endif

#include <dataport/port_types.h>

#define ZR_NBYTES_32 40

typedef enum {
  ZR_MODE_CALIBRATED, /* valid calibration */
  ZR_MODE_DEFAULT,    /* using defaults */
  ZR_MODE_COASTING,   /* coasting from previous calibration */
  ZR_MODE_N           /* number of modes */
} zr_params_mode_t;

typedef struct {

  si32 mode; /* see zr_params_mode_t */
  si32 accum_period; /* secs */
  si32 n_valid_pairs; /* number of valid Z-R pairs used */
  si32 z_to_gauge_lag; /* Z to gauge lag (secs) */
  si32 calib_time; /* time of last calibration - used to
		    * determine if coasting is valid, or
		    * whether we should go to default values */
  si32 spare[2];
  fl32 mean_gauge_accum; /* mean gauge-based accumulation (mm) */
  fl32 coeff; /* ZR coefficient */
  fl32 expon; /* ZR exponent */

} zr_params_t;

/*
 * prototypes
 */

/****************************
 * zr_params_to_BE
 *
 * Swaps ZR params to BigEnd in place
 */

extern void zr_params_to_BE(zr_params_t *zrp);

/****************************
 * zr_params_from_BE
 *
 * Swaps ZR params from BigEnd in place
 */

extern void zr_params_from_BE(zr_params_t *zrp);

/****************************
 * zr_params_print
 *
 * Prints out struct
 */

extern void zr_params_print(FILE *out, const char *spacer, zr_params_t *zrp);

#ifdef __cplusplus
}
#endif

#endif

