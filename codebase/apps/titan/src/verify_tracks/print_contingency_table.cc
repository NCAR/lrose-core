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
/*****************************************************************************
 * print_contingency_table.c
 *
 * debug printout of contingency table
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * January 1992
 *
 *****************************************************************************/

#include "verify_tracks.h"

void print_contingency_table(FILE *fout,
			     const char *label,
			     vt_count_t *count)

{
  
  double w, x, y, z;
  double pod, pod_denom;
  double far, far_denom;
  double csi, csi_denom;
  double hss, hss_denom;

  x = count->n_success;
  y = count->n_failure;
  z = count->n_false_alarm;
  w = count->n_non_event;

  pod_denom = x + y;
  far_denom = x + z;
  csi_denom = x + y + z;
  hss_denom =  (y * y) + (z * z) + (2.0 * x * w) + (y + z) * (x + w);

  if (pod_denom > 0)
    pod = x / pod_denom;
  else
    pod = 0.0;

  if (far_denom > 0)
    far = z / far_denom;
  else
    far = 0.0;

  if (csi_denom > 0)
    csi = x / csi_denom;
  else
    csi = 0.0;

  if (hss_denom > 0)
    hss = (2.0 * (x * w - y * z)) / hss_denom;
  else
    hss = 0.0;

  fprintf(fout, "\n%s\n\n", label);
  
  fprintf(fout, "n_success     : %g\n", count->n_success);
  fprintf(fout, "n_failure     : %g\n", count->n_failure);
  fprintf(fout, "n_false_alarm : %g\n", count->n_false_alarm);
  fprintf(fout, "n_non_event   : %g\n", count->n_non_event);
  
  fprintf(fout, "\n");

  fprintf(fout, "POD           : %g\n", pod);
  fprintf(fout, "FAR           : %g\n", far);
  fprintf(fout, "CSI           : %g\n", csi);
  fprintf(fout, "HSS           : %g\n", hss);
  
}
