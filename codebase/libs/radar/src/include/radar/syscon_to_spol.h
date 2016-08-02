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
/******************************************************************/
/**
 *
 * /file <syscon_to_spol.h>
 *
 * Shared memory structures for passing data from SysCon to
 * Spol applictaions via shared memory.
 * iwrf time series.
 *
 * CSU-CHILL/NCAR
 * IWRF - INTEGRATED WEATHER RADAR FACILILTY
 *
 * Mike Dixon, RAL, NCAR, Boulder, CO
 * July 2010
 *
 *********************************************************************/

#ifndef _SYSCON_TO_SPOL_H_
#define _SYSCON_TO_SPOL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <radar/iwrf_data.h>
#include <radar/iwrf_rsm.h>

  typedef struct {

    iwrf_radar_info_t radarInfo;
    iwrf_scan_segment_t scanSegment;
    iwrf_ts_processing_t tsProcessing;
    iwrf_xmit_power_t xmitPower;

    iwrf_event_notice_t startOfSweep;
    iwrf_event_notice_t endOfSweep;
    iwrf_event_notice_t startOfVolume;
    iwrf_event_notice_t endOfVolume;

  } syscon_spol_shmem_t;

#ifdef __cplusplus
}
#endif

#endif

