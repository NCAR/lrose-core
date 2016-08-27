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
/* ANSI headers */
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <memory.h>

/* non-ANSI */
#include <fcntl.h>	/* fctl, F_SETFL, FNDELAY */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>  /* sockaddr */
#include <sys/un.h>	/* sockaddr_un */
#include <netdb.h>    	/* hostent */
#include <netinet/in.h>  /* htonl ntohl sin_family sin_addr */
#include <unistd.h>

#include <toolsa/globals.h>
#include <toolsa/err.h>
#include <toolsa/port.h>
#include <toolsa/str.h>
#include <toolsa/utim.h>

#include <toolsa/servmap.h>
#include <toolsa/smu.h>

#include <toolsa/sok2.h>

#define MAX_HOSTNAME 128
#define MAX_HOSTS 16
#define SOK2_HEADER_CODE 0xF0F0F0F0

/* internal structures */

/* this is a Message Header Structure */
typedef struct OldHeader_ {
   short  		id;	 /* record type */
   unsigned short 	length;  /* # 16 bit words ! */
   long   		seq_no;  /* sequence number, all messages from
				    one server socket */
   } OldHeader;

typedef struct Header_code_{
    long long1;
    long long2;
    } Header_code;

/* Message Read structure */
typedef struct Read_t_ {
   int		  sd; 		/* socket descriptor */
   int		  status;	/* 0 = waiting for input 
				   1 = reading header - part 1
				   2 = reading header - part 2 (new only)
			           3 = reading message
				 */

   char 	  *buffer;	/* malloc'ed buffer */
   long		  buffer_size;	/* malloc size */

   int		  wantb; 	/* want to read (bytes) */
   int		  haveb;	/* have read (bytes) */
   char           *cbuffer;	/* current buffer */

   int		  head_type;    /* 0 = old, 1 = new */
   OldHeader	  oldhead;	/* old style header */
   SOK2head	  newhead;	/* new style header */

   int		  ready;	/* if theres a message ready */
   } Read_t;

/* Message Write structure */
typedef struct Write_t_ {
   int		  sd; 		/* socket descriptor */
   int		  status;	/* 0 = writing header
				   1 = writing message  
				 */

   char 	  *message;	/* pointer to message*/
   int		  mess_len;     /* message length */

   int		  wantb; 	/* want to write (bytes) */
   int		  haveb;	/* have written (bytes) */
   char           *cbuffer;	/* current buffer */

   OldHeader	  oldhead;	/* old style header */
   SOK2head	  newhead;	/* new style header */

   long		  seq_no;
   int		  write_pending; /* in middle of writing */
   } Write_t;

/* a client has one connection, a server may have 0 or many */
typedef struct Conn_t_ {
    int used;			/* currently in use */
    int  sd;			/* socket descriptor */
    struct sockaddr sadd;	/* socket address of the other side of connection */
    int  head_type;    		/* 0 = old, 1 = new */

	/* optional reconnection parameters (clients only) */
    int		retry_secs;	/* retry down connection */
    int		down;	        /* TRUE if currently down */
    time_t      time_down;    	/* time it went down */
    time_t      last_retry;   	/* last time retried */

    Read_t rd;			/* message read struct */
    Write_t wrt;		/* message write struct */
    } Conn_t;

typedef struct Sok2_t_ {
    char	host[ MAX_HOSTNAME];
    int		port;
    int 	type;  		/* 0 = not used, 1 = server, 2 = client */
    int 	sd;		/* server listen socket descriptor 
				  (not used for client) */
    Conn_t	*conn;		/* malloced array of connections */
    int		nconns;		/* numer of connections in conn[] */
   } Sok2_t;

static Sok2_t *Sockets;		/* array of Sok2_t structures */
static int    Nsockets = 0;     /* number of Sok2_t structures */

#define DEFAULT_CONNECT_WAIT 25   /* wait for connect to complete in seconds */
#define DEFAULT_RETRY_SECS 30
#define MAX_APPLIC_NAME 40
#define MAX_SERVICE_FILENAME  100

static char Application[ MAX_APPLIC_NAME] = "NotSet";
static char ServiceFile[ MAX_SERVICE_FILENAME] = "NotSet";

/* server mapper */
static char *Hosts[MAX_HOSTS];
static int HostCount = 0;
static SERVMAP_info_t *RegInfo; /* to do timer - based server mapper registration */
static int RegCount = 0;
static int UseServerMapper = FALSE;

static int Debug = FALSE;
static int DownSockets = FALSE;
static int Listen_backlog = 5;	/* default backlog parameter to listen */

static fd_set Listen_fds;  /* server "listen" sockets */
static fd_set Read_fds;    /* possible incoming messages */
static int Select_width;   /* #fds in fd_set */
static fd_set Write_fds;   /* pending write sockets */

/************** Sok2_t utilities *********************************/
#define SOCKETS_INC 5
static int AddSocket( void)
    /* get new Sok2_t struct, return its index */
    {
	int 	i, old_size;
	Sok2_t	*sokp;

	/* first time */
  	if (0 == Nsockets)
	    {
	    if (NULL == (Sockets = (Sok2_t *) 
			 calloc( SOCKETS_INC, sizeof(Sok2_t))))
		{
	    	ERRprintf(ERR_RESOURCE, "AddSock calloc failed n = %d", SOCKETS_INC);
		return -1;
		}
	    Nsockets = SOCKETS_INC;
	    return 0;
	    }

	/* look for unused ones */
	for (i=0; i< Nsockets; i++)
	    if (Sockets[i].type == 0)
		return i;

	/* expand the array */
	old_size = Nsockets;
	Nsockets += SOCKETS_INC;
	if (NULL == (sokp = (Sok2_t *) 
		     realloc( Sockets, Nsockets * sizeof(Sok2_t))))
	    {
	    ERRprintf(ERR_RESOURCE, "AddSock realloc failed n = %d", Nsockets);
	    return -1;
	    }

	/* zero out the new stuff */
	STRset( &sokp[ old_size], 0, SOCKETS_INC * sizeof(Sok2_t));

	/* success */
	Sockets = sokp;
	return old_size;
    }

static Sok2_t *FindServerSocket( int sd)
    /* find the socket struct for this socket descriptor */
    {
	int i;

	for (i=0; i<Nsockets; i++)
	    if (Sockets[i].type == 1)
		{
		if (Sockets[i].sd == sd)
		    return &Sockets[i];
		}
	return NULL;
    }

/************** Conn_t utilities *********************************/
#define CONN_INC 5
static Conn_t *AddConn( Sok2_t *sp)
    /* add new connection to this socket */
    {
	int i, old_size;
	Conn_t	*connp;

	/* first time */
  	if (0 == sp->nconns)
	    {
	    if (NULL == (sp->conn = (Conn_t *) 
			calloc( CONN_INC, sizeof(Conn_t))))
		{
		ERRprintf(ERR_RESOURCE, "AddConn calloc failed n = %d", CONN_INC);
		return NULL;
		}
	    sp->nconns = CONN_INC;
	    return sp->conn;
	    }

	/* look for unused ones */
	for (i=0; i< sp->nconns; i++)
	    if (!sp->conn[i].used)
		return &sp->conn[i];

	/* expand the array */
	old_size = sp->nconns;
	if (NULL == (connp = (Conn_t *) 
		     realloc( sp->conn, (sp->nconns + CONN_INC) * sizeof(Conn_t))))
	    {
	    ERRprintf(ERR_RESOURCE, "AddConn realloc failed n = %d", 
			sp->nconns + CONN_INC);
	    return NULL;
	    }

	/* zero out the new stuff */
	STRset( &connp[ old_size], 0, CONN_INC * sizeof(Conn_t));

	/* success */
	sp->conn = connp;
	sp->nconns += CONN_INC;
	return &sp->conn[ old_size];
    }

