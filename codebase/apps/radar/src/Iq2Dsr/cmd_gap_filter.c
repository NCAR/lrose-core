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
/***************************************************************************
 * apply gap in-fill filter to CMD flag - C implementation
 *
 * After CMD is run, the CMD flag field tends to have gaps
 * which really should be filtered, since they are surrounded by
 * filtered gates. This infill process is designed to fill the
 * gaps in the flag field.
 *
 * Initialization:
 *
 * A template of weights, of length 6, is computed with the following values:
 *   1, 1/2, 1/3, 1/4,, 1/5, 1/6
 *
 * Computing the forward sum of weights:
 *
 *   For each gate at which the flag is not yet set, compute the sum of
 *   the weights for each of the previous 6 gates at which the
 *   flag field is set.
 *   A weight of 1 applies to the previous gate, 1/2 applies to the
 *   second previous gate, etc.
 * 
 * Computing the reverse sum of weights:
 *
 *   For each gate at which the flag is not yet set, compute the sum of
 *   the weights for each of the next 6 gates at which the
 *   flag field is set.
 *   The weights are used in the reverse sense, i.e 1 applies to the next
 *   gate, 1/2 applies to the second next gate etc/
 *
 * The sum-of-weights threshold defaults to 0.35
 * 
 * The threshold test will succeed with:
 *    a single adjacent flag gate, or
 *    2 consecutive gates starting 2 gates away, or
 *    3 consecutive gates starting 3 gates away, or
 *    4 consecutive gates starting 4 gates away, etc.
 *
 * The test will also succeed with a mixture of flagged and unflagged gates at
 * various distances from the test gate.
 *
 * Checking the sums against the threshold:
 *
 * If both the forward sum and the reverse sum exceed the threshold, then
 * this gate is considered likely to have clutter, and the cmd_flag is
 * set.
 */

void apply_cmd_flag_gap_filter(int n_gates,
                               double *cmd,
                               int *cmd_flag,
                               int n_weights,
                               double sum_wt_threshold)

{

  /* set up weights for distance differences */

  double *weights = (double *) malloc(n_weights * sizeof(double));
  for (int ii = 0; ii < n_weights; ii++) {
    weights[ii] = 1.0 / (ii + 1.0);
  }
                     
  /* allocate arrays for sum of weights with the flag field */
  
  double *sumWtsForward = (double *) malloc(n_gates * sizeof(double));
  double *sumWtsReverse = (double *) malloc(n_gates * sizeof(double));

  /* initialize */

  int done = 0, count = 0;
  int igate, kgate;
  int jj;
  
  while (!done) {

    done = 1;
    
    /* compute sum in forward direction */
    
    for (igate = 0; igate < n_gates; igate++) {
      if (cmd_flag[igate]) {
        sumWtsForward[igate] = 1.0;
        continue; /* flag already set, don't need to modify this gate */
      }
      sumWtsForward[igate] = 0.0;
      for (jj = 0; jj < n_weights; jj++) {
        kgate = igate - jj - 1;
        if (kgate >= 0) {
          if (cmd_flag[kgate]) {
            sumWtsForward[igate] += weights[jj] * cmd[kgate];
          }
        }
      }
    }
    
    /* compute sum in reverse direction */
    
    for (igate = n_gates - 1; igate >= 0; igate--) {
      if (cmd_flag[igate]) {
        sumWtsReverse[igate] = 1.0;
        continue; /* flag already set, don't need to modify this gate */
      }
      sumWtsReverse[igate] = 0.0;
      for (jj = 0; jj < n_weights; jj++) {
        kgate = igate + jj + 1;
        if (kgate < n_gates) {
          if (cmd_flag[kgate]) {
            sumWtsReverse[igate] += weights[jj] * cmd[kgate];
          }
        }
      }
    }
    
    /* fill in flag field if flag field is not already set and */
    /* both forward sum and reverse sum exceed threshold */
    
    for (igate = 0; igate < n_gates; igate++) {
      if (!cmd_flag[igate]) {
        if (sumWtsForward[igate] > sum_wt_threshold &&
            sumWtsReverse[igate] > sum_wt_threshold) {
          cmd_flag[igate] = 1;
          done = 0;
        }
      }
    }

    if (count >= 3) {
      done = 1;
    }

    count++;

  } /* while (!done) */

  free(sumWtsForward);
  free(sumWtsReverse);
  free(weights);

}

