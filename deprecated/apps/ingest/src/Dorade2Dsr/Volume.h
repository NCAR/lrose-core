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
 *	$Id: Volume.h,v 1.3 2016/03/07 01:23:00 dixon Exp $
 *
 *	Module:		 Volume.h
 *	Original Author: Richard E. K. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date: 2016/03/07 01:23:00 $
 *
 * revision history
 * ----------------
 * $Log: Volume.h,v $
 * Revision 1.3  2016/03/07 01:23:00  dixon
 * Changing copyright/license to BSD
 *
 * Revision 1.2  2012/05/17 01:49:05  dixon
 * Moving port_includes up into main source dir
 *
 * Revision 1.1  2007-09-22 21:08:21  dixon
 * adding
 *
 * Revision 1.1  2002/02/07 18:31:36  dixon
 * added
 *
 * Revision 1.1  1996/12/20 20:00:09  oye
 * New.
 *
 * Revision 1.2  1994/04/05  15:36:47  case
 * moved an ifdef RPC and changed an else if to make HP compiler happy.
 *
 * Revision 1.4  1992/04/20  17:18:31  thor
 * Latest Eldora/Asterea revisions included.
 *
 * Revision 1.3  1991/10/15  17:57:07  thor
 * Fixed to meet latest version of tape spec.
 *
 * Revision 1.2  1991/10/09  15:24:44  thor
 * Fixed incorrect flight number variable and added needed padding.
 *
 * Revision 1.1  1991/08/30  18:39:40  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCVolumeh
#define INCVolumeh

#ifdef OK_RPC

#if defined(UNIX) && defined(sun)
#include <rpc/rpc.h>
#else 
#if defined(WRS)
#include "rpc/rpc.h"
#endif
#endif /* UNIX */

#endif /* OK_RPC */

struct volume_d {
    char  volume_des[4];	/* Volume descriptor identifier: ASCII */
				/* characters "VOLD" stand for Volume */
				/* Descriptor. */
    si32  volume_des_length;	/* Volume descriptor length in bytes. */
    si16 format_version;	/* ELDORA/ASTRAEA field tape format */
				/* revision number. */
    si16 volume_num;		/* Volume Number into current tape. */
    si32  maximum_bytes;	/* Maximum number of bytes in any. */
				/* physical record in this volume. */
    char  proj_name[20];		/* Project number or name. */
    si16 year;			/* Year data taken in years. */
    si16 month;		/* Month data taken in months. */
    si16 day;			/* Day data taken in days. */
    si16 data_set_hour;	/* hour data taken in hours. */
    si16 data_set_minute;	/* minute data taken in minutes. */
    si16 data_set_second;	/* second data taken in seconds. */
    char  flight_num[8];	/* Flight number. */
    char  gen_facility[8];	/* identifier of facility that */
				/* generated this recording. */
    si16 gen_year;		/* year this recording was generated */
				/* in years. */
    si16 gen_month;		/* month this recording was generated */
				/* in months. */
    si16 gen_day;		/* day this recording was generated in days. */
    si16 number_sensor_des;	/* Total Number of sensor descriptors */
				/* that follow. */
}; /* End of Structure */



typedef struct volume_d volume_d;
typedef struct volume_d VOLUME;

#ifdef OK_RPC
#if defined(sun) || defined(WRS)
bool_t xdr_volume_d(XDR *, VOLUME *);
#endif

#endif /* OK_RPC */

#endif /* INCVolumeh */