static Conn_t *FindConn( int sd, char *calling)
    /* find the connect struct for this socket descriptor */
    {
	int i,j;

	for (i=0; i<Nsockets; i++)
	    for (j=0; j < Sockets[i].nconns; j++)
	        if (Sockets[i].conn[j].used && (Sockets[i].conn[j].sd == sd))
	    	    {
		    return &Sockets[i].conn[j];
		    }

        ERRprintf(ERR_PROGRAM, " %s cant find connection %d; killing it", calling, sd);
	
	/* get rid of this guy!! */
      	shutdown( sd, 2);
        close( sd);
        FD_CLR( sd, &Read_fds);
        FD_CLR( sd, &Write_fds);
        FD_CLR( sd, &Listen_fds);

	return NULL;
    }

static void InitConnection( Conn_t *cp, int newsd)
    {
	cp->used = TRUE;
	cp->down = FALSE;
	cp->sd = newsd;
	
	cp->rd.sd = newsd;
	cp->rd.status = 0;
	cp->rd.ready = FALSE;

	cp->wrt.sd = newsd;
	cp->wrt.status = 0;
	cp->wrt.write_pending = FALSE;

	FD_SET( newsd, &Read_fds);
    }

static int KillConnection( Conn_t *cp)
    {
      	time_t    now;

      	shutdown( cp->sd, 2);
        close( cp->sd);
        FD_CLR( cp->sd, &Read_fds);
        FD_CLR( cp->sd, &Write_fds);
        FD_CLR( cp->sd, &Listen_fds);

	/* temp 
	printf("kill connection %d\n", cp->sd); */

      	cp->down = TRUE;
      	cp->sd = -1;

	if (cp->retry_secs > 0)
	    {
      	    time( &now);
            cp->time_down = now;
            cp->last_retry = now;
	    DownSockets = TRUE;
	    }
	else
      	    cp->used = FALSE;

	return 1;
    }


/*_____________________________________________________________________________
 *
 * Description of function
 *_____________________________________________________________________________
 */

static Conn_t *
ConnOk( int idx, int client)
{
    Sok2_t 	*sp;
	
    if ((idx < 0) || (idx >= Nsockets))
	return NULL;

    sp = &Sockets[ idx];

    if (sp->type == 0)
	return NULL;

    if (sp->type == 2)
	client = 0;

    if ((client < 0) || (client >= sp->nconns))
	return NULL;

    return &sp->conn[ client];
}


/************** fdset utilities ********************************/
static void FdUnion( fd_set *result, fd_set *set1, fd_set *set2)
    {
	int i;
	FD_ZERO( result);
	for (i=0; i< Select_width; i++)
	     if ( FD_ISSET( i, set1) || 
		  ((set2 != NULL) && FD_ISSET( i, set2)))
		FD_SET(i, result);
    }

static int FdisEmpty( fd_set *set)
    {
	int i;
	for (i=0; i< Select_width; i++)
	    if ( FD_ISSET( i, set))
		return FALSE;

	return TRUE;
    }

static int FdSetisEmpty( fd_set *set1, fd_set *set2)
    {
	int i;
	for (i=0; i< Select_width; i++)
	    if ( FD_ISSET( i, set1) &&  FD_ISSET( i, set2))
		return FALSE;

	return TRUE;
    }

/*******************Non Blocking write *********************************/

/*_____________________________________________________________________________
 *
 * Nb_write: this will write everything in the buffer
 * until done or an error occurs. Errors EWOULDBLOCK and EPIPE (lost
 * connection) dont generate an error message.
 * Return 1 if write is complete, 0 EWOULDBLOCK, < ) error.
 *_____________________________________________________________________________
 */

static int 
Nb_write( Write_t *wrt)
{
    int ret;

    while ( wrt->haveb < wrt->wantb) {
	errno = 0;
	if (0 > (ret = write( wrt->sd, wrt->cbuffer + wrt->haveb, 
			     wrt->wantb - wrt->haveb)))	{
	    /* SUN4 will return EWOULDBLOCK, SYSV will return EAGAIN */
	    if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
		/* set this guy for "pending write" */
		FD_SET( wrt->sd, &Write_fds);
		return 0;
	    } else {	
		/* no error message on lost conection */
		if ((errno == EPIPE) || (errno == ENOTCONN))
		    return -5;

		ERRprintf( ERR_WARNING, 
		          "%s; nonblock write failed ret= %d errno= %d",
	                  Application, ret, errno);
	    }
	    return -1;
	} /* ret < 0 */

	else if (ret == 0) {
	    /* lost connection */
	    return -1;
	} else {
	    wrt->haveb += ret;
	}

    } /* loop over write until done or EWOULDBLOCK */
	 
    return (1);
}


static int WriteBuffer( Conn_t *cp)
    {
    	int ret = Nb_write( &cp->wrt);

	if (0 == ret)			/* WOULD BLOCK */
	    return 0;		 	
	if (0 > ret)			/* ERROR */
	    {
	    KillConnection( cp);
	    return ret;
	    }

	/* message complete */

      	/* clear this guy for "pending write" */
      	FD_CLR( cp->wrt.sd, &Write_fds);
	cp->wrt.write_pending = FALSE;

	return 1;	
   }


static int WriteMessage( int sd)
    {
	Conn_t *conn = FindConn( sd, "writeMess");
	Write_t *wrt;
    	int ret;

	if (NULL == conn)
	    return -1;
	wrt = &conn->wrt;

	/* write as much as you can */
	ret  = Nb_write( wrt);

	if (0 == ret)			/* WOULD BLOCK */
	    return 0;		 	
	if (0 > ret)			/* ERROR */
	    {
	    KillConnection( conn);
	    return ret;
	    }

	/* completed the write */

      	if (wrt->status == 0) 
            {
            /* header is complete, start on the message itself */
	    wrt->cbuffer = wrt->message;
	    wrt->wantb = wrt->mess_len;
	    wrt->haveb = 0;
	    wrt->status = 1;

	    return WriteBuffer( conn);
	    }
      	else if (wrt->status == 1)
	    return WriteBuffer( conn);

	return -1; /* shouldnt get here */
    }


/************** Non blocking read *****************************/

/* Nb_read: this will read "want" number of bytes,
   until done or "EWOULDBLOCK" occurs.  
   Return 1 if complete read, 0 incomplete read, -1 error, kill connection.
 */
static int Nb_read( Read_t *rd)
{
    int ret;

    while ( rd->haveb < rd->wantb) {
	errno = 0;
	if (0 > (ret = read( rd->sd, rd->cbuffer + rd->haveb,
			      rd->wantb - rd->haveb))) {
	    /* error EWOULDBLOCK (SUN4) or EAGAIN (SYSV) is ok */
	    if ((errno == EWOULDBLOCK) || (errno == EAGAIN))
		return (0);

	    /* all other errors except these give error message */
	    if ((errno != ECONNRESET) &&
		(errno != ECONNREFUSED) &&
		(errno != ETIMEDOUT))
		ERRprintf( ERR_WARNING,
			  "%s: non blocking read failed ret= %d errno= %d",
			  Application, ret, errno);

	    /* all except EWOULDBLOCK,  kill the connection */
	    return (-1);
	} else if (ret == 0) {
	    /* lost connection */
	    return (-1);
	} else {
	    rd->haveb += ret;
	}

    } /* while */
	 
    return (1);
}

	
static int ReadBuffer( Conn_t *cp)
    {
    	int ret = Nb_read( &cp->rd);

	if (0 == ret)			/* WOULD BLOCK */
	    return 0;		 	
	if (0 > ret)			/* ERROR */
	    {
	    KillConnection( cp);
	    return ret;
	    }

	/* message complete */
	cp->rd.ready = TRUE;
	FD_CLR( cp->sd, &Read_fds);
	return 1;	
   }

