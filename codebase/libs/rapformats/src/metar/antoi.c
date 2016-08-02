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

/********************************************************************/
/*                                                                  */
/*  Title:         antoi                                            */
/*  Date:          Jan 28, 1991                                     */
/*  Organization:  W/OSO242 - Graphics and Display Section          */
/*  Programmer:    Allan Darling                                    */
/*  Language:      C/370                                            */
/*                                                                  */
/*  Abstract:      This function will convert a character array     */
/*                 (string) of length (len) into an integer.        */
/*                 The integer is created via a call to the         */
/*                 function atoi.  This function extends the        */
/*                 functionality of atoi by removing the            */
/*                 requirement for a sentinal delimited string      */
/*                 as input.                                        */
/*                                                                  */
/*  Input: - Pointer to an array of characters.                     */
/*         - Integer indicating the number of character to include  */
/*           in the conversion.                                     */
/*                                                                  */
/*  Output:- An integer corresponding to the value in the character */
/*           array or MAXNEG (-2147483648) if the function is       */
/*           unable to acquire system storage.                      */
/*                                                                  */
/*  Modification History:                                           */
/*                 None                                             */
/*  NCAR/RAP - F. Hage. Removed Malloc for performance reasons.     */
/*                 Interpretable integers can never exceed 20       */
/*                 characters (2^^64) - So allocate a 24 byte space */
/*                                                                  */
/********************************************************************/

#include "metar_private.h"
#include "string.h"
 
int antoi(char * string, int len)
{
    /*******************/
    /* local variables */
    /*******************/
 
    char tmpstr[24];
 
    /*****************/
    /* function body */
    /*****************/

    /* clamp to a max of 20 digits */
    len = (len < 24) ? len : 20;
 
    /* Fill with nulls to force termination */
    memset(tmpstr,0,24);

    memcpy(tmpstr,string,len); /* copy the string */
 
    return atoi(tmpstr);
 
} /* end antoi */
 
