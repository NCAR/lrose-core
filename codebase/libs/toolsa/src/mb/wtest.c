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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

#include <toolsa/mb.h>
#include <toolsa/umisc.h>

static void Set_timer (int sec);
int send_msg (char *message, int n_send, int fd);


int test0 (char *name);
int test1 (char *name);
int test2 (char *name);
int test3 (char *name);
int test4 (char *name);
int test5 (char *name);
void sigalrm_int();


/****************************************************************
*/

void main (int argc, char **argv)
{
static char name[128];
/*static char name[] = "/scratch/test_mb";*/
static int t_number;

    if (argc < 2) {
	printf ("Usage: wtest test_number\n");
	exit (0);
    }

    if (sscanf (argv[1], "%d", &t_number) < 1) {
	printf ("Can not read test_number. Usage: wtest test_number\n");
	exit (0);
    }

    if (argc == 3) strcpy (name, argv[2]);
    else strcpy (name, "test_mb");

    printf ("MB: %s\n", name);


    switch (t_number) {
	case 0:
	test0 (name);
	break;

	case 1:
	test1 (name);
	break;

	case 2:
	test2 (name);
	break;

	case 3:
	test3 (name);
	break;

	case 4:
	test4 (name);
	break;

	case 5:
	test5 (name);
	break;

	default:
	printf ("test %d not implemented\n", t_number);
	break;
    }

    exit (0);
}


/**************************************************************************
Test 0: Testing the correctness of the file usage
*/

int test0 (char *name)
{
    int fd;
    static char message [256];
    int cnt;
    unsigned int ret_id;

    printf ("Testing sequential write:\n");

    /* open an MB for writing. create it if needed. */
    fd = MB_open (name, MB_INIT | MB_SEQUENTIAL, 0666, 20, 30);

    if (fd < 0) {               /* open failed */
        printf ("MB_open failed. The return number is %d\n", fd);
        exit (-1);
    }


    cnt = 10;
    while (1) {
        int ret;
	int rad;
	int len;

	rad = rand() % 10;
	cnt++;

	sprintf (message, "This is a test message: %6d\n", cnt);
	len = strlen (message) + 1;    /* */
	/*
        ret = MB_write (fd, message + rad, len, MB_NEW_ID, &ret_id);
	*/
        ret = MB_write (fd, message, len, cnt, &ret_id);
/*
if (cnt % 31 == 0) 
    printf ("MB_clear = %d\n", MB_clear (fd));
*/
        if (ret > 0) {   /* success */
	    printf ("Message sent (id = %d): %d\n", ret_id, cnt); 
	    umsleep (300); 

	    continue;
        }
    

	printf ("MB_write failed. The return number = %d\n", ret);
	exit (-1);
    }

}


/**************************************************************************
Testing large messages
*/

#define LARGE_MSG_SIZE 100000

int test1 (char *name)
{
    int fd;
    static char message [LARGE_MSG_SIZE];
    int cnt;

    printf ("Testing large messages:\n");

    /* open an MB for writing. create it if needed. */
    fd = MB_open (name, MB_INIT | MB_SEQUENTIAL | MB_PARITY, 0666, 30000, 5);
    if (fd < 0) {               /* open failed */
        printf ("MB_open failed. The return number is %d\n", fd);
        exit (-1);
    }

    for (cnt = 0; cnt < LARGE_MSG_SIZE; cnt++) 
	message[cnt] = (char) (cnt % 256);

    cnt = 0;
    while (1) {
        int ret;
	int len;
	unsigned int ret_id;

	len = 30000;
	if (cnt % 5 == 4) len = 10003;
	if (cnt % 5 == 3) len = 1001;
	if (cnt % 5 == 2) len = 111;

        ret = MB_write (fd, message, len, MB_NEW_ID, &ret_id);
	cnt++;

        if (ret > 0) {   /* success */
	    printf ("Message sent: %d. len = %d, id = %d\n", cnt, len, ret_id); 
	    umsleep (2000); 
	    continue;
        }
    

        if (ret == MB_FULL) {    /* MB is full. We will retry */
/*	    printf ("MB is full\n"); */
	    umsleep (100); 
	    continue;
        }

        else {                     /* other errors */
	    printf ("MB_write failed. The return number = %d\n", ret);
	    exit (-1);
        }
    }

}

