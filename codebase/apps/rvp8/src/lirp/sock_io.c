/* This program contains the socket connection routines used in the 
 	server process for the Level I Record and Playback (LIRP) program.
	gtm
	3/18/2003
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define	EXTERN	extern			/* Defined as extern here so variables in tsrpClient.h parsed correctly */

/******************************************************************************/
#define VERBOSE
#ifdef VERBOSE 
#define  LOG_MSG() if (LogFp != NULL) fprintf (LogFp, "%s", LogMsg)
#define  PRT_MSG() fprintf (LogFp, "%s", LogMsg)
#elif !VERBOSE
#define  LOG_MSG()
#define  PRT_MSG()
#endif
/******************************************************************************/

#include "lirpClient.h"
#include "lirpErrors.h"

/******************************************************************************/

/* Global Variables */

EXTERN FILE	*LogFp;   

/******************************************************************************/

/* Function Prototypes */

int	descriptor_ready (int sd, long usec, int io_type);
int	chk_and_read (int sd, void *buf, long bytes_to_read);
int	chk_and_write (int sd, void *buf, long bytes_to_write);

/******************************************************************************/

#define	IO_OP(x) ( ( (x) == READING ) ? "reading" : "writing" )

/******************************************************************************/
/******************************************************************************/

/*	Function Name:
	descriptor_ready

	Purpose:
	This function checks if a file descriptor is ready for reading, writing, or
	an out-of bounds message operation.  It returns the number of file
	descriptors ready for the requested io operation.  The only valid return
	values are 0 or 1.  See comment prior to the return call for more
	information.  If an error occurs in the select call, it returns -1.

	Formal Parameters:
	int	fd						File descriptor to check
	int	sec					Number of seconds to wait before returning from the
									select call if the descriptor is not immediately
									ready for the requested io operation
	int	io_type				Type of io operation to check for:
										READIG - reading
										WRITING - writing
										OUT_OF_BNDS - out-of-bounds message

	Global Variables:
	none

	Function Calls:
	none
*/

int	descriptor_ready (int sd, long usec, int io_type)

{
	int				num_ready = 0;				/* Number of file descriptors ready for io (should be 0 or 1) */
	fd_set			io_set;					/* Set of bits for file descriptors for reading or writing */
	struct timeval	wait;						/* Determines how long to wait in the select call before returning */

/* char				*fn = "DESCRIPTOR_READY"; */

	wait.tv_sec = 0;						/* Set time in seconds to wait */
	wait.tv_usec = usec;							/* Dont' worry about microseconds */

	FD_ZERO (&io_set);						/* Clear all bits for the file descriptors */
	FD_SET (sd, &io_set);					/* Set the bit for this file descriptor for reading or writing */

   errno = EINTR;
   while (errno == EINTR)
   {
      errno = 0;    /* if read is successful, errno will not change and we'll break out of the loop */
      switch (io_type)							/* Io_type determines i/o operation to check for */
      {
         case READING :							/* Check this file descriptor for reading */
         {
            num_ready = select (sd + 1, &io_set, NULL, NULL, &wait);
            break;
         }
         case WRITING :							/* Check this file descriptor for writing */
         {
            num_ready = select (sd + 1, NULL, &io_set, NULL, &wait);
            break;
         }
         default :								/* Check this file descriptor for out-of-bounds message */
         {
            num_ready = select (sd + 1, NULL, NULL, &io_set, &wait);
            break;
         }
      }

      if (num_ready == FAILURE)				/* Error occurred in select call */
      {
         if (errno != EINTR)
         {
            ErrorNum = errno;
            ErrorCode = ERR_SELECT;
            printf ("socket error, setting ExtraMessage\n");
            sprintf (ExtraMessage, "%s\n", strerror (ErrorNum));
            printf ("descriptor_ready: %s\n", ExtraMessage);
            return (num_ready);		/* If error is not EINTR, return to break out of loop */
         }
         /* else if errno == EINTR, try select call again */
      }
   }  /* end while */

/* Select returns the number of descriptors ready for the requested io
 	operation(s).  Normally you would call FD_ISSET (sd, &io_set) to find out
	if the particular descriptor sd is ready for the io operation.  It is not
	necessary in this case as we only set one descriptor.  Since we only set
	one descriptor, select only checks one descriptor for the requested io
	operation.  So, the only positive value that select can return is 1 and a
	return of 1 means that the lone descriptor we are interested in is ready
	for the io operation.  Hence, no need to call FD_ISSET. */

/*
switch (num_ready) {
	case -1 : {
		printf ("%s:  leaving with select error\n", fn);
		sprintf (LogMsg, "%s:  leaving with select error\n", fn);
		break;
	}
	case 0 : {
		printf ("%s:  leaving with socket %d not ready for %s\n", fn, sd, IO_OP(io_type));
		sprintf (LogMsg, "%s:  leaving with socket %d not ready for %s\n", fn, sd, IO_OP(io_type));
		break;
	}
	default : {
		printf ("%s:  leaving with socket %d ready for %s\n", fn, sd, IO_OP(io_type));
		sprintf (LogMsg, "%s:  leaving with socket %d ready for %s\n", fn, sd, IO_OP(io_type));
		break;
	}
} 
LOG_MSG(); */

	return (num_ready);						/* Return number of file descriptors ready for io; should be 0 or 1 */

}	/* End of function DESCRIPTOR_READY */

