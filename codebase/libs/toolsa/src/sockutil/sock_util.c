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
/*****************************************************************************
 * SOCKUTIL.C : Socket subroutines for applications and servers
 *   simple, blocking socket routine calls
 *
 * Written by F. Hage 2/88
 * Modified by Mike Dixon March 1992
 * Modified by JCaron April 1992; put into /shared
 * 	ansi c; put in toolsa
 *   3/26/93: SYSV port
 *
 */

/* ANSI */

#include <sys/time.h>
#include <sys/socket.h>  /* sockaddr */
#include <sys/un.h>	/* sockaddr_un */
#include <netdb.h>    	/* hostent */
#include <unistd.h>

#include <sys/un.h>
#include <netinet/in.h>

#include <toolsa/umisc.h>
#include <toolsa/sockutil.h>
#include <dataport/bigend.h>
#include <dataport/port_types.h>

int SKUerrno = 0;

#define DEFAULT_RETRIES 100
#define SOCKUTIL_HEADER_CODE 0xf0f0f0f0
#define FALSE 0
#define TRUE 1

typedef struct {
  
  ui32 long1; /* set to SOCKUTIL_HEADER_CODE */
  ui32 long2; /* set to SOCKUTIL_HEADER_CODE */
  
} SKU_code_t;

int SKU_use_old_header = FALSE;

/************************
 * set the socket options
 *
 */

void SetSocketOptions(int sd)
{

  int val;
  int valen = sizeof(val);
  struct linger sl;
  
  /*
   * reuse the socket
   */

  val = 1;
  errno = 0;
  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, valen)){
    fprintf(stderr,"SetSocketOptions(): setsockopt() failed");
  }

  /*
   * make sockets disappear quickly on close
   */

  errno = 0;
  memset(&sl, 0, sizeof(sl));
  sl.l_onoff = 0;
  if (setsockopt(sd, SOL_SOCKET, SO_LINGER, (char *) &sl, sizeof(sl))) {
     fprintf(stderr,"SetSocketOptions(): setsockopt() failed");
  }
}

/****************************************************
 * SKU_read_select - waits for read access
 *
 * returns 1 on success, -1 on timeout, -2 on select failure
 *
 * Blocks if wait_msecs == -1
 */

int SKU_read_select(int sd, long wait_msecs)

{
  
  int ret, maxfdp1;
  fd_set read_fd;
  
  struct timeval wait;
  struct timeval * waitp;
  
  waitp = &wait;
  
  /*
   * listen only on sd socket
   */

  FD_ZERO(&read_fd);
  FD_SET(sd, &read_fd);
  maxfdp1 = sd + 1;
  
 again:

  /*
   * set timeval structure
   */
  
  if (-1 == wait_msecs) {
    waitp = NULL;
  } else {
    wait.tv_sec = wait_msecs / 1000;
    wait_msecs -= wait.tv_sec * 1000;
    wait.tv_usec = wait_msecs * 1000;
  }
  
  errno = 0;
  if (0 > (ret = select(maxfdp1, &read_fd, NULL, NULL, waitp))) {

      if (errno == EINTR) /* system call was interrupted */
	goto again;
      
      SKUerrno = errno;
      fprintf(stderr,"Read select failed on server %d; error = %d\n",
	      sd, errno);
      return -2; /* select failed */

    } 
  
  if (ret == 0) {

#ifdef DEBUG
    fprintf(stderr,
            "SKU_read_select: Select timed out after %d msecs\n",
            (int) wait_msecs);
#endif

    return (-1); /* timeout */
  }
  
  return (1);

}

/**************************************
 * SKU_write_select - waits for write access
 *
 * returns 1 on success, -1 on timeout, -2 on select failure
 *
 * Blocks if wait_msecs == -1
 */

int SKU_write_select(int sd, long wait_msecs)

