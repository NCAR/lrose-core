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
#ifndef SOK2_WAS_INCLUDED
#define SOK2_WAS_INCLUDED

#include <time.h>

/* 

KEYWORD: socket, sockets, interprocess communication, IPC, client, server

NAME

   SOK2 - socket library; 2 way client/server IPC

   Created: 5/12/92 JCaron.
   Modified: 
	9/24/92: add SOK2oldMessage()
	9/30/92: add SOK2_SERVMAP to setServiceFile, add SOK2register(),
		SOK2servmapInfo()
	3/19/93: clarify SOK2openClient() reconn_sec = 0 usage

   Overview: 
	SOK2 is an evolution from socklib and SOCK. It does not fork
	a separate process, and does not use shared memory or semaphores.
	It does use non-blocking I/O,  LL/RAP message headers (new or old type),
	and provides automatic reconnection, and an interface to the server mapper. 
	It is designed for 2-way client-server IPC,  and also handles the
	traditional "1-way" TDWR socketing.  

	Platforms: SUN, SGI, and IBM RS6000 Unix.

	Here are its main characteristics:

	    1. Single threaded.  This means that only when you call a
	SOK2 routine is there any work done reading or writing sockets.

	    2. Buffering. SOK2  mallocs buffers as needed for reading
	messages; it does not transfer messages to another buffer on writes, 
	but simply uses pointers into the message provided by the user.
		Note that Unix provides 4K buffers on both ends of a socket
	connection.  If you send larger messages than this, you may need to
	implement your own buffering.

	    3. Non-blocking, optional wait.  All system calls are non-blocking;
	you may poll or specify a timeout period.

	    4. Message Headers.  All messages use the LL/RAP message headers.
	you may provide them or have SOK2 add them.

	    5. All connections are inherently 2-way.  A "one-way" server is 
	implemented by calling SOK2sendMessageAll(). 

	    6. Servers and clients may be dynamically added and deleted.

	    7. Error messages are put out (to ERR) only on exceptional
	conditions.  No Error messages are sent out for connections
	not being made, or lost, etc.

	    8. Provides an interface to register servers with the server mapper,
	and to find servers using the server mapper.

 */

/* SOK2setServiceFile() parameters */
#define SOK2_ENV  0
#define SOK2_FILE 1
#define SOK2_SERVMAP  2

/* This is to indicate when you are dealing with indices into SOK2
   data structures */
typedef int SOK2idx;

/* this is the "new" message header type.  A SOK2 message with this "new"
 * header will contain an SKU_old_header_t header (defined in sockutil.h)
 * with SOK2_HEADER_CODE (0xF0F0F0F0 as defined in sok2.c) in the two longs
 * making up the old header, followed by this header.   */
typedef struct SOK2head_ {
  long id;		/* product id ("record type") */
  long len;		/* length in bytes */
  long seq_no;		/* sequence number */
  } SOK2head ;
  

/************Initialization************************/

extern int SOK2init( char *applic_name );
/* char *applic_name; Application name, used for error messages.
 *
 * Return 1 on success, < 0 on failure.
 */


extern void SOK2exit( int code );
/*    int code;	recommended to pass 0 on "normal" exit; values < 0 reserved.

      Call when all done; you should trap interupts in order to gracefully exit,
      and call this as part of your exit procecedure.  Here are the recommended
      interupts to trap (call SOK2exit() from within your Process_exit()):
      		signal( SIGINT, Process_exit);
      		signal( SIGHUP, Process_exit);
      		signal( SIGTERM, Process_exit);

      This one is called by SOK2init()
      		signal( SIGPIPE, SIG_IGN);
      If you want to trap this, call it after calling SOK2init()

 */

/************** Finding a server ******************************/

extern int  SOK2setServiceFile( int where, char *name );
/*  int where;  = SOK2_ENV, interpret name an an environmental variable
		            whose value is the name of the Service file.
		= SOK2_FILE, use Service file "name".
		= SOK2_SERVMAP, use server mapper on host "name". you may call this multiple
			times if you want to wuery/register with multiple server mappers. You
			may use host = "local" to mean local host, or use, host = "NONE" to skip. 

    char *name;  environmental variable name, filename, or hostname

        Return 1 = success, < 0 = failure:
	    -1 : environmental variable not found.
	

   Use this call to set the name of the RAP_SERVICES file, or to indicate
   to use the server mapper to find where servers are located. 
 
   Service File:
     You specify a file that contains lines of the form
      	service host protocol port
     where:
	service = named service
        host = host name, must be in rhost file; "local" means whatever
		machine this process is running on.
	protocol = "TCP" 
	port = TCP port number 
	
        Example: the "old" way of doing it would be:
	     SOK2setServiceFile( SOK2_ENV, "ALG_SERVICES");
   and to set the environmental variable ALG_SERVICES to the proper filename.

   Server Mapper:
	The server mapper is a process that dynamically tracks where servers
   are located (i.e host, port).  Servers register themselves every minute or
   two; clients may query the server mapper to find where a server is located.
	The Server Mapper typically runs on two hosts to provide redundancy.
   Thus SOK2setServiceFile() may be called twice, to set both host names.
   Servers will be registered on both hosts; when searching for a server, the
   server mapper on the first host is queried; only if it fails is the second
   host queried. 
	As of 09/30/92, the default hosts are stratus and thunder; these will be
   used unless SOK2setServiceFile() is called.
	See toolsa/servmap/servmap.h for more info.
 */


