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
#ifndef XRS_WAS_INCLUDED
#define XRS_WAS_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

/* XRS : X RESOURCES: cover for Xrm routines...
 *
 *      Routines to get default parameters for X applications 
 *	Based on code by Mike Dixon RAP/NCAR * June 1990
 *      F. Hage RAP/NCAR * April 1991
 *      4/13/93 Jcaron: make into XRS module
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
 * Examples:
 *
 * double value = XRSgetDouble(db,"application_name_or_class.origin_longitude", -107.34);
 * char * ptr = XRSgetString(db, "application_name_or_class.fatal_error_message", "Help!");
 * long val = XRSgetLong(db, "application_name_or_class.debug_flag", 0);
 *
 * Make sure you use the routine of the proper type. One can do sophisticated
 * hierarchal pattern matching using wild cards in the search string to
 * obtain values that are useful across applications or software objects. 
 * More than one "." separater  can exist in the pattern string.
 * See the X11 manuals 1 & 2 for more details.
 */
/*  */
#include <X11/Xresource.h> 

/* XRSgetXXXX:
	search the X resource database "db" for "resource_name".
   	if not found, return "default" else return value converted to correct type
 */

extern float XRSgetFloat( XrmDatabase db, char *resource_name, float hard_default);
extern double XRSgetDouble( XrmDatabase db, char *resource_name, double hard_default);
extern long XRSgetLong( XrmDatabase db, char *resource_name, long hard_default);
extern char *XRSgetString( XrmDatabase db, char *resource_name, char *hard_default);

#ifdef __cplusplus
}
#endif
#endif