static void StartBuffer( Read_t *rd)
    {
	int len = rd->newhead.len;

	/* allocate buffer space if needed */
	if(len) {
	    if (rd->buffer == NULL)
	    {
		if (NULL == (rd->buffer = (char *) calloc( len + 1, 1)))
		{
		    ERRprintf( ERR_RESOURCE, "SOK2 StartBuffer calloc failed, len = %d", len);
		    exit(10);
		}
		rd->buffer_size = len;
 	    }
	    else if (rd->buffer_size < len)
	    {
		free (rd->buffer);
		if (NULL == (rd->buffer = (char *) calloc( len + 1, 1)))
		{
		    ERRprintf( ERR_RESOURCE, "SOK2 StartBuffer calloc2 failed, len = %d", len);
		    exit(10);
		}
		rd->buffer_size = len;
 	    }
	    rd->buffer[len] = '\0';
	}
	/* start up the buffer read */
        rd->cbuffer = rd->buffer;
        rd->wantb = len;
        rd->haveb = 0;
        rd->status = 3;
    }

static int ReadHeaderPart2( Conn_t *cp)
    {
    	int ret = Nb_read( &cp->rd);

	if (0 == ret)			/* WOULD BLOCK */
	    return 0;		 	
	if (0 > ret)			/* ERROR */
	    {
	    KillConnection( cp);
	    return ret;
	    }

	/* put header in host byte order */
       	cp->rd.newhead.id = ntohl(cp->rd.newhead.id);
       	cp->rd.newhead.len = ntohl(cp->rd.newhead.len);
       	cp->rd.newhead.seq_no = ntohl(cp->rd.newhead.seq_no);

	StartBuffer( &cp->rd);
      	return (ReadBuffer( cp));
    }

static int ReadHeaderPart1( Conn_t *cp)
    {
	Header_code 	*code;
	Read_t	*rd = &cp->rd;
    	int ret = Nb_read( rd);

	if (0 == ret)			/* WOULD BLOCK */
	    return 0;		 	
	if (0 > ret)			/* ERROR */
	    {
	    KillConnection( cp);	
	    return ret;
	    }

 	/* first part of header is read in. see if old or new  */
	code = (Header_code *) &rd->oldhead;
    	rd->head_type = ((code->long1 == SOK2_HEADER_CODE) &&
                (code->long2 == SOK2_HEADER_CODE));

	/* if old header, convert to new header */
	if (0 == rd->head_type)
	    {
       	    rd->newhead.id = ntohs((unsigned short) rd->oldhead.id);
       	    rd->newhead.len = (long) ntohs(rd->oldhead.length) * 2 - sizeof(OldHeader);
       	    rd->newhead.seq_no = ntohs((unsigned long) rd->oldhead.seq_no);

	    StartBuffer( rd);
      	    return (ReadBuffer( cp));
	    }
	else
	    {
	    /* start reading the new header */
            rd->cbuffer = (char *) &rd->newhead;
            rd->wantb = sizeof( SOK2head);
            rd->haveb = 0;
            rd->status = 2;
	
	    return ReadHeaderPart2( cp);
	    }
    }


/* continue to read in the message; return 1 if message
   completely read in, 0 if not done yet, -1 lost connection */

static int ReadMessage( int sd)
{
    Conn_t		*cp;
    Read_t 	*rd;

    if (NULL == (cp = FindConn( sd, "ReadMess")))
	return -1;

    rd = &cp->rd;

    if (rd->status == 0) {
	/* start reading in the header */
	rd->cbuffer = (char *) &rd->oldhead;
	rd->wantb = sizeof( OldHeader);
	rd->haveb = 0;
	rd->status = 1;

	return ReadHeaderPart1( cp);
    }
    else if (rd->status == 1)
	return ReadHeaderPart1( cp);
    else if (rd->status == 2)
	return ReadHeaderPart2( cp);
    else if (rd->status == 3)
	return ReadBuffer( cp);

    return -1; /* shouldnt get here */
}



/******************* socket calls ****************************************/

static int InitSocket( int family, int sd, char *where)
    {
      	int val;
      	int valen = sizeof(val);
	struct linger sl;

	/* allow the socket to be reused */
   	val = 1;
      	errno = 0;
      	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, valen) < 0)
      	    {
            ERRprintf(ERR_WARNING, "%s %s: setsockopt REUSEADDR failed; errno = %d", 
 		Application, where, errno);
            return(FALSE);
            }
   
      	/* make sockets disappear quickly on close */
	sl.l_onoff = 0; /* dont linger */
     	errno = 0;
      	if (setsockopt(sd, SOL_SOCKET, SO_LINGER, (char *) &sl, sizeof(sl) ) < 0)
            {
            ERRprintf(ERR_WARNING, "%s %s: setsockopt DONTLINGER failed; errno = %d", 
 		Application, where, errno);
            return(FALSE);
            }
 
      /* we want to detect when connection is lost */
      if (family == AF_INET)
 	 {
	 val = 1; /* enable this option */
         errno = 0;
         if (setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, (char *) &val, valen) < 0)
            {
            ERRprintf(ERR_WARNING, "%s %s: setsockopt KEEPALIVE failed; errno = %d", 
 		Application, where, errno);
            }

         /* if (getsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, &val, &valen) == -1)
            {
            ERRprintf(ERR_WARNING, "%s: getsockopt KEEPALIVE failed; errno = %d", 
 		where, errno);
            }
         fprintf( stderr, "%s getopt on socket %d, val = %d, len = %d\n",
		where, sd, val, valen); */
	 }

      /* set the socket descriptor to no delay.
	 SUN4 : use FNDELAY
	 SYSV: O_NONBLOCK is preferable to O_NDELAY, both because its POSIX, and because
	 the O_NDELAY returns 0 on block, while O_NONBLOCK returns -1 (SYSV)
       */
      errno = 0; 
#ifdef SUNOS4
      if (fcntl(sd, F_SETFL, FNDELAY) == -1)
#else
      if (fcntl(sd, F_SETFL, O_NONBLOCK) == -1)
#endif
         {
         ERRprintf(ERR_WARNING, "%s %s: set no delay on socket %d; errno = %d", 
 		Application, where, sd, errno);
         return(FALSE);
         }


      return (TRUE);
   }

/* Accept a new connection on a listening socket sd 
   Return new socket desc., and its "sockaddr".
*/
static int Accept( int sd, struct sockaddr * saddp)
{
    int 	new_sd, family;
    size_t  	sadd_len;

    static union sunion {
	struct sockaddr_in sin;
	struct sockaddr_un sund;
    } sadd;
 
    family = AF_INET;

    /* family specific addresses */
    if (family == AF_INET) {
	 sadd_len = sizeof (struct sockaddr_in);
    } else if (family == AF_UNIX) {
	 sadd_len = sizeof (struct sockaddr_un);
     }

    /* accept the connection */
    errno = 0;
    if ((new_sd = accept(sd, (struct sockaddr *) &sadd, (socklen_t*) &sadd_len)) < 0) {
	ERRprintf(ERR_RESOURCE, "%s: accept failed, errno = %d", 
		  Application, errno);
	return -1;
    }

    /* set socket options */	
    if (!InitSocket( family, new_sd, "Accept"))
	return (-1);

    /* success */
    memcpy( saddp, &sadd, sizeof(struct sockaddr));
    return ( new_sd);
}

      
/* Accept a new connection on a server socket */
static int AcceptNewClient( int sd)
    {
      	int		 new_sd;
	Sok2_t		*sok;
	Conn_t		*connp;
      	struct sockaddr sadd;
 
      	if (NULL == (sok = FindServerSocket( sd)))
	    {
	    ERRprintf( ERR_PROGRAM, "Accept cant find sd %d", sd);
            return -1;
	    }

        /* accept the connection */
        if (0 > (new_sd = Accept( sd, &sadd))) 
	    {
	    /* disable further accepting */
	    ERRprintf( ERR_WARNING, "Disable accept on %d\n", sd);
	    FD_CLR( sd, &Listen_fds);
            return -1;
	    }

	/* put it in this server's connection array */
   	if (NULL == (connp = AddConn( sok)))
	    {
	    /* disable further acceping */
	    ERRprintf( ERR_WARNING, "Disable accept on %d\n", sd);
	    FD_CLR( sd, &Listen_fds);
            return -1;
	    }

	/* success */
	InitConnection(connp, new_sd);
	connp->retry_secs = 0;  /* no retry connection; just kill it */
       	memcpy( &connp->sadd, &sadd, sizeof(struct sockaddr));

        return 1;
    }



