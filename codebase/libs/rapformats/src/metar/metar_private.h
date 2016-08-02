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
/************************************************************
 * metar_private.h
 *
 * Private include for metar
 *
 * Jaimi Yee, RAP, NCAR, P.O. BOX 3000, Boulder,
 *            CO, 80303, USA
 *
 * April 1998
 *
 ************************************************************/

#include <stdio.h>
#include <stdlib.h>

/*
 * private function prototypes
 */
 
extern int antoi(char * string, int len);

extern float fracPart( char *string );

extern int nisalnum(char *s, int n);
 
extern int nisalpha(char *s, int n);
 
extern int niscntrl(char *s, int n);
 
extern int nisdigit(char *s, int n);
 
extern int nisgraph(char *s, int n);
 
extern int nislower(char *s, int n);
 
extern int nisprint(char *s, int n);
 
extern int nispunct(char *s, int n);
 
extern int nisspace(char *s, int n);
 
extern int nisupper(char *s, int n);
 
extern int nisxdigi(char *s, int n);
 


 

