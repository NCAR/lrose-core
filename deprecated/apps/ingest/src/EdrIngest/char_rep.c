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
 *  $Id: char_rep.c,v 1.6 2016/03/07 01:23:00 dixon Exp $
 *
 *  $Log: char_rep.c,v $
 *  Revision 1.6  2016/03/07 01:23:00  dixon
 *  Changing copyright/license to BSD
 *
 *  Revision 1.5  2005/12/15 16:48:04  dettling
 *  *** empty log message ***
 *
 *  Revision 1.4  2005/05/14 18:13:36  insitu
 *  changed code so that error messages from aircraft will be put into
 *  database
 *
 *  Revision 1.3  2005/05/13 15:06:34  insitu
 *  updated code
 *
 *  Revision 1.2  2005/03/09 20:36:04  insitu
 *  fixed compile problems
 *
 *  Revision 1.1  2004/12/03 19:00:05  dettling
 *  Initial check in.
 *
 *  Revision 1.2  2004/11/04 16:26:50  limber
 *  checking in current local version of code
 *
 *  Revision 1.1  1999/06/04 19:46:28  dettling
 *  *** empty log message ***
 *
 *  Revision 1.1  1998/09/16 22:33:46  ldm
 *  9/16/1998 - Celia Chen
 *  These programs are used to decode the real time ACARS data (come from
 *  FSL via LDM).  The output files are used in web page display at the moment.
 *  This is the initial check-in.
 *
 *  Revision 1.2  1998/03/12 21:40:23  morse
 *  mods for troubleshooting
 *
 *  Revision 1.1  1998/03/11 22:06:36  morse
 *  mods for character reporting for UAL
 *
 *
 */

# ifndef    lint
static char RCSid[] = "$Id: char_rep.c,v 1.6 2016/03/07 01:23:00 dixon Exp $";
# endif     /* not lint */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/


/************************************************************************

Module:	char_rep.c

Author:	C S Morse

Date:	Thu Mar  5 14:25:53 1998

Description: routines for single character reporting of average/peak 
             turbulence

************************************************************************/



/* System include files / Local include files */

# include "turb.h"
# include "char_rep.h"
# include "lscale.h"

/* Constant definitions / Macro definitions / Type definitions */


/* External global variables / Non-static global variables / Static globals */

static char Report_NA = NA_CHR;			/* NA report character	*/

/* Configurable Parameters */
static int N_report_thr = 7;		/* Number of report thrs	*/
static int Report_thr[MAX_REPORT_THR] = 	/* Reporting thresholds	*/
{ 10, 20, 30, 40, 50, 60, 70 };			/* as scaled epsilons	*/
static char Report_char[] = EXT_CHR_SET;	/* Reporting characters	*/

static int N_report_char = 36;		/* 36 for N_report_thr = 7	*/

/* External functions / Internal global functions / Internal static functions */

static int calc_interpreted_value( int index );

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

char 
ACT_generate_report_char( int epsilon_avg,	/* I - "average" scaled epsilon	*/
			  int epsilon_pk 	/* I - "peak" scaled epsilon	*/
			  )
{
  int ix = 0;
  int i,j;

  if ( epsilon_avg == NR_FLAG || epsilon_pk == NR_FLAG )
    return( Report_NA );

  /* Check for special values in the epsilons */
  switch ( epsilon_pk )
    {
    case LEPS_BADCGA:
      return( NA_BAD_CGA_CHR );
    case LEPS_BADALT:
      return( NA_BAD_ALT_CHR );
    case LEPS_BADMACH:
      return( NA_BAD_MACH_CHR );
    case LEPS_BADMASS:
    case LEPS_BADFLAP: 
    case LEPS_BADAUTO: 
      return( NA_BAD_OTHER_CHR );
    default:
	;
    }

  /* Find the character index associated with mean/peak combination */
  for ( i=0; i<=N_report_thr; i++ )
    {
      /* set ix to the lowest value for the peak thr exceedence */
      if ( i<N_report_thr && epsilon_pk >= Report_thr[i] )
	ix += i+1;
      else
	{
	  /* increment ix for every threshold average exceeds */
	  for ( j=0; j<=i; j++ )
	    {
	      if ( epsilon_avg >= Report_thr[j] )
		ix++;
	      else
		break;
	    }
	  break;
	}
    }

  return( Report_char[ix] );
}