/*_____________________________________________________________________________
 *
 *  Connect to a given socket.  Return socket descriptor for connection 
 *  or negative value on failure. Special errors: -3 = connect failed;
 *  -2 = gethostbyname failed. 
 *
 * 7/28/93 JCaron:
 * In non-blocking socket, a connect across a router will "succeed" with
 * errno = EINPROGRESS. Therefore, do a select on the socket to verify that the socket
 * is really connected, waiting a maximum of wait_msecs.
 * see Stevens, Network Programming, p 321 and p 331.
 *_____________________________________________________________________________
 */

static int 
Connect( int family, char *name, int port, struct sockaddr *saddp, int wait_secs)
{
    struct hostent 	*hp;
    int 		sd, ret, sadd_len;
    struct sockaddr_in sin;
    struct sockaddr_un sund;
    struct sockaddr    *sadd;
    int debug = FALSE;

    assert(name && *name);
    /* assert(port > 0); */
    assert(saddp);


    /* create a socket */
    errno = 0;
    if ((sd = socket(family, SOCK_STREAM, 0)) < 0) {
	ERRprintf(ERR_RESOURCE, "%s Connect: create socket failed; errno = %d", 
		  Application, errno);
	return(-1);
    }

    /* set socket options */	
    if (!InitSocket( family, sd, "Connect"))
	return (-1);
    
    if (family == AF_INET) { 
	/* get the destination host address */
	if  ((hp = gethostbyname(name)) == NULL) {
	    ERRprintf(ERR_WARNING,
		      "%s Connect: gethostbyname failed; host = %s, errno = %d", 
		      Application, name, errno);
	    return(-2);
	}
  
	/* copy destination address information into the address structure */
	memset((char *) &sin, 0, sizeof(sin));
	memcpy((char *) &sin.sin_addr, hp->h_addr, hp->h_length);
	sin.sin_family = hp->h_addrtype;
	sin.sin_port = htons(port);
	
	sadd = (struct sockaddr *) &sin;
	sadd_len = sizeof (sin);

    } else if (family == AF_UNIX) {
	sund.sun_family = AF_UNIX;
	strcpy( sund.sun_path, name);

	sadd = (struct sockaddr *) &sund;
	sadd_len = sizeof (sund);
    } else {
	/* invalid family */
	ERRprintf(ERR_PROGRAM, "%s Connect: invalid family = %d on %s %d", 
		  Application, family, name, port);
	return(-1);
    }
  
 try_connect:
    /* connect to the destination socket */
    errno = 0;
    if (connect(sd, sadd, sadd_len) < 0) {
	if (errno == EINTR)
	    goto try_connect;
	if ((errno != EINPROGRESS) && (errno != ECONNREFUSED) && (errno != ETIMEDOUT))
	    ERRprintf(ERR_WARNING,"%s connect failed; errno = %d",  Application, errno);
	
	if (errno == EINPROGRESS) {
	    /* wait in a select call until connect is verified */
	    fd_set 		read_fds, write_fds;
	    struct timeval  	wait;
		
	try_select:
	    /* wait for the fd to be write enabled */
	    FD_ZERO( &read_fds);
	    FD_ZERO( &write_fds);
	    FD_SET( sd, &read_fds);
	    FD_SET( sd, &write_fds);

	    errno = 0;
	    wait.tv_sec = wait_secs;
	    wait.tv_usec = 0;
	    ret = select( Select_width, &read_fds, &write_fds, NULL, &wait);
	    if (ret > 0) {
		int read_set = FD_ISSET( sd, &read_fds);
		if (debug) printf("Connect: select succeeded, errno = %d, read set = %d\n", 
				  errno, read_set);

		/* read is set means connection refused; we will assume this here
		 * however, a better thing to do would be to actually try to read a message
		 * because presumably theres a chance that a legitimate message could 
		 * arrive here.
		 * however, we will have to recode things a bit to allow reading at this early 
		 * clientOpne() sequence, so for now well kludge it.
		 */
		if (!read_set)
		    goto success;
	    } else {
		/* ok to have system call interrupted */
		if (errno == EINTR)
		    goto try_select;

		if (debug) printf("Connect: select returned %d errno = %d\n", ret, errno);
	    }

	}

	/* fatal error */
	shutdown(sd, 2);
	close(sd);
	return(-3);
    }
    
 success:
    memcpy( (char *)saddp, (char *)sadd, sizeof(struct sockaddr));
    return ( sd);
}


 /* Create a socket that listens for connections */
static int Listen( int family, char *host, int port)
   {
      int sd, sadd_len;
      struct sockaddr_in sin;
      struct sockaddr_un sund;
      struct sockaddr    *sadd;
  
      /* create a socket */
      errno = 0;
      if ((sd = socket(family, SOCK_STREAM, 0)) < 0)
         {
         ERRprintf(ERR_RESOURCE, "%s: create socket failed; errno = %d", 
 		Application, errno);
         return(-1);
         }

      /* set socket options */	
      if (!InitSocket( family, sd, "Listen"))
	 return (-1);

      /* family specific addresses */
      if (family == AF_INET)
   	 { 
         STRset( &sin, 0, sizeof(sin));
         sin.sin_family = AF_INET;
         sin.sin_addr.s_addr = htonl(INADDR_ANY);
         sin.sin_port = htons(port);

         sadd = (struct sockaddr *) &sin;
	 sadd_len = sizeof (sin);
	 }
      else if (family == AF_UNIX)
	 {
	 char *name = host;
	 sund.sun_family = AF_UNIX;
	 strcpy( sund.sun_path, name);

         sadd = (struct sockaddr *) &sund;
	 sadd_len = sizeof (sund);

	 /* make sure file doesnt already exist */
	 errno = 0;
   	 if (0 > unlink(name))
            if (errno != ENOENT)
               ERRprintf(ERR_INFO, "error %d unlinking unix file %d", 
		 errno, name);
	 }
      else
	 {
	 /* invalid family */
         ERRprintf(ERR_PROGRAM, "%s: invalid family = %d on %s %d", 
 		Application, family, host, port);
         return(-1);
	 }
  
      /* bind this address */
      errno = 0;
      if (bind(sd, sadd, sadd_len) < 0)
         {
         ERRprintf(ERR_WARNING, "%s: bind failed on %s %d; errno = %d", 
 		Application, host, port, errno);

         (void) shutdown(sd, 2);
         (void) close(sd);

         return(-1);
         }

      /* listen for connections */
      errno = 0;
      if (listen(sd, Listen_backlog) < 0)
         {
         ERRprintf(ERR_WARNING, "%s: listen failed; errno = %d", 
 		Application, errno);

         (void)shutdown(sd, 2);
         (void)close(sd);

         return(-1);
         }
  
      /* success */
      return (sd);
   }

static time_t Reconnect( void)
    /* if enough time has elapsed, try to reconnect down sockets.
       return the next time a reconnect is needed, in secs from now */
    {
      	time_t now_time = time( NULL);
	int	i, j, newsd; 
	int	all = TRUE;
	time_t	next, next_secs = LONG_MAX;
	Sok2_t *sp;
      	Conn_t *cp;

      	/* find all down sockets and see if its time to try and reconnect */
      	for (i=0; i< Nsockets; i++)
	    {
	    sp = &Sockets[i];

	    for (j=0; j< sp->nconns; j++)
		{
	  	cp = &sp->conn[j];
		if ((cp->down) && (cp->retry_secs > 0))
	    	    {
	    	    /* time to retry yet? */
	    	    next = cp->last_retry + cp->retry_secs - now_time;
	    	    if (next > 0)
	       		{
               		next_secs = MIN( next, next_secs);
	       		all = FALSE;
	       		continue;
	       		}
	
	    	    /* retry */
            	    cp->last_retry = now_time;

            	    if (0 > (newsd = Connect( AF_INET, sp->host, sp->port, &cp->sadd, DEFAULT_CONNECT_WAIT)))
	       		{
	       		/* connect failed */
               		next_secs = MIN( cp->retry_secs, next_secs);
	       		all = FALSE; 
	       		}
	   	    else
	       		{
	       		/* connect succeeded */
			InitConnection( cp, newsd);
			}
	       	    } /* down socket */

	    	} /* loop over connections */
	 } /*  loop over sok2 */

      	if (all)
	    {
            DownSockets = FALSE;
	    return 0;
	    }
	else
      	    return (next_secs);
    }