{
  
  int ret, maxfdp1;
  fd_set write_fd;
  
  struct timeval wait;
  struct timeval * waitp;
  
  waitp = &wait;
  
  /*
   * listen only on sd socket
   */

  FD_ZERO(&write_fd);
  FD_SET(sd, &write_fd);
  maxfdp1 = sd + 1;
  
 again:

  /*
   * set timeval structure
   */

  if (-1 == wait_msecs) {
    waitp = NULL;
  } else {
    wait.tv_sec = wait_msecs / 1000;
    wait_msecs -= wait.tv_sec * 1000;
    wait.tv_usec = wait_msecs * 1000;
  }
  
  errno = 0;
  if (0 > (ret = select(maxfdp1, NULL, &write_fd, NULL, waitp))) {

      if (errno == EINTR) /* system call was interrupted */
	goto again;
      
      SKUerrno = errno;
      fprintf(stderr,"Write select failed on server %d; error = %d\n",
	      sd, errno);
      return -2; /* select failed */

    } 
  
  if (ret == 0) {
    return (-1); /* timeout */
  }
  
  return (1);
}

/*************************************************************************
 * SKU_open_client: Open an Internet socket stream to server on
 * hostname at port: port
 *
 * Returns file discriptor or error;
 * Errors: 
 *   -1 = Could not find host name 
 *   -2 = Could not setup socket (max file descriptors?)
 *   -3 = Could not connect to specified host and port 
 */

int SKU_open_client(const char *hostname, int port)

{ 
  
  int sockfd; /* socket file descriptor */
  struct sockaddr_in rem_soc;
  
  struct hostent *hostport; /* host port info */

  /* Initialize rem_soc */
  memset((void*)&rem_soc, 0, sizeof(rem_soc));

  hostport = gethostbyname(hostname); /* get the remote host info */
  if(!hostport) return(-1);
  
  /*
   * copy the remote sockets internet address to local hostport struc
   */
  
#ifdef AIX
  /* AIX has different sockaddr_in structure ! see /usr/include/netinet/in.h */
  rem_soc.sin_len = sizeof(rem_soc);
#endif
  
  rem_soc.sin_family = AF_INET;
  memcpy((char *) &rem_soc.sin_addr, (char *) hostport->h_addr, 
	 sizeof(rem_soc.sin_addr));
  
  rem_soc.sin_port = port; /* fill in port number */
  rem_soc.sin_port = htons(rem_soc.sin_port); 
  
  /*
   * get a file descriptor for the connection to the remote port
   */
  
  if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
    return(-2);
  }
  
  /*
   * Connect the local socket to the remote port ID
   */
  errno = 0;
  if(connect(sockfd, (struct sockaddr *) &rem_soc,
	     sizeof(rem_soc)) < 0) {
    SKUerrno = errno;
    SKU_close(sockfd);
    return(-3);
  }
  
  SetSocketOptions(sockfd);
  
  return(sockfd);
  
}

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

int SKU_open_client_timed(const char *hostname, int port, long wait_msecs)

{ 
  
  int sockfd; /* socket file descriptor */
  struct sockaddr_in rem_soc;
  struct hostent *hostport; /* host port info */
  int select_status;
  /* initalize rem_soc */
  memset((void*) &rem_soc, 0, sizeof(rem_soc));
  
  hostport = gethostbyname(hostname); /* get the remote host info */
  if(!hostport) return(-1);
  
  /*
   * copy the remote sockets internet address to local hostport struc
   */
  
#ifdef AIX
  /* AIX has different sockaddr_in structure ! see /usr/include/netinet/in.h */
  rem_soc.sin_len = sizeof(rem_soc);
#endif
  
  rem_soc.sin_family = AF_INET;
  memcpy((char *) &rem_soc.sin_addr, (char *) hostport->h_addr, 
	 sizeof(rem_soc.sin_addr));
  
  rem_soc.sin_port = port; /* fill in port number */
  rem_soc.sin_port = htons(rem_soc.sin_port); 
  
  /*
   * get a file descriptor for the connection to the remote port
   */
  
  if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
    return(-2);
  }
  
  /*
   * Wait for the socket to be ready.
   */

  select_status = SKU_write_select(sockfd, wait_msecs);
  
  if (select_status != 1)
  {
    fprintf(stderr,
	    "*** Error %d returned from SKU_read_select\n",
	    select_status);
    
    SKU_close(sockfd);
    return(-4);
  }
  
  /*
   * Connect the local socket to the remote port ID
   */
  errno = 0;
  if(connect(sockfd, (struct sockaddr *) &rem_soc,
	     sizeof(rem_soc)) < 0) {
    SKUerrno = errno;
    SKU_close(sockfd);
    return(-3);
  }
  
  SetSocketOptions(sockfd);
  
  return(sockfd);
  
}

