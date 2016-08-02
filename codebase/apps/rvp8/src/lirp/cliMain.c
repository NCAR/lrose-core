/******************************************************************************/
/******************************************************************************/
/* Function Name:
     cliMain.c

   Purpose:
      This is the main function for the client process that runs on the host  
      computer for the Level I Recorder.  This is the routine that the user
      interacts with.  It performs three main tasks:  1) it checks for and 
      responds to keyboard input from the user (currently not implemented), 
      2) it checks for and responds to incoming messages from the server, 
      including data messages, and 3) it writes the data to the RAID Array.

   Formal Parameters:  
      hostname (IP Address)
      data request

   Global Variables:
      DataReadBuf[MAX_DATAMSG_SIZE] - buffer to hold the read data message. It 
                                  is big enough to hold one entire data message.
      CntlReadBuf[20b] - generic buffer to hold the read incoming message.
                            Once the message type is determined, this buffer 
                            will be cast to the appropriate message data 
                            structure.
      DataMsgIn - pointer for the incoming data message.  Will point to 
                  DataReadBuf[].
      InfoMsgIn - pointer for incoming error and warning messages.  Will point
                  to CntlReadBuf[].
      CntlMsgIn - pointer to incoming control message ("done" message). Will 
                  point to CntlReadBuf[].
      CntlMsgOut - storage for messages going to the server.
                       

   Function Calls:
      Routines written as a part of Time Series Record and Playback Server.  
      cliCleanUp - closes socket communications.
      cliInitalize - establishes socket communications.

*/
/******************************************************************************/
/******************************************************************************/

#define   EXTERN      /* Define as nothing so variables defined in cliMain.h
                                          will be "non-extern" */
#include <fcntl.h>  /* for creating files */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>    /*  time related routines */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "lirpClient.h"    /* Contains information shared bewteen modules */
#include "lirpErrors.h"    /* Contains information shared bewteen modules */
#include "lirpCommon.h"    /* Information shared between client and server */
#include "rdasubs_lib.h"   /* RVP8 routines for the Az, El conversions */

/******************************************************************************/

/* Local Macros */

#define   DEFAULT_TIME   5      /* Default time value for recording */
#define   INBUF_SIZE     20
#define   MATCH          0      /* Do strings match? */
#define   MAX_FLUSH      5      /* Maximum number of consecutive "misses" */
                                /*   for a socket read */

#define   MAX_REC_TIME   10000000 /* Maximum time in minutes to record I&Q data */
#define   MIN_REC_TIME   0        /* Minimum time in minutes to record I&Q data */
#define   LIRP_PORT      4999   /* Port used on server process for socket */
                              /*   connections */
#define   READY         1     /* Used to check if the file descriptor is */
                              /*   ready for io */
#define  SERVER         7  /* Error source indicator - Error is from Server */
#define  CLIENT         8  /* Error source indicator - Error is from Client */


/******************************************************************************/

/* Type Definitions */

typedef struct sockaddr_in   sockaddr_t;   /* Defined for convenience */
typedef unsigned char      buffer_t;      /* Type for socket buffers */


/******************************************************************************/

/*
 * GLOBAL Variables *
 */

static char      LogFileName[STD_MSG_SIZE+1] = "";   /* Name of log file */
static char      *ProgramName;                     /* Points to name of this */
                                                   /*   program without path */
static int  CreateLogFile = FALSE;          /* Boolean:  create log file? */
static int      RecTime;      /* Requested time for recording (default: 5 min)*/
static int      PackedData = FALSE;   /* Requested packed data */
                                      /*    (default: floating point) */
static int  ErrorSource = CLIENT;     /* Assume error is from Client */

FILE      *LogFp = NULL;                     /* Log file pointer */

unsigned char InfoReadBuf[INBUF_SIZE];   /* global incoming control msg buffer*/
unsigned char DataReadBuf[MAX_DATAMSG_SIZE]; /* global incoming data msg buffer */

struct msgCntl CntlMsgOut;       /* control messge to be sent to server */

struct msgData *DataMsgIn;       /* data message buffer to read in to */
struct msgInfo *InfoMsgIn;       /* information message to read in to */

FILE * raid_fd;          /* file descriptor for RAID output file */
/*  End GLOBAL Definitions */

/******************************************************************************/

/* Function Prototypes */

static int  cliBuildSendMsg (UINT1 action, int c_sd);
static int  clean_up (int c_sd, int d_sd, FILE * raid_fd);
static int  connect_to_server (char *hostname, int *c_sd, int *d_sd);      
static int  create_connection (sockaddr_t *s_addr, int *sd);
static int  cliCreateFilename (char * fn, float Az, float El, int packedData);
static int  flush_data_socket (int sd, buffer_t *buf, int size);
static char *get_program_name (char *path_name);
static int  get_ready (int num_args, char *arguments[]);
static int  parse_cmd_line (int arg_cnt, char *args[]);
static void print_usage (char *prog_name);
static void process_error (int err_code);
static void process_warning (int warn_code);
static int  cliProcessMsgs (int c_sd, int d_sd);
static int  req_param_exist (void);
static int  cliWriteToRAID (long int pulsesReadBuf, 
                            long int indexBB, 
                            FILE ** raid_fd);

extern int  descriptor_ready (int sd, int sec, int io_type);
extern int  chk_and_read (int sd, void *buf, long bytes_to_read);
extern int  chk_and_write (int sd, void *buf, long bytes_to_write);

/******************************************************************************/
/******************************************************************************/

int main (int argc, char *argv[])
{
   int   data_sd;         /* Socket descriptor for data messages */
   int   cntl_sd;         /* Socket descriptor for informational messages */
   int   ret_val;         /* Return value from function */

   /* char  *fn = "Main"; */

/*  ***************************
 *    Initialization Section
 *  ***************************/

   ProgramName = get_program_name (argv[0]);    /* Want name without path */

   /* 5 is the minimum number for argc with the defined command line */
   if (argc < 3)
   {
      print_usage (ProgramName);
      exit (0);
   }

   /*  get_ready opens a log file, and calls parse_cmd_line */
   if ( (ret_val = get_ready (argc, argv)) != SUCCESS )
   {
      process_error (ret_val);
      clean_up (cntl_sd, data_sd, raid_fd);
      exit (ret_val);
   }

   if ((ret_val = connect_to_server (HostName, &cntl_sd, &data_sd)) != SUCCESS )
   {
      process_error (ret_val);
      clean_up (cntl_sd, data_sd, raid_fd);
      exit (ret_val);
   }
   
/*  At this point we're connected to the server, we have parsed the command   */
/*  line.  In the command line there must be a data request.  Now call        */
/*  cliProcessMsgs to do all communications with the server.                  */
   
   
/*  ***************************
 *      Process Messages
 *  ***************************/
   
   /* cliProcessMsgs monitors all the input methods and handles them */ 
   /* it also sends the first data request message from parse_cmd_line */
   /* called from get_ready */
   if ( (ret_val = cliProcessMsgs (cntl_sd, data_sd)) != SUCCESS )
   {
      process_error (ret_val);
      clean_up (cntl_sd, data_sd, raid_fd);
      exit (ret_val);
   }

   clean_up (cntl_sd, data_sd, raid_fd);

   exit (0);

}   /* End of function MAIN */

