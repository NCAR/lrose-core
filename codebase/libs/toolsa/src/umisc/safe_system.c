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
/*********************************************************************
 * SAFE_SYSEM.c Make a system() like call, killing the child if it
 * doesn't exit within the time out. It is "safe" because we don't use
 * SIGALARM and we keep track of the child process ID and send a kill 9 to it
 * 
 * Frank Hage 1998 NCAR, Research Applications Program
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <toolsa/str.h>
 
#define MAX_CMD_LEN 16384
#define MAX_ARGS 256
/**********************************************************************
 * SAFE_SYSTEM. Safely execute a command, susch that it will be killed if it
 * doesn't exit with the timeout_seconds

 * Returns 0 on Success, -1 : on command error,
 *                       -2 : Child Timed out
 *                       -3 : Couldn't kill the child
 */
 
int safe_system(const char * command, int timeout_seconds)

{
   pid_t pid;
   int status;
   char cmd[MAX_CMD_LEN];
   char *cmd_arg[MAX_ARGS];
   char *envStr;
   int num_args;
   int command_len;
   struct timeval tm;

   /* if $BYPASS_SAFE_SYSTEM is true, use system */

   envStr = getenv("BYPASS_SAFE_SYSTEM");
   if (envStr && STRequal(envStr, "true")) {
     return system(command);
   }

   /* First copy the command */
   if(command == NULL) return -1;

   command_len = strlen(command);

   if(command_len >= MAX_CMD_LEN) {
	fprintf(stderr,"Command too long: %d with max of %d\n:: %s\n",
	    command_len,MAX_CMD_LEN,command);
	return -1;
    }

   strncpy(cmd,command,MAX_CMD_LEN-1);

   /* Parse the command string - Note this modifies the string cmd
    * replacing spaces, and quotes with nulls 
    */
   cmd_arg[0] = strtok(cmd,"' ");
   num_args = 1;

   /* Fill the command argument pointer array - The last one is null */
   while((cmd_arg[num_args] = strtok(NULL,"' ")) != NULL
	   && num_args < (MAX_ARGS -1)) num_args++;

   /* Insure the list of arguments is null terminated */
   cmd_arg[MAX_ARGS -1] = NULL;

   /* Fork a child and exec the command */
   if((pid = fork()) < 0) {
	return -1;

   } else {

       if(pid == 0) { /* Child */

	   /* Execute the command */
	   execvp(cmd_arg[0],cmd_arg);
	   _exit(127); /* exec error */

       } else { /* Parent */
	   
	   /* Set a count down and start waiting for the child to exit
	   */
	   while (timeout_seconds-- >= 0) {

	       /* Normal Child Exit - Will be Reaped here */
	       if(waitpid(pid, &status, WNOHANG) != 0) return 0;

	       tm.tv_sec = 1;
	       tm.tv_usec = 0;
	       select(0,NULL,NULL,NULL,&tm); /* Wait a second */
	   } 

	   /* OK. The count down time has expired - Kill the Child */
	   if(kill(pid,9) !=0 ) {
	       perror("safe_sys:");
	       return -3;
	   }

	   if(waitpid(pid, &status, 0) <= 0) return -3;

	   return -2;

       }
   }

}

