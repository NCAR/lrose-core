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

/********************************************************************
* TEST TCP/IP SOCKET RECEIVING PROGRAM.
* Program is designed to give some ideas as to how to receive GTS
* style messages on a TCP/IP Socket connection.
********************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include  <memory.h>
#include  <sys/socket.h>
#include  <netinet/in.h>
#include  <netdb.h>
#include <sys/time.h>

#define SERVICE 	39000
#define MAX_MSGSIZE 	1000000
#define MAX_BUFLEN  		MAX_MSGSIZE + 100
#define  SOH     		'\001'
#define  ETX     		'\003'
#define  GTS_LENFIELD  		8
#define  GTS_SOCKET_HEADER  	10

static void SetupService();
static void RecvData();
static void AcceptConnection();
static int  ExtractMsg(char *buffer, int *buflen);
static int  CheckMsgBoundaries (char *, int);
static int  FindMessage (char *, int, int *);
static void ShiftBuffer (char *, int *, int);

static struct sockaddr_in dest;

static int	pr_sock, msgsock;
static char	buffer[MAX_BUFLEN+1];
static int	buflen = 0;

/********************************************************************
*				MAIN
* Listen for incoming IP calls and read any incoming messages on 
* the first call established.
*
* 1. Ignore SIGPIPE signals. These are generated if a connection
*    is lost. By default they cause a program to terminate.
* 2. Set-up a listening socket for incoming msgs (SetupService)
* 3. Accept the first call received (AcceptConnection)
* 4. Read any messages on this connection (RecvData)
* 5. Close the call and close the listening socket.
********************************************************************/
main(int argc, char *argv[])
{

int y = 1;

while (y == 1)
{
	signal (SIGPIPE,SIG_IGN);

	SetupService();
	AcceptConnection();
	RecvData();

	close(msgsock);
	close(pr_sock);
}
}

/********************************************************************
*			SETUP SERVICE
* Listen for calls on a given Service/Port.
* 1. Create a socket
* 2. Set the socket KEEPALIVE option.
*    This enables the automatic periodic transmission of "check" 
*    messages to be sent on the connection. If the destination 
*    does not respond then it is considered broken and this process
*    is notified (by SIGPIPE or end-of-file)
* 3. Set the socket REUSEADDR option. Enable quicker restarting of
*    terminated processes.
* 4. Bind the socket to the required Service/Port
* 5. Start listening for calls.
********************************************************************/
static void SetupService()
{
int	on = 1;
int	rc;

memset ((char *)&dest, 0, sizeof dest);
dest.sin_addr.s_addr = INADDR_ANY;
dest.sin_family = AF_INET;
dest.sin_port = htons(SERVICE);

pr_sock = socket (AF_INET, SOCK_STREAM, 0);
if (pr_sock < 0) {
   printf("sock error\n");
   exit(1);
   }

 rc = setsockopt(pr_sock,SOL_SOCKET,SO_KEEPALIVE,(char *)&on,sizeof(on));
 if (rc != 0) {
    printf("keepalive error\n");
    exit(1);
    }
 rc = setsockopt(pr_sock,SOL_SOCKET,SO_REUSEADDR,(char *)&on,sizeof(on));
 if (rc != 0) {
    printf("reuse error\n");
    exit(1);
    }

rc = bind(pr_sock,(struct sockaddr *)&dest,sizeof dest);
if ( rc < 0) { 
   printf("bind error\n");
   exit(1);
   }


rc = listen(pr_sock,1);
if ( rc < 0) { 
   printf("listen error\n");
   exit(1);
   }

printf("listening\n");
}


/********************************************************************
*			ACCEPT CONNECTION
* Wait for an incoming call (accept).
* Return the socket of the call established.
********************************************************************/
static void AcceptConnection()
{
int	addrlen;

printf("waiting connection\n");


addrlen = sizeof(dest);
printf("%d\n",addrlen);
msgsock = accept (pr_sock,&dest,&addrlen);
if ( msgsock < 0) { 
   printf("accept error\n");
   exit(1);
   }
printf("connected\n");
}

/********************************************************************
*			RECV DATA
* Read data from the message/call socket.
* Extract GTS messages from this data.
* Keep reading until the sender drops the call or there is an error.
********************************************************************/
static void RecvData()
{
int     numr = 1;
int     rc = 0;

while (numr > 0 && rc >= 0) {
    numr = read(msgsock,buffer+buflen, MAX_BUFLEN-buflen);
    if (numr > 0) {
       buflen += numr;
       buffer[buflen] = '\0';
       /*printf("buffer = %s\n",buffer);*/
       rc = ExtractMsg(buffer,&buflen);
       }
   }
}