/**************************************************************************
Testing interupts
*/

int test2 (char *name)
{
    extern void sigalrm_int();

    printf ("Testing interupts\n");

    signal (SIGALRM, sigalrm_int); 
    Set_timer (50);  

    test1 (name);
    return (0);
}


/**************************************************************************
Test 3: Testing critical message sizes
*/

int test3 (char *name)
{
    int fd;
    static char message [256];
    int cnt, ret;
    MB_status mb_st_buf;

    printf ("Testing critical message sizes:\n");

    /* open an MB for writing. create it if needed. */
    fd = MB_open (name, MB_INIT | MB_SEQUENTIAL | MB_MUST_READ | MB_PARITY, 0666, 32, 2);
    if (fd < 0) {               /* open failed */
        printf ("MB_open failed. The return number is %d\n", fd);
        exit (-1);
    }
    ret = MB_stat (fd, &mb_st_buf);
    if (ret < 0) {               /* open failed */
        printf ("MB_stat failed. The return number is %d\n", fd);
        exit (-1);
    }
    printf ("size = %d\n", mb_st_buf.size);
    printf ("maxn_msgs = %d\n", mb_st_buf.maxn_msgs);
    printf ("msg_size = %d\n", mb_st_buf.msg_size);
    printf ("perm = %d\n", mb_st_buf.perm);
    printf ("flags = %d\n", mb_st_buf.flags);
    printf ("n_msgs = %d\n", mb_st_buf.n_msgs);
    printf ("latest_id = %d\n", mb_st_buf.latest_id);


    cnt = 0;
    printf ("Testing 32 byte message:\n");
    sprintf (message, "This is a test message: %6d\n", cnt);
    send_msg (message, 40, fd);

    cnt++;
    printf ("Testing 33 byte message:\n");
    sprintf (message, "This is a test message$: %6d\n", cnt);
    send_msg (message, 20, fd);

    cnt++;
    printf ("Testing 31 byte message:\n");
    sprintf (message, "This is a test message: %5d\n", cnt);
    send_msg (message, 40, fd);

    cnt++;
    printf ("Testing 28 byte message:\n");
    sprintf (message, "This is a  message: %6d\n", cnt);
    send_msg (message, 40, fd);

    cnt++;
    printf ("Testing 29 byte message:\n");
    sprintf (message, "This is a  message?: %6d\n", cnt);
    send_msg (message, 40, fd);

    cnt++;
    printf ("Testing 30 byte message:\n");
    sprintf (message, "This is a  message&?: %6d\n", cnt);
    send_msg (message, 40, fd);

    cnt++;
    printf ("Testing 24 byte message:\n");
    sprintf (message, "This is message: %5d\n", cnt);
    send_msg (message, 40, fd);

    cnt++;
    printf ("Testing 23 byte message:\n");
    sprintf (message, "This message@@: %5d\n", cnt);
    send_msg (message, 40, fd);
    
    exit (0);
}

/**************************************************************************
Test 4: Testing random write
*/

int test4 (char *name)
{
    int fd;
    static char message [256];
    int cnt;

    printf ("Testing random message write:\n");

    /* open an MB for writing. create it if needed. */
    fd = MB_open (name, MB_INIT | MB_PARITY, 0666, 25, 100);
    if (fd < 0) {               /* open failed */
        printf ("MB_open failed. The return number is %d\n", fd);
        exit (-1);
    }

    cnt = 0;
    while (1) {
        int ret;
	int rad;
	int len;
	int id;
	unsigned int ret_id;

	rad = rand() % 10;
	id = rand() * 10 + rand ();

	sprintf (message, "This  message: id = %d: %6d\n", id, cnt);
	len = strlen (message + rad) + 1;    /* */

        ret = MB_write (fd, message + rad, len, id, &ret_id);
	cnt++;

	if (cnt %1000 == 0) 
	    printf ("%d messages sent\n", cnt); 

        if (ret > 0) {   /* success */
/*	    printf ("Message sent (id = %d): %d\n", ret_id, cnt); 
	    umsleep (300); 
*/
	    continue;
        }
        else {                     /* other errors */
	    printf ("MB_write failed. The return number = %d\n", ret);
        }
    }
    return (0);
}


