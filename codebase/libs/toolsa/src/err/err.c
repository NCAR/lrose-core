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
/* ERR.c */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <syslog.h>

#include <toolsa/os_config.h>

#include <toolsa/globals.h>
#include <toolsa/str.h>
#include <toolsa/err.h>

#define MAX_LOGFILE_NAME 60
#define MAX_PRIORITIES 7
#define MAX_USERFUNCS 5

/* global variables */
static char 	  *Application = "Application Name Unknown";
static char       Logfile[MAX_LOGFILE_NAME] = "errlog";
static int    Log = FALSE;
static int    Override = FALSE;
static int    Standard = TRUE;
static int    SysLog = FALSE;
static int    User = FALSE;

static ERRuserFuncp UserFunc[ MAX_USERFUNCS] = { NULL, NULL, NULL, NULL, NULL};
static int UserOn[ MAX_USERFUNCS];
static int UserCount = 0;

static char *PriorityName[ MAX_PRIORITIES] = 
					{"EMERGENCY!!!",
					"ALERT!",
					"RESOURCE EXHAUSTED",
					"PROGRAM ERROR",
					"Warning",
					"Information Message",
					"Debug" };

static int PriorityLevel[ MAX_PRIORITIES] = 
					{ LOG_EMERG,
					LOG_ALERT,
					LOG_ERR,
					LOG_ERR,
					LOG_WARNING,
					LOG_INFO,
					LOG_DEBUG };
	
#define LOG_FACILITY	LOG_LOCAL0


static char *GetTime( void)
   {
      time_t t;
      time( &t);
      return( ctime( &t));
   }


static void
SysLogOn(void)
{
    static int first = 1;

    if(first) {
	openlog(Application, LOG_PID, LOG_FACILITY);
	first = 0;
    }
    SysLog = TRUE;
}

	

void ERRinit( char *applic, int argc, char *argv[])
   {
      int i;
      int process = FALSE;
      int on = TRUE;
      int from_main = TRUE;

      if (applic != NULL)
	 {
	 if (NULL != (Application = malloc( strlen( applic) + 1)))
	    strcpy( Application, applic);
	 STRncopy( Logfile, applic, MAX_LOGFILE_NAME);
	 STRconcat( Logfile, ".errlog", MAX_LOGFILE_NAME);
	 }
      else
	 from_main = FALSE; 

      for (i=0; i<argc; i++)
   	 {
	 if (STRequal( argv[i], "-ERR"))
	    {
	    if (from_main)
	       Override = TRUE;
	    process = TRUE;
	    continue;
	    }
	 if (!process)
	    continue;

	 if (STRequal( argv[i], "LOG"))
	    Log = on;
	 else if (STRequal( argv[i], "OFF"))
	    on = FALSE;
	 else if (STRequal( argv[i], "ON"))
	    on = TRUE;
	 else if (STRequal( argv[i], "OPS"))
	    {
	    Standard = FALSE;
	    SysLogOn();
	    }
	 else if (STRequal( argv[i], "SLOG"))
	     SysLogOn();
	 else if (STRequal( argv[i], "STD"))
	    Standard = on;
	 else if (STRequal( argv[i], "USR"))
	    User = on;
	 else
	    process =FALSE;
	 }

   }

void ERRcontrolStr( char *cont_str, int max_str)
   {
      cont_str[0] = EOS;

      if (!Standard)
	STRconcat( cont_str, "OFF STD ", max_str);
      else
 	STRconcat(cont_str, "ON STD ", max_str);     


      if (!Log)
	STRconcat( cont_str, "OFF LOG ", max_str);
      else
	STRconcat(cont_str, "ON LOG ", max_str);     


      if (!SysLog)
	STRconcat( cont_str, "OFF SLOG ", max_str);
      else
	STRconcat(cont_str, "ON SLOG ", max_str); 
    
      if (!User)
	STRconcat( cont_str, "OFF USR ", max_str);
      else
	STRconcat(cont_str, "ON USR ", max_str);     
   }