/******************************************************************************/
/******************************************************************************/

/*	Function Name:
	chk_and_read

	Purpose:
	This function checks if the passed file descriptor is available for reading.
	If it is, the function reads from the file descriptor into the passed
	buffer.  It returns the number of bytes read on success and FAILURE (-1)
	otherwise.

	Formal Parameters:
	int	*fd					Socket descriptor from which to read
	void	*buf					Buffer to contain the bytes read
	long	bytes_to_read		Number of bytes to read

	Global Variables:
	int	ErrorNum

	Function Calls:
	none
*/

int	chk_and_read (int fd, void *buf, long bytes_to_read)

{
	int	bytes_read = 0;				/* Number of bytes read (haven't read anything, yet) */
	long	useconds = 8000.0; 			/* Number of seconds to wait before returning from select */
												/* We want to return immediately, so set to 0 */
/*	char	*fn = "CHK_AND_READ"; */


	switch ( descriptor_ready (fd, useconds, READING) )
	{
		case -1 :							/* An error occurred in descriptor_ready (the select call) */
		{
			return (FAILURE);          /* ErrorCode is set in descriptor_ready, tells which call failed */
			break;
		}
		case 0 :								/* File descriptor not ready for reading so just reeturn */
		{
			break;
		}
		default :							/* File descriptor is ready for reading */
		{
		/*	sprintf (LogMsg, "%s:  before call to read %d bytes\n", fn, bytes_to_read);
			LOG_MSG(); */

         errno = EINTR;
         while (errno == EINTR)
         {
            errno = 0;
            if ( (bytes_read = read (fd, buf, bytes_to_read)) < 0 )		/* Some error occurred */
            {
               if (errno != EINTR)
               {
                  ErrorNum = errno;														/* Save error number */
                  ErrorCode = ERR_READ_SOCK;
                  sprintf (ExtraMessage, "%s\n", strerror (ErrorNum));
                  return (FAILURE);
               }
               /* else, ignore EINTR and try to read again */
            }
         } /* end while */
			break;
		}
	}

	return (bytes_read);

}	/* End of function CHK_AND_READ */

/******************************************************************************/
/******************************************************************************/

/*	Function Name:
	chk_and_write

	Purpose:
	This function checks if the passed file descriptor is available for writing.
	If it is, the function writes to the file descriptor from the passed buffer.
	It returns the number of bytes written on success and FAILURE (-1)
	otherwise.

	Formal Parameters:
	int	*fd					File descriptor to which to write
	void	*buf					Buffer to contain the bytes to write
	long	bytes_to_write		Number of bytes to write

	Global Variables:
	int	ErrorNum

	Function Calls:
	none
*/

int	chk_and_write (int fd, void *buf, long bytes_to_write)

{
	int	bytes_written = 0;			/* Number of bytes written (haven't written anything, yet) */
	long  useconds = 8000.0; 			/* Number of seconds to wait before returning from select */
												/* We want to return immediately, so set to 0 */
/*	char	*fn = "CHK_AND_WRITE"; */

	switch ( descriptor_ready (fd, useconds, WRITING) )
	{
		case -1 :							/* An error occurred in descriptor_ready (the select call) */
		{
			return (FAILURE);          /* ErrorCode is set in descriptor_ready, tells which call failed */
			break;
		}
		case 0 :								/* File descriptor not ready for reading so just reeturn */
		{
			break;
		}
		default :							/* File descriptor is ready for reading */
		{
         errno = EINTR;
         while (errno == EINTR)
         {
            errno = 0;
            if ( (bytes_written = write (fd, buf, bytes_to_write)) < 0 )		/* Some error occurred */
            {
               if (errno != EINTR)
               {
                  ErrorNum = errno;																/* Save error number */
                  ErrorCode = ERR_WRITE_SOCK;
                  sprintf (ExtraMessage, "%s\n", strerror (ErrorNum));
                  return (FAILURE);
               }
               /* else, ignore EINTR and try to write again */
            }
         } /* end while */
			break;
		}
	}

	return (bytes_written);

}	/* End of function CHK_AND_WRITE */

/******************************************************************************/
/******************************************************************************/
