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
/*
 * RCS info
 *  $Author: dixon $
 *  $Locker:  $
 *  $Date: 2016/03/07 01:23:00 $
 *  $Id: lscale.h,v 1.2 2016/03/07 01:23:00 dixon Exp $
 *  $Revision: 1.2 $
 *  $State: Exp $
 *
 *  $Log: lscale.h,v $
 *  Revision 1.2  2016/03/07 01:23:00  dixon
 *  Changing copyright/license to BSD
 *
 *  Revision 1.1  2004/12/03 19:00:05  dettling
 *  Initial check in.
 *
 *  Revision 1.2  2004/11/04 16:26:50  limber
 *  checking in current local version of code
 *
 *  Revision 1.1  2004/01/08 17:37:56  limber
 *  added C++ code to process raw edr data
 *
 *  Revision 1.1  1999/05/19 17:35:11  dettling
 *  Initial checkin of Insitu module.
 *  Sue Dettling May 19, 1999
 *
 *  Revision 1.1  1998/09/16 22:29:57  ldm
 *  9/16/1998 - Celia Chen
 *  These include files are used by decode.c which decodes real time ACARS
 *  data for web display.
 *  Initial check-in.
 *
 *  Revision 1.2  1998/03/12 21:38:14  morse
 *  added explicit ucopyright
 *
 *  Revision 1.1  1997/04/09 23:22:18  morse
 *  initial Allied delivery
 *
 * Revision 1.2  1996/12/11  01:06:10  morse
 * added new scale factors
 *
 * Revision 1.1  1996/11/28  01:22:09  morse
 * mods and new files for ual testing. new names for library functions
 * and using real_t instead of double
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/


/************************************************************************

Header: lscale.h

Author: C S Morse

Date:	Tue Nov 19 11:34:44 1996

Description:	definitions for scaled integer calculations

*************************************************************************/

# ifndef    SCALED_INT_H
# define    SCALED_INT_H


/* System include files / Local include files */


/* Constant definitions / Macro definitions / Type definitions */

# define LNA			(99999.0)
# define BAD_VALUE		(99990.0)

# define CGA_SCALE		1000
# define BP_FILTER_COEFF_SCALE	10000
# define EFACTOR_SCALE		1000
# define EPSILON_SCALE		100
# define MACH_SCALE		100
# define VEL_SCALE		100
# define PCT_SCALE		1000
# define FLAP_SCALE		10
# define HZ_SCALE		100
# define MASS_SCALE		1
# define ALT_SCALE		1

/* External global variables / Non-static global variables / Static globals */


/* External functions / Internal global functions / Internal static functions */


# endif     /* SCALED_INT_H */
