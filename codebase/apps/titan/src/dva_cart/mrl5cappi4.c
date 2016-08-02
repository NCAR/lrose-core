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
/* Program which processes radar volume scan data into 18 CAPPIs
   referenced to sea level. CAPPIs are encoded to conform with 
   TITAN routine.
   Programmers : Zanda Botha (socketing) and 
		 Marion Mittermaier (data processing)
   Last Revision : 17/10/96					*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <float.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
/*
#include "/usr/include/ncurses/unctrl.h"
#include "/usr/include/ncurses/ncurses.h"
*/

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>

#define  HOST   "localhost"
#define  RECV_PORT   16000      
#define  SEND_PORT   20000      
#define  SITE        22

#define  FOREVER            1
#define  TRUE               1
#define  FALSE              0
#define  CONNECTED          1
#define  READ_DATA_OK       1
#define  SEND_DATA_OK       1
#define  DISCONNECT        -1
#define  CLIENT_OPEN_ERROR -2
#define  SEND_ERROR        -3
#define  OPEN_ERROR        -4
#define  READ_ERROR        -5
#define  CLIENT_RECV_ERROR -8
#define  ACCEPT_ERROR      -6
#define  TIME_OUT          -7
#define  ELEVATION_STEPS   18

typedef struct 
{
    unsigned short date;
    unsigned short hour;
    unsigned short min;
    unsigned short start_azimuth;
    unsigned short end_azimuth;
    unsigned short raycount;
    unsigned short azimuth;
    unsigned short elevation;
    unsigned short mds;
    unsigned short noise;
    unsigned short rvpchi;
    unsigned short rvpclo;
    short phi;
    short plo;
    short xmt;
    unsigned short site_blk;
    unsigned short rvpc[224];
    unsigned short iff[16];
} RAY;


typedef struct
{
    unsigned char id;
    unsigned short  count;
    unsigned char not_used;
    unsigned short  ad[4];
    unsigned char flags[4];
    unsigned short  non_rcc0;
    unsigned short  non_rcc1;
    unsigned short  non_rcc2;
    unsigned short  non_rcc3;
    unsigned char   spare[36];
    unsigned short  number_of_cappis;
    unsigned short  raycount;
} STATUS;

struct volume_scan_data
{
    STATUS status;
    RAY volscan[10000];
} data;

typedef struct
{
    unsigned short  date;
    unsigned short  bhour;
    unsigned short  bmin;
    unsigned short  bsec;
    unsigned short  fhour;
    unsigned short  fmin;
    unsigned short  fsec;
    STATUS  rccstat;
} CAPPI_STATUS;

struct cappi_data
{
    CAPPI_STATUS stat;
    char fin_dbz[201][201];
} cappi;

struct match_grid
{
    struct grid_pos
    {
        unsigned short  z;
        unsigned short x1;
        unsigned short y1;
        unsigned short x2;
        unsigned short y2;
        unsigned short x3;
        unsigned short y3;
        unsigned short x4;
        unsigned short y4;
        unsigned short ii;
        unsigned short a1;
        unsigned short a2;
        unsigned short a3;
        unsigned short a4;
        unsigned short ll;
        short range_diff1;
        short range_diff2;
        short el_diff1;
        short el_diff2;
        short az_diff1;
        short az_diff2;
    } pos[10101];

} height[20];		/* if 1st index = 1, dimension must be +1 */

FILE  *tab, *range, *grid, *vip2dbz, *viptobyte, *len;

unsigned short  idbz[4096], val[4096], ntel[20];
unsigned short  idiff[4096];
unsigned short  ivip[18][360][224], rawvip;
unsigned short  azold;
unsigned short  cappiheight;
unsigned short  date;
unsigned short  pid;
unsigned short  first_fl;

short rngcor[224];
short mvipa[201][201], mvipb[201][201];

