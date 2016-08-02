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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* SCCS info
 *   %W% %D% %T%
 *   %F% %E% %U%
 *
 * RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 18:45:40 $
 *   $Id: flt_path.c,v 1.4 2016/03/03 18:45:40 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * flt_path.c: Routines to manipulate flight path data.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <toolsa/os_config.h>

#include <dataport/bigend.h>
#include <dataport/port_types.h>

#include <rapformats/flt_path.h>


/************************************************************************
 * FLTPATH_print_path(): Print the SPDB path information to the indicated
 *                       stream in ASCII format.
 */

void FLTPATH_print_path(FILE *stream, FLTPATH_path_t *path)
{
  int i;
  
  fprintf(stream, "\nPath info:\n");
  fprintf(stream, "   num_pts = %d\n", path->num_pts);
  for (i = 0; i < path->num_pts; i++)
    fprintf(stream, "      %f %f %f %f\n",
	    path->pts[i].loc.x,
	    path->pts[i].loc.y,
	    path->pts[i].time,
	    path->pts[i].cost);
  fprintf(stream, "\n");
  
  return;
}


/************************************************************************
 * FLTPATH_path_from_BE() - Convert the flight path information from
 *                          big-endian format to native format.
 */

void FLTPATH_path_from_BE(FLTPATH_path_t *path)
{
  int array_size;
  
  path->num_pts = BE_to_si32(path->num_pts);

  array_size = path->num_pts * sizeof(FLTPATH_leg_t);
  
  BE_to_array_32((void *)path->pts, array_size);
  
  return;
}


/************************************************************************
 * FLTPATH_path_to_BE() - Convert the flight path information from
 *                        native format to big-endian format.
 */

void FLTPATH_path_to_BE(FLTPATH_path_t *path)
{
  int array_size = sizeof(si32) +
    (path->num_pts * sizeof(FLTPATH_leg_t));
  
  BE_from_array_32((void *)path, array_size);
  
  return;
}
