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
#ifndef SOCKUTIL_WAS_INCLUDED
#define SOCKUTIL_WAS_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <dataport/port_types.h>
#include <toolsa/globals.h>
#include <toolsa/servmap.h>


  /*
   * Heartbeat function typedef
   */

  typedef void (*SKU_heartbeat_t)(const char *label);
  
  extern int SKUerrno; /* for passing errno back to calling routine */

  /* old message header struct - this is/was used by the TDWR 
   * products
   */

  typedef struct {
    si16 id;
    ui16 len; /* (message length + header size) / 2 */
    si32 seq_no;
  } SKU_old_header_t;

  /* new message header structs - the new message header is 8 bytes
   * of code (all f0) followed by the SKU_header_t
   */

  typedef struct {
    si32 id;
    si32 len; /* message size in bytes, not including header */
    si32 seq_no;
  } SKU_header_t;

  /**********************************************************************
   * The following is the "lowest level" access to socket calls.
   * There is no use of message headers; all calls are blocking and
   * work with one fd at a time. 
   * Error messages to stderr.
   */

  extern int SKU_read_select(int sd, long wait_msecs);
  /**************************************
   * SKU_read_select - waits for read access
   *
   * returns 1 on success, -1 on timeout, -2 on failure
   */

  extern int SKU_write_select(int sd, long wait_msecs);
  /**************************************
   * SKU_write_select - waits for write access
   *
   * returns 1 on success, -1 on timeout, -2 on failure
   */

  extern void SKU_close(int fd);
  /* shutdown and close this socket; may be used on
   * server or client socket
   */

  extern void SKU_close_no_hangup(int fd);
  /* close the socket without doing anything else.  This
   * should be used by the parent process if it spawns a
   * child process to handle communications over the
   * socket.
   */

  extern int SKU_get_client(int protofd);
  /* Waits until a client has connected on the server socket "protofd",
   * then accepts the client and returns its file descriptor.
   * Return   =  0 : success
   *          = -1 : accept failure (Note: Different from SKU_get_client_timed)
   */

  extern int SKU_get_client_timed(int protofd, long wait_msecs);
  /* Waits until a client has connected on the server socket "protofd",
   * or until "wait_msecs" millisecs has passed, (0= no wait, -1= wait forever)
   * Accepts the client and returns its file descriptor.
   * Return = -1 : timeout
   *        = -2 : select failure	
   *        = -3 : accept failure	
   * Not available under AIX
   */

  extern int SKU_open_client(const char *hostname, int port);
  /* Open an Internet socket stream as a client to the 
   * server "hostname, port"
   *
   * Returns: >= 0 file descriptor
   * 	    < 0:  errors: 
   *   -1 = Could not find host name 
   *   -2 = Could not setup socket (max file descriptors?)
   *   -3 = Could not connect to specified host and port;
   *        get SKUerrno for reason 
   */

  extern int SKU_open_client_timed(const char *hostname,
				   int port, long wait_msecs);
  /*************************************************************************
   * SKU_open_client_timed: Open an Internet socket stream to server on
   * hostname at port: port.  Timeout if the connection can't be made
   * within the given time period.
   *
   * Returns file discriptor or error;
   * Errors: 
   *   -1 = Could not find host name 
   *   -2 = Could not setup socket (max file descriptors?)
   *   -3 = Could not connect to specified host and port 
   *   -4 = Timed out trying to connect
   */

  extern int SKU_open_unix_client(const char *sock_file);
  /*********************************************************************
   * SKU_open_unix_client: Open a unix socket stream to server on
   * hostname at port: port
   *
   * Returns file discriptor or error;
   * Errors: 
   *   -1 = Could not find host name 
   *   -2 = Could not setup socket (max file descriptors?)
   *   -3 = Could not connect to specified host and port 
   */

  extern int SKU_open_server(int port);
  /* Open up an Internet Stream socket for use as a server
   * and puts it into listen mode.
   * Returns server filedescriptor: use this in next accept call
   * OR if < 0 an error: 
   *   -1 = Could not open socket (max file descriptors ?)
   *   -2 = Could not bind to specified port
   *   -3 = Could not listen;  get SKUerrno for reason 
   */

  extern int SKU_open_unix_server(const char *sock_file);
  /************************************************************************
   * SKU_open_unix_server: Open up a Unix Stream socket for use as a server
   * and puts it into listen mode.
   * Returns proto filedescriptor: use this in next accept call
   * OR : 
   *   -1 = Could not open socket (max file descriptors ?)
   *   -2 = Could not bind to specified port
   *   -3 = Could not listen 
   */


  extern long SKU_read(int fd, void *message, long len, int retries);
  /* Read a  message of length "len" bytes from socket fd;
   * allow "retries" maximum errors before quitting (-1 = default)
   * returns number of bytes read.
   * Note that you must know the exact length of the message.
   */

  extern long SKU_read_timed(int fd, void *message, long len,
			     int retries, long wait_msecs);
  /* try to read until "wait_msecs" millisecs has passed,
   * (0= no wait, -1= wait forever)
   * Return -1 if timed out.
   * 
   * read a  message of length "len" bytes from socket fd;
   * allow "retries" maximum errors before quitting (-1 = default)
   * returns number of bytes read.
   * Note that you must know the exact length of the message.
   *
   * Note also that the timeout period applies to the time until
   * the first byte of the message is received.  After that, this
   * routine will not return until the entire message is received
   * or more than retries errors have occurred.
   */