char gridfile[32];
char socketmsg[ 11 ];
char *prog = "mrl5cappi4";
char command[10];

float  binval;

int  recv_fd, send_fd, client_fd;
int  socketing = CONNECTED, send_sok = CONNECTED;
int  ch;
int  number_of_cappis;

int s;				/* connected socket descriptor */
int ls;				/* listen socket descriptor */

struct hostent *hp;		/* pointer to host info for remote host */
struct servent *sp;		/* pointer to service information */

long timevar;			/* contains time returned by time() */
char *ctime();			/* declare time formatting routine */

struct linger linger;		/* allow a lingering, graceful close; */
				/* used when setting SO_LINGER */

struct sockaddr_in myaddr_in;	/* for local socket address */
struct sockaddr_in peeraddr_in;	/* for peer socket address */
int addrlen;

char *hostname;		        /* Points to the remote host's name string.*/


main (int argc, char *argv[]) 
{
    if  (argc != 2)
    {
        printf ("%s - Please enter number of cappi's to create eg. %s 2\n", prog, prog);
        exit (1);
    }

    number_of_cappis = atoi (argv[1]); 
    printf(" %d \n", number_of_cappis);

    /*
     *  Startup server program
     */

     system ("nice -n 5 mrl5read &");
    /*   system ("mrl5read &");*/

    GET_STARTED ();

    while  ( FOREVER )
    {
	if ((socketing = RECV_DATA ()) < 0)
	{
	    /*
	     *  Server closed socket - reset everything then try to
	     *  connect again.
	     */

	    close (recv_fd);
	    SETUP_RECV_SOCKET (RECV_PORT, &recv_fd, prog);                         
	    continue;
	}

        printf ("%s - UNPACK DATA\n", prog);
	UNPACK_DATA ();

        PROCESS_CAPPI();

	/*
	 *  Check if user pressed any key to quit.
         *  if  ((ch = curs_getch ()) == ERR)  continue;
	 */

     /*   printf ("%s - TERMINATED\n", prog);
        sprintf (command, "kill %d", pid);
        system (command);
        printf ("%s - CHILD PROCESS %d TERMINATED\n", prog, pid);
        break;
      */
    }

    close (recv_fd);
    close (ls);
    close (s);
}



RECV_DATA () 
{
   int  items = 0;
   char *bufptr;
   int  bufcnt;
   int  buflen;
   int  bufsize;
   int  size;
   short i;

   /*
    *  Receive status record.
    */

   items = recv (recv_fd, (char *) &data.status, sizeof (STATUS), 0);
   if (items == -1)             
   {
       printf ("%s - RECV ERROR STATUS - items %d\n", prog, items);
       return (READ_ERROR);
   }

   printf ("\n%s - RECEIVED STATUS %d\n", prog, items);
   pid = data.status.number_of_cappis; 
   printf ("%s - CHILD PROCESS %d\n", prog, pid);

   /*
    *  Read volume scan data.
    */

   for (i = 0; i < data.status.raycount; i++)
   {
       bufcnt = 0;
       bufsize = sizeof (RAY) ;
       bufptr = (char *) &data.volscan[i];
       buflen = bufsize;

       while ( bufcnt < bufsize)
       {
           items = recv (recv_fd, bufptr, buflen, 0);
           if (items <= 0)
           {
	       printf ("%s - RECV ERROR RAY - items %d\n", prog, items);
	       return (READ_ERROR);
           }

	   bufptr += items;
	   bufcnt += items;
	   buflen -= items;
       }
   }

   printf ("%s - RECEIVED RAYS %d\n", prog, i);

   return (READ_DATA_OK);
}


