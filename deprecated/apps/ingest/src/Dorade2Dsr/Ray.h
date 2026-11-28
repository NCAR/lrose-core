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
/*
 *	$Id: Ray.h,v 1.3 2016/03/07 01:23:00 dixon Exp $
 *
 *	Module:		 Ray.h
 *	Original Author: Richard E. K. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date: 2016/03/07 01:23:00 $
 *
 * revision history
 * ----------------
 * $Log: Ray.h,v $
 * Revision 1.3  2016/03/07 01:23:00  dixon
 * Changing copyright/license to BSD
 *
 * Revision 1.2  2012/05/17 01:49:05  dixon
 * Moving port_includes up into main source dir
 *
 * Revision 1.1  2007-09-22 21:08:21  dixon
 * adding
 *
 * Revision 1.1  2002/02/07 18:31:35  dixon
 * added
 *
 * Revision 1.1  1996/12/20 19:58:33  oye
 * New.
 *
 * Revision 1.2  1994/04/05  15:35:58  case
 * moved an ifdef RPC and changed an else if to make HP compiler happy.
 *
 * Revision 1.4  1992/07/28  17:33:03  thor
 * Added ray_status.
 *
 * Revision 1.3  1992/04/20  17:18:31  thor
 * Latest Eldora/Asterea revisions included.
 *
 * Revision 1.2  1991/10/15  17:56:43  thor
 * Fixed to meet latest version of tape spec.
 *
 * Revision 1.1  1991/08/30  18:39:38  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCRayh
#define INCRayh

#ifdef OK_RPC

#if defined(UNIX) && defined(sun)
#include <rpc/rpc.h>
#else
#if defined(WRS)
#include "rpc/rpc.h"
#endif
#endif /* UNIX */

#endif /* OK_RPC */

struct ray_i {
    char  ray_info[4];		/* Identifier for a data ray info. */
				/* block (ascii characters "RYIB"). */
    si32 ray_info_length;	/* length of a data ray info block in */
				/* bytes. */
    si32  sweep_num;		/* sweep number for this radar. */
    si32  julian_day;		/* Guess. */
    si16 hour;			/* Hour in hours. */
    si16 minute;		/* Minute in minutes. */
    si16 second;		/* Second in seconds. */
    si16 millisecond;		/* Millisecond in milliseconds. */
    fl32 azimuth;		/* Azimuth in degrees. */
    fl32 elevation;		/* Elevation in degrees. */
    fl32 peak_power;		/* Last measured peak transmitted */
				/* power in kw. */
    fl32 true_scan_rate;	/* Actual scan rate in degrees/second. */
    si32  ray_status;		/* 0 = normal, 1 = transition, 2 = bad. */
}; /* End of Structure */


typedef struct ray_i ray_i;
typedef struct ray_i RAY;

#ifdef OK_RPC
#if defined(sun) || defined(WRS)
bool_t xdr_ray_i(XDR *, RAY *);
#endif

#endif /* OK_RPC */

#endif /* INCRayh */