extern int SOK2findService( char *service, char **host, int *port );
/*  char *service;	name of service in RAP_SERVICES file
    char **host;	pointer to host and port where service is found
    int  *port; 	

   Using the method set by SOK2setServiceFile, return the host and port of the 
   named service.  

   Return 1 on success, < 0 on failure (with ERR message):
	-4 = gethostname() failed
	-6 = service file could not be opened
	-7 = service not in service file
  	-8 = syntax error in service file for this service
        (if SOK2_SERVMAP, the errors from SOK2servmapInfo() may also be returned).
	
   If you use this call with SOK2_SERVMAP, you will query the server mapper for
   a service of type "OldServer", subtype "service", instance = "".  These "old"
   services are registered via a RAP_SERVICES file read in by the server mapper.
   "New" servers register themselves, and you should find where they are by calling
   SOK2servmapInfo(), or more generally SMU_requestInfo().
 */

extern void SOK2register( int port, char *type, char *subtype, char *instance);
extern void SOK2registerTimer( int port, char *type, char *subtype, char *instance);
extern void SOK2registerStatus( int port, char *type, char *subtype, char *instance, time_t last_data, time_t last_request);

/* Register this server at this port with the server mapper. 
   SOK2registerTimer will register the service every SERVMAP_REGISTER_INTERVAL secs,
   by using the interval timer.  If the application is already using the timer,
   it must call SOK2register() itself every SERVMAP_REGISTER_INTERVAL secs.
   SOK2registerStatus() is just like SOK2register(), but allows the server to 
   also send status info fields to the server mapper.

   These routines will assume realtime = TRUE, historical = FALSE. 
   Type, subtype, and instance are used to identify the server.
   See servmap.h.
   You may also use SMU_register() to register with the server mapper.
   That is a more general interface.
 */

extern int SOK2servmapInfo( char *type, char *subtype, char *instance, 
 							char **host, int *port);
/* Find the server charactorized by type, subtype, and instance by querying the
   server mapper(s) specified by SOK2setServiceFile( SOK2_SERVMAP, <host>). 
   Return 1 on success, <= 0 on failure:
	 0 : no servers fit the type, subtype, and instance.
	-1 : could not contact server mapper at either host specified in 
	     SOK2setServiceFile() calls.
  
   SOK2servmapInfo will assume realtime = TRUE, historical = FALSE, and will 
   use the first one found if there are more than one.
   See servmap.h.
   You may also use SMU_register() to register with the server mapper.
   That is a more general interface.
 */
 

/*******************Opening a connection****************************/

extern
SOK2idx SOK2openClientWait( char *host, int port, int reconn_secs, int wait_secs);

extern SOK2idx SOK2openClient( char *host, int port, int reconn_secs );
   /* open a client socket to the server at INET host, port.
      If host = "local", use the host the prcess is running on
      If the connection goes down, try to reconnect every reconn_secs.
      If reconn_secs < 0, use the default.
      If reconn_secs = 0, dont try to reconnect, but just send a return of
	-3 to the application. You must then monitor the socket in case the connection
	is broken; if broken, you must call SOK2close() before reconnecting with
	SOK2openClient().

      Return an index to be used in further calls, else < 0 on error:
	-1 = general error.
	-2 = unknown host machine (gethostbyname() failed)
	-3 = connect() call failed, reconn_secs = 0
	-4 = host = "local" and gethostname failed
   */
   

extern SOK2idx  SOK2openServer( int port );
   /* open a server INET socket at the given port. 

      Return an index to be used in further calls, else < 0 on error:
	-1 = general error. see errlog for more info
    */

extern int SOK2close( SOK2idx idx );
   /* close a slient or server socket, and all their connections.
       return TRUE on success, FALSE : idx out of range */


/********************Messages **************************************/

extern int SOK2getMessage( int wait_msecs, SOK2idx *idx, int *client, 
		           SOK2head *head, char **message, int *mess_len );