static struct timeval *CalcTimeout( int wait_msecs, int reconn_secs, int *timeout_reconn)
    /* set timeout_reconn TRUE if timeout due to reconnection */
    {
    	static struct timeval wait;

	*timeout_reconn = FALSE;

    	if ((0 > wait_msecs) && (reconn_secs == 0))
	    return NULL;
    	else if (0 > wait_msecs)
	    {
	    wait.tv_sec = reconn_secs;
	    wait.tv_usec = 0;
	    *timeout_reconn = TRUE;
	    }
    	else if ((reconn_secs == 0) || (reconn_secs > wait_msecs/1000))
	    {
	    wait.tv_sec = wait_msecs / 1000;
	    wait_msecs -= wait.tv_sec * 1000;
	    wait.tv_usec = wait_msecs * 1000;
	    }
    	else
	    {
	    wait.tv_sec = reconn_secs;
	    wait.tv_usec = 0;
	    *timeout_reconn = TRUE;
	    }
      	return &wait;	
    }

/**************************************************/
/* this is the main i/o routine; find out if theres any messages to read,
   write, or new client connections to accept.
   Do as much as you can until i/o would block, and/or timeout expires,
   or a message is completely read (return_on & 1) or a message is
   completely written (read_on & 2)
*/
static int DoInputOutput( int wait_msecs, unsigned return_on)
    /* wait_msecs = millisecs before timeout 
     * return_on =  bit 0 = return on message read 
     *	            bit 1 = return on message written
     */
    {  
      	int    		sd, ret;
	int 		put_message = FALSE, got_message = FALSE;
      	fd_set 		read_on, write_on;
	int		msecs, reconn_secs, timeout_reconn;
	unsigned long 	elapsed_ms;
    	struct timeval  *waitp;
  
        UTIMtime_diff (TRUE);

 	/* loop until timeout */
      	while (TRUE)
	    { 
            elapsed_ms = UTIMtime_diff( FALSE);

	    /* see if any reconnections need to be done */
	    if (DownSockets)
	        reconn_secs = Reconnect();
	    else 
		reconn_secs = 0;
	    
	    /* recalculate needed timeout */
	    if (wait_msecs > 0)
		{
	    	msecs = wait_msecs - elapsed_ms;
	    	if (msecs <= 0)
		    return 0;
		}
	    else
		msecs = wait_msecs;
	    waitp = CalcTimeout( msecs, reconn_secs, &timeout_reconn);

	    /* these are the sockets to select on */
	    FdUnion( &read_on, &Read_fds, &Listen_fds);
     	    FdUnion( &write_on, &Write_fds, NULL);

	    errno = 0;
	    if (0 > (ret = select( Select_width, &read_on, &write_on, NULL, waitp)))
	    	{
                /* ok to have system call interrupted */
	    	if (errno == EINTR)
 	            continue;
    
	    	ERRprintf(ERR_WARNING, "%s: select failed; errno = %d", 
		     Application, errno);
	    	return ret;
	    	}

	    /* timeout */
            if (ret == 0)
		{
		if (timeout_reconn)
	     	    continue;
		else
		    return 0;
		}
	  
	    /* process write sockets first */
	    for ( sd=0; sd < Select_width; sd++)
	 	{
	        if ( FD_ISSET( sd, &write_on) && FD_ISSET( sd, &Write_fds))
		    {
		    if (1 == WriteMessage(sd))
		  	put_message = TRUE;
		    }
		}
	    
	    /* process read sockets */
	    for ( sd=0; sd < Select_width; sd++) {

		/* look for new clients */
	        if ( FD_ISSET( sd, &read_on) && FD_ISSET( sd, &Listen_fds)) {
		    AcceptNewClient( sd);
		} else if ( FD_ISSET( sd, &read_on) && FD_ISSET( sd, &Read_fds)) {
		    /* a message to read */
		    if ( 1 == ReadMessage( sd))
			got_message = TRUE;
		    }
		}

	    if (got_message && (return_on & BIT0))
	 	return 1;

	    if (put_message && (return_on & BIT1))
	 	return 1;

	    } /* loop until timeout */
    }

/********** external interface routines */

int SOK2init( char *applic_name)
{
    static int ncalls = 0;
    int i;
    char *envstr;

    ncalls++;

    if (ncalls > 1) {
	ERRprintf(ERR_PROGRAM, 
		  "%s called SOK2init %d times",
		  applic_name, ncalls);
	return 0;
    }
	
    STRncopy( Application, applic_name, MAX_APPLIC_NAME);
    
    if (NULL != (envstr = getenv( "SOK2_LISTEN_BACKLOG")))
    {
	Listen_backlog = atoi( envstr);
	printf("Listen Backlog = %d\n", Listen_backlog);
    }

    /* no default server mapper hosts */
    for (i=0; i <MAX_HOSTS; i++)
	Hosts[i] = NULL;	
    HostCount = 0;
    
    /* number of fds in an fd_set ; could try to reduce this to min necessary
       by calling ??
       */
    Select_width = FD_SETSIZE;

    FD_ZERO( &Listen_fds);
    FD_ZERO( &Read_fds);
    FD_ZERO( &Write_fds);
    
    PORTsignal( SIGPIPE, SIG_IGN);
    return 1;
    
}



void SOK2exit( int code)
{
    int 	i, j;
    Sok2_t 	*sp;
	
    /* close all sockets */
    for (i=0; i<Nsockets; i++) {
	sp = &Sockets[i];

	if (sp->type == 0)
	    continue;

	if (sp->type == 1) {
	    shutdown( sp->sd, 2);
	    close( sp->sd);
	}

	for (j=0; j < sp->nconns; j++)
	    if ((sp->conn[j].used) && (!sp->conn[j].down)) {
		shutdown( sp->conn[j].sd, 2);
		close(  sp->conn[j].sd);
	    }
    }

    Nsockets = 0;
    return;
}


int  SOK2setServiceFile( int where, char *name )
{
  char	 *value;
  int i;

  UseServerMapper = (where == SOK2_SERVMAP);
	
  switch (where) {

  case SOK2_ENV:
    if (NULL == (value = getenv( name))) {
      ERRprintf( ERR_WARNING, 
		"Environmental variable %s not set", name);
      return (-1);
    } else {
      STRncopy( ServiceFile, value, MAX_SERVICE_FILENAME);
    }
    break;

  case SOK2_FILE:
    STRncopy( ServiceFile, name, MAX_SERVICE_FILENAME);
    break;

  case SOK2_SERVMAP:
    if (HostCount >= MAX_HOSTS) {
      ERRprintf( ERR_RESOURCE, "Too many servmap hosts, max = %d", MAX_HOSTS);
      return (-2);
    }
    for (i = 0; i < HostCount; i++) {
      if (!strcmp(name, Hosts[i]))
	return (1);
    } /* i */
    
    Hosts[ HostCount] = (char *) malloc( strlen(name) + 1);
    strcpy( Hosts[ HostCount], name);
    
    HostCount++;
    break;
  }
  
  return (1);
}


