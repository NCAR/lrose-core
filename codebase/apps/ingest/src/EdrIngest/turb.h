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
 *  $Id: turb.h,v 1.3 2016/03/07 01:23:00 dixon Exp $
 *  $Revision: 1.3 $
 *  $State: Exp $
 *
 *  $Log: turb.h,v $
 *  Revision 1.3  2016/03/07 01:23:00  dixon
 *  Changing copyright/license to BSD
 *
 *  Revision 1.2  2005/05/13 15:06:34  insitu
 *  updated code
 *
 *  Revision 1.1  2004/12/03 19:00:05  dettling
 *  Initial check in.
 *
 *  Revision 1.2  2004/11/04 16:26:50  limber
 *  checking in current local version of code
 *
 *  Revision 1.1  1999/05/19 17:35:16  dettling
 *  Initial checkin of Insitu module.
 *  Sue Dettling May 19, 1999
 *
 *  Revision 1.1  1998/09/16 22:29:59  ldm
 *  9/16/1998 - Celia Chen
 *  These include files are used by decode.c which decodes real time ACARS
 *  data for web display.
 *  Initial check-in.
 *
 *  Revision 1.4  1998/03/12 21:38:17  morse
 *  added explicit ucopyright
 *
 *  Revision 1.3  1998/03/12 21:02:46  morse
 *  mods for 3/13/98 UAL/Allied delivery
 *
 *  Revision 1.2  1997/12/17 23:41:52  morse
 *  mods for troubleshooting UAL B757 missing data
 *  mod to make percentile values configurable
 *
 * Revision 1.1  1997/04/09  23:22:34  morse
 * initial Allied delivery
 *
 * Revision 1.2  1996/11/28  01:22:19  morse
 * mods and new files for ual testing. new names for library functions
 * and using real_t instead of double
 *
 * Revision 1.1  1995/10/23  21:16:52  morse
 * initial checkin
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/


/************************************************************************

Header: turb.h

Author: C S Morse

Date:	Tue Jul 27 17:00:00 1993

Description:	 Basic definitions used in in-situ turbulence program.

*************************************************************************/

# ifndef    TURBULENCE_H
# define    TURBULENCE_H


/* System include files / Local include files */


/* Constant definitions / Macro definitions / Type definitions */

/* Simpler, if perhaps less efficient macros to avoid explicit typing */
/* MACRO arguments should not have side effects as they are evaluated twice */
# define SQR(a) ((a)*(a))
# define CUB(a) ((a)*(a)*(a))

# define TWOPI 	(2.0*PI)
# define HALFPI (PI/2.0)

# define DEGPERRAD 	(180.0/M_PI)
# define SEC_PER_MIN 	60.0
# define G 		9.8	/* m/sec^2 per g unit			*/
# define G_TO_MPS 	9.8	/* m/sec^2 per g unit			*/

# define NA 	(-99999.0)

# define FALSE 	0

/* Trouble shooting */
# define LEPS_BADCGA	254
# define LEPS_BADALT	253
# define LEPS_BADMACH	252
# define LEPS_BADMASS	251
# define LEPS_BADFLAP	250
# define LEPS_BADAUTO	249
# define LEPS_TROUBLE	LEPS_BADAUTO

/* Autopilot Status */

# define OFF 0
# define ON  1

/* Status Flags */

# define NR_FLAG 255		/* No Report AlphaNumeric Flag		*/

# define END 		255	
# define VALID		0	
# define INVALID	1

# define NEW		0
# define OLD		1

# define SUCCESS	0
# define FAILURE       -1

/* Status Masks */

# define RESET_CGA  0x01
# define BAD_PRF    0x02
# define OLD_TAS    0x04
# define RESET_TAS  0x08
# define BAD_ALT    0x10
# define BAD_MACH   0x20
# define BAD_MASS   0x40
# define BAD_FLAP   0x80
# define BAD_AUTO   0x04	/* Reusing OLD_TAS mask not required onboard */

/* Flags for integral approximation */

# define LOW 	0
# define HIGH 	1

/* Number of realtime parameters	*/

# define NRTP 7	

enum realtime_parms
{
  ALT = 0,	/* Altitude Index			*/
  MASS,		/* Mass Index				*/
  TAS,		/* TAS Index				*/
  FLAP,		/* Flap Index				*/
  MACH,		/* Mach Index				*/
  AUTO,		/* Autopilot Index			*/
  CGA		/* CGA Index				*/
};

/* Specify precision level at compile time */

#ifdef SINGLE_PRECISION
typedef float real_t;
#else
typedef double real_t;
#endif

/* External global variables / Non-static global variables / Static globals */


/* External functions / Internal global functions / Internal static functions */

extern void log_error( char *err_msg );		/* Error Logging Routine */

# endif     /* TURBULENCE_H */