/******************************************************************************/
/*
   Function Name:
    cliBuildSendMsg

   Purpose:
   This function builds Control Messages to send to the Server.  The Control
   message requests an action from the Server.  Information from the command
   line is passed via this message.  After the message is built, it is
   written to the control socket.

   Formal Parameters:
   UINT1 action      Action requested - Stop, Quit, Data Request
   int   *c_sd       Socket descriptor for control messages

   Global Variables:
   RecTime           Amount of time to record requested by the user.

   Function Calls:
   chk_and_write     Writes message to the specified socket.
*/
/******************************************************************************/
static int cliBuildSendMsg (UINT1 action, int c_sd)
{
   int bc;  /* count of bytes written to socket */
   int done = FALSE;  /* flag for attempting to send data */
   size_t msgSize;
   char  *fn = "cliBuildSendMsg";

   CntlMsgOut.type = MSG_CNTL;
   CntlMsgOut.action = action;
   CntlMsgOut.boundType = DRTIME;
   CntlMsgOut.duration = RecTime;  /* global - in parse_cmd_line */
   CntlMsgOut.packedData = PackedData;  /* global - in parse_cmd_line */
   CntlMsgOut.begAz = 0.0;
   CntlMsgOut.endAz = 0.0;
   CntlMsgOut.begEl = 0.0;
   CntlMsgOut.endEl = 0.0;
   CntlMsgOut.numVCPs = 0;
   msgSize = sizeof (CntlMsgOut);

   do 
   {
      bc = chk_and_write (c_sd, &CntlMsgOut, msgSize); 
      if (bc == 0)
      {
         sprintf (LogMsg,"%s:  Socket not ready, bc = %d\n", fn, bc);
         LOG_MSG();
         PRT_MSG();
         return (ErrorNum);
      }
      else if (bc == FAILURE)
      {      
         ErrorNum = errno;
         sprintf (ExtraMessage, "  Error number %d:  %s \n", 
                      ErrorNum, strerror (ErrorNum));
         return (ErrorCode);
      }
      else if (bc != msgSize)
      {
         sprintf (LogMsg, "  Control message not sent!\n");
         LOG_MSG(); 
         PRT_MSG(); 
         return (ErrorCode);
      }
      else 
      {
         sprintf (LogMsg, "  Sent control message successfully\n");
         LOG_MSG(); 
         PRT_MSG(); 
         done = TRUE;
      }
   } while (!done);

   return (SUCCESS);

} /* end cliBuildSendMsg */

/******************************************************************************/
/******************************************************************************/

static int   clean_up (int c_sd, int d_sd, FILE * raid_fd)

{
   if (raid_fd != NULL)
   {
      sprintf (LogMsg, "\nClosing raid file ...\n");
      LOG_MSG();
      PRT_MSG();
      fclose (raid_fd);
   }

   sprintf (LogMsg, "Closing control and data socket descriptors ...\n");
   LOG_MSG();
   PRT_MSG();

   close (c_sd);
   close (d_sd);

   if (LogFp != NULL)
   {
      sprintf (LogMsg, "\nClosing log file ...\n");
      LOG_MSG();
      PRT_MSG();
      fclose (LogFp);
   }
   return (SUCCESS);
}   /* End of function CLEAN_UP */

/******************************************************************************/
/******************************************************************************/

/*   Function Name:
   connect_to_server

   Purpose:
   This function established a socket connection between the lirp client and
   the lirp server running on the ORDA (currently RVP8).

   Formal Parameters:
   char   *hostname            Hostname of server (RVP8)
   int   *c_sd                  Socket descriptor for control messages
   int   *d_sd                  Socket descriptor for data messages

   Global Variables:
   none

   Function Calls:
   none
*/

static int   connect_to_server (char *hostname, int *c_sd, int *d_sd)      

{
   int            ret_val;         /* Return value from function */
   struct hostent   *host;         /* Contains information on the server (host) */
   sockaddr_t      srv_addr;      /* Internet address of server */

   sprintf (LogMsg, "Getting server host information for host %s ...\n", 
                     hostname); 
   LOG_MSG();
   PRT_MSG();

   /* Get information on host */
   if ( (host = gethostbyname (hostname)) == (struct hostent *) NULL ) 
   {
      return (ERR_HOST_INFO);
   }
   
   /* Now, fill in the fields in the server address structure */

   memset (&srv_addr, 0, sizeof (srv_addr));   /* Clear server address space */
   /* Copy host ip address */
   memcpy (&srv_addr.sin_addr, host->h_addr, host->h_length);
   srv_addr.sin_family = AF_INET;          /* Internet type socket */
   srv_addr.sin_port = htons (LIRP_PORT);  /* Set port used by server process */

   /* Create a socket for informational messages and connect to server   */
   if ( (ret_val = create_connection (&srv_addr, c_sd)) != SUCCESS )
   {                                    
      return (ret_val);
   }

   /*    Create a socket for data messages and connect to server */
   if ( (ret_val = create_connection (&srv_addr, d_sd)) != SUCCESS )      
   {                                    
      return (ret_val);
   }

   /* printf ("\nLeaving connect_to_server...\n" ); */
   return (SUCCESS);      /* It worked! */

}   /* End of function CONNECT_TO_SERVER */

/******************************************************************************/
/******************************************************************************/

/*   Function Name:
   create_connection

   Purpose:
   This function established a socket connection between the tsrp client and
   the tsrp server running on the ORDA (currently RVP8).

   Formal Parameters:
   struct sockaddr_in   s_addr      Pointer to structure containing
                                    socket address information
   int   *sd                        Pointer to socket descriptor

   Global Variables:
   none

   Function Calls:
   none
*/

static int   create_connection (sockaddr_t *s_addr, int *sd)

{
   unsigned int   max_sock_buf = 33554432;     /*   Max receive     */
                                               /* buffer for socket */
   int   attempts;  /* Counter for number of attempts at connecting to server */
   /* char  *fn = "create_connection"; */

   /* Attempt to connect to server a finite number of times */
   for (attempts = 1; attempts < MAX_ATTEMPTS + 1; attempts++)
   {                              
      sprintf (LogMsg, "Creating socket ...\n");
      LOG_MSG(); 
      PRT_MSG(); 

      /* Try to create socket */
      if ( (*sd  = socket (AF_INET, SOCK_STREAM, 0)) < 0 )
      {
         return (ERR_SOCK_GEN);         /* Socket creation failed */
      }                                 /*     so retun error     */

      /* Now let's increase the size of the socket send buffer */
      if ( setsockopt (*sd, SOL_SOCKET, SO_RCVBUF, &max_sock_buf,
                           sizeof (max_sock_buf)) < 0 )
      {
         perror ("Error in setting socket receive buffer\n");
         return (ERR_SOCK_GEN);
      }
      sprintf (LogMsg, "Attempting to connect to server ... attempt #%d\n", 
                  attempts);
      LOG_MSG(); 
      PRT_MSG(); 

      /* Now, try to connect to server process */
      if ( connect (*sd, (struct sockaddr *) s_addr, sizeof (*s_addr)) 
                        == SUCCESS )
      {
         /* Successfully connected to server */
         sprintf (LogMsg, "\nConnected to server %s\n", HostName);
         LOG_MSG(); 
         PRT_MSG(); 
         break;                                                   
      }

      ErrorNum = errno;                     /* Save copy of errno */
      close (*sd);      /* Connect failed, so can no longer use this socket */

      /* Continue if we've got more to go and the error is non-fatal */
      if ( attempts <  MAX_ATTEMPTS  &&
           (ErrorNum == EINPROGRESS || ErrorNum == ECONNREFUSED) )
      {
         sleep (2);      /* Wait two seconds before next attempt */
         printf ("Waiting for lirpServer on RVP8 to come up.... \n");
      }
      else      /*  Still cannot connect after maximum attempts */
               /*    or received a fatal error */
      {
         sprintf (ExtraMessage, "\nErrno is %d => %s\n\n", 
                                    ErrorNum, strerror(ErrorNum));
         return (ERR_CONNECT);
      }
   }

   printf ("Connection to lirpServer on RVP8 established, time series starting.... \n");
   return (SUCCESS);

}   /* End of function CREATE_CONNECTION */