int SOK2findService( char *service, char **host, int *port )
{ 
#define MAX_TOKEN 40
    FILE	*fp; 
    int		done, ret = 1, family; 
    char 	line[512]; 
    char	*ptr, token[ MAX_TOKEN];
    static 	char hostname[ MAX_HOSTNAME];

    if (UseServerMapper)
	return SOK2servmapInfo( SERVMAP_TYPE_OLD_SERVER, service, "",
			       host, port);  	

    if (NULL == (fp=fopen( ServiceFile, "r" ))) {
	ERRprintf( ERR_WARNING, "Service File %s could not be opened", ServiceFile);
	return -6;
    }
 
    done = FALSE; 
    while( !done && fgets( line, sizeof( line ), fp ) != NULL ) { 
	if( (int) line[0] != '#' ) {
	    ptr = line;
	    if (NULL == STRtokn( &ptr, token, MAX_TOKEN, " ,"))
		continue;
	    if (STRequal( service, token)) {
		/* host name */
		if (NULL == STRtokn( &ptr, hostname, MAX_HOSTNAME, " ,"))
		{ ret = -8; goto out; }
		*host = hostname;
		if (STRequal( hostname, "local")) {
		    STRncopy( hostname, PORThostname(), MAX_HOSTNAME);
		}

		/* protocol */
		if (NULL == STRtokn( &ptr, token,  MAX_TOKEN, " ,"))
		{ ret = -8; goto out;}
		if (STRequal(token, "TCP"))
		    family = AF_INET;
		else if (STRequal(token, "UNIX"))
		    family = AF_UNIX;
		else
		{ ret = -8; goto out; }

		/* port */
		if (NULL == STRtokn( &ptr, token, MAX_TOKEN, " ,"))
		{ ret = -8; goto out; }
		if (family != AF_UNIX) 
		    if(0 == (*port = atoi( token)))
		    { ret = -8; goto out; }

		done = TRUE;
	    } /* this is the service wanted */
	} /* not a comment */
    } /* fgets */

    if( !done ) 
	ret = -7;

 out:
    fclose( fp ); 
    return( ret);
} 


int SOK2servmapInfo( char *type, char *subtype, char *instance, char **host, int *port)
    {
	SERVMAP_request_t req;
	SERVMAP_info_t *info;
	int		i, ret = -1;
	int		nservers;

	STRset( &req, 0, sizeof(SERVMAP_request_t));

	STRncopy( req.server_type, type, SERVMAP_NAME_MAX);
	STRncopy( req.server_subtype, subtype, SERVMAP_NAME_MAX);
	STRncopy( req.instance, instance, SERVMAP_INSTANCE_MAX);

	req.want_realtime = TRUE;
	req.time = 0;

	/* query all server mappers until success */
	for (i=0; i< HostCount; i++)
	    {
	    if (0 < (ret = SMU_requestInfo( &req, &nservers, &info, Hosts[i], NULL)))
		{
		/* ignores other servers if more than one */
		*host = info->host;
		*port = info->port;
		return 1;
		}
	    }
	return ret;
    }

static void Timer( int sig)
    {
	int i;

	for (i=0; i < RegCount; i++)
	    SOK2register( RegInfo[i].port, RegInfo[i].server_type, 
		RegInfo[i].server_subtype, RegInfo[i].instance);
    }

void SOK2registerTimer( int port, char *type, char *subtype, char *instance)
    {
      	struct itimerval timer;
	SERVMAP_info_t *rp;

	/* keep a global struct for timer based server mapper registration */
	RegCount++;
	if (RegInfo == NULL)
	    RegInfo = (SERVMAP_info_t *) malloc( sizeof(SERVMAP_info_t));
	else
	    RegInfo = (SERVMAP_info_t *) realloc( RegInfo, RegCount * sizeof(SERVMAP_info_t));
	rp = &RegInfo[ RegCount-1];

	STRncopy( rp->server_type, type, SERVMAP_NAME_MAX);
	STRncopy( rp->server_subtype, subtype, SERVMAP_NAME_MAX);
	STRncopy( rp->instance, instance, SERVMAP_INSTANCE_MAX);
	rp->port = port;

	/* register now */
	SOK2register( port, type, subtype, instance);

      	/* set the timer the first time */
	if (RegCount == 1)
	    {
      	    timer.it_interval.tv_sec = SERVMAP_REGISTER_INTERVAL;
      	    timer.it_interval.tv_usec = 0;
      	    timer.it_value.tv_sec = SERVMAP_REGISTER_INTERVAL;
      	    timer.it_value.tv_usec = 0;
      	    if (0 > setitimer( ITIMER_REAL, &timer, NULL))
	    	ERRprintf(ERR_PROGRAM, "setitimer failed errno = %d\n", errno);
      	    else
	    	PORTsignal( SIGALRM, Timer);
	    }
    }

void SOK2register( int port, char *type, char *subtype, char *instance)
    {
	SERVMAP_info_t info;
	int i;

	STRset( &info, 0, sizeof(SERVMAP_info_t));

	STRncopy( info.server_type, type, SERVMAP_NAME_MAX);
	STRncopy( info.server_subtype, subtype, SERVMAP_NAME_MAX);
	STRncopy( info.instance, instance, SERVMAP_INSTANCE_MAX);
	
	/* SMU_register fills in the hostname */
	info.port = port;

	info.realtime_avail = TRUE;
	info.start_time = 0;
	info.end_time = 0;

	for (i=0; i<HostCount; i+=2)
	    SMU_register( &info, Hosts[i], Hosts[i+1]);
    }

void SOK2registerStatus( int port, char *type, char *subtype, char *instance,
			 time_t last_data, time_t last_request)
    {
	SERVMAP_info_t info;
	int i;

	STRset( &info, 0, sizeof(SERVMAP_info_t));

	STRncopy( info.server_type, type, SERVMAP_NAME_MAX);
	STRncopy( info.server_subtype, subtype, SERVMAP_NAME_MAX);
	STRncopy( info.instance, instance, SERVMAP_INSTANCE_MAX);
	
	/* SMU_register fills in the hostname */
	info.port = port;

	info.realtime_avail = TRUE;
	info.start_time = 0;
	info.end_time = 0;
	info.last_data = (long) last_data;
	info.last_request = (long) last_request;

	for (i=0; i<HostCount; i+=2)
	    SMU_register( &info, Hosts[i], Hosts[i+1]);
    }

/*****************************************************************************/

SOK2idx SOK2openClientWait( char *host, int port, int reconn_secs, int wait_secs)
    {
	int	idx, sd;
	time_t	now;
	Sok2_t *sp;
    	struct sockaddr sadd;	

	/* Make sure that SOK2init() has been called -- wilsher 19 July 1994  */
	assert(Select_width > 0);

	if (reconn_secs < 0)
	    reconn_secs = DEFAULT_RETRY_SECS;
	if (wait_secs < 0)
	    wait_secs = DEFAULT_CONNECT_WAIT;

	/* initialize the Sok2_t struct */
	if (0 > (idx = AddSocket()))
	    return -1;
	sp = &Sockets[idx];

	if (STRequal( host, "local"))
	    {
	    STRncopy( sp->host, PORThostname(), MAX_HOSTNAME);
	    }
	else
	    STRncopy( sp->host, host, MAX_HOSTNAME);

	/* try to make the socket connection */
	sd = Connect( AF_INET, sp->host, port, &sadd, wait_secs);
	if ((sd < 0) && (sd != -3))
	    return sd;

	/* if failed, and no retry */
	if ((-3 == sd) && (0 == reconn_secs))
	    return -3;

	/* initialize the connection; note we have left sp.type = 0 until we
	   know we are going to return a valid sidx; this avoids memory leaks  */
	if (NULL == (sp->conn = (Conn_t *) calloc( 1, sizeof(Conn_t))))
	    return -1;
	memcpy(&sp->conn->sadd, &sadd, sizeof(struct sockaddr));
	sp->port = port;
	sp->type = 2;
	sp->sd = -1;
	sp->nconns = 1;
	sp->conn->retry_secs = reconn_secs;

	/* deal with failed connection */
	if (-3 == sd) 
	    { 
	    sp->conn->used = TRUE;
	    sp->conn->rd.status = 0;
	    sp->conn->rd.ready = FALSE;
	    sp->conn->wrt.status = 0;
	    sp->conn->wrt.write_pending = FALSE;

	    /* mark as down */
      	    sp->conn->down = TRUE;
      	    time( &now);
            sp->conn->time_down = now;
            sp->conn->last_retry = now;
	    DownSockets = TRUE;
	    }
	else
	    {
	    InitConnection( sp->conn, sd);
	    }

	return idx;
    }