UNPACK_DATA ()
{
    int  i, n;
    unsigned short  el, az, bcdval, ray;
    unsigned short  rvpc[224];            

    /*
     *  Set flag to store start time.
     */

    first_fl = 1;

    /*
     *  Load cappi structure.
     */

    for  (i = 0; i < data.status.raycount; i++)
    {
        if  (data.volscan[i].raycount == 0 ||
             data.volscan[i].date == 0 ||
             data.volscan[i].azimuth == 0)   
            continue;
           
	ray = BYTESWAP (data.volscan[i].raycount) & 0x01FF;
	el = (BYTESWAP (data.volscan[i].raycount) & 0xFE00) >> 9;

        if  (el == 18 && ray == 0 ) continue;

        cappi.stat.date = data.volscan[i].date;
             
	bcdval = BYTESWAP (data.volscan[i].azimuth);	 
	CONVERT (&bcdval, &binval);
	az = (unsigned short) ( binval );	 
	if (az == 360) az = 0;
	if (az == azold) continue;
	azold = az;

        if  (first_fl == 1)
        {
            cappi.stat.bhour = data.volscan[i].hour;      
            cappi.stat.bmin = data.volscan[i].min;       
            first_fl = 0;
        }

	for  (n = 0; n < 224; n++)
	{
	    rvpc[n] = BYTESWAP (data.volscan[i].rvpc[n]);
	    rawvip = (unsigned short) (rvpc[n] / 8.);
	    ivip[el-1][az][n] = rawvip + 1200;
	}   
   } 

}


CONVERT (unsigned short *bcdval, float *binval)
{
   /*
    *  Conversion of bcd (binary coded decimal) format to integer.
    */

    unsigned short val_12, val_8, val_4, val_0;

    val_12 = (*bcdval >> 12) * 1000;
    val_8 = ((*bcdval >> 8) & 0x0F) * 100;
    val_4 = ((*bcdval >> 4) & 0x0F) * 10;
    val_0 = *bcdval & 0x0F;
    *binval = (val_12 + val_8 + val_4 + val_0) / 10.;
}


BYTESWAP (unsigned short *ndata)
{
   #define LSB(x) * (((unsigned char *) &x) + 1)
   #define MSB(x) * ((unsigned char *) &x) 

   unsigned short nword;

   MSB (nword) = LSB (ndata);
   LSB (nword) = MSB (ndata);

   return (nword);
}


PROCESS_CAPPI ()
{
   unsigned short i;

   SEND_STATUS ();

   for  (cappiheight = 1; cappiheight < number_of_cappis+1; cappiheight++)
   {
       printf ("%s - LOAD GRID FILE  %d \n", prog, cappiheight);

       sprintf (gridfile, "/home/marion/hp/grid_mrl5_b%02d", cappiheight);
       grid = fopen (gridfile, "r");

       fread(&height[cappiheight].pos, sizeof(height[cappiheight].pos), 1, grid);
       
       HORIZ ();
       VERTIGO (); 

       SEND_CAPPI ();

       fclose (grid);
   }

}