/******************************************************************************/

/*   Function Name:
   cliCreateFilename

   Purpose:
   This function creates the name of the data file written to the RAID.  The
   file name needs to be descriptive enough to know what data is contained 
   in the file.

   Formal Parameters:
   char * outfile       Pointer to the string in which the name is put
   float  fAz           Azimuth of the current pulse, info for the file name
   float  fEl           Elevation of the current pulse, info for the file name
   
   Global Variables:
   none

   Function Calls:
   gmtime               Puts time and date into a structure strftime can use
   time                 Gets time and date of the moment the file is created
   strftime             Converts time and date to a string

*/
/******************************************************************************/

static int  cliCreateFilename (char * outfile, float fAz, float fEl, 
                               int packedData)
{
#define TSIZE 22  /* size of string to hold time - usually 14 characters */
   time_t tmpTime;
   struct tm *dtPtr;
   int iAz, iEl;
   char timeString[TSIZE];
   static int count = 0;  /* emergency counter if time is not available */
   /* char *fn = "cliCreateFilename"; */

   /* truncate the floating point representation of Az and El to integer */
   iEl = fEl * 10.0;  /* want to see 0.5 deg differences */
   iAz = fAz;

   /* get the current time */
   if ((tmpTime = time (&tmpTime)) != FAILURE)
   {
      dtPtr = gmtime (&tmpTime);  /* convert it to GMT and tm struct */
      strftime(timeString, TSIZE, "%Y%m%d_%H%M%SZ", dtPtr);
   }
   else
   {
      /* create an alternative filename if could not get time */
      sprintf (timeString, "no_time_%0.5d", count);
      count ++;
   }

   /* create the filename */
   if ( (packedData == LOWSNRPACK) )
   {
      sprintf (outfile, "rvp8_p_%s_%0.3d_%0.3d", timeString, iEl, iAz);
   }
	else if ( (packedData == HIGHSNRPACK) )
   {
      sprintf (outfile, "rvp8_hp_%s_%0.3d_%0.3d", timeString, iEl, iAz);
   }
   else
   {
      sprintf (outfile, "rvp8_%s_%0.3d_%0.3d", timeString, iEl, iAz);
   }

   /* truncate the floating point representation of Az and El to integer */
   return (SUCCESS);
} /* end cliCreateFileName */
/******************************************************************************/
/******************************************************************************
 * flush_data_socket
 *
 *    Purpose:
 *     This function "flushes the receive buffer for the data socket until no
 *     more data is received.  This is brute force determined by checking the
 *     socket for reading until it is not ready for reading for MAX_FLUSH times.
 *     I am assuming that indicates all the data has been read from the socket
 *     and the receive buffer is empty.  I need to check for other system calls
 *     that may help in this regard.  Perhaps a call from the svr process that
 *     ensures all bytes from the last write have been sent, not just copied to
 *     the send buffer in the kernel.  It returns the total number of bytes read
 *     if successful and FAILURE otherwise.  The global ErrorCode is set in
 *     chk_and_read if an error occurs in that function.
 *
 *     Formal Parameters:
 *      int      sd             Socket descriptor
 *      buffer_t buf            Buffer to contain the data read from socket
 *      int      size           The size of the buffer
 *
 *     Global Parameters:
 *       ErrorNum
 *
 *     Function Calls:
 *       chk_and_read
 *                                                          */

static int  flush_data_socket (int sd, buffer_t *buf, int size)

{
      static buffer_t *IqPtr;     /* Will point to next position in IqBuffer */
      static buffer_t *IqTail;    /* Will point to last address in IqBuffer */
      int   bcount;              /* Number of bytes read */
      /* Number of consecutive times socket not ready for reading */
      int   miss_cnt = 0;
      int   total_cnt = 0;       /* Total number of bytes read while flushing */

      do
      {
         /* printf ("Flushing socket receive buffer ...\n"); */
         /* Read some data from socket */
         if ( (bcount = chk_and_read (sd, buf, size)) > 0 )    
         {
         /* The following lines copy the data from the read buffer into a
          * global buffer.  This allows you to immediately use the buffer again
          * for another "flush."  You may want to create a function to do this
          * ... maybe build_data_buffer (buf, bcount) or something.  That way
          * you can handle the situation where copying the data from the read
          * buffer to the master buffer would write past the end of the master
          * buffer's address space.  Perhaps, you would then copy the master
          * buffer to the RAID, clear the master buffer, copy the read buffer
          * into the master buffer, and set IqPtr to the beginning of the
          * master buffer.
          */

            /* There is room in the "master" buffer */
            if (IqPtr + bcount - 1 <= IqTail) 
            {
               /* Copy data to "master" buffer */
               memcpy (IqPtr, buf, bcount);
               IqPtr += bcount;             /* Update tail pointer */
            }
            else         /* Not enough room left in "master" buffer */
            {
               /* write master buffer to file
                * clear master buffer
                * copy read buffer into master buffer
                * IqPtr = IqBuffer
                */
               sprintf (LogMsg, " No room in master buffer\n"); 
               DEBUG_MSG();
               DEBUGPRT_MSG();
            }
            memset (buf, 0, size);   /* Now, clear read buffer */
            total_cnt += bcount;     /* Update counter */
            miss_cnt = 0;            /* Just read so clear miss count */

            sprintf (LogMsg, " socket ready; read %d bytes\n", bcount);
            DEBUG_MSG();
            DEBUGPRT_MSG();
         }
         else if (bcount == 0)      /* Socket not ready for reading */
         {
            if (miss_cnt > 0)          /* Attempt before this was not */
            {                          /* successful ... so increment */
               miss_cnt++;             /*  consecutive miss counter   */
            }
            else                       /*  Attemp before this was   */
            {                          /*   successful so this is   */
               miss_cnt = 1;           /* considered a "first" miss */
            }

            sprintf (LogMsg, " socket not ready\n"); 
            DEBUG_MSG();
            DEBUGPRT_MSG();
         }
         else       /* An error occurred in chk_and_read */
         {
            return (ErrorCode);     /* ErrorCode is set in chk_and_read */
         }

     } while (bcount > 0  ||  miss_cnt < MAX_FLUSH);

     /* sprintf (LogMsg, "Returning with total_cnt = %d\n", total_cnt);
     DEBUG_MSG();
     DEBUGPRT_MSG(); */

     return (total_cnt);

}  /* End of function FLUSH_DATA_SOCKET */