/********************************************************************
*			EXTRACT MSG
*   DESCRIPTION
*   This function accepts a buffer of data on input, along with the 
*   amount of data in the buffer, and extracts GTS messages from this
*   buffer.  
* 
*   Messages that are in the buffer are identified as follows...
* 
*   - The first 8 bytes of the message buffer HAVE to be a message 
*     length in character format. 
*     If the length exceeds the GTS defined maximum message size, or
*     does not consist of numeric characters, then an error is returned
*     (lost synchronisation).
*
*   - Immediately following the message length is a 2 character
*	Message Type: "AN" = Alphanumeric, "BI" = binary, “FX” = Fax
* 
*   - The GTS message begins with a SOH character, and is terminated
*     with a ETX character, if this does not occur, then an error is
*     returned (lost synchronization).
* 
*   - If a GTS message is identified, then it is extracted and the
*     message is shifted out of the buffer.
* 
*   - As there may be more than 1 message in the buffer, this function
*     will loop (extracting messages) until either and 
*     error or incomplete message is detected.
*  
*  
*   RETURNS     = 0  - Not a complete message in the buffer.
* 		< 0  - Fatal error in the format of the buffer.
*  		> 0  - Success, the message(s) have been extracted
********************************************************************/
static int ExtractMsg(char *buffer, int *buflen)
{
int   	rc, msglen;
char	msg[MAX_MSGSIZE+1];
char filename[200],lfilename[200];
char tempstr[20],timetemp[128];
int milisec;
FILE *out_file,*logfile;
time_t timedata;
struct tm *ts_struct;
struct timeval tmb;
struct timezone tmz;


/* FIND THE FIRST MESSAGE IN THE BUFFER */
rc = FindMessage (buffer, *buflen, &msglen);
   
/* WHILE A VALID MESSAGE LENGTH IS FOUND IN THE MESSAGE BUFFER... */
while ( rc > 0 ) {

  /* ENSURE THAT THE FIRST CHARACTER AFTER THE MESSAGE LENGTH IS
     A 'SOH' CHARACTER, AND THE LAST CHARACTER AS INDICATED BY 
     THE MESSAGE LENGTH IS AN 'ETX' CHARACTER. */
  if ( (rc = CheckMsgBoundaries (buffer, msglen)) < 0 ) 
     continue;
   
  /* PRINT THE EXTRACTED MESSAGE */
 /* Added + 10 for Ingest system */ 
  memcpy(msg,buffer+GTS_SOCKET_HEADER+10,msglen);
  msg[msglen] = '\0';
  
  /* Insert: Print extracted message to file as well as header to logfile ...*/
 /* printf("GTS MSG = \n%s\n",msg);*/
        gettimeofday(&tmb,&tmz);
        milisec = tmb.tv_usec;
        while (milisec >=  100)
        {
                milisec /= 10;
        }
        ts_struct = localtime(&tmb.tv_sec);
	/* Mercury - sprintf(filename,"/usr2/dbnsawb/pending/WMO.0111.WA92.0.0.%d%02d.0",tmb.tv_sec,milisec);*/
 	sprintf(filename,"/home/dbnsawb/pending/WMO.0111.WA92.0.0.%d%02d.0",tmb.tv_sec,milisec);
	/* Andrew - sprintf(filename,"/home/andrew/pending/WMO.0111.WA92.0.0.%d%02d.0",tmb.tv_sec,milisec);*/
        sprintf(lfilename,"WMO.0111.WA92.0.0.%d%02d.0",tmb.tv_sec,milisec);
        /* Mercury - logfile = fopen("/usr2/dbnsawb/log/receive_data.log","a"); */
	logfile = fopen("/home/dbnsawb/log/receive_data.log","a");
	/* Andrew - logfile = fopen("/home/andrew/receive_data.log","a"); */
        if ((out_file = fopen(filename,"wb")) != NULL)
        {
            memset(tempstr,0,20);
            memcpy(&tempstr[0],&msg[0],18);
            strftime(timetemp,128,"%Y/%m/%d %T",ts_struct);
            fprintf(logfile,"%s - %s - %s\n",tempstr,timetemp,lfilename);
            fprintf(out_file,"%s",msg);
            fclose(out_file);
        }
        fclose(logfile);
    
  /* SHIFT THE JUST INJECTED MESSAGE OUT OF THE MESSAGE BUFFER,
     AND LOOP BACK TO LOOK FOR A NEW MESSAGE.  */
    
  ShiftBuffer (buffer, buflen, msglen);

  /* FIND THE FIRST MESSAGE IN THE SHIFTED BUFFER */
  rc = FindMessage (buffer, *buflen, &msglen);

  }
    
return (rc);
}