HORIZ ()
{
   unsigned short  i, k, n, kk, m, x[4], y[4], iy, ix, az[4];  
   short  j, ivip1, ivip2, nvip1, nvip2, nvip, mvip ;


   for (n = 0; n < ntel[cappiheight]; n++)
   {
       i = height[cappiheight].pos[n].ii - 1;    /* Bin number. */
       j = height[cappiheight].pos[n].ll - 1;	 /* Elevation step number from 1 - 18. */
       az[0] = height[cappiheight].pos[n].a1 - 1;    /* Azimuth. */
       az[1] = height[cappiheight].pos[n].a2 - 1;    /* Azimuth. */
       az[2] = height[cappiheight].pos[n].a3 - 1;    /* Azimuth. */
       az[3] = height[cappiheight].pos[n].a4 - 1;    /* Azimuth. */
       x[0] = height[cappiheight].pos[n].x1 - 1;
       y[0] = height[cappiheight].pos[n].y1 - 1;
       x[1] = height[cappiheight].pos[n].x2 - 1;
       y[1] = height[cappiheight].pos[n].y2 - 1;
       x[2] = height[cappiheight].pos[n].x3 - 1;
       y[2] = height[cappiheight].pos[n].y3 - 1;
       x[3] = height[cappiheight].pos[n].x4 - 1;
       y[3] = height[cappiheight].pos[n].y4 - 1;

     if (j == -1) continue;

     for (m = 0; m < 4; m++)
     {
       k = az[m];
       ix = x[m];
       iy = y[m];

       kk = k + 1;
       if ( kk == 360 ) kk = 0;

       /* 
	*  Average #1, azi at bin i+1 1200 vips offset. 
	*/

       ivip1 = ivip[j][kk][i+1] + height[cappiheight].pos[n].az_diff2;
       ivip2 = ivip[j][k][i+1] + height[cappiheight].pos[n].az_diff1;
       nvip = AVERAGE (&ivip1, &ivip2);
       nvip1 = nvip;

       /* 
	*  Average #2 , azi at bin i.
	*/

       ivip1 = ivip[j][kk][i] + height[cappiheight].pos[n].az_diff2;
       ivip2 = ivip[j][k][i]+ height[cappiheight].pos[n].az_diff1;
       nvip = AVERAGE (&ivip1, &ivip2);
       nvip2 = nvip;

       /* 
	*  Average #3 , range.
	*/

       ivip1 = nvip1 + height[cappiheight].pos[n].range_diff2;
       ivip2 = nvip2 + height[cappiheight].pos[n].range_diff1;
       nvip = AVERAGE (&ivip1, &ivip2);
       mvipa[iy][ix] = nvip; 

       /* 
	*  Average #1, azi at bin i+1 1200 vips offset. 
        *  NEXT ELEVATION.
	*/

       ivip1 = ivip[j+1][kk][i+1] + height[cappiheight].pos[n].az_diff2;
       ivip2 = ivip[j+1][k][i+1] + height[cappiheight].pos[n].az_diff1;
       nvip = AVERAGE (&ivip1, &ivip2);
       nvip1 = nvip;

       /* 
	*  Average #2 , azi at bin i.
	*/

       ivip1 = ivip[j+1][kk][i] + height[cappiheight].pos[n].az_diff2;
       ivip2 = ivip[j+1][k][i]+ height[cappiheight].pos[n].az_diff1;
       nvip = AVERAGE (&ivip1, &ivip2);
       nvip2 = nvip;

       /* 
	*  Average #3 , range.         
	*/

       ivip1 = nvip1 + height[cappiheight].pos[n].range_diff2;
       ivip2 = nvip2 + height[cappiheight].pos[n].range_diff1;
       nvip = AVERAGE (&ivip1, &ivip2);
       mvipb[iy][ix] = nvip; 

     }
   }  

}
	

VERTIGO ()
{

   unsigned short  ans, m, n, x[4], y[4], iy, ix;
   short  ncnt, ivip1, ivip2, nvip1, nvip2, nvip, mvip;


   for (n = 0; n < ntel[cappiheight]; n++)
   {
       x[0] = height[cappiheight].pos[n].x1 - 1;
       y[0] = height[cappiheight].pos[n].y1 - 1;
       x[1] = height[cappiheight].pos[n].x2 - 1;
       y[1] = height[cappiheight].pos[n].y2 - 1;
       x[2] = height[cappiheight].pos[n].x3 - 1;
       y[2] = height[cappiheight].pos[n].y3 - 1;
       x[3] = height[cappiheight].pos[n].x4 - 1;
       y[3] = height[cappiheight].pos[n].y4 - 1;

     for (m = 0; m < 4; m++)
     {
       ix = x[m];
       iy = y[m];

       /* 
	*  Average ( 3 horiz + 3 horiz ) + 1 vert = 7.
	*/

       ivip1 = mvipa[iy][ix] + height[cappiheight].pos[n].el_diff1;
       ivip2 = mvipb[iy][ix] + height[cappiheight].pos[n].el_diff2;
       nvip = AVERAGE (&ivip1, &ivip2);

       mvip = nvip - 1200;

       if (mvip > 100) 
       {
	   ans =  val[mvip] ;
	  /* ans = (unsigned short) ( 0.1 * idbz[mvip] + 0.5);*/
	   cappi.fin_dbz[iy][ix] = (char) ans;
       }

     }
   }

}