/************************************************************************

Function Name: 	ACT_interpret_report_char

Description:	interpret the reported character as a mean and peak epsilon

Returns:	0 on success; -1 if reported character was not in list

Globals:	Report_NA	used
                Report_char	used
		N_report_thr	used
		Report_thr	used

Notes:	Scaled Epsilon values are reported as the bin midpoint.

************************************************************************/

int 
ACT_interpret_report_char( char r_char,		/* I - reported char	*/
			   int *epsilon_avg,	/* O - scaled eps mean	*/
			   int *epsilon_pk	/* O - scaled eps peak	*/
			   )
{
  int rc = 0;
  int ix, i,j;
  
  if ( r_char == Report_NA )
    *epsilon_avg = *epsilon_pk = (int)(LNA);
  else if ( r_char == NA_BAD_CGA_CHR )
    *epsilon_avg = *epsilon_pk = LEPS_BADCGA;
  else if ( r_char == NA_BAD_ALT_CHR )
    *epsilon_avg = *epsilon_pk = LEPS_BADALT;
  else if ( r_char == NA_BAD_MACH_CHR )
    *epsilon_avg = *epsilon_pk = LEPS_BADMACH;
  else if ( r_char == NA_BAD_OTHER_CHR )
    *epsilon_avg = *epsilon_pk = LEPS_TROUBLE;  
  else
    {
      ix = 0;
      while ( ix < N_report_char && r_char != Report_char[ix] )
	ix++;
      
      if ( ix >= N_report_char )
	{
	  /* *epsilon_avg = *epsilon_pk = int(LNA);  */
	  *epsilon_avg = *epsilon_pk = (int)(BAD_VALUE);
	  rc = -1;
	}
      else
	{
	  for ( i=0; i<N_report_thr; i++ )
	    {
	      if ( ix - (i+1) >= 0 )
		ix -= (i+1);
	      else
		break;
	    }
	  
	  /* Report midpoint of bin as value */
	  *epsilon_pk = calc_interpreted_value( i );
	  *epsilon_avg = calc_interpreted_value( ix );
	}
    }

  return( rc );
}

/************************************************************************

Function Name: 	ACT_setup_char_reporting

Description:	sets parameters for reporting mean and peak as single 
                character

Returns:	VALID on success; INVALID otherwise

Globals:	N_report_thr	set
                Report_thr	set
		Report_char	set
		N_report_char	set (calculated)

Notes:

************************************************************************/

int 
ACT_setup_char_reporting( int n_thr,	/* I - number of thresholds	*/
			  int *thr	/* I - scaled epsilon thrs	*/
			  )
{
  int i;
  
  /* Use a negative value of n_thr to indicate the use of default values */
  if ( n_thr <= 0 )
    return( VALID );

  if ( n_thr > MAX_REPORT_THR )
    return( INVALID );
  
  N_report_thr = n_thr;
  
  for ( i=0; i<n_thr; i++ )
    Report_thr[i] = thr[i];
  
  N_report_char = ( N_report_thr + 2 ) * ( N_report_thr + 1 ) / 2;
  
  return( 0 );
}

/************************************************************************

Function Name: 	calc_interpreted_value

Description:	calculates an interpreted value from a bin index and the 
                associated bin thresholds

Returns:	the interpreted value

Globals:	Report_thr	used
                N_report_thr	used

Notes:	For the lowest index returns average of lowest bin threshold and zero;
        for highest index returns theoretical midpoint of bin if it had same 
	width as next lowest bin; for others returns midpoint of bin.

************************************************************************/

static int 
calc_interpreted_value( int index	/* I - bin index	*/
			)
{
  int value;
  
  if ( index == 0 )
    value = Report_thr[index]/2;
  else if ( index == N_report_thr )
    value = Report_thr[index-1] + ( Report_thr[index-1] - Report_thr[index-2] )/2;
  else
    value = ( Report_thr[index] + Report_thr[index-1] )/2;

  return( value );
}







