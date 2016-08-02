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
#include <toolsa/mb.h>
#include <toolsa/umisc.h>

int test0 (char *name);
int test1 (char *name);
int test2 (char *name);
int test3 (char *name);
int test4 (char *name);
int test5 (char *name);


/*********************************************************************
*/

void main (int argc, char **argv)
{
static char name[128];
/*static char name[] = "/scratch/test_mb"; */
static int t_number;

    if (argc < 2) {
	printf ("Usage: rtest test_number\n");
	exit (0);
    }

    if (sscanf (argv[1], "%d", &t_number) < 1) {
	printf ("Can not read test_number. Usage: rtest test_number\n");
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


/********************************************************************
Test 0: Testing sequential read
*/

int test0 (char *name)
{
    int fd;
    char message [256];
    int cnt;

    /* open an MB for reading */
    fd = MB_open (name, MB_SEQUENTIAL, 0, 0, 0);
    if (fd < 0) {               /* open failed */
	printf ("MB_open failed. The error number is %d\n", fd);
	exit (-1);
    }

    cnt = 0;
    while (1) {
	int ret;
	unsigned int ret_id;

	ret = MB_read (fd, message, 128, MB_CURRENT, &ret_id);

	if (ret > 0) {  /* success */
	    message[ret - 1] = '\0';
	    printf ("Message read (%d, id = %d): %s\n", ret, ret_id, message); 
	    cnt++;
	    continue;
	}

	if (ret == MB_NOT_FOUND) {    /* MB is empty. We will retry */
	    printf ("MB is empty\n"); 
	    umsleep (200);  
	    continue;
	}

	else {                     /* other errors */
	    printf ("MB_read failed. The return number = %d\n", ret);
	    exit (-1);
	}
    }
	
}

/********************************************************************
Test 1: Testing large messages
*/

#define LARGE_MSG_SIZE 100000

int test1 (char *name)
{
    int fd;
    static char message [LARGE_MSG_SIZE];
    static char true_message [LARGE_MSG_SIZE];
    int cnt;

    for (cnt = 0; cnt < LARGE_MSG_SIZE; cnt++) 
	true_message[cnt] = (char)(cnt % 256);

    /* open an MB for reading. */
    fd = MB_open (name, MB_SEQUENTIAL  | MB_PARITY, 0, 0, 0);
    if (fd < 0) {               /* open failed */
	printf ("MB_open failed. The error number is %d\n", fd);
	exit (-1);
    }

    cnt = 0;
    while (1) {
	int ret;
	int i;
	unsigned int ret_id;

	ret = MB_read (fd, message, LARGE_MSG_SIZE, MB_CURRENT, &ret_id);

	if (ret > 0) {  /* success */
	    printf ("Message read (len = %d, id = %d)\n", ret, ret_id); 
    	        for (i = 0; i < ret; i++) {
		    if (true_message[i] != message[i]) {
		        printf ("error found at %d. cnt = %d\n", i, cnt);
		        exit (0);
		    }
		}
	    cnt++;
	    continue;
	}

	if (ret == 0) {    /* MB is empty. We will retry */
/*	    printf ("MB is empty\n"); */
	    umsleep (200);  
	    continue;
	}

	else {                     /* other errors */
	    printf ("MB_read failed. The return number = %d\n", ret);
	    exit (-1);
	}
    }
	
}


/**************************************************************************
Testing interupts
*/

int test2 (char *name)
{

    test1 (name);
    return (0);

}

/********************************************************************
Test 3: Testing critical message sizes
*/

int test3 (char *name)
{
    int fd;
    char message [256];
    int cnt;

    /* open an MB for reading. */
    fd = MB_open (name, MB_SEQUENTIAL | MB_MUST_READ  | MB_PARITY, 0, 0, 0);
    if (fd < 0) {               /* open failed */
	printf ("MB_open failed. The error number is %d\n", fd);
	exit (-1);
    }
    printf ("MB of 97 bytes opened\n");

    cnt = 0;
    while (1) {
	int ret;
	unsigned int ret_id;

	ret = MB_read (fd, message, 128, MB_CURRENT, &ret_id);

	if (ret > 0) {  /* success */
		message[ret - 1] = '\0';
		printf ("Message read (%d, id = %d): %s\n", ret, ret_id, message); 
	    cnt++;
	    continue;
	}

	if (ret == 0) {    /* MB is empty. We will retry */
	    printf ("MB is empty\n"); 
	    umsleep (200);  
	    continue;
	}

	else {                     /* other errors */
	    printf ("MB_read failed. The return number = %d\n", ret);
	    exit (-1);
	}
    }
	
}


/********************************************************************
Test 4: Testing random write/read
*/

int test4 (char *name)
{
    int fd;
    char message [256];
    MB_List list[16];
    int i, ret;

    /* open an MB for reading. */
    fd = MB_open (name, MB_PARITY, 0666, 25, 100);
    if (fd < 0) {               /* open failed */
	printf ("MB_open failed. The error number is %d\n", fd);
	exit (-1);
    }

    ret = MB_get_list (fd, list, 16, MB_LARGEST);
    if (ret < 0) {
	printf ("MB_get_list failed. The error number is %d\n", ret);
	exit (-1);
    }
    for (i=0; i<ret; i++){
	int r;
	unsigned int ret_id;

	printf ("	List %d: id = %d, size = %d\n", i, list[i].id, list[i].size);
	r = MB_read (fd, message, 128, list[i].id, &ret_id);
	if (r <= 0) 
	    printf ("Reading msg (id = %d) failed (ret = %d)\n", list[i].id, r);
	else {
	    printf ("Message (%d) read (ret = %d, id = %d): %s\n", list[i].id, 
			r, ret_id, message); 
	}
    }
    return (0);
}


/********************************************************************
Test 5: Testing replaceable read
*/

int test5 (char *name)
{
    int fd;
    char message [256];
    int cnt;

    /* open an MB for reading */
    fd = MB_open (name, MB_REPLACE | MB_PARITY, 0666, 128, 4);
    if (fd < 0) {               /* open failed */
	printf ("MB_open failed. The error number is %d\n", fd);
	exit (-1);
    }

    cnt = 0;
    while (1) {
	int ret;
	unsigned int ret_id;

	ret = MB_read (fd, message, 128, 23, &ret_id);

	if (ret > 0) {  /* success */
	    message[ret - 1] = '\0';
	    printf ("Message read (%d, id = %d): %s\n", ret, ret_id, message); 
	    cnt++;
	    umsleep (100);
	    continue;
	}

	if (ret == MB_NOT_FOUND) {    /* MB is empty. We will retry */
	    printf ("MB is empty\n"); 
	    umsleep (200);  
	    continue;
	}

	else {                     /* other errors */
	    printf ("MB_read failed. The return number = %d\n", ret);
	    exit (-1);
	}
    }
	
}