/******************************************************************************/

/*   Function Name:
   get_program_name

   Purpose:
   This function strips the path from the program name and returns a pointer to
   just the name.  If there is no path before the name, it returns a pointer to
   the string passed as the formal parameter.

   Formal Parameters:
   char   *path_name         Full name of program

   Global Variables:
   none

   Function Call:
   none
*/

/******************************************************************************/

static char   *get_program_name (char *path_name)

{
   char   *name;      /* Points to first character after path */

   /* Full_name contains a slash, i.e., a path */
   if ( (name = strrchr (path_name, '/')) != NULL )
   {
      name++;      /* Increment to point past the slash */
   }
   else            /* No forward slash, '/', in name */
   {               /* so simply set name to argv[0]  */
      name = path_name;
   }

   return (name);

}   /* End of function GET_PROGRAM_NAME */

/******************************************************************************/

/*   Function Name:
   get_ready

   Purpose:
   This function opens file(s), sets default values, etc. in preparation for
   doing the main work of this program.  The global BlocksInGbBound is
   calculated the way it is (with parentheses) to ensure there are no integer
   data overruns.

   Formal Parameters:
   int   num_args           Number of command line arguments
   char   *arguments[]      Pointer to array of command line arguments

   Global Variables:
   bb_size_t   ReqSizeInBB  DataSize in # of BLOCKSIZE blocks and leftover bytes

   Function Calls:
   parse_cmd_line
*/

/******************************************************************************/

static int   get_ready (int num_args, char *arguments[])

{
   int   ret_val;                       /* Return value from function call */

   setbuf (stdout, 0);         /* Flush buffer when writing to standard out */
   setbuf (stderr, 0);         /* Flush buffer when writing to standard error */

   strcpy (ExtraMessage, "");          /*   Set   */
   strcpy (HostName, "");              /* initial */
   RecTime = MAX_REC_TIME * 60;        /* values  */

   /*   Parse command line and ensure parameters are valid */
   if ( (ret_val = parse_cmd_line (num_args, arguments)) != SUCCESS ) 
   { 
      return (ret_val);
   }

   /* Were the required parameters entered on the command line? */
   if (req_param_exist () != SUCCESS) 
   {
      return (ERR_NO_REQ_OPT);
   }

   /* Print informational messages */
   sprintf (LogMsg, "Host name for server process is %s\n", HostName); 
   LOG_MSG();
   PRT_MSG();
   sprintf (LogMsg, "Will request level 1 recording for %d minutes\n", 
               (RecTime/60));
   LOG_MSG(); 
   PRT_MSG();

   /* User wants to create a log file during program run */
   if (CreateLogFile == TRUE) 
   {
      /* Create log file name */
      strcat ( strcpy (LogFileName, ProgramName), ".log" );  

      /* Now, open up log file */
      if ( (LogFp = fopen (LogFileName, "w")) == NULL )
      {
         ErrorNum = errno;
         sprintf (ExtraMessage, "%s\nError number %d:  %s \n\n", 
                     LogFileName, ErrorNum, strerror (ErrorNum));
         return (ERR_FILE_OPEN);
      }
      /* Turn off buffering of log file forcing data to file */
      /*    after every fprintf */
      setbuf (LogFp, NULL);

      sprintf (LogMsg, "Log file %s opened successfully for writing\n", 
                  LogFileName);
      LOG_MSG();
      PRT_MSG();
   }

   /* don't do for now */
   /* printf ("\n> ");  */     /* Write prompt for possible operator input */

   return (SUCCESS);

}   /* End of function GET_READY */

/******************************************************************************/

/*   Function Name:
   parse_cmd_line

   Purpose:
   This function parses the command line arguments.  Legal options begin with
   the dash character, '-'.  It returns SUCCESS if all command line arguments
   are parsed successfully and FAILURE otherwise.

   I'd like to rework this function to make it simpler, perhaps using a while loop ... when I have time.

   Formal Parameters:
   int          arg_cnt            Number of command line arguments
   char          *args[]            Array of strings containing the command line arguments

   Global Variables:
   NoOp

   Function Calls:
   validate_name, validate_ops
*/

/******************************************************************************/

static int  parse_cmd_line (int arg_cnt, char *args[])

{
   int    i;
   int   time;          /* Local copy of recording time in minutes */
   char   *end_char;    /* Points to first character not converted */
                        /*   in call to strtol */
   char   option;       /* Actual command line parameter */
                        /*  (character following the dash, '-') */

   for (i = 1; i < arg_cnt; i++)
   {
      /* A '-' indicates the next character is an option */
      if ( args[i][0] == '-' )   
      {
         if ( args[i][1] != '\0' )      /* A character option exists */
         {
            option = args[i][1];         /* Get the actual character option */
         }
         else                    /* No character option - an error condition */
         {
            return (ERR_NO_OPT);
         }
         /* This switch processes command line options which don't require */
         /* values, checks for expected values for other options, and checks */
         /* for illegal options */
         switch (option) 
         {
            case 'l' :      /* Request to create a log file */
            {
               CreateLogFile = TRUE;
               break;
            }
            case 'p' :      /* Request to create a log file */
            {
               PackedData = TRUE;
               break;
            }
            /* Now make sure values for options that require them exist */
            case 'h' :   
               /* Indicates an expected value does not exist */
               if ( args[i + 1][0] == '-' ) 
               {
                  sprintf (ExtraMessage, "Option %c expects a hostname\n\n", 
                                                                  option);
                  return (ERR_NO_OP_VALUE);
               }
               break;
            case 't' :
            {
               /* Indicates an expected value does not exist */
               if ( atoi(args[i+1]) == 0 ) 
               {
                  sprintf (ExtraMessage,"Option %c expects a numeric value\n\n",
                                                                  option);
                  return (ERR_NO_OP_VALUE);
               }
               break;
            }
            default :
            {
               return (ERR_ILLEGAL_OPT);
            }
         }
      }
      else       /* This should be a value to an option */
      {
         switch (option)    /* Check the option */
         {
            case 'h' :      /* Name of host where server program runs */
            {
               /* Copy name of host into global variable */
               strncpy (HostName, args[i], sizeof (HostName));   
               break;
            }
            case 't' :            /* Recording time */
            {
               /* Retrieve recording time from string */
               time = strtol (args[i], &end_char, 10); 
               if (*end_char != '\0')   /*    Unable to convert as string     */
               {                        /* contained non-numerical characters */
                  sprintf (ExtraMessage, "%s\n\n", args[i]);
                  return (ERR_STR_TO_INT);
               }

               /*   Ensure recording time is within range */
	       if (time < 0) {
		 RecTime = MAX_REC_TIME * 60;
	       } else if ( time >= MIN_REC_TIME  &&  time <= MAX_REC_TIME ) {         
		 RecTime = time * 60; /* It is, so set time - convert to sec */
               } else {  /* Requested recording time is out of range
			  *   - default value used */
		 printf ("\nIllegal time value:  %d.  Recording time must \
                              be between %d and %d inclusive\n",
			 time, MIN_REC_TIME, MAX_REC_TIME);
		 printf ("Recording time will be set to %d minutes\n\n", 
			 DEFAULT_TIME);
               }
               break;
            }
            case '?' : /* List all valid options */
            {
               print_usage (ProgramName);
               break;
            }
            default :
            {
               printf ("Illegal option %s\n", args[i]);
            }
         }
      }
   }

   return (SUCCESS);

}   /* End of fucntion PARSE_CMD_LINE */

