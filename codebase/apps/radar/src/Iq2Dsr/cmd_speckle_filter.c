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
static void run_speckle_filter(int n_gates,
                               double *cmd,
                               int *cmd_flag,
                               int run_len,
                               double min_valid_cmd);
         
/***************************************************************************
 * Apply speckle filter for various length speckle
 * starting with the longest and working down.
 *
 *   n_gates: number of gates in beam
 *   cmd: cmd value at each gate
 *   cmd_flag: cmd flag at each gate
 *   n_thresholds: number of modified cmd thresholds
 *   run_len: array of run lengths for each modified threshold
 *   min_valid_cmd: array of modified thresholds
 */

void apply_cmd_speckle_filter(int n_gates,
                              double *cmd,
                              int *cmd_flag,
                              int n_thresholds,
                              int *run_len,
                              double *min_valid_cmd)
  
{

  int max_run_len;
  int this_run_len;

  /* compute the max specified speckle run length */

  max_run_len = 0;
  for (int ii = 0; ii < n_thresholds; ii++) {
    if (run_len[ii] > max_run_len) {
      max_run_len = run_len[ii];
    }
  }

  /* for each specified speckle run length, apply the speckle filter */

  this_run_len = max_run_len;
  while (this_run_len > 0) {
    for (int ii = 0; ii < n_thresholds; ii++) {
      /* apply threshold for this run len */
      if (run_len[ii] == this_run_len) {
        run_speckle_filter(n_gates, cmd, cmd_flag,
                           this_run_len, min_valid_cmd[ii]);
        break;
      }
    }
    this_run_len--;
  }

}

/***************************************************************************
 *
 * Run speckle filter for a given length and threshold.
 * 
 * If a run is less than or equal to the min length,
 * apply the more stringent CMD threshold to modify
 * the flag field.
 *
 *   n_gates: number of gates in beam
 *   cmd: cmd value at each gate
 *   cmd_flag: cmd flag at each gate
 *   run_len: run length for modified threshold
 *   min_valid_cmd: modified threshold
 */

static void run_speckle_filter(int n_gates,
                               double *cmd,
                               int *cmd_flag,
                               int run_len,
                               double min_valid_cmd)
         
{

  int count = 0;
  /* loop through all gates */
  for (int ii = 0; ii < n_gates; ii++) {
    /* check for CMD flag status */
    if (cmd_flag[ii]) {
      /* set, so count up length of run */
      count++;
    } else {
      /* not set, end of run */
      if (count <= run_len) {
        /* run too short, indicates possible speckle */
        for (int jj = ii - count; jj < ii; jj++) {
          /* check for CMD values below threshold */
          if (cmd[jj] < min_valid_cmd) {
            cmd_flag[jj] = false;
          }
        }
      }
      count = 0;
    }
  } /* ii */

}