/*  Input: 
	int wait_msecs;	number of millisecs to wait for new messages to
			be ready.  0 = no wait, -1 = wait forever.
    Output:
	SOK2idx *idx;	socket index that will match one returned from 
			SOK2openClient() or SOK2openServer()
	int *client;    if idx is a server, this is the index of the client that
			the message is from.  If sok_idx is a client, then = -1.
	SOK2head *head;	pointer to the message header	
	char **message; pointer to the message	
	char *mess_len; length in bytes of the message

    Returns:
	1 if message ready, 0 = timed out, < 0 error.

    This looks on all enabled sockets for incoming messages.  When an 
    entire message has been read, it returns with it. 
    If polling (wait_msecs = 0), you should keep calling this routine until it
    returns zero.
    Note that when SOK2getMessage() returns with a message, that socket is disabled
    for further reading until SOK2getMessage() is called again. Thus you must
    finish processing the message, or copy it out before calling SOK2getMessage()
    again.
    If there are no sockets opened, returns -10. This can happen if you are waiting to
    find a server, or waiting for a server to come up again.
 */


extern int SOK2sendMessage(int wait_msecs, SOK2idx idx, int client, int product_id,
			   char *message, int mess_len );
/*  Input: 
	int wait_msecs;	number of millisecs to wait for the messages to get sent.
			0 = no wait, -1 = wait forever.
	SOK2idx idx;    socket index that must match one returned from SOK2openServer().
	int  client;    if idx is a server, this is the index of the client that
			the message is for.  If idx is a client, then not used.
	int product_id; put in the packet header. 
	char *message;  pointer to the message	
	int mess_len;   length in bytes of the message
    
    Returns:
	1 if message sent, 0 = incomplete, < 0 error.
	-1 : general eror
	-2 : bad index
	-4 : client index out of range
	-5 : client not connected
	-7 : connection still has previous write pending
	-8: client down
 */
	
extern int SOK2sendMessageAll( int wait_msecs, SOK2idx idx, int product_id,
		        char *message, int mess_len, int force );

/* This call is for Servers only; send the message to all connected clients.

    Input: 
	int wait_msecs;	number of millisecs to wait for the messages to get sent.
			0 = no wait, -1 = wait forever.
	SOK2idx idx;    socket index that must match one returned from SOK2openServer().
	int product_id; put in the packet header. 
	char *message;  pointer to the message	
	int mess_len;   length in bytes of the message
	int force;	if TRUE, and any clients have messages pending, disconnect
			from them.  if FALSE, any clients with messages pending will
			be skipped. (see also killClient() and killClientsPending())
    
    Returns:
	1 if message sent, 0 = incomplete, < 0 error:
	-1 : general eror
	-2 : bad index
 */
	

extern int SOK2continueWrite( int wait_msecs );
/* This will continue to write any messages that were begun with SOK2sendMessage()
   but not completed (SOK2sendMessage() returned 0).

   Input: 
	int wait_msecs;	number of millisecs to wait for the messages to get sent.
			0 = no wait, -1 = wait forever.

   Returns:
	1 if all messages are complete,
        0 incomplete, timed out before all messages completed.

   Note that when you call SOK2getMessage(), SOK2 tries to complete any
   pending writes also.  Thus this call is necessary only if you dont
   hang on SOK2getMessage();
	
*/

extern int SOK2continueIO( int wait_msecs );
/* This will do any type of socket I/O until a message is done reading,
   writing, or a connection is made, or it times out 

   Input: 
	int wait_msecs;	number of millisecs debore timeout.
			0 = no wait, -1 = wait forever	
*/

extern char *SOK2oldMessage( SOK2head *head, char *message);
/* This will convert a message to the "old" RAP/LL type message; it will
   convert head to an "old" header, and copy the old header and the message to a 
   contiguous memory, and return a pointer to it. You must free() it when done.
   Return NULL on failure.
 */
 
/****************************************************************************/

extern int SOK2killClient( SOK2idx idx, int client );
   /* for Servers only, disconnect from the given client.
      return 1 on success, <0 on failure:
	-1 : general eror
	-2 : bad index
	-3 : not a server
	-4 : not connected
    */

extern int SOK2killClientsPending( SOK2idx idx);
   /* for Servers only, disconnect from any clients with pending writes.
      If you are a One-way server using a single buffer for messages, and the
      previous sendMessageAll() returned 0 (not completed); you must call this
      before reusing the message buffer.  
      Note that if you are using multiple message buffers, you can set force = TRUE on 
      sendMessageAll().

      return 1 on success, <0 on failure:
	-1 : general eror
	-2 : bad index
	-3 : not a server
    */

extern int SOK2statusConnection( SOK2idx idx, int client, int *write_pending );
   /*   Return 1 if connected, else 0, else < 0 on error.
	Set write_pending to 1 if a write is still pending on this connection.
	Both Servers and Clients can use this; "client" is ignored if the socket
	is a client socket.
    */


extern char *SOK2whoisConnected( SOK2idx idx, int client );
   /* return the name (host,port) of the other side of this client */


#endif


#ifdef __cplusplus
}
#endif
