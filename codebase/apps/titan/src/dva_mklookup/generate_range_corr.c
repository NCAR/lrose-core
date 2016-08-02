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
 * generate_range_corr.c
 *
 * Mike Dixon
 * Marion Mittermaier
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Aug 1998
 *
 ****************************************************************************/

#include "dva_mklookup.h"

int generate_range_corr(dva_rdas_cal_t *cal)

{

  int m;
  int irc, irc0;
  int dvip, byte_val;
  int ilen;

  double range, rc;
  double start_of_first_bin;
  double vips_per_dB;
  double fdbz;
  double scale, bias;

  FILE *vip2byte;
  FILE *rcorr;

  /*
   * open output files
   */
  
  if ((vip2byte = fopen(Glob->params.vip2byte_table_file_path, "w"))
      == NULL) {
    fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
    fprintf(stderr, "Cannot open vip2byte table output file\n");
    perror(Glob->params.vip2byte_table_file_path);
    return (-1);
  }

  if ((rcorr = fopen(Glob->params.range_correction_table_file_path, "w"))
      == NULL) {
    fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
    fprintf(stderr, "Cannot open range_correction table output file\n");
    perror(Glob->params.range_correction_table_file_path);
    fclose(vip2byte);
    return (-1);
  }

  /*
   * compute range correction
   */
 
  start_of_first_bin = cal->start_range - 0.5 * cal->gate_spacing;
  vips_per_dB = (cal->viphi - cal->viplo) / (cal->dbzhi - cal->dbzlo);
  
  for (m = 1; m <= cal->ngates; m++) {
	    
    range = start_of_first_bin + cal->gate_spacing * (m-1);
    rc = 20.0 * log10((range + 0.5 * cal->gate_spacing) / 100.0);
    irc = (int) floor(rc * vips_per_dB + 0.5);
    if (m == 1) {
      irc0 = irc;
    }
    fprintf(rcorr, "%6d\n", irc);

  }

  /*
   * compute vip2byte
   */

  bias = -30.0;
  scale = 0.5;
  ilen = 4096 - irc0;

  for (dvip = 1; dvip <= ilen; dvip++) {

    fdbz = ((dvip - cal->viplo) * (1.0 / vips_per_dB)) +
      (cal->dbzlo + irc0 / vips_per_dB);

    byte_val = (fdbz - bias) / scale;

    fprintf(vip2byte, "%4d\n", byte_val);

  } /* dvip */

  fclose(vip2byte);
  fclose(rcorr);
  
  return (0);

}

