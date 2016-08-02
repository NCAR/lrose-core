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
#ifdef __cplusplus
 extern "C" {
#endif
#ifndef ERR_WAS_INCLUDED
#define ERR_WAS_INCLUDED

/* ERR.h : error processing module
 *  created: 1/17/91 JCaron
 * modified: 9/13/91 ANSI - only version
 *
 *
 * This module creates a centralized place for programming/system errors
 * to be recorded/controlled.
 *
 */
/* these are the possible error levels */
#define ERR_ALERT 1
#define ERR_RESOURCE 2
#define ERR_PROGRAM 3
#define ERR_WARNING 4
#define ERR_INFO 5
#define ERR_DEBUG 5


#define ERR_MESS_MAX	4092

typedef void (*ERRuserFuncp)( int level, char *message);

extern void ERRcontrol( char *control_string);
/* This allows you to progammatically do exactly the same thing as ERRinit 
	does with the command line. Note, however, that if there are command
 	line arguments to ERR, calls to ERRcontrol() will be ignored. 
	Example:
	   ERRcontrol("OFF STD LOG ON SLOG"); turn off stderr and local 
		file logging, turn on system logging.
 */
   
extern void ERRcontrolStr( char *control_string, int max_str);
/*  This is essentially the inverse of ERRcontrol; it constructs a control
	string based on the current settings.  This allows you to pass the 
	current ERR control to forked processes.  Note "User" functions 
	are not included.
	Example:
	   From the example in ERRcontrol(), above:
		"OFF STD OFF LOG ON SLOG"
 */   

extern void ERRlogfile( char *name);
/* Set the local log filename, used ith "ON LOG". This must be done before
	any call to ERRprintf().  Default log filename is "errlog".
 */

extern void ERRinit( char *application, int argc, char *argv[]);
/* This must be the first call to ERR.  This allows you to change the
   behavior of ERR from a command line argument.
   ERR will look in the command line arguments for an -ERR token, 
   then process any tokens that follow that match: 
		[OFF|ON] [LOG|OPS|SLOG|STD|USR] 
   where
      LOG = send output to log file
      OPS = put in "standard operations" mode
      SLOG = send to system logger (Sun only)
      STD = send to stderr
      USR = enable user defined error routiners, if defined
 
	Examples:
	   -ERR OFF STD ON LOG   turn off stderr, turn on local log file 
	   -ERR LOG OFF STD      same thing as above
           -ERR OPS		 put in "operations" mode
	   -ERR SLOG		 send to system logger 

*/

extern void ERRprintf(int level, char *format, ...);
/* Create an error message using the given format statement and a variable
   	list of arguments, exactly as with printf. 
	level is one of:
	   ERR_ALERT : probable hardware error, database corruption, etc that
		should be corrected immediately
	   ERR_RESOURCE : resource exhaustion, such as a file descriptor or
		memory.
	   ERR_PROGRAM : impossible condition, probably program logic error.
	   ERR_WARNING : worrisome condition exists. 
	   ERR_INFO: informational message 
	   ERR_DEBUG: debugging messages
 */

extern int  ERRuser( ERRuserFuncp uf); 
/* This allows you to set ap a "user" error handling routine. Whenever an
   error is received by ERR, UserFunc() is called.
   Returns a user function index for use in ERRuserActivate(), below.
 */
    
extern void ERRuserActivate( int usr_idx, int turnon);
/* This allows you to activate/deactivate various user error handlers
   dynamically.  user_idx is the return value from ERRuser(); 
   turnon has the value TRUE(1) or FALSE(0). 
   Note that when ERRuser() is called, the default state is ON.
 */

	
#endif
#ifdef __cplusplus
}
#endif