/******************************************************************************/

/*   Function Name:
   print_usage

   Purpose:
   This function displays the usage information.

   Formal Parameters:
   char   *prog_name         Program name

   Global Variables:
   none

   Function Calls:
   none
*/

/******************************************************************************/

static void   print_usage (char *prog_name)

{
   printf ("\nUsage:  %s -h <host name> -t <time in min> [-p] [-l]\n", prog_name);
   printf ("        where:\n\n");
   printf ("          -h <host name> host name of machine where server\n");
   printf ("               process is running\n");
   printf ("          -t <time in min> time in minutes to record level 1\n"); 
   printf ("               data\n");
   printf ("          -p  packed data format (default is floating point).\n");
   printf ("          -l  create log file.\n\n");
           
}   /* End of function PRINT_USAGE */

/******************************************************************************/

/*   Function Name:
   process_error

   Purpose:
   This function displays an error message depending on the error code passed
   to it.

   Formal Parameters:
   int   err_code            Number indicating type of error

   Global Variables:
   none

   Function Calls:
   none
*/

/******************************************************************************/

static void   process_error (int err_code)

{
   /* Ensure order of the following error messages is the same as the order
      of the error code macros defined at top of file. */

   static char   *err_msgs[] =
   {   "",
      "Unable to connect to server:  ",                  /* 1,  ERR_CONNECT */
      "Unable to obtain host server information",        /* 2,  ERR_HOST_INFO */
      "No character option following the dash, \'-\'\n", /* 3,  ERR_NO_OPT */
      "Illegal request.  Valid requests are\n",        /* 4,  ERR_ILLEGAL_REQ */
      "Illegal command line option\n",                 /* 5,  ERR_ILLEGAL_OPT */
      "No value entered for a command line option\n",  /* 6,  ERR_NO_OP_VALUE */
      "A required command line option not entered\n",   /* 7,  ERR_NO_REQ_OPT */
      "Unable to open file ",                            /* 8,  ERR_FILE_OPEN */
      "Unable to convert a string to an integer:  ",    /* 9,  ERR_STR_TO_INT */
      "Unable to generate socket\n",                     /* 10, ERR_SOCK_GEN */
      "Error occurred when binding socket to address/port\n", /* 11, ERR_BIND */
      "Unable to create connection queue for socket\n",     /* 12, ERR_LISTEN */
      "Error occurred attempting to accept a socket connection\n", /* 13, 
                                                                   ERR_ACCEPT */
      "Error occurred attempting to read to a socket\n", /* 14, ERR_READ_SOCK */
      "Error occurred attempting to write from a socket\n",  /* 15, 
                                                               ERR_WRITE_SOCK */
      "Error occurred during the select call\n",  /* 16 ERR_SELECT */
      "The RVP8 is not responding to the attach command\n", /* 17 ERR_RVP*_NOT_RESP */
      "The server received an invalid message from the client\n", /* 18 ERR_INVALID MSG */
      "Exceeded maximum number of errors \n",  /* 19 ERR_EXCEED_MAXERR  */
      "Data from RVP8 overwritten and is therefore corrupt\n", /* 20 ERR_DATA_CLOBBERED */
      "Exceeded maximum attempts to write to socket\n",  /* 21 ERR_MAX_TRY_WRITE   */
      "Exceeded maximum attempts to read from socket\n",  /* 22 ERR_MAX_TRY_READ   */
      "Could not write to the RAID Array\n",          /* 23 ERR_RAID_WRITE   */
      "Could not open output file on the RAID Array\n"  /* 24 ERR_FILEOPEN   */
   };

   if (ErrorSource == CLIENT)
      printf ("\n\n\007\007>>>>> CLIENT ERROR <<<<<\n\n");
   else
      printf ("\n\n\007\007>>>>> SERVER ERROR <<<<<\n\n");
   if (err_code > 0) 
   {
      printf ("%s\n", err_msgs[err_code]);
   }
   else
      printf ("Failure");

   if (CreateLogFile == TRUE)     /* Send error message to log file, as well */
   {
      if (ErrorSource == CLIENT)
         fprintf (LogFp, "\n\n\007\007>>>>> CLIENT ERROR <<<<<\n\n");
      else
         fprintf (LogFp, "\n\n\007\007>>>>> SERVER ERROR <<<<<\n\n");
      if (err_code > 0)
         fprintf (LogFp, "%s", err_msgs[err_code]);
      else
         fprintf (LogFp, "Failure");
   }

   /* No match so ExtraMessage contains ... uhh ... an extra message.  */
   /*    Display it */
   if ( strcmp (ExtraMessage, "") != MATCH ) 
   {    
      printf ("%s", ExtraMessage);
      if (CreateLogFile == TRUE)   /* Send error message to log file, as well */
      {
         fprintf (LogFp, "%s", ExtraMessage);
      }

      strcpy (ExtraMessage, "");         /* Re-initialize to null string */
   }
   else            /* Since no extra message printed, need another newline */
   {
      printf ("\n");   
      
      if (CreateLogFile == TRUE)   /* Send error message to log file, as well */
      {
         fprintf (LogFp, "\n");   
      }
   }

}   /* End of function PROCESS_ERROR */

/******************************************************************************/

/*   Function Name:
   process_warning

   Purpose:
   This function displays a warning message depending on the warning code passed
   to it.

   Formal Parameters:
   int   warn_code            Number indicating type of warning 

   Global Variables:
   none

   Function Calls:
   none
*/

/******************************************************************************/

static void   process_warning (int warn_code)