/********************************************************************
*			FIND MESSAGE
* Check that the complete message is at the start of the buffer.
* 1. Check the first 8 characters which are the message length
* 2. Check the next 2 characters - Message Type
* 3. Check that the complete message, as defined by the "message length"
*    field, is in the buffer.
* Return codes:
*     0 = message incomplete
*     1 = message complete
*    -1 = error
********************************************************************/
static int  FindMessage (char *buffer, int buflen, int *mlen)
{
   char  charlen[GTS_LENFIELD+1];
   int   intlen;

   *mlen = 0;

   /* IF THE LENGTH OF THE PASSED MESSAGE BUFFER IS NOT GREATER THAN
      10 CHARACTERS THEN RETURN 'INCOMPLETE'.   */
   if ( buflen < GTS_SOCKET_HEADER ) {
      return (0);
      }

   /* CHECK THAT THE MESSAGE TYPE IS VALID */
   if (strncmp(buffer+GTS_LENFIELD,"AN",2) && strncmp(buffer+GTS_LENFIELD,"BI",2) && strncmp(buffer+GTS_LENFIELD,"FX",2)) {
      printf("ERROR: Message Type field invalid");
      return (-1);
      }

   /* EXTRACT THE MESSAGE LENGTH */
   strncpy (charlen, buffer, GTS_LENFIELD);
   charlen[GTS_LENFIELD] = '\0';

   /* CHECK THAT THE MESSAGE LENGTH CHARACTER STRING COMPRISES 
      ENTIRELY OF DIGITS.  RETURN AN ERROR IF THIS IS NOT THE CASE. */
   if ( strspn (charlen, "0123456789") != strlen (charlen) ) {
      printf("ERROR: length not numeric");
      return (-1);
      }
   
   /* CONVERT THE MESSAGE LENGTH CHARACTER STRING TO AN INTEGER. */
   intlen = atoi (charlen);
    
   /* CHECK THAT THE LENGTH EXTRACTED FROM THE BUFFER IS NOT GREATER
      THAN THE GTS DEFINED MAXIMUM MESSAGE SIZE - RETURN AN ERROR IF
      THIS IS THE CASE. */
   if ( intlen > MAX_MSGSIZE ) {
      printf("ERROR: message overlength");
      return (-1);
      }

   /* CHECK IF THE ENTIRE MESSAGE HAS BEEN RECEIVED. RETURN IF NOT */
   if ( buflen < intlen + GTS_SOCKET_HEADER ) {
      return (0);
      }

   *mlen = intlen;
   return (1);
}

/********************************************************************
*			   CHECK MSG BOUNDARIES
* Confirm the first character after the Socket Header is
* a SOH, and the last character in the message (given by the message
* length) is an ETX.
********************************************************************/
static int  CheckMsgBoundaries (char *buffer, int msglen)
{

   /*  CHECK THAT THE FIRST CHARACTER (AFTER THE MESSAGE LENGTH
       FIELD) IS AN SOH CHARACTER - RETURN AN ERROR IF IT ISN'T. */
   if ( buffer[GTS_SOCKET_HEADER] != SOH ) {
      printf("ERROR: SOH not found\n");
      return (-1);
      }
   
   /*  CHECK THAT THE LAST CHARACTER (ACCORDING TO THE MESSAGE LENGTH
       FIELD) IS AN ETX CHARACTER - RETURN AN ERROR IF IT ISN'T. */ 
   if ( buffer[msglen+GTS_SOCKET_HEADER-1] != ETX ) {
      printf("ERROR: ETX not found\n");
      return (-1);
      }

   return (1);
}

/********************************************************************
*			   SHIFT BUFFER	
* Shift the leading message in the buffer out of the buffer.  This may
* either empty the buffer, or move all or part of a new message to the
* start of the buffer.
********************************************************************/
static void  ShiftBuffer (char *buffer, int *buflen, int msglen)
{
   int  shiftlen;

   /* CALCULATE THE AMOUNT OF DATA TO BE SHIFTED OUT OF THE BUFFER. */
   shiftlen = msglen + GTS_SOCKET_HEADER;

   /* SHIFT THE 'PROCESSED' DATA  OUT OF THE BUFFER BY MOVING THE
      UNPROCESSED DATA OVER THE TOP OF IT. 
      CALCULATE THE NEW AMOUNT OF DATA IN THE BUFFER. */
   *buflen = *buflen - shiftlen;
   memcpy (buffer, buffer + shiftlen, *buflen);
}