AVERAGE (short *ivip1, short *ivip2)
{
   short  ix, max, min, verskil, nvip;

   if (*ivip1 > *ivip2) 
   {
       verskil =  *ivip1 - *ivip2;
       max = *ivip1;
       min = *ivip2;
   }
   else 
   {
       verskil =  *ivip2 - *ivip1;
       max = *ivip2;
       min = *ivip1;
   }

   if (verskil <= 4095) 
       ix = verskil;      
   else 
   if (verskil > 4095)
       ix = 4095;

   nvip = max - idiff[ix];
   return (nvip);
}	


GET_STARTED ()
{

    SETUP_RECV_SOCKET (RECV_PORT, &recv_fd);
    SETUP_SEND_SOCKET ();

    LOAD_FILES ();

    binval = 0;
    azold = 0;
}


LOAD_FILES ()
{
   int i ;

   printf ("%s - LOAD TABLES    \n", prog);



  /*
   *  Grid file lengths 
   */

   len = fopen ( "/home/marion/hp/grid_lengths", "r");

   for (i = 1; i < number_of_cappis+2; i++)
   {
       fscanf(len,"%u", &ntel[i]);
    }



   /*
    *  Vip difference lookup table.
    */

   if  ((tab = fopen ("/home/marion/hp/tabel_new", "r")) == NULL)
   {
       printf ("%s - CAN'T OPEN tabel_new\n", prog);
       exit (1);
   }

   for (i = 0; i < 5001; i++)	
       fscanf (tab, "%u", &idiff[i]);
   fclose (tab);



   /*
    *  Vip to byte encoding table.
    */
   

   if  ((viptobyte = fopen ("/home/marion/hp/vip2byte", "r")) == NULL)
   {
       printf ("%s - CAN'T OPEN vip2byte\n", prog);
       exit (1);
   }

   for (i = 0; i < 4096; i++)	
       fscanf (viptobyte, "%u", &val[i]);
   fclose (viptobyte);


   /*
    *  Range correction table.
    */

   if  ((range = fopen ("/home/marion/hp/Range_corrections_new", "r")) == NULL)
   {
       printf ("%s - CAN'T OPEN Range_corrections\n", prog);
       exit (1);
   }

   for (i = 0; i < 224; i++) 		
       fscanf (range, "%d", &rngcor[i]);
   fclose (range);



   /*
    *  Vip-dbz conversion table.
    */ 

   if  ((vip2dbz = fopen ("/home/marion/hp/viptodbz_new", "r")) == NULL)
   {
       printf ("%s - CAN'T OPEN viptodbz_new\n", prog);
       exit (1);
   }

   for (i = 0; i < 4096; i++)		
       fscanf ( vip2dbz, "%u", &idbz[i]);
   fclose (vip2dbz);

   

}