{
   /* Ensure order of the following error messages is the same as the order
      of the warning code macros defined at top of file. */

   static char   *warn_msgs[] =
   {   "",
      "Server is done collecting data for the latest data request\n", /* 101, DATA_REQ_DONE */ 
      "Client requested terminating data collection\n", /* 102 WRN_DATA_REQ_STOP */
      "Client requested that the Server terminate\n",   /* 103 WRN_SERVER_QUIT   */
      "Previous data request terminated, new data request active\n", /* 104 WRN_START_NEW_REQ */
      "Sequence Number is not ok from the RVP8\n",     /* 105  WRN_SEQNUM_NOTOK */
      "Sequence number is expired, missing data!!!\n",   /* 106 WRN_EXPIRED_SEQNUM */
   };

   warn_code -= WARNING_OFFSET;
   printf ("\n\n\007-----  WARNING -----\n");
   if (warn_code > 0) 
      printf ("%s", warn_msgs[warn_code]);
   else
      printf ("Failure");

   if (CreateLogFile == TRUE)     /* Send warnor message to log file, as well */
   {
      fprintf (LogFp, "\n\n\007-----  WARNING -----\n");
      if (warn_code > 0)
         fprintf (LogFp, "%s", warn_msgs[warn_code]);
      else
         fprintf (LogFp, "Failure");
   }

   /* print out sequence number information if the seq num had expired */
   if ( (InfoMsgIn->detail == WRN_EXPIRED_SEQNUM) || 
      (InfoMsgIn->detail == WRN_SEQNUM_NOTOK) )
   {
      printf (" Expired or Old  Sequence Number = %ld\n", InfoMsgIn->expSeqNum);
      printf (" New Sequence Number = %ld\n", InfoMsgIn->newSeqNum);
      if (CreateLogFile == TRUE)
      {
         fprintf (LogFp," Expired or Old Sequence Number = %ld\n", 
                                                         InfoMsgIn->expSeqNum);
         fprintf (LogFp, " New Sequence Number = %ld\n", InfoMsgIn->newSeqNum);
      }
   }

   /* No match so ExtraMessage contains ... uhh ... an extra message.  */
   /*    Display it */
   if ( strcmp (ExtraMessage, "") != MATCH ) 
   {    
      printf ("%s", ExtraMessage);
      if (CreateLogFile == TRUE)  /* Send warnor message to log file, as well */
      {
         fprintf (LogFp, "%s", ExtraMessage);
      }

      strcpy (ExtraMessage, "");         /* Re-initialize to null string */
   }
   else            /* Since no extra message printed, need another newline */
   {
      printf ("\n");   
      if (CreateLogFile == TRUE)  /* Send warnor message to log file, as well */
      {
         fprintf (LogFp, "\n");   
      }
   }
}   /* End of function PROCESS_WARNING */

/******************************************************************************/

/*   Function Name:
   cliProcessMsgs

   Purpose:
   This function is the work horse of the program.  It sends a data request to
   the server via the established socket.  It then receives the requested data
   and calls a function to record the data to the RAID.  It also receives 
   operator input from the keyboard and sends the corresponding message to 
   the server.

   Formal Parameters:
     int   c_sd            Socket descriptor for control messages
     int   d_sd            Socket descriptor for data messages

   Global Variables:
     DataMsgIn             Pointer to data message structure
     DataReadBuf           Buffer to read a data message in to 
     ErrorCode             Indicates if socket error is due to a Select call
                            or the Read call
     ErrorNum              Captures errno

   Function Calls:
     chk_and_read          Reads messages from either the data or control socket
     cliBuildSendMsg       Sends control message to the Server
     cliWriteToRAID        Writes the large data buffer to a file
     flush_data_socket     Empties data socket after request for data ends
     process_error         Processes Errors that occur - usually logs and stops
                              the Server
     process_warning       Processes Warnings that occur - usually logs warning

   Return Values:
     SUCCESS               No problems
     ErrorCode             Error encountered during socket read
     ret_val               Error encounterd in the called function
*/

/******************************************************************************/