/**************************************************************************
Test 5: Testing replaceable write
*/

int test5 (char *name)
{
    int fd;
    static char message [256];
    int cnt;
    unsigned int ret_id;

    printf ("Testing replaceable mb:\n");

    /* open an MB for writing. create it if needed. */
    fd = MB_open (name, MB_INIT | MB_REPLACE | MB_PARITY, 0666, 128, 4);
    if (fd < 0) {               /* open failed */
        printf ("MB_open failed. The return number is %d\n", fd);
        exit (-1);
    }

    printf ("write id = 0 (ret = %d)\n", 
	MB_write (fd, message, 128, 2444, &ret_id));

    cnt = 0;
    while (1) {
        int ret;
	int rad;
	int len;

	rad = rand() % 10;
	cnt++;

	sprintf (message, "This is a test message: %6d\n", cnt);
	len = strlen (message + rad) + 1;    /* */

        ret = MB_write (fd, message + rad, 128, 23, &ret_id);

        if (ret > 0) {   /* success */
	    printf ("Message sent (id = %d): %d\n", ret_id, cnt); 
	    umsleep (300); 
	    continue;
        }
    

        if (ret == MB_FULL) {    /* MB is full. We will retry */
	    printf ("MB is full\n"); 
	    umsleep (300); 
	    continue;
        }

        else {                     /* other errors */
	    printf ("MB_write failed. The return number = %d\n", ret);
	    exit (-1);
        }
    }

}

/**************************************************************************
*/

   
int send_msg (char *message, int n_send, int fd)
{
    int cnt;

    cnt = 0;
    while (1) {
        int ret;
	int len;
	unsigned int ret_id;

	len = strlen (message) + 1;    /* */

        ret = MB_write (fd, message, len, MB_NEW_ID, &ret_id);

        if (ret > 0) {   /* success */
	    printf ("Message sent: id = %d\n", ret_id); 
	    cnt++;
	    if (cnt >= n_send) return (0);
	    umsleep (300); 
	    continue;
        }
    

        if (ret == MB_FULL) {    /* MB is full. We will retry */
	    printf ("MB is full\n"); 
	    umsleep (300); 
	    continue;
        }

        else {                     /* other errors */
	    printf ("MB_write failed. The return number = %d\n", ret);
	    cnt++;
	    if (cnt >= n_send) return (0);
	    umsleep (300); 
        }
    }
    return (0);
}




#include <sys/types.h>
#include <sys/time.h>



/**********************************************************************
*/

void sigalrm_int()
{
static int cnt = 0;

    if (cnt % 10 == 0) printf ("timer called %d times\n", cnt);
    cnt++;

    signal (SIGALRM, sigalrm_int); 

    return;
}


/******************************************************************

	Set_timer ()			Date: 2/16/94

	This function sets the UNIX timer. It causes UNIX to send 
	back a SIGALRM signal every "sec" seconds if sec > 0. It 
	stops the timer if sec = 0.

	Returns: This function has no return value.
*/

static void
  Set_timer
  (
      int sec			/* The timer period in seconds */
) {
    struct itimerval value;

    value.it_value.tv_sec = 0;
    value.it_interval.tv_sec = 0;
    value.it_value.tv_usec = sec * 1000;
    value.it_interval.tv_usec = sec * 1000;
    setitimer (ITIMER_REAL, &value, NULL);

    return;
}

