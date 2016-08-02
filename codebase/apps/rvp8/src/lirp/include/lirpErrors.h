
/* This header file contains macros, global variables, and type definitions
 	for reporting error messages.  All aspects of the lirp use this code - 
   communications, server, and client.
	dss - gtm
	4/25/2003
*/

/******************************************************************************/

/* define maximum error count macros */
#define CONTIN_MISS     500  /* max number of tries to read/write to socket */
                             /*  when the socket is not ready               */

/* Error codes - ensure this order the same as error messages in process_error */

#define	ERR_CONNECT			1		/* Unable to connect to server */
#define	ERR_HOST_INFO		2		/* Unable to obtain information about the    */
                                 /*    host server                            */
#define	ERR_NO_OPT			3		/* No character option followed the dash on  */
                                 /*    command line                           */
#define	ERR_ILLEGAL_REQ	4		/* Illegal request                           */
                                 /*    (legal requests are read or write)     */
#define	ERR_ILLEGAL_OPT	5		/* Illegal command line option               */
#define	ERR_NO_OP_VALUE	6		/* An expected value for an option was not   */
                                 /*    on command line                        */
#define	ERR_NO_REQ_OPT		7		/* Required command line option not entered  */
#define	ERR_FILE_OPEN		8		/* Error attempting to open a file           */
#define	ERR_STR_TO_INT		9		/* Error in converting a string to an        */
                                 /*    integer                                */
#define	ERR_SOCK_GEN		10		/* Unable to create socket                   */
#define	ERR_BIND				11		/* Error when binding socket ip address/port */
#define	ERR_LISTEN			12		/* Error when creating connection queue for  */
                                 /*    socket                                 */
#define	ERR_ACCEPT			13		/* Error when attempting to accept a socket  */
                                 /*    connection                             */
#define	ERR_READ_SOCK		14		/* Error when trying to read from a socket   */
#define	ERR_WRITE_SOCK		15		/* Error when trying to write to a socket    */
#define	ERR_SELECT			16		/* Error occurred during the select call     */
#define  ERR_RVP8_NOT_RESP    17     /* rvp8 is not responding to attach cmd  */
#define  ERR_INVALID_MSG      18     /* server received an invalid message    */
#define  ERR_EXCEED_MAXERR    19     /* count for allowed errors is exceeded  */
#define  ERR_DATA_CLOBBERED   20     /* data in rvp8 buffer was overwritten   */
                                     /*   while copying it to a buffer        */
#define  ERR_MAX_TRY_WRITE    21     /* Exceeded CONTIN_MISS during write     */
#define  ERR_MAX_TRY_READ     22     /* Exceeded CONTIN_MISS during read      */
#define  ERR_RAID_WRITE       23     /* Client could not write to the RAID    */
#define  ERR_FILEOPEN         24     /* Client could not open output file on  */
                                     /*   the RAID    */


/*   Warning  details     */

#define WARNING_OFFSET       100

#define  DATA_REQ_DONE       101     /* client is done collecting data for    */
                                     /*  latest data request */
#define  WRN_DATA_REQ_STOP   102     /* feedback message to client - server   */
                                     /*   stopped collecting data at the      */
                                     /*   client's request                    */
#define  WRN_SERVER_QUIT     103     /* feedback message to client - server   */
                                     /*   is shutting down at client's reqest */
#define  WRN_START_NEW_REQ   104     /* a new data request is received from   */
                                     /*   client, server is terminating the   */
                                     /*   previous req                        */
#define  WRN_SEQNUM_NOTOK    105     /* NOT SENDING THIS! rvp8 still filling  */
                                     /*   current pulse iwith data            */
#define  WRN_EXPIRED_SEQNUM  106     /* data associated with this  sequence   */
                                     /*   number is expired                   */


