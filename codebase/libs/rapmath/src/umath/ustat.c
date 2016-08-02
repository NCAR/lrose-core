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
/*************************************************************************
 * ustat.c - math utilities library
 *
 * statistical routines
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * Jan 1996
 *
 ***************************************************************************/

#include <rapmath/umath.h>

/*****************
 * ucompute_cont()
 * 
 * compute contingency table
 */

void ucompute_cont(ucont_table_t *cont)

{

  double w, x, y, z;
  double pod, pod_denom;
  double far, far_denom;
  double csi, csi_denom;
  double hss, hss_denom;
  double gss, gss_denom;

  x = cont->n_success;
  y = cont->n_failure;
  z = cont->n_false_alarm;
  w = cont->n_non_event;

  pod_denom = x + y;
  far_denom = x + z;
  csi_denom = x + y + z;
  hss_denom = (y * y) + (z * z) + (2.0 * x * w) + (y + z) * (x + w);
  gss_denom = (y + z) * (x + y + z + w) + ((x * w) - (y * z));

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

  if (gss_denom > 0)
    gss = (x * w - y * z) / gss_denom;
  else
    gss = 0.0;

  cont->pod = pod;
  cont->far = far;
  cont->csi = csi;
  cont->hss = hss;
  cont->gss = gss;

  return;

}

/***************
 * uprint_cont()
 * 
 * print contingency table
 */

void uprint_cont(ucont_table_t *cont,
		 FILE *out,
		 char *label,
		 char *spacer,
		 int print_hss,
		 int print_gss)
     
{

  fprintf(out, "CONTINGENCY TABLE - %s\n", label);
  fprintf(out, "\n");
  fprintf(out, "%sN_success     : %g\n", spacer, cont->n_success);
  fprintf(out, "%sN_failure     : %g\n", spacer, cont->n_failure);
  fprintf(out, "%sN_false_alarm : %g\n", spacer, cont->n_false_alarm);
  fprintf(out, "%sN_non_event   : %g\n", spacer, cont->n_non_event);
  fprintf(out, "%sPOD           : %.2f\n", spacer, cont->pod);
  fprintf(out, "%sFAR           : %.2f\n", spacer, cont->far);
  fprintf(out, "%sCSI           : %.2f\n", spacer, cont->csi);
  if (print_hss)
    fprintf(out, "%sHSS           : %.2f\n", spacer, cont->gss);
  if (print_gss)
    fprintf(out, "%sGSS           : %.2f\n", spacer, cont->hss);
  
}
 

double usdev(double sum, double sumsq, double n)

{

  double comp;

  comp = n * sumsq - sum * sum;

  if (comp <= 0.0 || n < 2.0) {
    return (0.0);
  } else {
    return (sqrt(comp) / (n - 1.0));
  }

}

