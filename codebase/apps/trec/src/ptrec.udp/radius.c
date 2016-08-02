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
 * radius
 ****************************************************************************/

#include "trec.h"

void radius(int **el_time,
	    int iel,
	    int k1,
	    int k2,
	    float *tkmove,
	    float *dt)

{

  int ihr1,imn1,isec1,ihr2,imn2,isec2,isec;

  ihr1 = el_time[iel][k1]/10000;
  ihr2 = el_time[iel][k2]/10000;
  imn1 = (el_time[iel][k1]-ihr1*10000)/100;
  imn2 = (el_time[iel][k2]-ihr2*10000)/100;
  isec1 = el_time[iel][k1]-ihr1*10000-imn1*100;
  isec2 = el_time[iel][k2]-ihr2*10000-imn2*100;
  isec = ihr1*3600+imn1*60+isec1-ihr2*3600-imn2*60-isec2;
  if(isec < 0)isec += 86400;
  *tkmove = Glob->params.max_vel * isec * 0.001;
  *dt = isec/60.;
}

