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
 * generate_displace_table.c
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

int generate_displace_table(dva_rdas_cal_t *cal)

{

  FILE *dtable;

  int i;

  int iver, iver1, iver2, iver3;
  double ver, ver1, ver2, ver3;
  
  double const1,const2,const3,const4,const5;
  double const6,const7,const8,const9,const10;
  double factor,diff;

  if ((dtable = fopen(Glob->params.displace_table_file_path, "w")) == NULL) {
    fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
    fprintf(stderr, "Cannot open displace table output file\n");
    perror(Glob->params.displace_table_file_path);
    return (-1);
  }

  /*
   * bepaal vip counts per db en ander konstantes
   */

  factor=(cal->viphi - cal->viplo) / (cal->dbzhi - cal->dbzlo);
  fprintf(stderr, "factor: %g\n", factor);

  /*
   * kwadratiese ontvanger
   */
  const1=10.*log10(2.);
  const2=log10(2.);
  const3=pow(10.0, (-1./10.));

  /*
   * linieere ontvanger
   */
  const4=20.*log10(2.);
  const5=log10(2.);
  const6=pow(10.,(-1./20.));

  /*
   * logaritmiese ontvanger
   */
  const7=0.5;

  /*
   * rainrate ontvanger (Z=200R**1.6)
   */
  const8=16.*log10(2.);
  const9=log10(2.);
  const10=pow(10.,(-1./16.));

  /*
   * bepaal tabel vir verskille
   */

  for (i = 1; i <= MAXVIP+1; i++) {

    diff= (double) i - 1.0;

    /*
     * kwadratiese ontvanger
     */

    ver = factor * const1 *
      (1.0 - (log10(1.0 + pow(const3,(diff/factor))))/const2);
    
    iver = ver + 0.5;

    /*
     * linieere ontvanger
     */

    ver1 = factor * const4 *
      (1.0 - (log10(1.0 + pow(const6, (diff/factor)))) / const5);

    iver1 = ver1 + 0.5;

    /*
     * logaritmiese ontvanger
     */

    ver2 = factor * const7 * (diff/factor);
    iver2 = ver2 + 0.5;

    /*
     * rainrate ontvanger
     */

    ver3 = factor *const8 *
      (1.0 - (log10(1.0 + pow(const10, (diff/factor)))) / const9);

    iver3 = ver3 + 0.5;

    fprintf(dtable, "%4d\n", iver);

  } /* i */

  fclose(dtable);

  return (0);

}
