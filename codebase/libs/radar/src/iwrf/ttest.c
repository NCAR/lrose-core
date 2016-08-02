/***********************************************************************
 * ttest.c
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * August 1996
 *
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "../include/radar/iwrf_data.h"

int main(int argc, char **argv)

{

  fprintf(stderr, "iwrf_radar_info_t size, doubles: %u, %g\n",
          sizeof(iwrf_radar_info_t),
          sizeof(iwrf_radar_info_t) / 8.0);

  fprintf(stderr, "iwrf_scan_segment_t size, doubles: %u, %g\n",
          sizeof(iwrf_scan_segment_t),
          sizeof(iwrf_scan_segment_t) / 8.0);

  fprintf(stderr, "iwrf_antenna_correction_t size, doubles: %u, %g\n",
          sizeof(iwrf_antenna_correction_t),
          sizeof(iwrf_antenna_correction_t) / 8.0);

  fprintf(stderr, "iwrf_ts_processing_t size, doubles: %u, %g\n",
          sizeof(iwrf_ts_processing_t),
          sizeof(iwrf_ts_processing_t) / 8.0);

  fprintf(stderr, "iwrf_xmit_power_t size, doubles: %u, %g\n",
          sizeof(iwrf_xmit_power_t),
          sizeof(iwrf_xmit_power_t) / 8.0);

  fprintf(stderr, "iwrf_xmit_sample_t size, doubles: %u, %g\n",
          sizeof(iwrf_xmit_sample_t),
          sizeof(iwrf_xmit_sample_t) / 8.0);

  fprintf(stderr, "iwrf_calibration_t size, doubles: %u, %g\n",
          sizeof(iwrf_calibration_t),
          sizeof(iwrf_calibration_t) / 8.0);

  fprintf(stderr, "iwrf_event_notice_t size, doubles: %u, %g\n",
          sizeof(iwrf_event_notice_t),
          sizeof(iwrf_event_notice_t) / 8.0);

  fprintf(stderr, "iwrf_phasecode_t size, doubles: %u, %g\n",
          sizeof(iwrf_phasecode_t),
          sizeof(iwrf_phasecode_t) / 8.0);

  fprintf(stderr, "iwrf_xmit_info_t size, doubles: %u, %g\n",
          sizeof(iwrf_xmit_info_t),
          sizeof(iwrf_xmit_info_t) / 8.0);

  fprintf(stderr, "iwrf_ts_pulse_hdr_t size, doubles: %u, %g\n",
          sizeof(iwrf_pulse_header_t),
          sizeof(iwrf_pulse_header_t) / 8.0);

  fprintf(stderr, "iwrf_rvp8_pulse_header_t size, doubles: %u, %g\n",
          sizeof(iwrf_rvp8_pulse_header_t),
          sizeof(iwrf_rvp8_pulse_header_t) / 8.0);

  fprintf(stderr, "iwrf_rvp8_ops_info_t size, doubles: %u, %g\n",
          sizeof(iwrf_rvp8_ops_info_t),
          sizeof(iwrf_rvp8_ops_info_t) / 8.0);


  fprintf(stderr, "iwrf_sync_t size, doubles: %u, %g\n",
          sizeof(iwrf_sync_t),
          sizeof(iwrf_sync_t) / 8.0);

  fprintf(stderr, "IWRF_SYNC_ID: %d, %x\n", IWRF_SYNC_ID, IWRF_SYNC_ID);
  fprintf(stderr, "IWRF_RADAR_INFO_ID: %d, %x\n", IWRF_RADAR_INFO_ID, IWRF_RADAR_INFO_ID);
  fprintf(stderr, "IWRF_SCAN_SEGMENT_ID: %d, %x\n", IWRF_SCAN_SEGMENT_ID, IWRF_SCAN_SEGMENT_ID);
  fprintf(stderr, "IWRF_ANTENNA_CORRECTION_ID: %d, %x\n", IWRF_ANTENNA_CORRECTION_ID, IWRF_ANTENNA_CORRECTION_ID);
  fprintf(stderr, "IWRF_TS_PROCESSING_ID: %d, %x\n", IWRF_TS_PROCESSING_ID, IWRF_TS_PROCESSING_ID);
  fprintf(stderr, "IWRF_XMIT_POWER_ID: %d, %x\n", IWRF_XMIT_POWER_ID, IWRF_XMIT_POWER_ID);
  fprintf(stderr, "IWRF_XMIT_SAMPLE_ID: %d, %x\n", IWRF_XMIT_SAMPLE_ID, IWRF_XMIT_SAMPLE_ID);
  fprintf(stderr, "IWRF_CALIBRATION_ID: %d, %x\n", IWRF_CALIBRATION_ID, IWRF_CALIBRATION_ID);
  fprintf(stderr, "IWRF_EVENT_NOTICE_ID: %d, %x\n", IWRF_EVENT_NOTICE_ID, IWRF_EVENT_NOTICE_ID);
  fprintf(stderr, "IWRF_PHASECODE_ID: %d, %x\n", IWRF_PHASECODE_ID, IWRF_PHASECODE_ID);
  fprintf(stderr, "IWRF_XMIT_INFO_ID: %d, %x\n", IWRF_XMIT_INFO_ID, IWRF_XMIT_INFO_ID);
  fprintf(stderr, "IWRF_PULSE_HEADER_ID: %d, %x\n", IWRF_PULSE_HEADER_ID, IWRF_PULSE_HEADER_ID);
  fprintf(stderr, "IWRF_RVP8_PULSE_HEADER_ID: %d, %x\n", IWRF_RVP8_PULSE_HEADER_ID, IWRF_RVP8_PULSE_HEADER_ID);
  fprintf(stderr, "IWRF_RVP8_OPS_INFO_ID: %d, %x\n", IWRF_RVP8_OPS_INFO_ID, IWRF_RVP8_OPS_INFO_ID);

  fprintf(stderr, "IWRF_SYNC_VAL_00: %d, %x\n", IWRF_SYNC_VAL_00, IWRF_SYNC_VAL_00);
  fprintf(stderr, "IWRF_SYNC_VAL_01: %d, %x\n", IWRF_SYNC_VAL_01, IWRF_SYNC_VAL_01);

  iwrf_sync_t sync;
  sync.magik[0] = IWRF_SYNC_VAL_00;
  sync.magik[1] = IWRF_SYNC_VAL_01;

  fprintf(stderr, "sync.magik[0]: %d, %x\n", sync.magik[0], sync.magik[0]);
  fprintf(stderr, "sync.magik[1]: %d, %x\n", sync.magik[1], sync.magik[1]);

  return 0;

}