/*************************************************************************
 * SKU_open_unix_client: Open a unix socket stream to server on
 * hostname at port: port
 *
 * Returns file discriptor or error;
 * Errors: 
 *   -1 = Could not find host name 
 *   -2 = Could not setup socket (max file descriptors?)
 *   -3 = Could not connect to specified host and port 
 */

int SKU_open_unix_client(const char *sock_file)

{ 
  
  int sockfd; /* socket file descriptor */
  struct sockaddr_un soc;
  
  soc.sun_family = AF_UNIX;
  strcpy(soc.sun_path, sock_file);
  
  /*
   * get a file descriptor for the connection to the remote port
   */
  
  if((sockfd = socket(AF_UNIX,SOCK_STREAM,0)) == -1) {
    return(-2);
  }
  
  /*
   * Connect the local socket to the remote port ID
   */
  errno = 0;
  if(connect(sockfd, (struct sockaddr *) &soc,
	     sizeof(soc)) < 0) {
    SKUerrno = errno;
    SKU_close(sockfd);
    return(-3);
  }
  
  SetSocketOptions(sockfd);
  
  return(sockfd);
  
}

/*****************************************************************************
 * SKU_open_server: Open up an Internet Stream socket for use as a server
 * and puts it into listen mode.
 * Returns proto filedescriptor: use this in next accept call
 * OR : 
 *   -1 = Could not open socket (max file descriptors ?)
 *   -2 = Could not bind to specified port
 *   -3 = Could not listen 
 *
 */


int SKU_open_server(int port)