static int cliProcessMsgs (int c_sd, int d_sd)
{
   static long indexBB = 0;        /* index of next avail char in DataReadBuf */
   static long pulsesReadBuf = 0;  /* count for # of pulses in DataReadBuf */
   static long rx_cnt = 0;    /* count to print feedback msgs to the screen */
   int    bc;                 /* byte count - number of bytes read */
   int    total_cnt;          /* total byte count from flush */
   int    miss_cnt;           /* number of CONSECUTIVE misses in data read */
   int    attempts;   /* number of nonconsecutive misses in data/control read */
   int    bufIndex;   /* data read buffer index when reading data header or   */
                      /*  control message in chunks (did not get all at once) */
   int    done = FALSE;       /* Haven't finished yet */
   int    ret_val;            /* success/fail indicator returned from called */
                              /*    function */
   int    num_reads = 0;      /* Number of times we've successfully read data */
   char   command_line[STD_MSG_SIZE+1];   /* Will hold requests entered from */
                                          /*    command line */
   int  infobufsize;       /*   this is so that we don't have to call */
   long int readbufsize;   /*   "sizeof" many times                   */

   /* permanent storage to hold data message header information */
   unsigned long  dataMsgBC = 0;  /* byte count holder */
   unsigned long  dataMsgPC = 0;  /* pulse count holder */
   long int       bytesLeft = 0;

   long int       reqBytes;

   /* char  *fn = "cliProcessMsgs"; */


   /* printf ("\n>  ");  */                     /* Print prompt */


   /* get sizes of input buffers here */
   infobufsize = sizeof (struct msgInfo);  /* get number of bytes TO READ */
   readbufsize = sizeof (DataReadBuf);
   /* add one for keyboard buffer */

   do
   {
      /* only get here if there was a command on the command line */
      /* send it first before monitoring for input */
      if ((ret_val = cliBuildSendMsg (CNTL_DATA_REQ, c_sd)) == FAILURE)
      {
            return (ret_val);  /* can't send data request - stuck! */
      }

      /*  Loop, checking information messages on control socket from server, 
       *  data messages on data socket from server, and finally, keyboard input
       *  by user.  Exit loop when server sends a done message or a fatal error
       *  occurs.                                                             */
      for (; ;)
      {
         /***********************************************************/
         /* First, check for messages from server on control socket */
         /*        !!!Assumption is that we are ONLY receiving      */
         /*                 Information Message Types!!!            */
         /***********************************************************/

         /* clear buffer before using */
         memset (&InfoReadBuf, (int) NULL, infobufsize);  

         if ( (bc = chk_and_read (c_sd, InfoReadBuf, infobufsize)) == FAILURE )
         {
            return (ErrorCode);        /* ErrorCode set in chk_and_read */
         }
         /* if we did not get all the message, try to read the rest of it */
         else if ((bc > 0) && (bc < infobufsize))
         {
            bytesLeft = infobufsize - bc;
            bufIndex = bc;
            do {
               if ((bc = chk_and_read(c_sd, &DataReadBuf[bufIndex], bytesLeft)) 
                              == FAILURE )
               {
                  return (ErrorCode);      /* ErrorCode set in chk_and_read */
               }
               else if (bc == 0)  
               {
                  if (miss_cnt > 0)
                  {
                     miss_cnt++;
                  }
                  else
                  {
                     miss_cnt = 1;
                  } 
               }
               if (++miss_cnt > CONTIN_MISS)
               {
                  sprintf (ExtraMessage, "Can't read remaining information message\n");
                  return (ERR_MAX_TRY_READ);
               }
               bytesLeft -= bc;
               bufIndex += bc;
            } while (bytesLeft > 0);
            bc = bufIndex;
            miss_cnt = 0;
         }
         /* ok, we've done our best to read in all of the control message */
         /*  if we've gotten it all, then process it                      */
         if (bc == infobufsize) 
         {
            InfoMsgIn = (struct msgInfo *) InfoReadBuf;
            /* InfoMsgIn->type always == MSG_INFO */
            if (InfoMsgIn->severity == INFO_ERR )
            {
               ErrorSource = SERVER;
               process_error (InfoMsgIn->detail);
               /* set Error Source back to Client */
               ErrorSource = CLIENT;
               /* tell server to stop */
               if ((ret_val = cliBuildSendMsg (CNTL_QUIT, c_sd)) 
                              == FAILURE)
               {
                     process_error (ret_val);
               }
               done = TRUE;  /* exit the client */
               break;
            }
            else /* InfoMsgIn->severity == INFO_WARN */
            {
               if (InfoMsgIn->detail == DATA_REQ_DONE)
               {
                  process_warning (DATA_REQ_DONE);
                  /* Need to "flush" the DATA socket receive buffer in the */
                  /*   kernel */
                  do   /* repeat until buffer is empty (bc = 0) */
                  {
                     if ( (bc = flush_data_socket (d_sd, DataReadBuf, 
                                                 readbufsize)) == FAILURE )
                     {
                        return (ErrorCode); /* exit upon error during read */
                     }
                     total_cnt += bc;       /* Update total flushed */

                  } while (bc != 0);

                  done = TRUE;  /* Set flag to break out of loop */
                  break;  /* break out of for (;;) loop */
               }
               else /* it is just another warning message */
               {
                  process_warning (InfoMsgIn->detail);
               }
            }  /* else severity == WARN */
         } /* if bc == infobufsize */

         /**************************************************************/
         /* Second, check for data messages from server on data socket */
         /*   Only get one message by getting the header, finding the  */
         /*   total number of bytes sent, then getting the remaining   */
         /*   number of bytes, then write that message to the RAID     */
         /**************************************************************/
         
         miss_cnt = 0;
         dataMsgBC = 0;
         bytesLeft = 0;

         /* get header for data message first so that we know how many */
         /*  bytes to read */
         /* memset (DataReadBuf, (int) NULL, DATAINFO);  */
         if ((bc = chk_and_read (d_sd, DataReadBuf, DATAINFO)) == FAILURE )
         {
            return (ErrorCode);      /* ErrorCode set in chk_and_read */
         }
         else if (bc == 0)  
         {
            if (miss_cnt > 0)
            {
               miss_cnt++;
            }
            else
            {
               miss_cnt = 1;
            }
         }
         else if ((bc < DATAINFO) && (bc > 0)) 
         {
            /* read again until all the data message header bytes are read */
            bytesLeft = DATAINFO - bc;
            bufIndex = bc;
            do {
               if ((bc = 
                     chk_and_read (d_sd, &DataReadBuf[bufIndex], bytesLeft)) 
                                        == FAILURE )
               {
                  return (ErrorCode);   /* ErrorCode set in chk_and_read */
               }
               if (++attempts > CONTIN_MISS)
               {
                  sprintf (ExtraMessage, "Can't read remaining data header\n");
                  return (ERR_MAX_TRY_READ);
               }
               bytesLeft -= bc;
               bufIndex += bc;
            } while (bytesLeft > 0);

            bc = bufIndex;
         }
         /* want to fall to here if able to read all the data header */
         if (bc == DATAINFO)  /* got all the data message hdr info */
         {
            miss_cnt = 0;  /* reset continuous misses if we get data */
            num_reads ++;  /* do this for testing purposes */
            /* save all data msg header info because the input buffer */
            /* will be written over */
            DataMsgIn = (struct msgData *) DataReadBuf;
            dataMsgBC = DataMsgIn->totalBytes;
            bytesLeft = DataMsgIn->totalBytes;
            dataMsgPC = DataMsgIn->numPulses;
            /* now loop and get all the data message with multiple reads */
            /* clear out buffer to read in to */
            /* memset (DataReadBuf, (int) NULL, readbufsize); */
            while ((bytesLeft > 0) && (miss_cnt < CONTIN_MISS))
            {
               if (bytesLeft > readbufsize)
               {
                  reqBytes = readbufsize;
               }
               else
               {
                  reqBytes = bytesLeft;
               }
               
               if((bc = chk_and_read (d_sd, &DataReadBuf[indexBB], reqBytes))
                                    == FAILURE )
               {
                  return (ErrorCode);
               }
               else if (bc == 0)  
               {
                  if (miss_cnt > 0)
                  {
                     miss_cnt++;
                  }
                  else
                  {
                     miss_cnt = 1;
                  } 
               }
               else  /* read data - increment index into DataReadBuf */
               {
                  indexBB += bc;  
                  bytesLeft -= bc; /* decr number of bytes to read */
                  miss_cnt = 0;  /* clear miss_cnt */

                  sprintf (LogMsg, 
                          "!!!read and copied %d bytes into BigBuf\n", bc);
                  DEBUG_MSG();
                  DEBUGPRT_MSG(); 
               }
            } /* end while (bytesLeft > 0) */
            if (miss_cnt >= CONTIN_MISS)
            {
               sprintf (LogMsg, "Exceeded Continuous Misses count\n", bc);
               LOG_MSG();
               PRT_MSG();
               sprintf (ExtraMessage, "Can't read remaining data message\n");
               return (ERR_MAX_TRY_READ); /* no data to read, socket not ready */
            }
            else /* got the entire data message so write it to a file */
            {
               miss_cnt = 0;
               /* count num pulses in DataReadBuf */
               pulsesReadBuf += dataMsgPC; 
               /* write the contents of BigBuf to disk, reset */
               if ((ret_val = 
                        cliWriteToRAID (pulsesReadBuf,indexBB, &raid_fd))
                                    == SUCCESS)
               {
                  sprintf (LogMsg, 
                      "!!!read and copied %ld bytes into file\n", indexBB);
                  DEBUG_MSG();
                  DEBUGPRT_MSG(); 
                  indexBB = 0;  
                  pulsesReadBuf = 0;  
               }
               else  /* write to RAID failed */
               { 
                  /* if error, send message to server to stop, log it */
                  return (ret_val);
               } 
               /* the following is a 'user feedback' message */
               rx_cnt++;
               if ((rx_cnt % 100) == 0) printf (".");
               if ((rx_cnt % 1000) == 0)
               {
                  /* ALWAYS print these */
                  printf ("\n Receiving Data \n");
                  if (LogFp != NULL)
                     fprintf (LogFp,"\n Receiving Data \n");
               }
            }
         } /* end else (get pulse header and I/Q information) */
         
         /********************************************/
         /* Last, check for input from the keyboard  */
         /********************************************/

        /*****????? This is not working !!!!!!!!!! ****************/
         memset (&command_line, (int) NULL, sizeof (command_line));
         if ( (bc = chk_and_read (fileno (stdin), command_line,
                                     sizeof (command_line))) > 0)   
         {
            /* ??? do call to str2tok and get cmd line in same format */
            /*  as argc and argv for parse_cmd_line */
            memset (command_line, (int) NULL, sizeof (command_line));
         }
         else if (bc == FAILURE)   /* Some error occurred */
         {
            return (ErrorCode);         /* ErrorCode set in chk_and_read */
         }
      } /* end for (;;) */  

   } while (!done); 

   return (SUCCESS);

}   /* End of function cliProcessMsgs */

/******************************************************************************/

/*   Function Name:
   req_param_exist

   Purpose:
   This function checks for the required parameters that should be entered on
   the command line.  It returns SUCCESS if they were and FAILURE otherwise.

   Formal Parameters:
   none

   Global Variables:
   char  HostName              Name of host where server program resides
   int   RecTime               Requested time for recording

   Function Calls:
   none
*/

/******************************************************************************/

static int   req_param_exist (void)

