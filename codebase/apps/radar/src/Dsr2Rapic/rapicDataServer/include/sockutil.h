#ifdef __cplusplus
 extern "C" {
#endif
#ifndef SOCKUTIL_WAS_INCLUDED
#define SOCKUTIL_WAS_INCLUDED

/* SCCS info
 *   %W% %D% %T%
 *   %F% %E% %U%
 *
 * RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2007/10/25 03:37:27 $
 *   $Id: sockutil.h,v 1.1 2007/10/25 03:37:27 dixon Exp $
 *   $Revision: 1.1 $
 *   $State: Exp $
 *
 *   $Log: sockutil.h,v $
 *   Revision 1.1  2007/10/25 03:37:27  dixon
 *   Adding BOM rapic server code for building Dsr2Rapic
 *
 *   Revision 1.1.1.1  2003/06/19 01:19:27  pjp
 *   3DRapic Sources
 *
 *   Revision 1.2  1999/02/04 05:54:06  sandy
 *   Rapic version 372
 *
 * Revision 3.26  1997/11/11  23:34:09  pjp
 * 3DRapic Source Code
 *
 * Revision 3.24  1997/10/23  05:11:54  pjp
 * 3DRapic Source Code
 *
 * Revision 3.24  1997/10/23  05:11:54  pjp
 * 3DRapic Source Code
 *
 * Revision 3.23  1997/10/17  08:40:08  pjp
 * 3DRapic Source Code
 *
 * Revision 3.23  1997/10/17  08:40:08  pjp
 * 3DRapic Source Code
 *
 * Revision 3.19  1997/08/29  03:46:37  pjp
 * 3DRapic Source Code
 *
 * Revision 3.19  1997/08/29  03:46:37  pjp
 * 3DRapic Source Code
 *
 * Revision 3.19  1997/08/29  03:15:29  pjp
 * 3DRapic Source Code
 *
 * Revision 1.2  94/11/18  01:18:43  dixon
 * Added cplusplus linking directive
 * 
 * Revision 1.1.1.1  1994/11/17  18:55:21  dixon
 * Initial import
 *
 * Revision 2.1  1994/09/22  22:14:02  deirdre
 * Change preprocessor symbol 'LINT' to 'lint', which is correct.
 *
 * Revision 2.0  1993/11/24  02:17:10  deirdre
 * Bring up to date with current revision
 *
 * Revision 1.1.1.1  1993/11/24  02:15:15  deirdre
 * Move to subdirectory
 *
 * Revision 2.0  1993/10/21  20:35:50  deirdre
 * New revision. DemVal 1st Release.
 *
 * Revision 1.5  1993/06/19  01:34:27  caron
 * old header length -> unsigned short
 *
 * Revision 1.4  1993/04/28  20:58:17  caron
 * *** empty log message ***
 *
 * Revision 1.3  1993/04/28  18:01:51  caron
 * add Mike Dixon SKU_read_message() routine
 *
 * Revision 1.2  1993/04/20  22:32:13  roach
 * *** empty log message ***
 *
 * Revision 1.1  93/04/16  13:46:35  roach
 * Initial checkin.
 * 
 */

#ifndef lint
static char RCS_id_sku[] = "$Id: sockutil.h,v 1.1 2007/10/25 03:37:27 dixon Exp $";
static char SCCS_id_sku[] = "%W% %D% %T%";
#endif /* not lint */

extern int SKUerrno;  /* for passing errno back to calling routine */

/**********************************************************************
 The following is the "lowest level" access to socket calls.
 There is no use of message headers; all calls are blocking and
 work with one fd at a time. 
 Error messages to stderr.
 */

extern void SKU_close( int fd );
/* shutdown and close this socket; may be used on
   server or client socket
 */

extern int SKU_get_client( int protofd );
/* Waits until a client has connected on the server socket "protofd",
   then accepts the client and returns its file descriptor.
   Return < 0 = error
 */

extern int SKU_get_client_timed( int protofd, long wait_msecs );
/* Waits until a client has connected on the server socket "protofd",
   or until "wait_msecs" millisecs has passed, (0= no wait, -1= wait forever)
   Accepts the client and returns its file descriptor.
   Return = -1 : timeout
	  = -2 : error	
  Not available under AIX
 */

extern int SKU_open_client( char *hostname, int port );
/* Open an Internet socket stream as a client to the 
 * server "hostname, port"
 *
 * Returns: >= 0 file descriptor
 * 	    < 0:  errors: 
 *   -1 = Could not find host name 
 *   -2 = Could not setup socket (max file descriptors?)
 *   -3 = Could not connect to specified host and port; get SKUerrno for reason 
 */

extern int SKU_open_server( int port );
/* Open up an Internet Stream socket for use as a server
 * and puts it into listen mode.
 * Returns server filedescriptor: use this in next accept call
 * OR if < 0 an error: 
 *   -1 = Could not open socket (max file descriptors ?)
 *   -2 = Could not bind to specified port
 *   -3 = Could not listen;  get SKUerrno for reason 
 */

extern long SKU_read( int fd, void *message, long len, int retries );
/* Read a  message of length "len" bytes from socket fd;
   allow "retries" maximum errors before quitting (-1 = default)
   returns number of bytes read.
   Note that you must know the exact length of the message.
 */

extern long SKU_read_timed( int fd, void *message, long len, int retries, long wait_msecs );
/* try to read until "wait_msecs" millisecs has passed, (0= no wait, -1= wait forever)
   Return -1 if timed out.

   read a  message of length "len" bytes from socket fd;
   allow "retries" maximum errors before quitting (-1 = default)
   returns number of bytes read.
   Note that you must know the exact length of the message.
 */

extern long SKU_write( int fd, void *message, long len, int retries );
/* Write the message of length "len" bytes to socket fd;
   allow "retries" maximum errors before quiting (-1 = default )
   returns number of bytes written.
 */

/***********************************************************************
 These calls are built on top of the above calls; they assume that you
 are reading and writing using headers to indicate message length, etc.
 The "old" message header is the LL/RAP TDWR header already in use.
 The "new" header is designed to deal with messages > 64K in length.
 Note that the readh() routine automatically detects which type
 of header is being read, sets SKU_use_old_header appropriately. Writeh()
 uses SKU_use_old_header to decide which type of header to output. Readh()
 always returns the header info in a new header structure, however.
 Note that these routines automatically convert the headers to/from
 network byte order.
 */

/* old message header struct - this is/was used by the TDWR 
 * products
 */

typedef struct {
  short id;
  unsigned short len;   /* (message length + header size ) / 2 */
  long seq_no;
  } SKU_old_header_t;

/* new message header structs - the new message header is 8 bytes
 * of code (all f0) followed by the SKU_header_t
 */

typedef struct {
  long id;
  long len;	/* message size in bytes, not including header */
  long seq_no;
  } SKU_header_t;
  
extern int SKU_use_old_header;
/* This is set by SKU_readh() to TRUE if the latest message used the
   old header, or to FALSE if it used the new header.
   SKU_writeh() uses this variable to control whether it sends out the
   old header or the new header; you may set this variable immediately
   before a call to SKU_writeh() to control its behavior.
 */

extern int SKU_read_header(int sockfd, SKU_header_t *header, long wait_msecs);
/* read just the header of a message
   note it is converted (if need be) to new type.
   wait wait_msecs millisecs; if not read by then, return -1
 */

extern int SKU_readh( int fd, void *message, long max_mess,
		        SKU_header_t *header, long wait_msecs );
/* Read a message with maximum length max_mess bytes. 
   wait wait_msecs millisecs; if not read by then, return -1
   a header is assumed to precede the actual message; it is passed back
   in *header (note converted to new type)
   Returns : 1 = success
	     -1 = failure
	     -2 = message exceeds max_mess
 */

extern int SKU_read_message(int fd, SKU_header_t *header, char **messagep, long *data_len, 
	long wait_msecs);
/* alternate interface (from Mike Dixon): SKU dynamically allocates the message for you;
 * do NOT free() it!!
 * returns 1 on success, -1 on error
 */

extern int SKU_writeh( int fd, void *message, long len, 
				long product_id, long seqno );
/* Write the message of length "len" bytes to socket fd. (Note NOT the
   "old" header length). The product_id and seqno go into the header.
   You may set the seqno to -1 and the system will automatically
   sequence for you.  Note that automatic sequencing is global for all sockets;
   if you are interleaving messages to different sockets, the messages
   arriving at a given socket will have gaps in the sequence number.
   Return 1 on success, -1 on failure
 */

extern int SKU_write_message(int sockfd, long product_id, char *data, long len);
/* alternate interface (from Mike Dixon) same as SKU_writeh except 
   seqno is handled internally here */

extern void SKU_set_headers_to_old(  void );
extern void SKU_set_headers_to_new(  void );
/* alternate access to SKU_use_old_header */


#endif
#ifdef __cplusplus
}
#endif
