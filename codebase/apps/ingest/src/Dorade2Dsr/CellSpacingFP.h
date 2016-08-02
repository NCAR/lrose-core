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
 *	$Id: CellSpacingFP.h,v 1.3 2016/03/07 01:23:00 dixon Exp $
 *
 *	Module:		 CellSpacing.h
 *	Original Author: Richard E. K. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date: 2016/03/07 01:23:00 $
 *
 * revision history
 * ----------------
 * $Log: CellSpacingFP.h,v $
 * Revision 1.3  2016/03/07 01:23:00  dixon
 * Changing copyright/license to BSD
 *
 * Revision 1.2  2012/05/17 01:49:05  dixon
 * Moving port_includes up into main source dir
 *
 * Revision 1.1  2007-09-22 21:08:21  dixon
 * adding
 *
 * Revision 1.1  2002/02/07 18:31:34  dixon
 * added
 *
 * Revision 1.1  1999/04/12 20:03:07  oye
 * Cell spacing information similar to ELDORA's "CSPD" but with floating
 * point values rather than short values. Meant to be substituted for the
 * CELLVECTOR and to make sweepfiles smaller for network transfers.
 *
 * Revision 1.2  1994/04/05  15:22:38  case
 * moved an ifdef RPC and changed else if to else and if on another line
 * >> to keep the HP compiler happy.
 * >> .
 *
 * Revision 1.3  1991/10/15  17:54:21  thor
 * Fixed to meet latest version of tape spec.
 *
 * Revision 1.2  1991/10/11  15:32:10  thor
 * Added variable for offset to first gate.
 *
 * Revision 1.1  1991/08/30  18:39:19  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCCellSpacingFPh
#define INCCellSpacingFPh

#include <dataport/port_types.h>

#ifdef OK_RPC

#if defined(UNIX) && defined(sun)
#include <rpc/rpc.h>
#else 
# if defined(WRS)
#   include "rpc/rpc.h"
# endif
#endif /* UNIX */

#endif /* OK_RPC */

struct cell_spacing_fp_d {
    char   cell_spacing_des[4]; /* Identifier for a cell spacing descriptor */
				/* (ascii characters CSFD). */
    si32   cell_spacing_des_len; /* Cell Spacing descriptor length in bytes. */
    si32   num_segments;	/* Number of segments that contain cells of */
    fl32   distToFirst;		/* Distance to first gate in meters. */
    fl32   spacing[8];		/* Width of cells in each segment in m. */
    si16   num_cells[8];	/* Number of cells in each segment. */
				/* equal widths. */
};				/* End of Structure */


typedef struct cell_spacing_fp_d cell_spacing_fp_d;
typedef struct cell_spacing_fp_d CELLSPACINGFP;

#ifdef OK_RPC
#if defined(sun) || defined(WRS)
bool_t xdr_cell_spacing_fp_d(XDR *, cell_spacing_fp_d *);
#endif

#endif /* OK_RPC */

#endif /* INCCellSpacingPFh */