void ERRcontrol( char *control)
   {
#define MAX_TOKEN 10
#define MAX_ARGS  15
      int count, i;
      char *argv[MAX_ARGS], token[MAX_TOKEN];
      char *ptr;
   
      if (Override)
	 return;

      ptr = control;
      if (NULL == (argv[0] = malloc( MAX_TOKEN)))
	 return;
      STRncopy( argv[0], "-ERR", MAX_TOKEN);

      count = 1;
      while ( count < MAX_ARGS)
	{	
        if (NULL == STRtokn( &ptr, token, MAX_TOKEN, " "))
	   break;
        if (NULL == (argv[ count] = malloc( MAX_TOKEN)))
	   break;
        strcpy( argv[ count], token);
	count++;
	}

      ERRinit( NULL, count, argv);

      for (i=0; i< count; i++)
	 free( argv[i]);
   }

void ERRlogfile(char *name)
   {
      STRncopy( Logfile, name, MAX_LOGFILE_NAME);
   }

static void SendOut( int priority, char *message)
   {
      char buffer[ ERR_MESS_MAX];
      static FILE *Logf = NULL;

      /* construct the message */
      snprintf(buffer, ERR_MESS_MAX,
               "%s: %s on %s ", Application, PriorityName[ priority],
	  GetTime());
      STRconcat(buffer, message, ERR_MESS_MAX); 
      STRconcat(buffer, "\n\n",  ERR_MESS_MAX); 

      /* record in a file */
      if (Log)
         {
	 if (Logf == NULL)
	    {
	    errno = 0;
            if (NULL == (Logf = fopen( Logfile, "a")))
               {
               fprintf(stderr, "failed to open error log file <%s> errno = %d\n", 
			Logfile, errno);
	       Standard = TRUE;
	       Log = FALSE;
               }
	    }
	
	 if (Log)
	    {
	    fputs( buffer, Logf);
	    fflush( Logf);
	    }
         }

      /* send to standard error */
      if (Standard)
         {
         fputs( buffer, stderr);
	 fflush( stderr);
         }

      /* user processing */
      if (User)
         {
	 int i;
	 for (i=0; i<UserCount; i++)
	    if (UserOn[i] && (UserFunc[i] != NULL))
	       (UserFunc[i])(priority, message);
	 }

      /* system logger */
      if (SysLog)
        syslog( PriorityLevel[priority] | LOG_FACILITY, message, "%s");
   }


void ERRprintf(int level, char *format, ...)
   {
      va_list  args;
      char     buffer[ ERR_MESS_MAX];

      /* make sure that priority level is in range */
      if ((level < 0) || (level >= MAX_PRIORITIES))
	 level = ERR_PROGRAM;

      /* construct the text to display, using the variable list of args */
      va_start( args, format);
      vsprintf( buffer, format, args);
      va_end( args);

      /* call the outputter */
      SendOut( level, buffer);      
   }

int ERRuser( ERRuserFuncp user)
   {
      if (UserCount < MAX_USERFUNCS)
	 {
	 UserFunc[ UserCount] = user;
	 UserOn[ UserCount] = TRUE;
	 UserCount++;
	 }
      return (UserCount-1 );
   }


void ERRuserActive( int usr_idx, int turnon)
   {
      if ((usr_idx >= 0) && (usr_idx < MAX_USERFUNCS))
	 UserOn[ usr_idx] = turnon;
   }



#ifdef TEST

void Special( int priority, char *message)
{
   printf("begin special\npriority %d <%s> end special\n", priority, message);
}

void main( argc, argv)
   int argc;
   char *argv[];
{
   ERRinit( "Test ERR", argc, argv);
 
   /* send only to stderr */
   ERRprintf(ERR_PROGRAM, "Test this %d\n <%s>", 999, "stderr only");

   /* send only to file errlog */
   ERRcontrol( "OFF STD ON LOG");
   ERRprintf(ERR_ALERT, "Test this %d\n <%s>", 999, "logger only");

   /* also to special processing routine */
   ERRuser( Special);
   ERRcontrol( "ON USR");
   ERRprintf(ERR_RESOURCE, "Test this %d\n <%s>", 999, "logger and special");

   /* send to system log */
   ERRcontrol("OFF LOG USER ON STD SLOG");
   ERRprintf(ERR_INFO, "Test this %d\n <%s>", 999, "standard and slogger");

}
#endif

