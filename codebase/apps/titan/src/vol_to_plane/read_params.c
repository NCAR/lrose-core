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
/*********************************************************************
 * read_params.c: reads the parameters, loads up the globals
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "vol_to_plane.h"

void read_params(void)

{

  /*
   * get globals from the parameters
   */
  
  /*
   * grid geometry
   */

  Glob->nx = uGetParamLong(Glob->prog_name, "nx", NX);
  Glob->ny = uGetParamLong(Glob->prog_name, "ny", NY);

  Glob->minx = uGetParamDouble(Glob->prog_name, "minx", MINX);
  Glob->miny = uGetParamDouble(Glob->prog_name, "miny", MINY);

  Glob->minz = uGetParamDouble(Glob->prog_name, "minz", MINZ);
  Glob->maxz = uGetParamDouble(Glob->prog_name, "maxz", MAXZ);

  Glob->dx = uGetParamDouble(Glob->prog_name, "dx", DX);
  Glob->dy = uGetParamDouble(Glob->prog_name, "dy", DY);


}