SOK2idx SOK2openClient( char *host, int port, int reconn_secs)
    {
	return SOK2openClientWait( host, port, reconn_secs, DEFAULT_CONNECT_WAIT);
    }

SOK2idx  SOK2openServer( int port)
    {
	int idx, sd;
	Sok2_t *sp;
	static char host[] = "server";

	/* Make sure that SOK2init() has been called -- wilsher 19 July 1994 */
	assert(Select_width > 0);

	if (port <= 0)
	    return -2;	/* illegal port no */

	if (0 > (sd = Listen( AF_INET, host, port)))
	    return sd;
	FD_SET( sd, &Listen_fds);

	/* initialize the Sok2_t struct */
	if (0 > (idx = AddSocket()))
	    return -1;
	sp = &Sockets[idx];

	STRncopy( sp->host, host, MAX_HOSTNAME);
	sp->port = port;
	sp->type = 1;
	sp->sd = sd;
	sp->conn = NULL;
	sp->nconns = 0;

	return idx;
    }

int SOK2close( SOK2idx idx)
   /* close a client or server socket, and all their connections */
    {
	Sok2_t 	*sp = &Sockets[ idx];
	int j;

	if ((idx < 0) || (idx >= Nsockets))
	    return FALSE;

	if (sp->type == 1)
	    {
      	    shutdown( sp->sd, 2);
            close( sp->sd);
	    FD_CLR( sp->sd, &Listen_fds);
	    FD_CLR( sp->sd, &Read_fds);
	    FD_CLR( sp->sd, &Write_fds);
	    }

	for (j=0; j < sp->nconns; j++)
	    {
	    Conn_t *cp = &sp->conn[j];
	    if (cp->used && !cp->down)
		{
		cp->retry_secs = 0;  /* tells it not to retry connection */
		KillConnection( cp);
		}
	    }

	if (NULL != sp->conn)
	    free (sp->conn);
	sp->conn = NULL;
	sp->type = 0;
	sp->nconns = 0;

	return TRUE;
    }

/*****************************************************************************/

    /* Check Sok2_t structure for any ready messages. Keep static info so
       that we query the structure uniformly
       Return TRUE if one found. */
static int CheckMessage( SOK2idx *idx, int *client, SOK2head *head, 
			    char **message, int *mess_len, Conn_t **cpp)
    {
	static int 	Sok_idx = 0, Conn_idx = 0;
	int		count = 0;

	/* anything been initialized ? */
	if (Nsockets == 0)
	   return FALSE;

	/* move through the sockets, looking for "ready" messages.
	   keep state, so that you query the structure uniformly */
	while(TRUE)
	    {
	    Sok2_t *sp = &Sockets[ Sok_idx];

	    if (sp->type == 0)  /* Sok may have closed since last call */
		goto next;

	    while (Conn_idx < sp->nconns)
		{
	    	Conn_t *cp = &sp->conn[ Conn_idx];
	        if (cp->used && !cp->down && cp->rd.ready)
		    {
		    *idx = Sok_idx;
		    *client = ((sp->type == 1) ? Conn_idx : -1);
		    memcpy( head, &cp->rd.newhead, sizeof( SOK2head));
		    *message = cp->rd.buffer;
		    *cpp = cp;
		    *mess_len = cp->rd.newhead.len;

		    cp->rd.ready = FALSE;
		    cp->rd.status = 0;
		    Conn_idx++;

		    return TRUE;
		    }

	        Conn_idx++;
		}

	next:
	    Sok_idx++;
	    Conn_idx = 0;
	    count++;

	    if (Sok_idx >= Nsockets)
		Sok_idx = 0;

	    if (count > Nsockets)
		break;
	    }
	
	return FALSE;
    }

int SOK2getMessage( int wait_msecs, SOK2idx *idx, int *client, 
		    SOK2head *head, char **message, int *mess_len )
{
    static Conn_t 	*cp = NULL;
    int			ret;

    /* Make sure that SOK2init() has been called -- wilsher 19 July 1994  */
    assert(Select_width > 0);

    /* anything been initialized ? */
    if (Nsockets == 0)
	return -10;

    /* reenable the socket with the previous message */
    if (NULL != cp)  {
	if (cp->used && !cp->down) /* may have gone down */
	    FD_SET(cp->sd, &Read_fds);
	cp = NULL;
    }

    /* first do any pending i/o */
    DoInputOutput( 0, 0);

    /* now see if theres a message ready */
    if (CheckMessage( idx, client, head, message, mess_len, &cp)) {
	ret = 1;
	goto out;
    }

    /* now do i/o until timeout or message is ready */
    if (0 == DoInputOutput( wait_msecs, BIT0)) {
	ret = 0;
	goto out;
    }

    /* should have a message */
    if (1 != (ret = CheckMessage( idx, client, head, message, mess_len, &cp)))
	ERRprintf( ERR_PROGRAM,"getMessage logic failed");

 out:
#ifdef MDEBUG
    if (!malloc_verify()) {
	printf("SOK2getMessage malloc_verify failed\n");
	kill(getpid(), SIGSEGV);
    }

#endif
    return ret;
}


static int SendMessage( int wait_msecs, SOK2idx idx, int client, int id, 
			char *message, int mess_len, Conn_t **cpp)
    {
	Sok2_t 	*sp = &Sockets[ idx];
	Conn_t	*cp;
	int	ret;

	/* anything been initialized ? */
	if (Nsockets == 0)
	   return -1;

	if (wait_msecs < 0)
	    wait_msecs = -1;

	/* check bad stuff */
	if (sp->type == 0)
	    return -2;

	if (sp->type == 2)
	    client = 0;
	else
	    {
	    if ((client < 0) || (client >= sp->nconns))
	    	return -4;
	    }
	cp = &sp->conn[ client];
	*cpp = cp;

	if (!cp->used)
	    return -5;

	if (cp->down)
	    return -5;

	if (cp->wrt.write_pending)
	    return -7;

	/* use new message type */
	cp->head_type = 1;

	if (cp->head_type == 0) /* old type */
	    {
	    /* must send even number of bytes */
	    mess_len += (mess_len % 2);

	    cp->wrt.oldhead.id = htons( id);
	    cp->wrt.oldhead.length = 
			htons( (unsigned short) ((mess_len + sizeof(OldHeader))/ 2));
	    cp->wrt.oldhead.seq_no = htonl( cp->wrt.seq_no++);
	    cp->wrt.cbuffer = (char *) &cp->wrt.oldhead;
	    cp->wrt.wantb = sizeof( OldHeader);
	    cp->wrt.haveb = 0;
	    }
	else
	    {
	    Header_code *code = (Header_code *) &cp->wrt.oldhead;
	    code->long1 = SOK2_HEADER_CODE;
	    code->long2 = SOK2_HEADER_CODE;
	    cp->wrt.newhead.id = htonl( id);
	    cp->wrt.newhead.len = htonl(mess_len);
	    cp->wrt.newhead.seq_no = htonl( cp->wrt.seq_no++);
	    cp->wrt.cbuffer = (char *) &cp->wrt.oldhead;
	    cp->wrt.wantb = sizeof( OldHeader) + sizeof(SOK2head);
	    cp->wrt.haveb = 0;
	    }

	cp->wrt.message = message;
	cp->wrt.mess_len = mess_len;
	cp->wrt.status = 0;

	/* now try to write the message out with no delays */
	if (Debug)
	   printf("Write seq %ld\n", cp->wrt.seq_no - 1);
	ret = WriteMessage( cp->sd);

	return ret;
    }