{ 
  
  int protofd; 	/* file descriptor */
  struct sockaddr_in loc_soc; /* local socket info */

  /* initialize loc_soc */
  memset((void*)&loc_soc, 0, sizeof(loc_soc));
  
  /*
   * get a file descriptor for the connection to the remote port
   */
  
  if((protofd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    fprintf(stderr,"Could not set up socket\n");
    return(-1);
  }
  
  /*
   * set socket options
   */
  SetSocketOptions(protofd);
  
#ifdef AIX
  /* AIX has different sockaddr_in structure ! see /usr/include/netinet/in.h */
  loc_soc.sin_len = sizeof(loc_soc);
#endif
  
  loc_soc.sin_port = htons(port);
  loc_soc.sin_family = AF_INET;
  loc_soc.sin_addr.s_addr = htonl(INADDR_ANY);
  
  /*
   * bind to a local port
   */

  errno = 0;
  if(bind(protofd,
	  (struct sockaddr *) &loc_soc,
	  sizeof(loc_soc)) < 0) {
    fprintf(stderr,"Couldn't bind port %d; error = %d\n", port, errno);
    SKU_close(protofd);
    return(-2);
  }
  
  /*
   * Wait for remote connection request
   * 
   *  Note: Pass zero as second arg to listen() so that the kernel
   *          does NOT queue up requests from clients. If the kernel
   *          is allowed to do this, clients can connect without
   *          blocking, then begin writing to the buffer. They die
   *          when the buffer fills up.
   */

  errno = 0;
  if (listen(protofd, 5) < 0) {
    SKUerrno = errno;
    fprintf(stderr,"Listen failure on port %d; error = %d\n", port, errno);
    SKU_close(protofd);
    return(-3);
  }
  
  return(protofd);

}

/*****************************************************************************
 * SKU_open_unix_server: Open up a Unix Stream socket for use as a server
 * and puts it into listen mode.
 * Returns proto filedescriptor: use this in next accept call
 * OR : 
 *   -1 = Could not open socket (max file descriptors ?)
 *   -2 = Could not bind to specified port
 *   -3 = Could not listen 
 *
 */


int SKU_open_unix_server(const char *sock_file)

{
  
  int protofd; 	/* file descriptor */
  int name_len;
  struct sockaddr_un soc; /* local socket info */

  name_len = sizeof(struct sockaddr_in);
  
  /*
   * get a file descriptor for the connection to the remote port
   */
  
  if((protofd = socket(AF_UNIX,SOCK_STREAM,0)) < 0) {
    fprintf(stderr,"Could not set up socket\n");
    return(-1);
  }
  
  /*
   * set socket options
   */

  SetSocketOptions(protofd);
  
#ifdef AIX
  /* AIX has different sockaddr_un structure ! see /usr/include/sys/un.h */
  soc.sun_len = sizeof(soc);
#endif
  
  soc.sun_family = AF_UNIX;
  strcpy(soc.sun_path, sock_file);
  
  /*
   * bind
   */

  unlink(sock_file);
  errno = 0;
  if(bind(protofd, (struct sockaddr *) &soc, name_len) < 0) {
    fprintf(stderr,"Couldn't bind addr %s; error = %d\n",
	    sock_file, errno);
    SKU_close(protofd);
    return(-2);
  }
  
  /*
   * Wait for remote connection request
   */

  errno = 0;
  if(listen(protofd, 5) < 0) {
    SKUerrno = errno;
    fprintf(stderr,"Listen failure on sock %s; error = %d\n",
	    sock_file, errno);
    SKU_close(protofd);
    return(-3);
  }
  
  return(protofd);

}

/*************************************************************************
 * SKU_get_client
 *
 * Gets the next client
 *
 * Waits until client has connected 
 *
 * Return   =  0 : success
 *          = -1 : accept failure (Note: Different from SKU_get_client_timed)
 *
 */

int SKU_get_client(int protofd)

{
  
  int sockfd; /* socket file descriptor */
  
  union sunion {
    struct sockaddr_in sin;
    struct sockaddr_un sund;  	/* need this to get sizeof right */
  } sadd;
  
  socklen_t name_len = sizeof(struct sockaddr_in);
  errno = 0;
  if((sockfd = accept(protofd,
		      (struct sockaddr *) &sadd,
		      &name_len)) < 0) {
    fprintf(stderr,"Accept failed on server %d; error = %d\n", 
	    protofd, errno);
    return(-1); /* Note: Different from SKU_get_client_timed */
  }
  
  SetSocketOptions(sockfd);
  return(sockfd);
}

/***************************************************************************
 * SKU_get_client_timed
 *
 * Gets the next client
 *
 * Waits until client has connected.
 *
 * Returns: sockfd on success
 *          -1 in timeout
 *          -2 on select failure
 *          -3 on accept failure
 *
 * Blocks if wait_msecs == -1
 */

int SKU_get_client_timed(int protofd, long wait_msecs)

{
  
  int ret;
  int sockfd; /* socket file descriptor */
  socklen_t name_len = sizeof(struct sockaddr_in);
  
  union sunion {
    struct sockaddr_in sin;
    struct sockaddr_un sund;
  } sadd;
  
  ret = SKU_read_select(protofd, wait_msecs);
  if (ret != 1) {
    return (ret); /* returns -1 or -2 from SKU_read_select */
  }
  
  /*
   * something ready to accept
   */

  errno = 0;
  if(0 > (sockfd = accept(protofd, (struct sockaddr *) &sadd,
			   &name_len))) {
    SKUerrno = errno;
    fprintf(stderr,"Accept failed on server %d; error = %d\n", 
	    protofd, errno);
    return(-3); /* accept error */
  } else {
    SetSocketOptions(sockfd);
    return(sockfd);
  }
  
}

/*************
 * SKU_close()
 *
 * Close the socket down properly
 */

void SKU_close(int fd)
{
  close(fd);
}

/*************
 * SKU_close_no_hangup()
 *
 * Close the socket down without issuing a HANGUP
 * or any other fancy stuff.
 */

void SKU_close_no_hangup(int fd)
{
  close(fd);
}

/***************************************************************************
 * SKU_read
 *
 * Reads a simple message of given length
 *
 * Memory allocated by calling routine
 *
 * Returns nbytes actually read 
 */

long SKU_read(int fd, void *mess, long len, int retries)
{
  
  long bytes_read,total,err_count;
  long target_size = len;
  char *ptr = (char *) mess;
  
  if (retries < 0)
    retries = DEFAULT_RETRIES;
  total = 0;
  err_count = 0;
  
  while(target_size) {
    errno = 0;
    bytes_read = read(fd, ptr, target_size);
    if(bytes_read <= 0) {
      if (errno != EINTR) { /* system call was not interrupted */
	err_count++;
      }
      if(err_count >= retries) return total;
      /* Block for 1 millisecond */
      uusleep(1000);
    } else {
      err_count = 0;
    }
    if (bytes_read > 0) {
      target_size -= bytes_read;
      ptr += bytes_read;
      total += bytes_read;
    }
  }
  
  return total;
}

/***************************************************************************
 * SKU_read_timed
 *
 * Reads a simple message of given length
 *
 * Memory allocated by calling routine
 *
 * Waits until something to read or times out and returns -1,
 *
 * Blocks if wait_msecs == -1
 *
 * Returns nbytes read
 */

long SKU_read_timed(int fd, void *mess, long len,
		    int retries, long wait_msecs)
{

  long bytes_read,total,err_count;
  long target_size = len;
  char *ptr = (char *) mess;
  
  if (1 != SKU_read_select(fd, wait_msecs)) {

#ifdef DEBUG
    fprintf(stderr, "SKU_read_timed:  "
	    "Error returned from SKU_read_select\n");
#endif

    return -1;
  }
  
  if (retries < 0) {
    retries = DEFAULT_RETRIES;
  }
  total = 0;
  err_count = 0;
  
  while(target_size) {
    errno = 0;
    bytes_read = read(fd, ptr, target_size);

#ifdef DEBUG
    fprintf(stderr, "SKU_read_timed:  target_size = %d, "
	    "bytes_read = %d, errors = %d\n",
	    (int) target_size, (int) bytes_read, (int) err_count);
#endif

    if (bytes_read <= 0) {
      if (errno != EINTR) { /* system call was not interrupted */
	err_count++;
      }
      if (err_count >= retries) {
        return total;
      }
    }
    else {
      err_count = 0;
      target_size -= bytes_read;
      ptr += bytes_read;
      total += bytes_read;
    }
  }
  
  return total;

}

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
 *
 * Blocks if wait_msecs == -1
 *
 * Reads chunks until a select times out. Returns nbytes read.
 */

long SKU_read_timed_hb(int fd, void *mess, long len,
		       int retries, long wait_msecs,
		       int chunk_size,
		       SKU_heartbeat_t heartbeat_func)
{

  long bytes_read,total,err_count;
  long target_size = len;
  char *ptr = (char *) mess;
  
  if (retries < 0) {
    retries = DEFAULT_RETRIES;
  }
  total = 0;
  err_count = 0;
  
  while(target_size) {

    if (heartbeat_func != NULL)
      heartbeat_func("In SKU_read_timed_hb()");
    
    if (1 != SKU_read_select(fd, wait_msecs)) {

#ifdef DEBUG
      fprintf(stderr, "SKU_read_timed:  "
	      "Error returned from SKU_read_select\n");
#endif

      if (total > 0)
	return total;
      
      return -1;
    }
  
    errno = 0;
    bytes_read = read(fd, ptr, MIN(target_size, chunk_size));

#ifdef DEBUG
    fprintf(stderr, "SKU_read_timed:  target_size = %d, "
	    "bytes_read = %d\n",
	    (int) target_size, (int) bytes_read);
#endif

    if(bytes_read <= 0) {
      if (errno != EINTR) { /* system call was not interrupted */
	err_count++;
      }
      if(err_count >= retries) return total;
    } else {
      err_count = 0;
      target_size -= bytes_read;
      ptr += bytes_read;
      total += bytes_read;
    }
  }
  
  return total;

}

/*********************************************************************
 * SKU_read_header()
 *
 * Reads a RAP header packet from the client
 *
 * Memory allocated by calling routine
 *
 * Times out after wait_msecs milliseconds
 *
 * Blocks if wait_msecs == -1
 *
 * returns 1 on success, -1 on failure
 *
 *********************************************************************/

int SKU_read_header(int sockfd, SKU_header_t *header, long wait_msecs)

{
  
  SKU_old_header_t old_header;
  SKU_code_t *code;
  
  /*
   * read in the first 8 bytes, which might be an old header
   * or the code bytes which precede a new header
   */
  
  if (SKU_read_timed(sockfd, &old_header,
		     8, -1, wait_msecs) != 8) {
    return (-1);
  }
  
  /*
   * check if this is a new header
   */
  
  code = (SKU_code_t *) &old_header;
  
  if (code->long1 == SOCKUTIL_HEADER_CODE &&
      code->long2 == SOCKUTIL_HEADER_CODE) {

    /*
     * is new header; read in next part
     */

#ifdef DEBUG
    fprintf(stderr, "SKU_read_header:  Reading new header\n");
#endif

    SKU_use_old_header = FALSE;
      
    if (SKU_read_timed(sockfd, header,
		       (long) sizeof(SKU_header_t), -1, wait_msecs)
	!= sizeof(SKU_header_t)) {
      return (-1);
    }
      
    header->id = BE_to_si32(header->id);
    header->len = BE_to_si32(header->len);
    header->seq_no = BE_to_si32(header->seq_no);

  } else {

    /*
     * is old header; convert to new form
     */
    
#ifdef DEBUG
    fprintf(stderr, "SKU_read_header: Reading old header\n");
#endif

    header->id = BE_to_si16(old_header.id);
    header->len = BE_to_si16(old_header.len) * 2 - sizeof(SKU_old_header_t);
    header->seq_no = BE_to_si32(old_header.seq_no);

#ifdef DEBUG
    fprintf(stderr, "SKU_read_header:  id = %d, len = %d, seq_no = %d\n",
	    header->id, header->len, header->seq_no);
#endif

  }
  
  return (1);

}

/*********************************************************************
 * SKU_readh()
 *
 * Reads a RAP header and data packet, timing out as necessary
 *
 * Memory allocated by calling routine. Header holds data length.
 *
 * Data lenght constrained by max_data.
 *
 * Blocks if wait_msecs == -1
 *
 * returns 1 on success, -1 on error, -2 if max len exceeded
 *
 *********************************************************************/

int SKU_readh(int sockfd, void *data, long max_data,
	      SKU_header_t *header, long wait_msecs)

{

  long len;
  int  n;
  
  /*
   * read the header
   */

  if (0 > SKU_read_header(sockfd, header, wait_msecs)) {
    return (-1);
  }
  
  /*
   * read the data
   */

  len = (header->len > max_data) ? max_data : header->len;
  n = SKU_read_timed(sockfd, data, len, -1, wait_msecs);
  
  if (n != len) {
    return (-1);
  } else if (header->len > max_data) {
    return (-2);
  } else {
    return (1);
  }

}

/*********************************************************************
 * SKU_read_message()
 *
 * NOT THREAD SAFE!
 *
 * reads a RAP header and message, timing out as necessary.
 *
 * allocs or reallocs memory for the data.
 *
 * Blocks if wait_msecs == -1
 *
 * returns 1 on success, -1 on error
 *
 *********************************************************************/

int SKU_read_message(int sockfd, SKU_header_t *header,
		     char **data, long *data_len, long wait_msecs)
{
  
  static char *read_buffer = NULL;
  static long nbytes_buffer = 0;
  
  /*
   * read a new packet into the buffer
   */
  
  if (SKU_read_header(sockfd, header, wait_msecs) < 0) {
    
#ifdef DEBUG
    fprintf(stderr, "SKU_read_message:  Error returned from SKU_read_header\n");
#endif
    
    return (-1);
  }
  
  if (header->len > nbytes_buffer) {
    
    if (read_buffer == NULL)
      read_buffer = (char *) umalloc (header->len);
    else
      read_buffer = (char *) urealloc (read_buffer, header->len);
    
    nbytes_buffer = header->len;
    
  } /* if (header->len > nbytes_buffer) */
  
#ifdef DEBUG
  fprintf(stderr, "SKU_read_message:  Calling SKU_read_timed, "
	  "target_size = %d\n",
	  header->len);
#endif

  if (SKU_read_timed(sockfd, read_buffer,
		     header->len, -1, wait_msecs) != header->len) {

#ifdef DEBUG
      fprintf(stderr, "SKU_read_message:  Error returned from SKU_read_timed\n");
#endif

    return (-1);
  }
  
  *data_len = header->len;
  *data = read_buffer;
  
  return (1);
  
}

/*************************************************************************
 * SKU_write
 *
 * writes simple message
 *
 * returns number of bytes written 
 */

long SKU_write(int fd, const void *mess, long len, int retries)

{
  long bytes_written,total,err_count;
  long target_size = len;
  char *ptr = (char *) mess;
  
  if (retries < 0) {
    retries = DEFAULT_RETRIES;
  }
  
  total = 0;
  err_count = 0;
  
  while(target_size > 0) {

    errno = 0;
    bytes_written = write(fd,ptr,target_size);

    if(bytes_written <= 0) {
      
      if (errno != EINTR) { /* system call was not interrupted */
	err_count++;
      }

      if(err_count >= retries) {
	return total;
      }

      /*
       * Block for 1 millisecond
       */

      uusleep(1000);
      
    } else {

      err_count = 0;
      
    }

    if (bytes_written > 0) {
      target_size -= bytes_written;
      ptr += bytes_written;
      total += bytes_written;
    }

  }
  
  return (total);
}


/*********************************************************************
 * SKU_write_header()
 * 
 * NOT THREAD SAFE!
 *
 * Writes RAP header packet, seqno passed in
 *
 * returns 1 on success, -1 on failure
 *
 *********************************************************************/

int SKU_write_header(int sockfd, long len,
		     long product_id, long seqno)

{
  
  static long global_seqno = 0;
  
  SKU_old_header_t old_header;
  SKU_header_t header;
  SKU_code_t code;
  
  if (seqno == -1)
    seqno = global_seqno++;
  
  if (SKU_use_old_header) {

    /*
     * send even number of bytes - round up !!
     */

    len += (len % 2);
    
    old_header.id = BE_from_si16(product_id);
    old_header.len = BE_from_si16(((len + sizeof(SKU_old_header_t)) / 2));
    old_header.seq_no = BE_from_si32(seqno);
      
    if (SKU_write(sockfd, &old_header,
		  (long) sizeof(SKU_old_header_t),
		  -1)
	!= sizeof(SKU_old_header_t)) {
      return (-1);
    }
      
  } else {

    code.long1 = SOCKUTIL_HEADER_CODE;
    code.long2 = SOCKUTIL_HEADER_CODE;
      
    if (SKU_write(sockfd, &code,
		  (long) sizeof(SKU_code_t),
		  -1)
	!= sizeof(SKU_code_t)) {
      return (-1);
    }
      
    header.seq_no = BE_from_si32(seqno);
    header.id = BE_from_si32(product_id);
    header.len = BE_from_si32(len);
    
    if (SKU_write(sockfd, &header,
		  (long) sizeof(SKU_header_t),
		  -1)
	!= sizeof(SKU_header_t)) {
      return (-1);
    }
    
  } 
  
  return (1);

}

/*********************************************************************
 * SKU_writeh() - Really a write_message function.
 *
 * Writes RAP packet and header, seqno passed in
 *
 * Also writes some data that goes with the header.
 *
 * returns 1 on success, -1 on failure
 *
 *********************************************************************/

int SKU_writeh(int sockfd, const void *data, long len,
	       long product_id, long seqno)
{
  
  /*
   * write the packet header
   */
  
  if (0 > SKU_write_header(sockfd, len, product_id, seqno)) {
    return (-1);
  }
  
  /*
   * messages using old headers can only send even byte data lengths ! 
   */
  if (SKU_use_old_header) 
    len += len % 2;
  
  if (SKU_write(sockfd, data, len, -1) != len)
    return (-1);
  
  return (1);
}

/*********************************************************************
 * SKU_writeh_timed()
 *
 * Writes RAP packet and header, seqno passed in
 *
 * Times out after wait_msecs
 *
 * Blocks if wait_msecs == -1
 *
 * returns 1 on success, -1 on failure
 *
 *********************************************************************/

int SKU_writeh_timed(int sockfd, void *data, long len,
		     long product_id, long seqno,
		     long wait_msecs)
{
  
  if (wait_msecs > 0) {
    if (SKU_write_select(sockfd, wait_msecs) != 1) {
      return (-1);
    }
  }

  return (SKU_writeh(sockfd, data, len, product_id, seqno));

}

/*********************************************************************
 *
 * SKU_write_message()
 *
 * NOT THREAD SAFE!
 *
 * Writes RAP header and data
 *
 * Sequence number automatically generated
 *
 * returns 1 on success, -1 on failure
 *
 *********************************************************************/

int SKU_write_message(int sockfd, long product_id,
		      const char *data, long len)
{
  
  static long seqno = 0; /* Only initialized first time??? -PTM */
  
  seqno++;
  return (SKU_writeh(sockfd, data, len, product_id, seqno));

}

/*********************************************************************
 * SKU_set_headers_to_old()
 *
 * Forces writes to use old headers, until a read finds a new header
 *
 *********************************************************************/

void SKU_set_headers_to_old(void)
     
{
  SKU_use_old_header = TRUE;
}

/*********************************************************************
 * SKU_set_headers_to_new()
 *
 * Forces writes to use new headers, until a read finds an old header
 *
 *********************************************************************/

void SKU_set_headers_to_new(void)    
{
  SKU_use_old_header = FALSE;
}


