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
 *  $Id: char_rep.h,v 1.2 2016/03/07 01:23:00 dixon Exp $
 *
 *  $Log: char_rep.h,v $
 *  Revision 1.2  2016/03/07 01:23:00  dixon
 *  Changing copyright/license to BSD
 *
 *  Revision 1.1  2004/12/03 19:00:05  dettling
 *  Initial check in.
 *
 *  Revision 1.2  2004/11/04 16:26:50  limber
 *  checking in current local version of code
 *
 *  Revision 1.1  1999/05/19 17:35:07  dettling
 *  Initial checkin of Insitu module.
 *  Sue Dettling May 19, 1999
 *
 *  Revision 1.1  1998/09/16 22:29:56  ldm
 *  9/16/1998 - Celia Chen
 *  These include files are used by decode.c which decodes real time ACARS
 *  data for web display.
 *  Initial check-in.
 *
 *  Revision 1.1  1998/03/12 21:00:49  morse
 *  single character reporting support
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/


/************************************************************************

Header: char_rep.h

Author: C S Morse

Date:	Thu Mar  5 14:30:20 1998

Description:	definitions for single character reporting of mean and peak 
                turbulence

*************************************************************************/

# ifndef    CHAR_REP_H
# define    CHAR_REP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* System include files / Local include files */


/* Constant definitions / Macro definitions / Type definitions */

/* Extended character set - Standard character set is first 36 entries */
# define EXT_CHR_SET	\
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz[]^{|}~!#$%&()*+"

/* These characters are available in both standard and extended char sets */
    
# define NA_CHR			'.'
# define NA_OK_CHR 		' '
# define NA_BAD_CGA_CHR		':'
# define NA_BAD_ALT_CHR 	','
# define NA_BAD_MACH_CHR	'-'
# define NA_BAD_OTHER_CHR	'/'

# define MAX_REPORT_THR 11	
# define MAX_REPORT_CHR 79

/* External global variables / Non-static global variables / Static globals */


/* External functions / Internal global functions / Internal static functions */

/************************************************************************

Function Name: 	ACT_generate_report_char

Description:	convert mean and peak epsilon values to a character for 
                reporting

Returns:	character to report

Globals:	Report_NA
                N_report_thr
		Report_thr
		Report_char

Notes:

************************************************************************/

extern char 
ACT_generate_report_char( int epsilon_avg,	/* I - "average" scaled epsilon */
			  int epsilon_pk 	/* I - "peak" scaled epsilon */
			  );

/************************************************************************

Function Name: 	ACT_interpret_report_char

Description:	interpret the reported character as a mean and peak epsilon

Returns:	0 on success; -1 if reported character was not in list

Globals:	Report_NA	used
                Report_char	used
		N_report_thr	used
		Report_thr	used

Notes:	Scaled Epsilon values are midpoint of bin.

************************************************************************/

extern int 
ACT_interpret_report_char( char r_char,		/* I - reported char*/
			   int *epsilon_avg,	/* O - scaled eps mean*/
			   int *epsilon_pk	/* O - scaled eps peak*/
			   );

/************************************************************************

Function Name: 	ACT_setup_char_reporting

Description:	sets parameters for reporting mean and peak as single 
                character

Returns:	VALID on success; INVALID otherwise

Globals:	N_report_thr	set
                Report_thr	set
		N_report_char	set (calculated)

Notes:

************************************************************************/

extern int 
ACT_setup_char_reporting( int n_thr,	/* I - number of thresholds*/
			  int *thr	/* I - scaled epsilon thrs*/
			  );


#ifdef __cplusplus
}
#endif

# endif     /* CHAR_REP_H */







