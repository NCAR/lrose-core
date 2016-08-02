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
/******************************************************************
 * GET_X_DEF.C : Routines to get default parameters for X applications 
 *
 *	Based on code by Mike Dixon RAP/NCAR * June 1990
 *
 * -F. Hage RAP/NCAR * April 1991
 *  4/13/93 Jcaron: make into XRSRC module
 *
 * Keyword: Runtime default value database management, access
 * Keyword: XResources, extraction, X11
 * 
 * These routines do not have to be used with an X application. The
 * Xrm....() functions  are standalone. All one has to do is to link in
 * libX11.a to the appication. To initialize the system, include
 * <X11/Xresource.h>, and define a data base handle: 
 *
 * XrmDatabase db;
 * Then call :
 *
 * db = XrmGetFileDatabase(file_name);
 * if(db == NULL) complain(); 
 *
 * The file should contain lines similar to the following:   
 *
 * application_name_or_class.debug_flag:         1
 * application_name_or_class.origin_longitude:       -104.761730
 * application_name_or_class.fatal_error_message:    Panic! Can't continue, fatal error
 * 
 * Call the following routines in this fashion:
 *
 * double value = GetDoubleDef(db,"application_name_or_class.origin_longitude",-107.34);
 * char * ptr = GetStringDef(db,"application_name_or_class.fatal_error_message","Help!");
 * long val = GetStringDef(db,"application_name_or_class.debug_flag",0);
 *
 * Make sure you use the routine of the proper type. One can do sophisticated
 * hierarchal pattern matching using wild cards in the search string to
 * obtain values that are useful across applications or software objects. 
 * More than one "." separater  can exist in the pattern string.
 * See the X11 manuals 1 & 2 for more details.
 *
 *
 * For the AWPG Displays 
 * Frank Hage   1991,1992 NCAR, Research Applications Program
 */


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>

#include <rapplot/xrs.h>
#include <toolsa/umisc.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>


/******************************************************************************
 * GETFLOATDEF: returns a float from a X resources database, or the default
 *                if the resource string is not found
 */
float XRSgetFloat( XrmDatabase db, char *string, float hard_def)
{
  float value;
  char  *end_pt;
  char	*stype;
  XrmValue rvalue;
  char line[BUFSIZ];
  char *ptr;

  STRncopy(line, string, BUFSIZ);
  if (usubstitute_env(line, BUFSIZ)) {
    ptr = string;
  } else {
    ptr = line;
  }

  
  if (XrmGetResource(db, ptr, "", &stype, &rvalue))  {
    errno = 0;
    value = (float) strtod(rvalue.addr, &end_pt);
    if(errno != 0) value = hard_def;
  } else {
    value = hard_def;
  }

  return value;
}
 
/******************************************************************************
 * GETDOUBLEDEF: returns a double from a X resources database, or the default
 *                if the resource string is not found
 */
double XRSgetDouble( XrmDatabase db, char *string, double hard_def)
{
  double value;
  char	*end_pt;
  char	*stype;
  XrmValue	rvalue;

  char line[BUFSIZ];
  char *ptr;

  STRncopy(line, string, BUFSIZ);
  if (usubstitute_env(line, BUFSIZ)) {
    ptr = string;
  } else {
    ptr = line;
  }

  
  if (XrmGetResource(db,ptr,"",&stype,&rvalue))  {
    errno = 0;
    value = strtod(rvalue.addr, &end_pt);
    if(errno != 0) value = hard_def;
  } else {
    value = hard_def;
  }


  return value;
}
 
/******************************************************************************
 * GETLONGDEF: returns a long from a X resources database, or the default
 *                if the resource string is not found
 */
long XRSgetLong( XrmDatabase db, char *string, long hard_def)
{
  long value;
  char *end_pt;
  char	*stype;
  XrmValue	rvalue;

  char line[BUFSIZ];
  char *ptr;

  STRncopy(line, string, BUFSIZ);
  if (usubstitute_env(line, BUFSIZ)) {
    ptr = string;
  } else {
    ptr = line;
  }

  
  if (XrmGetResource(db,ptr,"",&stype,&rvalue))  {
    errno = 0;
    value =  strtol(rvalue.addr, &end_pt,10);
    if(errno != 0) value = hard_def;
  } else {
    value = hard_def;
  }


  return value;
}

/******************************************************************************
 * GETSTRINGDEF: returns a pointer to a string from a X resources database,
 *	 or the default if the resource string is not found
 *
 * Quick hack for substituting env variables.
 * If env variable is embedded, malloc space and expand. This should
 * not be called multiple times for the same param, otherwise mem
 * will be wasted.
 *
 */

char *XRSgetString( XrmDatabase db, char *string, char *hard_def)
{
  char	*stype;
  XrmValue	rvalue;
  char sbuf[1024];
  char *expanded;
  char *c;

  char line[BUFSIZ];
  char *ptr;

  STRncopy(line, string, BUFSIZ);
  if (usubstitute_env(line, BUFSIZ)) {
    ptr = string;
  } else {
    ptr = line;
  }

  if (XrmGetResource(db,ptr,"",&stype,&rvalue))  {
    /* remove trailing blanks or tabs - dixon */
    c = rvalue.addr + (strlen(rvalue.addr) - 1);
    while (*c == ' ' || *c == '\t') {
      *c = '\0';
    }
    STRncopy(sbuf, rvalue.addr, 1024);
    usubstitute_env(sbuf, 1024);
    if (strlen(sbuf) > strlen(rvalue.addr)) {
      expanded = umalloc(strlen(sbuf) + 1);
      strcpy(expanded, sbuf);
      return (expanded);
    } else {
      strcpy(rvalue.addr,sbuf);
      return rvalue.addr;
    }
  } else {
    return hard_def;
  }
}