extern long SKU_read_timed_hb(int fd, void *mess, long len,
			      int retries, long wait_msecs,
			      int chunk_size,
			      SKU_heartbeat_t heartbeat_func);
  /***************************************************************************
   * SKU_read_timed_hb
   *
   * Reads a simple message of given length.  Executes given heartbeat
   * function periodically during read operation.  This is useful for
   * reading large buffers of data off of slow connections.
   *
   * chunk_size indicates the number of bytes to read in each chunk.
   * Chunks are read until the desired message length is reached or
   * the select() before a chunk times out.
   *
   * Memory allocated by calling routine
   *
   * Blocks if wait_msecs == -1
   *
   * Reads chunks until a select times out. Returns nbytes read.
   * If it times out before any data is received, returns -1.
   */

  extern long SKU_write(int fd, const void *message, long len, int retries);
  /* Write the message of length "len" bytes to socket fd;
   * allow "retries" maximum errors before quiting (-1 = default)
   * returns number of bytes written.
   */

  /***********************************************************************
   * These calls are built on top of the above calls; they assume that you
   * are reading and writing using headers to indicate message length, etc.
   * The "old" message header is the LL/RAP TDWR header already in use.
   * The "new" header is designed to deal with messages > 64K in length.
   * Note that the readh() routine automatically detects which type
   * of header is being read, sets SKU_use_old_header appropriately. Writeh()
   * uses SKU_use_old_header to decide which type of header to output. Readh()
   * always returns the header info in a new header structure, however.
   * Note that these routines automatically convert the headers to/from
   * network byte order.
   */

  extern int SKU_use_old_header;
  /* This is set by SKU_readh() to TRUE if the latest message used the
   * old header, or to FALSE if it used the new header.
   * SKU_writeh() uses this variable to control whether it sends out the
   * old header or the new header; you may set this variable immediately
   * before a call to SKU_writeh() to control its behavior.
   */

  extern int SKU_read_header(int sockfd, SKU_header_t *header,
			     long wait_msecs);
  /* read just the header of a message
   * note it is converted (if need be) to new type.
   * wait wait_msecs millisecs; if not read by then, return -1
   */

  /* write just the header of a message
   * returns 1 on success, -1 on failure
   */
  extern int SKU_write_header(int sockfd, long len,
			      long product_id, long seqno);

  extern int SKU_readh(int fd, void *message, long max_mess,
		       SKU_header_t *header, long wait_msecs);
  /* Read a message with maximum length max_mess bytes. 
   * wait wait_msecs millisecs; if not read by then, return -1
   * a header is assumed to precede the actual message; it is passed back
   * in *header (note converted to new type)
   * Returns : 1 = success
   * -1 = failure
   * -2 = message exceeds max_mess
   */

  extern int SKU_read_message(int fd, SKU_header_t *header, char **messagep,
			      long *data_len, 
			      long wait_msecs);
  /* alternate interface (from Mike Dixon)
   * SKU dynamically allocates the message for you;
   * do NOT free() it!!
   * returns 1 on success, -1 on error
   */

  extern int SKU_writeh(int fd, const void *message, long len, 
			long product_id, long seqno);
  /* Write the message of length "len" bytes to socket fd. (Note NOT the
   * "old" header length). The product_id and seqno go into the header.
   * You may set the seqno to -1 and the system will automatically
   * sequence for you. Note that automatic sequencing is global for all
   * sockets; if you are interleaving messages to different sockets, the
   * messages arriving at a given socket will have gaps in the sequence
   * number.
   * Return 1 on success, -1 on failure
   */

  extern int SKU_writeh_timed(int sockfd, void *data, long len,
			      long product_id, long seqno,
			      long wait_msecs);
  /*
   * timed version of SKU_writeh
   */

  extern int SKU_write_message(int sockfd, long product_id,
			       const char *data, long len);
  /* alternate interface (from Mike Dixon) same as SKU_writeh except 
   * seqno is handled internally here
   */

  extern void SKU_set_headers_to_old(void);
  extern void SKU_set_headers_to_new(void);

  /***** named server interface *****************/

  /*
   * named server struct - this is initialized and then used in
   * reads with auto_connect
   */

  typedef struct {
    int port;
    int default_port;
    char servmap_host_1[SERVMAP_HOST_MAX];
    char servmap_host_2[SERVMAP_HOST_MAX];
    char type[SERVMAP_NAME_MAX];
    char subtype[SERVMAP_NAME_MAX];
    char instance[SERVMAP_INSTANCE_MAX];
    char host[SERVMAP_HOST_MAX];
    char default_host[SERVMAP_HOST_MAX];
    int fd;
  } SKU_named_server_t;

  extern int SKU_init_named_server(SKU_named_server_t *server,
				   const char *servmap_host_1,
				   const char *servmap_host_2,
				   const char *type,
				   const char *subtype,
				   const char *instance,
				   const char *default_host,
				   int default_port);

  extern int SKU_open_named_conn(SKU_named_server_t *server);

  extern void SKU_close_named_conn(SKU_named_server_t *server);

  extern int SKU_read_message_named(SKU_named_server_t *server,
				    SKU_header_t *header,
				    char **data,
				    long *data_len,
				    long wait_msecs);

  extern int SKU_read_message_auto_connect(SKU_named_server_t *server,
					   SKU_header_t *header,
					   char **data,
					   long *data_len,
					   long wait_msecs);

  extern int SKU_write_message_named(SKU_named_server_t *server,
				     long product_id,
				     char *data, long len);


  extern int SKU_read_select(int sd, long wait_msecs);
  extern int SKU_write_select(int sd, long wait_msecs);

#ifdef __cplusplus
}
#endif

#endif