{
   if (strcmp (HostName, "") == MATCH)         /* Hostname for server process */
   {
      sprintf (ExtraMessage, "Hostname for server process not entered.\n");
      return (FAILURE);
   }

   if (RecTime == 0)            /* Length of time to record level 1 data */
   {
      sprintf (ExtraMessage, "Time to record level 1 data not entered.\n");
      return (FAILURE);
   }

   return (SUCCESS);

}   /* End of REQ_PARAM_EXIST */

/******************************************************************************/

/*   Function Name:
      cliWriteToRAID  

   Purpose:
   This function writes the contents of DataReadBuf, the read buffer, to
   the RAID Array.  It returns SUCCESS if everything was written and FAILURE 
   otherwise.

   Formal Parameters:
     pulsesReadBuf      Number of pulses in DataReadBuf (in one data message)
     bytesBB            Number of bytes of data in the data read buffer
     raid_fd            File descriptor for the file to write to on the RAID

   Global Variables:
     char  DataReadBuf  Buffer to be written to the RAID 

   Function Calls:
     fPDegFromBin2      Sigmet routine to convert binary angle to float. 
                           Values range between 0 - 360
     fwrite
*/

/******************************************************************************/

static int cliWriteToRAID
                     (long int pulsesReadBuf, long int bytesBB, FILE ** raid_fd)
{
#define DEG_IN_CIRCLE 360
#define NAMESIZE 80
   static int currSect = -1;     /* Current elevation being written to a file */
                                 /* when this changes, open a new file        */
   static int azBound = 0;     /* boundary to determine if new file is needed */
   float az, el;         /* holds azimuth and elevation from the pulse header */
   struct pulseData *pulsePtr;  /* ptr to current pulse header information */
                                   /*     being written to the file */
   long int bbIndex;            /* array index into BigBuf */
   long int startIndex;         /* beginning array index into BigBuf if a     */
                                /*  chunk of BigBuf has already been written  */
   long int bytesToWrite;       /* Number of bytes in BigBuf to write to file */
   long int pulseSize;     /* storage to hold structure size to increment ptr */
   int hdrSize;            /* storage to hold structure size of pulse pointer */
   const float numSect = 4;     /* sectors per elev scan and # files per scan */
   int sectorWidth;
   int   i, ret_val;
   int   iAz;         /* use integer math to figure out which sector az is in */
   char outfilename[NAMESIZE];    /* file name for RAID output file */

   /* char  *fn = "cliWriteToRaid"; */

   sectorWidth = DEG_IN_CIRCLE / numSect;
   pulsePtr = (struct pulseData *) DataReadBuf; /* cast BigBuf to the pulse */
                                                /* header structure so */
                                                /* we can see what is in it */
   bbIndex = 0;
   startIndex = 0;
   hdrSize = sizeof (struct pulseData);  /* store size of pulse header */

   for (i = 0; i < pulsesReadBuf; i++)
   {
     az = (pulsePtr->header.iAz / 65536.0) * 360.0; /* avoid use of SIGMET function */
     el = (pulsePtr->header.iEl / 65536.0) * 360.0; /* avoid use of Sigmet function */
     /* az = fPDegFromBin2( pulsePtr->header.iAz );  convert Sigmet's        */
     /* el = fPDegFromBin2( pulsePtr->header.iEl );  binary to degrees     */
      iAz = az;                     /* cast float to int */
      azBound = iAz / sectorWidth;  /* results in an integer sector number */
     
      /* Files are being created based on a 90 degree quadrant in az */
      /* if azimuth boundary is exceeded */
      /*  or if for some reason the file descriptor is null - open new file */
      if ((*raid_fd == NULL) || (azBound != currSect)) 
      {
         /*  write the pulses already checked into the file before closing it */
         /* startIndex and bbIndex define what part (or all) of DataReadBuf will be */
         /*  written to the file.  Now write the data to the file */
         if (*raid_fd != NULL)
         {
            bytesToWrite = bbIndex - startIndex;
            if (bytesToWrite > 0)
            {
               /* write the data in DataReadBuf to the file before closing it */
               ret_val = fwrite(&DataReadBuf[startIndex], 1, bytesToWrite, *raid_fd);
               if ((ret_val == 0) || (ret_val < bytesToWrite))
               {
                  /* if we could not write to the RAID, return an error */
                  sprintf(LogMsg,"  bbIndex = %ld, startIndex = %ld, ret_val = %d\n", 
                              bbIndex, startIndex, ret_val);
                  DEBUG_MSG();
                  DEBUGPRT_MSG();
                  /* if we could not write to the RAID, return an error */
                  return (ERR_RAID_WRITE);  
               }
               startIndex = bbIndex;  /* reset beginning to next pulse */
            }
            if (fclose (*raid_fd) != SUCCESS)
            {
               sprintf (LogMsg, 
              "Failed to close current data file before opening new file\n");
               LOG_MSG();
               PRT_MSG();
               *raid_fd = NULL;
            }
         }

         cliCreateFilename (outfilename, az, el, pulsePtr->packedData);

         /* open file - open/create to append */
         *raid_fd = fopen(outfilename, "ab");
         if (*raid_fd == NULL)
         {
            sprintf (LogMsg, "\n Error opening file: %s\n", outfilename);
            LOG_MSG();
            PRT_MSG();
            /* fix this error code */
            sprintf (ExtraMessage, "Error opening file: %s\n", outfilename);
            return (ERR_FILEOPEN);
         }

         /* adjust azimuth boundary to determine new files */
         currSect = azBound;
      }

      /* Calculate number of bytes in this pulse */
      /* the first 2 is the number of components in the vector - I and Q  */
      if ((pulsePtr->packedData == LOWSNRPACK) || (pulsePtr->packedData == HIGHSNRPACK))
      {
         pulseSize = hdrSize + pulsePtr->header.iNumVecs * 2 * 2; 
      }
      else
      {
         pulseSize = hdrSize + pulsePtr->header.iNumVecs * 2 * sizeof(float);
      }

      /* adjust the index into DataReadBuf to include this pulse */
      /*  in the next write to the file                     */
      bbIndex += pulseSize;
      /* point to next pulse in the structure */
      pulsePtr = (struct pulseData *) &DataReadBuf[bbIndex];   

   } /* end for loop through pulses */

   /* startIndex and bbIndex define what part (or all) of DataReadBuf will be */
   /*  written to the file.  Now write the data to the file */
   if (*raid_fd != NULL)
   {
      bytesToWrite = bbIndex - startIndex;
      if (bytesToWrite > 0)
      {
         /* write the data in DataReadBuf to the file before closing it */
         ret_val = fwrite(&DataReadBuf[startIndex], 1, bytesToWrite, *raid_fd);
         if ((ret_val == 0) || (ret_val < bytesToWrite))
         {
            /* if we could not write to the RAID, return an error */
            sprintf(LogMsg,"  bbIndex = %ld, startIndex = %ld, ret_val = %d\n", 
                        bbIndex, startIndex, ret_val);
            DEBUG_MSG();
            DEBUGPRT_MSG();
            /* if we could not write to the RAID, return an error */
            return (ERR_RAID_WRITE);  
         }
      }
   }

   return (SUCCESS);
}  /* end cliWriteToRAID */