int 
SOK2sendMessage( int wait_msecs, SOK2idx idx, int client, int id,
		char *message, int mess_len )
{
    Conn_t	*cp;
    int	ret;
    
    /* Make sure that SOK2init() has been called -- wilsher 19 July 1994  */
    assert(Select_width > 0);
    
    /* first do any pending i/o */
    DoInputOutput( 0, 0);
    
    /* now try to send the message */
    ret = SendMessage( wait_msecs, idx, client, id, message, mess_len, &cp);
    if (0 != ret)
	goto out;
    
    /* socket may have gone down */
    if (cp->down)
    {
	ret = -5;
	goto out;
    }
    
    /* write didnt complete:
     * keep trying until message is written or we run out of time 
     */
    while (FD_ISSET( cp->sd, &Write_fds))
    {
	if (0 == (ret = DoInputOutput( wait_msecs, BIT1)))
	{
	    /* indicate message not done */
	    cp->wrt.write_pending = TRUE;
	    break;
	}
	
	/* socket may have gone down */
	if (cp->down)
	{
	    ret = -5;
	    goto out;
	}
    }
    
 out:
#ifdef MDEBUG
    if (!malloc_verify())
    {
	printf("SOK2sendMessage malloc_verify failed\n");
	kill(getpid(), SIGSEGV);
    }
    
#endif
    
    return ret;
}


int SOK2sendMessageAll( int wait_msecs, SOK2idx idx, int id,
		        char *message, int mess_len, int force )
{
    Sok2_t 	*sp = &Sockets[ idx];
    Conn_t	*cp;
    int	i, ret = 1;
    fd_set	client_fds;
    
    /* Make sure that SOK2init() has been called -- wilsher 19 July 1994  */
    assert(Select_width > 0);

    /* first do any pending i/o */
    DoInputOutput( 0, 0);

    /* check bad stuff */
    if (sp->type != 1)
	return -2;
    
    /* now try to send the message to each connected client */
    FD_ZERO( &client_fds);
    for (i=0; i<sp->nconns; i++)
    {
	cp = &sp->conn[i];
	if (cp->used && !cp->down)
	{
	    if (cp->wrt.write_pending)
	    {
		if (force)
		    KillConnection( cp);
		continue;
	    }
	    
	    SendMessage( wait_msecs, idx, i, id, message, mess_len, &cp);
	    if (cp->sd >= 0)
		FD_SET( cp->sd, &client_fds);
	}
    }
    
    /* write didnt complete:
     *   keep trying until all messages are written or we run out of time 
     */
    while (!FdSetisEmpty( &client_fds, &Write_fds))
    {
	if (Debug)
	    printf(" sendAll try with wait %d\n", wait_msecs);
	if (0 == (ret = DoInputOutput( wait_msecs, BIT1)))
	{
	    for (i=0; i<Select_width; i++)
		if (FD_ISSET(i, &Write_fds))
		{
		    /* indicate messages not done */
		    if (NULL != (cp = FindConn(i, "sendAll")))
			cp->wrt.write_pending = TRUE;
		}
	    break;
	}
    }
    
    return ret;
}

int SOK2continueWrite( int wait_msecs)
    {
	while (!FdisEmpty( &Write_fds))
	    {
	    if (0 == DoInputOutput( wait_msecs, BIT1))
	    	return 0;
	    }
	return 1;
    }

int SOK2continueIO( int wait_msecs)
    {
	return DoInputOutput( wait_msecs, BIT0 | BIT1);
    }


char *SOK2oldMessage( SOK2head *head, char *message)
   {
  	int size;
	char *oldmess;
	OldHeader oldh;

	/* convert new header to old header */
	oldh.id = (short) head->id;
	oldh.seq_no = head->seq_no;
	oldh.length = (unsigned short)( (head->len + sizeof(OldHeader))/2 );

	/* malloc enough space */
	size = sizeof(OldHeader) + head->len;
	if (NULL == (oldmess = (char *) malloc( size)))
	    return NULL;
	
	memcpy( oldmess, (char *) &oldh, sizeof(OldHeader));
	memcpy( oldmess + sizeof(OldHeader), message, head->len);

	return oldmess;
    }

 	

/**********************  status / kill *****************************************/

int SOK2killClient( SOK2idx idx, int client)
    /* for Servers only, disconnect from the given client.
       return 1 on success, < 0 on failure
    */
    {
	Sok2_t 	*sp = &Sockets[ idx];
	Conn_t	*cp;

	if (sp->type != 1) /* must be a server type socket */
	    return -3;

	if (NULL == (cp = ConnOk( idx, client)))
	    return -2;

	if (!cp->used) /* not connected */
	    return -4;

	cp->retry_secs = 0;  /* tells it not to retry connection */
	return KillConnection( cp);
    }


int SOK2killClientsPending( SOK2idx idx)
  /* for Servers only, disconnect from any clients with pending writes.
      If you are a One-way server using a single buffer for messages, and the
      previous sendMessageAll() returned 0 (not completed); you must call this
      before reusing the message buffer.  
      Note that if you are using multiple message buffers, you can set 
      force = TRUE on sendMessageAll().

      Return number of clients killed, or < 0 on error.
    */
    {
	Sok2_t 	*sp = &Sockets[ idx];
	int j;

	if ((idx < 0) || (idx >= Nsockets))
	    return -2;

	if (sp->type != 1)
	    return -3;

	for (j=0; j < sp->nconns; j++)
	    {
	    Conn_t *cp = &sp->conn[j];
	    if (cp->used && cp->wrt.write_pending)
		{
		cp->retry_secs = 0;  /* tells it not to retry connection */
		KillConnection( cp);
		}
	    }

	return 1;
    }


int SOK2statusConnection( SOK2idx idx, int client, int *write_pending)
   /*   Return TRUE (1) if connected, else FALSE (0), else < 0 on error.
	Return write_pending = TRUE if there`s a pending write on this client.
	Both Servers and Clients can use this; "client" is ignored if the socket
	is a client socket.
    */
    {
	Conn_t	*cp;

	if (NULL == (cp = ConnOk( idx, client)))
	    return FALSE;

	*write_pending = cp->wrt.write_pending;
	return (cp->used && !cp->down);
    }


/************ socket names ********************************/
static char *get_dot_name( long ladd)
   {
      static char name[40];
      int	a,b,c,d;

      a = ladd & 0xFF;
      ladd = ladd >> 8;
      b = ladd & 0xFF;
      ladd = ladd >> 8;
      c = ladd & 0xFF;
      ladd = ladd >> 8;
      d = ladd & 0xFF;
      sprintf( name,"%d.%d.%d.%d",d,c,b,a);
      return name;
   }

static char *SockGetNameAddress( struct sockaddr *sock_add)
    {
	static char name[120];
      	char 	*host;
      	int	port = 0;

      	if ((unsigned short) sock_add->sa_family ==  AF_INET)
            {
	    int 		length;
            struct sockaddr_in *isap; 
            struct hostent     *hent;

            isap = (struct sockaddr_in *) sock_add;
	    length = sizeof(struct in_addr);
         
            /* get the host name for this address */
	    /*  this call seems to fail.. bug in SunOS ? */
            if (NULL == (hent = gethostbyaddr( (char *) &isap->sin_addr, length, AF_INET)))
            	host = get_dot_name( isap->sin_addr.s_addr); 
	    else
	        host = hent->h_name;

	    port = isap->sin_port;
            }
	else
	    return "Not Inet socket";

      sprintf( name, "%s %d", host, port);
      return name;
    }

char *SOK2whoisConnected( SOK2idx idx, int client)
    {
	Conn_t	*cp;

	if (NULL == (cp = ConnOk( idx, client)))
	    return "Bad Params";

	return SockGetNameAddress( &cp->sadd);
    }

#ifdef NOTNOW

void SOK2GetFds(fd_set *listen,
		fd_set *read,
		fd_set *write,
		int *width)

{

  *listen = Listen_fds;
  *read = Read_fds;
  *write = Write_fds;
  *width = Select_width;

}

void SOK2GetSockets(Sok2_t **sockets,
		    int *nsockets)

{

  *sockets = Sockets;
  *nsockets = Nsockets;

}

#endif
