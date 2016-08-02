#ifndef RSTS_SUN_POS_WAS_INCLUDED
#define RSTS_SUN_POS_WAS_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

/**
 * copyright
 * @file
 * @author Team RSIS
 * @date 04-07-2004
 * @brief This file interfaces to the NOVAS libraries to calculate the position of the Sun for Suncheck tests.
 *
 * Change Log:
 *
 * Programmer Name--------------------Description-----------------Date
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "novas.h"

/* Function Declarations */

extern void rsts_SunNovasComputePos(site_info here, double deltat, double *SunAz, double *SunEl,double *distanceAU);
extern void rsts_SunNovasComputePosAtTime(site_info here, double deltat, double *SunAz, double *SunEl, time_t time );

#ifdef __cplusplus
}
#endif

#endif