SETUP_RECV_SOCKET (port, fd)
int port;
int *fd;
{
    struct  sockaddr_in sock_addr;
    struct  hostent *hp;
    struct  linger sl;
    int  return_code;
    int  sptr;
    int  reuse;

    printf ("%s - SETUP RECEIVE SOCKET\n", prog);

    sptr = *fd;

    /*
     *  Clear out address structure.
     */

    memset ((char *) &sock_addr, 0, sizeof (struct sockaddr_in));

    /*
     *  Get host information.
     */

    hp = gethostbyname (HOST);
    if (hp == NULL)
    {
	printf ("%s - %s not found in /etc/hosts\n", prog, HOST);
	exit (-1);
    }

    /*
     *  Try to connect to server until request accepted.
     */

    do
    {

	/*
	 *  Create the socket.
	 */

	if ((sptr = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
	    printf ("%s - CREATE SOCKET ERROR\n", prog);
	    exit (-1);
	}

	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = RECV_PORT; 
	sock_addr.sin_addr.s_addr = ((struct in_addr *) (hp->h_addr))->s_addr;

	reuse = 1;
	setsockopt (sptr, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof (reuse));
	sl.l_onoff = 0;
	setsockopt (sptr, SOL_SOCKET, SO_LINGER, (char *) &sl, sizeof (reuse));

	/*
	 *  Connect to remote server.
	 */

	if ((return_code = connect (sptr, (char *) &sock_addr, sizeof (sock_addr))) >= 0)
	{
	    printf ("%s - SERVER ACCEPTED CONNECT REQUEST\n", prog);
	    return_code = 1;
	    *fd = sptr;
	    return (CONNECTED);
	}
	else
	{
	    switch ( errno )
	    {
		case EADDRINUSE :
		printf ("%s - CONNECTING TO SERVER - EADDRINUSE\n", prog);
		break;

		case EBADF :
		printf ("%s - CONNECTING TO SERVER - EBADF\n", prog);
		break;

		case EINVAL :
		printf ("%s - CONNECTING TO SERVER - EINVAL\n", prog);
		break;

		case ENOTCONN :
		printf ("%s - CONNECTING TO SERVER - ENOTCONN\n", prog);
		break;

		case EPERM :
		printf ("%s - CONNECTING TO SERVER - EPERM\n", prog);
		break;

		case EOPNOTSUPP :
		printf ("%s - CONNECTING TO SERVER - EOPNOTSUPP\n", prog);
		break;

		case EWOULDBLOCK :
		printf ("%s - CONNECTING TO SERVER - EWOULDBLOCK\n", prog);
		break;

		case EINPROGRESS :
		printf ("%s - CONNECTING TO SERVER - EINPROGRESS\n", prog);
		break;

                /*
		default :
		printf ("%s - CONNECTING TO SERVER - %d %d\n", 
			prog, errno, return_code);
                */

	    }
	}

	close (sptr);

    } while (return_code < 0);

}



SETUP_SEND_SOCKET ()
{

   /* 
    *  Clear out address structures.
    */

    memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
    memset ((char *)&peeraddr_in, 0, sizeof(struct sockaddr_in));

   /* 
    *  Set up address structure for the listen socket. 
    */

    myaddr_in.sin_family = AF_INET;
    myaddr_in.sin_addr.s_addr = INADDR_ANY;

   /*
    *  Get the port number for this program.
    */

    sp = getservbyname ("wbfsoc", "tcp");
    if (sp == NULL) 
    {
        printf("%s - wbfsoc not found in /etc/services\n", prog);
	exit(1);
    }
    myaddr_in.sin_port = sp->s_port;

    /* 
     *  Create the listen socket. 
     */

    if ((ls = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
	printf("%s - Unable to create socket\n", prog);
	exit(1);
    }

    /* 
     *  Bind the listen address to the socket. 
     */

    if (bind (ls, &myaddr_in, sizeof(struct sockaddr_in)) < 0) 
    {
        printf("%s - Unable to bind address\n", prog);
        exit(1);
    }

    printf("%s - LISTEN CALL\n", prog);
    if (listen (ls, 5) < 0) 
    {
        printf("%s - NO CONNECT REQUESTS PRESENT\n", prog);
        send_sok = DISCONNECT;
        return (send_sok);
    }
 
    addrlen = sizeof(struct sockaddr_in);
    s = accept(ls, &peeraddr_in, &addrlen);
    if ( s == -1) exit(1);

    printf("%s - CLIENT ACCEPTED\n", prog);

    hp = gethostbyaddr ((char *) &peeraddr_in.sin_addr,
    sizeof (struct in_addr),
    peeraddr_in.sin_family);
    if (hp == NULL) 
    {
        printf ("%s - GET HOST ERROR\n", prog);
        return (CLIENT_OPEN_ERROR);
    }
    else 
        hostname = hp->h_name;	

    linger.l_onoff  =1;
    linger.l_linger =1;
    if (setsockopt (s, SOL_SOCKET, SO_LINGER, &linger,
    				sizeof(linger)) == -1) 
    {
        printf("%s - SETSOCKOPT ERROR\n", prog);
        return (CLIENT_OPEN_ERROR);
    }

    send_sok = CONNECTED;

}



SEND_CAPPI ()
{
    switch (send_sok)
    {    
	 case CONNECTED :
	     
	     send_sok = CAPPI_TO_SOCKET ();
	     break;

	 case DISCONNECT :
	 case OPEN_ERROR :
	 case CLIENT_OPEN_ERROR :
	 case ACCEPT_ERROR :
	 case SEND_ERROR :
	 case CLIENT_RECV_ERROR :
	    
	     close (ls);
	     close (s);
	     if ((send_sok = SETUP_SEND_SOCKET ()) == CONNECTED )
		  send_sok = CAPPI_TO_SOCKET ();
	      break; 
    }
}



SEND_STATUS ()
{
    short i, j, k;
    int   items;
    char  *bufptr;
    unsigned short  ihr, min, isec;
    unsigned short  btime, ftime;
    unsigned short  date;

    /*
     *  Edit data structure.
     */

    data.status.number_of_cappis = (unsigned short) number_of_cappis;
    cappi.stat.rccstat = data.status;                        
    cappi.stat.fhour = data.volscan[data.status.raycount-1].hour;       
    cappi.stat.fmin = data.volscan[data.status.raycount-1].min;           

    date = BYTESWAP (cappi.stat.date); 

    printf("%s - %u ", prog, date);

    ihr = BYTESWAP (cappi.stat.bhour); 
    min = BYTESWAP (cappi.stat.bmin);  
    isec = (unsigned short) fmod( (float) min, 60.);
    min = min / 60;
    btime = ihr*10000 + min*100 + isec;
    cappi.stat.bmin = BYTESWAP (min);
    cappi.stat.bsec = BYTESWAP (isec);
    printf("%u:%u:%u  ", ihr, min, isec);     

    ihr = BYTESWAP (cappi.stat.fhour);
    min = BYTESWAP (cappi.stat.fmin); 
    isec = (unsigned short) fmod( (float) min, 60.);
    min = min / 60;
    ftime = ihr*10000 + min*100 + isec;
    cappi.stat.fmin = BYTESWAP (min);
    cappi.stat.fsec = BYTESWAP (isec);
    printf("%u:%u:%u \n", ihr, min, isec);     

    /*
     *  Send status data.
     */

    if  ((items = send (s, (char *) &cappi.stat, sizeof (cappi.stat), 0)) != sizeof (cappi.stat)) 
    {
        printf ("%s - SEND ERROR STATUS %d errno:%d \n", prog, items, errno);
        return (SEND_ERROR);
    }
    
    printf ("%s - SEND STATUS %d\n", prog, items);

}



CAPPI_TO_SOCKET ()
{
    short i, j, k;
    int   items;
    char  *bufptr;

    /*
     *  Send cappi 201x201.
     */

    for ( i = 0; i < 201; i++ )
    {
        bufptr = (char *) &cappi.fin_dbz[i][0];

        if  ((items = send (s, bufptr, 201, 0)) != 201) 
        {
            printf ("%s - SEND ERROR CAPPI %d errno - %d \n", prog, items, errno);
            return (SEND_ERROR);
        }
    }

    printf ("%s - SEND CAPPI %d \n", prog, cappiheight);

    return (SEND_DATA_OK);
}
